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

#include "module_dependencies.h"
#include "tools.h"

// This is here so I stop commiting changes meant for debugging
#ifndef CHARGE_LIMITS_TIME_MODIFIER
# define CHARGE_LIMITS_TIME_MODIFIER 60
#endif

extern ChargeLimits charge_limits;

static uint32_t map_duration(uint32_t val)
{
    switch (val)
    {
        case 1:
            return 15 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 2:
            return 30 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 3:
            return 45 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 4:
            return 60 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 5:
            return 2 * CHARGE_LIMITS_TIME_MODIFIER * 60 * 1000;
        case 6:
            return 3 * CHARGE_LIMITS_TIME_MODIFIER * 60 * 1000;
        case 7:
            return 4 * CHARGE_LIMITS_TIME_MODIFIER * 60 * 1000;
        case 8:
            return 6 * CHARGE_LIMITS_TIME_MODIFIER * 60 * 1000;
        case 9:
            return 8 * CHARGE_LIMITS_TIME_MODIFIER * 60 * 1000;
        case 10:
            return 12 * CHARGE_LIMITS_TIME_MODIFIER * 60 * 1000;
        default:
            return 0;
    }
}

void ChargeLimits::pre_setup()
{
    config = Config::Object({
        {"duration", Config::Uint(0, 0, 10)},
        {"energy_wh", Config::Uint32(0)},
    });

    state = Config::Object({
        {"start_timestamp_ms", Config::Uint32(0)},
        {"start_energy_kwh", Config::Float(NAN)},
        {"target_timestamp_ms", Config::Uint32(0)},
        {"target_energy_kwh", Config::Float(NAN)}
    });

    override_duration = Config::Object({
        {"duration", Config::Uint32(0)}
    });

    override_energy = Config::Object({
        {"energy_wh", Config::Uint32(0)}
    });

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_trigger(AutomationTriggerID::ChargeLimits, *Config::Null());

    automation.register_action(
        AutomationActionID::ChargeLimits,
        Config::Object({
            {"restart", Config::Bool(false)},
            {"duration", Config::Int32(0)},
            {"energy_wh", Config::Int32(0)}
        }),
        [this](const Config *conf) {
            if (conf->get("restart")->asBool()) {
                api.callCommand("charge_limits/restart", {});
            }

            int duration = conf->get("duration")->asInt();
            if (duration != -1) {
                api.callCommand("charge_limits/override_duration", Config::ConfUpdateObject {{
                    {"duration", static_cast<uint32_t>(duration)}
                }});
            }

            int energy_wh = conf->get("energy_wh")->asInt();
            if (energy_wh != -1) {
                api.callCommand("charge_limits/override_energy", Config::ConfUpdateObject{{
                    {"energy_wh", static_cast<uint32_t>(energy_wh)}
                }});
            }
        }
    );
#endif
}

void ChargeLimits::setup()
{
    api.restorePersistentConfig("charge_limits/default_limits", &config);
    config_in_use = config;
    initialized = true;
}

