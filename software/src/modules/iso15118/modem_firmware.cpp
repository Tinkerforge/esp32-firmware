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

#include "modem_firmware.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "generated/qca_firmware.h"

#include "slac.h"
#include "pib_manager.h"

#include "gcc_warnings.h"

#if !QCA_FIRMWARE_AVAILABLE
#define QCA_FIRMWARE_VERSION ""
#endif

// Writes per boot. A failed or unverifiable write is retried once.
static constexpr uint8_t  MAX_FIRMWARE_WRITES               = 2;
static constexpr uint8_t  MAX_PIB_WRITES                    = 2;
static constexpr uint8_t  VERSION_REQUEST_TRIES             = 3;
static constexpr millis_t VERSION_REQUEST_TIMEOUT           = 2000_ms;

// Version strings seen so far are up to 40 (modem) and 23 (embedded) characters long.
static constexpr uint16_t MODEM_VERSION_API_LENGTH          = 64;
static constexpr uint16_t EMBEDDED_VERSION_API_LENGTH       = 32;
static_assert(sizeof(QCA_FIRMWARE_VERSION) - 1 <= EMBEDDED_VERSION_API_LENGTH, "Embedded PLC modem firmware version too long for API");

// plctool waits 5 seconds after the commit before talking to the modem again.
static constexpr seconds_t RESET_WAIT                       = 5_s;

// Time the modem has to come up again after an update (while no EV is connected).
static constexpr seconds_t VERIFY_TIMEOUT                   = 90_s;

#if QCA_FIRMWARE_AVAILABLE
// Regions of the PIB that legitimately differ between the embedded PIB and the PIB in the modem's memory.
struct PibRegion {
    size_t offset;
    size_t length;
};

static constexpr PibRegion PIB_IGNORED_REGIONS[] = {
    {0x374, 4},   // PIB image checksum (NVM header)
    {0x3BC, 4},   // NVM header checksum
    {0x3D2, 16},  // Device access key (DAK), the modem keeps its own
    {0x424, 16},  // Network membership key (NMK), set by CM_SET_KEY
    {0x4B8, 7},   // Network identifier (NID), derived from the NMK
    {0x4BF, 1},   // AutoFWUpgradeable, cleared by the modem after a reset
};

// The modem reports e.g. "MAC-QCA7006-HPGP-3.6.0.41-00-20260923-CS", the embedded version is the part after the device-specific prefix.
static bool version_matches(const char *modem_version)
{
    const size_t version_length  = strlen(modem_version);
    const size_t expected_length = strlen(QCA_FIRMWARE_VERSION);

    return version_length > expected_length &&
           modem_version[version_length - expected_length - 1] == '-' &&
           strcmp(modem_version + version_length - expected_length, QCA_FIRMWARE_VERSION) == 0;
}

static bool pib_matches(const uint8_t *pib, size_t pib_length)
{
    if (pib == nullptr || pib_length != qca_firmware_pib_length) {
        return false;
    }

    for (size_t i = 0; i < pib_length; i++) {
        if (pib[i] == qca_firmware_pib_data[i]) {
            continue;
        }

        bool ignored = false;
        for (const PibRegion &region : PIB_IGNORED_REGIONS) {
            if (i >= region.offset && i < region.offset + region.length) {
                ignored = true;
                break;
            }
        }

        if (!ignored) {
            iso15118.trace("ModemFirmware: PIB differs at offset 0x%04X", static_cast<unsigned>(i));
            return false;
        }
    }

    return true;
}
#endif

bool ModemFirmware::is_available()
{
    return QCA_FIRMWARE_AVAILABLE != 0;
}

void ModemFirmware::pre_setup()
{
    api_state = Config::Object({
        {"available",        Config::Bool(is_available())},
        {"embedded_version", Config::Str(QCA_FIRMWARE_VERSION, 0, EMBEDDED_VERSION_API_LENGTH)},
        {"modem_version",    Config::Str("", 0, MODEM_VERSION_API_LENGTH)},
        {"state",            Config::Enum(ModemUpdateState::Unknown)},
        {"progress",         Config::Uint8(0)},
        {"error",            Config::Str("", 0, 160)},
    });

    update_config = Config::Object({
        {"force", Config::Bool(false)},
    });
}

