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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "bindings/errors.h"
#include "bindings/hal_common.h"
#include "tools.h"
#include "string_builder.h"
#include "evse_bricklet_firmware_bin.embedded.h"

#include "module_available.h"

EVSE::EVSE() : DeviceModule(evse_bricklet_firmware_bin_data,
                            evse_bricklet_firmware_bin_length,
                            "evse",
                            "EVSE",
                            "EVSE",
                            [](){evse_common.setup_evse();}) {}

void EVSE::pre_init()
{
#if MODULE_ESP32_BRICK_AVAILABLE()
    auto esp_brick = esp32_brick;
#elif MODULE_ESP32_ETHERNET_BRICK_AVAILABLE()
    auto esp_brick = esp32_ethernet_brick;
#else
    #warning "Using EVSE module without ESP32 Brick or ESP32 Ethernet Brick module. Pre-init will not work!"
    return;
#endif

    if (!esp_brick.initHAL())
        return;

    defer {
        esp_brick.destroyHAL();
    };

    TF_EVSE evse;
    int result = tf_evse_create(&evse, nullptr, &hal);
    if (result != TF_E_OK)
        return;

    defer {
        tf_evse_destroy(&evse);
    };

    uint32_t evse_uptime = 0;
    tf_evse_get_low_level_state(&evse, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &evse_uptime);
    if (evse_uptime > 10000) {
        tf_evse_set_indicator_led(&evse, 2005, 3000, nullptr);
    }
}

