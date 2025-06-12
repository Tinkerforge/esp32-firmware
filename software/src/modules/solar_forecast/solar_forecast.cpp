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
#include "solar_forecast.h"

#include <math.h>
#include <time.h>
#include <lwip/inet.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

#include "gcc_warnings.h"

#if !BUILD_IS_SIGNED()
#define SOLAR_FORECAST_USE_TEST_DATA
#endif

#define CHECK_FOR_SF_TIMEOUT 15000
#define CHECK_INTERVAL 2*60*60*1000
#define CHECK_DELAY_MIN 2500

enum Resolution {
    RESOLUTION_15MIN,
    RESOLUTION_60MIN
};

#ifdef SOLAR_FORECAST_USE_TEST_DATA
// https://api.forecast.solar/estimate/51.8847388/8.626098/90/25/65
//static const char *test_data = "{\"result\":{\"watts\":{\"2024-08-06 05:56:31\":0,\"2024-08-06 06:15:00\":824,\"2024-08-06 07:00:00\":1873,\"2024-08-06 08:00:00\":3211,\"2024-08-06 09:00:00\":4460,\"2024-08-06 10:00:00\":6923,\"2024-08-06 11:00:00\":11874,\"2024-08-06 12:00:00\":15565,\"2024-08-06 13:00:00\":17623,\"2024-08-06 14:00:00\":17656,\"2024-08-06 15:00:00\":15257,\"2024-08-06 16:00:00\":12611,\"2024-08-06 17:00:00\":11749,\"2024-08-06 18:00:00\":8164,\"2024-08-06 19:00:00\":4580,\"2024-08-06 20:00:00\":2371,\"2024-08-06 21:06:10\":0,\"2024-08-07 05:58:06\":0,\"2024-08-07 06:15:00\":738,\"2024-08-07 07:00:00\":1719,\"2024-08-07 08:00:00\":2972,\"2024-08-07 09:00:00\":4245,\"2024-08-07 10:00:00\":6018,\"2024-08-07 11:00:00\":8399,\"2024-08-07 12:00:00\":10054,\"2024-08-07 13:00:00\":11140,\"2024-08-07 14:00:00\":11493,\"2024-08-07 15:00:00\":11163,\"2024-08-07 16:00:00\":10139,\"2024-08-07 17:00:00\":8738,\"2024-08-07 18:00:00\":7033,\"2024-08-07 19:00:00\":4549,\"2024-08-07 20:00:00\":2405,\"2024-08-07 21:04:21\":0},\"watt_hours_period\":{\"2024-08-06 05:56:31\":0,\"2024-08-06 06:15:00\":127,\"2024-08-06 07:00:00\":1011,\"2024-08-06 08:00:00\":2542,\"2024-08-06 09:00:00\":3836,\"2024-08-06 10:00:00\":5692,\"2024-08-06 11:00:00\":9399,\"2024-08-06 12:00:00\":13720,\"2024-08-06 13:00:00\":16594,\"2024-08-06 14:00:00\":17640,\"2024-08-06 15:00:00\":16457,\"2024-08-06 16:00:00\":13934,\"2024-08-06 17:00:00\":12180,\"2024-08-06 18:00:00\":9957,\"2024-08-06 19:00:00\":6372,\"2024-08-06 20:00:00\":3476,\"2024-08-06 21:06:10\":1307,\"2024-08-07 05:58:06\":0,\"2024-08-07 06:15:00\":104,\"2024-08-07 07:00:00\":921,\"2024-08-07 08:00:00\":2346,\"2024-08-07 09:00:00\":3609,\"2024-08-07 10:00:00\":5132,\"2024-08-07 11:00:00\":7209,\"2024-08-07 12:00:00\":9227,\"2024-08-07 13:00:00\":10597,\"2024-08-07 14:00:00\":11317,\"2024-08-07 15:00:00\":11328,\"2024-08-07 16:00:00\":10651,\"2024-08-07 17:00:00\":9439,\"2024-08-07 18:00:00\":7886,\"2024-08-07 19:00:00\":5791,\"2024-08-07 20:00:00\":3477,\"2024-08-07 21:04:21\":1290},\"watt_hours\":{\"2024-08-06 05:56:31\":0,\"2024-08-06 06:15:00\":127,\"2024-08-06 07:00:00\":1138,\"2024-08-06 08:00:00\":3680,\"2024-08-06 09:00:00\":7516,\"2024-08-06 10:00:00\":13208,\"2024-08-06 11:00:00\":22607,\"2024-08-06 12:00:00\":36327,\"2024-08-06 13:00:00\":52921,\"2024-08-06 14:00:00\":70561,\"2024-08-06 15:00:00\":87018,\"2024-08-06 16:00:00\":100952,\"2024-08-06 17:00:00\":113132,\"2024-08-06 18:00:00\":123089,\"2024-08-06 19:00:00\":129461,\"2024-08-06 20:00:00\":132937,\"2024-08-06 21:06:10\":134244,\"2024-08-07 05:58:06\":0,\"2024-08-07 06:15:00\":104,\"2024-08-07 07:00:00\":1025,\"2024-08-07 08:00:00\":3371,\"2024-08-07 09:00:00\":6980,\"2024-08-07 10:00:00\":12112,\"2024-08-07 11:00:00\":19321,\"2024-08-07 12:00:00\":28548,\"2024-08-07 13:00:00\":39145,\"2024-08-07 14:00:00\":50462,\"2024-08-07 15:00:00\":61790,\"2024-08-07 16:00:00\":72441,\"2024-08-07 17:00:00\":81880,\"2024-08-07 18:00:00\":89766,\"2024-08-07 19:00:00\":95557,\"2024-08-07 20:00:00\":99034,\"2024-08-07 21:04:21\":100324},\"watt_hours_day\":{\"2024-08-06\":134244,\"2024-08-07\":100324}},\"message\":{\"code\":0,\"type\":\"success\",\"text\":\"\",\"pid\":\"342877Z0\",\"info\":{\"latitude\":51.8847,\"longitude\":8.6261,\"distance\":0,\"place\":\"Helleforthstraße 18-20, 33758 Schloß Holte-Stukenbrock, Germany\",\"timezone\":\"Europe/Berlin\",\"time\":\"2024-08-06T16:55:13+02:00\",\"time_utc\":\"2024-08-06T14:55:13+00:00\"},\"ratelimit\":{\"zone\":\"IP 82.198.84.162\",\"period\":3600,\"limit\":12,\"remaining\":10}}}";
static const char *test_data = "{\"result\":{\"watts\":{\"2024-08-15 06:10:54\":0,\"2024-08-15 06:15:00\":878,\"2024-08-15 07:00:00\":1934,\"2024-08-15 08:00:00\":3213,\"2024-08-15 09:00:00\":4420,\"2024-08-15 10:00:00\":6475,\"2024-08-15 11:00:00\":9964,\"2024-08-15 12:00:00\":15072,\"2024-08-15 13:00:00\":21999,\"2024-08-15 14:00:00\":22271,\"2024-08-15 15:00:00\":18290,\"2024-08-15 16:00:00\":13245,\"2024-08-15 17:00:00\":10684,\"2024-08-15 18:00:00\":7207,\"2024-08-15 19:00:00\":4309,\"2024-08-15 20:00:00\":2460,\"2024-08-15 20:48:53\":0,\"2024-08-16 06:12:31\":0,\"2024-08-16 06:30:00\":578,\"2024-08-16 07:00:00\":1269,\"2024-08-16 08:00:00\":2444,\"2024-08-16 09:00:00\":4132,\"2024-08-16 10:00:00\":6614,\"2024-08-16 11:00:00\":9038,\"2024-08-16 12:00:00\":10617,\"2024-08-16 13:00:00\":11356,\"2024-08-16 14:00:00\":11947,\"2024-08-16 15:00:00\":12356,\"2024-08-16 16:00:00\":10965,\"2024-08-16 17:00:00\":8961,\"2024-08-16 18:00:00\":7029,\"2024-08-16 19:00:00\":4087,\"2024-08-16 20:00:00\":1845,\"2024-08-16 20:46:51\":0},\"watt_hours_period\":{\"2024-08-15 06:10:54\":0,\"2024-08-15 06:15:00\":30,\"2024-08-15 07:00:00\":1055,\"2024-08-15 08:00:00\":2574,\"2024-08-15 09:00:00\":3817,\"2024-08-15 10:00:00\":5448,\"2024-08-15 11:00:00\":8220,\"2024-08-15 12:00:00\":12518,\"2024-08-15 13:00:00\":18536,\"2024-08-15 14:00:00\":22135,\"2024-08-15 15:00:00\":20281,\"2024-08-15 16:00:00\":15768,\"2024-08-15 17:00:00\":11965,\"2024-08-15 18:00:00\":8946,\"2024-08-15 19:00:00\":5758,\"2024-08-15 20:00:00\":3385,\"2024-08-15 20:48:53\":1002,\"2024-08-16 06:12:31\":0,\"2024-08-16 06:30:00\":84,\"2024-08-16 07:00:00\":462,\"2024-08-16 08:00:00\":1857,\"2024-08-16 09:00:00\":3288,\"2024-08-16 10:00:00\":5373,\"2024-08-16 11:00:00\":7826,\"2024-08-16 12:00:00\":9828,\"2024-08-16 13:00:00\":10987,\"2024-08-16 14:00:00\":11652,\"2024-08-16 15:00:00\":12152,\"2024-08-16 16:00:00\":11661,\"2024-08-16 17:00:00\":9963,\"2024-08-16 18:00:00\":7995,\"2024-08-16 19:00:00\":5558,\"2024-08-16 20:00:00\":2966,\"2024-08-16 20:46:51\":720},\"watt_hours\":{\"2024-08-15 06:10:54\":0,\"2024-08-15 06:15:00\":30,\"2024-08-15 07:00:00\":1085,\"2024-08-15 08:00:00\":3659,\"2024-08-15 09:00:00\":7476,\"2024-08-15 10:00:00\":12924,\"2024-08-15 11:00:00\":21144,\"2024-08-15 12:00:00\":33662,\"2024-08-15 13:00:00\":52198,\"2024-08-15 14:00:00\":74333,\"2024-08-15 15:00:00\":94614,\"2024-08-15 16:00:00\":110382,\"2024-08-15 17:00:00\":122347,\"2024-08-15 18:00:00\":131293,\"2024-08-15 19:00:00\":137051,\"2024-08-15 20:00:00\":140436,\"2024-08-15 20:48:53\":141438,\"2024-08-16 06:12:31\":0,\"2024-08-16 06:30:00\":84,\"2024-08-16 07:00:00\":546,\"2024-08-16 08:00:00\":2403,\"2024-08-16 09:00:00\":5691,\"2024-08-16 10:00:00\":11064,\"2024-08-16 11:00:00\":18890,\"2024-08-16 12:00:00\":28718,\"2024-08-16 13:00:00\":39705,\"2024-08-16 14:00:00\":51357,\"2024-08-16 15:00:00\":63509,\"2024-08-16 16:00:00\":75170,\"2024-08-16 17:00:00\":85133,\"2024-08-16 18:00:00\":93128,\"2024-08-16 19:00:00\":98686,\"2024-08-16 20:00:00\":101652,\"2024-08-16 20:46:51\":102372},\"watt_hours_day\":{\"2024-08-15\":141438,\"2024-08-16\":102372}},\"message\":{\"code\":0,\"type\":\"success\",\"text\":\"\",\"pid\":\"wcx7nz26\",\"info\":{\"latitude\":51.8847,\"longitude\":8.6261,\"distance\":0,\"place\":\"Helleforthstraße 18-20, 33758 Schloß Holte-Stukenbrock, Germany\",\"timezone\":\"Europe/Berlin\",\"time\":\"2024-08-15T16:29:33+02:00\",\"time_utc\":\"2024-08-15T14:29:33+00:00\"},\"ratelimit\":{\"zone\":\"IP 82.198.84.162\",\"period\":3600,\"limit\":12,\"remaining\":6}}}";
static const size_t test_data_length = constexpr_strlen(test_data);
#endif

