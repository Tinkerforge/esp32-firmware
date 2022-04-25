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


#include "task_scheduler.h"
#include "web_server.h"

#include "esp_httpd_priv.h"

extern TaskScheduler task_scheduler;
extern WebServer server;
extern EventLog logger;

bool WebSockets::haveWork(ws_work_item *item) {
    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};

    if(work_queue.empty())
        return false;

    *item = work_queue.front();
    work_queue.pop_front();
    return true;
}

static void work(void *arg)
{
    WebSockets *ws = (WebSockets *) arg;

    ws_work_item wi;
    while (ws->haveWork(&wi)) {
        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

        ws_pkt.payload = (uint8_t *)wi.payload;
        ws_pkt.len = wi.payload_len;
        ws_pkt.type = wi.payload_len == 0 ? HTTPD_WS_TYPE_PING : HTTPD_WS_TYPE_TEXT;

        for(int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
            if (wi.fds[i] == -1) {
                continue;
            }

            if (httpd_ws_get_fd_info(wi.hd, wi.fds[i]) != HTTPD_WS_CLIENT_WEBSOCKET) {
                continue;
            }

            if (httpd_ws_send_frame_async(wi.hd, wi.fds[i], &ws_pkt) == ESP_FAIL) {
                // This closes the fd.
                // Already enqueued work items that use this fd
                // will be handled by the httpd_ws_get_fd_info check above.
                ws->keepAliveCloseDead(wi.fds[i]);
            }
        }

        wi.clear();
    }

    ws->worker_active = false;
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

            ws->keepAliveAdd(sock);

            if (ws->on_client_connect_fn) {
                ws->on_client_connect_fn(WebSocketsClient{sock, ws});
            }
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
        logger.printfln("httpd_ws_recv_frame failed to get frame len with %d", ret);
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
            logger.printfln("httpd_ws_recv_frame failed with %d", ret);
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
    for(int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] == fd) {
            // fd is alreaedy in the keep alive array. Only update last_pong to prevent instantly closing the new connection.
            // This can happen if web sockets are opened and closed rapidly (so that LWIP "reuses" the fd) and we miss a close frame.
            keep_alive_last_pong[i] = millis();
            return;
        }
    }

    for(int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] != -1)
            continue;
        keep_alive_fds[i] = fd;
        keep_alive_last_pong[i] = millis();
        return;
    }
}

void WebSockets::keepAliveRemove(int fd)
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for(int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] != fd)
            continue;
        keep_alive_fds[i] = -1;
        keep_alive_last_pong[i] = 0;
        break;
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

void WebSockets::pingActiveClients() {
    if (!this->haveActiveClient())
        return;

    // Copy over to not hold both mutexes at the same time.
    int fds[MAX_WEB_SOCKET_CLIENTS];
    {
        std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
        memcpy(fds, keep_alive_fds, sizeof(fds));
    }

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    work_queue.emplace_back(server.httpd, fds, nullptr, 0);
}

void WebSockets::checkActiveClients() {
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for(int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] == -1)
            continue;

        if (httpd_ws_get_fd_info(server.httpd, keep_alive_fds[i]) != HTTPD_WS_CLIENT_WEBSOCKET || deadline_elapsed(keep_alive_last_pong[i] + 10000)) {
            this->keepAliveCloseDead(keep_alive_fds[i]);
        }
    }
}

void WebSockets::receivedPong(int fd) {
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for(int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] != fd)
            continue;

        keep_alive_last_pong[i] = millis();
    }
}

void WebSocketsClient::send(const char *payload, size_t payload_len)
{
    ws->sendToClient(payload, payload_len, fd);
}

void WebSockets::sendToClient(const char *payload, size_t payload_len, int fd)
{
    if (httpd_ws_get_fd_info(server.httpd, fd) != HTTPD_WS_CLIENT_WEBSOCKET)
        return;

    char *payload_copy = (char *)malloc(payload_len * sizeof(char));
    if (payload_copy == nullptr) {
        return;
    }

    memcpy(payload_copy, payload, payload_len);

    int fds[MAX_WEB_SOCKET_CLIENTS] = {fd, -1, -1, -1, -1};

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    work_queue.emplace_back(server.httpd, fds, payload_copy, payload_len);
}

bool WebSockets::haveActiveClient()
{
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for(int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] != -1)
            return true;
    }
    return false;
}

bool WebSockets::haveFreeSlot() {
    std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
    for(int i = 0; i < MAX_WEB_SOCKET_CLIENTS; ++i) {
        if (keep_alive_fds[i] == -1)
            return true;
    }
    return false;
}

void WebSockets::sendToAllOwned(char *payload, size_t payload_len)
{
    if (!this->haveActiveClient()) {
        free(payload);
        return;
    }

    // Copy over to not hold both mutexes at the same time.
    int fds[MAX_WEB_SOCKET_CLIENTS];
    {
        std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
        memcpy(fds, keep_alive_fds, sizeof(fds));
    }

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    work_queue.emplace_back(server.httpd, fds, payload, payload_len);
}

void WebSockets::sendToAll(const char *payload, size_t payload_len)
{
    if (!this->haveActiveClient())
        return;

    char *payload_copy = (char *)malloc(payload_len * sizeof(char));
    if (payload_copy == nullptr) {
        return;
    }
    memcpy(payload_copy, payload, payload_len);

    // Copy over to not hold both mutexes at the same time.
    int fds[MAX_WEB_SOCKET_CLIENTS];
    {
        std::lock_guard<std::recursive_mutex> lock{keep_alive_mutex};
        memcpy(fds, keep_alive_fds, sizeof(fds));
    }

    std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
    work_queue.emplace_back(server.httpd, fds, payload_copy, payload_len);
}

void WebSockets::triggerHttpThread() {
    {
        std::lock_guard<std::recursive_mutex> lock{work_queue_mutex};
        if (work_queue.empty()) {
            return;
        }
    }

    if (worker_active) {
        return;
    }

    if (httpd_queue_work(server.httpd, work, this) != ESP_OK) {
        return;
    }

    worker_active = true;
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
        this->pingActiveClients();
    }, 1000, 1000);

    task_scheduler.scheduleWithFixedDelay([this](){
        checkActiveClients();
    }, 100, 100);
}

void WebSockets::onConnect(std::function<void(WebSocketsClient)> fn)
{
    on_client_connect_fn = fn;
}
