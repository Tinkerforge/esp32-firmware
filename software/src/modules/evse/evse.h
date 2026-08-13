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

#include "device_module.h"
#include "config.h"
#include "bindings/bricklet_evse.h"
#include "modules/evse_common/evse_common.h"

struct EVSEAllData {
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
    int16_t indication;
    uint16_t duration;
    uint32_t button_press_time;
    uint32_t button_release_time;
    bool button_pressed;
    uint8_t led_state;
    uint16_t cp_pwm_duty_cycle;
    uint16_t adc_values[2];
    int16_t voltages[3];
    uint32_t resistances[2];
    bool gpio[5];
    bool car_stopped_charging;
    uint32_t time_since_state_change;
    uint32_t uptime;

    // get_all_charging_slots
    uint16_t max_current[20];
    uint8_t active_and_clear_on_disconnect[20];

    // get_charging_slot_default(CHARGING_SLOT_EXTERNAL)
    uint16_t external_default_current;
    bool external_default_enabled;
    bool external_default_clear_on_disconnect;

    // get_user_calibration
    bool user_calibration_active;
    int16_t voltage_diff;
    int16_t voltage_mul;
    int16_t voltage_div;
    int16_t resistance_2700;
    int16_t resistance_880[14];

    // Set by fetch_all_data() on success, cleared on failure.
    bool valid;
};

class EVSE final : public DeviceModule<TF_EVSE,
                                       tf_evse_create,
                                       tf_evse_get_bootloader_mode,
                                       tf_evse_reset,
                                       tf_evse_destroy>,
                   public IEvseBackend
{
public:
    EVSE();

    // IModule implementation (inherited through DeviceModule and IEvseBackend)
    void pre_init() override;
    void pre_setup() override;
    void setup() override {}; // Override empty: Base method sets initialized to true, but we want EvseCommon to decide this.
    void register_urls() override {this->DeviceModule::register_urls();};

protected:
    // IEvseBackend implementation
    bool is_initialized() override { return initialized; }
    void set_initialized(bool initialized) override { this->initialized = initialized; }

    void post_setup() override {};
    void post_register_urls() override;

    bool setup_device() override {return this->DeviceModule::setup_device();}
    bool is_in_bootloader(int rc) override  {return this->DeviceModule::is_in_bootloader(rc);}

    void factory_reset() override;
    void reset() override { this->DeviceModule::reset(); }

    void set_data_storage(uint8_t page, const uint8_t *data) override;
    void get_data_storage(uint8_t page, uint8_t *data) override;

    void set_indicator_led(int16_t indication, uint16_t duration, uint16_t color_h, uint8_t color_s, uint8_t color_v, uint8_t *ret_status) override;

    // Not supported, does nothing
    void set_control_pilot_disconnect(bool cp_disconnect, bool *cp_disconnected) override {
        (void)cp_disconnect; // not supported
        (void)cp_disconnected;
    }
    // Not supported, always returns false
    bool get_control_pilot_disconnect() override { return false; }

    void set_boost_mode(bool enabled) override;

    int get_charging_slot(uint8_t slot, uint16_t *ret_current, bool *ret_enabled, bool *ret_reset_on_dc) override;
    int set_charging_slot(uint8_t slot, uint16_t current, bool enabled, bool reset_on_dc) override;
    void set_charging_slot_max_current(uint8_t slot, uint16_t current) override;
    void set_charging_slot_clear_on_disconnect(uint8_t slot, bool clear_on_disconnect) override;
    void set_charging_slot_active(uint8_t slot, bool enabled) override;
    int get_charging_slot_default(uint8_t slot, uint16_t *ret_max_current, bool *ret_enabled, bool *ret_clear_on_disconnect) override;
    int set_charging_slot_default(uint8_t slot, uint16_t current, bool enabled, bool clear_on_disconnect) override;

    [[gnu::const]] size_t get_debug_header_length() const override;
    void get_debug_header(StringBuilder *sb) override;
    [[gnu::const]] size_t get_debug_line_length() const override;
    void get_debug_line(StringBuilder *sb) override;
    void update_all_data() override;
    void fetch_all_data() override;
    void publish_all_data() override;
    //End IEvseBackend implementation

    // PhaseSwitcherBackend implementation
    uint32_t get_phase_switcher_priority()                           override {return 4;}
    bool phase_switching_capable()                                   override {return false;}
    bool can_switch_phases_now(uint32_t phases_wanted)               override {return false;}
    uint32_t get_phases()                                            override {return 3;}
    PhaseSwitcherBackend::SwitchingState get_phase_switching_state() override {return PhaseSwitcherBackend::SwitchingState::Ready;} // Don't report an error when phase_switching_capable() is false.
    bool switch_phases(uint32_t phases_wanted)                       override {return false;}
    bool is_external_control_allowed()                               override {return false;}

    ConfigRoot user_calibration;

private:
    void get_debug_line_impl(StringBuilder *sb);

    EVSEAllData all_data;
};

#include "generated/module_available_end.h"
