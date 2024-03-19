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
#include "module_dependencies.h"

#include "api.h"
#include "event_log.h"

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
        {"charge_manager_available_current", Config::Uint32(0)},
        {"charge_manager_allocated_current", Config::Uint32(0)},
        {"max_current_limited", Config::Uint32(0)},
        {"uptime_past_hysteresis", Config::Bool(false)},
        {"is_3phase", Config::Bool(false)},
        {"wants_3phase", Config::Bool(false)},
        {"wants_3phase_last", Config::Bool(false)},
        {"is_on_last", Config::Bool(false)},
        {"wants_on_last", Config::Bool(false)},
        {"phase_state_change_blocked", Config::Bool(false)},
        {"phase_state_change_delay", Config::Uint32(0)},
        {"on_state_change_blocked", Config::Bool(false)},
        {"on_state_change_delay", Config::Uint32(0)},
        {"charging_blocked", Config::Uint32(0)},
        {"switching_state", Config::Uint32(0)},
    });

    config = ConfigRoot{Config::Object({
        {"enabled", Config::Bool(PM_ENABLED_DEFAULT)},
        {"phase_switching_mode", Config::Uint(PHASE_SWITCHING_AUTOMATIC, PHASE_SWITCHING_MIN, PHASE_SWITCHING_MAX)},
        {"excess_charging_enable", Config::Bool(false)},
        {"default_mode", Config::Uint(0, 0, 3)},
        {"meter_slot_grid_power", Config::Uint(0, 0, METERS_SLOTS - 1)},
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

    debug_config = Config::Object({
        {"hysteresis_time", Config::Uint(HYSTERESIS_MIN_TIME_MINUTES, 0, 60)}, // in minutes
    });

    // Runtime config
    charge_mode = Config::Object({
        {"mode", Config::Uint(0, 0, 3)},
    });
    charge_mode_update = charge_mode;

    external_control = Config::Object({
        {"phases_wanted", Config::Uint32(0)},
    });
    external_control_update = external_control;

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
        });

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
        });

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
        });

    automation.register_action(
        AutomationActionID::PMBlockCharge,
        Config::Object({
            {"slot", Config::Uint(0, 0, 3)},
            {"block", Config::Bool(false)}
        }),
        [this](const Config *cfg) {
            this->charging_blocked.pin[cfg->get("slot")->asUint()] = static_cast<uint8_t>(cfg->get("block")->asBool());
        });

    automation.register_trigger(
        AutomationTriggerID::PMPowerAvailable,
        Config::Object({
            {"power_available", Config::Bool(false)}
        }));

    automation.register_trigger(
        AutomationTriggerID::PMGridPowerDraw,
        Config::Object({
            {"drawing_power", Config::Bool(false)}
        }));
#endif
}

