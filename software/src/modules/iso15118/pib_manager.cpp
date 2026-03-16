/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "pib_manager.h"

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"

#include "slac.h"
#include "tools/malloc.h"

#include "gcc_warnings.h"

static constexpr seconds_t PIB_READ_TIMEOUT    =  5_s;
static constexpr seconds_t PIB_SESSION_TIMEOUT = 60_s;
static constexpr seconds_t PIB_WRITE_TIMEOUT   = 90_s;
static constexpr seconds_t PIB_COMMIT_TIMEOUT  = 90_s;

// PIB up/download will only be used during testing and provisioning.
// Just allocate buffer lazily in PSRAM.
static uint8_t *ensure_buffer(uint8_t *&buf)
{
    if (buf == nullptr) {
        buf = static_cast<uint8_t *>(calloc_psram_or_dram(PIB_BUFFER_SIZE, 1));
    }
    return buf;
}

// fdchecksum32: 32-bit ones' complement checksum (from open-plc-utils)
uint32_t PibManager::fdchecksum32(const uint8_t *data, size_t size)
{
    uint32_t checksum = 0;
    // Process 4 bytes at a time
    while (size >= 4) {
        uint32_t val = static_cast<uint32_t>(data[0])
                     | (static_cast<uint32_t>(data[1]) << 8)
                     | (static_cast<uint32_t>(data[2]) << 16)
                     | (static_cast<uint32_t>(data[3]) << 24);
        checksum ^= val;
        data += 4;
        size -= 4;
    }
    // Process remaining bytes
    if (size > 0) {
        uint32_t val = 0;
        for (size_t i = 0; i < size; i++) {
            val |= static_cast<uint32_t>(data[i]) << (i * 8);
        }
        checksum ^= val;
    }
    return ~checksum;
}

[[gnu::const]]
const char *PibManager::validate_pib_size(size_t size)
{
    if (size < PIB_HEADER_SIZE) {
        return "PIB too small (< 12 bytes header)";
    }

    if ((size % 4) != 0) {
        return "PIB size must be a multiple of 4 bytes";
    }

    return nullptr;
}

void PibManager::set_error(const char *msg)
{
    error_message = msg;
    logger.printfln("PibManager: %s", msg);
}

bool PibManager::start_read()
{
    if (is_busy()) {
        return false;
    }

    if (!iso15118.qca700x.is_spi_ready()) {
        set_error("QCA700x SPI not initialized");
        return false;
    }

    if (ensure_buffer(pib_buffer) == nullptr) {
        return false;
    }

    pib_length = 0;
    transfer_offset = 0;
    error_message = nullptr;
    timeout_deadline = 0_us;
    state = PibState::ReadSendRequest;
    schedule_task();
    return true;
}

bool PibManager::set_write_data(const uint8_t *data, size_t size)
{
    if (is_busy()) {
        return false;
    }

    if ((size > PIB_BUFFER_SIZE) || (size == 0)) {
        return false;
    }

    if (ensure_buffer(pib_buffer) == nullptr) {
        return false;
    }

    memcpy(pib_buffer, data, size);
    pib_length = size;
    return true;
}

bool PibManager::start_write()
{
    if (is_busy()) {
        return false;
    }

    if (!iso15118.qca700x.is_spi_ready()) {
        set_error("QCA700x SPI not initialized");
        return false;
    }

    if ((pib_buffer == nullptr) || (pib_length == 0)) {
        return false;
    }

    // Validate PIB before starting write
    const char *err = validate_pib_size(pib_length);
    if (err != nullptr) {
        set_error(err);
        state = PibState::WriteError;
        return false;
    }

    transfer_offset = 0;
    error_message = nullptr;
    timeout_deadline = 0_us;

    // Initiate random session ID
    session_id = esp_random();

    state = PibState::WriteSessionSendRequest;
    schedule_task();
    return true;
}

void PibManager::schedule_task()
{
    (void)task_scheduler.scheduleWithFixedDelay([this]() {
        state_machine_loop();
    }, 20_ms);
}

