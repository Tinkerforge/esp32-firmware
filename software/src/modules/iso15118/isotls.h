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

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/net_sockets.h"

#if defined(MBEDTLS_SSL_SESSION_TICKETS) && defined(MBEDTLS_SSL_TICKET_C) && defined(MBEDTLS_SSL_PROTO_TLS1_3)
#define ISO15118_TLS_TICKETS 1
#include "mbedtls/ssl_ticket.h"
#else
#define ISO15118_TLS_TICKETS 0
#endif

#include "TFTools/Micros.h"

// =============================================================================
// TLS Configuration for ISO 15118
// =============================================================================
//
// This implementation supports both ISO 15118-2 and ISO 15118-20 TLS settings.
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
// ISO 15118-20 TLS requirements (Section 7.7.3):
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
    // Loads ISO 15118-2 (secp256r1) and enabled ISO 15118-20
    // (secp521r1/Ed448) certificate sets and configures TLS negotiation.
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

    // Set when the certificate source changed, triggers a reload before the next TLS connection
    bool certs_dirty = false;

    // Mutual TLS authentication control (ISO 15118-20)
    // When enabled (default), TLS 1.3 handshakes request and verify the EVCC
    // certificate per [V2G20-2400]. When disabled, TLS 1.3 uses unilateral
    // authentication (like TLS 1.2), which avoids the ~5s secp521r1 handshake
    // overhead on the ESP32 but is not standard-compliant.
    void set_mutual_auth_enabled(bool enabled);
    bool is_mutual_auth_enabled() const;

    // TLS version queries
    bool is_tls13_active() const;

    bool is_mutual_auth_session() const { return mutual_auth_session; }
    bool is_resumed_session() const { return resumed_session; }

    // Returns the negotiated TLS version string (e.g., "TLSv1.2", "TLSv1.3")
    // Only valid after handshake completes
    const char *get_tls_version_string() const;

    // Returns the negotiated cipher suite name
    const char *get_cipher_suite() const;

    // Certificate selection callback (called from mbedtls cert_cb)
    // Selects the appropriate certificate based on the negotiated TLS version.
    // Returns 0 on success, non-zero on failure.
    int select_certificate_for_handshake(mbedtls_ssl_context *ssl);

    int select_iso20_certificate_authority(mbedtls_ssl_context *ssl, const unsigned char *data, size_t data_len);
    int get_iso20_certificate_authorities(mbedtls_ssl_context *ssl, const unsigned char **data, size_t *data_len) const;
    int select_iso2_trusted_ca(mbedtls_ssl_context *ssl, const unsigned char *data, size_t data_len);
    int accept_iso2_status_request_v2(mbedtls_ssl_context *ssl, const unsigned char *data, size_t data_len);
    int iso2_status_request_v2_available(mbedtls_ssl_context *ssl) const;
    int get_iso2_status_response_v2(mbedtls_ssl_context *ssl, const unsigned char **response_list, size_t *response_list_len);

    bool get_ocsp_staple(const mbedtls_ssl_context *ssl_ctx, size_t index, const unsigned char **der, size_t *der_len) const;

private:
    static constexpr size_t CERTS_MAX_VERIFY = 8;

    struct verification_context_t {
        mbedtls_x509_crt *certs[CERTS_MAX_VERIFY];
        StaticSemaphore_t sem_buf;
        SemaphoreHandle_t sem_handle;
        uint8_t leaf_sha256[32];
        bool leaf_cert_cached;
        bool async_started;
        bool intermediates_valid;
        mbedtls_x509_crt *anchor_root; // trust store root the chain verified against
    };

    struct cert_cache_entry {
        micros_t last_seen;
        uint8_t sha256[32];
        char *dn;
        cert_cache_entry *next;
    };

    bool load_certificates();
    bool parse_iso2_candidates();
    void free_iso2_candidate(size_t index);
    bool parse_iso20_candidates();
    void free_iso20_candidate(size_t index);
    bool build_iso20_certificate_authorities();
    bool configure_ssl_policy();
    bool apply_group_policy();
    bool leaf_cert_is_cached();
    void cache_leaf_cert();
    void verify_intermediate_certs();
    mbedtls_x509_crt *find_anchor_by_name(const mbedtls_x509_crt *topmost) const;
    void hand_off_vehicle_chain();
    static void verify_certs_task(void *ctx);
    static int cert_verify(void *ctx, mbedtls_x509_crt *cert, int index, uint32_t *flags);

#if ISO15118_TLS_TICKETS
    static constexpr uint32_t TICKET_LIFETIME_S = 3600;
    bool setup_tickets();
    static int ticket_write_cb(void *ctx, const mbedtls_ssl_session *session, unsigned char *start, const unsigned char *end, size_t *tlen, uint32_t *lifetime);
    static int ticket_parse_cb(void *ctx, mbedtls_ssl_session *session, unsigned char *buf, size_t len);
    mbedtls_ssl_ticket_context *ticket_ctx = nullptr;
