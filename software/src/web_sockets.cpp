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

#include <esp_http_server.h>

#include "task_scheduler.h"
#include "web_server.h"

#include "esp_httpd_priv.h"

#include <mutex>
#include <deque>

extern TaskScheduler task_scheduler;
extern WebServer server;
extern EventLog logger;

static const size_t max_clients = 7;

struct ws_work_item {
    httpd_handle_t hd;
    int fd;
    char *payload;
    size_t payload_len;
    int *payload_ref_counter;

    ws_work_item(httpd_handle_t hd,
                 int fd,
                 char *payload,
                 size_t payload_len,
                 int *payload_ref_counter) :
                    hd(hd), fd(fd), payload(payload), payload_len(payload_len), payload_ref_counter(payload_ref_counter)
    {}

    void clear()
    {
        if (this->payload_ref_counter == nullptr)
            return;

        if (*this->payload_ref_counter > 0) {
            --(*this->payload_ref_counter);
        }
        if (*this->payload_ref_counter == 0) {
            free(this->payload);
            free(this->payload_ref_counter);
        }
    }
};

std::mutex work_queue_mutex;
std::deque<ws_work_item> work_queue;

static void removeFd(wss_keep_alive_t h, int fd)
{
    wss_keep_alive_remove_client(h, fd);
}

esp_err_t wss_open_fd(wss_keep_alive_t hd, int sockfd)
{
    //logger.printfln("New client connected %d", sockfd);
    return wss_keep_alive_add_client(hd, sockfd);
}

void wss_close_fd(wss_keep_alive_t hd, int sockfd)
{
    //logger.printfln("Client disconnected %d", sockfd);
    removeFd(hd, sockfd);
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
            //logger.printfln("Responding WS handshake to sock %d", aux->sd->fd);
            struct httpd_data *hd = (struct httpd_data *)server.httpd;
            esp_err_t ret = httpd_ws_respond_server_handshake(&hd->hd_req, nullptr);
            if (ret != ESP_OK) {
                return ret;
            }

            aux->sd->ws_handshake_done = true;
            aux->sd->ws_handler = ws_handler;
            aux->sd->ws_control_frames = true;
            aux->sd->ws_user_ctx = req->user_ctx;

            //logger.printfln("Handshake done, the new connection was opened");

            int sock = httpd_req_to_sockfd(req);
            WebSockets *ws = (WebSockets *)req->user_ctx;
            wss_open_fd(ws->keep_alive, sock);

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
    //logger.printfln("frame len is %d", ws_pkt.len);
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
        free(buf);
        WebSockets *ws = (WebSockets *)req->user_ctx;
        return wss_keep_alive_client_is_active(ws->keep_alive, httpd_req_to_sockfd(req));
    } else if (ws_pkt.type == HTTPD_WS_TYPE_PONG) {
        // If it was a PONG, update the keep-alive
        //logger.printfln("Received PONG message");
        free(buf);
        WebSockets *ws = (WebSockets *)req->user_ctx;
        return wss_keep_alive_client_is_active(ws->keep_alive, httpd_req_to_sockfd(req));
    } else if (ws_pkt.type == HTTPD_WS_TYPE_TEXT) {
        // If it was a TEXT message, print it
        logger.printfln("Received packet with message: %s", ws_pkt.payload);
    } else if (ws_pkt.type == HTTPD_WS_TYPE_CLOSE) {
        // If it was a CLOSE, remove it from the keep-alive list
        free(buf);
        WebSockets *ws = (WebSockets *)req->user_ctx;
        wss_close_fd(ws->keep_alive, httpd_req_to_sockfd(req));
        return ESP_OK;
    }
    free(buf);
    return ESP_OK;
}

bool client_not_alive_cb(wss_keep_alive_t h, int fd)
{
    //logger.printfln("Client not alive, closing fd %d", fd);
    httpd_sess_trigger_close(wss_keep_alive_get_user_ctx(h), fd);

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
    wss_close_fd(h, fd);
    return true;
}

static void work(void *arg)
{
    while (!work_queue.empty()) {
        ws_work_item wi = work_queue.front();
        {
            std::lock_guard<std::mutex> lock{work_queue_mutex};
            work_queue.pop_front();
        }
        if (httpd_ws_get_fd_info(wi.hd, wi.fd) != HTTPD_WS_CLIENT_WEBSOCKET) {
            wi.clear();
            continue;
        }

        httpd_ws_frame_t ws_pkt;
        memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));

        ws_pkt.payload = (uint8_t *)wi.payload;
        ws_pkt.len = wi.payload_len;
        ws_pkt.type = wi.payload_len == 0 ? HTTPD_WS_TYPE_PING : HTTPD_WS_TYPE_TEXT;

        auto result = httpd_ws_send_frame_async(wi.hd, wi.fd, &ws_pkt);
        if (result != ESP_OK) {
            printf("failed to send %s frame to fd %d: %d\n", wi.payload_len == 0 ? "HTTPD_WS_TYPE_PING" : "HTTPD_WS_TYPE_TEXT", wi.fd, result);
        }

        wi.clear();
    }
}

