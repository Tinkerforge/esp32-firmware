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

#include "event_log_prefix.h"
#include "../generated/module_dependencies.h"
#include "tools.h"
#include "tools/malloc.h"

#include <string.h>

#include "mbedtls/asn1.h"
#include "mbedtls/oid.h"
#include "mbedtls/platform_util.h"

namespace {

constexpr size_t X509_NAME_ATTRIBUTE_MAX = 16;

struct x509_name_attribute_t {
    mbedtls_asn1_buf oid = {};
    mbedtls_asn1_buf value = {};
    bool continues_rdn = false;
};

bool parse_x509_name(const unsigned char *der, size_t der_len, x509_name_attribute_t *attributes, size_t *attribute_count)
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

unsigned char ascii_lower(unsigned char value)
{
    return value >= 'A' && value <= 'Z' ? value + ('a' - 'A') : value;
}

bool normalized_directory_string_byte(const mbedtls_asn1_buf &value, size_t *offset, unsigned char *result)
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

bool x509_name_value_equal(const mbedtls_asn1_buf &a, const mbedtls_asn1_buf &b)
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
bool x509_name_equal(const unsigned char *a_der, size_t a_len, const unsigned char *b_der, size_t b_len)
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

size_t der_length_size(size_t len)
{
    if (len < 128) {
        return 1;
    }
    size_t bytes = 0;
    for (; len != 0; len >>= 8) {
        ++bytes;
    }
    return 1 + bytes;
}

uint8_t *write_der_length(uint8_t *p, size_t len)
{
    if (len < 128) {
        *p++ = static_cast<uint8_t>(len);
        return p;
    }
    const size_t bytes = der_length_size(len) - 1;
    *p++ = static_cast<uint8_t>(0x80 | bytes);
    for (size_t i = bytes; i > 0; --i) {
        *p++ = static_cast<uint8_t>(len >> ((i - 1) * 8));
    }
    return p;
}

struct iso20_dn_field_t {
    const char *oid;
    size_t oid_len;
    int default_tag;
    const unsigned char *value = nullptr;
    size_t value_len = 0;
    int value_tag = 0;
};

const mbedtls_x509_name *find_name_field(const mbedtls_x509_name *name, const char *oid, size_t oid_len)
{
    for (; name != nullptr; name = name->next) {
        if ((name->oid.len == oid_len) && (memcmp(name->oid.p, oid, oid_len) == 0)) {
            return name;
        }
    }
    return nullptr;
}

size_t iso20_dn_content_size(const iso20_dn_field_t *fields, size_t count)
{
    size_t result = 0;
    for (size_t i = 0; i < count; ++i) {
        const size_t oid_tlv = 1 + der_length_size(fields[i].oid_len) + fields[i].oid_len;
        const size_t value_tlv = 1 + der_length_size(fields[i].value_len) + fields[i].value_len;
        const size_t attribute = oid_tlv + value_tlv;
        const size_t attribute_tlv = 1 + der_length_size(attribute) + attribute;
        result += 1 + der_length_size(attribute_tlv) + attribute_tlv;
    }
    return result;
}

size_t iso20_dn_size(const iso20_dn_field_t *fields, size_t count)
{
    const size_t content = iso20_dn_content_size(fields, count);
    return 1 + der_length_size(content) + content;
}

uint8_t *write_iso20_dn(uint8_t *p, const iso20_dn_field_t *fields, size_t count)
{
    const size_t content_len = iso20_dn_content_size(fields, count);
    *p++ = MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE;
    p = write_der_length(p, content_len);

    for (size_t i = 0; i < count; ++i) {
        const size_t oid_tlv = 1 + der_length_size(fields[i].oid_len) + fields[i].oid_len;
        const size_t value_tlv = 1 + der_length_size(fields[i].value_len) + fields[i].value_len;
        const size_t attribute_len = oid_tlv + value_tlv;
        const size_t attribute_tlv = 1 + der_length_size(attribute_len) + attribute_len;
        *p++ = MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SET;
        p = write_der_length(p, attribute_tlv);
        *p++ = MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE;
        p = write_der_length(p, attribute_len);
        *p++ = MBEDTLS_ASN1_OID;
        p = write_der_length(p, fields[i].oid_len);
        memcpy(p, fields[i].oid, fields[i].oid_len);
        p += fields[i].oid_len;
        *p++ = static_cast<uint8_t>(fields[i].value_tag != 0 ? fields[i].value_tag : fields[i].default_tag);
        p = write_der_length(p, fields[i].value_len);
        if (fields[i].value_len > 0) {
            memcpy(p, fields[i].value, fields[i].value_len);
            p += fields[i].value_len;
        }
    }
    return p;
}

} // namespace

