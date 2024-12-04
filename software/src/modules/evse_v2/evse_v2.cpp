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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "bindings/errors.h"
#include "build.h"
#include "tools.h"
#include "string_builder.h"
#include "evse_v2_bricklet_firmware_bin.embedded.h"

extern EVSEV2 evse_v2;

extern void evse_v2_button_recovery_handler();

static void energy_meter_values_callback(struct TF_EVSEV2 * evse_v2, float power, float current[3], bool phases_active[3], bool phases_connected[3], void *user_data);

EVSEV2::EVSEV2() : DeviceModule(evse_v2_bricklet_firmware_bin_data,
                                evse_v2_bricklet_firmware_bin_length,
                                "evse",
                                "EVSE 2.0",
                                "EVSE",
                                [](){evse_common.setup_evse();}) {}

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
            }, Config::get_prototype_uint16_0(), 7, 7, Config::type_id<Config::ConfUint>())
        },
        {"voltages", Config::Array({
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
                Config::Int16(0),
            }, Config::get_prototype_int16_0(), 7, 7, Config::type_id<Config::ConfInt>())
        },
        {"resistances", Config::Array({
                Config::Uint32(0),
                Config::Uint32(0),
            }, Config::get_prototype_uint32_0(), 2, 2, Config::type_id<Config::ConfUint>())
        },
        {"gpio", Config::Array({
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            Config::Bool(false), Config::Bool(false),  Config::Bool(false),Config::Bool(false),
            }, Config::get_prototype_bool_false(), 24, 24, Config::type_id<Config::ConfBool>())},
        {"charging_time", Config::Uint32(0)},
        {"time_since_state_change", Config::Uint32(0)},
        {"uptime", Config::Uint32(0)},
        {"time_since_dc_fault_check", Config::Uint32(0)},
        {"temperature", Config::Int16(0)},
        {"phases_current", Config::Uint16(0)},
        {"phases_requested", Config::Uint16(0)},
        {"phases_state", Config::Uint16(0)},
        {"phases_info", Config::Uint16(0)},
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
    button_configuration_update = button_configuration;

    ev_wakeup = Config::Object({
        {"enabled", Config::Bool(false)}
    });
    ev_wakeup_update = ev_wakeup;

    phase_auto_switch = ev_wakeup;
    phase_auto_switch_update = ev_wakeup;

    phases_connected = Config::Object({
        {"phases", Config::Uint8(0)}
    });
    phases_connected_update = phases_connected;

    control_pilot_disconnect = Config::Object({
        {"disconnect", Config::Bool(false)}
    });

    control_pilot_disconnect_update = control_pilot_disconnect;

    gp_output = Config::Object({
        {"gp_output", Config::Uint(0, 0, 1)}
    });

    gp_output_update = gp_output;

#if MODULE_AUTOMATION_AVAILABLE()
    // Create a temporary config that allocates a schema.
    auto automation_cfg = Config::Object({
        {"closed", Config::Bool(true)}
    });

    automation.register_trigger(
        AutomationTriggerID::EVSEButton,
        *Config::Null()
    );

    automation.register_trigger(
        AutomationTriggerID::EVSEShutdownInput,
        automation_cfg
    );

#if BUILD_IS_WARP2()
    automation.register_trigger(
        AutomationTriggerID::EVSEGPInput,
        automation_cfg
    );

    automation.register_action(
        AutomationActionID::EVSEGPOutput,
        automation_cfg,
        [this](const Config *config) {
            is_in_bootloader(tf_evse_v2_set_gp_output(&device, config->get("closed")->asBool() ? 0 : 1));
        }
    );
#endif

#endif
}

void EVSEV2::post_setup()
{
    if (!device_found)
        return;

    task_scheduler.scheduleOnce([this](){
        uint32_t press_time = 0;
        tf_evse_v2_get_button_press_boot_time(&device, true, &press_time);
        if (press_time != 0)
            logger.printfln("Reset boot button press time");
    }, 40_s);

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
        if (!rtc.clock_synced(&tv))
            return;

        struct tm timeinfo;
        localtime_r(&tv.tv_sec, &timeinfo);

        if (timeinfo.tm_hour == 3) {
            tf_evse_v2_trigger_dc_fault_test(&device, 0xDCFAE550, nullptr);
        }
    }, 1_m/* wait for ntp sync */, 1_h);
}

