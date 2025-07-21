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

#define TRACE_LOG_PREFIX nullptr

#include "battery_control.h"
#include "module_dependencies.h"

#include "event_log_prefix.h"
#include "tools.h"
#include "tools/malloc.h"

#include "gcc_warnings.h"

static constexpr const uint16_t MAX_RULES_PER_TYPE = 32;
static_assert(MAX_RULES_PER_TYPE < 256, "MAX_RULES_PER_TYPE must be below 256 to fit into an uint8_t");

void BatteryControl::pre_setup()
{
    trace_buffer_idx = logger.alloc_trace_buffer("battery_control", 8192);

    config = Config::Object({
        {"forbid_discharge_during_fast_charge", Config::Bool(false)},
        {"cheap_tariff_quarters",     Config::Uint8(0, sizeof(tariff_schedule))},
        {"expensive_tariff_quarters", Config::Uint8(0, sizeof(tariff_schedule))},
    });

    rule_prototype = Config::Object({
        {"desc",          Config::Str   ("", 0, 32)},
        {"soc_cond",      Config::Enum  (RuleCondition::Ignore)},
        {"soc_th",        Config::Uint8 (0, 100)}, // in percent (0 to 100 %)
        {"price_cond",    Config::Enum  (RuleCondition::Ignore)},
        {"price_th",      Config::Int16 (0)}, // in ct/10   (-32 to 32 EUR)
        {"forecast_cond", Config::Enum  (RuleCondition::Ignore)},
        {"forecast_th",   Config::Uint16(0)}, // in kWh     (0 to 65 MWh)
        {"schedule_cond", Config::Enum  (ScheduleRuleCondition::Ignore)},
    });

    rules_permit_grid_charge = Config::Array({}, &rule_prototype, 0, MAX_RULES_PER_TYPE, Config::type_id<Config::ConfObject>());
    rules_forbid_discharge   = Config::Array({}, &rule_prototype, 0, MAX_RULES_PER_TYPE, Config::type_id<Config::ConfObject>());
    rules_forbid_charge      = Config::Array({}, &rule_prototype, 0, MAX_RULES_PER_TYPE, Config::type_id<Config::ConfObject>());

    low_level_config = Config::Object({
        {"rewrite_period", Config::Uint(60, 5, 99999)}, // in seconds
    });

    state = Config::Object({
        {"grid_charge_permitted", Config::Bool(false)},
        {"discharge_forbidden",   Config::Bool(false)},
        {"charge_forbidden",      Config::Bool(false)},
    });
}

void BatteryControl::setup()
{
    api.restorePersistentConfig("battery_control/config",                   &config);
    api.restorePersistentConfig("battery_control/rules_permit_grid_charge", &rules_permit_grid_charge);
    api.restorePersistentConfig("battery_control/rules_forbid_discharge",   &rules_forbid_discharge);
    api.restorePersistentConfig("battery_control/rules_forbid_charge",      &rules_forbid_charge);
    api.restorePersistentConfig("battery_control/low_level_config",         &low_level_config);

    initialized = true;

    for (size_t i = 0; i < OPTIONS_BATTERIES_MAX_SLOTS(); i++) {
        if (batteries.get_battery_class(i) != BatteryClassID::None) {
            have_battery = true;
            break;
        }
    }

    if (!have_battery) {
        return;
    }

    for (size_t i = 0; i < ARRAY_SIZE(soc_cache); i++) {
        soc_cache[i] = std::numeric_limits<uint8_t>::max();
    }

    forbid_discharge_during_fast_charge = config.get("forbid_discharge_during_fast_charge")->asBool() && charge_manager.get_charger_count() > 0;
    rewrite_period = seconds_t{this->low_level_config.get("rewrite_period")->asUint()};

    const size_t permit_grid_charge_rules_cnt = rules_permit_grid_charge.count();
    const size_t forbid_discharge_rules_cnt   = rules_forbid_discharge.count();
    const size_t forbid_charge_rules_cnt      = rules_forbid_charge.count();
    const size_t total_rules_cnt = permit_grid_charge_rules_cnt + forbid_discharge_rules_cnt + forbid_charge_rules_cnt;

    if (total_rules_cnt > 0) {
        control_rule *rules = static_cast<decltype(rules)>(malloc_psram_or_dram(total_rules_cnt * sizeof(*rules))); // Not large but rarely accessed, so it can live in PSRAM.
        if (!rules) {
            logger.printfln("Failed to allocate memory for rules");
            return;
        }

        control_rule *pgc_rules = rules;
        if (permit_grid_charge_rules_cnt > 0) {
            permit_grid_charge_rules       = pgc_rules;
            permit_grid_charge_rules_count = static_cast<uint8_t>(permit_grid_charge_rules_cnt);

            preprocess_rules(&rules_permit_grid_charge, pgc_rules, permit_grid_charge_rules_cnt);
        }

        control_rule *fd_rules = pgc_rules + permit_grid_charge_rules_count;
        if (forbid_discharge_rules_cnt > 0) {
            forbid_discharge_rules       = fd_rules;
            forbid_discharge_rules_count = static_cast<uint8_t>(forbid_discharge_rules_cnt);

            preprocess_rules(&rules_forbid_discharge, fd_rules, forbid_discharge_rules_cnt);
        }

        control_rule *fc_rules = fd_rules + forbid_discharge_rules_count;
        if (forbid_charge_rules_cnt > 0) {
            forbid_charge_rules       = fc_rules;
            forbid_charge_rules_count = static_cast<uint8_t>(forbid_charge_rules_cnt);

            preprocess_rules(&rules_forbid_charge, fc_rules, forbid_charge_rules_cnt);
        }
    }
}

