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

#include "evse.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "web_server.h"
#include "modules.h"

extern uint32_t local_uid_num;
extern bool firmware_update_allowed;

#define SLOT_ACTIVE(x) ((bool)(x & 0x01))
#define SLOT_CLEAR_ON_DISCONNECT(x) ((bool)(x & 0x02))

void EVSE::pre_setup()
{
    // States
    state = Config::Object({
        {"iec61851_state", Config::Uint8(0)},
        {"charger_state", Config::Uint8(0)},
        {"contactor_state", Config::Uint8(0)},
        {"contactor_error", Config::Uint8(0)},
        {"allowed_charging_current", Config::Uint16(0)},
        {"error_state", Config::Uint8(0)},
        {"lock_state", Config::Uint8(0)}
    });

    hardware_configuration = Config::Object({
        {"jumper_configuration", Config::Uint8(0)},
        {"has_lock_switch", Config::Bool(false)},
        {"evse_version", Config::Uint8(0)}
    });

    low_level_state = Config::Object ({
        {"led_state", Config::Uint8(0)},
        {"cp_pwm_duty_cycle", Config::Uint16(0)},
        {"adc_values", Config::Array({
                Config::Uint16(0),
                Config::Uint16(0)
            }, new Config{Config::Uint16(0)}, 2, 2, Config::type_id<Config::ConfUint>())
        },
        {"voltages", Config::Array({
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
            }, new Config{Config::Int16(0)}, 3, 3, Config::type_id<Config::ConfInt>())
        },
        {"resistances", Config::Array({
                Config::Uint32(0),
                Config::Uint32(0),
            }, new Config{Config::Uint32(0)}, 2, 2, Config::type_id<Config::ConfUint>())
        },
        {"gpio", Config::Array({
            Config::Bool(false),
            Config::Bool(false),
            Config::Bool(false),
            Config::Bool(false),
            Config::Bool(false),
            }, new Config{Config::Bool(false)}, 5, 5, Config::type_id<Config::ConfBool>())},
        {"charging_time", Config::Uint32(0)},
        {"time_since_state_change", Config::Uint32(0)},
        {"uptime", Config::Uint32(0)}
    });

    button_state = Config::Object({
        {"button_press_time", Config::Uint32(0)},
        {"button_release_time", Config::Uint32(0)},
        {"button_pressed", Config::Bool(false)},
    });

    Config *evse_charging_slot = new Config{Config::Object({
        {"max_current", Config::Uint8(0)},
        {"active", Config::Bool(false)},
        {"clear_on_disconnect", Config::Bool(false)}
    })};

    slots = Config::Array({},
        evse_charging_slot,
        CHARGING_SLOT_COUNT, CHARGING_SLOT_COUNT,
        Config::type_id<Config::ConfObject>());

    for (int i = 0; i < CHARGING_SLOT_COUNT; ++i)
        slots.add();

    indicator_led = Config::Object({
        {"indication", Config::Int16(0)},
        {"duration", Config::Uint16(0)},
    });

    // Actions

    // TODO indicator LED

    auto_start_charging = Config::Object({
        {"auto_start_charging", Config::Bool(true)}
    });

    auto_start_charging_update = Config::Object({
        {"auto_start_charging", Config::Bool(true)}
    });

    global_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    global_current_update = global_current;

    management_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    management_enabled_update = management_enabled;

    user_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    user_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    user_enabled_update = user_enabled;

    external_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    external_enabled_update = external_enabled;

    external_defaults = Config::Object({
        {"current", Config::Uint16(0)},
        {"clear_on_disconnect", Config::Bool(false)},
    });
    external_defaults_update = external_defaults;

    management_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    management_current_update = management_current;

    external_current = Config::Object({
        {"current", Config::Uint16(32000)}
    });

    external_current_update = external_current;

    external_clear_on_disconnect = Config::Object({
        {"clear_on_disconnect", Config::Bool(false)}
    });

    external_clear_on_disconnect_update = external_clear_on_disconnect;

    user_calibration = Config::Object({
        {"user_calibration_active", Config::Bool(false)},
        {"voltage_diff", Config::Int16(0)},
        {"voltage_mul", Config::Int16(0)},
        {"voltage_div", Config::Int16(0)},
        {"resistance_2700", Config::Int16(0)},
        {"resistance_880", Config::Array({
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
            }, new Config{Config::Int16(0)}, 14, 14, Config::type_id<Config::ConfInt>())}
    });

    modbus_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    modbus_enabled_update = modbus_enabled;

    ocpp_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    ocpp_enabled_update = ocpp_enabled;

    boost_mode = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    boost_mode_update = boost_mode;

    require_meter_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });

    require_meter_enabled_update = require_meter_enabled;
}

