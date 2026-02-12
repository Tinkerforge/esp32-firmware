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
#include "rule_action.enum.h"
#include "rule_condition.enum.h"
#include "schedule_rule_condition.enum.h"
#include "../batteries/battery_mode.enum.h"

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

    static constexpr int32_t UNAVAILABLE_CONDITION_CACHE       = std::numeric_limits<int32_t>::min();
    static constexpr int32_t UNAVAILABLE_SOC_CACHE             = std::numeric_limits<uint8_t>::max();
    static constexpr uint8_t UNAVAILABLE_TARIFF_SCHEDULE_CACHE = std::numeric_limits<uint8_t>::max();

private:
    struct control_rule {
        uint32_t      time_start_s; // in seconds since midnight
        uint32_t      time_end_s;   // in seconds since midnight
        int32_t       price_th;     // in ct/1000
        int32_t       forecast_th;  // in Wh
        uint8_t       soc_th;       // in percent
        RuleCondition soc_cond;
        RuleCondition price_cond;
        RuleCondition forecast_cond;
        ScheduleRuleCondition schedule_cond;
        RuleCondition time_cond;
        RuleCondition fast_chg_cond;
        RuleAction    action;
        uint8_t       index;
    };

    void preprocess_rules(const Config *rules_config, control_rule *rules);

    void update_avg_soc();
    void update_tariff_schedule();
    void evaluate_tariff_schedule();
    void update_solar_forecast(int localtime_hour_now, const Config *state);
    void schedule_evaluation();
    RuleAction evaluate_rules(const control_rule *rules, size_t rules_count, const char *rules_type_name, uint32_t time_since_midnight_s, uint8_t *active_rule_out);
    void evaluate_all_rules();
    void set_mode(BatteryMode new_mode);
    void fast_charge_update();

    ConfigRoot config;
    ConfigRoot rules_charge;
    ConfigRoot rules_discharge;
    ConfigRoot state;

    Config rule_prototype;

    size_t trace_buffer_idx;

    struct battery_control_data {
        uint64_t evaluation_task_id = 0;

        const control_rule *charge_rules    = nullptr;
        const control_rule *discharge_rules = nullptr;
        uint8_t charge_rules_count    = 0;
        uint8_t discharge_rules_count = 0;

        bool evaluation_must_update_soc = false;
        bool evaluation_must_check_rules = false;
        bool network_connect_seen = false;

        bool have_soc_rule             = false;
        bool have_price_rule           = false;
        bool have_solar_forecast_rule  = false;
        bool have_tariff_schedule_rule = false;
        bool have_time_rule            = false;
        bool have_fast_chg_rule        = false;

        int32_t soc_cache_avg  = UNAVAILABLE_CONDITION_CACHE;
        int32_t price_cache    = UNAVAILABLE_CONDITION_CACHE;
        int32_t forecast_cache = UNAVAILABLE_CONDITION_CACHE;
        uint8_t soc_cache[OPTIONS_METERS_MAX_SLOTS()];
        TristateBool fast_charger_in_c_cache = TristateBool::Undefined;

        int32_t tariff_schedule_start_min = 0;
        uint8_t tariff_schedule[24 * 4] = {0};
        uint8_t tariff_schedule_cache = UNAVAILABLE_TARIFF_SCHEDULE_CACHE;

        uint8_t max_used_batteries = 0;

        BatteryMode last_mode = BatteryMode::None;
    };

    battery_control_data *data = nullptr;
};
