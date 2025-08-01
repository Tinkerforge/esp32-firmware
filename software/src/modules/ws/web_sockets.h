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

#include <functional>
#include <atomic>
#include <mutex>
#include <deque>
#include <esp_http_server.h>

#include "config.h"

#define MAX_WEB_SOCKET_CLIENTS 5
#define MAX_WEB_SOCKET_WORK_ITEMS_IN_QUEUE 32

class WebSockets;

struct WebSocketsClient {
    int fd;
    WebSockets *ws;

    bool sendOwnedNoFreeBlocking_HTTPThread(char *payload, size_t payload_len, httpd_ws_type_t ws_type = HTTPD_WS_TYPE_TEXT);
    void close_HTTPThread();
};

struct ws_work_item {
    int fds[MAX_WEB_SOCKET_CLIENTS];
    char *payload;
    size_t payload_len;
    httpd_ws_type_t ws_type;
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
    void pre_reboot();
    void start(const char *uri, const char *state_path, httpd_handle_t httpd, const char *supported_subprotocol = nullptr);

    bool sendToClient(const char *payload, size_t payload_len, int sock, httpd_ws_type_t ws_type = HTTPD_WS_TYPE_TEXT);
    bool sendToClientOwned(char *payload, size_t payload_len, int sock, httpd_ws_type_t ws_type = HTTPD_WS_TYPE_TEXT);
    bool sendToAll(const char *payload, size_t payload_len, httpd_ws_type_t ws_type = HTTPD_WS_TYPE_TEXT);
    bool sendToAllOwned(char *payload, size_t payload_len, httpd_ws_type_t ws_type = HTTPD_WS_TYPE_TEXT);

    bool haveFreeSlot();
    bool haveActiveClient();
    void pingActiveClients();
    void checkActiveClients();
    void closeLRUClient();
    void receivedPong(int fd);

    void cleanUpQueue();
    bool queueFull();

    void onConnect_HTTPThread(std::function<void(WebSocketsClient)> &&fn);
    void onBinaryDataReceived_HTTPThread(std::function<void(const int fd, httpd_ws_frame_t *ws_pkt)> &&fn);

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
    micros_t keep_alive_last_pong[MAX_WEB_SOCKET_CLIENTS];

    std::recursive_mutex work_queue_mutex;
    std::deque<ws_work_item> work_queue;

    std::atomic<uint8_t> worker_active;
    micros_t last_worker_run = 0_us;
    uint32_t worker_poll_count = 0;

    httpd_handle_t httpd;

    std::function<void(WebSocketsClient)> on_client_connect_fn;
    std::function<void(const int fd, httpd_ws_frame_t *ws_pkt)> on_binary_data_received_fn;

    Config state_peers_prototype;
    ConfigRoot state;

    const char *handler_uri;
};
