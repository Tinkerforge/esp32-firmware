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

#include "evse_v2.h"

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

void EVSEV2::pre_setup()
{
    // States
    state = Config::Object({
        {"iec61851_state", Config::Uint8(0)},
        {"charger_state", Config::Uint8(0)},
        {"contactor_state", Config::Uint8(0)},
        {"contactor_error", Config::Uint8(0)},
        {"allowed_charging_current", Config::Uint16(0)},
        {"error_state", Config::Uint8(0)},
        {"lock_state", Config::Uint8(0)},
        {"dc_fault_current_state", Config::Uint8(0)},
    });

    hardware_configuration = Config::Object({
        {"jumper_configuration", Config::Uint8(0)},
        {"has_lock_switch", Config::Bool(false)},
        {"evse_version", Config::Uint8(0)},
        {"energy_meter_type", Config::Uint8(0)}
    });

    low_level_state = Config::Object ({
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
        {"uptime", Config::Uint32(0)},
        {"time_since_dc_fault_check", Config::Uint32(0)}
    });

    energy_meter_values = Config::Object({
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

    energy_meter_errors = Config::Object({
        {"local_timeout", Config::Uint32(0)},
        {"global_timeout", Config::Uint32(0)},
        {"illegal_function", Config::Uint32(0)},
        {"illegal_data_access", Config::Uint32(0)},
        {"illegal_data_value", Config::Uint32(0)},
        {"slave_device_failure", Config::Uint32(0)},
    });

    button_state = Config::Object({
        {"button_press_time", Config::Uint32(0)},
        {"button_release_time", Config::Uint32(0)},
        {"button_pressed", Config::Bool(false)},
    });

    Config *evse_charging_slot = new Config{Config::Object({
        {"max_current", Config::Uint32(0)},
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

    reset_dc_fault_current_state = Config::Object({
        {"password", Config::Uint32(0)} // 0xDC42FA23
    });

    // TODO indicator LED


    // EVSE configs
    gpio_configuration = Config::Object({
        {"shutdown_input", Config::Uint8(0)},
        {"input", Config::Uint8(0)},
        {"output", Config::Uint8(0)}
    });

    gpio_configuration_update = gpio_configuration;

    button_configuration = Config::Object({
        {"button", Config::Uint8(2)}
    });

    button_configuration_update = Config::Object({
        {"button", Config::Uint8(2)}
    });

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

    modbus_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    modbus_enabled_update = modbus_enabled;

    ocpp_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    ocpp_enabled_update = ocpp_enabled;

    ev_wakeup = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    ev_wakeup_update = ev_wakeup;

    boost_mode = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    boost_mode_update = boost_mode;

    control_pilot_disconnect = Config::Object({
        {"disconnect", Config::Bool(false)}
    });

    control_pilot_disconnect_update = control_pilot_disconnect;

    require_meter_enabled = Config::Object({
        {"enabled", Config::Bool(false)}
    });

    require_meter_enabled_update = require_meter_enabled;

    gp_output = Config::Object({
        {"gp_output", Config::Uint(0, 0, 1)}
    });

    gp_output_update = gp_output;
}

bool EVSEV2::apply_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear)
{
    uint16_t old_current;
    bool old_enabled;
    bool old_clear;
    int rc = tf_evse_v2_get_charging_slot_default(&device, slot, &old_current, &old_enabled, &old_clear);
    if (rc != TF_E_OK) {
        is_in_bootloader(rc);
        logger.printfln("Failed to apply slot default (read failed). rc %d", rc);
        return false;
    }

    if ((old_current == current) && (old_enabled == enabled) && (old_clear == clear))
        return false;

    rc = tf_evse_v2_set_charging_slot_default(&device, slot, current, enabled, clear);
    if (rc != TF_E_OK) {
        is_in_bootloader(rc);
        logger.printfln("Failed to apply slot default (write failed). rc %d", rc);
        return false;
    }
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
    uint16_t global_current;
    bool global_active;
    int rc = tf_evse_v2_get_charging_slot(&device, CHARGING_SLOT_GLOBAL, &global_current, &global_active, nullptr);
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
        tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_GLOBAL, global_current, true, false);

    // Slot 6 (user) depends on user config.
    // It can be enabled per API (is stored in the EVSEs flash, not ours).
    // Set clear to true and current to 0 in any case: If disabled those are ignored anyway.
    bool user_enabled;
    rc = tf_evse_v2_get_charging_slot(&device, CHARGING_SLOT_USER, nullptr, &user_enabled, nullptr);
    if (rc != TF_E_OK) {
        is_in_bootloader(rc);
        logger.printfln("Failed to apply defaults (cm read failed). rc %d", rc);
        return;
    }
    if (this->apply_slot_default(CHARGING_SLOT_USER, 0, user_enabled, true))
        tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_USER, 0, user_enabled, true);

    // Slot 7 (charge manager) can be enabled per API (is stored in the EVSEs flash, not ours).
    // Set clear to true and current to 0 in any case: If disabled those are ignored anyway.
    bool cm_enabled;
    rc = tf_evse_v2_get_charging_slot(&device, CHARGING_SLOT_CHARGE_MANAGER, nullptr, &cm_enabled, nullptr);
    if (rc != TF_E_OK) {
        is_in_bootloader(rc);
        logger.printfln("Failed to apply defaults (cm read failed). rc %d", rc);
        return;
    }
    if (this->apply_slot_default(CHARGING_SLOT_CHARGE_MANAGER, 0, cm_enabled, true))
        tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_CHARGE_MANAGER, 0, cm_enabled, true);

    // Slot 8 (external) is controlled via API, no need to change anything here

    // Disabling all unused charging slots.
    for (int i = CHARGING_SLOT_COUNT; i < CHARGING_SLOT_COUNT_SUPPORTED_BY_EVSE; i++) {
        bool active;
        is_in_bootloader(tf_evse_v2_get_charging_slot_default(&device, i, NULL, &active, NULL));
        if (active)
            is_in_bootloader(tf_evse_v2_set_charging_slot_default(&device, i, 32000, false, false));
        is_in_bootloader(tf_evse_v2_set_charging_slot_active(&device, i, false));
    }
}

void EVSEV2::set_charge_limits_slot(uint16_t current, bool enabled)
{
    is_in_bootloader(tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_CHARGE_LIMITS, current, enabled, false));
}
/*
void EVSEV2::set_charge_time_restriction_slot(uint16_t current, bool enabled)
{
    is_in_bootloader(tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_TIME_RESTRICTION, current, enabled, true));
}
*/
void EVSEV2::factory_reset()
{
    tf_evse_v2_factory_reset(&device, 0x2342FACD);
}

