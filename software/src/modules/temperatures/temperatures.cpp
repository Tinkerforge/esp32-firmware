/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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
#include "temperatures.h"

#include <math.h>
#include <time.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "tools/string_builder.h"
#include "options.h"

static constexpr auto CHECK_INTERVAL = 6_h; // Update every 6 hours
static constexpr auto RETRY_INTERVAL = 10_min; // Retry after 10 minutes on error

void Temperatures::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"source", Config::Enum(TemperatureSource::WeatherService)},
        {"api_url", Config::Str(OPTIONS_TEMPERATURES_API_URL(), 0, 64)},
        {"lat", Config::Int32(0)},  // Latitude in 1/10000 degrees (e.g., 519035 = 51.9035°)
        {"lon", Config::Int32(0)},  // Longitude in 1/10000 degrees (e.g., 86720 = 8.6720°)
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
    }), [this](Config &update, ConfigSource source) -> String {
        const String &api_url = update.get("api_url")->asString();

        if ((api_url.length() > 0) && !api_url.startsWith("https://")) {
            return "HTTPS required for Temperatures API URL";
        }

        // If config changes, trigger a new update
        if ((update.get("enable")->asBool()                    != config.get("enable")->asBool()) ||
            (update.get("source")->asEnum<TemperatureSource>() != config.get("source")->asEnum<TemperatureSource>()) ||
            (update.get("api_url")->asString()                 != config.get("api_url")->asString()) ||
            (update.get("lat")->asInt()                        != config.get("lat")->asInt()) ||
            (update.get("lon")->asInt()                        != config.get("lon")->asInt()) ||
            (update.get("cert_id")->asInt()                    != config.get("cert_id")->asInt())) {
            state.get("last_sync")->updateUint(0);
            state.get("last_check")->updateUint(0);
            state.get("next_check")->updateUint(0);

            // Reset temperature data
            temperatures.get("first_date")->updateUint(0);
            temperatures.get("temperatures")->removeAll();
            today_min = INT16_MAX;
            today_max = INT16_MIN;
            today_avg = INT16_MAX;
            tomorrow_min = INT16_MAX;
            tomorrow_max = INT16_MIN;
            tomorrow_avg = INT16_MAX;

            if (boot_stage == BootStage::LOOP) {
                task_scheduler.scheduleOnce([this]() {
                    this->update();
                });
            }
        }

        return "";
    }};

    state = Config::Object({
        {"last_sync",  Config::Uint32(0)}, // unix timestamp in minutes
        {"last_check", Config::Uint32(0)}, // unix timestamp in minutes
        {"next_check", Config::Uint32(0)}, // unix timestamp in minutes
    });

    temperatures = Config::Object({
        {"first_date",    Config::Uint32(0)},                                         // UTC timestamp in minutes of local midnight
        {"temperatures",  Config::Array({}, Config::get_prototype_int16_0(), 0, 49)}, // 47-49 hourly temperatures in °C * 10
    });

    temperatures_update = Config::Object({
        {"first_date",    Config::Uint32(0)},                                         // UTC timestamp in minutes of local midnight
        {"temperatures",  Config::Array({}, Config::get_prototype_int16_0(), 0, 49)}, // 47-49 hourly temperatures in °C * 10
    });
}

void Temperatures::setup()
{
    api.restorePersistentConfig("temperatures/config", &config);

    initialized = true;
}

