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

#include <esp_system.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "digest_auth.h"
#include "cool_string.h"
#include "esp_httpd_priv.h"
#include "tools/malloc.h"
#include "tools/memory.h"
#include "tools/net.h"
#include "options.h"

#include "sdkconfig.h"
#ifndef CONFIG_ESP_HTTPS_SERVER_ENABLE
#define CONFIG_ESP_HTTPS_SERVER_ENABLE 0
#endif
#ifndef MODULE_CERTS_AVAILABLE
#define MODULE_CERTS_AVAILABLE() 0
#endif

#if CONFIG_ESP_HTTPS_SERVER_ENABLE
#define HTTPS_AVAILABLE() 1
#else
#define HTTPS_AVAILABLE() 0
#endif

#if OPTIONS_WEB_SERVER_HTTPS_ENABLED() && !HTTPS_AVAILABLE()
#error "HTTPS selected in options but is not available"
#endif

#if HTTPS_AVAILABLE()
#include <esp_https_server.h>
#endif

#include "gcc_warnings.h"

#define HTTPD_STACK_SIZE 8192

static void dummy_free_fn(void *) {}

// Only one handler will be registered with the httpd and it will match everything.
static bool custom_uri_match(const char *ref_uri, const char *in_uri, size_t len)
{
    return true;
}

static void custom_close_fn(httpd_handle_t hd, struct sock_db *session)
{
    // If a close_fn is registered, httpd won't close the fd by itself.
    close(session->fd);

#if MODULE_WS_AVAILABLE()
    if (session->ws_handshake_done && !session->ws_control_frames) {
        // Upon receiving a WS close frame, web_sockets sets ws_control_frames to false to mark this connection as handled.
        // If ws_control_frames is false, the websocket close event was already handled.
        return;
    }

    if (session->ws_user_ctx) {
        WebSockets *web_sockets = static_cast<WebSockets *>(session->ws_user_ctx);
        web_sockets->notify_unclean_close(session);
    }
#endif
}

#if HTTPS_AVAILABLE()
static bool load_certs_with_fallback(httpd_ssl_config_t *ssl_config, const cert_load_info *load_info, Cert *cert)
{
    if (!cert->load_external_with_internal_fallback(load_info)) {
        return false;
    }

    cert->get_data(&ssl_config->servercert, &ssl_config->servercert_len, &ssl_config->prvtkey_pem, &ssl_config->prvtkey_len);

    ssl_config->servercert_len += 1;
    ssl_config->prvtkey_len    += 1;

    return true;
}
#endif