void SolarForecast::pre_setup()
{
    planes = new_array_psram_or_dram<SolarForecastPlane>(SOLAR_FORECAST_PLANES);

    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"api_url", Config::Str(BUILD_SOLAR_FORECAST_API_URL, 0, 64)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
    }), [this](Config &update, ConfigSource source) -> String {
        const String &api_url = update.get("api_url")->asString();

        if ((api_url.length() > 0) && !api_url.startsWith("https://")) {
            return "HTTPS required for Solar Forecast API URL";
        }

        this->next_update();
        return "";
    }};

    state = Config::Object({
        {"wh_today",           Config::Int32(-1)},
        {"wh_today_remaining", Config::Int32(-1)},
        {"wh_tomorrow",        Config::Int32(-1)},
        {"rate_limit",         Config::Int8(-1) },
        {"rate_remaining",     Config::Int8(-1) },
        {"next_api_call",      Config::Uint32(0)}, // unix timestamp in minutes
    });

    for (size_t plane_index = 0; plane_index < SOLAR_FORECAST_PLANES; plane_index++) {
        SolarForecastPlane &plane = planes[plane_index];
        plane.config = ConfigRoot{Config::Object({
            {"enable", Config::Bool(false)},
            {"name", Config::Str(String("#") + plane_index, 0, 16)},
            {"lat", Config::Int(0, -900000, 900000)},    // in 1/10000 degrees
            {"long", Config::Int(0, -1800000, 1800000)}, // in 1/10000 degrees
            {"dec", Config::Uint(0, 0, 90)},             // in degrees
            {"az", Config::Int(0, -180, 180)},           // in degrees
            {"wp", Config::Uint(0)}                      // in watt-peak
        }), [this, plane_index](Config &update, ConfigSource source) -> String {
            // If the config changes for a plane, we reset the state and forecast and trigger a new update
            SolarForecastPlane &p = this->planes[plane_index];
            p.state.get("next_check")->updateUint(0);
            p.state.get("last_check")->updateUint(0);
            p.state.get("last_sync")->updateUint(0);
            p.state.get("place")->updateString("Unknown");
            p.forecast.get("first_date")->updateUint(0);
            p.forecast.get("forecast")->removeAll();

            this->update_cached_wh_state();
            this->next_update();
            return "";
        }};

        plane.state = Config::Object({
            {"last_sync",  Config::Uint32(0)}, // unix timestamp in minutes
            {"last_check", Config::Uint32(0)}, // unix timestamp in minutes
            {"next_check", Config::Uint32(0)}, // unix timestamp in minutes
            {"place", Config::Str("Unknown", 0, 128)}
        });

        plane.forecast = Config::Object({
            {"first_date", Config::Uint32(0)}, // unix timestamp in minutes
            {"resolution", Config::Uint(RESOLUTION_60MIN, RESOLUTION_15MIN, RESOLUTION_60MIN)}, // currently only 60 minutes supported
            {"forecast",   Config::Array({}, Config::get_prototype_uint32_0(), 0, 49, Config::type_id<Config::ConfUint>())} // in watt hours, 48 hours + 1 for switch to DST
        });

        plane.index = plane_index;
    }

