/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "charge_condition.h"

#include <Arduino.h>
#include "time.h"
#include "api.h"
#include "task_scheduler.h"
#include "tools.h"

#include "modules.h"

void ChargeCondition::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"duration_limit", Config::Uint(0, 0, 10)},
        {"energy_limit_kwh", Config::Uint32(0)},
        {"time_restriction_enabled", Config::Bool(false)},
        {"blocked_hours", Config::Array({Config::Uint32(0),
                                        Config::Uint32(0),
                                        Config::Uint32(0),
                                        Config::Uint32(0),
                                        Config::Uint32(0),
                                        Config::Uint32(0),
                                        Config::Uint32(0)}, new Config(Config::Uint32(0)), 7, 7, Config::type_id<Config::ConfUint>())}
    })};

    state = ConfigRoot{Config::Object({
        {"start_time_mil", Config::Uint32(0)},
        {"start_energy_kwh", Config::Uint32(0)},
        {"duration_left_sec", Config::Uint32(0)},
        {"energy_left_kwh", Config::Uint32(0)}
    })};
}

void ChargeCondition::setup()
{
    api.restorePersistentConfig("charge_condition/config", &config);
    config_in_use = config;
    initialized = true;
}

static uint32_t map_duration(uint32_t val)
{
    switch (val)
    {
        case 1:
            return 15 * 60;
        case 2:
            return 30 * 60;
        case 3:
            return 45 * 60;
        case 4:
            return 60 * 60;
        case 5:
            return 2 * 60 * 60;
        case 6:
            return 3 * 60 * 60;
        case 7:
            return 4 * 60 * 60;
        case 8:
            return 6 * 60 * 60;
        case 9:
            return 8 * 60 * 60;
        case 10:
            return 12 * 60 * 60;
        default:
            return 0;
    }
}

void ChargeCondition::register_urls()
{
#if MODULE_CHARGE_TRACKER_AVAILABLE()
    api.addPersistentConfig("charge_condition/config", &config, {}, 1000);
    api.addState("charge_condition/state", &state, {}, 1000);

    api.addCommand("charge_condition/override", &config_in_use, {}, {}, false);

    if ((config_in_use.get("duration_limit")->asUint() > 0 || config_in_use.get("energy_limit_kwh")->asUint() > 0) && api.hasFeature("evse"))
    {
#if MODULE_EVSE_V2_AVAILABLE()
        evse_v2.set_charge_condition(32000, true);
#elif MODULE_EVSE_AVAILABLE()
        evse.set_charge_condition(32000, true);
#endif
        task_scheduler.scheduleWithFixedDelay([this](){
            if (charge_tracker.current_charge.get("user_id")->asInt() != -1)
            {
                if (api.hasFeature("meter") && state.get("start_energy_kwh")->asUint() == 0)
                    state.get("start_energy_kwh")->updateUint((uint32_t)(charge_tracker.current_charge.get("meter_start")->asFloat() * 1000));
                if (config_in_use.get("duration_limit")->asUint() > 0)
                {
                    if (state.get("start_time_mil")->asUint() == 0)
                        state.get("start_time_mil")->updateUint(millis());

                    int time_left = map_duration(config_in_use.get("duration_limit")->asUint()) * 1000 - (millis() - state.get("start_time_mil")->asUint());
                    if (time_left <= 0)
                    {
#if MODULE_EVSE_V2_AVAILABLE()
                        evse_v2.set_charge_condition(0, true);
#elif MODULE_EVSE_AVAILABLE()
                        evse.set_charge_condition(0, true);
#endif
                        state.get("duration_left_sec")->updateUint(0);
                    }
                    else
                        state.get("duration_left_sec")->updateUint(time_left / 1000);
                }
                if (api.hasFeature("meter") && config_in_use.get("energy_limit_kwh")->asUint() > 0)
                {
                    int energy_left = state.get("start_energy_kwh")->asUint() + config_in_use.get("energy_limit_kwh")->asUint() - meter.values.get("energy_abs")->asFloat() * 1000;
                    if (energy_left <= 0)
                    {
#if MODULE_EVSE_V2_AVAILABLE()
                            evse_v2.set_charge_condition(0, true);
#elif MODULE_EVSE_AVAILABLE()
                            evse.set_charge_condition(0, true);
#endif
                        state.get("energy_left_kwh")->updateUint(0);
                    }
                    else
                        state.get("energy_left_kwh")->updateUint(energy_left);
                }
            }
            else
            {
                config_in_use = config;
#if MODULE_EVSE_V2_AVAILABLE()
                evse_v2.set_charge_condition(32000, true);
#elif MODULE_EVSE_AVAILABLE()
                evse.set_charge_condition(32000, true);
#endif
                state.get("start_time_mil")->updateUint(0);
                state.get("start_energy_kwh")->updateUint(0);
                state.get("duration_left_sec")->updateUint(0);
                state.get("energy_left_kwh")->updateUint(0);
            }
        }, 0, 1000);
    }
#endif

    if (config_in_use.get("time_restriction_enabled")->asBool())
    {
#if MODULE_EVSE_V2_AVAILABLE()
        evse_v2.set_charge_time_restriction(32000, true);
#endif
        task_scheduler.scheduleWithFixedDelay([this]() {
            timeval now;
            if (clock_synced(&now))
            {
                tm tm_now;
                localtime_r(&now.tv_sec, &tm_now);
                uint32_t hours = config_in_use.get("blocked_hours")->get((tm_now.tm_wday + 6) % 7)->asUint();
                bool forbidden = hours & (1 << tm_now.tm_hour);
                logger.printfln("%u, %i, %u, %u", hours, forbidden, tm_now.tm_hour, (tm_now.tm_wday + 6) % 7);
#if MODULE_EVSE_V2_AVAILABLE()
                evse_v2.set_charge_time_restriction(forbidden ? 0: 32000, true);
#endif
            }
        }, 0, 1000);
    }
    else
        evse_v2.set_charge_time_restriction(32000, false);

}

void ChargeCondition::loop()
{

}