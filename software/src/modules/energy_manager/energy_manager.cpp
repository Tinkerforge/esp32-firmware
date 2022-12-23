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
#include "task_scheduler.h"
#include "tools.h"
#include "web_server.h"
#include "modules.h"

extern EventLog logger;

extern TaskScheduler task_scheduler;
extern WebServer server;

extern API api;

const char* ENERGY_MANAGER_INPUT_CONFIG_STR[]      = {"input3_config",      "input4_config"};
const char* ENERGY_MANAGER_INPUT_CONFIG_IF_STR[]   = {"input3_config_if",   "input4_config_if"};
const char* ENERGY_MANAGER_INPUT_CONFIG_THEN_STR[] = {"input3_config_then", "input4_config_then"};

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
        {"energy_meter_power", Config::Float(0)},
        {"energy_meter_energy_rel", Config::Float(0)},
        {"energy_meter_energy_abs", Config::Float(0)},
        {"energy_meter_phases_active", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            new Config{Config::Bool(false)}, 3, 3, Config::type_id<Config::ConfBool>())
        },
        {"energy_meter_phases_connected", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            new Config{Config::Bool(false)},3, 3, Config::type_id<Config::ConfBool>())
        }
    });

    // Config
    energy_manager_config = Config::Object({
        {"excess_charging_enable", Config::Bool(false)},
        {"contactor_installed", Config::Bool(true)},
        {"phase_switching_mode", Config::Uint8(PHASE_SWITCHING_AUTOMATIC)},
        {"maximum_power_from_grid", Config::Int32(0)},
        {"maximum_available_current", Config::Uint32(0)}, // Keep in sync with charge_manager.cpp
        {"minimum_current", Config::Uint(6000, 6000, 32000)}, // Keep in sync with charge_manager.cpp
        {"relay_config", Config::Uint8(0)},
        {"relay_config_if", Config::Uint8(0)},
        {"relay_config_is", Config::Uint8(0)},
        {"relay_config_then", Config::Uint8(0)},
        {"input3_config", Config::Uint8(0)},
        {"input3_config_if", Config::Uint8(0)},
        {"input3_config_then", Config::Uint8(0)},
        {"input4_config", Config::Uint8(0)},
        {"input4_config_if", Config::Uint8(0)},
        {"input4_config_then", Config::Uint8(0)}
    });

    switching_state = SwitchingState_Monitoring;
}

void EnergyManager::apply_defaults()
{
    // TODO: Configure Energy Manager
}

void EnergyManager::handle_relay_config_if_input(uint8_t input)
{
    if (input > 1) {
        logger.printfln("Unknown handle_relay_config_if input: %u", input);
        return;
    }

    // Check if condition is satisfied and set relay according to configuration
    uint8_t relay_config_is = energy_manager_config_in_use.get("relay_config_is")->asUint();
    uint8_t relay_config_then = energy_manager_config_in_use.get("relay_config_then")->asUint();
    if (((relay_config_is == RELAY_CONFIG_IS_HIGH) && (all_data.input[input])) ||
        ((relay_config_is == RELAY_CONFIG_IS_LOW) && (!all_data.input[input]))) {
        tf_warp_energy_manager_set_output(&device, relay_config_then == RELAY_CONFIG_THEN_CLOSED);
    } else {
        tf_warp_energy_manager_set_output(&device, relay_config_then != RELAY_CONFIG_THEN_CLOSED);
    }
}

void EnergyManager::handle_relay_config_if_phase_switching()
{
    // TODO
}

void EnergyManager::handle_relay_config_if_meter()
{
    // TODO
}