#ifdef DEBUG_FS_ENABLE
    debug_forecast_update = Config::Int32(0);
#endif
}

void SolarForecast::setup()
{
    api.restorePersistentConfig("solar_forecast/config", &config);

    for (size_t plane_index = 0; plane_index < SOLAR_FORECAST_PLANES; plane_index++) {
        SolarForecastPlane &plane = planes[plane_index];
        api.restorePersistentConfig(get_path(plane, SolarForecast::PathType::Config), &plane.config);
    }

    initialized = true;
}

void SolarForecast::register_urls()
{
    api.addPersistentConfig("solar_forecast/config", &config);
    api.addState("solar_forecast/state", &state);
    for (size_t plane_index = 0; plane_index < SOLAR_FORECAST_PLANES; plane_index++) {
        SolarForecastPlane &plane = planes[plane_index];
        api.addPersistentConfig(get_path(plane, SolarForecast::PathType::Config), &plane.config, {}, {"lat", "long"});
        api.addState(get_path(plane, SolarForecast::PathType::State),    &plane.state, {}, {"place"});
        api.addState(get_path(plane, SolarForecast::PathType::Forecast), &plane.forecast);
    }

    task_scheduler.scheduleWhenClockSynced([this]() {
        this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
            this->update();
        }, 100_ms, millis_t{CHECK_INTERVAL});

        task_scheduler.scheduleWallClock([this]() {
            this->update_cached_wh_state();
        }, 60_min, 0_ms, false);
    });

