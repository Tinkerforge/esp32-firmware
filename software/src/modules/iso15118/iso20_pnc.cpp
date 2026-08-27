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

// Plug and Charge for ISO 15118-20: the PnC authorization mode with
// contract chain and signature verification and the certificate
// installation forwarding to the CSMS (OCPP M01). See iso20.cpp for the
// base session handling.

#include "iso20.h"
#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "build.h"
#include "tools/malloc.h"

#include "gcc_warnings.h"

void ISO20::reset_pnc_session()
{
    pnc_offered = false;
    pnc_verified = false;
    cert_install_offered = false;
    // A running verify task keeps using the job, it is reused once done
    if (pnc_job != nullptr && pnc_job->result != PncVerifyResult::Running) {
        pnc_job->result = PncVerifyResult::Idle;
    }
#if MODULE_OCPP_AVAILABLE()
    // A terminal result belongs to the previous V2G session. Pending requests
    // remain owned by their OCPP callback until they complete.
    if (ocpp.get_iso15118_ev_cert_status() != Ocpp::EvCertStatus::Pending) {
        ocpp.reset_iso15118_ev_cert_response();
    }
#endif
}

// PnC is offered after a mutual TLS handshake with a live OCPP
// certificate store, the contract chain validates against its MO and
// V2G roots. The certificate installation service additionally needs
// ISO15118Ctrlr Enabled and ContractCertificateInstallationEnabled.
void ISO20::offer_pnc(struct iso20_AuthorizationSetupResType *res)
{
    pnc_offered = false;
    cert_install_offered = false;
    if (!iso15118.supports_pnc()) {
        return;
    }
#if MODULE_OCPP_AVAILABLE()
    if (ocpp.is_iso15118_enabled() && iso15118.common.tls.is_mutual_auth_session() && ocpp.is_iso15118_store_live()) {
        pnc_offered = true;
        cert_install_offered = ocpp.is_iso15118_contract_install_enabled();
    }
#endif

    if (!pnc_offered) {
        return;
    }

    res->AuthorizationServices.array[1] = iso20_authorizationType_PnC;
    res->AuthorizationServices.arrayLen = 2;
    res->CertificateInstallationService = cert_install_offered ? 1 : 0;

    // [V2G20-2565] The GenChallenge sent here must come back signed in the PnC
    // AuthorizationReq. PnC mode carries the GenChallenge, so the xs:choice
    // uses PnC mode whenever PnC is offered.
    for (size_t i = 0; i < PNC_CHALLENGE_LEN; i++) {
        gen_challenge[i] = static_cast<uint8_t>(random(256));
    }
    res->EIM_ASResAuthorizationMode_isUsed = 0;
    res->PnC_ASResAuthorizationMode_isUsed = 1;
    memcpy(res->PnC_ASResAuthorizationMode.GenChallenge.bytes, gen_challenge, PNC_CHALLENGE_LEN);
    res->PnC_ASResAuthorizationMode.GenChallenge.bytesLen = PNC_CHALLENGE_LEN;
    res->PnC_ASResAuthorizationMode.SupportedProviders_isUsed = 0;
    iso15118.trace("ISO20: Offering PnC%s", cert_install_offered ? " and certificate installation" : "");
}

