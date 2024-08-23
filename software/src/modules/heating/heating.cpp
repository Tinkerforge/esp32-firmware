/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
#include "heating.h"

#include <time.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

#define HEATING_UPDATE_INTERVAL 1000*60

extern Heating heating;

#define extended_logging(...) \
    if(extended_logging_active) { \
        logger.printfln(__VA_ARGS__); \
    }

void Heating::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"minimum_control_holding_time", Config::Uint(15, 0, 60)},
        {"extended_logging_active", Config::Bool(false)},
        {"winter_start_day", Config::Uint(1, 1, 31)},
        {"winter_start_month", Config::Uint(11, 1, 12)},
        {"winter_end_day", Config::Uint(15, 1, 31)},
        {"winter_end_month", Config::Uint(3, 1, 12)},
        {"winter_dynamic_price_control_active", Config::Bool(false)},
        {"winter_dynamic_price_control_threshold", Config::Uint(80, 0, 100)},
        {"winter_pv_excess_control_active", Config::Bool(false)},
        {"winter_pv_excess_control_threshold", Config::Uint(0)},
        {"summer_block_time_active", Config::Bool(false)},
        {"summer_block_time_morning", Config::Int(8*60)},  // localtime in minutes since 00:00
        {"summer_block_time_evening", Config::Int(20*60)}, // localtime in minutes since 00:00
        {"summer_yield_forecast_active", Config::Bool(false)},
        {"summer_yield_forecast_threshold", Config::Uint(0)},
        {"summer_dynamic_price_control_active", Config::Bool(false)},
        {"summer_dynamic_price_control_threshold", Config::Uint(80, 0, 100)},
        {"summer_pv_excess_control_active", Config::Bool(false)},
        {"summer_pv_excess_control_threshold", Config::Uint(0)},
        {"summer_pv_excess_control_holding_time", Config::Uint(15, 0, 12*60)},
        {"p14enwg_active", Config::Bool(false)},
        {"p14enwg_input", Config::Uint(0, 0, 3)},
        {"p14enwg_active_type", Config::Uint(0, 0, 1)}
    }), [this](Config &update, ConfigSource source) -> String {
        task_scheduler.scheduleOnce([this]() {
            this->update();
        }, 10);
        return "";
    }};

    state = Config::Object({

    });
}

void Heating::setup()
{
    api.restorePersistentConfig("heating/config", &config);

    initialized = true;
}

void Heating::register_urls()
{
    api.addPersistentConfig("heating/config", &config);
    api.addState("heating/state",             &state);

    task_scheduler.scheduleWhenClockSynced([this]() {
        task_scheduler.scheduleWithFixedDelay([this]() {
            this->update();
        }, 0, HEATING_UPDATE_INTERVAL);
    });
}