#ifdef DEBUG_FS_ENABLE
    api.addCommand("solar_forecast/debug_forecast_update", &debug_forecast_update, {}, [this](String &/*errmsg*/) {
        const int32_t wh_fake_forecast = debug_forecast_update.asInt();

        state.get("wh_today"          )->updateInt(wh_fake_forecast);
        state.get("wh_today_remaining")->updateInt(wh_fake_forecast);
        state.get("wh_tomorrow"       )->updateInt(wh_fake_forecast);
    }, false);
#endif
}

void SolarForecast::next_update() {
    // Don't re-schedule task if it hasn't been scheduled yet.
    // That is the case when the config is initially loaded during setup.
    if (!task_id) {
        return;
    }

    // Find smallest next check time.
    // But we wait for at least CHECK_DELAY_MIN ms before the next check.
    // This way we avoid hammering the server and also the ESP has some time in-between.
    int first_delay_ms = CHECK_INTERVAL;
    for (size_t plane_index = 0; plane_index < SOLAR_FORECAST_PLANES; plane_index++) {
        SolarForecastPlane &plane = planes[plane_index];
        if (plane.config.get("enable")->asBool()) {
            uint32_t next_check = plane.state.get("next_check")->asUint();
            if(next_check == 0) {
                first_delay_ms = CHECK_DELAY_MIN;
                break;
            } else {
                uint32_t current_time = rtc.timestamp_minutes();
                if(next_check < current_time) {
                    first_delay_ms = CHECK_DELAY_MIN;
                    break;
                }

                first_delay_ms = std::min(first_delay_ms, static_cast<int>(next_check - rtc.timestamp_minutes()) * 60 * 1000);
            }
        }
    }

    if(next_sync_forced != 0) {
        first_delay_ms = std::max(first_delay_ms, static_cast<int>(next_sync_forced - rtc.timestamp_minutes()) * 60 * 1000);
    }

    state.get("next_api_call")->updateUint(rtc.timestamp_minutes() + static_cast<uint32_t>(first_delay_ms) / (60 * 1000));

    // Cancel current task
    task_scheduler.cancel(task_id);

    // And schedule a new one that will run after the given delay,
    // but with the standard interval afterwards again
    this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
        this->update();
    }, millis_t{first_delay_ms}, millis_t{CHECK_INTERVAL});
}