void EVSE::pre_setup()
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
        {"lock_state", Config::Uint8(0)}
    });

    evse_common.hardware_configuration = Config::Object({
        {"jumper_configuration", Config::Uint8(0)},
        {"has_lock_switch", Config::Bool(false)},
        {"evse_version", Config::Uint8(0)}
    });

    evse_common.low_level_state = Config::Object ({
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

void EVSE::post_register_urls()
{
    api.addState("evse/user_calibration", &user_calibration);
    api.addCommand("evse/user_calibration_update", &user_calibration, {}, [this](){
        int16_t resistance_880[14];
        user_calibration.get("resistance_880")->fillInt16Array(resistance_880, ARRAY_SIZE(resistance_880));

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

void EVSE::set_indicator_led(int16_t indication, uint16_t duration, uint16_t color_h, uint8_t color_s, uint8_t color_v, uint8_t *ret_status)
{
    // EVSE 1.0 does not support setting the LED's color.
    (void)color_h;
    (void)color_s;
    (void)color_v;
    tf_evse_set_indicator_led(&device, indication, duration, ret_status);
}

void EVSE::set_boost_mode(bool enabled)
{
    is_in_bootloader(tf_evse_set_boost_mode(&device, enabled));
}

int EVSE::get_charging_slot(uint8_t slot, uint16_t *ret_current, bool *ret_enabled, bool *ret_reset_on_dc)
{
    return tf_evse_get_charging_slot(&device, slot, ret_current, ret_enabled, ret_reset_on_dc);
}

int EVSE::set_charging_slot(uint8_t slot, uint16_t current, bool enabled, bool reset_on_dc)
{
    return tf_evse_set_charging_slot(&device, slot, current, enabled, reset_on_dc);
}

void EVSE::set_charging_slot_max_current(uint8_t slot, uint16_t current)
{
    is_in_bootloader(tf_evse_set_charging_slot_max_current(&device, slot, current));
}

void EVSE::set_charging_slot_clear_on_disconnect(uint8_t slot, bool clear_on_disconnect)
{
    is_in_bootloader(tf_evse_set_charging_slot_clear_on_disconnect(&device, slot, clear_on_disconnect));
}

void EVSE::set_charging_slot_active(uint8_t slot, bool enabled)
{
    tf_evse_set_charging_slot_active(&device, slot, enabled);
}

int EVSE::get_charging_slot_default(uint8_t slot, uint16_t *ret_max_current, bool *ret_enabled, bool *ret_clear_on_disconnect)
{
    return tf_evse_get_charging_slot_default(&device, slot, ret_max_current, ret_enabled, ret_clear_on_disconnect);
}

int EVSE::set_charging_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear_on_disconnect)
{
    return tf_evse_set_charging_slot_default(&device, slot, current, enabled, clear_on_disconnect);
}

static const char *debug_header =
    "STATE,"
    "iec61851_state,"
    "charger_state,"
    "contactor_state,"
    "contactor_error,"
    "allowed_charging_current,"
    "error_state,"
    "lock_state,"
    "HARDWARE_CONFIG,"
    "jumper_configuration,"
    "has_lock_switch,"
    "evse_version,"
    "LL_STATE,"
    "led_state,"
    "cp_pwm_duty_cycle,"
    "charging_time,"
    "time_since_state_change,"
    "uptime,"
    "ADC_VALUES,"
    "adc_cp_pe,"
    "adc_pp_pe,"
    "VOLTAGES,"
    "voltage_cp_pe,"
    "voltage_pp_pe,"
    "voltage_cp_pe_high,"
    "RESISTANCES,"
    "resistance_cp_pe,"
    "resistance_pp_pe,"
    "GPIOs,"
    "gpio_input,"
    "gpio_output,"
    "gpio_motor_input_switch,"
    "gpio_contactor,"
    "gpio_motor_fault,"
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

static const size_t debug_header_len = strlen(debug_header);

size_t EVSE::get_debug_header_length() const
{
    return debug_header_len;
}

void EVSE::get_debug_header(StringBuilder *sb)
{
    sb->puts(debug_header, debug_header_len);
}

size_t EVSE::get_debug_line_length() const
{
    return 512; // FIXME: currently max ~290, make tighter estimate
}

void EVSE::get_debug_line(StringBuilder *sb)
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

    // get_all_charging_slots
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
        sb->puts("get_all_data_1 failed");
        return;
    }

    rc = tf_evse_get_all_charging_slots(&device, max_current, active_and_clear_on_disconnect);

    if (rc != TF_E_OK) {
        logger.printfln("get_all_charging_slots %d", rc);
        is_in_bootloader(rc);
        sb->puts("get_all_charging_slots failed");
        return;
    }

    sb->printf(","
             "%u,%u,%u,%u,%u,%u,%u,,"
             "%u,%c,%u,,"
             "%u,%u,%u,%u,%u,,"
             "%u,%u,,"
             "%d,%d,%d,,"
             "%u,%u,,"
             "%c,%c,%c,%c,%c,,"
             "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u",
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
}

void EVSE::update_all_data()
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
    uint8_t jumper_configuration;
    bool has_lock_switch;
    uint8_t evse_version;
    bool boost_mode_enabled;

    // get_all_data_2
    int16_t indication;
    uint16_t duration;
    uint32_t button_press_time;
    uint32_t button_release_time;
    bool button_pressed;

    // get_low_level_state
    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;
    uint16_t adc_values[2];
    int16_t voltages[3];
    uint32_t resistances[2];
    bool gpio[5];
    uint32_t charging_time;
    uint32_t time_since_state_change;
    uint32_t uptime;

    // get_all_charging_slots
    uint16_t max_current[20];
    uint8_t active_and_clear_on_disconnect[20];

    // get_user_calibration
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

    if (contactor_error_changed) {
        if (contactor_error != 0) {
            logger.printfln("Contactor error %d", contactor_error);
        } else {
            logger.printfln("Contactor error cleared");
        }
    }

    if (error_state_changed) {
        if (error_state != 0) {
            logger.printfln("Error state %d", error_state);
        } else {
            logger.printfln("Error state cleared");
        }
    }

    // get_hardware_configuration
    evse_common.hardware_configuration.get("jumper_configuration")->updateUint(jumper_configuration);
    evse_common.hardware_configuration.get("has_lock_switch")->updateBool(has_lock_switch);
    evse_common.hardware_configuration.get("evse_version")->updateUint(evse_version);

    // get_low_level_state
    evse_common.low_level_state.get("led_state")->updateUint(led_state);
    evse_common.low_level_state.get("cp_pwm_duty_cycle")->updateUint(cp_pwm_duty_cycle);

    for (int i = 0; i < ARRAY_SIZE(adc_values); ++i)
        evse_common.low_level_state.get("adc_values")->get(i)->updateUint(adc_values[i]);

    for (int i = 0; i < ARRAY_SIZE(voltages); ++i)
        evse_common.low_level_state.get("voltages")->get(i)->updateInt(voltages[i]);

    for (int i = 0; i < ARRAY_SIZE(resistances); ++i)
        evse_common.low_level_state.get("resistances")->get(i)->updateUint(resistances[i]);

    for (int i = 0; i < ARRAY_SIZE(gpio); ++i)
        evse_common.low_level_state.get("gpio")->get(i)->updateBool(gpio[i]);

    evse_common.low_level_state.get("charging_time")->updateUint(charging_time);
    evse_common.low_level_state.get("time_since_state_change")->updateUint(time_since_state_change);
    evse_common.low_level_state.get("uptime")->updateUint(uptime);

    for (int i = 0; i < CHARGING_SLOT_COUNT; ++i) {
        evse_common.slots.get(i)->get("max_current")->updateUint(max_current[i]);
        evse_common.slots.get(i)->get("active")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[i]));
        evse_common.slots.get(i)->get("clear_on_disconnect")->updateBool(SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[i]));
    }

    evse_common.auto_start_charging.get("auto_start_charging")->updateBool(
        !evse_common.slots.get(CHARGING_SLOT_AUTOSTART_BUTTON)->get("clear_on_disconnect")->asBool());

    // get_button_state
    evse_common.button_state.get("button_press_time")->updateUint(button_press_time);
    evse_common.button_state.get("button_release_time")->updateUint(button_release_time);
    evse_common.button_state.get("button_pressed")->updateBool(button_pressed);

    evse_common.boost_mode.get("enabled")->updateBool(boost_mode_enabled);

    // get_indicator_led
    evse_common.indicator_led.get("indication")->updateInt(indication);
    evse_common.indicator_led.get("duration")->updateUint(duration);

    evse_common.auto_start_charging.get("auto_start_charging")->updateBool(!SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[CHARGING_SLOT_AUTOSTART_BUTTON]));

    evse_common.management_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_CHARGE_MANAGER]));

    evse_common.user_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_USER]));

    evse_common.modbus_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_MODBUS_TCP]));
    evse_common.ocpp_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_OCPP]));

    if (evse_common.external_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_EXTERNAL]))) {
#if MODULE_AUTOMATION_AVAILABLE()
        automation.set_enabled(AutomationTriggerID::EVSEExternalCurrentWd, evse_common.external_enabled.get("enabled")->asBool());
#endif
    }

    evse_common.external_clear_on_disconnect.get("clear_on_disconnect")->updateBool(SLOT_CLEAR_ON_DISCONNECT(active_and_clear_on_disconnect[CHARGING_SLOT_EXTERNAL]));

    evse_common.global_current.get("current")->updateUint(max_current[CHARGING_SLOT_GLOBAL]);
    evse_common.management_current.get("current")->updateUint(max_current[CHARGING_SLOT_CHARGE_MANAGER]);
    evse_common.external_current.get("current")->updateUint(max_current[CHARGING_SLOT_EXTERNAL]);
    evse_common.user_current.get("current")->updateUint(max_current[CHARGING_SLOT_USER]);

    evse_common.external_defaults.get("current")->updateUint(external_default_current);
    evse_common.external_defaults.get("clear_on_disconnect")->updateBool(external_default_clear_on_disconnect);

    evse_common.require_meter_enabled.get("enabled")->updateBool(SLOT_ACTIVE(active_and_clear_on_disconnect[CHARGING_SLOT_REQUIRE_METER]));

    // get_user_calibration
    user_calibration.get("user_calibration_active")->updateBool(user_calibration_active);
    user_calibration.get("voltage_diff")->updateInt(voltage_diff);
    user_calibration.get("voltage_mul")->updateInt(voltage_mul);
    user_calibration.get("voltage_div")->updateInt(voltage_div);
    user_calibration.get("resistance_2700")->updateInt(resistance_2700);

    for (int i = 0; i < ARRAY_SIZE(resistance_880); ++i)
        user_calibration.get("resistance_880")->get(i)->updateInt(resistance_880[i]);

#if MODULE_WATCHDOG_AVAILABLE()
    static size_t watchdog_handle = watchdog.add("evse_all_data", "EVSE not reachable");
    watchdog.reset(watchdog_handle);
#endif
}