void BatteryControl::register_urls()
{
    api.addPersistentConfig("battery_control/config",                   &config);
    api.addPersistentConfig("battery_control/rules_permit_grid_charge", &rules_permit_grid_charge);
    api.addPersistentConfig("battery_control/rules_forbid_discharge",   &rules_forbid_discharge);
    api.addPersistentConfig("battery_control/rules_forbid_charge",      &rules_forbid_charge);
    api.addPersistentConfig("battery_control/low_level_config",         &low_level_config);
    api.addState("battery_control/state", &state);
}

void BatteryControl::register_events()
{
    const size_t total_rules_cnt = static_cast<size_t>(permit_grid_charge_rules_count) + static_cast<size_t>(forbid_discharge_rules_count) + static_cast<size_t>(forbid_charge_rules_count);

    if (forbid_discharge_during_fast_charge || total_rules_cnt > 0) {
#if MODULE_NETWORK_AVAILABLE()
        network.on_network_connected([this](const Config *connected) {
            if (!connected->asBool()) {
                return EventResult::OK;
            }

            task_scheduler.scheduleWithFixedDelay([this]() {
                this->periodic_update();
            }, 5_s, 1_s);

            return EventResult::Deregister;
        });
#else
        task_scheduler.scheduleWithFixedDelay([this]() {
            this->periodic_update();
        }, 10_s, 1_s);
#endif
    }

    if (total_rules_cnt == 0) {
        return;
    }

    for (uint32_t slot = 0; slot < OPTIONS_METERS_MAX_SLOTS(); slot++) {
        if (meters.get_meter_location(slot) != MeterLocation::Battery) {
            continue;
        }
        logger.tracefln(this->trace_buffer_idx, "Meter %lu is a battery", slot);

        // Register on the ConfigRoot.
        event.registerEvent(meters.get_path(slot, Meters::PathType::ValueIDs), {}, [this, slot](const Config *cfg) {
            const size_t count = cfg->count();

            if (count == 0) {
                logger.tracefln(this->trace_buffer_idx, "Ignoring blank value IDs update from meter in slot %lu.", slot);
                return EventResult::OK;
            }

            const MeterValueID soc_vid = MeterValueID::StateOfCharge;
            uint32_t soc_index = std::numeric_limits<decltype(soc_index)>::max();
            meters.fill_index_cache(slot, 1, &soc_vid, &soc_index);

            if (soc_index == std::numeric_limits<decltype(soc_index)>::max()) {
                logger.printfln("Battery meter in slot %lu doesn't provide SOC.", slot);
            } else {
                event.registerEvent(meters.get_path(slot, Meters::PathType::Values), {static_cast<size_t>(soc_index)}, [this, slot](const Config *config_soc) {
                    const float soc = config_soc->asFloat();

                    if (!isnan(soc)) {
                        this->soc_cache[slot] = static_cast<uint8_t>(soc);
                        logger.tracefln(this->trace_buffer_idx, "meter %lu SOC=%hhu%%", slot, this->soc_cache[slot]);

                        this->evaluation_must_update_soc  = true;
                        this->evaluation_must_check_rules = true;
                        this->schedule_evaluation();
                    } else {
                        logger.tracefln(this->trace_buffer_idx, "Ignoring uninitialized SOC from battery meter %lu", slot);
                    }

                    return EventResult::OK;
                });
            }

            return EventResult::Deregister;
        });
    }

#if MODULE_DAY_AHEAD_PRICES_AVAILABLE()
    event.registerEvent("day_ahead_prices/state", {}, [this](const Config *cfg) {
        const uint8_t event_api_backend_flag = event.get_api_backend_flag();

        const Config *current_price_cfg = static_cast<const Config *>(cfg->get("current_price"));

        if (current_price_cfg->was_updated(event_api_backend_flag)) {
            const int32_t price = current_price_cfg->asInt();

            logger.tracefln(this->trace_buffer_idx, "current_price=%li", price);

            if (price == std::numeric_limits<decltype(price)>::max()) {
                this->price_cache = std::numeric_limits<decltype(this->price_cache)>::min();
            } else {
                this->price_cache = price;
            }

            this->evaluation_must_check_rules = true;
            this->schedule_evaluation();
        }

        const Config *last_sync_cfg = static_cast<const Config *>(cfg->get("last_sync"));

        if (last_sync_cfg->was_updated(event_api_backend_flag) && last_sync_cfg->asUint() != 0) {
            logger.tracefln(this->trace_buffer_idx, "DAP last_sync at %lu", last_sync_cfg->asUint());
            this->update_tariff_schedule();
        }

        return EventResult::OK;
    });

    if (config.get("cheap_tariff_quarters")->asUint() != 0 || config.get("expensive_tariff_quarters")->asUint() != 0) {
        // Call update_tariff_schedule at 20:00.
        // The wall clock task has to run hourly and check the local time manually.
        // Registering the wall clock task with a 24h interval and a time-zone-dependent delay would schedule the task at midnight with a large delay.
        // If the device was restarted between midnight and 20:00, the task would not be scheduled until the next day.
        // Checking the local time here also means that DST doesn't matter.
        task_scheduler.scheduleWallClock([this]() {
            time_t time_utc = time(nullptr);
            struct tm tm_local;
            localtime_r(&time_utc, &tm_local);

            if (tm_local.tm_hour == 20) {
                this->update_tariff_schedule();
            }
        }, 1_h, 0_ms, false);

        task_scheduler.scheduleWallClock([this]() {
            this->evaluate_tariff_schedule();
        }, 15_min, 100_ms, false); // Slightly delayed to give the previous task a chance to update the schedule first.
    }
#endif

#if MODULE_SOLAR_FORECAST_AVAILABLE()
    event.registerEvent("solar_forecast/state", {"wh_tomorrow"}, [this](const Config *cfg) {
        const int32_t wh_tomorrow = cfg->asInt();

        if (wh_tomorrow < 0) {
            logger.tracefln(this->trace_buffer_idx, "Ignoring uninitialized forecast: %li", wh_tomorrow);
            return EventResult::OK;
        }

        logger.tracefln(this->trace_buffer_idx, "wh_tomorrow=%li", wh_tomorrow);
        this->forecast_cache = wh_tomorrow;
        this->evaluation_must_check_rules = true;
        this->schedule_evaluation();

        return EventResult::OK;
    });
#endif
}