void SolarForecast::handle_new_data()
{
    // Deserialize json received from API
    DynamicJsonDocument json_doc{SOLAR_FORECAST_MAX_ARDUINO_JSON_BUFFER_SIZE};
    DeserializationError error = deserializeJson(json_doc, json_buffer, json_buffer_position);
    if (error) {
        logger.printfln("Error during JSON deserialization: %s", error.c_str());
        logger.printfln("Next solar forecast API call will be in 30 minutes");
        next_sync_forced = rtc.timestamp_minutes() + 30;
        download_state = SF_DOWNLOAD_STATE_ERROR;
    } else {
        JsonObject js_message = json_doc["message"];
        JsonInteger code      = js_message["code"];
        if (code != 0) {
            JsonString text = js_message["text"];
            logger.printfln("Solar Forecast server returned error code %lld (%s)", code, text.c_str());
            if(code == 429) { // 429 = rate limit reached
                logger.printfln("Solar Forecast rate limit reached, next solar forecast API call will be in 2 hours");
                next_sync_forced = rtc.timestamp_minutes() + 120;
                state.get("rate_remaining")->updateInt(0);
            } else {
                // Wait 30 minutes after unknown error
                logger.printfln("Next solar forecast API call will be in 30 minutes");
                next_sync_forced = rtc.timestamp_minutes() + 30;
            }
            return;
        } else {
            JsonObject js_info      = js_message["info"];
            JsonObject js_ratelimit = js_message["ratelimit"];

            JsonString place         = js_info["place"];
            plane_current->state.get("place")->updateString(place.c_str());

            const int32_t  limit     = js_ratelimit["limit"    ].as<int32_t>();
            const int32_t  remaining = js_ratelimit["remaining"].as<int32_t>();
            const uint32_t period    = js_ratelimit["period"   ].as<uint32_t>();
            state.get("rate_limit")->updateInt(limit);
            state.get("rate_remaining")->updateInt(remaining);
            if (remaining == 0) {
                logger.printfln("Solar Forecast rate limit reached, next solar forecast API call will be in 2 hours");
                next_sync_forced = rtc.timestamp_minutes() + 120;
            } else {
                next_sync_forced = 0;
            }

            JsonObject js_result     = json_doc["result"];
            JsonObject js_wh_period  = js_result["watt_hours_period"];
            bool first = true;
            char day_start0 = 0;
            char day_start1 = 0;

            Config *forecast_cfg = static_cast<Config *>(plane_current->forecast.get("forecast"));

            size_t forecast_count = forecast_cfg->count();
            for (size_t i = 0; i < forecast_count; i++) {
                forecast_cfg->get(i)->updateUint(0);
            }
            for (size_t i = forecast_count; i < 48; i++) {
                forecast_cfg->add();
            }

            for (JsonPair pair : js_wh_period) {
                const char *key = pair.key().c_str();
                uint32_t value = pair.value().as<uint32_t>();

                // Calculate start time of the data from first day
                if (first) {
                    first = false;
                    day_start0 = key[8];
                    day_start1 = key[9];

                    // Parse date of first day
                    struct tm tm;
                    if (!strptime(key, "%Y-%m-%d", &tm)) {
                        logger.printfln("Cannot parse first day's date: '%s'", key);
                        continue;
                    }

                    // 00:00:00 of first day
                    tm.tm_hour = 0;
                    tm.tm_min  = 0;
                    tm.tm_sec  = 0;

                    // mktime needs tm_isdst to be set, but strptime doesn't set it.
                    // Use -1 to have mktime figure out if DST is in effect.
                    tm.tm_isdst = -1;

                    // Set first date as unix time in minutes
                    plane_current->forecast.get("first_date")->updateUint(static_cast<uint32_t>(mktime(&tm) / 60));
                }

                // Add 24 hours for second day
                const uint32_t index_add = ((day_start0 == key[8]) && (day_start1 == key[9])) ? 0 : 24;
                const uint32_t index = index_add + static_cast<uint32_t>(key[11] - '0')*10 + (key[12] - '0');
                if(index > 47) {
                    logger.printfln("Found impossible index: %lu (date %s)", index, key);
                    continue;
                }
                // We add up all kWh values that correspond to the same hour
                // The data is sometimes split up in two values for the same hour
                auto forecast_value = forecast_cfg->get(index);
                const uint32_t old_value = forecast_value->asUint();
                forecast_value->updateUint(value + old_value);
            }

            const uint32_t current_minutes = rtc.timestamp_minutes();
            plane_current->state.get("last_sync")->updateUint(current_minutes);
            plane_current->state.get("last_check")->updateUint(current_minutes);

            // For the next check we take the period given by the server and multiply it by two
            // to be a good "free tier user" and not hit the server too often.
            // Usually the period is 3600 seconds (one hour), so we will check every two hours.
            plane_current->state.get("next_check")->updateUint(current_minutes + period*2/60);

            update_cached_wh_state();
        }
    }
}

