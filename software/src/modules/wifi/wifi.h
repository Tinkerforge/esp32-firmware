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

#include "config.h"

#include "module.h"

enum class WifiState {
    NOT_CONFIGURED,
    NOT_CONNECTED,
    CONNECTING,
    CONNECTED
};

class Wifi final : public IModule
{
public:
    Wifi(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    bool was_connected = false;

    WifiState get_connection_state() const;
    bool is_sta_enabled() const;

private:
    void apply_soft_ap_config_and_start();
    bool apply_sta_config_and_connect();
    bool apply_sta_config_and_connect(WifiState current_state);

    int get_ap_state();

    void start_scan();
    void check_for_scan_completion();
    String get_scan_results();

    ConfigRoot ap_config;
    ConfigRoot sta_config;
    ConfigRoot state;

    OwnedConfig ap_config_in_use;
    OwnedConfig sta_config_in_use;

    bool soft_ap_running = false;

    uint32_t last_connected_ms = 0;
};