void Heating::update()
{
    const bool extended_logging_active = config.get("extended_logging_active")->asBool();
    // Only update if clock is synced. Heating control depends on time of day.
    struct timeval tv_now;
    if (!clock_synced(&tv_now)) {
        extended_logging("Clock not synced. Skipping update.");
        return;
    }

    // TODO: Does §14 EnWG need a smaller update interval or is it enough to check it every minute?
    const bool     p14enwg_active      = config.get("p14enwg_active")->asBool();
    const uint32_t p14enwg_input       = config.get("p14enwg_input")->asUint();
    const uint32_t p14enwg_active_type = config.get("p14enwg_active_type")->asUint();
    if(p14enwg_active) {
        bool p14enwg_on  = false;
        bool input_value = false; // energy_manager.get_input(p14enwg_input);
        if (p14enwg_active_type == 0 && input_value) {
            p14enwg_on = true;
        } else if(p14enwg_active_type == 1 && !input_value) {
            p14enwg_on = true;
        }

        if(p14enwg_on) {
            extended_logging("§14 EnWG blocks heating. Turning on SG ready output 0.");
            // energy_manager.set_sg_ready_on(SG_READY_OUTPUT0);
        } else {
            extended_logging("§14 EnWG does not block heating. Turning off SG ready output 0.");
            // energy_manager.set_sg_ready_off(SG_READY_OUTPUT0);
        }
    }

    // Get values from config
    const uint8_t minimum_control_holding_time = config.get("minimum_control_holding_time")->asUint();
    const uint32_t minutes = timestamp_minutes();
    if(minutes < (last_sg_ready_change + minimum_control_holding_time)) {
        extended_logging("Minimum control holding time not reached. Current time: %d, last change: %d, minimum holding time: %d.", minutes, last_sg_ready_change, minimum_control_holding_time);
        return;
    }

    const uint32_t winter_start_day                       = config.get("winter_start_day")->asUint();
    const uint32_t winter_start_month                     = config.get("winter_start_month")->asUint();
    const uint32_t winter_end_day                         = config.get("winter_end_day")->asUint();
    const uint32_t winter_end_month                       = config.get("winter_end_month")->asUint();
    const bool     winter_dynamic_price_control_active    = config.get("winter_dynamic_price_control_active")->asBool();
    const uint32_t winter_dynamic_price_control_threshold = config.get("winter_dynamic_price_control_threshold")->asUint();
    const bool     winter_pv_excess_control_active        = config.get("winter_pv_excess_control_active")->asBool();
    const uint32_t winter_pv_excess_control_threshold     = config.get("winter_pv_excess_control_threshold")->asUint();
    const bool     summer_block_time_active               = config.get("summer_block_time_active")->asBool();
    const int32_t  summer_block_time_morning              = config.get("summer_block_time_morning")->asInt();
    const int32_t  summer_block_time_evening              = config.get("summer_block_time_evening")->asInt();
    const bool     summer_yield_forecast_active           = config.get("summer_yield_forecast_active")->asBool();
    const uint32_t summer_yield_forecast_threshold        = config.get("summer_yield_forecast_threshold")->asUint();
    const bool     summer_dynamic_price_control_active    = config.get("summer_dynamic_price_control_active")->asBool();
    const uint32_t summer_dynamic_price_control_threshold = config.get("summer_dynamic_price_control_threshold")->asUint();
    const bool     summer_pv_excess_control_active        = config.get("summer_pv_excess_control_active")->asBool();
    const uint32_t summer_pv_excess_control_threshold     = config.get("summer_pv_excess_control_threshold")->asUint();

    const time_t now              = time(NULL);
    const struct tm *current_time = localtime(&now);
    const int current_month       = current_time->tm_mon + 1;
    const int current_day         = current_time->tm_mday;
    const int current_minutes     = current_time->tm_hour * 60 + current_time->tm_min;

    const bool is_winter = ((current_month == winter_start_month) && (current_day >= winter_start_day )) ||
                           ((current_month == winter_end_month  ) && (current_day <= winter_end_day   )) ||
                           ((current_month > winter_start_month ) && (current_month < winter_end_month));

    bool sg_ready_on = false;

    if (is_winter) { // Winter
        extended_logging("It is winter. Current month: %d, winter start month: %d, winter end month: %d, current day: %d, winter start day: %d, winter end day: %d.", current_month, winter_start_month, winter_end_month, current_day, winter_start_day, winter_end_day);
        if (!winter_dynamic_price_control_active && !winter_pv_excess_control_active) {
            extended_logging("It is winter but no winter control active.");
        } else {
            if (winter_dynamic_price_control_active) {
                const auto price_average = day_ahead_prices.get_average_price_today();
                const auto price_current = day_ahead_prices.get_current_price();

                if (!price_average.data_available) {
                    extended_logging("Average price for today not available. Ignoring dynamic price control.");
                } else if (!price_current.data_available) {
                    extended_logging("Current price not available. Ignoring dynamic price control.");
                } else {
                    if (price_current.data < price_average.data * winter_dynamic_price_control_threshold / 100.0) {
                        extended_logging("Price is below threshold. Average price: %dmct, current price: %dmct, threshold: %d%%.", price_average.data, price_current.data, winter_dynamic_price_control_threshold);
                        sg_ready_on = true;
                    } else {
                        extended_logging("Price is above threshold. Average price: %dmct, current price: %dmct, threshold: %d%%.", price_average.data, price_current.data, winter_dynamic_price_control_threshold);
                        sg_ready_on = false;
                    }
                }
            }

            if (winter_pv_excess_control_active) {
                const int watt_current = 0; // meters.get_current_pv_excess();
                if (watt_current > winter_pv_excess_control_threshold) {
                    extended_logging("Current PV excess is above threshold. Current PV excess: %dW, threshold: %dW.", watt_current, winter_pv_excess_control_threshold);
                    sg_ready_on = sg_ready_on || true;
                }
            }
        }
    } else { // Summer
        extended_logging("It is summer. Current month: %d, winter start month: %d, winter end month: %d, current day: %d, winter start day: %d, winter end day: %d.", current_month, winter_start_month, winter_end_month, current_day, winter_start_day, winter_end_day);
        bool blocked = false;
        bool is_morning = false;
        bool is_evening = false;
        if (summer_block_time_active) {
            if (current_minutes <= summer_block_time_morning) {       // if is between 00:00 and summer_block_time_morning
                extended_logging("We are in morning block time. Current time: %d, block time morning: %d.", current_minutes, summer_block_time_morning);
                blocked    = true;
                is_morning = true;
            } else if(summer_block_time_evening <= current_minutes) { // if is between summer_block_time_evening and 23:59
                extended_logging("We are in evening block time. Current time: %d, block time evening: %d.", current_minutes, summer_block_time_evening);
                blocked    = true;
                is_evening = true;
            } else {
                extended_logging("We are not in block time. Current time: %d, block time morning: %d, block time evening: %d.", current_minutes, summer_block_time_morning, summer_block_time_evening);
            }
        }

        // If we are in block time and px excess control is active,
        // we check the expected px excess and unblock if it is below the threshold.
        if (blocked && summer_yield_forecast_active) {
            extended_logging("We are in block time and yield forecast is active.");
            DataReturn<uint32_t> kwh_expected = {false, 0};
            if (is_morning) {
                kwh_expected = solar_forecast.get_kwh_today();
            } else if (is_evening) {
                kwh_expected = solar_forecast.get_kwh_tomorrow();
            } else {
                extended_logging("We are in block time but not in morning or evening. Ignoring yield forecast.");
            }

            if(!kwh_expected.data_available) {
                extended_logging("Expected PV yield not available. Ignoring yield forecast.");
            } else {
                if (kwh_expected.data < summer_yield_forecast_threshold) {
                    extended_logging("Expected PV yield %dkWh is below threshold of %dkWh.", kwh_expected.data, summer_yield_forecast_threshold);
                    blocked = false;
                } else {
                    extended_logging("Expected PV yield %dkWh is above or equasl to threshold of %dkWh.", kwh_expected.data, summer_yield_forecast_threshold);
                }
            }
        }

        if (blocked) {
            extended_logging("We are in a block time.");
            sg_ready_on = false;
        } else {
            if (!summer_dynamic_price_control_active && !summer_pv_excess_control_active) {
                extended_logging("It is summer but no summer control active.");
            } else {
                if (summer_dynamic_price_control_active) {
                    const auto price_average = day_ahead_prices.get_average_price_today();
                    const auto price_current = day_ahead_prices.get_current_price();

                    if (!price_average.data_available) {
                        extended_logging("Average price for today not available. Ignoring dynamic price control.");
                    } else if (!price_current.data_available) {
                        extended_logging("Current price not available. Ignoring dynamic price control.");
                    } else {
                        if (price_current.data < price_average.data * summer_dynamic_price_control_threshold / 100.0) {
                            extended_logging("Price is below threshold. Average price: %dmct, current price: %dmct, threshold: %d%%.", price_average.data, price_current.data, summer_dynamic_price_control_threshold);
                            sg_ready_on = true;
                        } else {
                            extended_logging("Price is above threshold. Average price: %dmct, current price: %dmct, threshold: %d%%.", price_average.data, price_current.data, summer_dynamic_price_control_threshold);
                            sg_ready_on = false;
                        }
                    }
                }

                if (summer_pv_excess_control_active) {
                    const int watt_current = 0; // meters.get_current_pv_excess();
                    if (watt_current > summer_pv_excess_control_threshold) {
                        extended_logging("Current PV excess is above threshold. Current PV excess: %dW, threshold: %dW.", watt_current, summer_pv_excess_control_threshold);
                        sg_ready_on = sg_ready_on || true;
                    }
                }
            }
        }
    }

    if (sg_ready_on) {
        extended_logging("Heating decision: Turning on SG Ready output 1.");
        // energy_manager.set_sg_ready_on(SG_READY_OUTPUT1);
    } else {
        extended_logging("Heating decision: Turning off SG Ready output 1.");
        // energy_manager.set_sg_ready_off(SG_READY_OUTPUT1);
    }

    // TODO: Only when sg ready above actually changed
    last_sg_ready_change = timestamp_minutes();
}