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
#include <lwip/inet.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "tools/string_builder.h"
#include "options.h"

static constexpr auto CHECK_INTERVAL = 6_h; // Update every 6 hours
static constexpr auto RETRY_INTERVAL = 10_min; // Retry after 5 minutes on error

void Temperatures::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
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
        if ((update.get("enable")->asBool()     != config.get("enable")->asBool()) ||
            (update.get("api_url")->asString()  != config.get("api_url")->asString()) ||
            (update.get("lat")->asInt()         != config.get("lat")->asInt()) ||
            (update.get("lon")->asInt()         != config.get("lon")->asInt()) ||
            (update.get("cert_id")->asInt()     != config.get("cert_id")->asInt())) {
            state.get("last_sync")->updateUint(0);
            state.get("last_check")->updateUint(0);
            state.get("next_check")->updateUint(0);

            // Reset temperature data
            temperatures.get("today_date")->updateUint(0);
            temperatures.get("today_min")->updateInt(INT16_MAX);
            temperatures.get("today_max")->updateInt(INT16_MIN);
            temperatures.get("today_avg")->updateInt(INT16_MAX);
            temperatures.get("tomorrow_date")->updateUint(0);
            temperatures.get("tomorrow_min")->updateInt(INT16_MAX);
            temperatures.get("tomorrow_max")->updateInt(INT16_MIN);
            temperatures.get("tomorrow_avg")->updateInt(INT16_MAX);

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
        {"today_date",    Config::Uint32(0)},     // unix timestamp in seconds
        {"today_min",     Config::Int16(INT16_MAX)},   // temperature in °C * 100 (e.g., 1050 = 10.50°C)
        {"today_max",     Config::Int16(INT16_MIN)},   // temperature in °C * 100
        {"today_avg",     Config::Int16(INT16_MAX)},   // temperature in °C * 100
        {"tomorrow_date", Config::Uint32(0)},     // unix timestamp in seconds
        {"tomorrow_min",  Config::Int16(INT16_MAX)},   // temperature in °C * 100
        {"tomorrow_max",  Config::Int16(INT16_MIN)},   // temperature in °C * 100
        {"tomorrow_avg",  Config::Int16(INT16_MAX)},   // temperature in °C * 100
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

    task_scheduler.scheduleWhenClockSynced([this]() {
        this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
            this->update();
        }, CHECK_INTERVAL);
    });
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
        return;
    }

    // Check if lat/lon are configured (non-zero)
    if (config.get("lat")->asInt() == 0 && config.get("lon")->asInt() == 0) {
        logger.printfln("Latitude and longitude not configured");
        download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
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
    // {"today":{"date":1768608000,"min":3.0,"max":10.4},"tomorrow":{"date":1768694400,"min":0.1,"max":5.0}}
    DynamicJsonDocument json_doc{TEMPERATURES_MAX_ARDUINO_JSON_BUFFER_SIZE};
    DeserializationError error = deserializeJson(json_doc, json_buffer, json_buffer_position);
    if (error) {
        logger.printfln("Error during JSON deserialization: %s", error.c_str());
        download_state = TEMPERATURES_DOWNLOAD_STATE_ERROR;
    } else {
        JsonObject today = json_doc["today"];
        if (today) {
            temperatures.get("today_date")->updateUint(today["date"].as<uint32_t>());
            temperatures.get("today_min")->updateInt(static_cast<int16_t>(roundf(today["min"].as<float>() * 100.0f)));
            temperatures.get("today_max")->updateInt(static_cast<int16_t>(roundf(today["max"].as<float>() * 100.0f)));
            temperatures.get("today_avg")->updateInt(static_cast<int16_t>(roundf(today["avg"].as<float>() * 100.0f)));
        }

        JsonObject tomorrow = json_doc["tomorrow"];
        if (tomorrow) {
            temperatures.get("tomorrow_date")->updateUint(tomorrow["date"].as<uint32_t>());
            temperatures.get("tomorrow_min")->updateInt(static_cast<int16_t>(roundf(tomorrow["min"].as<float>() * 100.0f)));
            temperatures.get("tomorrow_max")->updateInt(static_cast<int16_t>(roundf(tomorrow["max"].as<float>() * 100.0f)));
            temperatures.get("tomorrow_avg")->updateInt(static_cast<int16_t>(roundf(tomorrow["avg"].as<float>() * 100.0f)));
        }

        const uint32_t current_minutes = rtc.timestamp_minutes();
        state.get("last_sync")->updateUint(current_minutes);
        state.get("last_check")->updateUint(current_minutes);
        state.get("next_check")->updateUint(current_minutes + (CHECK_INTERVAL / 1_min).as<uint32_t>());
    }
}

// Create API path that includes currently configured latitude and longitude
String Temperatures::get_api_url_with_path()
{
    char buf[256];
    StringWriter sw(buf, ARRAY_SIZE(buf));

    sw.puts(config.get("api_url")->asString());

    if (*(sw.getRemainingPtr() - 1) != '/') {
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
    return temperatures.get("today_min")->asInt();
}

int16_t Temperatures::get_today_max()
{
    return temperatures.get("today_max")->asInt();
}

int16_t Temperatures::get_today_avg()
{
    return temperatures.get("today_avg")->asInt();
}

int16_t Temperatures::get_tomorrow_min()
{
    return temperatures.get("tomorrow_min")->asInt();
}

int16_t Temperatures::get_tomorrow_max()
{
    return temperatures.get("tomorrow_max")->asInt();
}

int16_t Temperatures::get_tomorrow_avg()
{
    return temperatures.get("tomorrow_avg")->asInt();
}