bool EVSE::apply_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear)
{
    uint16_t old_current;
    bool old_enabled;
    bool old_clear;
    int rc = tf_evse_get_charging_slot_default(&device, slot, &old_current, &old_enabled, &old_clear);
    if (rc != TF_E_OK) {
        is_in_bootloader(rc);
        logger.printfln("Failed to apply slot default (read failed). rc %d", rc);
        return false;
    }

    if ((old_current == current) && (old_enabled == enabled) && (old_clear == clear))
        return false;

    rc = tf_evse_set_charging_slot_default(&device, slot, current, enabled, clear);
    if (rc != TF_E_OK) {
        is_in_bootloader(rc);
        logger.printfln("Failed to apply slot default (write failed). rc %d", rc);
        return false;
    }
    return true;
}

void EVSE::apply_defaults()
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
    uint16_t global_current;
    bool global_active;
    int rc = tf_evse_get_charging_slot(&device, CHARGING_SLOT_GLOBAL, &global_current, &global_active, nullptr);
    if (rc != TF_E_OK) {
        is_in_bootloader(rc);
        logger.printfln("Failed to apply defaults (global read failed). rc %d", rc);
        return;
    }
    // If this is the first start-up, this slot will not be active.
    // In the old firmwares, the global current was not persistant
    // so setting it to 32000 is expected after start-up.
    if (!global_active)
        global_current = 32000;

    if (this->apply_slot_default(CHARGING_SLOT_GLOBAL, global_current, true, false))
        tf_evse_set_charging_slot(&device, CHARGING_SLOT_GLOBAL, global_current, true, false);

    // Slot 6 (user) depends on user config.
    // It can be enabled per API (is stored in the EVSEs flash, not ours).
    // Set clear to true and current to 0 in any case: If disabled those are ignored anyway.
    bool user_enabled;
    rc = tf_evse_get_charging_slot(&device, CHARGING_SLOT_USER, nullptr, &user_enabled, nullptr);
    if (rc != TF_E_OK) {
        is_in_bootloader(rc);
        logger.printfln("Failed to apply defaults (cm read failed). rc %d", rc);
        return;
    }
    if (this->apply_slot_default(CHARGING_SLOT_USER, 0, user_enabled, true))
        tf_evse_set_charging_slot(&device, CHARGING_SLOT_USER, 0, user_enabled, true);

    // Slot 7 (charge manager) can be enabled per API (is stored in the EVSEs flash, not ours).
    // Set clear to true and current to 0 in any case: If disabled those are ignored anyway.
    bool cm_enabled;
    rc = tf_evse_get_charging_slot(&device, CHARGING_SLOT_CHARGE_MANAGER, nullptr, &cm_enabled, nullptr);
    if (rc != TF_E_OK) {
        is_in_bootloader(rc);
        logger.printfln("Failed to apply defaults (cm read failed). rc %d", rc);
        return;
    }
    if (this->apply_slot_default(CHARGING_SLOT_CHARGE_MANAGER, 0, cm_enabled, true))
        tf_evse_set_charging_slot(&device, CHARGING_SLOT_CHARGE_MANAGER, 0, cm_enabled, true);

    // Slot 8 (external) is controlled via API, no need to change anything here

    // Disabling all unused charging slots.
    for (int i = CHARGING_SLOT_COUNT; i < CHARGING_SLOT_COUNT_SUPPORTED_BY_EVSE; i++) {
        bool active;
        is_in_bootloader(tf_evse_get_charging_slot_default(&device, i, NULL, &active, NULL));
        if (active)
            is_in_bootloader(tf_evse_set_charging_slot_default(&device, i, 32000, false, false));
        is_in_bootloader(tf_evse_set_charging_slot_active(&device, i, false));
    }
}

void EVSE::factory_reset()
{
    tf_evse_factory_reset(&device, 0x2342FACD);
}

void EVSE::set_data_storage(uint8_t page, const uint8_t *data)
{
    tf_evse_set_data_storage(&device, page, data);
}

void EVSE::get_data_storage(uint8_t page, uint8_t *data)
{
    tf_evse_get_data_storage(&device, page, data);
}

void EVSE::set_indicator_led(int16_t indication, uint16_t duration, uint8_t *ret_status)
{
    tf_evse_set_indicator_led(&device, indication, duration, ret_status);
}

