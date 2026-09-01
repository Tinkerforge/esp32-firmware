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

#include <string.h>

#include "mbedtls/asn1.h"
#include "mbedtls/platform_util.h"
#include "mbedtls/sha1.h"

extern ISO15118 iso15118;

namespace {

// Extracts the RFC 6066 subjectPublicKey value from a complete SubjectPublicKeyInfo.
bool spki_bitstring(const mbedtls_x509_buf *spki, mbedtls_asn1_buf *out)
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

} // namespace

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

        candidate.ocsp_complete = certificate_store_live && candidate.cert_count > 0 && candidate.cert_count <= ISO2_OCSP_MAX;
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