void EnergyManager::handle_input_config_rule_based(uint8_t input)
{
    bool allowed = true;
    uint8_t input_config_if = energy_manager_config_in_use.get(ENERGY_MANAGER_INPUT_CONFIG_IF_STR[input])->asUint();
    uint8_t input_config_then = energy_manager_config_in_use.get(ENERGY_MANAGER_INPUT_CONFIG_THEN_STR[input])->asUint();
    if (((input_config_if == INPUT_CONFIG_IF_HIGH) && (all_data.input[input])) ||
        ((input_config_if == INPUT_CONFIG_IF_LOW) && (!all_data.input[input]))) {
        allowed = (input_config_then == INPUT_CONFIG_THEN_ALLOW);
    } else {
        allowed = (input_config_then != INPUT_CONFIG_THEN_ALLOW);
    }

    input_charging_allowed[input] = allowed;
}

void EnergyManager::handle_input_config_contactor_check(uint8_t input)
{
    // TODO
}

void EnergyManager::update_io()
{
    uint32_t time = micros();

    // Handle relay
    uint8_t relay_config = energy_manager_config_in_use.get("relay_config")->asUint();
    if (relay_config == RELAY_CONFIG_RULE_BASED) {
        uint8_t relay_config_if = energy_manager_config_in_use.get("relay_config_if")->asUint();
        switch(relay_config_if) {
            case RELAY_CONFIG_IF_INPUT3:          handle_relay_config_if_input(0);          break;
            case RELAY_CONFIG_IF_INPUT4:          handle_relay_config_if_input(1);          break;
            case RELAY_CONFIG_IF_PHASE_SWITCHING: handle_relay_config_if_phase_switching(); break;
            case RELAY_CONFIG_IF_METER:           handle_relay_config_if_meter();           break;
            default: logger.printfln("Unknown RELAY_CONFIG_IF: %u", relay_config_if);       break;
        }
    }

    // We "over-sample" the two inputs compared to the other data in the all_data struct
    // to make sure that we can always react in a timely manner to input changes
    int rc = tf_warp_energy_manager_get_input(&device, all_data.input);
    if (rc != TF_E_OK) {
        logger.printfln("get_input error %d", rc);
    }

    // Handle input3 and input4
    for (uint8_t input = 0; input < 2; input++) {
        uint8_t input_config = energy_manager_config_in_use.get(ENERGY_MANAGER_INPUT_CONFIG_STR[input])->asUint();
        switch(input_config) {
            case INPUT_CONFIG_DEACTIVATED:                                                          break;
            case INPUT_CONFIG_RULES_BASED:     handle_input_config_rule_based(input);               break;
            case INPUT_CONFIG_CONTACTOR_CHECK: handle_input_config_contactor_check(input);          break;
            default: logger.printfln("Unknown INPUT_CONFIG: %u for input %u", input_config, input); break;
        }
    }

    static uint32_t time_max = 0;
    time = micros() - time;
    if (time > time_max) {
        time_max = time;
        logger.printfln("energy_manager::update_io() took %uus", time_max);
    }
}