bool ISOTLS::build_iso20_certificate_authorities()
{
    // Reserve CertificateRequest framing and signature_algorithms. All roots
    // required by V2G20-2401 must fit; partial advertisement is not allowed.
    constexpr size_t capacity = MBEDTLS_SSL_OUT_CONTENT_LEN - 256;
    iso20_certificate_authorities = static_cast<uint8_t *>(calloc_psram_or_dram(capacity, 1));
    if (iso20_certificate_authorities == nullptr) {
        return false;
    }

    size_t used = 2;
    for (const mbedtls_x509_crt *root = trusted_ca_iso20;
         root != nullptr && root->raw.p != nullptr; root = root->next) {
        unsigned char serial[40];
        size_t serial_offset = 0;
        while (serial_offset + 1 < root->serial.len && root->serial.p[serial_offset] == 0) {
            ++serial_offset;
        }
        const size_t serial_len = root->serial.len - serial_offset;
        if (serial_len == 0 || serial_len > sizeof(serial) / 2) {
            iso15118.trace("ISOTLS: Root serial is too large for V2G20-2403 DN");
            goto fail;
        }
        static constexpr char hex[] = "0123456789ABCDEF";
        for (size_t i = 0; i < serial_len; ++i) {
            serial[2 * i] = hex[root->serial.p[serial_offset + i] >> 4];
            serial[2 * i + 1] = hex[root->serial.p[serial_offset + i] & 0x0F];
        }

        iso20_dn_field_t fields[] = {
            {MBEDTLS_OID_AT_COUNTRY, MBEDTLS_OID_SIZE(MBEDTLS_OID_AT_COUNTRY), MBEDTLS_ASN1_PRINTABLE_STRING},
            {MBEDTLS_OID_AT_ORGANIZATION, MBEDTLS_OID_SIZE(MBEDTLS_OID_AT_ORGANIZATION), MBEDTLS_ASN1_UTF8_STRING},
            {MBEDTLS_OID_AT_ORG_UNIT, MBEDTLS_OID_SIZE(MBEDTLS_OID_AT_ORG_UNIT), MBEDTLS_ASN1_UTF8_STRING},
            {MBEDTLS_OID_AT_CN, MBEDTLS_OID_SIZE(MBEDTLS_OID_AT_CN), MBEDTLS_ASN1_UTF8_STRING},
            {MBEDTLS_OID_AT_SERIAL_NUMBER, MBEDTLS_OID_SIZE(MBEDTLS_OID_AT_SERIAL_NUMBER), MBEDTLS_ASN1_PRINTABLE_STRING,
             serial, serial_len * 2, MBEDTLS_ASN1_PRINTABLE_STRING},
        };
        for (size_t i = 0; i < ARRAY_SIZE(fields) - 1; ++i) {
            const mbedtls_x509_name *source = find_name_field(&root->issuer, fields[i].oid, fields[i].oid_len);
            if (source != nullptr) {
                fields[i].value = source->val.p;
                fields[i].value_len = source->val.len;
                fields[i].value_tag = source->val.tag;
            }
        }

        uint8_t encoded[512];
        const size_t dn_len = iso20_dn_size(fields, ARRAY_SIZE(fields));
        if (dn_len > sizeof(encoded)) {
            iso15118.trace("ISOTLS: V2G20-2403 root DN is too large: %zu bytes", dn_len);
            goto fail;
        }
        write_iso20_dn(encoded, fields, ARRAY_SIZE(fields));

        bool duplicate = false;
        const uint8_t *existing = iso20_certificate_authorities + 2;
        const uint8_t *existing_end = iso20_certificate_authorities + used;
        while (existing < existing_end) {
            const size_t existing_len = (static_cast<size_t>(existing[0]) << 8) | existing[1];
            existing += 2;
            if (existing_len == dn_len && memcmp(existing, encoded, dn_len) == 0) {
                duplicate = true;
                break;
            }
            existing += existing_len;
        }
        if (duplicate) {
            continue;
        }
        if ((dn_len > UINT16_MAX) || (used > (capacity - 2 - dn_len)) || ((used - 2) > (UINT16_MAX - 2 - dn_len))) {
            iso15118.trace("ISOTLS: All V2G/OEM root DNs do not fit in CertificateRequest [V2G20-2401]");
            goto fail;
        }
        iso20_certificate_authorities[used++] = static_cast<uint8_t>(dn_len >> 8);
        iso20_certificate_authorities[used++] = static_cast<uint8_t>(dn_len);
        memcpy(iso20_certificate_authorities + used, encoded, dn_len);
        used += dn_len;
    }

    if (used == 2) {
        free_any(iso20_certificate_authorities);
        iso20_certificate_authorities = nullptr;
        return true; // AMD1: omit the extension when no applicable roots exist.
    }
    iso20_certificate_authorities[0] = static_cast<uint8_t>((used - 2) >> 8);
    iso20_certificate_authorities[1] = static_cast<uint8_t>(used - 2);
    iso20_certificate_authorities_len = used;
    iso15118.trace("ISOTLS: Built %zu-byte CertificateRequest root DN list [V2G20-2401/2403]", used);
    return true;

fail:
    free_any(iso20_certificate_authorities);
    iso20_certificate_authorities = nullptr;
    iso20_certificate_authorities_len = 0;
    return false;
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
        if (ret == 0 && certificate_store_live) {
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

int ISOTLS::get_iso20_certificate_authorities(mbedtls_ssl_context *ssl_ctx, const unsigned char **data, size_t *data_len) const
{
    if ((ssl_ctx != ssl) || (data == nullptr) || (data_len == nullptr) || (iso20_certificate_authorities == nullptr) || (iso20_certificate_authorities_len == 0)) {
        return -1;
    }
    *data = iso20_certificate_authorities;
    *data_len = iso20_certificate_authorities_len;
    return 0;
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