void PowerManager::setup()
{
    initialized = true;

    api.restorePersistentConfig("power_manager/config", &config);

#if MODULE_DEBUG_AVAILABLE()
    api.restorePersistentConfig("power_manager/debug_config", &debug_config);
#endif

    if (!config.get("enabled")->asBool()) {
        return;
    }

    charge_manager.set_allocated_current_callback([this](uint32_t current_ma) {
        //logger.printfln("power_manager: allocated current callback: %u", current_ma);
        charge_manager_allocated_current_ma = current_ma;
    });

    // Cache config for energy update
    default_mode                = config.get("default_mode")->asUint();
    excess_charging_enable      = config.get("excess_charging_enable")->asBool();
    meter_slot_power            = config.get("meter_slot_grid_power")->asUint();
    target_power_from_grid_w    = config.get("target_power_from_grid")->asInt();    // watt
    guaranteed_power_w          = config.get("guaranteed_power")->asUint();         // watt
    phase_switching_mode        = config.get("phase_switching_mode")->asUint();
    switching_hysteresis_ms     = debug_config.get("hysteresis_time")->asUint() * 60 * 1000;        // milliseconds (from minutes)
    max_current_unlimited_ma    = charge_manager.config.get("maximum_available_current")->asUint(); // milliampere
    min_current_1p_ma           = charge_manager.config.get("minimum_current_1p")->asUint();        // milliampere
    min_current_3p_ma           = charge_manager.config.get("minimum_current")->asUint();           // milliampere

    mode = default_mode;
    charge_mode.get("mode")->updateUint(mode);

    if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL)
        state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_UNAVAILABLE);

    // Set up meter power filter.
    uint32_t power_mavg_span_s;
    switch (config.get("cloud_filter_mode")->asUint()) {
        default:
        case CLOUD_FILTER_OFF:    power_mavg_span_s =   0; break;
        case CLOUD_FILTER_LIGHT:  power_mavg_span_s = 120; break;
        case CLOUD_FILTER_MEDIUM: power_mavg_span_s = 240; break;
        case CLOUD_FILTER_STRONG: power_mavg_span_s = 480; break;
    }
    if (power_mavg_span_s <= 0) {
        power_at_meter_mavg_values_count = 1;
    } else {
        power_at_meter_mavg_values_count = static_cast<int32_t>(power_mavg_span_s * 1000 / PM_TASK_DELAY_MS);
    }
    power_at_meter_mavg_values_w = static_cast<int32_t *>(heap_caps_malloc_prefer(static_cast<size_t>(power_at_meter_mavg_values_count) * sizeof(power_at_meter_mavg_values_w[0]), 2, MALLOC_CAP_32BIT, MALLOC_CAP_SPIRAM));

    // If the user accepts the additional wear, the minimum hysteresis time is 10s. Less than that will cause the control algorithm to oscillate.
    uint32_t hysteresis_min_ms = 10 * 1000;  // milliseconds
    if (switching_hysteresis_ms < hysteresis_min_ms)
        switching_hysteresis_ms = hysteresis_min_ms;

    // Pre-calculate various limits
    int32_t min_phases;
    if (phase_switching_mode == PHASE_SWITCHING_ALWAYS_1PHASE) {
        min_phases = 1;
        max_phases = 1;
    } else if (phase_switching_mode == PHASE_SWITCHING_ALWAYS_3PHASE) {
        min_phases = 3;
        max_phases = 3;
    } else { // automatic, external or PV1P/FAST3P
        min_phases = 1;
        max_phases = 3;
    }
    if (min_phases < 3) {
        overall_min_power_w = static_cast<int32_t>(230 * 1 * min_current_1p_ma / 1000);
    } else {
        overall_min_power_w = static_cast<int32_t>(230 * 3 * min_current_3p_ma / 1000);
    }

    const int32_t max_1phase_w = static_cast<int32_t>(230 * 1 * max_current_unlimited_ma / 1000);
    const int32_t min_3phase_w = static_cast<int32_t>(230 * 3 * min_current_3p_ma / 1000);

    if (min_3phase_w > max_1phase_w) { // have dead current range
        int32_t range_width = min_3phase_w - max_1phase_w;
        threshold_3to1_w = max_1phase_w + static_cast<int32_t>(0.25 * range_width);
        threshold_1to3_w = max_1phase_w + static_cast<int32_t>(0.75 * range_width);
    } else { // no dead current range, use simple limits
        threshold_3to1_w = min_3phase_w;
        threshold_1to3_w = max_1phase_w;
    }

    low_level_state.get("overall_min_power")->updateInt(overall_min_power_w);
    low_level_state.get("threshold_3to1")->updateInt(threshold_3to1_w);
    low_level_state.get("threshold_1to3")->updateInt(threshold_1to3_w);

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
    if (excess_charging_enable && !power_meter_available) {
        set_config_error(PM_CONFIG_ERROR_FLAGS_EXCESS_NO_METER_MASK);
        logger.printfln("power_manager: Excess charging enabled but configured meter can't provide power values.");
    }

    task_scheduler.scheduleOnce([this]() {
        // Tell CM how many phases are available. is_3phase is updated in the previous call to update_all_data().
        // set_available_phases() uses callCommand(), which is not available during setup phase, so schedule a task for it.
        set_available_phases(is_3phase ? 3 : 1);

        // Can't check for chargers in setup() because CM's setup() hasn't run yet to load the charger configuration.
        if (charge_manager.get_charger_count() <= 0) {
            logger.printfln("power_manager: No chargers configured. Won't try to distribute energy.");
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
                    logger.printfln("power_manager: Invalid configuration: Phase switching enabled but %s.", err_reason);
                    set_config_error(PM_CONFIG_ERROR_FLAGS_PHASE_SWITCHING_MASK);
                    return;
                }
        }
    }

    if (max_current_unlimited_ma == 0) {
        logger.printfln("power_manager: No maximum current configured for chargers. Disabling energy distribution.");
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
    }, PM_TASK_DELAY_MS, PM_TASK_DELAY_MS);

    task_scheduler.scheduleOnce([this]() {
        uptime_past_hysteresis = true;
        low_level_state.get("uptime_past_hysteresis")->updateBool(uptime_past_hysteresis);
    }, switching_hysteresis_ms);
}

