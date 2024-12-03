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
#include "tools.h"
#include <cmath>

static constexpr auto CHECK_INTERVAL = 1_m;
static constexpr auto PRICE_UPDATE_INTERVAL = 15_m;

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

void DayAheadPrices::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"api_url", Config::Str(BUILD_DAY_AHEAD_PRICE_API_URL, 0, 64)},
        {"region", Config::Uint(REGION_DE, REGION_DE, REGION_LU)},
        {"resolution", Config::Uint(RESOLUTION_60MIN, RESOLUTION_15MIN, RESOLUTION_60MIN)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
        {"vat", Config::Uint(0, 0, 10000)}, // in %/100
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
            (update.get("enable")->asBool()     != config.get("enable")->asBool()) ||
            (update.get("api_url")->asString()  != config.get("api_url")->asString()) ||
            (update.get("cert_id")->asInt()     != config.get("cert_id")->asInt()) ) {
            state.get("last_sync")->updateUint(0);
            state.get("last_check")->updateUint(0);
            state.get("next_check")->updateUint(0);
            state.get("current_price")->updateInt(0);
            prices.get("first_date")->updateUint(0);
            prices.get("prices")->removeAll();
            prices.get("resolution")->updateUint(update.get("resolution")->asUint());
            current_price_available = false;
            task_scheduler.scheduleOnce([this]() {
                this->update();
            });
        }

        return "";
    }};

    state = Config::Object({
        {"last_sync",  Config::Uint32(0)}, // unix timestamp in minutes
        {"last_check", Config::Uint32(0)}, // unix timestamp in minutes
        {"next_check", Config::Uint32(0)}, // unix timestamp in minutes
        {"current_price", Config::Int32(INT32_MAX)} // in ct/1000 per kWh
    });

    prices = Config::Object({
        {"first_date", Config::Uint32(0)}, // unix timestamp in minutes
        {"resolution", Config::Uint(RESOLUTION_60MIN, RESOLUTION_15MIN, RESOLUTION_60MIN)},
        {"prices",     Config::Array({}, Config::get_prototype_int32_0(), 0, DAY_AHEAD_PRICE_MAX_AMOUNT, Config::type_id<Config::ConfInt>())}
    });

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_trigger(
        AutomationTriggerID::DayAheadPriceNow,
        Config::Object({
            {"type", Config::Int32(INT32_MAX)},
            {"comparison", Config::Int32(INT32_MAX)},
            {"value", Config::Int32(INT32_MAX)},
        })
    );
#endif
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

    task_scheduler.scheduleWhenClockSynced([this]() {
        this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
            this->update();
        }, CHECK_INTERVAL);
    });

    task_scheduler.scheduleWallClock([this]() {
        this->update_current_price();
    }, PRICE_UPDATE_INTERVAL, 0_ms, true);
}

void DayAheadPrices::update_current_price()
{
    static int32_t last_price = INT32_MAX;

    const uint32_t resolution_divisor = config.get("resolution")->asUint() == RESOLUTION_15MIN ? 15 : 60;
    const uint32_t diff = rtc.timestamp_minutes() - prices.get("first_date")->asUint();
    const uint32_t index = diff/resolution_divisor;
    if (prices.get("prices")->count() <= index) {
        state.get("current_price")->updateInt(INT32_MAX);
        current_price_available = false;
    } else {
        state.get("current_price")->updateInt(prices.get("prices")->get(index)->asInt());
        current_price_available = true;
    }

#if MODULE_AUTOMATION_AVAILABLE()
    if (boot_stage > BootStage::SETUP) {
        int32_t current_price = state.get("current_price")->asInt();
        if ((current_price != INT32_MAX) && (current_price != last_price)) {
            automation.trigger(AutomationTriggerID::DayAheadPriceNow, &current_price, this);
            last_price = current_price;
        }
    }
#endif
}

