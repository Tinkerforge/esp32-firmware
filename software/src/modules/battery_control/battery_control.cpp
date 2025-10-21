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

static_assert(OPTIONS_BATTERY_CONTROL_MAX_RULES_PER_TYPE() < 256, "OPTIONS_BATTERY_CONTROL_MAX_RULES_PER_TYPE must be below 256 to fit into an uint8_t");

#if MODULE_DAY_AHEAD_PRICES_AVAILABLE() || MODULE_SOLAR_FORECAST_AVAILABLE()
static int get_localtime_hour()
{
    const time_t time_utc = time(nullptr);
    struct tm tm_local;
    localtime_r(&time_utc, &tm_local);

    return tm_local.tm_hour;
}
#endif

void BatteryControl::pre_setup()
{
    trace_buffer_idx = logger.alloc_trace_buffer("battery_control", 32768);

    config = Config::Object({
        {"cheap_tariff_quarters",     Config::Uint8(0, sizeof(battery_control_data::tariff_schedule))},
        {"expensive_tariff_quarters", Config::Uint8(0, sizeof(battery_control_data::tariff_schedule))},
    });

    rule_prototype = Config::Object({
        {"enabled",       Config::Bool  (true)},
        {"desc",          Config::Str   ("", 0, 32)},
        {"soc_cond",      Config::Enum  (RuleCondition::Ignore)},
        {"soc_th",        Config::Uint8 (0, 100)}, // in percent (0 to 100 %)
        {"price_cond",    Config::Enum  (RuleCondition::Ignore)},
        {"price_th",      Config::Int16 (0)}, // in ct/10   (-32 to 32 EUR)
        {"forecast_cond", Config::Enum  (RuleCondition::Ignore)},
        {"forecast_th",   Config::Uint16(0)}, // in kWh     (0 to 65 MWh)
        {"schedule_cond", Config::Enum  (ScheduleRuleCondition::Ignore)},
        {"time_cond",     Config::Enum  (RuleCondition::Ignore)},
        {"time_start",    Config::Uint16(0)}, // in minutes since midnight
        {"time_end",      Config::Uint16(0)}, // in minutes since midnight
        {"fast_chg_cond", Config::Enum  (RuleCondition::Ignore)},
    });

    rules_permit_grid_charge = Config::Array({}, &rule_prototype, 0, OPTIONS_BATTERY_CONTROL_MAX_RULES_PER_TYPE(), Config::type_id<Config::ConfObject>());
    rules_forbid_discharge   = Config::Array({}, &rule_prototype, 0, OPTIONS_BATTERY_CONTROL_MAX_RULES_PER_TYPE(), Config::type_id<Config::ConfObject>());
    rules_forbid_charge      = Config::Array({}, &rule_prototype, 0, OPTIONS_BATTERY_CONTROL_MAX_RULES_PER_TYPE(), Config::type_id<Config::ConfObject>());

    state = Config::Object({
        {"grid_charge_permitted", Config::Bool(false)},
        {"discharge_forbidden",   Config::Bool(false)},
        {"charge_forbidden",      Config::Bool(false)},
    });
}

static size_t count_enabled_rules(const Config &cfg)
{
    const size_t total_count = cfg.count();
    size_t enabled_count = 0;

    for (size_t i = 0; i < total_count; i++) {
        if (cfg.get(i)->get("enabled")->asBool()) {
            enabled_count++;
        }
    }

    return enabled_count;
}