void PowerManager::register_urls()
{
    api.addState("power_manager/state", &state);

    api.addPersistentConfig("power_manager/config", &config);
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
        just_switched_mode = runtime_mode->updateUint(new_mode);
        mode = new_mode;

        logger.printfln("power_manager: Switched mode %u->%u", old_mode, mode);
    }, false);

    api.addState("power_manager/external_control", &external_control);
    api.addCommand("power_manager/external_control_update", &external_control_update, {}, [this]() {
        uint32_t external_control_state = state.get("external_control")->asUint();
        switch (external_control_state) {
            case EXTERNAL_CONTROL_STATE_DISABLED:
                logger.printfln("power_manager: Ignoring external control phase change request: External control is not enabled.");
                return;
            case EXTERNAL_CONTROL_STATE_UNAVAILABLE:
                logger.printfln("power_manager: Ignoring external control phase change request: Phase switching is currently unavailable.");
                return;
            case EXTERNAL_CONTROL_STATE_SWITCHING:
                logger.printfln("power_manager: Ignoring external control phase change request: Phase switching in progress.");
                return;
            default:
                break; // All good, proceed.
        }

        auto phases_wanted = external_control.get("phases_wanted");
        uint32_t old_phases = phases_wanted->asUint();
        uint32_t new_phases = external_control_update.get("phases_wanted")->asUint();

        if (new_phases == old_phases) {
            logger.printfln("power_manager: Ignoring external control phase change request: Value is already %u.", new_phases);
            return;
        }

        if (new_phases == 2 || new_phases > 3) {
            logger.printfln("power_manager: Ignoring external control phase change request: Value %u is invalid.", new_phases);
            return;
        }

        logger.printfln("power_manager: External control phase change request: switching from %u to %u", old_phases, new_phases);
        phases_wanted->updateUint(new_phases);
    }, true);
}

void PowerManager::register_phase_switcher_backend(PhaseSwitcherBackend *backend)
{
    phase_switcher_backend = backend;
}

void PowerManager::set_available_current(uint32_t current)
{
    is_on_last = current > 0;

    String err = api.callCommand("charge_manager/available_current_update", Config::ConfUpdateObject{{
        {"current", current},
    }});

    if (!err.isEmpty())
        logger.printfln("power_manager: set_available_current failed: %s", err.c_str());

    charge_manager_available_current_ma = current;
    low_level_state.get("charge_manager_available_current")->updateUint(current);
}

void PowerManager::set_available_phases(uint32_t phases)
{
    String err = api.callCommand("charge_manager/available_phases_update", Config::ConfUpdateObject{{
        {"phases", phases},
    }});

    if (!err.isEmpty())
        logger.printfln("power_manager: set_available_phases failed: %s", err.c_str());
}

