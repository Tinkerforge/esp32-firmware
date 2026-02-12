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
#include "tools/string_builder.h"

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
    trace_buffer_idx = logger.alloc_trace_buffer("battery_control", 64*1024u);

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
        {"action",        Config::Enum  (RuleAction::Normal)},
    });

    rules_charge    = Config::Array({}, &rule_prototype, 0, OPTIONS_BATTERY_CONTROL_MAX_RULES_PER_TYPE(), Config::type_id<Config::ConfObject>());
    rules_discharge = Config::Array({}, &rule_prototype, 0, OPTIONS_BATTERY_CONTROL_MAX_RULES_PER_TYPE(), Config::type_id<Config::ConfObject>());

    state = Config::Object({
        {"mode",                  Config::Enum(BatteryMode::None)},
        {"active_charge_rule",    Config::Uint8(255)},
        {"active_discharge_rule", Config::Uint8(255)},
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
    api.restorePersistentConfig("battery_control/config",          &config);
    api.restorePersistentConfig("battery_control/rules_charge",    &rules_charge);
    api.restorePersistentConfig("battery_control/rules_discharge", &rules_discharge);

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

    for (size_t i = 0; i < ARRAY_SIZE(data->soc_cache); i++) {
        data->soc_cache[i] = UNAVAILABLE_SOC_CACHE;
    }

    const size_t charge_rules_cnt    = count_enabled_rules(rules_charge);
    const size_t discharge_rules_cnt = count_enabled_rules(rules_discharge);
    const size_t total_rules_cnt = charge_rules_cnt + discharge_rules_cnt;

    if (total_rules_cnt > 0) {
        control_rule *rules = static_cast<decltype(rules)>(malloc_psram_or_dram(total_rules_cnt * sizeof(*rules))); // Not large but rarely accessed, so it can live in PSRAM.
        if (!rules) {
            logger.printfln("Failed to allocate memory for rules");
            return;
        }

        control_rule *chg_rules = rules;
        if (charge_rules_cnt > 0) {
            data->charge_rules       = chg_rules;
            data->charge_rules_count = static_cast<uint8_t>(charge_rules_cnt);

            preprocess_rules(&rules_charge, chg_rules);
        }

        control_rule *dchg_rules = chg_rules + data->charge_rules_count;
        if (discharge_rules_cnt > 0) {
            data->discharge_rules       = dchg_rules;
            data->discharge_rules_count = static_cast<uint8_t>(discharge_rules_cnt);

            preprocess_rules(&rules_discharge, dchg_rules);
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

        logger.tracefln(this->trace_buffer_idx, "Conditions:%s", buf);
    }
}

void BatteryControl::register_urls()
{
    api.addPersistentConfig("battery_control/config",          &config);
    api.addPersistentConfig("battery_control/rules_charge",    &rules_charge);
    api.addPersistentConfig("battery_control/rules_discharge", &rules_discharge);
    api.addState("battery_control/state", &state);
}

void BatteryControl::register_events()
{
    if (data == nullptr) {
        return;
    }

    const size_t total_rules_cnt = static_cast<size_t>(data->charge_rules_count) + static_cast<size_t>(data->discharge_rules_count);

    if (total_rules_cnt == 0) {
        return;
    }

    if (data->have_fast_chg_rule) {
#if MODULE_NETWORK_AVAILABLE()
        network.on_network_connected([this](const Config *connected) {
            if (!connected->asBool()) {
                return EventResult::OK;
            }

            task_scheduler.scheduleUncancelable([this]() {
                this->fast_charge_update();
            }, 5_s, 1_s);

            return EventResult::Deregister;
        });
#else
        task_scheduler.scheduleUncancelable([this]() {
            this->fast_charge_update();
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
                    logger.printfln("Battery meter in slot %lu doesn't provide SoC.", slot);
                } else {
                    event.registerEvent(meters.get_path(slot, Meters::PathType::Values), {static_cast<size_t>(soc_index)}, [this, slot](const Config *config_soc) {
                        const float soc = config_soc->asFloat();

                        if (!isnan(soc)) {
                            this->data->soc_cache[slot] = static_cast<uint8_t>(soc);
                            logger.tracefln(this->trace_buffer_idx, "meter %lu SoC=%hhu%%", slot, this->data->soc_cache[slot]);

                            this->data->evaluation_must_update_soc  = true;
                            this->data->evaluation_must_check_rules = true;
                            this->schedule_evaluation();
                        } else {
                            logger.tracefln(this->trace_buffer_idx, "Ignoring uninitialized SoC from battery meter %lu", slot);
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
                        this->data->price_cache = UNAVAILABLE_CONDITION_CACHE;
                        logger.tracefln(this->trace_buffer_idx, "current_price from DAP unavailable");
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

                if (last_sync_cfg->was_updated(event_api_backend_flag)) {
                    const uint32_t last_sync = last_sync_cfg->asUint();

                    if (last_sync == 0) {
                        logger.tracefln(this->trace_buffer_idx, "DAP not synced", );
                        data->tariff_schedule_start_min = 0;
                        this->evaluate_tariff_schedule();
                    } else {
                        logger.tracefln(this->trace_buffer_idx, "DAP last_sync at %lu", last_sync);
                        this->update_tariff_schedule();
                    }
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

    task_scheduler.scheduleOnce([this]() {
        if (this->data->last_mode == BatteryMode::None) {
            logger.tracefln(this->trace_buffer_idx, "Defaulting to Normal mode after 2 minutes without data trigger");

            this->set_mode(BatteryMode::Normal);
        }
    }, 2_min);
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
        rule->action        = rule_config->get("action"       )->asEnum<RuleAction>();
        rule->index         = static_cast<uint8_t>(i);

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
        if (soc == UNAVAILABLE_SOC_CACHE) {
            continue;
        }
        soc_sum += soc;
        soc_count++;
    }

    if (soc_count == 0) {
        data->soc_cache_avg = UNAVAILABLE_CONDITION_CACHE;
        logger.tracefln(this->trace_buffer_idx, "soc_avg has no data: No battery meters have SoC data.");
        return;
    }

    data->soc_cache_avg = static_cast<decltype(data->soc_cache_avg)>(soc_count == 1 ? soc_sum : soc_sum / soc_count);

    logger.tracefln(trace_buffer_idx, "soc_avg=%li", data->soc_cache_avg);
}

#if MODULE_DAY_AHEAD_PRICES_AVAILABLE()
static char tariff_schedule_quarter_to_char(uint8_t quarter) {
    switch (quarter) {
        case 0:                          return 'N';
        case BC_SCHEDULE_CHEAP_MASK:     return 'C';
        case BC_SCHEDULE_EXPENSIVE_MASK: return 'E';
        case BatteryControl::UNAVAILABLE_TARIFF_SCHEDULE_CACHE: return 'U';
        default: { // Multiple bits set :-?
            if (quarter < 10) {
                return '0' + quarter;
            } else if (quarter < 36) {
                return 'a' + quarter - 10;
            } else {
                return '?';
            }
        }
    }
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
            if (schedule_start_s == now_s) {
                logger.tracefln(this->trace_buffer_idx, "Schedule begins now");
            } else {
                logger.tracefln(this->trace_buffer_idx, "Schedule begins %llis in the past: %i-%02i-%02i %02i:%02i:%02i", now_s - schedule_start_s, start_date.tm_year+1900, start_date.tm_mon+1, start_date.tm_mday, start_date.tm_hour, start_date.tm_min, start_date.tm_sec);
            }
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
        char str[sizeof(data->tariff_schedule) + 1];

        for (size_t i = 0; i < sizeof(data->tariff_schedule); i++) {
            data->tariff_schedule[i] = cheap_hours[i]     << BC_SCHEDULE_CHEAP_POS
                                     | expensive_hours[i] << BC_SCHEDULE_EXPENSIVE_POS;

            str[i] = tariff_schedule_quarter_to_char(data->tariff_schedule[i]);
        }

        str[sizeof(str) - 1] = '\n';

        logger.trace_plain(this->trace_buffer_idx, str, sizeof(str));
    } else {
        data->tariff_schedule_start_min = 0;
        memset(data->tariff_schedule, 0, sizeof(data->tariff_schedule));
    }

    evaluate_tariff_schedule();
}

void BatteryControl::evaluate_tariff_schedule()
{
    uint8_t charge_permitted_schedule_cache_update;

    if (data->tariff_schedule_start_min <= 0) {
        logger.tracefln(this->trace_buffer_idx, "Tariff schedule unavailable");
        charge_permitted_schedule_cache_update = UNAVAILABLE_TARIFF_SCHEDULE_CACHE;
    } else {
        const int32_t now_min = static_cast<int32_t>(time(nullptr) / 60);

        if (now_min < data->tariff_schedule_start_min) {
            logger.printfln("Tariff schedule starts in the future: %li < %li", now_min, data->tariff_schedule_start_min);
            charge_permitted_schedule_cache_update = UNAVAILABLE_TARIFF_SCHEDULE_CACHE;
        } else {
            const uint32_t quarter_index = static_cast<uint32_t>(now_min - data->tariff_schedule_start_min) / 15;

            if (quarter_index >= sizeof(data->tariff_schedule)) {
                logger.printfln("Quarter index beyond tariff schedule: %lu >= %u", quarter_index, sizeof(data->tariff_schedule));
                charge_permitted_schedule_cache_update = UNAVAILABLE_TARIFF_SCHEDULE_CACHE;
            } else {
                charge_permitted_schedule_cache_update = data->tariff_schedule[quarter_index];
            }
        }
    }

    if (data->tariff_schedule_cache != charge_permitted_schedule_cache_update) {
        data->tariff_schedule_cache  = charge_permitted_schedule_cache_update;
        logger.tracefln(this->trace_buffer_idx, "Tariff schedule=%c", tariff_schedule_quarter_to_char(data->tariff_schedule_cache));
        data->evaluation_must_check_rules = true;
        schedule_evaluation();
    }
}
#endif

#if MODULE_SOLAR_FORECAST_AVAILABLE()
void BatteryControl::update_solar_forecast(int localtime_hour_now, const Config *sf_state)
{
    const char *const forecast_field_name = localtime_hour_now < 20 ? "wh_today" : "wh_tomorrow";
    int32_t wh_forecast = sf_state->get(forecast_field_name)->asInt();

    if (wh_forecast < 0) {
        logger.tracefln(this->trace_buffer_idx, "%s forecast unavailable: %li", forecast_field_name, wh_forecast);
        wh_forecast = UNAVAILABLE_CONDITION_CACHE;
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
        this->data->evaluation_task_id = 0;

        if (this->data->evaluation_must_check_rules) {
            this->data->evaluation_must_check_rules = false;

            if (this->data->evaluation_must_update_soc) {
                this->data->evaluation_must_update_soc = false;

                this->update_avg_soc();
            }

            this->evaluate_all_rules();
        } else {
            logger.printfln("Evaluation scheduled but nothing to do");
        }
    });
}

static bool rule_condition_failed(const RuleCondition cond, const int32_t th, const int32_t value)
{
    if (cond == RuleCondition::Ignore) {
        return false;
    }

    if (value == BatteryControl::UNAVAILABLE_CONDITION_CACHE) {
        return true; // Input value is uninitialized, rule cannot be true.
    }

    if (cond == RuleCondition::BelowOrNo) {
        return !(value < th); // intentionally negated
    }

    // RuleCondition::AboveOrYes
    return !(value > th); // intentionally negated
}

static bool tristate_rule_condition_failed(const RuleCondition cond, const TristateBool value)
{
    if (cond == RuleCondition::Ignore) {
        return false;
    }

    if (value == TristateBool::Undefined) {
        return true; // Input value is uninitialized, rule cannot be true.
    }

    if (cond == RuleCondition::BelowOrNo) {
        return value != TristateBool::False; // intentionally negated
    }

    // RuleCondition::AboveOrYes
    return value != TristateBool::True; // intentionally negated
}

static bool schedule_rule_condition_failed(const ScheduleRuleCondition cond, const uint8_t value)
{
    if (cond == ScheduleRuleCondition::Ignore) {
        return false;
    }

    if (value == BatteryControl::UNAVAILABLE_TARIFF_SCHEDULE_CACHE) {
        return true; // Input value is uninitialized, rule cannot be true.
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

RuleAction BatteryControl::evaluate_rules(const control_rule *rules, size_t rules_count, const char *rules_type_name, uint32_t time_since_midnight_s, uint8_t *active_rule_out)
{
    for (size_t i = 0; i < rules_count; i++) {
        const control_rule *rule = rules + i;

        if (rule_condition_failed(rule->soc_cond,      rule->soc_th,      data->soc_cache_avg )) continue;
        if (rule_condition_failed(rule->price_cond,    rule->price_th,    data->price_cache   )) continue;
        if (rule_condition_failed(rule->forecast_cond, rule->forecast_th, data->forecast_cache)) continue;
        if (tristate_rule_condition_failed(rule->fast_chg_cond, data->fast_charger_in_c_cache)) continue;
        if (schedule_rule_condition_failed(rule->schedule_cond, data->tariff_schedule_cache)) continue;
        if (time_rule_condition_failed(rule->time_cond, rule->time_start_s, rule->time_end_s, time_since_midnight_s)) continue;

        // Complete rule matches.
        logger.tracefln(this->trace_buffer_idx, "%s %zu match", rules_type_name, i);
        *active_rule_out = rule->index;
        return rule->action;
    }

    return RuleAction::Normal;
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

    RuleAction charge_action;
    RuleAction discharge_action;
    BatteryMode new_mode;
    uint8_t active_charge_rule    = std::numeric_limits<uint8_t>::max();
    uint8_t active_discharge_rule = std::numeric_limits<uint8_t>::max();

    do {
        charge_action = evaluate_rules(data->charge_rules, data->charge_rules_count, "Charge", time_since_midnight_s, &active_charge_rule);

        // Forced charging overrides everything.
        if (charge_action == RuleAction::Force) {
            new_mode = BatteryMode::ChargeFromGrid;
            discharge_action = static_cast<RuleAction>(std::numeric_limits<std::underlying_type<RuleAction>::type>::max());
            break;
        }

        discharge_action = evaluate_rules(data->discharge_rules, data->discharge_rules_count, "Discharge", time_since_midnight_s, &active_discharge_rule);

        // Forced discharge overrides normal mode.
        if (discharge_action == RuleAction::Force) {
            new_mode = BatteryMode::DischargeToGrid;
            break;
        }

        // Check normal or blocking actions
        if (charge_action == RuleAction::Normal) {
            if (discharge_action == RuleAction::Normal) {
                new_mode = BatteryMode::Normal;
            } else { // block discharge
                new_mode = BatteryMode::ChargeFromExcess;
            }
        } else { // block charge
            if (discharge_action == RuleAction::Normal) {
                new_mode = BatteryMode::DischargeToLoad;
            } else { // block discharge
                new_mode = BatteryMode::Block;
            }
        }
    } while (false);

    logger.tracefln(this->trace_buffer_idx, "Chg=%c DChg=%c Mode=%u",
                    get_rule_action_name(charge_action)[0],
                    get_rule_action_name(discharge_action)[0],
                    static_cast<unsigned>(new_mode));

    state.get("active_charge_rule"   )->updateUint(active_charge_rule);
    state.get("active_discharge_rule")->updateUint(active_discharge_rule);

    set_mode(new_mode);
}

void BatteryControl::set_mode(BatteryMode new_mode)
{
    if (new_mode == data->last_mode) {
        return;
    }

    data->last_mode = new_mode;
    state.get("mode")->updateEnum(new_mode);

    for (uint32_t battery_i = 0; battery_i < data->max_used_batteries; battery_i++) {
        IBattery *battery = batteries.get_battery(battery_i);

        if (battery == nullptr) {
            logger.printfln("Cannot set mode on non-existing battery slot %lu", battery_i);
            continue;
        }

        battery->set_mode(new_mode);
    }
}

void BatteryControl::fast_charge_update()
{
    const TristateBool fast_charger_in_c_cm = static_cast<TristateBool>(charge_manager.fast_charger_in_c);

    if (fast_charger_in_c_cm != data->fast_charger_in_c_cache) {
        logger.tracefln(trace_buffer_idx, "fast_charger_in_c=%i", static_cast<int>(fast_charger_in_c_cm));
        data->fast_charger_in_c_cache = fast_charger_in_c_cm;

        data->evaluation_must_check_rules = true;
        schedule_evaluation();
    }
}
