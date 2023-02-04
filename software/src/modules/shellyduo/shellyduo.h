/* warp-charger
 * Copyright (C)      2021 Birger Schmidt <bs-warp@netgaroo.com>
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
#include <AsyncTCP.h>

class ShellyDuo {
public:
    ShellyDuo();
    void setup();
    void pre_setup();
    void register_urls();
    void loop();

    bool initialized = false;

    ConfigRoot shellyduo_update;
    ConfigRoot shellyduo_prepare_command;
    ConfigRoot shellyduo_color;
    ConfigRoot shellyduo_config;

    void prepareCommand(String uri);
    void color(int color);
    void update();

    /* void replyToServer(void *arg); */
    /* void handleData(void *arg, AsyncClient *client, void *data, size_t len); */
    /* void onConnect(void *arg, AsyncClient *client); */

private:
    void lookup();
    int feedback();
    void parseFeedback(char* buffer, size_t len);
    void sendCommand();

    bool debug = false;
    String _location, _support;

    uint32_t last_shellyduo_update = 0;
    bool shutdown_logged = false;

    AsyncClient *client_tcp = new AsyncClient;
};
