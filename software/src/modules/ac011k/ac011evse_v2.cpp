/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2021-2022 Birger Schmidt <bs-warp@netgaroo.com>
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

#include "ac011evse_v2.h"

/* anchor to fix the diff to ac011k.cpp */

/*********************************************************************************************************
 *
 * This code is mostly a copy of the TinkerForge evse_v2 module code in src/modules/evse_v2/evse_v2.cpp
 * Nevertheless it contains the needed alterations to allow it to work on th EN+ hardware.   
 *
 * This file may need to be in sync with the original for the other modules (backend as well as frontend) 
 * to work properly.          
 *                                                                  
 *********************************************************************************************************/
#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "web_server.h"
#include "modules.h"
#include "../meter/meter.h"

extern EventLog logger;

extern TaskScheduler task_scheduler;
extern WebServer server;

extern API api;
extern bool firmware_update_allowed;

#define SLOT_ACTIVE(x) ((bool)(x & 0x01))
#define SLOT_CLEAR_ON_DISCONNECT(x) ((bool)(x & 0x02))

void EVSEV2::pre_setup()
{
    // States
    evse_state = Config::Object({
        {"iec61851_state", Config::Uint8(0)},
        {"charger_state", Config::Uint8(0)},
        {"GD_state", Config::Uint8(0)},
        {"contactor_state", Config::Uint8(0)},
        {"contactor_error", Config::Uint8(0)},
        {"allowed_charging_current", Config::Uint16(0)},
        {"error_state", Config::Uint8(0)},
        {"lock_state", Config::Uint8(0)},
        {"dc_fault_current_state", Config::Uint8(0)},
        {"time_since_state_change", Config::Uint32(0)},
        {"last_state_change", Config::Uint32(0)},
    });

    evse_hardware_configuration = Config::Object({
        {"Hardware", Config::Str("", 0, 20)},
        {"FirmwareVersion", Config::Str("", 0, 20)},
        {"SerialNumber", Config::Str("", 0, 20)},
        {"evse_found", Config::Bool(false)},
        {"initialized", Config::Bool(false)},
        {"GDFirmwareVersion", Config::Uint16(0)},
        {"jumper_configuration", Config::Uint8(3)}, // 3 = 16 Ampere = 11KW for the EN+ wallbox
        {"has_lock_switch", Config::Bool(false)},   // no key lock switch
        {"evse_version", Config::Uint16(0)},
        {"energy_meter_type", Config::Uint8(METER_TYPE_INTERNAL)}
    });

    evse_low_level_state = Config::Object ({
        {"led_state", Config::Uint8(0)},
        {"cp_pwm_duty_cycle", Config::Uint16(0)},
        {"adc_values", Config::Array({
                Config::Uint16(0),
                Config::Uint16(0),
                Config::Uint16(0),
                Config::Uint16(0),
                Config::Uint16(0),
                Config::Uint16(0),
                Config::Uint16(0),
            }, new Config{Config::Uint16(0)}, 7, 7, Config::type_id<Config::ConfUint>())
        },
        {"voltages", Config::Array({
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
            }, new Config{Config::Int16(0)}, 7, 7, Config::type_id<Config::ConfInt>())
        },
        {"resistances", Config::Array({
                Config::Uint32(0),
                Config::Uint32(0),
            }, new Config{Config::Uint32(0)}, 2, 2, Config::type_id<Config::ConfUint>())
        },
        {"gpio", Config::Array({
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            }, new Config{Config::Bool(false)}, 24, 24, Config::type_id<Config::ConfBool>())},
        {"charging_time", Config::Uint32(0)},
        {"time_since_state_change", Config::Uint32(0)},
        {"uptime", Config::Uint32(0)}
    });

    evse_energy_meter_values = Config::Object({
        {"power", Config::Float(0)},
        {"energy_rel", Config::Float(0)},
        {"energy_abs", Config::Float(0)},
        {"phases_active", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            new Config{Config::Bool(false)},
            3, 3, Config::type_id<Config::ConfBool>())},
        {"phases_connected", Config::Array({Config::Bool(false),Config::Bool(false),Config::Bool(false)},
            new Config{Config::Bool(false)},
            3, 3, Config::type_id<Config::ConfBool>())}
    });

    evse_energy_meter_errors = Config::Object({
        {"local_timeout", Config::Uint32(0)},
        {"global_timeout", Config::Uint32(0)},
        {"illegal_function", Config::Uint32(0)},
        {"illegal_data_access", Config::Uint32(0)},
        {"illegal_data_value", Config::Uint32(0)},
        {"slave_device_failure", Config::Uint32(0)},
    });

    evse_button_state = Config::Object({
        {"button_press_time", Config::Uint32(0)},
        {"button_release_time", Config::Uint32(0)},
        {"button_pressed", Config::Bool(false)},
    });

    Config *evse_charging_slot = new Config{Config::Object({
        {"max_current", Config::Uint32(0)},
        {"active", Config::Bool(false)},
        {"clear_on_disconnect", Config::Bool(false)}
    })};

    evse_slots = Config::Array({},
        evse_charging_slot,
        CHARGING_SLOT_COUNT, CHARGING_SLOT_COUNT,
        Config::type_id<Config::ConfObject>());

    for (int i = 0; i < CHARGING_SLOT_COUNT; ++i)
        evse_slots.add();

    evse_indicator_led = Config::Object({
        {"indication", Config::Int16(0)},
        {"duration", Config::Uint16(0)},
    });

    // Actions

    evse_reset_dc_fault_current_state = Config::Object({
        {"password", Config::Uint32(0)} // 0xDC42FA23
    });

    // TODO indicator LED


    // EVSE configs
    evse_gpio_configuration = Config::Object({
        {"shutdown_input", Config::Uint8(0)},
        {"input", Config::Uint8(0)},
        {"output", Config::Uint8(0)}
    });

    evse_gpio_configuration_update = evse_gpio_configuration;

    evse_button_configuration = Config::Object({
        {"button", Config::Uint8(2)}
    });

    evse_button_configuration_update = Config::Object({
        {"button", Config::Uint8(2)}
    });

    evse_control_pilot_configuration = Config::Object({
        {"control_pilot", Config::Uint8(0)}
    });

    evse_control_pilot_configuration_update = Config::Object({
        {"control_pilot", Config::Uint8(0)}
    });

    evse_control_pilot_connected = Config::Object({
        {"connected", Config::Bool(true)}
    });

    evse_auto_start_charging = Config::Object({
        {"auto_start_charging", Config::Bool(true)}
    });

    evse_auto_start_charging_update = Config::Object({
        {"auto_start_charging", Config::Bool(true)}
    });

    evse_global_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    evse_global_current_update = evse_global_current;

    evse_management_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    evse_management_enabled_update = evse_management_enabled;

    evse_user_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    evse_user_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    evse_user_enabled_update = evse_user_enabled;

    evse_external_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    evse_external_enabled_update = evse_external_enabled;

    evse_external_defaults = Config::Object({
        {"current", Config::Uint16(0)},
        {"clear_on_disconnect", Config::Bool(false)},
    });
    evse_external_defaults_update = evse_external_defaults;

    evse_management_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    evse_management_current_update = evse_management_current;

    evse_external_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    evse_external_current_update = evse_external_current;

    evse_external_clear_on_disconnect = Config::Object({
        {"clear_on_disconnect", Config::Bool(false)}
    });

    evse_external_clear_on_disconnect_update = evse_external_clear_on_disconnect;

    evse_modbus_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    evse_modbus_enabled_update = evse_modbus_enabled;

    evse_ocpp_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    evse_ocpp_enabled_update = evse_ocpp_enabled;
}

