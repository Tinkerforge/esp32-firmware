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

#pragma once

#include <FS.h> // FIXME: without this include here there is a problem with the IPADDR_NONE define in <lwip/ip4_addr.h>
#include <esp_http_client.h>
#include <ArduinoJson.h>

#include <TFTools/Option.h>

#include "async_https_client.h"
#include "module.h"
#include "config.h"
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

// TODO: We can reduce this again after reducing data on weekend on API server
#define DAY_AHEAD_PRICE_MAX_JSON_LENGTH 4096*2
#define DAY_AHEAD_PRICE_MAX_ARDUINO_JSON_BUFFER_SIZE 8192

#define DAY_AHEAD_PRICE_MAX_AMOUNT (25*4*2) // Two days with 15min resolution and one additional hour for daylight savings time switch
#define DAY_AHEAD_PRICE_CALENDAR_SLOTS_PER_DAY 96  // 24h * 4 (15min resolution)
#define DAY_AHEAD_PRICE_CALENDAR_SLOTS (7 * DAY_AHEAD_PRICE_CALENDAR_SLOTS_PER_DAY) // 7 days * 96 = 672

enum DAPDownloadState {
    DAP_DOWNLOAD_STATE_OK,
    DAP_DOWNLOAD_STATE_PENDING,
    DAP_DOWNLOAD_STATE_ABORTED,
    DAP_DOWNLOAD_STATE_ERROR
};

// DP result cache entry for blocked slot selection.
// Avoids redundant DP computations when update() calls both
// update_plan() and handle_dynamic_price for the same control period blocks.
struct DpCacheEntry {
    int32_t  start_index;
    uint8_t  N;
    uint8_t  K;
    uint8_t  B;
    bool     find_expensive;
    bool     success;
    uint32_t generation;
    bool     valid;
    bool     result[96]; // max dp_N
};

static constexpr size_t DP_CACHE_SIZE = 12;

class DayAheadPrices final : public IModule
#if MODULE_AUTOMATION_AVAILABLE()
                          , public IAutomationBackend
#endif
{
private:
    void update();
    void retry_update(millis_t delay);
    String get_api_url_with_path();
    int get_max_price_values();
    bool time_between(const uint32_t index, const uint32_t start, const uint32_t end, const uint32_t first_date, const uint8_t resolution);
    void handle_new_data();
    void handle_cleanup();

    void update_minmaxavg_price();
    void update_current_price();
    void update_prices_sorted();
    bool dp_select_slots(const int32_t *slot_prices, const uint8_t N, const uint8_t K, const uint8_t B, bool *result, const bool find_expensive);
    void update_calendar();
    void ensure_psram_allocated();

    micros_t last_update_begin;
    char *json_buffer = nullptr;
    uint32_t json_buffer_position = 0;
    bool current_price_available = false;
    int8_t calendar_last_generated_wday = -1; // day-of-week for which calendar prices were last generated
    AsyncHTTPSClient https_client;

    uint64_t task_id = 0;

    DAPDownloadState download_state =  DAP_DOWNLOAD_STATE_OK;

    time_t last_update_minmaxavg;
    Option<int32_t> price_minimum_today;
    Option<int32_t> price_minimum_tomorrow;
    Option<int32_t> price_average_today;
    Option<int32_t> price_average_tomorrow;
    Option<int32_t> price_maximum_today;
    Option<int32_t> price_maximum_tomorrow;

    int32_t *raw_prices = nullptr; // allocated via ensure_psram_allocated() on first use (never freed)
    size_t raw_prices_count = 0;

    size_t prices_sorted_count = 0;
    typedef std::pair<uint8_t, int32_t> PriceSorted;
    PriceSorted *prices_sorted = nullptr; // allocated via ensure_psram_allocated() on first use (never freed)
    int32_t prices_sorted_first_date = 0;

    uint32_t dp_cache_generation = 0;
    DpCacheEntry *dp_cache = nullptr; // allocated via ensure_psram_allocated() on first use (never freed)
    size_t dp_cache_next = 0;

    Option<micros_t> last_no_prices_available = {};

    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot prices;
    ConfigRoot prices_update;
    ConfigRoot calendar;

#ifdef DEBUG_FS_ENABLE
    ConfigRoot debug_price_update;
#endif

public:
    DayAheadPrices(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    esp_err_t update_event_handler_impl(esp_http_client_event_t *event);

    inline bool is_enabled() { return config.get("enable")->asBool() || config.get("enable_calendar")->asBool(); }
    Option<int32_t> get_minimum_price_between(const uint32_t start, const uint32_t end);
    Option<int32_t> get_minimum_price_today();
    Option<int32_t> get_minimum_price_tomorrow();
    Option<int32_t> get_average_price_between(const uint32_t start, const uint32_t end);
    Option<int32_t> get_average_price_today();
    Option<int32_t> get_average_price_tomorrow();
    Option<int32_t> get_maximum_price_between(const uint32_t start, const uint32_t end);
    Option<int32_t> get_maximum_price_today();
    Option<int32_t> get_maximum_price_tomorrow();
    Option<int32_t> get_current_price();
    Option<int32_t> get_current_price_net();
    bool get_cheap_and_expensive_1h(const int32_t start_time, const uint8_t duration_1h, const uint8_t amount_1h, bool *cheap_hours, bool *expensive_hours);
    bool get_cheap_and_expensive_15m(const int32_t start_time, const uint8_t duration_15m, const uint8_t amount_15m, bool *cheap_hours, bool *expensive_hours);
    bool get_cheap_15m(const int32_t start_time, const uint8_t duration_15m, const uint8_t amount_15m, bool *cheap_hours);
    bool get_expensive_15m(const int32_t start_time, const uint8_t duration_15m, const uint8_t amount_15m, bool *expensive_hours);
    bool get_cheap_1h(const int32_t start_time, const uint8_t duration_1h, const uint8_t amount_1h, bool *cheap_hours);
    bool get_expensive_1h(const int32_t start_time, const uint8_t duration_1h, const uint8_t amount_1h, bool *expensive_hours);
    bool get_cheap_and_expensive_15m_blocked(const int32_t start_time, const uint8_t duration_15m, const uint8_t amount_15m, const uint8_t min_block_15m, bool *cheap_hours, bool *expensive_hours);
    bool get_cheap_1h_blocked(const int32_t start_time, const uint8_t duration_1h, const uint8_t amount_1h, const uint8_t min_block_15m, bool *cheap_hours);
    bool get_expensive_1h_blocked(const int32_t start_time, const uint8_t duration_1h, const uint8_t amount_1h, const uint8_t min_block_15m, bool *expensive_hours);
    bool is_start_time_cheap_15m(const int32_t start_time, const uint8_t duration_15m, const uint8_t amount_15m);
    bool is_start_time_cheap_1h(const int32_t start_time, const uint8_t duration_1h, const uint8_t amount_1h);
    int32_t get_grid_cost_plus_tax_plus_markup();
    int32_t add_below(const int32_t start_time, const int32_t price, bool *hours, const uint32_t hours_length);
    int32_t remove_above(const int32_t start_time, const int32_t price, bool *hours, const uint32_t hours_length);

    bool prices_sorted_available = false;

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif
};

#include "module_available_end.h"
