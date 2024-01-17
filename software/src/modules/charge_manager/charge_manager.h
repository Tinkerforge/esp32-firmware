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
#include "module_dependencies.h"

class ChargeManager final : public IModule
{
public:
    ChargeManager(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void distribute_current();
    void start_manager_task();
    void check_watchdog();
    bool have_chargers();
    bool seen_all_chargers();
    bool is_charging_stopped(uint32_t last_update_cutoff);
    void set_all_control_pilot_disconnect(bool disconnect);
    bool are_all_control_pilot_disconnected(uint32_t last_update_cutoff);
    bool is_control_pilot_disconnect_supported(uint32_t last_update_cutoff);
    void set_allocated_current_callback(std::function<void(uint32_t)> callback);

    const char *get_charger_name(uint8_t idx);

#if MODULE_AUTOMATION_AVAILABLE()
    bool action_triggered(Config *config, void *data);
#endif

    ConfigRoot config;

    ConfigRoot state;

    ConfigRoot available_current;
    ConfigRoot available_current_update;
    ConfigRoot available_phases;
    ConfigRoot available_phases_update;
    ConfigRoot control_pilot_disconnect;

    uint32_t last_available_current_update = 0;
    bool watchdog_triggered = false;

    struct ChargerState {
        uint32_t last_update;
        uint32_t uid;
        uint32_t uptime;
        uint32_t last_sent_config;
        uint32_t power_total_count;
        float power_total_sum;
        float energy_abs;

        // last current limit send to the charger
        uint16_t allocated_current;

        // maximum current supported by the charger
        uint16_t supported_current;

        // last current limit reported by the charger
        uint16_t allowed_current;

        // requested current calculated with the line currents reported by the charger
        uint16_t requested_current;

        // 0 - no vehicle, 1 - user blocked, 2 - manager blocked, 3 - car blocked, 4 - charging, 5 - error, 6 - charged
        uint8_t state;

        // 0 - okay, 1 - unreachable, 2 - FW mismatch, 3 - not managed
        uint8_t error;
        uint8_t charger_state;
        bool wants_to_charge;
        bool wants_to_charge_low_priority;
        bool is_charging;

        // last CP disconnect support reported by the charger: false - CP disconnect not supported, true - CP disconnect supported
        bool cp_disconnect_supported;

        // last CP disconnect state reported by the charger: false - automatic, true - disconnected
        bool cp_disconnect_state;

        // last CP disconnect request sent to charger: false - automatic/don't care, true - disconnect
        bool cp_disconnect;
        bool meter_supported;
    };

    ChargerState *charger_state = nullptr;
    size_t charger_count = 0;

private:
    bool all_chargers_seen = false;
    bool printed_all_chargers_seen = false;
    std::function<void(uint32_t)> allocated_current_callback;

    std::unique_ptr<char[]> distribution_log;

    std::unique_ptr<const char *[]> hosts;
    uint32_t default_available_current;
    uint32_t minimum_current;
    uint32_t minimum_current_1p;
    uint16_t requested_current_threshold;
    uint16_t requested_current_margin;
};
