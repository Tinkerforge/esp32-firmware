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

extern SolarForecast dap;

enum Resolution {
    RESOLUTION_15MIN,
    RESOLUTION_60MIN
};

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

        return "";
    }};

    state = Config::Object({
        {"rate_limit", Config::Uint8(0)},
        {"rate_remaining", Config::Uint8(0)},
    });

    uint8_t index = 0;
    for (SolarForecastPlane &plane : planes) {
        plane.config = ConfigRoot{Config::Object({
            {"active", Config::Bool(false)},
            {"latitude", Config::Float(0)},
            {"longitude", Config::Float(0)},
            {"declination", Config::Int16(0)},
            {"azimuth", Config::Int16(0)},
            {"kwp", Config::Float(0)}
        })};

        plane.state = Config::Object({
            {"last_sync",  Config::Uint32(0)}, // unix timestamp in minutes
            {"last_check", Config::Uint32(0)}, // unix timestamp in minutes
            {"next_check", Config::Uint32(0)}, // unix timestamp in minutes
            {"place", Config::Str("Unknown", 0, 128)}
        });

        plane.forecast = Config::Object({
            {"first_date", Config::Uint32(0)}, // unix timestamp in minutes
            {"resolution", Config::Uint(RESOLUTION_60MIN, RESOLUTION_15MIN, RESOLUTION_60MIN)},
            {"forecast",   Config::Array({}, new Config{Config::Uint32(0)}, 0, 200, Config::type_id<Config::ConfInt>())}
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

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->update();
    }, 0, CHECK_INTERVAL);
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
    if (http_client != nullptr) {
        return;
    }

    if (config.get("enable")->asBool() == false) {
        return;
    }

    // Find plane that is due for update
    plane_current = nullptr;
    for (SolarForecastPlane &plane : planes) {
        if (plane.config.get("active")->asBool() && (plane.state.get("next_check")->asUint() < timestamp_minutes())) {
            plane_current = &plane;
            break;
        }
    }

    if(plane_current == nullptr) {
        // No plane due for update found
        return;
    }

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
        + String(plane.config.get("latitude")->asFloat(),    4) + "/"
        + String(plane.config.get("longitude")->asFloat(),   4) + "/"
        + String(plane.config.get("declination")->asFloat(), 4) + "/"
        + String(plane.config.get("azimuth")->asFloat(),     4) + "/"
        + String(plane.config.get("kwp")->asFloat(),         4);

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
