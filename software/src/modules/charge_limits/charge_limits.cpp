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

#include "generated/module_dependencies.h"
#include "tools.h"

// This is here so I stop commiting changes meant for debugging
#ifndef CHARGE_LIMITS_TIME_MODIFIER
# define CHARGE_LIMITS_TIME_MODIFIER 60
#endif

static uint32_t map_duration(uint32_t val)
{
    switch (val)
    {
        case 1:
            return      15 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 2:
            return      30 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 3:
            return      45 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 4:
            return      60 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 5:
            return  2 * 60 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 6:
            return  3 * 60 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 7:
            return  4 * 60 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 8:
            return  6 * 60 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 9:
            return  8 * 60 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        case 10:
            return 12 * 60 * CHARGE_LIMITS_TIME_MODIFIER * 1000;
        default:
            return 0;
    }
}

void ChargeLimits::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"duration",       Config::Uint(0, 0, 10)},
        {"energy_wh",      Config::Uint32(0)},
#if OPTIONS_PRODUCT_ID_IS_WARP4()
        {"soc_target_pct", Config::Uint(0, 0, 100)},
#endif
    }), [this](Config &cfg, ConfigSource source) -> String {
        if (source == ConfigSource::File)
            return "";

        if (active_limits.get("duration")->asUint() == this->config.get("duration")->asUint())
            this->apply_duration_override(cfg.get("duration")->asUint());

        if (active_limits.get("energy_wh")->asUint() == this->config.get("energy_wh")->asUint())
            this->apply_energy_override(cfg.get("energy_wh")->asUint());

#if OPTIONS_PRODUCT_ID_IS_WARP4()
        if (active_limits.get("soc_target_pct")->asUint() == this->config.get("soc_target_pct")->asUint())
            this->apply_soc_target_override(cfg.get("soc_target_pct")->asUint());
#endif

        return "";
    }};

    state = Config::Object({
        {"start_timestamp_ms", Config::Uint32(0)},
        {"start_energy_kwh",   Config::Float(NAN)},
        {"target_timestamp_ms", Config::Uint32(0)},
        {"target_energy_kwh",  Config::Float(NAN)},
#if OPTIONS_PRODUCT_ID_IS_WARP4()
        {"soc_target_pct",     Config::Uint(0, 0, 100)},
        {"current_soc_pct",    Config::Float(NAN)},
#endif
    });

    override_duration = Config::Object({
        {"duration", Config::Uint(0, 0, 10)}
    });

    override_energy = Config::Object({
        {"energy_wh", Config::Uint32(0)}
    });

#if OPTIONS_PRODUCT_ID_IS_WARP4()
    override_soc = Config::Object({
        {"soc_target_pct", Config::Uint(0, 0, 100)}
    });
#endif

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_trigger(AutomationTriggerID::ChargeLimits, *Config::Null());

    automation.register_action(
        AutomationActionID::ChargeLimits,
        Config::Object({
            {"restart",        Config::Bool(false)},
            {"duration",       Config::Int(0, -1, 10)},
            {"energy_wh",      Config::Int(0, -1, std::numeric_limits<int32_t>::max())},
#if OPTIONS_PRODUCT_ID_IS_WARP4()
            {"soc_target_pct", Config::Int(0, -1, 100)},
#endif
        }),
        [this](const Config *conf) {
            if (conf->get("restart")->asBool()) {
                api.callCommand("charge_limits/restart");
            }

            int duration = conf->get("duration")->asInt();
            if (duration >= 0) {
                this->apply_duration_override(static_cast<uint32_t>(duration));
            }

            int energy_wh = conf->get("energy_wh")->asInt();
            if (energy_wh >= 0) {
                this->apply_energy_override(static_cast<uint32_t>(energy));
            }

#if OPTIONS_PRODUCT_ID_IS_WARP4()
            int soc_target_pct = conf->get("soc_target_pct")->asInt();
            if (soc_target_pct >= 0) {
                this->apply_soc_target_override(static_cast<uint32_t>(soc_target_pct));
            }
#endif
        }
    );
#endif
}

void ChargeLimits::setup()
{
    api.restorePersistentConfig("charge_limits/default_limits", &config);
    active_limits = config;
    initialized = true;
}