bool EVSEV2::apply_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear)
{
    uint16_t old_current = evse_slots.get(slot)->get("max_current")->asUint();
    bool old_enabled     = evse_slots.get(slot)->get("active")->asBool();
    bool old_clear       = evse_slots.get(slot)->get("clear_on_disconnect")->asBool();

    if ((old_current == current) && (old_enabled == enabled) && (old_clear == clear))
        return false;

    //rc = tf_evse_v2_set_charging_slot_default(&device, slot, current, enabled, clear);
    evse_slots.get(slot)->get("max_current")->updateUint(current);
    evse_slots.get(slot)->get("active")->updateBool(enabled);
    evse_slots.get(slot)->get("clear_on_disconnect")->updateBool(clear);
    api.writeConfig("evse/slots", &evse_slots);

	logger.printfln("set - slot %d: max_current: %d, active: %s, clear_on_disconnect: %s",
            slot,
            current,
            enabled ?"true":"false",
            clear ?"true":"false");

    return true;
}

void EVSEV2::apply_defaults()
{
    // Maybe this is the first start-up after updating the EVSE to firmware 2.1.0 (or higher)
    // (Or the first start-up at all)
    // Make sure, that the charging slot defaults are the expected ones.

    // Slot 0 to 3 are readonly

    // Slot 4 (auto start): Enabled is read only, current and clear depend on the auto start setting
    // Doing anything with this slot should be unnecessary, as we would only change the current to 32000 or 0
    // depending on clear_on_disconnect. Any other value could be considered a bug in the EVSE firmware.

    // Slot 5 (global) has to be always enabled and never cleared. current is set per API
    // The charger API ensures that the slot's current and its default are always the same,
    // but if for any reason, they are not equal when booting up, just set the current value as
    // default. In the common case this has no effect. Also set enabled and clear in case this is
    // the first start-up.

    // If this is the first start-up, this slot will not be active.
    // In the old firmwares, the global current was not persistant
    // so setting it to 32000 is expected after start-up.

    // Slot 6 (user) depends on user config.
    // It can be enabled per API (is stored in the EVSEs flash, not ours).
    // Set clear to true and current to 0 in any case: If disabled those are ignored anyway.

    // Slot 7 (charge manager) can be enabled per API (is stored in the EVSEs flash, not ours).
    // Set clear to true and current to 0 in any case: If disabled those are ignored anyway.

    // Slot 8 (external) is controlled via API, no need to change anything here
}

void EVSEV2::factory_reset()
{
    logger.printfln("AC011K factory reset of the GD chip is not implemented. And probably doesn't have to.");
}

void EVSEV2::setup()
{
    
    /* TODO: make reset work on AC011K */
    /* task_scheduler.scheduleOnce([this](){ */
    /*     uint32_t press_time = 0; */
    /*     tf_evse_v2_get_button_press_boot_time(&device, true, &press_time); */
    /*     if (press_time != 0) */
    /*         logger.printfln("Reset boot button press time"); */
    /* }, 40000); */

    // Get all data once before announcing the EVSE feature.
    api.addFeature("evse");
    /* api.addFeature("cp_disconnect"); */
    /* api.addFeature("button_configuration"); */
    api.addFeature("meter_phases");
    api.addFeature("meter_all_values");

    
}

