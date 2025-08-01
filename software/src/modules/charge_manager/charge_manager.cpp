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

#include "charge_manager.h"

#include <algorithm>
#include <esp_http_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "current_allocator.h"
#include "build.h"
#include "tools.h"
#include "cm_phase_rotation.enum.h"

static constexpr micros_t WATCHDOG_TIMEOUT = 30_s;

// If this is an energy manager, we have exactly one charger and the margin is still the default,
// double it to react faster if more current is available.
#define REQUESTED_CURRENT_MARGIN_DEFAULT 3000

#if MODULE_AUTOMATION_AVAILABLE()
bool ChargeManager::has_triggered(const Config *conf, void *data)
{
    switch (conf->getTag<AutomationTriggerID>()) {
        case AutomationTriggerID::ChargeManagerWd:
            return true;

        default:
            return false;
    }
}
#endif

static PhaseRotation convert_phase_rotation(CMPhaseRotation pr) {
    switch (pr) {
        case CMPhaseRotation::Unknown: return PhaseRotation::Unknown;
        case CMPhaseRotation::L123:    return PhaseRotation::L123;
        case CMPhaseRotation::L132:    return PhaseRotation::L132;
        case CMPhaseRotation::L231:    return PhaseRotation::L231;
        case CMPhaseRotation::L213:    return PhaseRotation::L213;
        case CMPhaseRotation::L321:    return PhaseRotation::L321;
        case CMPhaseRotation::L312:    return PhaseRotation::L312;
        default: return PhaseRotation::Unknown;
    }
}

