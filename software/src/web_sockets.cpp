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

#include "web_sockets.h"
#include "web_dependencies.h"

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "web_server.h"

#include "esp_httpd_priv.h"

#define KEEP_ALIVE_TIMEOUT_MS 10000
#define WORKER_START_ERROR_MIN_UPTIME_FOR_REBOOT 60 * 60 * 1000

#if MODULE_WATCHDOG_AVAILABLE()
static int watchdog_handle = -1;
#endif

void clear_ws_work_item(ws_work_item *wi)
{
    free(wi->payload);
    wi->payload = nullptr;
}

bool WebSockets::haveWork(ws_work_item *item)
{
    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};

    if (work_queue.empty())
        return false;

    *item = work_queue.front();
    work_queue.pop_front();
    return true;
}

void WebSockets::cleanUpQueue()
{
    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    while (!work_queue.empty()) {
        ws_work_item *wi = &work_queue.front();
        for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
            if (wi->fds[i] != -1) {
                return;
            }
        }
        clear_ws_work_item(wi);
        // Every fd was -1.
        work_queue.pop_front();
    }
}

bool WebSockets::queueFull()
{
    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    if (work_queue.size() < MAX_WEB_SOCKET_WORK_ITEMS_IN_QUEUE) {
        return false;
    }

    cleanUpQueue();

    if (work_queue.size() >= MAX_WEB_SOCKET_WORK_ITEMS_IN_QUEUE) {
        return true;
    }
    logger.printfln("WebSocket work queue was full but %u items were cleaned.", MAX_WEB_SOCKET_WORK_ITEMS_IN_QUEUE - work_queue.size());

    return false;
}

static bool send_ws_work_item(WebSockets *ws, ws_work_item wi) {
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

    ws_pkt.payload = (uint8_t *)wi.payload;
    ws_pkt.len = wi.payload_len;
    ws_pkt.type = wi.payload_len == 0 ? HTTPD_WS_TYPE_PING : HTTPD_WS_TYPE_TEXT;

    bool result = true;

    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (wi.fds[i] == -1) {
            continue;
        }

        if (httpd_ws_get_fd_info(wi.hd, wi.fds[i]) != HTTPD_WS_CLIENT_WEBSOCKET) {
            continue;
        }

        if (httpd_ws_send_frame_async(wi.hd, wi.fds[i], &ws_pkt) != ESP_OK) {
            ws->keepAliveCloseDead(wi.fds[i]);
            result = false;
        }
    }

    return result;
}

static void work(void *arg)
{
    WebSockets *ws = (WebSockets *)arg;

    ws_work_item wi;
    while (ws->haveWork(&wi)) {
        send_ws_work_item(ws, wi);
        clear_ws_work_item(&wi);
    }

    ws->worker_start_errors = 0;
    ws->worker_active = false;
#if MODULE_WATCHDOG_AVAILABLE()
    watchdog.reset(watchdog_handle);
#endif
}