// [V2G20-2564][V2G20-2565] Prepare the PnC verification job: contract
// chain, trust stores, reference digest input (EXI fragment of the
// PnC_AReqAuthorizationMode element), SignedInfo fragment and signature.
// The crypto runs in a task, the authorization loop polls the result.
bool ISO20::start_pnc_verify(const struct iso20_AuthorizationReqType *req)
{
#if MODULE_OCPP_AVAILABLE()
    const iso20_PnC_AReqAuthorizationModeType *mode = &req->PnC_AReqAuthorizationMode;
    const iso20_MessageHeaderType *header = &req->Header;

    if (!header->Signature_isUsed || header->Signature.SignedInfo.Reference.arrayLen < 1) {
        iso15118.trace("ISO20: PnC AuthorizationReq without header signature");
        return false;
    }

    if (pnc_job == nullptr) {
        pnc_job = new PncVerifyJob();
    }
    PncVerifyJob *job = pnc_job;

    job->chain = heap_alloc_array<uint8_t>(PNC_CHAIN_MAX_CERTS * PNC_CERT_MAX);
    if (job->chain == nullptr) {
        return false;
    }
    const iso20_ContractCertificateChainType *chain = &mode->ContractCertificateChain;
    if ((chain->Certificate.bytesLen == 0) || (chain->Certificate.bytesLen > PNC_CERT_MAX)) {
        return false;
    }
    memcpy(job->chain.get(), chain->Certificate.bytes, chain->Certificate.bytesLen);
    job->cert_len[0] = chain->Certificate.bytesLen;
    job->cert_count = 1;
    for (uint16_t i = 0; i < chain->SubCertificates.Certificate.arrayLen; i++) {
        const auto *sub = &chain->SubCertificates.Certificate.array[i];
        if ((job->cert_count >= PNC_CHAIN_MAX_CERTS) || (sub->bytesLen == 0) || (sub->bytesLen > PNC_CERT_MAX)) {
            return false;
        }
        memcpy(job->chain.get() + job->cert_count * PNC_CERT_MAX, sub->bytes, sub->bytesLen);
        job->cert_len[job->cert_count] = sub->bytesLen;
        job->cert_count++;
    }

    job->mo_roots = ocpp.get_iso15118_root_bundle(Ocpp::RootGroup::MO);
    job->v2g_roots = ocpp.get_iso15118_root_bundle(Ocpp::RootGroup::V2G);

    const auto *reference = &header->Signature.SignedInfo.Reference.array[0];
    if ((reference->DigestValue.bytesLen == 0) || (reference->DigestValue.bytesLen > PNC_DIGEST_MAX)) {
        return false;
    }
    memcpy(job->expected_digest, reference->DigestValue.bytes, reference->DigestValue.bytesLen);
    job->expected_digest_len = reference->DigestValue.bytesLen;

    // [V2G20-2474] sha512 with secp521r1, the algorithm URIs carry the actual choice
    job->digest_md = pnc_md_from_algorithm(reference->DigestMethod.Algorithm.characters, reference->DigestMethod.Algorithm.charactersLen);
    job->sig_md = pnc_md_from_algorithm(header->Signature.SignedInfo.SignatureMethod.Algorithm.characters,
                                        header->Signature.SignedInfo.SignatureMethod.Algorithm.charactersLen);
    if (job->digest_md == MBEDTLS_MD_NONE || job->sig_md == MBEDTLS_MD_NONE) {
        iso15118.trace("ISO20: PnC signature with unsupported algorithm");
        return false;
    }

    const auto *sig_value = &header->Signature.SignatureValue.CONTENT;
    if ((sig_value->bytesLen == 0) || (sig_value->bytesLen > PNC_SIG_MAX)) {
        return false;
    }
    memcpy(job->sig, sig_value->bytes, sig_value->bytesLen);
    job->sig_len = sig_value->bytesLen;

    static constexpr size_t ref_buf_size = 8192;
    job->ref_exi = heap_alloc_array<uint8_t>(ref_buf_size);
    auto frag = static_cast<struct iso20_exiFragment *>(calloc_psram_or_dram(1, sizeof(struct iso20_exiFragment)));
    if ((job->ref_exi == nullptr) || (frag == nullptr)) {
        free(frag);
        return false;
    }
    init_iso20_exiFragment(frag);
    frag->PnC_AReqAuthorizationMode = *mode;
    frag->PnC_AReqAuthorizationMode_isUsed = 1;
    exi_bitstream ref_exi;
    exi_bitstream_init(&ref_exi, job->ref_exi.get(), ref_buf_size, 0, nullptr);
    int ret = encode_iso20_exiFragment(&ref_exi, frag);
    free(frag);
    if (ret != 0) {
        iso15118.trace("ISO20: PnC reference fragment encode failed: %d", ret);
        return false;
    }
    job->ref_exi_len = exi_bitstream_get_length(&ref_exi);

    static constexpr size_t si_buf_size = 1024;
    job->signed_info_exi = heap_alloc_array<uint8_t>(si_buf_size);
    auto si_frag = static_cast<struct iso20_xmldsigFragment *>(calloc_psram_or_dram(1, sizeof(struct iso20_xmldsigFragment)));
    if ((job->signed_info_exi == nullptr) || (si_frag == nullptr)) {
        free(si_frag);
        return false;
    }
    init_iso20_xmldsigFragment(si_frag);
    si_frag->SignedInfo = header->Signature.SignedInfo;
    si_frag->SignedInfo_isUsed = 1;
    exi_bitstream si_exi;
    exi_bitstream_init(&si_exi, job->signed_info_exi.get(), si_buf_size, 0, nullptr);
    ret = encode_iso20_xmldsigFragment(&si_exi, si_frag);
    free(si_frag);
    if (ret != 0) {
        iso15118.trace("ISO20: PnC SignedInfo fragment encode failed: %d", ret);
        return false;
    }
    job->signed_info_exi_len = exi_bitstream_get_length(&si_exi);

    return pnc_verify_start(job);
#else
    (void)req;
    return false;
#endif
}

