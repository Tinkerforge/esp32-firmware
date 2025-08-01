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

#include "region.enum.h"
#include "resolution.enum.h"

#include <time.h>
#include <lwip/inet.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "options.h"
#include <cmath>

static constexpr auto CHECK_INTERVAL = 1_min;
static constexpr auto PRICE_UPDATE_INTERVAL = 15_min;

static const char * const region_str[] {
    "de", "at", "lu"
};

static const char * const resolution_str[] {
    "15min", "60min"
};

void DayAheadPrices::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"api_url", Config::Str(OPTIONS_DAY_AHEAD_PRICE_API_URL(), 0, 64)},
        {"region", Config::Enum(Region::DE)},
        {"resolution", Config::Enum(Resolution::Min60)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
        {"vat", Config::Uint(0, 0, 10000)}, // in %/100
        {"grid_costs_and_taxes", Config::Uint(0, 0, 99000)}, // in ct/1000 per kWh
        {"supplier_markup", Config::Uint(0, 0, 99000)},      // in ct/1000 per kWh
        {"supplier_base_fee", Config::Uint(0, 0, 99000)},    // in ct per month
    }), [this](Config &update, ConfigSource source) -> String {
        const String &api_url = update.get("api_url")->asString();

        if ((api_url.length() > 0) && !api_url.startsWith("https://")) {
            return "HTTPS required for Day Ahead Price API URL";
        }

        if (!update.get("enable")->asBool() && (prices_sorted != nullptr)) {
            delete_array_psram_or_dram(prices_sorted);
            prices_sorted = nullptr;
        }

        // If region or resolution changes we discard the current state
        // and trigger a new update (with the new config).
        if ((update.get("region")->asEnum<Region>()         != config.get("region")->asEnum<Region>()) ||
            (update.get("resolution")->asEnum<Resolution>() != config.get("resolution")->asEnum<Resolution>()) ||
            (update.get("enable")->asBool()                 != config.get("enable")->asBool()) ||
            (update.get("api_url")->asString()              != config.get("api_url")->asString()) ||
            (update.get("cert_id")->asInt()                 != config.get("cert_id")->asInt()) ) {
            state.get("last_sync")->updateUint(0);
            state.get("last_check")->updateUint(0);
            state.get("next_check")->updateUint(0);
            state.get("current_price")->updateInt(INT32_MAX);
            prices.get("first_date")->updateUint(0);
            prices.get("prices")->removeAll();
            prices.get("resolution")->updateEnum(update.get("resolution")->asEnum<Resolution>());
            current_price_available = false;

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
        {"current_price", Config::Int32(INT32_MAX)} // in ct/1000 per kWh
    });

    prices = Config::Object({
        {"first_date", Config::Uint32(0)}, // unix timestamp in minutes
        {"resolution", Config::Enum(Resolution::Min60)},
        {"prices",     Config::Array({}, Config::get_prototype_int32_0(), 0, DAY_AHEAD_PRICE_MAX_AMOUNT)}
    });

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_trigger(
        AutomationTriggerID::DayAheadPriceNow,
        Config::Object({
            {"type",       Config::Uint8(0, 1)},
            {"comparison", Config::Uint8(0, 1)},
            {"value",      Config::Int32(0)},
        })
    );
#endif

#ifdef DEBUG_FS_ENABLE
    debug_price_update = Config::Int32(0);
#endif
}

void DayAheadPrices::setup()
{
    api.restorePersistentConfig("day_ahead_prices/config", &config);
    prices.get("resolution")->updateEnum(config.get("resolution")->asEnum<Resolution>());

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

#ifdef DEBUG_FS_ENABLE
    api.addCommand("day_ahead_prices/debug_price_update", &debug_price_update, {}, [this](String &/*errmsg*/) {
        const int32_t fake_price = debug_price_update.asInt();

        state.get("current_price")->updateInt(fake_price);
    }, false);
#endif
}

