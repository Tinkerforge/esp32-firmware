/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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
#include <ArduinoJson.h>

#include "async_https_client.h"
#include "module.h"
#include "config.h"

#define TEMPERATURES_MAX_JSON_LENGTH 512
#define TEMPERATURES_MAX_ARDUINO_JSON_BUFFER_SIZE 512

enum TemperaturesDownloadState {
    TEMPERATURES_DOWNLOAD_STATE_OK,
    TEMPERATURES_DOWNLOAD_STATE_PENDING,
    TEMPERATURES_DOWNLOAD_STATE_ABORTED,
    TEMPERATURES_DOWNLOAD_STATE_ERROR
};

class Temperatures final : public IModule
{
public:
    Temperatures(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    int16_t get_today_min();
    int16_t get_today_max();
    int16_t get_today_avg();
    int16_t get_tomorrow_min();
    int16_t get_tomorrow_max();
    int16_t get_tomorrow_avg();

private:
    void update();
    void retry_update(millis_t delay);
    String get_api_url_with_path();
    void handle_new_data();
    void handle_cleanup();

    char *json_buffer = nullptr;
    uint32_t json_buffer_position = 0;
    AsyncHTTPSClient https_client;
    uint64_t task_id = 0;

    TemperaturesDownloadState download_state = TEMPERATURES_DOWNLOAD_STATE_OK;

    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot temperatures;
};
