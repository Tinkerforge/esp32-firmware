/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include <stdlib.h>
#include <FS.h> // FIXME: without this include here there is a problem with the IPADDR_NONE define in <lwip/ip4_addr.h>
#include <esp_http_client.h>
#include <esp_tls_errors.h>
#include <vector>
#include <TFTools/Micros.h>

enum class AsyncHTTPSClientError
{
    NoHTTPSURL,
    Busy,
    NoCert,
    Timeout,
    ShortRead,
    HTTPError,
    HTTPClientInitFailed,
    HTTPClientSetCookieFailed,
    HTTPClientSetHeaderFailed,
    HTTPClientSetBodyFailed,
    HTTPClientError,
    HTTPStatusError,
};

enum class AsyncHTTPSClientEventType
{
    Error,
    Data,
    Aborted,
    Finished,
    Redirect,
};

struct AsyncHTTPSClientEvent
{
    AsyncHTTPSClientEventType type;

    union {
        // AsyncHTTPSClientEventType::Error
        struct {
            AsyncHTTPSClientError error;
            esp_tls_error_handle_t error_handle; // AsyncHTTPSClientError::HTTPError
            esp_err_t error_http_client; // AsyncHTTPSClientError::HTTPClientError
            int error_http_status; // AsyncHTTPSClientError::HTTPStatusError
        };

        // AsyncHTTPSClientEventType::Data
        struct {
            size_t data_chunk_offset;
            const void *data_chunk;
            size_t data_chunk_len;
            ssize_t data_complete_len; // -1 if chunked response
            bool data_is_complete;
        };

        // AsyncHTTPSClientEventType::Redirect
        struct {
            int redirect_status_code;
        };
    };
};

const char *translate_error(AsyncHTTPSClientEvent *event);
size_t translate_HTTPError_detailed(const esp_tls_error_handle_t error_handle, char *buf, size_t buflen, bool include_sock_errno = false);

class AsyncHTTPSClient final
{
public:
    AsyncHTTPSClient(bool use_cookies = false);
    ~AsyncHTTPSClient();

    void download_async(const char *url, int cert_id, std::function<void(AsyncHTTPSClientEvent *event)> &&callback);
    void post_async(const char *url, int cert_id, const char *body, int body_size, std::function<void(AsyncHTTPSClientEvent *event)> &&callback);
    void put_async(const char *url, int cert_id, const char *body, int body_size, std::function<void(AsyncHTTPSClientEvent *event)> &&callback);
    void delete_async(const char *url, int cert_id, const char *body, int body_size, std::function<void(AsyncHTTPSClientEvent *event)> &&callback);
    void abort_async();
    void set_header(const char *key, const char *value);
    void set_header(const String &key, const String &value);
    bool is_busy() const { return in_progress; }
    void fetch(const char *url, int cert_id, esp_http_client_method_t method, const char *body, int body_size, std::function<void(AsyncHTTPSClientEvent *event)> &&callback);

private:
    void add_default_headers();
    void error_abort(AsyncHTTPSClientError error, esp_err_t error_http_client = ESP_OK, int error_http_status = -1);
    void clear();
    void parse_cookie(const char *cookie);
    static esp_err_t event_handler(esp_http_client_event_t *event);

    std::unique_ptr<unsigned char[]> cert = nullptr;
    std::function<void(AsyncHTTPSClientEvent *event)> callback;
    std::vector<std::pair<String, String>> headers;
    String cookies = "";
    String owned_body;
    bool in_progress = false;
    bool abort_requested = false;
    esp_http_client_handle_t http_client = nullptr;
    micros_t last_async_alive = 0_us;
    size_t received_len = 0;
    bool use_cookies;
    uint64_t task_id = 0;
};