void SolarForecast::handle_cleanup()
{
    free_any(json_buffer);
    json_buffer = nullptr;
    json_buffer_position = 0;
}

void SolarForecast::retry_update(millis_t delay)
{
    // Cancel current task
    task_scheduler.cancel(task_id);

    // And schedule a new one that will run after the given delay,
    // but with the standard interval afterwards again
    this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
        this->update();
    }, delay, millis_t{CHECK_INTERVAL});
}

void SolarForecast::update()
{
    if (!config.get("enable")->asBool()) {
        return;
    }

    // Only update if network is connected and clock is synced
    struct timeval tv_now;
    if (!network.is_connected() || !rtc.clock_synced(&tv_now)) {
        retry_update(1_s);
        return;
    }

    if (download_state == SF_DOWNLOAD_STATE_PENDING) {
        return;
    }

    // Find plane that is due for update
    plane_current = nullptr;
    for (size_t plane_index = 0; plane_index < SOLAR_FORECAST_PLANES; plane_index++) {
        SolarForecastPlane &plane = planes[plane_index];
        if (plane.config.get("enable")->asBool()) {
            uint32_t next_check = plane.state.get("next_check")->asUint();
            if((next_check < rtc.timestamp_minutes() || next_check == 0)) {
                plane_current = &plane;
                break;
            }
        }
    }

    if(plane_current == nullptr) {
        // No plane due for update found
        return;
    }

    if (config.get("api_url")->asString().length() == 0) {
        logger.printfln("No solar forecast API server configured");
        download_state = SF_DOWNLOAD_STATE_ERROR;
        return;
    }

    if (json_buffer == nullptr) {
        json_buffer = static_cast<char *>(malloc_psram_or_dram(SOLAR_FORECAST_MAX_JSON_LENGTH * sizeof(char)));
    } else {
        logger.printfln("JSON Buffer was potentially not freed correctly");
    }
    json_buffer_position = 0;

#ifdef SOLAR_FORECAST_USE_TEST_DATA
    // Copy test data to temporary buffer
    logger.printfln("Using test data");
    memcpy(json_buffer + json_buffer_position, test_data, test_data_length);
    json_buffer_position += test_data_length;
    json_buffer[json_buffer_position] = '\0';

    handle_new_data();
    handle_cleanup();
    next_update();

    return;
#endif

    plane_current->state.get("last_check")->updateUint(rtc.timestamp_minutes());

    download_state = SF_DOWNLOAD_STATE_PENDING;
    https_client.download_async(get_api_url_with_path(*plane_current).c_str(), config.get("cert_id")->asInt(), [this](AsyncHTTPSClientEvent *event) {
        switch (event->type) {
        case AsyncHTTPSClientEventType::Error:
            switch (event->error) {
            case AsyncHTTPSClientError::NoHTTPSURL:
                logger.printfln("No HTTPS update URL");
                break;

            case AsyncHTTPSClientError::Busy:
                logger.printfln("HTTP client is busy");
                break;

            case AsyncHTTPSClientError::NoCert:
                logger.printfln("Certificate with ID %ld is not available", config.get("cert_id")->asInt());
                break;

            case AsyncHTTPSClientError::Timeout:
                logger.printfln("Forecast download timed out");
                break;

            case AsyncHTTPSClientError::ShortRead:
                logger.printfln("Solar forecast download ended prematurely");
                break;

            case AsyncHTTPSClientError::HTTPError: {
                char buf[212];
                translate_HTTPError_detailed(event->error_handle, buf, ARRAY_SIZE(buf), true);
                logger.printfln("Forecast download failed: %s", buf);
                break;
            }
            case AsyncHTTPSClientError::HTTPClientInitFailed:
                logger.printfln("Error while creating HTTP client");
                break;

            case AsyncHTTPSClientError::HTTPClientError:
                logger.printfln("Error while downloading solar forecast: %s (0x%lX)", esp_err_to_name(event->error_http_client), static_cast<uint32_t>(event->error_http_client));
                break;

            case AsyncHTTPSClientError::HTTPStatusError:
                logger.printfln("HTTP error while downloading solar forecast: %d", event->error_http_status);
                break;

            // use default to prevent warnings since we dont use a body, cookies or headers here
            case AsyncHTTPSClientError::HTTPClientSetCookieFailed:
            case AsyncHTTPSClientError::HTTPClientSetHeaderFailed:
            case AsyncHTTPSClientError::HTTPClientSetBodyFailed:
            default:
                logger.printfln("Uncovered error, this should never happen!");
                break;
            }

            if (event->error_http_status == 429) {
                // Wait for 120 minutes when rate limit is reached
                logger.printfln("Solar Forecast rate limit reached, next solar forecast API call will be in 2 hours");
                next_sync_forced = rtc.timestamp_minutes() + 120;
                state.get("rate_remaining")->updateInt(0);
            } else {
                // Wait 30 minutes after other errors
                logger.printfln("Next solar forecast API call will be in 30 minutes");
                next_sync_forced = rtc.timestamp_minutes() + 30;
            }

            download_state = SF_DOWNLOAD_STATE_ERROR;
            handle_cleanup();
            next_update();

            break;

        case AsyncHTTPSClientEventType::Data:
            if(json_buffer == nullptr) {
                logger.printfln("JSON Buffer was not allocated correctly before receiving data");
                next_sync_forced = rtc.timestamp_minutes() + 30;

                download_state = SF_DOWNLOAD_STATE_ERROR;
                handle_cleanup();
                next_update();
                break;
            }

            if(json_buffer_position + event->data_chunk_len >= SOLAR_FORECAST_MAX_JSON_LENGTH) {
                logger.printfln("JSON Buffer overflow");
                next_sync_forced = rtc.timestamp_minutes() + 30;

                download_state = SF_DOWNLOAD_STATE_ERROR;
                handle_cleanup();
                next_update();
                break;
            }

            memcpy(json_buffer + json_buffer_position, event->data_chunk, event->data_chunk_len);
            json_buffer_position += event->data_chunk_len;
            break;

        case AsyncHTTPSClientEventType::Aborted:
            if (download_state == SF_DOWNLOAD_STATE_PENDING) {
                logger.printfln("Update check aborted, next solar forecast API call will be in 30 minutes");
                next_sync_forced = rtc.timestamp_minutes() + 30;
                download_state = SF_DOWNLOAD_STATE_ABORTED;
            }
            handle_cleanup();
            next_update();

            break;

        case AsyncHTTPSClientEventType::Finished:
            if(json_buffer == nullptr) {
                logger.printfln("JSON Buffer was not allocated correctly before finishing");
                next_sync_forced = rtc.timestamp_minutes() + 30;

                download_state = SF_DOWNLOAD_STATE_ERROR;
                break;
            } else {
                json_buffer[json_buffer_position] = '\0';
                handle_new_data();
            }

            handle_cleanup();

            if (download_state == SF_DOWNLOAD_STATE_PENDING) {
                download_state = SF_DOWNLOAD_STATE_OK;
            }

            next_update();

            break;

        default:
            esp_system_abort("Invalid event");
            break;
        }
    });
}

