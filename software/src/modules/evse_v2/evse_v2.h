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

#pragma once

#include "bindings/bricklet_evse_v2.h"

#include "config.h"
#include "device_module.h"
#include "evse_v2_bricklet_firmware_bin.embedded.h"

#define CHARGING_SLOT_COUNT 14
#define CHARGING_SLOT_COUNT_SUPPORTED_BY_EVSE 20

#define CHARGING_SLOT_INCOMING_CABLE 0
#define CHARGING_SLOT_OUTGOING_CABLE 1
#define CHARGING_SLOT_SHUTDOWN_INPUT 2
#define CHARGING_SLOT_GP_INPUT 3
#define CHARGING_SLOT_AUTOSTART_BUTTON 4
#define CHARGING_SLOT_GLOBAL 5
#define CHARGING_SLOT_USER 6
#define CHARGING_SLOT_CHARGE_MANAGER 7
#define CHARGING_SLOT_EXTERNAL 8
#define CHARGING_SLOT_MODBUS_TCP 9
#define CHARGING_SLOT_MODBUS_TCP_ENABLE 10
#define CHARGING_SLOT_OCPP 11
#define CHARGING_SLOT_CHARGE_LIMITS 12
#define CHARGING_SLOT_REQUIRE_METER 13

#define IEC_STATE_A 0
#define IEC_STATE_B 1
#define IEC_STATE_C 2
#define IEC_STATE_D 3
#define IEC_STATE_EF 4

#define CHARGER_STATE_NOT_PLUGGED_IN 0
#define CHARGER_STATE_WAITING_FOR_RELEASE 1
#define CHARGER_STATE_READY_TO_CHARGE 2
#define CHARGER_STATE_CHARGING 3
#define CHARGER_STATE_ERROR 4

#define DATA_STORE_PAGE_CHARGE_TRACKER 0
#define DATA_STORE_PAGE_RECOVERY 15

void evse_v2_button_recovery_handler();
#define TF_ESP_PREINIT evse_v2_button_recovery_handler();

class EVSEV2 : public DeviceModule<TF_EVSEV2,
                                   evse_v2_bricklet_firmware_bin_data,
                                   evse_v2_bricklet_firmware_bin_length,
                                   tf_evse_v2_create,
                                   tf_evse_v2_get_bootloader_mode,
                                   tf_evse_v2_reset,
                                   tf_evse_v2_destroy>
{
public:
    EVSEV2() : DeviceModule("evse", "EVSE 2.0", "EVSE 2.0", std::bind(&EVSEV2::setup_evse, this)) {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;

    // Called in evse_v2_meter setup
    void update_all_data();

    void setup_evse();
    String get_evse_debug_header();
    String get_evse_debug_line();
    void set_managed_current(uint16_t current);

    void set_user_current(uint16_t current);

    void set_modbus_current(uint16_t current);
    void set_modbus_enabled(bool enabled);

    void set_require_meter_blocking(bool blocking);
    void set_require_meter_enabled(bool enabled);
    bool get_require_meter_blocking();
    bool get_require_meter_enabled();

    void set_charge_limits_slot(uint16_t current, bool enabled);
    //void set_charge_time_restriction_slot(uint16_t current, bool enabled);

    void set_ocpp_current(uint16_t current);
    uint16_t get_ocpp_current();

    bool apply_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear);
    void apply_defaults();

    void factory_reset();

    uint16_t get_all_energy_meter_values(float *ret_values);
    void reset_energy_meter_relative_energy();
    void set_data_storage(uint8_t page, const uint8_t *data);
    void get_data_storage(uint8_t page, uint8_t *data);
    void set_indicator_led(int16_t indication, uint16_t duration, uint8_t *ret_status);

    void check_debug();

    bool debug = false;

    ConfigRoot evse_state;
    ConfigRoot evse_hardware_configuration;
    ConfigRoot evse_low_level_state;
    ConfigRoot evse_energy_meter_values;
    ConfigRoot evse_energy_meter_errors;
    ConfigRoot evse_button_state;
    ConfigRoot evse_slots;
    ConfigRoot evse_indicator_led;
    ConfigRoot evse_reset_dc_fault_current_state;
    ConfigRoot evse_gpio_configuration;
    ConfigRoot evse_gpio_configuration_update;
    ConfigRoot evse_button_configuration;
    ConfigRoot evse_button_configuration_update;
    ConfigRoot evse_auto_start_charging;
    ConfigRoot evse_auto_start_charging_update;
    ConfigRoot evse_global_current;
    ConfigRoot evse_global_current_update;
    ConfigRoot evse_management_enabled;
    ConfigRoot evse_management_enabled_update;
    ConfigRoot evse_user_current;
    ConfigRoot evse_user_enabled;
    ConfigRoot evse_user_enabled_update;
    ConfigRoot evse_external_enabled;
    ConfigRoot evse_external_enabled_update;
    ConfigRoot evse_external_defaults;
    ConfigRoot evse_external_defaults_update;
    ConfigRoot evse_management_current;
    ConfigRoot evse_management_current_update;
    ConfigRoot evse_external_current;
    ConfigRoot evse_external_current_update;
    ConfigRoot evse_external_clear_on_disconnect;
    ConfigRoot evse_external_clear_on_disconnect_update;
    ConfigRoot evse_modbus_enabled;
    ConfigRoot evse_modbus_enabled_update;
    ConfigRoot evse_ocpp_enabled;
    ConfigRoot evse_ocpp_enabled_update;
    ConfigRoot evse_ev_wakeup;
    ConfigRoot evse_ev_wakeup_update;
    ConfigRoot evse_boost_mode;
    ConfigRoot evse_boost_mode_update;
    ConfigRoot evse_control_pilot_disconnect;
    ConfigRoot evse_control_pilot_disconnect_update;
    ConfigRoot evse_require_meter_enabled;
    ConfigRoot evse_require_meter_enabled_update;
    ConfigRoot evse_gp_output;
    ConfigRoot evse_gp_output_update;

    uint32_t last_current_update = 0;
    bool shutdown_logged = false;
    uint32_t last_debug_check = 0;
};
