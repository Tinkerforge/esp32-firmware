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

#include <time.h>
#include <lwip/inet.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);

#define CHECK_FOR_SF_TIMEOUT 15000
#define CHECK_INTERVAL 15*60*1000
#define CHECK_DELAY_MIN 2500

extern SolarForecast dap;

enum Resolution {
    RESOLUTION_15MIN,
    RESOLUTION_60MIN
};

// https://api.forecast.solar/estimate/51.8847388/8.626098/90/25/65
const String test_data = "{\"result\":{\"watts\":{\"2024-08-06 05:56:31\":0,\"2024-08-06 06:15:00\":824,\"2024-08-06 07:00:00\":1873,\"2024-08-06 08:00:00\":3211,\"2024-08-06 09:00:00\":4460,\"2024-08-06 10:00:00\":6923,\"2024-08-06 11:00:00\":11874,\"2024-08-06 12:00:00\":15565,\"2024-08-06 13:00:00\":17623,\"2024-08-06 14:00:00\":17656,\"2024-08-06 15:00:00\":15257,\"2024-08-06 16:00:00\":12611,\"2024-08-06 17:00:00\":11749,\"2024-08-06 18:00:00\":8164,\"2024-08-06 19:00:00\":4580,\"2024-08-06 20:00:00\":2371,\"2024-08-06 21:06:10\":0,\"2024-08-07 05:58:06\":0,\"2024-08-07 06:15:00\":738,\"2024-08-07 07:00:00\":1719,\"2024-08-07 08:00:00\":2972,\"2024-08-07 09:00:00\":4245,\"2024-08-07 10:00:00\":6018,\"2024-08-07 11:00:00\":8399,\"2024-08-07 12:00:00\":10054,\"2024-08-07 13:00:00\":11140,\"2024-08-07 14:00:00\":11493,\"2024-08-07 15:00:00\":11163,\"2024-08-07 16:00:00\":10139,\"2024-08-07 17:00:00\":8738,\"2024-08-07 18:00:00\":7033,\"2024-08-07 19:00:00\":4549,\"2024-08-07 20:00:00\":2405,\"2024-08-07 21:04:21\":0},\"watt_hours_period\":{\"2024-08-06 05:56:31\":0,\"2024-08-06 06:15:00\":127,\"2024-08-06 07:00:00\":1011,\"2024-08-06 08:00:00\":2542,\"2024-08-06 09:00:00\":3836,\"2024-08-06 10:00:00\":5692,\"2024-08-06 11:00:00\":9399,\"2024-08-06 12:00:00\":13720,\"2024-08-06 13:00:00\":16594,\"2024-08-06 14:00:00\":17640,\"2024-08-06 15:00:00\":16457,\"2024-08-06 16:00:00\":13934,\"2024-08-06 17:00:00\":12180,\"2024-08-06 18:00:00\":9957,\"2024-08-06 19:00:00\":6372,\"2024-08-06 20:00:00\":3476,\"2024-08-06 21:06:10\":1307,\"2024-08-07 05:58:06\":0,\"2024-08-07 06:15:00\":104,\"2024-08-07 07:00:00\":921,\"2024-08-07 08:00:00\":2346,\"2024-08-07 09:00:00\":3609,\"2024-08-07 10:00:00\":5132,\"2024-08-07 11:00:00\":7209,\"2024-08-07 12:00:00\":9227,\"2024-08-07 13:00:00\":10597,\"2024-08-07 14:00:00\":11317,\"2024-08-07 15:00:00\":11328,\"2024-08-07 16:00:00\":10651,\"2024-08-07 17:00:00\":9439,\"2024-08-07 18:00:00\":7886,\"2024-08-07 19:00:00\":5791,\"2024-08-07 20:00:00\":3477,\"2024-08-07 21:04:21\":1290},\"watt_hours\":{\"2024-08-06 05:56:31\":0,\"2024-08-06 06:15:00\":127,\"2024-08-06 07:00:00\":1138,\"2024-08-06 08:00:00\":3680,\"2024-08-06 09:00:00\":7516,\"2024-08-06 10:00:00\":13208,\"2024-08-06 11:00:00\":22607,\"2024-08-06 12:00:00\":36327,\"2024-08-06 13:00:00\":52921,\"2024-08-06 14:00:00\":70561,\"2024-08-06 15:00:00\":87018,\"2024-08-06 16:00:00\":100952,\"2024-08-06 17:00:00\":113132,\"2024-08-06 18:00:00\":123089,\"2024-08-06 19:00:00\":129461,\"2024-08-06 20:00:00\":132937,\"2024-08-06 21:06:10\":134244,\"2024-08-07 05:58:06\":0,\"2024-08-07 06:15:00\":104,\"2024-08-07 07:00:00\":1025,\"2024-08-07 08:00:00\":3371,\"2024-08-07 09:00:00\":6980,\"2024-08-07 10:00:00\":12112,\"2024-08-07 11:00:00\":19321,\"2024-08-07 12:00:00\":28548,\"2024-08-07 13:00:00\":39145,\"2024-08-07 14:00:00\":50462,\"2024-08-07 15:00:00\":61790,\"2024-08-07 16:00:00\":72441,\"2024-08-07 17:00:00\":81880,\"2024-08-07 18:00:00\":89766,\"2024-08-07 19:00:00\":95557,\"2024-08-07 20:00:00\":99034,\"2024-08-07 21:04:21\":100324},\"watt_hours_day\":{\"2024-08-06\":134244,\"2024-08-07\":100324}},\"message\":{\"code\":0,\"type\":\"success\",\"text\":\"\",\"pid\":\"342877Z0\",\"info\":{\"latitude\":51.8847,\"longitude\":8.6261,\"distance\":0,\"place\":\"Helleforthstraße 18-20, 33758 Schloß Holte-Stukenbrock, Germany\",\"timezone\":\"Europe/Berlin\",\"time\":\"2024-08-06T16:55:13+02:00\",\"time_utc\":\"2024-08-06T14:55:13+00:00\"},\"ratelimit\":{\"zone\":\"IP 82.198.84.162\",\"period\":3600,\"limit\":12,\"remaining\":10}}}";

