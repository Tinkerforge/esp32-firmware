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

#include "vehicle_certificate.h"
#include "tools/certificate_time.h"

#include <cstring>
#include "mbedtls/asn1.h"
#include "mbedtls/oid.h"
#include "mbedtls/sha1.h"

namespace {

constexpr int sequence = MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE;
constexpr unsigned char aia_oid[] = {0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x01};
constexpr unsigned char ocsp_oid[] = {0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x01};
constexpr unsigned char eku_oid[] = {0x55, 0x1d, 0x25};

// All views borrow the parsed certificate's DER. No allocation or network I/O.
struct Der {
    unsigned char *p;
    const unsigned char *end;

    bool take(int tag, Der &value)
    {
        size_t length;
        if (mbedtls_asn1_get_tag(&p, end, &length, tag) != 0) {
            return false;
        }
        value = {p, p + length};
        p += length;
        return true;
    }

    bool empty() const { return p == end; }
    size_t size() const { return static_cast<size_t>(end - p); }
};

template<size_t N>
bool equals(const Der &value, const unsigned char (&oid)[N])
{
    return value.size() == N && memcmp(value.p, oid, N) == 0;
}

bool usable_url(const Der &uri, size_t capacity)
{
    const size_t len = uri.size();
    if (len == 0 || len >= capacity || len > 255) {
        return false;
    }

    size_t prefix;
    if (len > 7 && memcmp(uri.p, "http://", 7) == 0) {
        prefix = 7;
    } else if (len > 8 && memcmp(uri.p, "https://", 8) == 0) {
        prefix = 8;
    } else {
        return false;
    }

    // Require an authority, printable IA5 data and no fragment/userinfo. An
    // unreachable but syntactically usable responder is handled asynchronously.
    if (uri.p[prefix] == '/' || uri.p[prefix] == '?' || uri.p[prefix] == ':') {
        return false;
    }

    for (size_t i = 0; i < len; ++i) {
        if (uri.p[i] <= 0x20 || uri.p[i] >= 0x7f || uri.p[i] == '#' || uri.p[i] == '@' || uri.p[i] == '\\') {
            return false;
        }
    }

    return true;
}

bool vehicle_role(const mbedtls_x509_name &subject)
{
    size_t count = 0;
    for (const mbedtls_x509_name *name = &subject; name != nullptr; name = name->next) {
        if (MBEDTLS_OID_CMP(MBEDTLS_OID_DOMAIN_COMPONENT, &name->oid) != 0) {
            continue;
        }

        // B.8 / AMD1 V2G20-3073/3406: the vehicle role is EV, not the
        // OEM provisioning or MSP contract role, regardless of trust anchor.
        if ((++count != 1) || (name->val.len < 2) || (memcmp(name->val.p + name->val.len - 2, "EV", 2) != 0)) {
            return false;
        }

        for (size_t i = 0; i < name->val.len; ++i) {
            if ((name->val.p[i] < 0x21) || (name->val.p[i] > 0x7e)) {
                return false;
            }
        }
    }
    return count == 1;
}

bool directory_string(const mbedtls_x509_buf &value)
{
    // V2G20-3038 / RFC 5280: CN, O and OU use UTF8String, 1..64
    // characters. Count Unicode scalar values, not their encoded bytes.
    if ((value.tag != MBEDTLS_ASN1_UTF8_STRING) || (value.len == 0) || (value.len > 256)) {
        return false;
    }
    size_t count = 0;
    for (size_t i = 0; i < value.len; ++count) {
        uint32_t code = value.p[i++];
        unsigned int continuation = 0;
        uint32_t minimum = 0;

        if (code >= 0xc2 && code <= 0xdf) {
            continuation = 1; minimum = 0x80; code &= 0x1f;
        } else if (code >= 0xe0 && code <= 0xef) {
            continuation = 2; minimum = 0x800; code &= 0x0f;
        } else if (code >= 0xf0 && code <= 0xf4) {
            continuation = 3; minimum = 0x10000; code &= 0x07;
        } else if (code >= 0x80) {
            return false;
        }

        while (continuation-- > 0) {
            if (i == value.len || (value.p[i] & 0xc0) != 0x80) {
                return false;
            }
            code = (code << 6) | (value.p[i++] & 0x3f);
        }

        if (code < minimum || code > 0x10ffff || (code >= 0xd800 && code <= 0xdfff)) {
            return false;
        }
    }
    return count <= 64;
}

bool evccid(const mbedtls_x509_buf &value)
{
    // V2G20-3087/2090/2093/2094/2095: C.5 syntax and C.6 check digit.
    // Hyphens are optional only at element boundaries. Case is insignificant.
    if ((value.len < 20) || (value.len > 64)) {
        return false;
    }

    unsigned char normalized[64];
    size_t length = 0;
    bool separator = false;
    for (size_t i = 0; i < value.len; ++i) {
        unsigned char c = value.p[i];
        if (c == '-') {
            if (separator || (length != 3 && length != 4 && i + 2 != value.len)) {
                return false;
            }
            separator = true;
            continue;
        }

        separator = false;
        if (c >= 'a' && c <= 'z') {
            c -= 'a' - 'A';
        }

        if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) || c == 'I' || c == 'O' || c == 'Q') {
            return false;
        }