void ChargeManager::pre_setup()
{
    this->trace_buffer_index = logger.alloc_trace_buffer("charge_manager", 2 << 20);

    config_chargers_prototype = Config::Object({
        {"host", Config::Str("", 0, 64)},
        {"name", Config::Str("", 0, 32)},
        {"rot", Config::Enum(CMPhaseRotation::Unknown)}
    });

    config = ConfigRoot{Config::Object({
        {"enable_charge_manager", Config::Bool(false)},
        {"enable_watchdog", Config::Bool(false)},
        {"default_available_current", Config::Uint32(0)},
        {"maximum_available_current", Config::Uint(0, 0, 32000 * MAX_CONTROLLED_CHARGERS)},
        {"minimum_current_auto", Config::Bool(true)},
        {"minimum_current", Config::Uint(6000, 6000, 32000)},
        {"minimum_current_1p", Config::Uint(6000, 6000, 32000)},
        {"minimum_current_vehicle_type", Config::Uint32(0)},
        {"verbose", Config::Bool(false)},
        // Line currents are used to calculate the "requested current"
        // if a charger is in state C for at least
        // "requested_current_threshold" s.
        // Otherwise the "requested current" is the "supported current"
        // to allow the car to ramp up fast.
        {"requested_current_threshold", Config::Uint16(60)},
        // Added to requested current before distributing current.
        // This margin has to be large enough to make sure
        // every vehicle can always request more current.
        // See https://github.com/Tinkerforge/warp-charger/blob/master/tools/current_ramp/allowed_vs_effective_3.gp
        {"requested_current_margin", Config::Uint16(REQUESTED_CURRENT_MARGIN_DEFAULT)},
        {"chargers", Config::Array({},
            &config_chargers_prototype,
            0, MAX_CONTROLLED_CHARGERS, Config::type_id<Config::ConfObject>()
        )}
    }), [](Config &conf, ConfigSource source) -> String {
        if (conf.get("enable_charge_manager")->asBool() && conf.get("chargers")->count() == 0)
            return "at least one charger is required";

        uint32_t default_available_current = conf.get("default_available_current")->asUint();
        uint32_t maximum_available_current = conf.get("maximum_available_current")->asUint();

        if (default_available_current > maximum_available_current)
            return "default_available_current can not be greater than maximum_available_current";

        if (conf.get("minimum_current_auto")->asBool()) {
            auto minimum_current_vehicle_type = conf.get("minimum_current_vehicle_type")->asUint();
            uint32_t min_1p;
            uint32_t min_3p;

            if (minimum_current_vehicle_type > 0) {
                min_1p = 6000;
                min_3p = 9200;
            } else {
                min_1p = 6000;
                min_3p = 6000;
            }

            conf.get("minimum_current_1p")->updateUint(min_1p);
            conf.get("minimum_current")->updateUint(min_3p);
        }

        auto chargers = conf.get("chargers");

        for (size_t i = 0; i < chargers->count(); i++)
            for (size_t a = i + 1; a < chargers->count(); a++)
                if (chargers->get(i)->get("host")->asString() == chargers->get(a)->get("host")->asString())
                    return "there must not be two chargers with the same hostname or IP address";

        return "";
    }};

    low_level_config = Config::Object({
        // 3 * 60 + 30 seconds to make sure we don't switch anything
        // while the EVSE is waking up a vehicle via IEC state F.
        {"global_hysteresis", Config::Uint(3 * 60 + 30, 0, 60 * 60)},
        {"wakeup_time", Config::Uint(3 * 60 + 30, 0, 60 * 60)},
        {"plug_in_time", Config::Uint(3 * 60 + 30, 0, 60 * 60)},
        {"enable_current_factor_pct", Config::Uint(150, 100, 300)},
        {"allocation_interval", Config::Uint(10, 1, 60 * 60)},
        {"rotation_interval", Config::Uint(15 * 60, 0, 24 * 60 * 60)},
    });

    const Config *config_prototype_int32_0 = Config::get_prototype_int32_0();

    state_chargers_prototype = Config::Object({
        {"s",  Config::Uint(0, 0, 6)},  // "state" - 0 - no vehicle, 1 - user blocked, 2 - manager blocked, 3 - car blocked, 4 - charging, 5 - error, 6 - charged
        {"e",  Config::Uint8(0)},       // "error" - 0 - okay, 1 - unreachable, 2 - FW mismatch, 3 - not managed, >= 192 client errors
        {"ac", Config::Uint16(0)},      // "allocated_current" - last current limit send to the charger
        {"ap", Config::Uint(0, 0, 3)},  // "allocated_phases" - last phase limit send to the charger
        {"sc", Config::Uint16(0)},      // "supported_current" - maximum current supported by the charger
        {"sp", Config::Uint(0, 0, 3)},  // "supported_phases" - maximum phases supported by the charger. Is 1 or 3 for a not phase-switchable charger. Bit 2 is set if it can be switched.
        {"lu", Config::Uint32(0)},      // "last_update" - The last time we received a CM packet from this charger
        {"n",  Config::Str("", 0, 32)}, // "name" - Configured display name. Has to be duplicated in case the config was written but we didn't reboot.
        {"u",  Config::Uint32(0)}       // "uid" - The ESP's UID
    });

    // This has to fit in the 6k WebSocket send buffer with 32 chargers with long names. (Currently 4076 bytes)
    // -> Be stingy with the key names. If we need more space we could switch to an array for all numbers.
    // This API only exists to communicate with the web interface and is not documented.
    state = Config::Object({
        {"state", Config::Uint8(0)}, // 0 - not configured, 1 - active, 2 - shutdown
        {"l_raw", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            config_prototype_int32_0,
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"l_min", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            config_prototype_int32_0,
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"l_spread", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            config_prototype_int32_0,
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"l_max_pv", Config::Int32(0)},
        {"alloc", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            config_prototype_int32_0,
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"chargers", Config::Array(
            {},
            &state_chargers_prototype,
            0, MAX_CONTROLLED_CHARGERS, Config::type_id<Config::ConfObject>()
        )}
    });

    low_level_state_chargers_prototype = Config::Object({
        {"b",  Config::Uint(0, 0, 0x3F)},  /* "bits"
                                                0 cp_disc_supported
                                                1 cp_disc_state
                                                2 phase_switch_supported
                                                3 phases (2)*/
        {"rc", Config::Uint16(0)},         // "requested_current" - either the supported current or (after requested_current_threshold is elapsed in state C) the max phase current + requested_current_margin
        {"ae", Config::Uint(0, 9, 99999)}, // "allocated_energy" in Wh, values > 99999 Wh are truncated to 99999.
        {"ls", Config::Uint32(0)},         // "last_switch_on" in millis
        {"lp", Config::Uint32(0)},         // "just_plugged_in_timestamp" in millis
        {"lw", Config::Uint32(0)},         // "last_wakeup" in millis
        {"ip", Config::Uint32(0)},         // "use_supported_current" in millis
    });

    low_level_state = Config::Object({
        {"last_hyst_reset", Config::Uint32(0)},
        {"wnd_min", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            config_prototype_int32_0,
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"wnd_max", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            config_prototype_int32_0,
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"chargers", Config::Array(
            {},
            &low_level_state_chargers_prototype,
            0, MAX_CONTROLLED_CHARGERS, Config::type_id<Config::ConfObject>()
        )}
    });

    available_current = ConfigRoot{Config::Object({
        {"current", Config::Uint32(0)},
    }), [this](const Config &conf, ConfigSource source) -> String {
        if (source == ConfigSource::API && !this->static_cm) {
            logger.printfln("Cannot set available_current via the API if the Power Manager is enabled.");
            return "Cannot set available_current if the Power Manager is enabled.";
        }

        uint32_t max_avail_current = this->get_maximum_available_current();

        if (conf.get("current")->asUint() > max_avail_current)
            return "Current too large: maximum available current is configured to " + String(max_avail_current);
        return "";
    }};
    available_current_update = available_current;

    control_pilot_disconnect = Config::Object({
        {"disconnect", Config::Bool(false)},
    });

    charge_mode = Config::Object({
        {"mode", Config::Enum(ConfigChargeMode::Fast)},
    });

#ifdef DEBUG_FS_ENABLE
    debug_limits_update = Config::Object({
        {"raw", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            config_prototype_int32_0,
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"min", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            config_prototype_int32_0,
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"spread", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0)
            },
            config_prototype_int32_0,
            4, 4, Config::type_id<Config::ConfInt>())
        },
        {"max_pv", Config::Int32(0)},
    });
