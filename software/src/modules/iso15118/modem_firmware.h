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

// Firmware update for the QCA700x PLC modem.
// The modem firmware (NVM) and PIB are embedded into the ESP32 firmware (see prepare.py).


#pragma once

#include <cstdint>
#include <cstddef>

#include <TFTools/Micros.h>

#include "config.h"
#include "generated/modem_update_state.enum.h"

class ModemFirmware final
{
public:
    ModemFirmware() {}

    void pre_setup();
    void setup();
    void register_urls();

    void state_machine_loop();
    void handle_sw_version(const uint8_t *version, size_t length);

    bool is_flashing() const { return phase == Phase::Flashing || phase == Phase::WaitAfterFlash; }
    bool is_busy() const { return phase != Phase::Idle && phase != Phase::WaitForIdleModem; }

    [[gnu::const]] static bool is_available();

    ConfigRoot api_state;

private:
    enum class Phase : uint8_t {
        Idle,
        WaitForIdleModem,
        WaitForVersion,
        WaitForPib,
        Flashing,
        WaitAfterFlash,
    };

    bool start_check(bool force_flash, String &errmsg);
    bool is_modem_idle() const;
    void send_version_request();
    void handle_pib_read();
    void decide();
    void start_flash(bool with_firmware);
    void finish_flash();
    void succeed();
    void fail(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    void set_state(ModemUpdateState state);

    ConfigRoot update_config;

    Phase phase = Phase::Idle;

    bool force = false;       // Flash even if version and PIB match (manual request)
    bool verifying = false;   // Checking the result of a write
    uint8_t firmware_writes = 0;
    uint8_t pib_writes = 0;

    uint8_t version_tries = 0;
    micros_t deadline = 0_us;
    bool version_received = false;
    char modem_version[129] = {};
    bool modem_pib_matches = false;
};
