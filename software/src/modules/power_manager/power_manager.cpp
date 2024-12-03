/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#define TRACE_LOG_PREFIX "charge_manager"

#include <type_traits>

#include "power_manager.h"
#include "module_dependencies.h"

#include "event_log_prefix.h"
#include "tools.h"

#include "gcc_warnings.h"

#define ENABLE_PM_TRACE 1

#define METER_SLOT_BATTERY_NO_BATTERY (255)

void PowerManager::pre_setup()
{
    // States
    state = Config::Object({
        {"config_error_flags", Config::Uint32(0)},
        {"external_control", Config::Uint32(EXTERNAL_CONTROL_STATE_DISABLED)},
    });

    const Config *config_prototype_int32_0 = Config::get_prototype_int32_0();

    low_level_state = Config::Object({
        {"power_at_meter", Config::Float(0)},
        {"power_at_battery", Config::Float(0)},
        {"power_available", Config::Int32(0)},
        {"i_meter", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
            },
            config_prototype_int32_0,
            3, 3, Config::type_id<Config::ConfInt>())
        },
        {"i_pp_max", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
            },
            config_prototype_int32_0,
            3, 3, Config::type_id<Config::ConfInt>())
        },
        {"i_pp_mavg", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
            },
            config_prototype_int32_0,
            3, 3, Config::type_id<Config::ConfInt>())
        },
        {"i_pp", Config::Array({
                Config::Int32(0),
                Config::Int32(0),
                Config::Int32(0),
            },
            config_prototype_int32_0,
            3, 3, Config::type_id<Config::ConfInt>())
        },
        {"overall_min_power", Config::Int32(0)},
        {"max_current_limited", Config::Int32(0)},
        {"is_3phase", Config::Bool(false)}, // obsolete / phase switcher?
        {"charging_blocked", Config::Uint32(0)},
    });

    config = ConfigRoot{Config::Object({
        {"enabled", Config::Bool(false)},
        {"phase_switching_mode", Config::Uint(PHASE_SWITCHING_AUTOMATIC, PHASE_SWITCHING_MIN, PHASE_SWITCHING_MAX)},
        {"excess_charging_enable", Config::Bool(false)},
        {"default_mode", Config::Uint(0, 0, 3)},
        {"meter_slot_grid_power", Config::Uint(POWER_MANAGER_DEFAULT_METER_SLOT, 0, METERS_SLOTS - 1)},
        {"meter_slot_battery_power", Config::Uint(METER_SLOT_BATTERY_NO_BATTERY, 0, METER_SLOT_BATTERY_NO_BATTERY)},
        {"battery_mode", Config::Uint(0, 0, static_cast<uint32_t>(BatteryMode::BatteryModeMax))},
        {"battery_inverted", Config::Bool(false)},
        {"battery_deadzone", Config::Uint(100, 0, 9999)}, // in watt
        {"target_power_from_grid", Config::Int32(0)}, // in watt
        {"guaranteed_power", Config::Uint(1380, 0, 22080)}, // in watt
        {"cloud_filter_mode", Config::Uint(CLOUD_FILTER_MEDIUM, CLOUD_FILTER_OFF, CLOUD_FILTER_STRONG)},
    }), [](const Config &cfg, ConfigSource source) -> String {
        const bool excess_charging_enable = cfg.get("excess_charging_enable")->asBool();

        if (cfg.get("phase_switching_mode")->asUint() == 3) { // external control
            if (excess_charging_enable) {
                return "Can't enable excess charging when external control is enabled for phase switching.";
            }
            if (cfg.get("default_mode")->asUint() != MODE_FAST) {
                return "Can't select any charging mode besides 'Fast' when external control is enabled for phase switching.";
            }
        }

        if (excess_charging_enable) {
            const uint32_t slot_grid    = cfg.get("meter_slot_grid_power"   )->asUint();
            const uint32_t slot_battery = cfg.get("meter_slot_battery_power")->asUint();

            if (slot_grid == slot_battery) {
                return "Grid and battery storage cannot use the same power meter";
            }
        }

        return "";
    }};

    dynamic_load_config = ConfigRoot{Config::Object({
        {"enabled",                  Config::Bool(false)},
        {"meter_slot_grid_currents", Config::Uint(POWER_MANAGER_DEFAULT_METER_SLOT, 0, METERS_SLOTS - 1)},
        {"current_limit",            Config::Uint(    0, 0, 524287)}, // mA, maximum is 524 A
        {"largest_consumer_current", Config::Uint(32000, 0, 524287)}, // mA, maximum is 524 A
        {"safety_margin_pct",        Config::Uint(    0, 0,     50)}, // percent
    }), [](const Config &cfg, ConfigSource source) -> String {
        if (cfg.get("enabled")->asBool()) {
            uint32_t current_limit_ma            = cfg.get("current_limit")->asUint();
            uint32_t largest_consumer_current_ma = cfg.get("largest_consumer_current")->asUint();

            // Check here so that the config can be saved with an invalid current_limit when dynamic load management is disabled.
            if (current_limit_ma < 16000) {
                return "Invalid current limit. Must be at least 16A.";
            }
            if (largest_consumer_current_ma > current_limit_ma) {
                return "Largest consumer current cannot be above current limit.";
            }
        }

        return "";
    }};

    // Runtime config
    charge_mode = Config::Object({
        {"mode", Config::Uint(0, 0, 3)},
    });
    charge_mode_update = charge_mode;

    external_control = Config::Object({
        {"phases_wanted", Config::Uint(0, 0, 3)},
    });
    external_control_update = ConfigRoot{
        external_control,
        [](Config &conf, ConfigSource source) -> String {
            auto phases = conf.get("phases_wanted")->asUint();
            if (phases != 1 && phases != 3)
                return "Invalid value for phases_wanted! Only 1 or 3 are allowed.";
            return "";
        }
    };