void BatteryControl::preprocess_rules(const Config *rules_config, control_rule *rules, size_t rules_count)
{
    for (size_t i = 0; i < rules_count; i++) {
        const auto rule_config = rules_config->get(i);
        control_rule *rule = rules + i;

        rule->soc_cond      =                       rule_config->get("soc_cond"     )->asEnum<RuleCondition>();
        rule->soc_th        = static_cast<uint8_t >(rule_config->get("soc_th"       )->asUint());
        rule->price_cond    =                       rule_config->get("price_cond"   )->asEnum<RuleCondition>();
        rule->price_th      =                       rule_config->get("price_th"     )->asInt()  *  100;         // ct/10 -> ct/1000
        rule->forecast_cond =                       rule_config->get("forecast_cond")->asEnum<RuleCondition>();
        rule->forecast_th   = static_cast<int32_t >(rule_config->get("forecast_th"  )->asUint() * 1000);        // kWh -> Wh
        rule->schedule_cond =                       rule_config->get("schedule_cond")->asEnum<ScheduleRuleCondition>();
    }
}

void BatteryControl::update_avg_soc()
{
    uint32_t soc_sum = 0;
    uint32_t soc_count = 0;

    for (size_t i = 0; i < ARRAY_SIZE(soc_cache); i++) {
        const uint8_t soc = soc_cache[i];
        if (soc == std::numeric_limits<decltype(soc)>::max()) {
            continue;
        }
        soc_sum += soc;
        soc_count++;
    }

    if (soc_count == 0) {
        logger.tracefln(this->trace_buffer_idx, "soc_avg has no data: No battery meters have SOC data.");
        return;
    }

    soc_cache_avg = static_cast<decltype(soc_cache_avg)>(soc_sum / soc_count);

    logger.tracefln(trace_buffer_idx, "soc_avg=%li", soc_cache_avg);
}