// Create API path including user configuration
String SolarForecast::get_api_url_with_path(const SolarForecastPlane &plane)
{
    char buf[256];
    StringWriter sw(buf, ARRAY_SIZE(buf));

    const String &api_url = config.get("api_url")->asString();
    sw.puts(api_url.c_str(), static_cast<ssize_t>(api_url.length()));

    if (*(sw.getRemainingPtr() - 1) != '/') {
        sw.putc('/');
    }

    sw.printf("estimate/%.4f/%.4f/%lu/%li/%.3f",
              plane.config.get("lat" )->asInt()  / 10000.0,
              plane.config.get("long")->asInt()  / 10000.0,
              plane.config.get("dec" )->asUint(),
              plane.config.get("az"  )->asInt(),
              plane.config.get("wp"  )->asUint() /  1000.0);

    return String(buf, sw.getLength());
}

static const char *solar_forecast_path_postfixes[] = {"", "config", "state", "forecast"};
static_assert(ARRAY_SIZE(solar_forecast_path_postfixes) == static_cast<uint32_t>(SolarForecast::PathType::_max) + 1, "Path postfix length mismatch");
String SolarForecast::get_path(const SolarForecastPlane &plane, const SolarForecast::PathType path_type)
{
    char buf[64];
    StringWriter sw(buf, ARRAY_SIZE(buf));

    sw.printf("solar_forecast/planes/%zu/%s", plane.index, solar_forecast_path_postfixes[static_cast<uint32_t>(path_type)]);

    return String(buf, sw.getLength());
}

