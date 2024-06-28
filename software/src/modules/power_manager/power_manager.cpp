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

#include "power_manager.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

#include "gcc_warnings.h"

#if MODULE_ENERGY_MANAGER_AVAILABLE()
#define PM_ENABLED_DEFAULT (true)
#else
#define PM_ENABLED_DEFAULT (false)
#endif

void PowerManager::pre_setup()
{
    // States
    state = Config::Object({
        {"config_error_flags", Config::Uint32(0)},
        {"external_control", Config::Uint32(EXTERNAL_CONTROL_STATE_DISABLED)},
    });

    low_level_state = Config::Object({
        {"power_at_meter", Config::Float(0)},
        {"power_at_meter_filtered", Config::Float(0)}, //TODO make this int?
        {"power_available", Config::Int32(0)},
        {"power_available_filtered", Config::Int32(0)},
        {"overall_min_power", Config::Int32(0)},
        {"threshold_3to1", Config::Int32(0)},
        {"threshold_1to3", Config::Int32(0)},
        {"charge_manager_available_current", Config::Int32(0)},
        {"charge_manager_allocated_current", Config::Int32(0)},
        {"max_current_limited", Config::Int32(0)},
        {"uptime_past_hysteresis", Config::Bool(false)},
        {"is_3phase", Config::Bool(false)},
        {"wants_3phase", Config::Bool(false)}, // obsolete
        {"wants_3phase_last", Config::Bool(false)}, // obsolete
        {"is_on_last", Config::Bool(false)}, // obsolete
        {"wants_on_last", Config::Bool(false)}, // obsolete
        {"phase_state_change_blocked", Config::Bool(false)}, // obsolete
        {"phase_state_change_delay", Config::Uint32(0)}, // obsolete
        {"on_state_change_blocked", Config::Bool(false)}, // obsolete
        {"on_state_change_delay", Config::Uint32(0)}, // obsolete
        {"charging_blocked", Config::Uint32(0)},
        {"switching_state", Config::Uint32(0)},
    });

    config = ConfigRoot{Config::Object({
        {"enabled", Config::Bool(PM_ENABLED_DEFAULT)},
        {"phase_switching_mode", Config::Uint(PHASE_SWITCHING_AUTOMATIC, PHASE_SWITCHING_MIN, PHASE_SWITCHING_MAX)},
        {"excess_charging_enable", Config::Bool(false)},
        {"default_mode", Config::Uint(0, 0, 3)},
        {"meter_slot_grid_power", Config::Uint(POWER_MANAGER_DEFAULT_METER_SLOT, 0, METERS_SLOTS - 1)},
        {"target_power_from_grid", Config::Int32(0)}, // in watt
        {"guaranteed_power", Config::Uint(1380, 0, 22080)}, // in watt
        {"cloud_filter_mode", Config::Uint(CLOUD_FILTER_MEDIUM, CLOUD_FILTER_OFF, CLOUD_FILTER_STRONG)},
    }), [](const Config &cfg, ConfigSource source) -> String {
        if (cfg.get("phase_switching_mode")->asUint() == 3) { // external control
            if (cfg.get("excess_charging_enable")->asBool() != false) {
                return "Can't enable excess charging when external control is enabled for phase switching.";
            }
            if (cfg.get("default_mode")->asUint() != MODE_FAST) {
                return "Can't select any charging mode besides 'Fast' when extrenal control is enabled for phase switching.";
            }
        }

        return "";
    }};

    dynamic_load_config = Config::Object({
        {"enabled", Config::Bool(false)},
        {"current_limit", Config::Uint(0, 16, 9999999)}, // < 10kA
        {"largest_consumer_current", Config::Uint(40, 0, 999999)}, // < 1kA
        {"safety_margin_pct", Config::Uint(5, 0, 50)},
    });

    debug_config = Config::Object({
        {"hysteresis_time", Config::Uint(HYSTERESIS_MIN_TIME_MINUTES, 0, 60)}, // in minutes
    });

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
            api.callCommand("power_manager/external_control_update", Config::ConfUpdateObject{{
                {"phases_wanted", cfg->get("phases_wanted")->asUint()}
            }});
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

            api.callCommand("power_manager/charge_mode_update", Config::ConfUpdateObject{{
                {"mode", configured_mode}
            }});
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
                this->reset_limit_max_current();
            } else {
                this->limit_max_current(static_cast<uint32_t>(current));
            }
        },
        nullptr,
        false);

