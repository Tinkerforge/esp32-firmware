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
#define CHECK_INTERVAL 15*60*1000
#define PRICE_UPDATE_INTERVAL 60*1000

enum Region {
    REGION_DE,
    REGION_AT,
    REGION_LU
};
static const char * const region_str[] {
    "de", "at", "lu"
};

enum Resolution {
    RESOLUTION_15MIN,
    RESOLUTION_60MIN
};
static const char * const resolution_str[] {
    "15min", "60min"
};

extern DayAheadPrices dap;

void DayAheadPrices::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(true)},
        {"api_url", Config::Str(BUILD_DAY_AHEAD_PRICE_API_URL, 0, 64)},
        {"region", Config::Uint(REGION_DE, REGION_DE, REGION_LU)},
        {"resolution", Config::Uint(RESOLUTION_60MIN, RESOLUTION_15MIN, RESOLUTION_60MIN)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
        {"grid_costs_and_taxes", Config::Uint(0, 0, 99000)}, // in ct/1000 per kWh
        {"supplier_markup", Config::Uint(0, 0, 99000)},      // in ct/1000 per kWh
        {"supplier_base_fee", Config::Uint(0, 0, 99000)},    // in ct per month
    }), [this](Config &update, ConfigSource source) -> String {
        String api_url = update.get("api_url")->asString();

        if ((api_url.length() > 0) && !api_url.startsWith("https://")) {
            return "HTTPS required for Day Ahead Price API URL";
        }

        // If region or resolution changes we discard the current state
        // and trigger a new update (with the new config).
        if ((update.get("region")->asUint()     != config.get("region")->asUint()) ||
            (update.get("resolution")->asUint() != config.get("resolution")->asUint()) ||
            (update.get("enable")->asBool()     != config.get("enable")->asBool())) {
            state.get("last_sync")->updateUint(0);
            state.get("last_check")->updateUint(0);
            state.get("next_check")->updateUint(0);
            state.get("current_price")->updateInt(0);
            prices.get("first_date")->updateUint(0);
            prices.get("prices")->removeAll();
            prices.get("resolution")->updateUint(update.get("resolution")->asUint());
            task_scheduler.scheduleOnce([this]() {
                this->update();
            }, 10);
        }

        return "";
    }};

    state = Config::Object({
        {"last_sync",  Config::Uint32(0)}, // unix timestamp in minutes
        {"last_check", Config::Uint32(0)}, // unix timestamp in minutes
        {"next_check", Config::Uint32(0)}, // unix timestamp in minutes
        {"current_price", Config::Int32(0)}
    });

    prices = Config::Object({
        {"first_date", Config::Uint32(0)}, // unix timestamp in minutes
        {"resolution", Config::Uint(RESOLUTION_60MIN, RESOLUTION_15MIN, RESOLUTION_60MIN)},
        {"prices",     Config::Array({}, new Config{Config::Int32(0)}, 0, 200, Config::type_id<Config::ConfInt>())}
    });
}

void DayAheadPrices::setup()
{
    api.restorePersistentConfig("day_ahead_prices/config", &config);
    prices.get("resolution")->updateUint(config.get("resolution")->asUint());

    json_buffer = nullptr;
    json_buffer_position = 0;

    initialized = true;
}

void DayAheadPrices::register_urls()
{
    api.addPersistentConfig("day_ahead_prices/config", &config);
    api.addState("day_ahead_prices/state",             &state);
    api.addState("day_ahead_prices/prices",            &prices);

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->update();
    }, 0, CHECK_INTERVAL);

    // TODO: Can we run this at xx:00, xx:15, xx:30 and xx:45?
    task_scheduler.scheduleWithFixedDelay([this]() {
        this->update_price();
    }, 0, PRICE_UPDATE_INTERVAL);
}