#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_action(
        AutomationActionID::PMPhaseSwitch,
        Config::Object({
            {"phases_wanted", Config::Uint(1)}
        }),
        [this](const Config *cfg) {
            const String err = api.callCommand("power_manager/external_control_update", Config::ConfUpdateObject{{
                {"phases_wanted", cfg->get("phases_wanted")->asUint()}
            }});
            if (!err.isEmpty()) {
                logger.printfln("Automation couldn't set external_control_update: %s", err.c_str());
            }
        }, nullptr, false);

    automation.register_action(
        AutomationActionID::PMChargeModeSwitch,
        Config::Object({
            {"mode", Config::Uint(0, 0, 4)}
        }),
        [this](const Config *cfg) {
            uint32_t configured_mode = cfg->get("mode")->asUint();

            // Automation rule configured to switch to default mode
            if (configured_mode == 4) {
                configured_mode = this->default_mode;
            }

            const String err = api.callCommand("power_manager/charge_mode_update", Config::ConfUpdateObject{{
                {"mode", configured_mode}
            }});
            if (!err.isEmpty()) {
                logger.printfln("Automation couldn't switch charge mode: %s", err.c_str());
            }
        },
        nullptr,
        false);

    automation.register_action(
        AutomationActionID::PMLimitMaxCurrent,
        Config::Object({
            {"current", Config::Int(0, -1)}
        }),
        [this](const Config *cfg) {
            int32_t current = cfg->get("current")->asInt();
            if (current == -1) {
                this->reset_max_current_limit();
            } else {
                this->set_max_current_limit(current);
            }
        },
        nullptr,
        false);

#if MODULE_EM_V1_AVAILABLE()
    automation.register_action(
        AutomationActionID::PMBlockCharge,
        Config::Object({
            {"slot", Config::Uint(0, 0, 3)},
            {"block", Config::Bool(false)}
        }),
        [this](const Config *cfg) {
            this->charging_blocked.pin[cfg->get("slot")->asUint()] = static_cast<uint8_t>(cfg->get("block")->asBool());
        });
#endif

    automation.register_trigger(
        AutomationTriggerID::PMPowerAvailable,
        Config::Object({
            {"power_available", Config::Bool(false)}
        }),
        nullptr,
        false
    );

    automation.register_trigger(
        AutomationTriggerID::PMGridPowerDraw,
        Config::Object({
            {"drawing_power", Config::Bool(false)}
        }),
        nullptr,
        false
    );
#endif
}

static void init_minmax_filter(PowerManager::minmax_filter *filter, size_t values_count, PowerManager::FilterType filter_type)
{
    if (values_count <= 0) {
        logger.printfln("Cannot create minmax filter with %zu values.", values_count);
        values_count = 1;
    }

    filter->history_length = static_cast<decltype(filter->history_length)>(values_count);
    filter->history_values = static_cast<decltype(filter->history_values)>(heap_caps_malloc_prefer(values_count * sizeof(filter->history_values[0]), 2, MALLOC_CAP_32BIT, MALLOC_CAP_SPIRAM)); // Prefer IRAM
    filter->type = filter_type;
}

static void init_mavg_filter(PowerManager::mavg_filter *filter, size_t values_count)
{
    if (values_count <= 0) {
        logger.printfln("Cannot create mavg filter with %zu values.", values_count);
        values_count = 1;
    }

    filter->mavg_values_count = static_cast<decltype(filter->mavg_values_count)>(values_count);
    filter->mavg_values       = static_cast<decltype(filter->mavg_values)>(heap_caps_malloc_prefer(values_count * sizeof(filter->mavg_values[0]), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_32BIT)); // Prefer SPIRAM
}

void PowerManager::setup()
{
    initialized = true;

    api.restorePersistentConfig("power_manager/config", &config);
    api.restorePersistentConfig("power_manager/dynamic_load_config", &dynamic_load_config);

    // Force-enable PM if either PV excess charging or dynamic load management are enabled.
    // Force-disable otherwise.
    if (config.get("excess_charging_enable")->asBool() || dynamic_load_config.get("enabled")->asBool()) {
        config.get("enabled")->updateBool(true);
    } else {
        config.get("enabled")->updateBool(false);
        return;
    }

#if MODULE_AUTOMATION_AVAILABLE()
    automation.set_enabled(AutomationActionID::PMChargeModeSwitch, true);
    automation.set_enabled(AutomationActionID::PMLimitMaxCurrent, true);
    automation.set_enabled(AutomationTriggerID::PMPowerAvailable, true);
    automation.set_enabled(AutomationTriggerID::PMGridPowerDraw, true);
#endif

    debug_protocol.register_backend(this);

    cm_limits             = charge_manager.get_limits();
    cm_allocated_currents = charge_manager.get_allocated_currents();

    // If the PM is enabled, make sure to override the CM's default current.
    zero_limits();

    // Cache config for energy update
    default_mode                = config.get("default_mode")->asUint();
    excess_charging_enabled     = config.get("excess_charging_enable")->asBool();
    meter_slot_power            = config.get("meter_slot_grid_power")->asUint();
    target_power_from_grid_w    = config.get("target_power_from_grid")->asInt();    // watt
    meter_slot_battery_power    = config.get("meter_slot_battery_power")->asUint();
    battery_mode                = config.get("battery_mode")->asEnum<BatteryMode>();
    battery_inverted            = config.get("battery_inverted")->asBool();
    battery_deadzone_w          = static_cast<uint16_t>(config.get("battery_deadzone")->asUint()); // watt
    guaranteed_power_w          = static_cast<int32_t>(config.get("guaranteed_power")->asUint()); // watt
    phase_switching_mode        = config.get("phase_switching_mode")->asUint();
    dynamic_load_enabled        = dynamic_load_config.get("enabled")->asBool();
    meter_slot_currents         = dynamic_load_config.get("meter_slot_grid_currents")->asUint();
    supply_cable_max_current_ma = static_cast<int32_t>(charge_manager.config.get("maximum_available_current")->asUint()); // milliampere
    min_current_1p_ma           = static_cast<int32_t>(charge_manager.config.get("minimum_current_1p")->asUint());        // milliampere
    min_current_3p_ma           = static_cast<int32_t>(charge_manager.config.get("minimum_current")->asUint());           // milliampere

    mode = default_mode;
    charge_mode.get("mode")->updateUint(mode);

    if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL) {
        state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_UNAVAILABLE);
        api.addFeature("phase_switch");