String EVSEV2::get_evse_debug_header()
{
    return "\"millis,"
           "STATE,"
           "iec61851_state,"
           "charger_state,"
           "contactor_state,"
           "contactor_error,"
           "allowed_charging_current,"
           "error_state,"
           "lock_state,"
           "dc_fault_current_state,"
           "HARDWARE CONFIG,"
           "jumper_configuration,"
           "has_lock_switch,"
           "evse_version,"
           "energy_meter_type,"
           "ENERGY METER,"
           "power,"
           "energy_relative,"
           "energy_absolute,"
           "phase_0_active,"
           "phase_1_active,"
           "phase_2_active,"
           "phase_0_connected,"
           "phase_1_connected,"
           "phase_2_connected,"
           "ENERGY METER ERRORS,"
           "local_timeout,"
           "global_timeout,"
           "illegal_function,"
           "illegal_data_access,"
           "illegal_data_value,"
           "slave_device_failure,"
           "LL-State,"
           "led_state,"
           "cp_pwm_duty_cycle,"
           "charging_time,"
           "time_since_state_change,"
           "uptime,"
           "ADC VALUES,"
           "CP/PE before (PWM high),"
           "CP/PE after (PWM high),"
           "CP/PE before (PWM low),"
           "CP/PE after (PWM low),"
           "PP/PE,"
           "+12V,"
           "-12V,"
           "VOLTAGES,"
           "CP/PE before (PWM high),"
           "CP/PE after (PWM high),"
           "CP/PE before (PWM low),"
           "CP/PE after (PWM low),"
           "PP/PE,"
           "+12V,"
           "-12V,"
           "RESISTANCES,"
           "CP/PE,"
           "PP/PE,"
           "GPIOs,"
           "Config Jumper 0 (0),"
           "Motor Fault (1),"
           "DC Error (2),"
           "Config Jumper 1 (3),"
           "DC Test (4),"
           "Enable (5),"
           "Switch (6),"
           "CP PWM (7),"
           "Input Motor Switch (8),"
           "Relay (Contactor) (9),"
           "GP Output (10),"
           "CP Disconnect (11),"
           "Motor Enable (12),"
           "Motor Phase (13),"
           "AC 1 (14),"
           "AC 2 (15),"
           "GP Input (16),"
           "DC X6 (17),"
           "DC X30 (18),"
           "LED (19),"
           "unused (20),"
           "unused (21),"
           "unused (22),"
           "unused (23)"
           "\"";
}

String EVSEV2::get_evse_debug_line() {
    if(!initialized)
        return "EVSE is not initialized!";

    uint8_t iec61851_state = 0;
    uint8_t charger_state = 0;
    uint8_t contactor_state = 0;
    uint8_t contactor_error = 0;
    uint8_t error_state = 0;
    uint8_t lock_state = 0;
    uint16_t allowed_charging_current = 0;
    uint32_t time_since_state_change = 0;
    uint32_t uptime = 0;

    int rc = 0;
    /* int rc = bs_evse_get_state( */
    /*     &iec61851_state, */
    /*     &charger_state, */
    /*     &contactor_state, */
    /*     &contactor_error, */
    /*     &allowed_charging_current, */
    /*     &error_state, */
    /*     &lock_state, */
    /*     &time_since_state_change, */
    /*     &uptime); */

    /* if(rc != TF_E_OK) { */
    /*     return String("evse_get_state failed: rc: ") + String(rc); */
    /* } */

    bool low_level_mode_enabled = false;
    uint8_t led_state = 0;
    uint16_t cp_pwm_duty_cycle = 0;

    uint16_t adc_values[2] = {0,0};
    int16_t voltages[3] = {0,0,0};
    uint32_t resistances[2] = {0,0};
    bool gpio[5] = {false,false,false,false,false};

//    rc = tf_evse_get_low_level_state(
//        &low_level_mode_enabled,
//        &led_state,
//        &cp_pwm_duty_cycle,
//        adc_values,
//        voltages,
//        resistances,
//        gpio);

    if(rc != TF_E_OK) {
        return String("evse_get_low_level_state failed: rc: ") + String(rc);
    }

    char line[150] = {0};
//                                                  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
    snprintf(line, sizeof(line)/sizeof(line[0]), "%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%c,%u,%u,%u,%u,%d,%d,%d,%u,%u,%c,%c,%c,%c,%c\n",
        millis(),
        iec61851_state,
        charger_state,
        contactor_state,
        contactor_error,

        allowed_charging_current,
        error_state,
        lock_state,
        time_since_state_change,
        uptime,

        low_level_mode_enabled ? '1' : '0',
        led_state,
        cp_pwm_duty_cycle,
        adc_values[0],
        adc_values[1],

        voltages[0],
        voltages[1],
        voltages[2],
        resistances[0],
        resistances[1],

        gpio[0] ? '1' : '0',
        gpio[1] ? '1' : '0',
        gpio[2] ? '1' : '0',
        gpio[3] ? '1' : '0',
        gpio[4] ? '1' : '0'
        );

    return String(line);
}

