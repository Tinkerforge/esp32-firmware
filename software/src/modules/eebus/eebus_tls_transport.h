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

#pragma once

#include "esp_transport.h"

/**
 * Create a custom TLS transport that uses mbedTLS directly with
 * MBEDTLS_SSL_VERIFY_NONE for the server certificate.
 *
 * EEBUS uses self-signed certificates, so the standard esp_transport_ssl
 * cannot be used (it requires CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY which
 * is disabled in the pre-compiled libs). This transport bypasses the
 * esp-tls layer entirely and configures mbedTLS directly.
 *
 * Supports mutual TLS: if client_cert_pem and client_key_pem are provided,
 * they are presented to the server during the TLS handshake.
 *
 * @param client_cert_pem  PEM-encoded client certificate (NULL-terminated), or NULL
 * @param client_cert_len  Length of client_cert_pem (0 for auto-detect via strlen)
 * @param client_key_pem   PEM-encoded client private key (NULL-terminated), or NULL
 * @param client_key_len   Length of client_key_pem (0 for auto-detect via strlen)
 * @return esp_transport_handle_t  The transport handle, or NULL on failure
 */
esp_transport_handle_t eebus_tls_transport_init(
    const char *client_cert_pem, size_t client_cert_len,
    const char *client_key_pem,  size_t client_key_len);