#if MODULE_AUTOMATION_AVAILABLE()
        automation.set_enabled(AutomationActionID::PMPhaseSwitch, true);
#endif
    }

    // Prepare value count for mavg filters
    size_t power_mavg_span_s;
    switch (config.get("cloud_filter_mode")->asUint()) {
        default:
        case CLOUD_FILTER_OFF:    power_mavg_span_s =   0; break;
        case CLOUD_FILTER_LIGHT:  power_mavg_span_s = 120; break;
        case CLOUD_FILTER_MEDIUM: power_mavg_span_s = 240; break;
        case CLOUD_FILTER_STRONG: power_mavg_span_s = 480; break;
    }

    size_t values_count;
    if (power_mavg_span_s <= 0) {
        values_count = 1;
    } else {
        values_count = power_mavg_span_s * 1000 / PM_TASK_DELAY_MS;
    }

    const size_t values_count_long_min = 60; // 1 hour @ 1 sample per minute

    // Set up meter power filters if excess charging is enabled
    if (excess_charging_enabled) {
        init_minmax_filter(&current_pv_minmax_ma,   values_count,          FilterType::MinMax);
        init_minmax_filter(&current_pv_long_min_ma, values_count_long_min, FilterType::MinOnly);
    }

    // Pre-calculate various limits
    overall_min_power_w = 230 * 1 * min_current_1p_ma / 1000;

    if (guaranteed_power_w < overall_min_power_w) {
        guaranteed_power_w = overall_min_power_w;
        logger.printfln("Raising guaranteed power to %i based on minimum charge current set in charge manager.", guaranteed_power_w);
    }

    // Calculate constants and set up meter current filters if dynamic load management is enabled
    if (dynamic_load_enabled) {
        int32_t current_limit_ma            = static_cast<int32_t>(dynamic_load_config.get("current_limit")->asUint());
        int32_t largest_consumer_current_ma = static_cast<int32_t>(dynamic_load_config.get("largest_consumer_current")->asUint());
        int32_t safety_margin_pct           = static_cast<int32_t>(dynamic_load_config.get("safety_margin_pct")->asUint());

        int32_t circuit_breaker_trip_point_ma = current_limit_ma * 7 / 5; // limit + 40%
        int32_t max_possible_ma = circuit_breaker_trip_point_ma - largest_consumer_current_ma;
        target_phase_current_ma = std::min(max_possible_ma, current_limit_ma) * (100 - safety_margin_pct) / 100;

        phase_current_max_increase_ma = target_phase_current_ma / 2; // Cap maximum ramp-up for safety. Might not be necessary.

        constexpr size_t min_filter_length = 4 * 60 * 1000 / PM_TASK_DELAY_MS; // 4min
        constexpr size_t preproc_filter_length = 10 * 1000 / PM_TASK_DELAY_MS; // 10s

        for (size_t i = 0; i < ARRAY_SIZE(currents_phase_min_ma); i++) {
            init_minmax_filter(currents_phase_min_ma + i,      min_filter_length,     FilterType::MinOnly);
            init_minmax_filter(currents_phase_long_min_ma + i, values_count_long_min, FilterType::MinOnly);

            init_minmax_filter(currents_phase_preproc_max_ma + i, preproc_filter_length, FilterType::MaxOnly);
            init_mavg_filter(currents_phase_preproc_mavg_ma + i,  preproc_filter_length);
        }

        currents_phase_preproc_mavg_limit        = target_phase_current_ma * 11 / 10; // target + 10%
        currents_phase_preproc_interpolate_limit = target_phase_current_ma *  6 /  5; // target + 20%
        currents_phase_preproc_interpolate_interval_quantized = (currents_phase_preproc_interpolate_limit - currents_phase_preproc_mavg_limit) / currents_phase_preproc_interpolate_quantization_factor;

        //logger.printfln("cb trip %i  max %i  target phase current %i  max inc %i  pp mavg limit %i  pp int limit %i", circuit_breaker_trip_point_ma, max_possible_ma, target_phase_current_ma, phase_current_max_increase_ma, currents_phase_preproc_mavg_limit, currents_phase_preproc_interpolate_limit);
    }

    low_level_state.get("overall_min_power")->updateInt(overall_min_power_w);

    // Update data from meter and phase switcher back-end, requires power filter to be set up.
    update_data();

    // Check for incomplete configuration after as much as possible has been set up.

    bool power_meter_available = false;
    bool current_meter_available = false;
#if MODULE_METERS_AVAILABLE()
    float unused_power;
    if (meters.get_power(meter_slot_power, &unused_power) == MeterValueAvailability::Unavailable) {
        meter_slot_power = UINT32_MAX;
    } else {
        power_meter_available = true;
    }

    if (meter_slot_battery_power < METER_SLOT_BATTERY_NO_BATTERY) {
        if (meters.get_power(meter_slot_battery_power, &unused_power) == MeterValueAvailability::Unavailable) {
            meter_slot_battery_power = std::numeric_limits<decltype(meter_slot_battery_power)>::max();
            logger.printfln("Battery storage configured but meter can't provide power values.");
        } else {
            have_battery = true;
        }
    }

    float unused_meter_currents[INDEX_CACHE_CURRENT_COUNT];
    if (meters.get_currents(meter_slot_currents, unused_meter_currents) == MeterValueAvailability::Unavailable) {
        meter_slot_currents = std::numeric_limits<decltype(meter_slot_currents)>::max();
    } else {
        current_meter_available = true;
    }

