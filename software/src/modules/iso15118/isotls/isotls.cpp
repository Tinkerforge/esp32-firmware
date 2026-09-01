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

#include "../isotls.h"
#include "../iso15118.h"

#include "event_log_prefix.h"
#include "tools/freertos.h"
#include "tools/malloc.h"

#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "mbedtls/error.h"

#include "gcc_warnings.h"

extern ISO15118 iso15118;

// File-static pointer to the single ISOTLS instance, used by the certificate
// selection callback. Only safe as long as there is exactly one ISOTLS
// instance in the firmware.
static ISOTLS *s_isotls_instance = nullptr;

static int tls_cert_selection_callback(mbedtls_ssl_context *ssl)
{
    if (s_isotls_instance == nullptr) {
        return -1;
    }
    return s_isotls_instance->select_certificate_for_handshake(ssl);
}

// Override of the hook added by mbedTLS OCSP stapling patch.
// Called while the TLS 1.3 Certificate message is written and
// the client sent a status_request extension.
// Returning 0 staples the DER OCSPResponse into the CertificateEntry [V2G20-2388].
extern "C" int mbedtls_ssl_tls13_ocsp_response_cb(mbedtls_ssl_context *ssl, size_t index, const unsigned char **der, size_t *der_len);

extern "C" int mbedtls_ssl_tls13_ocsp_response_cb(mbedtls_ssl_context *ssl, size_t index, const unsigned char **der, size_t *der_len)
{
    (void)ssl;
    if (s_isotls_instance == nullptr || !s_isotls_instance->get_ocsp_staple(ssl, index, der, der_len)) {
        return -1;
    }
    return 0;
}

extern "C" int mbedtls_ssl_tls13_certificate_authorities_cb(mbedtls_ssl_context *ssl, const unsigned char *data, size_t data_len)
{
    return s_isotls_instance == nullptr ? -1 : s_isotls_instance->select_iso20_certificate_authority(ssl, data, data_len);
}

extern "C" int mbedtls_ssl_tls13_write_certificate_authorities_cb(mbedtls_ssl_context *ssl, const unsigned char **data, size_t *data_len)
{
    return s_isotls_instance == nullptr ? -1 : s_isotls_instance->get_iso20_certificate_authorities(ssl, data, data_len);
}

extern "C" int mbedtls_ssl_tls12_trusted_ca_keys_cb(mbedtls_ssl_context *ssl, const unsigned char *data, size_t data_len)
{
    return s_isotls_instance == nullptr ? -1 : s_isotls_instance->select_iso2_trusted_ca(ssl, data, data_len);
}

extern "C" int mbedtls_ssl_tls12_status_request_v2_cb(mbedtls_ssl_context *ssl, const unsigned char *data, size_t data_len)
{
    return s_isotls_instance == nullptr ? -1 : s_isotls_instance->accept_iso2_status_request_v2(ssl, data, data_len);
}

extern "C" int mbedtls_ssl_tls12_status_request_v2_available_cb(mbedtls_ssl_context *ssl)
{
    return s_isotls_instance == nullptr ? -1 : s_isotls_instance->iso2_status_request_v2_available(ssl);
}

extern "C" int mbedtls_ssl_tls12_status_request_v2_response_cb(mbedtls_ssl_context *ssl, const unsigned char **response_list, size_t *response_list_len)
{
    return s_isotls_instance == nullptr ? -1 : s_isotls_instance->get_iso2_status_response_v2(ssl, response_list, response_list_len);
}

// EWOULDBLOCK and EAGAIN can be the same value depending on compiler version
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlogical-op"

