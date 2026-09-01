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
#include "options.h"
#if OPTIONS_ISO15118_DEV_CERTS_ENABLED()
#include "../dev_certs.h"
#endif

#include "event_log_prefix.h"
#include "../generated/module_dependencies.h"
#include "tools/malloc.h"

#include <string.h>
#include <memory>

namespace {

uint8_t *copy_pem(const char *pem, size_t *len_out)
{
    size_t len = strlen(pem) + 1;
    uint8_t *buf = static_cast<uint8_t*>(calloc_psram_or_dram(len, 1));
    if (buf != nullptr) {
        memcpy(buf, pem, len);
        *len_out = len;
    }
    return buf;
}

} // namespace

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
    certificate_store_live = store_live;
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