void DayAheadPrices::update_prices_sorted()
{
    auto p = prices.get("prices");
    const uint32_t num_prices = p->count();

    // No price data available
    if (num_prices == 0) {
        prices_sorted_available = false;
        return;
    }

    // Put prices in array with pair of index and price
    std::fill_n(prices_sorted, DAY_AHEAD_PRICE_MAX_AMOUNT, std::pair<uint8_t, int32_t>(-1, 0));

    const uint8_t multiplier = config.get("resolution")->asUint() == RESOLUTION_60MIN ? 4 : 1;
    prices_sorted_count = MIN(num_prices * multiplier, DAY_AHEAD_PRICE_MAX_AMOUNT);
    for (uint8_t i = 0; i < prices_sorted_count/multiplier; i++) {
        if (config.get("resolution")->asUint() == RESOLUTION_15MIN) {
            prices_sorted[i] = std::make_pair(i, p->get(i)->asInt());
        } else {
            prices_sorted[i*4+0] = std::make_pair(i*4+0, p->get(i)->asInt());
            prices_sorted[i*4+1] = std::make_pair(i*4+1, p->get(i)->asInt());
            prices_sorted[i*4+2] = std::make_pair(i*4+2, p->get(i)->asInt());
            prices_sorted[i*4+3] = std::make_pair(i*4+3, p->get(i)->asInt());
        }
    }

    // Sort prices today by comparator (ascending for cheap hours, descending for expensive hours)
    std::sort(&prices_sorted[0], &prices_sorted[0]+prices_sorted_count,  [](const std::pair<uint8_t, int32_t> &a, const std::pair<uint8_t, int32_t> &b) {
        return a.second < b.second;
    });

    prices_sorted_available = true;
}

void DayAheadPrices::update_minmaxavg_price()
{
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&last_update_minmaxavg))
        return;

    // Minimum, average, maximum today
    const uint32_t start_today = last_update_minmaxavg / 60;
    const uint32_t end_today   = start_today + 24*60 - 1;
    price_minimum_today = get_minimum_price_between(start_today, end_today);
    price_average_today = get_average_price_between(start_today, end_today);
    price_maximum_today = get_maximum_price_between(start_today, end_today);

    // Minimum, average, maximum tomorrow
    const uint32_t start_tomorrow = start_today + 24*60;
    const uint32_t end_tomorrow   = start_tomorrow + 24*60 - 1;
    price_minimum_tomorrow = get_minimum_price_between(start_tomorrow, end_tomorrow);
    price_average_tomorrow = get_average_price_between(start_tomorrow, end_tomorrow);
    price_maximum_tomorrow = get_maximum_price_between(start_tomorrow, end_tomorrow);
}

void DayAheadPrices::retry_update(millis_t delay)
{
    // Cancel current task
    task_scheduler.cancel(task_id);

    // And schedule a new one that will run after the given delay,
    // but with the standard interval afterwards again
    this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
        this->update();
    }, delay, CHECK_INTERVAL);
}