void WebServer::post_setup()
{
    if (this->httpd != nullptr) {
        return;
    }

    listen_port_handlers_t *default_handlers = static_cast<listen_port_handlers_t *>(perm_aligned_alloc(alignof(listen_port_handlers_t), sizeof(listen_port_handlers_t), DRAM));
    *default_handlers = {
        .port = 0,
        .supports_http_api = true,
        .handlers = nullptr,
        .wildcard_handlers = nullptr,
    };

    // Certificate buffers must live until httpd has started.
    Cert certificates[WEB_SERVER_MAX_PORTS] = {};

#if HTTPS_AVAILABLE()
    httpd_ssl_config_t ssl_configs[WEB_SERVER_MAX_PORTS] = {
        HTTPD_SSL_PORT_CONFIG_DEFAULT(),
        HTTPD_SSL_PORT_CONFIG_DEFAULT(),
        HTTPD_SSL_PORT_CONFIG_DEFAULT(),
    };
    size_t ssl_configs_used = 0;

    bool http_fallback_needed = false;
    bool https_multiport_needed = false;

#if MODULE_NETWORK_AVAILABLE()
    const TransportMode transport_mode = network.get_transport_mode();
#else
    const TransportMode transport_mode = TransportMode::InsecureAndSecure;
#endif

    // === Secure or mixed mode ===

    if (transport_mode != TransportMode::Insecure) {
        httpd_ssl_config_t *ssl_config = ssl_configs + ssl_configs_used;

        int8_t cert_id = -1;
        int8_t key_id  = -1;

#if MODULE_NETWORK_AVAILABLE()
        cert_id = network.get_cert_id();
        key_id  = network.get_key_id();
#endif

        const cert_load_info load_info = {
            .cert_id = cert_id,
            .key_id = key_id,
            .cert_path = "/web_server/cert",
            .key_path  = "/web_server/key",
            .generator_fn = Cert::default_certificate_generator_fn,
        };

        const bool cert_ok = load_certs_with_fallback(ssl_config, &load_info, certificates + ssl_configs_used);

        if (cert_ok) {
            ssl_config->transport_mode = HTTPD_SSL_TRANSPORT_SECURE;

#if MODULE_NETWORK_AVAILABLE()
            ssl_config->port_secure = network.get_web_server_port_secure();
#else
            ssl_config->port_secure = 443;
#endif

            listen_port_handlers[ssl_configs_used] = default_handlers;
            ssl_configs_used++;
            https_multiport_needed = true;
        } else {
            http_fallback_needed = true;
        }
    }

    // === Insecure or mixed mode or fallback when secure-only is not available ===

    if (transport_mode != TransportMode::Secure || http_fallback_needed) {
        httpd_ssl_config_t *ssl_config = ssl_configs + ssl_configs_used;

        ssl_config->transport_mode = HTTPD_SSL_TRANSPORT_INSECURE;

#if MODULE_NETWORK_AVAILABLE()
        ssl_config->port_insecure = network.get_web_server_port();
#else
        ssl_config->port_insecure = 80;
#endif

        listen_port_handlers[ssl_configs_used] = default_handlers;
        ssl_configs_used++;
    }

    while (extra_ports != nullptr) {
        WebServerExtraPortData *extra_port = extra_ports;

        if (ssl_configs_used >= std::size(ssl_configs)) {
            logger.printfln("Cannot listen on extra port %hu: All ports in use", extra_port->port);
            break;
        } else {
            do {
                httpd_ssl_config_t *ssl_config = ssl_configs + ssl_configs_used;

                if (extra_port->transport_mode == TransportMode::Insecure) {
                    ssl_config->transport_mode = HTTPD_SSL_TRANSPORT_INSECURE;
                    ssl_config->port_insecure = extra_port->port;
                } else {
                    ssl_config->transport_mode = HTTPD_SSL_TRANSPORT_SECURE;
                    ssl_config->port_secure = extra_port->port;

                    if (!load_certs_with_fallback(ssl_config, &extra_port->cert_info, certificates + ssl_configs_used)) {
                        logger.printfln("Cannot listen on extra port %hu: Failed to load certificate", extra_port->port);
                        break;
                    }
                }

                listen_port_handlers_t *port_handler = static_cast<listen_port_handlers_t *>(perm_aligned_alloc(alignof(listen_port_handlers_t), sizeof(listen_port_handlers_t), DRAM));
                *port_handler = {
                    .port = extra_port->port,
                    .supports_http_api = false,
                    .handlers = nullptr,
                    .wildcard_handlers = nullptr,
                };

                listen_port_handlers[ssl_configs_used] = port_handler;
                ssl_configs_used++;

                https_multiport_needed = true;
            } while (false);
        }

        extra_ports = extra_port->next;
        extra_port->next = nullptr;
        free(extra_port);
    }
#else // HTTPS_AVAILABLE()
    listen_port_handlers[0] = default_handlers;
#endif // HTTPS_AVAILABLE()

    // === Basic httpd config ===

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    #pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
    httpd_config_t httpd_config = HTTPD_DEFAULT_CONFIG();
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

    httpd_config.lru_purge_enable = true;
    httpd_config.stack_size = HTTPD_STACK_SIZE;
    httpd_config.max_uri_handlers = 1; // Only one handler will be registered directly with the httpd.
    httpd_config.global_user_ctx = this;
    // httpd_stop calls free on the pointer passed as global_user_ctx if we don't override the free_fn.
    httpd_config.global_user_ctx_free_fn = dummy_free_fn;
    httpd_config.max_open_sockets = 10;
    httpd_config.enable_so_linger = true;
    httpd_config.linger_timeout = 1; // Try to get WS close, TLS close and error messages out. This is mostly necessary for connections over WireGuard/remote access. Only blocks as long as necessary but capped to 1 second.
    httpd_config.uri_match_fn = custom_uri_match;
    httpd_config.close_fn = custom_close_fn;

#if MODULE_NETWORK_AVAILABLE()
    httpd_config.server_port = network.get_web_server_port(); // Only used in single-port-mode
#endif


    // Start the httpd server
#if HTTPS_AVAILABLE()
    if (https_multiport_needed) {
        logger.printfln("Starting multi-port server with %zu port%s", ssl_configs_used, ssl_configs_used == 1 ? "": "s");

        esp_err_t result_https = httpd_ssl_start_multi(&this->httpd, &httpd_config, ssl_configs, ssl_configs_used);
        if (result_https != ESP_OK) {
            httpd = nullptr;
            logger.printfln("Failed to start https web server: %s (0x%X)", esp_err_to_name(result_https), static_cast<unsigned>(result_https));
            return;
        }
    } else {
#endif
        logger.printfln("Starting single-port server on port %hhu", httpd_config.server_port);

        esp_err_t result_http = httpd_start(&this->httpd, &httpd_config);
        if (result_http != ESP_OK) {
            httpd = nullptr;
            logger.printfln("Failed to start http web server: %s (0x%X)", esp_err_to_name(result_http), static_cast<unsigned>(result_http));
            return;
        }
#if HTTPS_AVAILABLE()
    }
#endif

    // Free certificate buffers early to hopefully reduce fragmentation.
    for (size_t i = 0; i < std::size(certificates); i++) {
        certificates[i].free();
    }

    const httpd_uri_t handler = {
        .uri = "", // URI can be zero-length because the custom URI matcher won't look at it anyway.
        .method = static_cast<httpd_method_t>(HTTP_ANY),
        .handler = low_level_handler,
        .user_ctx = nullptr,
        .is_websocket = false,
        .handle_ws_control_frames = true,
        .supported_subprotocol = nullptr,
    };

    const esp_err_t ret = httpd_register_uri_handler(httpd, &handler);
    if (ret != ESP_OK) {
        logger.printfln("Cannot register handler: %s (0x%lx)", esp_err_to_name(ret), static_cast<uint32_t>(ret));
    }

#if MODULE_DEBUG_AVAILABLE()
    debug.register_task("httpd", HTTPD_STACK_SIZE);
#endif
}

