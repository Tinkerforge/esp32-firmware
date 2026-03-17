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

#include <cstdint>
#include <forward_list>
#include <functional>
#include <IPAddress.h>
#include <WString.h>
#include <esp_http_server.h>
#include <esp_https_server.h>
#include <TFJson.h>

#include "cert.h"
#include "generated/transport_mode.enum.h"

#include "module.h"
#include "tools/malloc.h"
#include "tools/string_builder.h"

constexpr size_t WEB_SERVER_MAX_PORTS = 3;
static_assert(WEB_SERVER_MAX_PORTS <= ESP_HTTPD_LISTEN_PORTS);

#define SEND_CHUNK_OR_FAIL(REQUEST, CHUNK) do { \
    const esp_err_t _send_err = REQUEST.sendChunk(CHUNK); \
    if (_send_err != ESP_OK) { \
        return WebServerRequestReturnProtect{.error = _send_err}; \
    } \
} while (false)

#define SEND_CHUNK_OR_FAIL_LEN(REQUEST, CHUNK, CHUNK_LENGTH) do { \
    const esp_err_t _send_err = REQUEST.sendChunk(CHUNK, CHUNK_LENGTH); \
    if (_send_err != ESP_OK) { \
        return WebServerRequestReturnProtect{.error = _send_err}; \
    } \
} while (false)

struct mbedtls_ssl_context;

// This struct is used to make sure a registered handler always calls
// one of the WebServerRequest methods that send a reponse.
// Any low-level handling errors can be passed inside,
// which will tell httpd to close the connection.
struct WebServerRequestReturnProtect {
    esp_err_t error = ESP_OK;
};

class WebServerRequest
{
public:
    WebServerRequest(httpd_req_t *req_) : req(req_) {}

    WebServerRequestReturnProtect send(uint16_t code, const char *content_type, const char *content, size_t content_len);

    inline WebServerRequestReturnProtect send_plain(uint16_t code, const char *content, size_t content_len) { return send(code, "text/plain; charset=utf-8", content, content_len); }
    inline WebServerRequestReturnProtect send_plain(uint16_t code, const char *content = "") { return send(code, "text/plain; charset=utf-8", content, strlen(content)); }
    inline WebServerRequestReturnProtect send_plain(uint16_t code, const String &content) { return send(code, "text/plain; charset=utf-8", content.c_str(), content.length()); }
    inline WebServerRequestReturnProtect send_plain(uint16_t code, const StringWriter &content) { return send(code, "text/plain; charset=utf-8", content.getPtr(), content.getLength()); }

    inline WebServerRequestReturnProtect send_json(uint16_t code, const char *content, size_t content_len) { return send(code, "application/json; charset=utf-8", content, content_len); }
    inline WebServerRequestReturnProtect send_json(uint16_t code, const char *content = "") { return send(code, "application/json; charset=utf-8", content, strlen(content)); }
    inline WebServerRequestReturnProtect send_json(uint16_t code, const String &content) { return send(code, "application/json; charset=utf-8", content.c_str(), content.length()); }
    inline WebServerRequestReturnProtect send_json(uint16_t code, const StringWriter &content) { return send(code, "application/json; charset=utf-8", content.getPtr(), content.getLength()); }
    inline WebServerRequestReturnProtect send_json(uint16_t code, const TFJsonSerializer &json) { return send(code, "application/json; charset=utf-8", json.buf, json.buf_strlen); }

    inline WebServerRequestReturnProtect send_bytes(uint16_t code, const char *content, size_t content_len) { return send(code, "application/octet-stream", content, content_len); }
    inline WebServerRequestReturnProtect send_bytes(uint16_t code, const char *content = "") { return send(code, "application/octet-stream", content, strlen(content)); }

    inline WebServerRequestReturnProtect send_html(uint16_t code, const char *content, size_t content_len) { return send(code, "text/html; charset=utf-8", content, content_len); }
    inline WebServerRequestReturnProtect send_html(uint16_t code, const char *content = "") { return send(code, "text/html; charset=utf-8", content, strlen(content)); }

    void beginChunkedResponse(uint16_t code, const char *content_type);
    inline void beginChunkedResponse_plain(uint16_t code) { return beginChunkedResponse(code, "text/plain; charset=utf-8"); }
    inline void beginChunkedResponse_json(uint16_t code) { return beginChunkedResponse(code, "application/json; charset=utf-8"); }
    inline void beginChunkedResponse_bytes(uint16_t code) { return beginChunkedResponse(code, "application/octet-stream"); }
    inline void beginChunkedResponse_html(uint16_t code) { return beginChunkedResponse(code, "text/html; charset=utf-8"); }
    inline void beginChunkedResponse_pdf(uint16_t code) { return beginChunkedResponse(code, "application/pdf"); }

    [[nodiscard]]        esp_err_t sendChunk(const char *chunk, size_t chunk_len);
    [[nodiscard]] inline esp_err_t sendChunk(const char *chunk        ) { return sendChunk(chunk,          strlen(chunk)); }
    [[nodiscard]] inline esp_err_t sendChunk(const String &chunk      ) { return sendChunk(chunk.c_str(),  chunk.length()); }
    [[nodiscard]] inline esp_err_t sendChunk(const StringWriter &chunk) { return sendChunk(chunk.getPtr(), chunk.getLength()); }

    WebServerRequestReturnProtect endChunkedResponse();

    void addResponseHeader(const char *field, const char *value);

    WebServerRequestReturnProtect requestAuthentication();

    String header(const char *header_name);

    inline size_t contentLength() { return req->content_len; }