uint16_t EVSEV2::get_all_energy_meter_values(float *ret_values)
{
    uint16_t len = 0;
    tf_evse_v2_get_all_energy_meter_values(&device, ret_values, &len);
    return len;
}

void EVSEV2::reset_energy_meter_relative_energy()
{
    tf_evse_v2_reset_energy_meter_relative_energy(&device);
}

void EVSEV2::set_data_storage(uint8_t page, const uint8_t *data)
{
    tf_evse_v2_set_data_storage(&device, page, data);
}

void EVSEV2::get_data_storage(uint8_t page, uint8_t *data)
{
    tf_evse_v2_get_data_storage(&device, page, data);
}

void EVSEV2::set_indicator_led(int16_t indication, uint16_t duration, uint8_t *ret_status)
{
    tf_evse_v2_set_indicator_led(&device, indication, duration, ret_status);
}

void EVSEV2::set_require_meter_blocking(bool blocking) {
    is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_REQUIRE_METER, blocking ? 0 : 32000));
}

void EVSEV2::set_require_meter_enabled(bool enabled) {
    if (!initialized)
        return;

    apply_slot_default(CHARGING_SLOT_REQUIRE_METER, 0, enabled, false);
    is_in_bootloader(tf_evse_v2_set_charging_slot_active(&device, CHARGING_SLOT_REQUIRE_METER, enabled));
}

bool EVSEV2::get_require_meter_blocking() {
    uint16_t current = 0;
    bool enabled = get_require_meter_enabled();
    if (!enabled)
        return false;

    is_in_bootloader(tf_evse_v2_get_charging_slot(&device, CHARGING_SLOT_REQUIRE_METER, &current, &enabled, nullptr));
    return enabled && current == 0;
}

bool EVSEV2::get_require_meter_enabled() {
    return require_meter_enabled.get("enabled")->asBool();
}