void WebServer::pre_reboot() {
    if (this->httpd == nullptr)
        return;

    httpd_stop(this->httpd);
}

void WebServer::runInHTTPThread(void (*fn)(void *arg), void *arg)
{
    httpd_queue_work(httpd, fn, arg);
}

// Check stack usage after increasing buffer size.
static const size_t SCRATCH_BUFSIZE = 2048;

// Don't inline the receive handler so that the scratch buffer doesn't always hog the stack.
[[gnu::noinline]]
esp_err_t WebServer::low_level_receive_handler(WebServerRequest *request, httpd_req_t *req, const WebServerHandler *handler)
{
    size_t remaining = req->content_len;
    size_t offset = 0;
    uint8_t scratch_buf[SCRATCH_BUFSIZE];

    while (remaining > 0) {
        const int recv_err = httpd_req_recv(req, reinterpret_cast<char *>(scratch_buf), std::min(remaining, SCRATCH_BUFSIZE));
        // Retry if timeout occurred
        if (recv_err == HTTPD_SOCK_ERR_TIMEOUT) {
            continue;
        }

        if (recv_err <= 0) {
            int error_code = errno;

            if (handler->callbackInMainThread) {
                auto result = task_scheduler.await([handler, request, error_code](){handler->uploadErrorCallback(*request, error_code);});
                if (result != TaskScheduler::AwaitResult::Done) {
                    return ESP_FAIL;
                }
            }
            else {
                handler->uploadErrorCallback(*request, error_code);
            }

            return ESP_FAIL;
        }

        const size_t received = static_cast<size_t>(recv_err);
        remaining -= received;
        bool result = false;
        if (handler->callbackInMainThread) {
            auto await_result = task_scheduler.await([handler, request, offset, &scratch_buf, received, remaining, &result]{result = handler->uploadCallback(*request, "not implemented", offset, scratch_buf, received, remaining);});
            if (await_result != TaskScheduler::AwaitResult::Done) {
                return ESP_FAIL;
            }
        } else {
            result = handler->uploadCallback(*request, "not implemented", offset, scratch_buf, received, remaining);
        }

        if (!result) {
            return ESP_FAIL;
        }

        offset += received;
    }

    return ESP_OK;
}

