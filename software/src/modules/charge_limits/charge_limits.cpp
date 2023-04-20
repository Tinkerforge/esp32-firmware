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

#include "charge_limits.h"

#include <Arduino.h>

#include "api.h"
#include "task_scheduler.h"
#include "tools.h"

#include "modules.h"

void ChargeLimits::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"duration", Config::Uint(0, 0, 10)},
        {"energy_wh", Config::Uint32(0)},
    })};

    state = ConfigRoot{Config::Object({
        {"start_timestamp_ms", Config::Uint32(0)},
        {"start_energy_kwh", Config::Float(NAN)},
        {"target_timestamp_ms", Config::Uint32(0)},
        {"target_energy_kwh", Config::Float(NAN)}
    })};

    override_duration = ConfigRoot{Config::Object({
        {"duration", Config::Uint32(0)}
    })};

    override_energy = ConfigRoot{Config::Object({
        {"energy_wh", Config::Uint32(0)}
    })};
}

void ChargeLimits::setup()
{
    api.restorePersistentConfig("charge_limits/default_limits", &config);
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

void ChargeLimits::register_urls()
{
#if MODULE_CHARGE_TRACKER_AVAILABLE()
    api.addPersistentConfig("charge_limits/default_limits", &config, {}, 1000);
    api.addState("charge_limits/state", &state, {}, 1000);
    api.addState("charge_limits/active_limits", &config_in_use, {}, 1000);

    api.addCommand("charge_limits/override_duration", &override_duration, {}, [this]() {
        config_in_use.get("duration")->updateUint(override_duration.get("duration")->asUint());
        state.get("target_timestamp_ms")->updateUint(state.get("start_timestamp_ms")->asUint() + map_duration(override_duration.get("duration")->asUint()));
    }, true);

    api.addCommand("charge_limits/override_energy", &override_energy, {}, [this]() {
        config_in_use.get("energy_wh")->updateUint(override_energy.get("energy_wh")->asUint());
        state.get("target_energy_kwh")->updateFloat(state.get("start_energy_kwh")->asFloat() + override_energy.get("energy_wh")->asUint() / 1000.0);
    }, true);

    //if we dont set the target timestamp right away we will have 0 seconds left displayed in the webinterface until we start and end a charge.
    state.get("target_timestamp_ms")->updateUint(map_duration(config_in_use.get("duration")->asUint()));

 #if MODULE_EVSE_V2_AVAILABLE()
    evse_v2.set_charge_limits_slot(32000, true);
 #elif MODULE_EVSE_AVAILABLE()
    evse.set_charge_limits_slot(32000, true);
 #endif


    task_scheduler.scheduleWithFixedDelay([this](){
        static bool was_charging = false;
        bool charging = charge_tracker.current_charge.get("user_id")->asInt() != -1;

        if (!charging && was_charging) {
            if (!api.restorePersistentConfig("charge_limits/default_limits", &config_in_use))
            {
                config_in_use.get("duration")->updateUint(config.get("duration")->asUint());
                config_in_use.get("energy_wh")->updateUint(config.get("energy_wh")->asUint());
            }

#if MODULE_EVSE_V2_AVAILABLE()
            evse_v2.set_charge_limits_slot(32000, true);
#elif MODULE_EVSE_AVAILABLE()
            evse.set_charge_limits_slot(32000, true);
#endif
            state.get("start_timestamp_ms")->updateUint(0);
            state.get("start_energy_kwh")->updateFloat(NAN);
            state.get("target_timestamp_ms")->updateUint(map_duration(config_in_use.get("duration")->asUint()));
            state.get("target_energy_kwh")->updateFloat(NAN);
        }


#if MODULE_EVSE_V2_AVAILABLE()
        auto uptime = evse_v2.evse_low_level_state.get("uptime")->asUint();
#elif MODULE_EVSE_AVAILABLE()
        auto uptime = evse.evse_low_level_state.get("uptime")->asUint();
#endif
        if (charging && !was_charging) {
            state.get("start_timestamp_ms")->updateUint(charge_tracker.current_charge.get("evse_uptime_start")->asUint());
            if (api.hasFeature("meter") && !isnan(charge_tracker.current_charge.get("meter_start")->asFloat()))
                state.get("start_energy_kwh")->updateFloat(charge_tracker.current_charge.get("meter_start")->asFloat());
        }

        uint16_t target_current = 32000;

        if (charging && config_in_use.get("duration")->asUint() > 0)
        {
            if (!was_charging)
                state.get("target_timestamp_ms")->updateUint(state.get("start_timestamp_ms")->asUint()
                                                                + map_duration(config_in_use.get("duration")->asUint()));

            if (a_after_b(uptime, state.get("target_timestamp_ms")->asUint()))
                target_current = 0;
        }

        if (charging && api.hasFeature("meter") && config_in_use.get("energy_wh")->asUint() > 0)
        {
            auto start = state.get("start_energy_kwh")->asFloat();
            if (!was_charging && !isnan(start))
                state.get("target_energy_kwh")->updateFloat(start + config_in_use.get("energy_wh")->asUint() / 1000.0);

            auto target = state.get("target_energy_kwh")->asFloat();
            auto now = meter.values.get("energy_abs")->asFloat();

            if (!isnan(target) && !isnan(now) && target <= now)
                target_current = 0;
        }

#if MODULE_EVSE_V2_AVAILABLE()
        evse_v2.set_charge_limits_slot(target_current, true);
#elif MODULE_EVSE_AVAILABLE()
        evse.set_charge_limits_slot(target_current, true);
#endif

        was_charging = charging;

    }, 0, 1000);
#endif
}