// mbedTLS send callback for non-blocking socket I/O
static int tls_net_send(void *ctx, const unsigned char *buf, size_t len)
{
    int fd = *static_cast<int*>(ctx);
    ssize_t ret = send(fd, buf, len, 0);

    if (ret < 0) {
        int saved_errno = errno;
        if (saved_errno == EWOULDBLOCK || saved_errno == EAGAIN) {
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }
        iso15118.trace("ISOTLS: send() failed on fd %d: errno %d (%s), len=%zu",
                        fd, saved_errno, strerror(saved_errno), len);
        return MBEDTLS_ERR_NET_SEND_FAILED;
    }
    return static_cast<int>(ret);
}

// mbedTLS receive callback for non-blocking socket I/O
static int tls_net_recv(void *ctx, unsigned char *buf, size_t len)
{
    int fd = *static_cast<int*>(ctx);
    ssize_t ret = recv(fd, buf, len, 0);

    if (ret < 0) {
        int saved_errno = errno;
        if (saved_errno == EWOULDBLOCK || saved_errno == EAGAIN) {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
        iso15118.trace("ISOTLS: recv() failed on fd %d: errno %d (%s), len=%zu",
                        fd, saved_errno, strerror(saved_errno), len);
        return MBEDTLS_ERR_NET_RECV_FAILED;
    }
    if (ret == 0) {
        iso15118.trace("ISOTLS: recv() returned 0 on fd %d (peer closed)", fd);
        return MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY;
    }
    return static_cast<int>(ret);
}

#pragma GCC diagnostic pop

bool ISOTLS::setup()
{
    if (initialized) {
        return true;
    }

    iso15118.trace("ISOTLS: Setting up TLS for ISO 15118 (TLS 1.2 + 1.3)");

    if (!load_certificates()) {
        iso15118.trace("ISOTLS: Failed to load certificates");
        cleanup();
        return false;
    }

    // Allocate mbedTLS contexts from PSRAM
    ssl = static_cast<mbedtls_ssl_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_ssl_context)));
    ssl_conf = static_cast<mbedtls_ssl_config*>(calloc_psram_or_dram(1, sizeof(mbedtls_ssl_config)));
    entropy = static_cast<mbedtls_entropy_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_entropy_context)));
    ctr_drbg = static_cast<mbedtls_ctr_drbg_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_ctr_drbg_context)));
    if (ssl == nullptr || ssl_conf == nullptr || entropy == nullptr || ctr_drbg == nullptr) {
        iso15118.trace("ISOTLS: Failed to allocate mbedTLS contexts");
        cleanup();
        return false;
    }

    // Initialize mbedTLS contexts
    mbedtls_ssl_init(ssl);
    mbedtls_ssl_config_init(ssl_conf);
    mbedtls_entropy_init(entropy);
    mbedtls_ctr_drbg_init(ctr_drbg);

    int ret;

    // Seed the random number generator
    const char *pers = "iso15118_secc";
    ret = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy,
                                reinterpret_cast<const unsigned char *>(pers), strlen(pers));
    if (ret != 0) {
        iso15118.trace("ISOTLS: mbedtls_ctr_drbg_seed failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }

    if (!parse_iso2_candidates()) {
        iso15118.trace("ISOTLS: No parseable ISO 15118-2 certificate candidates");
        cleanup();
        return false;
    }

    if (iso20_allowed && !parse_iso20_candidates()) {
        iso15118.trace("ISOTLS: No parseable ISO 15118-20 certificate candidates");
        iso20_allowed = false;
    }

    // Parse trusted root CA certificates for mutual TLS (ISO 15118-20)
    // [V2G20-2400] SECC shall request EVCC certificate via CertificateRequest
    // [V2G20-2338] SECC shall have at least one V2G or OEM root CA certificate
    trusted_ca_iso20 = static_cast<mbedtls_x509_crt*>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
    if (trusted_ca_iso20 == nullptr) {
        iso15118.trace("ISOTLS: Failed to allocate trusted CA context");
        cleanup();
        return false;
    }
    mbedtls_x509_crt_init(trusted_ca_iso20);

    if (oem_root_ca_pem_iso20 != nullptr) {
        ret = mbedtls_x509_crt_parse(trusted_ca_iso20, oem_root_ca_pem_iso20, oem_root_ca_pem_len_iso20);
        if (ret != 0) {
            iso15118.trace("ISOTLS: OEM Root CA parse failed: -0x%04x", static_cast<unsigned>(-ret));
        } else {
            iso15118.trace("ISOTLS: OEM Root CA parsed successfully");
        }
    }

    if (v2g_root_ca_pem_iso20 != nullptr) {
        ret = mbedtls_x509_crt_parse(trusted_ca_iso20, v2g_root_ca_pem_iso20, v2g_root_ca_pem_len_iso20);
        if (ret != 0) {
            iso15118.trace("ISOTLS: V2G Root CA parse failed: -0x%04x", static_cast<unsigned>(-ret));
        } else {
            iso15118.trace("ISOTLS: V2G Root CA parsed successfully");
        }
    }

    int trusted_ca_count = 0;
    for (mbedtls_x509_crt *root = trusted_ca_iso20; root != nullptr && root->raw.p != nullptr; root = root->next) {
        ++trusted_ca_count;
    }
    if (trusted_ca_count > 0) {
        iso15118.trace("ISOTLS: %d trusted root CA(s) loaded for ISO 15118-20 mutual TLS", trusted_ca_count);
        if (!build_iso20_certificate_authorities()) {
            iso15118.trace("ISOTLS: Failed to build complete V2G20-2401 CertificateRequest CA list");
            cleanup();
            return false;
        }
    } else {
        iso15118.trace("ISOTLS: WARNING: No trusted root CAs loaded - mutual TLS authentication disabled");
        mbedtls_x509_crt_free(trusted_ca_iso20);
        free_any(trusted_ca_iso20);
        trusted_ca_iso20 = nullptr;
    }

    if (!configure_ssl_policy()) {
        cleanup();
        return false;
    }

    // Certificate selection callback (f_cert_cb): fires after ClientHello
    // processing, when the negotiated TLS version is known. The callback selects:
    //   - TLS 1.3 -> ISO 15118-20 cert (secp521r1) + mutual auth with trusted CAs
    //   - TLS 1.2 -> ISO 15118-2 cert  (secp256r1) + unilateral auth (no client cert)
    // This is deterministic and independent of client signature_algorithms ordering.
    s_isotls_instance = this;
    mbedtls_ssl_conf_cert_cb(ssl_conf, tls_cert_selection_callback);
    iso15118.trace("ISOTLS: Registered certificate selection callback");

#if ISO15118_TLS_TICKETS
    if (!setup_tickets()) {
        iso15118.trace("ISOTLS: Session tickets unavailable, continuing without resumption");
    }
#endif

    // Setup SSL context with configuration
    ret = mbedtls_ssl_setup(ssl, ssl_conf);
    if (ret != 0) {
        iso15118.trace("ISOTLS: mbedtls_ssl_setup failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }

    initialized = true;
    iso15118.trace("ISOTLS: TLS setup complete");
    return true;
}

void ISOTLS::cleanup()
{
    end_session();

    if (s_isotls_instance == this) {
        s_isotls_instance = nullptr;
    }

    if (ssl != nullptr) {
        mbedtls_ssl_free(ssl);
        free_any(ssl);
        ssl = nullptr;
    }

    if (ssl_conf != nullptr) {
        mbedtls_ssl_config_free(ssl_conf);
        free_any(ssl_conf);
        ssl_conf = nullptr;
    }

#if ISO15118_TLS_TICKETS
    if (ticket_ctx != nullptr) {
        mbedtls_ssl_ticket_free(ticket_ctx);
        free_any(ticket_ctx);
        ticket_ctx = nullptr;
    }
#endif

    for (size_t i = 0; i < ISO2_CANDIDATE_MAX; ++i) {
        free_iso2_candidate(i);
    }
    iso2_candidate_count = 0;
    selected_iso2_candidate = 0;
    selected_iso20_candidate = 0;
    iso20_certificate_authorities_seen = false;
    iso20_certificate_authority_matched = false;
    certificate_store_live = false;
    iso2_status_v2_requested = false;

    for (size_t i = 0; i < ISO20_CANDIDATE_MAX; ++i) {
        free_iso20_candidate(i);
    }
    iso20_candidate_count = 0;

    if (trusted_ca_iso20 != nullptr) {
        mbedtls_x509_crt_free(trusted_ca_iso20);
        free_any(trusted_ca_iso20);
        trusted_ca_iso20 = nullptr;
    }
    free_any(iso20_certificate_authorities);
    iso20_certificate_authorities = nullptr;
    iso20_certificate_authorities_len = 0;

    if (entropy != nullptr) {
        mbedtls_entropy_free(entropy);
        free_any(entropy);
        entropy = nullptr;
    }

    if (ctr_drbg != nullptr) {
        mbedtls_ctr_drbg_free(ctr_drbg);
        free_any(ctr_drbg);
        ctr_drbg = nullptr;
    }

    if (oem_root_ca_pem_iso20 != nullptr) {
        free_any(oem_root_ca_pem_iso20);
        oem_root_ca_pem_iso20 = nullptr;
        oem_root_ca_pem_len_iso20 = 0;
    }

    if (v2g_root_ca_pem_iso20 != nullptr) {
        free_any(v2g_root_ca_pem_iso20);
        v2g_root_ca_pem_iso20 = nullptr;
        v2g_root_ca_pem_len_iso20 = 0;
    }

    initialized = false;
}

#if ISO15118_TLS_TICKETS
// TLS 1.3 session resumption via psk_dhe_ke session tickets [V2G20-1675 ff].
// The SECC issues one NewSessionTicket after a full TLS 1.3 handshake and
// accepts the resulting PSK only together with an ephemeral key share
// (psk_dhe_ke, Table 9 of ISO 15118-20).
// Early data is rejected (MBEDTLS_SSL_EARLY_DATA off) [V2G20-1612/2031].
// PSKs only ever come from tickets, so a PSK handshake can only resume a
// previously established session [V2G20-1679].
bool ISOTLS::setup_tickets()
{
    ticket_ctx = static_cast<mbedtls_ssl_ticket_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_ssl_ticket_context)));
    if (ticket_ctx == nullptr) {
        return false;
    }

    mbedtls_ssl_ticket_init(ticket_ctx);

    // [V2G20-2024..2026] lifetime within 20 s and 86400 s. Expired tickets
    // fail the parse and fall back to a full handshake [V2G20-2033]
    int ret = mbedtls_ssl_ticket_setup(ticket_ctx, mbedtls_ctr_drbg_random, ctr_drbg, MBEDTLS_CIPHER_AES_256_GCM, TICKET_LIFETIME_S);
    if (ret != 0) {
        iso15118.trace("ISOTLS: mbedtls_ssl_ticket_setup failed: -0x%04x", static_cast<unsigned>(-ret));
        mbedtls_ssl_ticket_free(ticket_ctx);
        free_any(ticket_ctx);
        ticket_ctx = nullptr;
        return false;
    }

    mbedtls_ssl_conf_session_tickets_cb(ssl_conf, ticket_write_cb, ticket_parse_cb, this);
    // One ticket per full handshake, we offer no VAS [V2G20-2023]
    mbedtls_ssl_conf_new_session_tickets(ssl_conf, 1);
    // psk_dhe_ke only besides the certificate based ephemeral mode [V2G20-1678]
    mbedtls_ssl_conf_tls13_key_exchange_modes(ssl_conf, MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_EPHEMERAL | MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_EPHEMERAL);

    iso15118.trace("ISOTLS: TLS 1.3 session tickets enabled (lifetime %us)", static_cast<unsigned>(TICKET_LIFETIME_S));
    return true;
}

