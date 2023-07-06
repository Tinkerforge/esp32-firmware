/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "config.h"
#include "module.h"

class IEvseBackend : virtual public IModule {
public:
    IEvseBackend() {}
    virtual ~IEvseBackend() {}

    virtual void post_setup() = 0;
    virtual void post_register_urls() = 0;

    virtual String get_evse_debug_header() = 0;
    virtual String get_evse_debug_line() = 0;

    virtual int get_charging_slot(uint8_t, uint16_t*, bool*, bool*) = 0;
    virtual int set_charging_slot(uint8_t, uint16_t, bool, bool) = 0;

    virtual void update_all_data() = 0;

    virtual void factory_reset() = 0;

    virtual void set_data_storage(uint8_t, const uint8_t*) = 0;
    virtual void get_data_storage(uint8_t, uint8_t*) = 0;
    virtual void set_indicator_led(int16_t, uint16_t, uint8_t*) = 0;

    virtual void check_debug() = 0;

    virtual void set_control_pilot_disconnect(bool, bool*) = 0;
    virtual bool get_control_pilot_disconnect() = 0;

    virtual void set_boost_mode(bool enabled) = 0;

    virtual void set_charging_slot_max_current(uint8_t slot, uint16_t current) = 0;
    virtual void set_charging_slot_clear_on_disconnect(uint8_t slot, bool clear_on_disconnect) = 0;
    virtual void set_charging_slot_active(uint8_t slot, bool enabled) = 0;
    virtual int get_charging_slot_default(uint8_t slot, uint16_t *ret_max_current, bool *ret_enabled, bool *ret_clear_on_disconnect) = 0;
    virtual int set_charging_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear_on_disconnect) = 0;

    virtual bool is_in_bootloader(int rc) = 0;

    ConfigRoot low_level_state;
    ConfigRoot management_enabled;
    ConfigRoot management_enabled_update;
    ConfigRoot state;
    ConfigRoot slots;
    ConfigRoot hardware_configuration;
    ConfigRoot indicator_led;
    ConfigRoot button_state;
    ConfigRoot external_current;
    ConfigRoot external_current_update;
    ConfigRoot external_clear_on_disconnect;
    ConfigRoot external_clear_on_disconnect_update;
    ConfigRoot management_current;
    ConfigRoot management_current_update;
    ConfigRoot boost_mode;
    ConfigRoot boost_mode_update;
    ConfigRoot auto_start_charging;
    ConfigRoot auto_start_charging_update;
    ConfigRoot global_current;
    ConfigRoot global_current_update;
    ConfigRoot user_current;
    ConfigRoot user_enabled;
    ConfigRoot user_enabled_update;
    ConfigRoot external_enabled;
    ConfigRoot external_enabled_update;
    ConfigRoot external_defaults;
    ConfigRoot external_defaults_update;
    ConfigRoot modbus_enabled;
    ConfigRoot modbus_enabled_update;
    ConfigRoot ocpp_enabled;
    ConfigRoot ocpp_enabled_update;
    ConfigRoot require_meter_enabled;
    ConfigRoot require_meter_enabled_update;

    uint32_t last_debug_check = 0;

    bool debug = false;
};

class EvseCommon final : public IModule {
private:
    IEvseBackend *backend = NULL;

public:
    EvseCommon();

    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;

    void set_managed_current(uint16_t);

    void set_user_current(uint16_t);

    void set_modbus_current(uint16_t);
    void set_modbus_enabled(bool);

    void set_require_meter_blocking(bool);
    void set_require_meter_enabled(bool);
    bool get_require_meter_blocking();
    bool get_require_meter_enabled();

    void set_charge_limits_slot(uint16_t, bool);

    void set_ocpp_current(uint16_t);
    uint16_t get_ocpp_current();

    void factory_reset();

    void set_data_storage(uint8_t, const uint8_t*);
    void get_data_storage(uint8_t, uint8_t*);
    void set_indicator_led(int16_t, uint16_t, uint8_t*);

    bool apply_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear);
    void apply_defaults();

    ConfigRoot& get_state();
    ConfigRoot& get_slots();
    ConfigRoot& get_low_level_state();
    bool get_management_enabled();

    uint32_t last_current_update = 0;
    bool shutdown_logged = false;
};
