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

#include <esp_http_server.h>

#include <functional>
#include <atomic>
#include <mutex>
#include <deque>

#define MAX_WEB_SOCKET_CLIENTS 5
#define MAX_WEB_SOCKET_WORK_ITEMS_IN_QUEUE 20

class WebSockets;

struct WebSocketsClient {
    int fd;
    WebSockets *ws;

    void send(const char *payload, size_t payload_len);
};

struct ws_work_item {
    httpd_handle_t hd;
    int fds[MAX_WEB_SOCKET_CLIENTS];
    char *payload;
    size_t payload_len;

    ws_work_item(httpd_handle_t hd,
                 int fds_to_send[MAX_WEB_SOCKET_CLIENTS],
                 char *payload,
                 size_t payload_len) :
                    hd(hd), fds(), payload(payload), payload_len(payload_len)
    {
        memcpy(this->fds, fds_to_send, sizeof(fds));
    }

    ws_work_item() : hd(nullptr), fds(), payload(nullptr), payload_len(0) {}

    void clear()
    {
        if (this->payload == nullptr)
            return;

        free(this->payload);
    }
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

    bool haveFreeSlot();
    bool haveActiveClient();
    void pingActiveClients();
    void checkActiveClients();
    void receivedPong(int fd);

    void onConnect(std::function<void(WebSocketsClient)> fn);

    void triggerHttpThread();
    bool haveWork(ws_work_item *item);

    void keepAliveAdd(int fd);
    void keepAliveRemove(int fd);
    void keepAliveCloseDead(int fd);

    // Using a recursive mutex simplifies the method implementations,
    // as every method can lock the mutex without considering that
    // it could be called by another method that locked the mutex.
    std::recursive_mutex keep_alive_mutex;
    int keep_alive_fds[MAX_WEB_SOCKET_CLIENTS] = {-1, -1, -1, -1, -1};
    uint32_t keep_alive_last_pong[MAX_WEB_SOCKET_CLIENTS];

    std::recursive_mutex work_queue_mutex;
    std::deque<ws_work_item> work_queue;

    // std::atomic<bool>.is_lock_free() is true!
    std::atomic<bool> worker_active;

    std::function<void(WebSocketsClient)> on_client_connect_fn;
};