void DayAheadPrices::update()
{
    if (config.get("enable")->asBool() == false) {
        return;
    }

    if (state.get("next_check")->asUint() > rtc.timestamp_minutes()) {
        return;
    }

    if (!network.is_connected()) {
        retry_update(1_s);
        return;
    }

    if (download_state == DAP_DOWNLOAD_STATE_PENDING) {
        return;
    }

    // Only update if clock is synced
    struct timeval tv_now;
    if (!rtc.clock_synced(&tv_now)) {
        retry_update(1_s);
        return;
    }

    download_state = DAP_DOWNLOAD_STATE_PENDING;
    state.get("last_check")->updateUint(rtc.timestamp_minutes());

    if (config.get("api_url")->asString().length() == 0) {
        logger.printfln("No day ahead price API server configured");
        download_state = DAP_DOWNLOAD_STATE_ERROR;
        return;
    }

    if(json_buffer == nullptr) {
        json_buffer = (char *)heap_caps_calloc_prefer(DAY_AHEAD_PRICE_MAX_JSON_LENGTH, sizeof(char), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    } else {
        logger.printfln("JSON Buffer was potentially not freed correctly");
        json_buffer_position = 0;
    }

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
                logger.printfln("Certificate with ID %d is not available", config.get("cert_id")->asInt());
                break;

            case AsyncHTTPSClientError::NoResponse:
                logger.printfln("Server %s did not respond", get_api_url_with_path().c_str());
                break;

            case AsyncHTTPSClientError::ShortRead:
                logger.printfln("Day ahead price download ended prematurely");
                break;

            case AsyncHTTPSClientError::HTTPError:
                logger.printfln("HTTP error while downloading day ahead prices");
                break;

            case AsyncHTTPSClientError::HTTPClientInitFailed:
                logger.printfln("Error while creating HTTP client");
                break;

            case AsyncHTTPSClientError::HTTPClientError:
                logger.printfln("Error while downloading day ahead prices: %s", esp_err_to_name(event->error_http_client));
                break;

            case AsyncHTTPSClientError::HTTPStatusError:
                logger.printfln("HTTP error while downloading day ahead prices: %d", event->error_http_status);
                break;

            // use default to prevent warnings since we dont use a body, cookies or headers here
            default:
                logger.printfln("Uncovered error, this should never happen!");
                break;
            }

            download_state = DAP_DOWNLOAD_STATE_ERROR;
            handle_cleanup();
            break;

        case AsyncHTTPSClientEventType::Data:
            if(json_buffer == nullptr) {
                logger.printfln("JSON Buffer was not allocated correctly");

                download_state = DAP_DOWNLOAD_STATE_ERROR;
                handle_cleanup();
                break;
            }

            if(json_buffer_position + event->data_chunk_len >= DAY_AHEAD_PRICE_MAX_JSON_LENGTH) {
                logger.printfln("JSON Buffer overflow");

                download_state = DAP_DOWNLOAD_STATE_ERROR;
                handle_cleanup();
                break;
            }

            memcpy(json_buffer + json_buffer_position, event->data_chunk, event->data_chunk_len);
            json_buffer_position += event->data_chunk_len;
            break;

        case AsyncHTTPSClientEventType::Aborted:
            if (download_state == DAP_DOWNLOAD_STATE_PENDING) {
                logger.printfln("Update check aborted");
                download_state = DAP_DOWNLOAD_STATE_ABORTED;
            }
            handle_cleanup();

            break;

        case AsyncHTTPSClientEventType::Finished:
            if(json_buffer == nullptr) {
                logger.printfln("JSON Buffer was not allocated correctly");

                download_state = DAP_DOWNLOAD_STATE_ERROR;
                break;
            } else {
                json_buffer[json_buffer_position] = '\0';
                handle_new_data();
            }
            handle_cleanup();

            if (download_state == DAP_DOWNLOAD_STATE_PENDING) {
                download_state = DAP_DOWNLOAD_STATE_OK;
            }

            break;
        }
    });
}

void DayAheadPrices::handle_cleanup()
{
    if (json_buffer != nullptr) {
        heap_caps_free(json_buffer);
    }
    json_buffer = nullptr;
    json_buffer_position = 0;
}

void DayAheadPrices::handle_new_data()
{
    // Deserialize json received from API
    DynamicJsonDocument json_doc{DAY_AHEAD_PRICE_MAX_ARDUINO_JSON_BUFFER_SIZE};
    DeserializationError error = deserializeJson(json_doc, json_buffer, json_buffer_position);
    if (error) {
        logger.printfln("Error during JSON deserialization: %s", error.c_str());
        download_state = DAP_DOWNLOAD_STATE_ERROR;
    } else {
        // Put data from json into day_ahead_prices/state object
        JsonArray js_prices = json_doc["prices"].as<JsonArray>();
        auto p = prices.get("prices");
        p->removeAll();
        int count = 0;
        int max_count = this->get_max_price_values();
        for(JsonVariant v : js_prices) {
            p->add()->updateInt(v.as<int>());
            count++;
            if(count >= max_count) {
                break;
            }
        }

        const uint32_t current_minutes = rtc.timestamp_minutes();
        state.get("last_sync")->updateUint(current_minutes);
        state.get("last_check")->updateUint(current_minutes);
        state.get("next_check")->updateUint(json_doc["next_date"].as<int>()/60);
        prices.get("first_date")->updateUint(json_doc["first_date"].as<int>()/60);

        update_current_price();
        update_minmaxavg_price();
        update_prices_sorted();
    }
}

// Create API path that includes currently configured region and resolution
String DayAheadPrices::get_api_url_with_path()
{
    String api_url = config.get("api_url")->asString();

    if (!api_url.endsWith("/")) {
        api_url += "/";
    }

    String region = region_str[config.get("region")->asUint()];
    String resolution = resolution_str[config.get("resolution")->asUint()];

    return api_url + "v1/day_ahead_prices/" + region + "/" + resolution;
}