        normalized[length++] = c;
    }
    if (separator || length < 20 || normalized[3] != 'V') {
        return false;
    }

    unsigned int checksum = 0;
    unsigned int position = 0;

    // Leading zeros are removed within each element before decimal expansion.
    const size_t starts[] = {0, 3, 4};
    for (size_t start : starts) {
        const size_t end = start == 0 ? 3 : start == 3 ? 4 : length - 1;
        while (start < end && normalized[start] == '0') {
            ++start;
        }

        for (size_t i = start; i < end; ++i) {
            const unsigned int n = normalized[i] <= '9' ? normalized[i] - '0' : normalized[i] - 'A' + 10;
            if (n >= 10) {
                checksum = (checksum + (n / 10) * ((1u << (position++ % 28)) % 11)) % 11;
            }
            checksum = (checksum + (n % 10) * ((1u << (position++ % 28)) % 11)) % 11;
        }
    }

    return normalized[length - 1] == (checksum == 10 ? 'X' : '0' + checksum);
}

bool vehicle_name(const mbedtls_x509_name &dn, bool leaf_subject)
{
    unsigned int organizations = 0;
    unsigned int common_names = 0;

    for (const mbedtls_x509_name *name = &dn; name != nullptr; name = name->next) {
        const auto &value = name->val;
        if (MBEDTLS_OID_CMP(MBEDTLS_OID_AT_ORGANIZATION, &name->oid) == 0) {
            if (++organizations != 1 || !directory_string(value)) {
                return false;
            }
        } else if (MBEDTLS_OID_CMP(MBEDTLS_OID_AT_CN, &name->oid) == 0) {
            if (++common_names != 1 || !directory_string(value) || (leaf_subject && !evccid(value))) {
                return false;
            }
        } else if (MBEDTLS_OID_CMP(MBEDTLS_OID_DOMAIN_COMPONENT, &name->oid) == 0) {
            if (value.tag != MBEDTLS_ASN1_IA5_STRING || value.len == 0) {
                return false;
            }
            for (size_t i = 0; i < value.len; ++i) {
                if (value.p[i] > 0x7f) {
                    return false;
                }
            }
        } else if (!leaf_subject && MBEDTLS_OID_CMP(MBEDTLS_OID_AT_ORG_UNIT, &name->oid) == 0) {
            if (!directory_string(value)) {
                return false;
            }
        } else if (!leaf_subject && MBEDTLS_OID_CMP(MBEDTLS_OID_AT_COUNTRY, &name->oid) == 0) {
            if (value.tag != MBEDTLS_ASN1_PRINTABLE_STRING || value.len != 2 ||
                value.p[0] < 'A' || value.p[0] > 'Z' || value.p[1] < 'A' || value.p[1] > 'Z') {
                return false;
            }
        } else {
            // V2G20-2598/3073: leaf subject is O, CN and the EV-role DC.
            return false;
        }
    }
    return organizations == 1 && common_names == 1;
}

bool key_identifier_matches(const mbedtls_x509_buf &identifier, const mbedtls_x509_crt &cert)
{
    // V2G20-3431/3432: RFC 5280 4.2.1.2 methods 1 and 2 hash only the
    // subjectPublicKey BIT STRING, excluding its unused-bits count byte.
    // SHA-1 here identifies a key; it does not authenticate a certificate.
    if (((identifier.len != 8) && (identifier.len != 20)) || (cert.pk_raw.p == nullptr)) {
        return false;
    }
    Der raw{cert.pk_raw.p, cert.pk_raw.p + cert.pk_raw.len}, spki{}, algorithm{}, bits{};
    if (!raw.take(sequence, spki) || !raw.empty() || !spki.take(sequence, algorithm) || !spki.take(MBEDTLS_ASN1_BIT_STRING, bits) || !spki.empty() || (bits.size() < 2) || (*bits.p != 0)) {
        return false;
    }
    unsigned char hash[20];
    if (mbedtls_sha1(bits.p + 1, bits.size() - 1, hash) != 0) {
        return false;
    }
    if (identifier.len == 20) {
        return memcmp(identifier.p, hash, 20) == 0;
    }
    hash[12] = (hash[12] & 0x0f) | 0x40;
    return memcmp(identifier.p, hash + 12, 8) == 0;
}

