/* esp32-firmware
 * Copyright (C) 2025 Olaf Lüke <olaf@tinkerforge.com>
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

// IPv6/TCP/DIN-SPEC-70121

#include "din70121.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools/malloc.h"

#include "cbv2g/exi_v2gtp.h"
#include "cbv2g/app_handshake/appHand_Decoder.h"
#include "cbv2g/app_handshake/appHand_Encoder.h"
#include "cbv2g/din/din_msgDefDecoder.h"
#include "cbv2g/din/din_msgDefEncoder.h"
#include "cbv2g/common/exi_bitstream.h"

void DIN70121::pre_setup()
{
    api_state = Config::Object({
        {"state", Config::Uint8(0)},
        {"session_id", Config::Tuple(4, Config::Uint8(0))},
        {"evcc_id", Config::Array({}, Config::get_prototype_uint8_0(), 0, 8, Config::type_id<Config::ConfUint>())},
        {"soc", Config::Int8(0)},
        {"ev_ready", Config::Int32(0)},
        {"ev_cabin_conditioning", Config::Int32(0)},
        {"ev_cabin_conditioning_is_used", Config::Bool(false)},
        {"ev_ress_conditioning", Config::Int32(0)},
        {"ev_ress_conditioning_is_used", Config::Bool(false)},
        {"ev_error_code", Config::Uint8(0)},
        {"ev_max_current_limit_val", Config::Int16(0)},
        {"ev_max_current_limit_mul", Config::Int8(0)},
        {"ev_max_power_limit_val", Config::Int16(0)},
        {"ev_max_power_limit_mul", Config::Int8(0)},
        {"ev_max_power_limit_is_used", Config::Bool(false)},
        {"ev_max_voltage_limit_val", Config::Int16(0)},
        {"ev_max_voltage_limit_mul", Config::Int8(0)},
        {"ev_energy_capacity_val", Config::Int16(0)},
        {"ev_energy_capacity_mul", Config::Int8(0)},
        {"ev_energy_capacity_is_used", Config::Bool(0)},
        {"ev_energy_request_val", Config::Int16(0)},
        {"ev_energy_request_mul", Config::Int8(0)},
        {"ev_energy_request_is_used", Config::Bool(false)},
        {"full_soc", Config::Int8(0)},
        {"full_soc_is_used", Config::Bool(false)},
        {"bulk_soc", Config::Int8(0)},
        {"bulk_soc_is_used", Config::Bool(false)}
    });
}

void DIN70121::handle_bitstream(exi_bitstream *exi)
{
    // Increment state on first call
    if (state == 0) {
        state = 1;
    }

    // We alloc the din buffers the very first time they are used.
    // This way it is not allocated if ISO15118 is not used.
    // If it is used once we can assume that it will be used all the time, so it stays allocated.
    if (dinDocDec == nullptr) {
        dinDocDec = static_cast<struct din_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct din_exiDocument)));
    }
    if (dinDocEnc == nullptr) {
        dinDocEnc = static_cast<struct din_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct din_exiDocument)));
    }
    memset(dinDocDec, 0, sizeof(struct din_exiDocument));
    memset(dinDocEnc, 0, sizeof(struct din_exiDocument));

    cancel_sequence_timeout(next_timeout);

    int ret = decode_din_exiDocument(exi, dinDocDec);
    logger.printfln("DIN70121: decode_din_exiDocument: %d", ret);

    dispatch_messages();

    trace_request_response();

    api_state.get("state")->updateUint(state);

    // DIN TS 70121:2024-11 [V2G-DC-443]: The SECC shall stop waiting for a request message
    // when V2G_SECC_Sequence_Timer >= V2G_SECC_Sequence_Timeout and no request was received.
    schedule_sequence_timeout(next_timeout, DIN70121_SECC_SEQUENCE_TIMEOUT, "DIN70121");
}

void DIN70121::dispatch_messages()
{
    auto &body = dinDocDec->V2G_Message.Body;

    // SessionSetupReq - no session validation needed (session is established here)
    V2G_DISPATCH("DIN70121", body, SessionSetupReq, handle_session_setup_req);
    if (body.SessionSetupReq_isUsed) return;

    // [V2G-DC-xxx] All messages after SessionSetup require session ID validation.
    // If the SessionID received does not match the previously communicated SessionID,
    // the SECC shall respond with FAILED_UnknownSession.
    if (!validate_session_id(dinDocDec->V2G_Message.Header.SessionID.bytes,
                             dinDocDec->V2G_Message.Header.SessionID.bytesLen,
                             iso15118.common.session_id,
                             SESSION_ID_LENGTH)) {
        logger.printfln("DIN70121: Session ID mismatch, sending FAILED_UnknownSession");
        send_failed_unknown_session();
        return;
    }

    // Implemented message handlers (session already validated)
    V2G_DISPATCH("DIN70121", body, ServiceDiscoveryReq,         handle_service_discovery_req);
    V2G_DISPATCH("DIN70121", body, ServicePaymentSelectionReq,  handle_service_payment_selection_req);
    V2G_DISPATCH("DIN70121", body, ContractAuthenticationReq,   handle_contract_authentication_req);
    V2G_DISPATCH("DIN70121", body, ChargeParameterDiscoveryReq, handle_charge_parameter_discovery_req);
    V2G_DISPATCH("DIN70121", body, SessionStopReq,              handle_session_stop_req);

    // Not yet implemented

    // All of these can only be used in the context of DC charging (we only use DIN70121 to obtain the current SoC).
    // We will not support them.
    V2G_NOT_IMPL("DIN70121", body, ServiceDetailReq);
    V2G_NOT_IMPL("DIN70121", body, PaymentDetailsReq);
    V2G_NOT_IMPL("DIN70121", body, PowerDeliveryReq);
    V2G_NOT_IMPL("DIN70121", body, ChargingStatusReq);
    V2G_NOT_IMPL("DIN70121", body, MeteringReceiptReq);
    V2G_NOT_IMPL("DIN70121", body, CertificateUpdateReq);
    V2G_NOT_IMPL("DIN70121", body, CertificateInstallationReq);
    V2G_NOT_IMPL("DIN70121", body, CableCheckReq);
    V2G_NOT_IMPL("DIN70121", body, PreChargeReq);
    V2G_NOT_IMPL("DIN70121", body, CurrentDemandReq);
    V2G_NOT_IMPL("DIN70121", body, WeldingDetectionReq);
}

void DIN70121::send_failed_unknown_session()
{
    auto &body_dec = dinDocDec->V2G_Message.Body;
    auto &body_enc = dinDocEnc->V2G_Message.Body;

    // Determine which message type was received and send the appropriate error response
    if (false
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, ServiceDiscovery,         din_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, ServicePaymentSelection,  din_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, ContractAuthentication,   din_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, ChargeParameterDiscovery, din_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, SessionStop,              din_responseCodeType_FAILED_UnknownSession))
    ) {
        iso15118.common.send_exi(Common::ExiType::Din);
        return;
    }

    // Unknown message type - this shouldn't happen but log it
    logger.printfln("DIN70121: Unknown message type for FAILED_UnknownSession");
}

void DIN70121::handle_session_setup_req()
{
    din_SessionSetupReqType *req = &dinDocDec->V2G_Message.Body.SessionSetupReq;
    din_SessionSetupResType *res = &dinDocEnc->V2G_Message.Body.SessionSetupRes;

    api_state.get("evcc_id")->removeAll();
    for (uint16_t i = 0; i < std::min(static_cast<uint16_t>(sizeof(req->EVCCID.bytes)), req->EVCCID.bytesLen); i++) {
        api_state.get("evcc_id")->add()->updateUint(req->EVCCID.bytes[i]);
    }

    // [V2G-DC-993] When receiving the SessionSetupReq with the parameter SessionID equal to zero (0), the
    //              SECC shall generate a new (not stored) SessionID value different from zero (0) and return
    //              this value in the SessionSetupRes message header.
    // [V2G-DC-872] If the SECC receives a SessionSetupReq including a SessionID value which is not equal
    //              to zero (0) and not equal to the SessionID value stored from the preceding V2G commu-
    //              nication session, it shall send a SessionID value in the SessionSetupRes message that is
    //              unequal to "0" and unequal to the SessionID value stored from the preceding V2G com-
    //              munication session and indicate the new V2G communication session with the Respon-
    //              seCode set to "OK_NewSessionEstablished" (refer also to [V2G-DC-393] for applicability
    //              of this response code).
    // [V2G-DC-934] If the SessionID is checked during the V2G communication session, the EVCC shall first
    //              compare the length and then the actual value.
    SessionIdResult result = check_session_id(
        dinDocDec->V2G_Message.Header.SessionID.bytes,
        dinDocDec->V2G_Message.Header.SessionID.bytesLen,
        iso15118.common.session_id,
        SESSION_ID_LENGTH
    );

    if (result == SessionIdResult::NewSession) {
        res->ResponseCode = din_responseCodeType_OK_NewSessionEstablished;
    } else {
        res->ResponseCode = din_responseCodeType_OK_OldSessionJoined;
    }

    for (uint16_t i = 0; i < SESSION_ID_LENGTH; i++) {
        api_state.get("session_id")->get(i)->updateUint(iso15118.common.session_id[i]);
    }

    dinDocEnc->V2G_Message.Body.SessionSetupRes_isUsed = 1;

    // EVSEID needs to be according to DIN SPEC 91286, it can be 0x00 if not available
    // Example EVSEID according to DIN SPEC 91286: +49*810*000*438

    // [V2G-DC-876] If the SECC wants to send zero, it shall send the EVSEID as single hexBinary value: ʺ0x00ʺ.
    res->EVSEID.bytes[0] = 0;
    res->EVSEID.bytesLen = 1;

    // [V2G-DC-878] An EVCC shall not expect a transmitted timestamp to be correct or a timestamp to be sent at all.
    // An EVSE may not send the timestamp.
    res->DateTimeNow_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Din);
    state = 2;
}

void DIN70121::handle_service_discovery_req()
{
    din_ServiceDiscoveryResType *res = &dinDocEnc->V2G_Message.Body.ServiceDiscoveryRes;

    dinDocEnc->V2G_Message.Body.ServiceDiscoveryRes_isUsed = 1;
    res->ResponseCode = din_responseCodeType_OK;

    // One payment option: EVSE handles payment
    res->PaymentOptions.PaymentOption.array[0] = din_paymentOptionType_ExternalPayment;
    res->PaymentOptions.PaymentOption.arrayLen = 1;

    // One service available: EV charging
    res->ChargeService.ServiceTag.ServiceID = 1;
    res->ChargeService.ServiceTag.ServiceCategory = din_serviceCategoryType_EVCharging;

    // EV can use offered service without payment
    res->ChargeService.FreeService = 1;

    // DC charging with CCS connector (normal DC charging)
    res->ChargeService.EnergyTransferType = din_EVSESupportedEnergyTransferType_DC_extended;

    // [V2G-DC-622] In the scope of this document, the optional element ServiceScope shall not be used.
    res->ChargeService.ServiceTag.ServiceScope_isUsed = 0;

    // [V2G-DC-549] In the scope of this document, the element “ServiceList” shall not be used.
    res->ServiceList_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Din);
    state = 3;
}

void DIN70121::handle_service_payment_selection_req()
{
    din_ServicePaymentSelectionReqType *req = &dinDocDec->V2G_Message.Body.ServicePaymentSelectionReq;
    din_ServicePaymentSelectionResType *res = &dinDocEnc->V2G_Message.Body.ServicePaymentSelectionRes;

    if (req->SelectedPaymentOption == din_paymentOptionType_ExternalPayment) {
        dinDocEnc->V2G_Message.Body.ServicePaymentSelectionRes_isUsed = 1;
        res->ResponseCode = din_responseCodeType_OK;

        iso15118.common.send_exi(Common::ExiType::Din);
        state = 4;
    }
}

void DIN70121::handle_contract_authentication_req()
{
    din_ContractAuthenticationResType *res = &dinDocEnc->V2G_Message.Body.ContractAuthenticationRes;

    // [V2G-DC-550] In the scope of this document, the element “GenChallenge” shall not be used.
    // [V2G-DC-545] In the scope of this document, the element “Id” shall not be used.
    // -> None of the request parameters are used in DIN SPEC 70121.

    dinDocEnc->V2G_Message.Body.ContractAuthenticationRes_isUsed = 1;

    // Set Authorisation to Finished here.
    // We want to go on ChargeParameteryDiscovery to read the SoC and then use Ongoing.
    res->ResponseCode = din_responseCodeType_OK;
    res->EVSEProcessing = din_EVSEProcessingType_Finished;
    iso15118.common.send_exi(Common::ExiType::Din);

    state = 5;
}

void DIN70121::handle_charge_parameter_discovery_req()
{
    din_ChargeParameterDiscoveryReqType* req = &dinDocDec->V2G_Message.Body.ChargeParameterDiscoveryReq;
    din_ChargeParameterDiscoveryResType* res = &dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes;

    api_state.get("soc")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
    api_state.get("ev_ready")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVReady);
    api_state.get("ev_cabin_conditioning")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVCabinConditioning);
    api_state.get("ev_cabin_conditioning_is_used")->updateBool(req->DC_EVChargeParameter.DC_EVStatus.EVCabinConditioning_isUsed);
    api_state.get("ev_ress_conditioning")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVRESSConditioning);
    api_state.get("ev_ress_conditioning_is_used")->updateBool(req->DC_EVChargeParameter.DC_EVStatus.EVRESSConditioning_isUsed);
    api_state.get("ev_error_code")->updateUint(req->DC_EVChargeParameter.DC_EVStatus.EVErrorCode);
    api_state.get("ev_max_current_limit_val")->updateInt(req->DC_EVChargeParameter.EVMaximumCurrentLimit.Value);
    api_state.get("ev_max_current_limit_mul")->updateInt(req->DC_EVChargeParameter.EVMaximumCurrentLimit.Multiplier);
    api_state.get("ev_max_power_limit_val")->updateInt(req->DC_EVChargeParameter.EVMaximumPowerLimit.Value);
    api_state.get("ev_max_power_limit_mul")->updateInt(req->DC_EVChargeParameter.EVMaximumPowerLimit.Multiplier);
    api_state.get("ev_max_power_limit_is_used")->updateBool(req->DC_EVChargeParameter.EVMaximumPowerLimit_isUsed);
    api_state.get("ev_max_voltage_limit_val")->updateInt(req->DC_EVChargeParameter.EVMaximumVoltageLimit.Value);
    api_state.get("ev_max_voltage_limit_mul")->updateInt(req->DC_EVChargeParameter.EVMaximumVoltageLimit.Multiplier);
    api_state.get("ev_energy_capacity_val")->updateInt(req->DC_EVChargeParameter.EVEnergyCapacity.Value);
    api_state.get("ev_energy_capacity_mul")->updateInt(req->DC_EVChargeParameter.EVEnergyCapacity.Multiplier);
    api_state.get("ev_energy_capacity_is_used")->updateBool(req->DC_EVChargeParameter.EVEnergyCapacity_isUsed);
    api_state.get("ev_energy_request_val")->updateInt(req->DC_EVChargeParameter.EVEnergyRequest.Value);
    api_state.get("ev_energy_request_mul")->updateInt(req->DC_EVChargeParameter.EVEnergyRequest.Multiplier);
    api_state.get("ev_energy_request_is_used")->updateBool(req->DC_EVChargeParameter.EVEnergyRequest_isUsed);
    api_state.get("full_soc")->updateInt(req->DC_EVChargeParameter.FullSOC);
    api_state.get("full_soc_is_used")->updateBool(req->DC_EVChargeParameter.FullSOC_isUsed);
    api_state.get("bulk_soc")->updateInt(req->DC_EVChargeParameter.BulkSOC);
    api_state.get("bulk_soc_is_used")->updateBool(req->DC_EVChargeParameter.BulkSOC_isUsed);

    logger.printfln("DIN70121: Current SoC %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);

    // Here we try to get the EV into a loop that calls ChargeParameterDiscoveryReq again and again
    // to be able to continously read the SoC.

    dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1;

    // [V2G-DC-493] After the EVCC has successfully processed a received ChargeParameterDiscoveryRes
    // message with ResponseCode equal to “OK” and EVSEProcessing equal to“Ongoing”, the EVCC shall
    // ignore the values of SAScheduleList and DC_EVSEChargeParameter contained in this
    // ChargeParameterDiscoveryRes message, and shall send another ChargeParameterDiscoveryReq
    // message and shall then wait for a ChargeParameterDiscoveryRes message. This following ChargeParameterDiscoveryReq message,
    // if any, may contain different values for the contained parameters than the preceding ChargeParameterDiscoveryReq message.


    // Get EV to send ChargeParameterDiscoveryReq again by using EVSE_IsolationMonitoringActive with EVSEProcessingType_Ongoing
    // See DIN/TS:70121:2024 [V2G-DC-966]
    res->ResponseCode = din_responseCodeType_OK;

    // [V2G-DC-863] If the EVCC receives a V2G response message with parameter EVSEProcessing equal to
    // ’Ongoing’ for the first time in a response message it shall start the timer V2G_EVCC_Ongo-
    // ing_Timer and wait for parameter EVSEProcessing equal to ’Finished’.

    // [V2G-DC-864] If [V2G-DC-863] applies, the EVCC shall stop the V2G communication session when
    // V2G_EVCC_Ongoing_Timer is equal or larger than V2G_EVCC_Ongoing_Timeout and no
    // parameter EVSEProcessing equal to ’Finished’ has been received.

    // [V2G-DC-1004] If [V2G-DC-863] applies, when the EVCC receives parameter EVSEProcessing equal to
    // ’Finished’, it shall stop the V2G_EVCC_Ongoing_Timer and end monitoring it.

    // TODO: Does [V2G-DC-863] + [V2G-DC-864] mean that we can only delay with EVSEProcessingType_Ongoing once?
    res->EVSEProcessing = din_EVSEProcessingType_Ongoing;

    // Invalid: An isolation test has not been carried out.
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus = din_isolationLevelType_Invalid;

    // [V2G-DC-500] For DC charging according to this document, the value of EVSENotification shall always be set to “None”.
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification = din_EVSENotificationType_None;
    res->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay = 0;

    // EVSE_IsolationMonitoringActive: After the charging station has confirmed HV isolation internally, it will remain in this state until the cable isolation integrity is checked
    // TODO: Try EVSEReady instead?
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode = din_DC_EVSEStatusCodeType_EVSE_Ready;

    // Mandatory charge parameters
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit = din_unitSymbolType_A;
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value = 500; // 500A
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier = 0;

    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit = din_unitSymbolType_V;
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value = 800; // 400V
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Multiplier = 0;

    res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Unit = din_unitSymbolType_A;
    res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Value = 0; // 0A
    res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Multiplier = 0;

    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit = din_unitSymbolType_V;
    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Value = 0; // 0V
    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Multiplier = 0;

    res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Unit = din_unitSymbolType_A;
    res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Value = 1; // 1A
    res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Multiplier = 0;

    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Unit = din_unitSymbolType_W;
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Value = 20000; // 20000W * 10^1 = 200kW
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Multiplier = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit_isUsed = 1; // Mandatory according to the list?

    res->DC_EVSEChargeParameter_isUsed = 1;

    // Optinal charge parameters
    res->DC_EVSEChargeParameter.EVSECurrentRegulationTolerance_isUsed = 0;
    res->DC_EVSEChargeParameter.EVSEEnergyToBeDelivered_isUsed = 0;

    // [V2G-DC-559] Since for DC charging according to this document, the EVCC is not able to provide a planned
    // departure time, SAScheduleList shall provide PMaxSchedule (refer to 9.5.2.10) covering at least 24 hours.
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax = SHRT_MAX;

    // [V2G-DC-338] The value of the duration element shall be defined as period in seconds.
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration = 86400;
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration_isUsed = 1; // Must be used in DIN

    // [V2G-DC-336] The value of the start element shall be defined in seconds from NOW.
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.start = 0; // Start of the interval, in seconds from NOW.
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval_isUsed = 1;

    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].TimeInterval_isUsed = 0; // no content
    res->SAScheduleList.SAScheduleTuple.array[0].SAScheduleTupleID = 1;
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.arrayLen = 1;
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleID = 1;

    // [V2G-DC-554] In the scope of this document, the element “SalesTariff” shall not be used.
    res->SAScheduleList.SAScheduleTuple.array[0].SalesTariff_isUsed = 0;
    res->SAScheduleList.SAScheduleTuple.arrayLen = 1;
    res->SAScheduleList_isUsed = 1;

    // [V2G-DC-882] The EV shall ignore the SASchedule received in ChargeParameterDiscoveryRes
    res->SASchedules_isUsed = 0;

    // Has no content
    res->EVSEChargeParameter_isUsed = 0;

    // [V2G-DC-552] In the scope of this document, the element “AC_EVSEChargeParameter” shall not be used
    res->AC_EVSEChargeParameter_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Din);
    state = 6;
}

void DIN70121::handle_session_stop_req()
{
    din_SessionStopResType *res = &dinDocEnc->V2G_Message.Body.SessionStopRes;

    dinDocEnc->V2G_Message.Body.SessionStopRes_isUsed = 1;
    res->ResponseCode = din_responseCodeType_OK;

    iso15118.common.send_exi(Common::ExiType::Din);
    state = 7;
}

void DIN70121::trace_header(const struct din_MessageHeaderType *header, const char *name)
{
    iso15118.trace("V2G_Message (%s)", name);
    iso15118.trace(" Header");
    iso15118.trace("  SessionID.bytes: %02x%02x%02x%02x", header->SessionID.bytes[0], header->SessionID.bytes[1], header->SessionID.bytes[2], header->SessionID.bytes[3]);
    iso15118.trace("  SessionID.bytesLen: %d", header->SessionID.bytesLen);
    iso15118.trace("  Notification_isUsed: %d", header->Notification_isUsed);
    if (header->Notification_isUsed) {
        iso15118.trace("  Notification.FaultCode: %d", header->Notification.FaultCode);
        iso15118.trace("  Notification.FaultMsg_isUsed: %d", header->Notification.FaultMsg_isUsed);
        if (header->Notification.FaultMsg_isUsed) {
            iso15118.trace("  Notification.FaultMsg.characters: %s", header->Notification.FaultMsg.characters);
            iso15118.trace("  Notification.FaultMsg.charactersLen: %d", header->Notification.FaultMsg.charactersLen);
        }
    }
    iso15118.trace("  Signature_isUsed: %d", header->Signature_isUsed);
}

void DIN70121::trace_request_response()
{
    // Trace requests
    if (dinDocDec->V2G_Message.Body.SessionSetupReq_isUsed) {
        din_SessionSetupReqType *req = &dinDocDec->V2G_Message.Body.SessionSetupReq;

        trace_header(&dinDocDec->V2G_Message.Header, "SessionSetup Request");
        iso15118.trace(" Body");
        iso15118.trace("  SessionSetupReq");
        iso15118.trace("   EVCCID: %02x%02x%02x%02x%02x%02x%02x%02x",
                       req->EVCCID.bytes[0], req->EVCCID.bytes[1], req->EVCCID.bytes[2], req->EVCCID.bytes[3],
                       req->EVCCID.bytes[4], req->EVCCID.bytes[5], req->EVCCID.bytes[6], req->EVCCID.bytes[7]);
        iso15118.trace("   EVCCID.bytesLen: %d", req->EVCCID.bytesLen);
    } else if (dinDocDec->V2G_Message.Body.ServiceDiscoveryReq_isUsed) {
        din_ServiceDiscoveryReqType *req = &dinDocDec->V2G_Message.Body.ServiceDiscoveryReq;

        trace_header(&dinDocDec->V2G_Message.Header, "ServiceDiscovery Request");
        iso15118.trace(" Body");
        iso15118.trace("  ServiceDiscoveryReq");
        iso15118.trace("   ServiceCategory_isUsed: %d", req->ServiceCategory_isUsed);
        if (req->ServiceCategory_isUsed) {
            iso15118.trace("   ServiceCategory: %d", req->ServiceCategory);
        }
        iso15118.trace("   ServiceScope_isUsed: %d", req->ServiceScope_isUsed);
        if (req->ServiceScope_isUsed) {
            iso15118.trace("   ServiceScope: %s", req->ServiceScope.characters);
        }
    } else if (dinDocDec->V2G_Message.Body.ServicePaymentSelectionReq_isUsed) {
        din_ServicePaymentSelectionReqType *req = &dinDocDec->V2G_Message.Body.ServicePaymentSelectionReq;

        trace_header(&dinDocDec->V2G_Message.Header, "ServicePaymentSelection Request");
        iso15118.trace(" Body");
        iso15118.trace("  ServicePaymentSelectionReq");
        iso15118.trace("   SelectedPaymentOption: %d", req->SelectedPaymentOption);
        for (uint16_t i = 0; i < req->SelectedServiceList.SelectedService.arrayLen; i++) {
            iso15118.trace("   SelectedService[%d]", i);
            iso15118.trace("    ServiceID: %d", req->SelectedServiceList.SelectedService.array[i].ServiceID);
            iso15118.trace("    ParameterSetID_isUsed: %d", req->SelectedServiceList.SelectedService.array[i].ParameterSetID_isUsed);
            if (req->SelectedServiceList.SelectedService.array[i].ParameterSetID_isUsed) {
                iso15118.trace("     ParameterSetID: %d", req->SelectedServiceList.SelectedService.array[i].ParameterSetID);
            }
        }
    } else if (dinDocDec->V2G_Message.Body.ContractAuthenticationReq_isUsed) {
        trace_header(&dinDocDec->V2G_Message.Header, "ContractAuthentication Request");
        iso15118.trace(" Body");
        iso15118.trace("  ContractAuthenticationReq");
        // [V2G-DC-550] In the scope of this document, the element "GenChallenge" shall not be used.
        // [V2G-DC-545] In the scope of this document, the element "Id" shall not be used.
    } else if (dinDocDec->V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed) {
        din_ChargeParameterDiscoveryReqType *req = &dinDocDec->V2G_Message.Body.ChargeParameterDiscoveryReq;

        trace_header(&dinDocDec->V2G_Message.Header, "ChargeParameterDiscovery Request");
        iso15118.trace(" Body");
        iso15118.trace("  ChargeParameterDiscoveryReq");
        iso15118.trace("   EVRequestedEnergyTransferType: %d", req->EVRequestedEnergyTransferType);
        iso15118.trace("   DC_EVChargeParameter_isUsed: %d", req->DC_EVChargeParameter_isUsed);
        if (req->DC_EVChargeParameter_isUsed) {
            iso15118.trace("    DC_EVStatus.EVRESSSOC: %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
            iso15118.trace("    DC_EVStatus.EVReady: %d", req->DC_EVChargeParameter.DC_EVStatus.EVReady);
            iso15118.trace("    DC_EVStatus.EVCabinConditioning: %d", req->DC_EVChargeParameter.DC_EVStatus.EVCabinConditioning);
            iso15118.trace("    DC_EVStatus.EVCabinConditioning_isUsed: %d", req->DC_EVChargeParameter.DC_EVStatus.EVCabinConditioning_isUsed);
            iso15118.trace("    DC_EVStatus.EVRESSConditioning: %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSConditioning);
            iso15118.trace("    DC_EVStatus.EVRESSConditioning_isUsed: %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSConditioning_isUsed);
            iso15118.trace("    DC_EVStatus.EVErrorCode: %d", req->DC_EVChargeParameter.DC_EVStatus.EVErrorCode);
            iso15118.trace("    EVMaximumCurrentLimit.Value: %d", req->DC_EVChargeParameter.EVMaximumCurrentLimit.Value);
            iso15118.trace("    EVMaximumCurrentLimit.Multiplier: %d", req->DC_EVChargeParameter.EVMaximumCurrentLimit.Multiplier);
            iso15118.trace("    EVMaximumPowerLimit.Value: %d", req->DC_EVChargeParameter.EVMaximumPowerLimit.Value);
            iso15118.trace("    EVMaximumPowerLimit.Multiplier: %d", req->DC_EVChargeParameter.EVMaximumPowerLimit.Multiplier);
            iso15118.trace("    EVMaximumPowerLimit_isUsed: %d", req->DC_EVChargeParameter.EVMaximumPowerLimit_isUsed);
            iso15118.trace("    EVMaximumVoltageLimit.Value: %d", req->DC_EVChargeParameter.EVMaximumVoltageLimit.Value);
            iso15118.trace("    EVMaximumVoltageLimit.Multiplier: %d", req->DC_EVChargeParameter.EVMaximumVoltageLimit.Multiplier);
            iso15118.trace("    EVEnergyCapacity.Value: %d", req->DC_EVChargeParameter.EVEnergyCapacity.Value);
            iso15118.trace("    EVEnergyCapacity.Multiplier: %d", req->DC_EVChargeParameter.EVEnergyCapacity.Multiplier);
            iso15118.trace("    EVEnergyCapacity_isUsed: %d", req->DC_EVChargeParameter.EVEnergyCapacity_isUsed);
            iso15118.trace("    EVEnergyRequest.Value: %d", req->DC_EVChargeParameter.EVEnergyRequest.Value);
            iso15118.trace("    EVEnergyRequest.Multiplier: %d", req->DC_EVChargeParameter.EVEnergyRequest.Multiplier);
            iso15118.trace("    EVEnergyRequest_isUsed: %d", req->DC_EVChargeParameter.EVEnergyRequest_isUsed);
            iso15118.trace("    FullSOC: %d", req->DC_EVChargeParameter.FullSOC);
            iso15118.trace("    FullSOC_isUsed: %d", req->DC_EVChargeParameter.FullSOC_isUsed);
            iso15118.trace("    BulkSOC: %d", req->DC_EVChargeParameter.BulkSOC);
            iso15118.trace("    BulkSOC_isUsed: %d", req->DC_EVChargeParameter.BulkSOC_isUsed);
        }
    } else if (dinDocDec->V2G_Message.Body.SessionStopReq_isUsed) {
        trace_header(&dinDocDec->V2G_Message.Header, "SessionStop Request");
        iso15118.trace(" Body");
        iso15118.trace("  SessionStopReq");
    }

    // Trace responses
    if (dinDocEnc->V2G_Message.Body.SessionSetupRes_isUsed) {
        din_SessionSetupResType *res = &dinDocEnc->V2G_Message.Body.SessionSetupRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "SessionSetup Response");
        iso15118.trace(" Body");
        iso15118.trace("  SessionSetupRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEID: %02x", res->EVSEID.bytes[0]);
        iso15118.trace("   EVSEID.bytesLen: %d", res->EVSEID.bytesLen);
        iso15118.trace("   DateTimeNow_isUsed: %d", res->DateTimeNow_isUsed);
    } else if (dinDocEnc->V2G_Message.Body.ServiceDiscoveryRes_isUsed) {
        din_ServiceDiscoveryResType *res = &dinDocEnc->V2G_Message.Body.ServiceDiscoveryRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "ServiceDiscovery Response");
        iso15118.trace(" Body");
        iso15118.trace("  ServiceDiscoveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   PaymentOptions.PaymentOption: %d", res->PaymentOptions.PaymentOption.array[0]);
        iso15118.trace("   ChargeService.ServiceTag.ServiceID: %d", res->ChargeService.ServiceTag.ServiceID);
        iso15118.trace("   ChargeService.ServiceTag.ServiceCategory: %d", res->ChargeService.ServiceTag.ServiceCategory);
        iso15118.trace("   ChargeService.FreeService: %d", res->ChargeService.FreeService);
        iso15118.trace("   ChargeService.EnergyTransferType: %d", res->ChargeService.EnergyTransferType);
    } else if (dinDocEnc->V2G_Message.Body.ServicePaymentSelectionRes_isUsed) {
        din_ServicePaymentSelectionResType *res = &dinDocEnc->V2G_Message.Body.ServicePaymentSelectionRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "ServicePaymentSelection Response");
        iso15118.trace(" Body");
        iso15118.trace("  ServicePaymentSelectionRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
    } else if (dinDocEnc->V2G_Message.Body.ContractAuthenticationRes_isUsed) {
        din_ContractAuthenticationResType *res = &dinDocEnc->V2G_Message.Body.ContractAuthenticationRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "ContractAuthentication Response");
        iso15118.trace(" Body");
        iso15118.trace("  ContractAuthenticationRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEProcessing: %d", res->EVSEProcessing);
    } else if (dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed) {
        din_ChargeParameterDiscoveryResType *res = &dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "ChargeParameterDiscovery Response");
        iso15118.trace(" Body");
        iso15118.trace("  ChargeParameterDiscoveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEProcessing: %d", res->EVSEProcessing);
        iso15118.trace("   DC_EVSEChargeParameter_isUsed: %d", res->DC_EVSEChargeParameter_isUsed);
        if (res->DC_EVSEChargeParameter_isUsed) {
            iso15118.trace("    DC_EVSEStatus.EVSEIsolationStatus: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus);
            iso15118.trace("    DC_EVSEStatus.EVSEIsolationStatus_isUsed: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed);
            iso15118.trace("    DC_EVSEStatus.EVSENotification: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification);
            iso15118.trace("    DC_EVSEStatus.NotificationMaxDelay: %lu", res->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay);
            iso15118.trace("    DC_EVSEStatus.EVSEStatusCode: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode);
            iso15118.trace("    EVSEMaximumCurrentLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value);
            iso15118.trace("    EVSEMaximumCurrentLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier);
            iso15118.trace("    EVSEMaximumVoltageLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value);
            iso15118.trace("    EVSEMaximumVoltageLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Multiplier);
            iso15118.trace("    EVSEMinimumCurrentLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Value);
            iso15118.trace("    EVSEMinimumCurrentLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Multiplier);
            iso15118.trace("    EVSEMinimumVoltageLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Value);
            iso15118.trace("    EVSEMinimumVoltageLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Multiplier);
            iso15118.trace("    EVSEPeakCurrentRipple.Value: %d", res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Value);
            iso15118.trace("    EVSEPeakCurrentRipple.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Multiplier);
            iso15118.trace("    EVSEMaximumPowerLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Value);
            iso15118.trace("    EVSEMaximumPowerLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Multiplier);
            iso15118.trace("    EVSEMaximumPowerLimit_isUsed: %d", res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit_isUsed);
        }
    } else if (dinDocEnc->V2G_Message.Body.SessionStopRes_isUsed) {
        din_SessionStopResType *res = &dinDocEnc->V2G_Message.Body.SessionStopRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "SessionStop Response");
        iso15118.trace(" Body");
        iso15118.trace("  SessionStopRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
    }
}