#endif

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_trigger(
        AutomationTriggerID::ChargeManagerWd,
        *Config::Null(),
        nullptr, false
    );

    automation.register_action(
        AutomationActionID::SetManagerCurrent,
        Config::Object({
            {"current", Config::Uint32(0)}
        }),
        [this](const Config *config) {
            String err = api.callCommand("charge_manager/available_current_update", Config::ConfUpdateObject{{
                {"current", config->get("current")->asUint()},
            }});
            if (!err.isEmpty()) {
                logger.printfln("Automation couldn't set manager current: %s", err.c_str());
            }
        },
        [this](const Config *config) -> String {
            uint32_t max_avail_current = this->get_maximum_available_current();

            if (config->get("current")->asUint() > max_avail_current)
                return "Current too large: maximum available current is configured to " + String(max_avail_current);
            return "";
        },
        false);

    automation.register_action(
        AutomationActionID::PMChargeModeSwitch,
        Config::Object({
            {"mode", Config::Enum(ConfigChargeMode::Fast)}
        }),
        [this](const Config *cfg) {
            const String err = api.callCommand("power_manager/charge_mode_update", Config::ConfUpdateObject{{
                {"mode", cfg->get("mode")->asEnumUnderlyingType<ConfigChargeMode>()}
            }});
            if (!err.isEmpty()) {
                logger.printfln("Automation couldn't switch charge mode: %s", err.c_str());
            }
        },
        nullptr,
        false);
#endif
}

