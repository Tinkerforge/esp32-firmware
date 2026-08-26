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

// Plug and Charge for ISO 15118-2: Contract payment, certificate
// installation/update forwarding to the CSMS (OCPP M01/M02) and the
// contract chain and signature checks. See iso2.cpp for the base session
// handling.

#include "iso2.h"
#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "build.h"
#include "tools/malloc.h"

#include "gcc_warnings.h"

static void fill_cert_chain_dummy(struct iso2_CertificateChainType *c)
{
    c->Id_isUsed = 0;
    c->Certificate.bytesLen = 0;
    c->SubCertificates_isUsed = 0;
}

// Mandatory fields of a failed CertificateInstallationRes/CertificateUpdateRes
// filled with minimal schema conform values
void ISO2::fill_cert_installation_res_dummy(struct iso2_CertificateInstallationResType *res, iso2_responseCodeType rc)
{
    res->ResponseCode = rc;
    fill_cert_chain_dummy(&res->SAProvisioningCertificateChain);
    fill_cert_chain_dummy(&res->ContractSignatureCertChain);
    res->ContractSignatureEncryptedPrivateKey.Id.charactersLen = 3;
    memcpy(res->ContractSignatureEncryptedPrivateKey.Id.characters, "id1", 3);
    res->ContractSignatureEncryptedPrivateKey.CONTENT.bytesLen = 0;
    res->DHpublickey.Id.charactersLen = 3;
    memcpy(res->DHpublickey.Id.characters, "id2", 3);
    res->DHpublickey.CONTENT.bytesLen = 0;
    res->eMAID.Id.charactersLen = 3;
    memcpy(res->eMAID.Id.characters, "id3", 3);
    res->eMAID.CONTENT.charactersLen = 0;
}

void ISO2::fill_cert_update_res_dummy(struct iso2_CertificateUpdateResType *res, iso2_responseCodeType rc)
{
    res->ResponseCode = rc;
    fill_cert_chain_dummy(&res->SAProvisioningCertificateChain);
    fill_cert_chain_dummy(&res->ContractSignatureCertChain);
    res->ContractSignatureEncryptedPrivateKey.Id.charactersLen = 3;
    memcpy(res->ContractSignatureEncryptedPrivateKey.Id.characters, "id1", 3);
    res->ContractSignatureEncryptedPrivateKey.CONTENT.bytesLen = 0;
    res->DHpublickey.Id.charactersLen = 3;
    memcpy(res->DHpublickey.Id.characters, "id2", 3);
    res->DHpublickey.CONTENT.bytesLen = 0;
    res->eMAID.Id.charactersLen = 3;
    memcpy(res->eMAID.Id.characters, "id3", 3);
    res->eMAID.CONTENT.charactersLen = 0;
    res->RetryCounter_isUsed = 0;
}

void ISO2::reset_pnc_session()
{
    pnc_offered = false;
    cert_service_offered = false;
    contract_selected = false;
    contract_validated = false;
    contract_leaf_len = 0;
}

// Contract payment and the certificate service are only offered over
// TLS with a live OCPP certificate store, the contract chain validates against its MO and V2G roots
void ISO2::offer_pnc(struct iso2_ServiceDiscoveryResType *res)
{
    pnc_offered = false;
    cert_service_offered = false;
#if MODULE_OCPP_AVAILABLE()
    if (iso15118.common.tls.is_session_active() && ocpp.is_iso15118_store_live()) {
        pnc_offered = true;
        cert_service_offered = ocpp.is_iso15118_contract_install_enabled();
    }
#endif

    if (pnc_offered) {
        res->PaymentOptionList.PaymentOption.array[1] = iso2_paymentOptionType_Contract;
        res->PaymentOptionList.PaymentOption.arrayLen = 2;
    }

    if (cert_service_offered) {
        // ContractCertificate VAS (Table 105), details in ServiceDetailRes
        res->ServiceList_isUsed = 1;
        res->ServiceList.Service.array[0].ServiceID = V2G_SERVICE_ID_CERTIFICATE;
        res->ServiceList.Service.array[0].ServiceCategory = iso2_serviceCategoryType_ContractCertificate;
        res->ServiceList.Service.array[0].ServiceName_isUsed = 0;
        res->ServiceList.Service.array[0].ServiceScope_isUsed = 0;
        res->ServiceList.Service.array[0].FreeService = 1;
        res->ServiceList.Service.arrayLen = 1;
    }
}