void BatteryControl::update_tariff_schedule()
{
    const uint8_t cheap_tariff_quarters     = static_cast<uint8_t>(config.get("cheap_tariff_quarters"    )->asUint());
    const uint8_t expensive_tariff_quarters = static_cast<uint8_t>(config.get("expensive_tariff_quarters")->asUint());

    if (cheap_tariff_quarters == 0 && expensive_tariff_quarters == 0) {
        return;
    }

    time_t now_s = time(nullptr);
    time_t schedule_start_s = now_s;

    do {
        struct tm start_date;
        localtime_r(&schedule_start_s, &start_date);

        // Set local time to 20h today
        start_date.tm_hour  = 20;
        start_date.tm_min   =  0;
        start_date.tm_sec   =  0;
        start_date.tm_isdst = -1; // let mktime figure out if DST is in effect

        schedule_start_s = mktime(&start_date);

        if (schedule_start_s <= now_s) {
            logger.tracefln(this->trace_buffer_idx, "Schedule begins %llis in the past | %i-%02i-%02i %02i:%02i:%02i", now_s - schedule_start_s, start_date.tm_year+1900, start_date.tm_mon+1, start_date.tm_mday, start_date.tm_hour, start_date.tm_min, start_date.tm_sec);
            break;
        }

        schedule_start_s -= 24 * 60 * 60; // yesterday
        // Loop to readjust to 20h in case DST changed.
    } while (true);

    tariff_schedule_start_min = static_cast<int32_t>(schedule_start_s / 60);

    bool cheap_hours    [sizeof(tariff_schedule)];
    bool expensive_hours[sizeof(tariff_schedule)];
    bool any_data_available = false;

    if (day_ahead_prices.get_cheap_and_expensive_15m(tariff_schedule_start_min, sizeof(cheap_hours),     cheap_tariff_quarters,     cheap_hours, nullptr)) {
        any_data_available = true;
    } else {
        memset(cheap_hours, false, sizeof(cheap_hours));
    }

    if (day_ahead_prices.get_cheap_and_expensive_15m(tariff_schedule_start_min, sizeof(expensive_hours), expensive_tariff_quarters, nullptr,     expensive_hours)) {
        any_data_available = true;
    } else {
        memset(expensive_hours, false, sizeof(expensive_hours));
    }

    if (any_data_available) {
        for (size_t i = 0; i < sizeof(tariff_schedule); i++) {
            tariff_schedule[i] = cheap_hours[i]     << BC_SCHEDULE_CHEAP_POS
                               | expensive_hours[i] << BC_SCHEDULE_EXPENSIVE_POS;
        }
    } else {
        memset(tariff_schedule, 0, sizeof(tariff_schedule));
    }

    char str[sizeof(tariff_schedule) + 1];
    for (size_t i = 0; i < sizeof(tariff_schedule); i++) {
        str[i] = '0' + tariff_schedule[i];
    }
    str[sizeof(str) - 1] = 0;
    logger.tracefln(this->trace_buffer_idx, "Schedule: %s", str);

    evaluate_tariff_schedule();
}

void BatteryControl::evaluate_tariff_schedule()
{
    const int32_t now_min = static_cast<int32_t>(time(nullptr) / 60);

    if (now_min < tariff_schedule_start_min) {
        logger.printfln("Tariff schedule starts in the future: %li < %li", now_min, tariff_schedule_start_min);
        tariff_schedule_cache = 0;
        return;
    }

    const uint32_t quarter_index = static_cast<uint32_t>(now_min - tariff_schedule_start_min) / 15;

    if (quarter_index >= sizeof(tariff_schedule)) {
        logger.printfln("Quarter index beyond tariff schedule: %lu >= %u", quarter_index, sizeof(tariff_schedule));
        tariff_schedule_cache = 0;
        return;
    }

    const uint8_t charge_permitted_schedule_cache_update = tariff_schedule[quarter_index];

    if (tariff_schedule_cache != charge_permitted_schedule_cache_update) {
        tariff_schedule_cache  = charge_permitted_schedule_cache_update;
        logger.tracefln(this->trace_buffer_idx, "tariff_schedule_cache=%hhu", tariff_schedule_cache);
        evaluation_must_check_rules = true;
        schedule_evaluation();
    }
}

