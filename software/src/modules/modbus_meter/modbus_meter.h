/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#pragma once

#include "bindings/bricklet_rs485.h"

#include "config.h"
#include "device_module.h"
#include "rs485_bricklet_firmware_bin.embedded.h"

#include "meter_defs.h"

class ModbusMeter : public DeviceModule<TF_RS485,
                                        rs485_bricklet_firmware_bin_data,
                                        rs485_bricklet_firmware_bin_length,
                                        tf_rs485_create,
                                        tf_rs485_get_bootloader_mode,
                                        tf_rs485_reset,
                                        tf_rs485_destroy>
{
public:
    ModbusMeter() : DeviceModule("rs485", "RS485", "Modbus Meter", std::bind(&ModbusMeter::setupRS485, this)) {}
    void pre_setup();
    void setup();
    void register_urls();
    void loop();

    enum class UserDataDone {
        NOT_DONE,
        DONE,
        ERROR
    };

    struct UserData {
        uint16_t *value_to_write;
        uint8_t expected_request_id;
        UserDataDone done;
    };

    ConfigRoot error_counters;

    ConfigRoot meter_type_override;

private:
    void modbus_read();
    void setupRS485();
    void checkRS485State();
    const RegRead *getNextRead(bool *trigger_fast_read_done, bool *trigger_slow_read_done);

    TF_RS485 rs485;
    size_t modbus_read_state_fast = 0;
    size_t modbus_read_state_slow = 0;
    bool last_read_was_fast = false;

    uint32_t next_modbus_read_deadline = 0;
    uint32_t next_power_history_entry = 0;
    UserData user_data;

    bool reset_requested;

    uint32_t callback_deadline_ms = 0;
    uint32_t next_read_deadline_ms = 0;

    uint16_t meter_type = 0;

    char uid[7] = {0};
};