#endif
    if (excess_charging_enabled && !power_meter_available) {
        set_config_error(PM_CONFIG_ERROR_FLAGS_EXCESS_NO_METER_MASK);
        logger.printfln("Excess charging enabled but configured meter can't provide power values.");
    }

    if (dynamic_load_enabled && !current_meter_available) {
        set_config_error(PM_CONFIG_ERROR_FLAGS_DLM_NO_METER_MASK);
        logger.printfln("Dynamic load management enabled but configured meter can't provide current values.");
    }

    task_scheduler.scheduleOnce([this]() {
        // Can't check for chargers in setup() because CM's setup() hasn't run yet to load the charger configuration.
        if (api.getState("charge_manager/config")->get("enable_charge_manager")->asBool()) {
            if (charge_manager.get_charger_count() <= 0) {
                logger.printfln("No chargers configured. Won't try to distribute energy.");
                set_config_error(PM_CONFIG_ERROR_FLAGS_NO_CHARGERS_MASK);
            }
        } else {
            logger.printfln("Charge manager not enabled. Won't try to distribute energy.");
        }
    });

    if (supply_cable_max_current_ma == 0) {
        logger.printfln("No maximum current configured for chargers. Disabling energy distribution.");
        set_config_error(PM_CONFIG_ERROR_FLAGS_NO_MAX_CURRENT_MASK);
        return;
    }

    // If external control is enabled, assume that the last requested amount of phases
    // is whatever the contactor is switched to at the moment, in order to preserve
    // that across a reboot. This will still fail on a power cycle or bricklet update,
    // which set the contactor back to single phase.
    if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL) {
        external_control.get("phases_wanted")->updateUint(current_phases);
    }

    // supply_cable_max_current_ma must be set before reset
    reset_max_current_limit();

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->update_data();
        this->update_energy();
        this->update_phase_switcher();
    }, millis_t{PM_TASK_DELAY_MS}, millis_t{PM_TASK_DELAY_MS});
}

void PowerManager::register_urls()
{
    api.addState("power_manager/state", &state);

    api.addPersistentConfig("power_manager/config", &config);
    api.addPersistentConfig("power_manager/dynamic_load_config", &dynamic_load_config);

    api.addState("power_manager/low_level_state", &low_level_state);

    api.addState("power_manager/charge_mode", &charge_mode);
    api.addCommand("power_manager/charge_mode_update", &charge_mode_update, {}, [this](String &/*errmsg*/) {
        uint32_t new_mode = this->charge_mode_update.get("mode")->asUint();

        if (new_mode == MODE_DO_NOTHING)
            return;

        auto runtime_mode = this->charge_mode.get("mode");
        uint32_t old_mode = runtime_mode->asUint();
        just_switched_mode |= runtime_mode->updateUint(new_mode); // If this callback runs again before just_switched_mode was consumed, keep it true instead of overwriting it to false;
        mode = new_mode;

        logger.printfln("Switched mode %u->%u", old_mode, mode);
    }, false);

    api.addState("power_manager/external_control", &external_control);

    if (!config.get("enabled")->asBool() && phase_switcher_backend->is_external_control_allowed() && phase_switcher_backend->phase_switching_capable() && !phase_switcher_backend->requires_cp_disconnect()) {
        logger.printfln("Disabled but phase switching backend can switch autonomously. Enabling external control API.");

        api.addFeature("phase_switch");
#if MODULE_AUTOMATION_AVAILABLE()
        automation.set_enabled(AutomationActionID::PMPhaseSwitch, true);
#endif

        api.addCommand("power_manager/external_control_update", &external_control_update, {}, [this](String &/*errmsg*/) {
            if (!phase_switcher_backend->is_external_control_allowed()) {
                logger.printfln("Ignoring external control phase change request: External control currently not allowed.");
                return;
            }

            switch (phase_switcher_backend->get_phase_switching_state()) {
                case PhaseSwitcherBackend::SwitchingState::Error:
                    logger.printfln("Ignoring external control phase change request: Phase switching in error state.");
                    return;
                case PhaseSwitcherBackend::SwitchingState::Busy:
                    logger.printfln("Ignoring external control phase change request: Phase switching in progress.");
                    return;
                case PhaseSwitcherBackend::SwitchingState::Ready:
                    // All good, proceed.
                    break;
                default:
                    esp_system_abort("Unexpected value of phase_switcher_backend->get_phase_switching_state()");
            }

            uint32_t phases_wanted = external_control_update.get("phases_wanted")->asUint();
            if (phase_switcher_backend->switch_phases(phases_wanted))
                state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_SWITCHING);
        }, true);

        task_scheduler.scheduleWithFixedDelay([this](){
            uint32_t ext_state = EXTERNAL_CONTROL_STATE_AVAILABLE;
            switch(phase_switcher_backend->get_phase_switching_state()) {
                case PhaseSwitcherBackend::SwitchingState::Error:
                    ext_state = EXTERNAL_CONTROL_STATE_UNAVAILABLE;
                    break;
                case PhaseSwitcherBackend::SwitchingState::Busy:
                    ext_state = EXTERNAL_CONTROL_STATE_SWITCHING;
                    break;
                case PhaseSwitcherBackend::SwitchingState::Ready:
                    ext_state = EXTERNAL_CONTROL_STATE_AVAILABLE;
                    break;
                default:
                    esp_system_abort("Unexpected value of phase_switcher_backend->get_phase_switching_state()");
            }

            state.get("external_control")->updateUint(ext_state);
            this->current_phases = phase_switcher_backend->get_phases();
            low_level_state.get("is_3phase")->updateBool(this->current_phases == 3);
        }, 1_s, 1_s);
    } else {
        api.addCommand("power_manager/external_control_update", &external_control_update, {}, [this](String &/*errmsg*/) {
            uint32_t external_control_state = state.get("external_control")->asUint();
            switch (external_control_state) {
                case EXTERNAL_CONTROL_STATE_DISABLED:
                    logger.printfln("Ignoring external control phase change request: External control is not enabled.");
                    return;
                case EXTERNAL_CONTROL_STATE_UNAVAILABLE:
                    logger.printfln("Ignoring external control phase change request: Phase switching is currently unavailable.");
                    return;
                case EXTERNAL_CONTROL_STATE_SWITCHING:
                    logger.printfln("Ignoring external control phase change request: Phase switching in progress.");
                    return;
                default:
                    break; // All good, proceed.
            }

            auto phases_wanted = external_control.get("phases_wanted");
            uint32_t old_phases = phases_wanted->asUint();
            uint32_t new_phases = external_control_update.get("phases_wanted")->asUint();

            if (!phases_wanted->updateUint(new_phases)) {
                logger.printfln("Ignoring external control phase change request: Value is already %u.", new_phases);
                return;
            }

            logger.printfln("External control phase change request: switching from %u to %u", old_phases, new_phases);
        }, true);
    }
}

