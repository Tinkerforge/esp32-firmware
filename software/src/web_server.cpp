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

#include "web_server.h"

#include "esp_log.h"
#include "esp_httpd_priv.h"

#include "task_scheduler.h"
#include "digest_auth.h"

#include "modules.h"

#include "tools.h"

#include <memory>

#define MAX_URI_HANDLERS 128

// Global definition here to match the declaration in web_server.h.
WebServer server;

void WebServer::start()
{
    if (this->httpd != nullptr) {
        return;
    }

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;
    config.stack_size = 8192;
    config.max_uri_handlers = MAX_URI_HANDLERS;
    config.global_user_ctx = this;
    config.max_open_sockets = 10;

    config.enable_so_linger = true;
    config.linger_timeout = 0;
#if MODULE_NETWORK_AVAILABLE()
    config.server_port = network.config.get("web_server_port")->asUint();
#endif

#if MODULE_HTTP_AVAILABLE()
    config.uri_match_fn = custom_uri_match;
#endif

    /*config.task_priority = tskIDLE_PRIORITY+7;
    config.core_id = 1;*/

    // Start the httpd server
    auto result = httpd_start(&this->httpd, &config);
    if (result != ESP_OK) {
        httpd = nullptr;
        logger.printfln("Failed to start web server! %s (%d)", esp_err_to_name(result), result);
        return;
    }
}

struct UserCtx {
    WebServer *server;
    WebServerHandler *handler;
};

static esp_err_t low_level_handler(httpd_req_t *req)
{
    auto ctx = (UserCtx *)req->user_ctx;
    auto request = WebServerRequest{req};

    if (ctx->server->auth_fn && !ctx->server->auth_fn(request)) {
        if (ctx->server->on_not_authorized) {
            ctx->server->on_not_authorized(request);
            return ESP_OK;
        }
        request.requestAuthentication();
        return ESP_OK;
    }

    ctx->handler->callback(request);
    return ESP_OK;
}

WebServerHandler *WebServer::on(const char *uri, httpd_method_t method, wshCallback callback)
{
    if (handler_count >= MAX_URI_HANDLERS) {
        logger.printfln("Can't add WebServer handler for %s: %d handlers already registered. Please increase MAX_URI_HANDLERS.", uri, handler_count);
        return nullptr;
    }

    handlers.emplace_front(uri, method, callback, wshUploadCallback());
    ++handler_count;
    WebServerHandler *result = &handlers.front();

    UserCtx *user_ctx = (UserCtx *)malloc(sizeof(UserCtx));
    user_ctx->server = this;
    user_ctx->handler = result;

    httpd_uri_t ll_handler = {};
    ll_handler.uri       = uri;
    ll_handler.method    = method;
    ll_handler.handler   = low_level_handler;
    ll_handler.user_ctx  = (void*)user_ctx;

    httpd_register_uri_handler(httpd, &ll_handler);
    return result;
}

static const size_t SCRATCH_BUFSIZE = 2048;

static esp_err_t low_level_upload_handler(httpd_req_t *req)
{
    auto ctx = (UserCtx *)req->user_ctx;
    auto request = WebServerRequest{req};
    if (ctx->server->auth_fn && !ctx->server->auth_fn(request)) {
        if (ctx->server->on_not_authorized) {
            ctx->server->on_not_authorized(request);
            return ESP_OK;
        }
        request.requestAuthentication();
        return ESP_OK;
    }

    size_t remaining = req->content_len;
    size_t index = 0;

    auto scratch_buf = heap_alloc_array<uint8_t>(SCRATCH_BUFSIZE);

    while (remaining > 0) {
        int received = httpd_req_recv(req, (char *)scratch_buf.get(), MIN(remaining, SCRATCH_BUFSIZE));
        // Retry if timeout occurred
        if (received == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;
        }

        if (received <= 0) {
            struct httpd_req_aux *ra = (struct httpd_req_aux *)req->aux;

            logger.printfln("File reception failed (%d fd %d errno %d %s)!\n", received, ra->sd->fd, errno, strerror(errno));
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;
        }

        remaining -= received;
        if (!ctx->handler->uploadCallback(request, "not implemented", index, scratch_buf.get(), received, remaining == 0)) {
            return ESP_FAIL;
        }

        index += received;
    }

    ctx->handler->callback(request);
    return ESP_OK;
}