void BatteryControl::schedule_evaluation()
{
    if (evaluation_task_id != 0) {
        return;
    }

    evaluation_task_id = task_scheduler.scheduleOnce([this]() {
        logger.tracefln(this->trace_buffer_idx, "Evaluating");

        this->evaluation_task_id = 0;

        if (this->evaluation_must_check_rules) {
            this->evaluation_must_check_rules = false;

            if (this->evaluation_must_update_soc) {
                this->evaluation_must_update_soc = false;

                this->update_avg_soc();
            }

            this->evaluate_all_rules();
        }

        this->evaluate_summary();
    });
}

static bool rule_condition_failed(const RuleCondition cond, const int32_t th, const int32_t value)
{
    if (cond == RuleCondition::Ignore) {
        return false;
    }

    if (value == std::numeric_limits<decltype(value)>::min()) {
        return true; // Input value is uninitialized, rule cannot be true.
    }

    if (cond == RuleCondition::Below) {
        return !(value < th); // intentionally negated
    }

    // RuleCondition::Above
    return !(value > th); // intentionally negated
}

static bool schedule_rule_condition_failed(const ScheduleRuleCondition cond, const uint8_t value)
{
    if (cond == ScheduleRuleCondition::Ignore) {
        return false;
    }

    if (cond == ScheduleRuleCondition::Cheap) {
        return !(value & BC_SCHEDULE_CHEAP_MASK); // intentionally negated
    }

    if (cond == ScheduleRuleCondition::NotCheap) {
        return   value & BC_SCHEDULE_CHEAP_MASK; // intentionally inverted
    }

    if (cond == ScheduleRuleCondition::Expensive) {
        return !(value & BC_SCHEDULE_EXPENSIVE_MASK); // intentionally negated
    }

    if (cond == ScheduleRuleCondition::NotExpensive) {
        return   value & BC_SCHEDULE_EXPENSIVE_MASK; // intentionally inverted
    }

    // ScheduleRuleCondition::Moderate
    return value != 0; // 0 = neither cheap nor expensive
}

TristateBool BatteryControl::evaluate_rules(const control_rule *rules, size_t rules_count, const char *rules_type_name)
{
    for (size_t i = 0; i < rules_count; i++) {
        const control_rule *rule = rules + i;

        if (rule_condition_failed(rule->soc_cond,      rule->soc_th,      soc_cache_avg )) continue;
        if (rule_condition_failed(rule->price_cond,    rule->price_th,    price_cache   )) continue;
        if (rule_condition_failed(rule->forecast_cond, rule->forecast_th, forecast_cache)) continue;
        if (schedule_rule_condition_failed(rule->schedule_cond, tariff_schedule_cache)) continue;

        // Complete rule matches.
        logger.tracefln(this->trace_buffer_idx, "%s rule %zu matches", rules_type_name, i);
        return TristateBool::True;
    }

    return TristateBool::False;
}

void BatteryControl::evaluate_all_rules()
{
    charge_permitted_by_rules    = evaluate_rules(permit_grid_charge_rules, permit_grid_charge_rules_count, "permit_grid_charge");
    discharge_forbidden_by_rules = evaluate_rules(forbid_discharge_rules,   forbid_discharge_rules_count,   "forbid_discharge"  );
    charge_forbidden_by_rules    = evaluate_rules(forbid_charge_rules,      forbid_charge_rules_count,      "forbid_charge"     );

    logger.tracefln(this->trace_buffer_idx, "charge_permitted_by_rules=%u discharge_forbidden_by_rules=%u charge_forbidden_by_rules=%u", static_cast<unsigned>(charge_permitted_by_rules), static_cast<unsigned>(discharge_forbidden_by_rules), static_cast<unsigned>(charge_forbidden_by_rules));
}