void EVSEV2::setup()
{
    setup_evse();
    if (!device_found)
        return;

    task_scheduler.scheduleOnce([this](){
        uint32_t press_time = 0;
        tf_evse_v2_get_button_press_boot_time(&device, true, &press_time);
        if (press_time != 0)
            logger.printfln("Reset boot button press time");
    }, 40000);

    // Get all data once before announcing the EVSE feature.
    update_all_data();
    api.addFeature("evse");
    api.addFeature("cp_disconnect");
    api.addFeature("button_configuration");

    task_scheduler.scheduleWithFixedDelay([this](){
        update_all_data();
    }, 0, 250);

    // The EVSE tests the DC fault protector
    // - on boot-up
    // - whenever a car is disconnected
    // - once every 24 hours
    // As the EVSE does not know the current time, we attempt to trigger a test
    // somewhere between 3 am and 4 am. DST shifts typically happen earlier.
    // Manually triggering a test resets the 24 hour counter, so that the
    // EVSE is now roughly synchronized and tests should typically not be
    // observable by the user.
    task_scheduler.scheduleWithFixedDelay([this]() {
        struct timeval tv;
        if (!clock_synced(&tv))
            return;

        struct tm timeinfo;
        localtime_r(&tv.tv_sec, &timeinfo);

        if (timeinfo.tm_hour == 3) {
            tf_evse_v2_trigger_dc_fault_test(&device, 0xDCFAE550, nullptr);
        }
    }, 60 * 1000 /* wait for ntp sync */, 60 * 60 * 1000);
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
           "unused (23),"
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

String EVSEV2::get_evse_debug_line()
{
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

    // get_low_level_state - 61 byte
    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;
    uint16_t adc_values[7];
    int16_t voltages[7];
    uint32_t resistances[2];
    bool gpio[24];
    uint32_t charging_time;
    uint32_t time_since_state_change;
    uint32_t time_since_dc_fault_check;
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
        logger.printfln("get_all_data_1 %d", rc);
        is_in_bootloader(rc);
        return "get_all_data_1 failed";
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
                                        &time_since_dc_fault_check,
                                        &uptime);

    if (rc != TF_E_OK) {
        logger.printfln("ll_state %d", rc);
        is_in_bootloader(rc);
        return "ll_state failed";
    }

    rc = tf_evse_v2_get_all_charging_slots(&device, max_current, active_and_clear_on_disconnect);

    if (rc != TF_E_OK) {
        logger.printfln("slots %d", rc);
        is_in_bootloader(rc);
        return "get_all_charging_slots failed";
    }

    // Currently max ~ 510
    char line[768] = {0};
    snprintf(line,
             sizeof(line) / sizeof(line[0]),
             "\"%lu,,"
             "%u,%u,%u,%u,%u,%u,%u,%u,,"
             "%u,%c,%u,%u,,"
             "%.3f,%.3f,%.3f,%c,%c,%c,%c,%c,%c,,"
             "%u,%u,%u,%u,%u,%u,,"
             "%u,%u,%u,%u,%u,,"
             "%u,%u,%u,%u,%u,%u,%u,,"
             "%d,%d,%d,%d,%d,%d,%d,,"
             "%u,%u,,"
             "%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,,"
             "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\"",
             millis(),
             iec61851_state,
             charger_state,
             contactor_state,
             contactor_error,
             allowed_charging_current,
             error_state,
             lock_state,
             dc_fault_current_state,

             jumper_configuration,
             has_lock_switch ? '1' : '0',
             evse_version,
             energy_meter_type,

             power,
             energy_relative,
             energy_absolute,
             phases_active[0] ? '1' : '0',
             phases_active[1] ? '1' : '0',
             phases_active[2] ? '1' : '0',
             phases_connected[0] ? '1' : '0',
             phases_connected[1] ? '1' : '0',
             phases_connected[2] ? '1' : '0',

             error_count[0],
             error_count[1],
             error_count[2],
             error_count[3],
             error_count[4],
             error_count[5],

             led_state,
             cp_pwm_duty_cycle,
             charging_time,
             time_since_state_change,
             uptime,

             adc_values[0],
             adc_values[1],
             adc_values[2],
             adc_values[3],
             adc_values[4],
             adc_values[5],
             adc_values[6],

             voltages[0],
             voltages[1],
             voltages[2],
             voltages[3],
             voltages[4],
             voltages[5],
             voltages[6],

             resistances[0],
             resistances[1],

             gpio[0] ? '1' : '0',
             gpio[1] ? '1' : '0',
             gpio[2] ? '1' : '0',
             gpio[3] ? '1' : '0',
             gpio[4] ? '1' : '0',
             gpio[5] ? '1' : '0',
             gpio[6] ? '1' : '0',
             gpio[7] ? '1' : '0',
             gpio[8] ? '1' : '0',
             gpio[9] ? '1' : '0',
             gpio[10] ? '1' : '0',
             gpio[11] ? '1' : '0',
             gpio[12] ? '1' : '0',
             gpio[13] ? '1' : '0',
             gpio[14] ? '1' : '0',
             gpio[15] ? '1' : '0',
             gpio[16] ? '1' : '0',
             gpio[17] ? '1' : '0',
             gpio[18] ? '1' : '0',
             gpio[19] ? '1' : '0',
             gpio[20] ? '1' : '0',
             gpio[21] ? '1' : '0',
             gpio[22] ? '1' : '0',
             gpio[23] ? '1' : '0',

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

void EVSEV2::set_managed_current(uint16_t current)
{
    is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_CHARGE_MANAGER, current));
    this->last_current_update = millis();
    this->shutdown_logged = false;
}

