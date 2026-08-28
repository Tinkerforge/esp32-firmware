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

#include "isotls.h"
#include "options.h"
#if OPTIONS_ISO15118_DEV_CERTS_ENABLED()
#include "dev_certs.h"
#endif

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tools/freertos.h"
#include "tools/malloc.h"

#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <memory>

#include "mbedtls/error.h"
#include "mbedtls/sha256.h"
#include "mbedtls/ssl_ciphersuites.h"
#include "mbedtls/version.h"
#include "mbedtls/x509_crt.h"

#include "gcc_warnings.h"

// ISO 15118 cipher suites (TLS 1.2 + TLS 1.3)
// TLS 1.3: [V2G20-2458] Table 6
// TLS 1.2: [V2G2-602] Table 7
static const int iso15118_ciphersuites[] = {
    // TLS 1.3 cipher suites (preferred)
    MBEDTLS_TLS1_3_CHACHA20_POLY1305_SHA256,          // [V2G20-2458]
    MBEDTLS_TLS1_3_AES_256_GCM_SHA384,                // [V2G20-2458]
    // TLS 1.2 cipher suites (fallback)
    MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256,  // [V2G2-602] IETF RFC 5289
    MBEDTLS_TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,   // [V2G2-602] IETF RFC 5289
    0
};

// ISO 15118 named groups, version dependent per HUB20-533-003/005.
// mbedTLS has a single group list for both TLS versions, so the list is
// selected per connection from the peeked ClientHello (see apply_group_policy).
//
// TLS 1.3 (ISO 15118-20 Table 7):
// [V2G20-2674] secp521r1 (primary signature curve)
// [V2G20-2319] x448 (alternative)
// A secp256r1 key_share must never be accepted in TLS 1.3 (HUB20-533-005).
// Clients whose initial key_share is not in this list (e.g. X25519) get a
// HelloRetryRequest.
static const uint16_t iso15118_groups_tls13[] = {
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP521R1,
    MBEDTLS_SSL_IANA_TLS_GROUP_X448,
    MBEDTLS_SSL_IANA_TLS_GROUP_NONE
};

// TLS 1.2 (ISO 15118-2):
// [V2G2-006] secp256r1
// The -20 groups stay allowed, HUB20-533-003 permits the union of both sets.
static const uint16_t iso15118_groups_tls12[] = {
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP256R1,
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP521R1,
    MBEDTLS_SSL_IANA_TLS_GROUP_X448,
    MBEDTLS_SSL_IANA_TLS_GROUP_NONE
};

// ISO 15118 signature algorithms (HUB20-533-001/002):
// ISO 15118-20 Table 8: ecdsa_secp521r1_sha512, ed448.
// ISO 15118-2 [V2G2-006]: ECDSA with SHA256 (secp256r1 leaf), TLS 1.2 only.
// Ed448 is currently not supported by mbedTLS and filtered out at runtime.
// NOTE: The explicit list is required because a our mbedTLS patch that
// widens the default TLS 1.2 list with the TLS 1.3 algorithms.
static const uint16_t iso15118_sig_algs[] = {
    MBEDTLS_TLS1_3_SIG_ECDSA_SECP521R1_SHA512,
    MBEDTLS_TLS1_3_SIG_ED448, // HUB: "Support for Ed448-based SECC certificates may be introduced in a future revision."
    MBEDTLS_TLS1_3_SIG_ECDSA_SECP256R1_SHA256,
    MBEDTLS_TLS1_3_SIG_NONE
};

// Certificate verification profile: a copy of the default profile, except that
// certificate signature verification is skipped so that it can be done
// manually in parallel.
static const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_custom =
{
    /* Hashes from SHA-256 and above. Note that this selection
     * should be aligned with ssl_preset_default_hashes in ssl_tls.c. */
    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA512),

    /* Any PK alg */
    0xFFFFFFF,

    /* Curves at or above 128-bit security level. Note that this selection
     * should be aligned with ssl_preset_default_curves in ssl_tls.c. */
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP256R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP384R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_SECP521R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_BP256R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_BP384R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_BP512R1) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_CURVE25519) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_ECP_DP_CURVE448) |
    0,

    2048, // rsa_min_bitlen doesn't matter because ISO15118 doesn't use RSA
    1,    // skip_signature_verification
};

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
    if (s_isotls_instance == nullptr || !s_isotls_instance->get_ocsp_staple(index, der, der_len)) {
        return -1;
    }
    return 0;
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

static uint8_t *copy_pem(const char *pem, size_t *len_out)
{
    size_t len = strlen(pem) + 1;
    uint8_t *buf = static_cast<uint8_t*>(calloc_psram_or_dram(len, 1));
    if (buf != nullptr) {
        memcpy(buf, pem, len);
        *len_out = len;
    }
    return buf;
}