void BatteryControl::evaluate_summary()
{
    if (charge_permitted != charge_permitted_by_rules) {
        charge_permitted  = charge_permitted_by_rules;
        update_charge_permitted(true);
    }

    TristateBool discharge_forbidden_update;

    if (discharge_forbidden_by_rules == TristateBool::True) {
        discharge_forbidden_update = TristateBool::True;
    } else if (forbid_discharge_during_fast_charge && fast_charger_in_c_cache) {
        discharge_forbidden_update = TristateBool::True;
    } else {
        discharge_forbidden_update = TristateBool::False;
    }

    if (discharge_forbidden != discharge_forbidden_update) {
        discharge_forbidden  = discharge_forbidden_update;
        update_discharge_forbidden(true);
    }

    if (charge_forbidden != charge_forbidden_by_rules) {
        charge_forbidden  = charge_forbidden_by_rules;
        update_charge_forbidden(true);
    }
}

void BatteryControl::periodic_update()
{
    bool skip_discharge_forbidden = false;

    if (forbid_discharge_during_fast_charge) {
        const bool fast_charger_in_c_cm = charge_manager.fast_charger_in_c;

        if (fast_charger_in_c_cm != fast_charger_in_c_cache) {
            logger.tracefln(trace_buffer_idx, "fast_charger_in_c=%i", fast_charger_in_c_cm);
            fast_charger_in_c_cache = fast_charger_in_c_cm;
            skip_discharge_forbidden = true;
            schedule_evaluation();
        }
    }

    if (deadline_elapsed(next_permit_grid_charge_update)) {
        update_charge_permitted(false);
    }

    if (!skip_discharge_forbidden && deadline_elapsed(next_discharge_forbidden_update)) {
        update_discharge_forbidden(false);
    }

    if (deadline_elapsed(next_charge_forbidden_update)) {
        update_charge_forbidden(false);
    }
}

struct battery_control_action_info {
    IBattery::Action influence_start_action;
    IBattery::Action influence_end_action;
    const char *influence_start_msg;
    const char *influence_end_msg;
    const char *state_name;
};

static constexpr uint32_t action_info_num_permit_grid_charge = 0;
static constexpr uint32_t action_info_num_forbid_discharge   = 1;
static constexpr uint32_t action_info_num_forbid_charge      = 2;

const battery_control_action_info action_infos[] = {
    {
        IBattery::Action::PermitGridCharge,
        IBattery::Action::RevokeGridChargeOverride,
        "Permit grid charge",
        "Revoke grid charge override",
        "grid_charge_permitted",
    },
    {
        IBattery::Action::ForbidDischarge,
        IBattery::Action::RevokeDischargeOverride,
        "Forbid discharge",
        "Revoke discharge override",
        "discharge_forbidden",
    },
    {
        IBattery::Action::ForbidCharge,
        IBattery::Action::RevokeChargeOverride,
        "Forbid charge",
        "Revoke charge override",
        "charge_forbidden",
    },
};

void BatteryControl::update_batteries_and_state(bool influence_active, bool changed, const battery_control_action_info *action, micros_t *next_update)
{
    if (influence_active) {
        if (changed) {
            logger.tracefln(trace_buffer_idx, "%s", action->influence_start_msg);
            logger.printfln("%s", action->influence_start_msg);
        }
        batteries.start_action_all(action->influence_start_action);
    } else {
        if (changed) {
            logger.tracefln(trace_buffer_idx, "%s", action->influence_end_msg);
            logger.printfln("%s", action->influence_end_msg);
        }
        batteries.start_action_all(action->influence_end_action);
    }

    this->state.get(action->state_name)->updateBool(influence_active);
    *next_update = now_us() + rewrite_period;
}

void BatteryControl::update_charge_permitted(bool changed)
{
    const bool influence_active = charge_permitted == TristateBool::True;
    update_batteries_and_state(influence_active, changed, action_infos + action_info_num_permit_grid_charge, &next_permit_grid_charge_update);
}

void BatteryControl::update_discharge_forbidden(bool changed)
{
    const bool influence_active = discharge_forbidden == TristateBool::True;
    update_batteries_and_state(influence_active, changed, action_infos + action_info_num_forbid_discharge, &next_discharge_forbidden_update);
}

void BatteryControl::update_charge_forbidden(bool changed)
{
    const bool influence_active = charge_forbidden == TristateBool::True;
    update_batteries_and_state(influence_active, changed, action_infos + action_info_num_forbid_charge, &next_charge_forbidden_update);
}
