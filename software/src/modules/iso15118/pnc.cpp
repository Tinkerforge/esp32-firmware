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

#include "pnc.h"

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tools/freertos.h"

#include <string.h>

#include "mbedtls/x509_crt.h"
#include "mbedtls/pk.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/oid.h"

#include "gcc_warnings.h"

static bool find_leaf_cn(const mbedtls_x509_crt *leaf, char *cn_out, size_t cn_cap)
{
    for (const mbedtls_x509_name *name = &leaf->subject; name != nullptr; name = name->next) {
        if (MBEDTLS_OID_CMP(MBEDTLS_OID_AT_CN, &name->oid) != 0) {
            continue;
        }
        size_t len = name->val.len < cn_cap - 1 ? name->val.len : cn_cap - 1;
        memcpy(cn_out, name->val.p, len);
        cn_out[len] = '\0';
        return true;
    }
    return false;
}

// Chain validation only. leaf_cn_out receives the leaf subject CN (the eMAID for contract leaves)
PncVerifyResult pnc_validate_chain(const uint8_t *chain, const size_t *cert_len, size_t cert_count,
                                   const char *mo_roots_pem, const char *v2g_roots_pem,
                                   char *leaf_cn_out, size_t leaf_cn_cap)
{
    if (chain == nullptr || cert_count == 0 || cert_count > PNC_CHAIN_MAX_CERTS) {
        return PncVerifyResult::ChainInvalid;
    }
    if (mo_roots_pem == nullptr && v2g_roots_pem == nullptr) {
        iso15118.trace("PNC: No MO or V2G roots installed, contract chain rejected");
        return PncVerifyResult::ChainInvalid;
    }

    PncVerifyResult result = PncVerifyResult::ChainInvalid;
    mbedtls_x509_crt certs;
    mbedtls_x509_crt trust;
    mbedtls_x509_crt_init(&certs);
    mbedtls_x509_crt_init(&trust);

    for (size_t i = 0; i < cert_count; i++) {
        int ret = mbedtls_x509_crt_parse_der(&certs, chain + i * PNC_CERT_MAX, cert_len[i]);
        if (ret != 0) {
            iso15118.trace("PNC: Contract chain certificate %zu parse failed: -0x%04x", i, static_cast<unsigned>(-ret));
            goto out;
        }
    }

    // HUB20-411-002/004: the contract chain validates against MO roots and V2G roots
    if (mo_roots_pem != nullptr) {
        mbedtls_x509_crt_parse(&trust, reinterpret_cast<const uint8_t *>(mo_roots_pem), strlen(mo_roots_pem) + 1);
    }
    if (v2g_roots_pem != nullptr) {
        mbedtls_x509_crt_parse(&trust, reinterpret_cast<const uint8_t *>(v2g_roots_pem), strlen(v2g_roots_pem) + 1);
    }

    {
        uint32_t flags = 0;
        int ret = mbedtls_x509_crt_verify(&certs, &trust, nullptr, nullptr, &flags, nullptr, nullptr);
        if (ret != 0) {
            iso15118.trace("PNC: Contract chain verification failed: -0x%04x flags 0x%08lx", static_cast<unsigned>(-ret), static_cast<unsigned long>(flags));
            if ((flags & MBEDTLS_X509_BADCERT_EXPIRED) != 0) {
                result = PncVerifyResult::ChainExpired;
            } else if ((flags & MBEDTLS_X509_BADCERT_FUTURE) != 0) {
                result = PncVerifyResult::ChainNotYetValid;
            }
            goto out;
        }
    }

    if (leaf_cn_out != nullptr && leaf_cn_cap > 0) {
        leaf_cn_out[0] = '\0';
        find_leaf_cn(&certs, leaf_cn_out, leaf_cn_cap);
    }
    result = PncVerifyResult::Ok;

out:
    mbedtls_x509_crt_free(&certs);
    mbedtls_x509_crt_free(&trust);
    return result;
}

// md(data) equals the expected digest
bool pnc_digest_matches(mbedtls_md_type_t md, const uint8_t *data, size_t data_len,
                        const uint8_t *expected, size_t expected_len)
{
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(md);
    if (info == nullptr) {
        return false;
    }
    uint8_t digest[PNC_DIGEST_MAX];
    size_t digest_len = mbedtls_md_get_size(info);
    if (digest_len > sizeof(digest) || digest_len != expected_len) {
        return false;
    }
    if (mbedtls_md(info, data, data_len, digest) != 0) {
        return false;
    }
    return memcmp(digest, expected, digest_len) == 0;
}

