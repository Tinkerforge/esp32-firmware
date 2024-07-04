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

#include "config.h"

#define MAX_WEB_SOCKET_CLIENTS 5
#define MAX_WEB_SOCKET_WORK_ITEMS_IN_QUEUE 32

class WebSockets;

struct WebSocketsClient {
    int fd;
    WebSockets *ws;

    bool sendOwnedNoFreeBlocking_HTTPThread(char *payload, size_t payload_len);
    void close_HTTPThread();
};

struct ws_work_item {
    int fds[MAX_WEB_SOCKET_CLIENTS];
    char *payload;
    size_t payload_len;
};

void clear_ws_work_item(ws_work_item *wi);

#define WEBSOCKET_WORKER_ENQUEUED 0
#define WEBSOCKET_WORKER_RUNNING 1
#define WEBSOCKET_WORKER_DONE 2

class WebSockets
{
public:
    WebSockets() : worker_active(WEBSOCKET_WORKER_DONE) {}

    void pre_setup();
    void start(const char *uri);

    bool sendToClient(const char *payload, size_t payload_len, int sock);
    bool sendToClientOwned(char *payload, size_t payload_len, int sock);
    bool sendToAll(const char *payload, size_t payload_len);
    bool sendToAllOwned(char *payload, size_t payload_len);

    bool haveFreeSlot();
    bool haveActiveClient();
    void pingActiveClients();
    void checkActiveClients();
    void receivedPong(int fd);

    void cleanUpQueue();
    bool queueFull();

    void onConnect_HTTPThread(std::function<void(WebSocketsClient)> fn);

    void triggerHttpThread();
    bool haveWork(ws_work_item *item);

    void keepAliveAdd(int fd);
    void keepAliveRemove(int fd);
    void keepAliveCloseDead(int fd);

    void updateDebugState();

    // Using a recursive mutex simplifies the method implementations,
    // as every method can lock the mutex without considering that
    // it could be called by another method that locked the mutex.
    std::recursive_mutex keep_alive_mutex;
    int keep_alive_fds[MAX_WEB_SOCKET_CLIENTS];
    uint32_t keep_alive_last_pong[MAX_WEB_SOCKET_CLIENTS];

    std::recursive_mutex work_queue_mutex;
    std::deque<ws_work_item> work_queue;

    std::atomic<uint8_t> worker_active;
    uint32_t last_worker_run = 0;
    uint32_t worker_poll_count = 0;

    std::function<void(WebSocketsClient)> on_client_connect_fn;

    ConfigRoot state;
};
