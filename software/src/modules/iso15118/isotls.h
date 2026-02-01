/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include <stdint.h>
#include <stddef.h>

#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/net_sockets.h"

// =============================================================================
// TLS Configuration for ISO 15118
// =============================================================================
//
// This implementation currently uses ISO 15118-2 TLS settings
// ISO 15118-20 has different TLS requirements (see TODOs below).
//
// -----------------------------------------------------------------------------
// ISO 15118-2 TLS requirements (Section 7.7.3):
// -----------------------------------------------------------------------------
// [V2G2-067] TLS version 1.2 according to IETF RFC 5246 shall be supported
// [V2G2-068] The SECC shall always act as the TLS server component
// [V2G2-602] The SECC shall support all cipher suites defined in Table 7:
//            - TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 (IETF RFC 5289)
//            - TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256 (IETF RFC 5289)
// [V2G2-006] ECC-based using secp256r1 curve with ECDSA signature algorithm
// [V2G2-007] Key length for ECC shall be 256 bit
//
// Authentication: Unilateral (EVCC authenticates SECC per Section 7.7.3.1)
// No client certificate verification - WARP is not public infrastructure.
//
// -----------------------------------------------------------------------------
// ISO 15118-20 TLS requirements (Section 7.7.3) -> TODO for full -20 support:
// -----------------------------------------------------------------------------
// [V2G20-1264] TLS version 1.3 according to IETF RFC 8446 shall be supported
// [V2G20-2374] The SECC shall always act as the TLS server component
// [V2G20-2458] The SECC shall support all cipher suites defined in Table 6:
//              - TLS_AES_256_GCM_SHA384 (IETF RFC 5116)
//              - TLS_CHACHA20_POLY1305_SHA256 (IETF RFC 8439)
// [V2G20-1634] The SECC shall support all named groups defined in Table 7:
//              - secp521r1 (primary, per [V2G20-2674])
//              - x448 (alternative, per [V2G20-2319])
// [V2G20-2675] Key length for ECC shall be 521 bit
//
// Authentication: Mutual (EVCC authenticates SECC AND SECC authenticates EVCC)
// [V2G20-2400] SECC shall request EVCC certificate via CertificateRequest
//
// [V2G20-1235] TLS shall always be applied for ISO 15118-20
// [V2G20-1237] If TLS 1.2 or lower, EVCC shall not offer ISO 15118-20
// [V2G20-2356] If TLS 1.2 or lower, SECC shall not select ISO 15118-20
// =============================================================================

// Define this to use embedded test certificates instead of LittleFS
#define USE_EMBEDDED_TLS_CERTS

// TLS handshake state machine
enum class TlsHandshakeState : uint8_t {
    NOT_STARTED,
    IN_PROGRESS,
    COMPLETED,
    FAILED
};

class ISOTLS final
{
public:
    ISOTLS() {}

    // Setup and teardown
    bool setup();
    void cleanup();

    // Session management
    bool start_session(int socket_fd);
    void end_session();

    // Handshake (call repeatedly until returns true or handshake fails)
    bool do_handshake();

    // Read/write (only valid after handshake completes)
    ssize_t read(uint8_t *data, size_t len);
    ssize_t write(const uint8_t *data, size_t len);

    // State queries
    bool is_initialized() const { return initialized; }
    bool is_session_active() const { return session_active; }
    TlsHandshakeState get_handshake_state() const { return handshake_state; }
    const char *get_cipher_suite() const;

private:
    bool load_certificates();

    // State
    bool initialized = false;
    bool session_active = false;
    TlsHandshakeState handshake_state = TlsHandshakeState::NOT_STARTED;

    // Socket file descriptor for current session
    int socket_fd = -1;

    // mbedTLS contexts (allocated from PSRAM)
    mbedtls_ssl_context *ssl = nullptr;
    mbedtls_ssl_config *ssl_conf = nullptr;
    mbedtls_x509_crt *cert_chain = nullptr;
    mbedtls_pk_context *private_key = nullptr;
    mbedtls_entropy_context *entropy = nullptr;
    mbedtls_ctr_drbg_context *ctr_drbg = nullptr;

    // Certificate data (loaded from files or embedded)
    uint8_t *cert_chain_pem = nullptr;
    size_t cert_chain_pem_len = 0;
    uint8_t *private_key_pem = nullptr;
    size_t private_key_pem_len = 0;
};