// ECDSA verify of md(data) against the raw r||s signature with the leaf public key of cert_der
bool pnc_verify_signature(const uint8_t *cert_der, size_t cert_der_len, mbedtls_md_type_t md,
                          const uint8_t *data, size_t data_len, const uint8_t *sig_rs, size_t sig_len)
{
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(md);
    if (info == nullptr || sig_len == 0 || (sig_len % 2) != 0) {
        return false;
    }

    uint8_t hash[PNC_DIGEST_MAX];
    size_t hash_len = mbedtls_md_get_size(info);
    if (hash_len > sizeof(hash) || mbedtls_md(info, data, data_len, hash) != 0) {
        return false;
    }

    bool ok = false;
    mbedtls_x509_crt cert;
    mbedtls_x509_crt_init(&cert);
    mbedtls_mpi r, s;
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    if (mbedtls_x509_crt_parse_der(&cert, cert_der, cert_der_len) != 0) {
        goto out;
    }
    if (mbedtls_pk_get_type(&cert.pk) != MBEDTLS_PK_ECKEY) {
        iso15118.trace("PNC: Contract leaf key is not an EC key");
        goto out;
    }

    if (mbedtls_mpi_read_binary(&r, sig_rs, sig_len / 2) != 0
     || mbedtls_mpi_read_binary(&s, sig_rs + sig_len / 2, sig_len / 2) != 0) {
        goto out;
    }

    {
        mbedtls_ecp_keypair *key = mbedtls_pk_ec(cert.pk);
        int ret = mbedtls_ecdsa_verify(&key->MBEDTLS_PRIVATE(grp), hash, hash_len, &key->MBEDTLS_PRIVATE(Q), &r, &s);
        if (ret != 0) {
            iso15118.trace("PNC: Signature verification failed: -0x%04x", static_cast<unsigned>(-ret));
            goto out;
        }
    }
    ok = true;

out:
    mbedtls_mpi_free(&r);
    mbedtls_mpi_free(&s);
    mbedtls_x509_crt_free(&cert);
    return ok;
}

// Map an XML dsig algorithm URI (sha256/sha512 suffix) to the md type
mbedtls_md_type_t pnc_md_from_algorithm(const char *algorithm, size_t len)
{
    if (len >= 6 && memcmp(algorithm + len - 6, "sha512", 6) == 0) {
        return MBEDTLS_MD_SHA512;
    }
    if (len >= 6 && memcmp(algorithm + len - 6, "sha256", 6) == 0) {
        return MBEDTLS_MD_SHA256;
    }
    return MBEDTLS_MD_NONE;
}

// Run pnc_verify_run in a task, job->result switches from Running when done
void pnc_verify_run(PncVerifyJob *job)
{
    PncVerifyResult chain_result = pnc_validate_chain(job->chain.get(), job->cert_len, job->cert_count,
                                                      job->mo_roots.get(), job->v2g_roots.get(), nullptr, 0);
    if (chain_result != PncVerifyResult::Ok) {
        job->result = chain_result;
        return;
    }

    if (!pnc_digest_matches(job->digest_md, job->ref_exi.get(), job->ref_exi_len, job->expected_digest, job->expected_digest_len)) {
        iso15118.trace("PNC: Reference digest mismatch");
        job->result = PncVerifyResult::SignatureInvalid;
        return;
    }

    if (!pnc_verify_signature(job->chain.get(), job->cert_len[0], job->sig_md,
                              job->signed_info_exi.get(), job->signed_info_exi_len, job->sig, job->sig_len)) {
        job->result = PncVerifyResult::SignatureInvalid;
        return;
    }

    job->result = PncVerifyResult::Ok;
}

static void pnc_verify_task(void *ctx)
{
    pnc_verify_run(static_cast<PncVerifyJob *>(ctx));
    vTaskDelete(nullptr);
}

// Validate the chain and verify digest plus signature, sets job->result
bool pnc_verify_start(PncVerifyJob *job)
{
    job->result = PncVerifyResult::Running;

    // Priority above httpd but below all other core 0 tasks
    const BaseType_t ret = xTaskCreatePinnedToCore(pnc_verify_task, "pnc_verify", 6144, job, 10, nullptr, 0);
    if (ret != pdPASS_nowarn) {
        pnc_verify_run(job);
    }
    return true;
}
