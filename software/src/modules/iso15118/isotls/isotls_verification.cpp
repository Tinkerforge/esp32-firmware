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
#include "vehicle_certificate.h"

#include "event_log_prefix.h"
#include "../generated/module_dependencies.h"
#include "tools/freertos.h"
#include "tools/malloc.h"

#include <stdlib.h>
#include <string.h>

#include "mbedtls/error.h"

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

    // Exempt only an actual installed anchor, never a peer certificate merely
    // claiming to be self-signed. Keep Mbed TLS's existing validation flags.
    bool anchor = false;
    for (const mbedtls_x509_crt *root = isotls->trusted_ca_iso20; root != nullptr; root = root->next) {
        if ((index > 0) && (root->raw.len == cert->raw.len) && (memcmp(root->raw.p, cert->raw.p, cert->raw.len) == 0)) {
            anchor = true;
            break;
        }
    }
    if (!anchor) {
        bool require_ocsp = true;
#if MODULE_OCPP_AVAILABLE()
        require_ocsp = ocpp.is_iso20_ocsp_required();
#endif
        *flags |= ISOVehicleCertificate::verify(*cert, index == 0, require_ocsp);
    }

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

    // Revalidate the complete chain on each full handshake. A leaf-only cache
    // cannot establish the signatures of newly presented intermediates or a
    // changed trust anchor. The successful result lasts for this session only.
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

    // Both cores still verify signatures in parallel, but the result must be
    // known before Mbed TLS accepts Certificate and issues session tickets.
    // Propagate failure through its normal, resumable certificate-alert path.
    if (verify_ctx->async_started) {
        xQueueSemaphoreTake(verify_ctx->sem_handle, portMAX_DELAY_nowarn);
        verify_ctx->async_started = false;
    }
    if (!verify_ctx->intermediates_valid) {
        *flags |= MBEDTLS_X509_BADCERT_NOT_TRUSTED;
    }

    // V2G20-3432: compare each non-anchor AKI with the actual issuer key.
    // The worker has now authenticated intermediates and selected the anchor.
    for (size_t i = 0; i < CERTS_MAX_VERIFY && verify_ctx->certs[i] != nullptr; ++i) {
        const mbedtls_x509_crt *child = verify_ctx->certs[i];
        const mbedtls_x509_crt *root = verify_ctx->anchor_root;
        if ((root != nullptr) && (child->raw.len == root->raw.len) && (memcmp(child->raw.p, root->raw.p, root->raw.len) == 0)) {
            break;
        }
        const mbedtls_x509_crt *issuer = (i + 1) < CERTS_MAX_VERIFY ? verify_ctx->certs[i + 1] : nullptr;
        if (issuer == nullptr) {
            issuer = root;
        }
        if (issuer != nullptr && !ISOVehicleCertificate::issuer_key_matches(*child, *issuer)) {
            *flags |= ISOVehicleCertificate::POLICY_FAILURE;
        }
    }

    return 0; // No error; verification failure is not an error
}