void EVSEV2::post_register_urls()
{
    api.addCommand("evse/reset_dc_fault_current_state", &reset_dc_fault_current_state, {}, [this](String &/*errmsg*/) {
        is_in_bootloader(tf_evse_v2_reset_dc_fault_current_state(&device, reset_dc_fault_current_state.get("password")->asUint()));
    }, true);

    api.addCommand("evse/trigger_dc_fault_test", Config::Null(), {}, [this](String &/*errmsg*/) {
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

    // Configurations. Note that those are _not_ configs in the api.addPersistentConfig sense:
    // The configs are stored on the EVSE itself, not the ESP's flash.
    // All _update APIs that write the EVSEs flash without checking first if this was a change
    // are marked as actions to make sure the flash is not written unnecessarily.

    api.addState("evse/gpio_configuration", &gpio_configuration);
    api.addCommand("evse/gpio_configuration_update", &gpio_configuration_update, {}, [this](String &/*errmsg*/) {
        is_in_bootloader(tf_evse_v2_set_gpio_configuration(&device, gpio_configuration_update.get("shutdown_input")->asUint(),
                                                                    gpio_configuration_update.get("input")->asUint(),
                                                                    gpio_configuration_update.get("output")->asUint()));
    }, true);

    api.addState("evse/button_configuration", &button_configuration);
    api.addCommand("evse/button_configuration_update", &button_configuration_update, {}, [this](String &/*errmsg*/) {
        is_in_bootloader(tf_evse_v2_set_button_configuration(&device, button_configuration_update.get("button")->asUint()));
    }, true);

    api.addState("evse/ev_wakeup", &ev_wakeup);
    api.addCommand("evse/ev_wakeup_update", &ev_wakeup_update, {}, [this](String &/*errmsg*/) {
        is_in_bootloader(tf_evse_v2_set_ev_wakeup(&device, ev_wakeup_update.get("enabled")->asBool()));
    }, true);

    api.addState("evse/phase_auto_switch", &phase_auto_switch);
    api.addCommand("evse/phase_auto_switch_update", &phase_auto_switch_update, {}, [this](String &/*errmsg*/) {
        is_in_bootloader(tf_evse_v2_set_phase_auto_switch(&device, phase_auto_switch_update.get("enabled")->asBool()));
    }, true);


    api.addState("evse/phases_connected", &phases_connected);
    api.addCommand("evse/phases_connected_update", &phases_connected_update, {}, [this](String &/*errmsg*/) {
        is_in_bootloader(tf_evse_v2_set_phases_connected(&device, phases_connected_update.get("phases")->asUint()));
    }, true);

    api.addState("evse/control_pilot_disconnect", &control_pilot_disconnect);
    api.addCommand("evse/control_pilot_disconnect_update", &control_pilot_disconnect_update, {}, [this](String &/*errmsg*/) {
        if (evse_common.management_enabled.get("enabled")->asBool()) { // Disallow updating control pilot configuration if management is enabled because the charge manager will override the CP config every second.
            logger.printfln("Control pilot cannot be (dis)connected by API while charge management is enabled.");
            return;
        }
        is_in_bootloader(tf_evse_v2_set_control_pilot_disconnect(&device, control_pilot_disconnect_update.get("disconnect")->asBool(), nullptr));
    }, true);

#if BUILD_IS_WARP2()
    api.addFeature("evse_gp_output");
    api.addState("evse/gp_output", &gp_output);
    api.addCommand("evse/gp_output_update", &gp_output_update, {}, [this](String &/*errmsg*/) {
        is_in_bootloader(tf_evse_v2_set_gp_output(&device, gp_output_update.get("gp_output")->asUint()));
    }, true);
#endif
}

void EVSEV2::register_events()
{
    // Register callback in the events stage so that it doesn't keep firing during the setup stage.
    tf_evse_v2_register_energy_meter_values_callback(&device, energy_meter_values_callback, nullptr);
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

void EVSEV2::set_control_pilot_disconnect(bool cp_disconnect, bool *cp_disconnected)
{
    is_in_bootloader(tf_evse_v2_set_control_pilot_disconnect(&device, cp_disconnect, cp_disconnected));
}

bool EVSEV2::get_control_pilot_disconnect()
{
    return control_pilot_disconnect.get("disconnect")->asBool();
}

void EVSEV2::set_boost_mode(bool enabled)
{
    is_in_bootloader(tf_evse_v2_set_boost_mode(&device, enabled));
}

int EVSEV2::get_charging_slot(uint8_t slot, uint16_t *ret_current, bool *ret_enabled, bool *ret_reset_on_dc)
{
    return tf_evse_v2_get_charging_slot(&device, slot, ret_current, ret_enabled, ret_reset_on_dc);
}

int EVSEV2::set_charging_slot(uint8_t slot, uint16_t current, bool enabled, bool reset_on_dc)
{
    return tf_evse_v2_set_charging_slot(&device, slot, current, enabled, reset_on_dc);
}

void EVSEV2::set_charging_slot_max_current(uint8_t slot, uint16_t current)
{
    is_in_bootloader(tf_evse_v2_set_charging_slot_max_current(&device, slot, current));
}

void EVSEV2::set_charging_slot_clear_on_disconnect(uint8_t slot, bool clear_on_disconnect)
{
    is_in_bootloader(tf_evse_v2_set_charging_slot_clear_on_disconnect(&device, slot, clear_on_disconnect));
}

void EVSEV2::set_charging_slot_active(uint8_t slot, bool enabled)
{
    tf_evse_v2_set_charging_slot_active(&device, slot, enabled);
}

int EVSEV2::get_charging_slot_default(uint8_t slot, uint16_t *ret_max_current, bool *ret_enabled, bool *ret_clear_on_disconnect)
{
    return tf_evse_v2_get_charging_slot_default(&device, slot, ret_max_current, ret_enabled, ret_clear_on_disconnect);
}

int EVSEV2::set_charging_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear_on_disconnect)
{
    return tf_evse_v2_set_charging_slot_default(&device, slot, current, enabled, clear_on_disconnect);
}

static const char *debug_header_prefix =
    "STATE,"
    "iec61851_state,"
    "charger_state,"
    "contactor_state,"
    "contactor_error,"
    "allowed_charging_current,"
    "error_state,"
    "lock_state,"
    "dc_fault_current_state,"
    "HARDWARE_CONFIG,"
    "jumper_configuration,"
    "has_lock_switch,"
    "evse_version,"
    "energy_meter_type,"
    "ENERGY_METER,"
    "power,"
    "voltage_0,"
    "voltage_1,"
    "voltage_2,"
    "current_0,"
    "current_1,"
    "current_2,"
    "phase_0_active,"
    "phase_1_active,"
    "phase_2_active,"
    "phase_0_connected,"
    "phase_1_connected,"
    "phase_2_connected,"
    "ENERGY_METER_ERRORS,"
    "local_timeout,"
    "global_timeout,"
    "illegal_function,"
    "illegal_data_access,"
    "illegal_data_value,"
    "slave_device_failure,"
    "LL_STATE,"
    "led_state,"
    "cp_pwm_duty_cycle,"
    "car_stopped_charging,"
    "time_since_state_change,"
    "uptime,"
    "ADC_VALUES,"
    "adc_cp_pe_before_pwm_high,"
    "adc_cp_pe_after_pwm_high,"
    "adc_cp_pe_before_pwm_low,"
    "adc_cp_pe_after_pwm_low,"
    "adc_pp_pe,"
    "adc_plus_12v,"
    "adc_minus_12v,"
    "VOLTAGES,"
    "voltage_cp_pe_before_pwm_high,"
    "voltage_cp_pe_after_pwm_high,"
    "voltage_cp_pe_before_pwm_low,"
    "voltage_cp_pe_after_pwm_low,"
    "voltage_pp_pe,"
    "voltage_plus_12v,"
    "voltage_minus_12v,"
    "RESISTANCES,"
    "resistance_cp_pe,"
    "resistance_pp_pe,";

static const char *debug_header_infix_v2 =
    "GPIOS,"
    "gpio_config_jumper_0,"
    "gpio_motor_fault,"
    "gpio_dc_error,"
    "gpio_config_jumper_1,"
    "gpio_dc_test,"
    "gpio_gp_shutdown,"
    "gpio_button,"
    "gpio_cp_pwm,"
    "gpio_motor_input_switch,"
    "gpio_contactor,"
    "gpio_gp_output,"
    "gpio_cp_disconnect,"
    "gpio_motor_active,"
    "gpio_motor_phase,"
    "gpio_contactor_check_before,"
    "gpio_contactor_check_after,"
    "gpio_gp_input,"
    "gpio_dc_x6,"
    "gpio_dc_x30,"
    "gpio_led,"
    "gpio_20,"
    "gpio_21,"
    "gpio_22,"
    "gpio_23,";

static const char *debug_header_infix_v3 =
    "GPIOS,"
    "gpio_dc_x30,"
    "gpio_dc_x6,"
    "gpio_dc_error,"
    "gpio_dc_test,"
    "gpio_led_status,"
    "gpio_button,"
    "gpio_led_red,"
    "gpio_led_blue,"
    "gpio_led_green,"
    "gpio_cp_pwm,"
    "gpio_contactor_1,"
    "gpio_contactor_0,"
    "gpio_contactor_1_feedback,"
    "gpio_contactor_0_feedback,"
    "gpio_pe_check,"
    "gpio_config_jumper_1,"
    "gpio_cp_disconnect,"
    "gpio_config_jumper_0,"
    "gpio_gp_shutdown,"
    "gpio_version_detect,"
    "gpio_20,"
    "gpio_21,"
    "gpio_22,"
    "gpio_23,";

static const char *debug_header_suffix =
    "SLOTS,"
    "slot_incoming_cable,"
    "slot_outgoing_cable,"
    "slot_shutdown_input,"
    "slot_gp_input,"
    "slot_autostart_button,"
    "slot_global,"
    "slot_user,"
    "slot_charge_manager,"
    "slot_external,"
    "slot_modbus_tcp,"
    "slot_modbus_tcp_enable,"
    "slot_ocpp,"
    "slot_charge_limits,"
    "slot_require_meter,"
    "slot_automation,"
    "slot_15,"
    "slot_16,"
    "slot_17,"
    "slot_18,"
    "slot_19";

static const size_t debug_header_prefix_len = strlen(debug_header_prefix);
static const size_t debug_header_infix_v2_len = strlen(debug_header_infix_v2);
static const size_t debug_header_infix_v3_len = strlen(debug_header_infix_v3);
static const size_t debug_header_suffix_len = strlen(debug_header_suffix);

[[gnu::const]]
size_t EVSEV2::get_debug_header_length() const
{
    return debug_header_prefix_len +
           (evse_common.get_evse_version() >= 30
            ? debug_header_infix_v3_len
            : debug_header_infix_v2_len) +
           debug_header_suffix_len;
}

void EVSEV2::get_debug_header(StringBuilder *sb)
{
    sb->puts(debug_header_prefix, debug_header_prefix_len);

    if (evse_common.get_evse_version() >= 30) {
        sb->puts(debug_header_infix_v3, debug_header_infix_v3_len);
    }
    else {
        sb->puts(debug_header_infix_v2, debug_header_infix_v2_len);
    }

    sb->puts(debug_header_suffix, debug_header_suffix_len);
}

[[gnu::const]]
size_t EVSEV2::get_debug_line_length() const
{
    return 768; // FIXME: currently max ~510, make tighter estimate
}

void EVSEV2::get_debug_line(StringBuilder *sb)
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

    // get_low_level_state
    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;
    uint16_t adc_values[7];
    int16_t voltages[7];
    uint32_t resistances[2];
    bool gpio[24];
    bool car_stopped_charging;
    uint32_t time_since_state_change;
    uint32_t time_since_dc_fault_check;
    uint32_t uptime;

    // get_all_charging_slots
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
        sb->puts("get_all_data_1 failed");
        return;
    }

    rc = tf_evse_v2_get_low_level_state(&device,
                                        &led_state,
                                        &cp_pwm_duty_cycle,
                                        adc_values,
                                        voltages,
                                        resistances,
                                        gpio,
                                        &car_stopped_charging,
                                        &time_since_state_change,
                                        &time_since_dc_fault_check,
                                        &uptime);

    if (rc != TF_E_OK) {
        logger.printfln("get_low_level_state %d", rc);
        is_in_bootloader(rc);
        sb->puts("get_low_level_state failed");
        return;
    }

    rc = tf_evse_v2_get_all_charging_slots(&device, max_current, active_and_clear_on_disconnect);

    if (rc != TF_E_OK) {
        logger.printfln("get_all_charging_slots %d", rc);
        is_in_bootloader(rc);
        sb->puts("get_all_charging_slots failed");
        return;
    }

    float all_values[METER_ALL_VALUES_RESETTABLE_COUNT];
    uint16_t all_values_len = 0;
    rc = tf_evse_v2_get_all_energy_meter_values(&device, all_values, &all_values_len);

    if (rc != TF_E_OK) {
        logger.printfln("get_all_energy_meter_values %d", rc);
        is_in_bootloader(rc);
        sb->puts("get_all_energy_meter_values failed");
        return;
    }

    sb->printf(","
             "%u,%u,%u,%u,%u,%u,%u,%u,,"
             "%u,%c,%u,%u,,"
             "%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%c,%c,%c,%c,%c,%c,,"
             "%u,%u,%u,%u,%u,%u,,"
             "%u,%u,%u,%u,%u,,"
             "%u,%u,%u,%u,%u,%u,%u,,"
             "%d,%d,%d,%d,%d,%d,%d,,"
             "%u,%u,,"
             "%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,%c,,"
             "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
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
             all_values[0],
             all_values[1],
             all_values[2],
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
             car_stopped_charging,
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
}

bool EVSEV2::phase_switching_capable()
{
    return evse_common.get_evse_version() >= 30 && evse_v2.phases_connected.get("phases")->asUint() > 1;
}

bool EVSEV2::can_switch_phases_now(uint32_t phases_wanted)
{
    if (phases_wanted != 1) {
        uint32_t phases_info = evse_common.low_level_state.get("phases_info")->asUint();
        if (phases_info & EVSEV2_PHASES_INFO_1P_CAR_MASK) {
            // Car wants to charge single-phase, don't allow switching to three phases.
            return false;
        }
    }

    return true;
}

uint32_t EVSEV2::get_phases()
{
    return evse_common.low_level_state.get("phases_current")->asUint();
}

PhaseSwitcherBackend::SwitchingState EVSEV2::get_phase_switching_state()
{
    if (evse_common.state.get("error_state")->asUint() != 0) {
        return PhaseSwitcherBackend::SwitchingState::Error;
    }

    uint32_t state = evse_common.low_level_state.get("phases_state")->asUint();
    if (state != 0) {
        return PhaseSwitcherBackend::SwitchingState::Busy;
    }

    return PhaseSwitcherBackend::SwitchingState::Ready;
}

bool EVSEV2::switch_phases(uint32_t phases_wanted)
{
    if (phases_wanted > 3) {
        logger.printfln("Invalid phases wanted: %u", phases_wanted);
        return false;
    }

    if (!can_switch_phases_now(phases_wanted)) {
        logger.printfln("Requested phase switch but can't switch at the moment.");
        return false;
    }

    int err = tf_evse_v2_set_phase_control(&device, phases_wanted);
    if (err == TF_E_OK) {
        return true;
    }

    logger.printfln("switch_phases failed: %s (%i)", tf_hal_strerror(err), err);

    return false;
}

bool EVSEV2::is_external_control_allowed()
{
    return !evse_common.management_enabled.get("enabled")->asBool();
}

void EVSEV2::update_all_data()
{
    if (!initialized)
        return;

    // get_all_data_1
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
    EVSEV2MeterData meter_data;

    // get_all_data_2
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
    uint8_t phases_state;
    uint8_t phases_info;
    bool phase_auto_switch_enabled;
    uint8_t phases_connected_;

    // get_low_level_state
    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;
    uint16_t adc_values[7];
    int16_t voltages[7];
    uint32_t resistances[2];
    bool gpio[24];
    bool car_stopped_charging;
    uint32_t time_since_state_change;
    uint32_t time_since_dc_fault_check;
    uint32_t uptime;

    // get_all_charging_slots
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
                                   &phases_state,
                                   &phases_info,
                                   &phase_auto_switch_enabled,
                                   &phases_connected_);

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
                                        &car_stopped_charging,
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
#if MODULE_FIRMWARE_UPDATE_AVAILABLE()
    firmware_update.vehicle_connected = charger_state != 0 && charger_state != 4;
#endif

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
#if BUILD_IS_WARP2()
            logger.printfln("Contactor error %u PE error %u", contactor_error >> 1, contactor_error & 1);
#elif BUILD_IS_WARP3()
            if (contactor_error & 1) {
                logger.printfln("Contactor error: PE error");
            }

            auto print_contactor_error = [](const uint8_t error, const bool contactor, const bool phase_switch, const bool contactor_aux, const bool phase_switch_aux) {
                logger.printfln("Contactor error (%u): Set C1 to '%s' and C2 to '%s' [%s] but see AuxC1 as '%s' and AuxC2 as '%s'",
                                error,
                                contactor        ? "closed" : "open",
                                phase_switch     ? "closed" : "open",
                                !contactor       ? "no charging" : (contactor_aux ? "3-phase charging" : "1-phase charging"),
                                contactor_aux    ? "closed" : "open",
                                phase_switch_aux ? "closed" : "open");
            };


            // bit3: contactor, bit2: phase_switch, bit1: contactor aux, bit0: phase_switch aux
            const uint8_t err = contactor_error >> 1;
            switch (err) {
                // contactor active + 3phase
                case /*0b0000*/  0: ; break; // contactor aux and phase switch aux active -> OK
                case /*0b0001*/  1: print_contactor_error(err, 1, 1, 1, 0); break;
                case /*0b0010*/  2: print_contactor_error(err, 1, 1, 0, 1); break;
                case /*0b0011*/  3: print_contactor_error(err, 1, 1, 0, 0); break;

                // contactor active + 1phase
                case /*0b0100*/  4: print_contactor_error(err, 1, 0, 1, 1); break;
                // case /*0b0101*/ 0: break; // contactor aux active and phase switch aux not active -> OK
                case /*0b0110*/  5: print_contactor_error(err, 1, 0, 0, 1); break;
                case /*0b0111*/  6: print_contactor_error(err, 1, 0, 0, 0); break;

                // contactor not active (1/3phase not relevant)
                case /*0b1000*/  7: print_contactor_error(err, 0, 1, 1, 1); break;
                case /*0b1001*/  8: print_contactor_error(err, 0, 1, 1, 0); break;
                case /*0b1010*/  9: print_contactor_error(err, 0, 1, 0, 1); break;
                // case /*0b1011*/  0: break; // contactor aux not active and phase switch aux not active -> OK
                case /*0b1100*/ 10: print_contactor_error(err, 0, 0, 1, 1); break;
                case /*0b1101*/ 11: print_contactor_error(err, 0, 0, 1, 0); break;
                case /*0b1110*/ 12: print_contactor_error(err, 0, 0, 0, 1); break;
                // case /*0b1111*/  0: break; // contactor aux not active and phase switch aux not active -> OK

                default: logger.printfln("Contactor error (%u): Unknown error", err); break; // Impossible to reach
            }
#endif
        } else {
            logger.printfln("Contactor/PE error cleared");
        }
    }

    if (error_state_changed) {
        if (error_state != 0) {
            logger.printfln("Error state %d", error_state);
        } else {
            logger.printfln("Error state cleared");
        }
    }

    if (dc_fault_current_state_changed) {
        if (dc_fault_current_state != 0) {
            logger.printfln("DC Fault current state %u (%s %u; sensor type %u)",
                            dc_fault_current_state,
                            dc_fault_current_state == 4 ? "calibration error code" : "pins",
                            dc_fault_pins,
                            dc_sensor_type);
        } else {
            logger.printfln("DC Fault current state cleared");
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

    for (size_t i = 0; i < ARRAY_SIZE(adc_values); ++i)
        evse_common.low_level_state.get("adc_values")->get(i)->updateUint(adc_values[i]);

    for (size_t i = 0; i < ARRAY_SIZE(voltages); ++i)
        evse_common.low_level_state.get("voltages")->get(i)->updateInt(voltages[i]);

    for (size_t i = 0; i < ARRAY_SIZE(resistances); ++i)
        evse_common.low_level_state.get("resistances")->get(i)->updateUint(resistances[i]);

    for (size_t i = 0; i < ARRAY_SIZE(gpio); ++i)
        evse_common.low_level_state.get("gpio")->get(i)->updateBool(gpio[i]);

#if MODULE_AUTOMATION_AVAILABLE()
    enum class InputState {
        Unknown,
        Open,
        Closed
    };

    static InputState last_shutdown_input_state = InputState::Unknown;
#if BUILD_IS_WARP2()
    bool gpio_enable = gpio[5];
#elif BUILD_IS_WARP3()
    bool gpio_enable = gpio[18];
#else
    #error "GPIO layout is unknown"
#endif

    InputState shutdown_input_state = gpio_enable ? InputState::Closed : InputState::Open;
    if (last_shutdown_input_state != shutdown_input_state) {
        // We need to schedule this since the first call of update_all_data happens before automation is initialized.
        task_scheduler.scheduleOnce([this, gpio_enable]() {
            automation.trigger(AutomationTriggerID::EVSEShutdownInput, (void *)&gpio_enable, this);
        });
        last_shutdown_input_state = shutdown_input_state;
    }

#if BUILD_IS_WARP2()
    static InputState last_input_state = InputState::Unknown;

    InputState input_state = gpio[16] ? InputState::Closed : InputState::Open;
    if (last_input_state != input_state) {
        // We need to schedule this since the first call of update_all_data happens before automation is initialized.
        task_scheduler.scheduleOnce([this, gpio]() {
            automation.trigger(AutomationTriggerID::EVSEGPInput, (void *)&gpio[16], this);
        });
        last_input_state = input_state;
    }
#endif

#endif

    evse_common.low_level_state.get("charging_time")->updateUint(car_stopped_charging);
    evse_common.low_level_state.get("time_since_state_change")->updateUint(time_since_state_change);
    evse_common.low_level_state.get("uptime")->updateUint(uptime);
    evse_common.low_level_state.get("time_since_dc_fault_check")->updateUint(time_since_dc_fault_check);

    for (size_t i = 0; i < CHARGING_SLOT_COUNT; ++i) {
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

    // get_button_state
    evse_common.button_state.get("button_press_time")->updateUint(button_press_time);
    evse_common.button_state.get("button_release_time")->updateUint(button_release_time);
    bool button_pressed_changed = evse_common.button_state.get("button_pressed")->updateBool(button_pressed);

#if MODULE_AUTOMATION_AVAILABLE()
    if (button_pressed_changed && button_pressed) {
        // Don't attempt to trigger actions during the setup stage because the automation rules are probably not loaded yet.
        // Losing the button press during startup is probably acceptable.
        if (boot_stage > BootStage::SETUP) {
            automation.trigger(AutomationTriggerID::EVSEButton, nullptr, this);
        }
    }
#else
    (void)button_pressed_changed;
#endif

    ev_wakeup.get("enabled")->updateBool(ev_wakeup_enabled);
    phase_auto_switch.get("enabled")->updateBool(phase_auto_switch_enabled);
    phases_connected.get("phases")->updateUint(phases_connected_);
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

#if BUILD_IS_WARP2()
    gp_output.get("gp_output")->updateUint(gpio[10] ? TF_EVSE_V2_OUTPUT_CONNECTED_TO_GROUND : TF_EVSE_V2_OUTPUT_HIGH_IMPEDANCE);
#endif

    evse_common.low_level_state.get("temperature")->updateInt(temperature);
    evse_common.low_level_state.get("phases_current")->updateUint(phases_current);
    evse_common.low_level_state.get("phases_requested")->updateUint(phases_requested);
    evse_common.low_level_state.get("phases_state")->updateUint(phases_state);
    evse_common.low_level_state.get("phases_info")->updateUint(phases_info);

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

static void energy_meter_values_callback(struct TF_EVSEV2 * /*evse_v2*/, float power, float current[3], bool phases_active[3], bool phases_connected[3], void *user_data)
{
#if MODULE_METERS_EVSE_V2_AVAILABLE()
    meters_evse_v2.energy_meter_values_callback(power, current);
#endif
}

#if MODULE_AUTOMATION_AVAILABLE()
bool EVSEV2::has_triggered(const Config *conf, void *data)
{
    const Config *cfg = static_cast<const Config *>(conf->get());
    switch (conf->getTag<AutomationTriggerID>())
    {
    case AutomationTriggerID::EVSEButton:
        return true;

#if BUILD_IS_WARP2()
    case AutomationTriggerID::EVSEGPInput:
#endif
    case AutomationTriggerID::EVSEShutdownInput:
        return *static_cast<bool *>(data) != cfg->get("closed")->asBool();
    default:
        break;
    }
    return false;
}
#endif