#endif
    bool ticket_psk_accepted = false;
    bool resumed_session = false;

    // State
    bool initialized = false;
    bool session_active = false;
    bool mutual_auth_enabled = true; // Default: enabled per [V2G20-2400]
    bool mutual_auth_session = false;
    bool group_policy_applied = false; // HUB20-533-005
    bool iso20_allowed = true;
    TlsHandshakeState handshake_state = TlsHandshakeState::NOT_STARTED;

    // Async verification
    verification_context_t *verification_context = nullptr;
    cert_cache_entry *peer_cert_cache = nullptr;

    // Socket file descriptor for current session
    int socket_fd = -1;

    // mbedTLS contexts (allocated from PSRAM)
    mbedtls_ssl_context *ssl = nullptr;
    mbedtls_ssl_config *ssl_conf = nullptr;
    mbedtls_entropy_context *entropy = nullptr;
    mbedtls_ctr_drbg_context *ctr_drbg = nullptr;

    // ISO 15118-2 certificates (secp256r1), freshest candidate first.
    static constexpr size_t ISO2_CANDIDATE_MAX = 4;
    static constexpr size_t ISO2_OCSP_MAX = 4;
    struct iso2_candidate_t {
        uint32_t chain_id = 0;
        mbedtls_x509_crt *cert_chain = nullptr;
        mbedtls_pk_context *private_key = nullptr;
        mbedtls_x509_crt *root = nullptr;
        uint8_t *cert_chain_pem = nullptr;
        size_t cert_chain_pem_len = 0;
        uint8_t *private_key_pem = nullptr;
        size_t private_key_pem_len = 0;
        uint8_t *root_pem = nullptr;
        size_t root_pem_len = 0;
        uint8_t root_cert_sha1[20] = {};
        uint8_t root_key_sha1[20] = {};
        uint8_t *ocsp_der[ISO2_OCSP_MAX] = {};
        size_t ocsp_der_len[ISO2_OCSP_MAX] = {};
        uint8_t *ocsp_response_list = nullptr;
        size_t ocsp_response_list_len = 0;
        size_t cert_count = 0;
        bool ocsp_complete = false;
    };
    iso2_candidate_t iso2_candidates[ISO2_CANDIDATE_MAX];
    size_t iso2_candidate_count = 0;
    size_t selected_iso2_candidate = 0;
    bool certificate_store_live = false;
    bool iso2_status_v2_requested = false;

    // ISO 15118-20 certificates and their anchoring roots. The roots are used
    // only for ClientHello certificate_authorities selection [V2G20-2379/3376]
    // and are never included in the transmitted chain [V2G20-2399].
    static constexpr size_t ISO20_CANDIDATE_MAX = 4;
    static constexpr size_t OCSP_STAPLE_MAX = 4;
    struct iso20_candidate_t {
        uint32_t chain_id = 0;
        mbedtls_x509_crt *cert_chain = nullptr;
        mbedtls_pk_context *private_key = nullptr;
        mbedtls_x509_crt *root = nullptr;
        uint8_t *cert_chain_pem = nullptr;
        size_t cert_chain_pem_len = 0;
        uint8_t *private_key_pem = nullptr;
        size_t private_key_pem_len = 0;
        uint8_t *root_pem = nullptr;
        size_t root_pem_len = 0;
        uint8_t *staple_der[OCSP_STAPLE_MAX] = {};
        size_t staple_der_len[OCSP_STAPLE_MAX] = {};
    };
    iso20_candidate_t iso20_candidates[ISO20_CANDIDATE_MAX];
    size_t iso20_candidate_count = 0;
    size_t selected_iso20_candidate = 0;
    bool iso20_certificate_authorities_seen = false;
    bool iso20_certificate_authority_matched = false;

    // Trusted root CA certificates for mutual TLS authentication (ISO 15118-20)
    // [V2G20-2400] SECC shall request EVCC certificate via CertificateRequest
    // [V2G20-2338] SECC shall have at least one V2G or OEM root CA certificate
    // Both OEM Root CA and V2G Root CA are loaded into this trust store.
    // During TLS 1.3 handshake, the EVCC's vehicle certificate chain is
    // verified against these root CAs.
    mbedtls_x509_crt *trusted_ca_iso20 = nullptr;
    uint8_t *iso20_certificate_authorities = nullptr;
    size_t iso20_certificate_authorities_len = 0;
    uint8_t *oem_root_ca_pem_iso20 = nullptr;
    size_t oem_root_ca_pem_len_iso20 = 0;
    uint8_t *v2g_root_ca_pem_iso20 = nullptr;
    size_t v2g_root_ca_pem_len_iso20 = 0;

};
