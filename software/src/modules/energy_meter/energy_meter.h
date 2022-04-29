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

#include "config.h"

#include "value_history.h"

#define ALL_VALUES_COUNT 85

#define ENERGY_METER_TYPE_NONE 0
#define ENERGY_METER_TYPE_SDM72DM 1
#define ENERGY_METER_TYPE_SDM630 2
#define ENERGY_METER_TYPE_SDM72DMV2 3

class EnergyMeter {
public:
    EnergyMeter();
    void setup();
    void register_urls();
    void loop();

    void updateMeterState(uint8_t new_state, uint8_t new_type);
    void updateMeterState(uint8_t new_state);
    void updateMeterType(uint8_t new_type);
    void updateMeterValues(float power, float energy_rel, float energy_abs);
    void updateMeterPhases(bool phases_connected[3], bool phases_active[3]);

    void updateMeterAllValues(int idx, float val);
    void updateMeterAllValues(float values[ALL_VALUES_COUNT]);
    void registerResetCallback(std::function<void(void)> cb);

    void setupMeter(uint8_t meter_type);

    bool initialized = false;
    bool meter_setup_done = false;

    ConfigRoot state;
    ConfigRoot values;
    ConfigRoot phases;
    ConfigRoot all_values;
    ConfigRoot reset;
    ConfigRoot last_reset;

    ValueHistory power_hist;

    char uid[7] = {0};

    std::vector<std::function<void(void)>> reset_callbacks;
};