void Temperatures::register_urls()
{
    api.addPersistentConfig("temperatures/config", &config);
    api.addState("temperatures/state",             &state);
    api.addState("temperatures/temperatures",      &temperatures);

    api.addCommand("temperatures/temperatures_update", &temperatures_update, {}, [this](Language /*language*/, String &errmsg) {
        if (!config.get("enable")->asBool()) {
            errmsg = "Temperatures not enabled";
            return;
        }

        if (config.get("source")->asEnum<TemperatureSource>() != TemperatureSource::Push) {
            errmsg = "Temperatures not in push mode";
            return;
        }

        const uint32_t first_date = temperatures_update.get("first_date")->asUint();

        if (first_date == 0) {
            errmsg = "first_date must be non-zero";
            return;
        }

        auto src_arr = temperatures_update.get("temperatures");
        const size_t count = src_arr->count();

        if ((count < 47) || (count > 49)) {
            errmsg = "temperatures array must have 47-49 elements";
            return;
        }

        // Store first_date (already in minutes from caller)
        temperatures.get("first_date")->updateUint(first_date);

        // Copy temperatures array
        auto dst_arr = temperatures.get("temperatures");
        while (dst_arr->count() > count) dst_arr->removeLast();
        while (dst_arr->count() < count) dst_arr->add();

        for (size_t i = 0; i < count; i++) {
            dst_arr->get(i)->updateInt(src_arr->get(i)->asInt());
        }

        // Compute day stats from the new data
        compute_day_stats();

        const uint32_t current_minutes = rtc.timestamp_minutes();
        state.get("last_sync")->updateUint(current_minutes);
        state.get("last_check")->updateUint(current_minutes);
        state.get("next_check")->updateUint(0);
    }, true);

    task_scheduler.scheduleWhenClockSynced([this]() {
        this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
            this->update();
        }, CHECK_INTERVAL);
    });

    // Recompute day stats at midnight so today/tomorrow boundaries shift correctly
    task_scheduler.scheduleWallClock([this]() {
        this->compute_day_stats();
    }, 60_min, 0_ms, false);
}

void Temperatures::retry_update(millis_t delay)
{
    // Cancel current task
    task_scheduler.cancel(task_id);

    // And schedule a new one that will run after the given delay,
    // but with the standard interval afterwards again
    this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
        this->update();
    }, delay, CHECK_INTERVAL);
}

void Temperatures::update()
{
    if (!config.get("enable")->asBool()) {
        return;
    }

    if (config.get("source")->asEnum<TemperatureSource>() != TemperatureSource::WeatherService) {
        return;
    }

    // Only update if network is connected and clock is synced
    struct timeval tv_now;
    if (!network.is_connected() || !rtc.clock_synced(&tv_now)) {
        retry_update(1_s);
        return;
    }

    if (download_state == TEMPERATURES_DOWNLOAD_STATE_PENDING) {
        return;
    }

    download_state = TEMPERATURES_DOWNLOAD_STATE_PENDING;
    state.get("last_check")->updateUint(rtc.timestamp_minutes());

    if (config.get("api_url")->asString().length() == 0) {
        logger.printfln("No temperatures API server configured");
        download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
        state.get("next_check")->updateUint(rtc.timestamp_minutes() + (RETRY_INTERVAL / 1_min).as<uint32_t>());
        retry_update(RETRY_INTERVAL);
        return;
    }

    // Check if lat/lon are configured (non-zero)
    if (config.get("lat")->asInt() == 0 && config.get("lon")->asInt() == 0) {
        logger.printfln("Latitude and longitude not configured");
        download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
        state.get("next_check")->updateUint(rtc.timestamp_minutes() + (RETRY_INTERVAL / 1_min).as<uint32_t>());
        retry_update(RETRY_INTERVAL);
        return;
    }

    if(json_buffer == nullptr) {
        json_buffer = (char *)calloc_psram_or_dram(TEMPERATURES_MAX_JSON_LENGTH, sizeof(char));
    } else {
        logger.printfln("JSON Buffer was potentially not freed correctly");
    }
    json_buffer_position = 0;

    https_client.download_async(get_api_url_with_path().c_str(), config.get("cert_id")->asInt(), [this](AsyncHTTPSClientEvent *event) {
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
                logger.printfln("Certificate with ID %li is not available", config.get("cert_id")->asInt());
                break;

            case AsyncHTTPSClientError::Timeout:
                logger.printfln("Temperatures download timed out");
                break;

            case AsyncHTTPSClientError::ShortRead:
                logger.printfln("Temperatures download ended prematurely");
                break;

            case AsyncHTTPSClientError::HTTPError: {
                char buf[216];
                translate_HTTPError_detailed(event->error_handle, buf, ARRAY_SIZE(buf), true);
                logger.printfln("Temperatures download failed: %s", buf);
                break;
            }
            case AsyncHTTPSClientError::HTTPClientInitFailed:
                logger.printfln("Error while creating HTTP client");
                break;

            case AsyncHTTPSClientError::HTTPClientError:
                logger.printfln("Error while downloading temperatures: %s (0x%lX)", esp_err_to_name(event->error_http_client), static_cast<uint32_t>(event->error_http_client));
                break;

            case AsyncHTTPSClientError::HTTPStatusError:
                logger.printfln("HTTP error while downloading temperatures: %d", event->error_http_status);
                break;

            default:
                logger.printfln("Uncovered error, this should never happen!");
                break;
            }

            download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
            state.get("next_check")->updateUint(rtc.timestamp_minutes() + (RETRY_INTERVAL / 1_min).as<uint32_t>());
            handle_cleanup();
            retry_update(RETRY_INTERVAL);
            break;

        case AsyncHTTPSClientEventType::Data:
            if(json_buffer == nullptr) {
                logger.printfln("JSON Buffer was not allocated correctly before receiving data");

                download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
                handle_cleanup();
                break;
            }

            if(json_buffer_position + event->data_chunk_len >= TEMPERATURES_MAX_JSON_LENGTH) {
                logger.printfln("JSON Buffer overflow");

                download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
                handle_cleanup();
                break;
            }

            memcpy(json_buffer + json_buffer_position, event->data_chunk, event->data_chunk_len);
            json_buffer_position += event->data_chunk_len;
            break;

        case AsyncHTTPSClientEventType::Aborted:
            if (download_state == TEMPERATURES_DOWNLOAD_STATE_PENDING) {
                logger.printfln("Update check aborted");
                download_state = TEMPERATURES_DOWNLOAD_STATE_ABORTED;
            }
            handle_cleanup();

            break;

        case AsyncHTTPSClientEventType::Finished:
            if(json_buffer == nullptr) {
                logger.printfln("JSON Buffer was not allocated correctly before finishing");

                download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
                break;
            } else {
                json_buffer[json_buffer_position] = '\0';
                handle_new_data();
            }
            handle_cleanup();

            if (download_state == TEMPERATURES_DOWNLOAD_STATE_PENDING) {
                download_state = TEMPERATURES_DOWNLOAD_STATE_OK;
            }

            break;

        case AsyncHTTPSClientEventType::Redirect:
            break;
        }
    });
}