void ModemFirmware::setup()
{
    // Check (and update if necessary) the modem once it is up and idle.
    // Also without embedded firmware, to report the modem's firmware version.
    // This only happens while the ISO 15118 state machines run, i.e. if ISO 15118 is enabled, the state stays Unknown until then.
    phase = Phase::WaitForIdleModem;
}

void ModemFirmware::register_urls()
{
    api.addState("iso15118/modem_firmware", &api_state);

    api.addCommand("iso15118/modem_firmware_update", &update_config, {}, [this](Language /*language*/, String &errmsg) {
        start_check(update_config.get("force")->asBool(), errmsg);
    }, true);
}

bool ModemFirmware::start_check(bool force_flash, String &errmsg)
{
    if (!is_available()) {
        errmsg = "This firmware was built without PLC modem firmware";
        return false;
    }

    if (is_busy()) {
        errmsg = "PLC modem firmware update already in progress";
        return false;
    }

    if (evse_common.get_state().get("iec61851_state")->asUint() != 0) {
        errmsg = "Unplug the vehicle before updating the PLC modem firmware";
        return false;
    }

    // Make sure the modem is powered and the state machines run, also if ISO 15118 is disabled.
    iso15118.prepare_for_pib();

    force = force_flash;
    verifying = false;
    firmware_writes = 0;
    pib_writes = 0;
    api_state.get("error")->updateString("");
    api_state.get("progress")->updateUint(0);

    phase = Phase::WaitForIdleModem;
    set_state(ModemUpdateState::Pending);

    iso15118.trace("ModemFirmware: %s requested", force ? "Update" : "Check");
    return true;
}

bool ModemFirmware::is_modem_idle() const
{
    return iso15118.is_pib_modem_ready() &&
           evse_common.get_state().get("iec61851_state")->asUint() == 0 &&
           !iso15118.pib_manager.is_busy();
}

void ModemFirmware::set_state(ModemUpdateState state)
{
    api_state.get("state")->updateEnum(state);
}

void ModemFirmware::send_version_request()
{
    CM_QualcommGetSwRequest req;
    iso15118.slac.fill_header_v0(&req.header, slac_mac_plc_peer, iso15118.slac.evse_mac,
                                 SLAC_MMTYPE_QUALCOMM_GET_SW | SLAC_MMTYPE_MODE_REQUEST);
    iso15118.qca700x.write_burst(reinterpret_cast<const uint8_t *>(&req), sizeof(req));

    version_tries++;
    deadline = calculate_deadline(VERSION_REQUEST_TIMEOUT);
    phase = Phase::WaitForVersion;
}

void ModemFirmware::handle_sw_version(const uint8_t *version, size_t length)
{
    if (length >= sizeof(modem_version)) {
        length = sizeof(modem_version) - 1;
    }

    memcpy(modem_version, version, length);
    modem_version[length] = '\0';

    // The full version is used for the comparison, the API only gets the first characters.
    api_state.get("modem_version")->updateString(String(modem_version, std::min<size_t>(length, MODEM_VERSION_API_LENGTH)));
    iso15118.trace("ModemFirmware: Modem firmware version: %s", modem_version);

    if (phase == Phase::WaitForVersion) {
        version_received = true;
    }
}

