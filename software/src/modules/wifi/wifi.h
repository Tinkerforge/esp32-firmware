/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "ArduinoJson.h"

#include "config.h"

#define MAX_CONNECT_ATTEMPT_INTERVAL_MS (5 * 60 * 1000)

enum class WifiState {
    NOT_CONFIGURED,
    NOT_CONNECTED,
    CONNECTING,
    CONNECTED
};

class Wifi
{
public:
    Wifi();
    void setup();
    void register_urls();
    void loop();

    bool initialized = false;

    bool was_connected = false;

    WifiState get_connection_state();

private:
    void apply_soft_ap_config_and_start();
    bool apply_sta_config_and_connect();

    int get_ap_state();

    void start_scan();
    void check_for_scan_completion();
    String get_scan_results();

    ConfigRoot wifi_ap_config;
    ConfigRoot wifi_sta_config;
    ConfigRoot wifi_state;

    ConfigRoot wifi_scan_config;

    ConfigRoot wifi_ap_config_in_use;
    ConfigRoot wifi_sta_config_in_use;

    bool soft_ap_running = false;
    uint32_t connect_attempt_interval_ms;

    uint32_t last_connected_ms;
};