void Temperatures::handle_cleanup()
{
    free_any(json_buffer);
    json_buffer = nullptr;
    json_buffer_position = 0;
}

void Temperatures::handle_new_data()
{
    // Deserialize json received from API
    // Expected format:
    // {"first_date":1768608000,"temperatures":[82,85,87,...]}
    // first_date is UTC timestamp of local midnight (in seconds from API).
    // temperatures is a flat array of 47-49 temperatures in °C * 10.
    DynamicJsonDocument json_doc{TEMPERATURES_MAX_ARDUINO_JSON_BUFFER_SIZE};
    DeserializationError error = deserializeJson(json_doc, json_buffer, json_buffer_position);
    if (error) {
        logger.printfln("Error during JSON deserialization: %s", error.c_str());
        download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
        state.get("next_check")->updateUint(rtc.timestamp_minutes() + (RETRY_INTERVAL / 1_min).as<uint32_t>());
        retry_update(RETRY_INTERVAL);
        return;
    }

    if (!json_doc.containsKey("first_date") || !json_doc.containsKey("temperatures")) {
        logger.printfln("JSON missing 'first_date' or 'temperatures' key");
        download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
        state.get("next_check")->updateUint(rtc.timestamp_minutes() + (RETRY_INTERVAL / 1_min).as<uint32_t>());
        retry_update(RETRY_INTERVAL);
        return;
    }

    // API sends first_date in seconds; store as minutes (like day_ahead_prices)
    const uint32_t first_date = json_doc["first_date"].as<uint32_t>() / 60;
    JsonArray temps_arr = json_doc["temperatures"].as<JsonArray>();
    const size_t count = temps_arr.size();

    if ((first_date == 0) || (count < 47) || (count > 49)) {
        logger.printfln("Invalid first_date (%lu) or temperatures count (%u)", static_cast<unsigned long>(first_date), static_cast<unsigned>(count));
        download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
        state.get("next_check")->updateUint(rtc.timestamp_minutes() + (RETRY_INTERVAL / 1_min).as<uint32_t>());
        retry_update(RETRY_INTERVAL);
        return;
    }

    // Store first_date (in minutes)
    temperatures.get("first_date")->updateUint(first_date);

    // Copy temperatures array
    auto dst_arr = temperatures.get("temperatures");
    while (dst_arr->count() > count) dst_arr->removeLast();
    while (dst_arr->count() < count) dst_arr->add();

    for (size_t i = 0; i < count; i++) {
        dst_arr->get(i)->updateInt(temps_arr[i].as<int16_t>());
    }

    // Compute day stats from the new data
    compute_day_stats();

    const uint32_t current_minutes = rtc.timestamp_minutes();
    state.get("last_sync")->updateUint(current_minutes);
    state.get("last_check")->updateUint(current_minutes);
    state.get("next_check")->updateUint(current_minutes + (CHECK_INTERVAL / 1_min).as<uint32_t>());
}