int ISOTLS::ticket_write_cb(void *ctx, const mbedtls_ssl_session *session, unsigned char *start, const unsigned char *end, size_t *tlen, uint32_t *lifetime)
{
    ISOTLS *self = static_cast<ISOTLS*>(ctx);

    // Tickets are TLS 1.3 only.
    if ((self->ssl == nullptr) || (mbedtls_ssl_get_version_number(self->ssl) != MBEDTLS_SSL_VERSION_TLS1_3)) {
        *tlen = 0;
        *lifetime = 0;
        return MBEDTLS_ERR_SSL_BAD_INPUT_DATA;
    }

    return mbedtls_ssl_ticket_write(self->ticket_ctx, session, start, end, tlen, lifetime);
}

int ISOTLS::ticket_parse_cb(void *ctx, mbedtls_ssl_session *session, unsigned char *buf, size_t len)
{
    ISOTLS *self = static_cast<ISOTLS*>(ctx);

    if (self->ssl == nullptr || mbedtls_ssl_get_version_number(self->ssl) != MBEDTLS_SSL_VERSION_TLS1_3) {
        return MBEDTLS_ERR_SSL_INVALID_MAC;
    }

    int ret = mbedtls_ssl_ticket_parse(self->ticket_ctx, session, buf, len);
    if (ret == 0) {
        // The EV offered a valid ticket PSK. Will become "resumed_session" once the handshake completed
        self->ticket_psk_accepted = true;
    }

    return ret;
}
#endif