void EVSEV2::set_managed_current(uint16_t current)
{
    //tf_evse_v2_set_charging_slot_max_current(CHARGING_SLOT_CHARGE_MANAGER, current));
    evse_slots.get(CHARGING_SLOT_CHARGE_MANAGER)->get("max_current")->updateUint(current);
    this->last_current_update = millis();
    this->shutdown_logged = false;
}

void EVSEV2::set_user_current(uint16_t current)
{
    //is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_USER, current));
    evse_slots.get(CHARGING_SLOT_USER)->get("max_current")->updateUint(current);
}

void EVSEV2::set_modbus_current(uint16_t current)
{
    //is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_MODBUS_TCP, current));
    evse_slots.get(CHARGING_SLOT_MODBUS_TCP)->get("max_current")->updateUint(current);
}

void EVSEV2::set_modbus_enabled(bool enabled)
{
    //is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_MODBUS_TCP_ENABLE, enabled ? 32000 : 0));
    evse_slots.get(CHARGING_SLOT_MODBUS_TCP_ENABLE)->get("max_current")->updateUint(enabled ? 32000 : 0);
}

void EVSEV2::set_ocpp_current(uint16_t current)
{
    //is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_OCPP, current));
    evse_slots.get(CHARGING_SLOT_OCPP)->get("max_current")->updateUint(current);
}

uint16_t EVSEV2::get_ocpp_current()
{
    return evse_slots.get(CHARGING_SLOT_OCPP)->get("max_current")->asUint();
}

void EVSEV2::register_urls()
{

#if MODULE_CM_NETWORKING_AVAILABLE()
    cm_networking.register_client([this](uint16_t current) {
        set_managed_current(current);
    });

    task_scheduler.scheduleWithFixedDelay([this](){
        uint16_t supported_current = 32000;
        for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
            if (i == CHARGING_SLOT_CHARGE_MANAGER)
                continue;
            if (!evse_slots.get(i)->get("active")->asBool())
                continue;
            supported_current = min(supported_current, (uint16_t)evse_slots.get(i)->get("max_current")->asUint());
        }

        cm_networking.send_client_update(
            evse_state.get("iec61851_state")->asUint(),
            evse_state.get("charger_state")->asUint(),
            evse_state.get("error_state")->asUint(),
            evse_low_level_state.get("uptime")->asUint(),
            evse_low_level_state.get("charging_time")->asUint(),
            evse_slots.get(CHARGING_SLOT_CHARGE_MANAGER)->get("max_current")->asUint(),
            supported_current,
            evse_management_enabled.get("enabled")->asBool()
        );
    }, 1000, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (!deadline_elapsed(this->last_current_update + 30000))
            return;
        if (!evse_management_enabled.get("enabled")->asBool()) {
            // Push back the next check for 30 seconds: If managed gets enabled,
            // we want to wait 30 seconds before setting the current for the first time.
            this->last_current_update = millis();
            return;
        }
        if(!this->shutdown_logged)
            logger.printfln("Got no managed current update for more than 30 seconds. Setting managed current to 0");
        this->shutdown_logged = true;
        evse_slots.get(CHARGING_SLOT_CHARGE_MANAGER)->get("max_current")->updateUint(0);
    }, 1000, 1000);
#endif

    // States
    api.addState("evse/state", &evse_state, {}, 1000);
    api.addState("evse/hardware_configuration", &evse_hardware_configuration, {}, 1000);
    api.addState("evse/low_level_state", &evse_low_level_state, {}, 1000);
    api.addState("evse/button_state", &evse_button_state, {}, 250);
    api.addPersistentConfig("evse/slots", &evse_slots, {}, 1000);
    api.addState("evse/indicator_led", &evse_indicator_led, {}, 1000);
    api.addState("evse/control_pilot_connected", &evse_control_pilot_connected, {}, 1000); //TODO

    // Actions
    api.addCommand("evse/reset_dc_fault_current_state", &evse_reset_dc_fault_current_state, {}, [this](){
        //is_in_bootloader(tf_evse_v2_reset_dc_fault_current_state(&device, evse_reset_dc_fault_current_state.get("password")->asUint()));
    }, true);

    api.addCommand("evse/stop_charging", Config::Null(), {}, [this](){
        if (evse_state.get("iec61851_state")->asUint() != IEC_STATE_A) {
            evse_slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("max_current")->updateUint(0);
            /* bs_evse_stop_charging(); // TODO scheduled function to start stop depending on slots?! */
        }
    }, true);

    api.addCommand("evse/start_charging", Config::Null(), {}, [this](){
        if (evse_state.get("iec61851_state")->asUint() != IEC_STATE_A) {
            evse_slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("max_current")->updateUint(32000);
            /* bs_evse_start_charging(); // TODO scheduled function to start stop depending on slots?! */
        }
    }, true);

#if MODULE_WS_AVAILABLE()
    server.on("/evse/start_debug", HTTP_GET, [this](WebServerRequest request) {
        task_scheduler.scheduleOnce([this](){
            ws.pushRawStateUpdate(this->get_evse_debug_header(), "evse/debug_header");
            debug = true;
        }, 0);
        return request.send(200);
    });

    server.on("/evse/stop_debug", HTTP_GET, [this](WebServerRequest request){
        task_scheduler.scheduleOnce([this](){
            debug = false;
        }, 0);
        return request.send(200);
    });