void PowerManager::update_data()
{
    // Update states from back-end
    is_3phase = phase_switcher_backend->phase_switching_capable() ? phase_switcher_backend->get_is_3phase() : phase_switching_mode == PHASE_SWITCHING_ALWAYS_3PHASE;
    have_phases = 1 + static_cast<uint32_t>(is_3phase) * 2;
    low_level_state.get("is_3phase")->updateBool(is_3phase);

#if MODULE_METERS_AVAILABLE()
    if (meters.get_power_virtual(meter_slot_power, &power_at_meter_raw_w) != MeterValueAvailability::Fresh)
        power_at_meter_raw_w = NAN;
#else
    power_at_meter_raw_w = NAN;
#endif

    if (!isnan(power_at_meter_raw_w)) {
        low_level_state.get("power_at_meter")->updateFloat(power_at_meter_raw_w);
        int32_t raw_power_w = static_cast<int32_t>(power_at_meter_raw_w);

        // Filtered/smoothed values must not be modified anywhere else.

        // Check if smooth values need to be initialized.
        if (power_at_meter_smooth_w == INT32_MAX) {
            for (int32_t i = 0; i < CURRENT_POWER_SMOOTHING_SAMPLES; i++) {
                power_at_meter_smooth_values_w[i] = raw_power_w;
            }
            power_at_meter_smooth_total = raw_power_w * CURRENT_POWER_SMOOTHING_SAMPLES;
        } else {
            power_at_meter_smooth_total = power_at_meter_smooth_total - power_at_meter_smooth_values_w[power_at_meter_smooth_position] + raw_power_w;
            power_at_meter_smooth_values_w[power_at_meter_smooth_position] = raw_power_w;
            power_at_meter_smooth_position++;
            if (power_at_meter_smooth_position >= CURRENT_POWER_SMOOTHING_SAMPLES)
                power_at_meter_smooth_position = 0;
        }

        // Signed division requires both numbers to be signed.
        static_assert(std::is_same<int32_t, decltype(power_at_meter_smooth_total)>::value, "power_at_meter_smooth_total must be signed");
        power_at_meter_smooth_w = power_at_meter_smooth_total / CURRENT_POWER_SMOOTHING_SAMPLES;

        // Check if filter values need to be initialized.
        if (power_at_meter_filtered_w == INT32_MAX) {
            for (int32_t i = 0; i < power_at_meter_mavg_values_count; i++) {
                power_at_meter_mavg_values_w[i] = raw_power_w;
            }
            power_at_meter_mavg_total = raw_power_w * power_at_meter_mavg_values_count;
        } else {
            power_at_meter_mavg_total = power_at_meter_mavg_total - power_at_meter_mavg_values_w[power_at_meter_mavg_position] + raw_power_w;
            power_at_meter_mavg_values_w[power_at_meter_mavg_position] = raw_power_w;
            power_at_meter_mavg_position++;
            if (power_at_meter_mavg_position >= power_at_meter_mavg_values_count)
                power_at_meter_mavg_position = 0;
        }

        // Signed division requires both numbers to be signed.
        static_assert(std::is_same<int32_t, decltype(power_at_meter_mavg_total       )>::value, "power_at_meter_mavg_total must be signed");
        static_assert(std::is_same<int32_t, decltype(power_at_meter_mavg_values_count)>::value, "power_at_meter_mavg_values_count must be signed");
        power_at_meter_filtered_w = power_at_meter_mavg_total / power_at_meter_mavg_values_count;

        low_level_state.get("power_at_meter_filtered")->updateFloat(static_cast<float>(power_at_meter_filtered_w));
    }

#if MODULE_AUTOMATION_AVAILABLE()
    TristateBool drawing_power = static_cast<TristateBool>(power_at_meter_raw_w > 0);
    if (drawing_power != automation_drawing_power_last && boot_stage > BootStage::SETUP) {
        automation.trigger_action(AutomationTriggerID::PMGridPowerDraw, nullptr, [this](const Config *cfg, void *data) -> bool {return this->action_triggered(cfg, data);});
        automation_drawing_power_last = drawing_power;
    }
#endif
}