Option<uint32_t> SolarForecast::get_wh_range(const uint32_t start, const uint32_t end)
{
    if (start > end) {
        return {};
    }

    uint32_t wh    = 0;
    bool found_any_data = false;

    for (size_t plane_index = 0; plane_index < SOLAR_FORECAST_PLANES; plane_index++) {
        const SolarForecastPlane &plane = planes[plane_index];
        if (plane.config.get("enable")->asBool()) {
            const uint32_t first_date = plane.forecast.get("first_date")->asUint();

            if (!first_date) {
                // Plane enabled but no data yet
                continue;
            }

            const Config *forecast_cfg = static_cast<const Config *>(plane.forecast.get("forecast"));
            const uint32_t forecast_length = static_cast<uint32_t>(forecast_cfg->count());

            uint32_t i_start; // First included forecast index
            if (start <= first_date) {
                i_start = 0;
            } else {
                i_start = (start - first_date) / 60;
                if (i_start >= forecast_length) {
                    continue;
                }
            }

            uint32_t i_end; // Last included forecast index
            if (end < first_date) {
                continue;
            } else {
                i_end = (end - first_date) / 60;
                if (i_end >= forecast_length) {
                    i_end = forecast_length - 1;
                }
            }

            for (uint32_t i = i_start; i <= i_end; i++) {
                wh += forecast_cfg->get(i)->asUint();
            }

            found_any_data = true;
        }
    }

    // We assume that we have valid data for the day if
    // there is at least one data point today
    if (!found_any_data) {
        return {};
    }

    return wh;
}

Option<uint32_t> SolarForecast::get_wh_today()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight)) {
        return {};
    }

    const uint32_t start = static_cast<uint32_t>(midnight / 60);
    const uint32_t end   = start + 60*24 - 1;
    return get_wh_range(start, end);
}

Option<uint32_t> SolarForecast::get_wh_today_remaining()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight)) {
        return {};
    }

    const uint32_t start = rtc.timestamp_minutes();
    const uint32_t end   = static_cast<uint32_t>(midnight / 60 + 60*24 - 1);
    return get_wh_range(start, end);
}

Option<uint32_t> SolarForecast::get_wh_tomorrow()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight)) {
        return {};
    }

    const uint32_t start = static_cast<uint32_t>(midnight / 60 + 60*24);
    const uint32_t end   = start + 60*24 - 1;
    return get_wh_range(start, end);
}

int32_t SolarForecast::get_cached_wh_today()
{
    return state.get("wh_today")->asInt();
}

int32_t SolarForecast::get_cached_wh_today_remaining()
{
    return state.get("wh_today_remaining")->asInt();
}

int32_t SolarForecast::get_cached_wh_tomorrow()
{
    return state.get("wh_tomorrow")->asInt();
}

void SolarForecast::update_cached_wh_state()
{
    Option<uint32_t> wh_today = get_wh_today();
    Option<uint32_t> wh_today_remaining = get_wh_today_remaining();
    Option<uint32_t> wh_tomorrow = get_wh_tomorrow();

    state.get("wh_today"          )->updateInt(wh_today.is_some()           ? static_cast<int32_t>(wh_today.unwrap())           : -1);
    state.get("wh_today_remaining")->updateInt(wh_today_remaining.is_some() ? static_cast<int32_t>(wh_today_remaining.unwrap()) : -1);
    state.get("wh_tomorrow"       )->updateInt(wh_tomorrow.is_some()        ? static_cast<int32_t>(wh_tomorrow.unwrap())        : -1);
}
