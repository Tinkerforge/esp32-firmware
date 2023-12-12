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
#include "module_dependencies.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "web_server.h"

extern EVSEV2 evse_v2;

extern bool firmware_update_allowed;

extern void evse_v2_button_recovery_handler();

EVSEV2::EVSEV2() : DeviceModule("evse", "EVSE 2.0", "EVSE", [](){evse_common.setup_evse();}) {}

void EVSEV2::pre_init()
{
    evse_v2_button_recovery_handler();
}

void EVSEV2::pre_setup()
{
    this->DeviceModule::pre_setup();

    // States
    evse_common.state = Config::Object({
        {"iec61851_state", Config::Uint8(0)},
        {"charger_state", Config::Uint8(0)},
        {"contactor_state", Config::Uint8(0)},
        {"contactor_error", Config::Uint8(0)},
        {"allowed_charging_current", Config::Uint16(0)},
        {"error_state", Config::Uint8(0)},
        {"lock_state", Config::Uint8(0)},
        {"dc_fault_current_state", Config::Uint8(0)},
    });

    evse_common.hardware_configuration = Config::Object({
        {"jumper_configuration", Config::Uint8(0)},
        {"has_lock_switch", Config::Bool(false)},
        {"evse_version", Config::Uint8(0)},
        {"energy_meter_type", Config::Uint8(0)}
    });

    evse_common.low_level_state = Config::Object ({
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
        {"time_since_dc_fault_check", Config::Uint32(0)},
        {"temperature", Config::Int16(0)},
        {"phases_current", Config::Uint16(0)},
        {"phases_requested", Config::Uint16(0)},
        {"phases_status", Config::Uint16(0)},
        {"dc_fault_pins", Config::Uint8(0)},
        {"dc_fault_sensor_type", Config::Uint8(0)}
    });


    // Actions

    reset_dc_fault_current_state = Config::Object({
        {"password", Config::Uint32(0)} // 0xDC42FA23
    });

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

    ev_wakeup = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    ev_wakeup_update = ev_wakeup;

    control_pilot_disconnect = Config::Object({
        {"disconnect", Config::Bool(false)}
    });

    control_pilot_disconnect_update = control_pilot_disconnect;

    gp_output = Config::Object({
        {"gp_output", Config::Uint(0, 0, 1)}
    });

    gp_output_update = gp_output;

#if MODULE_CRON_AVAILABLE()
    cron.register_trigger(
        CronTriggerID::EVSEButton,
        Config::Object({
            {"button_pressed", Config::Bool(false)}
        })
    );

    cron.register_trigger(
        CronTriggerID::EVSEGPInput,
        Config::Object({
            {"high", Config::Bool(false)}
        })
    );

    cron.register_trigger(
        CronTriggerID::EVSEShutdownInput,
        Config::Object({
            {"high", Config::Bool(false)}
        })
    );

    cron.register_action(
        CronActionID::EVSEGPOutput,
        Config::Object({
            {"state", Config::Uint(0, 0, 1)}
        }),
        [this](const Config *config) {
            is_in_bootloader(tf_evse_v2_set_gp_output(&device, config->get("state")->asUint()));
        }
    );
#endif
}

#if MODULE_CRON_AVAILABLE()
static bool trigger_action(Config *cfg, void *data) {
    return evse_v2.action_triggered(cfg, data);
}
#endif

