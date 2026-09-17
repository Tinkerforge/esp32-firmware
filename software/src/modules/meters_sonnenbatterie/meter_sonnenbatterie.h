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

#pragma once

#include <stdint.h>

#include "config.h"
#include "modules/meters/generated/meter_location.enum.h"
#include "modules/meters/imeter.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MeterSonnenbatterie final : public IMeter
{
public:
    struct sonnenbatterie_api_values {
                                //"Apparent_output":240
                                //"BackupBuffer":"0"
                                //"BatteryCharging":false
                                //"BatteryDischarging":false
                                //"Consumption_Avg":2710
                                //"Consumption_W":2776
        float f_ac;             //"Fac":50.000156
                                //"FlowConsumptionBattery":false
                                //"FlowConsumptionGrid":false
                                //"FlowConsumptionProduction":true
                                //"FlowGridBattery":false
                                //"FlowProductionBattery":false
                                //"FlowProductionGrid":true
        float grid_feed_w;      //"GridFeedIn_W":12345
                                //"IsSystemInstalled":1
                                //"OperatingMode":"2"
        float battery_out_w;    //"Pac_total_W":12345
                                //"Production_W":5699
                                //"RSOC":100
                                //"RemainingCapacity_Wh":10285
                                //"Sac1":79
                                //"Sac2":79
                                //"Sac3":82
                                //"SystemStatus":"OnGrid"
                                //"Timestamp":"2023-09-10 12:52:15"
        float soc;              //"USOC":100
        float u_ac;             //"Uac":234.0
        float u_bat;            //"Ubat":52.0
                                //"dischargeNotAllowed":false
                                //"generator_autostart":false
    };

    MeterSonnenbatterie(uint32_t slot_) : slot(slot_) {}

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(Config *config) override;

    bool supports_power() override {return true;}

    void push_data(const sonnenbatterie_api_values &api_values);

private:
    uint32_t slot;
    MeterLocation location;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
