/* esp32-firmware
 * Copyright (C) 2022 Olaf Lüke <olaf@tinkerforge.com>
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

#include "energy_manager.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "modules.h"
#include "task_scheduler.h"
#include "tools.h"
#include "web_server.h"

void EnergyManager::pre_setup()
{
    // States
    energy_manager_state = Config::Object({
        {"contactor", Config::Bool(false)},
        {"led_rgb", Config::Array({Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)},
            new Config{Config::Uint8(0)}, 3, 3, Config::type_id<Config::ConfUint>())
        },
        {"gpio_input_state", Config::Array({Config::Bool(false), Config::Bool(false)},
            new Config{Config::Bool(false)}, 2, 2, Config::type_id<Config::ConfBool>())
        },
        {"gpio_output_state", Config::Bool(false)},
        {"input_voltage", Config::Uint16(0)},
        {"contactor_check_state", Config::Uint8(0)},
        {"energy_meter_type", Config::Uint8(0)},
        {"energy_meter_power", Config::Float(0)}, // watt
        {"energy_meter_energy_import", Config::Float(0)}, // kWh
        {"energy_meter_energy_export", Config::Float(0)}, // kWh
        // Derived states
        {"phases_switched", Config::Uint8(0)},
        {"contactor_check_tripped", Config::Bool(false)},
    });

    // Config
    energy_manager_config = ConfigRoot(Config::Object({
        {"excess_charging_enable", Config::Bool(false)},
        {"contactor_installed", Config::Bool(false)},
        {"phase_switching_mode", Config::Uint8(PHASE_SWITCHING_AUTOMATIC)},
        {"target_power_from_grid", Config::Int32(0)}, // in watt
        {"guaranteed_power", Config::Uint(0, 0, 22000)}, // in watt
        {"maximum_available_current", Config::Uint32(0)}, // Keep in sync with charge_manager.cpp
        {"minimum_current", Config::Uint(6000, 6000, 32000)}, // Keep in sync with charge_manager.cpp
        {"hysteresis_time", Config::Uint(HYSTERESIS_MIN_TIME_MINUTES, 0, 60)}, // in minutes
        {"hysteresis_wear_accepted", Config::Bool(false)},
        {"relay_config", Config::Uint8(0)},
        {"relay_config_when", Config::Uint8(0)},
        {"relay_config_is", Config::Uint8(0)},
        {"input3_config", Config::Uint8(0)},
        {"input3_config_limit", Config::Int32(0)},
        {"input3_config_when", Config::Uint8(0)},
        {"input4_config", Config::Uint8(0)},
        {"input4_config_limit", Config::Int32(0)},
        {"input4_config_when", Config::Uint8(0)},
    }), [](const Config &cfg) -> String {
        uint32_t switching_hysteresis_min = cfg.get("hysteresis_time")->asUint(); // minutes
        uint32_t hysteresis_wear_ok       = cfg.get("hysteresis_wear_accepted")->asBool();

        if (switching_hysteresis_min < HYSTERESIS_MIN_TIME_MINUTES && !hysteresis_wear_ok)
            return "Switching hysteresis time cannot be shorter than " MACRO_VALUE_TO_STRING(HYSTERESIS_MIN_TIME_MINUTES) " minutes without accepting additional wear.";

        uint32_t input3_config = cfg.get("input3_config")->asUint();
        uint32_t input4_config = cfg.get("input4_config")->asUint();

        if (input3_config == input4_config) {
            if (input3_config == INPUT_CONFIG_EXCESS_CHARGING)
                return "Cannot configure both input 3 and input 4 to switch excess charging.";
        }

        return "";
    });

    switching_state = SwitchingState::Monitoring;
}

void EnergyManager::apply_defaults()
{
    // TODO: Configure Energy Manager
}

void EnergyManager::setup_energy_manager()
{
    if (!this->DeviceModule::setup_device()) {
        logger.printfln("energy_manager: setup_device error. Reboot in 5 Minutes.");

        task_scheduler.scheduleOnce([](){
            trigger_reboot("Energy Manager");
        }, 5 * 60 * 1000);
        return;
    }

    this->apply_defaults();
    initialized = true;
}

void EnergyManager::setup()
{
    setup_energy_manager();
    if (!device_found)
        return;

    // Forgets all settings when new setting is introduced: "Failed to restore persistent config energy_manager_config: JSON object is missing key 'input3_config_limit'\nJSON object is missing key 'input4_config_limit'"
    api.restorePersistentConfig("energy_manager/config", &energy_manager_config);
    energy_manager_config_in_use = energy_manager_config;

    if ((energy_manager_config_in_use.get("phase_switching_mode")->asUint() == PHASE_SWITCHING_AUTOMATIC) && !energy_manager_config_in_use.get("contactor_installed")->asBool()) {
        logger.printfln("energy_manager: Invalid configuration: Automatic phase switching selected but no contactor installed.");
        return;
    }

#if MODULE_CHARGE_MANAGER_AVAILABLE()
    charge_manager.set_allocated_current_callback([this](uint32_t current_ma){
        //logger.printfln("energy_manager: allocated current callback: %u", current_ma);
        charge_manager_allocated_current_ma = current_ma;
    });
#endif

    // Set up output relay and input pins
    output = new OutputRelay(energy_manager_config_in_use);
    input3 = new InputPin(3, 0, energy_manager_config_in_use);
    input4 = new InputPin(4, 1, energy_manager_config_in_use);

    // Cache config for energy update
    excess_charging_enable          = energy_manager_config_in_use.get("excess_charging_enable")->asBool();
    target_power_from_grid_conf_w   = energy_manager_config_in_use.get("target_power_from_grid")->asInt();          // watt
    guaranteed_power_conf_w         = energy_manager_config_in_use.get("guaranteed_power")->asUint();               // watt
    max_current_unlimited_ma        = energy_manager_config_in_use.get("maximum_available_current")->asUint();      // milliampere
    min_current_ma                  = energy_manager_config_in_use.get("minimum_current")->asUint();                // milliampere
    contactor_installed             = energy_manager_config_in_use.get("contactor_installed")->asBool();
    phase_switching_mode            = energy_manager_config_in_use.get("phase_switching_mode")->asUint();
    switching_hysteresis_ms         = energy_manager_config_in_use.get("hysteresis_time")->asUint() * 60 * 1000;    // milliseconds (from minutes)
    hysteresis_wear_ok              = energy_manager_config_in_use.get("hysteresis_wear_accepted")->asBool();

    // If the user accepts the additional wear, the minimum hysteresis time is 10s. Less than that will cause the control algorithm to oscillate.
    uint32_t hysteresis_min_ms = hysteresis_wear_ok ? 10 * 1000 : HYSTERESIS_MIN_TIME_MINUTES * 60 * 1000;  // milliseconds
    if (switching_hysteresis_ms < hysteresis_min_ms)
        switching_hysteresis_ms = hysteresis_min_ms;

    // Pre-calculate various limits
    int32_t min_phases;
    if (phase_switching_mode == PHASE_SWITCHING_ALWAYS_1PHASE) {
        min_phases = 1;
    } else if (phase_switching_mode == PHASE_SWITCHING_ALWAYS_3PHASE) {
        min_phases = 3;
    } else { // automatic
        min_phases = 1;
    }
    overall_min_power_w = 230 * min_phases * min_current_ma / 1000;

    const int32_t max_1phase_w = 230 * 1 * max_current_unlimited_ma / 1000;
    const int32_t min_3phase_w = 230 * 3 * min_current_ma / 1000;

    if (min_3phase_w > max_1phase_w) { // have dead current range
        int32_t range_width = min_3phase_w - max_1phase_w;
        threshold_3to1_w = max_1phase_w + static_cast<int32_t>(0.25 * range_width);
        threshold_1to3_w = max_1phase_w + static_cast<int32_t>(0.75 * range_width);
    } else { // no dead current range, use simple limits
        threshold_3to1_w = min_3phase_w;
        threshold_1to3_w = max_1phase_w;
    }

    api.addFeature("energy_manager");

    task_scheduler.scheduleWithFixedDelay([this](){
        this->update_all_data();
    }, 0, 250);

    task_scheduler.scheduleWithFixedDelay([this](){
        this->update_io();
    }, 250, 250);

    if (max_current_unlimited_ma == 0) {
        logger.printfln("energy_manager: No maximum current configured. Disabling energy distribution.");
        return;
    }

#if !MODULE_CHARGE_MANAGER_AVAILABLE()
    logger.printfln("energy_manager: Module 'Charge Manager' not available. Disabling energy distribution.");
    return;
#endif

    task_scheduler.scheduleWithFixedDelay([this](){
        this->update_energy();
    }, 250, 250);

    task_scheduler.scheduleOnce([this](){
        uptime_past_hysteresis = true;
    }, switching_hysteresis_ms);
}

void EnergyManager::register_urls()
{
    if (!device_found)
        return;

#if MODULE_WS_AVAILABLE()
    server.on("/energy_manager/start_debug", HTTP_GET, [this](WebServerRequest request) {
        task_scheduler.scheduleOnce([this](){
            ws.pushRawStateUpdate(this->get_energy_manager_debug_header(), "energy_manager/debug_header");
            debug = true;
        }, 0);
        return request.send(200);
    });

    server.on("/energy_manager/stop_debug", HTTP_GET, [this](WebServerRequest request){
        task_scheduler.scheduleOnce([this](){
            debug = false;
        }, 0);
        return request.send(200);
    });
#endif

    api.addPersistentConfig("energy_manager/config", &energy_manager_config, {}, 1000);
    api.addState("energy_manager/state", &energy_manager_state, {}, 1000);

    this->DeviceModule::register_urls();
}

void EnergyManager::loop()
{
    this->DeviceModule::loop();

#if MODULE_WS_AVAILABLE()
    static uint32_t last_debug = 0;
    if (debug && deadline_elapsed(last_debug + 50)) {
        last_debug = millis();
        ws.pushRawStateUpdate(this->get_energy_manager_debug_line(), "energy_manager/debug");
    }
#endif
}

void EnergyManager::update_all_data()
{
    uint32_t time = micros();

    update_all_data_struct();

    energy_manager_state.get("contactor")->updateBool(all_data.contactor_value);
    energy_manager_state.get("led_rgb")->get(0)->updateUint(all_data.rgb_value_r);
    energy_manager_state.get("led_rgb")->get(1)->updateUint(all_data.rgb_value_g);
    energy_manager_state.get("led_rgb")->get(2)->updateUint(all_data.rgb_value_b);
    energy_manager_state.get("gpio_input_state")->get(0)->updateBool(all_data.input[0]);
    energy_manager_state.get("gpio_input_state")->get(1)->updateBool(all_data.input[1]);
    energy_manager_state.get("gpio_output_state")->updateBool(all_data.output);
    energy_manager_state.get("input_voltage")->updateUint(all_data.voltage);
    energy_manager_state.get("contactor_check_state")->updateUint(all_data.contactor_check_state);

    if (all_data.energy_meter_type != METER_TYPE_NONE) {
        energy_manager_state.get("energy_meter_type")->updateUint(all_data.energy_meter_type);
        energy_manager_state.get("energy_meter_power")->updateFloat(all_data.power);
        energy_manager_state.get("energy_meter_energy_import")->updateFloat(all_data.energy_import);
        energy_manager_state.get("energy_meter_energy_export")->updateFloat(all_data.energy_export);
    }

    // Update states derived from all_data
    is_3phase   = contactor_installed ? all_data.contactor_value : phase_switching_mode == PHASE_SWITCHING_ALWAYS_3PHASE;
    have_phases = 1 + is_3phase * 2;
    energy_manager_state.get("phases_switched")->updateUint(have_phases);

    power_at_meter_w = all_data.energy_meter_type ? all_data.power : meter.values.get("power")->asFloat(); // watt

    if (contactor_installed) {
        if ((all_data.contactor_check_state & 1) == 0) {
            logger.printfln("Contactor check tripped. Check contactor.");
            contactor_check_tripped = true;
            energy_manager_state.get("contactor_check_tripped")->updateBool(true);
        } else if (contactor_check_tripped) {
            logger.printfln("Contactor check tripped in the past but reports ok now. Check contactor and reboot Energy Manager to clear.");
        }
    }

    static uint32_t time_max = 2000;
    time = micros() - time;
    if (time > time_max) {
        time_max = time;
        logger.printfln("energy_manager::update_all_data() took %uus", time_max);
    }
}

void EnergyManager::update_all_data_struct()
{
    int rc = tf_warp_energy_manager_get_all_data_1(
        &device,
        &all_data.contactor_value,
        &all_data.rgb_value_r,
        &all_data.rgb_value_g,
        &all_data.rgb_value_b,
        &all_data.power,
        &all_data.energy_import,
        &all_data.energy_export,
        &all_data.energy_meter_type,
        all_data.error_count,
        all_data.input,
        &all_data.output,
        &all_data.voltage,
        &all_data.contactor_check_state
    );

    if (rc != TF_E_OK) {
        logger.printfln("get_all_data_1 error %d", rc);
    }
}

void EnergyManager::update_io()
{
    uint32_t time = micros();

    output->update();

    // Oversampling inputs is currently not used because all of the implemented input pin functions require update_energy() to run anyway.
    //// We "over-sample" the two inputs compared to the other data in the all_data struct
    //// to make sure that we can always react in a timely manner to input changes
    //int rc = tf_warp_energy_manager_get_input(&device, all_data.input);
    //if (rc != TF_E_OK) {
    //    logger.printfln("get_input error %d", rc);
    //}

    // Restore values that can be changed by input pins.
    max_current_limited_ma      = max_current_unlimited_ma;
    target_power_from_grid_w    = target_power_from_grid_conf_w;
    guaranteed_power_w          = guaranteed_power_conf_w;

    input3->update(all_data.input[0]);
    input4->update(all_data.input[1]);

    static uint32_t time_max = 2000;
    time = micros() - time;
    if (time > time_max) {
        time_max = time;
        logger.printfln("energy_manager::update_io() took %uus", time_max);
    }
}

void EnergyManager::limit_max_current(uint32_t limit_ma)
{
    if (max_current_limited_ma > limit_ma)
        max_current_limited_ma = limit_ma;
}

void EnergyManager::override_grid_draw(int32_t limit_w)
{
    target_power_from_grid_w = limit_w;
}

void EnergyManager::override_guaranteed_power(uint32_t power_w)
{
    guaranteed_power_w = power_w;
}

void EnergyManager::set_available_current(uint32_t current)
{
    is_on_last = current > 0;
#if MODULE_CHARGE_MANAGER_AVAILABLE()
    charge_manager.set_available_current(current);
#endif
}

void EnergyManager::update_energy()
{
#if !MODULE_CHARGE_MANAGER_AVAILABLE()
    logger.printfln("energy_manager: Module 'Charge Manager' not available. update_energy() does nothing.");
#else
    uint32_t time = micros();

    static SwitchingState prev_state = switching_state;
    if (switching_state != prev_state) {
        logger.printfln("energy_manager: now in state %u", switching_state);
        prev_state = switching_state;
    }

    if (contactor_check_tripped) {
        set_available_current(0);
        return;
    }

    if (switching_state == SwitchingState::Monitoring) {
        const bool     is_on = is_on_last;
        const uint32_t charge_manager_allocated_power_w = 230 * have_phases * charge_manager_allocated_current_ma / 1000; // watt

        if (charging_blocked.combined) {
            if (is_on) {
                phase_state_change_blocked_until = on_state_change_blocked_until = millis() + switching_hysteresis_ms;
            }
            set_available_current(0);
            just_switched_phases = false;
            return;
        }

        // Evil: Allow runtime changes, overrides input pins!
        excess_charging_enable      = energy_manager_config.get("excess_charging_enable")->asBool();
        target_power_from_grid_w    = energy_manager_config.get("target_power_from_grid")->asInt(); // watt

        int32_t power_available_w; // watt
        if (!excess_charging_enable) {
            power_available_w = 230 * 3 * max_current_limited_ma / 1000;
        } else {
            // Excess charging enabled; use a simple P controller to adjust available power.
            int32_t p_error_w  = target_power_from_grid_w - power_at_meter_w;

            int32_t p_adjust_w;
            if (!is_on) {
                // When the power is not on, use p=1 so that the switch-on threshold can be reached properly.
                p_adjust_w = p_error_w;
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

            power_available_w  = static_cast<int32_t>(charge_manager_allocated_power_w) + p_adjust_w;
        }

        if (power_available_w < static_cast<int32_t>(guaranteed_power_w))
            power_available_w = static_cast<int32_t>(guaranteed_power_w);

        // CP disconnect support unknown if some chargers haven't replied yet.
        if (!charge_manager.seen_all_chargers()) {
            logger.printfln("energy_manager: Not seen all chargers yet.");
            return;
        }

        // Check how many phases are wanted.
        if (phase_switching_mode == PHASE_SWITCHING_ALWAYS_1PHASE) {
            wants_3phase = false;
        } else if (phase_switching_mode == PHASE_SWITCHING_ALWAYS_3PHASE) {
            wants_3phase = true;
        } else { // automatic
            if (is_3phase) {
                wants_3phase = power_available_w >= threshold_3to1_w;
            } else { // is 1phase
                wants_3phase = power_available_w > threshold_1to3_w;
            }
        }

        // Need to get the time here instead of using deadline_elapsed(), to avoid stopping the charge when the phase switch deadline check fails but the start/stop deadline check succeeds.
        uint32_t time_now = millis();

        // Remember last decision change to start hysteresis time.
        if (wants_3phase != wants_3phase_last) {
            logger.printfln("energy_manager: wants_3phase decision changed to %i", wants_3phase);
            phase_state_change_blocked_until = time_now + switching_hysteresis_ms;
            wants_3phase_last = wants_3phase;
        }

        // Check if phase switching is allowed right now.
        bool switch_phases = false;
        if (wants_3phase != is_3phase) {
            if (!contactor_installed) {
                logger.printfln("energy_manager: Phase switch wanted but no contactor installed. Check configuration.");
            } else if (!charge_manager.is_control_pilot_disconnect_supported(time_now - 5000)) {
                logger.printfln("energy_manager: Phase switch wanted but not supported by all chargers.");
            } else if (!uptime_past_hysteresis) {
                // (Re)booted recently. Allow immediate switching.
                logger.printfln("energy_manager: Free phase switch to %s during start-up period. available=%i", wants_3phase ? "3 phases" : "1 phase", power_available_w);
                switch_phases = true;
            } else if (!is_on && a_after_b(time_now, on_state_change_blocked_until) && a_after_b(time_now, phase_state_change_blocked_until - switching_hysteresis_ms/2)) {
                // On/off deadline passed and at least half of the phase switching deadline passed.
                logger.printfln("energy_manager: Free phase switch to %s while power is off. available=%i", wants_3phase ? "3 phases" : "1 phase", power_available_w);
                switch_phases = true;
            } else if (!a_after_b(time_now, phase_state_change_blocked_until)) {
                //logger.printfln("energy_manager: Phase switch wanted but decision changed too recently. Have to wait another %ums.", phase_state_change_blocked_until - time_now);
            } else {
                logger.printfln("energy_manager wants phase change to %s: available=%i", wants_3phase ? "3 phases" : "1 phase", power_available_w);
                switch_phases = true;
            }
        }

        // Switch phases or deal with what's available.
        if (switch_phases) {
            set_available_current(0);
            switching_state = SwitchingState::Stopping;
            switching_start = time_now;
        } else {
            // Check against overall minimum power, to avoid wanting to switch off when available power is below 3-phase minimum but switch to 1-phase is possible.
            bool wants_on = power_available_w >= overall_min_power_w;

            // Remember last decision change to start hysteresis time.
            if (wants_on != wants_on_last) {
                logger.printfln("energy_manager: wants_on decision changed to %i", wants_on);
                on_state_change_blocked_until = time_now + switching_hysteresis_ms;
                wants_on_last = wants_on;
            }

            uint32_t current_available_ma;
            if (power_available_w <= 0)
                current_available_ma = 0;
            else
                current_available_ma = (power_available_w * 1000) / (230 * have_phases) * wants_on;

            // Check if switching on/off is allowed right now.
            if (wants_on != is_on) {
                if (a_after_b(time_now, on_state_change_blocked_until)) {
                    // Start/stop allowed
                    logger.printfln("energy_manager: Switch %s", wants_on ? "on" : "off");
                } else if (!uptime_past_hysteresis) {
                    // (Re)booted recently. Allow immediate switching.
                    logger.printfln("energy_manager: Free switch-%s during start-up period.", wants_on ? "on" : "off");
                    // Only one immediate switch on/off allowed; mark as used.
                    uptime_past_hysteresis = true;
                } else if (just_switched_phases && a_after_b(time_now, on_state_change_blocked_until - switching_hysteresis_ms/2)) {
                    logger.printfln("energy_manager: Opportunistic switch-%s", wants_on ? "on" : "off");
                } else { // Switched too recently
                    //logger.printfln("energy_manager: Start/stop wanted but decision changed too recently. Have to wait another %ums.", off_state_change_blocked_until - time_now);
                    if (is_on) { // Is on, needs to stay on at minimum current.
                        current_available_ma = min_current_ma;
                    } else { // Is off, needs to stay off.
                        current_available_ma = 0;
                    }
                }
            }

            // Apply minimum/maximum current limits.
            if (current_available_ma < min_current_ma) {
                if (current_available_ma != 0)
                    current_available_ma = min_current_ma;
            } else if (current_available_ma > max_current_limited_ma) {
                current_available_ma = max_current_limited_ma;
            }

            set_available_current(current_available_ma);
            just_switched_phases = false;
        }

        const uint32_t print_every = 8;
        if (print_every > 0) {
            static uint32_t last_print = 0;
            last_print = (last_print + 1) % print_every;
            if (last_print == 0)
                logger.printfln("power_at_meter_w %i | target_power_from_grid_w %i | power_available_w %i | wants_3phase %i | is_3phase %i | is_on %i | max_current_limited_ma %u | cm avail ma %u | cm alloc ma %u",
                    power_at_meter_w, target_power_from_grid_w, power_available_w, wants_3phase, is_3phase, is_on, max_current_limited_ma, charge_manager.charge_manager_available_current.get("current")->asUint(), charge_manager_allocated_current_ma);
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
            switching_start = millis();
        }
    } else if (switching_state == SwitchingState::TogglingContactor) {
        tf_warp_energy_manager_set_contactor(&device, wants_3phase);

        if (all_data.contactor_value == wants_3phase) {
            switching_state = SwitchingState::ConnectingCP;
            switching_start = millis();
        }
    } else if (switching_state == SwitchingState::ConnectingCP) {
        charge_manager.set_all_control_pilot_disconnect(false);

        switching_state = SwitchingState::Monitoring;
        switching_start = 0;

        just_switched_phases = true;
    }

    static uint32_t time_max = 20000;
    time = micros() - time;
    if (time > time_max) {
        time_max = time;
        logger.printfln("energy_manager::update_energy() took %uus", time_max);
    }
#endif
}

uint16_t EnergyManager::get_energy_meter_detailed_values(float *ret_values)
{
    uint16_t len = 0;
    tf_warp_energy_manager_get_energy_meter_detailed_values(&device, ret_values, &len);
    return len;
}

void EnergyManager::set_output(bool output)
{
    int result = tf_warp_energy_manager_set_output(&device, output);
    if (result != TF_E_OK)
        logger.printfln("energy_manager: Failed to set output relay: error %i", result);
}

String EnergyManager::get_energy_manager_debug_header()
{
    return "\"millis,,"
           "contactor_value,,"
           "rgb_value_r,"
           "rgb_value_g,"
           "rgb_value_b,,"
           "power,"
           "energy_import,"
           "energy_export,,"
           "energy_meter_type,"
           "error_count[0],"
           "error_count[1],"
           "error_count[2],"
           "error_count[3],"
           "error_count[4],"
           "error_count[5],,"
           "input3,"
           "input4,,"
           "output,,"
           "input_voltage,,"
           "contactor_check_state,"
           "\"";
}

String EnergyManager::get_energy_manager_debug_line()
{
    update_all_data_struct();

    char line[512] = {0};
    snprintf(line,
             sizeof(line) / sizeof(line[0]),
             "\"%lu,,"
             "%u,,"
             "%u,%u,%u,,"
             "%.3f,%.3f,%.3f,,"
             "%u,%u,%u,%u,%u,%u,%u,,"
             "%c,%c,,"
             "%u,,"
             "%u,,"
             "%u\"",
             millis(),
             all_data.contactor_value,

             all_data.rgb_value_r,
             all_data.rgb_value_g,
             all_data.rgb_value_b,

             all_data.power,
             all_data.energy_import,
             all_data.energy_export,

             all_data.energy_meter_type,
             all_data.error_count[0],
             all_data.error_count[1],
             all_data.error_count[2],
             all_data.error_count[3],
             all_data.error_count[4],
             all_data.error_count[5],

             all_data.input[0] ? '1' : '0',
             all_data.input[1] ? '1' : '0',

             all_data.output,
             all_data.voltage,
             all_data.contactor_check_state);

    return String(line);
}