bool critical_eku(const mbedtls_x509_crt &cert)
{
    // Mbed TLS exposes parsed EKU values but not the extension's critical bit.
    if (cert.v3_ext.p == nullptr) {
        return false;
    }
    Der raw{cert.v3_ext.p, cert.v3_ext.p + cert.v3_ext.len}, extensions{};
    if (!raw.take(sequence, extensions) || !raw.empty()) {
        return false;
    }
    while (!extensions.empty()) {
        Der extension{}, oid{}, value{};
        int critical = 0;
        if (!extensions.take(sequence, extension) || !extension.take(MBEDTLS_ASN1_OID, oid)) {
            return false;
        }
        if (!extension.empty() && (*extension.p == MBEDTLS_ASN1_BOOLEAN) && (mbedtls_asn1_get_bool(&extension.p, extension.end, &critical) != 0)) {
            return false;
        }
        if (!extension.take(MBEDTLS_ASN1_OCTET_STRING, value) || !extension.empty()) {
            return false;
        }
        if (equals(oid, eku_oid)) {
            return critical != 0;
        }
    }
    return false;
}

}

bool ISOVehicleCertificate::issuer_key_matches(const mbedtls_x509_crt &cert, const mbedtls_x509_crt &issuer)
{
    return key_identifier_matches(cert.authority_key_id.keyIdentifier, issuer);
}

bool ISOVehicleCertificate::ocsp_url(const mbedtls_x509_crt &cert, char *url, size_t capacity)
{
    if ((url == nullptr) || (capacity == 0)) {
        return false;
    }

    url[0] = '\0';
    if ((cert.v3_ext.p == nullptr) || (cert.v3_ext.len == 0)) {
        return false;
    }

    Der raw{cert.v3_ext.p, cert.v3_ext.p + cert.v3_ext.len}, extensions{}, selected{};
    if (!raw.take(sequence, extensions) || !raw.empty()) {
        return false;
    }

    bool seen_aia = false;
    while (!extensions.empty()) {
        Der extension{}, oid{}, value{};
        if (!extensions.take(sequence, extension) || !extension.take(MBEDTLS_ASN1_OID, oid)) {
            return false;
        }

        if (!extension.empty() && *extension.p == MBEDTLS_ASN1_BOOLEAN) {
            int critical;
            if (mbedtls_asn1_get_bool(&extension.p, extension.end, &critical) != 0) {
                return false;
            }
        }

        if (!extension.take(MBEDTLS_ASN1_OCTET_STRING, value) || !extension.empty()) {
            return false;
        }

        if (!equals(oid, aia_oid)) {
            continue;
        }

        Der descriptions{};
        if (seen_aia || !value.take(sequence, descriptions) || !value.empty() || descriptions.empty()) {
            return false;
        }

        seen_aia = true;
        while (!descriptions.empty()) {
            Der description{}, method{}, location{};
            if (!descriptions.take(sequence, description) || !description.take(MBEDTLS_ASN1_OID, method) || description.empty()) {
                return false;
            }

            const int tag = *description.p;
            if ((tag & MBEDTLS_ASN1_CONTEXT_SPECIFIC) == 0 || !description.take(tag, location) || !description.empty()) {
                return false;
            }

            if (equals(method, ocsp_oid) && tag == (MBEDTLS_ASN1_CONTEXT_SPECIFIC | 6) &&
                usable_url(location, capacity) && selected.p == nullptr) {
                selected = location;
            }
        }
    }

    if (selected.p == nullptr) {
        return false;
    }

    memcpy(url, selected.p, selected.size());
    url[selected.size()] = '\0';
    return true;
}