void EVSE::setup()
{
    setup_evse();
    if (!device_found)
        return;

    // Get all data once before announcing the EVSE feature.
    update_all_data();
    api.addFeature("evse");

    task_scheduler.scheduleWithFixedDelay([this](){
        update_all_data();
    }, 0, 250);
}

String EVSE::get_evse_debug_header()
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
           "HARDWARE CONFIG,"
           "jumper_configuration,"
           "has_lock_switch,"
           "evse_version,"
           "LL-State,"
           "led_state,"
           "cp_pwm_duty_cycle,"
           "charging_time,"
           "time_since_state_change,"
           "uptime,"
           "ADC VALUES,"
           "CP/PE,"
           "PP/PE,"
           "VOLTAGES,"
           "CP/PE,"
           "PP/PE,"
           "CP/PE (High),"
           "RESISTANCES,"
           "CP/PE,"
           "PP/PE,"
           "GPIOs,"
           "Input (0),"
           "Output (1),"
           "Motor Input (2),"
           "Relay (3),"
           "Motor Fault (4),"
           "SLOTS,"
           "incoming_cable,"
           "outgoing_cable,"
           "shutdown_input,"
           "gp_input,"
           "autostart_button,"
           "global,"
           "user,"
           "charge_manager,"
           "external,"
           "modbus_tcp,"
           "modbus_tcp_enable,"
           "ocpp,"
           "charge_limits,"
           "require_meter,"
           "unused (14),"
           "unused (15),"
           "unused (16),"
           "unused (17),"
           "unused (18),"
           "unused (19),"
           "\"";
}

String EVSE::get_evse_debug_line()
{
    uint8_t iec61851_state;
    uint8_t charger_state;
    uint8_t contactor_state;
    uint8_t contactor_error;
    uint16_t allowed_charging_current;
    uint8_t error_state;
    uint8_t lock_state;
    uint8_t jumper_configuration;
    bool has_lock_switch;
    uint8_t evse_version;

    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;
    uint16_t adc_values[2];
    int16_t voltages[3];
    uint32_t resistances[2];
    bool gpio[5];
    uint32_t charging_time;
    uint32_t time_since_state_change;
    uint32_t uptime;

    // get_all_charging_slots - 60 byte
    uint16_t max_current[20];
    uint8_t active_and_clear_on_disconnect[20];

    int rc = tf_evse_get_all_data_1(&device,
                                    &iec61851_state,
                                    &charger_state,
                                    &contactor_state,
                                    &contactor_error,
                                    &allowed_charging_current,
                                    &error_state,
                                    &lock_state,
                                    &jumper_configuration,
                                    &has_lock_switch,
                                    &evse_version,
                                    &led_state,
                                    &cp_pwm_duty_cycle,
                                    adc_values,
                                    voltages,
                                    resistances,
                                    gpio,
                                    &charging_time,
                                    &time_since_state_change,
                                    &uptime,
                                    // We don't care about the led and button state here. TODO: do we really not?
                                    nullptr,
                                    nullptr,
                                    nullptr,
                                    nullptr,
                                    nullptr,
                                    nullptr);

    if (rc != TF_E_OK) {
        logger.printfln("get_all_data_1 %d", rc);
        is_in_bootloader(rc);
        return "get_all_data_1 failed";
    }

    rc = tf_evse_get_all_charging_slots(&device, max_current, active_and_clear_on_disconnect);

    if (rc != TF_E_OK) {
        logger.printfln("slots %d", rc);
        is_in_bootloader(rc);
        return "get_all_charging_slots failed";
    }

    // Currently max ~ 290
    char line[512] = {0};
    snprintf(line,
             sizeof(line) / sizeof(line[0]),
             "\"%lu,,"
             "%u,%u,%u,%u,%u,%u,%u,,"
             "%u,%c,%u,,"
             "%u,%u,%u,%u,%u,,"
             "%u,%u,,"
             "%d,%d,%d,,"
             "%u,%u,,"
             "%c,%c,%c,%c,%c,,"
             "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\"",
             millis(),
             iec61851_state,
             charger_state,
             contactor_state,
             contactor_error,
             allowed_charging_current,
             error_state,
             lock_state,

             jumper_configuration,
             has_lock_switch ? '1' : '0',
             evse_version,

             led_state,
             cp_pwm_duty_cycle,
             charging_time,
             time_since_state_change,
             uptime,

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
             gpio[4] ? '1' : '0',

             SLOT_ACTIVE(active_and_clear_on_disconnect[0]) ? max_current[0] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[1]) ? max_current[1] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[2]) ? max_current[2] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[3]) ? max_current[3] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[4]) ? max_current[4] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[5]) ? max_current[5] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[6]) ? max_current[6] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[7]) ? max_current[7] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[8]) ? max_current[8] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[9]) ? max_current[9] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[10]) ? max_current[10] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[11]) ? max_current[11] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[12]) ? max_current[12] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[13]) ? max_current[13] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[14]) ? max_current[14] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[15]) ? max_current[15] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[16]) ? max_current[16] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[17]) ? max_current[17] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[18]) ? max_current[18] : 32000,
             SLOT_ACTIVE(active_and_clear_on_disconnect[19]) ? max_current[19] : 32000);
    return String(line);
}