void PowerManager::register_phase_switcher_backend(PhaseSwitcherBackend *backend)
{
    if (backend->get_phase_switcher_priority() > phase_switcher_backend->get_phase_switcher_priority()) {
        phase_switcher_backend = backend;
    }
}

void PowerManager::zero_limits()
{
    cm_limits->raw.pv = 0;
    cm_limits->raw.l1 = 0;
    cm_limits->raw.l2 = 0;
    cm_limits->raw.l3 = 0;
}

[[gnu::noinline]] // Don't put msg buffer on calling function's stack.
static void abort_on_invalid_history_length(int32_t history_length)
{
    char msg[52]; // Message buffer must be on the stack to be included in a coredump.
    snprintf(msg, ARRAY_SIZE(msg), "Invalid minmax filter history length %i", history_length);
    esp_system_abort(msg);
}

static void update_minmax_filter(int32_t new_value, PowerManager::minmax_filter *filter)
{
    // Check if filter history needs to be initialized
    if (filter->min == INT32_MAX) {
        filter->min = new_value;
        filter->max = filter->type == PowerManager::FilterType::MinOnly ? -1 : new_value; // Unused max uses -1 to avoid underflows from INT32_MIN

        if (filter->history_length <= 0) {
            abort_on_invalid_history_length(filter->history_length);
        } else if (filter->history_length == 1) {
            filter->history_pos = 0; // History contains only a single value, next value will overwrite.
        } else {
            filter->history_pos = 1; // Position for next value
        }
        filter->history_min_pos = 0;
        filter->history_max_pos = 0;

        filter->history_values[0] = new_value;
        // Other values don't need to be initialized because they won't be read before the first value is removed,
        // at which point all values must have been written.

        return;
    }

    int32_t history_pos = filter->history_pos;

    filter->history_values[history_pos] = new_value;

    if (filter->type != PowerManager::FilterType::MaxOnly) {
        if (new_value <= filter->min) {
            filter->min = new_value;
            filter->history_min_pos = history_pos;
        } else {
            if (filter->history_min_pos == history_pos) {
                // Current minimum was just replaced, need to find new minimum.
                int32_t *values = filter->history_values;
                int32_t min = INT32_MAX;
                int32_t min_pos = 0;

                // Search older data after current position in history
                for (int32_t i = history_pos + 1; i < filter->history_length; i++) {
                    if (values[i] <= min) {
                        min = values[i];
                        min_pos = i;
                    }
                }
                // Search newer data before current position in history
                for (int32_t i = 0; i <= history_pos; i++) {
                    if (values[i] <= min) {
                        min = values[i];
                        min_pos = i;
                    }
                }

                filter->min = min;
                filter->history_min_pos = min_pos;
            }
        }
    }

    if (filter->type != PowerManager::FilterType::MinOnly) {
        if (new_value >= filter->max) {
            filter->max = new_value;
            filter->history_max_pos = history_pos;
        } else {
            if (filter->history_max_pos == history_pos) {
                // Current maximum was just replaced, need to find new maximum.
                int32_t *values = filter->history_values;
                int32_t max = INT32_MIN;
                int32_t max_pos = 0;

                // Search older data after current position in history
                for (int32_t i = history_pos + 1; i < filter->history_length; i++) {
                    if (values[i] >= max) {
                        max = values[i];
                        max_pos = i;
                    }
                }
                // Search newer data before current position in history
                for (int32_t i = 0; i <= history_pos; i++) {
                    if (values[i] >= max) {
                        max = values[i];
                        max_pos = i;
                    }
                }

                filter->max = max;
                filter->history_max_pos = max_pos;
            }
        }
    }

    history_pos++;
    if (history_pos >= filter->history_length) {
        history_pos = 0;
    }
    filter->history_pos = history_pos;
}

static int32_t update_mavg_filter(int32_t filter_input, PowerManager::mavg_filter *filter)
{
    // Check if filter values need to be initialized
    if (filter->filtered_val == INT32_MAX) {
        int32_t values_count = filter->mavg_values_count;

        filter->filtered_val = filter_input;
        filter->mavg_total = filter_input * values_count;

        int32_t *values = filter->mavg_values;
        int32_t *values_end = values + values_count;
        while (values < values_end) {
            *values = filter_input;
            values++;
        }

        return filter_input;
    }

    int32_t mavg_position = filter->mavg_position;

    int32_t mavg_total = filter->mavg_total - filter->mavg_values[mavg_position] + filter_input;
    filter->mavg_values[mavg_position] = filter_input;
    filter->mavg_total = mavg_total;

    mavg_position++;
    if (mavg_position >= filter->mavg_values_count) {
        mavg_position = 0;
    }
    filter->mavg_position = mavg_position;

    // Signed division requires both numbers to be signed
    static_assert(std::is_same<int32_t, decltype(filter->mavg_total       )>::value, "filter's mavg_total must be signed");
    static_assert(std::is_same<int32_t, decltype(filter->mavg_values_count)>::value, "filter's mavg_values_count must be signed");

    filter->filtered_val = mavg_total / filter->mavg_values_count;

    return filter->filtered_val;
}

