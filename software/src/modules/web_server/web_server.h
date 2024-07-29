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

#include <esp_http_server.h>
#include <forward_list>
#include <functional>
#include <Arduino.h>

#include "module.h"

// This struct is used to make sure a registered handler always calls
// one of the WebServerRequest methods that send a reponse.
struct WebServerRequestReturnProtect {
    char pad;
};

class WebServerRequest
{
public:
    WebServerRequest(httpd_req_t *req, bool keep_alive = false);

    WebServerRequestReturnProtect send(uint16_t code, const char *content_type = "text/plain; charset=utf-8", const char *content = "", ssize_t content_len = HTTPD_RESP_USE_STRLEN);

    void beginChunkedResponse(uint16_t code, const char *content_type = "text/plain; charset=utf-8");

    int sendChunk(const char *chunk, ssize_t chunk_len);

    WebServerRequestReturnProtect endChunkedResponse();

    void addResponseHeader(const char *field, const char *value);

    WebServerRequestReturnProtect requestAuthentication();

    String header(const char *header_name);

    size_t contentLength();

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

private:
    httpd_req_t *req;
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
                                                                     callback(std::forward<wshCallback>(callback)),
                                                                     uploadCallback(std::forward<wshUploadCallback>(uploadCallback)),
                                                                     uploadErrorCallback(std::forward<wshUploadErrorCallback>(uploadErrorCallback)) {}

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

    void onAuthenticate_HTTPThread(std::function<bool(WebServerRequest)> auth_fn)
    {
        this->auth_fn = auth_fn;
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