float ChargeLimits::get_energy_limit() {
    float target = state.get("target_energy_kwh")->asFloat();
    float start = state.get("start_energy_kwh")->asFloat();
    return target - start;
}

void ChargeLimits::apply_duration_override(uint32_t duration) {
    was_triggered = false;
    active_limits.get("duration")->updateUint(duration);

    if (duration == 0)
        state.get("target_timestamp_ms")->updateUint(0);
    else
        state.get("target_timestamp_ms")->updateUint(state.get("start_timestamp_ms")->asUint() + map_duration(duration));
}

void ChargeLimits::apply_energy_override(uint32_t energy_wh) {
    was_triggered = false;
    active_limits.get("energy_wh")->updateUint(energy_wh);

    if (energy_wh == 0)
        state.get("target_energy_kwh")->updateFloat(NAN);
    else
        state.get("target_energy_kwh")->updateFloat(state.get("start_energy_kwh")->asFloat() + energy_wh / 1000.0f);
}

#if OPTIONS_PRODUCT_ID_IS_WARP4()
void ChargeLimits::apply_soc_target_override(uint32_t soc_target) {
    was_triggered = false;
    // This is not the same pattern as the apply_duration/energy_override functions,
    // because the SoC target is an absolute value (i.e. "charge to 80%"), not a relative
    // one such as the duration ("charge for one hour") or the energy "charge 10 kWh").
    active_limits.get("soc_target_pct")->updateUint(override_soc.get("soc_target_pct")->asUint());
    state.get("soc_target_pct")->updateUint(override_soc.get("soc_target_pct")->asUint());
}
#endif