void EVSEV2::post_setup() {
    if (!device_found)
        return;

#if MODULE_CRON_AVAILABLE()
    task_scheduler.scheduleOnce([this]() {
        cron.trigger_action(CronTriggerID::EVSEButton, nullptr, trigger_action);
        cron.trigger_action(CronTriggerID::EVSEGPInput, nullptr, trigger_action);
        cron.trigger_action(CronTriggerID::EVSEShutdownInput, nullptr, trigger_action);
    }, 0);
#endif

    task_scheduler.scheduleOnce([this](){
        uint32_t press_time = 0;
        tf_evse_v2_get_button_press_boot_time(&device, true, &press_time);
        if (press_time != 0)
            logger.printfln("Reset boot button press time");
    }, 40000);

    // Get all data once before announcing the EVSE feature.
    api.addFeature("cp_disconnect");
    api.addFeature("button_configuration");


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

void EVSEV2::post_register_urls() {
    api.addCommand("evse/reset_dc_fault_current_state", &reset_dc_fault_current_state, {}, [this](){
        is_in_bootloader(tf_evse_v2_reset_dc_fault_current_state(&device, reset_dc_fault_current_state.get("password")->asUint()));
    }, true);

    api.addCommand("evse/trigger_dc_fault_test", Config::Null(), {}, [this](){
        if (evse_common.state.get("iec61851_state")->asUint() != IEC_STATE_A) {
            logger.printfln("Can't trigger DC fault test: IEC state is not A.");
            return;
        }
        bool success = false;
        int rc = tf_evse_v2_trigger_dc_fault_test(&device, 0xDCFAE550, &success);
        if (!success) {
            logger.printfln("Failed to start DC fault test. rc %d", rc);
        }
    }, true);

#if MODULE_DEBUG_AVAILABLE()
    api.addCommand("evse/debug_switch_to_one_phase", Config::Null(), {}, [this]() {
        is_in_bootloader(tf_evse_v2_set_phase_control(&device, 1));
    }, true);

    api.addCommand("evse/debug_switch_to_three_phases", Config::Null(), {}, [this]() {
        is_in_bootloader(tf_evse_v2_set_phase_control(&device, 3));
    }, true);
#endif

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
        if (evse_common.management_enabled.get("enabled")->asBool()) { // Disallow updating control pilot configuration if management is enabled because the charge manager will override the CP config every second.
            logger.printfln("evse: Control pilot cannot be (dis)connected by API while charge management is enabled.");
            return;
        }
        is_in_bootloader(tf_evse_v2_set_control_pilot_disconnect(&device, control_pilot_disconnect_update.get("disconnect")->asBool(), nullptr));
    }, true);

    api.addState("evse/gp_output", &gp_output, {}, 1000);
    api.addCommand("evse/gp_output_update", &gp_output_update, {}, [this](){
        is_in_bootloader(tf_evse_v2_set_gp_output(&device, gp_output_update.get("gp_output")->asUint()));
    }, true);
}

void EVSEV2::factory_reset()
{
    tf_evse_v2_factory_reset(&device, 0x2342FACD);
}

void EVSEV2::set_data_storage(uint8_t page, const uint8_t *data)
{
    tf_evse_v2_set_data_storage(&device, page, data);
}

void EVSEV2::get_data_storage(uint8_t page, uint8_t *data)
{
    tf_evse_v2_get_data_storage(&device, page, data);
}

void EVSEV2::set_indicator_led(int16_t indication, uint16_t duration, uint16_t color_h, uint8_t color_s, uint8_t color_v, uint8_t *ret_status)
{
    tf_evse_v2_set_indicator_led(&device, indication, duration, color_h, color_s, color_v, ret_status);
}

void EVSEV2::set_control_pilot_disconnect(bool cp_disconnect, bool *cp_disconnected) {
    is_in_bootloader(tf_evse_v2_set_control_pilot_disconnect(&device, cp_disconnect, cp_disconnected));
}

bool EVSEV2::get_control_pilot_disconnect() {
    return control_pilot_disconnect.get("disconnect")->asBool();
}

void EVSEV2::set_boost_mode(bool enabled) {
    is_in_bootloader(tf_evse_v2_set_boost_mode(&device, enabled));
}

int EVSEV2::get_charging_slot(uint8_t slot, uint16_t *ret_current, bool *ret_enabled, bool *ret_reset_on_dc) {
    return tf_evse_v2_get_charging_slot(&device, slot, ret_current, ret_enabled, ret_reset_on_dc);
}

int EVSEV2::set_charging_slot(uint8_t slot, uint16_t current, bool enabled, bool reset_on_dc) {
    return tf_evse_v2_set_charging_slot(&device, slot, current, enabled, reset_on_dc);
}