bool ISOTLS::start_session(int fd)
{
    if (!initialized || ssl == nullptr) {
        iso15118.trace("ISOTLS: Cannot start session - not initialized");
        return false;
    }

    // Reset SSL state for new connection
    mbedtls_ssl_session_reset(ssl);

    // Store socket fd and set up I/O callbacks
    socket_fd = fd;
    mbedtls_ssl_set_bio(ssl, &socket_fd, tls_net_send, tls_net_recv, nullptr);

    handshake_state = TlsHandshakeState::IN_PROGRESS;
    session_active = false;
    group_policy_applied = false;
    mutual_auth_session = false;
    ticket_psk_accepted = false;
    resumed_session = false;
    selected_iso2_candidate = 0;
    selected_iso20_candidate = 0;
    iso20_certificate_authorities_seen = false;
    iso20_certificate_authority_matched = false;
    iso2_status_v2_requested = false;

    iso15118.trace("ISOTLS: Starting TLS session on socket %d", fd);
    return true;
}

void ISOTLS::end_session()
{
    if (session_active && ssl != nullptr) {
        // Send close_notify alert (best effort, ignore errors)
        mbedtls_ssl_close_notify(ssl);
    }

    // Free a stale verification context from an aborted handshake. The next
    // handshake would otherwise inherit it in mid-handshake state.
    if (verification_context != nullptr) {
        if (verification_context->async_started) {
            xQueueSemaphoreTake(verification_context->sem_handle, portMAX_DELAY_nowarn);
        }
        vQueueDelete(static_cast<QueueHandle_t>(verification_context->sem_handle));
        free(verification_context);
        verification_context = nullptr;
    }

    session_active = false;
    mutual_auth_session = false;
    ticket_psk_accepted = false;
    resumed_session = false;
    selected_iso2_candidate = 0;
    iso2_status_v2_requested = false;
    handshake_state = TlsHandshakeState::NOT_STARTED;
    socket_fd = -1;
}