#endif

    // TODO: indicator led update as API?

    api.addState("evse/external_current", &evse_external_current, {}, 1000);
    api.addCommand("evse/external_current_update", &evse_external_current_update, {}, [this](){
        evse_slots.get(CHARGING_SLOT_EXTERNAL)->get("max_current")->updateUint(evse_external_current_update.get("current")->asUint());
    }, false);

    api.addState("evse/external_clear_on_disconnect", &evse_external_clear_on_disconnect, {}, 1000);
    api.addCommand("evse/external_clear_on_disconnect_update", &evse_external_clear_on_disconnect_update, {}, [this](){
        evse_slots.get(CHARGING_SLOT_EXTERNAL)->get("clear_on_disconnect")->updateBool(evse_external_clear_on_disconnect_update.get("clear_on_disconnect")->asBool());
    }, false);

    api.addState("evse/management_current", &evse_management_current, {}, 1000);
    api.addCommand("evse/management_current_update", &evse_management_current_update, {}, [this](){
        this->set_managed_current(evse_management_current_update.get("current")->asUint());
    }, false);

    // Configurations. Note that those are _not_ configs in the api.addPersistentConfig sense:
    // The configs are stored on the EVSE itself, not the ESP's flash.
    // All _update APIs that write the EVSEs flash without checking first if this was a change
    // are marked as actions to make sure the flash is not written unnecessarily.
    api.addState("evse/gpio_configuration", &evse_gpio_configuration, {}, 1000);
    api.addCommand("evse/gpio_configuration_update", &evse_gpio_configuration_update, {}, [this](){
        /* is_in_bootloader(tf_evse_v2_set_gpio_configuration(&device, evse_gpio_configuration_update.get("shutdown_input")->asUint(), */
        /*                                                             evse_gpio_configuration_update.get("input")->asUint(), */
        /*                                                             evse_gpio_configuration_update.get("output")->asUint())); */
    }, true);

    api.addState("evse/button_configuration", &evse_button_configuration, {}, 1000);
    api.addCommand("evse/button_configuration_update", &evse_button_configuration_update, {}, [this](){
        /* is_in_bootloader(tf_evse_v2_set_button_configuration(&device, evse_button_configuration_update.get("button")->asUint())); */
    }, true);

    api.addState("evse/control_pilot_configuration", &evse_control_pilot_configuration, {}, 1000);
    api.addCommand("evse/control_pilot_configuration_update", &evse_control_pilot_configuration_update, {}, [this](){
        /*auto cp = */ evse_control_pilot_configuration_update.get("control_pilot")->asUint();
        /* int rc = tf_evse_v2_set_control_pilot_configuration(&device, cp, nullptr); */
        /* logger.printfln("updating control pilot to %u.", cp); */
        /* is_in_bootloader(rc); */
    }, true);

    api.addState("evse/auto_start_charging", &evse_auto_start_charging, {}, 1000);
    api.addCommand("evse/auto_start_charging_update", &evse_auto_start_charging_update, {}, [this](){
        // 1. set auto start
        // 2. make persistent
        // 3. fake a start/stop charging

        /* evse_auto_start_charging.get("auto_start_charging")->updateBool( */
        /*     !evse_slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("clear_on_disconnect")->asBool()); */
        bool enable_auto_start = evse_auto_start_charging_update.get("auto_start_charging")->asBool();


        if (enable_auto_start) {
            this->apply_slot_default(CHARGING_SLOT_AUTOSTART_BUTTON, 32000, true, false);
        } else {
            this->apply_slot_default(CHARGING_SLOT_AUTOSTART_BUTTON, 0, true, true);
        }

        if (enable_auto_start) {
            evse_slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("max_current")->updateUint(32000);
        } else {
            // Only "stop" charging if no car is currently plugged in.
            // Clear on disconnect only triggers once, so we have to zero the current manually here.
            uint8_t iec_state = evse_state.get("iec61851_state")->asUint();
            if (iec_state != 2 && iec_state != 3)
                evse_slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("max_current")->updateUint(0);
        }
    }, false);

    api.addState("evse/global_current", &evse_global_current, {}, 1000);
    api.addCommand("evse/global_current_update", &evse_global_current_update, {}, [this](){
        uint16_t current = evse_global_current_update.get("current")->asUint();
        this->apply_slot_default(CHARGING_SLOT_GLOBAL, current, true, false);
    }, false);

    api.addState("evse/management_enabled", &evse_management_enabled, {}, 1000);
    api.addCommand("evse/management_enabled_update", &evse_management_enabled_update, {}, [this](){
        bool enabled = evse_management_enabled_update.get("enabled")->asBool();

        if (enabled == evse_management_enabled.get("enabled")->asBool())
            return;

        if (enabled)
            this->apply_slot_default(CHARGING_SLOT_CHARGE_MANAGER, 0, true, true);
        else
            this->apply_slot_default(CHARGING_SLOT_CHARGE_MANAGER, 32000, false, false);
    }, false);

    api.addState("evse/user_current", &evse_user_current, {}, 1000);
    api.addState("evse/user_enabled", &evse_user_enabled, {}, 1000);
    api.addCommand("evse/user_enabled_update", &evse_user_enabled_update, {}, [this](){
        bool enabled = evse_user_enabled_update.get("enabled")->asBool();

        if (enabled == evse_user_enabled.get("enabled")->asBool())
            return;

#if MODULE_USERS_AVAILABLE()
        if (enabled) {
            users.stop_charging(0, true);
        }
#endif

        if (enabled)
            this->apply_slot_default(CHARGING_SLOT_USER, 0, true, true);
        else
            this->apply_slot_default(CHARGING_SLOT_USER, 32000, false, false);
    }, false);

    api.addState("evse/external_enabled", &evse_external_enabled, {}, 1000);
    api.addCommand("evse/external_enabled_update", &evse_external_enabled_update, {}, [this](){
        bool enabled = evse_external_enabled_update.get("enabled")->asBool();

        if (enabled == evse_external_enabled.get("enabled")->asBool())
            return;

        this->apply_slot_default(CHARGING_SLOT_EXTERNAL, 32000, enabled, false);
    }, false);

    api.addState("evse/external_defaults", &evse_external_defaults, {}, 1000);
    api.addCommand("evse/external_defaults_update", &evse_external_defaults_update, {}, [this](){
        //bool enabled;
        //tf_evse_v2_get_charging_slot_default(&device, CHARGING_SLOT_EXTERNAL, nullptr, &enabled, nullptr);
        this->apply_slot_default(CHARGING_SLOT_EXTERNAL, evse_external_defaults_update.get("current")->asUint(), evse_slots.get(CHARGING_SLOT_EXTERNAL)->get("active")->asBool(), evse_external_defaults_update.get("clear_on_disconnect")->asBool());
    }, false);

    api.addState("evse/modbus_tcp_enabled", &evse_modbus_enabled, {}, 1000);
    api.addCommand("evse/modbus_tcp_enabled_update", &evse_modbus_enabled_update, {}, [this](){
        bool enabled = evse_modbus_enabled_update.get("enabled")->asBool();

        if (enabled == evse_modbus_enabled.get("enabled")->asBool())
            return;

        if (enabled) {
            //tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP, 32000, true, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP, 32000, true, false);

            //tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, true, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, true, false);
        }
        else {
            //tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP, 32000, false, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP, 32000, false, false);

            //tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, false, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, false, false);
        }
    }, false);

    api.addState("evse/ocpp_enabled", &evse_ocpp_enabled, {}, 1000);
    api.addCommand("evse/ocpp_enabled_update", &evse_ocpp_enabled_update, {}, [this](){
        bool enabled = evse_ocpp_enabled_update.get("enabled")->asBool();

        if (enabled == evse_ocpp_enabled.get("enabled")->asBool())
            return;

        if (enabled) {
            this->apply_slot_default(CHARGING_SLOT_OCPP, 32000, true, false);
        }
        else {
            this->apply_slot_default(CHARGING_SLOT_OCPP, 32000, false, false);
        }
    }, false);
}