void EVSEV2::set_charging_slot_max_current(uint8_t slot, uint16_t current) {
    is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, slot, current));
}

void EVSEV2::set_charging_slot_clear_on_disconnect(uint8_t slot, bool clear_on_disconnect) {
    is_in_bootloader(tf_evse_v2_set_charging_slot_clear_on_disconnect(&device, slot, clear_on_disconnect));
}

void EVSEV2::set_charging_slot_active(uint8_t slot, bool enabled) {
    tf_evse_v2_set_charging_slot_active(&device, slot, enabled);
}

int EVSEV2::get_charging_slot_default(uint8_t slot, uint16_t *ret_max_current, bool *ret_enabled, bool *ret_clear_on_disconnect) {
    return tf_evse_v2_get_charging_slot_default(&device, slot, ret_max_current, ret_enabled, ret_clear_on_disconnect);
}

int EVSEV2::set_charging_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear_on_disconnect) {
    return tf_evse_v2_set_charging_slot_default(&device, slot, current, enabled, clear_on_disconnect);
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
           "current_0,"
           "current_1,"
           "current_2,"
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
    float currents[3];
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
                                       currents,
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
             "%.3f,%.3f,%.3f,%.3f,%c,%c,%c,%c,%c,%c,,"
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
             currents[0],
             currents[1],
             currents[2],
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
    struct meter_data meter_data;

    // get_all_data_2 - 26 byte
    uint8_t shutdown_input_configuration;
    uint8_t input_configuration;
    uint8_t output_configuration;
    int16_t indication;
    uint16_t duration;
    uint16_t color_h;
    uint8_t color_s;
    uint8_t color_v;
    uint8_t button_cfg;
    uint32_t button_press_time;
    uint32_t button_release_time;
    bool button_pressed;
    bool ev_wakeup_enabled;
    bool cp_disconnect;
    bool boost_mode_enabled;
    int16_t temperature;
    uint8_t phases_current;
    uint8_t phases_requested;
    uint8_t phases_status;

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
                                       &meter_data.meter_type,
                                       &meter_data.power,
                                       meter_data.currents,
                                       meter_data.phases_active,
                                       meter_data.phases_connected,
                                       meter_data.error_count);

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
                                   &color_h,
                                   &color_s,
                                   &color_v,
                                   &button_cfg,
                                   &button_press_time,
                                   &button_release_time,
                                   &button_pressed,
                                   &ev_wakeup_enabled,
                                   &cp_disconnect,
                                   &boost_mode_enabled,
                                   &temperature,
                                   &phases_current,
                                   &phases_requested,
                                   &phases_status);

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

    evse_common.state.get("iec61851_state")->updateUint(iec61851_state);
    evse_common.state.get("charger_state")->updateUint(charger_state);
    evse_common.state.get("contactor_state")->updateUint(contactor_state);
    bool contactor_error_changed = evse_common.state.get("contactor_error")->updateUint(contactor_error);
    evse_common.state.get("allowed_charging_current")->updateUint(allowed_charging_current);
    bool error_state_changed = evse_common.state.get("error_state")->updateUint(error_state);
    evse_common.state.get("lock_state")->updateUint(lock_state);

    uint8_t dc_fault_pins =  (dc_fault_current_state & 0x38) >> 3; //0b0011'1000
    uint8_t dc_sensor_type = (dc_fault_current_state & 0x40) >> 6; //0b0100'0000
    dc_fault_current_state = (dc_fault_current_state & 0x07) >> 0; //0b0000'0111

    bool dc_fault_current_state_changed = evse_common.state.get("dc_fault_current_state")->updateUint(dc_fault_current_state);
    evse_common.low_level_state.get("dc_fault_pins")->updateUint(dc_fault_pins);
    evse_common.low_level_state.get("dc_fault_sensor_type")->updateUint(dc_sensor_type);

    if (contactor_error_changed) {
        if (contactor_error != 0) {
            logger.printfln("EVSE: Contactor error %u PE error %u", contactor_error >> 1, contactor_error & 1);
        } else {
            logger.printfln("EVSE: Contactor/PE error cleared");
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
            logger.printfln("EVSE: DC Fault current state %u (%s %u; sensor type %u)",
                                dc_fault_current_state,
                                dc_fault_current_state == 4 ? "calibration error code" : "pins",
                                dc_fault_pins,
                                dc_sensor_type);
        } else {
            logger.printfln("EVSE: DC Fault current state cleared");
        }
    }

    // get_hardware_configuration
    evse_common.hardware_configuration.get("jumper_configuration")->updateUint(jumper_configuration);
    evse_common.hardware_configuration.get("has_lock_switch")->updateBool(has_lock_switch);
    evse_common.hardware_configuration.get("evse_version")->updateUint(evse_version);
    evse_common.hardware_configuration.get("energy_meter_type")->updateUint(meter_data.meter_type);

    // get_low_level_state
    evse_common.low_level_state.get("led_state")->updateUint(led_state);
    evse_common.low_level_state.get("cp_pwm_duty_cycle")->updateUint(cp_pwm_duty_cycle);

    for (int i = 0; i < sizeof(adc_values) / sizeof(adc_values[0]); ++i)
        evse_common.low_level_state.get("adc_values")->get(i)->updateUint(adc_values[i]);

    for (int i = 0; i < sizeof(voltages) / sizeof(voltages[0]); ++i)
        evse_common.low_level_state.get("voltages")->get(i)->updateInt(voltages[i]);

    for (int i = 0; i < sizeof(resistances) / sizeof(resistances[0]); ++i)
        evse_common.low_level_state.get("resistances")->get(i)->updateUint(resistances[i]);

