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
extern TF_HAL hal;
extern WebServer server;

extern API api;

EnergyManager::EnergyManager() : DeviceModule("energy_manager", "Energy Manager", "Energy Manager", std::bind(&EnergyManager::setup_energy_manager, this))
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
        {"mains_power_reception", Config::Float(22)},
        {"minimum_charging", Config::Float(1.4)},
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

void EnergyManager::setup()
{
    setup_energy_manager();
    if (!device_found)
        return;

    api.restorePersistentConfig("energy_manager/config", &energy_manager_config);
    energy_manager_config_in_use = energy_manager_config;

    task_scheduler.scheduleWithFixedDelay("update_all_data", [this](){
        update_all_data();
    }, 0, 250);

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
    snprintf(line, sizeof(line)/sizeof(line[0]),
             "\"%lu,,"
             "%u,,"
             "%u,%u,%u,,"
             "%.3f,%.3f,%.3f,%c,%c,%c,%c,%c,%c,,"
             "%u,%u,%u,%u,%u,%u,%u,,"
             "%u,%u,,"
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
             all_data.phases_active[0] ? '0' : '1',
             all_data.phases_active[1] ? '0' : '1',
             all_data.phases_active[2] ? '0' : '1',
             all_data.phases_connected[0] ? '0' : '1',
             all_data.phases_connected[1] ? '0' : '1',
             all_data.phases_connected[2] ? '0' : '1',
             all_data.energy_meter_type,
             all_data.error_count[0],
             all_data.error_count[1],
             all_data.error_count[2],
             all_data.error_count[3],
             all_data.error_count[4],
             all_data.error_count[5],
             all_data.input[0] ? '0' : '1',
             all_data.input[1] ? '0' : '1',
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


#ifdef MODULE_WS_AVAILABLE
    server.on("/energy_manager/start_debug", HTTP_GET, [this](WebServerRequest request) {
        task_scheduler.scheduleOnce("enable energy_manager debug", [this](){
            ws.pushStateUpdate(this->get_energy_manager_debug_header(), "energy_manager/debug_header");
            debug = true;
        }, 0);
        request.send(200);
    });

    server.on("/energy_manager/stop_debug", HTTP_GET, [this](WebServerRequest request){
        task_scheduler.scheduleOnce("enable energy_manager debug", [this](){
            debug = false;
        }, 0);
        request.send(200);
    });
#endif

    api.addPersistentConfig("energy_manager/config", &energy_manager_config, {}, 1000);
    api.addState("energy_manager/state", &energy_manager_state, {}, 1000);

    this->DeviceModule::register_urls();
}

void EnergyManager::loop()
{
    this->DeviceModule::loop();

#ifdef MODULE_WS_AVAILABLE
    static uint32_t last_debug = 0;
    if (debug && deadline_elapsed(last_debug + 50)) {
        last_debug = millis();
        ws.pushStateUpdate(this->get_energy_manager_debug_line(), "energy_manager/debug");
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

void EnergyManager::update_all_data_struct() {
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