void ChargeManager::start_manager_task()
{
    auto get_charger_name_fn = [this](uint8_t i){ return this->get_charger_name(i);};

    cm_networking.register_manager(this->hosts.get(), charger_count, [this, get_charger_name_fn](uint8_t client_id, cm_state_v1 *v1, cm_state_v2 *v2, cm_state_v3 *v3) mutable {
            if (update_from_client_packet(
                    client_id,
                    v1,
                    v2,
                    v3,
                    this->ca_config,
                    this->charger_state,
                    this->charger_allocation_state,
                    this->hosts.get(),
                    get_charger_name_fn
                    ))
                update_charger_state_config(client_id);
    }, [this](uint8_t client_id, uint8_t error){
        //TODO bounds check
        auto &target_alloc = this->charger_allocation_state[client_id];
        target_alloc.state = 5;
        target_alloc.error = error;
        //TODO: should we call update_charger_state_config(client_id); here? This is currently missing but smells weird.
    });

    millis_t cm_send_delay = 1000_ms / millis_t{charger_count};

    task_scheduler.scheduleWithFixedDelay([this](){
        static int i = 0;

        if (i >= charger_count)
            i = 0;

        auto &charger_alloc = this->charger_allocation_state[i];
        if(cm_networking.send_manager_update(i, charger_alloc.allocated_current, charger_alloc.cp_disconnect, charger_alloc.allocated_phases))
            ++i;

    }, cm_send_delay);
}

// This is a separate function to simplify the control flow.
void ChargeManager::update_charger_state_from_mode(ChargerState *state, int charger_idx) {
    auto mode = state->charge_mode;

    // Every charger is off by default.
    // If no bit in mode is set the charger stays off.
    state->off = true;
    state->observe_pv_limit = false;
    state->eco_fast = false;
    state->guaranteed_pv_current = 0;

    for (uint8_t m = ChargeMode::_max; m >= ChargeMode::_min; m = ((uint8_t)m >> 1)) {
        switch((ChargeMode::Type)(mode & m)) {
            case ChargeMode::Fast:
                state->off = false;
                state->observe_pv_limit = false;
                return;

            case ChargeMode::Eco:
#if MODULE_ECO_AVAILABLE()
                switch (eco.get_charge_decision(charger_idx)) {
                    case Eco::ChargeDecision::Fast:
                        state->off = false;
                        state->observe_pv_limit = false;
                        state->eco_fast = true;
                        return;
                    case Eco::ChargeDecision::Normal:
                        continue;
                }
#endif
                continue;

            case ChargeMode::Min: {
                // TODO maybe support guaranteed power == enable_current of this specific charger?
                state->off = false;
                state->guaranteed_pv_current = this->guaranteed_pv_current;
                state->observe_pv_limit = true;
                continue;
            }

            case ChargeMode::PV:
                state->off = false;
                state->observe_pv_limit = true;
                return;
        }
    }
}

uint8_t ChargeManager::translate_charge_mode(ConfigChargeMode power_manager_charge_mode) {
    switch (power_manager_charge_mode) {
        case ConfigChargeMode::Fast:
            return ChargeMode::Fast;
        case ConfigChargeMode::Off:
            return 0;
        case ConfigChargeMode::PV:
            return ChargeMode::PV;
        case ConfigChargeMode::MinPV:
            return ChargeMode::Min | ChargeMode::PV;
        case ConfigChargeMode::Default:
            return this->translate_charge_mode(this->pm_default_charge_mode);
        case ConfigChargeMode::Min:
            return ChargeMode::Min;
        case ConfigChargeMode::Eco:
            return ChargeMode::Eco;
        case ConfigChargeMode::EcoPV:
            return ChargeMode::Eco | ChargeMode::PV;
        case ConfigChargeMode::EcoMin:
            return ChargeMode::Eco | ChargeMode::Min;
        case ConfigChargeMode::EcoMinPV:
            return ChargeMode::Eco | ChargeMode::Min | ChargeMode::PV;
    }
    return 0;
}