#if MODULE_CRON_AVAILABLE()
    if (evse_common.low_level_state.get("gpio")->get(5)->asBool() != gpio[5])
        cron.trigger_action(CronTriggerID::EVSEShutdownInput, nullptr, &trigger_action);

    if (evse_common.low_level_state.get("gpio")->get(16)->asBool() != gpio[16])
        cron.trigger_action(CronTriggerID::EVSEGPInput, nullptr, &trigger_action);
#endif

    for (int i = 0; i < sizeof(gpio) / sizeof(gpio[0]); ++i)
        evse_common.low_level_state.get("gpio")->get(i)->updateBool(gpio[i]);

    evse_common.low_level_state.get("charging_time")->updateUint(charging_time);
    evse_common.low_level_state.get("time_since_state_change")->updateUint(time_since_state_change);
    evse_common.low_level_state.get("uptime")->updateUint(uptime);
    evse_common.low_level_state.get("time_since_dc_fault_check")->updateUint(time_since_dc_fault_check);

    for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
        evse_common.slots.get(i)->get("max_current")->updateUint(max_current[i]);
        evse_common.slots.get(i)->get("active")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[i]));
        evse_common.slots.get(i)->get("clear_on_disconnect")->updateBool(SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[i]));
    }

    evse_common.auto_start_charging.get("auto_start_charging")->updateBool(
        !evse_common.slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("clear_on_disconnect")->asBool());

    // get_gpio_configuration
    gpio_configuration.get("shutdown_input")->updateUint(shutdown_input_configuration);
    gpio_configuration.get("input")->updateUint(input_configuration);
    gpio_configuration.get("output")->updateUint(output_configuration);

    // get_button_configuration
    button_configuration.get("button")->updateUint(button_cfg);

#if MODULE_CRON_AVAILABLE()
    if (evse_common.button_state.get("button_pressed")->asBool() != button_pressed)
        cron.trigger_action(CronTriggerID::EVSEButton, nullptr, &trigger_action);
