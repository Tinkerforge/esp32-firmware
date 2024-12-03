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

#define SOLAR_FORECAST_MAX_JSON_LENGTH 8192
#define SOLAR_FORECAST_MAX_ARDUINO_JSON_BUFFER_SIZE 8192
#define SOLAR_FORECAST_PLANES 6

enum SFDownloadState {
    SF_DOWNLOAD_STATE_OK,
    SF_DOWNLOAD_STATE_PENDING,
    SF_DOWNLOAD_STATE_ABORTED,
    SF_DOWNLOAD_STATE_ERROR
};

class SolarForecast final : public IModule
{
public:
    enum class PathType {
        Base     = 0,
        Config   = 1,
        State    = 2,
        Forecast = 3,
        _max     = 3
    };

    SolarForecast(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    esp_err_t update_event_handler_impl(esp_http_client_event_t *event);
    void next_update();

    Option<uint32_t> get_wh_today();
    Option<uint32_t> get_wh_tomorrow();

    ConfigRoot config;
    ConfigRoot state;

    uint64_t task_id = 0;

private:
    class SolarForecastPlane {
    public:
        uint8_t index;
        ConfigRoot config;
        ConfigRoot forecast;
        ConfigRoot state;
    };

    void update();
    void retry_update(millis_t delay);
    void update_price();
    String get_path(const SolarForecastPlane &plane, const PathType path_type);
    String get_api_url_with_path(const SolarForecastPlane &plane);
    uint32_t get_timestamp_today_00_00_in_minutes();
    bool forecast_time_between(const uint32_t first_date, const uint32_t index, const uint32_t start, const uint32_t end);
    void handle_new_data();
    void handle_cleanup();

    SolarForecastPlane *plane_current;

    uint32_t last_update_begin;
    char *json_buffer;
    uint32_t json_buffer_position;
    uint32_t next_sync_forced = 0;
    AsyncHTTPSClient https_client;

    SFDownloadState download_state = SF_DOWNLOAD_STATE_OK;
    SolarForecastPlane planes[SOLAR_FORECAST_PLANES];
};