WebServerHandler *WebServer::on(const char *uri, httpd_method_t method, wshCallback callback, wshUploadCallback uploadCallback)
{
    if (handler_count >= MAX_URI_HANDLERS) {
        logger.printfln("Can't add WebServer handler for %s: %d handlers already registered. Please increase MAX_URI_HANDLERS.", uri, handler_count);
        return nullptr;
    }

    handlers.emplace_front(uri, method, callback, uploadCallback);
    ++handler_count;
    WebServerHandler *result = &handlers.front();

    UserCtx *user_ctx = (UserCtx *)malloc(sizeof(UserCtx));
    user_ctx->server = this;
    user_ctx->handler = result;

    httpd_uri_t ll_handler = {};
    ll_handler.uri       = uri;
    ll_handler.method    = method;
    ll_handler.handler   = low_level_upload_handler;
    ll_handler.user_ctx  = (void*)user_ctx;

    httpd_register_uri_handler(httpd, &ll_handler);
    return result;
}

void WebServer::onNotAuthorized(wshCallback callback)
{
    this->on_not_authorized = callback;
}

// From: https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
const char *httpStatusCodeToString(int code)
{
    switch (code) {
        case 100: return "100 Continue";                        //[RFC7231, Section 6.2.1]
        case 101: return "101 Switching Protocols";             //[RFC7231, Section 6.2.2]
        case 102: return "102 Processing";                      //[RFC2518]
        case 103: return "103 Early Hints";                     //[RFC8297]
        //104-199,Unassigned,

        case 200: return "200 OK";                              //[RFC7231, Section 6.3.1]
        case 201: return "201 Created";                         //[RFC7231, Section 6.3.2]
        case 202: return "202 Accepted";                        //[RFC7231, Section 6.3.3]
        case 203: return "203 Non-Authoritative Information";   //[RFC7231, Section 6.3.4]
        case 204: return "204 No Content";                      //[RFC7231, Section 6.3.5]
        case 205: return "205 Reset Content";                   //[RFC7231, Section 6.3.6]
        case 206: return "206 Partial Content";                 //[RFC7233, Section 4.1]
        case 207: return "207 Multi-Status";                    //[RFC4918]
        case 208: return "208 Already Reported";                //[RFC5842]
        //209-225,Unassigned,
        case 226: return "226 IM Used";                         //[RFC3229]
        //227-299,Unassigned,

        case 300: return "300 Multiple Choices";                //[RFC7231, Section 6.4.1]
        case 301: return "301 Moved Permanently";               //[RFC7231, Section 6.4.2]
        case 302: return "302 Found";                           //[RFC7231, Section 6.4.3]
        case 303: return "303 See Other";                       //[RFC7231, Section 6.4.4]
        case 304: return "304 Not Modified";                    //[RFC7232, Section 4.1]
        case 305: return "305 Use Proxy";                       //[RFC7231, Section 6.4.5]
        case 306: return "306 (Unused)";                        //[RFC7231, Section 6.4.6]
        case 307: return "307 Temporary Redirect";              //[RFC7231, Section 6.4.7]
        case 308: return "308 Permanent Redirect";              //[RFC7538]
        //309-399,Unassigned,

        case 400: return "400 Bad Request";                     //[RFC7231, Section 6.5.1]
        case 401: return "401 Unauthorized";                    //[RFC7235, Section 3.1]
        case 402: return "402 Payment Required";                //[RFC7231, Section 6.5.2]
        case 403: return "403 Forbidden";                       //[RFC7231, Section 6.5.3]
        case 404: return "404 Not Found";                       //[RFC7231, Section 6.5.4]
        case 405: return "405 Method Not Allowed";              //[RFC7231, Section 6.5.5]
        case 406: return "406 Not Acceptable";                  //[RFC7231, Section 6.5.6]
        case 407: return "407 Proxy Authentication Required";   //[RFC7235, Section 3.2]
        case 408: return "408 Request Timeout";                 //[RFC7231, Section 6.5.7]
        case 409: return "409 Conflict";                        //[RFC7231, Section 6.5.8]
        case 410: return "410 Gone";                            //[RFC7231, Section 6.5.9]
        case 411: return "411 Length Required";                 //[RFC7231, Section 6.5.10]
        case 412: return "412 Precondition Failed";             //[RFC7232, Section 4.2][RFC8144, Section 3.2]
        case 413: return "413 Payload Too Large";               //[RFC7231, Section 6.5.11]
        case 414: return "414 URI Too Long";                    //[RFC7231, Section 6.5.12]
        case 415: return "415 Unsupported Media Type";          //[RFC7231, Section 6.5.13][RFC7694, Section 3]
        case 416: return "416 Range Not Satisfiable";           //[RFC7233, Section 4.4]
        case 417: return "417 Expectation Failed";              //[RFC7231, Section 6.5.14]
        // Technically unasigned, but what if someone wants to build a smart tea kettle?
        case 418: return "418 I'm a teapot";                    //[RFC2324, Section 2.3.2][RFC7168, Section 2.3.3]
        //419-420,Unassigned,
        case 421: return "421 Misdirected Request";             //[RFC7540, Section 9.1.2]
        case 422: return "422 Unprocessable Entity";            //[RFC4918]
        case 423: return "423 Locked";                          //[RFC4918]
        case 424: return "424 Failed Dependency";               //[RFC4918]
        case 425: return "425 Too Early";                       //[RFC8470]
        case 426: return "426 Upgrade Required";                //[RFC7231, Section 6.5.15]
        //427,Unassigned,
        case 428: return "428 Precondition Required";           //[RFC6585]
        case 429: return "429 Too Many Requests";               //[RFC6585]
        //430,Unassigned,
        case 431: return "431 Request Header Fields Too Large"; //[RFC6585]
        //432-450,Unassigned,
        case 451: return "451 Unavailable For Legal Reasons";   //[RFC7725]
        //452-499,Unassigned,

        case 500: return "500 Internal Server Error";           //[RFC7231, Section 6.6.1]
        case 501: return "501 Not Implemented";                 //[RFC7231, Section 6.6.2]
        case 502: return "502 Bad Gateway";                     //[RFC7231, Section 6.6.3]
        case 503: return "503 Service Unavailable";             //[RFC7231, Section 6.6.4]
        case 504: return "504 Gateway Timeout";                 //[RFC7231, Section 6.6.5]
        case 505: return "505 HTTP Version Not Supported";      //[RFC7231, Section 6.6.6]
        case 506: return "506 Variant Also Negotiates";         //[RFC2295]
        case 507: return "507 Insufficient Storage";            //[RFC4918]
        case 508: return "508 Loop Detected";                   //[RFC5842]
        //509,Unassigned,
        case 510: return "510 Not Extended";                    //[RFC2774]
        case 511: return "511 Network Authentication Required"; //[RFC6585]
        //512-599,Unassigned,

        default:  return "";
    }
}

