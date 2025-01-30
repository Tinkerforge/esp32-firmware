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
#include <TFJson.h>
#include <mbedtls/base64.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "modules/charge_manager/charge_manager_private.h"

#define ECO_EXTENDED_LOGGING 1

#define extended_logging(fmt, ...) \
    do { \
        if (ECO_EXTENDED_LOGGING) { \
            logger.tracefln(this->trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
        } \
    } while (0)

void Eco::pre_setup()
{
    this->trace_buffer_index = logger.alloc_trace_buffer("eco", 1 << 20);

    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"mode_after", Config::Uint(3, 0, 3)},
        {"park_time", Config::Bool(false)},
        {"park_time_duration", Config::Uint(8)}, // in hours
        {"charge_below", Config::Bool(false)},
        {"charge_below_threshold", Config::Int32(0)}, // in ct
        {"block_above", Config::Bool(false)},
        {"block_above_threshold", Config::Int32(20)}, // in ct
        {"yield_forecast", Config::Bool(false)},
        {"yield_forecast_threshold", Config::Uint(0)} // in kWh/day
    }), [this](Config &update, ConfigSource source) -> String {
        task_scheduler.scheduleOnce([this]() {
            this->update();
        });
        return "";
    }};

    charge_plan = Config::Object({
        {"enable",Config::Bool(false)},
        {"departure", Config::Enum(Departure::Tomorrow)},
        {"time", Config::Uint(8*60, 0, 24*60)}, // localtime in minutes since 00:00
        {"amount", Config::Uint(4)}  // h or kWh depending on configuration (currently only h supported)
    });

    state_chargers_prototype = Config::Object({
        {"start", Config::Uint(0)}, // Start of charge (minutes since epoch)
        {"amount", Config::Uint(0)}, // Amount of charge since start (currently in minutes)
        {"chart", Config::Str("", 0, 33)} // Base64 encoded chart data
    });

    state = Config::Object({
        {"last_save", Config::Uint(0)},
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

    const size_t controlled_chargers = charge_manager.get_charger_count();
    for (size_t i = 0; i < controlled_chargers; i++) {
        last_seen_plug_in[i] = 0_us;
        state.get("chargers")->add();
    }

    std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);

    // Wait for clock to be synced and day ahead prices to be available to initialize chart data
    task_scheduler.scheduleWhenClockSynced([this]() {
        task_scheduler.scheduleWithFixedDelay([this]() {
            if (day_ahead_prices.prices_sorted_available) {
                bool cheap_hours[48*4] = {false};
                set_chargers_state_chart_data(255, cheap_hours, 0);
                task_scheduler.cancel(task_scheduler.currentTaskId());
            }
        }, 1_s);
    });

    initialized = true;
}

void Eco::register_urls()
{
    api.addPersistentConfig("eco/config", &config);
    api.addState("eco/state",             &state);

    api.addState("eco/charge_plan", &charge_plan);
    api.addCommand("eco/charge_plan_update", &charge_plan, {}, [this](String &/*errmsg*/) {
        state.get("last_save")->updateUint(rtc.timestamp_minutes());
        update();
    }, false);

    server.on("/eco/chart", HTTP_PUT, [this](WebServerRequest request) {
        timeval tv;
        if (!rtc.clock_synced(&tv)) {
            return request.send(503, "text/plain", "Clock not yet synced");
        }

        auto buf = heap_alloc_array<char>(1024);
        if (request.contentLength() > 1024) {
            return request.send(413);
        }

        auto received = request.receive(buf.get(), 1024);
        if (received < 0) {
            return request.send(500, "text/plain", "Failed to receive request payload");
        }

        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, buf.get(), 1024);
        if (error) {
            String errorString = String("Failed to deserialize string: ") + error.c_str();
            return request.send(400, "text/plain", errorString.c_str());
        }

        if (!doc.containsKey("time")) {
            return request.send(400, "text/plain", "Missing 'time' field");
        }

        if (!doc.containsKey("amount")) {
            return request.send(400, "text/plain", "Missing 'amount' field");
        }

        if (!doc.containsKey("departure")) {
            return request.send(400, "text/plain", "Missing 'departure' field");
        }

        if (!doc.containsKey("current_time")) {
            return request.send(400, "text/plain", "Missing 'current_time' field");
        }

        if (doc["departure"].as<uint32_t>() > static_cast<std::underlying_type<Departure>::type>(Departure::Daily)) {
            return request.send(400, "text/plain", "Invalid 'departure' field");
        }

        uint32_t current_time_1m = doc["current_time"].as<uint32_t>();
        uint32_t amount_1m       = doc["amount"].as<uint32_t>()*60;
        auto end_time_1m = get_end_time_1m(
            static_cast<Departure>(doc["departure"].as<uint32_t>()),
            amount_1m,
            doc["time"].as<uint32_t>(),
            current_time_1m*60
        );

        if (end_time_1m.first != 0) {
            return request.send(400, "text/plain", "Charge plan not available");
        }

        uint32_t duration_remaining_1m = end_time_1m.second - current_time_1m;
        if (config.get("park_time")->asBool()) {
            const uint32_t park_time_duration_1m = config.get("park_time_duration")->asUint()*60;
            duration_remaining_1m = std::min(park_time_duration_1m, duration_remaining_1m);
        }
        const uint32_t duration_15m   = (duration_remaining_1m + 14)/15; // Round up to 15 minutes
        const uint32_t duration_uint8 = (duration_15m + 7)/8;
        const uint32_t amount_15m     = amount_1m/15;

        bool cheap_prices[duration_15m];
        char cheap_prices_bin[duration_uint8] = {0};
        day_ahead_prices.get_cheap_15m(current_time_1m, duration_15m, amount_15m, cheap_prices);
        for (size_t i = 0; i < duration_15m; i++) {
            cheap_prices_bin[i/8] |= (cheap_prices[i] ? 1 : 0) << (i % 8);
        }
        return request.send(200, "application/octet-stream", cheap_prices_bin, duration_uint8);
    });

    task_scheduler.scheduleWallClock([this]() {
        this->update();
    }, 1_m, 0_ms, true);
}


