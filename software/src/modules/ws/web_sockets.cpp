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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "rollback_timing.h"
#include "tools.h"
#include "tools/net.h"
#include "tools/memory.h"
#include "esp_httpd_priv.h"

#include "gcc_warnings.h"

static constexpr micros_t KEEP_ALIVE_TIMEOUT = 10_s;

static void clear_ws_work_item(ws_work_item *wi)
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
    // Print only to the console because printing to the event log
    // would generate more websocket messages to fill up the queue.
    printf("web_sockets: Work queue was full but %u items were cleaned.\n", MAX_WEB_SOCKET_WORK_ITEMS_IN_QUEUE - work_queue.size());

    return false;
}

bool WebSockets::send_ws_work_item(const ws_work_item *wi)
{
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

    ws_pkt.payload = reinterpret_cast<uint8_t *>(wi->payload);
    ws_pkt.len = wi->payload_len;
    ws_pkt.type = wi->ws_type;

    bool result = true;

    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (wi->fds[i] == -1) {
            continue;
        }

        if (httpd_ws_get_fd_info(this->httpd, wi->fds[i]) != HTTPD_WS_CLIENT_WEBSOCKET) {
            continue;
        }

        if (httpd_ws_send_frame_async(this->httpd, wi->fds[i], &ws_pkt) != ESP_OK) {
            this->keepAliveCloseDead_HTTPThread(wi->fds[i]);
            result = false;
        }
    }

    return result;
}

void WebSockets::work(void *arg)
{
    WebSockets *ws = static_cast<WebSockets *>(arg);
    ws->worker_active = WEBSOCKET_WORKER_RUNNING;

    ws_work_item wi;
    while (ws->haveWork(&wi)) {
        ws->send_ws_work_item(&wi);
        clear_ws_work_item(&wi);
    }

    ws->worker_active = WEBSOCKET_WORKER_DONE;
#if MODULE_WATCHDOG_AVAILABLE()
    watchdog.reset(ws->watchdog_handle);
#endif
}

esp_err_t WebSockets::ws_handler(httpd_req_t *req)
{
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
        buf = static_cast<uint8_t *>(calloc(1, ws_pkt.len + 1));
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

    WebSocketsClient *client = static_cast<WebSocketsClient *>(req->sess_ctx);
    httpd_ws_type_t frame_type;

    if (ws_pkt.type == HTTPD_WS_TYPE_CONTINUE) {
        frame_type = client->last_received_frame_type;
    } else {
        frame_type = ws_pkt.type;
        client->last_received_frame_type = frame_type;
    }

    if (frame_type == HTTPD_WS_TYPE_PING) {
        // We use a patched version of esp-idf that does not handle ping frames in a strange way.
        // We have to send the pong ourselves.
        ws_pkt.type = HTTPD_WS_TYPE_PONG;
        httpd_ws_send_frame(req, &ws_pkt);
    } else if (frame_type == HTTPD_WS_TYPE_PONG) {
        // If it was a PONG, update the keep-alive
        WebSockets *ws = static_cast<WebSockets *>(req->user_ctx);
        ws->receivedPong(httpd_req_to_sockfd(req));
    } else if (frame_type == HTTPD_WS_TYPE_TEXT) {
        // If it was a TEXT message, print it
        logger.printfln("Ignoring received packet with message: \"%.20s\" (web sockets are unidirectional for now)", ws_pkt.payload);
        // FIXME: input handling
    } else if (frame_type == HTTPD_WS_TYPE_BINARY) {
        WebSockets *ws = static_cast<WebSockets *>(req->user_ctx);
        if (ws->on_binary_data_received_fn != nullptr) {
            ws->on_binary_data_received_fn(client, &ws_pkt);
        }
    } else if (frame_type == HTTPD_WS_TYPE_CLOSE) {
        // If it was a CLOSE, remove it from the keep-alive list
        WebSockets *ws = static_cast<WebSockets *>(req->user_ctx);
        ws->keepAliveRemove(httpd_req_to_sockfd(req));

        // Abuse the ws_control_frames flag to indicate that we received a close frame.
        // Any socket that still has both ws_handshake_done and ws_control_frames set to true on close wasn't closed cleanly.
        struct httpd_req_aux *aux = static_cast<struct httpd_req_aux *>(req->aux);
        aux->sd->ws_control_frames = false;

        if (ws->on_client_disconnect_fn != nullptr) {
            ws->on_client_disconnect_fn(client, true);
        }
    } else {
        logger.printfln("Received unexpected frame type=%u final=%i len=%zu", static_cast<unsigned>(ws_pkt.type), ws_pkt.final, ws_pkt.len);
    }

    free(buf);
    return ESP_OK;
}