void BatteryControl::setup()
{
    api.restorePersistentConfig("battery_control/config",                   &config);
    api.restorePersistentConfig("battery_control/rules_permit_grid_charge", &rules_permit_grid_charge);
    api.restorePersistentConfig("battery_control/rules_forbid_discharge",   &rules_forbid_discharge);
    api.restorePersistentConfig("battery_control/rules_forbid_charge",      &rules_forbid_charge);

    initialized = true;

    if (!batteries.get_enabled()) {
        return;
    }

    uint8_t max_used_batteries = 0;

    for (uint8_t i = OPTIONS_BATTERIES_MAX_SLOTS(); i > 0; i--) {
        if (batteries.get_battery_class(i - 1) != BatteryClassID::None) {
            max_used_batteries = i;
            break;
        }
    }

    if (max_used_batteries == 0) {
        return;
    }

    void *ptr = malloc(sizeof(*data));

    if (ptr == nullptr) {
        logger.printfln("No memory for runtime data. Module disabled.");
        return;
    }

    data = new(ptr) typeof(*data); // Use placement new to initialize member variables.

    data->max_used_batteries = max_used_batteries;
    data->battery_repeats = static_cast<battery_repeat_data *>(malloc(max_used_batteries * sizeof(battery_repeat_data)));

    if (data->battery_repeats == nullptr) {
        logger.printfln("No memory for battery repeats. Module disabled.");
        return;
    }

    for (size_t battery_i = 0; battery_i < max_used_batteries; battery_i++) {
        const IBattery *battery = batteries.get_battery(battery_i);
        uint16_t intervals_s[6];

        battery->get_repeat_intervals(intervals_s);

        battery_repeat_data *repeat_data = data->battery_repeats + battery_i;

        for (size_t action_i = 0; action_i < ARRAY_SIZE(intervals_s); action_i++) {
            const uint16_t interval_s = intervals_s[action_i];

            if (interval_s != 0) {
                data->must_repeat = true;
            }

            repeat_data->repeat_interval[action_i]    = seconds_t{interval_s};
            repeat_data->next_action_update[action_i] = 0_us;
        }
    }

    for (size_t i = 0; i < ARRAY_SIZE(data->soc_cache); i++) {
        data->soc_cache[i] = std::numeric_limits<uint8_t>::max();
    }

    const size_t permit_grid_charge_rules_cnt = count_enabled_rules(rules_permit_grid_charge);
    const size_t forbid_discharge_rules_cnt   = count_enabled_rules(rules_forbid_discharge);
    const size_t forbid_charge_rules_cnt      = count_enabled_rules(rules_forbid_charge);
    const size_t total_rules_cnt = permit_grid_charge_rules_cnt + forbid_discharge_rules_cnt + forbid_charge_rules_cnt;

    if (total_rules_cnt > 0) {
        control_rule *rules = static_cast<decltype(rules)>(malloc_psram_or_dram(total_rules_cnt * sizeof(*rules))); // Not large but rarely accessed, so it can live in PSRAM.
        if (!rules) {
            logger.printfln("Failed to allocate memory for rules");
            return;
        }

        control_rule *pgc_rules = rules;
        if (permit_grid_charge_rules_cnt > 0) {
            data->permit_grid_charge_rules       = pgc_rules;
            data->permit_grid_charge_rules_count = static_cast<uint8_t>(permit_grid_charge_rules_cnt);

            preprocess_rules(&rules_permit_grid_charge, pgc_rules);
        }

        control_rule *fd_rules = pgc_rules + data->permit_grid_charge_rules_count;
        if (forbid_discharge_rules_cnt > 0) {
            data->forbid_discharge_rules       = fd_rules;
            data->forbid_discharge_rules_count = static_cast<uint8_t>(forbid_discharge_rules_cnt);

            preprocess_rules(&rules_forbid_discharge, fd_rules);
        }

        control_rule *fc_rules = fd_rules + data->forbid_discharge_rules_count;
        if (forbid_charge_rules_cnt > 0) {
            data->forbid_charge_rules       = fc_rules;
            data->forbid_charge_rules_count = static_cast<uint8_t>(forbid_charge_rules_cnt);

            preprocess_rules(&rules_forbid_charge, fc_rules);
        }
    }

    {
        char buf[56];
        StringWriter sw(buf, sizeof(buf));

        if (data->have_soc_rule            ) sw.puts(" SoC");
        if (data->have_price_rule          ) sw.puts(" Price");
        if (data->have_solar_forecast_rule ) sw.puts(" SolarForecast");
        if (data->have_tariff_schedule_rule) sw.puts(" TariffSchedule");
        if (data->have_time_rule           ) sw.puts(" Time");
        if (data->have_fast_chg_rule       ) sw.puts(" FastChg");

        if (sw.getLength() == 0) {
            sw.puts(" [none]");
        }

        logger.tracefln(this->trace_buffer_idx, "Rules:%s", buf);
    }
}

void BatteryControl::register_urls()
{
    api.addPersistentConfig("battery_control/config",                   &config);
    api.addPersistentConfig("battery_control/rules_permit_grid_charge", &rules_permit_grid_charge);
    api.addPersistentConfig("battery_control/rules_forbid_discharge",   &rules_forbid_discharge);
    api.addPersistentConfig("battery_control/rules_forbid_charge",      &rules_forbid_charge);
    api.addState("battery_control/state", &state);
}