std::pair<uint8_t, uint32_t> Eco::get_end_time_1m(const Departure departure, const uint32_t desired_amount_1m, const uint32_t time_1m, const time_t save_time_1s)
{
    // Currently we assume that the amount is in hours, later we may add support for kWh
    const uint32_t save_time_midnight_1m = get_localtime_midnight_in_utc(save_time_1s) / 60;

    time_t midnight_1s;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight_1s)) {
        return std::make_pair(1, 0);
    }
    const uint32_t today_midnight_1m  = midnight_1s / 60;

    const uint32_t add_1m             = (((departure == Departure::Today) || (departure == Departure::Daily)) ? 0 : 24*60) + time_1m;
          uint32_t end_time_1m        = (departure == Departure::Daily) ? (today_midnight_1m + add_1m) : (save_time_midnight_1m + add_1m);
    const uint32_t current_time_1m    = rtc.timestamp_minutes();

    if (current_time_1m >= end_time_1m) {
        // If the current time is after the planned charge ending time
        // and the departure is set to "Daily" or "Tomorrow" we disable the charge plan.
        // TODO: If we have not reached the desired charge amount at this point,
        //       we could try to charge until the amount is reached or the car is disconnected.
        if ((departure == Departure::Today) || (departure == Departure::Tomorrow)) {
            return std::make_pair(2, end_time_1m);
        // If the current time is after the planned charge ending time
        // and the departure is set to "Daily" we increase the end time by 24 hours.
        // For daily departure the new period under consideration will restart
        // immediately after the time is reached.
        } else if (departure == Departure::Daily) {
            end_time_1m += 24*60;
        }
    }

    return std::make_pair(0, end_time_1m);
}

void Eco::set_chargers_state_chart_data(const uint8_t charger_id, bool *chart, uint8_t chart_length)
{
    // If chart is nullptr or chart_length is 0, we create a default chart with the current configuration
    if((chart_length == 0) && (chart != nullptr)) {
        const uint32_t current_time_1m = rtc.timestamp_minutes();
        const uint32_t amount_1m       = charge_plan.get("amount")->asUint()*60;
        auto end_time_1m = get_end_time_1m(
            charge_plan.get("departure")->asEnum<Departure>(),
            amount_1m,
            charge_plan.get("time")->asUint(),
            ((time_t)current_time_1m)*60
        );
        if (end_time_1m.first == 0) {
            uint32_t duration_remaining_1m = end_time_1m.second - current_time_1m;
            if (config.get("park_time")->asBool()) {
                const uint32_t park_time_duration_1m = config.get("park_time_duration")->asUint()*60;
                duration_remaining_1m = std::min(park_time_duration_1m, duration_remaining_1m);
            }
            const uint32_t duration_15m = (duration_remaining_1m + 14)/15; // Round up to 15 minutes
            const uint32_t amount_15m   = amount_1m/15;
            day_ahead_prices.get_cheap_15m(current_time_1m, duration_15m, amount_15m, chart);

            chart_length = duration_15m;
        }
    }

    const size_t chart_bin_length    = (chart_length + 7)/8;
    const size_t chart_base64_length = 4 + 4*chart_bin_length/3;

    char chart_base64[chart_base64_length + 1] = {0};
    if ((chart != nullptr) && (chart_length != 0)) {
        char chart_bin[chart_bin_length] = {0};

        for (size_t i = 0; i < chart_length; i++) {
            chart_bin[i/8] |= (chart[i] ? 1 : 0) << (i % 8);
        }

        size_t chart_base64_written = 0;
        mbedtls_base64_encode((unsigned char *)chart_base64, chart_base64_length, &chart_base64_written, (const unsigned char *)chart_bin, chart_bin_length);
    }

    if(charger_id == 255) {
        const size_t controlled_chargers = charge_manager.get_charger_count();
        for (size_t i = 0; i < controlled_chargers; i++) {
            state.get("chargers")->get(i)->get("chart")->updateString(chart_base64);
        }
    } else {
        state.get("chargers")->get(charger_id)->get("chart")->updateString(chart_base64);
    }
}