esp_err_t WebServer::low_level_handler(httpd_req_t *req)
{
    auto *server = static_cast<WebServer *>(httpd_get_global_user_ctx(req->handle));
    auto request = WebServerRequest{req};

    if (server->auth_fn && !server->auth_fn(request)) {
        bool auth_by_remote_access = false;

#if MODULE_REMOTE_ACCESS_AVAILABLE()
        const IPAddress local_address = request.getLocalAddress();

        // If this times out, it will probably unpredictably overwrite something on the stack.
        task_scheduler.await([&local_address, &auth_by_remote_access]() {
            auth_by_remote_access = remote_access.is_connected_local_ip(local_address);
        });
#endif

        if (!auth_by_remote_access) {
            if (server->on_not_authorized) {
                const WebServerRequestReturnProtect ret = server->on_not_authorized(request);
                return ret.error;
            }
            const WebServerRequestReturnProtect ret = request.requestAuthentication();
            return ret.error;
        }
    }

    const struct httpd_req_aux *aux = static_cast<struct httpd_req_aux *>(req->aux);
    const size_t listen_port_index = aux->sd->listen_port_index;

    if (listen_port_index >= std::size(server->listen_port_handlers)) {
        logger.printfln("Received request with invalid listen port index %zu", listen_port_index);
        return ESP_FAIL;
    }

    const listen_port_handlers_t *port_handlers = server->listen_port_handlers[listen_port_index];

    if (port_handlers == nullptr) {
        logger.printfln("Received request with unused listen port index %zu", listen_port_index);
        return ESP_FAIL;
    }

    const char *uri = req->uri;
    const size_t uri_len = strlen(uri);
    const httpd_method_t method = static_cast<httpd_method_t>(req->method);

    const WebServerHandler *handler = server->match_handlers(port_handlers, uri, uri_len, method);

    if (handler == nullptr) {
#if MODULE_HTTP_AVAILABLE()
        // No simple handler for this URI, pass to HTTP API.
        if (port_handlers->supports_http_api && http.api_handler(request, uri_len)) {
            // HTTP API handled the request.
            return ESP_OK;
        }
#endif

        handler = server->match_wildcard_handlers(port_handlers, uri, uri_len, method);

        if (handler == nullptr) {
            const WebServerRequestReturnProtect ret = request.send_plain(404, "Nothing matches the given URI.");
            return ret.error;
        }
    }

    if (aux->ws_handshake_detect && !handler->is_websocket) {
        const WebServerRequestReturnProtect ret = request.send_plain(400, "Bad Request: WebSocket not supported");
        return ret.error;
    }

    if (handler->accepts_upload) {
        if (req->content_len == 0) {
            if (request.header("Content-Length").isEmpty()) {
                // Probably a chunked encoding. Not supported.
                if (handler->callbackInMainThread) {
                    task_scheduler.await([handler, &request](){handler->uploadErrorCallback(request, EBADMSG);});
                }
                else {
                    handler->uploadErrorCallback(request, EBADMSG);
                }
                return ESP_FAIL;
            } else {
                // This is really a request were there are 0 bytes to receive. Call the upload handler once.
                bool result = false;
                if (handler->callbackInMainThread) {
                    task_scheduler.await([handler, &request, &result]{result = handler->uploadCallback(request, "not implemented", 0, nullptr, 0, 0);});
                } else {
                    result = handler->uploadCallback(request, "not implemented", 0, nullptr, 0, 0);
                }

                if (!result) {
                    return ESP_FAIL;
                }
            }
        } else {
            // Receive payload
            const esp_err_t ret = low_level_receive_handler(&request, req, handler);

            if (ret != ESP_OK) {
                return ret;
            }
        }
    }

    if (handler->callbackInMainThread) {
        struct {
            WebServerRequest *req;
            esp_err_t error;
        } rq = {
            .req = &request,
            .error = ESP_ERR_NOT_FINISHED,
        };

        task_scheduler.await([handler, &rq]() {
            const WebServerRequestReturnProtect ret = handler->callback(*rq.req);
            rq.error = ret.error;
        });

        return rq.error;
    } else {
        const WebServerRequestReturnProtect ret = handler->callback(request);
        return ret.error;
    }
}