void BatteryControl::register_events()
{
    if (data == nullptr) {
        return;
    }

    const size_t total_rules_cnt = static_cast<size_t>(data->permit_grid_charge_rules_count) + static_cast<size_t>(data->forbid_discharge_rules_count) + static_cast<size_t>(data->forbid_charge_rules_count);

    if (total_rules_cnt == 0) {
        return;
    }

    if (data->have_fast_chg_rule || data->must_repeat) {
#if MODULE_NETWORK_AVAILABLE()
        network.on_network_connected([this](const Config *connected) {
            if (!connected->asBool()) {
                return EventResult::OK;
            }

            task_scheduler.scheduleUncancelable([this]() {
                this->periodic_update();
            }, 5_s, 1_s);

            return EventResult::Deregister;
        });
#else
        task_scheduler.scheduleUncancelable([this]() {
            this->periodic_update();
        }, 10_s, 1_s);
#endif
    }

    if (data->have_soc_rule) {
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
                            this->data->soc_cache[slot] = static_cast<uint8_t>(soc);
                            logger.tracefln(this->trace_buffer_idx, "meter %lu SOC=%hhu%%", slot, this->data->soc_cache[slot]);

                            this->data->evaluation_must_update_soc  = true;
                            this->data->evaluation_must_check_rules = true;
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
    }

    if (data->have_tariff_schedule_rule || data->have_solar_forecast_rule) {
#if MODULE_DAY_AHEAD_PRICES_AVAILABLE() || MODULE_SOLAR_FORECAST_AVAILABLE()
        // Update forecasts at 20:00.
        // The wall clock task has to run hourly and check the local time manually.
        // Registering the wall clock task with a 24h interval and a time-zone-dependent delay would schedule the task at midnight with a large delay.
        // If the device was restarted between midnight and 20:00, the task would not be scheduled until the next day.
        // Checking the local time here also means that DST doesn't matter.
        task_scheduler.scheduleWallClock([this]() {
            const int localtime_hour_now = get_localtime_hour();

            if (localtime_hour_now != 20) {
                return;
            }

#if MODULE_DAY_AHEAD_PRICES_AVAILABLE()
            if (this->data->have_tariff_schedule_rule) {
                this->update_tariff_schedule();
            }
#endif

#if MODULE_SOLAR_FORECAST_AVAILABLE()
            if (this->data->have_solar_forecast_rule) {
                this->update_solar_forecast(localtime_hour_now, api.getState("solar_forecast/state"));
            }
#endif
        }, 1_h, 0_ms, false);
#endif
    }

#if MODULE_DAY_AHEAD_PRICES_AVAILABLE()
    if (data->have_price_rule || data->have_tariff_schedule_rule) {
        event.registerEvent("day_ahead_prices/state", {}, [this](const Config *cfg) {
            const uint8_t event_api_backend_flag = event.get_api_backend_flag();

            if (this->data->have_price_rule) {
                const Config *current_price_cfg = static_cast<const Config *>(cfg->get("current_price"));

                if (current_price_cfg->was_updated(event_api_backend_flag)) {
                    const int32_t price = current_price_cfg->asInt();

                    if (price == std::numeric_limits<decltype(price)>::max()) {
                        this->data->price_cache = std::numeric_limits<decltype(this->data->price_cache)>::min();
                        logger.tracefln(this->trace_buffer_idx, "Ignoring uninitialized current_price from DAP");
                    } else {
                        this->data->price_cache = price;
                        logger.tracefln(this->trace_buffer_idx, "current_price=%li", price);
                    }

                    this->data->evaluation_must_check_rules = true;
                    this->schedule_evaluation();
                }
            }

            if (this->data->have_tariff_schedule_rule) {
                const Config *last_sync_cfg = static_cast<const Config *>(cfg->get("last_sync"));

                if (last_sync_cfg->was_updated(event_api_backend_flag) && last_sync_cfg->asUint() != 0) {
                    logger.tracefln(this->trace_buffer_idx, "DAP last_sync at %lu", last_sync_cfg->asUint());
                    this->update_tariff_schedule();
                }
            }

            return EventResult::OK;
        });
    }

    if (data->have_tariff_schedule_rule && (config.get("cheap_tariff_quarters")->asUint() != 0 || config.get("expensive_tariff_quarters")->asUint() != 0)) {
        task_scheduler.scheduleWallClock([this]() {
            this->evaluate_tariff_schedule();
        }, 15_min, 100_ms, false); // Slightly delayed to give the hourly task a chance to update the schedule first.
    }
#endif

#if MODULE_SOLAR_FORECAST_AVAILABLE()
    if (data->have_solar_forecast_rule) {
        event.registerEvent("solar_forecast/state", {}, [this](const Config *sf_state) {
            update_solar_forecast(get_localtime_hour(), sf_state);

            return EventResult::OK;
        });
    }
#endif

    if (data->have_time_rule) {
        task_scheduler.scheduleWallClock([this]() {
            this->data->evaluation_must_check_rules = true;
            this->schedule_evaluation();
        }, 1_min, 0_ms, false);
    }
}