static esp_err_t ws_handler(httpd_req_t *req)
{
    if (req->method == HTTP_GET) {
        auto request = WebServerRequest{req};
        if (server.auth_fn && !server.auth_fn(request)) {
            if (server.on_not_authorized) {
                server.on_not_authorized(request);
                return ESP_OK;
            }
            request.requestAuthentication();
            return ESP_OK;
        }

        struct httpd_req_aux *aux = (struct httpd_req_aux *)req->aux;
        if (aux->ws_handshake_detect) {
            WebSockets *ws = (WebSockets *)req->user_ctx;
            if (!ws->haveFreeSlot()) {
                request.send(503);
                return ESP_FAIL;
            }

            struct httpd_data *hd = (struct httpd_data *)server.httpd;
            esp_err_t ret = httpd_ws_respond_server_handshake(&hd->hd_req, nullptr);
            if (ret != ESP_OK) {
                return ret;
            }

            aux->sd->ws_handshake_done = true;
            aux->sd->ws_handler = ws_handler;
            aux->sd->ws_control_frames = true;
            aux->sd->ws_user_ctx = req->user_ctx;

            int sock = httpd_req_to_sockfd(req);

            if (ws->on_client_connect_fn) {
                // call the client connect callback before adding the client to
                // the keep alive list to ensure that the full state is send by the
                // callback before any other message with a partial state might
                // be send to all clients known by the keep alive list
                ws->on_client_connect_fn(WebSocketsClient{sock, ws});
            }

            ws->keepAliveAdd(sock);
        }
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

    // First receive the full ws message
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        //logger.printfln("httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }

    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = (uint8_t *)calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            logger.printfln("Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            //logger.printfln("httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
    }

    if (ws_pkt.type == HTTPD_WS_TYPE_PING) {
        // We use a patched version of esp-idf that does not handle ping frames in a strange way.
        // We have to send the pong ourselves.
        ws_pkt.type = HTTPD_WS_TYPE_PONG;
        httpd_ws_send_frame(req, &ws_pkt);
    } else if (ws_pkt.type == HTTPD_WS_TYPE_PONG) {
        // If it was a PONG, update the keep-alive
        WebSockets *ws = (WebSockets *)req->user_ctx;
        ws->receivedPong(httpd_req_to_sockfd(req));
    } else if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
        // If it was a TEXT message, print it
        logger.printfln("Ignoring received packet with message: \"%s\" (web sockets are unidirectional for now)", ws_pkt.payload);
        // FIXME: input handling
    } else if (ws_pkt.type == HTTPD_WS_TYPE_CLOSE) {
        // If it was a CLOSE, remove it from the keep-alive list
        WebSockets *ws = (WebSockets *)req->user_ctx;
        ws->keepAliveRemove(httpd_req_to_sockfd(req));
    }
    free(buf);
    return ESP_OK;
}

void WebSockets::keepAliveAdd(int fd)
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] == fd) {
            // fd is alreaedy in the keep alive array. Only update last_pong to prevent instantly closing the new connection.
            // This can happen if web sockets are opened and closed rapidly (so that LWIP "reuses" the fd) and we miss a close frame.
            keep_alive_last_pong[i] = millis();
            return;
        }
    }

    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] != -1)
            continue;
        keep_alive_fds[i] = fd;
        keep_alive_last_pong[i] = millis();
        return;
    }
}

void WebSockets::keepAliveRemove(int fd)
{
    {
        std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
        for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
            if (keep_alive_fds[i] != fd)
                continue;
            keep_alive_fds[i] = -1;
            keep_alive_last_pong[i] = 0;
            break;
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
        for (int i = 0; i < work_queue.size(); ++i)
            for (int j = 0; j < MAX_WEB_SOCKET_CLIENTS; ++j)
                if (work_queue[i].fds[j] == fd)
                    work_queue[i].fds[j] = -1;
    }
}

void WebSockets::keepAliveCloseDead(int fd)
{
    this->keepAliveRemove(fd);
    // Don't kill this socket if it is a HTTP socket:
    // Sometimes a fd is reused so fast that the keep alive does not notice
    // the closed fd before it is reopened as normal HTTP connection.
    if (httpd_ws_get_fd_info(server.httpd, fd) == HTTPD_WS_CLIENT_HTTP)
        return;

    httpd_sess_trigger_close(server.httpd, fd);

    // Seems like we have to do everything by ourselves...
    // In the case that the client is really dead (for example: someone pulled the ethernet cable)
    // We manually have to delete the session, thus closing the fd.
    // If we don't do this, httpd_get_client_list will still return the fd
    // and httpd_ws_get_fd_info will return that this fd is active.
    // We then will attempt to send data to the fd, running into a timeout.
    // The httpd task will then block forever, as other tasks will generate data
    // faster than we are able to throw it away via send timeouts.
    // Unfortunately there is no API to throw away a web socket connection, so
    // we have to poke around in the internal structures here.
    struct httpd_data *hd = (struct httpd_data *)server.httpd;

    struct sock_db *current = hd->hd_sd;
    struct sock_db *end = hd->hd_sd + hd->config.max_open_sockets - 1;

    while (current <= end) {
        if (current->fd == fd) {
            httpd_sess_delete(hd, current);
            break;
        }
        current++;
    }

    // Sometimes the deletion is not complete, but leaves an invalid socket. Also remove those.
    httpd_sess_delete_invalid(hd);
}

