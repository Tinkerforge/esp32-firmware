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

#include <Arduino.h>

#include <functional>

#include "keep_alive.h"

class WebSockets;

struct WebSocketsClient {
    int fd;
    WebSockets *ws;

    void send(const char *payload, size_t payload_len);
};

class WebSockets {
public:
    WebSockets()
    {
    }
    void start(const char *uri);
    void stop()
    {
    }

    void sendToClient(const char *payload, size_t payload_len, int sock);
    void sendToAll(const char *payload, size_t payload_len);
    void sendToAllOwned(char *payload, size_t payload_len);

    bool haveActiveClient();

    void onConnect(std::function<void(WebSocketsClient)> fn);

    std::function<void(WebSocketsClient)> on_client_connect_fn;
    wss_keep_alive_t keep_alive;
};
