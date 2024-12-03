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

enum DAPDownloadState {
    DAP_DOWNLOAD_STATE_OK,
    DAP_DOWNLOAD_STATE_PENDING,
    DAP_DOWNLOAD_STATE_ABORTED,
    DAP_DOWNLOAD_STATE_ERROR
};

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

    micros_t last_update_begin;
    char *json_buffer;
    uint32_t json_buffer_position;
    bool current_price_available = false;
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

    bool prices_sorted_available = false;
    uint8_t prices_sorted_count = 0;
    std::pair<uint8_t, int32_t> prices_sorted[DAY_AHEAD_PRICE_MAX_AMOUNT];

public:
    DayAheadPrices(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    esp_err_t update_event_handler_impl(esp_http_client_event_t *event);
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
    bool get_cheap_and_expensive_hours(const int32_t start_time, const uint8_t duration, const uint8_t amount, bool *cheap_hours, bool *expensive_hours);
    bool get_cheap_hours(const int32_t start_time, const uint8_t duration, const uint8_t amount, bool *cheap_hours);
    bool get_expensive_hours(const int32_t start_time, const uint8_t duration, const uint8_t amount, bool *expensive_hours);
    bool is_start_time_cheap(const int32_t start_time, const uint8_t duration, const uint8_t amount);
    int32_t get_grid_cost_plus_tax_plus_markup();

    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot prices;

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif
};

#include "module_available_end.h"
