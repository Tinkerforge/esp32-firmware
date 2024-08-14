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

#include "module.h"
#include "config.h"

#define SOLAR_FORECAST_MAX_JSON_LENGTH 4096*4 // TODO: How big does this need to be?
#define SOLAR_FORECAST_PLANES 6

enum SFDownloadState {
    SF_DOWNLOAD_STATE_OK,
    SF_DOWNLOAD_STATE_PENDING,
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

    ConfigRoot config;
    ConfigRoot state;

private:
    class SolarForecastPlane {
    public:
        uint8_t index;
        ConfigRoot config;
        ConfigRoot forecast;
        ConfigRoot state;
    };

    void update();
    void update_price();
    String get_path(const SolarForecastPlane &plane, const PathType path_type);
    const char* get_api_url_with_path(const SolarForecastPlane &plane);

    SolarForecastPlane *plane_current;

    std::unique_ptr<unsigned char[]> cert = nullptr;
    esp_http_client_handle_t http_client = nullptr;
    uint32_t last_update_begin;
    bool download_complete;
    char *json_buffer;
    uint32_t json_buffer_position;

    SFDownloadState download_state = SF_DOWNLOAD_STATE_OK;
    SolarForecastPlane planes[SOLAR_FORECAST_PLANES];
};