void ChargeManager::setup()
{
    api.restorePersistentConfig("charge_manager/config", &config);

    // We could move this below the enable_charge_manager check, but want to always see
    // the configured values in debug reports even if the charge manager is currently
    // disabled.
    api.restorePersistentConfig("charge_manager/low_level_config", &low_level_config);

    // Always set initialized so that the front-end is displayed.
    initialized = true;

    if (!config.get("enable_charge_manager")->asBool() || config.get("chargers")->count() == 0) {
        return;
    }

    // If enable_charge_manager is false, leave charger_count as 0.
    this->charger_count = config.get("chargers")->count();

    state.get("state")->updateUint(1);

    this->ca_config = new CurrentAllocatorConfig();

    ca_config->global_hysteresis                    = seconds_t{low_level_config.get("global_hysteresis")->asUint()};
    ca_config->wakeup_time                          = seconds_t{low_level_config.get("wakeup_time")->asUint()};
    ca_config->plug_in_time                         = seconds_t{low_level_config.get("plug_in_time")->asUint()};
    ca_config->allocation_interval                  = seconds_t{low_level_config.get("allocation_interval")->asUint()};
    ca_config->rotation_interval                    = seconds_t{low_level_config.get("rotation_interval")->asUint()};
    ca_config->enable_current_factor                = low_level_config.get("enable_current_factor_pct")->asUint() / 100.0f;

    ca_config->minimum_current_3p = config.get("minimum_current")->asUint();
    ca_config->minimum_current_1p = config.get("minimum_current_1p")->asUint();
    ca_config->requested_current_margin = config.get("requested_current_margin")->asUint();
    ca_config->requested_current_threshold = config.get("requested_current_threshold")->asUint();

    this->ca_state = new CurrentAllocatorState();

    auto default_current = config.get("default_available_current")->asUint();
    available_current.get("current")->updateUint(default_current);

    low_level_state.get("chargers")->reserve(charger_count);
    state.get("chargers")->reserve(charger_count);

    for (size_t i = 0; i < charger_count; ++i) {
        low_level_state.get("chargers")->add();
        auto state_last_charger = state.get("chargers")->add();
        state_last_charger->get("n")->updateString(config.get("chargers")->get(i)->get("name")->asString());
    }

    size_t hosts_buf_size = 0;
    for (size_t i = 0; i < charger_count; ++i) {
        hosts_buf_size += config.get("chargers")->get(i)->get("host")->asString().length() + 1; //null terminator
    }

    char *hosts_buf = (char *)calloc_psram_or_dram(hosts_buf_size, sizeof(char));
    this->hosts = heap_alloc_array<const char *>(charger_count);
    size_t hosts_written = 0;

    for (size_t i = 0; i < charger_count; ++i) {
        hosts[i] = hosts_buf + hosts_written;
        memcpy(hosts_buf + hosts_written, config.get("chargers")->get(i)->get("host")->asEphemeralCStr(), config.get("chargers")->get(i)->get("host")->asString().length());
        hosts_written += config.get("chargers")->get(i)->get("host")->asString().length();
        hosts_buf[hosts_written] = '\0';
        ++hosts_written;
    }

    ca_config->charger_count = this->charger_count;
    this->charger_state = (ChargerState *)calloc_psram_or_dram(this->charger_count, sizeof(ChargerState));
    this->charger_allocation_state = (ChargerAllocationState *)calloc_psram_or_dram(this->charger_count, sizeof(ChargerAllocationState));

    for (size_t i = 0; i < charger_count; ++i) {
        charger_state[i].phase_rotation = convert_phase_rotation(config.get("chargers")->get(i)->get("rot")->asEnum<CMPhaseRotation>());
        charger_state[i].last_phase_switch = -ca_config->global_hysteresis;
    }

    // TODO: Change all currents everywhere to int32_t or int16_t.
    int def_cur = (int) default_current;
    this->limits.raw = {0, def_cur, def_cur, def_cur};
    this->limits.min = {0, def_cur, def_cur, def_cur};
    this->limits.spread = {0, def_cur, def_cur, def_cur};
    this->limits.max_pv = 0;

    start_manager_task();

    auto get_charger_name_fn = [this](uint8_t idx) {return this->get_charger_name(idx);};
    auto notify_charger_unresponsive_fn = [](uint8_t charger_index) {return cm_networking.notify_charger_unresponsive(charger_index);};

    this->next_allocation = now_us() + ca_config->allocation_interval;

    task_scheduler.scheduleWithFixedDelay([this, get_charger_name_fn, notify_charger_unresponsive_fn](){
            if (!deadline_elapsed(this->next_allocation))
                return;

            this->next_allocation = now_us() + ca_config->allocation_interval;

            uint32_t allocated_current = 0;

            // Use copy to not zero static limits forever.
            CurrentLimits tmp_limits;
            if (!seen_all_chargers()) {
                tmp_limits.raw = Cost{0, 0, 0, 0};
                tmp_limits.min = Cost{0, 0, 0, 0};
                tmp_limits.max_pv = 0;
            } else {
                tmp_limits = this->limits;
            }

            this->limits_post_allocation = tmp_limits;

            for(size_t i = 0; i < charger_count; ++i) {
                update_charger_state_from_mode(&charger_state[i], i);
            }

            int result = allocate_current(
                this->ca_config,
                &this->limits_post_allocation,
                this->control_pilot_disconnect.get("disconnect")->asBool(),
                this->charger_state,
                this->hosts.get(),
                get_charger_name_fn,
                notify_charger_unresponsive_fn,

                this->ca_state,
                this->charger_allocation_state,
                &allocated_current
            );

            for (size_t i = 0; i < 4; i++) {
                allocated_currents[i] = tmp_limits.raw[i] - limits_post_allocation.raw[i];
                this->state.get("l_raw")->get(i)->updateInt(tmp_limits.raw[i]);
                this->state.get("l_min")->get(i)->updateInt(tmp_limits.min[i]);
                this->state.get("l_spread")->get(i)->updateInt(tmp_limits.spread[i]);
                this->state.get("alloc")->get(i)->updateInt(allocated_currents[i]);
                this->low_level_state.get("wnd_min")->get(i)->updateInt(this->ca_state->control_window_min[i]);
                this->low_level_state.get("wnd_max")->get(i)->updateInt(this->ca_state->control_window_max[i]);
            }
            this->state.get("l_max_pv")->updateInt(tmp_limits.max_pv);
            this->low_level_state.get("last_hyst_reset")->updateUint(this->ca_state->last_hysteresis_reset.to<millis_t>().as<uint32_t>());

            for (int i = 0; i < this->charger_count; ++i) {
                update_charger_state_config(i);
            }

            this->state.get("state")->updateUint(result);
        }, 1_s);

    if (config.get("verbose")->asBool()) {
        ca_config->distribution_log = heap_alloc_array<char>(DISTRIBUTION_LOG_LEN);
        ca_config->distribution_log_len = DISTRIBUTION_LOG_LEN;
    } else {
        ca_config->distribution_log = nullptr;
        ca_config->distribution_log_len = 0;
    }
}

