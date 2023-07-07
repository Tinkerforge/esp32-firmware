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

#include "bindings/bricklet_evse.h"

#include "config.h"
#include "device_module.h"
#include "evse_bricklet_firmware_bin.embedded.h"
#include "../evse_common/evse_common.h"

class EVSE final : public DeviceModule<TF_EVSE,
                                       evse_bricklet_firmware_bin_data,
                                       evse_bricklet_firmware_bin_length,
                                       tf_evse_create,
                                       tf_evse_get_bootloader_mode,
                                       tf_evse_reset,
                                       tf_evse_destroy>, public IEvseBackend {
public:
    EVSE();

    // IModule implementation (inherited through DeviceModule and IEvseBackend)
    void pre_setup() override;
    void setup() override {}; // Override empty: Base method sets initialized to true, but we want EvseCommon to decide this.
    void register_urls() override {this->DeviceModule::register_urls();};
    void loop() override {this->DeviceModule::loop();};

protected:
    // IEvseBackend implementation
    void post_setup() override {};
    void post_register_urls() override;

    bool setup_device() override {return this->DeviceModule::setup_device();}
    bool is_in_bootloader(int rc) override  {return this->DeviceModule::is_in_bootloader(rc);}

    void factory_reset() override;
    void reset() override { this->DeviceModule::reset(); }

    void set_data_storage(uint8_t page, const uint8_t *data) override;
    void get_data_storage(uint8_t page, uint8_t *data) override;

    void set_indicator_led(int16_t indication, uint16_t duration, uint8_t *ret_status) override;

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

    String get_evse_debug_header() override;
    String get_evse_debug_line() override;
    void update_all_data() override;
    //End IEvseBackend implementation

    ConfigRoot user_calibration;
};