bool check_client_alive_cb(wss_keep_alive_t h, int fd)
{
    httpd_handle_t hd = (httpd_handle_t)wss_keep_alive_get_user_ctx(h);

    {
        std::lock_guard<std::mutex> lock{work_queue_mutex};
        work_queue.emplace_back(hd, fd, nullptr, 0, nullptr);
    }
    return httpd_queue_work(hd, work, nullptr) == ESP_OK;
}

void WebSocketsClient::send(const char *payload, size_t payload_len)
{
    ws->sendToClient(payload, payload_len, fd);
}

void WebSockets::sendToClient(const char *payload, size_t payload_len, int sock)
{
    httpd_handle_t httpd = server.httpd;
    int *payload_ref_counter = (int *)malloc(sizeof(int));
    if (payload_ref_counter == nullptr)
        return;
    char *payload_copy = (char *)malloc(payload_len * sizeof(char));
    if (payload_copy == nullptr) {
        free(payload_ref_counter);
        return;
    }

    memcpy(payload_copy, payload, payload_len);

    *payload_ref_counter = 1;

    {
        // TODO: locking here means, that we assume, the sock fd is valid up to now.
        // This holds true if sendToClient is only called from the onClientConnect callback,
        // i.e. from the same thread.
        std::lock_guard<std::mutex> lock{work_queue_mutex};
        work_queue.emplace_back(httpd, sock, payload_copy, payload_len, payload_ref_counter);
    }

    if (httpd_queue_work(httpd, work, nullptr) != ESP_OK) {
        logger.printfln("httpd_queue_work failed!");
    }
}

bool WebSockets::haveActiveClient()
{
    httpd_handle_t httpd = server.httpd;
    size_t clients = 7;
    int client_fds[7];

    auto result = httpd_get_client_list(httpd, &clients, client_fds);
    if (result != ESP_OK) {
        logger.printfln("httpd_get_client_list failed! %d", result);
        return false;
    }

    int active_clients = 0;
    int http_clients = 0;
    int invalid_clients = 0;
    int unknown_clients = 0;
    //printf("payload (len: %d) after copy: %s\n", payload_len, payload_copy);

    for (size_t i = 0; i < clients; ++i)
        if (httpd_ws_get_fd_info(httpd, client_fds[i]) == HTTPD_WS_CLIENT_WEBSOCKET)
            ++active_clients;
        else if (httpd_ws_get_fd_info(httpd, client_fds[i]) == HTTPD_WS_CLIENT_HTTP)
            ++http_clients;
        else if (httpd_ws_get_fd_info(httpd, client_fds[i]) == HTTPD_WS_CLIENT_INVALID)
            ++invalid_clients;
        else
            ++unknown_clients;

    return active_clients != 0;
}

void WebSockets::sendToAllOwned(char *payload, size_t payload_len)
{
    httpd_handle_t httpd = server.httpd;
    size_t clients = 7;
    int client_fds[7];

    auto result = httpd_get_client_list(httpd, &clients, client_fds);
    if (result != ESP_OK) {
        logger.printfln("httpd_get_client_list failed! %d", result);
        return;
    }

    int active_clients = 0;
    int http_clients = 0;
    int invalid_clients = 0;
    int unknown_clients = 0;
    //printf("payload (len: %d) after copy: %s\n", payload_len, payload_copy);

    for (size_t i = 0; i < clients; ++i)
        if (httpd_ws_get_fd_info(httpd, client_fds[i]) == HTTPD_WS_CLIENT_WEBSOCKET)
            ++active_clients;
        else if (httpd_ws_get_fd_info(httpd, client_fds[i]) == HTTPD_WS_CLIENT_HTTP)
            ++http_clients;
        else if (httpd_ws_get_fd_info(httpd, client_fds[i]) == HTTPD_WS_CLIENT_INVALID)
            ++invalid_clients;
        else
            ++unknown_clients;

    /*printf("active_clients %d, http_clients %d, invalid_clients %d, unknown_clients %d\n",
            active_clients,
            http_clients,
            invalid_clients,
            unknown_clients);*/

    if (active_clients == 0)
        return;

    int *payload_ref_counter = (int *)malloc(sizeof(int));
    if (payload_ref_counter == nullptr)
        return;

    *payload_ref_counter = active_clients;

    std::lock_guard<std::mutex> lock{work_queue_mutex};
    for (size_t i = 0; i < clients; ++i) {
        int sock = client_fds[i];
        if (httpd_ws_get_fd_info(httpd, sock) != HTTPD_WS_CLIENT_WEBSOCKET) {
            continue;
        }

        work_queue.emplace_back(httpd, sock, payload, payload_len, payload_ref_counter);

        if (httpd_queue_work(httpd, work, nullptr) != ESP_OK) {
            logger.printfln("httpd_queue_work failed!");
        }
    }
}