void SolarForecast::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(true)},
        {"api_url", Config::Str(BUILD_SOLAR_FORECAST_API_URL, 0, 64)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
    }), [this](Config &update, ConfigSource source) -> String {
        String api_url = update.get("api_url")->asString();

        if ((api_url.length() > 0) && !api_url.startsWith("https://")) {
            return "HTTPS required for Solar Forecast API URL";
        }

        this->next_update();
        return "";
    }};

    state = Config::Object({
        {"rate_limit", Config::Int8(-1)},
        {"rate_remaining", Config::Int8(-1)},
    });

    uint8_t index = 0;
    for (SolarForecastPlane &plane : planes) {
        plane.config = ConfigRoot{Config::Object({
            {"active", Config::Bool(false)},
            {"name", Config::Str("#" + index, 0, 16)},
            {"latitude", Config::Int(0, -900000, 900000)},    // in 1/10000 degrees
            {"longitude", Config::Int(0, -1800000, 1800000)}, // in 1/10000 degrees
            {"declination", Config::Uint(0, 0, 90)},          // in degrees
            {"azimuth", Config::Int(0, -180, 180)},           // in degrees
            {"kwp", Config::Uint(0)}                          // in 1/100 kilowatt-peak
        }), [this, index](Config &update, ConfigSource source) -> String {
            // If the config changes for a plane, we reset the state and forecast and trigger a new update
            SolarForecastPlane &p = this->planes[index];
            p.state.get("next_check")->updateUint(0);
            p.state.get("last_check")->updateUint(0);
            p.state.get("last_sync")->updateUint(0);
            p.state.get("place")->updateString("Unknown");
            p.forecast.get("first_date")->updateUint(0);
            p.forecast.get("forecast")->removeAll();

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
            {"forecast",   Config::Array({}, new Config{Config::Uint32(0)}, 0, 49, Config::type_id<Config::ConfInt>())} // in watt hours, 48 hours + 1 for switch to DST
        });

        plane.index = index++;
    }
}

void SolarForecast::setup()
{
    api.restorePersistentConfig("solar_forecast/config", &config);
    for (SolarForecastPlane &plane : planes) {
        api.restorePersistentConfig(get_path(plane, SolarForecast::PathType::Config), &plane.config);
    }

    json_buffer = nullptr;
    json_buffer_position = 0;

    initialized = true;
}