void EnergyManager::update_energy()
{
    uint32_t time = micros();

    if (switching_state == SwitchingState_Monitoring) {
        const int32_t  power_at_meter_w = all_data.power * 1000; // watt
        const bool     is_3phase        = contactor_installed ? all_data.contactor_value : phase_switching_mode == PHASE_SWITCHING_ALWAYS_3PHASE;
        const uint32_t have_phases      = 1 + is_3phase * 2;
        const bool     was_on           = charge_manager_allocated_current_ma != 0;

        const uint32_t charge_manager_allocated_power_w = 230 * have_phases * charge_manager_allocated_current_ma / 1000; // watt
        const int32_t  power_available_w = excess_charging_enable ? charge_manager_allocated_power_w + max_power_from_grid_w - power_at_meter_w : 230 * 3 * max_current_ma / 1000; // watt

        if (!input_charging_allowed[0] || !input_charging_allowed[1]) {
            if (was_on) {
                phase_state_change_blocked_until = on_state_change_blocked_until = millis() + switching_hysteresis_ms;
            }
            set_available_current(0);
            just_switched_phases = false;
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
            } else if (!was_on && a_after_b(time_now, on_state_change_blocked_until) && a_after_b(time_now, phase_state_change_blocked_until - switching_hysteresis_ms/2)) {
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
            switching_state = SwitchingState_Stopping;
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
            if (wants_on != was_on) {
                if (a_after_b(time_now, on_state_change_blocked_until)) {
                    // Start/stop allowed
                    logger.printfln("energy_manager: Switch %s", wants_on ? "on" : "off");
                } else if (just_switched_phases && a_after_b(time_now, on_state_change_blocked_until - switching_hysteresis_ms/2)) {
                    logger.printfln("energy_manager: Opportunistic switch %s", wants_on ? "on" : "off");
                } else { // Switched too recently
                    //logger.printfln("energy_manager: Start/stop wanted but decision changed too recently. Have to wait another %ums.", off_state_change_blocked_until - time_now);
                    if (was_on) { // Was on, needs to stay on at minimum current.
                        current_available_ma = min_current_ma;
                    } else { // Was off, needs to stay off.
                        current_available_ma = 0;
                    }
                }
            }

            // Apply minimum/maximum current limits.
            if (current_available_ma < min_current_ma) {
                if (current_available_ma != 0)
                    current_available_ma = min_current_ma;
            } else if (current_available_ma > max_current_ma) {
                current_available_ma = max_current_ma;
            }

            set_available_current(current_available_ma);
            just_switched_phases = false;
        }

        //static uint32_t last_print = 0;
        //last_print = (last_print + 1) % 1;
        //if (last_print == 0)
        //    logger.printfln("power_at_meter_w %i | max_power_from_grid_w %i | power_available_w %i | wants_3phase %i | is_3phase %i | last_current_available_ma %i",
        //        power_at_meter_w, max_power_from_grid_w, power_available_w, wants_3phase, is_3phase, last_current_available_ma);
    } else if (switching_state == SwitchingState_Stopping) {
        set_available_current(0);

        if (charge_manager.is_charging_stopped(switching_start)) {
            switching_state = SwitchingState_DisconnectingCP;
            switching_start = millis();
        }
    } else if (switching_state == SwitchingState_DisconnectingCP) {
        charge_manager.set_all_control_pilot_disconnect(true);

        if (charge_manager.are_all_control_pilot_disconnected(switching_start)) {
            switching_state = SwitchingState_TogglingContactor;
            switching_start = millis();
        }
    } else if (switching_state == SwitchingState_TogglingContactor) {
        tf_warp_energy_manager_set_contactor(&device, wants_3phase);

        if (all_data.contactor_check_state == (wants_3phase ? 1 : 0)) {
            switching_state = SwitchingState_ConnectingCP;
            switching_start = millis();
        }
    } else if (switching_state == SwitchingState_ConnectingCP) {
        charge_manager.set_all_control_pilot_disconnect(false);

        switching_state = SwitchingState_Monitoring;
        switching_start = 0;

        just_switched_phases = true;
    }

    static uint32_t time_max = 0;
    time = micros() - time;
    if (time > time_max) {
        time_max = time;
        logger.printfln("energy_manager::update_energy() took %uus", time_max);
    }
}

void EnergyManager::set_available_current(uint32_t current_ma) {
    charge_manager.set_available_current(current_ma);
    last_current_available_ma = current_ma;
}