void PowerManager::update_data()
{
    // TODO remove have_phases and is_3phase
    // Update states from back-end
    current_phases = phase_switcher_backend->get_phases();
    low_level_state.get("is_3phase")->updateBool(current_phases == 3);

#if MODULE_METERS_AVAILABLE()
    if (meters.get_power(meter_slot_power, &power_at_meter_raw_w) != MeterValueAvailability::Fresh) {
        power_at_meter_raw_w = NAN;
    }
    if (have_battery) {
        if (meters.get_power(meter_slot_battery_power, &power_at_battery_raw_w) != MeterValueAvailability::Fresh) {
            power_at_battery_raw_w = NAN;
        }
    }
#endif

    if (!isnan(power_at_meter_raw_w)) {
        low_level_state.get("power_at_meter")->updateFloat(power_at_meter_raw_w);

#if MODULE_AUTOMATION_AVAILABLE()
        TristateBool drawing_power = static_cast<TristateBool>(power_at_meter_raw_w > 0);
        if (drawing_power != automation_drawing_power_last && boot_stage > BootStage::SETUP) {
            automation.trigger(AutomationTriggerID::PMGridPowerDraw, nullptr, this);
            automation_drawing_power_last = drawing_power;
        }
#endif
    }

    if (!isnan(power_at_battery_raw_w)) {
        if (battery_inverted) {
            power_at_battery_raw_w *= -1;
        }
        low_level_state.get("power_at_battery")->updateFloat(power_at_battery_raw_w);
    }

    float meter_currents[INDEX_CACHE_CURRENT_COUNT];
#if MODULE_METERS_AVAILABLE()
    MeterValueAvailability ret = meters.get_currents(meter_slot_currents, meter_currents);
    if (ret != MeterValueAvailability::Fresh)
#else
    // Unconditionally execute block
#endif
    { // Don't place bracket in sections removed by precompiler to make VSCode happy
        for (size_t i = 0; i < INDEX_CACHE_CURRENT_COUNT; i++) {
            meter_currents[i] = NAN;
        }
    }

    for (size_t i = 0; i < INDEX_CACHE_CURRENT_COUNT; i++) {
        float meter_current_a = meter_currents[i];
        if (!isnan(meter_current_a)) {
            // TODO Store in low_level_state

            if (dynamic_load_enabled) {
                int32_t raw_current_ma = static_cast<int32_t>(meter_current_a * 1000);
                currents_at_meter_raw_ma[i] = raw_current_ma;
            }
        }
    }
}

