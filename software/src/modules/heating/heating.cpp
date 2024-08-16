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

void Heating::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"minimum_control_holding_time", Config::Uint(15, 0, 60)},
        {"winter_start_day", Config::Uint(1, 1, 31)},
        {"winter_start_month", Config::Uint(11, 1, 12)},
        {"winter_end_day", Config::Uint(15, 1, 31)},
        {"winter_end_month", Config::Uint(3, 1, 12)},
        {"winter_dynamic_price_control_active", Config::Bool(false)},
        {"winter_dynamic_price_control_threshold", Config::Uint(80, 0, 100)},
        {"winter_pv_excess_control_active", Config::Bool(false)},
        {"winter_pv_excess_control_threshold", Config::Uint(0)},
        {"summer_block_time1_active", Config::Bool(false)},
        {"summer_block_time2_active", Config::Bool(false)},
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
    // Only update if clock is synced. Heating control depends on time of day.
    struct timeval tv_now;
    if (!clock_synced(&tv_now)) {
        return;
    }

    // Get values from config
    const uint8_t minimum_control_holding_time = config.get("minimum_control_holding_time")->asUint();
    if(timestamp_minutes() < last_sg_ready_change + minimum_control_holding_time) {
        logger.printfln("Minimum control holding time not reached.");
        return;
    }

    const uint8_t winter_start_day                       = config.get("winter_start_day")->asUint();
    const uint8_t winter_start_month                     = config.get("winter_start_month")->asUint();
    const uint8_t winter_end_day                         = config.get("winter_end_day")->asUint();
    const uint8_t winter_end_month                       = config.get("winter_end_month")->asUint();
    const bool winter_dynamic_price_control_active       = config.get("winter_dynamic_price_control_active")->asBool();
    const uint8_t winter_dynamic_price_control_threshold = config.get("winter_dynamic_price_control_threshold")->asUint();
    const bool winter_pv_excess_control_active           = config.get("winter_pv_excess_control_active")->asBool();
    const uint8_t winter_pv_excess_control_threshold     = config.get("winter_pv_excess_control_threshold")->asUint();
    const bool summer_block_time1_active                 = config.get("summer_block_time1_active")->asBool();
    const bool summer_block_time2_active                 = config.get("summer_block_time2_active")->asBool();
    const bool summer_yield_forecast_active              = config.get("summer_yield_forecast_active")->asBool();
    const uint8_t summer_yield_forecast_threshold        = config.get("summer_yield_forecast_threshold")->asUint();
    const bool summer_dynamic_price_control_active       = config.get("summer_dynamic_price_control_active")->asBool();
    const uint8_t summer_dynamic_price_control_threshold = config.get("summer_dynamic_price_control_threshold")->asUint();
    const bool summer_pv_excess_control_active           = config.get("summer_pv_excess_control_active")->asBool();
    const uint8_t summer_pv_excess_control_threshold     = config.get("summer_pv_excess_control_threshold")->asUint();
    const bool p14enwg_active                            = config.get("p14enwg_active")->asBool();
    const uint8_t p14enwg_input                          = config.get("p14enwg_input")->asUint();
    const uint8_t p14enwg_active_type                    = config.get("p14enwg_active_type")->asUint();

    const time_t now              = time(NULL);
    const struct tm *current_time = localtime(&now);
    const int current_month       = current_time->tm_mon + 1;
    const int current_day         = current_time->tm_mday;

    const bool is_winter = ((current_month == winter_start_month) && (current_day >= winter_start_day )) ||
                           ((current_month == winter_end_month  ) && (current_day <= winter_end_day   )) ||
                           ((current_month > winter_start_month ) && (current_month < winter_end_month));

    bool sg_ready_on = false;

    if (is_winter) { // Winter
        if (!winter_dynamic_price_control_active && !winter_pv_excess_control_active) {
            logger.printfln("It is winter but no winter control active.");
        } else {
            if (winter_dynamic_price_control_active) {
                const float average_price = 0; //day_ahead_prices.get_average_price_today();
                const float price         = 0; //day_ahead_prices.get_price_now();

                if (price < average_price * winter_dynamic_price_control_threshold / 100.0) {
                    logger.printfln("Price is below threshold. Turning on SG ready.");
                    sg_ready_on = true;
                } else {
                    logger.printfln("Price is above threshold. Turning off SG ready.");
                    sg_ready_on = false;
                }
            }

            if (winter_pv_excess_control_active) {
                const int watt_expected = 0; // meters.get_expected_pv_excess();
                if (watt_expected > winter_pv_excess_control_threshold) {
                    logger.printfln("Expected PV excess is above threshold. Turning on SG ready.");
                    sg_ready_on = sg_ready_on || true;
                }
            }
        }
    } else { // Summer
        bool blocked = false;
        if (summer_block_time1_active) {
            // Check if we are in the first block time
        }

        if (summer_block_time2_active) {
            // Check if we are in the second block time
        }

        // If we are in block time and px excess control is active,
        // we check the expected px excess and unblock if it is below the threshold.
        if (blocked && summer_yield_forecast_active) {
            const int watt_expected = 0; // solar_forecast.get_expected_pv_excess();
            if (watt_expected < summer_yield_forecast_threshold) {
                logger.printfln("Expected PV yield is below threshold.");
                blocked = false;
            } else {
                logger.printfln("Expected PV yield is above threshold.");
            }
        }

        if (blocked) {
            logger.printfln("We are in a block time. Turning off SG ready.");
            sg_ready_on = false;
        } else {
            if (!summer_dynamic_price_control_active && !summer_pv_excess_control_active) {
                logger.printfln("It is summer but no summer control active.");
            } else {
                if (summer_dynamic_price_control_active) {
                    const float average_price = 0; //day_ahead_prices.get_average_price_today();
                    const float price         = 0; //day_ahead_prices.get_price_now();

                    if (price < average_price * summer_dynamic_price_control_threshold / 100.0) {
                        logger.printfln("Price is below threshold. Turning on SG ready.");
                        sg_ready_on = true;
                    } else {
                        logger.printfln("Price is above threshold. Turning off SG ready.");
                        sg_ready_on = false;
                    }
                }

                if (summer_pv_excess_control_active) {
                    const int watt_expected = 0; // meters.get_expected_pv_excess();
                    if (watt_expected > summer_pv_excess_control_threshold) {
                        logger.printfln("Expected PV excess is above threshold. Turning on SG ready.");
                        sg_ready_on = sg_ready_on || true;
                    }
                }
            }
        }
    }

    if (sg_ready_on) {
        logger.printfln("SG ready output1 on.");
        // energy_manager.set_sg_ready_on(SG_READY_OUTPUT1);
    } else {
        logger.printfln("SG ready output1 off.");
        // energy_manager.set_sg_ready_off(SG_READY_OUTPUT1);
    }

    // TODO: Only when sg ready above actually changed
    last_sg_ready_change = timestamp_minutes();

    bool p14enwg_on = false;
    if(p14enwg_active) {
        bool input_value = false; // energy_manager.get_input(p14enwg_input);
        if (p14enwg_active_type == 0 && input_value) {
            p14enwg_on = true;
        } else if(p14enwg_active_type == 1 && !input_value) {
            p14enwg_on = true;
        }

        if(p14enwg_on) {
            logger.printfln("§14 EnWG blocks heating.");
            // energy_manager.set_sg_ready_on(SG_READY_OUTPUT0);
        } else {
            logger.printfln("§14 EnWG does not block heating.");
            // energy_manager.set_sg_ready_off(SG_READY_OUTPUT0);
        }
    }
}