void BatteryControl::preprocess_rules(const Config *rules_config, control_rule *rules)
{
    const size_t rules_count = rules_config->count();
    size_t enabled_i = 0;

    for (size_t i = 0; i < rules_count; i++) {
        const auto rule_config = rules_config->get(i);

        if (!rule_config->get("enabled")->asBool()) {
            continue;
        }

        control_rule *rule = rules + enabled_i;
        enabled_i++;

        rule->soc_cond      = rule_config->get("soc_cond"     )->asEnum<RuleCondition>();
        rule->soc_th        = rule_config->get("soc_th"       )->asUint8();
        rule->price_cond    = rule_config->get("price_cond"   )->asEnum<RuleCondition>();
        rule->price_th      = rule_config->get("price_th"     )->asInt16()  *  100;     // ct/10 -> ct/1000
        rule->forecast_cond = rule_config->get("forecast_cond")->asEnum<RuleCondition>();
        rule->forecast_th   = rule_config->get("forecast_th"  )->asUint16() * 1000;     // kWh -> Wh
        rule->schedule_cond = rule_config->get("schedule_cond")->asEnum<ScheduleRuleCondition>();
        rule->time_cond     = rule_config->get("time_cond"    )->asEnum<RuleCondition>();
        rule->time_start_s  = rule_config->get("time_start"   )->asUint16() * 60UL;
        rule->time_end_s    = rule_config->get("time_end"     )->asUint16() * 60UL;
        rule->fast_chg_cond = rule_config->get("fast_chg_cond")->asEnum<RuleCondition>();

        if (rule->soc_cond      != RuleCondition::Ignore        ) data->have_soc_rule             = true;
        if (rule->price_cond    != RuleCondition::Ignore        ) data->have_price_rule           = true;
        if (rule->forecast_cond != RuleCondition::Ignore        ) data->have_solar_forecast_rule  = true;
        if (rule->schedule_cond != ScheduleRuleCondition::Ignore) data->have_tariff_schedule_rule = true;
        if (rule->time_cond     != RuleCondition::Ignore        ) data->have_time_rule            = true;
        if (rule->fast_chg_cond != RuleCondition::Ignore        ) data->have_fast_chg_rule        = true;
    }
}

void BatteryControl::update_avg_soc()
{
    uint32_t soc_sum = 0;
    uint32_t soc_count = 0;

    for (size_t i = 0; i < ARRAY_SIZE(data->soc_cache); i++) {
        const uint8_t soc = data->soc_cache[i];
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

    data->soc_cache_avg = static_cast<decltype(data->soc_cache_avg)>(soc_sum / soc_count);

    logger.tracefln(trace_buffer_idx, "soc_avg=%li", data->soc_cache_avg);
}

#if MODULE_DAY_AHEAD_PRICES_AVAILABLE()
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

    data->tariff_schedule_start_min = static_cast<int32_t>(schedule_start_s / 60);

    bool cheap_hours    [sizeof(data->tariff_schedule)];
    bool expensive_hours[sizeof(data->tariff_schedule)];
    bool any_data_available = false;

    if (day_ahead_prices.get_cheap_and_expensive_15m(data->tariff_schedule_start_min, sizeof(cheap_hours),     cheap_tariff_quarters,     cheap_hours, nullptr)) {
        any_data_available = true;
    } else {
        memset(cheap_hours, false, sizeof(cheap_hours));
    }

    if (day_ahead_prices.get_cheap_and_expensive_15m(data->tariff_schedule_start_min, sizeof(expensive_hours), expensive_tariff_quarters, nullptr,     expensive_hours)) {
        any_data_available = true;
    } else {
        memset(expensive_hours, false, sizeof(expensive_hours));
    }

    if (any_data_available) {
        for (size_t i = 0; i < sizeof(data->tariff_schedule); i++) {
            data->tariff_schedule[i] = cheap_hours[i]     << BC_SCHEDULE_CHEAP_POS
                                     | expensive_hours[i] << BC_SCHEDULE_EXPENSIVE_POS;
        }
    } else {
        memset(data->tariff_schedule, 0, sizeof(data->tariff_schedule));
    }

    char str[sizeof(data->tariff_schedule) + 1];
    for (size_t i = 0; i < sizeof(data->tariff_schedule); i++) {
        str[i] = '0' + data->tariff_schedule[i];
    }
    str[sizeof(str) - 1] = 0;
    logger.tracefln(this->trace_buffer_idx, "Schedule: %s", str);

    evaluate_tariff_schedule();
}
#endif