void DayAheadPrices::update_current_price()
{
    const uint32_t resolution_divisor = config.get("resolution")->asEnum<Resolution>() == Resolution::Min15 ? 15 : 60;
    const uint32_t diff = rtc.timestamp_minutes() - prices.get("first_date")->asUint();
    const uint32_t index = diff/resolution_divisor;
    const Config *p = static_cast<const Config *>(prices.get("prices"));

    if (p->count() <= index) {
        state.get("current_price")->updateInt(INT32_MAX);
        current_price_available = false;
    } else {
        int32_t current_price = p->get(index)->asInt();
        const bool price_changed = state.get("current_price")->updateInt(current_price);
        current_price_available = true;

#if MODULE_AUTOMATION_AVAILABLE()
        if (price_changed && boot_stage > BootStage::SETUP) {
            automation.trigger(AutomationTriggerID::DayAheadPriceNow, &current_price, this);
        }
#else
        (void)price_changed;
#endif
    }
}

void DayAheadPrices::update_prices_sorted()
{
    if (prices_sorted == nullptr) {
        prices_sorted = new_array_psram_or_dram<PriceSorted>(DAY_AHEAD_PRICE_MAX_AMOUNT);
    }

    const Config *p = static_cast<const Config *>(prices.get("prices"));
    const size_t num_prices = p->count();

    // No price data available
    if (num_prices == 0) {
        prices_sorted_available = false;
        return;
    }

    // Put prices in array with pair of index and price
    std::fill_n(prices_sorted, DAY_AHEAD_PRICE_MAX_AMOUNT, std::pair<uint8_t, int32_t>(std::numeric_limits<uint8_t>::max(), 0));

    const size_t multiplier = config.get("resolution")->asEnum<Resolution>() == Resolution::Min60 ? 4 : 1;
    prices_sorted_count = std::min(num_prices * multiplier, static_cast<size_t>(DAY_AHEAD_PRICE_MAX_AMOUNT));

    for (size_t i = 0; i < prices_sorted_count/multiplier; i++) {
        const int32_t price = p->get(i)->asInt();
        if (multiplier == 1) {
            prices_sorted[i] = std::make_pair(i, price);
        } else {
            prices_sorted[i*4+0] = std::make_pair(static_cast<uint8_t>(i*4+0), price);
            prices_sorted[i*4+1] = std::make_pair(static_cast<uint8_t>(i*4+1), price);
            prices_sorted[i*4+2] = std::make_pair(static_cast<uint8_t>(i*4+2), price);
            prices_sorted[i*4+3] = std::make_pair(static_cast<uint8_t>(i*4+3), price);
        }
    }

    // Sort prices today by comparator (ascending for cheap hours, descending for expensive hours)
    std::stable_sort(&prices_sorted[0], &prices_sorted[0]+prices_sorted_count,  [](const std::pair<uint8_t, int32_t> &a, const std::pair<uint8_t, int32_t> &b) {
        return a.second < b.second;
    });

    prices_sorted_first_date = prices.get("first_date")->asUint();
    prices_sorted_available  = true;
}

void DayAheadPrices::update_minmaxavg_price()
{
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&last_update_minmaxavg)) {
        return;
    }

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
    if (!config.get("enable")->asBool()) {
        return;
    }

    if (state.get("next_check")->asUint() > rtc.timestamp_minutes()) {
        return;
    }

    // Only update if network is connected and clock is synced
    struct timeval tv_now;
    if (!network.is_connected() || !rtc.clock_synced(&tv_now)) {
        retry_update(1_s);
        return;
    }

    if (download_state == DAP_DOWNLOAD_STATE_PENDING) {
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
        json_buffer = (char *)calloc_psram_or_dram(DAY_AHEAD_PRICE_MAX_JSON_LENGTH, sizeof(char));
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
                logger.printfln("DAP download timed out");
                break;

            case AsyncHTTPSClientError::ShortRead:
                logger.printfln("Day ahead price download ended prematurely");
                break;

            case AsyncHTTPSClientError::HTTPError: {
                char buf[216];
                translate_HTTPError_detailed(event->error_handle, buf, ARRAY_SIZE(buf), true);
                logger.printfln("DAP download failed: %s", buf);
                break;
            }
            case AsyncHTTPSClientError::HTTPClientInitFailed:
                logger.printfln("Error while creating HTTP client");
                break;

            case AsyncHTTPSClientError::HTTPClientError:
                logger.printfln("Error while downloading day ahead prices: %s (0x%lX)", esp_err_to_name(event->error_http_client), static_cast<uint32_t>(event->error_http_client));
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
                logger.printfln("JSON Buffer was not allocated correctly before receiving data");

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
                logger.printfln("JSON Buffer was not allocated correctly before finishing");

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

        case AsyncHTTPSClientEventType::Redirect:
            break;
        }
    });
}