uint32_t ISOVehicleCertificate::verify(const mbedtls_x509_crt &cert, bool leaf, bool require_ocsp)
{
    uint32_t flags = 0;
    // V2G20-1001/2432, Annex B.8: the generic TLS profile also permits
    // P-256/P-384 and SHA-256/SHA-384 for legacy connections. Vehicle
    // certificates in ISO-20 must use the P-521/SHA-512 or Ed448 profile.
    const mbedtls_pk_type_t key_type = mbedtls_pk_get_type(&cert.pk);
    const bool p521 = ((key_type == MBEDTLS_PK_ECKEY) || (key_type == MBEDTLS_PK_ECDSA)) && (mbedtls_pk_ec(cert.pk)->MBEDTLS_PRIVATE(grp).id == MBEDTLS_ECP_DP_SECP521R1);
    const bool ed448 = key_type == MBEDTLS_PK_ED448;
    const bool signature_allowed = ((cert.MBEDTLS_PRIVATE(sig_pk) == MBEDTLS_PK_ECDSA) && (cert.MBEDTLS_PRIVATE(sig_md) == MBEDTLS_MD_SHA512)) || (cert.MBEDTLS_PRIVATE(sig_pk) == MBEDTLS_PK_ED448);
    if ((!p521 && !ed448) || !signature_allowed) {
        flags |= POLICY_FAILURE;
    }

    if (cert.version != 3 || !vehicle_role(cert.subject)) {
        flags |= POLICY_FAILURE;
    }
    if (!vehicle_name(cert.subject, leaf) || !vehicle_name(cert.issuer, false)) {
        flags |= POLICY_FAILURE;
    }

    const auto &authority = cert.authority_key_id;
    if (!key_identifier_matches(cert.subject_key_id, cert) ||
        ((authority.keyIdentifier.len != 8) && (authority.keyIdentifier.len != 20)) ||
        (authority.authorityCertIssuer.buf.p != nullptr) || (authority.authorityCertSerialNumber.len != 0)) {
        flags |= POLICY_FAILURE;
    }
    // AKI derivation is checked against the authenticated issuer after chain
    // verification, never against an unverified issuer name or its claimed SKI.

    if (!(cert.MBEDTLS_PRIVATE(ext_types) & MBEDTLS_X509_EXT_BASIC_CONSTRAINTS) || (cert.MBEDTLS_PRIVATE(ca_istrue) != 0) == leaf) {
        flags |= POLICY_FAILURE;
    }

    const unsigned int usage = cert.MBEDTLS_PRIVATE(key_usage);
    const unsigned int required = leaf ? MBEDTLS_X509_KU_DIGITAL_SIGNATURE : MBEDTLS_X509_KU_KEY_CERT_SIGN;
    const unsigned int forbidden = MBEDTLS_X509_KU_DATA_ENCIPHERMENT | MBEDTLS_X509_KU_ENCIPHER_ONLY | MBEDTLS_X509_KU_DECIPHER_ONLY | MBEDTLS_X509_KU_CRL_SIGN | (leaf ? MBEDTLS_X509_KU_KEY_CERT_SIGN : 0);
    if (!(cert.MBEDTLS_PRIVATE(ext_types) & MBEDTLS_X509_EXT_KEY_USAGE) || ((usage & required) != required) || (usage & forbidden)) {
        flags |= MBEDTLS_X509_BADCERT_KEY_USAGE;
    }

    // V2G20-1001/2432, AMD1 Tables B.13/B.14: a vehicle leaf requires
    // critical clientAuth EKU, optionally serverAuth. Vehicle CAs omit EKU.
    const bool has_eku = cert.MBEDTLS_PRIVATE(ext_types) & MBEDTLS_X509_EXT_EXTENDED_KEY_USAGE;
    if (leaf) {
        bool client_auth = false;
        for (const mbedtls_x509_sequence *eku = &cert.ext_key_usage; eku != nullptr; eku = eku->next) {
            const bool client = MBEDTLS_OID_CMP(MBEDTLS_OID_CLIENT_AUTH, &eku->buf) == 0;
            client_auth |= client;
            if (!client && MBEDTLS_OID_CMP(MBEDTLS_OID_SERVER_AUTH, &eku->buf) != 0) {
                flags |= MBEDTLS_X509_BADCERT_EXT_KEY_USAGE;
            }
        }

        if (!has_eku || !client_auth || !critical_eku(cert)) {
            flags |= MBEDTLS_X509_BADCERT_EXT_KEY_USAGE;
        }
    } else if (has_eku) {
        flags |= MBEDTLS_X509_BADCERT_EXT_KEY_USAGE;
    }

    flags |= certificate_time_flags(cert, time(nullptr));

    char url[256];
    if (require_ocsp && !ocsp_url(cert, url, sizeof(url))) {
        flags |= POLICY_FAILURE;
    }

    return flags;
}