void WebSockets::keepAliveAdd(int fd)
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] == fd) {
            // fd is already in the keep alive array. Only update last_pong to prevent instantly closing the new connection.
            // This can happen if web sockets are opened and closed rapidly (so that LWIP "reuses" the fd) and we miss a close frame.
            keep_alive_last_pong[i] = now_us();
            return;
        }
    }

    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] != -1)
            continue;
        keep_alive_fds[i] = fd;
        keep_alive_last_pong[i] = now_us();
        return;
    }
}

void WebSockets::keepAliveRemove(int fd)
{
    if (fd < 0) {
        return;
    }

    {
        std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
        for (size_t i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
            if (keep_alive_fds[i] != fd)
                continue;
            keep_alive_fds[i] = -1;
            keep_alive_last_pong[i] = 0_us;
            break;
        }
    }

    {
        std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
        for (size_t i = 0; i < work_queue.size(); ++i)
            for (size_t j = 0; j < MAX_WEB_SOCKET_CLIENTS; ++j)
                if (work_queue[i].fds[j] == fd)
                    work_queue[i].fds[j] = -1;
    }
}

void WebSockets::keepAliveCloseDead_async(int fd)
{
    if (fd < 0) {
        return;
    }

    this->keepAliveRemove(fd);

    // Don't kill this socket if it is a HTTP socket:
    // Sometimes a fd is reused so fast that the keep alive does not notice
    // the closed fd before it is reopened as normal HTTP connection.
    // This should probably be removed because it should be called from the HTTP thread only.
    if (httpd_ws_get_fd_info(httpd, fd) == HTTPD_WS_CLIENT_HTTP) {
        logger.printfln("keepAliveCloseDead_async encountered fd reused for HTTP");
        return;
    }

    httpd_sess_trigger_close(httpd, fd);
}

void WebSockets::keepAliveCloseDead_HTTPThread(int fd)
{
    if (fd < 0) {
        return;
    }

    this->keepAliveRemove(fd);

    // Don't kill this socket if it is a HTTP socket:
    // Sometimes a fd is reused so fast that the keep alive does not notice
    // the closed fd before it is reopened as normal HTTP connection.
    if (httpd_ws_get_fd_info(httpd, fd) == HTTPD_WS_CLIENT_HTTP) {
        logger.printfln("keepAliveCloseDead_HTTPThread encountered fd reused for HTTP");
        return;
    }

    // This old comment might not be relevant anymore:
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
    struct httpd_data *hd = static_cast<struct httpd_data *>(httpd);
    struct sock_db *session = httpd_sess_get(hd, fd);

    httpd_sess_delete(hd, session);
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

    work_queue.push_back({{}, nullptr, 0, HTTPD_WS_TYPE_PING});
    memcpy(work_queue.back().fds, fds, sizeof(fds));
}

void WebSockets::checkActiveClients()
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] == -1)
            continue;

        // This should probably be removed because it should be called from the HTTP thread only.
        // Alternative: Move this check to the HTTP thread and call httpd_sess_delete_invalid there as well.
        // The timeout check below can then run only once a second.
        const httpd_ws_client_info_t type = httpd_ws_get_fd_info(httpd, keep_alive_fds[i]);

        if  (type != HTTPD_WS_CLIENT_WEBSOCKET) {
            logger.printfln("checkActiveClients encountered fd reused for HTTP, type %u", static_cast<unsigned>(type));
            this->keepAliveCloseDead_async(keep_alive_fds[i]);
            continue;
        }

        if (deadline_elapsed(keep_alive_last_pong[i] + KEEP_ALIVE_TIMEOUT)) {
            this->keepAliveCloseDead_async(keep_alive_fds[i]);
        }
    }
}

void WebSockets::closeLRUClient_HTTPThread()
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] == -1)
            return; //Found free slot

        const httpd_ws_client_info_t type = httpd_ws_get_fd_info(httpd, keep_alive_fds[i]);

        if  (type != HTTPD_WS_CLIENT_WEBSOCKET) {
            // Found non-websocket fd.
            // Probably was a websocket, was then closed
            // and re-opened as non-websocket-fd.
            logger.printfln("closeLRUClient_HTTPThread encountered fd reused for HTTP, type %u", static_cast<unsigned>(type));
            this->keepAliveCloseDead_HTTPThread(keep_alive_fds[i]);
            return;
        }
    }

    auto min_fd_idx = 0;
    for (int i = 1; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_last_pong[i] < keep_alive_last_pong[min_fd_idx]) {
            min_fd_idx = i;
        }
    }

    this->keepAliveCloseDead_HTTPThread(keep_alive_fds[min_fd_idx]);
}