WebServerHandler *WebServer::on(const char *uri,
                                httpd_method_t method,
                                wshCallback &&callback,
                                uint16_t port)
{
    return addHandler(port,
                      uri,
                      method,
                      false, // isWebsocket
                      true,
                      std::move(callback),
                      wshUploadCallback(),
                      wshUploadErrorCallback());
}

WebServerHandler *WebServer::on(const char *uri,
                                httpd_method_t method,
                                wshCallback &&callback,
                                wshUploadCallback &&uploadCallback,
                                wshUploadErrorCallback &&uploadErrorCallback,
                                uint16_t port)
{
    return addHandler(port,
                      uri,
                      method,
                      false, // isWebsocket
                      true,
                      std::move(callback),
                      std::move(uploadCallback),
                      std::move(uploadErrorCallback));
}

WebServerHandler *WebServer::on_HTTPThread(const char *uri,
                                           httpd_method_t method,
                                           wshCallback &&callback,
                                           uint16_t port)
{
    return addHandler(port,
                      uri,
                      method,
                      false, // isWebsocket
                      false,
                      std::move(callback),
                      wshUploadCallback(),
                      wshUploadErrorCallback());
}

WebServerHandler *WebServer::on_HTTPThread(const char *uri,
                                           httpd_method_t method,
                                           wshCallback &&callback,
                                           wshUploadCallback &&uploadCallback,
                                           wshUploadErrorCallback &&uploadErrorCallback,
                                           uint16_t port)
{
    return addHandler(port,
                      uri,
                      method,
                      false, // isWebsocket
                      false,
                      std::move(callback),
                      std::move(uploadCallback),
                      std::move(uploadErrorCallback));
}

WebServerHandler *WebServer::onWS_HTTPThread(const char *uri, httpd_handle_t *http_handle_out, wshCallback &&callback, uint16_t port)
{
    WebServerHandler *handler = addHandler(port,
                      uri,
                      HTTP_GET,
                      true, // isWebsocket
                      false,
                      std::move(callback),
                      wshUploadCallback(),
                      wshUploadErrorCallback());

    if (handler != nullptr) {
        *http_handle_out = httpd;
    }

    return handler;
}

void WebServer::onNotAuthorized_HTTPThread(wshCallback &&callback)
{
    this->on_not_authorized = std::move(callback);
}

void WebServer::onAuthenticate_HTTPThread(std::function<bool(WebServerRequest)> &&auth_fn_)
{
    this->auth_fn = std::move(auth_fn_);
}

static WebServerHandler **web_server_find_handler_placement(WebServerHandler **handler_target, WebServerSortOrder sort_order, size_t uri_length)
{
    while (*handler_target != nullptr){
        WebServerHandler *next_handler = *handler_target;

        if (sort_order == WebServerSortOrder::ASCENDING) {
            if (next_handler->uri_len > uri_length) {
                break;
            }
        } else { // DESCENDING
            if (next_handler->uri_len < uri_length) {
                break;
            }
        }

        handler_target = &next_handler->next;
    }

    return handler_target;
}