void PowerManager::update_energy()
{
    if (switching_state != switching_state_prev) {
        logger.printfln("power_manager: Now in state %i", static_cast<int>(switching_state));
        switching_state_prev = switching_state;
        low_level_state.get("switching_state")->updateUint(static_cast<uint32_t>(switching_state));
    }

    if (phase_switcher_backend->get_phase_switching_state() == PhaseSwitcherBackend::SwitchingState::Error) {
        set_available_current(0);

        if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL)
            state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_UNAVAILABLE);

        return;
    }

    if (switching_state == SwitchingState::Monitoring) {
        const bool     is_on = is_on_last;
        const uint32_t charge_manager_allocated_power_w = 230 * have_phases * charge_manager_allocated_current_ma / 1000; // watt

        low_level_state.get("charge_manager_allocated_current")->updateUint(charge_manager_allocated_current_ma);
        low_level_state.get("max_current_limited")->updateUint(max_current_limited_ma);
        low_level_state.get("charging_blocked")->updateUint(charging_blocked.combined);

        if (charging_blocked.combined) {
            if (is_on) {
                phase_state_change_blocked_until = on_state_change_blocked_until = millis() + switching_hysteresis_ms;
            }
            set_available_current(0);
            just_switched_phases = false;

            if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL)
                state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_UNAVAILABLE);

            return;
        }

        int32_t p_error_w, p_error_filtered_w;
        if (!excess_charging_enable) {
            p_error_w          = 0;
            p_error_filtered_w = 0;
        } else {
            if (power_at_meter_smooth_w == INT32_MAX) {
                if (!printed_skipping_energy_update) {
                    logger.printfln("power_manager: Pausing energy updates because power value is not available yet.");
                    printed_skipping_energy_update = true;
                }
                return;
            } else {
                if (printed_skipping_energy_update) {
                    logger.printfln("power_manager: Resuming energy updates because power value is now available.");
                    printed_skipping_energy_update = false;
                }
            }
            p_error_w          = target_power_from_grid_w - power_at_meter_smooth_w;
            p_error_filtered_w = target_power_from_grid_w - power_at_meter_filtered_w;

#if MODULE_ENERGY_MANAGER_AVAILABLE()
            if (p_error_w > 200) {
                energy_manager.update_grid_balance_led(EmRgbLed::GridBalance::Export);
            } else if (p_error_w < -200) {
                energy_manager.update_grid_balance_led(EmRgbLed::GridBalance::Import);
            } else {
                energy_manager.update_grid_balance_led(EmRgbLed::GridBalance::Balanced);
            }
#endif
        }

        switch (mode) {
            case MODE_FAST:
                power_available_w          = static_cast<int32_t>(230 * max_phases * max_current_limited_ma / 1000);
                power_available_filtered_w = power_available_w;
                break;
            case MODE_OFF:
            default:
                power_available_w          = 0;
                power_available_filtered_w = 0;
                break;
            case MODE_PV:
            case MODE_MIN_PV:
                // Excess charging enabled; use a simple P controller to adjust available power.
                int32_t p_adjust_w;
                int32_t p_adjust_filtered_w;
                if (!is_on) {
                    // When the power is not on, use p=1 so that the switch-on threshold can be reached properly.
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

                power_available_w          = static_cast<int32_t>(charge_manager_allocated_power_w) + p_adjust_w;
                power_available_filtered_w = static_cast<int32_t>(charge_manager_allocated_power_w) + p_adjust_filtered_w;

                if (mode != MODE_MIN_PV)
                    break;

                // Check against guaranteed power only in MIN_PV mode.
                if (power_available_w          < static_cast<int32_t>(guaranteed_power_w))
                    power_available_w          = static_cast<int32_t>(guaranteed_power_w);
                if (power_available_filtered_w < static_cast<int32_t>(guaranteed_power_w))
                    power_available_filtered_w = static_cast<int32_t>(guaranteed_power_w);

                break;
        }

        if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL && external_control.get("phases_wanted")->asUint() == 0) {
            power_available_w          = 0;
            power_available_filtered_w = 0;
        }

        // CP disconnect support unknown if some chargers haven't replied yet.
        if (!charge_manager.seen_all_chargers()) {
            // Don't constantly complain if we don't have any chargers configured.
            if (charge_manager.get_charger_count() > 0) {
                if (!printed_not_seen_all_chargers) {
                    logger.printfln("power_manager: Not seen all chargers yet.");
                    printed_not_seen_all_chargers = true;
                }
            }

            if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL)
                state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_UNAVAILABLE);

            return;
        } else if (!printed_seen_all_chargers) {
            logger.printfln("power_manager: Seen all chargers.");
            printed_seen_all_chargers = true;
        }

        // Check how many phases are wanted.
        if (phase_switching_mode == PHASE_SWITCHING_ALWAYS_1PHASE) {
            wants_3phase = false;
        } else if (phase_switching_mode == PHASE_SWITCHING_ALWAYS_3PHASE) {
            wants_3phase = true;
        } else if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL) {
            wants_3phase = external_control.get("phases_wanted")->asUint() == 3;
        } else if (phase_switching_mode == PHASE_SWITCHING_PV1P_FAST3P) {
            wants_3phase = mode == MODE_FAST;
        } else { // automatic
            if (is_3phase) {
                wants_3phase = power_available_filtered_w >= threshold_3to1_w;
            } else { // is 1phase
                wants_3phase = power_available_filtered_w > threshold_1to3_w;
            }
        }

        // Need to get the time here instead of using deadline_elapsed(), to avoid stopping the charge when the phase switch deadline check fails but the start/stop deadline check succeeds.
        uint32_t time_now = millis();

        low_level_state.get("power_available")->updateInt(power_available_w);
        low_level_state.get("power_available_filtered")->updateInt(power_available_filtered_w);
        low_level_state.get("wants_3phase")->updateBool(wants_3phase);
        low_level_state.get("wants_3phase_last")->updateBool(wants_3phase_last);
        low_level_state.get("is_on_last")->updateBool(is_on_last);
        low_level_state.get("wants_on_last")->updateBool(wants_on_last);

        // Remember last decision change to start hysteresis time.
        if (wants_3phase != wants_3phase_last) {
            logger.printfln("power_manager: wants_3phase decision changed to %i", wants_3phase);
            phase_state_change_blocked_until = time_now + switching_hysteresis_ms;
            wants_3phase_last = wants_3phase;
        }

        bool phase_state_change_is_blocked = a_after_b(phase_state_change_blocked_until, time_now);
        bool on_state_change_is_blocked = a_after_b(on_state_change_blocked_until, time_now);
        low_level_state.get("phase_state_change_blocked")->updateBool(phase_state_change_is_blocked);
        low_level_state.get("phase_state_change_delay")->updateUint(phase_state_change_is_blocked ? phase_state_change_blocked_until - time_now : 0);
        low_level_state.get("on_state_change_blocked")->updateBool(on_state_change_is_blocked);
        low_level_state.get("on_state_change_delay")->updateUint(on_state_change_is_blocked ? on_state_change_blocked_until - time_now : 0);

        // Check if phase switching is allowed right now.
        bool switch_phases = false;
        if (wants_3phase != is_3phase) {
            if (!phase_switcher_backend->phase_switching_capable()) {
                logger.printfln("power_manager: Phase switch wanted but not available. Check configuration.");
            } else if (!phase_switcher_backend->can_switch_phases_now(wants_3phase)) {
                // Back-end can't switch to the requested phases at the moment. Try again later.
            } else if (!charge_manager.is_control_pilot_disconnect_supported(time_now - 5000)) {
                logger.printfln("power_manager: Phase switch wanted but not supported by all chargers.");
            } else if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL) {
                // Switching phases is always allowed when under external control.
                switch_phases = true;
            } else if (!uptime_past_hysteresis) {
                // (Re)booted recently. Allow immediate switching.
                logger.printfln("power_manager: Immediate phase switch to %s during start-up period. available (filtered)=%i", wants_3phase ? "3 phases" : "1 phase", power_available_filtered_w);
                switch_phases = true;
                // Only one immediate switch on/off allowed; mark as used.
                uptime_past_hysteresis = true;
                low_level_state.get("uptime_past_hysteresis")->updateBool(uptime_past_hysteresis);
            } else if (just_switched_mode) {
                // Just switched modes. Allow immediate switching.
                logger.printfln("power_manager: Immediate phase switch to %s after changing modes. available (filtered)=%i", wants_3phase ? "3 phases" : "1 phase", power_available_filtered_w);
                switch_phases = true;
            } else if (!is_on && !on_state_change_is_blocked && a_after_b(time_now, phase_state_change_blocked_until - switching_hysteresis_ms/2)) {
                // On/off deadline passed and at least half of the phase switching deadline passed.
                logger.printfln("power_manager: Immediate phase switch to %s while power is off. available (filtered)=%i", wants_3phase ? "3 phases" : "1 phase", power_available_filtered_w);
                switch_phases = true;
            } else if (phase_state_change_is_blocked) {
                //logger.printfln("power_manager: Phase switch wanted but decision changed too recently. Have to wait another %ums.", phase_state_change_blocked_until - time_now);
            } else {
                logger.printfln("power_manager wants phase change to %s: available (filtered)=%i", wants_3phase ? "3 phases" : "1 phase", power_available_filtered_w);
                switch_phases = true;
            }
        }

        if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL) {
            if (charge_manager.is_control_pilot_disconnect_supported(time_now - 5000)) {
                state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_AVAILABLE);
            } else {
                state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_UNAVAILABLE);
            }
        }

        // Switch phases or deal with what's available.
        if (switch_phases) {
            switching_state = SwitchingState::StartSwitching;
        } else {
            // Check against overall minimum power, to avoid wanting to switch off when available power is below 3-phase minimum but switch to 1-phase is possible.
            bool wants_on = power_available_filtered_w >= overall_min_power_w;

#if MODULE_AUTOMATION_AVAILABLE()
            TristateBool automation_power_available = static_cast<TristateBool>(wants_on);
            if (automation_power_available != automation_power_available_last) {
                automation.trigger_action(AutomationTriggerID::PMPowerAvailable, &wants_on, [this](const Config *cfg, void *data) -> bool {return this->action_triggered(cfg, data);});
                automation_power_available_last = automation_power_available;
            }
#endif

            // Remember last decision change to start hysteresis time.
            if (wants_on != wants_on_last) {
                logger.printfln("power_manager: wants_on decision changed to %i", wants_on);
                on_state_change_blocked_until = time_now + switching_hysteresis_ms;
                on_state_change_is_blocked = true; // Set manually because the usual update already ran before the phase switching code.
                wants_on_last = wants_on;
            }

            // Filtered power can be above the threshold while current power is below it.
            // In that case, don't switch yet but also don't change wants_on_last state.
            // Instead, wait for the current power to raise above threshold while wants_on(_last) is still true.
            // The inverse applies to the switch-off period.
            if (is_on) {
                // Power is on. Stay on if currently available power is above threshold, even if filtered power might be below it.
                // Requires both filtered and current available power to be below the minimum limit to switch off.
                if (power_available_w >= overall_min_power_w)
                    wants_on = true;
            } else {
                // Power is off. Stay off if currently available power is below threshold, even if filtered power might be below it.
                // Requires both filtered and current available power to be above the minimum limit to switch on.
                if (power_available_w < overall_min_power_w)
                    wants_on = false;
            }

            uint32_t min_current_now_ma = is_3phase ? min_current_3p_ma : min_current_1p_ma;

            uint32_t current_available_ma;
            if (!wants_on) {
                current_available_ma = 0;
            } else if (power_available_w <= 0) {
                current_available_ma = 0;
            } else {
                current_available_ma = (static_cast<uint32_t>(power_available_w) * 1000) / (230 * have_phases);
            }

            // Check if switching on/off is allowed right now.
            if (wants_on != is_on) {
                if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL) {
                    // Switching on/off is always allowed when under external control.
                } else if (!on_state_change_is_blocked) {
                    // Start/stop allowed
                    logger.printfln("power_manager: Switch %s, power available: %i, current available: %u", wants_on ? "on" : "off", power_available_w, current_available_ma);
                } else if (!uptime_past_hysteresis) {
                    // (Re)booted recently. Allow immediate switching.
                    logger.printfln("power_manager: Immediate switch-%s during start-up period, power available: %i, current available: %u", wants_on ? "on" : "off", power_available_w, current_available_ma);
                    // Only one immediate switch on/off allowed; mark as used.
                    uptime_past_hysteresis = true;
                    low_level_state.get("uptime_past_hysteresis")->updateBool(uptime_past_hysteresis);
                } else if (just_switched_mode) {
                    // Just switched modes. Allow immediate switching.
                    logger.printfln("power_manager: Immediate switch-%s after changing modes, power available: %i, current available: %u", wants_on ? "on" : "off", power_available_w, current_available_ma);
                } else if (just_switched_phases && a_after_b(time_now, on_state_change_blocked_until - switching_hysteresis_ms/2)) {
                    logger.printfln("power_manager: Opportunistic switch-%s, power available: %i, current available: %u", wants_on ? "on" : "off", power_available_w, current_available_ma);
                } else { // Switched too recently
                    //logger.printfln("power_manager: Start/stop wanted but decision changed too recently. Have to wait another %ums.", off_state_change_blocked_until - time_now);
                    if (is_on) { // Is on, needs to stay on at minimum current.
                        current_available_ma = min_current_now_ma;
                    } else { // Is off, needs to stay off.
                        current_available_ma = 0;
                    }
                }
            } else { // Don't want to change.
                if (is_on) {
                    // Power is on and wants on too, need to ensure minimum current because unfiltered power available can be negative.
                    if (current_available_ma < min_current_now_ma) {
                        current_available_ma = min_current_now_ma;
                    }
                }
            }

            // Apply minimum/maximum current limits.
            if (current_available_ma < min_current_now_ma) {
                if (current_available_ma != 0)
                    current_available_ma = min_current_now_ma;
            } else if (current_available_ma > max_current_limited_ma) {
                current_available_ma = max_current_limited_ma;
            }

            set_available_current(current_available_ma);
            just_switched_phases = false;
            just_switched_mode = false;
        }
    } else if (switching_state == SwitchingState::StartSwitching) {
            set_available_phases(wants_3phase ? 3 : 1);

            if (phase_switcher_backend->requires_cp_disconnect()) {
                set_available_current(0);

                switching_state = SwitchingState::Stopping;
                switching_start = millis();
            } else {
                switching_state = SwitchingState::TogglingContactor;
            }
    } else if (switching_state == SwitchingState::Stopping) {
        set_available_current(0);

        if (charge_manager.is_charging_stopped(switching_start)) {
            switching_state = SwitchingState::DisconnectingCP;
            switching_start = millis();
        }
    } else if (switching_state == SwitchingState::DisconnectingCP) {
        charge_manager.set_all_control_pilot_disconnect(true);

        if (charge_manager.are_all_control_pilot_disconnected(switching_start)) {
            switching_state = SwitchingState::TogglingContactor;
            switching_start = 0;
        }
    } else if (switching_state == SwitchingState::TogglingContactor) {
        if (!phase_switcher_backend->switch_phases_3phase(wants_3phase)) {
            logger.printfln("power_manager: Toggling phases failed (3p=%i)", wants_3phase);
        } else {
            switching_state = SwitchingState::WaitUntilSwitched;
        }
    } else if (switching_state == SwitchingState::WaitUntilSwitched) {
        if (phase_switcher_backend->get_phase_switching_state() == PhaseSwitcherBackend::SwitchingState::Ready) {
            if (phase_switcher_backend->get_is_3phase() != wants_3phase) {
                logger.printfln("power_manager: Incorrect number of phases after switching, wanted %u. Trying again.", wants_3phase ? 3u : 1u);
                switching_state = SwitchingState::StartSwitching;
            } else {
                if (phase_switcher_backend->requires_cp_disconnect()) {
                    charge_manager.set_all_control_pilot_disconnect(false);
                }
                switching_state = SwitchingState::Monitoring;

                just_switched_phases = true;
            }
        }
    }

    if (phase_switching_mode == PHASE_SWITCHING_EXTERNAL_CONTROL && switching_state != SwitchingState::Monitoring)
        state.get("external_control")->updateUint(EXTERNAL_CONTROL_STATE_SWITCHING);
}

void PowerManager::limit_max_current(uint32_t limit_ma)
{
    if (max_current_limited_ma > limit_ma)
        max_current_limited_ma = limit_ma;
}

void PowerManager::reset_limit_max_current()
{
    max_current_limited_ma = max_current_unlimited_ma;
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
bool PowerManager::action_triggered(const Config *automation_config, void *data)
{
    const Config *cfg = static_cast<const Config *>(automation_config->get());

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

    switch (automation_config->getTag<AutomationTriggerID>()) {
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