void EVSE::set_managed_current(uint16_t current)
{
    is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_CHARGE_MANAGER, current));
    this->last_current_update = millis();
    this->shutdown_logged = false;
}

void EVSE::set_user_current(uint16_t current)
{
    is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_USER, current));
}

void EVSE::set_modbus_current(uint16_t current)
{
    is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_MODBUS_TCP, current));
}

void EVSE::set_modbus_enabled(bool enabled)
{
    is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_MODBUS_TCP_ENABLE, enabled ? 32000 : 0));
}


void EVSE::set_charge_limits_slot(uint16_t current, bool enabled)
{
    is_in_bootloader(tf_evse_set_charging_slot(&device, CHARGING_SLOT_CHARGE_LIMITS, current, enabled, true));
}
/*
void EVSE::set_charge_time_restriction_slot(uint16_t current, bool enabled)
{
    is_in_bootloader(tf_evse_set_charging_slot(&device, CHARGING_SLOT_TIME_RESTRICTION, current, enabled, true));
}
*/
void EVSE::set_ocpp_current(uint16_t current)
{
     is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_OCPP, current));
}

uint16_t EVSE::get_ocpp_current()
{
    return slots.get(CHARGING_SLOT_OCPP)->get("max_current")->asUint();
}

void EVSE::set_require_meter_blocking(bool blocking) {
    is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_REQUIRE_METER, blocking ? 0 : 32000));
}

void EVSE::set_require_meter_enabled(bool enabled) {
    if (!initialized)
        return;

    apply_slot_default(CHARGING_SLOT_REQUIRE_METER, 0, enabled, false);
    is_in_bootloader(tf_evse_set_charging_slot_active(&device, CHARGING_SLOT_REQUIRE_METER, enabled));
}

bool EVSE::get_require_meter_blocking() {
    uint16_t current = 0;
    bool enabled = get_require_meter_enabled();
    if (!enabled)
        return false;

    is_in_bootloader(tf_evse_get_charging_slot(&device, CHARGING_SLOT_REQUIRE_METER, &current, &enabled, nullptr));
    return enabled && current == 0;
}

bool EVSE::get_require_meter_enabled() {
    return require_meter_enabled.get("enabled")->asBool();
}

void EVSE::check_debug()
{
    task_scheduler.scheduleOnce([this](){
        if (deadline_elapsed(last_debug_check + 60000) && debug == true)
        {
            logger.printfln("Debug log creation canceled because no continue call was received for more than 60 seconds.");
            debug = false;
        }
        else if (debug == true)
            check_debug();
    }, 70000);
}

void EVSE::register_urls()
{
#if MODULE_CM_NETWORKING_AVAILABLE()
    cm_networking.register_client([this](uint16_t current, bool cp_disconnect_requested) {
        set_managed_current(current);

        (void)cp_disconnect_requested; // not supported
    });

    task_scheduler.scheduleWithFixedDelay([this](){
        uint16_t supported_current = 32000;
        for(int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
            if (i == CHARGING_SLOT_CHARGE_MANAGER)
                continue;
            if (!slots.get(i)->get("active")->asBool())
                continue;
            supported_current = min(supported_current, (uint16_t)slots.get(i)->get("max_current")->asUint());
        }

        cm_networking.send_client_update(
            local_uid_num,
            state.get("iec61851_state")->asUint(),
            state.get("charger_state")->asUint(),
            low_level_state.get("time_since_state_change")->asUint(),
            state.get("error_state")->asUint(),
            low_level_state.get("uptime")->asUint(),
            low_level_state.get("charging_time")->asUint(),
            slots.get(CHARGING_SLOT_CHARGE_MANAGER)->get("max_current")->asUint(),
            supported_current,
            management_enabled.get("enabled")->asBool(),
            false // CP disconnect not supported
        );
    }, 1000, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (!deadline_elapsed(this->last_current_update + 30000))
            return;
        if (!management_enabled.get("enabled")->asBool()) {
            // Push back the next check for 30 seconds: If managed gets enabled,
            // we want to wait 30 seconds before setting the current for the first time.
            this->last_current_update = millis();
            return;
        }
        if(!this->shutdown_logged)
            logger.printfln("Got no managed current update for more than 30 seconds. Setting managed current to 0");
        this->shutdown_logged = true;
        is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_CHARGE_MANAGER, 0));
    }, 1000, 1000);
