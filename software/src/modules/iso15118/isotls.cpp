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
#include "mbedtls/asn1.h"
#include "mbedtls/sha1.h"
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
    if (s_isotls_instance == nullptr || !s_isotls_instance->get_ocsp_staple(ssl, index, der, der_len)) {
        return -1;
    }
    return 0;
}

extern "C" int mbedtls_ssl_tls13_certificate_authorities_cb(mbedtls_ssl_context *ssl, const unsigned char *data, size_t data_len)
{
    return s_isotls_instance == nullptr ? -1 : s_isotls_instance->select_iso20_certificate_authority(ssl, data, data_len);
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

static constexpr size_t X509_NAME_ATTRIBUTE_MAX = 16;

struct x509_name_attribute_t {
    mbedtls_asn1_buf oid = {};
    mbedtls_asn1_buf value = {};
    bool continues_rdn = false;
};

static bool parse_x509_name(const unsigned char *der, size_t der_len, x509_name_attribute_t *attributes, size_t *attribute_count)
{
    if (der == nullptr) {
        return false;
    }
    unsigned char *p = const_cast<unsigned char *>(der);
    const unsigned char *end = der + der_len;
    size_t sequence_len = 0;
    size_t count = 0;

    if ((mbedtls_asn1_get_tag(&p, end, &sequence_len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) != 0) || (sequence_len != static_cast<size_t>(end - p))) {
        return false;
    }

    while (p < end) {
        size_t set_len = 0;
        if ((mbedtls_asn1_get_tag(&p, end, &set_len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SET) != 0) || (set_len == 0) || (set_len > static_cast<size_t>(end - p))) {
            return false;
        }
        const unsigned char *set_end = p + set_len;

        while (p < set_end) {
            if (count >= X509_NAME_ATTRIBUTE_MAX) {
                return false;
            }
            size_t attribute_len = 0;
            if ((mbedtls_asn1_get_tag(&p, set_end, &attribute_len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) != 0) ||
                (attribute_len == 0) || (attribute_len > static_cast<size_t>(set_end - p))) {
                return false;
            }
            const unsigned char *attribute_end = p + attribute_len;
            x509_name_attribute_t &attribute = attributes[count];
            if (mbedtls_asn1_get_tag(&p, attribute_end, &attribute.oid.len, MBEDTLS_ASN1_OID) != 0) {
                return false;
            }
            attribute.oid.tag = MBEDTLS_ASN1_OID;
            attribute.oid.p = p;
            p += attribute.oid.len;
            if (p >= attribute_end) {
                return false;
            }
            attribute.value.tag = *p++;
            if ((mbedtls_asn1_get_len(&p, attribute_end, &attribute.value.len) != 0) || (attribute.value.len > static_cast<size_t>(attribute_end - p))) {
                return false;
            }
            attribute.value.p = p;
            p += attribute.value.len;
            if (p != attribute_end) {
                return false;
            }
            attribute.continues_rdn = p < set_end;
            ++count;
        }
    }

    *attribute_count = count;
    return count > 0;
}

static unsigned char ascii_lower(unsigned char value)
{
    return value >= 'A' && value <= 'Z' ? value + ('a' - 'A') : value;
}

static bool normalized_directory_string_byte(const mbedtls_asn1_buf &value, size_t *offset, unsigned char *result)
{
    size_t end = value.len;
    while (end > 0 && value.p[end - 1] == ' ') {
        --end;
    }
    while (*offset < end && value.p[*offset] == ' ') {
        ++*offset;
    }
    if (*offset >= end) {
        return false;
    }

    *result = ascii_lower(value.p[(*offset)++]);
    if (*result == ' ') {
        while (*offset < end && value.p[*offset] == ' ') {
            ++*offset;
        }
    }
    return true;
}

static bool x509_name_value_equal(const mbedtls_asn1_buf &a, const mbedtls_asn1_buf &b)
{
    if (a.tag == b.tag && a.len == b.len && memcmp(a.p, b.p, a.len) == 0) {
        return true;
    }
    const bool a_directory_string = (a.tag == MBEDTLS_ASN1_UTF8_STRING) || (a.tag == MBEDTLS_ASN1_PRINTABLE_STRING);
    const bool b_directory_string = (b.tag == MBEDTLS_ASN1_UTF8_STRING) || (b.tag == MBEDTLS_ASN1_PRINTABLE_STRING);
    if (!a_directory_string || !b_directory_string) {
        return false;
    }

    size_t a_offset = 0;
    size_t b_offset = 0;
    unsigned char a_byte = 0;
    unsigned char b_byte = 0;
    while (true) {
        const bool a_available = normalized_directory_string_byte(a, &a_offset, &a_byte);
        const bool b_available = normalized_directory_string_byte(b, &b_offset, &b_byte);
        if (a_available != b_available) {
            return false;
        }
        if (!a_available) {
            return true;
        }
        if (a_byte != b_byte) {
            return false;
        }
    }
}

// RFC 5280 7.1 name comparison for [V2G20-2379] / AMD1 [V2G20-3376].
// Unsupported string equivalences are rejected conservatively.
static bool x509_name_equal(const unsigned char *a_der, size_t a_len, const unsigned char *b_der, size_t b_len)
{
    x509_name_attribute_t a[X509_NAME_ATTRIBUTE_MAX];
    x509_name_attribute_t b[X509_NAME_ATTRIBUTE_MAX];
    size_t a_count = 0;
    size_t b_count = 0;
    if (!parse_x509_name(a_der, a_len, a, &a_count) ||
        !parse_x509_name(b_der, b_len, b, &b_count) || (a_count != b_count)) {
        return false;
    }

    for (size_t i = 0; i < a_count; ++i) {
        if (a[i].oid.len != b[i].oid.len ||
            memcmp(a[i].oid.p, b[i].oid.p, a[i].oid.len) != 0 ||
            a[i].continues_rdn != b[i].continues_rdn ||
            !x509_name_value_equal(a[i].value, b[i].value)) {
            return false;
        }
    }
    return true;
}

// Extracts the RFC 6066 subjectPublicKey value from a complete SubjectPublicKeyInfo.
static bool spki_bitstring(const mbedtls_x509_buf *spki, mbedtls_asn1_buf *out)
{
    unsigned char *p = spki->p;
    const unsigned char *end = spki->p + spki->len;
    size_t len = 0;

    if (mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) != 0 ||
        len != static_cast<size_t>(end - p)) {
        return false;
    }
    const unsigned char *spki_end = p + len;
    if (mbedtls_asn1_get_tag(&p, spki_end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE) != 0 ||
        len > static_cast<size_t>(spki_end - p)) {
        return false;
    }
    p += len;

    mbedtls_asn1_bitstring bitstring;
    if (mbedtls_asn1_get_bitstring(&p, spki_end, &bitstring) != 0 || bitstring.unused_bits != 0 || p != spki_end) {
        return false;
    }

    out->p = bitstring.p;
    out->len = bitstring.len;
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

bool ISOTLS::load_certificates()
{
    // With a live OCPP certificate store the store is authoritative.
    // TLS 1.3 with the -20 chain is only served when the chain is time valid and its OCSP status is good.
    // Private mode may waive OCSP only when PnC is not compiled in [HUB20-532-002].
    // The embedded dev certificates only serve on bench and development setups.
    std::unique_ptr<char[]> live_v2g_roots,   live_oem_roots;
    bool store_live = false;
    iso20_allowed = false;
#if MODULE_OCPP_AVAILABLE()
    store_live = ocpp.is_iso15118_store_live();
    live_v2g_roots = ocpp.get_iso15118_root_bundle(Ocpp::RootGroup::V2G);
    live_oem_roots = ocpp.get_iso15118_root_bundle(Ocpp::RootGroup::OEM);
    if (store_live) {
        Ocpp::Iso15118SeccChain chains[ISO20_CANDIDATE_MAX];
        const size_t chain_count = ocpp.get_iso15118_secc_chains(true, chains, ISO20_CANDIDATE_MAX);
        for (size_t i = 0; i < chain_count; ++i) {
            if (!ocpp.is_iso20_tls_ready(chains[i].chain_id) ||
                chains[i].chain_pem == nullptr || chains[i].key_pem == nullptr) {
                continue;
            }

            iso20_candidate_t &candidate = iso20_candidates[iso20_candidate_count];
            candidate.chain_id = chains[i].chain_id;
            if (chains[i].root_pem == nullptr) {
                continue;
            }
            candidate.cert_chain_pem = copy_pem(chains[i].chain_pem.get(), &candidate.cert_chain_pem_len);
            candidate.private_key_pem = copy_pem(chains[i].key_pem.get(), &candidate.private_key_pem_len);
            candidate.root_pem = copy_pem(chains[i].root_pem.get(), &candidate.root_pem_len);
            if ((candidate.cert_chain_pem == nullptr) || (candidate.private_key_pem == nullptr) || (candidate.root_pem == nullptr)) {
                free_iso20_candidate(iso20_candidate_count);
                continue;
            }

            const size_t cert_count = platform_cert_count21(chains[i].chain_pem.get());
            const bool ocsp_required = ocpp.is_iso20_ocsp_required();
            bool staples_complete = !ocsp_required || (cert_count > 0 && cert_count <= OCSP_STAPLE_MAX);
            for (size_t cert_idx = 0; cert_idx < cert_count && cert_idx < OCSP_STAPLE_MAX; ++cert_idx) {
                std::unique_ptr<uint8_t[]> der;
                size_t der_len = 0;
                if (!ocpp.get_iso15118_ocsp_staple(candidate.chain_id, static_cast<uint8_t>(cert_idx), &der, &der_len) || der_len == 0) {
                    if (ocsp_required) {
                        staples_complete = false;
                        break;
                    }
                    continue;
                }
                candidate.staple_der[cert_idx] = static_cast<uint8_t *>(calloc_psram_or_dram(der_len, 1));
                if (candidate.staple_der[cert_idx] == nullptr) {
                    if (ocsp_required) {
                        staples_complete = false;
                        break;
                    }
                    continue;
                }
                memcpy(candidate.staple_der[cert_idx], der.get(), der_len);
                candidate.staple_der_len[cert_idx] = der_len;
            }
            if (!staples_complete) {
                free_iso20_candidate(iso20_candidate_count);
                continue;
            }
            ++iso20_candidate_count;
        }
        iso20_allowed = iso20_candidate_count > 0;
    }
#endif

    const char *fallback_chain_iso20 = nullptr;
    const char *fallback_key_iso20 = nullptr;
    const char *fallback_oem_roots = nullptr;
    const char *fallback_v2g_roots = nullptr;
#if OPTIONS_ISO15118_DEV_CERTS_ENABLED()
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

    if (!store_live && fallback_chain_iso20 != nullptr && fallback_key_iso20 != nullptr) {
        iso20_candidate_t &candidate = iso20_candidates[0];
        candidate.cert_chain_pem = copy_pem(fallback_chain_iso20, &candidate.cert_chain_pem_len);
        candidate.private_key_pem = copy_pem(fallback_key_iso20, &candidate.private_key_pem_len);
        candidate.root_pem = copy_pem(fallback_v2g_roots, &candidate.root_pem_len);
        if ((candidate.cert_chain_pem != nullptr) && (candidate.private_key_pem != nullptr) && (candidate.root_pem != nullptr)) {
            iso20_candidate_count = 1;
            iso20_allowed = true;
        } else {
            free_iso20_candidate(0);
        }
    }

    // ISO 15118-2 chain selection depends on the EVCC's ClientHello, so a
    // single preselected chain is insufficient. The EVCC indicates its V2G
    // roots with trusted_ca_keys [V2G2-651]. The SECC must select a chain
    // anchored at an indicated root and omit that root from the TLS chain
    // [V2G2-871]. If none matches, another valid root-excluded chain is sent
    // [V2G2-923]. Keep each candidate bound to its chain ID so an accepted
    // status_request_v2 can return the OCSP responses for every certificate
    // of the chain actually selected for the handshake [V2G2-871].

    // We need this for -2 only:
    // - ISO 15118-2 uses TLS 1.2 trusted_ca_keys.
    // - ISO 15118-20 uses TLS 1.3 certificate_authorities.
    // - ISO 15118-2 uses status_request_v2 with a standalone CertificateStatus message.
    // - ISO 15118-20 uses status_request, with the OCSP response attached to each TLS 1.3 CertificateEntry.

    // For -20 see get_iso15118_secc_chain [V2G20-2379] [V2G20-2399]
    iso2_store_live = store_live;
#if MODULE_OCPP_AVAILABLE()
    if (store_live) {
        Ocpp::Iso15118SeccChain chains[ISO2_CANDIDATE_MAX];
        const size_t chain_count = ocpp.get_iso15118_secc_chains(false, chains, ISO2_CANDIDATE_MAX);

        for (size_t i = 0; i < chain_count; ++i) {
            iso2_candidate_t &candidate = iso2_candidates[iso2_candidate_count];
            candidate.chain_id = chains[i].chain_id;
            if (chains[i].chain_pem == nullptr || chains[i].key_pem == nullptr || chains[i].root_pem == nullptr) {
                continue;
            }
            candidate.cert_chain_pem = copy_pem(chains[i].chain_pem.get(), &candidate.cert_chain_pem_len);
            candidate.private_key_pem = copy_pem(chains[i].key_pem.get(), &candidate.private_key_pem_len);
            candidate.root_pem = copy_pem(chains[i].root_pem.get(), &candidate.root_pem_len);
            if (candidate.cert_chain_pem == nullptr || candidate.private_key_pem == nullptr || candidate.root_pem == nullptr) {
                free_iso2_candidate(iso2_candidate_count);
                continue;
            }

            for (size_t cert_idx = 0; cert_idx < ISO2_OCSP_MAX; ++cert_idx) {
                std::unique_ptr<uint8_t[]> der;
                size_t der_len = 0;
                if (!ocpp.get_iso15118_ocsp_staple(candidate.chain_id, static_cast<uint8_t>(cert_idx), &der, &der_len) || der_len == 0) {
                    continue;
                }
                candidate.ocsp_der[cert_idx] = static_cast<uint8_t *>(calloc_psram_or_dram(der_len, 1));
                if (candidate.ocsp_der[cert_idx] != nullptr) {
                    memcpy(candidate.ocsp_der[cert_idx], der.get(), der_len);
                    candidate.ocsp_der_len[cert_idx] = der_len;
                }
            }
            ++iso2_candidate_count;
        }
    }
#endif

#if OPTIONS_ISO15118_DEV_CERTS_ENABLED()
    if (!store_live) {
        iso2_candidate_t &candidate = iso2_candidates[0];
        candidate.cert_chain_pem = copy_pem(dev_cert_chain_pem_iso2, &candidate.cert_chain_pem_len);
        candidate.private_key_pem = copy_pem(dev_private_key_pem_iso2, &candidate.private_key_pem_len);
        candidate.root_pem = copy_pem(dev_v2g_root_ca_pem_iso2, &candidate.root_pem_len);
        if (candidate.cert_chain_pem != nullptr && candidate.private_key_pem != nullptr && candidate.root_pem != nullptr) {
            iso2_candidate_count = 1;
        }
    }
#endif

    if (iso2_candidate_count == 0) {
        iso15118.trace("ISOTLS: No usable ISO 15118-2 SECC chain available, TLS unavailable");
        return false;
    }

    if (live_oem_roots || !store_live) {
        oem_root_ca_pem_iso20 = copy_pem(live_oem_roots ? live_oem_roots.get() : fallback_oem_roots, &oem_root_ca_pem_len_iso20);
    }
    if (live_v2g_roots || !store_live) {
        v2g_root_ca_pem_iso20 = copy_pem(live_v2g_roots ? live_v2g_roots.get() : fallback_v2g_roots, &v2g_root_ca_pem_len_iso20);
    }

    iso15118.trace("ISOTLS: Loaded %zu ISO 15118-2 SECC candidate(s) from %s",
                    iso2_candidate_count, store_live ? "OCPP store" : "dev certs");
    if (iso20_allowed) {
        iso15118.trace("ISOTLS: Loaded %zu ISO 15118-20 SECC candidate(s) from %s",
                        iso20_candidate_count, store_live ? "OCPP store" : "dev certs");
    } else {
        iso15118.trace("ISOTLS: -20 SECC chain missing, expired or without good OCSP status, offering TLS 1.2 only [HUB20-532-002]");
    }
    iso15118.trace("ISOTLS: Trusted roots for ISO 15118-20 mutual TLS: OEM from %s (%zu bytes), V2G from %s (%zu bytes)",
                    live_oem_roots ? "OCPP store" : "dev certs", oem_root_ca_pem_len_iso20 > 0 ? oem_root_ca_pem_len_iso20 - 1 : 0,
                    live_v2g_roots ? "OCPP store" : "dev certs", v2g_root_ca_pem_len_iso20 > 0 ? v2g_root_ca_pem_len_iso20 - 1 : 0);

    return true;
}

void ISOTLS::free_iso2_candidate(size_t index)
{
    iso2_candidate_t &candidate = iso2_candidates[index];

    if (candidate.cert_chain != nullptr) {
        mbedtls_x509_crt_free(candidate.cert_chain);
        free_any(candidate.cert_chain);
    }
    if (candidate.private_key != nullptr) {
        mbedtls_pk_free(candidate.private_key);
        free_any(candidate.private_key);
    }
    if (candidate.root != nullptr) {
        mbedtls_x509_crt_free(candidate.root);
        free_any(candidate.root);
    }
    free_any(candidate.cert_chain_pem);
    if (candidate.private_key_pem != nullptr) {
        mbedtls_platform_zeroize(candidate.private_key_pem, candidate.private_key_pem_len);
    }
    free_any(candidate.private_key_pem);
    free_any(candidate.root_pem);
    for (size_t i = 0; i < ISO2_OCSP_MAX; ++i) {
        free_any(candidate.ocsp_der[i]);
    }
    free_any(candidate.ocsp_response_list);
    candidate = {};
}

void ISOTLS::free_iso20_candidate(size_t index)
{
    iso20_candidate_t &candidate = iso20_candidates[index];
    if (candidate.cert_chain != nullptr) {
        mbedtls_x509_crt_free(candidate.cert_chain);
        free_any(candidate.cert_chain);
    }
    if (candidate.private_key != nullptr) {
        mbedtls_pk_free(candidate.private_key);
        free_any(candidate.private_key);
    }
    if (candidate.root != nullptr) {
        mbedtls_x509_crt_free(candidate.root);
        free_any(candidate.root);
    }
    free_any(candidate.cert_chain_pem);
    if (candidate.private_key_pem != nullptr) {
        mbedtls_platform_zeroize(candidate.private_key_pem, candidate.private_key_pem_len);
    }
    free_any(candidate.private_key_pem);
    free_any(candidate.root_pem);
    for (size_t i = 0; i < OCSP_STAPLE_MAX; ++i) {
        free_any(candidate.staple_der[i]);
    }
    candidate = {};
}

bool ISOTLS::parse_iso20_candidates()
{
    size_t valid_count = 0;
    const size_t loaded_count = iso20_candidate_count;
    for (size_t i = 0; i < loaded_count; ++i) {
        iso20_candidate_t &candidate = iso20_candidates[i];
        candidate.cert_chain = static_cast<mbedtls_x509_crt *>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
        candidate.private_key = static_cast<mbedtls_pk_context *>(calloc_psram_or_dram(1, sizeof(mbedtls_pk_context)));
        candidate.root = static_cast<mbedtls_x509_crt *>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
        if (candidate.cert_chain != nullptr) {
            mbedtls_x509_crt_init(candidate.cert_chain);
        }
        if (candidate.private_key != nullptr) {
            mbedtls_pk_init(candidate.private_key);
        }
        if (candidate.root != nullptr) {
            mbedtls_x509_crt_init(candidate.root);
        }
        if (candidate.cert_chain == nullptr || candidate.private_key == nullptr || candidate.root == nullptr) {
            free_iso20_candidate(i);
            continue;
        }

        int ret = mbedtls_x509_crt_parse(candidate.cert_chain, candidate.cert_chain_pem, candidate.cert_chain_pem_len);
        if (ret == 0) {
            ret = mbedtls_x509_crt_parse(candidate.root, candidate.root_pem, candidate.root_pem_len);
        }
        if (ret == 0) {
            ret = mbedtls_pk_parse_key(candidate.private_key, candidate.private_key_pem, candidate.private_key_pem_len,
                                       nullptr, 0, mbedtls_ctr_drbg_random, ctr_drbg);
        }
        if (ret == 0) {
            ret = mbedtls_pk_check_pair(&candidate.cert_chain->pk, candidate.private_key,
                                        mbedtls_ctr_drbg_random, ctr_drbg);
        }
#if MODULE_OCPP_AVAILABLE()
        if (ret == 0 && iso2_store_live) {
            const OcppCurve21 curve = (mbedtls_pk_get_type(&candidate.cert_chain->pk) == MBEDTLS_PK_ED448) ? OcppCurve21::Ed448 : OcppCurve21::Secp521r1;
            if (!ocpp.is_iso20_suite_enabled(curve)) {
                iso15118.trace("ISOTLS: Ignoring disabled ISO20 %s candidate %zu", curve == OcppCurve21::Ed448 ? "ed448" : "ecdsa", i);
                free_iso20_candidate(i);
                continue;
            }
        }
#endif
        if (ret != 0) {
            iso15118.trace("ISOTLS: Ignoring invalid ISO20 candidate %zu: -0x%04x", i, static_cast<unsigned>(-ret));
            free_iso20_candidate(i);
            continue;
        }

        iso15118.trace("ISOTLS: ISO20 candidate %zu (chain ID %u): %s",
                        valid_count, static_cast<unsigned>(candidate.chain_id),
                        mbedtls_pk_get_type(&candidate.cert_chain->pk) == MBEDTLS_PK_ED448 ? "ed448" : "ecdsa");
        if (valid_count != i) {
            iso20_candidates[valid_count] = candidate;
            candidate = {};
        }
        ++valid_count;
    }
    iso20_candidate_count = valid_count;
    return valid_count > 0;
}

bool ISOTLS::parse_iso2_candidates()
{
    size_t valid_count = 0;
    const size_t loaded_count = iso2_candidate_count;

    for (size_t i = 0; i < loaded_count; ++i) {
        iso2_candidate_t &candidate = iso2_candidates[i];
        candidate.cert_chain = static_cast<mbedtls_x509_crt *>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
        candidate.private_key = static_cast<mbedtls_pk_context *>(calloc_psram_or_dram(1, sizeof(mbedtls_pk_context)));
        candidate.root = static_cast<mbedtls_x509_crt *>(calloc_psram_or_dram(1, sizeof(mbedtls_x509_crt)));
        if (candidate.cert_chain != nullptr) {
            mbedtls_x509_crt_init(candidate.cert_chain);
        }
        if (candidate.private_key != nullptr) {
            mbedtls_pk_init(candidate.private_key);
        }
        if (candidate.root != nullptr) {
            mbedtls_x509_crt_init(candidate.root);
        }
        if (candidate.cert_chain == nullptr || candidate.private_key == nullptr || candidate.root == nullptr) {
            iso15118.trace("ISOTLS: Failed to allocate ISO2 candidate %zu contexts", i);
            free_iso2_candidate(i);
            continue;
        }

        int ret = mbedtls_x509_crt_parse(candidate.cert_chain, candidate.cert_chain_pem, candidate.cert_chain_pem_len);
        if (ret == 0) {
            ret = mbedtls_pk_parse_key(candidate.private_key, candidate.private_key_pem, candidate.private_key_pem_len,
                                       nullptr, 0, mbedtls_ctr_drbg_random, ctr_drbg);
        }
        if (ret == 0) {
            ret = mbedtls_x509_crt_parse(candidate.root, candidate.root_pem, candidate.root_pem_len);
        }
        if (ret == 0) {
            ret = mbedtls_pk_check_pair(&candidate.cert_chain->pk, candidate.private_key,
                                        mbedtls_ctr_drbg_random, ctr_drbg);
        }
        if (ret != 0) {
            iso15118.trace("ISOTLS: Ignoring invalid ISO2 candidate %zu: -0x%04x", i, static_cast<unsigned>(-ret));
            free_iso2_candidate(i);
            continue;
        }

        mbedtls_asn1_buf root_key;
        if (mbedtls_sha1(candidate.root->raw.p, candidate.root->raw.len, candidate.root_cert_sha1) != 0 ||
            !spki_bitstring(&candidate.root->pk_raw, &root_key) ||
            mbedtls_sha1(root_key.p, root_key.len, candidate.root_key_sha1) != 0) {
            iso15118.trace("ISOTLS: Ignoring ISO2 candidate %zu with invalid root SPKI", i);
            free_iso2_candidate(i);
            continue;
        }

        for (mbedtls_x509_crt *cert = candidate.cert_chain; cert != nullptr && cert->raw.p != nullptr; cert = cert->next) {
            ++candidate.cert_count;
        }

        candidate.ocsp_complete = iso2_store_live && candidate.cert_count > 0 && candidate.cert_count <= ISO2_OCSP_MAX;
        size_t response_list_len = 0;
        for (size_t cert_idx = 0; cert_idx < candidate.cert_count && cert_idx < ISO2_OCSP_MAX; ++cert_idx) {
            if (candidate.ocsp_der[cert_idx] == nullptr || candidate.ocsp_der_len[cert_idx] == 0 ||
                candidate.ocsp_der_len[cert_idx] > 0xFFFFFF ||
                response_list_len > SIZE_MAX - 3 - candidate.ocsp_der_len[cert_idx]) {
                candidate.ocsp_complete = false;
                break;
            }
            response_list_len += 3 + candidate.ocsp_der_len[cert_idx];
        }

        if (candidate.ocsp_complete) {
            candidate.ocsp_complete = response_list_len <= 0xFFFFFF && response_list_len <= MBEDTLS_SSL_OUT_CONTENT_LEN - 8;
        }
        if (candidate.ocsp_complete) {
            candidate.ocsp_response_list = static_cast<uint8_t *>(calloc_psram_or_dram(response_list_len, 1));
            if (candidate.ocsp_response_list == nullptr) {
                candidate.ocsp_complete = false;
            } else {
                uint8_t *p = candidate.ocsp_response_list;
                for (size_t cert_idx = 0; cert_idx < candidate.cert_count; ++cert_idx) {
                    const size_t der_len = candidate.ocsp_der_len[cert_idx];
                    p[0] = static_cast<uint8_t>(der_len >> 16);
                    p[1] = static_cast<uint8_t>(der_len >> 8);
                    p[2] = static_cast<uint8_t>(der_len);
                    memcpy(p + 3, candidate.ocsp_der[cert_idx], der_len);
                    p += 3 + der_len;
                }
                candidate.ocsp_response_list_len = response_list_len;
            }
        }

        iso15118.trace("ISOTLS: ISO2 candidate %zu (chain ID %u): %zu certificate(s), OCSP multi %s",
                        valid_count, static_cast<unsigned>(candidate.chain_id), candidate.cert_count,
                        candidate.ocsp_complete ? "ready" : "unavailable");

        if (valid_count != i) {
            iso2_candidates[valid_count] = candidate;
            candidate = {};
        }
        ++valid_count;
    }

    iso2_candidate_count = valid_count;
    return valid_count > 0;
}

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

    for (size_t i = 0; i < ISO2_CANDIDATE_MAX; ++i) {
        free_iso2_candidate(i);
    }
    iso2_candidate_count = 0;
    selected_iso2_candidate = 0;
    selected_iso20_candidate = 0;
    iso20_certificate_authorities_seen = false;
    iso20_certificate_authority_matched = false;
    iso2_store_live = false;
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
    const BaseType_t ret = xTaskCreatePinnedToCore(verify_certs_task, "verify_certs", 12288, ctx, 10, nullptr, 0); // Priority above httpd but below all other core 0 tasks.

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

int ISOTLS::select_iso20_certificate_authority(mbedtls_ssl_context *ssl_ctx, const unsigned char *data, size_t data_len)
{
    selected_iso20_candidate = 0;
    iso20_certificate_authorities_seen = data != nullptr;
    iso20_certificate_authority_matched = false;
    if (ssl_ctx != ssl || iso20_candidate_count == 0) {
        return -1;
    }
    if (data == nullptr) {
        return 0;
    }
    if (data_len < 5) {
        return -1;
    }

    const size_t list_len = (static_cast<size_t>(data[0]) << 8) | data[1];
    if (list_len != data_len - 2) {
        return -1;
    }

    // Candidates are already ordered freshest first. Select the freshest
    // candidate whose anchoring root DN occurs anywhere in the EVCC list.
    for (size_t candidate_idx = 0; candidate_idx < iso20_candidate_count; ++candidate_idx) {
        const iso20_candidate_t &candidate = iso20_candidates[candidate_idx];
        if (candidate.root == nullptr) {
            continue;
        }

        const unsigned char *p = data + 2;
        const unsigned char *end = data + data_len;
        while (p < end) {
            const size_t dn_len = (static_cast<size_t>(p[0]) << 8) | p[1];
            p += 2;
            if (x509_name_equal(candidate.root->subject_raw.p, candidate.root->subject_raw.len, p, dn_len)) {
                selected_iso20_candidate = candidate_idx;
                iso20_certificate_authority_matched = true;
                iso15118.trace("ISOTLS: ClientHello certificate_authorities selected ISO20 candidate %zu (chain ID %u)",
                               candidate_idx, static_cast<unsigned>(candidate.chain_id));
                return 0;
            }
            p += dn_len;
        }
    }

    iso15118.trace("ISOTLS: No ClientHello certificate_authorities match, retaining ISO20 fallback candidate 0 [V2G20-2399]");
    return 0;
}

int ISOTLS::select_iso2_trusted_ca(mbedtls_ssl_context *ssl_ctx, const unsigned char *data, size_t data_len)
{
    selected_iso2_candidate = 0;
    if (ssl_ctx != ssl || data == nullptr || data_len < 2 || iso2_candidate_count == 0) {
        return -1;
    }

    const size_t list_len = (static_cast<size_t>(data[0]) << 8) | data[1];
    if (list_len != data_len - 2) {
        return -1;
    }

    const unsigned char *p = data + 2;
    const unsigned char *end = data + data_len;
    bool matched = false;
    size_t matched_candidate = 0;

    while (p < end) {
        const uint8_t identifier_type = *p++;
        const unsigned char *identifier = nullptr;
        size_t identifier_len = 0;

        switch (identifier_type) {
            case 0: // pre_agreed
                if (!matched) {
                    matched = true;
                    matched_candidate = 0;
                }
                break;

            case 1: // key_sha1_hash
            case 3: // cert_sha1_hash
                if (static_cast<size_t>(end - p) < 20) {
                    selected_iso2_candidate = 0;
                    return -1;
                }
                identifier = p;
                identifier_len = 20;
                p += 20;
                break;

            case 2: // x509_name
                if (static_cast<size_t>(end - p) < 2) {
                    selected_iso2_candidate = 0;
                    return -1;
                }
                identifier_len = (static_cast<size_t>(p[0]) << 8) | p[1];
                p += 2;
                if (identifier_len == 0 || identifier_len > static_cast<size_t>(end - p)) {
                    selected_iso2_candidate = 0;
                    return -1;
                }
                identifier = p;
                p += identifier_len;
                break;

            default:
                selected_iso2_candidate = 0;
                return -1;
        }

        if (!matched && identifier_type != 0) {
            for (size_t i = 0; i < iso2_candidate_count; ++i) {
                const iso2_candidate_t &candidate = iso2_candidates[i];
                bool candidate_matches = false;
                if (identifier_type == 1) {
                    candidate_matches = memcmp(identifier, candidate.root_key_sha1, identifier_len) == 0;
                } else if (identifier_type == 2) {
                    candidate_matches = identifier_len == candidate.root->subject_raw.len &&
                                        memcmp(identifier, candidate.root->subject_raw.p, identifier_len) == 0;
                } else {
                    candidate_matches = memcmp(identifier, candidate.root_cert_sha1, identifier_len) == 0;
                }
                if (candidate_matches) {
                    matched = true;
                    matched_candidate = i;
                    break;
                }
            }
        }
    }

    selected_iso2_candidate = matched ? matched_candidate : 0;
    iso15118.trace("ISOTLS: trusted_ca_keys %s, ISO2 candidate %zu selected",
                    matched ? "matched" : "had no match", selected_iso2_candidate);
    return 0;
}

int ISOTLS::accept_iso2_status_request_v2(mbedtls_ssl_context *ssl_ctx, const unsigned char *data, size_t data_len)
{
    iso2_status_v2_requested = false;
    if (ssl_ctx != ssl || data == nullptr || data_len < 2 || iso2_candidate_count == 0) {
        return -1;
    }

    const size_t list_len = (static_cast<size_t>(data[0]) << 8) | data[1];
    if (list_len != data_len - 2) {
        return -1;
    }

    const unsigned char *p = data + 2;
    const unsigned char *end = data + data_len;
    bool has_ocsp_multi = false;

    while (p < end) {
        if (static_cast<size_t>(end - p) < 3) {
            return -1;
        }
        const uint8_t status_type = *p++;
        const size_t request_len = (static_cast<size_t>(p[0]) << 8) | p[1];
        p += 2;
        if (request_len > static_cast<size_t>(end - p)) {
            return -1;
        }

        const unsigned char *request_end = p + request_len;
        if (status_type == 2) { // ocsp_multi
            if (static_cast<size_t>(request_end - p) < 2) {
                return -1;
            }
            const size_t responder_id_list_len = (static_cast<size_t>(p[0]) << 8) | p[1];
            p += 2;
            if (responder_id_list_len > static_cast<size_t>(request_end - p)) {
                return -1;
            }
            p += responder_id_list_len;
            if (static_cast<size_t>(request_end - p) < 2) {
                return -1;
            }
            const size_t request_extensions_len = (static_cast<size_t>(p[0]) << 8) | p[1];
            p += 2;
            if (request_extensions_len != static_cast<size_t>(request_end - p)) {
                return -1;
            }
            p += request_extensions_len;
            has_ocsp_multi = true;
        } else {
            p = request_end;
        }
    }

    if (!has_ocsp_multi) {
        return -1;
    }

    iso2_status_v2_requested = true;
    return 0;
}

int ISOTLS::iso2_status_request_v2_available(mbedtls_ssl_context *ssl_ctx) const
{
    if (ssl_ctx != ssl || !iso2_status_v2_requested || selected_iso2_candidate >= iso2_candidate_count) {
        return -1;
    }

    const iso2_candidate_t &candidate = iso2_candidates[selected_iso2_candidate];
    return candidate.ocsp_complete && candidate.ocsp_response_list != nullptr && candidate.ocsp_response_list_len > 0 ? 0 : -1;
}

int ISOTLS::get_iso2_status_response_v2(mbedtls_ssl_context *ssl_ctx, const unsigned char **response_list, size_t *response_list_len)
{
    if (ssl_ctx != ssl || response_list == nullptr || response_list_len == nullptr || !iso2_status_v2_requested ||
        selected_iso2_candidate >= iso2_candidate_count) {
        return -1;
    }

    const iso2_candidate_t &candidate = iso2_candidates[selected_iso2_candidate];
    if (!candidate.ocsp_complete || candidate.ocsp_response_list == nullptr || candidate.ocsp_response_list_len == 0) {
        return -1;
    }

    *response_list = candidate.ocsp_response_list;
    *response_list_len = candidate.ocsp_response_list_len;
    return 0;
}

void ISOTLS::set_mutual_auth_enabled(bool enabled)
{
    mutual_auth_enabled = enabled;
    iso15118.trace("ISOTLS: Mutual TLS authentication %s", enabled ? "enabled" : "disabled");
}

bool ISOTLS::get_ocsp_staple(const mbedtls_ssl_context *ssl_ctx, size_t index,
                             const unsigned char **der, size_t *der_len) const
{
    if (ssl_ctx != ssl) {
        return false;
    }
    const mbedtls_x509_crt *selected = mbedtls_ssl_get_hs_own_cert(ssl_ctx);
    const iso20_candidate_t *candidate = nullptr;
    for (size_t i = 0; i < iso20_candidate_count; ++i) {
        if (iso20_candidates[i].cert_chain == selected) {
            candidate = &iso20_candidates[i];
            break;
        }
    }
    if ((candidate == nullptr) || (index >= OCSP_STAPLE_MAX) || (candidate->staple_der[index] == nullptr) || (candidate->staple_der_len[index] == 0)) {
        return false;
    }
    *der = candidate->staple_der[index];
    *der_len = candidate->staple_der_len[index];
    return true;
}

bool ISOTLS::is_mutual_auth_enabled() const
{
    return mutual_auth_enabled;
}