void EVSEV2::set_user_current(uint16_t current)
{
    is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_USER, current));
}

void EVSEV2::set_modbus_current(uint16_t current)
{
    is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_MODBUS_TCP, current));
}

void EVSEV2::set_modbus_enabled(bool enabled)
{
    is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_MODBUS_TCP_ENABLE, enabled ? 32000 : 0));
}

void EVSEV2::set_ocpp_current(uint16_t current)
{
     is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_OCPP, current));
}

uint16_t EVSEV2::get_ocpp_current()
{
    return slots.get(CHARGING_SLOT_OCPP)->get("max_current")->asUint();
}

void EVSEV2::check_debug()
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

void EVSEV2::register_urls()
{

#if MODULE_CM_NETWORKING_AVAILABLE()
    cm_networking.register_client([this](uint16_t current, bool cp_disconnect_requested) {
        set_managed_current(current);

        is_in_bootloader(tf_evse_v2_set_control_pilot_disconnect(&device, cp_disconnect_requested, nullptr));
    });

    task_scheduler.scheduleWithFixedDelay([this](){
        uint16_t supported_current = 32000;
        for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
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
            control_pilot_disconnect.get("disconnect")->asBool()
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
        is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_CHARGE_MANAGER, 0));
    }, 1000, 1000);
