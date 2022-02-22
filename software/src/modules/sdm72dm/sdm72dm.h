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

#include "ArduinoJson.h"

#include "bindings/bricklet_rs485.h"

#include "config.h"
#include "ringbuffer.h"
#include "malloc_tools.h"
#include "device_module.h"
#include "rs485_bricklet_firmware_bin.embedded.h"

// How many hours to keep the coarse history for
#define HISTORY_HOURS 48
// How many minutes to keep the fine history for.
// This also controls the coarseness of the coarse history.
// For example 4 means that we accumulate 4 minutes of samples
// with the maximum rate i.e. ~ 3 samples per second (Querying the state
// takes about 380 ms).
// When we have 4 minutes worth of samples, we take the average
// and add it to the coarse history.
#define HISTORY_MINUTE_INTERVAL 4

#define RING_BUF_SIZE (HISTORY_HOURS * (60 / HISTORY_MINUTE_INTERVAL) + 1)

class SDM72DM : public DeviceModule<TF_RS485,
                                    rs485_bricklet_firmware_bin_data,
                                    rs485_bricklet_firmware_bin_length,
                                    tf_rs485_create,
                                    tf_rs485_get_bootloader_mode,
                                    tf_rs485_reset,
                                    tf_rs485_destroy> {
public:
    SDM72DM();
    void setup();
    void register_urls();
    void loop();

    enum class UserDataDone {
        NOT_DONE,
        DONE,
        ERROR
    };

    struct UserData {
        Config *value_to_write;
        Config *state;
        uint8_t expected_request_id;
        UserDataDone done;
    };

private:
    void modbus_read();
    void setupRS485();
    void checkRS485State();

    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot energy_meter_reset;
    ConfigRoot error_counters;

    TF_RS485 rs485;
    int modbus_read_state = 0;
    int samples_last_interval = 0;
    int samples_per_interval = -1;
    uint32_t interval_end_ms = 1000 * 60 * HISTORY_MINUTE_INTERVAL;
    TF_Ringbuffer<int16_t, 3 * 60 * HISTORY_MINUTE_INTERVAL, uint32_t, malloc_32bit_addressed, heap_caps_free> interval_samples;

    uint32_t next_modbus_read_deadline = 0;
    uint32_t next_power_history_entry = 0;
    UserData user_data;
    TF_Ringbuffer<int16_t, HISTORY_HOURS * (60 / HISTORY_MINUTE_INTERVAL) + 1, uint32_t, malloc_32bit_addressed, heap_caps_free> power_history;

    bool energy_meter_reset_requested;

    uint32_t callback_deadline_ms = 0;
    uint32_t next_read_deadline_ms = 0;

    char uid[7] = {0};
};
