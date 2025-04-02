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
#include "config_charge_mode.enum.h"

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

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

    size_t get_charger_count();

    const String &get_charger_host(uint8_t idx);
    const char *get_charger_name(uint8_t idx);
    bool is_only_proxy() { return this->config.get("chargers")->count() == 1 && !this->config.get("enable_charge_manager")->asBool(); }

    uint32_t get_maximum_available_current();

    CurrentLimits *get_limits() {
        // TODO: Maybe add separate function for this?
        static_cm = false;
        return &limits;
    }
    const Cost *get_allocated_currents() {return &allocated_currents;}

    void trigger_allocator_run() {next_allocation = 0_us;}
    void skip_global_hysteresis();
    void enable_fast_single_charger_mode();

    const ChargerState *get_charger_state(uint8_t idx);
    ChargerState *get_mutable_charger_state(uint8_t idx);

    size_t trace_buffer_index;

    uint8_t translate_charge_mode(ConfigChargeMode power_manager_charge_mode);

private:
    bool seen_all_chargers();
    void start_manager_task();
    void check_watchdog();

    void update_charger_state_config(uint8_t idx);
    void update_charger_state_from_mode(ChargerState *state, int charger_idx);

    size_t charger_count = 0;

    micros_t last_available_current_update = 0_us;

    ChargerState *charger_state = nullptr;

    ConfigRoot config;
    ConfigRoot low_level_config;

    ConfigRoot state;
    ConfigRoot low_level_state;

    ConfigRoot available_current;
    ConfigRoot available_current_update;

    ConfigRoot control_pilot_disconnect;

    Config config_chargers_prototype;
    Config state_chargers_prototype;
    Config low_level_state_chargers_prototype;

    ConfigRoot charge_mode;

#ifdef DEBUG_FS_ENABLE
    ConfigRoot debug_limits_update;
#endif

    CurrentLimits limits, limits_post_allocation;
    Cost allocated_currents;

    micros_t next_allocation = 0_us;
    bool static_cm = true;

    bool all_chargers_seen = false;

    bool watchdog_triggered = false;

    std::unique_ptr<const char *[]> hosts;
    uint16_t requested_current_threshold;
    uint16_t requested_current_margin;

    uint32_t guaranteed_pv_current;
    ConfigChargeMode pm_default_charge_mode;

    ChargerAllocationState *charger_allocation_state = nullptr;
    CurrentAllocatorConfig *ca_config = nullptr;
    CurrentAllocatorState *ca_state = nullptr;
};

#include "module_available_end.h"