void WebSockets::sendToAll(const char *payload, size_t payload_len)
{
    httpd_handle_t httpd = server.httpd;
    size_t clients = 7;
    int client_fds[7];

    auto result = httpd_get_client_list(httpd, &clients, client_fds);
    if (result != ESP_OK) {
        logger.printfln("httpd_get_client_list failed! %d", result);
        return;
    }

    int active_clients = 0;
    int http_clients = 0;
    int invalid_clients = 0;
    int unknown_clients = 0;
    //printf("payload (len: %d) after copy: %s\n", payload_len, payload_copy);

    for (size_t i = 0; i < clients; ++i)
        if (httpd_ws_get_fd_info(httpd, client_fds[i]) == HTTPD_WS_CLIENT_WEBSOCKET)
            ++active_clients;
        else if (httpd_ws_get_fd_info(httpd, client_fds[i]) == HTTPD_WS_CLIENT_HTTP)
            ++http_clients;
        else if (httpd_ws_get_fd_info(httpd, client_fds[i]) == HTTPD_WS_CLIENT_INVALID)
            ++invalid_clients;
        else
            ++unknown_clients;

    /*printf("active_clients %d, http_clients %d, invalid_clients %d, unknown_clients %d\n",
            active_clients,
            http_clients,
            invalid_clients,
            unknown_clients);*/

    if (active_clients == 0)
        return;

    int *payload_ref_counter = (int *)malloc(sizeof(int));
    if (payload_ref_counter == nullptr)
        return;

    *payload_ref_counter = active_clients;

    char *payload_copy = (char *)malloc(payload_len * sizeof(char));
    if (payload_copy == nullptr) {
        free(payload_ref_counter);
        return;
    }

    memcpy(payload_copy, payload, payload_len);

    std::lock_guard<std::mutex> lock{work_queue_mutex};
    for (size_t i = 0; i < clients; ++i) {
        int sock = client_fds[i];
        if (httpd_ws_get_fd_info(httpd, sock) != HTTPD_WS_CLIENT_WEBSOCKET) {
            continue;
        }

        work_queue.emplace_back(httpd, sock, payload_copy, payload_len, payload_ref_counter);

        if (httpd_queue_work(httpd, work, nullptr) != ESP_OK) {
            logger.printfln("httpd_queue_work failed!");
        }
    }
}

void WebSockets::start(const char *uri)
{
    wss_keep_alive_config_t keep_alive_config = {};
    // As defined in KEEP_ALIVE_CONFIG_DEFAULT()
    keep_alive_config.task_stack_size = 2048;
    keep_alive_config.task_prio = tskIDLE_PRIORITY + 1;
    keep_alive_config.keep_alive_period_ms = 5000;
    keep_alive_config.not_alive_after_ms = 10000;

    keep_alive_config.max_clients = max_clients;
    keep_alive_config.client_not_alive_cb = client_not_alive_cb;
    keep_alive_config.check_client_alive_cb = check_client_alive_cb;

    this->keep_alive = wss_keep_alive_start(&keep_alive_config);

    httpd_handle_t httpd = server.httpd;

    httpd_uri_t ws = {};
    ws.uri = uri;
    ws.method = HTTP_GET;
    ws.handler = ws_handler;
    ws.user_ctx = this;
    ws.is_websocket = false;
    ws.handle_ws_control_frames = true;

    httpd_register_uri_handler(httpd, &ws);
    wss_keep_alive_set_user_ctx(keep_alive, httpd);

    //server.onConnect([this](int fd) {removeFd(this->keep_alive, fd);});
    //server.onDisconnect([this](int fd) {removeFd(this->keep_alive, fd);});
}

void WebSockets::onConnect(std::function<void(WebSocketsClient)> fn)
{
    on_client_connect_fn = fn;
}