void PowerManager::update_energy()
{
#if ENABLE_PM_TRACE
    char trace_log[256];
    size_t trace_log_len = snprintf_u(trace_log, sizeof(trace_log), "PM");
#endif

    if (!excess_charging_enabled) {
        power_available_w = INT32_MAX;
    } else {
        int32_t p_error_w;

        if (isnan(power_at_meter_raw_w) || (have_battery && isnan(power_at_battery_raw_w))) {
            if (!printed_skipping_energy_update) {
                logger.printfln("PV excess charging unavailable because power values are not available yet.");
                printed_skipping_energy_update = true;
            }

            p_error_w = INT32_MAX;
        } else {
            if (printed_skipping_energy_update) {
                logger.printfln("PV excess charging available because power values are now available.");
                printed_skipping_energy_update = false;
            }

            // Grid: +import -export
            const int32_t power_grid_raw_w = static_cast<int32_t>(power_at_meter_raw_w);
            int32_t power_combined_raw_w;

            if (have_battery) {
                // Battery: +charging -discharging
                int32_t power_battery_w = static_cast<int32_t>(power_at_battery_raw_w);

                // Only take battery charging power into account if chargers are preferred,
                // by simply ignoring the battery's power while it's charging.
                if (power_battery_w > 0 && battery_mode != BatteryMode::PreferChargers) {
                    power_battery_w = 0;
                }

                int32_t power_grid_adjusted_w = power_grid_raw_w;

                if (power_grid_adjusted_w > 0) {
                    if (power_grid_adjusted_w > battery_deadzone_w) {
                        power_grid_adjusted_w -= battery_deadzone_w;
                    } else {
                        power_grid_adjusted_w = 0;
                    }
                } else if (power_grid_adjusted_w < 0) {
                    if (power_grid_adjusted_w < -battery_deadzone_w) {
                        power_grid_adjusted_w += battery_deadzone_w;
                    } else {
                        power_grid_adjusted_w = 0;
                    }
                } // Nothing to do if power_grid_adjusted_w == 0.

                // Combined: +deficit from +grid import and -discharging
                //           -surplus from -grid export and +charging
                // Therefore, battery power must be subtracted.
                power_combined_raw_w = power_grid_adjusted_w - power_battery_w;
            } else {
                // No battery, use grid power as-is.
                power_combined_raw_w = power_grid_raw_w;
            }

            p_error_w = target_power_from_grid_w - power_combined_raw_w;

#if MODULE_EM_V1_AVAILABLE()
            if (p_error_w > 200) {
                em_v1.update_grid_balance_led(EmRgbLed::GridBalance::Export);
            } else if (p_error_w < -200) {
                em_v1.update_grid_balance_led(EmRgbLed::GridBalance::Import);
            } else {
                em_v1.update_grid_balance_led(EmRgbLed::GridBalance::Balanced);
            }
#endif
        }

        switch (mode) {
            case MODE_FAST:
                power_available_w = INT32_MAX;
                break;
            case MODE_OFF:
            default:
                power_available_w = 0;
                break;
            case MODE_PV:
            case MODE_MIN_PV:
                if (p_error_w == INT32_MAX) {
                    power_available_w = 0;
                } else {
                    // Excess charging uses an adaptive P controller to adjust available power.
                    int32_t p_adjust_w;
                    int32_t cm_allocated_power_w = cm_allocated_currents->pv * 230 / 1000; // ma -> watt

                    if (cm_allocated_power_w <= 0) {
                        // When no power was allocated to any charger, use p=1 so that the threshold for switching on can be reached properly.
                        p_adjust_w = p_error_w;

                        // Sanity check
                        if (cm_allocated_power_w < 0) {
                            logger.printfln("Negative cm_allocated_power_w: %i  cm_allocated_currents(%i %i %i %i)", cm_allocated_power_w, cm_allocated_currents->pv, cm_allocated_currents->l1, cm_allocated_currents->l2, cm_allocated_currents->l3);
                            cm_allocated_power_w = 0;
                        }
                    } else {
                        // Some EVs may only be able to adjust their charge power in steps of 1500W,
                        // so smaller factors are required for smaller errors.
                        int32_t p_error_abs_w = abs(p_error_w);
                        if (p_error_abs_w < 1000) {
                            // Use p=0.5 for small differences so that the controller can converge without oscillating too much.
                            p_adjust_w = p_error_w / 2;
                        } else if (p_error_abs_w < 1500) {
                            // Use p=0.75 for medium differences so that the controller can converge reasonably fast while still avoiding too many oscillations.
                            p_adjust_w = p_error_w * 3 / 4;
                        } else {
                            // Use p=0.875 for large differences so that the controller can converge faster.
                            p_adjust_w = p_error_w * 7 / 8;
                        }
                    }

                    power_available_w = cm_allocated_power_w + p_adjust_w;

                    if (mode != MODE_MIN_PV)
                        break;

                    // Check against guaranteed power only in MIN_PV mode.
                    if (power_available_w < guaranteed_power_w)
                        power_available_w = guaranteed_power_w;

                    break;
                }
        }
    }

    low_level_state.get("power_available")->updateInt(power_available_w);

    if (power_available_w == INT32_MAX) {
        cm_limits->raw.pv = INT32_MAX;
        cm_limits->min.pv = INT32_MAX;
        cm_limits->spread.pv = INT32_MAX;
        cm_limits->max_pv = INT32_MAX;
    } else {
        int32_t pv_raw_ma = power_available_w * 1000 / 230;
        update_minmax_filter(pv_raw_ma, &current_pv_minmax_ma);

        if (current_long_min_iterations == 0) {
            update_minmax_filter(current_pv_floating_min_ma, &current_pv_long_min_ma);

            current_pv_floating_min_ma = pv_raw_ma;
        } else {
            if (pv_raw_ma < current_pv_floating_min_ma) {
                current_pv_floating_min_ma = pv_raw_ma;
            }
        }

        int32_t pv_long_min_ma = std::min(current_pv_floating_min_ma, current_pv_long_min_ma.min);

        cm_limits->raw.pv = pv_raw_ma;
        cm_limits->min.pv = current_pv_minmax_ma.min;
        cm_limits->max_pv = current_pv_minmax_ma.max;
        cm_limits->spread.pv = pv_long_min_ma;

#if ENABLE_PM_TRACE
        trace_log_len += snprintf_u(trace_log + trace_log_len, sizeof(trace_log) - trace_log_len, " PV m=%5iw avl=%5iw %5i<<%5i<%5i<%5i",
            static_cast<int32_t>(power_at_meter_raw_w), power_available_w,
            pv_long_min_ma, current_pv_minmax_ma.min, pv_raw_ma, current_pv_minmax_ma.max);
#endif
    }

#if MODULE_AUTOMATION_AVAILABLE()
    bool power_above_min = current_pv_minmax_ma.min >= overall_min_power_w;
    TristateBool automation_power_available = static_cast<TristateBool>(power_above_min);
    if (automation_power_available != automation_power_available_last) {
        automation.trigger(AutomationTriggerID::PMPowerAvailable, &power_above_min, this);
        automation_power_available_last = automation_power_available;
    }
#endif

    // Cache low-level state configs
    Config *state_i_meter   = static_cast<Config *>(low_level_state.get("i_meter"));
    Config *state_i_pp_max  = static_cast<Config *>(low_level_state.get("i_pp_max"));
    Config *state_i_pp_mavg = static_cast<Config *>(low_level_state.get("i_pp_mavg"));
    Config *state_i_pp      = static_cast<Config *>(low_level_state.get("i_pp"));

    for (size_t cm_phase = 1; cm_phase <= 3; cm_phase++) {
        if (!dynamic_load_enabled) {
            cm_limits->raw[cm_phase]    = max_current_limited_ma;
            cm_limits->min[cm_phase]    = max_current_limited_ma;
            cm_limits->spread[cm_phase] = max_current_limited_ma;
        } else {
            size_t pm_phase = cm_phase - 1; // PM is 0-based but CM is 1-based because of PV@0

            int32_t phase_current_meter_ma = currents_at_meter_raw_ma[pm_phase];

            if (phase_current_meter_ma == INT32_MAX) {
                if (!printed_skipping_currents_update && cm_phase == 1) {
                    logger.printfln("Dynamic load management unavailable because current values are not available yet.");
                    printed_skipping_currents_update = true;
                }

                cm_limits->raw[cm_phase] = 0;
                cm_limits->min[cm_phase] = 0;
            } else {
                if (printed_skipping_currents_update && cm_phase == 1) {
                    logger.printfln("Dynamic load management available because current values are now available.");
                    printed_skipping_currents_update = false;
                }

                // Preprocess meter value for CM. Estimates a sensible current value for the past 10s interval.
                minmax_filter *phase_preproc_max_ma = currents_phase_preproc_max_ma + pm_phase;
                update_minmax_filter(phase_current_meter_ma, phase_preproc_max_ma);
                mavg_filter *phase_preproc_mavg_ma = currents_phase_preproc_mavg_ma + pm_phase;
                int32_t phase_preproc_mavg_val_ma = update_mavg_filter(phase_current_meter_ma, phase_preproc_mavg_ma);

                int32_t phase_max_mavg_ma = (phase_preproc_max_ma->max + phase_preproc_mavg_val_ma) / 2;

                int32_t phase_preproc_ma;
                if (phase_current_meter_ma < currents_phase_preproc_mavg_limit) {
                    // raw < target + 10% -> mavg
                    phase_preproc_ma = phase_preproc_mavg_val_ma;
                } else if (phase_current_meter_ma > currents_phase_preproc_interpolate_limit) {
                    // raw > target + 20% -> (max+mavg)/2
                    phase_preproc_ma = phase_max_mavg_ma;

                    // Limit exceeded too much, trigger CM allocator run
                    if (deadline_elapsed(cm_allocator_trigger_hysteresis)) {
                        charge_manager.trigger_allocator_run();
                        cm_allocator_trigger_hysteresis = now_us() + 5_s;
                    }
                } else {
                    // else -> interpolate between mavg and (max+mavg)/2
                    int32_t interval_max_quantized = (phase_current_meter_ma - currents_phase_preproc_mavg_limit) / currents_phase_preproc_interpolate_quantization_factor;
                    int32_t interval_mavg_quantized = currents_phase_preproc_interpolate_interval_quantized - interval_max_quantized;
                    int32_t part_max  = phase_max_mavg_ma         * interval_max_quantized  / currents_phase_preproc_interpolate_interval_quantized;
                    int32_t part_mavg = phase_preproc_mavg_val_ma * interval_mavg_quantized / currents_phase_preproc_interpolate_interval_quantized;
                    phase_preproc_ma = part_mavg + part_max;
                }

                // Update low-level state
                state_i_meter  ->get(pm_phase)->updateInt(phase_current_meter_ma);
                state_i_pp_max ->get(pm_phase)->updateInt(phase_preproc_max_ma->max);
                state_i_pp_mavg->get(pm_phase)->updateInt(phase_preproc_mavg_val_ma);
                state_i_pp     ->get(pm_phase)->updateInt(phase_preproc_ma);

                // Current controller
                int32_t current_error_ma = target_phase_current_ma - phase_preproc_ma;

                int32_t cm_allocated_phase_current_ma = (*cm_allocated_currents)[cm_phase];
                int32_t current_adjust_ma;

                if (cm_allocated_phase_current_ma <= 0) {
                    // When no current was allocated to any charger on this phase, use p=1 so that the threshold for switching on can be reached properly.
                    current_adjust_ma = current_error_ma;

                    // Sanity check
                    if (cm_allocated_phase_current_ma < 0) {
                        logger.printfln("Negative cm_allocated_phase_current_ma: cm_allocated_currents(%i %i %i %i)", cm_allocated_currents->pv, cm_allocated_currents->l1, cm_allocated_currents->l2, cm_allocated_currents->l3);
                        cm_allocated_phase_current_ma = 0;
                    }
                } else {
                    if (current_error_ma < 0) {
                        // Negative error = over limit, reduce by 93.75% of error
                        current_adjust_ma = current_error_ma - current_error_ma / 16;
                    } else {
                        // Positive error = below limit, increase by 50% of error
                        current_adjust_ma = current_error_ma / 2;

                        // Cap to maximum allowed increase
                        current_adjust_ma = std::min(current_adjust_ma, phase_current_max_increase_ma);
                    }
                }

                int32_t phase_limit_raw_ma = cm_allocated_phase_current_ma + current_adjust_ma;

                phase_limit_raw_ma = min(phase_limit_raw_ma, max_current_limited_ma);

                // Calculate min
                minmax_filter *phase_min_ma = currents_phase_min_ma + pm_phase;
                update_minmax_filter(phase_limit_raw_ma, phase_min_ma);

                // Calculate long min
                if (current_long_min_iterations == 0) {
                    update_minmax_filter(currents_phase_floating_min_ma[pm_phase], currents_phase_long_min_ma + pm_phase);

                    currents_phase_floating_min_ma[pm_phase] = phase_limit_raw_ma;
                } else {
                    if (phase_limit_raw_ma < currents_phase_floating_min_ma[pm_phase]) {
                        currents_phase_floating_min_ma[pm_phase] = phase_limit_raw_ma;
                    }
                }

                int32_t phase_long_min_ma = std::min(currents_phase_floating_min_ma[pm_phase], currents_phase_long_min_ma[pm_phase].min);

                // Store limits
                cm_limits->raw[cm_phase] = phase_limit_raw_ma;
                cm_limits->min[cm_phase] = phase_min_ma->min;
                cm_limits->spread[cm_phase] = phase_long_min_ma;

#if ENABLE_PM_TRACE
                trace_log_len += snprintf_u(trace_log + trace_log_len, sizeof(trace_log) - trace_log_len, "  L%u m=%5i p=%5i err=%5i adj=%5i %5i<<%5i<%5i",
                    cm_phase, phase_current_meter_ma, phase_preproc_ma, current_error_ma, current_adjust_ma,
                    phase_long_min_ma, phase_min_ma->min, phase_limit_raw_ma);
#endif
            }
        }
    }

#if ENABLE_PM_TRACE
    trace_log_len += snprintf_u(trace_log + trace_log_len, sizeof(trace_log) - trace_log_len, "\n");
    logger.trace_plain(charge_manager.trace_buffer_index, trace_log, trace_log_len);
#endif

    // Calculate long-term minimum over one-minute blocks
    if (++current_long_min_iterations > 60 * (1000 / PM_TASK_DELAY_MS)) {
        current_long_min_iterations = 0;
    }

    low_level_state.get("charging_blocked")->updateUint(charging_blocked.combined);

    if ((mode == MODE_OFF) ||
        (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL && external_control.get("phases_wanted")->asUint() == 0)) {
        zero_limits();
    }

    // TODO check this
    if (charging_blocked.combined) {
        zero_limits();

        if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL)
            state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_UNAVAILABLE);

        return;
    }

    just_switched_mode = false;
}