#if MODULE_ENERGY_MANAGER_AVAILABLE()
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

void PowerManager::setup()
{
    initialized = true;

    api.restorePersistentConfig("power_manager/config", &config);
    api.restorePersistentConfig("power_manager/dynamic_load_config", &dynamic_load_config);

#if MODULE_DEBUG_AVAILABLE()
    api.restorePersistentConfig("power_manager/debug_config", &debug_config);
#endif

    if (!config.get("enabled")->asBool()) {
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
    set_available_current(0, 0, 0, 0);

    // Cache config for energy update
    default_mode                = config.get("default_mode")->asUint();
    excess_charging_enabled     = config.get("excess_charging_enable")->asBool();
    meter_slot_power            = config.get("meter_slot_grid_power")->asUint();
    target_power_from_grid_w    = config.get("target_power_from_grid")->asInt();    // watt
    guaranteed_power_w          = config.get("guaranteed_power")->asUint();         // watt
    phase_switching_mode        = config.get("phase_switching_mode")->asUint();
    switching_hysteresis_ms     = debug_config.get("hysteresis_time")->asUint() * 60 * 1000;        // milliseconds (from minutes)
    dynamic_load_enabled        = dynamic_load_config.get("enabled")->asBool();
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

    // Set up meter power filters if excess charging is enabled
    if (excess_charging_enabled) {
        power_at_meter_smooth_w.mavg_values_count = CURRENT_POWER_SMOOTHING_SAMPLES;
        power_at_meter_smooth_w.mavg_values = static_cast<int32_t *>(heap_caps_malloc_prefer(CURRENT_POWER_SMOOTHING_SAMPLES * sizeof(power_at_meter_smooth_w.mavg_values[0]), 1, MALLOC_CAP_32BIT));

        power_at_meter_filtered_w.mavg_values_count = static_cast<int32_t>(values_count);
        power_at_meter_filtered_w.mavg_values = static_cast<int32_t *>(heap_caps_malloc_prefer(values_count * sizeof(power_at_meter_filtered_w.mavg_values[0]), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_32BIT));
    }

    // Set up meter current filters if dynamic load management is enabled
    if (dynamic_load_enabled) {
        for (size_t i = 0; i < ARRAY_SIZE(currents_at_meter_filtered_ma); i++) {
            mavg_filter *filter = currents_at_meter_filtered_ma + i;
            filter->mavg_values_count = static_cast<int32_t>(values_count);
            filter->mavg_values = static_cast<int32_t *>(heap_caps_malloc_prefer(values_count * sizeof(filter->mavg_values[0]), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_32BIT));
        }
    }

    // If the user accepts the additional wear, the minimum hysteresis time is 10s. Less than that will cause the control algorithm to oscillate.
    uint32_t hysteresis_min_ms = 10 * 1000;  // milliseconds
    if (switching_hysteresis_ms < hysteresis_min_ms)
        switching_hysteresis_ms = hysteresis_min_ms;

    // Pre-calculate various limits
    overall_min_power_w = 230 * 1 * min_current_1p_ma / 1000;

    if (static_cast<int32_t>(guaranteed_power_w) < overall_min_power_w) { // Cast safeguards against overall_min_power_w being negative and guaranteed_power_w is unlikely to be larger than 2GW.
        guaranteed_power_w = static_cast<uint32_t>(overall_min_power_w);  // This is now safe.
        logger.printfln("Raising guaranteed power to %u based on minimum charge current set in charge manager.", guaranteed_power_w);
    }

    if (dynamic_load_enabled) {
        int32_t current_limit_ma            = static_cast<int32_t>(dynamic_load_config.get("current_limit")->asUint());
        int32_t largest_consumer_current_ma = static_cast<int32_t>(dynamic_load_config.get("largest_consumer_current")->asUint());
        int32_t safety_margin_pct           = static_cast<int32_t>(dynamic_load_config.get("safety_margin_pct")->asUint());

        int32_t circuit_breaker_trip_point_ma = current_limit_ma + current_limit_ma / 2; // equal to *1.5
        int32_t max_possible_ma = circuit_breaker_trip_point_ma - largest_consumer_current_ma;
        target_phase_current_ma = std::min(max_possible_ma, current_limit_ma) * (100 - safety_margin_pct) / 100;

        phase_current_max_increase_ma = target_phase_current_ma / 4;

        //logger.printfln("cb trip %i  max %i  target phase current %i  max inc %i", circuit_breaker_trip_point_ma, max_possible_ma, target_phase_current_ma, phase_current_max_increase_ma); // TODO store in state instead
    }

    low_level_state.get("overall_min_power")->updateInt(overall_min_power_w);
    low_level_state.get("threshold_3to1")->updateInt(0); // TODO dead code
    low_level_state.get("threshold_1to3")->updateInt(0); // TODO dead code

    // Update data from meter and phase switcher back-end, requires power filter to be set up.
    update_data();

    // Check for incomplete configuration after as much as possible has been set up.

    bool power_meter_available = false;
#if MODULE_METERS_AVAILABLE()
    float unused_power;
    if (meters.get_power_virtual(meter_slot_power, &unused_power) == MeterValueAvailability::Unavailable) {
        meter_slot_power = UINT32_MAX;
    } else {
        power_meter_available = true;
    }
#endif
    if (excess_charging_enabled && !power_meter_available) {
        set_config_error(PM_CONFIG_ERROR_FLAGS_EXCESS_NO_METER_MASK);
        logger.printfln("Excess charging enabled but configured meter can't provide power values.");
    }

    task_scheduler.scheduleOnce([this]() {
        // Tell CM how many phases are available. is_3phase is updated in the previous call to update_all_data().
        // set_available_phases() uses callCommand(), which is not available during setup phase, so schedule a task for it.
        set_available_phases(is_3phase ? 3 : 1);

        // Can't check for chargers in setup() because CM's setup() hasn't run yet to load the charger configuration.
        if (charge_manager.get_charger_count() <= 0) {
            logger.printfln("No chargers configured. Won't try to distribute energy.");
            set_config_error(PM_CONFIG_ERROR_FLAGS_NO_CHARGERS_MASK);
        }
    }, 0);

    // The default configuration after a factory reset must be good enough for everything to run without crashing.
    if (!phase_switcher_backend->phase_switching_capable()) {
        switch (phase_switching_mode) {
            case PHASE_SWITCHING_ALWAYS_1PHASE:
            case PHASE_SWITCHING_ALWAYS_3PHASE:
                break;
            default: {
                    const char *err_reason;
#if MODULE_ENERGY_MANAGER_AVAILABLE()
                    err_reason = "no contactor installed";
#elif 0 // FIXME: charger back-end
                    err_reason = "charger doesn't support it";
#else
                    err_reason = "not supported by back-end";
#endif
                    logger.printfln("Invalid configuration: Phase switching enabled but %s.", err_reason);
                    set_config_error(PM_CONFIG_ERROR_FLAGS_PHASE_SWITCHING_MASK);
                    return;
                }
        }
    }

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
        uint32_t phases_wanted = is_3phase ? 3 : 1;
        external_control.get("phases_wanted")->updateUint(phases_wanted);
    }

    reset_limit_max_current();

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->update_data();
        this->update_energy();
        this->update_phase_switcher();
    }, PM_TASK_DELAY_MS, PM_TASK_DELAY_MS);
}