void EnergyManager::setup()
{
    setup_energy_manager();
    if (!device_found)
        return;

    api.restorePersistentConfig("energy_manager/config", &energy_manager_config);
    energy_manager_config_in_use = energy_manager_config;

    if ((energy_manager_config_in_use.get("phase_switching_mode")->asUint() == PHASE_SWITCHING_AUTOMATIC) && !energy_manager_config_in_use.get("contactor_installed")->asBool()) {
        logger.printfln("energy_manager: Invalid configuration: Automatic phase switching selected but no contactor installed.");
        return;
    }

    charge_manager.set_allocated_current_callback([this](uint32_t current_ma){
        //logger.printfln("energy_manager: allocated current callback: %u", current_ma);
        charge_manager_allocated_current_ma = current_ma;
    });

    // Cache config for energy update
    max_power_from_grid_w   = energy_manager_config_in_use.get("maximum_power_from_grid")->asInt();     // watt
    max_current_ma          = energy_manager_config_in_use.get("maximum_available_current")->asUint();  // milliampere
    min_current_ma          = energy_manager_config_in_use.get("minimum_current")->asUint();            // milliampere
    excess_charging_enable  = energy_manager_config_in_use.get("excess_charging_enable")->asBool();
    contactor_installed     = energy_manager_config_in_use.get("contactor_installed")->asBool();
    phase_switching_mode    = energy_manager_config_in_use.get("phase_switching_mode")->asUint();
    switching_hysteresis_ms = 10 * 1000; // milliseconds

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

    const int32_t max_1phase_w = 230 * 1 * max_current_ma / 1000;
    const int32_t min_3phase_w = 230 * 3 * min_current_ma / 1000;

    if (min_3phase_w > max_1phase_w) { // have dead current range
        int32_t range_width = min_3phase_w - max_1phase_w;
        threshold_3to1_w = max_1phase_w + static_cast<int32_t>(0.25 * range_width);
        threshold_1to3_w = max_1phase_w + static_cast<int32_t>(0.75 * range_width);
    } else { // no dead current range, use simple limits
        threshold_3to1_w = min_3phase_w;
        threshold_1to3_w = max_1phase_w;
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        this->update_all_data();
    }, 0, 250);

    task_scheduler.scheduleWithFixedDelay([this](){
        this->update_io();
    }, 10, 10);

    task_scheduler.scheduleWithFixedDelay([this](){
        this->update_energy();
    }, 250, 250);

    initialized = true;
}

String EnergyManager::get_energy_manager_debug_header()
{
    return "\"millis,"
           "contactor_value,"
           "rgb_value_r,"
           "rgb_value_g,"
           "rgb_value_b,"
           "power,"
           "energy_relative,"
           "energy_absolute,"
           "phase_0_active,"
           "phase_1_active,"
           "phase_2_active,"
           "phase_0_connected,"
           "phase_1_connected,"
           "phase_2_connected,"
           "available"
           "ENERGY METER ERRORS,"
           "input,"
           "output,"
           "input_voltage,"
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
             "%.3f,%.3f,%.3f,%c,%c,%c,%c,%c,%c,,"
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
             all_data.energy_relative,
             all_data.energy_absolute,
             all_data.phases_active[0] ? '1' : '0',
             all_data.phases_active[1] ? '1' : '0',
             all_data.phases_active[2] ? '1' : '0',
             all_data.phases_connected[0] ? '1' : '0',
             all_data.phases_connected[1] ? '1' : '0',
             all_data.phases_connected[2] ? '1' : '0',

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

void EnergyManager::setup_energy_manager()
{
    if (!this->DeviceModule::setup_device()) {
        logger.printfln("setup_device error");
        return;
    }

    this->apply_defaults();
    initialized = true;
}

void EnergyManager::update_all_data()
{
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

    energy_manager_state.get("energy_meter_type")->updateUint(all_data.energy_meter_type);
    energy_manager_state.get("energy_meter_power")->updateFloat(all_data.power);
    energy_manager_state.get("energy_meter_energy_rel")->updateFloat(all_data.energy_relative);
    energy_manager_state.get("energy_meter_energy_abs")->updateFloat(all_data.energy_absolute);
    for (int i = 0; i < 3; i++) {
        energy_manager_state.get("energy_meter_phases_active")->get(i)->updateBool(all_data.phases_active[i]);
    }
    for (int i = 0; i < 3; i++) {
        energy_manager_state.get("energy_meter_phases_connected")->get(i)->updateBool(all_data.phases_connected[i]);
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
        &all_data.energy_relative,
        &all_data.energy_absolute,
        all_data.phases_active,
        all_data.phases_connected,
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