#endif

    // get_button_state
    evse_common.button_state.get("button_press_time")->updateUint(button_press_time);
    evse_common.button_state.get("button_release_time")->updateUint(button_release_time);
    evse_common.button_state.get("button_pressed")->updateBool(button_pressed);

    ev_wakeup.get("enabled")->updateBool(ev_wakeup_enabled);
    evse_common.boost_mode.get("enabled")->updateBool(boost_mode_enabled);

    control_pilot_disconnect.get("disconnect")->updateBool(cp_disconnect);

    // get_indicator_led
    evse_common.indicator_led.get("indication")->updateInt(indication);
    evse_common.indicator_led.get("duration")->updateUint(duration);
    evse_common.indicator_led.get("color_h")->updateUint(color_h);
    evse_common.indicator_led.get("color_s")->updateUint(color_s);
    evse_common.indicator_led.get("color_v")->updateUint(color_v);

    evse_common.auto_start_charging.get("auto_start_charging")->updateBool(!SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[CHARGING_SLOT_AUTOSTART_BUTTON]));

    evse_common.management_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_CHARGE_MANAGER]));

    evse_common.user_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_USER]));

    evse_common.modbus_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_MODBUS_TCP]));
    evse_common.ocpp_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_OCPP]));

    evse_common.external_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_EXTERNAL]));
    evse_common.external_clear_on_disconnect.get("clear_on_disconnect")->updateBool(SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[CHARGING_SLOT_EXTERNAL]));

    evse_common.global_current.get("current")->updateUint(max_current[CHARGING_SLOT_GLOBAL]);
    evse_common.management_current.get("current")->updateUint(max_current[CHARGING_SLOT_CHARGE_MANAGER]);
    evse_common.external_current.get("current")->updateUint(max_current[CHARGING_SLOT_EXTERNAL]);
    evse_common.user_current.get("current")->updateUint(max_current[CHARGING_SLOT_USER]);

    evse_common.external_defaults.get("current")->updateUint(external_default_current);
    evse_common.external_defaults.get("clear_on_disconnect")->updateBool(external_default_clear_on_disconnect);

    evse_common.require_meter_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_REQUIRE_METER]));

    gp_output.get("gp_output")->updateUint(gpio[10] ? TF_EVSE_V2_OUTPUT_CONNECTED_TO_GROUND : TF_EVSE_V2_OUTPUT_HIGH_IMPEDANCE);

    evse_common.low_level_state.get("temperature")->updateInt(temperature);
    evse_common.low_level_state.get("phases_current")->updateUint(phases_current);
    evse_common.low_level_state.get("phases_requested")->updateUint(phases_requested);
    evse_common.low_level_state.get("phases_status")->updateUint(phases_status);

#if MODULE_WATCHDOG_AVAILABLE()
    static size_t watchdog_handle = watchdog.add("evse_v2_all_data", "EVSE not reachable");
    watchdog.reset(watchdog_handle);
#endif

#if MODULE_METERS_EVSE_V2_AVAILABLE()
    meters_evse_v2.update_from_evse_v2_all_data(&meter_data);
#endif
}

uint16_t EVSEV2::get_all_energy_meter_values(float *ret_values)
{
    uint16_t len = 0;
    tf_evse_v2_get_all_energy_meter_values(&device, ret_values, &len);
    return len;
}

bool EVSEV2::reset_energy_meter_relative_energy()
{
    tf_evse_v2_reset_energy_meter_relative_energy(&device);
    return true;
}

uint8_t EVSEV2::get_energy_meter_type()
{
    return evse_common.hardware_configuration.get("energy_meter_type")->asUint();
}

#if MODULE_CRON_AVAILABLE()
bool EVSEV2::action_triggered(Config *config, void *data) {
    auto cfg = config->get();
    switch (config->getTag<CronTriggerID>())
    {
    case CronTriggerID::EVSEButton:
        // This check happens before the new state is written to the config.
        // Because of this we need to check if the current state in config is different than our desired state.
        if (evse_common.button_state.get("button_pressed")->asBool() != cfg->get("button_pressed")->asBool())
            return true;
        break;

    case CronTriggerID::EVSEGPInput:
        if (evse_common.low_level_state.get("gpio")->get(16)->asBool() != cfg->get("high")->asBool())
            return true;
        break;

    case CronTriggerID::EVSEShutdownInput:
        if (evse_common.low_level_state.get("gpio")->get(5)->asBool() != cfg->get("high")->asBool())
            return true;
        break;

    default:
        break;
    }
    return false;
}
#endif