esp_err_t DayAheadPrices::update_event_handler_impl(esp_http_client_event_t *event)
{
    if (download_complete) {
        return ESP_OK;
    }

    switch (event->event_id) {
    case HTTP_EVENT_ERROR: {
        logger.printfln("HTTP error while downloading json");
        download_state = DAP_DOWNLOAD_STATE_ERROR;
        download_complete = true;
        break;
    }

    case HTTP_EVENT_ON_DATA: {
        int code = esp_http_client_get_status_code(http_client);
        // Check status code
        if (code != 200) {
            logger.printfln("HTTP error while downloading json: %d", code);
            download_state = DAP_DOWNLOAD_STATE_ERROR;
            download_complete = true;
            break;
        }

        // Check length
        if((event->data_len + json_buffer_position) > (DAY_AHEAD_PRICE_MAX_JSON_LENGTH - 1)) {
            logger.printfln("JSON buffer too small");
            download_state = DAP_DOWNLOAD_STATE_ERROR;
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
    return static_cast<DayAheadPrices *>(event->user_data)->update_event_handler_impl(event);
}

void DayAheadPrices::update_price()
{
    const uint32_t resolution_divisor = config.get("resolution")->asUint() == RESOLUTION_15MIN ? 15 : 60;
    const uint32_t diff = timestamp_minutes() - prices.get("first_date")->asUint();
    const uint32_t index = diff/resolution_divisor;
    if (prices.get("prices")->count() <= index) {
        state.get("current_price")->updateInt(0);
    } else {
        state.get("current_price")->updateInt(prices.get("prices")->get(index)->asInt());
    }
}

void DayAheadPrices::update()
{
    if (http_client != nullptr) {
        return;
    }

    if (state.get("next_check")->asUint() > timestamp_minutes()) {
        return;
    }

    if (config.get("enable")->asBool() == false) {
        return;
    }

    download_state = DAP_DOWNLOAD_STATE_PENDING;
    state.get("last_check")->updateUint(timestamp_minutes());

    if (config.get("api_url")->asString().length() == 0) {
        logger.printfln("No day ahead price API server configured");
        download_state = DAP_DOWNLOAD_STATE_ERROR;
        return;
    }

    esp_http_client_config_t http_config = {};

    http_config.url = get_api_url_with_path();
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
            download_state = DAP_DOWNLOAD_STATE_ERROR;
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
        json_buffer = (char *)heap_caps_calloc_prefer(DAY_AHEAD_PRICE_MAX_JSON_LENGTH, sizeof(char), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    } else {
        logger.printfln("JSON Buffer was potentially not freed correctly");
        json_buffer_position = 0;
    }

    // Start async JSON download and check every 100ms
    download_complete = false;
    task_scheduler.scheduleWithFixedDelay([this]() {
        // Check for global timeout
        if (deadline_elapsed(last_update_begin + CHECK_FOR_DAP_TIMEOUT)) {
            logger.printfln("API server %s did not respond in time", config.get("api_url")->asString().c_str());
            download_state = DAP_DOWNLOAD_STATE_ERROR;
            download_complete = true;
        }

        if (!download_complete) {
            // If download is not complete start a new download
            esp_err_t err = esp_http_client_perform(http_client);

            if (err == ESP_ERR_HTTP_EAGAIN) {
                // Nothing to do, just wait for more data
            } else if (err != ESP_OK) {
                logger.printfln("Error while downloading json: %s", esp_err_to_name(err));
                download_state = DAP_DOWNLOAD_STATE_ERROR;
                download_complete = true;
            } else if (download_state == DAP_DOWNLOAD_STATE_PENDING) {
                // If we reach here the download finished and no error occurred during the download
                download_state = DAP_DOWNLOAD_STATE_OK;
                download_complete = true;
            }
        }

        if (download_complete) {
            if(download_state == DAP_DOWNLOAD_STATE_OK) {
                // Deserialize json received from API
                DynamicJsonDocument json_doc{DAY_AHEAD_PRICE_MAX_JSON_LENGTH*2};
                DeserializationError error = deserializeJson(json_doc, json_buffer, json_buffer_position);
                if (error) {
                    logger.printfln("Error during JSON deserialization: %s", error.c_str());
                    download_state = DAP_DOWNLOAD_STATE_ERROR;
                } else {
                    // Put data from json into day_ahead_prices/state object
                    JsonArray js_prices = json_doc["prices"].as<JsonArray>();
                    prices.get("prices")->removeAll();
                    int count = 0;
                    int max_count = this->get_max_price_values();
                    for(JsonVariant v : js_prices) {
                        prices.get("prices")->add()->updateInt(v.as<int>());
                        count++;
                        if(count >= max_count) {
                            break;
                        }
                    }

                    const uint32_t current_minutes = timestamp_minutes();
                    state.get("last_sync")->updateUint(current_minutes);
                    state.get("last_check")->updateUint(current_minutes);
                    state.get("next_check")->updateUint(json_doc["next_date"].as<int>()/60);
                    prices.get("first_date")->updateUint(json_doc["first_date"].as<int>()/60);
                    this->update_price();
                }
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

// Create API path that includes currently configured region and resolution
const char* DayAheadPrices::get_api_url_with_path()
{
    static String api_url_with_path;

    String api_url = config.get("api_url")->asString();

    if (!api_url.endsWith("/")) {
        api_url += "/";
    }

    String region = region_str[config.get("region")->asUint()];
    String resolution = resolution_str[config.get("resolution")->asUint()];

    api_url_with_path = api_url + "v1/day_ahead_prices/" + region + "/" + resolution;
    return api_url_with_path.c_str();
}

int DayAheadPrices::get_max_price_values()
{
    // We save maximal 2 days with 24 hours each and one additional hour for daylight savings time switch.
    // Depending on resoultion we have 4 or 1 data points per hour.
    return (2*24 + 1) * (config.get("resolution")->asUint() == RESOLUTION_15MIN ? 4 : 1);
}