#endif

    // States
    api.addState("evse/state", &state, {}, 1000);
    api.addState("evse/hardware_configuration", &hardware_configuration, {}, 1000);
    api.addState("evse/low_level_state", &low_level_state, {}, 1000);
    api.addState("evse/button_state", &button_state, {}, 250);
    api.addState("evse/slots", &slots, {}, 1000);
    api.addState("evse/indicator_led", &indicator_led, {}, 1000);

    // Actions
    api.addCommand("evse/reset_dc_fault_current_state", &reset_dc_fault_current_state, {}, [this](){
        is_in_bootloader(tf_evse_v2_reset_dc_fault_current_state(&device, reset_dc_fault_current_state.get("password")->asUint()));
    }, true);

    api.addCommand("evse/stop_charging", Config::Null(), {}, [this](){
        if (state.get("iec61851_state")->asUint() != IEC_STATE_A)
            is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_AUTOSTART_BUTTON, 0));
    }, true);

    api.addCommand("evse/start_charging", Config::Null(), {}, [this](){
        if (state.get("iec61851_state")->asUint() != IEC_STATE_A)
            is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_AUTOSTART_BUTTON, 32000));
    }, true);

    api.addCommand("evse/trigger_dc_fault_test", Config::Null(), {}, [this](){
        if (state.get("iec61851_state")->asUint() != IEC_STATE_A) {
            logger.printfln("Can't trigger DC fault test: IEC state is not A.");
            return;
        }
        bool success = false;
        int rc = tf_evse_v2_trigger_dc_fault_test(&device, 0xDCFAE550, &success);
        if (!success) {
            logger.printfln("Failed to start DC fault test. rc %d", rc);
        }
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
        tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_EXTERNAL, external_current_update.get("current")->asUint());
    }, false);

    api.addState("evse/external_clear_on_disconnect", &external_clear_on_disconnect, {}, 1000);
    api.addCommand("evse/external_clear_on_disconnect_update", &external_clear_on_disconnect_update, {}, [this](){
        is_in_bootloader(tf_evse_v2_set_charging_slot_clear_on_disconnect(&device, CHARGING_SLOT_EXTERNAL, external_clear_on_disconnect_update.get("clear_on_disconnect")->asBool()));
    }, false);

    api.addState("evse/management_current", &management_current, {}, 1000);
    api.addCommand("evse/management_current_update", &management_current_update, {}, [this](){
        this->set_managed_current(management_current_update.get("current")->asUint());
    }, false);

    // Configurations. Note that those are _not_ configs in the api.addPersistentConfig sense:
    // The configs are stored on the EVSE itself, not the ESP's flash.
    // All _update APIs that write the EVSEs flash without checking first if this was a change
    // are marked as actions to make sure the flash is not written unnecessarily.
    api.addState("evse/gpio_configuration", &gpio_configuration, {}, 1000);
    api.addCommand("evse/gpio_configuration_update", &gpio_configuration_update, {}, [this](){
        is_in_bootloader(tf_evse_v2_set_gpio_configuration(&device, gpio_configuration_update.get("shutdown_input")->asUint(),
                                                                    gpio_configuration_update.get("input")->asUint(),
                                                                    gpio_configuration_update.get("output")->asUint()));
    }, true);

    api.addState("evse/button_configuration", &button_configuration, {}, 1000);
    api.addCommand("evse/button_configuration_update", &button_configuration_update, {}, [this](){
        is_in_bootloader(tf_evse_v2_set_button_configuration(&device, button_configuration_update.get("button")->asUint()));
    }, true);

    api.addState("evse/ev_wakeup", &ev_wakeup, {}, 1000);
    api.addCommand("evse/ev_wakeup_update", &ev_wakeup_update, {}, [this](){
        is_in_bootloader(tf_evse_v2_set_ev_wakeup(&device, ev_wakeup_update.get("enabled")->asBool()));
    }, true);

    api.addState("evse/control_pilot_disconnect", &control_pilot_disconnect, {}, 1000);
    api.addCommand("evse/control_pilot_disconnect_update", &control_pilot_disconnect_update, {}, [this](){
        if (management_enabled.get("enabled")->asBool()) { // Disallow updating control pilot configuration if management is enabled because the charge manager will override the CP config every second.
            logger.printfln("evse: Control pilot cannot be (dis)connected by API while charge management is enabled.");
            return;
        }
        is_in_bootloader(tf_evse_v2_set_control_pilot_disconnect(&device, control_pilot_disconnect_update.get("disconnect")->asBool(), nullptr));
    }, true);

    api.addState("evse/boost_mode", &boost_mode, {}, 1000);
    api.addCommand("evse/boost_mode_update", &boost_mode_update, {}, [this](){
        is_in_bootloader(tf_evse_v2_set_boost_mode(&device, boost_mode_update.get("enabled")->asBool()));
    }, true);

    api.addState("evse/auto_start_charging", &auto_start_charging, {}, 1000);
    api.addCommand("evse/auto_start_charging_update", &auto_start_charging_update, {}, [this](){
        // 1. set auto start
        // 2. make persistent
        // 3. fake a start/stop charging

        bool enable_auto_start = auto_start_charging_update.get("auto_start_charging")->asBool();

        is_in_bootloader(tf_evse_v2_set_charging_slot_clear_on_disconnect(&device, CHARGING_SLOT_AUTOSTART_BUTTON, !enable_auto_start));

        if (enable_auto_start) {
            this->apply_slot_default(CHARGING_SLOT_AUTOSTART_BUTTON, 32000, true, false);
        } else {
            this->apply_slot_default(CHARGING_SLOT_AUTOSTART_BUTTON, 0, true, true);
        }

        if (enable_auto_start) {
            is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_AUTOSTART_BUTTON, 32000));
        } else {
            // Only "stop" charging if no car is currently plugged in.
            // Clear on disconnect only triggers once, so we have to zero the current manually here.
            uint8_t iec_state = state.get("iec61851_state")->asUint();
            if (iec_state != 2 && iec_state != 3)
                is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_AUTOSTART_BUTTON, 0));
        }
    }, false);

    api.addState("evse/global_current", &global_current, {}, 1000);
    api.addCommand("evse/global_current_update", &global_current_update, {}, [this](){
        uint16_t current = global_current_update.get("current")->asUint();
        is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, CHARGING_SLOT_GLOBAL, current));
        this->apply_slot_default(CHARGING_SLOT_GLOBAL, current, true, false);
    }, false);

    api.addState("evse/management_enabled", &management_enabled, {}, 1000);
    api.addCommand("evse/management_enabled_update", &management_enabled_update, {}, [this](){
        bool enabled = management_enabled_update.get("enabled")->asBool();

        if (enabled == management_enabled.get("enabled")->asBool())
            return;

        if (enabled)
            tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_CHARGE_MANAGER, 0, true, true);
        else
            tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_CHARGE_MANAGER, 32000, false, false);

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
            tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_USER, 0, true, true);
        else
            tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_USER, 32000, false, false);

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

        tf_evse_v2_set_charging_slot_active(&device, CHARGING_SLOT_EXTERNAL, enabled);
        this->apply_slot_default(CHARGING_SLOT_EXTERNAL, 32000, enabled, false);
    }, false);

    api.addState("evse/external_defaults", &external_defaults, {}, 1000);
    api.addCommand("evse/external_defaults_update", &external_defaults_update, {}, [this](){
        bool enabled;
        tf_evse_v2_get_charging_slot_default(&device, CHARGING_SLOT_EXTERNAL, nullptr, &enabled, nullptr);
        this->apply_slot_default(CHARGING_SLOT_EXTERNAL, external_defaults_update.get("current")->asUint(), enabled, external_defaults_update.get("clear_on_disconnect")->asBool());
    }, false);

    api.addState("evse/modbus_tcp_enabled", &modbus_enabled, {}, 1000);
    api.addCommand("evse/modbus_tcp_enabled_update", &modbus_enabled_update, {}, [this](){
        bool enabled = modbus_enabled_update.get("enabled")->asBool();

        if (enabled == modbus_enabled.get("enabled")->asBool())
            return;

        if (enabled) {
            tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP, 32000, true, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP, 32000, true, false);

            tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, true, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, true, false);
        }
        else {
            tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP, 32000, false, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP, 32000, false, false);

            tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, false, false);
            this->apply_slot_default(CHARGING_SLOT_MODBUS_TCP_ENABLE, 32000, false, false);
        }
    }, false);

    api.addState("evse/ocpp_enabled", &ocpp_enabled, {}, 1000);
    api.addCommand("evse/ocpp_enabled_update", &ocpp_enabled_update, {}, [this](){
        bool enabled = ocpp_enabled_update.get("enabled")->asBool();

        if (enabled == ocpp_enabled.get("enabled")->asBool())
            return;

        if (enabled) {
            tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_OCPP, 32000, true, false);
            this->apply_slot_default(CHARGING_SLOT_OCPP, 32000, true, false);
        }
        else {
            tf_evse_v2_set_charging_slot(&device, CHARGING_SLOT_OCPP, 32000, false, false);
            this->apply_slot_default(CHARGING_SLOT_OCPP, 32000, false, false);
        }
    }, false);

    api.addState("evse/gp_output", &gp_output, {}, 1000);
    api.addCommand("evse/gp_output_update", &gp_output_update, {}, [this](){
        is_in_bootloader(tf_evse_v2_set_gp_output(&device, gp_output_update.get("gp_output")->asUint()));
    }, true);

    this->DeviceModule::register_urls();
}