#endif

    // States
    api.addState("evse/state", &state, {}, 1000);
    api.addState("evse/hardware_configuration", &hardware_configuration, {}, 1000);
    api.addState("evse/low_level_state", &low_level_state, {}, 1000);
    api.addState("evse/button_state", &button_state, {}, 250);
    api.addState("evse/slots", &slots, {}, 1000);
    api.addState("evse/indicator_led", &indicator_led, {}, 1000);

    api.addCommand("evse/stop_charging", Config::Null(), {}, [this](){
        if (state.get("iec61851_state")->asUint() != IEC_STATE_A)
            is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_AUTOSTART_BUTTON, 0));
    }, true);

    api.addCommand("evse/start_charging", Config::Null(), {}, [this](){
        if (state.get("iec61851_state")->asUint() != IEC_STATE_A)
            is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_AUTOSTART_BUTTON, 32000));
    }, true);

#if MODULE_WS_AVAILABLE()
    server.on("/evse/start_debug", HTTP_GET, [this](WebServerRequest request) {
        task_scheduler.scheduleOnce([this](){
            last_debug_check = millis();
            check_debug();
            ws.pushRawStateUpdate(this->get_evse_debug_header(), "evse/debug_header");
            debug = true;
        }, 0);
        return request.send(200);
    });

    server.on("/evse/continue_debug", HTTP_GET, [this](WebServerRequest request) {
        last_debug_check = millis();
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

    api.addState("evse/external_current", &external_current, {}, 1000);
    api.addCommand("evse/external_current_update", &external_current_update, {}, [this](){
        tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_EXTERNAL, external_current_update.get("current")->asUint());
    }, false);

    api.addState("evse/external_clear_on_disconnect", &external_clear_on_disconnect, {}, 1000);
    api.addCommand("evse/external_clear_on_disconnect_update", &external_clear_on_disconnect_update, {}, [this](){
        is_in_bootloader(tf_evse_set_charging_slot_clear_on_disconnect(&device, CHARGING_SLOT_EXTERNAL, external_clear_on_disconnect_update.get("clear_on_disconnect")->asBool()));
    }, false);

    api.addState("evse/management_current", &management_current, {}, 1000);
    api.addCommand("evse/management_current_update", &management_current_update, {}, [this](){
        this->set_managed_current(management_current_update.get("current")->asUint());
    }, false);

    api.addState("evse/boost_mode", &boost_mode, {}, 1000);
    api.addCommand("evse/boost_mode_update", &boost_mode_update, {}, [this](){
        logger.printfln("Setting boost mode to %s", boost_mode_update.get("enabled")->asBool() ? "enabled" : "disabled");
        int rc = tf_evse_set_boost_mode(&device, boost_mode_update.get("enabled")->asBool());
        logger.printfln("rc %d", rc),
        is_in_bootloader(rc);
    }, true);

    // Configurations. Note that those are _not_ configs in the api.addPersistentConfig sense:
    // The configs are stored on the EVSE itself, not the ESP's flash.
    // All _update APIs that write the EVSEs flash without checking first if this was a change
    // are marked as actions to make sure the flash is not written unnecessarily.
    api.addState("evse/auto_start_charging", &auto_start_charging, {}, 1000);
    api.addCommand("evse/auto_start_charging_update", &auto_start_charging_update, {}, [this](){
        // 1. set auto start
        // 2. make persistent
        // 3. fake a start/stop charging

        bool enable_auto_start = auto_start_charging_update.get("auto_start_charging")->asBool();

        is_in_bootloader(tf_evse_set_charging_slot_clear_on_disconnect(&device, CHARGING_SLOT_AUTOSTART_BUTTON, !enable_auto_start));

        if (enable_auto_start) {
            this->apply_slot_default(CHARGING_SLOT_AUTOSTART_BUTTON, 32000, true, false);
        } else {
            this->apply_slot_default(CHARGING_SLOT_AUTOSTART_BUTTON, 0, true, true);
        }

        if (enable_auto_start) {
            is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_AUTOSTART_BUTTON, 32000));
        }
        else {
            // Only "stop" charging if no car is currently plugged in.
            // Clear on disconnect only triggers once, so we have to zero the current manually here.
            uint8_t iec_state = state.get("iec61851_state")->asUint();
            if (iec_state != 2 && iec_state != 3)
                is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_AUTOSTART_BUTTON, 0));
        }
    }, false);

    api.addState("evse/global_current", &global_current, {}, 1000);
    api.addCommand("evse/global_current_update", &global_current_update, {}, [this](){
        uint16_t current = global_current_update.get("current")->asUint();
        is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, CHARGING_SLOT_GLOBAL, current));
        this->apply_slot_default(CHARGING_SLOT_GLOBAL, current, true, false);
    }, false);

    api.addState("evse/management_enabled", &management_enabled, {}, 1000);
    api.addCommand("evse/management_enabled_update", &management_enabled_update, {}, [this](){
        bool enabled = management_enabled_update.get("enabled")->asBool();

        if (enabled == management_enabled.get("enabled")->asBool())
            return;

        if (enabled)
            tf_evse_set_charging_slot(&device, CHARGING_SLOT_CHARGE_MANAGER, 0, true, true);
        else
            tf_evse_set_charging_slot(&device, CHARGING_SLOT_CHARGE_MANAGER, 32000, false, false);

        if (enabled)
            this->apply_slot_default(CHARGING_SLOT_CHARGE_MANAGER, 0, true, true);
        else
            this->apply_slot_default(CHARGING_SLOT_CHARGE_MANAGER, 32000, false, false);
    }, false);

    api.addState("evse/user_current", &user_current, {}, 1000);
    api.addState("evse/user_enabled", &user_enabled, {}, 1000);
    api.addCommand("evse/user_enabled_update", &user_enabled_update, {}, [this](){
        bool enabled = user_enabled_update.get("enabled")->asBool();

        if (enabled == user_enabled.get("enabled")->asBool())
            return;

#if MODULE_USERS_AVAILABLE()
        if (enabled) {
            users.stop_charging(0, true);
        }
#endif

        if (enabled)
            tf_evse_set_charging_slot(&device, CHARGING_SLOT_USER, 0, true, true);
        else
            tf_evse_set_charging_slot(&device, CHARGING_SLOT_USER, 32000, false, false);

        if (enabled)
            this->apply_slot_default(CHARGING_SLOT_USER, 0, true, true);
        else
            this->apply_slot_default(CHARGING_SLOT_USER, 32000, false, false);
    }, false);

    api.addState("evse/external_enabled", &external_enabled, {}, 1000);
    api.addCommand("evse/external_enabled_update", &external_enabled_update, {}, [this](){
        bool enabled = external_enabled_update.get("enabled")->asBool();

        if (enabled == external_enabled.get("enabled")->asBool())
            return;

        tf_evse_set_charging_slot_active(&device, CHARGING_SLOT_EXTERNAL, enabled);
        this->apply_slot_default(CHARGING_SLOT_EXTERNAL, 32000, enabled, false);
    }, false);

    api.addState("evse/external_defaults", &external_defaults, {}, 1000);
    api.addCommand("evse/external_defaults_update", &external_defaults_update, {}, [this](){
        bool enabled;
        tf_evse_get_charging_slot_default(&device, CHARGING_SLOT_EXTERNAL, nullptr, &enabled, nullptr);

        this->apply_slot_default(CHARGING_SLOT_EXTERNAL, external_defaults_update.get("current")->asUint(), enabled, external_defaults_update.get("clear_on_disconnect")->asBool());
    }, false);

    api.addState("evse/modbus_tcp_enabled", &modbus_enabled, {}, 1000);
    api.addCommand("evse/modbus_tcp_enabled_update", &modbus_enabled_update, {}, [this](){
        bool enabled = modbus_enabled_update.get("enabled")->asBool();

        if (enabled == modbus_enabled.get("enabled")->asBool())
            return;

        if (enabled) {
            tf_evse_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP, 32000, true, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP, 32000, true, false);

            tf_evse_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, true, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, true, false);
        }
        else {
            tf_evse_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP, 32000, false, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP, 32000, false, false);

            tf_evse_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, false, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, false, false);
        }
    }, false);

    api.addState("evse/ocpp_enabled", &ocpp_enabled, {}, 1000);
    api.addCommand("evse/ocpp_enabled_update", &ocpp_enabled_update, {}, [this](){
        bool enabled = ocpp_enabled_update.get("enabled")->asBool();

        if (enabled == ocpp_enabled.get("enabled")->asBool())
            return;

        if (enabled) {
            tf_evse_set_charging_slot(&device, CHARGING_SLOT_OCPP, 32000, true, false);
            this->apply_slot_default(CHARGING_SLOT_OCPP, 32000, true, false);
        }
        else {
            tf_evse_set_charging_slot(&device, CHARGING_SLOT_OCPP, 32000, false, false);
            this->apply_slot_default(CHARGING_SLOT_OCPP, 32000, false, false);
        }
    }, false);

    api.addState("evse/user_calibration", &user_calibration, {}, 1000);
    api.addCommand("evse/user_calibration_update", &user_calibration, {}, [this](){
        int16_t resistance_880[14];
        user_calibration.get("resistance_880")->fillArray<int16_t, Config::ConfInt>(resistance_880, sizeof(resistance_880)/sizeof(resistance_880[0]));

        is_in_bootloader(tf_evse_set_user_calibration(&device,
            0xCA11B4A0,
            user_calibration.get("user_calibration_active")->asBool(),
            user_calibration.get("voltage_diff")->asInt(),
            user_calibration.get("voltage_mul")->asInt(),
            user_calibration.get("voltage_div")->asInt(),
            user_calibration.get("resistance_2700")->asInt(),
            resistance_880
            ));
    }, true);

    this->DeviceModule::register_urls();
}