WebServerHandler *WebServer::addHandler(uint16_t port,
                                        const char *uri,
                                        httpd_method_t method,
                                        bool isWebsocket,
                                        bool callbackInMainThread,
                                        wshCallback &&callback,
                                        wshUploadCallback &&uploadCallback,
                                        wshUploadErrorCallback &&uploadErrorCallback)
{
    if (port == 80 || port == 443) {
        esp_system_abort("Don't register handlers on port 80 or 443. Use port 0 to register web interface handlers.");
    }

    if (boot_stage < BootStage::REGISTER_URLS) {
        esp_system_abort("Attempted to register URL handler before REGISTER_URLS stage");
    }

    listen_port_handlers_t *port_handlers;

    if (port == 0) {
        port_handlers = listen_port_handlers[0]; // Web interface handlers are always first.
    } else {
        port_handlers = nullptr;

        for (size_t i = 1; i < std::size(listen_port_handlers); i++) {
            listen_port_handlers_t *handlers = listen_port_handlers[i];

            if (handlers->port == port) {
                port_handlers = handlers;
                break;
            }
        }

        if (port_handlers == nullptr) {
            logger.printfln("Cannot add URI '%s' for unregistered port %hu", uri, port);
            return nullptr;
        }
    }

    size_t uri_len = strlen(uri);

    if (uri_len == 0 || uri[0] != '/') {
        logger.printfln("Attempted to register handler for invalid URI '%s'", uri);
        return nullptr;
    }

    if (!address_is_in_rodata(uri)) {
        uri = perm_strdup(uri);
    }

    WebServerHandler **handler_target;

    if (uri[uri_len - 1] == '*') { // Is wildcard handler?
        uri_len--; // Ignore trailing asterisk.
        handler_target = web_server_find_handler_placement(&port_handlers->wildcard_handlers, WebServerSortOrder::DESCENDING, uri_len);
    } else {
        handler_target = web_server_find_handler_placement(&port_handlers->handlers, WebServerSortOrder::ASCENDING, uri_len);
    }

    void *arena_ptr = perm_aligned_alloc(alignof(WebServerHandler), sizeof(WebServerHandler), DRAM);

    if (arena_ptr == nullptr) {
        logger.printfln("Not enough memory in arena for URI handler");
        return nullptr;
    }

    WebServerHandler *new_handler = new(arena_ptr) WebServerHandler{uri,
                           uri_len,
                           method,
                           isWebsocket,
                           callbackInMainThread,
                           std::move(callback),
                           std::move(uploadCallback),
                           std::move(uploadErrorCallback)};

    new_handler->next = *handler_target;
    *handler_target = new_handler;

    return new_handler;
}

void WebServer::register_extra_port(WebServerExtraPortData *port_data)
{
    if (boot_stage > BootStage::SETUP) {
        esp_system_abort("Extra ports can only be registered in (pre)setup");
    }

    assert(port_data != nullptr);

    port_data->next = extra_ports;
    extra_ports = port_data;
}

const WebServerHandler *WebServer::match_handlers(const listen_port_handlers_t *port_handlers, const char *req_uri, size_t req_uri_len, httpd_method_t method)
{
    const WebServerHandler *handler = port_handlers->handlers;

    while (handler != nullptr) {
        if (handler->uri_len > req_uri_len) {
            // This and all upcoming handlers have longer URIs.
            return nullptr;
        }

        if (handler->uri_len == req_uri_len
            && (handler->method == method || handler->method == static_cast<httpd_method_t>(HTTP_ANY))
            && strcmp(handler->uri, req_uri) == 0) {
                return handler;
        }

        handler = handler->next;
    }

    return nullptr;
}

const WebServerHandler *WebServer::match_wildcard_handlers(const listen_port_handlers_t *port_handlers, const char *req_uri, size_t req_uri_len, httpd_method_t method)
{
    const WebServerHandler *handler = port_handlers->wildcard_handlers;

    while (handler != nullptr) {
        const size_t wildcard_len = handler->uri_len; // Excludes trailing asterisk.

        if (wildcard_len <= req_uri_len
            && (handler->method == method || handler->method == static_cast<httpd_method_t>(HTTP_ANY))
            && strncmp(handler->uri, req_uri, wildcard_len) == 0) {
                return handler;
        }

        handler = handler->next;
    }

    return nullptr;
}

