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

#include <forward_list>
#include <functional>
#include <IPAddress.h>
#include <WString.h>
#include <esp_http_server.h>
#include <TFJson.h>

#include "module.h"
#include "tools/string_builder.h"

// This struct is used to make sure a registered handler always calls
// one of the WebServerRequest methods that send a reponse.
struct WebServerRequestReturnProtect {
    char pad;
};

class WebServerRequest
{
public:
    WebServerRequest(httpd_req_t *req, bool keep_alive = false);

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

    int sendChunk(const char *chunk, size_t chunk_len);
    inline int sendChunk(const char *chunk) { return sendChunk(chunk, strlen(chunk)); }
    inline int sendChunk(const String &chunk) { return sendChunk(chunk.c_str(), chunk.length()); }
    inline int sendChunk(const StringWriter &chunk) { return sendChunk(chunk.getPtr(), chunk.getLength()); }

    WebServerRequestReturnProtect endChunkedResponse();

    void addResponseHeader(const char *field, const char *value);

    WebServerRequestReturnProtect requestAuthentication();

    String header(const char *header_name);

    size_t contentLength();

    [[gnu::warn_unused_result]]
    int receive(char *buf, size_t buf_len);

    int method()
    {
        return req->method;
    }

    const char *methodString()
    {
        switch (method()) {
            case HTTP_GET:
                return "GET";
            case HTTP_PUT:
                return "PUT";
            case HTTP_POST:
                return "POST";
        }
        return "";
    }

    String uri()
    {
        return String(req->uri);
    }

    const char *uriCStr()
    {
        return req->uri;
    }

    WebServerRequestReturnProtect unsafe_ResponseAlreadySent()
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
    WebServerHandler(bool callbackInMainThread,
                     wshCallback &&callback,
                     wshUploadCallback &&uploadCallback,
                     wshUploadErrorCallback &&uploadErrorCallback) : //uri(uri),
                                                                     //method(method),
                                                                     callbackInMainThread(callbackInMainThread),
                                                                     callback(std::move(callback)),
                                                                     uploadCallback(std::move(uploadCallback)),
                                                                     uploadErrorCallback(std::move(uploadErrorCallback)) {}

    bool callbackInMainThread;
    wshCallback callback;
    wshUploadCallback uploadCallback;
    wshUploadErrorCallback uploadErrorCallback;

#ifdef DEBUG_FS_ENABLE
    const char *uri;
    httpd_method_t method;
    bool accepts_upload;
#endif
};

class WebServer final : public IModule
{
public:
    WebServer() : httpd(nullptr), handlers() {}

    void post_setup();
    void pre_reboot();

    void runInHTTPThread(void (*fn)(void *arg), void *arg);

    WebServerHandler *on(const char *uri, httpd_method_t method, wshCallback &&callback);
    WebServerHandler *on(const char *uri, httpd_method_t method, wshCallback &&callback, wshUploadCallback &&uploadCallback, wshUploadErrorCallback &&uploadErrorCallback);
    WebServerHandler *on_HTTPThread(const char *uri, httpd_method_t method, wshCallback &&callback);
    WebServerHandler *on_HTTPThread(const char *uri, httpd_method_t method, wshCallback &&callback, wshUploadCallback &&uploadCallback, wshUploadErrorCallback &&uploadErrorCallback);
    void onNotAuthorized_HTTPThread(wshCallback &&callback);

    void onAuthenticate_HTTPThread(std::function<bool(WebServerRequest)> &&auth_fn)
    {
        this->auth_fn = std::move(auth_fn);
    }

    httpd_handle_t httpd;
    std::forward_list<WebServerHandler> handlers;
    int handler_count = 0;
    wshCallback on_not_authorized;

    std::function<bool(WebServerRequest)> auth_fn;

private:
    WebServerHandler *addHandler(const char *uri,
                                 httpd_method_t method,
                                 bool callbackInMainThread,
                                 wshCallback &&callback,
                                 wshUploadCallback &&uploadCallback,
                                 wshUploadErrorCallback &&uploadErrorCallback);
};