// Don't inline error logger, keeps the buffer off the stack.
[[gnu::noinline]]
static void log_mbedtls_error(int error, const char *msg)
{
    char error_buf[128];
    mbedtls_strerror(error, error_buf, sizeof(error_buf));
    iso15118.trace("ISOTLS: %s: -0x%04x (%s)", msg, static_cast<unsigned>(-error), error_buf);
}

bool ISOTLS::do_handshake()
{
    if (!initialized || ssl == nullptr) {
        iso15118.trace("ISOTLS: TLS not initialized");
        return false;
    }

    // The group policy needs the peeked ClientHello before the handshake starts
    if (!group_policy_applied && !apply_group_policy()) {
        return false;
    }

    int ret = mbedtls_ssl_handshake(ssl);

    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
        // Handshake in progress, need to call again
        return false;
    }

    if (verification_context != nullptr && verification_context->async_started) {
        xQueueSemaphoreTake(verification_context->sem_handle, portMAX_DELAY_nowarn);
        verification_context->async_started = false;
    }

    if (ret == 0) {
        // The verification context is allocated per mutual auth ClientHello,
        // but certificates only arrive on a full handshake. A TLS 1.3 session
        // resumed via ticket PSK completes without any certificate exchange,
        // mbedTLS fails a full VERIFY_REQUIRED handshake without client certs
        // itself, so an empty context here can only be a PSK resumption.
        const bool certs_presented = verification_context != nullptr && verification_context->certs[0] != nullptr;
        if (!certs_presented || verification_context->intermediates_valid) {
            // Handshake completed successfully
            handshake_state = TlsHandshakeState::COMPLETED;
            session_active = true;

            const char *tls_version = get_tls_version_string();
            const char *cipher = get_cipher_suite();

            iso15118.trace("ISOTLS: Handshake successful: %s, using %s", tls_version ? tls_version : "TLS version unknown", cipher ? cipher : "unknown cipher suite");

            if (certs_presented && !verification_context->leaf_cert_cached) {
                cache_leaf_cert();
            }

            // [V2G20-2356] If TLS 1.2 or lower, SECC shall not select ISO 15118-20
            if (is_tls13_active()) {
                // Accepted ticket PSK and no presented client chain: This was a PSK resumption [V2G20-2677]
                resumed_session = ticket_psk_accepted && !certs_presented;
                if (resumed_session) {
                    iso15118.trace("ISOTLS: TLS 1.3 session resumed via ticket PSK - no V2G allowed [V2G20-2677]");
                } else {
                    iso15118.trace("ISOTLS: TLS 1.3 negotiated - ISO 15118-20 allowed");
                }

                // Log mutual authentication result
                if (!resumed_session && mutual_auth_enabled && trusted_ca_iso20 != nullptr) {
                    uint32_t verify_flags = mbedtls_ssl_get_verify_result(ssl);
                    if (verify_flags == 0) {
                        iso15118.trace("ISOTLS: Mutual TLS: EVCC certificate verified successfully");
                    } else {
                        // This should not happen with VERIFY_REQUIRED (handshake would have failed),
                        // but log it for completeness
                        char vrfy_buf[256];
                        mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", verify_flags);
                        iso15118.trace("ISOTLS: Mutual TLS: EVCC certificate verification issues:\n%s", vrfy_buf);
                    }
                }

                if (certs_presented) {
                    mutual_auth_session = true;
                    hand_off_vehicle_chain();
                }
            } else {
                iso15118.trace("ISOTLS: TLS 1.2 negotiated - ISO 15118-20 NOT allowed per [V2G20-2356]");
            }
        } else {
            iso15118.trace("Intermediate certificate validation failed");
            ret = MBEDTLS_ERR_X509_CERT_VERIFY_FAILED;

            // end_session() won't send a session close alert because the session isn't marked as active, so send an appropriate alert here.
            mbedtls_ssl_send_alert_message(ssl, MBEDTLS_SSL_ALERT_LEVEL_FATAL, MBEDTLS_SSL_ALERT_MSG_BAD_CERT);
        }
    }

    if (verification_context != nullptr) {
        vQueueDelete(static_cast<QueueHandle_t>(verification_context->sem_handle));
        free(verification_context);
        verification_context = nullptr;
    }

    if (ret == 0) {
        return true;
    } else {
        // Handshake failed
        log_mbedtls_error(ret, "Handshake failed");

        // HUB20-533-004 requires a handshake_failure alert
        if (ret == MBEDTLS_ERR_SSL_HANDSHAKE_FAILURE) {
            mbedtls_ssl_send_alert_message(ssl, MBEDTLS_SSL_ALERT_LEVEL_FATAL, MBEDTLS_SSL_ALERT_MSG_HANDSHAKE_FAILURE);
        }

        handshake_state = TlsHandshakeState::FAILED;

        return false;
    }
}