void DayAheadPrices::handle_cleanup()
{
    free_any(json_buffer);
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
        const size_t old_count = p->count();
        const size_t max_count = static_cast<size_t>(this->get_max_price_values());
        size_t count = 0;

        for (JsonVariant v : js_prices) {
            auto price_elem = count < old_count ? p->get(count) : p->add();
            price_elem->updateInt(v.as<int>());
            count++;
            if (count >= max_count) {
                break;
            }
        }

        for (size_t i = old_count; i > count; i--) {
            p->removeLast();
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
    char buf[256];
    StringWriter sw(buf, ARRAY_SIZE(buf));

    const String &api_url = config.get("api_url")->asString();
    sw.puts(api_url.c_str(), static_cast<ssize_t>(api_url.length()));

    if (*(sw.getRemainingPtr() - 1) != '/') {
        sw.putc('/');
    }

    sw.puts("v1/day_ahead_prices/");
    sw.puts(region_str[static_cast<std::underlying_type<Region>::type>(config.get("region")->asEnum<Region>())]);
    sw.putc('/');
    sw.puts(resolution_str[static_cast<std::underlying_type<Resolution>::type>(config.get("resolution")->asEnum<Resolution>())]);

    return String(buf, sw.getLength());
}

int DayAheadPrices::get_max_price_values()
{
    // We save maximal 2 days with 24 hours each and one additional hour for daylight savings time switch.
    // Depending on resoultion we have 4 or 1 data points per hour.
    return (2*24 + 1) * (config.get("resolution")->asEnum<Resolution>() == Resolution::Min15 ? 4 : 1);
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
    const uint32_t resolution = config.get("resolution")->asEnum<Resolution>() == Resolution::Min15 ? 15 : 60;

    int32_t min = INT32_MAX;
    int32_t count = 0;
    for (uint32_t i = 0; i < num_prices; i++) {
        if(time_between(i, start, end, first_date, resolution)) {
            const int32_t price = p->get(i)->asInt();
            min = std::min(min, price);
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
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight)) {
        return {};
    }

    if (last_update_minmaxavg != midnight) {
        update_minmaxavg_price();
    }

    return price_minimum_today;
}

Option<int32_t> DayAheadPrices::get_minimum_price_tomorrow()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight)) {
        return {};
    }

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
    const uint32_t resolution = config.get("resolution")->asEnum<Resolution>() == Resolution::Min15 ? 15 : 60;

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
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight)) {
        return {};
    }

    if (last_update_minmaxavg != midnight) {
        update_minmaxavg_price();
    }

    return price_average_today;
}

Option<int32_t> DayAheadPrices::get_average_price_tomorrow()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight)) {
        return {};
    }

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
    const uint32_t resolution = config.get("resolution")->asEnum<Resolution>() == Resolution::Min15 ? 15 : 60;

    int32_t max = INT32_MIN;
    int32_t count = 0;
    for (uint32_t i = 0; i < num_prices; i++) {
        if(time_between(i, start, end, first_date, resolution)) {
            const int32_t price = p->get(i)->asInt();
            max = std::max(max, price);
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
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight)) {
        return {};
    }

    if (last_update_minmaxavg != midnight) {
        update_minmaxavg_price();
    }

    return price_maximum_today;
}