void WebSockets::receivedPong(int fd)
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] != fd)
            continue;

        keep_alive_last_pong[i] = now_us();
    }
}

void WebSockets::fakeReceivedPongAll()
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] != -1) {
            keep_alive_last_pong[i] = now_us();
        }
    }
}

bool WebSockets::sendToClient(const char *payload, size_t payload_len, int fd, httpd_ws_type_t ws_type)
{
    // Connection was closed -> message was "sent", as in it has not to be resent
    if (httpd_ws_get_fd_info(httpd, fd) != HTTPD_WS_CLIENT_WEBSOCKET)
        return true;

    char *payload_copy = static_cast<char *>(malloc(payload_len * sizeof(char)));
    if (payload_copy == nullptr) {
        return false;
    }

    memcpy(payload_copy, payload, payload_len);

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    if (queueFull()) {
        free(payload_copy);
        return false;
    }

    work_queue.push_back({{fd, -1, -1, -1, -1}, payload_copy, payload_len, ws_type});
    return true;
}

bool WebSockets::sendToClientOwned(char *payload, size_t payload_len, int fd, httpd_ws_type_t ws_type)
{
    if (httpd_ws_get_fd_info(httpd, fd) != HTTPD_WS_CLIENT_WEBSOCKET) {
        free(payload);
        return true;
    }

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    if (queueFull()) {
        free(payload);
        return false;
    }

    work_queue.push_back({{fd, -1, -1, -1, -1}, payload, payload_len, ws_type});
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

bool WebSockets::sendToAllOwned(char *payload, size_t payload_len, httpd_ws_type_t ws_type)
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
    work_queue.push_back({{}, payload, payload_len, ws_type});
    memcpy(work_queue.back().fds, fds, sizeof(fds));
    return true;
}

bool WebSockets::sendToAllOwnedNoFreeBlocking_HTTPThread(char *payload, size_t payload_len, httpd_ws_type_t ws_type)
{
    if (!this->haveActiveClient()) {
        return true;
    }

    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    ws_work_item wi{{}, payload, payload_len, ws_type};
    memcpy(wi.fds, keep_alive_fds, sizeof(keep_alive_fds));
    return this->send_ws_work_item(&wi);
}

bool WebSockets::sendToAll(const char *payload, size_t payload_len, httpd_ws_type_t ws_type)
{
    if (!this->haveActiveClient())
        return true;

    char *payload_copy = static_cast<char *>(malloc(payload_len * sizeof(char)));
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

    work_queue.push_back({{}, payload_copy, payload_len, ws_type});
    memcpy(work_queue.back().fds, fds, sizeof(fds));
    return true;
}

void WebSockets::triggerHttpThread()
{
    if (worker_active == WEBSOCKET_WORKER_RUNNING) {
        return;
    }

    if (worker_active == WEBSOCKET_WORKER_ENQUEUED) {
        // Protect against stuck localhost communication that blocks the
        // httpd_queue_work control socket. While the worker is enqueued,
        // poke localhost in regular intervals to get things going again.
        // A poll count of 32 results in a poke interval of roughly 4s.
        if ((++worker_poll_count) % 32 == 0) {
            // Don't log this because it happens constantly during a firmware upload.
            //logger.printfln("Poking localhost to get the worker unstuck");
            poke_localhost();
        }
        return;
    }

    // Don't schedule work task if no work is pending.
    // Schedule it anyway once in a while to reset the watchdog.
#if MODULE_WATCHDOG_AVAILABLE()
    if (!deadline_elapsed(last_worker_run + WEB_SERVER_DEAD_TIMEOUT / 8_us))
#endif
    {
        std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
        if (work_queue.empty()) {
            return;
        }
    }

    // If we don't set worker_active to "enqueued" BEFORE enqueueing the worker,
    // we can be preempted after enqueueing, but before we set worker_active to "enqueued"
    // the worker can then run to completion, we then set worker_active to "enqueued" and are
    // NEVER able to start the worker again.
    worker_active = WEBSOCKET_WORKER_ENQUEUED;
    errno = 0;
    esp_err_t err = httpd_queue_work(httpd, work, this);
    if (err == ESP_OK) {
        last_worker_run = now_us();
        worker_poll_count = 0;
    } else {
        logger.printfln("Failed to start WebSocket worker: %i | %s (%i)", err, strerror(errno), errno);
        worker_active = WEBSOCKET_WORKER_DONE;
    }
}