#ifdef DEBUG_FS_ENABLE
void WebServer::get_handlers(WebServerHandler **handlers_out, WebServerHandler **wildcard_handlers_out)
{
    const listen_port_handlers_t *port_handlers = listen_port_handlers[0];

    *handlers_out          = port_handlers->handlers;
    *wildcard_handlers_out = port_handlers->wildcard_handlers;
}
#endif

WebServerHandler::WebServerHandler(const char *uri_, size_t uri_len_, httpd_method_t method_, bool isWebsocket_, bool callbackInMainThread_, wshCallback &&callback_, wshUploadCallback &&uploadCallback_, wshUploadErrorCallback &&uploadErrorCallback_) :
    uri(uri_),
    uri_len(uri_len_),
    method(method_),
    is_websocket(isWebsocket_),
    accepts_upload(uploadCallback_ != nullptr), // Compare with input parameter before std::move
    callbackInMainThread(callbackInMainThread_),
    callback(std::move(callback_)),
    uploadCallback(std::move(uploadCallback_)),
    uploadErrorCallback(std::move(uploadErrorCallback_))
{
}

// From: https://www.iana.org/assignments/http-status-codes/http-status-codes.xhtml
static const char *httpStatusCodeToString(int code)
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

WebServerRequestReturnProtect WebServerRequest::send(uint16_t code, const char *content_type, const char *content, size_t content_len)
{
    auto result = httpd_resp_set_type(req, content_type);
    if (result != ESP_OK) {
        printf("Failed to set response type: %s (0x%X)\n", esp_err_to_name(result), static_cast<unsigned>(result));
        return WebServerRequestReturnProtect{};
    }

    result = httpd_resp_set_status(req, httpStatusCodeToString(code));
    if (result != ESP_OK) {
        printf("Failed to set response status: %s (0x%X)\n", esp_err_to_name(result), static_cast<unsigned>(result));
        return WebServerRequestReturnProtect{};
    }

    result = httpd_resp_send(req, content, static_cast<ssize_t>(content_len));

    if (result != ESP_OK) {
        printf("Failed to send response: %s (0x%X)\n", esp_err_to_name(result), static_cast<unsigned>(result));
    }

    return WebServerRequestReturnProtect{result};
}

void WebServerRequest::beginChunkedResponse(uint16_t code, const char *content_type)
{
    if (chunkedResponseState != ChunkedResponseState::NotStarted) {
        esp_system_abort("BUG: Multiple calls to beginChunkedResponse detected!");
    }

    auto result = httpd_resp_set_type(req, content_type);
    if (result != ESP_OK) {
        chunkedResponseState = ChunkedResponseState::Failed;
        printf("Failed to set response type: %s (0x%X)\n", esp_err_to_name(result), static_cast<unsigned>(result));
        return;
    }

    result = httpd_resp_set_status(req, httpStatusCodeToString(code));
    if (result != ESP_OK) {
        chunkedResponseState = ChunkedResponseState::Failed;
        printf("Failed to set response status: %s (0x%X)\n", esp_err_to_name(result), static_cast<unsigned>(result));
        return;
    }

    chunkedResponseState = ChunkedResponseState::Started;
}

int WebServerRequest::sendChunk(const char *chunk, size_t chunk_len)
{
    switch (chunkedResponseState) {
        case ChunkedResponseState::Failed:
            return ESP_FAIL;
        case ChunkedResponseState::NotStarted:
            esp_system_abort("BUG: sendChunk was called before beginChunkedResponse!");
        case ChunkedResponseState::Ended:
            esp_system_abort("BUG: sendChunk was called after endChunkedResponse");
        case ChunkedResponseState::Started:
            break;
        default:
            esp_system_abort("BUG: sendChunk in invalid state");
    }

    if (chunk_len == 0)
        logger.printfln("BUG: sendChunk was called with chunk_len == 0! Use endChunkedResponse!");

    auto result = httpd_resp_send_chunk(req, chunk, static_cast<ssize_t>(chunk_len));
    if (result != ESP_OK) {
        chunkedResponseState = ChunkedResponseState::Failed;
        printf("Failed to send response chunk: %s (0x%X)\n", esp_err_to_name(result), static_cast<unsigned>(result));

        // "When you are finished sending all your chunks, you must call this function with buf_len as 0."
        httpd_resp_send_chunk(req, nullptr, 0);
    }
    return result;
}