Option<int32_t> DayAheadPrices::get_maximum_price_tomorrow()
{
    time_t midnight;
    if (!get_localtime_today_midnight_in_utc().try_unwrap(&midnight)) {
        return {};
    }

    if (last_update_minmaxavg != midnight) {
        update_minmaxavg_price();
    }

    return price_maximum_tomorrow;
}

Option<int32_t> DayAheadPrices::get_current_price_net()
{
    if (!current_price_available) {
        return {};
    }

    return state.get("current_price")->asInt();
}

Option<int32_t> DayAheadPrices::get_current_price()
{
    if (!current_price_available) {
        return {};
    }

    return (int32_t)std::round(state.get("current_price")->asInt()*(1 + config.get("vat")->asUint()/10000.0));
}

int32_t DayAheadPrices::get_grid_cost_plus_tax_plus_markup()
{
    return config.get("grid_costs_and_taxes")->asUint() + config.get("supplier_markup")->asUint();
}

int32_t DayAheadPrices::add_below(const int32_t start_time, const int32_t price, bool *hours, const uint32_t hours_length)
{
    if (hours == nullptr) {
        return -1;
    }

    auto p = prices.get("prices");
    const uint8_t resolution_mul = (config.get("resolution")->asEnum<Resolution>() == Resolution::Min15) ? 1 : 4;
    const size_t num_prices = p->count()*resolution_mul;

    // No price data available
    if (num_prices == 0) {
        return -2;
    }

    const uint32_t first_date = prices.get("first_date")->asUint();
    const int32_t start_index = (start_time - first_date) / 15;
    if ((start_index < 0) || (start_index >= num_prices)) {
        return -3;
    }

    int32_t last_changed_index = -1;
    for (uint8_t i = start_index; (i < num_prices) && ((i-start_index) < hours_length); i++) {
        if (p->get(i/resolution_mul)->asInt() < price) {
            last_changed_index = i - start_index;
            hours[last_changed_index] = true;
        }
    }

    return last_changed_index+1;
}

int32_t DayAheadPrices::remove_above(const int32_t start_time, const int32_t price, bool *hours, const uint32_t hours_length)
{
    if (hours == nullptr) {
        return -1;
    }

    auto p = prices.get("prices");
    const uint8_t resolution_mul = (config.get("resolution")->asEnum<Resolution>() == Resolution::Min15) ? 1 : 4;
    const size_t num_prices = p->count()*resolution_mul;

    // No price data available
    if (num_prices == 0) {
        return -2;
    }

    const uint32_t first_date = prices.get("first_date")->asUint();
    const int32_t start_index = (start_time - first_date) / 15;
    if ((start_index < 0) || (start_index >= num_prices)) {
        return -3;
    }

    int32_t last_changed_index = -1;
    for (uint8_t i = start_index; (i < num_prices) && ((i-start_index) < hours_length); i++) {
        if (p->get(i/resolution_mul)->asInt() > price) {
            last_changed_index = i - start_index;
            hours[last_changed_index] = false;
        }
    }

    return last_changed_index+1;
}

bool DayAheadPrices::is_start_time_cheap_15m(const int32_t start_time, const uint8_t duration_15m, const uint8_t amount_15m)
{
    bool cheap_hours[duration_15m] = {false};
    const bool ret = get_cheap_and_expensive_15m(start_time, duration_15m, amount_15m, cheap_hours, nullptr);
    return ret && cheap_hours[0];
}

bool DayAheadPrices::is_start_time_cheap_1h(const int32_t start_time, const uint8_t duration_1h, const uint8_t amount_1h)
{
    return is_start_time_cheap_15m(start_time, duration_1h*4, amount_1h*4);
}