int DayAheadPrices::get_max_price_values()
{
    // We save maximal 2 days with 24 hours each and one additional hour for daylight savings time switch.
    // Depending on resoultion we have 4 or 1 data points per hour.
    return (2*24 + 1) * (config.get("resolution")->asUint() == RESOLUTION_15MIN ? 4 : 1);
}

bool DayAheadPrices::time_between(const uint32_t index, const uint32_t start, const uint32_t end, const uint32_t first_date, const uint8_t resolution) {
    const uint32_t dap_time = first_date + index * resolution;

    return (dap_time >= start) && (dap_time <= end);
}

Option<int32_t> DayAheadPrices::get_minimum_price_between(const uint32_t start, const uint32_t end)
{
    auto p = prices.get("prices");
    const uint32_t num_prices = p->count();

    // No price data available
    if (num_prices == 0) {
        return {};
    }

    const uint32_t first_date = prices.get("first_date")->asUint();
    const uint32_t resolution = config.get("resolution")->asUint() == RESOLUTION_15MIN ? 15 : 60;

    int32_t min = INT32_MAX;
    int32_t count = 0;
    for (uint32_t i = 0; i < num_prices; i++) {
        if(time_between(i, start, end, first_date, resolution)) {
            const int32_t price = p->get(i)->asInt();
            min = MIN(min, price);
            count++;
        }
    }

    // No data available for today
    if (count == 0) {
        return {};
    }

    return min;
}

Option<int32_t> DayAheadPrices::get_minimum_price_today()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight))
        return {};

    if (last_update_minmaxavg != midnight) {
        update_minmaxavg_price();
    }

    return price_minimum_today;
}

Option<int32_t> DayAheadPrices::get_minimum_price_tomorrow()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight))
        return {};

    if (last_update_minmaxavg != midnight) {
        update_minmaxavg_price();
    }

    return price_minimum_tomorrow;
}

Option<int32_t> DayAheadPrices::get_average_price_between(const uint32_t start, const uint32_t end)
{
    auto p = prices.get("prices");
    const uint32_t num_prices = p->count();

    // No price data available
    if (num_prices == 0) {
        return {};
    }

    const uint32_t first_date = prices.get("first_date")->asUint();
    const uint32_t resolution = config.get("resolution")->asUint() == RESOLUTION_15MIN ? 15 : 60;

    int32_t sum = 0;
    int32_t count = 0;
    for (uint32_t i = 0; i < num_prices; i++) {
        if(time_between(i, start, end, first_date, resolution)) {
            sum += p->get(i)->asInt();
            count++;
        }
    }

    // No data available for today
    if (count == 0) {
        return {};
    }

    return sum / count;
}

Option<int32_t> DayAheadPrices::get_average_price_today()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight))
        return {};

    if (last_update_minmaxavg != midnight) {
        update_minmaxavg_price();
    }

    return price_average_today;
}

Option<int32_t> DayAheadPrices::get_average_price_tomorrow()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight))
        return {};

    if (last_update_minmaxavg != midnight) {
        update_minmaxavg_price();
    }

    return price_average_tomorrow;
}

Option<int32_t> DayAheadPrices::get_maximum_price_between(const uint32_t start, const uint32_t end)
{
    auto p = prices.get("prices");
    const uint32_t num_prices = p->count();

    // No price data available
    if (num_prices == 0) {
        return {};
    }

    const uint32_t first_date = prices.get("first_date")->asUint();
    const uint32_t resolution = config.get("resolution")->asUint() == RESOLUTION_15MIN ? 15 : 60;

    int32_t max = INT32_MIN;
    int32_t count = 0;
    for (uint32_t i = 0; i < num_prices; i++) {
        if(time_between(i, start, end, first_date, resolution)) {
            const int32_t price = p->get(i)->asInt();
            max = MAX(max, price);
            count++;
        }
    }

    // No data available for today
    if (count == 0) {
        return {};
    }

    return max;
}

Option<int32_t> DayAheadPrices::get_maximum_price_today()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight))
        return {};

    if (last_update_minmaxavg != midnight) {
        update_minmaxavg_price();
    }

    return price_maximum_today;
}