// Classification of a peeked ClientHello: Does the client offer TLS 1.3
enum class ClientHelloVersion : uint8_t {
    Incomplete, // Not enough data peeked yet, more may arrive
    Tls13,      // TLS 1.3 offered, or undeterminable from a complete or capped record
    NoTls13,    // TLS 1.2 or lower only
};

// Parses the first TLS record of a ClientHello, without consuming it.
// Returns Incomplete only while more data may still arrive (record incomplete
// and below cap). Whenever the answer can not be determined from the available
// data, Tls13 is returned so that the stricter TLS 1.3 group policy applies.
static ClientHelloVersion classify_client_hello(const uint8_t *buf, size_t len, size_t cap)
{
    if (len < 5) {
        return ClientHelloVersion::Incomplete;
    }

    if (buf[0] != 0x16) { // Not a TLS handshake record, the TLS stack will reject it
        return ClientHelloVersion::NoTls13;
    }

    const size_t record_len = (static_cast<size_t>(buf[3]) << 8) | buf[4];
    const bool record_complete = (len - 5) >= record_len;
    const uint8_t *p = buf + 5;
    const uint8_t *end = p + std::min(len - 5, record_len);

    // Out of data inside the record: wait for more, otherwise assume TLS 1.3
    const ClientHelloVersion out_of_data = (!record_complete && len < cap) ? ClientHelloVersion::Incomplete : ClientHelloVersion::Tls13;

    // Handshake header: type (1), length (3)
    if (static_cast<size_t>(end - p) < 4) {
        return out_of_data;
    }
    if (p[0] != 0x01) { // Not a ClientHello
        return ClientHelloVersion::NoTls13;
    }
    p += 4;

    // legacy_version (2), random (32)
    if (static_cast<size_t>(end - p) < 34) {
        return out_of_data;
    }
    p += 34;

    // session_id, cipher_suites, compression_methods
    for (int field = 0; field < 3; field++) {
        const size_t len_size = (field == 1) ? 2 : 1; // cipher_suites has a 2 byte length
        if (static_cast<size_t>(end - p) < len_size) {
            return out_of_data;
        }
        size_t field_len = (len_size == 2) ? ((static_cast<size_t>(p[0]) << 8) | p[1]) : p[0];
        p += len_size;
        if (static_cast<size_t>(end - p) < field_len) {
            return out_of_data;
        }
        p += field_len;
    }

    if (p == end && record_complete) {
        return ClientHelloVersion::NoTls13; // No extensions at all
    }

    if (static_cast<size_t>(end - p) < 2) {
        return out_of_data;
    }
    size_t extensions_len = (static_cast<size_t>(p[0]) << 8) | p[1];
    p += 2;

    const uint8_t *extensions_end = (static_cast<size_t>(end - p) < extensions_len) ? end : p + extensions_len;
    const bool extensions_complete = (static_cast<size_t>(end - p) >= extensions_len) && record_complete;

    while (p < extensions_end) {
        if (static_cast<size_t>(extensions_end - p) < 4) {
            return extensions_complete ? ClientHelloVersion::NoTls13 : out_of_data;
        }
        const uint16_t ext_type = static_cast<uint16_t>((p[0] << 8) | p[1]);
        const size_t ext_len = (static_cast<size_t>(p[2]) << 8) | p[3];
        p += 4;
        if (static_cast<size_t>(extensions_end - p) < ext_len) {
            return out_of_data;
        }

        if (ext_type == 43) { // supported_versions
            if (ext_len < 1) {
                return ClientHelloVersion::NoTls13;
            }
            const size_t list_len = std::min(static_cast<size_t>(p[0]), ext_len - 1);
            for (size_t i = 0; i + 1 < list_len; i += 2) {
                if (p[1 + i] == 0x03 && p[2 + i] == 0x04) {
                    return ClientHelloVersion::Tls13;
                }
            }
            return ClientHelloVersion::NoTls13;
        }

        p += ext_len;
    }

    // All available extensions seen without supported_versions
    return extensions_complete ? ClientHelloVersion::NoTls13 : out_of_data;
}

