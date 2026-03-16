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

// PIB (Parameter Information Block) Manager for QCA7000/7005 PLC modules.
// Allows reading and writing the PIB over the existing SPI connection.
//
// Protocol: VS_MODULE_OPERATION (0xA0B0/0xA0B1) with fragmented transfers.

#pragma once

#include <cstdint>
#include <cstddef>

#include <TFTools/Micros.h>

// Maximum PIB size we support (QCA7000 PIBs are typically ~8-10 KB)
#define PIB_BUFFER_SIZE 16384

// Minimum PIB size (basic header)
#define PIB_HEADER_SIZE              12

enum class PibState : uint8_t {
    Idle,

    // Read states
    ReadSendRequest,
    ReadWaitConfirmation,
    ReadComplete,
    ReadError,

    // Write states
    WriteSessionSendRequest,
    WriteSessionWaitConfirmation,
    WriteFragmentSendRequest,
    WriteFragmentWaitConfirmation,
    WriteCommitSendRequest,
    WriteCommitWaitConfirmation,
    WriteResetSendRequest,
    WriteComplete,
    WriteError,
};

class PibManager final
{
public:
    PibManager() {}

    // Called from SLAC::handle_tap() when a VS_MODULE_OPERATION.CNF (0xA0B1) is received.
    void handle_confirmation(const uint8_t *data, size_t length);

    // Start a PIB read operation (called from http thread)
    // Returns false if busy.
    bool start_read();

    // Start a PIB write operation with data already in the buffer (called from http thread).
    // Returns false if busy.
    bool start_write();

    // Copy PIB data into the internal buffer for writing (called from http thread).
    // Returns false if size exceeds PIB_BUFFER_SIZE or if busy.
    bool set_write_data(const uint8_t *data, size_t size);

    PibState get_state() const { return state; }
    bool is_busy() const { return state != PibState::Idle && state != PibState::ReadComplete && state != PibState::ReadError && state != PibState::WriteComplete && state != PibState::WriteError; }
    const uint8_t *get_buffer() const { return pib_buffer; }
    size_t get_buffer_length() const { return pib_length; }
    const char *get_error() const { return error_message; }
    void reset() { state = PibState::Idle; error_message = nullptr; }

    [[gnu::const]] static const char *validate_pib_size(size_t size);

private:
    void state_machine_loop();
    void schedule_task();

    void send_read_request();
    void send_session_request();
    void send_write_fragment();
    void send_commit_request();
    void send_reset_request();

    void handle_read_confirmation(const uint8_t *data, size_t length);
    void handle_session_confirmation(const uint8_t *data, size_t length);
    void handle_write_confirmation(const uint8_t *data, size_t length);
    void handle_commit_confirmation(const uint8_t *data, size_t length);

    void set_error(const char *msg);

    static uint32_t fdchecksum32(const uint8_t *data, size_t size);

    PibState state = PibState::Idle;

    uint8_t *pib_buffer = nullptr;
    size_t pib_length = 0;      // Current valid data length in buffer

    // Transfer tracking
    size_t transfer_offset = 0; // Current byte offset within the PIB
    uint32_t session_id = 0;    // Random session ID for write operations
    micros_t timeout_deadline = 0_us;

    const char *error_message = nullptr;
    uint64_t task_id = 0;
};