void PibManager::state_machine_loop()
{
    // Check for timeouts
    if (is_busy() && (timeout_deadline != 0_us) && deadline_elapsed(timeout_deadline)) {
        set_error("Operation timed out");
        if (state >= PibState::ReadSendRequest && state <= PibState::ReadWaitConfirmation) {
            state = PibState::ReadError;
        } else {
            state = PibState::WriteError;
        }
        task_scheduler.cancel(task_scheduler.currentTaskId());
        return;
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (state) {
        case PibState::ReadSendRequest:
            send_read_request();
            break;
        case PibState::WriteSessionSendRequest:
            send_session_request();
            break;
        case PibState::WriteFragmentSendRequest:
            send_write_fragment();
            break;
        case PibState::WriteCommitSendRequest:
            send_commit_request();
            break;
        case PibState::WriteResetSendRequest:
            send_reset_request();
            break;
        case PibState::ReadComplete:
        case PibState::ReadError:
        case PibState::WriteComplete:
        case PibState::WriteError:
            // Terminal state reached, self-cancel the periodic task
            task_scheduler.cancel(task_scheduler.currentTaskId());
            break;
        default:
            break;
    }
#pragma GCC diagnostic pop
}

void PibManager::send_read_request()
{
    VS_ModuleOperationReadRequest req = {};
    iso15118.slac.fill_header_v0(&req.header, slac_mac_plc_peer, iso15118.slac.evse_mac,
                                  SLAC_MMTYPE_QUALCOMM_MODULE_OP | SLAC_MMTYPE_MODE_REQUEST);
    req.mod_op        = PIB_MOD_OP_READ_MEMORY;
    req.module_id     = PIB_MODULE_ID_PARAMETERS;
    req.module_length = PIB_MODULE_SIZE;
    req.module_offset = static_cast<uint32_t>(transfer_offset);

    logger.printfln("PIB: Sending read request (offset=%u, frame_size=%zu)",
                    static_cast<unsigned>(transfer_offset), sizeof(req));
    iso15118.qca700x.write_burst(reinterpret_cast<const uint8_t *>(&req), sizeof(req));

    timeout_deadline = calculate_deadline(PIB_READ_TIMEOUT);
    state = PibState::ReadWaitConfirmation;
}

void PibManager::send_session_request()
{
    VS_ModuleOperationSessionRequest req = {};
    iso15118.slac.fill_header_v0(&req.header, slac_mac_plc_peer, iso15118.slac.evse_mac,
                                  SLAC_MMTYPE_QUALCOMM_MODULE_OP | SLAC_MMTYPE_MODE_REQUEST);
    req.session_id                = session_id;
    req.module_spec.module_id     = PIB_MODULE_ID_PARAMETERS;
    req.module_spec.module_sub_id = 0;
    req.module_spec.module_length = static_cast<uint32_t>(pib_length);
    req.module_spec.module_chksum = fdchecksum32(pib_buffer, pib_length);

    iso15118.qca700x.write_burst(reinterpret_cast<const uint8_t *>(&req), sizeof(req));

    timeout_deadline = calculate_deadline(PIB_SESSION_TIMEOUT);
    state = PibState::WriteSessionWaitConfirmation;
}

void PibManager::send_write_fragment()
{
    size_t remaining = pib_length - transfer_offset;
    size_t chunk_size = remaining > PIB_MODULE_SIZE ? PIB_MODULE_SIZE : remaining;

    VS_ModuleOperationWriteRequest req = {};
    iso15118.slac.fill_header_v0(&req.header, slac_mac_plc_peer, iso15118.slac.evse_mac,
                                  SLAC_MMTYPE_QUALCOMM_MODULE_OP | SLAC_MMTYPE_MODE_REQUEST);
    req.mod_op_data_len = static_cast<uint16_t>(19 + chunk_size);
    req.session_id      = session_id;
    req.module_id       = PIB_MODULE_ID_PARAMETERS;
    req.module_length   = static_cast<uint16_t>(chunk_size);
    req.module_offset   = static_cast<uint32_t>(transfer_offset);
    memcpy(req.module_data, pib_buffer + transfer_offset, chunk_size);

    // Send the full struct (includes the full 1400-byte data array, to match the plctool behavior)
    iso15118.qca700x.write_burst(reinterpret_cast<const uint8_t *>(&req), sizeof(req));

    timeout_deadline = calculate_deadline(PIB_WRITE_TIMEOUT);
    state = PibState::WriteFragmentWaitConfirmation;
}

void PibManager::send_commit_request()
{
    VS_ModuleOperationCommitRequest req = {};
    iso15118.slac.fill_header_v0(&req.header, slac_mac_plc_peer, iso15118.slac.evse_mac,
                                  SLAC_MMTYPE_QUALCOMM_MODULE_OP | SLAC_MMTYPE_MODE_REQUEST);
    req.session_id = session_id;

    iso15118.qca700x.write_burst(reinterpret_cast<const uint8_t *>(&req), sizeof(req));

    timeout_deadline = calculate_deadline(PIB_COMMIT_TIMEOUT);
    state = PibState::WriteCommitWaitConfirmation;
}

void PibManager::send_reset_request()
{
    CM_QualcommResetDeviceRequest req = {};
    iso15118.slac.fill_header_v0(&req.header, slac_mac_plc_peer, iso15118.slac.evse_mac,
                                  SLAC_MMTYPE_QUALCOMM_CM_RESET_DEVICE | SLAC_MMTYPE_MODE_REQUEST);

    iso15118.qca700x.write_burst(reinterpret_cast<const uint8_t *>(&req), sizeof(req));

    // The modem resets immediately upon receiving this command and will not
    // send a confirmation back over SPI.
    logger.printfln("PIB write complete (modem reset sent)");
    state = PibState::WriteComplete;
}

void PibManager::handle_confirmation(const uint8_t *data, size_t length)
{
    // Minimum size: header(14) + mmv(1) + mmtype(2) + oui(3) + mstatus(2) = 22
    // For read CNF we need the mod_op field to distinguish, which is at OUI+9
    if (length < 22) {
        logger.printfln("PIB: Confirmation too short (%u bytes)", static_cast<unsigned>(length));
        return;
    }

    logger.printfln("PIB: Received confirmation (%u bytes, state=%u)",
                    static_cast<unsigned>(length), static_cast<unsigned>(state));

    // The mod_op field tells us which sub-operation this confirms.
    // In a confirmation, the layout after OUI is:
    //   mstatus(2) + err_rec_code(2) + reserved(4) + num_op_data(1) + mod_op(2)
    // That's at offset OUI+9 from the OUI start, which is header(14)+mmv(1)+mmtype(2)+OUI(3)+9 = offset 29
    // But we receive the raw frame, so parse from the struct offsets directly.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
    switch (state) {
        case PibState::ReadWaitConfirmation:
            handle_read_confirmation(data, length);
            break;
        case PibState::WriteSessionWaitConfirmation:
            handle_session_confirmation(data, length);
            break;
        case PibState::WriteFragmentWaitConfirmation:
            handle_write_confirmation(data, length);
            break;
        case PibState::WriteCommitWaitConfirmation:
            handle_commit_confirmation(data, length);
            break;
        default:
            // Unexpected confirmation, ignore
            break;
    }
#pragma GCC diagnostic pop
}

void PibManager::handle_read_confirmation(const uint8_t *data, size_t length)
{
    if (length < sizeof(VS_ModuleOperationReadConfirmation) - PIB_MODULE_SIZE) {
        set_error("Read confirmation too short");
        state = PibState::ReadError;
        return;
    }

    const auto *cnf = reinterpret_cast<const VS_ModuleOperationReadConfirmation *>(data);

    if (cnf->mstatus != 0) {
        set_error("Read confirmation: modem returned error");
        state = PibState::ReadError;
        return;
    }

    uint16_t frag_len    = cnf->module_length;
    uint32_t frag_offset = cnf->module_offset;

    if (frag_offset != transfer_offset) {
        set_error("Read confirmation: unexpected offset");
        state = PibState::ReadError;
        return;
    }

    if (transfer_offset + frag_len > PIB_BUFFER_SIZE) {
        set_error("Read confirmation: PIB exceeds buffer size");
        state = PibState::ReadError;
        return;
    }

    // Verify we have enough data in the frame
    size_t expected_size = sizeof(VS_ModuleOperationReadConfirmation) - PIB_MODULE_SIZE + frag_len;
    if (length < expected_size) {
        set_error("Read confirmation: frame too short for declared data length");
        state = PibState::ReadError;
        return;
    }

    memcpy(pib_buffer + transfer_offset, cnf->module_data, frag_len);
    transfer_offset += frag_len;
    pib_length = transfer_offset;

    if (frag_len < PIB_MODULE_SIZE) {
        // Last fragment, read complete
        logger.printfln("PIB read complete: %u bytes", static_cast<unsigned>(pib_length));
        state = PibState::ReadComplete;
    } else {
        // More data to read
        state = PibState::ReadSendRequest;
    }
}

void PibManager::handle_session_confirmation(const uint8_t *data, size_t length)
{
    if (length < sizeof(VS_ModuleOperationSessionConfirmation)) {
        set_error("Session confirmation too short");
        state = PibState::WriteError;
        return;
    }

    const auto *cnf = reinterpret_cast<const VS_ModuleOperationSessionConfirmation *>(data);

    if (cnf->mstatus != 0) {
        set_error("Session start failed: modem returned error");
        state = PibState::WriteError;
        return;
    }

    if (cnf->session_id != session_id) {
        set_error("Session confirmation: session ID mismatch");
        state = PibState::WriteError;
        return;
    }

    if (cnf->module_status.mod_status != 0) {
        set_error("Session confirmation: module status error");
        state = PibState::WriteError;
        return;
    }

    logger.printfln("PIB write session started (session_id=%08lx)", session_id);

    // Start writing fragments
    transfer_offset = 0;
    state = PibState::WriteFragmentSendRequest;
}

void PibManager::handle_write_confirmation(const uint8_t *data, size_t length)
{
    if (length < sizeof(VS_ModuleOperationWriteConfirmation)) {
        set_error("Write confirmation too short");
        state = PibState::WriteError;
        return;
    }

    const auto *cnf = reinterpret_cast<const VS_ModuleOperationWriteConfirmation *>(data);

    if (cnf->mstatus != 0) {
        set_error("Write fragment failed: modem returned error");
        state = PibState::WriteError;
        return;
    }

    if (cnf->session_id != session_id) {
        set_error("Write confirmation: session ID mismatch");
        state = PibState::WriteError;
        return;
    }

    transfer_offset += cnf->module_length;

    if (transfer_offset >= pib_length) {
        // All fragments written, commit
        logger.printfln("PIB write fragments complete (%u bytes), committing", static_cast<unsigned>(pib_length));
        state = PibState::WriteCommitSendRequest;
    } else {
        // More fragments to write
        state = PibState::WriteFragmentSendRequest;
    }
}

void PibManager::handle_commit_confirmation(const uint8_t *data, size_t length)
{
    if (length < sizeof(VS_ModuleOperationCommitConfirmation)) {
        set_error("Commit confirmation too short");
        state = PibState::WriteError;
        return;
    }

    const auto *cnf = reinterpret_cast<const VS_ModuleOperationCommitConfirmation *>(data);

    if (cnf->mstatus != 0) {
        logger.printfln("PIB: Commit failed: mstatus=0x%04X, err_rec_code=0x%04X",
                        static_cast<unsigned>(cnf->mstatus), static_cast<unsigned>(cnf->err_rec_code));
        set_error("Commit failed: modem returned error");
        state = PibState::WriteError;
        return;
    }

    if (cnf->session_id != session_id) {
        set_error("Commit confirmation: session ID mismatch");
        state = PibState::WriteError;
        return;
    }

    logger.printfln("PIB commit successful, resetting modem");

    // Reset the QCA modem after successful commit
    state = PibState::WriteResetSendRequest;
}