void BatteryControl::evaluate_tariff_schedule()
{
    uint8_t charge_permitted_schedule_cache_update;

    if (data->tariff_schedule_start_min <= 0) {
        logger.printfln("Tariff schedule uninitialized: %li", data->tariff_schedule_start_min);
        charge_permitted_schedule_cache_update = 0;
    } else {
        const int32_t now_min = static_cast<int32_t>(time(nullptr) / 60);

        if (now_min < data->tariff_schedule_start_min) {
            logger.printfln("Tariff schedule starts in the future: %li < %li", now_min, data->tariff_schedule_start_min);
            charge_permitted_schedule_cache_update = 0;
        } else {
            const uint32_t quarter_index = static_cast<uint32_t>(now_min - data->tariff_schedule_start_min) / 15;

            if (quarter_index >= sizeof(data->tariff_schedule)) {
                logger.printfln("Quarter index beyond tariff schedule: %lu >= %u", quarter_index, sizeof(data->tariff_schedule));
                charge_permitted_schedule_cache_update = 0;
            } else {
                charge_permitted_schedule_cache_update = data->tariff_schedule[quarter_index];
            }
        }
    }

    if (data->tariff_schedule_cache != charge_permitted_schedule_cache_update) {
        data->tariff_schedule_cache  = charge_permitted_schedule_cache_update;
        logger.tracefln(this->trace_buffer_idx, "tariff_schedule_cache=%hhu", data->tariff_schedule_cache);
        data->evaluation_must_check_rules = true;
        schedule_evaluation();
    }
}

#if MODULE_SOLAR_FORECAST_AVAILABLE()
void BatteryControl::update_solar_forecast(int localtime_hour_now, const Config *sf_state)
{
    const char *const forecast_field_name = localtime_hour_now < 20 ? "wh_today" : "wh_tomorrow";
    const int32_t wh_forecast = sf_state->get(forecast_field_name)->asInt();

    if (wh_forecast < 0) {
        logger.tracefln(this->trace_buffer_idx, "Ignoring uninitialized %s forecast: %li", forecast_field_name, wh_forecast);
        return;
    }

    if (this->data->forecast_cache == wh_forecast) {
        logger.tracefln(this->trace_buffer_idx, "%s=%li unchanged", forecast_field_name, wh_forecast);
    } else {
        logger.tracefln(this->trace_buffer_idx, "%s=%li changed", forecast_field_name, wh_forecast);
        this->data->forecast_cache = wh_forecast;
        this->data->evaluation_must_check_rules = true;
        this->schedule_evaluation();
    }
}
#endif