// PnC authorization: The GenChallenge must match, then the contract chain
// and the header signature are verified in a task while the response stays at EVSEProcessing Ongoing.
void ISO20::authorize_pnc(const struct iso20_AuthorizationReqType *req, struct iso20_AuthorizationResType *res)
{
    if (pnc_verified) {
        return;
    }

    const auto *mode = &req->PnC_AReqAuthorizationMode;
    PncVerifyResult verify = pnc_job != nullptr ? pnc_job->result : PncVerifyResult::Idle;

    if (verify == PncVerifyResult::Running) {
        res->EVSEProcessing = iso20_processingType_Ongoing;
    } else if (verify == PncVerifyResult::Idle) {
        // [V2G20-2216][V2G20-2565] Challenge must match before anything is verified
        if (mode->GenChallenge.bytesLen != PNC_CHALLENGE_LEN || memcmp(mode->GenChallenge.bytes, gen_challenge, PNC_CHALLENGE_LEN) != 0) {
            iso15118.trace("ISO20: PnC GenChallenge mismatch");
            res->ResponseCode = iso20_responseCodeType_WARNING_ChallengeInvalid;
        } else if (start_pnc_verify(req)) {
            res->EVSEProcessing = iso20_processingType_Ongoing;
        } else {
            res->ResponseCode = iso20_responseCodeType_WARNING_CertificateValidationError;
        }
    } else {
        // [V2G20-1978] Verification done, report the result with EVSEProcessing Finished.
        // On a warning the EV may retry with a different chain, so the job resets to Idle.
        switch (verify) {
            case PncVerifyResult::Ok:
                pnc_verified = true;
                iso15118.trace("ISO20: PnC contract chain and signature verified");
                break;
            case PncVerifyResult::ChainExpired:
                res->ResponseCode = iso20_responseCodeType_WARNING_CertificateExpired; // [V2G20-2212]
                break;
            case PncVerifyResult::ChainNotYetValid:
                res->ResponseCode = iso20_responseCodeType_WARNING_CertificateNotYetValid; // [V2G20-2213]
                break;
            case PncVerifyResult::ChainInvalid:
            case PncVerifyResult::SignatureInvalid:
            case PncVerifyResult::Idle:
            case PncVerifyResult::Running:
            default:
                res->ResponseCode = iso20_responseCodeType_WARNING_CertificateValidationError; // [V2G20-2215][V2G20-2564]
                break;
        }
        pnc_job->result = PncVerifyResult::Idle;
    }
}

// [V2G20-1978] Mandatory response fields filled with minimal XSD conform values
void ISO20::fill_cert_installation_dummy(struct iso20_CertificateInstallationResType *res, iso20_responseCodeType code, iso20_processingType processing)
{
    res->ResponseCode = code;
    res->EVSEProcessing = processing;
    res->CPSCertificateChain.Certificate.bytesLen = 1;
    res->CPSCertificateChain.Certificate.bytes[0] = 0;
    res->CPSCertificateChain.SubCertificates_isUsed = 0;
    res->SignedInstallationData.Id.charactersLen = 3;
    memcpy(res->SignedInstallationData.Id.characters, "id1", 3);
    res->SignedInstallationData.ContractCertificateChain.Certificate.bytesLen = 1;
    res->SignedInstallationData.ContractCertificateChain.Certificate.bytes[0] = 0;
    res->SignedInstallationData.ContractCertificateChain.SubCertificates.Certificate.arrayLen = 1;
    res->SignedInstallationData.ContractCertificateChain.SubCertificates.Certificate.array[0].bytesLen = 1;
    res->SignedInstallationData.ContractCertificateChain.SubCertificates.Certificate.array[0].bytes[0] = 0;
    res->SignedInstallationData.ECDHCurve = iso20_ecdhCurveType_SECP521;
    res->SignedInstallationData.DHPublicKey.bytesLen = 1;
    res->SignedInstallationData.DHPublicKey.bytes[0] = 0;
    res->SignedInstallationData.SECP521_EncryptedPrivateKey_isUsed = 1;
    res->SignedInstallationData.SECP521_EncryptedPrivateKey.bytesLen = 94;
    memset(res->SignedInstallationData.SECP521_EncryptedPrivateKey.bytes, 0, 94);
    res->SignedInstallationData.X448_EncryptedPrivateKey_isUsed = 0;
    res->SignedInstallationData.TPM_EncryptedPrivateKey_isUsed = 0;
    res->RemainingContractCertificateChains = 0;
}

