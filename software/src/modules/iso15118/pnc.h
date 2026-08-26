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
#include <memory>

#include "mbedtls/md.h"

// Plug and Charge helpers: contract certificate chain validation against
// the MO and V2G trust stores and XML signature verification over EXI
// encoded fragments, shared between the -2 and -20 session layers.

static constexpr size_t PNC_CHAIN_MAX_CERTS = 4;
static constexpr size_t PNC_CERT_MAX = 1600;
static constexpr size_t PNC_SIG_MAX = 132;    // secp521r1 r||s
static constexpr size_t PNC_DIGEST_MAX = 64;
static constexpr size_t PNC_CHALLENGE_LEN = 16;

enum class PncVerifyResult : uint8_t {
    Idle,
    Running,
    Ok,
    ChainExpired,
    ChainNotYetValid,
    ChainInvalid,
    SignatureInvalid,
};

struct PncVerifyJob {
    // Contract chain DER, leaf first
    std::unique_ptr<uint8_t[]> chain;
    size_t cert_len[PNC_CHAIN_MAX_CERTS];
    size_t cert_count = 0;

    // Trust store PEM bundles, either may be null
    std::unique_ptr<char[]> mo_roots;
    std::unique_ptr<char[]> v2g_roots;

    // EXI fragment of the signed element and the expected digest from the Reference
    std::unique_ptr<uint8_t[]> ref_exi;
    size_t ref_exi_len = 0;
    uint8_t expected_digest[PNC_DIGEST_MAX];
    size_t expected_digest_len = 0;

    // EXI fragment of the SignedInfo element and the raw r||s signature
    std::unique_ptr<uint8_t[]> signed_info_exi;
    size_t signed_info_exi_len = 0;
    uint8_t sig[PNC_SIG_MAX];
    size_t sig_len = 0;

    mbedtls_md_type_t digest_md = MBEDTLS_MD_NONE;
    mbedtls_md_type_t sig_md = MBEDTLS_MD_NONE;

    volatile PncVerifyResult result = PncVerifyResult::Idle;
};

void pnc_verify_run(PncVerifyJob *job);
bool pnc_verify_start(PncVerifyJob *job);
PncVerifyResult pnc_validate_chain(const uint8_t *chain, const size_t *cert_len, size_t cert_count,
                                   const char *mo_roots_pem, const char *v2g_roots_pem,
                                   char *leaf_cn_out, size_t leaf_cn_cap);
bool pnc_verify_signature(const uint8_t *cert_der, size_t cert_der_len, mbedtls_md_type_t md,
                          const uint8_t *data, size_t data_len, const uint8_t *sig_rs, size_t sig_len);
bool pnc_digest_matches(mbedtls_md_type_t md, const uint8_t *data, size_t data_len,
                        const uint8_t *expected, size_t expected_len);
mbedtls_md_type_t pnc_md_from_algorithm(const char *algorithm, size_t len);