void PowerManager::update_phase_switcher()
{
    if (phase_switcher_backend->get_phase_switching_state() == PhaseSwitcherBackend::SwitchingState::Error) {
        if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL)
            state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_UNAVAILABLE);
    }

    // TODO Check this
    if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL)
        state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_SWITCHING);
}

void PowerManager::set_max_current_limit(int32_t limit_ma)
{
    if (limit_ma > supply_cable_max_current_ma) {
        max_current_limited_ma = supply_cable_max_current_ma;
    } else {
        max_current_limited_ma = limit_ma;
    }
    low_level_state.get("max_current_limited")->updateInt(max_current_limited_ma);
}

void PowerManager::reset_max_current_limit()
{
    // Reset to maximum. Setter limits to allowed maximum.
    set_max_current_limit(INT32_MAX);
}

bool PowerManager::get_enabled() const
{
    return config.get("enabled")->asBool();
}

uint32_t PowerManager::get_phases() const
{
    return current_phases;
}

void PowerManager::set_config_error(uint32_t config_error_mask)
{
    config_error_flags |= config_error_mask;
    state.get("config_error_flags")->updateUint(config_error_flags);

#if MODULE_EM_COMMON_AVAILABLE()
    // TODO FIXME
    em_common.set_error(ERROR_FLAGS_BAD_CONFIG_MASK);
#endif
}

#if MODULE_AUTOMATION_AVAILABLE()
bool PowerManager::has_triggered(const Config *conf, void *data)
{
    const Config *cfg = static_cast<const Config *>(conf->get());

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

    switch (conf->getTag<AutomationTriggerID>()) {
        case AutomationTriggerID::PMPowerAvailable:
            return (*static_cast<bool *>(data) == cfg->get("power_available")->asBool());

        case AutomationTriggerID::PMGridPowerDraw:
            return ((power_at_meter_raw_w > 0) == cfg->get("drawing_power")->asBool());

        default:
            break;
    }

#pragma GCC diagnostic pop

    return false;
}
#endif