void WebSockets::pre_setup() {
    state = Config::Object({
        {"keep_alive_fds", Config::Tuple(MAX_WEB_SOCKET_CLIENTS, Config::Int32(-1))},
        {"keep_alive_pongs", Config::Tuple(MAX_WEB_SOCKET_CLIENTS, Config::Uptime())},
        {"keep_alive_peers", Config::Tuple(MAX_WEB_SOCKET_CLIENTS, Config::Str("", 0, INET6_ADDRSTRLEN))},
        {"worker_active", Config::Uint8(WEBSOCKET_WORKER_DONE)},
        {"last_worker_run", Config::Uptime()},
        {"queue_len", Config::Uint16(0)}
    });

    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        keep_alive_fds[i] = -1;
        keep_alive_last_pong[i] = 0_us;
    }
}

void WebSockets::updateDebugState()
{
    {
        std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};

        state.get("worker_active"  )->updateUint(worker_active);
        state.get("last_worker_run")->updateUptime(last_worker_run);
        state.get("queue_len"      )->updateUint(work_queue.size());
    }

    {
        std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};

        Config *state_keep_alive_fds   = static_cast<Config *>(state.get("keep_alive_fds"));
        Config *state_keep_alive_pongs = static_cast<Config *>(state.get("keep_alive_pongs"));
        Config *state_keep_alive_peers = static_cast<Config *>(state.get("keep_alive_peers"));

        for (size_t i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
            state_keep_alive_fds->get(i)->updateInt(keep_alive_fds[i]);
            state_keep_alive_pongs->get(i)->updateUptime(keep_alive_last_pong[i]);

            if (keep_alive_fds[i] == -1) {
                state_keep_alive_peers->get(i)->updateString("NC");
                continue;
            }

            const IPAddress peer_address = tf_peer_address_of_sockfd(keep_alive_fds[i]);
            state_keep_alive_peers->get(i)->updateString(peer_address.toString());
        }
    }
}

static void session_ctx_free_fn(void *ctx)
{
    if (ctx == nullptr) {
        return;
    }

    delete static_cast<WebSocketsClient *>(ctx);
}

