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

#define METER_ALL_VALUES_COUNT 85

#define METER_TYPE_NONE 0
#define METER_TYPE_SDM72DM 1
#define METER_TYPE_SDM630 2
#define METER_TYPE_SDM72DMV2 3

#define METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L1 0
#define METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L2 1
#define METER_ALL_VALUES_LINE_TO_NEUTRAL_VOLTS_L3 2
#define METER_ALL_VALUES_CURRENT_L1_A 3
#define METER_ALL_VALUES_CURRENT_L2_A 4
#define METER_ALL_VALUES_CURRENT_L3_A 5
#define METER_ALL_VALUES_POWER_L1_W 6
#define METER_ALL_VALUES_POWER_L2_W 7
#define METER_ALL_VALUES_POWER_L3_W 8
#define METER_ALL_VALUES_VOLT_AMPS_L1 9
#define METER_ALL_VALUES_VOLT_AMPS_L2 10
#define METER_ALL_VALUES_VOLT_AMPS_L3 11
#define METER_ALL_VALUES_VOLT_AMPS_REACTIVE_L1 12
#define METER_ALL_VALUES_VOLT_AMPS_REACTIVE_L2 13
#define METER_ALL_VALUES_VOLT_AMPS_REACTIVE_L3 14
#define METER_ALL_VALUES_POWER_FACTOR_L1 15
#define METER_ALL_VALUES_POWER_FACTOR_L2 16
#define METER_ALL_VALUES_POWER_FACTOR_L3 17
#define METER_ALL_VALUES_PHASE_ANGLE_L1_DEGREE 18
#define METER_ALL_VALUES_PHASE_ANGLE_L2_DEGREE 19
#define METER_ALL_VALUES_PHASE_ANGLE_L3_DEGREE 20
#define METER_ALL_VALUES_AVERAGE_LINE_TO_NEUTRAL_VOLTS 21
#define METER_ALL_VALUES_AVERAGE_LINE_CURRENT_A 22
#define METER_ALL_VALUES_SUM_OF_LINE_CURRENTS_A 23
#define METER_ALL_VALUES_TOTAL_SYSTEM_POWER_W 24
#define METER_ALL_VALUES_TOTAL_SYSTEM_VOLT_AMPS 25
#define METER_ALL_VALUES_TOTAL_SYSTEM_VAR 26
#define METER_ALL_VALUES_TOTAL_SYSTEM_POWER_FACTOR 27
#define METER_ALL_VALUES_TOTAL_SYSTEM_PHASE_ANGLE_DEGREE 28
#define METER_ALL_VALUES_FREQUENCY_OF_SUPPLY_VOLTAGES_HERTZ 29
#define METER_ALL_VALUES_TOTAL_IMPORT_KWH 30
#define METER_ALL_VALUES_TOTAL_EXPORT_KWH 31
#define METER_ALL_VALUES_TOTAL_IMPORT_KVARH 32
#define METER_ALL_VALUES_TOTAL_EXPORT_KVARH 33
#define METER_ALL_VALUES_TOTAL_VAH 34
#define METER_ALL_VALUES_AH 35
#define METER_ALL_VALUES_TOTAL_SYSTEM_POWER_DEMAND_W 36
#define METER_ALL_VALUES_MAXIMUM_TOTAL_SYSTEM_POWER_DEMAND_W 37
#define METER_ALL_VALUES_TOTAL_SYSTEM_VA_DEMAND 38
#define METER_ALL_VALUES_MAXIMUM_TOTAL_SYSTEM_VA_DEMAND 39
#define METER_ALL_VALUES_NEUTRAL_CURRENT_DEMAND_A 40
#define METER_ALL_VALUES_MAXIMUM_NEUTRAL_CURRENT_DEMAND_A 41
#define METER_ALL_VALUES_LINE1_TO_LINE2_VOLTS 42
#define METER_ALL_VALUES_LINE2_TO_LINE3_VOLTS 43
#define METER_ALL_VALUES_LINE3_TO_LINE1_VOLTS 44
#define METER_ALL_VALUES_AVERAGE_LINE_TO_LINE_VOLTS 45
#define METER_ALL_VALUES_NEUTRAL_CURRENT_A 46
#define METER_ALL_VALUES_LN_VOLTS_THD_L1 47
#define METER_ALL_VALUES_LN_VOLTS_THD_L2 48
#define METER_ALL_VALUES_LN_VOLTS_THD_L3 49
#define METER_ALL_VALUES_CURRENT_THD_L1_A 50
#define METER_ALL_VALUES_CURRENT_THD_L2_A 51
#define METER_ALL_VALUES_CURRENT_THD_L3_A 52
#define METER_ALL_VALUES_AVERAGE_LINE_TO_NEUTRAL_VOLTS_THD 53
#define METER_ALL_VALUES_AVERAGE_LINE_CURRENT_THD_A 54
#define METER_ALL_VALUES_CURRENT_DEMAND_L1_A 55
#define METER_ALL_VALUES_CURRENT_DEMAND_L2_A 56
#define METER_ALL_VALUES_CURRENT_DEMAND_L3_A 57
#define METER_ALL_VALUES_MAXIMUM_CURRENT_DEMAND_L1_A 58
#define METER_ALL_VALUES_MAXIMUM_CURRENT_DEMAND_L2_A 59
#define METER_ALL_VALUES_MAXIMUM_CURRENT_DEMAND_L3_A 60
#define METER_ALL_VALUES_LINE1_TO_LINE2_VOLTS_THD_PERCENT 61
#define METER_ALL_VALUES_LINE2_TO_LINE3_VOLTS_THD_PERCENT 62
#define METER_ALL_VALUES_LINE3_TO_LINE1_VOLTS_THD_PERCENT 63
#define METER_ALL_VALUES_AVERAGE_LINE_TO_LINE_VOLTS_THD_PERCENT 64
#define METER_ALL_VALUES_TOTAL_KWH_SUM 65
#define METER_ALL_VALUES_TOTAL_KVARH_SUM 66
#define METER_ALL_VALUES_IMPORT_KWH_L1 67
#define METER_ALL_VALUES_IMPORT_KWH_L2 68
#define METER_ALL_VALUES_IMPORT_KWH_L3 69
#define METER_ALL_VALUES_EXPORT_KWH_L1 70
#define METER_ALL_VALUES_EXPORT_KWH_L2 71
#define METER_ALL_VALUES_EXPORT_KWH_L3 72
#define METER_ALL_VALUES_TOTAL_KWH_L1 73
#define METER_ALL_VALUES_TOTAL_KWH_L2 74
#define METER_ALL_VALUES_TOTAL_KWH_L3 75
#define METER_ALL_VALUES_IMPORT_KVARH_L1 76
#define METER_ALL_VALUES_IMPORT_KVARH_L2 77
#define METER_ALL_VALUES_IMPORT_KVARH_L3 78
#define METER_ALL_VALUES_EXPORT_KVARH_L1 79
#define METER_ALL_VALUES_EXPORT_KVARH_L2 80
#define METER_ALL_VALUES_EXPORT_KVARH_L3 81
#define METER_ALL_VALUES_TOTAL_KVARH_L1 82
#define METER_ALL_VALUES_TOTAL_KVARH_L2 83
#define METER_ALL_VALUES_TOTAL_KVARH_L3 84

class Meter
{
public:
    Meter(){}
    void pre_setup();
    void setup();
    void register_urls();
    void loop();

    void updateMeterState(uint8_t new_state, uint8_t new_type);
    void updateMeterState(uint8_t new_state);
    void updateMeterType(uint8_t new_type);
    void updateMeterValues(float power, float energy_rel, float energy_abs);
    void updateMeterPhases(bool phases_connected[3], bool phases_active[3]);

    void updateMeterAllValues(int idx, float val);
    void updateMeterAllValues(float values[METER_ALL_VALUES_COUNT]);
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

    std::vector<std::function<void(void)>> reset_callbacks;
};