void PowerManager::register_urls()
{
    api.addState("power_manager/state", &state);

    api.addPersistentConfig("power_manager/config", &config);
    api.addPersistentConfig("power_manager/dynamic_load_config", &dynamic_load_config);
#if MODULE_DEBUG_AVAILABLE()
    api.addPersistentConfig("power_manager/debug_config", &debug_config);
#endif

    api.addState("power_manager/low_level_state", &low_level_state);

    api.addState("power_manager/charge_mode", &charge_mode);
    api.addCommand("power_manager/charge_mode_update", &charge_mode_update, {}, [this]() {
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

    if (!config.get("enabled")->asBool() && phase_switcher_backend->phase_switching_capable() && !phase_switcher_backend->requires_cp_disconnect()) {
        logger.printfln("Disabled but phase switching backend can switch autonomously. Enabling external control API.");

        api.addFeature("phase_switch");
#if MODULE_AUTOMATION_AVAILABLE()
        automation.set_enabled(AutomationActionID::PMPhaseSwitch, true);
#endif

        api.addCommand("power_manager/external_control_update", &external_control_update, {}, [this]() {
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

            bool _wants_3phase = external_control_update.get("phases_wanted")->asUint() == 3;
            if (phase_switcher_backend->switch_phases_3phase(_wants_3phase))
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
            low_level_state.get("is_3phase")->updateBool(phase_switcher_backend->get_is_3phase());
        }, 1000, 1000);
    } else {
        api.addCommand("power_manager/external_control_update", &external_control_update, {}, [this]() {
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

void PowerManager::set_available_current(int32_t current_pv, int32_t current_L1, int32_t current_L2, int32_t current_L3)
{
    cm_limits->raw.pv = current_pv;
    cm_limits->raw.l1 = current_L1;
    cm_limits->raw.l2 = current_L2;
    cm_limits->raw.l3 = current_L3;

    low_level_state.get("charge_manager_available_current")->updateInt(current_pv);
}

void PowerManager::set_available_phases(uint32_t phases)
{
    String err = api.callCommand("charge_manager/available_phases_update", Config::ConfUpdateObject{{
        {"phases", phases},
    }});

    if (!err.isEmpty())
        logger.printfln("set_available_phases failed: %s", err.c_str());
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
    is_3phase = phase_switcher_backend->phase_switching_capable() ? phase_switcher_backend->get_is_3phase() : phase_switching_mode == PHASE_SWITCHING_ALWAYS_3PHASE;
    have_phases = 1 + static_cast<int32_t>(is_3phase) * 2;
    low_level_state.get("is_3phase")->updateBool(is_3phase);

#if MODULE_METERS_AVAILABLE()
    if (meters.get_power_virtual(meter_slot_power, &power_at_meter_raw_w) != MeterValueAvailability::Fresh)
        power_at_meter_raw_w = NAN;
#else
    power_at_meter_raw_w = NAN;
#endif

    if (!isnan(power_at_meter_raw_w)) {
        low_level_state.get("power_at_meter")->updateFloat(power_at_meter_raw_w);

        // Filter power value only when excess charging is enabled
        if (excess_charging_enabled) {
            int32_t raw_power_w = static_cast<int32_t>(power_at_meter_raw_w);

            // Filtered/smoothed values must not be modified anywhere else

            update_mavg_filter(raw_power_w, &power_at_meter_smooth_w);
            int32_t filtered_w = update_mavg_filter(raw_power_w, &power_at_meter_filtered_w);

            low_level_state.get("power_at_meter_filtered")->updateFloat(static_cast<float>(filtered_w));
        }

#if MODULE_AUTOMATION_AVAILABLE()
        TristateBool drawing_power = static_cast<TristateBool>(power_at_meter_raw_w > 0);
        if (drawing_power != automation_drawing_power_last && boot_stage > BootStage::SETUP) {
            automation.trigger(AutomationTriggerID::PMGridPowerDraw, nullptr, this);
            automation_drawing_power_last = drawing_power;
        }
#endif
    }

    float meter_currents[INDEX_CACHE_CURRENT_COUNT];
#if MODULE_METERS_AVAILABLE()
    MeterValueAvailability ret = meters.get_currents(meter_slot_power, meter_currents);
    if (ret != MeterValueAvailability::Fresh) {
#else
    if (true) {
#endif
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

                // Filtered values must not be modified anywhere else

                /*int32_t filtered_current_ma =*/ update_mavg_filter(raw_current_ma, &currents_at_meter_filtered_ma[i]);

                // TODO Store in low_level_state
            }
        }
    }
}

void PowerManager::update_energy()
{
    int32_t cm_total_allocated_current_ma = 0;

    for (size_t i = 1; i <= 3; i++) {
        int32_t allocated_current = (*cm_allocated_currents)[i];
        if (allocated_current < 0) {
            logger.printfln("CM allocated current on L%u negative: %i", i, allocated_current);
        } else {
            cm_total_allocated_current_ma += allocated_current;
        }
    }

    low_level_state.get("charge_manager_allocated_current")->updateInt(cm_total_allocated_current_ma);

    if (!excess_charging_enabled) {
        power_available_w          = INT32_MAX;
        power_available_filtered_w = INT32_MAX;
    } else {
        int32_t power_smooth_w   = power_at_meter_smooth_w.filtered_val;
        int32_t power_filtered_w = power_at_meter_filtered_w.filtered_val;

        if (power_smooth_w == INT32_MAX) {
            if (!printed_skipping_energy_update) {
                logger.printfln("PV excess charging unavailable because power values are not available yet.");
                printed_skipping_energy_update = true;
            }

            power_available_w          = 0;
            power_available_filtered_w = 0;
        } else {
            if (power_filtered_w == INT32_MAX) {
                logger.printfln("Uninitialized power_at_meter_filtered_w leaked");
                return;
            }

            if (printed_skipping_energy_update) {
                logger.printfln("PV excess charging available because power values are now available.");
                printed_skipping_energy_update = false;
            }

            int32_t p_error_w          = target_power_from_grid_w - power_smooth_w;
            int32_t p_error_filtered_w = target_power_from_grid_w - power_filtered_w;

#if MODULE_ENERGY_MANAGER_AVAILABLE()
            if (p_error_w > 200) {
                energy_manager.update_grid_balance_led(EmRgbLed::GridBalance::Export);
            } else if (p_error_w < -200) {
                energy_manager.update_grid_balance_led(EmRgbLed::GridBalance::Import);
            } else {
                energy_manager.update_grid_balance_led(EmRgbLed::GridBalance::Balanced);
            }
#endif

            switch (mode) {
                case MODE_FAST:
                    power_available_w          = INT32_MAX;
                    power_available_filtered_w = INT32_MAX;
                    break;
                case MODE_OFF:
                default:
                    power_available_w          = 0;
                    power_available_filtered_w = 0;
                    break;
                case MODE_PV:
                case MODE_MIN_PV:
                    // Excess charging uses an adaptive P controller to adjust available power.
                    int32_t p_adjust_w;
                    int32_t p_adjust_filtered_w;
                    const int32_t cm_allocated_power_w = cm_total_allocated_current_ma * 230 / 1000; // ma -> watt

                    if (cm_allocated_power_w == 0) {
                        // When no power was allocated to any charger, use p=1 so that the threshold for switching on can be reached properly.
                        p_adjust_w          = p_error_w;
                        p_adjust_filtered_w = p_error_filtered_w;
                    } else {
                        // Some EVs may only be able to adjust their charge power in steps of 1500W,
                        // so smaller factors are required for smaller errors.
                        int32_t p_error_abs_w = abs(p_error_w);
                        if (p_error_abs_w < 1000) {
                            // Use p=0.5 for small differences so that the controller can converge without oscillating too much.
                            p_adjust_w          = p_error_w          / 2;
                            p_adjust_filtered_w = p_error_filtered_w / 2;
                        } else if (p_error_abs_w < 1500) {
                            // Use p=0.75 for medium differences so that the controller can converge reasonably fast while still avoiding too many oscillations.
                            p_adjust_w          = p_error_w          * 3 / 4;
                            p_adjust_filtered_w = p_error_filtered_w * 3 / 4;
                        } else {
                            // Use p=0.875 for large differences so that the controller can converge faster.
                            p_adjust_w          = p_error_w          * 7 / 8;
                            p_adjust_filtered_w = p_error_filtered_w * 7 / 8;
                        }
                    }

                    power_available_w          = cm_allocated_power_w + p_adjust_w;
                    power_available_filtered_w = cm_allocated_power_w + p_adjust_filtered_w;

                    if (mode != MODE_MIN_PV)
                        break;

                    // Check against guaranteed power only in MIN_PV mode.
                    if (power_available_w          < static_cast<int32_t>(guaranteed_power_w))
                        power_available_w          = static_cast<int32_t>(guaranteed_power_w);
                    if (power_available_filtered_w < static_cast<int32_t>(guaranteed_power_w))
                        power_available_filtered_w = static_cast<int32_t>(guaranteed_power_w);

                    break;
            }
        }
    }

    low_level_state.get("power_available")->updateInt(power_available_w);
    low_level_state.get("power_available_filtered")->updateInt(power_available_filtered_w);

    cm_limits->raw.pv      = power_available_w          == INT32_MAX ? INT32_MAX : power_available_w          * 1000 / 230;
    cm_limits->filtered.pv = power_available_filtered_w == INT32_MAX ? INT32_MAX : power_available_filtered_w * 1000 / 230;
    cm_limits->min.pv = cm_limits->raw.pv;
    cm_limits->max.pv = cm_limits->raw.pv;

#if MODULE_AUTOMATION_AVAILABLE()
    bool wants_on = power_available_filtered_w >= overall_min_power_w;
    TristateBool automation_power_available = static_cast<TristateBool>(wants_on);
    if (automation_power_available != automation_power_available_last) {
        automation.trigger(AutomationTriggerID::PMPowerAvailable, &wants_on, this);
        automation_power_available_last = automation_power_available;
    }
#endif

    for (size_t i = 1; i < 4; i++) {
        if (!dynamic_load_enabled) {
            cm_limits->raw[i]      = supply_cable_max_current_ma;
            cm_limits->filtered[i] = supply_cable_max_current_ma;
            cm_limits->min[i]      = supply_cable_max_current_ma;
            cm_limits->max[i]      = supply_cable_max_current_ma;
        } else {
            int32_t phase_current_raw_ma      = currents_at_meter_raw_ma[i - 1]; // No PV phase at 0
            int32_t phase_current_filtered_ma = currents_at_meter_filtered_ma[i - 1].filtered_val;

            if (phase_current_raw_ma == INT32_MAX) {
                cm_limits->raw[i]      = 0;
                cm_limits->filtered[i] = 0;
                cm_limits->min[i]      = 0;
                cm_limits->max[i]      = 0;
            } else {
                int32_t current_error_ma          = target_phase_current_ma - phase_current_raw_ma;
                int32_t current_error_filtered_ma = target_phase_current_ma - phase_current_filtered_ma;

                int32_t current_adjust_ma;
                int32_t current_adjust_filtered_ma;

                if (cm_total_allocated_current_ma == 0) {
                    // When no current was allocated to any charger, use p=1 so that the threshold for switching on can be reached properly.
                    current_adjust_ma          = current_error_ma;
                    current_adjust_filtered_ma = current_error_filtered_ma;
                } else {
                    if (current_error_ma < 0) {
                        // Negative error = over limit, reduce by 93.75% of error
                        current_adjust_ma          = current_error_ma          - current_error_ma          / 16;
                        current_adjust_filtered_ma = current_error_filtered_ma - current_error_filtered_ma / 16;
                    } else {
                        // Positive error = below limit, increase by 50% of error
                        current_adjust_ma          = current_error_ma / 2;
                        current_adjust_filtered_ma = current_error_filtered_ma / 2;

                        // Cap to maximum allowed increase
                        current_adjust_ma          = std::min(current_adjust_ma,          phase_current_max_increase_ma);
                        current_adjust_filtered_ma = std::min(current_adjust_filtered_ma, phase_current_max_increase_ma);
                    }
                }

                int32_t cm_allocated_phase_current_ma = (*cm_allocated_currents)[i];
                int32_t phase_limit_raw_ma      = cm_allocated_phase_current_ma + current_adjust_ma;
                int32_t phase_limit_filtered_ma = cm_allocated_phase_current_ma + current_adjust_filtered_ma;

                phase_limit_raw_ma      = min(phase_limit_raw_ma,      supply_cable_max_current_ma);
                phase_limit_filtered_ma = min(phase_limit_filtered_ma, supply_cable_max_current_ma);

                cm_limits->raw[i]      = phase_limit_raw_ma;
                cm_limits->filtered[i] = phase_limit_filtered_ma;

                cm_limits->min[i] = phase_limit_raw_ma;
                cm_limits->max[i] = phase_limit_raw_ma;

                //logger.printfln("L%u  meter=%i err=%i adj=%i limit=%i", i, phase_current_raw_ma, current_error_ma, current_adjust_ma, phase_limit_raw_ma);
            }
        }
    }

    low_level_state.get("max_current_limited")->updateInt(max_current_limited_ma); // TODO Calculate/use this?
    low_level_state.get("charging_blocked")->updateUint(charging_blocked.combined);

    if ((mode == MODE_OFF) ||
        (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL && external_control.get("phases_wanted")->asUint() == 0)) {
        set_available_current(0, 0, 0, 0);
    }

    // TODO check this
    if (charging_blocked.combined) {
        set_available_current(0, 0, 0, 0);

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

void PowerManager::limit_max_current(uint32_t limit_ma)
{
    if (max_current_limited_ma > static_cast<int32_t>(limit_ma))
        max_current_limited_ma = static_cast<int32_t>(limit_ma);
}

void PowerManager::reset_limit_max_current()
{
    max_current_limited_ma = supply_cable_max_current_ma;
}

bool PowerManager::get_enabled() const
{
    return config.get("enabled")->asBool();
}

bool PowerManager::get_is_3phase() const
{
    return is_3phase;
}

void PowerManager::set_config_error(uint32_t config_error_mask)
{
    config_error_flags |= config_error_mask;
    state.get("config_error_flags")->updateUint(config_error_flags);

#if MODULE_ENERGY_MANAGER_AVAILABLE()
    // TODO FIXME
    energy_manager.set_error(ERROR_FLAGS_BAD_CONFIG_MASK);
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