// Peeks the ClientHello and selects the group list for this connection: TLS 1.3 capable
// clients must not get a secp256r1 key exchange (HUB20-533-005), TLS 1.2 only
// clients need secp256r1 [V2G2-006].
// Returns false while the ClientHello is not fully peeked yet.
bool ISOTLS::apply_group_policy()
{
    static constexpr size_t peek_cap = 1024;

    uint8_t *buf = static_cast<uint8_t*>(malloc_psram_or_dram(peek_cap));
    ClientHelloVersion result = ClientHelloVersion::Tls13;

    if (buf != nullptr) {
        ssize_t len = recv(socket_fd, buf, peek_cap, MSG_PEEK);
        if (len < 0) {
            free_any(buf);
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                return false; // No data yet
            }
            // Socket error: proceed, the handshake will report it
        } else if (len == 0) {
            free_any(buf); // Peer closed: proceed, the handshake will report it
        } else {
            result = classify_client_hello(buf, static_cast<size_t>(len), peek_cap);
            free_any(buf);
            if (result == ClientHelloVersion::Incomplete) {
                return false;
            }
        }
    }

    const bool tls13 = (result == ClientHelloVersion::Tls13);
    mbedtls_ssl_conf_groups(ssl_conf, tls13 ? iso15118_groups_tls13 : iso15118_groups_tls12);
    iso15118.trace("ISOTLS: ClientHello offers %s, using the %s group list",
                    tls13 ? "TLS 1.3" : "TLS 1.2 or lower", tls13 ? "TLS 1.3" : "TLS 1.2");
    group_policy_applied = true;
    return true;
}

bool ISOTLS::load_certificates()
{
    // With a live OCPP certificate store the store is authoritative.
    // TLS 1.3 with the -20 chain is only served when the chain is time valid and its OCSP status is good.
    // Private mode may waive OCSP only when PnC is not compiled in [HUB20-532-002].
    // The embedded dev certificates only serve on bench and development setups.
    std::unique_ptr<char[]> live_chain_iso2,  live_key_iso2;
    std::unique_ptr<char[]> live_chain_iso20, live_key_iso20;
    std::unique_ptr<char[]> live_v2g_roots,   live_oem_roots;
    bool store_live = false;
    iso20_allowed = true;
#if MODULE_OCPP_AVAILABLE()
    store_live = ocpp.is_iso15118_store_live();
    ocpp.get_iso15118_secc_chain(false, &live_chain_iso2,  &live_key_iso2);
    ocpp.get_iso15118_secc_chain(true,  &live_chain_iso20, &live_key_iso20);
    live_v2g_roots = ocpp.get_iso15118_root_bundle(Ocpp::RootGroup::V2G);
    live_oem_roots = ocpp.get_iso15118_root_bundle(Ocpp::RootGroup::OEM);
    if (store_live) {
        iso20_allowed = ocpp.is_iso20_tls_ready() && live_chain_iso20 && live_key_iso20;
    }
#endif

    const char *fallback_chain_iso2 = nullptr;
    const char *fallback_key_iso2 = nullptr;
    const char *fallback_chain_iso20 = nullptr;
    const char *fallback_key_iso20 = nullptr;
    const char *fallback_oem_roots = nullptr;
    const char *fallback_v2g_roots = nullptr;
#if OPTIONS_ISO15118_DEV_CERTS_ENABLED()
    fallback_chain_iso2 = dev_cert_chain_pem_iso2;
    fallback_key_iso2 = dev_private_key_pem_iso2;
    fallback_chain_iso20 = dev_cert_chain_pem_iso20;
    fallback_key_iso20 = dev_private_key_pem_iso20;
    fallback_oem_roots = dev_oem_root_ca_pem_iso20;
    fallback_v2g_roots = dev_v2g_root_ca_pem_iso20;
#else
    if (!store_live) {
        iso15118.trace("ISOTLS: Development certificates disabled and OCPP store unavailable, TLS unavailable");
        return false;
    }
#endif

    if (store_live && (!live_chain_iso2 || !live_key_iso2)) {
        iso15118.trace("ISOTLS: OCPP store live but no -2 SECC chain installed, TLS unavailable");
        return false;
    }

    cert_chain_pem_iso2   = copy_pem(live_chain_iso2 ? live_chain_iso2.get() : fallback_chain_iso2, &cert_chain_pem_len_iso2);
    private_key_pem_iso2  = copy_pem(live_key_iso2   ? live_key_iso2.get()   : fallback_key_iso2,   &private_key_pem_len_iso2);
    if (iso20_allowed) {
        cert_chain_pem_iso20  = copy_pem(live_chain_iso20 ? live_chain_iso20.get() : fallback_chain_iso20, &cert_chain_pem_len_iso20);
        private_key_pem_iso20 = copy_pem(live_key_iso20   ? live_key_iso20.get()   : fallback_key_iso20,   &private_key_pem_len_iso20);
    }
    if (live_oem_roots || !store_live) {
        oem_root_ca_pem_iso20 = copy_pem(live_oem_roots ? live_oem_roots.get() : fallback_oem_roots, &oem_root_ca_pem_len_iso20);
    }
    if (live_v2g_roots || !store_live) {
        v2g_root_ca_pem_iso20 = copy_pem(live_v2g_roots ? live_v2g_roots.get() : fallback_v2g_roots, &v2g_root_ca_pem_len_iso20);
    }

    if ((cert_chain_pem_iso2 == nullptr) || (private_key_pem_iso2 == nullptr) ||
        (iso20_allowed && ((cert_chain_pem_iso20 == nullptr) || (private_key_pem_iso20 == nullptr)))) {
        iso15118.trace("ISOTLS: Failed to allocate memory for certificates");
        return false;
    }

#if MODULE_OCPP_AVAILABLE()
    // Retained OCSP responses of the served -20 chain, leaf first, for stapling into the TLS 1.3 Certificate message [V2G20-2388]
    if (iso20_allowed && store_live) {
        for (size_t i = 0; i < OCSP_STAPLE_MAX; ++i) {
            std::unique_ptr<uint8_t[]> der;
            size_t der_len = 0;
            if (!ocpp.get_iso15118_ocsp_staple(static_cast<uint8_t>(i), &der, &der_len)) {
                continue;
            }
            staple_der[i] = static_cast<uint8_t*>(calloc_psram_or_dram(der_len, 1));
            if (staple_der[i] == nullptr) {
                continue;
            }
            memcpy(staple_der[i], der.get(), der_len);
            staple_der_len[i] = der_len;
            iso15118.trace("ISOTLS: OCSP staple for -20 chain certificate %zu: %zu bytes", i, der_len);
        }
    }
#endif

    iso15118.trace("ISOTLS: ISO 15118-2 SECC chain (secp256r1) from %s: chain=%zu bytes, key=%zu bytes",
                    live_chain_iso2 ? "OCPP store" : "dev certs",
                    cert_chain_pem_len_iso2 - 1, private_key_pem_len_iso2 - 1);
    if (iso20_allowed) {
        iso15118.trace("ISOTLS: ISO 15118-20 SECC chain (secp521r1) from %s: chain=%zu bytes, key=%zu bytes",
                        live_chain_iso20 ? "OCPP store" : "dev certs",
                        cert_chain_pem_len_iso20 - 1, private_key_pem_len_iso20 - 1);
    } else {
        iso15118.trace("ISOTLS: -20 SECC chain missing, expired or without good OCSP status, offering TLS 1.2 only [HUB20-532-002]");
    }
    iso15118.trace("ISOTLS: Trusted roots for ISO 15118-20 mutual TLS: OEM from %s (%zu bytes), V2G from %s (%zu bytes)",
                    live_oem_roots ? "OCPP store" : "dev certs", oem_root_ca_pem_len_iso20 > 0 ? oem_root_ca_pem_len_iso20 - 1 : 0,
                    live_v2g_roots ? "OCPP store" : "dev certs", v2g_root_ca_pem_len_iso20 > 0 ? v2g_root_ca_pem_len_iso20 - 1 : 0);

    return true;
}

