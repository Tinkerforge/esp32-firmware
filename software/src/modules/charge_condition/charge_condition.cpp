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
    })};

    state = ConfigRoot{Config::Object({
        {"start_timestamp_mil", Config::Uint32(0)},
        {"start_energy_kwh", Config::Uint32(0)},
        {"target_timestamp_mil", Config::Uint32(0)},
        {"target_energy_kwh", Config::Uint32(0)}
    })};

    override_duration = ConfigRoot{Config::Object({
        {"duration", Config::Uint32(0)}
    })};

    override_energy = ConfigRoot{Config::Object({
        {"energy", Config::Uint32(0)}
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
            return 15 * 60 * 1000;
        case 2:
            return 30 * 60 * 1000;
        case 3:
            return 45 * 60 * 1000;
        case 4:
            return 60 * 60 * 1000;
        case 5:
            return 2 * 60 * 60 * 1000;
        case 6:
            return 3 * 60 * 60 * 1000;
        case 7:
            return 4 * 60 * 60 * 1000;
        case 8:
            return 6 * 60 * 60 * 1000;
        case 9:
            return 8 * 60 * 60 * 1000;
        case 10:
            return 12 * 60 * 60 * 1000;
        default:
            return 0;
    }
}

void ChargeCondition::register_urls()
{
#if MODULE_CHARGE_TRACKER_AVAILABLE()
    api.addPersistentConfig("charge_condition/config", &config, {}, 1000);
    api.addState("charge_condition/state", &state, {}, 1000);
    api.addState("charge_condition/live_config", &config_in_use, {}, 1000);

    api.addCommand("charge_condition/override_duration", &override_duration, {}, [this]() {
        config_in_use.get("duration_limit")->updateUint(override_duration.get("duration")->asUint());
        state.get("target_timestamp_mil")->updateUint(state.get("start_timestamp_mil")->asUint() + map_duration(override_duration.get("duration")->asUint()));
    }, true);

    api.addCommand("charge_condition/override_energy", &override_energy, {}, [this]() {
        config_in_use.get("energy_limit_kwh")->updateUint(override_energy.get("energy")->asUint());
        state.get("target_energy_kwh")->updateUint(state.get("start_energy_kwh")->asUint() + override_energy.get("energy")->asUint());
    }, true);

    //if we dont set the target timestamp right away we will have 0 seconds left displayed in the webinterface until we start and end a charge.
    state.get("target_timestamp_mil")->updateUint(map_duration(config_in_use.get("duration_limit")->asUint()));

 #if MODULE_EVSE_V2_AVAILABLE()
    evse_v2.set_charge_condition_slot(32000, true);
 #elif MODULE_EVSE_AVAILABLE()
    evse.set_charge_condition_slot(32000, true);
 #endif
    task_scheduler.scheduleWithFixedDelay([this](){
        static bool was_charging = false;

        if (charge_tracker.current_charge.get("user_id")->asInt() != -1)
        {
 #if MODULE_EVSE_V2_AVAILABLE()
            if (!was_charging)
                state.get("start_timestamp_mil")->updateUint(evse_v2.evse_low_level_state.get("uptime")->asUint());
 #elif MODULE_EVSE_AVAILABLE()
            if (!was_charging)
                state.get("start_timestamp_mil")->updateUint(evse.evse_low_level_state.get("uptime")->asUint());
 #endif
            if (api.hasFeature("meter") && !was_charging)
                state.get("start_energy_kwh")->updateUint((uint32_t)(charge_tracker.current_charge.get("meter_start")->asFloat() * 1000));
            if (config_in_use.get("duration_limit")->asUint() > 0)
            {
 #if MODULE_EVSE_V2_AVAILABLE()

                if (!was_charging)
                    state.get("target_timestamp_mil")->updateUint(state.get("start_timestamp_mil")->asUint() + map_duration(config_in_use.get("duration_limit")->asUint()));

                int time_left = map_duration(config_in_use.get("duration_limit")->asUint()) - (evse_v2.evse_low_level_state.get("uptime")->asUint() - state.get("start_timestamp_mil")->asUint());
                if (time_left <= 0)
                    evse_v2.set_charge_condition_slot(0, true);
 #elif MODULE_EVSE_AVAILABLE()
                if (state.get("start_timestamp_mil")->asUint() == 0)
                    state.get("start_timestamp_mil")->updateUint(evse.evse_low_level_state.get("uptime")->asUint());

                if (state.get("target_timestamp_mil")->asUint() == 0)
                    state.get("target_timestamp_mil")->updateUint(evse.evse_low_level_state.get("uptime")->asUint() + map_duration(config_in_use.get("duration_limit")->asUint()));

                int time_left = map_duration(config_in_use.get("duration_limit")->asUint()) - (evse.evse_low_level_state.get("uptime")->asUint() - state.get("start_timestamp_mil")->asUint());
                if (time_left <= 0)
                    evse.set_charge_condition_slot(0, true);
 #endif
            }

            if (api.hasFeature("meter") && config_in_use.get("energy_limit_kwh")->asUint() > 0)
            {
                if (!was_charging)
                    state.get("target_energy_kwh")->updateUint(state.get("start_energy_kwh")->asUint() + config_in_use.get("energy_limit_kwh")->asUint());
                if (state.get("target_energy_kwh")->asUint() <= (uint32_t)(meter.values.get("energy_abs")->asFloat() * 1000))
                {
 #if MODULE_EVSE_V2_AVAILABLE()
                        evse_v2.set_charge_condition_slot(0, true);
 #elif MODULE_EVSE_AVAILABLE()
                        evse.set_charge_condition_slot(0, true);
 #endif
                }
            }

            was_charging = true;
        }
        else if (was_charging)
        {
            was_charging = false;

            if (!api.restorePersistentConfig("charge_condition/config", &config_in_use))
            {
                config_in_use.get("duration_limit")->updateUint(config.get("duration_limit")->asUint());
                config_in_use.get("energy_limit_kwh")->updateUint(config.get("energy_limit_kwh")->asUint());
            }

 #if MODULE_EVSE_V2_AVAILABLE()
            evse_v2.set_charge_condition_slot(32000, true);
 #elif MODULE_EVSE_AVAILABLE()
            evse.set_charge_condition_slot(32000, true);
 #endif
            state.get("start_timestamp_mil")->updateUint(0);
            state.get("start_energy_kwh")->updateUint(0);
            state.get("target_timestamp_mil")->updateUint(map_duration(config_in_use.get("duration_limit")->asUint()));
            state.get("target_energy_kwh")->updateUint(0);
        }
    }, 0, 1000);
#endif
}

void ChargeCondition::loop()
{

}