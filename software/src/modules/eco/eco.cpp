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
#include "eco.h"

#include <time.h>
#include <type_traits>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"


void Eco::pre_setup()
{
    this->trace_buffer_index = logger.alloc_trace_buffer("eco", 1 << 20);

    config = ConfigRoot{Config::Object({
        {"charge_plan_active", Config::Bool(false)},
        {"mode_after_charge_plan", Config::Uint(3, 0, 3)},
        {"service_life_active", Config::Bool(false)},
        {"service_life", Config::Uint(8)},
        {"charge_below_active", Config::Bool(false)},
        {"charge_below", Config::Int32(0)}, // in ct
        {"block_above_active", Config::Bool(false)},
        {"block_above", Config::Int32(20)}, // in ct
        {"yield_forecast_active", Config::Bool(false)},
        {"yield_forecast", Config::Uint(0)} // in kWh/day
    }), [this](Config &update, ConfigSource source) -> String {
        task_scheduler.scheduleOnce([this]() {
            this->update();
        });
        return "";
    }};

    charge_plan = Config::Object({
        {"enabled",Config::Bool(false)},
        {"depature", Config::Enum(Depature::Tomorrow, Depature::Today, Depature::Daily)},
        {"time", Config::Int(8*60)}, // localtime in minutes since 00:00
        {"amount", Config::Uint(4)}  // h or kWh depending on configuration (currently only h supported)
    });
    charge_plan_update = charge_plan;

    // TODO: If we don't need any more state information per charger we can use a simple array here.
    state_chargers_prototype = Config::Object({
        {"amount", Config::Uint(0)} // Amount of charge since start (h or kWh depending on configuration)
    });

    state = Config::Object({
        {"last_charge_plan_save", Config::Uint(0)},
        {"chargers", Config::Array(
            {},
            &state_chargers_prototype,
            0, MAX_CONTROLLED_CHARGERS, Config::type_id<Config::ConfObject>()
        )}
    });
}

void Eco::setup()
{
    api.restorePersistentConfig("eco/config", &config);

    const size_t controlled_chargers = charge_manager.config.get("chargers")->count();
    for (size_t i = 0; i < controlled_chargers; i++) {
        state.get("chargers")->add();
    }

    std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);

    initialized = true;
}

void Eco::register_urls()
{
    api.addPersistentConfig("eco/config", &config);
    api.addState("eco/state",             &state);

    api.addState("eco/charge_plan", &charge_plan);
    api.addCommand("eco/charge_plan_update", &charge_plan_update, {}, [this](String &/*errmsg*/) {
        charge_plan = charge_plan_update;
        state.get("last_charge_plan_save")->updateUint(rtc.timestamp_minutes());
        update();
    }, false);

    task_scheduler.scheduleWallClock([this]() {
        this->update();
    }, 1_m, 0_ms, true);
}

void Eco::update()
{
    // If we don't yet have day ahead prices, we can't make a decision
    int32_t current_price;
    if (!day_ahead_prices.get_current_price_net().try_unwrap(&current_price)) {
        std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);
        return;
    }

    // Check for price below "charge below" threshold
    if (config.get("charge_below_active")->asBool()) {
        const int32_t charge_below = config.get("charge_below")->asInt()*1000; // *1000 since the current price is in ct/1000
        if (current_price < charge_below) {
            std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Fast);
            return;
        }
    }

    // Check for price above "block above" threshold
    if (config.get("block_above_active")->asBool()) {
        const int32_t block_above = config.get("block_above")->asInt()*1000; // *1000 since the current price is in ct/1000
        if (current_price > block_above) {
            std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);
            return;
        }
    }

    if (config.get("charge_plan_active")->asBool() && charge_plan.get("enabled")->asBool()) {
        // Currently we assume that the amount is in hours, later we may add support for kWh
        const uint32_t hours_desired      = charge_plan.get("amount")->asUint();
        const Depature depature           = charge_plan.get("depature")->asEnum<Depature>();

        const time_t   save_time          = state.get("last_charge_plan_save")->asUint()*60;
        const uint32_t save_time_midnight = get_localtime_midnight_in_utc(save_time) / 60;

        time_t midnight;
        if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight)) {
            std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);
            return;
        }
        const uint32_t today_midnight     = midnight / 60;

        const uint32_t time               = charge_plan.get("time")->asInt();
        const uint32_t minutes_add        = (((depature == Depature::Today) || (depature == Depature::Daily)) ? 0 : 24*60) + time;
              uint32_t end_time           = (depature == Depature::Daily) ? (today_midnight + minutes_add) : (save_time_midnight + minutes_add);
        const uint32_t current_time       = rtc.timestamp_minutes();

        if (current_time >= end_time) {
            // If the current time is after the planned charge ending time
            // and the depature is set to "Daily" or "Tomorrow" we disable the charge plan.
            // TODO: If we have not reached the desired charge amount at this point,
            //       we could try to charge until the amount is reached or the car is disconnected.
            if ((depature == Depature::Today) || (depature == Depature::Tomorrow)) {
                disable_charge_plan();
                std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);
                return;
            // If the current time is after the planned charge ending time
            // and the depature is set to "Daily" we increase the end time by 24 hours.
            // For daily depature the new period under consideration will restart
            // immediately after the time is reached.
            } else if (depature == Depature::Daily) {
                end_time += 24*60;
            }
        }

        const uint32_t duration_remaining = end_time - current_time;

        for (uint8_t charger_id = 0; charger_id < config.get("chargers")->count(); charger_id++) {
            // TODO: Check if charger is connected to a car?
            const uint32_t hours_charged = state.get("chargers")->get(charger_id)->get("amount")->asUint()/60; // assumes that amount is in minutes

            // If the desired amount of charge is reached, we are done with fast charging for this car.
            if (hours_desired <= hours_charged) {
                charge_decision[charger_id] = ChargeDecision::Normal;
                continue;
            }

            if (config.get("yield_forecast_active")->asBool()) {
                const uint32_t kwh_threshold = config.get("yield_forecast")->asUint();
                if (kwh_threshold > 0) {
                    const uint32_t kwh_expected = 0; // TODO: get expected yield in wh from solar forecast
                    if (kwh_expected > kwh_threshold) {
                        charge_decision[charger_id] = ChargeDecision::Normal;
                        continue;
                    }
                }
            }

            const uint32_t hours_remaining    = hours_desired - hours_charged;
            if (day_ahead_prices.is_start_time_cheap(current_time, duration_remaining, hours_remaining)) {
                charge_decision[charger_id] = ChargeDecision::Fast;
            } else {
                charge_decision[charger_id] = ChargeDecision::Normal;
            }
        }
    } else {
        std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);
    }
}

void Eco::disable_charge_plan()
{
    charge_plan.get("enabled")->updateBool(false);
}

Eco::ChargeDecision Eco::get_charge_decision(const uint8_t charger_id)
{
    if (charger_id >= charge_plan.get("chargers")->count()) {
        return ChargeDecision::Normal;
    }

    return charge_decision[charger_id];
}