void ISO2::handle_service_detail_req()
{
    // [V2G2-543] FAILED_ServiceIDInvalid if the ServiceID was not offered.
    // Table 106: certificate service parameter sets, 1 = Installation, 2 = Update
    iso2_ServiceDetailReqType *req = &iso2DocDec->V2G_Message.Body.ServiceDetailReq;
    iso2_ServiceDetailResType *res = &iso2DocEnc->V2G_Message.Body.ServiceDetailRes;

    iso2DocEnc->V2G_Message.Body.ServiceDetailRes_isUsed = 1;
    res->ServiceID = req->ServiceID;
    res->ServiceParameterList_isUsed = 0;

    if (req->ServiceID == V2G_SERVICE_ID_CERTIFICATE && cert_service_offered) {
        res->ResponseCode = iso2_responseCodeType_OK;
        res->ServiceParameterList_isUsed = 1;

        auto fill_set = [](iso2_ParameterSetType *set, int16_t set_id, const char *value) {
            set->ParameterSetID = set_id;
            auto *param = &set->Parameter.array[0];
            memcpy(param->Name.characters, "Service", 7);
            param->Name.charactersLen = 7;
            param->boolValue_isUsed = 0;
            param->byteValue_isUsed = 0;
            param->shortValue_isUsed = 0;
            param->intValue_isUsed = 0;
            param->physicalValue_isUsed = 0;
            param->stringValue_isUsed = 1;
            size_t len = strlen(value);
            memcpy(param->stringValue.characters, value, len);
            param->stringValue.charactersLen = static_cast<uint16_t>(len);
            set->Parameter.arrayLen = 1;
        };
        fill_set(&res->ServiceParameterList.ParameterSet.array[0], 1, "Installation");
        fill_set(&res->ServiceParameterList.ParameterSet.array[1], 2, "Update");
        res->ServiceParameterList.ParameterSet.arrayLen = 2;
    } else {
        res->ResponseCode = iso2_responseCodeType_FAILED_ServiceIDInvalid;
    }

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = ISO2State::ServiceDetail;
}

void ISO2::handle_certificate_installation_req()
{
    start_cert_forward(false);
}

void ISO2::handle_certificate_update_req()
{
    start_cert_forward(true);
}

// M01.FR.01/M02: the raw EXI request is forwarded to the CSMS via
// Get15118EVCertificate (action Install or Update). ISO 15118-2 has no
// Ongoing mechanism here, the response is held back until the CSMS
// result arrives (EV message timeout 5 s per Table 109).
void ISO2::start_cert_forward(bool update)
{
#if MODULE_OCPP_AVAILABLE()
    if (cert_service_offered) {
        size_t rx_len = 0;
        const uint8_t *rx = iso15118.common.get_rx_exi(&rx_len);
        if (rx != nullptr && ocpp.request_iso15118_ev_certificate(false, update, rx, rx_len, -1)) {
            state = ISO2State::CertificateInstallation;
            poll_cert_forward(update, iso15118.common.get_active_socket(), 0);
            return;
        }
        iso15118.trace("ISO2: Get15118EVCertificate request refused");
    } else {
        iso15118.trace("ISO2: Certificate%sReq but service not offered", update ? "Update" : "Installation");
    }
#endif
    send_cert_forward_result(update, true);
}

void ISO2::poll_cert_forward(bool update, int socket, uint8_t attempts)
{
#if MODULE_OCPP_AVAILABLE()
    if ((iso15118.common.get_active_socket() != socket) || (iso15118.common.exi_in_use != Common::ExiType::Iso2)) {
        return;
    }

    if ((ocpp.get_iso15118_ev_cert_status() == Ocpp::EvCertStatus::Pending) && (attempts < 40)) {
        task_scheduler.scheduleOnce([this, update, socket, attempts]() {
            poll_cert_forward(update, socket, attempts + 1);
        }, 100_ms);
        return;
    }

    send_cert_forward_result(update, ocpp.get_iso15118_ev_cert_status() != Ocpp::EvCertStatus::Accepted);
#endif
}

void ISO2::send_cert_forward_result(bool update, bool failed)
{
#if MODULE_OCPP_AVAILABLE()
    if (!failed) {
        std::unique_ptr<uint8_t[]> exi;
        size_t exi_len = 0;
        int32_t remaining = 0;
        if (ocpp.take_iso15118_ev_cert_response(&exi, &exi_len, &remaining)) {
            iso15118.trace("ISO2: Forwarding Certificate%sRes", update ? "Update" : "Installation");
            iso15118.common.send_exi_raw(exi.get(), exi_len, Common::ExiType::Iso2);
            return;
        }
        failed = true;
    }
#endif

    iso15118.trace("ISO2: Certificate%s failed", update ? "Update" : "Installation");
    auto &body = iso2DocEnc->V2G_Message.Body;
    memset(&body, 0, sizeof(body));
    if (update) {
        body.CertificateUpdateRes_isUsed = 1;
        fill_cert_update_res_dummy(&body.CertificateUpdateRes, iso2_responseCodeType_FAILED_NoCertificateAvailable);
    } else {
        body.CertificateInstallationRes_isUsed = 1;
        fill_cert_installation_res_dummy(&body.CertificateInstallationRes, iso2_responseCodeType_FAILED_NoCertificateAvailable);
    }
    iso15118.common.send_exi(Common::ExiType::Iso2);
}

