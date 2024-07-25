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

#include "module.h"
#include "config.h"
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

#include "current_limits.h"

struct CurrentAllocatorConfig;
struct CurrentAllocatorState;
struct ChargerState;
struct ChargerAllocationState;

class ChargeManager final : public IModule
#if MODULE_AUTOMATION_AVAILABLE()
                          , public IAutomationBackend
#endif
{
public:
    ChargeManager(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void start_manager_task();
    void check_watchdog();
    bool get_charger_count();
    bool seen_all_chargers();
    bool is_charging_stopped(uint32_t last_update_cutoff);
    void set_all_control_pilot_disconnect(bool disconnect);
    bool are_all_control_pilot_disconnected(uint32_t last_update_cutoff);
    bool is_control_pilot_disconnect_supported(uint32_t last_update_cutoff);
    void set_allocated_current_callback(std::function<void(uint32_t)> &&callback);

    const char *get_charger_name(uint8_t idx);

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

    void update_charger_state_config(uint8_t idx);

    ConfigRoot config;
    ConfigRoot low_level_config;

    ConfigRoot state;
    ConfigRoot low_level_state;

    ConfigRoot available_current;
    ConfigRoot available_current_update;
    ConfigRoot control_pilot_disconnect;

    uint32_t last_available_current_update = 0;
    bool watchdog_triggered = false;

    size_t charger_count = 0;
    ChargerState *charger_state = nullptr;

    CurrentLimits *get_limits() {
        // TODO: Maybe add separate function for this?
        static_cm = false;
        return &limits;
    }
    const Cost *get_allocated_currents() {return &allocated_currents;}

    void trigger_allocator_run() {next_allocation = 0_us;}

private:
    CurrentLimits limits, limits_post_allocation;
    Cost allocated_currents;

    micros_t next_allocation = 0_us;
    bool static_cm = true;

    bool all_chargers_seen = false;

    std::unique_ptr<const char *[]> hosts;
    uint32_t default_available_current;
    uint16_t requested_current_threshold;
    uint16_t requested_current_margin;

    ChargerAllocationState *charger_allocation_state = nullptr;
    CurrentAllocatorConfig *ca_config = nullptr;
    CurrentAllocatorState *ca_state = nullptr;
};