void EVSEV2::loop()
{
#if MODULE_WS_AVAILABLE()
    static uint32_t last_debug = 0;
    if (debug && deadline_elapsed(last_debug + 50)) {
        last_debug = millis();
        ws.pushRawStateUpdate(this->get_evse_debug_line(), "evse/debug");
    }
#endif
}

void EVSEV2::setup_evse()
{
    this->apply_defaults();
    // initialized is set to true after we got the SerialNumber from the GD chip
}


void EVSEV2::update_all_data()
{
    if (!initialized)
        return;

/* The TinkerForge hardware stores and maintains a lot of the state in the EVSE bricklet.
   Our situation is different on the AC011K hardware. 
   Therefore we do not have to update all the states, as we rely on the data we have in the ESP32.

    // get_all_data_1 - 51 byte
    uint8_t iec61851_state;
    uint8_t charger_state;
    uint8_t contactor_state;
    uint8_t contactor_error;
    uint16_t allowed_charging_current;
    uint8_t error_state;
    uint8_t lock_state;
    uint8_t dc_fault_current_state;
    uint8_t jumper_configuration;
    bool has_lock_switch;
    uint8_t evse_version;
    uint8_t energy_meter_type;
    float power;
    float energy_relative;
    float energy_absolute;
    bool phases_active[3];
    bool phases_connected[3];
    uint32_t error_count[6];

    // get_all_data_2 - 19 byte
    uint8_t shutdown_input_configuration;
    uint8_t input_configuration;
    uint8_t output_configuration;
    int16_t indication;
    uint16_t duration;
    uint8_t button_configuration;
    uint32_t button_press_time;
    uint32_t button_release_time;
    bool button_pressed;
    uint8_t control_pilot;
    bool control_pilot_connected;

    // get_low_level_state - 57 byte
    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;
    uint16_t adc_values[7];
    int16_t voltages[7];
    uint32_t resistances[2];
    bool gpio[24];
    uint32_t charging_time;
    uint32_t time_since_state_change;
    uint32_t uptime;

    // get_all_charging_slots - 60 byte
    uint16_t max_current[20];
    uint8_t active_and_clear_on_disconnect[20];

    int rc = tf_evse_v2_get_all_data_1(&device,
                                       &iec61851_state,
                                       &charger_state,
                                       &contactor_state,
                                       &contactor_error,
                                       &allowed_charging_current,
                                       &error_state,
                                       &lock_state,
                                       &dc_fault_current_state,
                                       &jumper_configuration,
                                       &has_lock_switch,
                                       &evse_version,
                                       &energy_meter_type,
                                       &power,
                                       &energy_relative,
                                       &energy_absolute,
                                       phases_active,
                                       phases_connected,
                                       error_count);

    if (rc != TF_E_OK) {
        logger.printfln("all_data_1 %d", rc);
        is_in_bootloader(rc);
        return;
    }

    rc = tf_evse_v2_get_all_data_2(&device,
                                   &shutdown_input_configuration,
                                   &input_configuration,
                                   &output_configuration,
                                   &indication,
                                   &duration,
                                   &button_configuration,
                                   &button_press_time,
                                   &button_release_time,
                                   &button_pressed,
                                   &control_pilot,
                                   &control_pilot_connected);

    if (rc != TF_E_OK) {
        logger.printfln("all_data_2 %d", rc);
        is_in_bootloader(rc);
        return;
    }

    rc = tf_evse_v2_get_low_level_state(&device,
                                        &led_state,
                                        &cp_pwm_duty_cycle,
                                        adc_values,
                                        voltages,
                                        resistances,
                                        gpio,
                                        &charging_time,
                                        &time_since_state_change,
                                        &uptime);

    if (rc != TF_E_OK) {
        logger.printfln("ll_state %d", rc);
        is_in_bootloader(rc);
        return;
    }

    rc = tf_evse_v2_get_all_charging_slots(&device, max_current, active_and_clear_on_disconnect);

    if (rc != TF_E_OK) {
        logger.printfln("slots %d", rc);
        is_in_bootloader(rc);
        return;
    }

    uint16_t external_default_current;
    bool external_default_enabled;
    bool external_default_clear_on_disconnect;

    rc = tf_evse_v2_get_charging_slot_default(&device,
                                              CHARGING_SLOT_EXTERNAL,
                                              &external_default_current,
                                              &external_default_enabled,
                                              &external_default_clear_on_disconnect);

    if (rc != TF_E_OK) {
        logger.printfln("external slot default %d", rc);
        is_in_bootloader(rc);
        return;
    }

    // We don't allow firmware updates when a vehicle is connected,
    // to be sure a potential EVSE firmware update does not interrupt a
    // charge and/or does strange stuff with the vehicle while updating.
    // However if we are in an error state, either after the EVSE update
    // the error is still there (this is fine for us) or it is cleared,
    // then the EVSE could potentially start to charge, which is okay,
    // as the ESP firmware is already running, so we can for example
    // track the charge.
    firmware_update_allowed = charger_state == 0 || charger_state == 4;

    // get_state

    evse_state.get("iec61851_state")->updateUint(iec61851_state);
    evse_state.get("charger_state")->updateUint(charger_state);
    evse_state.get("contactor_state")->updateUint(contactor_state);
    bool contactor_error_changed = evse_state.get("contactor_error")->updateUint(contactor_error);
    evse_state.get("allowed_charging_current")->updateUint(allowed_charging_current);
    bool error_state_changed = evse_state.get("error_state")->updateUint(error_state);
    evse_state.get("lock_state")->updateUint(lock_state);
    bool dc_fault_current_state_changed = evse_state.get("dc_fault_current_state")->updateUint(dc_fault_current_state);

    if (contactor_error_changed) {
        if (contactor_error != 0) {
            logger.printfln("EVSE: Contactor error %d", contactor_error);
        } else {
            logger.printfln("EVSE: Contactor error cleared");
        }
    }

    if (error_state_changed) {
        if (error_state != 0) {
            logger.printfln("EVSE: Error state %d", error_state);
        } else {
            logger.printfln("EVSE: Error state cleared");
        }
    }

    if (dc_fault_current_state_changed) {
        if (dc_fault_current_state != 0) {
            logger.printfln("EVSE: DC Fault current state %d", dc_fault_current_state);
        } else {
            logger.printfln("EVSE: DC Fault current state cleared");
        }
    }

    // get_hardware_configuration
    evse_hardware_configuration.get("jumper_configuration")->updateUint(jumper_configuration);
    evse_hardware_configuration.get("has_lock_switch")->updateBool(has_lock_switch);
    evse_hardware_configuration.get("evse_version")->updateUint(evse_version);
    evse_hardware_configuration.get("energy_meter_type")->updateUint(energy_meter_type);

    // get_low_level_state
    evse_low_level_state.get("led_state")->updateUint(led_state);
    evse_low_level_state.get("cp_pwm_duty_cycle")->updateUint(cp_pwm_duty_cycle);

    for (int i = 0; i < sizeof(adc_values) / sizeof(adc_values[0]); ++i)
        evse_low_level_state.get("adc_values")->get(i)->updateUint(adc_values[i]);

    for (int i = 0; i < sizeof(voltages) / sizeof(voltages[0]); ++i)
        evse_low_level_state.get("voltages")->get(i)->updateInt(voltages[i]);

    for (int i = 0; i < sizeof(resistances) / sizeof(resistances[0]); ++i)
        evse_low_level_state.get("resistances")->get(i)->updateUint(resistances[i]);

    for (int i = 0; i < sizeof(gpio) / sizeof(gpio[0]); ++i)
        evse_low_level_state.get("gpio")->get(i)->updateBool(gpio[i]);

    evse_low_level_state.get("charging_time")->updateUint(charging_time);
    evse_low_level_state.get("time_since_state_change")->updateUint(time_since_state_change);
    evse_low_level_state.get("uptime")->updateUint(uptime);

    for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
        evse_slots.get(i)->get("max_current")->updateUint(max_current[i]);
        evse_slots.get(i)->get("active")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[i]));
        evse_slots.get(i)->get("clear_on_disconnect")->updateBool(SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[i]));
    }

    evse_auto_start_charging.get("auto_start_charging")->updateBool(
        !evse_slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("clear_on_disconnect")->asBool());

    // get_energy_meter_values
    evse_energy_meter_values.get("power")->updateFloat(power);
    evse_energy_meter_values.get("energy_rel")->updateFloat(energy_relative);
    evse_energy_meter_values.get("energy_abs")->updateFloat(energy_absolute);

    for (int i = 0; i < 3; ++i)
        evse_energy_meter_values.get("phases_active")->get(i)->updateBool(phases_active[i]);

    for (int i = 0; i < 3; ++i)
        evse_energy_meter_values.get("phases_connected")->get(i)->updateBool(phases_connected[i]);

    // get_energy_meter_errors
    evse_energy_meter_errors.get("local_timeout")->updateUint(error_count[0]);
    evse_energy_meter_errors.get("global_timeout")->updateUint(error_count[1]);
    evse_energy_meter_errors.get("illegal_function")->updateUint(error_count[2]);
    evse_energy_meter_errors.get("illegal_data_access")->updateUint(error_count[3]);
    evse_energy_meter_errors.get("illegal_data_value")->updateUint(error_count[4]);
    evse_energy_meter_errors.get("slave_device_failure")->updateUint(error_count[5]);

    // get_gpio_configuration
    evse_gpio_configuration.get("shutdown_input")->updateUint(shutdown_input_configuration);
    evse_gpio_configuration.get("input")->updateUint(input_configuration);
    evse_gpio_configuration.get("output")->updateUint(output_configuration);

    // get_button_configuration
    evse_button_configuration.get("button")->updateUint(button_configuration);

    // get_button_state
    evse_button_state.get("button_press_time")->updateUint(button_press_time);
    evse_button_state.get("button_release_time")->updateUint(button_release_time);
    evse_button_state.get("button_pressed")->updateBool(button_pressed);

    // get_control_pilot
    evse_control_pilot_configuration.get("control_pilot")->updateUint(control_pilot);
    evse_control_pilot_connected.get("connected")->updateBool(control_pilot_connected);

    // get_indicator_led
    evse_indicator_led.get("indication")->updateInt(indication);
    evse_indicator_led.get("duration")->updateUint(duration);
    */

    /* 
    We just can not get all the data above and therefore ignore it and either
    stick to our own fake default values, or 
    use the values that we ijected at the apropiate plce/time when communicating with the GD chip.
    */

    evse_auto_start_charging.get("auto_start_charging")->updateBool(!evse_slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("clear_on_disconnect")->asBool());

    evse_management_enabled.get("enabled")->updateBool(evse_slots.get(CHARGING_SLOT_CHARGE_MANAGER)->get("active")->asBool());

    evse_user_enabled.get("enabled")->updateBool(evse_slots.get(CHARGING_SLOT_USER)->get("active")->asBool());

    evse_modbus_enabled.get("enabled")->updateBool(evse_slots.get(CHARGING_SLOT_MODBUS_TCP)->get("active")->asBool());
    evse_ocpp_enabled.get("enabled")->updateBool(evse_slots.get(CHARGING_SLOT_OCPP)->get("active")->asBool());

    evse_external_enabled.get("enabled")->updateBool(evse_slots.get(CHARGING_SLOT_EXTERNAL)->get("active")->asBool());
    evse_external_clear_on_disconnect.get("clear_on_disconnect")->updateBool(evse_slots.get(CHARGING_SLOT_EXTERNAL)->get("clear_on_disconnect")->asBool());

    evse_global_current.get("current")->updateUint(evse_slots.get(CHARGING_SLOT_GLOBAL)->get("max_current")->asUint());
    evse_management_current.get("current")->updateUint(evse_slots.get(CHARGING_SLOT_CHARGE_MANAGER)->get("max_current")->asUint());
    evse_external_current.get("current")->updateUint(evse_slots.get(CHARGING_SLOT_EXTERNAL)->get("max_current")->asUint());
    evse_user_current.get("current")->updateUint(evse_slots.get(CHARGING_SLOT_USER)->get("max_current")->asUint());

    evse_external_defaults.get("current")->updateUint(evse_slots.get(CHARGING_SLOT_EXTERNAL)->get("max_current")->asUint());
    evse_external_defaults.get("clear_on_disconnect")->updateBool(evse_slots.get(CHARGING_SLOT_EXTERNAL)->get("clear_on_disconnect")->asBool());

    /* 
     * do the things that would have been done on the EVSE on the TinkerForge Box like
     * - take note of times
     *
     * in ac011k.cpp (evse_slot_machine)
     * - calculate the *allowed_charging_current* and 
     * - actually start/stop a charge
     *
     */

    evse_low_level_state.get("time_since_state_change")->updateUint(evse_state.get("time_since_state_change")->asUint());
    evse_low_level_state.get("uptime")->updateUint(millis());
    //evse_slot_machine();

#if MODULE_WATCHDOG_AVAILABLE()
    static size_t watchdog_handle = watchdog.add("evse_v2_all_data", "EVSE not reachable", 10 * 60 * 1000);
    watchdog.reset(watchdog_handle);
#endif
}
