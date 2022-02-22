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

#define CHARGING_SLOT_INCOMING_CABLE 0
#define CHARGING_SLOT_OUTGOING_CABLE 1
#define CHARGING_SLOT_SHUTDOWN_INPUT 2
#define CHARGING_SLOT_GP_INPUT 3
#define CHARGING_SLOT_AUTOSTART_BUTTON 4
#define CHARGING_SLOT_GLOBAL 5
#define CHARGING_SLOT_USER 6
#define CHARGING_SLOT_CHARGE_MANAGER 7
#define CHARGING_SLOT_EXTERNAL 8

#define IEC_STATE_A 0
#define IEC_STATE_B 1
#define IEC_STATE_C 2
#define IEC_STATE_D 3
#define IEC_STATE_EF 4

class EVSEV2 : public DeviceModule<TF_EVSEV2,
                                   evse_v2_bricklet_firmware_bin_data,
                                   evse_v2_bricklet_firmware_bin_length,
                                   tf_evse_v2_create,
                                   tf_evse_v2_get_bootloader_mode,
                                   tf_evse_v2_reset,
                                   tf_evse_v2_destroy>  {
public:
    EVSEV2();
    void setup();
    void register_urls();
    void loop();


    // Called in evse_v2_meter setup
    void update_all_data();

    void setup_evse();
    bool flash_firmware();
    bool flash_plugin(int regular_plugin_upto);
    bool wait_for_bootloader_mode(int mode);
    String get_evse_debug_header();
    String get_evse_debug_line();
    void set_managed_current(uint16_t current);

    void set_user_current(uint16_t current);

    bool apply_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear);
    void apply_defaults();

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
    ConfigRoot evse_stop_charging;
    ConfigRoot evse_start_charging;
    ConfigRoot evse_gpio_configuration;
    ConfigRoot evse_gpio_configuration_update;
    ConfigRoot evse_button_configuration;
    ConfigRoot evse_button_configuration_update;
    ConfigRoot evse_control_pilot_configuration;
    ConfigRoot evse_control_pilot_configuration_update;
    ConfigRoot evse_auto_start_charging;
    ConfigRoot evse_auto_start_charging_update;
    ConfigRoot evse_global_current;
    ConfigRoot evse_global_current_update;
    ConfigRoot evse_management_enabled;
    ConfigRoot evse_management_enabled_update;
    ConfigRoot evse_user_slot_enabled;
    ConfigRoot evse_user_slot_enabled_update;
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

    uint32_t last_current_update = 0;
    bool shutdown_logged = false;
};