void WebSockets::pingActiveClients()
{
    if (!this->haveActiveClient())
        return;

    // Copy over to not hold both mutexes at the same time.
    int fds[MAX_WEB_SOCKET_CLIENTS];
    {
        std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
        memcpy(fds, keep_alive_fds, sizeof(fds));
    }

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    if (queueFull()) {
        return;
    }

    work_queue.push_back({server.httpd, {}, nullptr, 0});
    memcpy(work_queue.back().fds, fds, sizeof(fds));
}

void WebSockets::checkActiveClients()
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] == -1)
            continue;

        if (httpd_ws_get_fd_info(server.httpd, keep_alive_fds[i]) != HTTPD_WS_CLIENT_WEBSOCKET || deadline_elapsed(keep_alive_last_pong[i] + KEEP_ALIVE_TIMEOUT_MS)) {
            this->keepAliveCloseDead(keep_alive_fds[i]);
        }
    }
}

void WebSockets::receivedPong(int fd)
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] != fd)
            continue;

        keep_alive_last_pong[i] = millis();
    }
}

bool WebSocketsClient::sendOwnedBlocking_HTTPThread(char *payload, size_t payload_len)
{
    ws_work_item wi{server.httpd, {this->fd, -1, -1, -1, -1}, payload, payload_len};
    bool result = send_ws_work_item(ws, wi);
    clear_ws_work_item(&wi);
    return result;
}

void WebSocketsClient::close_HTTPThread() {
    ws->keepAliveCloseDead(fd);
}

bool WebSockets::sendToClient(const char *payload, size_t payload_len, int fd)
{
    // Connection was closed -> message was "sent", as in it has not to be resent
    if (httpd_ws_get_fd_info(server.httpd, fd) != HTTPD_WS_CLIENT_WEBSOCKET)
        return true;

    char *payload_copy = (char *)malloc(payload_len * sizeof(char));
    if (payload_copy == nullptr) {
        return false;
    }

    memcpy(payload_copy, payload, payload_len);

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    if (queueFull()) {
        free(payload_copy);
        return false;
    }

    work_queue.push_back({server.httpd, {fd, -1, -1, -1, -1}, payload_copy, payload_len});
    return true;
}

bool WebSockets::sendToClientOwned(char *payload, size_t payload_len, int fd)
{
    if (httpd_ws_get_fd_info(server.httpd, fd) != HTTPD_WS_CLIENT_WEBSOCKET) {
        free(payload);
        return true;
    }

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    if (queueFull()) {
        free(payload);
        return false;
    }

    work_queue.push_back({server.httpd, {fd, -1, -1, -1, -1}, payload, payload_len});
    return true;
}

bool WebSockets::haveActiveClient()
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] != -1)
            return true;
    }
    return false;
}

bool WebSockets::haveFreeSlot()
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] == -1)
            return true;
    }
    return false;
}

bool WebSockets::sendToAllOwned(char *payload, size_t payload_len)
{
    if (!this->haveActiveClient()) {
        free(payload);
        return true;
    }

    // Copy over to not hold both mutexes at the same time.
    int fds[MAX_WEB_SOCKET_CLIENTS];
    {
        std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
        memcpy(fds, keep_alive_fds, sizeof(fds));
    }

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    if (queueFull()) {
        free(payload);
        return false;
    }
    work_queue.push_back({server.httpd, {}, payload, payload_len});
    memcpy(work_queue.back().fds, fds, sizeof(fds));
    return true;
}

bool WebSockets::sendToAll(const char *payload, size_t payload_len)
{
    if (!this->haveActiveClient())
        return true;

    char *payload_copy = (char *)malloc(payload_len * sizeof(char));
    if (payload_copy == nullptr) {
        return false;
    }
    memcpy(payload_copy, payload, payload_len);

    // Copy over to not hold both mutexes at the same time.
    int fds[MAX_WEB_SOCKET_CLIENTS];
    {
        std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
        memcpy(fds, keep_alive_fds, sizeof(fds));
    }

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    if (queueFull()) {
        free(payload_copy);
        return false;
    }

    work_queue.push_back({server.httpd, {}, payload_copy, payload_len});
    memcpy(work_queue.back().fds, fds, sizeof(fds));
    return true;
}

static uint32_t last_worker_run = 0;