ssize_t ISOTLS::read(uint8_t *data, size_t len)
{
    if (!session_active || (ssl == nullptr)) {
        return -1;
    }

    int ret = mbedtls_ssl_read(ssl, data, len);

    if (ret >= 0) {
        return ret;
    }

    if ((ret == MBEDTLS_ERR_SSL_WANT_READ) || (ret == MBEDTLS_ERR_SSL_WANT_WRITE)) {
        errno = EWOULDBLOCK;
        return -1;
    }

    if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
        return 0; // Connection closed
    }

    log_mbedtls_error(ret, "Read error");
    return -1;
}

ssize_t ISOTLS::write(const uint8_t *data, size_t len)
{
    if (!session_active || ssl == nullptr) {
        return -1;
    }

    int ret = mbedtls_ssl_write(ssl, data, len);

    if (ret >= 0) {
        return ret;
    }

    if ((ret == MBEDTLS_ERR_SSL_WANT_READ) || (ret == MBEDTLS_ERR_SSL_WANT_WRITE)) {
        errno = EWOULDBLOCK;
        return -1;
    }

    log_mbedtls_error(ret, "Write error");
    return -1;
}

const char *ISOTLS::get_cipher_suite() const
{
    if (!session_active || (ssl == nullptr)) {
        return nullptr;
    }
    return mbedtls_ssl_get_ciphersuite(ssl);
}

