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

#include "config.h"

#include "module.h"

class ChargeManager final : public IModule
{
public:
    ChargeManager(){}
    void pre_setup();
    void setup();
    void register_urls();

    void start_evse_state_update();
    void send_current();
    void distribute_current();
    void start_manager_task();
    void check_watchdog();
    void set_available_current(uint32_t current);
    bool have_chargers();
    bool seen_all_chargers();
    bool is_charging_stopped(uint32_t last_update_cutoff);
    void set_all_control_pilot_disconnect(bool disconnect);
    bool are_all_control_pilot_disconnected(uint32_t last_update_cutoff);
    bool is_control_pilot_disconnect_supported(uint32_t last_update_cutoff);
    void set_allocated_current_callback(std::function<void(uint32_t)> callback);

    ConfigRoot charge_manager_config;
    ConfigRoot charge_manager_config_in_use;

    ConfigRoot charge_manager_state;

    ConfigRoot charge_manager_available_current;
    ConfigRoot charge_manager_control_pilot_disconnect;

    uint32_t last_available_current_update = 0;

private:
    bool all_chargers_seen = false;
    std::function<void(uint32_t)> allocated_current_callback;

    std::unique_ptr<char[]> distribution_log;
};