void ChargeManager::check_watchdog()
{
    if (this->watchdog_triggered || !deadline_elapsed(last_available_current_update + WATCHDOG_TIMEOUT))
        return;

    uint32_t default_available_current = config.get("default_available_current")->asUint();

    logger.printfln("Watchdog triggered! Received no available current update for %d s. Setting available current to %lu mA", WATCHDOG_TIMEOUT.to<seconds_t>().as<int>(), default_available_current);

    // Use the command to update the available current.
    // Setting available_current directly will not update the limits struct.
    const String err = api.callCommand("charge_manager/available_current_update", Config::ConfUpdateObject{{
        {"current", default_available_current},
    }});
    if (!err.isEmpty()) {
        logger.printfln("Watchdog couldn't set manager current: %s", err.c_str());
    }

    this->watchdog_triggered = true; // Set this here because the command call always sets it to false;

#if MODULE_AUTOMATION_AVAILABLE()
    automation.trigger(AutomationTriggerID::ChargeManagerWd, nullptr, this);
#endif
}

size_t ChargeManager::get_charger_count()
{
    return charger_count;
}

bool ChargeManager::seen_all_chargers()
{
    if (all_chargers_seen)
        return true;

    if (now_us() > 30_s) {
        all_chargers_seen = true;
        return true;
    }

    for (size_t i = 0; i < charger_count; ++i)
        if (this->charger_state[i].last_update == 0_us)
            return false;

    logger.printfln("Seen all chargers.");

    all_chargers_seen = true;
    return true;
}