void WebSockets::triggerHttpThread()
{
    if (worker_active) {
        // Protect against lost UDP packet in httpd_queue_work control socket.
        // If the packet that enqueues the worker is lost
        // worker_active must be reset or web sockets will never send data again.
        if (last_worker_run != 0 && deadline_elapsed(last_worker_run + KEEP_ALIVE_TIMEOUT_MS * 2)) {
            logger.printfln("WebSocket worker ran %u seconds. Control socket drop? Restarting worker.", (KEEP_ALIVE_TIMEOUT_MS * 2) / 1000U);
            last_worker_run = millis();
            worker_active = false;

            worker_start_errors += (KEEP_ALIVE_TIMEOUT_MS * 2) / 100; // count a hanging worker as if we've attempted to start the worker the whole time.

            std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
            while (!work_queue.empty()) {
                ws_work_item *wi = &work_queue.front();
                clear_ws_work_item(wi);
                work_queue.pop_front();
            }
        }
        return;
    }

    last_worker_run = millis();
/*
    {
        std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
        if (work_queue.empty()) {
            return;
        }
    }
*/
    // If we don't set worker_active to true BEFORE enqueueing the worker,
    // we can be preempted after enqueueing, but before we set worker_active to true
    // the worker can then run to completion, we then set worker_active to true and are
    // NEVER able to start the worker again.
    worker_active = true;
    if (httpd_queue_work(server.httpd, work, this) != ESP_OK) {
        logger.printfln("Failed to start WebSocket worker!");
        worker_active = false;
        ++worker_start_errors;
    }
}

void WebSockets::pre_setup() {
    state = Config::Object({
        {"keep_alive_fds", Config::Array({}, new Config{Config::Int(-1)}, MAX_WEB_SOCKET_CLIENTS, MAX_WEB_SOCKET_CLIENTS, Config::type_id<Config::ConfInt>())},
        {"keep_alive_pongs", Config::Array({}, new Config{Config::Uint(0)}, MAX_WEB_SOCKET_CLIENTS, MAX_WEB_SOCKET_CLIENTS, Config::type_id<Config::ConfUint>())},
        {"worker_active", Config::Bool(false)},
        {"last_worker_run", Config::Uint32(0)},
        {"queue_len", Config::Uint32(0)}
    });

    for(int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        state.get("keep_alive_fds")->add();
        state.get("keep_alive_pongs")->add();
        keep_alive_fds[i] = -1;
        keep_alive_last_pong[i] = 0;
    }
}

void WebSockets::updateDebugState() {
    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    std::lock_guard<std::recursive_mutex> lock2{keep_alive_mutex};
    for(int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        state.get("keep_alive_fds")->get(i)->updateInt(keep_alive_fds[i]);
        state.get("keep_alive_pongs")->get(i)->updateUint(keep_alive_last_pong[i]);
        state.get("worker_active")->updateBool(worker_active);
        state.get("last_worker_run")->updateUint(last_worker_run);
        state.get("queue_len")->updateUint(work_queue.size());
    }
}

void WebSockets::start(const char *uri)
{
    httpd_handle_t httpd = server.httpd;

    httpd_uri_t ws = {};
    ws.uri = uri;
    ws.method = HTTP_GET;
    ws.handler = ws_handler;
    ws.user_ctx = this;
    ws.is_websocket = false;
    ws.handle_ws_control_frames = true;

    httpd_register_uri_handler(httpd, &ws);

    task_scheduler.scheduleWithFixedDelay([this](){
        this->triggerHttpThread();
    }, 100, 100);

    task_scheduler.scheduleWithFixedDelay([this](){
        this->updateDebugState();
    }, 1000, 1000);

#if MODULE_WATCHDOG_AVAILABLE()
    task_scheduler.scheduleOnce([this]() {
        watchdog_handle = watchdog.add(
            "websocket_worker",
            "Websocket worker was not able to start for five minutes. The control socket is probably dead.");
    }, WORKER_START_ERROR_MIN_UPTIME_FOR_REBOOT);
#endif

    task_scheduler.scheduleWithFixedDelay([this](){
        this->pingActiveClients();
    }, 1000, 1000);

    task_scheduler.scheduleWithFixedDelay([this](){
        checkActiveClients();
    }, 100, 100);

    api.addState("info/ws", &state, {}, 1000);
}

void WebSockets::onConnect_HTTPThread(std::function<void(WebSocketsClient)> fn)
{
    on_client_connect_fn = fn;
}