// Compute min/avg/max for today and tomorrow from the flat temperatures array.
// Uses localtime_r/mktime to find local midnight boundaries, then converts
// to array indices via (midnight_utc - first_date_seconds) / 3600.
void Temperatures::compute_day_stats()
{
    const uint32_t first_date = temperatures.get("first_date")->asUint();
    auto temps_arr = temperatures.get("temperatures");
    const size_t count = temps_arr->count();

    if (first_date == 0 || count < 47) {
        return;
    }

    // first_date is stored in minutes; convert to seconds for timestamp math
    const uint32_t first_date_seconds = first_date * 60;

    struct timeval tv_now;
    if (!rtc.clock_synced(&tv_now)) {
        return;
    }

    const time_t now_utc = tv_now.tv_sec;

    // Find local midnight of today: convert now to local time, zero out h/m/s, convert back
    struct tm tm_local;
    localtime_r(&now_utc, &tm_local);

    struct tm tm_today_midnight = tm_local;
    tm_today_midnight.tm_hour = 0;
    tm_today_midnight.tm_min = 0;
    tm_today_midnight.tm_sec = 0;
    tm_today_midnight.tm_isdst = -1; // let mktime figure out DST
    const time_t today_midnight_utc = mktime(&tm_today_midnight);

    // Tomorrow midnight: add 1 day
    struct tm tm_tomorrow_midnight = tm_local;
    tm_tomorrow_midnight.tm_mday += 1;
    tm_tomorrow_midnight.tm_hour = 0;
    tm_tomorrow_midnight.tm_min = 0;
    tm_tomorrow_midnight.tm_sec = 0;
    tm_tomorrow_midnight.tm_isdst = -1;
    const time_t tomorrow_midnight_utc = mktime(&tm_tomorrow_midnight);

    // Day-after-tomorrow midnight (end of tomorrow)
    struct tm tm_day_after = tm_local;
    tm_day_after.tm_mday += 2;
    tm_day_after.tm_hour = 0;
    tm_day_after.tm_min = 0;
    tm_day_after.tm_sec = 0;
    tm_day_after.tm_isdst = -1;
    const time_t day_after_midnight_utc = mktime(&tm_day_after);

    // Convert UTC timestamps to array indices
    // Each array element is 1 hour apart, starting at first_date_seconds
    auto utc_to_index = [first_date_seconds](time_t utc) -> int32_t {
        return static_cast<int32_t>((utc - static_cast<time_t>(first_date_seconds)) / 3600);
    };

    const int32_t today_start_idx    = utc_to_index(today_midnight_utc);
    const int32_t tomorrow_start_idx = utc_to_index(tomorrow_midnight_utc);
    const int32_t day_after_idx      = utc_to_index(day_after_midnight_utc);

    // Helper to compute min/avg/max over a slice [start_idx, end_idx)
    auto compute_slice = [&](int32_t start_idx, int32_t end_idx,
                             int16_t &out_min, int16_t &out_max, int16_t &out_avg) {
        // Clamp to valid array bounds
        if (start_idx < 0) start_idx = 0;
        if (end_idx > static_cast<int32_t>(count)) end_idx = static_cast<int32_t>(count);

        if (start_idx >= end_idx) {
            out_min = INT16_MAX;
            out_max = INT16_MIN;
            out_avg = INT16_MAX;
            return;
        }

        int16_t min_val = INT16_MAX;
        int16_t max_val = INT16_MIN;
        int32_t sum = 0;

        for (int32_t i = start_idx; i < end_idx; i++) {
            const int16_t val = temps_arr->get(static_cast<size_t>(i))->asInt();
            if (val < min_val) min_val = val;
            if (val > max_val) max_val = val;
            sum += val;
        }

        out_min = min_val;
        out_max = max_val;
        out_avg = static_cast<int16_t>(sum / (end_idx - start_idx));
    };

    compute_slice(today_start_idx, tomorrow_start_idx, today_min, today_max, today_avg);
    compute_slice(tomorrow_start_idx, day_after_idx, tomorrow_min, tomorrow_max, tomorrow_avg);
}