void SolarForecast::register_urls()
{
    api.addPersistentConfig("solar_forecast/config", &config);
    api.addState("solar_forecast/state", &state);
    for (SolarForecastPlane &plane : planes) {
        api.addPersistentConfig(get_path(plane, SolarForecast::PathType::Config), &plane.config);
        api.addState(get_path(plane, SolarForecast::PathType::State),    &plane.state);
        api.addState(get_path(plane, SolarForecast::PathType::Forecast), &plane.forecast);
    }

    task_scheduler.scheduleWhenClockSynced([this]() {
        this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
            this->update();
        }, 100, CHECK_INTERVAL);
    });
}

void SolarForecast::next_update() {
    // Find smallest next check time.
    // But we wait for at least CHECK_DELAY_MIN ms before the next check.
    // This way we avoid hammering the server and also the ESP has some time in-between.
    int first_delay_ms = CHECK_INTERVAL;
    for (SolarForecastPlane &plane : planes) {
        if (plane.config.get("active")->asBool()) {
            uint32_t next_check = plane.state.get("next_check")->asUint();
            if(next_check == 0) {
                first_delay_ms = CHECK_DELAY_MIN;
                break;
            } else {
                uint32_t current_time = timestamp_minutes();
                if(next_check < current_time) {
                    first_delay_ms = CHECK_DELAY_MIN;
                    break;
                }

                first_delay_ms = std::min(first_delay_ms, (int)(next_check - timestamp_minutes()) * 60 * 1000);
            }
        }
    }

    logger.printfln("Next Solar Forecast update in %d ms", first_delay_ms);

    // Cancel current task
    task_scheduler.cancel(task_id);

    // And schedule a new one that will run after the given delay,
    // but with the standard interval afterwards again
    this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
        this->update();
    }, first_delay_ms, CHECK_INTERVAL);
}

esp_err_t SolarForecast::update_event_handler_impl(esp_http_client_event_t *event)
{
    if (download_complete) {
        return ESP_OK;
    }

    switch (event->event_id) {
    case HTTP_EVENT_ERROR: {
        logger.printfln("HTTP error while downloading json");
        download_state = SF_DOWNLOAD_STATE_ERROR;
        download_complete = true;
        break;
    }

    case HTTP_EVENT_ON_DATA: {
        int code = esp_http_client_get_status_code(http_client);
        // Check status code
        if (code != 200) {
            logger.printfln("HTTP error while downloading json: %d", code);
            download_state = SF_DOWNLOAD_STATE_ERROR;
            download_complete = true;
            break;
        }

        // Check length
        if((event->data_len + json_buffer_position) > (SOLAR_FORECAST_MAX_JSON_LENGTH - 1)) {
            logger.printfln("JSON buffer too small");
            download_state = SF_DOWNLOAD_STATE_ERROR;
            download_complete = true;
            break;
        }

        // Copy data to temporary buffer
        memcpy(json_buffer + json_buffer_position, event->data, event->data_len);
        json_buffer_position += event->data_len;
        break;
    }

    case HTTP_EVENT_ON_FINISH:
        json_buffer[json_buffer_position] = '\0';
        download_complete = true;
        break;

    default:
        break;
    }

    return ESP_OK;
}

static esp_err_t update_event_handler(esp_http_client_event_t *event)
{
    return static_cast<SolarForecast *>(event->user_data)->update_event_handler_impl(event);
}