bool ISOTLS::setup()
{
    if (initialized) {
        return true;
    }

    iso15118.trace("ISOTLS: Setting up TLS for ISO 15118 (TLS 1.2 + 1.3)");

    if (!load_certificates()) {
        iso15118.trace("ISOTLS: Failed to load certificates");
        return false;
    }

    // Allocate mbedTLS contexts from PSRAM
    ssl = static_cast<mbedtls_ssl_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_ssl_context)));
    ssl_conf = static_cast<mbedtls_ssl_config*>(calloc_psram_or_dram(1, sizeof(mbedtls_ssl_config)));
    entropy = static_cast<mbedtls_entropy_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_entropy_context)));
    ctr_drbg = static_cast<mbedtls_ctr_drbg_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_ctr_drbg_context)));
    cert_chain_iso2 = static_cast<mbedtls_x509_crt*>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
    private_key_iso2 = static_cast<mbedtls_pk_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_pk_context)));
    cert_chain_iso20 = static_cast<mbedtls_x509_crt*>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
    private_key_iso20 = static_cast<mbedtls_pk_context*>(calloc_psram_or_dram(1, sizeof(mbedtls_pk_context)));

    if (ssl == nullptr || ssl_conf == nullptr || entropy == nullptr || ctr_drbg == nullptr ||
        cert_chain_iso2 == nullptr || private_key_iso2 == nullptr ||
        cert_chain_iso20 == nullptr || private_key_iso20 == nullptr) {
        iso15118.trace("ISOTLS: Failed to allocate mbedTLS contexts");
        cleanup();
        return false;
    }

    // Initialize mbedTLS contexts
    mbedtls_ssl_init(ssl);
    mbedtls_ssl_config_init(ssl_conf);
    mbedtls_entropy_init(entropy);
    mbedtls_ctr_drbg_init(ctr_drbg);
    mbedtls_x509_crt_init(cert_chain_iso2);
    mbedtls_pk_init(private_key_iso2);
    mbedtls_x509_crt_init(cert_chain_iso20);
    mbedtls_pk_init(private_key_iso20);

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

    // Parse ISO 15118-2 certificates (secp256r1)
    ret = mbedtls_x509_crt_parse(cert_chain_iso2, cert_chain_pem_iso2, cert_chain_pem_len_iso2);
    if (ret != 0) {
        iso15118.trace("ISOTLS: ISO2 mbedtls_x509_crt_parse failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }

    ret = mbedtls_pk_parse_key(private_key_iso2, private_key_pem_iso2, private_key_pem_len_iso2,
                               nullptr, 0, mbedtls_ctr_drbg_random, ctr_drbg);
    if (ret != 0) {
        iso15118.trace("ISOTLS: ISO2 mbedtls_pk_parse_key failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }
    iso15118.trace("ISOTLS: ISO 15118-2 certificates parsed successfully (secp256r1)");

    // Parse ISO 15118-20 certificates (secp521r1) only when the -20 chain may be served [HUB20-532-002]
    if (iso20_allowed) {
        ret = mbedtls_x509_crt_parse(cert_chain_iso20, cert_chain_pem_iso20, cert_chain_pem_len_iso20);
        if (ret != 0) {
            iso15118.trace("ISOTLS: ISO20 mbedtls_x509_crt_parse failed: -0x%04x", static_cast<unsigned>(-ret));
            cleanup();
            return false;
        }

        ret = mbedtls_pk_parse_key(private_key_iso20, private_key_pem_iso20, private_key_pem_len_iso20,
                                   nullptr, 0, mbedtls_ctr_drbg_random, ctr_drbg);
        if (ret != 0) {
            iso15118.trace("ISOTLS: ISO20 mbedtls_pk_parse_key failed: -0x%04x", static_cast<unsigned>(-ret));
            cleanup();
            return false;
        }
        iso15118.trace("ISOTLS: ISO 15118-20 certificates parsed successfully (secp521r1)");
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

    int trusted_ca_count = 0;
    if (oem_root_ca_pem_iso20 != nullptr) {
        ret = mbedtls_x509_crt_parse(trusted_ca_iso20, oem_root_ca_pem_iso20, oem_root_ca_pem_len_iso20);
        if (ret != 0) {
            iso15118.trace("ISOTLS: OEM Root CA parse failed: -0x%04x", static_cast<unsigned>(-ret));
        } else {
            trusted_ca_count++;
            iso15118.trace("ISOTLS: OEM Root CA parsed successfully");
        }
    }

    if (v2g_root_ca_pem_iso20 != nullptr) {
        ret = mbedtls_x509_crt_parse(trusted_ca_iso20, v2g_root_ca_pem_iso20, v2g_root_ca_pem_len_iso20);
        if (ret != 0) {
            iso15118.trace("ISOTLS: V2G Root CA parse failed: -0x%04x", static_cast<unsigned>(-ret));
        } else {
            trusted_ca_count++;
            iso15118.trace("ISOTLS: V2G Root CA parsed successfully");
        }
    }

    if (trusted_ca_count > 0) {
        iso15118.trace("ISOTLS: %d trusted root CA(s) loaded for ISO 15118-20 mutual TLS", trusted_ca_count);
    } else {
        iso15118.trace("ISOTLS: WARNING: No trusted root CAs loaded - mutual TLS authentication disabled");
        mbedtls_x509_crt_free(trusted_ca_iso20);
        free_any(trusted_ca_iso20);
        trusted_ca_iso20 = nullptr;
    }

    // Configure SSL
    ret = mbedtls_ssl_config_defaults(ssl_conf,
                                      MBEDTLS_SSL_IS_SERVER,
                                      MBEDTLS_SSL_TRANSPORT_STREAM,
                                      MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        iso15118.trace("ISOTLS: mbedtls_ssl_config_defaults failed: -0x%04x", static_cast<unsigned>(-ret));
        cleanup();
        return false;
    }

    // Allow both TLS 1.2 and 1.3, let client negotiate
    // [V2G20-2356] If TLS 1.2 negotiated, SECC shall not select ISO 15118-20
    // Without a servable -20 chain only TLS 1.2 is offered [HUB20-532-002]
    mbedtls_ssl_conf_min_tls_version(ssl_conf, MBEDTLS_SSL_VERSION_TLS1_2);
    mbedtls_ssl_conf_max_tls_version(ssl_conf, iso20_allowed ? MBEDTLS_SSL_VERSION_TLS1_3 : MBEDTLS_SSL_VERSION_TLS1_2);
    mbedtls_ssl_conf_ciphersuites(ssl_conf, iso15118_ciphersuites);
    // Strict default, replaced per connection in apply_group_policy()
    mbedtls_ssl_conf_groups(ssl_conf, iso15118_groups_tls13);
    mbedtls_ssl_conf_sig_algs(ssl_conf, iso15118_sig_algs);

    // Default authmode: VERIFY_NONE (safe fallback for TLS 1.2 / ISO 15118-2)
    // For TLS 1.3 / ISO 15118-20, mutual authentication is enabled
    // per-handshake in the cert_cb callback using:
    //   mbedtls_ssl_set_hs_authmode(MBEDTLS_SSL_VERIFY_REQUIRED)
    //   mbedtls_ssl_set_hs_ca_chain(trusted_ca_iso20)
    // This way TLS 1.2 stays unilateral and TLS 1.3 gets mutual auth.
    mbedtls_ssl_conf_authmode(ssl_conf, MBEDTLS_SSL_VERIFY_NONE);

    mbedtls_ssl_conf_cert_profile(ssl_conf, &mbedtls_x509_crt_profile_custom);

    mbedtls_ssl_conf_rng(ssl_conf, mbedtls_ctr_drbg_random, ctr_drbg);

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

    if (cert_chain_iso2 != nullptr) {
        mbedtls_x509_crt_free(cert_chain_iso2);
        free_any(cert_chain_iso2);
        cert_chain_iso2 = nullptr;
    }

    if (private_key_iso2 != nullptr) {
        mbedtls_pk_free(private_key_iso2);
        free_any(private_key_iso2);
        private_key_iso2 = nullptr;
    }

    if (cert_chain_iso20 != nullptr) {
        mbedtls_x509_crt_free(cert_chain_iso20);
        free_any(cert_chain_iso20);
        cert_chain_iso20 = nullptr;
    }

    if (private_key_iso20 != nullptr) {
        mbedtls_pk_free(private_key_iso20);
        free_any(private_key_iso20);
        private_key_iso20 = nullptr;
    }

    if (trusted_ca_iso20 != nullptr) {
        mbedtls_x509_crt_free(trusted_ca_iso20);
        free_any(trusted_ca_iso20);
        trusted_ca_iso20 = nullptr;
    }

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

    if (cert_chain_pem_iso2 != nullptr) {
        free_any(cert_chain_pem_iso2);
        cert_chain_pem_iso2 = nullptr;
        cert_chain_pem_len_iso2 = 0;
    }

    if (private_key_pem_iso2 != nullptr) {
        free_any(private_key_pem_iso2);
        private_key_pem_iso2 = nullptr;
        private_key_pem_len_iso2 = 0;
    }

    if (cert_chain_pem_iso20 != nullptr) {
        free_any(cert_chain_pem_iso20);
        cert_chain_pem_iso20 = nullptr;
        cert_chain_pem_len_iso20 = 0;
    }

    if (private_key_pem_iso20 != nullptr) {
        free_any(private_key_pem_iso20);
        private_key_pem_iso20 = nullptr;
        private_key_pem_len_iso20 = 0;
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

    for (size_t i = 0; i < OCSP_STAPLE_MAX; ++i) {
        if (staple_der[i] != nullptr) {
            free_any(staple_der[i]);
            staple_der[i] = nullptr;
        }
        staple_der_len[i] = 0;
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

bool ISOTLS::leaf_cert_is_cached()
{
    const mbedtls_x509_crt *leaf_cert = verification_context->certs[0];
    mbedtls_sha256(leaf_cert->raw.p, leaf_cert->raw.len, verification_context->leaf_sha256, 0);

    for (cert_cache_entry *entry = peer_cert_cache; entry != nullptr; entry = entry->next) {
        if (memcmp(verification_context->leaf_sha256, entry->sha256, sizeof(entry->sha256)) == 0) {
            entry->last_seen = now_us();
            iso15118.trace("ISOTLS: Found cached certificate of peer '%s'", entry->dn);

            return true;
        }
    }

    return false;
}

void ISOTLS::cache_leaf_cert()
{
    size_t entry_count = 0;
    micros_t lru = std::numeric_limits<micros_t>::max();

    cert_cache_entry **lru_entry_ptr = nullptr;
    cert_cache_entry *lru_entry = nullptr;

    cert_cache_entry **entry_ptr = &peer_cert_cache;
    cert_cache_entry *entry = peer_cert_cache;

    while (entry != nullptr) {
        entry_count++;

        if (entry->last_seen < lru) {
            lru = entry->last_seen;
            lru_entry_ptr = entry_ptr;
            lru_entry = entry;
        }

        entry_ptr = &entry->next;
        entry = entry->next;
    }

    if (entry_count < 10) {
        // Allocate new entry
        entry = static_cast<cert_cache_entry *>(perm_aligned_alloc_prefer(alignof(cert_cache_entry), sizeof(cert_cache_entry), RAM::PSRAM, RAM::DRAM));
    } else {
        // Reuse existing entry
        *lru_entry_ptr = lru_entry->next;
        entry = lru_entry;
        free(entry->dn);
    }

    // Insert at the beginning
    entry->next = peer_cert_cache;
    peer_cert_cache = entry;

    entry->last_seen = now_us();
    memcpy(entry->sha256, verification_context->leaf_sha256, sizeof(entry->sha256));

    const mbedtls_x509_crt *leaf_cert = verification_context->certs[0];
    const mbedtls_asn1_buf &subject = leaf_cert->subject.val;
    const size_t subject_len = subject.len;

    entry->dn = static_cast<char *>(malloc_psram_or_dram(subject_len + 1));
    memcpy(entry->dn, subject.p, subject_len);
    entry->dn[subject_len] = 0;

    iso15118.trace("ISOTLS: Caching certificate of peer '%s'", entry->dn);
}

static bool cert_signature_is_valid(const mbedtls_x509_crt *child, mbedtls_x509_crt *parent)
{
    const int sig_result = x509_crt_check_signature(child, parent, nullptr);

    if (sig_result == 0) {
        return true;
    }

    log_mbedtls_error(sig_result, "Certificate signature check failed");
    return false;
}

mbedtls_x509_crt *ISOTLS::find_anchor_by_name(const mbedtls_x509_crt *topmost) const
{
    for (mbedtls_x509_crt *root = trusted_ca_iso20; root != nullptr; root = root->next) {
        if (topmost->issuer_raw.len == root->subject_raw.len
         && memcmp(topmost->issuer_raw.p, root->subject_raw.p, root->subject_raw.len) == 0) {
            return root;
        }
    }

    return nullptr;
}

void ISOTLS::hand_off_vehicle_chain()
{
#if MODULE_OCPP_AVAILABLE()
    // HUB20-432-001/002: The vehicle chain status request is sent right
    // after the handshake completes, the -20 authorization loop polls the
    // result. A missing anchor fails the request, which the poll reports
    // as Unknown (fail closed).
    mbedtls_x509_crt **certs = verification_context->certs;
    size_t count = 0;

    while (count < CERTS_MAX_VERIFY && certs[count] != nullptr) {
        count++;
    }

    if (count == 0) {
        return;
    }

    // mbedTLS appends the trust anchor to the presented chain, so the
    // topmost entry is the self-signed root. The root is the anchor, not
    // part of the vehicle chain whose revocation status is checked
    // (HUB20-432-006: Leaf, Sub2, Sub1).
    mbedtls_x509_crt *root = verification_context->anchor_root;
    size_t chain_len = count;
    mbedtls_x509_crt *topmost = certs[count - 1];

    if (topmost->issuer_raw.len == topmost->subject_raw.len
     && memcmp(topmost->issuer_raw.p, topmost->subject_raw.p, topmost->subject_raw.len) == 0) {
        root = topmost;
        chain_len = count - 1;
    } else if (root == nullptr) {
        // Cached leaf shortcut, the verify task did not run
        root = find_anchor_by_name(topmost);
    }

    if (chain_len == 0) {
        return;
    }

    Ocpp::VehicleChainCertDer chain[CERTS_MAX_VERIFY];

    for (size_t i = 0; i < chain_len; i++) {
        chain[i] = {certs[i]->raw.p, certs[i]->raw.len};
    }

    bool ok = ocpp.request_iso15118_vehicle_chain_status(chain, chain_len,
                                                         root != nullptr ? root->raw.p : nullptr,
                                                         root != nullptr ? root->raw.len : 0);
    iso15118.trace("ISOTLS: Vehicle chain status check %s (%zu certificates)", ok ? "started" : "not started", chain_len);
#endif
}

void ISOTLS::verify_intermediate_certs()
{
    mbedtls_x509_crt **certs = verification_context->certs;
    bool success = true;
    size_t topmost_idx = CERTS_MAX_VERIFY - 1;

    // Check all intermediate certificates; first is leaf, last is topmost
    for (size_t i = 1; i < CERTS_MAX_VERIFY - 1; i++) {
        mbedtls_x509_crt *child  = certs[i];
        mbedtls_x509_crt *parent = certs[i + 1];

        if (parent == nullptr) {
            topmost_idx = i;
            break;
        }

        if (!cert_signature_is_valid(child, parent)) {
            iso15118.trace("ISOTLS: Intermediate certificate %zu failed verification", i);
            success = false;
            break;
        }
    }

    // The chain must anchor to the trust store by key, not just by name.
    // The topmost presented certificate has to be verified against the
    // matching trusted root here. Roots sharing a subject name are all
    // tried, the successful one becomes the anchor.
    if (success) {
        mbedtls_x509_crt *topmost = certs[topmost_idx];
        success = false;

        for (mbedtls_x509_crt *root = trusted_ca_iso20; root != nullptr; root = root->next) {
            if (topmost->issuer_raw.len != root->subject_raw.len
             || memcmp(topmost->issuer_raw.p, root->subject_raw.p, root->subject_raw.len) != 0) {
                continue;
            }

            if (cert_signature_is_valid(topmost, root)) {
                verification_context->anchor_root = root;
                success = true;
                break;
            }
        }

        if (!success) {
            iso15118.trace("ISOTLS: Topmost certificate failed verification against the trust store");
        }
    }

    verification_context->intermediates_valid = success;
}

void ISOTLS::verify_certs_task(void *ctx)
{
    ISOTLS *isotls = static_cast<ISOTLS *>(ctx);
    isotls->verify_intermediate_certs();

    // Wake main task
    xSemaphoreGive_nowarn(isotls->verification_context->sem_handle);

    // Exit RTOS task
    vTaskDelete(NULL);
}

int ISOTLS::cert_verify(void *ctx, mbedtls_x509_crt *cert, int index, uint32_t *flags)
{
    if (index < 0) {
        return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
    }

    if (static_cast<size_t>(index) > CERTS_MAX_VERIFY - 1) {
        iso15118.trace("Too many certificates in chain: %i/%zu", index + 1, CERTS_MAX_VERIFY);
        return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
    }

    // Log peer certificate issuer -> subject for debugging
    iso15118.trace("ISOTLS: EVCC certificate: %.*s -> %.*s", static_cast<int>(cert->issuer.val.len), cert->issuer.val.p, static_cast<int>(cert->subject.val.len), cert->subject.val.p);

    ISOTLS *isotls = static_cast<ISOTLS *>(ctx);
    verification_context_t *verify_ctx = isotls->verification_context;
    verify_ctx->certs[index] = cert;

    if (index > 0) {
        // Leaf not reached, more certs to come.
        return 0;
    }

    // Leaf reached, verify certificates.

    mbedtls_x509_crt *leaf_cert = verify_ctx->certs[0];
    mbedtls_x509_crt *ca_cert   = verify_ctx->certs[1];

    if (ca_cert == nullptr) {
        iso15118.trace("ISOTLS: Rejecting self-signed peer certificate");
        *flags |= MBEDTLS_X509_BADCERT_NOT_TRUSTED;

        return 0; // No error; verification failure is not an error
    }

    if (isotls->leaf_cert_is_cached()) {
        verify_ctx->leaf_cert_cached = true;
        verify_ctx->intermediates_valid = true;

        return 0;
    }

    // The verify task checks the intermediate signatures and the trust
    // store anchoring of the topmost certificate, so it runs even when the chain has no intermediates.
    const BaseType_t ret = xTaskCreatePinnedToCore(verify_certs_task, "verify_certs", 3072, ctx, 10, nullptr, 0); // Priority above httpd but below all other core 0 tasks.

    if (ret == pdPASS_nowarn) {
        verify_ctx->async_started = true;
    } else {
        iso15118.trace("ISOTLS: verify_certs task could not be created");

        // Verify certs now. This will probably cause the peer to time out.
        isotls->verify_intermediate_certs();
    }

    // Verify leaf certificate
    if (!cert_signature_is_valid(leaf_cert, ca_cert)) {
        iso15118.trace("ISOTLS: Leaf certificate failed verification");
        *flags |= MBEDTLS_X509_BADCERT_NOT_TRUSTED;
    }

    return 0; // No error; verification failure is not an error
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

    if (ver == MBEDTLS_SSL_VERSION_TLS1_3 && iso20_allowed && cert_chain_iso20 != nullptr && private_key_iso20 != nullptr) {
        iso15118.trace("ISOTLS: cert_cb: TLS 1.3 negotiated, selecting ISO 15118-20 cert (secp521r1)");
        int ret = mbedtls_ssl_set_hs_own_cert(ssl_ctx, cert_chain_iso20, private_key_iso20);
        if (ret != 0) {
            iso15118.trace("ISOTLS: cert_cb: Failed to set own cert: -0x%04x", static_cast<unsigned>(-ret));
            return ret;
        }

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
    } else if (cert_chain_iso2 != nullptr && private_key_iso2 != nullptr) {
        iso15118.trace("ISOTLS: cert_cb: TLS 1.2 negotiated, selecting ISO 15118-2 cert (secp256r1)");
        // ISO 15118-2: Unilateral authentication only (no client cert)
        mbedtls_ssl_set_hs_authmode(ssl_ctx, MBEDTLS_SSL_VERIFY_NONE);
        return mbedtls_ssl_set_hs_own_cert(ssl_ctx, cert_chain_iso2, private_key_iso2);
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

bool ISOTLS::get_ocsp_staple(size_t index, const unsigned char **der, size_t *der_len) const
{
    if (index >= OCSP_STAPLE_MAX || staple_der[index] == nullptr || staple_der_len[index] == 0) {
        return false;
    }
    *der = staple_der[index];
    *der_len = staple_der_len[index];
    return true;
}

bool ISOTLS::is_mutual_auth_enabled() const
{
    return mutual_auth_enabled;
}