void BatteryControl::schedule_evaluation()
{
    if (data->evaluation_task_id != 0) {
        return;
    }

    data->evaluation_task_id = task_scheduler.scheduleOnce([this]() {
        logger.tracefln(this->trace_buffer_idx, "Evaluating");

        this->data->evaluation_task_id = 0;

        if (this->data->evaluation_must_check_rules) {
            this->data->evaluation_must_check_rules = false;

            if (this->data->evaluation_must_update_soc) {
                this->data->evaluation_must_update_soc = false;

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

    if (cond == RuleCondition::BelowOrNo) {
        return !(value < th); // intentionally negated
    }

    // RuleCondition::AboveOrYes
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

static bool time_rule_condition_failed(const RuleCondition cond, const uint32_t time_start_s, const uint32_t time_end_s, uint32_t time_since_midnight_s)
{
    if (cond == RuleCondition::Ignore) {
        return false;
    }

    const bool inside_time_window = time_start_s <= time_since_midnight_s && time_since_midnight_s < time_end_s; // not <= end

    if (cond == RuleCondition::BelowOrNo) {
        return inside_time_window; // intentionally inverted
    }

    // RuleCondition::AboveOrYes
    return !inside_time_window; // intentionally negated
}

TristateBool BatteryControl::evaluate_rules(const control_rule *rules, size_t rules_count, const char *rules_type_name, uint32_t time_since_midnight_s)
{
    for (size_t i = 0; i < rules_count; i++) {
        const control_rule *rule = rules + i;

        if (rule_condition_failed(rule->soc_cond,      rule->soc_th,      data->soc_cache_avg )) continue;
        if (rule_condition_failed(rule->price_cond,    rule->price_th,    data->price_cache   )) continue;
        if (rule_condition_failed(rule->forecast_cond, rule->forecast_th, data->forecast_cache)) continue;
        if (rule_condition_failed(rule->fast_chg_cond, 0, data->fast_charger_in_c_cache * 2 - 1)) continue;
        if (schedule_rule_condition_failed(rule->schedule_cond, data->tariff_schedule_cache)) continue;
        if (time_rule_condition_failed(rule->time_cond, rule->time_start_s, rule->time_end_s, time_since_midnight_s)) continue;

        // Complete rule matches.
        logger.tracefln(this->trace_buffer_idx, "%s rule %zu matches", rules_type_name, i);
        return TristateBool::True;
    }

    return TristateBool::False;
}

void BatteryControl::evaluate_all_rules()
{
    uint32_t time_since_midnight_s;

    if (data->have_time_rule) {
        const time_t time_utc = time(nullptr);
        struct tm tm_local;
        localtime_r(&time_utc, &tm_local);

        time_since_midnight_s = static_cast<uint32_t>((tm_local.tm_hour * 60 + tm_local.tm_min) * 60 + tm_local.tm_sec);
    } else {
        time_since_midnight_s = 0;
    }

    data->action_influence_active[static_cast<size_t>(ActionPair::PermitGridCharge)].activated_by_rules = evaluate_rules(data->permit_grid_charge_rules, data->permit_grid_charge_rules_count, "Permit grid charge", time_since_midnight_s);
    data->action_influence_active[static_cast<size_t>(ActionPair::ForbidDischarge )].activated_by_rules = evaluate_rules(data->forbid_discharge_rules,   data->forbid_discharge_rules_count,   "Forbid discharge",   time_since_midnight_s);
    data->action_influence_active[static_cast<size_t>(ActionPair::ForbidCharge    )].activated_by_rules = evaluate_rules(data->forbid_charge_rules,      data->forbid_charge_rules_count,      "Forbid charge",      time_since_midnight_s);

    logger.tracefln(this->trace_buffer_idx, "Rules: PChg=%u FDis=%u FChg=%u",
                    static_cast<unsigned>(data->action_influence_active[static_cast<size_t>(ActionPair::PermitGridCharge)].activated_by_rules),
                    static_cast<unsigned>(data->action_influence_active[static_cast<size_t>(ActionPair::ForbidDischarge )].activated_by_rules),
                    static_cast<unsigned>(data->action_influence_active[static_cast<size_t>(ActionPair::ForbidCharge    )].activated_by_rules));
}

void BatteryControl::evaluate_summary()
{
    evaluate_action_pair(ActionPair::PermitGridCharge);
    evaluate_action_pair(ActionPair::ForbidDischarge);
    evaluate_action_pair(ActionPair::ForbidCharge);
}

void BatteryControl::evaluate_action_pair(ActionPair action_pair)
{
    action_influence_data *action_influence = data->action_influence_active + static_cast<size_t>(action_pair);

    if (action_influence->activated != action_influence->activated_by_rules) {
        action_influence->activated  = action_influence->activated_by_rules;

        update_batteries_and_state(action_pair, true);
    }
}

void BatteryControl::periodic_update()
{
    if (data->have_fast_chg_rule) {
        const bool fast_charger_in_c_cm = charge_manager.fast_charger_in_c;

        if (fast_charger_in_c_cm != data->fast_charger_in_c_cache) {
            logger.tracefln(trace_buffer_idx, "fast_charger_in_c=%i", fast_charger_in_c_cm);
            data->fast_charger_in_c_cache = fast_charger_in_c_cm;

            data->evaluation_must_check_rules = true;
            schedule_evaluation();
        }
    }

    if (data->must_repeat) {
        const micros_t now = now_us();

        for (size_t action_pair_i = 0; action_pair_i < ARRAY_SIZE(data->action_influence_active); action_pair_i++) {
            const action_influence_data *action_influence = data->action_influence_active + action_pair_i;
            const bool influence_active = action_influence->activated == TristateBool::True;
            const size_t active_action_num = action_pair_i * 2 + (influence_active ? 0 : 1);

            for (uint8_t battery_i = 0; battery_i < data->max_used_batteries; battery_i++) {
                if (now >= data->battery_repeats[battery_i].next_action_update[active_action_num]) {
                    update_batteries_and_state(static_cast<ActionPair>(action_pair_i), false, battery_i);
                }
            }
        }
    }
}

struct battery_control_action_info {
    BatteryAction influence_start_action;
    BatteryAction influence_end_action;
    const char *influence_start_msg;
    const char *influence_end_msg;
    const char *state_name;
};

const battery_control_action_info action_infos[] = {
    {
        BatteryAction::PermitGridCharge,
        BatteryAction::RevokeGridChargeOverride,
        "Permit grid charge",
        "Revoke grid charge override",
        "grid_charge_permitted",
    },
    {
        BatteryAction::ForbidDischarge,
        BatteryAction::RevokeDischargeOverride,
        "Forbid discharge",
        "Revoke discharge override",
        "discharge_forbidden",
    },
    {
        BatteryAction::ForbidCharge,
        BatteryAction::RevokeChargeOverride,
        "Forbid charge",
        "Revoke charge override",
        "charge_forbidden",
    },
};

void BatteryControl::update_batteries_and_state(ActionPair action_pair, bool changed, uint32_t battery_slot)
{
    const size_t action_pair_num = static_cast<size_t>(action_pair);
    const battery_control_action_info *action_info = action_infos + action_pair_num;

    const bool influence_active = data->action_influence_active[action_pair_num].activated == TristateBool::True;

    this->state.get(action_info->state_name)->updateBool(influence_active);

    BatteryAction action_to_start;

    if (influence_active) {
        if (changed) {
            logger.tracefln(trace_buffer_idx, "%s", action_info->influence_start_msg);
            logger.printfln("%s", action_info->influence_start_msg);
        }
        action_to_start = action_info->influence_start_action;
    } else {
        if (changed) {
            logger.tracefln(trace_buffer_idx, "%s", action_info->influence_end_msg);
            logger.printfln("%s", action_info->influence_end_msg);
        }
        action_to_start = action_info->influence_end_action;
    }

    //logger.printfln("Starting action %u on battery slot %i", static_cast<unsigned>(action_to_start), static_cast<int>(battery_slot));

    uint32_t battery_i_start;
    uint32_t battery_i_end;

    if (battery_slot == std::numeric_limits<decltype(battery_slot)>::max()) {
        battery_i_start = 0;
        battery_i_end   = data->max_used_batteries;
    } else {
        battery_i_start = battery_slot;
        battery_i_end   = battery_slot + 1;
    }

    for (uint32_t battery_i = battery_i_start; battery_i < battery_i_end; battery_i++) {
        IBattery *battery = batteries.get_battery(battery_i);

        if (battery == nullptr) {
            logger.printfln("Cannot execute action on non-existing battery slot %lu", battery_i);
        } else {
            battery_repeat_data *battery_repeat = data->battery_repeats + battery_i;

            static_assert(OPTIONS_BATTERIES_MAX_SLOTS() < std::numeric_limits<uint16_t>::max());
            const uint16_t battery_i_capture = static_cast<uint16_t>(battery_i);
            const uint8_t  action_num        = static_cast<std::underlying_type<decltype(action_to_start)>::type>(action_to_start);

            battery->start_action(action_to_start, [battery_repeat, battery_i_capture, action_num](bool success) {
                micros_t next_update = now_us();

                if (success) {
                    next_update += battery_repeat->repeat_interval[action_num];
                } else {
                    next_update += 5_s;

                    const uint32_t slot = battery_i_capture; // printfln_battery expects a variable named "slot".
                    logger.printfln_battery("Failed to start action %hhu", action_num);
                }

                battery_repeat->next_action_update[action_num] = next_update;
            });
        }
    }
}