const char *ISOTLS::get_tls_version_string() const
{
    if (!session_active || (ssl == nullptr)) {
        return nullptr;
    }
    return mbedtls_ssl_get_version(ssl);
}

bool ISOTLS::is_tls13_active() const
{
    if (!session_active || (ssl == nullptr)) {
        return false;
    }

    return mbedtls_ssl_get_version_number(ssl) == MBEDTLS_SSL_VERSION_TLS1_3;
}

int ISOTLS::select_certificate_for_handshake(mbedtls_ssl_context *ssl_ctx)
{
    size_t sni_len;
    const unsigned char *sni = mbedtls_ssl_get_hs_sni(ssl_ctx, &sni_len);

    if (sni_len > 0) {
        iso15118.trace("ISOTLS: cert_cb: EVCC thinks we are '%.*s'", static_cast<int>(sni_len), sni);
    } else {
        iso15118.trace("ISOTLS: cert_cb: No SNI from EVCC");
    }

    mbedtls_ssl_protocol_version ver = mbedtls_ssl_get_version_number(ssl_ctx);

    if (ver == MBEDTLS_SSL_VERSION_TLS1_3 && iso20_allowed && iso20_candidate_count > 0) {
        if (selected_iso20_candidate >= iso20_candidate_count) {
            selected_iso20_candidate = 0;
        }
        mbedtls_ssl_set_hs_own_cert(ssl_ctx, nullptr, nullptr);
        const size_t first = iso20_certificate_authority_matched ? selected_iso20_candidate : 0;
        const size_t end = iso20_certificate_authority_matched ? selected_iso20_candidate + 1 : iso20_candidate_count;
        for (size_t i = first; i < end; ++i) {
            iso20_candidate_t &candidate = iso20_candidates[i];
            int ret = mbedtls_ssl_set_hs_own_cert(ssl_ctx, candidate.cert_chain, candidate.private_key);
            if (ret != 0) {
                iso15118.trace("ISOTLS: cert_cb: Failed to add ISO20 candidate %zu: -0x%04x", i, static_cast<unsigned>(-ret));
                return ret;
            }
        }
        iso15118.trace("ISOTLS: cert_cb: TLS 1.3 offering ISO20 candidate range [%zu, %zu), certificate_authorities %s",
                       first, end, !iso20_certificate_authorities_seen ? "absent" :
                       (iso20_certificate_authority_matched ? "matched" : "unmatched fallback"));

        // [V2G20-2400] SECC shall request EVCC certificate via CertificateRequest
        // Enable mutual authentication for TLS 1.3 (ISO 15118-20) if enabled
        if (mutual_auth_enabled) {
            if (trusted_ca_iso20 != nullptr) {
                mbedtls_ssl_set_hs_ca_chain(ssl_ctx, trusted_ca_iso20, nullptr);
                mbedtls_ssl_set_hs_authmode(ssl_ctx, MBEDTLS_SSL_VERIFY_REQUIRED);

                if (verification_context == nullptr) {
                    verification_context = static_cast<decltype(verification_context)>(heap_caps_calloc(1, sizeof(*verification_context), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));

                    if (verification_context == nullptr) {
                        return MBEDTLS_ERR_SSL_ALLOC_FAILED;
                    }

                    verification_context->sem_handle = xSemaphoreCreateBinaryStatic_nowarn(&verification_context->sem_buf);

                    if (verification_context->sem_handle == nullptr) {
                        free(verification_context);
                        verification_context = nullptr;

                        return MBEDTLS_ERR_SSL_ALLOC_FAILED;
                    }
                } else {
                    // Second ClientHello after a HelloRetryRequest, the callback runs once per ClientHello. Reuse the context.
                    memset(verification_context->certs, 0, sizeof(verification_context->certs));
                    verification_context->leaf_cert_cached = false;
                    verification_context->async_started = false;
                    verification_context->intermediates_valid = false;
                    verification_context->anchor_root = nullptr;
                }

                mbedtls_ssl_set_verify(ssl_ctx, &cert_verify, this);

                iso15118.trace("ISOTLS: cert_cb: Mutual TLS enabled - EVCC certificate will be verified");
            } else {
                iso15118.trace("ISOTLS: cert_cb: WARNING: No trusted CAs loaded, mutual TLS disabled");
            }
        } else {
            mbedtls_ssl_set_hs_authmode(ssl_ctx, MBEDTLS_SSL_VERIFY_NONE);
            iso15118.trace("ISOTLS: cert_cb: Mutual TLS disabled by configuration");
        }

        return 0;
    } else if (iso2_candidate_count > 0) {
        if (selected_iso2_candidate >= iso2_candidate_count) {
            selected_iso2_candidate = 0;
        }
        iso2_candidate_t &candidate = iso2_candidates[selected_iso2_candidate];
        iso15118.trace("ISOTLS: cert_cb: TLS 1.2 negotiated, selecting ISO 15118-2 candidate %zu (chain ID %u)",
                        selected_iso2_candidate, static_cast<unsigned>(candidate.chain_id));
        // ISO 15118-2: Unilateral authentication only (no client cert)
        mbedtls_ssl_set_hs_authmode(ssl_ctx, MBEDTLS_SSL_VERIFY_NONE);
        mbedtls_ssl_set_hs_own_cert(ssl_ctx, nullptr, nullptr);
        return mbedtls_ssl_set_hs_own_cert(ssl_ctx, candidate.cert_chain, candidate.private_key);
    }

    iso15118.trace("ISOTLS: cert_cb: No matching certificate available for TLS version 0x%04x",
                    static_cast<unsigned>(ver));
    return -1;
}

void ISOTLS::set_mutual_auth_enabled(bool enabled)
{
    mutual_auth_enabled = enabled;
    iso15118.trace("ISOTLS: Mutual TLS authentication %s", enabled ? "enabled" : "disabled");
}

bool ISOTLS::is_mutual_auth_enabled() const
{
    return mutual_auth_enabled;
}