void Eco::update()
{
    // 48 hours with 15 minute resolution is the theortical maximum
    bool cheap_hours[48*4] = {false};

    // Update eco charger state once per minute, independent of the eco charge decision
    for (uint8_t charger_id = 0; charger_id < state.get("chargers")->count(); charger_id++) {
        auto *charger_state = charge_manager.get_charger_state(charger_id);
        if (charger_state == nullptr) {
            state.get("chargers")->get(charger_id)->get("start")->updateUint(0);
            state.get("chargers")->get(charger_id)->get("amount")->updateUint(0);
        } else {
            const uint32_t minutes_in_state_c = charger_state->time_in_state_c.to<minutes_t>().as<uint32_t>();
            state.get("chargers")->get(charger_id)->get("amount")->updateUint(minutes_in_state_c);
            extended_logging("Charger %d: Update minutes in state C to %d", charger_id, minutes_in_state_c);

            if (charger_state->last_plug_in == 0_us) {
                state.get("chargers")->get(charger_id)->get("start")->updateUint(0);
                state.get("chargers")->get(charger_id)->get("amount")->updateUint(0);
                extended_logging("Charger %d: Update amount and start time to 0", charger_id);

            // Only update "start" when there is a transition from one car to the next.
            // Otherwise the start time may "jitter" for a given charging-session since
            // the cpu time and the rtc time may not run completely in sync.
            } else if (charger_state->last_plug_in != last_seen_plug_in[charger_id]) {
                last_seen_plug_in[charger_id] = charger_state->last_plug_in;

                const micros_t time_from_now_to_plug_in = now_us() - charger_state->last_plug_in;
                const uint32_t epoch_to_plug_in_minutes = rtc.timestamp_minutes() - time_from_now_to_plug_in.to<minutes_t>().as<uint32_t>();
                state.get("chargers")->get(charger_id)->get("start")->updateUint(epoch_to_plug_in_minutes);
                extended_logging("Charger %d: Update start time to %d", charger_id, epoch_to_plug_in_minutes);
            }
        }
    }

    // If we don't yet have day ahead prices, we can't make a decision
    int32_t current_price;
    if (!day_ahead_prices.get_current_price_net().try_unwrap(&current_price)) {
        std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);
        set_chargers_state_chart_data(255, cheap_hours, 0);
        extended_logging("Charger all: No current price available -> Normal");
        return;
    }

    // Check for price below "charge below" threshold
    if (config.get("charge_below")->asBool()) {
        const int32_t charge_below = config.get("charge_below_threshold")->asInt()*1000; // *1000 since the current price is in ct/1000
        if (current_price < charge_below) {
            std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Fast);
            set_chargers_state_chart_data(255, cheap_hours, 0);
            extended_logging("Charger all: Current price (%d) below threshold (%d)-> Fast", current_price, charge_below);
            return;
        }
    }

    // Check for price above "block above" threshold
    if (config.get("block_above")->asBool()) {
        const int32_t block_above = config.get("block_above_threshold")->asInt()*1000; // *1000 since the current price is in ct/1000
        if (current_price > block_above) {
            std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);
            set_chargers_state_chart_data(255, cheap_hours, 0);
            extended_logging("Charger all: Current price (%d) above threshold (%d)-> Normal", current_price, block_above);
            return;
        }
    }

    if (config.get("enable")->asBool() && charge_plan.get("enable")->asBool()) {
        const uint32_t desired_amount_1m = charge_plan.get("amount")->asUint()*60;
        const uint32_t current_time_1m   = rtc.timestamp_minutes();

        auto end_time_1m = get_end_time_1m(
            charge_plan.get("departure")->asEnum<Departure>(),
            desired_amount_1m,
            charge_plan.get("time")->asUint(),
            state.get("last_save")->asUint()*60
        );

        if (end_time_1m.first == 1) {
            std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);
            set_chargers_state_chart_data(255, cheap_hours, 0);
            extended_logging("Charger all: Midnight not available -> Normal");
            return;
        }

        if (end_time_1m.first == 2) {
            disable_charge_plan();
            std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);
            set_chargers_state_chart_data(255, cheap_hours, 0);
            extended_logging("Charger all: Current time (%dm) after planned charge ending time (%dm) -> Normal", current_time_1m, end_time_1m.second);
            return;
        }

        uint32_t duration_remaining_1m = end_time_1m.second - current_time_1m;

        // Limit duration_remaining to park_time_duration (if enabled)
        const uint32_t park_time_duration_1m = config.get("park_time_duration")->asUint()*60;

        for (uint8_t charger_id = 0; charger_id < state.get("chargers")->count(); charger_id++) {
            const uint32_t charged_amount_1m = state.get("chargers")->get(charger_id)->get("amount")->asUint();
            const uint32_t start_time_1m     = state.get("chargers")->get(charger_id)->get("start")->asUint();

            // Check if car is charging on this charger
            if (start_time_1m == 0) {
                charge_decision[charger_id] = ChargeDecision::Normal;
                set_chargers_state_chart_data(charger_id, cheap_hours, 0);
                extended_logging("Charger %d: Car not charging (start_time = 0) -> Normal", charger_id);
                continue;
            }

            // If the desired amount of charge is reached, we are done with fast charging for this car.
            if (desired_amount_1m <= charged_amount_1m) {
                charge_decision[charger_id] = ChargeDecision::Normal;
                set_chargers_state_chart_data(charger_id, cheap_hours, 0);
                extended_logging("Charger %d: Desired charge amount reached (%dm <= %dm) -> Normal", charger_id, desired_amount_1m, charged_amount_1m);
                continue;
            }

            // Check if pv yield forecast is above threshold (we don't want to fast charge if we expect high solar yield)
            if (config.get("yield_forecast")->asBool()) {
                const uint32_t kwh_threshold = config.get("yield_forecast_threshold")->asUint();
                if (kwh_threshold > 0) {
                    auto wh_expected = solar_forecast.get_wh_range(start_time_1m, end_time_1m.second);
                    if (wh_expected.is_none()) {
                        charge_decision[charger_id] = ChargeDecision::Normal;
                        set_chargers_state_chart_data(charger_id, cheap_hours, 0);
                        extended_logging("Charger %d: Expected PV yield not available. Ignoring yield forecast. -> Normal", charger_id);
                        continue;
                    }

                    const uint32_t kwh_expected = wh_expected.unwrap()/1000;
                    if (kwh_expected > kwh_threshold) {
                        charge_decision[charger_id] = ChargeDecision::Normal;
                        set_chargers_state_chart_data(charger_id, cheap_hours, 0);
                        extended_logging("Charger %d: Expected PV yield %d kWh is above threshold of %d kWh. -> Normal", charger_id, kwh_expected, kwh_threshold);
                        continue;
                    }
                }
            }

            // Check time from start to end and limit to park_time_duration
            if (config.get("park_time")->asBool()) {
                const uint32_t current_park_time = current_time_1m - start_time_1m;
                duration_remaining_1m = std::min(duration_remaining_1m, park_time_duration_1m - current_park_time);
            }

            // Check if the current day ahead price slot is cheap
            const uint32_t amount_remaining_15m   = (desired_amount_1m/15) - (charged_amount_1m/15); // Round down to 15 minutes
            const uint32_t duration_remaining_15m = (duration_remaining_1m + 14)/15;                 // Round up to 15 minutes

            const bool ret = day_ahead_prices.get_cheap_15m(current_time_1m, duration_remaining_15m, amount_remaining_15m, cheap_hours);
            if(ret && cheap_hours[0]) {
                charge_decision[charger_id] = ChargeDecision::Fast;
            } else {
                charge_decision[charger_id] = ChargeDecision::Normal;
            }
            set_chargers_state_chart_data(charger_id, cheap_hours, duration_remaining_15m);
            extended_logging("Charger %d: Current price (%d) is %s -> %s [current_time %dm, duration_remaining %dm, desired_amount %dm, charged_amount %dm]", charger_id, current_price, (charge_decision[charger_id] == ChargeDecision::Fast) ? "cheap" : "expensive", (charge_decision[charger_id] == ChargeDecision::Fast) ? "Fast" : "Normal", current_time_1m, duration_remaining_1m, desired_amount_1m, charged_amount_1m);
        }
    } else {
        std::fill_n(charge_decision, MAX_CONTROLLED_CHARGERS, ChargeDecision::Normal);
        set_chargers_state_chart_data(255, cheap_hours, 0);
        extended_logging("Charger all: Eco or charge plan disabled -> Normal");
    }
}

void Eco::disable_charge_plan()
{
    charge_plan.get("enable")->updateBool(false);
}

Eco::ChargeDecision Eco::get_charge_decision(const uint8_t charger_id)
{
    if (charger_id >= state.get("chargers")->count()) {
        return ChargeDecision::Normal;
    }

    return charge_decision[charger_id];
}