void ISO20::handle_certificate_installation_req()
{
    // M01: the raw EXI request is forwarded to the CSMS via
    // Get15118EVCertificate, the returned EXI CertificateInstallationRes
    // is sent to the EV with RemainingContractCertificateChains patched
    // to remainingContracts (M01.FR.05). While the CSMS response is
    // pending the SECC answers with EVSEProcessing Ongoing.
    iso20_CertificateInstallationResType *res = &iso20DocEnc->CertificateInstallationRes;

    iso20DocEnc->CertificateInstallationRes_isUsed = 1;
    prepare_header(&res->Header);

#if MODULE_OCPP_AVAILABLE()
    iso20_CertificateInstallationReqType *req = &iso20DocDec->CertificateInstallationReq;

    if (!cert_install_offered) {
        iso15118.trace("ISO20: CertificateInstallationReq but service not offered");
        fill_cert_installation_dummy(res, iso20_responseCodeType_FAILED, iso20_processingType_Finished);
    } else {
        switch (ocpp.get_iso15118_ev_cert_status()) {
            case Ocpp::EvCertStatus::Pending:
                fill_cert_installation_dummy(res, iso20_responseCodeType_OK, iso20_processingType_Ongoing);
                break;

            case Ocpp::EvCertStatus::Accepted: {
                std::unique_ptr<uint8_t[]> exi;
                size_t exi_len = 0;
                int32_t remaining = 0;
                ocpp.take_iso15118_ev_cert_response(&exi, &exi_len, &remaining);

                exi_bitstream bs;
                exi_bitstream_init(&bs, exi.get(), exi_len, 0, nullptr);
                memset(iso20DocEnc, 0, sizeof(struct iso20_exiDocument));
                if (decode_iso20_exiDocument(&bs, iso20DocEnc) != 0 || !iso20DocEnc->CertificateInstallationRes_isUsed) {
                    iso15118.trace("ISO20: CSMS EXI response is not a CertificateInstallationRes");
                    memset(iso20DocEnc, 0, sizeof(struct iso20_exiDocument));
                    iso20DocEnc->CertificateInstallationRes_isUsed = 1;
                    prepare_header(&res->Header);
                    fill_cert_installation_dummy(res, iso20_responseCodeType_WARNING_NoCertificateAvailable, iso20_processingType_Finished);
                } else {
                    // M01.FR.05
                    res->RemainingContractCertificateChains = remaining < 0 ? 0 : (remaining > 255 ? 255 : static_cast<uint8_t>(remaining));
                    iso15118.trace("ISO20: Forwarding CertificateInstallationRes, %ld chains remaining", static_cast<long>(remaining));
                }
                break;
            }

            case Ocpp::EvCertStatus::Failed:
                iso15118.trace("ISO20: Get15118EVCertificate failed");
                fill_cert_installation_dummy(res, iso20_responseCodeType_WARNING_NoCertificateAvailable, iso20_processingType_Finished);
                break;

            case Ocpp::EvCertStatus::Idle:
            default: {
                size_t rx_len = 0;
                const uint8_t *rx = iso15118.common.get_rx_exi(&rx_len);
                // M01.FR.01/03: action Install, maximumContractCertificateChains from the EV request
                if (rx != nullptr && ocpp.request_iso15118_ev_certificate(true, false, rx, rx_len, req->MaximumContractCertificateChains)) {
                    fill_cert_installation_dummy(res, iso20_responseCodeType_OK, iso20_processingType_Ongoing);
                } else {
                    iso15118.trace("ISO20: Get15118EVCertificate request refused");
                    fill_cert_installation_dummy(res, iso20_responseCodeType_WARNING_NoCertificateAvailable, iso20_processingType_Finished);
                }
                break;
            }
        }
    }
#else
    fill_cert_installation_dummy(res, iso20_responseCodeType_FAILED, iso20_processingType_Finished);
#endif

    iso15118.common.send_exi(Common::ExiType::Iso20);
    state = ISO20State::CertificateInstallation;
}
