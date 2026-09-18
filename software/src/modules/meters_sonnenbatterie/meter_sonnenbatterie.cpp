/* esp32-firmware
 * Copyright (C) 2026 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#define EVENT_LOG_PREFIX "meters_sonnenbat"

#include "generated/module_dependencies.h"
#include "meter_sonnenbatterie.h"

#include "event_log_prefix.h"
#include "modules/meters/generated/meter_value_id.h"

#include "gcc_warnings.h"

static const MeterValueID value_ids_grid[] = {
                                            //"Apparent_output":240
                                            //"BackupBuffer":"0"
    // -                                    //"BatteryCharging":false
    // -                                    //"BatteryDischarging":false
                                            //"Consumption_Avg":2710
                                            //"Consumption_W":2776
    MeterValueID::FrequencyLAvg,            //"Fac":50.000156
                                            //"FlowConsumptionBattery":false
                                            //"FlowConsumptionGrid":false
                                            //"FlowConsumptionProduction":true
                                            //"FlowGridBattery":false
                                            //"FlowProductionBattery":false
                                            //"FlowProductionGrid":true
    MeterValueID::PowerActiveLSumImExDiff,  //"GridFeedIn_W":12345
                                            //"IsSystemInstalled":1
                                            //"OperatingMode":"2"
    // -                                    //"Pac_total_W":12345
                                            //"Production_W":5699
    // -                                    //"RSOC":100
                                            //"RemainingCapacity_Wh":10285
                                            //"Sac1":79
                                            //"Sac2":79
                                            //"Sac3":82
                                            //"SystemStatus":"OnGrid"
                                            //"Timestamp":"2023-09-10 12:52:15"
    // -                                    //"USOC":100
    MeterValueID::VoltageLNAvg,             //"Uac":234
                                            //"Ubat":52
                                            //"dischargeNotAllowed":false
                                            //"generator_autostart":false
};

static const MeterValueID value_ids_battery[] = {
                                            //"Apparent_output":240
                                            //"BackupBuffer":"0"
    // -                                    //"BatteryCharging":false
    // -                                    //"BatteryDischarging":false
                                            //"Consumption_Avg":2710
                                            //"Consumption_W":2776
    // -                                    //"Fac":50.000156
                                            //"FlowConsumptionBattery":false
                                            //"FlowConsumptionGrid":false
                                            //"FlowConsumptionProduction":true
                                            //"FlowGridBattery":false
                                            //"FlowProductionBattery":false
                                            //"FlowProductionGrid":true
    // -                                    //"GridFeedIn_W":12345
                                            //"IsSystemInstalled":1
                                            //"OperatingMode":"2"
    MeterValueID::PowerActiveLSumImExDiff,  //"Pac_total_W":12345
                                            //"Production_W":5699
    // -                                    //"RSOC":100
                                            //"RemainingCapacity_Wh":10285
                                            //"Sac1":79
                                            //"Sac2":79
                                            //"Sac3":82
                                            //"SystemStatus":"OnGrid"
                                            //"Timestamp":"2023-09-10 12:52:15"
    MeterValueID::StateOfCharge,            //"USOC":100
    // -                                    //"Uac":234
    MeterValueID::VoltageDC,                //"Ubat":52.0
                                            //"dischargeNotAllowed":false
                                            //"generator_autostart":false
};

MeterClassID MeterSonnenbatterie::get_class() const
{
    return MeterClassID::Sonnenbatterie;
}

void MeterSonnenbatterie::setup(Config *config)
{
    location = config->get("location")->asEnum<MeterLocation>();

    if (location == MeterLocation::Grid) {
        meters.declare_value_ids(slot, value_ids_grid,    std::size(value_ids_grid));
    } else if (location == MeterLocation::Battery) {
        meters.declare_value_ids(slot, value_ids_battery, std::size(value_ids_battery));
    } else {
        logger.printfln_meter("Invalid location");
        return;
    }

    meters_sonnenbatterie.apply_config(config);
}

void MeterSonnenbatterie::push_data(const sonnenbatterie_api_values &api_values)
{
    float meter_values[3];
    static_assert(std::size(meter_values) >= std::size(value_ids_grid   ), "Check array sizes");
    static_assert(std::size(meter_values) >= std::size(value_ids_battery), "Check array sizes");

    if (location == MeterLocation::Grid) {
        //"Fac":50.000156
        meter_values[0] = api_values.f_ac;

        //"GridFeedIn_W":12345
        meter_values[1] = -api_values.grid_feed_w;

        //"Uac":234
        meter_values[2] = api_values.u_ac;
    } else if (location == MeterLocation::Battery) {
        //"Pac_total_W":12345
        meter_values[0] = -api_values.battery_out_w;

        //"USOC":100
        meter_values[1] = api_values.soc;

        //"Ubat":52.0
        meter_values[2] = api_values.u_bat;
    } else {
        // Already complained in setup()
        return;
    }

    meters.update_all_values(slot, meter_values);
}