void ChargeLimits::register_urls()
{
    api.addPersistentConfig("charge_limits/default_limits", &config);
    api.addState("charge_limits/state", &state);
    api.addState("charge_limits/active_limits", &active_limits);

    api.addCommand("charge_limits/override_duration", &override_duration, {}, [this](Language /*language*/, String &/*errmsg*/) {
        this->apply_duration_override(override_duration.get("duration")->asUint());
    }, true);

    api.addCommand("charge_limits/override_energy", &override_energy, {}, [this](Language /*language*/, String &/*errmsg*/) {
        this->apply_energy_override(override_energy.get("energy_wh")->asUint());
    }, true);

#if OPTIONS_PRODUCT_ID_IS_WARP4()
    api.addCommand("charge_limits/override_soc", &override_soc, {}, [this](Language /*language*/, String &/*errmsg*/) {
        this->apply_soc_target_override(override_soc.get("soc_target_pct")->asUint());
    }, true);
#endif

    api.addCommand("charge_limits/restart", Config::Null(), {}, [this](Language /*language*/, String &/*errmsg*/) {
        if (charge_tracker.current_charge.get("user_id")->asInt() == -1) {
            return;
        }

        was_triggered = false;
        auto time_now = evse_common.get_low_level_state().get("uptime")->asUint();
        state.get("start_timestamp_ms")->updateUint(time_now);
        state.get("target_timestamp_ms")->updateUint(time_now + map_duration(active_limits.get("duration")->asUint()));

        float energy_now;
        evse_common.get_charger_meter_energy(&energy_now); // TODO: Use value freshness?
        if (!isnan(energy_now)) {
            state.get("start_energy_kwh")-> updateFloat(energy_now);
            state.get("target_energy_kwh")->updateFloat(energy_now + active_limits.get("energy_wh")->asUint() / 1000.0f);
        }

#if OPTIONS_PRODUCT_ID_IS_WARP4()
        state.get("soc_target_pct")->updateUint(active_limits.get("soc_target_pct")->asUint());
#endif
    }, true);

    //if we dont set the target timestamp right away we will have 0 seconds left displayed in the webinterface until we start and end a charge.
    state.get("target_timestamp_ms")->updateUint(map_duration(active_limits.get("duration")->asUint()));

    evse_common.set_charge_limits_slot(32000, true);


    task_scheduler.scheduleUncancelable([this](){
        bool charging = charge_tracker.current_charge.get("user_id")->asInt() != -1;
        uint16_t target_current = 32000;

        if (!charging) {
            if (was_charging) {
                // Reset limits to configured defaults.
                active_limits.get("duration"      )->updateUint(config.get("duration"      )->asUint());
                active_limits.get("energy_wh"     )->updateUint(config.get("energy_wh"     )->asUint());
#if OPTIONS_PRODUCT_ID_IS_WARP4()
                active_limits.get("soc_target_pct")->updateUint(config.get("soc_target_pct")->asUint());
#endif

                evse_common.set_charge_limits_slot(32000, true);

                state.get("start_timestamp_ms")->updateUint(0);
                state.get("start_energy_kwh")->updateFloat(NAN);
                state.get("target_timestamp_ms")->updateUint(map_duration(active_limits.get("duration")->asUint()));
                state.get("target_energy_kwh")->updateFloat(NAN);
#if OPTIONS_PRODUCT_ID_IS_WARP4()
                state.get("soc_target_pct")->updateUint(0);
                state.get("current_soc_pct")->updateFloat(NAN);
#endif
            }
        } else { // charging
            float energy_now_kwh;
            evse_common.get_charger_meter_energy(&energy_now_kwh); // TODO: Use freshness check?

            if (!was_charging) {
                state.get("start_timestamp_ms")->updateUint(charge_tracker.current_charge.get("evse_uptime_start")->asUint());
                if (!isnan(charge_tracker.current_charge.get("meter_start")->asFloat()))
                    state.get("start_energy_kwh")->updateFloat(charge_tracker.current_charge.get("meter_start")->asFloat());
            }

            if (active_limits.get("duration")->asUint() > 0) {
                if (!was_charging)
                    state.get("target_timestamp_ms")->updateUint(state.get("start_timestamp_ms")->asUint()
                                                                    + map_duration(active_limits.get("duration")->asUint()));

                uint32_t uptime = evse_common.get_low_level_state().get("uptime")->asUint();
                if (a_after_b(uptime, state.get("target_timestamp_ms")->asUint()))
                    target_current = 0;
            }

            if (active_limits.get("energy_wh")->asUint() > 0 && !isnan(energy_now_kwh)) {
                if (!was_charging) {
                    float start = state.get("start_energy_kwh")->asFloat();
                    if (!isnan(start)) {
                        state.get("target_energy_kwh")->updateFloat(start + active_limits.get("energy_wh")->asUint() / 1000.0f);
                    }
                }

                float target = state.get("target_energy_kwh")->asFloat();
                if (!isnan(target)) {
                    if (!isnan(energy_now_kwh) && target <= energy_now_kwh) {
                        target_current = 0;
                    }
                }
            }

            // SoC target check
#if OPTIONS_PRODUCT_ID_IS_WARP4()
            if (active_limits.get("soc_target_pct")->asUint() > 0 && ev_meter_has_soc) {
                if (!was_charging) {
                    state.get("soc_target_pct")->updateUint(active_limits.get("soc_target_pct")->asUint());
                }

                float soc = NAN;
                meters.get_soc(ev_meter_slot, &soc);
                state.get("current_soc_pct")->updateFloat(soc);

                if (!isnan(soc) && soc >= static_cast<float>(active_limits.get("soc_target_pct")->asUint())) {
                    target_current = 0;
                }
            }
#endif
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

    }, 1_s);
}

#if OPTIONS_PRODUCT_ID_IS_WARP4()
void ChargeLimits::setup_ev_meter_soc(uint32_t slot)
{
    event.registerEvent(meters.get_path(slot, Meters::PathType::ValueIDs), {}, [this, slot](const Config *cfg) {
        const size_t count = cfg->count();

        if (count == 0) {
            return EventResult::OK;
        }

        const MeterValueID soc_vid = MeterValueID::StateOfCharge;
        uint32_t soc_index = UINT32_MAX;
        meters.fill_index_cache(slot, 1, &soc_vid, &soc_index);

        if (soc_index == UINT32_MAX) {
            return EventResult::OK;
        }

        ev_meter_slot = slot;
        ev_meter_soc_index = soc_index;
        ev_meter_has_soc = true;

        return EventResult::Deregister;
    });
}

void ChargeLimits::register_events()
{
    for (uint32_t slot = 0; slot < OPTIONS_METERS_MAX_SLOTS(); slot++) {
        if (meters.get_meter_location(slot) == MeterLocation::EV) {
            setup_ev_meter_soc(slot);
            break;
        }
    }
}
#endif

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