void ISO2::handle_payment_details_req()
{
    // [V2G2-475 ff] Validate the contract chain against the MO and V2G
    // trust stores, store the leaf for the AuthorizationReq signature
    // check and respond with a fresh GenChallenge.
    iso2_PaymentDetailsReqType *req = &iso2DocDec->V2G_Message.Body.PaymentDetailsReq;
    iso2_PaymentDetailsResType *res = &iso2DocEnc->V2G_Message.Body.PaymentDetailsRes;

    iso2DocEnc->V2G_Message.Body.PaymentDetailsRes_isUsed = 1;

    contract_validated = false;
    for (size_t i = 0; i < PNC_CHALLENGE_LEN; i++) {
        gen_challenge[i] = static_cast<uint8_t>(random(256));
    }
    res->GenChallenge.bytesLen = PNC_CHALLENGE_LEN;
    memcpy(res->GenChallenge.bytes, gen_challenge, PNC_CHALLENGE_LEN);

    timeval now;
    if (!rtc.clock_synced(&now)) {
        now.tv_sec = 0;
    }
    res->EVSETimeStamp = now.tv_sec;

#if MODULE_OCPP_AVAILABLE()
    if (!contract_selected) {
        res->ResponseCode = iso2_responseCodeType_FAILED_SequenceError;
    } else {
        const iso2_CertificateChainType *chain = &req->ContractSignatureCertChain;
        auto chain_buf = heap_alloc_array<uint8_t>(PNC_CHAIN_MAX_CERTS * PNC_CERT_MAX);
        size_t cert_len[PNC_CHAIN_MAX_CERTS] = {0};
        size_t cert_count = 0;
        bool copy_ok = (chain_buf != nullptr) && (chain->Certificate.bytesLen > 0) && (chain->Certificate.bytesLen <= ISO2_CERT_MAX);
        if (copy_ok) {
            memcpy(chain_buf.get(), chain->Certificate.bytes, chain->Certificate.bytesLen);
            cert_len[0] = chain->Certificate.bytesLen;
            cert_count = 1;
            if (chain->SubCertificates_isUsed) {
                for (uint16_t i = 0; i < chain->SubCertificates.Certificate.arrayLen; i++) {
                    const auto *sub = &chain->SubCertificates.Certificate.array[i];
                    if ((cert_count >= PNC_CHAIN_MAX_CERTS) || (sub->bytesLen == 0) || (sub->bytesLen > ISO2_CERT_MAX)) {
                        copy_ok = false;
                        break;
                    }
                    memcpy(chain_buf.get() + cert_count * PNC_CERT_MAX, sub->bytes, sub->bytesLen);
                    cert_len[cert_count] = sub->bytesLen;
                    cert_count++;
                }
            }
        }

        if (!copy_ok) {
            res->ResponseCode = iso2_responseCodeType_FAILED_CertChainError;
        } else {
            auto mo_roots = ocpp.get_iso15118_root_bundle(Ocpp::RootGroup::MO);
            auto v2g_roots = ocpp.get_iso15118_root_bundle(Ocpp::RootGroup::V2G);
            char emaid_cn[64];
            PncVerifyResult result = pnc_validate_chain(chain_buf.get(), cert_len, cert_count, mo_roots.get(), v2g_roots.get(), emaid_cn, sizeof(emaid_cn));
            switch (result) {
                case PncVerifyResult::Ok:
                    memcpy(contract_leaf, chain->Certificate.bytes, chain->Certificate.bytesLen);
                    contract_leaf_len = chain->Certificate.bytesLen;
                    contract_validated = true;
                    res->ResponseCode = iso2_responseCodeType_OK;
                    iso15118.trace("ISO2: Contract chain validated, eMAID '%.*s', leaf CN '%s'",
                                   req->eMAID.charactersLen, req->eMAID.characters, emaid_cn);
                    break;
                case PncVerifyResult::ChainExpired:
                    res->ResponseCode = iso2_responseCodeType_FAILED_CertificateExpired; // [V2G2-468]
                    break;
                case PncVerifyResult::ChainNotYetValid:
                case PncVerifyResult::ChainInvalid:
                case PncVerifyResult::SignatureInvalid:
                case PncVerifyResult::Idle:
                case PncVerifyResult::Running:
                default:
                    res->ResponseCode = iso2_responseCodeType_FAILED_CertChainError; // [V2G2-469]
                    break;
            }
        }
    }
#else
    res->ResponseCode = iso2_responseCodeType_FAILED;
#endif

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = ISO2State::PaymentDetails;
}

