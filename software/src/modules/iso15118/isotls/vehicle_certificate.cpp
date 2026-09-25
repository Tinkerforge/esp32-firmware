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

namespace {

constexpr int sequence = MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE;
constexpr unsigned char aia_oid[] = {0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x01, 0x01};
constexpr unsigned char ocsp_oid[] = {0x2b, 0x06, 0x01, 0x05, 0x05, 0x07, 0x30, 0x01};

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

    if (!(cert.MBEDTLS_PRIVATE(ext_types) & MBEDTLS_X509_EXT_BASIC_CONSTRAINTS) || (cert.MBEDTLS_PRIVATE(ca_istrue) != 0) == leaf) {
        flags |= POLICY_FAILURE;
    }

    const unsigned int usage = cert.MBEDTLS_PRIVATE(key_usage);
    const unsigned int required = leaf ? MBEDTLS_X509_KU_DIGITAL_SIGNATURE : MBEDTLS_X509_KU_KEY_CERT_SIGN;
    if (!(cert.MBEDTLS_PRIVATE(ext_types) & MBEDTLS_X509_EXT_KEY_USAGE) || !(usage & required) || (leaf && (usage & (MBEDTLS_X509_KU_KEY_CERT_SIGN | MBEDTLS_X509_KU_CRL_SIGN)))) {
        flags |= MBEDTLS_X509_BADCERT_KEY_USAGE;
    }

    // EKU is optional in the original B.8 profile. If supplied, it must
    // explicitly authorize clientAuth; anyExtendedKeyUsage is insufficient.
    if (leaf && (cert.MBEDTLS_PRIVATE(ext_types) & MBEDTLS_X509_EXT_EXTENDED_KEY_USAGE)) {
        bool client_auth = false;
        for (const mbedtls_x509_sequence *eku = &cert.ext_key_usage; eku != nullptr; eku = eku->next) {
            client_auth |= MBEDTLS_OID_CMP(MBEDTLS_OID_CLIENT_AUTH, &eku->buf) == 0;
        }

        if (!client_auth) {
            flags |= MBEDTLS_X509_BADCERT_EXT_KEY_USAGE;
        }
    }

    flags |= certificate_time_flags(cert, time(nullptr));

    char url[256];
    if (require_ocsp && !ocsp_url(cert, url, sizeof(url))) {
        flags |= POLICY_FAILURE;
    }

    return flags;
}