    [[gnu::warn_unused_result]]
    int receive(char *buf, size_t buf_len);

    inline httpd_method_t method()
    {
        return static_cast<httpd_method_t>(req->method);
    }

    const char *methodString();

    inline String uri()
    {
        return String(req->uri);
    }

    inline const char *uriCStr()
    {
        return req->uri;
    }

    inline WebServerRequestReturnProtect unsafe_ResponseAlreadySent()
    {
        return WebServerRequestReturnProtect{};
    }

    IPAddress getLocalAddress();
    IPAddress getPeerAddress();

private:
    httpd_req_t *req;
    enum class ChunkedResponseState {
        NotStarted,
        Started,
        Failed,
        Ended
    };
    ChunkedResponseState chunkedResponseState = ChunkedResponseState::NotStarted;
};

using wshCallback = std::function<WebServerRequestReturnProtect(WebServerRequest request)>;
using wshUploadCallback = std::function<bool(WebServerRequest request, String filename, size_t offset, uint8_t *data, size_t len, size_t remaining)>;
using wshUploadErrorCallback = std::function<WebServerRequestReturnProtect(WebServerRequest request, int error_code)>;

struct WebServerHandler {
    WebServerHandler(const char *uri, size_t uri_len, httpd_method_t method, bool isWebsocket, bool callbackInMainThread, wshCallback &&callback, wshUploadCallback &&uploadCallback, wshUploadErrorCallback &&uploadErrorCallback);

    const char *uri;
    size_t uri_len;
    httpd_method_t method;
    bool is_websocket;
    bool accepts_upload;
    bool callbackInMainThread;
    wshCallback callback;
    wshUploadCallback uploadCallback;
    wshUploadErrorCallback uploadErrorCallback;
    WebServerHandler *next = nullptr;
};

enum class WebServerSortOrder {
    ASCENDING,
    DESCENDING,
};

struct WebServerExtraPortData {
    uint16_t port;
    bool supports_user_authentication;
    TransportMode transport_mode;
    cert_load_info cert_info;
    WebServerExtraPortData *next;
};

class WebServer final : public IModule
{
public:
    WebServer() {}

    void post_setup();
    void pre_reboot();

    void runInHTTPThread(void (*fn)(void *arg), void *arg);

    WebServerHandler *on(const char *uri, httpd_method_t method, wshCallback &&callback, uint16_t port = 0);
    WebServerHandler *on(const char *uri, httpd_method_t method, wshCallback &&callback, wshUploadCallback &&uploadCallback, wshUploadErrorCallback &&uploadErrorCallback, uint16_t port = 0);
    WebServerHandler *on_HTTPThread(const char *uri, httpd_method_t method, wshCallback &&callback, uint16_t port = 0);
    WebServerHandler *on_HTTPThread(const char *uri, httpd_method_t method, wshCallback &&callback, wshUploadCallback &&uploadCallback, wshUploadErrorCallback &&uploadErrorCallback, uint16_t port = 0);
    WebServerHandler *onWS_HTTPThread(const char *uri, httpd_handle_t *httpd_handle_out, wshCallback &&callback, uint16_t port = 0);
    void onNotAuthorized_HTTPThread(wshCallback &&callback);
    void onAuthenticate_HTTPThread(std::function<bool(WebServerRequest)> &&auth_fn);

    void register_extra_port(WebServerExtraPortData *port_data);
    bool reload_extra_port_cert(cert_load_info *load_info, uint16_t port, bool allow_fallback = true, String *cert_error = nullptr);
    bool reload_web_server_cert(int16_t cert_id, int16_t key_id, bool allow_fallback = true, String *cert_error = nullptr);

#ifdef DEBUG_FS_ENABLE
    void get_handlers(WebServerHandler **handlers, WebServerHandler **wildcard_handlers);
#endif

private:
    struct listen_port_handlers_t {
        uint16_t port;
        uint8_t supports_user_authentication : 1;
        uint8_t listen_index_0_ra_only : 1;
        bool supports_http_api;
        WebServerHandler *handlers;
        WebServerHandler *wildcard_handlers;
        mbedtls_x509_crt *own_cert;
        mbedtls_pk_context *own_cert_key;
    };

    static esp_err_t custom_open_fn(httpd_handle_t httpd, int sockfd, size_t listen_index);
    static int custom_tls_handshake_callback(mbedtls_ssl_context *ssl);

    static esp_err_t low_level_receive_handler(WebServerRequest *request, httpd_req_t *req, const WebServerHandler *handler);
    static esp_err_t low_level_handler(httpd_req_t *req);

    WebServerHandler *addHandler(uint16_t port, const char *uri, httpd_method_t method, bool isWebsocket, bool callbackInMainThread, wshCallback &&callback, wshUploadCallback &&uploadCallback, wshUploadErrorCallback &&uploadErrorCallback);

    bool load_certs(const cert_load_info *load_info, uint16_t port, bool allow_fallback, String *cert_error);

    const WebServerHandler *match_handlers(const listen_port_handlers_t *port_handlers, const char *req_uri, size_t req_uri_len, httpd_method_t method);
    const WebServerHandler *match_wildcard_handlers(const listen_port_handlers_t *port_handlers, const char *req_uri, size_t req_uri_len, httpd_method_t method);
    listen_port_handlers_t *find_handlers(uint16_t port);

    httpd_handle_t httpd = nullptr;
    wshCallback on_not_authorized;
    std::function<bool(WebServerRequest)> auth_fn;
    listen_port_handlers_t *listen_port_handlers[WEB_SERVER_MAX_PORTS] = {};
    WebServerExtraPortData *extra_ports;
};
