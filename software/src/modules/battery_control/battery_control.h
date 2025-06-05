/* esp32-firmware
 * Copyright (C) 2025 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <stdint.h>

#include "config.h"
#include "module.h"
#include "TFTools/Micros.h"
#include "tools/tristate_bool.h"

struct battery_control_action_info;

class BatteryControl final : public IModule
{
public:
    BatteryControl() {}

    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

private:
    enum class RuleCondition : uint8_t {
        Ignore = 0,
        Below  = 1,
        Above  = 2,
    };

    struct control_rule {
        int32_t  price_th;    // in ct/1000
        int32_t  forecast_th; // in Wh
        uint8_t  soc_th;      // in percent
        RuleCondition soc_cond;
        RuleCondition price_cond;
        RuleCondition forecast_cond;
    };

    void preprocess_rules(const Config *rules_config, control_rule *rules, size_t rules_count);

    void update_avg_soc();
    void schedule_evaluation();
    bool rule_condition_failed(BatteryControl::RuleCondition cond, int32_t th, int32_t value);
    TristateBool evaluate_rules(const control_rule *rules, size_t rules_count);
    void evaluate_all_rules();
    void evaluate_summary();
    void periodic_update();

    void update_batteries_and_state(bool influence_active, bool changed, const battery_control_action_info *action, micros_t *next_update);
    void update_charge_permitted(bool changed);
    void update_discharge_forbidden(bool changed);
    void update_charge_forbidden(bool changed);

    micros_t rewrite_period = 0_us;
    micros_t next_permit_grid_charge_update  = 0_us;
    micros_t next_discharge_forbidden_update = 0_us;
    micros_t next_charge_forbidden_update    = 0_us;

    ConfigRoot config;
    ConfigRoot rules_permit_grid_charge;
    ConfigRoot rules_forbid_discharge;
    ConfigRoot rules_forbid_charge;
    ConfigRoot low_level_config;
    ConfigRoot state;

    Config rule_prototype;

    uint64_t evaluation_task_id = 0;

    const control_rule *permit_grid_charge_rules = nullptr;
    const control_rule *forbid_discharge_rules   = nullptr;
    const control_rule *forbid_charge_rules      = nullptr;
    uint8_t permit_grid_charge_rules_count = 0;
    uint8_t forbid_discharge_rules_count   = 0;
    uint8_t forbid_charge_rules_count      = 0;

    bool forbid_discharge_during_fast_charge = false;
    bool evaluation_must_update_soc = false;
    bool evaluation_must_check_rules = false;
    bool have_battery = false;
    bool network_connect_seen = false;

    int32_t soc_cache_avg  = std::numeric_limits<decltype(soc_cache_avg)>::min();
    int32_t price_cache    = std::numeric_limits<decltype(soc_cache_avg)>::min();
    int32_t forecast_cache = std::numeric_limits<decltype(soc_cache_avg)>::min();
    uint8_t soc_cache[METERS_SLOTS];
    bool fast_charger_in_c_cache = false;

    TristateBool charge_permitted_by_rules = TristateBool::Undefined;
    TristateBool charge_permitted          = TristateBool::Undefined;

    TristateBool discharge_forbidden_by_rules = TristateBool::Undefined;
    TristateBool discharge_forbidden          = TristateBool::Undefined;

    TristateBool charge_forbidden_by_rules = TristateBool::Undefined;
    TristateBool charge_forbidden          = TristateBool::Undefined;
};