// [V2G2-461] Verify the header signature of the AuthorizationReq against
// the contract leaf from PaymentDetailsReq: The reference digest covers
// the EXI fragment of the AuthorizationReq element, the signature covers
// the EXI fragment of SignedInfo.
bool ISO2::verify_authorization_signature(const struct iso2_AuthorizationReqType *req, const struct iso2_MessageHeaderType *header)
{
    if (!header->Signature_isUsed || (header->Signature.SignedInfo.Reference.arrayLen < 1)) {
        iso15118.trace("ISO2: PnC AuthorizationReq without header signature");
        return false;
    }
    const auto *reference = &header->Signature.SignedInfo.Reference.array[0];
    const auto *sig_value = &header->Signature.SignatureValue.CONTENT;
    if ((reference->DigestValue.bytesLen == 0) || (sig_value->bytesLen == 0) || (sig_value->bytesLen > PNC_SIG_MAX)) {
        return false;
    }

    static constexpr size_t ref_buf_size = 512;
    auto ref_buf = heap_alloc_array<uint8_t>(ref_buf_size);
    auto frag = static_cast<struct iso2_exiFragment *>(calloc_psram_or_dram(1, sizeof(struct iso2_exiFragment)));
    if (ref_buf == nullptr || frag == nullptr) {
        free(frag);
        return false;
    }
    init_iso2_exiFragment(frag);
    frag->AuthorizationReq = *req;
    frag->AuthorizationReq_isUsed = 1;
    exi_bitstream ref_exi;
    exi_bitstream_init(&ref_exi, ref_buf.get(), ref_buf_size, 0, nullptr);
    int ret = encode_iso2_exiFragment(&ref_exi, frag);
    free(frag);
    if (ret != 0) {
        iso15118.trace("ISO2: PnC reference fragment encode failed: %d", ret);
        return false;
    }

    if (!pnc_digest_matches(MBEDTLS_MD_SHA256, ref_buf.get(), exi_bitstream_get_length(&ref_exi),
                            reference->DigestValue.bytes, reference->DigestValue.bytesLen)) {
        iso15118.trace("ISO2: PnC reference digest mismatch");
        return false;
    }

    static constexpr size_t si_buf_size = 1024;
    auto si_buf = heap_alloc_array<uint8_t>(si_buf_size);
    auto si_frag = static_cast<struct iso2_xmldsigFragment *>(calloc_psram_or_dram(1, sizeof(struct iso2_xmldsigFragment)));
    if (si_buf == nullptr || si_frag == nullptr) {
        free(si_frag);
        return false;
    }
    init_iso2_xmldsigFragment(si_frag);
    si_frag->SignedInfo = header->Signature.SignedInfo;
    si_frag->SignedInfo_isUsed = 1;
    exi_bitstream si_exi;
    exi_bitstream_init(&si_exi, si_buf.get(), si_buf_size, 0, nullptr);
    ret = encode_iso2_xmldsigFragment(&si_exi, si_frag);
    free(si_frag);
    if (ret != 0) {
        iso15118.trace("ISO2: PnC SignedInfo fragment encode failed: %d", ret);
        return false;
    }

    return pnc_verify_signature(contract_leaf, contract_leaf_len, MBEDTLS_MD_SHA256,
                                si_buf.get(), exi_bitstream_get_length(&si_exi),
                                sig_value->bytes, sig_value->bytesLen);
}

// PnC authorization after Contract payment was selected: GenChallenge
// from PaymentDetailsRes must come back, then the header signature is
// verified against the validated contract leaf.
void ISO2::authorize_pnc(const struct iso2_AuthorizationReqType *req, struct iso2_AuthorizationResType *res)
{
    if (!contract_validated) {
        res->ResponseCode = iso2_responseCodeType_FAILED_SequenceError;
    } else if (!req->GenChallenge_isUsed || (req->GenChallenge.bytesLen != PNC_CHALLENGE_LEN) || (memcmp(req->GenChallenge.bytes, gen_challenge, PNC_CHALLENGE_LEN) != 0)) {
        // [V2G2-475]
        iso15118.trace("ISO2: PnC GenChallenge mismatch");
        res->ResponseCode = iso2_responseCodeType_FAILED_ChallengeInvalid;
    } else if (!verify_authorization_signature(req, &iso2DocDec->V2G_Message.Header)) {
        // [V2G2-461]
        res->ResponseCode = iso2_responseCodeType_FAILED_SignatureError;
    } else {
        iso15118.trace("ISO2: PnC authorization signature verified");
    }
}