Option<int32_t> DayAheadPrices::get_maximum_price_tomorrow()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight))
        return {};

    if (last_update_minmaxavg != midnight) {
        update_minmaxavg_price();
    }

    return price_maximum_tomorrow;
}

Option<int32_t> DayAheadPrices::get_current_price_net()
{
    if (!current_price_available)
        return {};
    return state.get("current_price")->asInt();
}

Option<int32_t> DayAheadPrices::get_current_price()
{
    if (!current_price_available)
        return {};
    return (int32_t)std::round(state.get("current_price")->asInt()*(1 + config.get("vat")->asUint()/10000.0));
}

int32_t DayAheadPrices::get_grid_cost_plus_tax_plus_markup()
{
    return config.get("grid_costs_and_taxes")->asUint() + config.get("supplier_markup")->asUint();
}

bool DayAheadPrices::is_start_time_cheap(const int32_t start_time, const uint8_t duration, const uint8_t amount)
{
    bool cheap_hours[duration*4] = {false};
    const bool ret = get_cheap_and_expensive_hours(start_time, duration, amount, cheap_hours, nullptr);
    return ret && cheap_hours[0];
}

bool DayAheadPrices::get_cheap_and_expensive_hours(const int32_t start_time, const uint8_t duration, const uint8_t amount, bool *cheap_hours, bool *expensive_hours)
{
    if (cheap_hours == nullptr && expensive_hours == nullptr) {
        return false;
    }

    if (prices_sorted_available == false) {
        return false;
    }

    const int32_t first_date  = prices.get("first_date")->asUint();
    const int32_t start_index = (start_time - first_date) / 15;
    const int32_t end_index   = start_index + duration*4;

    if(start_index >= end_index) {
        return false;
    }

    if (cheap_hours != nullptr) {
        std::fill_n(cheap_hours, duration*4, false);

        uint8_t cheap_count = 0;
        for (uint8_t i = 0; i < prices_sorted_count; i++) {
            auto price_index = prices_sorted[i].first;
            if ((price_index >= start_index) && (price_index < end_index)) {
                cheap_count++;
                if(cheap_count > amount*4) {
                    break;
                }
                cheap_hours[price_index-start_index] = true;
            }
        }
    }

    if (expensive_hours != nullptr) {
        std::fill_n(expensive_hours, duration*4, false);

        int16_t expensive_count = 0;
        for (int16_t i = prices_sorted_count-1; i > 0; i--) {
            auto price_index = prices_sorted[i].first;
            if (price_index >= start_index && price_index < end_index) {
                expensive_count++;
                if(expensive_count > amount*4) {
                    break;
                }
                expensive_hours[price_index-start_index] = true;
            }
        }
    }

    return true;
}

bool DayAheadPrices::get_cheap_hours(const int32_t start_time, const uint8_t duration, const uint8_t amount, bool *cheap_hours)
{
    return get_cheap_and_expensive_hours(start_time, duration, amount, cheap_hours, nullptr);
}

bool DayAheadPrices::get_expensive_hours(const int32_t start_time, const uint8_t duration, const uint8_t amount, bool *expensive_hours)
{
    return get_cheap_and_expensive_hours(start_time, duration, amount, nullptr, expensive_hours);
}

#if MODULE_AUTOMATION_AVAILABLE()
bool DayAheadPrices::has_triggered(const Config *conf, void *data)
{
    if (conf->getTag<AutomationTriggerID>() == AutomationTriggerID::DayAheadPriceNow) {
        const Config *cfg = static_cast<const Config *>(conf->get());
        const int32_t current_price = *(int32_t *)data;
        const int32_t type = cfg->get("type")->asInt();
        const int32_t comparison = cfg->get("comparison")->asInt();
        const int32_t value = cfg->get("value")->asInt();

        if (type == 0) {// average
            auto avg = get_average_price_today();
            if (comparison == 0) { // greater
                return avg.is_some() && (current_price > (avg.unwrap()*value/100));
            } else if (comparison == 1) { // less
                return avg.is_some() && (current_price < (avg.unwrap()*value/100));
            }
        } else if (type == 1) { // absolute
            if (comparison == 0) { // greater
                return current_price > (value*1000);
            } else if (comparison == 1) { // less
                return current_price < (value*1000);
            }
        }
    }

    return false;
}
#endif