void WebSockets::start(const char *uri, const char *state_path, const char *supported_subprotocol, uint16_t port)
{
    if (this->running) {
        esp_system_abort("WebSockets already started");
    }

    this->running = true;

    if (!this->uri_handler_registered) {
        if (supported_subprotocol != nullptr && !address_is_in_rodata(supported_subprotocol)) {
            supported_subprotocol = perm_strdup(supported_subprotocol);
        }

        server.onWS_HTTPThread(uri, &this->httpd, [this, supported_subprotocol](WebServerRequest request) -> WebServerRequestReturnProtect {
            if (!this->running) {
                return request.send_plain(503, "Endpoint not available");
            }

            struct httpd_data *hd = static_cast<struct httpd_data *>(this->httpd);
            httpd_req_t *req = &hd->hd_req;
            struct httpd_req_aux *aux = static_cast<struct httpd_req_aux *>(req->aux);

            if (!aux->ws_handshake_detect) {
                return request.send_plain(426, "Upgrade Required: WebSockets only");
            }

            if (!this->haveFreeSlot()) {
                this->closeLRUClient_HTTPThread();
            }

            const esp_err_t ret = httpd_ws_respond_server_handshake(req, supported_subprotocol);
            if (ret != ESP_OK) {
                if (ret == ESP_ERR_NOT_FOUND)       return request.send_plain(400, "Bad Request: Missing Sec-WebSocket headers");
                if (ret == ESP_ERR_INVALID_VERSION) return request.send_plain(400, "Bad Request: Invalid WebSocket version");
                if (ret == ESP_ERR_INVALID_ARG)     return request.send_plain(500, "Server error: EINVAL");

                return WebServerRequestReturnProtect{.error = ret};
            }

            aux->sd->ws_handshake_done = true;
            aux->sd->ws_handler = WebSockets::ws_handler;
            aux->sd->ws_control_frames = true;
            aux->sd->ws_user_ctx = this;

            const int sock = httpd_req_to_sockfd(req);
            bool success = true;

            if (this->on_client_connect_fn) {
                WebSocketsClient *client = new WebSocketsClient{sock, this};

                httpd_sess_set_ctx(httpd, sock, client, session_ctx_free_fn);

                // call the client connect callback before adding the client to
                // the keep alive list to ensure that the full state is send by the
                // callback before any other message with a partial state might
                // be send to all clients known by the keep alive list
                success = this->on_client_connect_fn(client);
            }

            if (success) {
                this->keepAliveAdd(sock);
                return request.unsafe_ResponseAlreadySent(); // Don't send a HTTP response after switching to WebSockets.
            } else {
                // Returning ESP_FAIL inside the WebServerRequestReturnProtect should tell httpd to close the connection.
                return WebServerRequestReturnProtect{.error = ESP_FAIL};
            }
        }, port);

        this->uri_handler_registered = true;
    }

    this->task_ids[0] = task_scheduler.scheduleWithFixedDelay([this](){
        this->triggerHttpThread();
    }, 100_ms, 100_ms);

    this->task_ids[1] = task_scheduler.scheduleWithFixedDelay([this](){
        this->updateDebugState();
    }, 1_s, 1_s);

#if MODULE_WATCHDOG_AVAILABLE()
    this->watchdog_handle = watchdog.add(
        "websocket_worker",
        "Websocket worker was not able to start for five minutes. The control socket is probably dead.",
        WEB_SERVER_DEAD_TIMEOUT,
        WEB_SERVER_DEAD_TIMEOUT,
        true); // Force the initial deadline to WEB_SERVER_DEAD_TIMEOUT (currently 5 minutes). The firmware is accepted as good after 7 minutes.
#endif

    this->task_ids[2] = task_scheduler.scheduleWithFixedDelay([this](){
        this->pingActiveClients();
    }, 1_s, 1_s);

    this->task_ids[3] = task_scheduler.scheduleWithFixedDelay([this](){
        checkActiveClients();
    }, 100_ms, 100_ms);

    if (state_path != nullptr && !this->state_handler_registered) {
        // TODO Add pull only states to API
        server.on(state_path, HTTP_GET, [this](WebServerRequest request) {
            if (!this->running) {
                return request.send_plain(503, "Endpoint not available");
            }

            String s = this->state.to_string();
            return request.send_json(200, s);
        });

        this->state_handler_registered = true;
    }
}

static constexpr char reboot_close_payload[] = {0x03, 0xE9, 'R', 'e', 'b', 'o', 'o', 't'}; // Code 1001: Going away

void WebSockets::stop() {
    if (!this->running) {
        return;
    }

    // Mark as not running before disconnecting clients to make sure no client (re)connects.
    this->running = false;

    // Send close frame to all clients
    this->sendToAll(reboot_close_payload, std::size(reboot_close_payload), HTTPD_WS_TYPE_CLOSE);

    worker_active = WEBSOCKET_WORKER_ENQUEUED;
    httpd_queue_work(httpd, WebSockets::work, this);

    // Give http thread 200ms to send the close frames.
    for(int i = 0; i < 10 && worker_active != WEBSOCKET_WORKER_DONE; ++i)
        delay(20);

    // Disconnect clients
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for (int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] == -1)
            continue;

        this->keepAliveCloseDead_async(keep_alive_fds[i]);
    }

#if MODULE_WATCHDOG_AVAILABLE()
    watchdog.remove(this->watchdog_handle);
#endif

    for (size_t i = 0; i < ARRAY_SIZE(this->task_ids); ++i) {
        task_scheduler.cancel(this->task_ids[i]);
        this->task_ids[i] = 0;
    }
}

void WebSockets::onConnect_HTTPThread(std::function<bool(WebSocketsClient *)> &&fn)
{
    on_client_connect_fn = std::move(fn);
}

void WebSockets::onDisconnect_HTTPThread(std::function<void(WebSocketsClient *client, bool clean_close)> &&fn)
{
    on_client_disconnect_fn = std::move(fn);
}

void WebSockets::onBinaryDataReceived_HTTPThread(std::function<void(WebSocketsClient *client, httpd_ws_frame_t *ws_pkt)> &&fn)
{
    on_binary_data_received_fn = std::move(fn);
}

void WebSockets::pre_reboot() {
    this->stop();
}

void WebSockets::notify_unclean_close(struct sock_db *session)
{
    keepAliveRemove(session->fd);

    if (on_client_disconnect_fn != nullptr && session->ctx != nullptr) {
        WebSocketsClient *client = static_cast<WebSocketsClient *>(session->ctx);
        on_client_disconnect_fn(client, false);
    }
}