void SolarForecast::update()
{
    logger.printfln("Solar Forecast update");
    if (http_client != nullptr) {
        return;
    }

    if (config.get("enable")->asBool() == false) {
        return;
    }

    // Only update if NTP is available
    struct timeval tv_now;
    if (!clock_synced(&tv_now)) {
        return;
    }

    // Find plane that is due for update
    plane_current = nullptr;
    for (SolarForecastPlane &plane : planes) {
        if (plane.config.get("active")->asBool()) {
            uint32_t next_check = plane.state.get("next_check")->asUint();
            if((next_check < timestamp_minutes() || next_check == 0)) {
                plane_current = &plane;
                break;
            }
        }
    }

    if(plane_current == nullptr) {
        // No plane due for update found
        return;
    }

    if(json_buffer == nullptr) {
        json_buffer = (char *)heap_caps_calloc_prefer(SOLAR_FORECAST_MAX_JSON_LENGTH, sizeof(char), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    } else {
        logger.printfln("JSON Buffer was potentially not freed correctly");
        json_buffer_position = 0;
    }

    // Copy test data to temporary buffer
    logger.printfln("JSON copy test");
    memcpy(json_buffer + json_buffer_position, test_data.c_str(), test_data.length());
    json_buffer_position += test_data.length();
    json_buffer[json_buffer_position] = '\0';
    download_complete = true;
    // Deserialize json received from API
    logger.printfln("JSON deserialization test");
    DynamicJsonDocument json_doc{SOLAR_FORECAST_MAX_JSON_LENGTH*2};
    DeserializationError error = deserializeJson(json_doc, json_buffer, json_buffer_position);
    if (error) {
        logger.printfln("Error during JSON deserialization: %s", error.c_str());
        download_state = SF_DOWNLOAD_STATE_ERROR;
    } else {
        JsonObject js_message = json_doc["message"];
        JsonInteger code      = js_message["code"];
        if (code != 0) {
            JsonString text = js_message["text"];
            logger.printfln("Solar Forecast server returned error code %ld (%s)", code, text.c_str());
        } else {
            JsonObject js_info       = js_message["info"];
            JsonObject js_reatelimit = js_message["ratelimit"];

            JsonString place         = js_info["place"];
            JsonString timezone      = js_info["timezone"];
            plane_current->state.get("place")->updateString(place.c_str());

            JsonInteger limit        = js_reatelimit["limit"];
            JsonInteger remaining    = js_reatelimit["remaining"];
            JsonInteger period       = js_reatelimit["period"];
            state.get("rate_limit")->updateInt(limit);
            state.get("rate_remaining")->updateInt(remaining);

            JsonObject js_result     = json_doc["result"];
            JsonObject js_wh_period  = js_result["watt_hours_period"];
            uint8_t index = 0;
            bool first = true;
            bool second_day = false;
            for (JsonPair pair : js_wh_period) {
                String key(pair.key().c_str());
                uint32_t value = pair.value().as<int>();

                bool found_match = false;
                while (!found_match) {
                    // Calculate start time of the data from first day
                    if (first) {
                        first = false;

                        // String for 00:00:00 of first day
                        const String first_date = key.substring(0, 11) + "00:00:00";

                        // String to tm struct
                        struct tm tm;
                        strptime(first_date.c_str(), "%Y-%m-%d %H:%M:%S", &tm);

                        // Set first date as unix time in minutes
                        plane_current->forecast.get("first_date")->updateUint(mktime(&tm) / 60);
                    }

                    // Make hour chars from index
                    const char hour0 = '0' + (index / 10);
                    const char hour1 = '0' + (index % 10);

                    // If the hour is in the data set the corresponding watt hours, otherwise assume 0 watt hours
                    if((key[11] == hour0) && (key[12] == hour1)) {
                        if (plane_current->index == 1) {
                            plane_current->forecast.get("forecast")->add()->updateUint(value/random(1, 10));
                        } else {
                            plane_current->forecast.get("forecast")->add()->updateUint(value);
                        }
                        found_match = true;
                    } else {
                        plane_current->forecast.get("forecast")->add()->updateUint(0);
                    }

                    // Go through two days
                    index++;
                    if (index == 24) {
                        if (second_day) {
                            break;
                        }
                        index = 0;
                        second_day = true;
                    }
                }
            }

            // fill rest of the forecast with 0
            while(plane_current->forecast.get("forecast")->count() < 48) {
                plane_current->forecast.get("forecast")->add()->updateUint(0);
            }

            const uint32_t current_minutes = timestamp_minutes();
            plane_current->state.get("last_sync")->updateUint(current_minutes);
            plane_current->state.get("last_check")->updateUint(current_minutes);

            // For the next check we take the period given by the server and multiply it by two
            // to be a good "free tier user" and not hit the server too often.
            // Usually the period is 3600 seconds (one hour), so we will check every two hours.
            plane_current->state.get("next_check")->updateUint(current_minutes + (period/60)*2);
        }
    }

    next_update();

    return;

    download_state = SF_DOWNLOAD_STATE_PENDING;
    state.get("last_check")->updateUint(timestamp_minutes());

    if (config.get("api_url")->asString().length() == 0) {
        logger.printfln("No day ahead price API server configured");
        download_state = SF_DOWNLOAD_STATE_ERROR;
        return;
    }

    esp_http_client_config_t http_config = {};

    http_config.url = get_api_url_with_path(*plane_current);
    http_config.event_handler = update_event_handler;
    http_config.user_data = this;
    http_config.is_async = true;
    http_config.timeout_ms = 500;

    const int cert_id = config.get("cert_id")->asInt();

    if (cert_id < 0) {
        http_config.crt_bundle_attach = esp_crt_bundle_attach;
    }
    else {
#if MODULE_CERTS_AVAILABLE()
        size_t cert_len = 0;

        cert = certs.get_cert(static_cast<uint8_t>(cert_id), &cert_len);

        if (cert == nullptr) {
            logger.printfln("Certificate with ID %d is not available", cert_id);
            download_state = SF_DOWNLOAD_STATE_ERROR;
            return;
        }

        http_config.cert_pem = (const char *)cert.get();
#else
        // defense in depth: it should not be possible to arrive here because in case
        // that the certs module is not available the cert_id should always be -1
        logger.printfln("Can't use custom certificate: certs module is not built into this firmware!");
        return;
#endif
    }

    http_client = esp_http_client_init(&http_config);

    if (http_client == nullptr) {
        logger.printfln("Error while creating HTTP client");
        cert.reset();
        return;
    }

    last_update_begin = millis();

    if(json_buffer == nullptr) {
        json_buffer = (char *)heap_caps_calloc_prefer(SOLAR_FORECAST_MAX_JSON_LENGTH, sizeof(char), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    } else {
        logger.printfln("JSON Buffer was potentially not freed correctly");
        json_buffer_position = 0;
    }

    // Start async JSON download and check every 100ms
    download_complete = false;
    task_scheduler.scheduleWithFixedDelay([this]() {
        // Check for global timeout
        if (deadline_elapsed(last_update_begin + CHECK_FOR_SF_TIMEOUT)) {
            logger.printfln("API server %s did not respond in time", config.get("api_url")->asString().c_str());
            download_state = SF_DOWNLOAD_STATE_ERROR;
            download_complete = true;
        }

        if (!download_complete) {
            // If download is not complete start a new download
            esp_err_t err = esp_http_client_perform(http_client);

            if (err == ESP_ERR_HTTP_EAGAIN) {
                // Nothing to do, just wait for more data
            } else if (err != ESP_OK) {
                logger.printfln("Error while downloading json: %s", esp_err_to_name(err));
                download_state = SF_DOWNLOAD_STATE_ERROR;
                download_complete = true;
            } else if (download_state == SF_DOWNLOAD_STATE_PENDING) {
                // If we reach here the download finished and no error occurred during the download
                download_state = SF_DOWNLOAD_STATE_OK;
                download_complete = true;
            }
        }

        if (download_complete) {
            if(download_state == SF_DOWNLOAD_STATE_OK) {
                // Deserialize json received from API
                json_buffer[json_buffer_position] = 0;
                logger.printfln("buffer: %s", json_buffer);
            }

            // Cleanup
            esp_http_client_close(http_client);
            esp_http_client_cleanup(http_client);
            http_client = nullptr;
            cert.reset();
            heap_caps_free(json_buffer);
            json_buffer = nullptr;
            json_buffer_position = 0;

            task_scheduler.cancel(task_scheduler.currentTaskId());
        }

    }, 100, 100);
}

// Create API path including user configuration
const char* SolarForecast::get_api_url_with_path(const SolarForecastPlane &plane)
{
    static String api_url_with_path;

    String api_url = config.get("api_url")->asString();

    if (!api_url.endsWith("/")) {
        api_url += "/";
    }

    api_url_with_path = api_url + "estimate/"
        + String(plane.config.get("latitude")->asInt()/10000.0, 4)  + "/"
        + String(plane.config.get("longitude")->asInt()/10000.0, 4) + "/"
        + String(plane.config.get("declination")->asUint())         + "/"
        + String(plane.config.get("azimuth")->asInt())              + "/"
        + String(plane.config.get("kwp")->asUint()/100.0, 2);

    return api_url_with_path.c_str();
}

static const char *solar_forecast_path_postfixes[] = {"", "config", "state", "forecast"};
static_assert(ARRAY_SIZE(solar_forecast_path_postfixes) == static_cast<uint32_t>(SolarForecast::PathType::_max) + 1, "Path postfix length mismatch");
String SolarForecast::get_path(const SolarForecastPlane &plane, const SolarForecast::PathType path_type)
{
    String path = "solar_forecast/planes/";
    path.concat(plane.index);
    path.concat('/');
    path.concat(solar_forecast_path_postfixes[static_cast<uint32_t>(path_type)]);

    return path;
}