void ModemFirmware::state_machine_loop()
{
    switch (phase) {
        case Phase::Idle:
            return;

        case Phase::WaitForIdleModem:
            if (!is_modem_idle()) {
                if (verifying) {
                    // Don't count the time while an EV is connected.
                    if (evse_common.get_state().get("iec61851_state")->asUint() != 0) {
                        deadline = calculate_deadline(VERIFY_TIMEOUT);
                    } else if (deadline_elapsed(deadline)) {
                        fail("PLC modem did not come up again after the update");
                    }
                }
                return;
            }

            set_state(verifying ? ModemUpdateState::Verifying : ModemUpdateState::Checking);
            version_tries = 0;
            version_received = false;
            modem_version[0] = '\0';
            send_version_request();
            return;

        case Phase::WaitForVersion:
            if (version_received) {
                if (!is_available()) {
                    // Nothing to compare with, only report the version.
                    phase = Phase::Idle;
                    set_state(ModemUpdateState::Unknown);
                    iso15118.stop_modem_if_disabled();
                    return;
                }

                if (!iso15118.pib_manager.start_read()) {
                    // EV connected in the meantime or similar, try again later.
                    phase = Phase::WaitForIdleModem;
                    return;
                }

                phase = Phase::WaitForPib;
                return;
            }

            if (deadline_elapsed(deadline)) {
                if (version_tries < VERSION_REQUEST_TRIES) {
                    send_version_request();
                } else {
                    fail("No response to VS_SW_VER.REQ from PLC modem");
                }
            }
            return;

        case Phase::WaitForPib:
            handle_pib_read();
            return;

        case Phase::Flashing:
        case Phase::WaitAfterFlash: {
            // The modem can't be used for ISO 15118 while it is updated. Let a connected EV charge with PWM.
            if (evse_common.get_state().get("iec61851_state")->asUint() != 0 && !iso15118.iec_temporary_active) {
                iso15118.trace("ModemFirmware: EV connected during modem update, switching to IEC 61851");
                iso15118.switch_to_iec_temporary();
            }

            if (phase == Phase::WaitAfterFlash) {
                if (deadline_elapsed(deadline)) {
                    finish_flash();
                }
                return;
            }

            const size_t total = iso15118.pib_manager.get_write_total();
            if (total > 0) {
                api_state.get("progress")->updateUint(static_cast<uint8_t>(iso15118.pib_manager.get_write_progress() * 100 / total));
            }

            const PibState pib_state = iso15118.pib_manager.get_state();
            if (pib_state == PibState::WriteComplete) {
                iso15118.pib_manager.reset();
                iso15118.trace("ModemFirmware: Write complete, waiting for modem reset");
                phase = Phase::WaitAfterFlash;
                deadline = calculate_deadline(RESET_WAIT);
            } else if (pib_state == PibState::WriteError) {
                char err[160];
                iso15118.pib_manager.format_error(err, sizeof(err));
                iso15118.pib_manager.reset();
                finish_flash();
                fail("Writing to PLC modem failed: %s", err);
            }
            return;
        }

        default:
            return;
    }
}

void ModemFirmware::handle_pib_read()
{
    const PibState pib_state = iso15118.pib_manager.get_state();

    if (pib_state == PibState::ReadComplete) {
#if QCA_FIRMWARE_AVAILABLE
        modem_pib_matches = pib_matches(iso15118.pib_manager.get_buffer(), iso15118.pib_manager.get_buffer_length());
#else
        modem_pib_matches = false;
#endif
        iso15118.pib_manager.reset();
        decide();
    } else if (pib_state == PibState::ReadError) {
        char err[160];
        iso15118.pib_manager.format_error(err, sizeof(err));
        iso15118.pib_manager.reset();
        fail("Reading PIB failed: %s", err);
    } else if (!iso15118.pib_manager.is_busy()) {
        // Read was aborted, try again later.
        phase = Phase::WaitForIdleModem;
    }
}