// Create API path that includes currently configured latitude and longitude
String Temperatures::get_api_url_with_path()
{
    char buf[256];
    StringWriter sw(buf, ARRAY_SIZE(buf));

    sw.puts(config.get("api_url")->asString());

    if (sw.getLength() > 0 && *(sw.getRemainingPtr() - 1) != '/') {
        sw.putc('/');
    }

    sw.puts("v1/temperatures/");

    // Convert lat/lon from 1/10000 degrees to decimal string
    // e.g., 519035 -> 51.9035
    const int32_t lat = config.get("lat")->asInt();
    const int32_t lon = config.get("lon")->asInt();

    char lat_str[16];
    snprintf(lat_str, sizeof(lat_str), "%s%ld.%04ld", lat < 0 ? "-" : "", labs(lat) / 10000, labs(lat) % 10000);
    sw.puts(lat_str);

    sw.putc('/');

    char lon_str[16];
    snprintf(lon_str, sizeof(lon_str), "%s%ld.%04ld", lon < 0 ? "-" : "", labs(lon) / 10000, labs(lon) % 10000);
    sw.puts(lon_str);

    return String(buf, sw.getLength());
}

int16_t Temperatures::get_today_min()
{
    return today_min;
}

int16_t Temperatures::get_today_max()
{
    return today_max;
}

int16_t Temperatures::get_today_avg()
{
    return today_avg;
}

int16_t Temperatures::get_tomorrow_min()
{
    return tomorrow_min;
}

int16_t Temperatures::get_tomorrow_max()
{
    return tomorrow_max;
}

int16_t Temperatures::get_tomorrow_avg()
{
    return tomorrow_avg;
}

int16_t Temperatures::get_current()
{
    const uint32_t first_date = temperatures.get("first_date")->asUint();
    auto temps_arr = temperatures.get("temperatures");
    const size_t count = temps_arr->count();

    if (first_date == 0 || count < 47) {
        return INT16_MAX; // No data available
    }

    struct timeval tv_now;
    if (!rtc.clock_synced(&tv_now)) {
        return INT16_MAX;
    }

    // first_date is stored in minutes; convert to seconds for timestamp math
    const uint32_t first_date_seconds = first_date * 60;
    const uint32_t now_utc = static_cast<uint32_t>(tv_now.tv_sec);

    if (now_utc < first_date_seconds) {
        // Before data starts — return first value
        return temps_arr->get(0)->asInt();
    }

    const uint32_t seconds_since_start = now_utc - first_date_seconds;
    const uint32_t hour_index = seconds_since_start / 3600;

    if (hour_index >= count - 1) {
        // At or past the last slot — return last value
        return temps_arr->get(count - 1)->asInt();
    }

    // Interpolate between hour_index and hour_index + 1
    const int32_t t0 = temps_arr->get(hour_index)->asInt();
    const int32_t t1 = temps_arr->get(hour_index + 1)->asInt();
    const uint32_t seconds_into_hour = seconds_since_start - hour_index * 3600;

    return static_cast<int16_t>(t0 + (t1 - t0) * static_cast<int32_t>(seconds_into_hour) / 3600);
}