bool DayAheadPrices::get_cheap_and_expensive_15m(const int32_t start_time, const uint8_t duration_15m, const uint8_t amount_15m, bool *cheap_hours, bool *expensive_hours)
{
    if (prices_sorted == nullptr) {
        return false;
    }

    if (cheap_hours == nullptr && expensive_hours == nullptr) {
        return false;
    }

    if (prices_sorted_available == false) {
        return false;
    }

    const int32_t start_index = (start_time - prices_sorted_first_date) / 15;
    const int32_t end_index   = start_index + duration_15m;

    if(start_index >= end_index) {
        return false;
    }

    if (cheap_hours != nullptr) {
        std::fill_n(cheap_hours, duration_15m, false);

        uint8_t cheap_count = 0;
        for (uint8_t i = 0; i < prices_sorted_count; i++) {
            auto price_index = prices_sorted[i].first;
            if ((price_index >= start_index) && (price_index < end_index)) {
                cheap_count++;
                if(cheap_count > amount_15m) {
                    break;
                }
                cheap_hours[price_index-start_index] = true;
            }
        }
    }

    if (expensive_hours != nullptr) {
        std::fill_n(expensive_hours, duration_15m, false);

        int16_t expensive_count = 0;
        for (int16_t i = prices_sorted_count-1; i > 0; i--) {
            auto price_index = prices_sorted[i].first;
            if (price_index >= start_index && price_index < end_index) {
                expensive_count++;
                if(expensive_count > amount_15m) {
                    break;
                }
                expensive_hours[price_index-start_index] = true;
            }
        }
    }

    return true;
}

bool DayAheadPrices::get_cheap_15m(const int32_t start_time, const uint8_t duration_15m, const uint8_t amount_15m, bool *cheap_hours)
{
    return get_cheap_and_expensive_15m(start_time, duration_15m, amount_15m, cheap_hours, nullptr);
}

bool DayAheadPrices::get_expensive_15m(const int32_t start_time, const uint8_t duration_15m, const uint8_t amount_15m, bool *expensive_hours)
{
    return get_cheap_and_expensive_15m(start_time, duration_15m, amount_15m, nullptr, expensive_hours);
}

bool DayAheadPrices::get_cheap_and_expensive_1h(const int32_t start_time, const uint8_t duration_1h, const uint8_t amount_1h, bool *cheap_hours, bool *expensive_hours)
{
    return get_cheap_and_expensive_15m(start_time, duration_1h*4, amount_1h*4, cheap_hours, expensive_hours);
}

bool DayAheadPrices::get_cheap_1h(const int32_t start_time, const uint8_t duration_1h, const uint8_t amount_1h, bool *cheap_hours)
{
    return get_cheap_and_expensive_1h(start_time, duration_1h, amount_1h, cheap_hours, nullptr);
}

bool DayAheadPrices::get_expensive_1h(const int32_t start_time, const uint8_t duration_1h, const uint8_t amount_1h, bool *expensive_hours)
{
    return get_cheap_and_expensive_1h(start_time, duration_1h, amount_1h, nullptr, expensive_hours);
}

#if MODULE_AUTOMATION_AVAILABLE()
bool DayAheadPrices::has_triggered(const Config *conf, void *data)
{
    if (conf->getTag<AutomationTriggerID>() == AutomationTriggerID::DayAheadPriceNow) {
        const Config *cfg = static_cast<const Config *>(conf->get());
        const uint32_t type         = cfg->get("type"      )->asUint();
        const uint32_t comparison   = cfg->get("comparison")->asUint();
        const int32_t value         = cfg->get("value"     )->asInt();
        const int32_t current_price = *static_cast<int32_t *>(data);

        if (type == 0) { // average
            auto avg = get_average_price_today();
            if (avg.is_some()) {
                const int32_t threshold = avg.unwrap() * value / 100; // value in %
                if (comparison == 0) { // greater
                    return current_price > threshold;
                } else if (comparison == 1) { // less
                    return current_price < threshold;
                }
            }
        } else if (type == 1) { // absolute
            const int32_t threshold = value * 1000; // value in ct
            if (comparison == 0) { // greater
                return current_price > threshold;
            } else if (comparison == 1) { // less
                return current_price < threshold;
            }
        }
    }

    return false;
}
#endif
