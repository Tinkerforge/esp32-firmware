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

EnergyManager::EnergyManager() : DeviceModule("energy_manager", "WARP Energy Manager", "Energy Manager", std::bind(&EnergyManager::setup_energy_manager, this))
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
        {"gpio_input_configuration", Config::Array({Config::Uint8(0), Config::Uint8(0)},
            new Config{Config::Uint8(0)}, 2, 2, Config::type_id<Config::ConfUint>())
        },
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
        {"phase_switching", Config::Uint8(2)},
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
    // Handle relay
    uint8_t relay_config = energy_manager_config_in_use.get("relay_config")->asUint();
    if (relay_config == RELAY_CONFIG_RULE_BASED) {
        uint8_t relay_config_if = energy_manager_config_in_use.get("relay_config_if")->asUint();
        switch(relay_config_if) {
            case RELAY_CONFIG_IF_INPUT3:          handle_relay_config_if_input(0);          break;
            case RELAY_CONFIG_IF_INPUT4:          handle_relay_config_if_input(1);          break;
            case RELAY_CONFIG_IF_PHASE_SWITCHING: handle_relay_config_if_phase_switching(); break;
            case RELAY_CONFIG_IF_METER:           handle_relay_config_if_meter();           break;
            default: logger.printfln("Unknown RELAY_CONFIG_IF: %u", relay_config_if);        break;
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
            case INPUT_CONFIG_DEACTIVATED:                                                         break;
            case INPUT_CONFIG_RULES_BASED:     handle_input_config_rule_based(input);              break;
            case INPUT_CONFIG_CONTACTOR_CHECK: handle_input_config_contactor_check(input);         break;
            default: logger.printfln("Unknown INPUT_CONFIG: %u for input %u", input_config, input); break;
        }
    }
}

void EnergyManager::update_energy()
{
    if (!input_charging_allowed[0] || !input_charging_allowed[1]) {
        // TODO: Turn charging off if running
        return;
    }

    const int32_t power_at_house_connection = all_data.power * 1000; // watt
    const int32_t power_max_from_grid       = energy_manager_config_in_use.get("maximum_power_from_grid")->asInt(); // watt
    const int32_t power_minimum_current     = energy_manager_config_in_use.get("minimum_current")->asUint(); // ampere
    const bool    is_3phase                 = all_data.contactor_value;
    const int32_t power_allowed             = power_max_from_grid - ((power_at_house_connection < 0) ? power_at_house_connection : 0); // watt

    if (power_allowed < power_minimum_current) {
        // TODO: Turn charging off if running
        return;
    }

    uint32_t ma_allowed;
    if (is_3phase) { // 3phase
        ma_allowed = power_allowed * 1000 / (3 * 230);
    } else { // 1phase
        ma_allowed = power_allowed * 1000 / (1 * 230);
    }

    // We can not charge with less than 6A.
    if (ma_allowed < 6000) {
        // TODO: Turn charging off if running
        return;
    }

    // TODO: Set "ma_allowed" in charge manager
}

void EnergyManager::setup()
{
    setup_energy_manager();
    if (!device_found)
        return;

    api.restorePersistentConfig("energy_manager/config", &energy_manager_config);
    energy_manager_config_in_use = energy_manager_config;

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
           "input_configuration,"
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
             "%u,%u,,"
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

             all_data.input_configuration[0],
             all_data.input_configuration[1],

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
    energy_manager_state.get("gpio_input_configuration")->get(0)->updateUint(all_data.input_configuration[0]);
    energy_manager_state.get("gpio_input_configuration")->get(1)->updateUint(all_data.input_configuration[1]);
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
        all_data.input_configuration,
        &all_data.voltage,
        &all_data.contactor_check_state
    );

    if (rc != TF_E_OK) {
        logger.printfln("get_all_data_1 error %d", rc);
    }
}