void EVSE::loop()
{
    this->DeviceModule::loop();

#if MODULE_WS_AVAILABLE()
    static uint32_t last_debug = 0;
    if (debug && deadline_elapsed(last_debug + 50)) {
        last_debug = millis();
        ws.pushRawStateUpdate(this->get_evse_debug_line(), "evse/debug");
    }
#endif
}

void EVSE::setup_evse()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    this->apply_defaults();
    initialized = true;
}

void EVSE::update_all_data()
{
    if (!initialized)
        return;

    // get_all_data_1 - 51 byte
    uint8_t iec61851_state;
    uint8_t charger_state;
    uint8_t contactor_state;
    uint8_t contactor_error;
    uint16_t allowed_charging_current;
    uint8_t error_state;
    uint8_t lock_state;
    uint8_t jumper_configuration;
    bool has_lock_switch;
    uint8_t evse_version;
    bool boost_mode_enabled;

    // get_all_data_2 - 18 byte
    int16_t indication;
    uint16_t duration;
    uint32_t button_press_time;
    uint32_t button_release_time;
    bool button_pressed;

    // get_low_level_state - 57 byte
    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;
    uint16_t adc_values[2];
    int16_t voltages[3];
    uint32_t resistances[2];
    bool gpio[5];
    uint32_t charging_time;
    uint32_t time_since_state_change;
    uint32_t uptime;

    // get_all_charging_slots - 60 byte
    uint16_t max_current[20];
    uint8_t active_and_clear_on_disconnect[20];

    // get_user_calibration - 37 byte
    bool user_calibration_active;
    int16_t voltage_diff;
    int16_t voltage_mul;
    int16_t voltage_div;
    int16_t resistance_2700;
    int16_t resistance_880[14];

    int rc = tf_evse_get_all_data_1(&device,
                                    &iec61851_state,
                                    &charger_state,
                                    &contactor_state,
                                    &contactor_error,
                                    &allowed_charging_current,
                                    &error_state,
                                    &lock_state,
                                    &jumper_configuration,
                                    &has_lock_switch,
                                    &evse_version,
                                    &led_state,
                                    &cp_pwm_duty_cycle,
                                    adc_values,
                                    voltages,
                                    resistances,
                                    gpio,
                                    &charging_time,
                                    &time_since_state_change,
                                    &uptime,
                                    &indication,
                                    &duration,
                                    &button_press_time,
                                    &button_release_time,
                                    &button_pressed,
                                    &boost_mode_enabled);

    if (rc != TF_E_OK) {
        logger.printfln("all_data_1 %d", rc);
        is_in_bootloader(rc);
        return;
    }

    rc = tf_evse_get_all_charging_slots(&device, max_current, active_and_clear_on_disconnect);

    if (rc != TF_E_OK) {
        logger.printfln("slots %d", rc);
        is_in_bootloader(rc);
        return;
    }

    uint16_t external_default_current;
    bool external_default_enabled;
    bool external_default_clear_on_disconnect;

    rc = tf_evse_get_charging_slot_default(&device,
                                           CHARGING_SLOT_EXTERNAL,
                                           &external_default_current,
                                           &external_default_enabled,
                                           &external_default_clear_on_disconnect);

    if (rc != TF_E_OK) {
        logger.printfln("external slot default %d", rc);
        is_in_bootloader(rc);
        return;
    }

    rc = tf_evse_get_user_calibration(&device,
                                      &user_calibration_active,
                                      &voltage_diff,
                                      &voltage_mul,
                                      &voltage_div,
                                      &resistance_2700,
                                      resistance_880);

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

    state.get("iec61851_state")->updateUint(iec61851_state);
    state.get("charger_state")->updateUint(charger_state);
    state.get("contactor_state")->updateUint(contactor_state);
    bool contactor_error_changed = state.get("contactor_error")->updateUint(contactor_error);
    state.get("allowed_charging_current")->updateUint(allowed_charging_current);
    bool error_state_changed = state.get("error_state")->updateUint(error_state);
    state.get("lock_state")->updateUint(lock_state);

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

    // get_hardware_configuration
    hardware_configuration.get("jumper_configuration")->updateUint(jumper_configuration);
    hardware_configuration.get("has_lock_switch")->updateBool(has_lock_switch);
    hardware_configuration.get("evse_version")->updateUint(evse_version);

    // get_low_level_state
    low_level_state.get("led_state")->updateUint(led_state);
    low_level_state.get("cp_pwm_duty_cycle")->updateUint(cp_pwm_duty_cycle);

    for (int i = 0; i < sizeof(adc_values) / sizeof(adc_values[0]); ++i)
        low_level_state.get("adc_values")->get(i)->updateUint(adc_values[i]);

    for (int i = 0; i < sizeof(voltages) / sizeof(voltages[0]); ++i)
        low_level_state.get("voltages")->get(i)->updateInt(voltages[i]);

    for (int i = 0; i < sizeof(resistances) / sizeof(resistances[0]); ++i)
        low_level_state.get("resistances")->get(i)->updateUint(resistances[i]);

    for (int i = 0; i < sizeof(gpio) / sizeof(gpio[0]); ++i)
        low_level_state.get("gpio")->get(i)->updateBool(gpio[i]);

    low_level_state.get("charging_time")->updateUint(charging_time);
    low_level_state.get("time_since_state_change")->updateUint(time_since_state_change);
    low_level_state.get("uptime")->updateUint(uptime);

    for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
        slots.get(i)->get("max_current")->updateUint(max_current[i]);
        slots.get(i)->get("active")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[i]));
        slots.get(i)->get("clear_on_disconnect")->updateBool(SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[i]));
    }

    auto_start_charging.get("auto_start_charging")->updateBool(
        !slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("clear_on_disconnect")->asBool());

    // get_button_state
    button_state.get("button_press_time")->updateUint(button_press_time);
    button_state.get("button_release_time")->updateUint(button_release_time);
    button_state.get("button_pressed")->updateBool(button_pressed);

    boost_mode.get("enabled")->updateBool(boost_mode_enabled);

    // get_indicator_led
    indicator_led.get("indication")->updateInt(indication);
    indicator_led.get("duration")->updateUint(duration);

    auto_start_charging.get("auto_start_charging")->updateBool(!SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[CHARGING_SLOT_AUTOSTART_BUTTON]));

    management_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_CHARGE_MANAGER]));

    user_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_USER]));

    modbus_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_MODBUS_TCP]));
    ocpp_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_OCPP]));

    external_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_EXTERNAL]));
    external_clear_on_disconnect.get("clear_on_disconnect")->updateBool(SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[CHARGING_SLOT_EXTERNAL]));

    global_current.get("current")->updateUint(max_current[CHARGING_SLOT_GLOBAL]);
    management_current.get("current")->updateUint(max_current[CHARGING_SLOT_CHARGE_MANAGER]);
    external_current.get("current")->updateUint(max_current[CHARGING_SLOT_EXTERNAL]);
    user_current.get("current")->updateUint(max_current[CHARGING_SLOT_USER]);

    external_defaults.get("current")->updateUint(external_default_current);
    external_defaults.get("clear_on_disconnect")->updateBool(external_default_clear_on_disconnect);

    require_meter_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_REQUIRE_METER]));

    // get_user_calibration
    user_calibration.get("user_calibration_active")->updateBool(user_calibration_active);
    user_calibration.get("voltage_diff")->updateInt(voltage_diff);
    user_calibration.get("voltage_mul")->updateInt(voltage_mul);
    user_calibration.get("voltage_div")->updateInt(voltage_div);
    user_calibration.get("resistance_2700")->updateInt(resistance_2700);

    for (int i = 0; i < sizeof(resistance_880) / sizeof(resistance_880[0]); ++i)
        user_calibration.get("resistance_880")->get(i)->updateInt(resistance_880[i]);

#if MODULE_WATCHDOG_AVAILABLE()
    static size_t watchdog_handle = watchdog.add("evse_all_data", "EVSE not reachable");
    watchdog.reset(watchdog_handle);
#endif
}
