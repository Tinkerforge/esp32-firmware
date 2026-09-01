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
#include "tools/freertos.h"
#include "tools/malloc.h"

#include <stdlib.h>
#include <string.h>
#include <limits>

#include "mbedtls/error.h"
#include "mbedtls/sha256.h"

namespace {

// Don't inline error logger, keeps the buffer off the stack.
[[gnu::noinline]]
void log_mbedtls_error(int error, const char *msg)
{
    char error_buf[128];
    mbedtls_strerror(error, error_buf, sizeof(error_buf));
    iso15118.trace("ISOTLS: %s: -0x%04x (%s)", msg, static_cast<unsigned>(-error), error_buf);
}

bool cert_signature_is_valid(const mbedtls_x509_crt *child, mbedtls_x509_crt *parent)
{
    const int sig_result = x509_crt_check_signature(child, parent, nullptr);

    if (sig_result == 0) {
        return true;
    }

    log_mbedtls_error(sig_result, "Certificate signature check failed");
    return false;
}

} // namespace

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
