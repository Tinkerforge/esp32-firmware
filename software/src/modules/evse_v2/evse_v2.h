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
#include "modules/evse_common/evse_common.h"
#include "bindings/bricklet_evse_v2.h"
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

#define EVSEV2_PHASES_INFO_1P_CAR_MASK (1 << 0)

struct EVSEV2MeterData {
    bool phases_active[3];
    bool phases_connected[3];
    uint8_t meter_type;
    float power;
    float currents[3];
    uint32_t error_count[6];
};

class EVSEV2 final : public DeviceModule<TF_EVSEV2,
                                         tf_evse_v2_create,
                                         tf_evse_v2_get_bootloader_mode,
                                         tf_evse_v2_reset,
                                         tf_evse_v2_destroy>,
                     public IEvseBackend
#if MODULE_AUTOMATION_AVAILABLE()
                   , public IAutomationBackend
#endif
{
public:
    EVSEV2();

    // IModule implementation (inherited through DeviceModule and IEvseBackend)
    void pre_init() override;
    void pre_setup() override;
    void setup() override {}; // Override empty: Base method sets initialized to true, but we want EvseCommon to decide this.
    void register_urls() override {this->DeviceModule::register_urls();};
    void register_events() override;
    void loop() override {this->DeviceModule::loop();};

protected:
    // IEvseBackend implementation
    bool is_initialized() override { return initialized; }
    void set_initialized(bool initialized) override { this->initialized = initialized; }

    void post_setup() override;
    void post_register_urls() override;

    bool setup_device() override {return this->DeviceModule::setup_device();}
    bool is_in_bootloader(int rc) override {return this->DeviceModule::is_in_bootloader(rc);}

    void factory_reset() override;
    void reset() override { this->DeviceModule::reset(); }

    void set_data_storage(uint8_t page, const uint8_t *data) override;
    void get_data_storage(uint8_t page, uint8_t *data) override;

    void set_indicator_led(int16_t indication, uint16_t duration, uint16_t color_h, uint8_t color_s, uint8_t color_v, uint8_t *ret_status) override;

    void set_control_pilot_disconnect(bool cp_disconnect, bool *cp_disconnected) override;
    bool get_control_pilot_disconnect() override;

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

    // PhaseSwitcherBackend implementation
    uint32_t get_phase_switcher_priority() override {return 12;}
    bool phase_switching_capable() override;
    bool can_switch_phases_now(uint32_t phases_wanted) override;
    bool requires_cp_disconnect() override {return false;}
    uint32_t get_phases() override;
    PhaseSwitcherBackend::SwitchingState get_phase_switching_state() override;
    bool switch_phases(uint32_t phases_wanted) override;
    bool is_external_control_allowed() override;

// To allow the meters_evse_v2 module to get/set energy meter values
public:
    void update_all_data() override;
    // End IEvseBackend implementation

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif
    uint16_t get_all_energy_meter_values(float *ret_values);
    bool reset_energy_meter_relative_energy();
    uint8_t get_energy_meter_type();

private:
    ConfigRoot reset_dc_fault_current_state;
    ConfigRoot gpio_configuration;
    ConfigRoot gpio_configuration_update;
    ConfigRoot button_configuration;
    ConfigRoot button_configuration_update;
    ConfigRoot ev_wakeup;
    ConfigRoot ev_wakeup_update;
    ConfigRoot phase_auto_switch;
    ConfigRoot phase_auto_switch_update;
    ConfigRoot phases_connected;
    ConfigRoot phases_connected_update;
    ConfigRoot control_pilot_disconnect;
    ConfigRoot control_pilot_disconnect_update;
    ConfigRoot gp_output;
    ConfigRoot gp_output_update;
};

#include "module_available_end.h"
