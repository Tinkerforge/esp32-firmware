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

}

void EVSE::post_setup() {}

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
           "unused (19)"
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

int EVSE::get_charging_slot(uint8_t slot, uint16_t *ret_current, bool *ret_enabled, bool *ret_reset_on_dc) {
    return tf_evse_get_charging_slot(&device, slot, ret_current, ret_enabled, ret_reset_on_dc);
}

int EVSE::set_charging_slot(uint8_t slot, uint16_t current, bool enabled, bool reset_on_dc) {
    return tf_evse_set_charging_slot(&device, slot, current, enabled, reset_on_dc);
}

bool EVSE::is_in_bootloader(int rc) {
    return DeviceModule::is_in_bootloader(rc);
}

uint16_t EVSE::get_ocpp_current()
{
    return slots.get(CHARGING_SLOT_OCPP)->get("max_current")->asUint();
}

void EVSE::check_debug()
{
    task_scheduler.scheduleOnce([this](){
        if (deadline_elapsed(last_debug_check + 60000) && debug)
        {
            logger.printfln("Debug log creation canceled because no continue call was received for more than 60 seconds.");
            debug = false;
        }
        else if (debug)
            check_debug();
    }, 70000);
}

void EVSE::set_control_pilot_disconnect(bool cp_disconnect, bool *cp_disconnected) {
    (void)cp_disconnect; // not supported
    (void)cp_disconnected;
}

void EVSE::set_charging_slot_max_current(uint8_t slot, uint16_t current) {
    is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, slot, current));
}

void EVSE::set_charging_slot_clear_on_disconnect(uint8_t slot, bool clear_on_disconnect) {
    is_in_bootloader(tf_evse_set_charging_slot_clear_on_disconnect(&device, slot, clear_on_disconnect));
}

void EVSE::set_boost_mode(bool enabled) {
    is_in_bootloader(tf_evse_set_boost_mode(&device, enabled));
}

void EVSE::set_charging_slot_active(uint8_t slot, bool enabled) {
    tf_evse_set_charging_slot_active(&device, slot, enabled);
}

int EVSE::get_charging_slot_default(uint8_t slot, uint16_t *ret_max_current, bool *ret_enabled, bool *ret_clear_on_disconnect) {
    return tf_evse_get_charging_slot_default(&device, slot, ret_max_current, ret_enabled, ret_clear_on_disconnect);
}

int EVSE::set_charging_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear_on_disconnect) {
    return tf_evse_set_charging_slot_default(&device, slot, current, enabled, clear_on_disconnect);
}

bool EVSE::get_control_pilot_disconnect() {
    return false; //cp-disconnect not supported
}

void EVSE::register_urls()
{
    // TODO: indicator led update as API?

    this->DeviceModule::register_urls();
}

void EVSE::post_register_urls() {
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
}

void EVSE::loop()
{
    this->DeviceModule::loop();
}

void EVSE::setup_evse()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    evse_common.apply_defaults();
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
