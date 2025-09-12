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
#include "options.h"
#include "TFTools/Micros.h"
#include "tools/tristate_bool.h"
#include "rule_condition.enum.h"
#include "schedule_rule_condition.enum.h"

#define BC_SCHEDULE_CHEAP_POS      (0)
#define BC_SCHEDULE_CHEAP_MASK     (1 << BC_SCHEDULE_CHEAP_POS)
#define BC_SCHEDULE_EXPENSIVE_POS  (1)
#define BC_SCHEDULE_EXPENSIVE_MASK (1 << BC_SCHEDULE_EXPENSIVE_POS)

class BatteryControl final : public IModule
{
public:
    BatteryControl() {}

    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

private:
    struct control_rule {
        int32_t       price_th;    // in ct/1000
        int32_t       forecast_th; // in Wh
        uint8_t       soc_th;      // in percent
        RuleCondition soc_cond;
        RuleCondition price_cond;
        RuleCondition forecast_cond;
        ScheduleRuleCondition schedule_cond;
    };

    struct battery_repeat_data {
        micros_t repeat_interval[6];
        micros_t next_action_update[6];
    };

    struct action_influence_data {
        TristateBool activated_by_rules = TristateBool::Undefined;
        TristateBool activated          = TristateBool::Undefined;
    };

    enum class ActionPair {
        PermitGridCharge = 0,
        ForbidDischarge  = 1,
        ForbidCharge     = 2,
    };

    void preprocess_rules(const Config *rules_config, control_rule *rules, size_t rules_count);

    void update_avg_soc();
    void update_tariff_schedule();
    void evaluate_tariff_schedule();
    void update_solar_forecast(int localtime_hour_now, const Config *state);
    void schedule_evaluation();
    TristateBool evaluate_rules(const control_rule *rules, size_t rules_count, const char *rules_type_name);
    void evaluate_all_rules();
    void evaluate_summary();
    void evaluate_action_pair(ActionPair action_pair);
    void periodic_update();

    void update_batteries_and_state(ActionPair action_pair, bool changed, uint32_t battery_slot = std::numeric_limits<uint32_t>::max());

    ConfigRoot config;
    ConfigRoot rules_permit_grid_charge;
    ConfigRoot rules_forbid_discharge;
    ConfigRoot rules_forbid_charge;
    ConfigRoot state;

    Config rule_prototype;

    size_t trace_buffer_idx;

    struct battery_control_data {
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
        bool network_connect_seen = false;

        int32_t soc_cache_avg  = std::numeric_limits<decltype(soc_cache_avg )>::min();
        int32_t price_cache    = std::numeric_limits<decltype(price_cache   )>::min();
        int32_t forecast_cache = std::numeric_limits<decltype(forecast_cache)>::min();
        uint8_t soc_cache[OPTIONS_METERS_MAX_SLOTS()];
        bool fast_charger_in_c_cache = false;

        int32_t tariff_schedule_start_min = 0;
        uint8_t tariff_schedule[24 * 4] = {0};
        uint8_t tariff_schedule_cache = 0;

        battery_repeat_data *battery_repeats = nullptr;
        uint8_t max_used_batteries           = 0;
        bool must_repeat                     = false;

        action_influence_data action_influence_active[3];
    };

    battery_control_data *data = nullptr;
};
