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
#include "day_ahead_prices.h"

#include <time.h>
#include <lwip/inet.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);

#define CHECK_FOR_DAP_TIMEOUT 15000
#define CHECK_INTERVAL 60*1000 // TODO: Change to 15*60*1000

extern DayAheadPrices dap;

void DayAheadPrices::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(true)},
        {"api_url", Config::Str(BUILD_DAY_AHEAD_PRICE_API_URL, 0, 64)},
        {"region", Config::Str("de", 2, 16)},
        {"resolution", Config::Str("15min", 0, 16)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
    }), [this](Config &update, ConfigSource source) -> String {
        String api_url    = update.get("api_url")->asString();
        String region     = update.get("region")->asString();
        String resolution = update.get("resolution")->asString();

        if ((api_url.length() > 0) && !api_url.startsWith("https://")) {
            return "HTTPS required for Day Ahead Price API URL";
        }
        if ((region != "de") && (region != "at") && (region != "lu")) {
            return "Region not supported";
        }
        if ((resolution != "15min") && (resolution != "60min")) {
            return "Resolution not supported";
        }

        return "";
    }};

    state = Config::Object({
        {"last_sync", Config::Uint32(0)}, // unix timestamp in minutes
        {"last_check", Config::Uint32(0)}, // unix timestamp in minutes
        {"next_check", Config::Uint32(0)}, // unix timestamp in minutes
        {"check_error", Config::Str("", 0, 16)},
    });
}

void DayAheadPrices::setup()
{
    api.restorePersistentConfig("dap/config", &config);

    api_url = config.get("api_url")->asString();

    if (api_url.length() > 0) {
        if (!api_url.endsWith("/")) {
            api_url += "/";
        }
    }

    cert_id = config.get("cert_id")->asInt();
    region = config.get("region")->asString();
    resolution = config.get("resolution")->asString();

    // Here api_url needs to end with "/" and region and resolution need to be valid strings
    // This should be ensured by the config validation in pre_setup
    api_url_with_path = api_url + "v1/day_ahead_prices/" + region + "/" + resolution;

    json_buffer = nullptr;
    json_buffer_position = 0;

    initialized = true;
}

void DayAheadPrices::register_urls()
{
    api.addPersistentConfig("dap/config", &config);
    api.addState("dap/state", &state);

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->update();
    }, 0, CHECK_INTERVAL);
}

esp_err_t DayAheadPrices::update_event_handler_impl(esp_http_client_event_t *event)
{
    if (download_complete) {
        return ESP_OK;
    }

    switch (event->event_id) {
    case HTTP_EVENT_ERROR: {
        logger.printfln("HTTP error while downloading json");
        state.get("check_error")->updateString("download_error");
        download_complete = true;
        break;
    }

    case HTTP_EVENT_ON_DATA: {
        int code = esp_http_client_get_status_code(http_client);
        // Check status code
        if (code != 200) {
            logger.printfln("HTTP error while downloading json: %d", code);
            state.get("check_error")->updateString("download_error");
            download_complete = true;
            break;
        }

        // Check length
        if((event->data_len + json_buffer_position) > (DAY_AHEAD_PRICE_MAX_JSON_LENGTH - 1)) {
            logger.printfln("JSON buffer too small");
            state.get("check_error")->updateString("json_overflow");
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

static esp_err_t update_event_handler(esp_http_client_event_t *event) {
    return static_cast<DayAheadPrices *>(event->user_data)->update_event_handler_impl(event);
}

void DayAheadPrices::update()
{
    if (http_client != nullptr) {
        return;
    }

    if(state.get("next_check")->asUint() > timestamp_minutes()) {
        return;
    }

    state.get("check_error")->updateString("pending");
    state.get("last_check")->updateUint(timestamp_minutes());

    if (api_url.length() == 0) {
        logger.printfln("No day ahead price API server configured");
        state.get("check_error")->updateString("no_server_url");
        return;
    }

    esp_http_client_config_t http_config = {};

    http_config.url = api_url_with_path.c_str();
    http_config.event_handler = update_event_handler;
    http_config.user_data = this;
    http_config.is_async = true;
    http_config.timeout_ms = 500;

    if (cert_id < 0) {
        http_config.crt_bundle_attach = esp_crt_bundle_attach;
    }
    else {
#if MODULE_CERTS_AVAILABLE()
        size_t cert_len = 0;

        cert = certs.get_cert(static_cast<uint8_t>(cert_id), &cert_len);

        if (cert == nullptr) {
            logger.printfln("Certificate with ID %d is not available", cert_id);
            state.get("check_error")->updateString("no_cert");
            return;
        }

        http_config.cert_pem = (const char *)cert.get();
#else
        // defense in depth: it should not be possible to arrive here because in case
        // that the certs module is not available the cert_id should always be -1
        logger.printfln("Can't use custom certitifate: certs module is not built into this firmware!");
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
        json_buffer = (char *)heap_caps_calloc_prefer(DAY_AHEAD_PRICE_MAX_JSON_LENGTH, sizeof(char), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    } else {
        logger.printfln("JSON Buffer was potentially not freed correctly");
    }

    // Start async JSON download and check every 100ms
    download_complete = false;
    task_scheduler.scheduleWithFixedDelay([this]() {
        // Check for global timeout
        if (deadline_elapsed(last_update_begin + CHECK_FOR_DAP_TIMEOUT)) {
            logger.printfln("API server %s did not respond in time", api_url.c_str());
            state.get("check_error")->updateString("timeout");
            download_complete = true;
        // If download is not complete start a new download
        }

        if (!download_complete) {
            esp_err_t err = esp_http_client_perform(http_client);

            if (err == ESP_ERR_HTTP_EAGAIN) {
                // Nothing to do, just wait for more data
            } else if (err != ESP_OK) {
                logger.printfln("Error while downloading json: %s", esp_err_to_name(err));
                state.get("check_error")->updateString("download_error");
                download_complete = true;
            } else if (state.get("check_error")->asString() == "pending") {
                state.get("check_error")->updateString("");
                download_complete = true;
            }
        }

        if (download_complete) {
            if(state.get("check_error")->asString() == "") {
                DeserializationError error = deserializeJson(json_doc, json_buffer, json_buffer_position);
                if (error) {
                    state.get("check_error")->updateString("json_failed");
                } else {
                    state.get("last_sync")->updateUint(timestamp_minutes());

                    int first_date   = json_doc["first_date"].as<int>();
                    int next_date    = json_doc["next_date"].as<int>();
                    state.get("next_check")->updateUint(next_date/60);

                    JsonArray prices = json_doc["prices"].as<JsonArray>();

                    // TODO: Remove this debug output
                    int count = 0;
                    for(JsonVariant v : prices) {
                        logger.printfln("price %d: %dct", first_date + 15*60*count, v.as<int>()/1000);
                        count++;
                    }
                    logger.printfln("first_date %d", first_date);
                    logger.printfln("next_date %d", next_date);
                }
            }

            // cleanup
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