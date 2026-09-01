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
#include "tools/malloc.h"

#include <sys/socket.h>
#include <errno.h>
#include <algorithm>

#include "mbedtls/ssl_ciphersuites.h"

extern ISO15118 iso15118;

namespace {

// ISO 15118 cipher suites (TLS 1.2 + TLS 1.3)
// TLS 1.3: [V2G20-2458] Table 6
// TLS 1.2: [V2G2-602] Table 7
const int iso15118_ciphersuites[] = {
    // TLS 1.3 cipher suites (preferred)
    MBEDTLS_TLS1_3_AES_256_GCM_SHA384,                // [V2G20-2458]
    MBEDTLS_TLS1_3_CHACHA20_POLY1305_SHA256,          // [V2G20-2458]
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
const uint16_t iso15118_groups_tls13[] = {
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP521R1,
    MBEDTLS_SSL_IANA_TLS_GROUP_X448,
    MBEDTLS_SSL_IANA_TLS_GROUP_NONE
};

// TLS 1.2 (ISO 15118-2):
// [V2G2-006] secp256r1
// The -20 groups stay allowed, HUB20-533-003 permits the union of both sets.
const uint16_t iso15118_groups_tls12[] = {
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP256R1,
    MBEDTLS_SSL_IANA_TLS_GROUP_SECP521R1,
    MBEDTLS_SSL_IANA_TLS_GROUP_X448,
    MBEDTLS_SSL_IANA_TLS_GROUP_NONE
};

// ISO 15118 signature algorithms (HUB20-533-001/002):
// ISO 15118-20 Table 8: ecdsa_secp521r1_sha512, ed448.
// ISO 15118-2 [V2G2-006]: ECDSA with SHA256 (secp256r1 leaf), TLS 1.2 only.
// NOTE: The explicit list is required because a our mbedTLS patch that
// widens the default TLS 1.2 list with the TLS 1.3 algorithms.
const uint16_t iso15118_sig_algs[] = {
    MBEDTLS_TLS1_3_SIG_ECDSA_SECP521R1_SHA512,
    MBEDTLS_TLS1_3_SIG_ED448, // HUB: "Support for Ed448-based SECC certificates may be introduced in a future revision."
    MBEDTLS_TLS1_3_SIG_ECDSA_SECP256R1_SHA256,
    MBEDTLS_TLS1_3_SIG_NONE
};

// Certificate verification profile: a copy of the default profile, except that
// certificate signature verification is skipped so that it can be done
// manually in parallel.
const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_custom =
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

} // namespace

bool ISOTLS::configure_ssl_policy()
{
    int ret = mbedtls_ssl_config_defaults(ssl_conf,
                                          MBEDTLS_SSL_IS_SERVER,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret != 0) {
        iso15118.trace("ISOTLS: mbedtls_ssl_config_defaults failed: -0x%04x", static_cast<unsigned>(-ret));
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
    return true;
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
