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

#include "module_dependencies.h"

extern ChargeLimits charge_limits;

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

#if MODULE_CRON_AVAILABLE()
    ConfUnionPrototype proto;
    proto.tag = CRON_TRIGGER_CHARGE_LIMITS;
    proto.config = *Config::Null();
    cron.register_trigger(proto);

    proto.tag = CRON_ACTION_CHARGE_LIMITS;
    proto.config = Config::Object({
        {"duration", Config::Uint32(0)},
        {"energy_wh", Config::Uint32(0)}
    });

    cron.register_action(proto, [this](const Config *conf) {
        config_in_use.get("duration")->updateUint(conf->get("duration")->asUint());
        state.get("target_timestamp_ms")->updateUint(state.get("start_timestamp_ms")->asUint() + map_duration(conf->get("duration")->asUint()));

        config_in_use.get("energy_wh")->updateUint(conf->get("energy_wh")->asUint());
        state.get("target_energy_kwh")->updateFloat(state.get("start_energy_kwh")->asFloat() + conf->get("energy_wh")->asUint() / 1000.0);
    });
#endif
}

void ChargeLimits::setup()
{
    api.restorePersistentConfig("charge_limits/default_limits", &config);
    config_in_use = config;
    initialized = true;
}

#if MODULE_CRON_AVAILABLE()
static bool trigger_action(Config *cfg, void *data) {
    return charge_limits.action_triggered(cfg, data);
}
#endif

void ChargeLimits::register_urls()
{
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

    evse_common.set_charge_limits_slot(32000, true);


    task_scheduler.scheduleWithFixedDelay([this](){
        static bool was_charging = false;
        bool charging = charge_tracker.current_charge.get("user_id")->asInt() != -1;

        if (!charging && was_charging) {
            if (!api.restorePersistentConfig("charge_limits/default_limits", &config_in_use))
            {
                config_in_use.get("duration")->updateUint(config.get("duration")->asUint());
                config_in_use.get("energy_wh")->updateUint(config.get("energy_wh")->asUint());
            }

            evse_common.set_charge_limits_slot(32000, true);

            state.get("start_timestamp_ms")->updateUint(0);
            state.get("start_energy_kwh")->updateFloat(NAN);
            state.get("target_timestamp_ms")->updateUint(map_duration(config_in_use.get("duration")->asUint()));
            state.get("target_energy_kwh")->updateFloat(NAN);
        }


        auto uptime = evse_common.get_low_level_state().get("uptime")->asUint();

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

#if MODULE_CRON_AVAILABLE()
        static bool was_triggered = false;
        if (target_current == 0 && !was_triggered) {
            cron.trigger_action(CRON_TRIGGER_CHARGE_LIMITS, nullptr, &trigger_action);
            was_triggered = true;
        } else if (!charging) {
            was_triggered = false;
        }
#endif

        evse_common.set_charge_limits_slot(target_current, true);

        was_charging = charging;

    }, 0, 1000);
}

#if MODULE_CRON_AVAILABLE()
    bool ChargeLimits::action_triggered(Config *config, void *data) {
        switch (config->getTag()) {
        case CRON_TRIGGER_CHARGE_LIMITS:
            return true;

        default:
            break;
        }
        return false;
    }
#endif