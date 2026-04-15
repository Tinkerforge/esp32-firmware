/* esp32-firmware
 * Copyright (C) 2026 Julius Dill <julius@tinkerforge.com>
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

// Custom TLS transport for EEBUS WebSocket client connections.
//
// EEBUS uses self-signed certificates identified by their SKI (Subject Key
// Identifier). Server certificate verification against a CA chain is neither
// possible nor desired — the peer is authenticated via its SKI during the
// SHIP handshake instead. The pre-compiled ESP-IDF libs do not have
// CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY enabled, so esp_transport_ssl
// refuses to connect when no CA cert is provided. This transport uses
// mbedTLS directly with MBEDTLS_SSL_VERIFY_NONE to bypass that limitation.

#include "eebus_tls_transport.h"

#include "event_log_prefix.h"

#include "esp_transport.h"
#include "esp_log.h"

#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/error.h"
#include "mbedtls/net_sockets.h"

#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

static const char *TAG = "eebus_tls";

/// Internal state stored as context data on the esp_transport handle.
struct eebus_tls_ctx {
    mbedtls_ssl_context    ssl;
    mbedtls_ssl_config     conf;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_x509_crt       client_cert;
    mbedtls_pk_context     client_key;
    int                    sockfd;
    bool                   has_client_cert;

    // Store PEM pointers (not owned, must outlive the transport)
    const char *client_cert_pem;
    size_t      client_cert_len;
    const char *client_key_pem;
    size_t      client_key_len;
};

[[gnu::noinline]]
static void log_mbedtls_error(const char *msg, int ret)
{
    char buf[128];
    mbedtls_strerror(ret, buf, sizeof(buf));
    ESP_LOGE(TAG, "%s: -0x%04x (%s)", msg, (unsigned)(-ret), buf);
}

// ============================================================================
// esp_transport callbacks
// ============================================================================

static int eebus_tls_connect(esp_transport_handle_t t, const char *host, int port, int timeout_ms)
{
    eebus_tls_ctx *ctx = static_cast<eebus_tls_ctx *>(esp_transport_get_context_data(t));
    if (ctx == nullptr) {
        return -1;
    }

    int ret;

    // --- 1. TCP connect ---
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *res = nullptr;

    ret = getaddrinfo(host, port_str, &hints, &res);
    if (ret != 0 || res == nullptr) {
        ESP_LOGE(TAG, "DNS resolution failed for %s:%d", host, port);
        return -1;
    }

    ctx->sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (ctx->sockfd < 0) {
        ESP_LOGE(TAG, "socket() failed: errno %d", errno);
        freeaddrinfo(res);
        return -1;
    }

    // Set socket to non-blocking for connect with timeout
    int flags = fcntl(ctx->sockfd, F_GETFL, 0);
    fcntl(ctx->sockfd, F_SETFL, flags | O_NONBLOCK);

    ret = connect(ctx->sockfd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    if (ret < 0 && errno != EINPROGRESS) {
        ESP_LOGE(TAG, "connect() failed: errno %d", errno);
        close(ctx->sockfd);
        ctx->sockfd = -1;
        return -1;
    }

    if (ret < 0) {
        // Wait for connect to complete
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(ctx->sockfd, &writefds);
        struct timeval tv = {
            .tv_sec = timeout_ms / 1000,
            .tv_usec = (timeout_ms % 1000) * 1000,
        };

        ret = select(ctx->sockfd + 1, nullptr, &writefds, nullptr, &tv);
        if (ret <= 0) {
            ESP_LOGE(TAG, "TCP connect timeout to %s:%d", host, port);
            close(ctx->sockfd);
            ctx->sockfd = -1;
            return -1;
        }

        int sock_err = 0;
        socklen_t sock_err_len = sizeof(sock_err);
        getsockopt(ctx->sockfd, SOL_SOCKET, SO_ERROR, &sock_err, &sock_err_len);
        if (sock_err != 0) {
            ESP_LOGE(TAG, "TCP connect failed: sock_err %d", sock_err);
            close(ctx->sockfd);
            ctx->sockfd = -1;
            return -1;
        }
    }

    // Set socket back to blocking for mbedTLS I/O
    fcntl(ctx->sockfd, F_SETFL, flags);

    // Set a recv timeout so mbedTLS handshake doesn't block forever
    struct timeval recv_tv = {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000,
    };
    setsockopt(ctx->sockfd, SOL_SOCKET, SO_RCVTIMEO, &recv_tv, sizeof(recv_tv));
    setsockopt(ctx->sockfd, SOL_SOCKET, SO_SNDTIMEO, &recv_tv, sizeof(recv_tv));

    // --- 2. mbedTLS config ---
    ret = mbedtls_ssl_config_defaults(&ctx->conf,
                                       MBEDTLS_SSL_IS_CLIENT,
                                       MBEDTLS_SSL_TRANSPORT_STREAM,
                                       MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        log_mbedtls_error("ssl_config_defaults", ret);
        close(ctx->sockfd);
        ctx->sockfd = -1;
        return -1;
    }

    // Skip server certificate verification — EEBUS identifies peers by SKI
    mbedtls_ssl_conf_authmode(&ctx->conf, MBEDTLS_SSL_VERIFY_NONE);
    mbedtls_ssl_conf_rng(&ctx->conf, mbedtls_ctr_drbg_random, &ctx->ctr_drbg);

    // Client certificate for mTLS (so the server can identify us by our SKI)
    if (ctx->has_client_cert) {
        ret = mbedtls_ssl_conf_own_cert(&ctx->conf, &ctx->client_cert, &ctx->client_key);
        if (ret != 0) {
            log_mbedtls_error("ssl_conf_own_cert", ret);
            close(ctx->sockfd);
            ctx->sockfd = -1;
            return -1;
        }
    }

    ret = mbedtls_ssl_setup(&ctx->ssl, &ctx->conf);
    if (ret != 0) {
        log_mbedtls_error("ssl_setup", ret);
        close(ctx->sockfd);
        ctx->sockfd = -1;
        return -1;
    }

    // Set hostname for SNI (Server Name Indication)
    mbedtls_ssl_set_hostname(&ctx->ssl, host);

    // Wire up I/O to the TCP socket
    mbedtls_ssl_set_bio(&ctx->ssl, &ctx->sockfd,
                        mbedtls_net_send, mbedtls_net_recv, nullptr);

    // --- 3. TLS handshake ---
    while ((ret = mbedtls_ssl_handshake(&ctx->ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            log_mbedtls_error("TLS handshake failed", ret);
            mbedtls_ssl_free(&ctx->ssl);
            mbedtls_ssl_init(&ctx->ssl); // re-init for potential reuse
            close(ctx->sockfd);
            ctx->sockfd = -1;
            return -1;
        }
    }

    ESP_LOGI(TAG, "TLS handshake complete with %s:%d (cipher: %s)",
             host, port, mbedtls_ssl_get_ciphersuite(&ctx->ssl));

    return 0;
}

static int eebus_tls_read(esp_transport_handle_t t, char *buffer, int len, int timeout_ms)
{
    eebus_tls_ctx *ctx = static_cast<eebus_tls_ctx *>(esp_transport_get_context_data(t));
    if (ctx == nullptr || ctx->sockfd < 0) {
        return -1;
    }

    // Check if there is already buffered data in mbedTLS
    size_t pending = mbedtls_ssl_get_bytes_avail(&ctx->ssl);
    if (pending == 0) {
        // No buffered data — wait for socket to become readable
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(ctx->sockfd, &readfds);
        struct timeval tv = {
            .tv_sec = timeout_ms / 1000,
            .tv_usec = (timeout_ms % 1000) * 1000,
        };
        int sel = select(ctx->sockfd + 1, &readfds, nullptr, nullptr, &tv);
        if (sel == 0) {
            return ERR_TCP_TRANSPORT_CONNECTION_TIMEOUT; // Timeout
        }
        if (sel < 0) {
            return -1;
        }
    }

    int ret = mbedtls_ssl_read(&ctx->ssl, reinterpret_cast<unsigned char *>(buffer), len);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
        return ERR_TCP_TRANSPORT_CONNECTION_TIMEOUT;
    }
    if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY || ret == 0) {
        return -1; // Connection closed
    }
    if (ret < 0) {
        log_mbedtls_error("ssl_read", ret);
        return -1;
    }
    return ret;
}

static int eebus_tls_write(esp_transport_handle_t t, const char *buffer, int len, int timeout_ms)
{
    eebus_tls_ctx *ctx = static_cast<eebus_tls_ctx *>(esp_transport_get_context_data(t));
    if (ctx == nullptr || ctx->sockfd < 0) {
        return -1;
    }

    int ret = mbedtls_ssl_write(&ctx->ssl, reinterpret_cast<const unsigned char *>(buffer), len);
    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
        // Wait for socket to become writable
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(ctx->sockfd, &writefds);
        struct timeval tv = {
            .tv_sec = timeout_ms / 1000,
            .tv_usec = (timeout_ms % 1000) * 1000,
        };
        int sel = select(ctx->sockfd + 1, nullptr, &writefds, nullptr, &tv);
        if (sel <= 0) {
            return -1;
        }
        // Retry write
        ret = mbedtls_ssl_write(&ctx->ssl, reinterpret_cast<const unsigned char *>(buffer), len);
    }
    if (ret < 0) {
        log_mbedtls_error("ssl_write", ret);
        return -1;
    }
    return ret;
}

static int eebus_tls_close(esp_transport_handle_t t)
{
    eebus_tls_ctx *ctx = static_cast<eebus_tls_ctx *>(esp_transport_get_context_data(t));
    if (ctx == nullptr) {
        return 0;
    }

    if (ctx->sockfd >= 0) {
        // Best-effort close_notify
        mbedtls_ssl_close_notify(&ctx->ssl);
        close(ctx->sockfd);
        ctx->sockfd = -1;
    }

    // Reset SSL state for potential reconnect
    mbedtls_ssl_free(&ctx->ssl);
    mbedtls_ssl_init(&ctx->ssl);
    mbedtls_ssl_config_free(&ctx->conf);
    mbedtls_ssl_config_init(&ctx->conf);

    return 0;
}

static int eebus_tls_poll_read(esp_transport_handle_t t, int timeout_ms)
{
    eebus_tls_ctx *ctx = static_cast<eebus_tls_ctx *>(esp_transport_get_context_data(t));
    if (ctx == nullptr || ctx->sockfd < 0) {
        return -1;
    }

    // If mbedTLS has buffered decrypted data, it's immediately readable
    if (mbedtls_ssl_get_bytes_avail(&ctx->ssl) > 0) {
        return 1;
    }

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(ctx->sockfd, &readfds);
    struct timeval tv = {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000,
    };
    return select(ctx->sockfd + 1, &readfds, nullptr, nullptr, &tv);
}

static int eebus_tls_poll_write(esp_transport_handle_t t, int timeout_ms)
{
    eebus_tls_ctx *ctx = static_cast<eebus_tls_ctx *>(esp_transport_get_context_data(t));
    if (ctx == nullptr || ctx->sockfd < 0) {
        return -1;
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(ctx->sockfd, &writefds);
    struct timeval tv = {
        .tv_sec = timeout_ms / 1000,
        .tv_usec = (timeout_ms % 1000) * 1000,
    };
    return select(ctx->sockfd + 1, nullptr, &writefds, nullptr, &tv);
}

static int eebus_tls_destroy(esp_transport_handle_t t)
{
    eebus_tls_ctx *ctx = static_cast<eebus_tls_ctx *>(esp_transport_get_context_data(t));
    if (ctx == nullptr) {
        return 0;
    }

    if (ctx->sockfd >= 0) {
        close(ctx->sockfd);
        ctx->sockfd = -1;
    }

    mbedtls_ssl_free(&ctx->ssl);
    mbedtls_ssl_config_free(&ctx->conf);
    mbedtls_ctr_drbg_free(&ctx->ctr_drbg);
    mbedtls_entropy_free(&ctx->entropy);

    if (ctx->has_client_cert) {
        mbedtls_x509_crt_free(&ctx->client_cert);
        mbedtls_pk_free(&ctx->client_key);
    }

    free(ctx);
    esp_transport_set_context_data(t, nullptr);

    return 0;
}

// ============================================================================
// Public API
// ============================================================================

esp_transport_handle_t eebus_tls_transport_init(
    const char *client_cert_pem, size_t client_cert_len,
    const char *client_key_pem,  size_t client_key_len)
{
    // Allocate context
    eebus_tls_ctx *ctx = static_cast<eebus_tls_ctx *>(calloc(1, sizeof(eebus_tls_ctx)));
    if (ctx == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate eebus_tls_ctx");
        return nullptr;
    }

    ctx->sockfd = -1;
    ctx->has_client_cert = false;

    // Init mbedTLS structures
    mbedtls_ssl_init(&ctx->ssl);
    mbedtls_ssl_config_init(&ctx->conf);
    mbedtls_entropy_init(&ctx->entropy);
    mbedtls_ctr_drbg_init(&ctx->ctr_drbg);

    // Seed the RNG
    int ret = mbedtls_ctr_drbg_seed(&ctx->ctr_drbg, mbedtls_entropy_func,
                                     &ctx->entropy, nullptr, 0);
    if (ret != 0) {
        log_mbedtls_error("ctr_drbg_seed", ret);
        free(ctx);
        return nullptr;
    }

    // Parse client certificate and key if provided
    if (client_cert_pem != nullptr && client_key_pem != nullptr) {
        mbedtls_x509_crt_init(&ctx->client_cert);
        mbedtls_pk_init(&ctx->client_key);

        size_t cert_len = client_cert_len > 0 ? client_cert_len : strlen(client_cert_pem) + 1;
        ret = mbedtls_x509_crt_parse(&ctx->client_cert,
                                      reinterpret_cast<const unsigned char *>(client_cert_pem),
                                      cert_len);
        if (ret != 0) {
            log_mbedtls_error("x509_crt_parse (client cert)", ret);
            mbedtls_x509_crt_free(&ctx->client_cert);
            mbedtls_pk_free(&ctx->client_key);
            mbedtls_ctr_drbg_free(&ctx->ctr_drbg);
            mbedtls_entropy_free(&ctx->entropy);
            free(ctx);
            return nullptr;
        }

        size_t key_len = client_key_len > 0 ? client_key_len : strlen(client_key_pem) + 1;
        ret = mbedtls_pk_parse_key(&ctx->client_key,
                                    reinterpret_cast<const unsigned char *>(client_key_pem),
                                    key_len, nullptr, 0,
                                    mbedtls_ctr_drbg_random, &ctx->ctr_drbg);
        if (ret != 0) {
            log_mbedtls_error("pk_parse_key (client key)", ret);
            mbedtls_x509_crt_free(&ctx->client_cert);
            mbedtls_pk_free(&ctx->client_key);
            mbedtls_ctr_drbg_free(&ctx->ctr_drbg);
            mbedtls_entropy_free(&ctx->entropy);
            free(ctx);
            return nullptr;
        }

        ctx->has_client_cert = true;
    }

    // Create esp_transport handle and wire up callbacks
    esp_transport_handle_t transport = esp_transport_init();
    if (transport == nullptr) {
        ESP_LOGE(TAG, "Failed to create esp_transport handle");
        if (ctx->has_client_cert) {
            mbedtls_x509_crt_free(&ctx->client_cert);
            mbedtls_pk_free(&ctx->client_key);
        }
        mbedtls_ctr_drbg_free(&ctx->ctr_drbg);
        mbedtls_entropy_free(&ctx->entropy);
        free(ctx);
        return nullptr;
    }

    esp_transport_set_context_data(transport, ctx);
    esp_transport_set_func(transport,
                           eebus_tls_connect,
                           eebus_tls_read,
                           eebus_tls_write,
                           eebus_tls_close,
                           eebus_tls_poll_read,
                           eebus_tls_poll_write,
                           eebus_tls_destroy);
    esp_transport_set_default_port(transport, 4712);

    return transport;
}