const String &ChargeManager::get_charger_host(uint8_t idx)
{
    // FIXME: This is broken after a config update without reboot.
    return this->config.get("chargers")->get(idx)->get("host")->asString();
}

const char *ChargeManager::get_charger_name(uint8_t idx)
{
    return this->state.get("chargers")->get(idx)->get("n")->asEphemeralCStr();
}

void ChargeManager::register_urls()
{
    // PowerManager::setup() runs after ChargeManager::setup()
    this->guaranteed_pv_current = (power_manager.get_guaranteed_power_w() * 1000) / 230;
    this->pm_default_charge_mode = power_manager.get_default_charge_mode();
    this->charge_mode.get("mode")->updateEnum(this->pm_default_charge_mode);

    auto default_mode = translate_charge_mode(this->pm_default_charge_mode);
    for (size_t i = 0; i < charger_count; ++i) {
        charger_state[i].charge_mode = default_mode;
    }

    bool enabled = config.get("enable_charge_manager")->asBool();

    if (enabled && charger_count > 0 && this->static_cm) {
        ca_config->enable_current_factor = 1;
    }

#if MODULE_AUTOMATION_AVAILABLE()
    automation.set_enabled(AutomationActionID::PMChargeModeSwitch, enabled);
    automation.set_enabled(AutomationTriggerID::ChargeManagerWd, enabled && this->static_cm);
    automation.set_enabled(AutomationActionID::SetManagerCurrent, enabled && this->static_cm);
#endif

    api.addPersistentConfig("charge_manager/config", &config);
    api.addPersistentConfig("charge_manager/low_level_config", &low_level_config);
    api.addState("charge_manager/state", &state);
    api.addState("charge_manager/low_level_state", &low_level_state);

    api.addState("charge_manager/available_current", &available_current);
    api.addCommand("charge_manager/available_current_update", &available_current_update, {}, [this](String &/*errmsg*/) {
        uint32_t current = this->available_current_update.get("current")->asUint();
        this->available_current.get("current")->updateUint(current);
        this->last_available_current_update = now_us();
        this->watchdog_triggered = false;

        for(size_t i = 1; i < 4; ++i) {
            this->limits.raw[i] = current;
            this->limits.min[i] = current;
            this->limits.spread[i] = current;
        }

    }, false);

#ifdef DEBUG_FS_ENABLE
    api.addCommand("charge_manager/debug_limits_update", &debug_limits_update, {}, [this](String &/*errmsg*/) {
        for(size_t i = 0; i < 4; ++i) {
            this->limits.raw[i] = debug_limits_update.get("raw")->get(i)->asInt();
            this->limits.min[i] = debug_limits_update.get("min")->get(i)->asInt();
            this->limits.spread[i] = debug_limits_update.get("spread")->get(i)->asInt();
        }
        this->limits.max_pv = debug_limits_update.get("max_pv")->asInt();
    }, false);
#endif

    // This is power_manager API that is now handled by the charge manager.
    api.addState("power_manager/charge_mode", &charge_mode);
    api.addCommand("power_manager/charge_mode_update", &charge_mode, {}, [this](String &errmsg) {
        // translate_charge_mode supports passing "default", but charge_mode should be updated as well.
        if (charge_mode.get("mode")->asEnum<ConfigChargeMode>() == ConfigChargeMode::Default)
            charge_mode.get("mode")->updateEnum(this->pm_default_charge_mode);

        auto new_mode = translate_charge_mode(this->charge_mode.get("mode")->asEnum<ConfigChargeMode>());

        for (size_t i = 0; i < charger_count; ++i) {
            charger_state[i].charge_mode = new_mode;
        }

        //logger.printfln("Charging mode %u requested but it was ignored", new_mode);
        //errmsg = "Charge mode switch ignored";
    }, false);

    if (static_cm && config.get("enable_watchdog")->asBool()) {
        task_scheduler.scheduleWithFixedDelay([this](){this->check_watchdog();}, 1_s, 1_s);
    }
}