void EVSEV2::loop()
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

void EVSEV2::setup_evse()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    this->apply_defaults();
    initialized = true;
}

void EVSEV2::update_all_data()
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
    uint8_t button_cfg;
    uint32_t button_press_time;
    uint32_t button_release_time;
    bool button_pressed;
    bool ev_wakeup_enabled;
    bool cp_disconnect;
    bool boost_mode_enabled;

    // get_low_level_state - 61 byte
    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;
    uint16_t adc_values[7];
    int16_t voltages[7];
    uint32_t resistances[2];
    bool gpio[24];
    uint32_t charging_time;
    uint32_t time_since_state_change;
    uint32_t time_since_dc_fault_check;
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
                                   &button_cfg,
                                   &button_press_time,
                                   &button_release_time,
                                   &button_pressed,
                                   &ev_wakeup_enabled,
                                   &cp_disconnect,
                                   &boost_mode_enabled);

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
                                        &time_since_dc_fault_check,
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

    state.get("iec61851_state")->updateUint(iec61851_state);
    state.get("charger_state")->updateUint(charger_state);
    state.get("contactor_state")->updateUint(contactor_state);
    bool contactor_error_changed = state.get("contactor_error")->updateUint(contactor_error);
    state.get("allowed_charging_current")->updateUint(allowed_charging_current);
    bool error_state_changed = state.get("error_state")->updateUint(error_state);
    state.get("lock_state")->updateUint(lock_state);
    bool dc_fault_current_state_changed = state.get("dc_fault_current_state")->updateUint(dc_fault_current_state);

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
    hardware_configuration.get("jumper_configuration")->updateUint(jumper_configuration);
    hardware_configuration.get("has_lock_switch")->updateBool(has_lock_switch);
    hardware_configuration.get("evse_version")->updateUint(evse_version);
    hardware_configuration.get("energy_meter_type")->updateUint(energy_meter_type);

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
    low_level_state.get("time_since_dc_fault_check")->updateUint(time_since_dc_fault_check);

    for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
        slots.get(i)->get("max_current")->updateUint(max_current[i]);
        slots.get(i)->get("active")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[i]));
        slots.get(i)->get("clear_on_disconnect")->updateBool(SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[i]));
    }

    auto_start_charging.get("auto_start_charging")->updateBool(
        !slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("clear_on_disconnect")->asBool());


    if (energy_meter_type != 0) {
        // get_energy_meter_values
        energy_meter_values.get("power")->updateFloat(power);
        energy_meter_values.get("energy_rel")->updateFloat(energy_relative);
        energy_meter_values.get("energy_abs")->updateFloat(energy_absolute);

        for (int i = 0; i < 3; ++i)
            energy_meter_values.get("phases_active")->get(i)->updateBool(phases_active[i]);

        for (int i = 0; i < 3; ++i)
            energy_meter_values.get("phases_connected")->get(i)->updateBool(phases_connected[i]);
    }

    // get_energy_meter_errors
    energy_meter_errors.get("local_timeout")->updateUint(error_count[0]);
    energy_meter_errors.get("global_timeout")->updateUint(error_count[1]);
    energy_meter_errors.get("illegal_function")->updateUint(error_count[2]);
    energy_meter_errors.get("illegal_data_access")->updateUint(error_count[3]);
    energy_meter_errors.get("illegal_data_value")->updateUint(error_count[4]);
    energy_meter_errors.get("slave_device_failure")->updateUint(error_count[5]);

    // get_gpio_configuration
    gpio_configuration.get("shutdown_input")->updateUint(shutdown_input_configuration);
    gpio_configuration.get("input")->updateUint(input_configuration);
    gpio_configuration.get("output")->updateUint(output_configuration);

    // get_button_configuration
    button_configuration.get("button")->updateUint(button_cfg);

    // get_button_state
    button_state.get("button_press_time")->updateUint(button_press_time);
    button_state.get("button_release_time")->updateUint(button_release_time);
    button_state.get("button_pressed")->updateBool(button_pressed);

    ev_wakeup.get("enabled")->updateBool(ev_wakeup_enabled);
    boost_mode.get("enabled")->updateBool(boost_mode_enabled);

    control_pilot_disconnect.get("disconnect")->updateBool(cp_disconnect);

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

    gp_output.get("gp_output")->updateUint(gpio[10] ? TF_EVSE_V2_OUTPUT_CONNECTED_TO_GROUND : TF_EVSE_V2_OUTPUT_HIGH_IMPEDANCE);

#if MODULE_WATCHDOG_AVAILABLE()
    static size_t watchdog_handle = watchdog.add("evse_v2_all_data", "EVSE not reachable");
    watchdog.reset(watchdog_handle);
#endif
}