WebServerRequestReturnProtect WebServerRequest::send(uint16_t code, const char *content_type, const char *content, ssize_t content_len)
{
    auto result = httpd_resp_set_type(req, content_type);
    if (result != ESP_OK) {
        printf("Failed to set response type: %d\n", result);
        return WebServerRequestReturnProtect{};
    }

    result = httpd_resp_set_status(req, httpStatusCodeToString(code));
    if (result != ESP_OK) {
        printf("Failed to set response status: %d\n", result);
        return WebServerRequestReturnProtect{};
    }

    struct httpd_req_aux *ra = (struct httpd_req_aux *)req->aux;
    int nodelay = 1;
    if (code >= 400) {
        // Copied over from esp-idf/components/esp_http_server/src/httpd_txrx.c
        // We want to set TCP_NODELAY but keep our (complete) set of HTTP status codes.

        /* Use TCP_NODELAY option to force socket to send data in buffer
         * This ensures that the error message is sent before the socket
         * is closed */
        if (setsockopt(ra->sd->fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
            /* If failed to turn on TCP_NODELAY, throw warning and continue */
            logger.printfln("error calling setsockopt : %d", errno);
            nodelay = 0;
        }
    }

    result = httpd_resp_send(req, content, content_len);

    if (code >= 400) {
        // Copied over from esp-idf/components/esp_http_server/src/httpd_txrx.c
        // We want to set TCP_NODELAY but keep our (complete) set of HTTP status codes.

        /* If TCP_NODELAY was set successfully above, time to disable it */
        if (nodelay == 1) {
            nodelay = 0;
            if (setsockopt(ra->sd->fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
                /* If failed to turn off TCP_NODELAY, throw error and
                 * return failure to signal for socket closure */
                logger.printfln("error calling setsockopt : %d", errno);
                result = ESP_ERR_INVALID_STATE;
            }
        }
    }

    if (result != ESP_OK) {
        printf("Failed to send response: %d\n", result);
    }
    return WebServerRequestReturnProtect{};
}

void WebServerRequest::beginChunkedResponse(uint16_t code, const char *content_type)
{
    auto result = httpd_resp_set_type(req, content_type);
    if (result != ESP_OK) {
        printf("Failed to set response type: %d\n", result);
        return;
    }

    result = httpd_resp_set_status(req, httpStatusCodeToString(code));
    if (result != ESP_OK) {
        printf("Failed to set response status: %d\n", result);
        return;
    }
}

int WebServerRequest::sendChunk(const char *chunk, ssize_t chunk_len)
{
    auto result = httpd_resp_send_chunk(req, chunk, chunk_len);
    if (result != ESP_OK) {
        printf("Failed to send response chunk: %d\n", result);
    }
    return result;
}

WebServerRequestReturnProtect WebServerRequest::endChunkedResponse()
{
    auto result = httpd_resp_send_chunk(req, nullptr, 0);
    if (result != ESP_OK) {
        printf("Failed to end chunked response: %d\n", result);
    }
    return WebServerRequestReturnProtect{};
}

void WebServerRequest::addResponseHeader(const char *field, const char *value)
{
    auto result = httpd_resp_set_hdr(req, field, value);
    if (result != ESP_OK) {
        printf("Failed to set response header: %d\n", result);
        return;
    }
}

WebServerRequestReturnProtect WebServerRequest::requestAuthentication()
{
    String payload = "Digest ";
    payload.concat(requestDigestAuthentication(nullptr));
    addResponseHeader("WWW-Authenticate", payload.c_str());
    return send(401);
}

String WebServerRequest::header(const char *header_name)
{
    auto buf_len = httpd_req_get_hdr_value_len(req, header_name) + 1;
    if (buf_len == 1)
        return "";

    CoolString result;
    result.reserve(buf_len);
    char *buf = result.begin();
    /* Copy null terminated value string into buffer */
    if (httpd_req_get_hdr_value_str(req, header_name, buf, buf_len) != ESP_OK) {
        return "";
    }
    result.setLength(buf_len);
    return result;
}

size_t WebServerRequest::contentLength()
{
    return req->content_len;
}

char *WebServerRequest::receive()
{
    char *result = (char *)malloc(sizeof(char) * contentLength());
    httpd_req_recv(req, result, contentLength());
    return result;
}

int WebServerRequest::receive(char *buf, size_t buf_len)
{
    if (buf_len < contentLength())
        return -1;
    return httpd_req_recv(req, buf, contentLength());
}

WebServerRequest::WebServerRequest(httpd_req_t *req, bool keep_alive) : req(req)
{
    if (!keep_alive)
        this->addResponseHeader("Connection", "close");
}