void ChargeManager::update_charger_state_config(uint8_t idx) {
    auto &charger = charger_state[idx];
    auto &charger_alloc = charger_allocation_state[idx];
    auto *charger_cfg = (Config *)this->state.get("chargers")->get(idx);
    auto *ll_charger_cfg = (Config *)this->low_level_state.get("chargers")->get(idx);
    charger_cfg->get("s")->updateUint(charger_alloc.state);
    charger_cfg->get("e")->updateUint(charger_alloc.error);
    charger_cfg->get("ac")->updateUint(charger_alloc.allocated_current);
    charger_cfg->get("ap")->updateUint(charger_alloc.allocated_phases);
    charger_cfg->get("sc")->updateUint(charger.supported_current);
    charger_cfg->get("sp")->updateUint((charger.phase_switch_supported ? 4 : 0) | (charger.phases));
    charger_cfg->get("lu")->updateUint(charger.last_update.to<millis_t>().as<uint32_t>());
    charger_cfg->get("u")->updateUint(charger.uid);

    uint8_t bits = (charger.phases << 3) | (charger.phase_switch_supported << 2) | (charger.cp_disconnect_state << 1) | charger.cp_disconnect_supported;
    ll_charger_cfg->get("b")->updateUint(bits);
    ll_charger_cfg->get("rc")->updateUint(charger.requested_current);
    ll_charger_cfg->get("ae")->updateUint(charger.allocated_energy * 1000);
    ll_charger_cfg->get("ls")->updateUint(charger.last_switch_on.to<millis_t>().as<uint32_t>());
    ll_charger_cfg->get("lp")->updateUint(charger.just_plugged_in_timestamp.to<millis_t>().as<uint32_t>());
    ll_charger_cfg->get("lw")->updateUint(charger.last_wakeup.to<millis_t>().as<uint32_t>());
    ll_charger_cfg->get("ip")->updateUint(charger.use_supported_current.to<millis_t>().as<uint32_t>());
}

uint32_t ChargeManager::get_maximum_available_current()
{
    return config.get("maximum_available_current")->asUint();
}

void ChargeManager::skip_global_hysteresis() {
    if (this->ca_state == nullptr || this->ca_config == nullptr)
        return;

    this->ca_state->last_hysteresis_reset = now_us() - this->ca_config->global_hysteresis - 1_us;
}

const ChargerState *ChargeManager::get_charger_state(uint8_t idx)
{
    return this->get_mutable_charger_state(idx);
}


ChargerState *ChargeManager::get_mutable_charger_state(uint8_t idx)
{
    return this->charger_state == nullptr ? nullptr : &this->charger_state[idx];
}

void ChargeManager::enable_fast_single_charger_mode()
{
    if (!config.get("enable_charge_manager")->asBool()) {
        logger.printfln("Cannot enable fast single charger mode because the charge manager is disabled");
        return;
    }

    if (charger_count != 1) {
        logger.printfln("Cannot enable fast single charger mode because %zu chargers are configured", charger_count);
        return;
    }

    ca_config->allocation_interval = 5_s;
    low_level_config.get("allocation_interval")->updateUint(5);

    logger.printfln("Enabled fast single charger mode");
}