void ChargeLimits::register_urls()
{
    api.addPersistentConfig("charge_limits/default_limits", &config);
    api.addState("charge_limits/state", &state);
    api.addState("charge_limits/active_limits", &config_in_use);

    api.addCommand("charge_limits/override_duration", &override_duration, {}, [this]() {
        was_triggered = false;
        config_in_use.get("duration")->updateUint(override_duration.get("duration")->asUint());

        if (override_duration.get("duration")->asUint() == 0)
            state.get("target_timestamp_ms")->updateUint(0);
        else
            state.get("target_timestamp_ms")->updateUint(state.get("start_timestamp_ms")->asUint() + map_duration(override_duration.get("duration")->asUint()));
    }, true);

    api.addCommand("charge_limits/override_energy", &override_energy, {}, [this]() {
        was_triggered = false;
        config_in_use.get("energy_wh")->updateUint(override_energy.get("energy_wh")->asUint());
        if (override_energy.get("energy_wh")->asUint() == 0)
            state.get("target_energy_kwh")->updateFloat(NAN);
        else
            state.get("target_energy_kwh")->updateFloat(state.get("start_energy_kwh")->asFloat() + override_energy.get("energy_wh")->asUint() / 1000.0);
    }, true);

    api.addCommand("charge_limits/restart", Config::Null(), {}, [this]() {
        if (charge_tracker.current_charge.get("user_id")->asInt() == -1) {
            return;
        }

        was_triggered = false;
        auto time_now = evse_common.get_low_level_state().get("uptime")->asUint();
        state.get("start_timestamp_ms")->updateUint(time_now);
        state.get("target_timestamp_ms")->updateUint(time_now + map_duration(config_in_use.get("duration")->asUint()));

        float energy_now;
        evse_common.get_charger_meter_energy(&energy_now); // TODO: Use value freshness?
        if (!isnan(energy_now)) {
            state.get("start_energy_kwh")-> updateFloat(energy_now);
            state.get("target_energy_kwh")->updateFloat(energy_now + config_in_use.get("energy_wh")->asUint() / 1000.0);
        }
    }, true);

    //if we dont set the target timestamp right away we will have 0 seconds left displayed in the webinterface until we start and end a charge.
    state.get("target_timestamp_ms")->updateUint(map_duration(config_in_use.get("duration")->asUint()));

    evse_common.set_charge_limits_slot(32000, true);


    task_scheduler.scheduleWithFixedDelay([this](){
        static bool was_charging = false;
        bool charging = charge_tracker.current_charge.get("user_id")->asInt() != -1;
        uint16_t target_current = 32000;

        if (!charging) {
            if (was_charging) {
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
        } else { // charging
            float energy_now_kwh;
            evse_common.get_charger_meter_energy(&energy_now_kwh); // TODO: Use freshness check?

            if (!was_charging) {
                state.get("start_timestamp_ms")->updateUint(charge_tracker.current_charge.get("evse_uptime_start")->asUint());
                if (!isnan(charge_tracker.current_charge.get("meter_start")->asFloat()))
                    state.get("start_energy_kwh")->updateFloat(charge_tracker.current_charge.get("meter_start")->asFloat());
            }

            if (config_in_use.get("duration")->asUint() > 0) {
                if (!was_charging)
                    state.get("target_timestamp_ms")->updateUint(state.get("start_timestamp_ms")->asUint()
                                                                    + map_duration(config_in_use.get("duration")->asUint()));

                uint32_t uptime = evse_common.get_low_level_state().get("uptime")->asUint();
                if (a_after_b(uptime, state.get("target_timestamp_ms")->asUint()))
                    target_current = 0;
            }

            if (config_in_use.get("energy_wh")->asUint() > 0 && !isnan(energy_now_kwh)) {
                if (!was_charging) {
                    float start = state.get("start_energy_kwh")->asFloat();
                    if (!isnan(start)) {
                        state.get("target_energy_kwh")->updateFloat(start + config_in_use.get("energy_wh")->asUint() / 1000.0);
                    }
                }

                float target = state.get("target_energy_kwh")->asFloat();
                if (!isnan(target)) {
                    if (!isnan(energy_now_kwh) && target <= energy_now_kwh) {
                        target_current = 0;
                    }
                }
            }
        }

#if MODULE_AUTOMATION_AVAILABLE()
        if (target_current == 0 && !was_triggered) {
            automation.trigger(AutomationTriggerID::ChargeLimits, nullptr, this);
            was_triggered = true;
        } else if (!charging) {
            was_triggered = false;
        }
#endif

        evse_common.set_charge_limits_slot(target_current, true);

        was_charging = charging;

    }, 0, 1000);
}

#if MODULE_AUTOMATION_AVAILABLE()
bool ChargeLimits::has_triggered(const Config *conf, void *data) {
    switch (conf->getTag<AutomationTriggerID>()) {
    case AutomationTriggerID::ChargeLimits:
        return true;

    default:
        break;
    }
    return false;
}
#endif