WebServerRequestReturnProtect WebServerRequest::endChunkedResponse()
{
    switch (chunkedResponseState) {
        case ChunkedResponseState::Failed:
            return WebServerRequestReturnProtect{};
        case ChunkedResponseState::NotStarted:
            esp_system_abort("BUG: endChunkedResponse was called before beginChunkedResponse!");
        case ChunkedResponseState::Ended:
            esp_system_abort("BUG: endChunkedResponse was called twice!");
        case ChunkedResponseState::Started:
            break;
        default:
            esp_system_abort("BUG: sendChunk in invalid state");
    }

    auto result = httpd_resp_send_chunk(req, nullptr, 0);
    if (result != ESP_OK) {
        chunkedResponseState = ChunkedResponseState::Failed;
        printf("Failed to end chunked response: %s (0x%X)\n", esp_err_to_name(result), static_cast<unsigned>(result));
    }

    chunkedResponseState = ChunkedResponseState::Ended;
    return WebServerRequestReturnProtect{};
}

void WebServerRequest::addResponseHeader(const char *field, const char *value)
{
    auto result = httpd_resp_set_hdr(req, field, value);
    if (result != ESP_OK) {
        printf("Failed to set response header: %s (0x%X)\n", esp_err_to_name(result), static_cast<unsigned>(result));
        return;
    }
}

WebServerRequestReturnProtect WebServerRequest::requestAuthentication()
{
    String payload = "Digest ";
    payload.concat(requestDigestAuthentication(nullptr));
    addResponseHeader("WWW-Authenticate", payload.c_str());
    return send_plain(401);
}

String WebServerRequest::header(const char *header_name)
{
    auto buf_len = httpd_req_get_hdr_value_len(req, header_name) + 1;
    if (buf_len == 1) {
        return "";
    }

    CoolString result;
    if (!result.reserve(buf_len)) {
        return "";
    }

    char *buf = result.begin();
    /* Copy null terminated value string into buffer */
    if (httpd_req_get_hdr_value_str(req, header_name, buf, buf_len) != ESP_OK) {
        return "";
    }
    result.setLength(static_cast<int>(buf_len));
    return result;
}

size_t WebServerRequest::contentLength()
{
    return req->content_len;
}

int WebServerRequest::receive(char *buf, size_t buf_len)
{
    if (buf_len < contentLength())
        return -1;

    size_t bytes_left = contentLength();
    while (bytes_left != 0) {
        int read = httpd_req_recv(req, buf, bytes_left);
        if (read <= 0) {
            return read;
        }
        bytes_left -= static_cast<size_t>(read);
        buf += read;
    }

    return static_cast<int>(contentLength());
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
const char *WebServerRequest::methodString()
{
    switch (method()) {
        case HTTP_GET:
            return "GET";
        case HTTP_PUT:
            return "PUT";
        case HTTP_POST:
            return "POST";
        default:
            return "";
    }
}
#pragma GCC diagnostic pop

IPAddress WebServerRequest::getLocalAddress()
{
    const int sockfd = httpd_req_to_sockfd(req);

    if (sockfd < 0) {
        logger.printfln("Attempted to get sockfd from invalid req");
        return IPAddress();
    }

    return tf_local_address_of_sockfd(sockfd);
}

IPAddress WebServerRequest::getPeerAddress()
{
    const int sockfd = httpd_req_to_sockfd(req);

    if (sockfd < 0) {
        logger.printfln("Attempted to get sockfd from invalid req");
        return IPAddress();
    }

    return tf_peer_address_of_sockfd(sockfd);
}