// A firmware update needs a PIB in the same session, but the modem merges that PIB with the PIB it used before.
// It keeps its old values (MAC, HFIDs, prescalers, ...) and only takes new fields and the version information from the new PIB.
// Thus the PIB is written again after a firmware update if it still differs.
void ModemFirmware::decide()
{
#if QCA_FIRMWARE_AVAILABLE
    const bool version_ok = version_matches(modem_version);
#else
    const bool version_ok = false;
#endif

    iso15118.trace("ModemFirmware: Modem version %s (%s), PIB %s",
                   modem_version, version_ok ? "matches" : "differs", modem_pib_matches ? "matches" : "differs");

    if (verifying) {
        if (version_ok && modem_pib_matches) {
            succeed();
            return;
        }

        if (!version_ok) {
            if (firmware_writes >= MAX_FIRMWARE_WRITES) {
                fail("Modem reports version %s after the update", modem_version);
                return;
            }

            logger.printfln("PLC modem firmware not updated (version %s), retrying", modem_version);
            start_flash(true);
            return;
        }

        if (pib_writes >= MAX_PIB_WRITES) {
            fail("Modem reports a different PIB after the update");
            return;
        }

        iso15118.trace("ModemFirmware: PIB differs after the update, writing PIB");
        start_flash(false);
        return;
    }

    if (version_ok && modem_pib_matches && !force) {
        iso15118.trace("ModemFirmware: Modem firmware is up to date");
        phase = Phase::Idle;
        set_state(ModemUpdateState::UpToDate);
        iso15118.stop_modem_if_disabled();
        return;
    }

    if (!version_ok) {
        logger.printfln("Updating PLC modem firmware from %s to %s", modem_version[0] != '\0' ? modem_version : "unknown version", QCA_FIRMWARE_VERSION);
    } else if (force) {
        logger.printfln("Reflashing PLC modem firmware %s", QCA_FIRMWARE_VERSION);
    } else {
        logger.printfln("Updating PLC modem PIB");
    }

    start_flash(!version_ok || force);
}

void ModemFirmware::start_flash(bool with_firmware)
{
#if QCA_FIRMWARE_AVAILABLE
    if (!is_modem_idle()) {
        // EV connected in the meantime. Check again once the modem is idle.
        phase = Phase::WaitForIdleModem;
        set_state(verifying ? ModemUpdateState::Verifying : ModemUpdateState::Pending);
        return;
    }

    const uint8_t *nvm = with_firmware ? qca_firmware_nvm_data : nullptr;
    const size_t nvm_length = with_firmware ? qca_firmware_nvm_length : 0;

    if (!iso15118.pib_manager.start_flash(qca_firmware_pib_data, qca_firmware_pib_length, nvm, nvm_length)) {
        const char *err = iso15118.pib_manager.get_error();
        iso15118.pib_manager.reset();
        fail("Starting update failed: %s", err != nullptr ? err : "unknown error");
        return;
    }

    if (with_firmware) {
        firmware_writes++;
    } else {
        pib_writes++;
    }

    force = false;
    verifying = false;

    // Keep SLAC and the rest of the ISO 15118 stack away from the modem until it was reset.
    iso15118.slac.state = SLACState::ModemUpdate;

    api_state.get("progress")->updateUint(0);
    phase = Phase::Flashing;
    set_state(ModemUpdateState::Flashing);
#else
    (void)with_firmware;
    fail("This firmware was built without PLC modem firmware");
#endif
}

void ModemFirmware::finish_flash()
{
    // Leave the flashing phase first, otherwise the ISO 15118 stack ignores the modem changes below.
    phase = Phase::WaitForIdleModem;
    verifying = true;
    deadline = calculate_deadline(VERIFY_TIMEOUT);
    set_state(ModemUpdateState::Verifying);

    if (iso15118.iec_temporary_active) {
        // An EV is connected and charges with PWM. Keep the modem off until it is
        // unplugged, rearm_for_next_ev() initializes the modem again.
        iso15118.disable_plc_modem();
    } else {
        iso15118.slac.api_state.get("modem_initialization_tries")->updateUint(0);
        iso15118.slac.state = SLACState::ModemReset;
    }
}

void ModemFirmware::succeed()
{
    phase = Phase::Idle;
    verifying = false;
    api_state.get("progress")->updateUint(100);
    api_state.get("error")->updateString("");
    set_state(ModemUpdateState::UpToDate);

    logger.printfln("PLC modem updated to firmware %s", modem_version);
    iso15118.stop_modem_if_disabled();
}

void ModemFirmware::fail(const char *fmt, ...)
{
    char buf[160];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    phase = Phase::Idle;
    verifying = false;
    force = false;
    api_state.get("error")->updateString(buf);
    set_state(ModemUpdateState::Failed);

    logger.printfln("PLC modem firmware update failed: %s", buf);
    iso15118.stop_modem_if_disabled();
}
