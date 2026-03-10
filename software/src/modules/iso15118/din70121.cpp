/* esp32-firmware
 * Copyright (C) 2025-2026 Olaf Lüke <olaf@tinkerforge.com>
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
#include "generated/module_dependencies.h"
#include "build.h"
#include "tools/malloc.h"

#include "cbv2g/exi_v2gtp.h"
#include "cbv2g/app_handshake/appHand_Decoder.h"
#include "cbv2g/app_handshake/appHand_Encoder.h"
#include "cbv2g/din/din_msgDefDecoder.h"
#include "cbv2g/din/din_msgDefEncoder.h"
#include "cbv2g/common/exi_bitstream.h"

#include "gcc_warnings.h"

void DIN70121::pre_setup()
{
    api_state = Config::Object({
        {"state", Config::Enum(DIN70121State::Idle)},
        {"session_id", Config::Tuple(4, Config::Uint8(0))},
        {"evcc_id", Config::Array({}, Config::get_prototype_uint8_0(), 0, 8, Config::type_id<Config::ConfUint>())},
        {"soc", Config::Int8(0)},
    });
}

void DIN70121::handle_bitstream(exi_bitstream *exi)
{
    // Increment state on first call
    if (state == DIN70121State::Idle) {
        state = DIN70121State::BitstreamReceived;
    }

    // Lazy-alloc DIN buffers on first use; they stay allocated for the session lifetime.
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
    if (ret != 0) {
        logger.printfln("DIN70121: Could not decode EXI document: %d", ret);
        return;
    }

    dispatch_messages();

    trace_request_response();

    api_state.get("state")->updateEnum(state);

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

    // All messages after SessionSetup require session ID validation.
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
    V2G_DISPATCH("DIN70121", body, CableCheckReq,               handle_cable_check_req);
    V2G_DISPATCH("DIN70121", body, PowerDeliveryReq,            handle_power_delivery_req);

    // We handle PreChargeReq and CurrentDemandReq to respond with FAILED + EVSE_Shutdown
    // as safety nets when an EV ignores our shutdown signals, matching ISO2 behavior.
    V2G_DISPATCH("DIN70121", body, PreChargeReq,                handle_pre_charge_req);
    V2G_DISPATCH("DIN70121", body, CurrentDemandReq,            handle_current_demand_req);

    // Not yet implemented

    // Not needed for our SoC-read-only flow.
    // We will not support them.
    V2G_NOT_IMPL("DIN70121", body, ServiceDetailReq);
    V2G_NOT_IMPL("DIN70121", body, PaymentDetailsReq);
    V2G_NOT_IMPL("DIN70121", body, ChargingStatusReq);
    V2G_NOT_IMPL("DIN70121", body, MeteringReceiptReq);
    V2G_NOT_IMPL("DIN70121", body, CertificateUpdateReq);
    V2G_NOT_IMPL("DIN70121", body, CertificateInstallationReq);
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
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, PowerDelivery,            din_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, CableCheck,               din_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, PreCharge,                din_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, CurrentDemand,            din_responseCodeType_FAILED_UnknownSession))
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

    // Reset soc_read flag for new session
    soc_read = false;

    api_state.get("evcc_id")->removeAll();
    for (uint16_t i = 0; i < std::min(static_cast<uint16_t>(sizeof(req->EVCCID.bytes)), req->EVCCID.bytesLen); i++) {
        api_state.get("evcc_id")->add()->updateUint(req->EVCCID.bytes[i]);
    }

    // [V2G-DC-993] SessionID=0 -> generate new SessionID.
    // [V2G-DC-872] SessionID mismatch -> new session with OK_NewSessionEstablished.
    // [V2G-DC-934] EVCC compares length first, then value.
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
    state = DIN70121State::SessionSetup;
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
    res->ChargeService.ServiceTag.ServiceID = V2G_SERVICE_ID_CHARGING;
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
    state = DIN70121State::ServiceDiscovery;
}

void DIN70121::handle_service_payment_selection_req()
{
    din_ServicePaymentSelectionReqType *req = &dinDocDec->V2G_Message.Body.ServicePaymentSelectionReq;
    din_ServicePaymentSelectionResType *res = &dinDocEnc->V2G_Message.Body.ServicePaymentSelectionRes;

    if (req->SelectedPaymentOption == din_paymentOptionType_ExternalPayment) {
        dinDocEnc->V2G_Message.Body.ServicePaymentSelectionRes_isUsed = 1;
        res->ResponseCode = din_responseCodeType_OK;

        iso15118.common.send_exi(Common::ExiType::Din);
        state = DIN70121State::ServicePaymentSelection;
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
    // We want to go on ChargeParameterDiscovery to read the SoC and then use Ongoing.
    res->ResponseCode = din_responseCodeType_OK;
    res->EVSEProcessing = din_EVSEProcessingType_Finished;
    iso15118.common.send_exi(Common::ExiType::Din);

    state = DIN70121State::ContractAuthentication;
}

void DIN70121::handle_charge_parameter_discovery_req()
{
    din_ChargeParameterDiscoveryReqType* req = &dinDocDec->V2G_Message.Body.ChargeParameterDiscoveryReq;
    din_ChargeParameterDiscoveryResType* res = &dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes;

    api_state.get("soc")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);

    logger.printfln("DIN70121: Current SoC %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);

    // Update EV data for meters module
    {
        EVData ev_data;
        ev_data.soc_present = static_cast<float>(req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);

        // Only update the SoC, all other values would be for DC charging.
        iso15118.common.update_ev_data(ev_data, EVDataProtocol::DIN);
    }

    dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1;

    // In read_soc_only mode: read SoC on first request, then signal EVSE_Shutdown on second request.
    // Also applies when charge_via_iso15118 is set: DIN 70121 is DC-only, so we can't do
    // AC charging via DIN. Read SoC if configured, then end session to fall back to IEC 61851.
    //
    // We use ResponseCode=OK with EVSEStatusCode=EVSE_Shutdown + EVSEProcessing=Ongoing.
    // Ongoing keeps the EV in a ChargeParameterDiscoveryReq loop that times out after ~10s
    // (per [V2G-DC-864]), after which the EV sends SessionStopReq or PowerDeliveryReq(Stop).
    if ((iso15118.is_read_soc_only() || iso15118.config.get("charge_via_iso15118")->asBool()) && soc_read) {
        logger.printfln("DIN70121: SoC already read, sending EVSE_Shutdown to end session");
        res->ResponseCode = din_responseCodeType_OK;
        res->EVSEProcessing = din_EVSEProcessingType_Ongoing;

        // Mandatory fields: DC_EVSEChargeParameter with valid DC_EVSEStatus and limits
        // must be present for a valid ChargeParameterDiscoveryRes EXI encoding.
        res->DC_EVSEChargeParameter_isUsed = 1;
        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus = din_isolationLevelType_Invalid;
        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification = din_EVSENotificationType_None;
        res->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay = 0;
        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode = din_DC_EVSEStatusCodeType_EVSE_Shutdown;
        res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier = 0;
        res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit = din_unitSymbolType_A;
        res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit_isUsed = 1;
        res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value = 0;
        res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Multiplier = 0;
        res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit = din_unitSymbolType_V;
        res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit_isUsed = 1;
        res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value = 0;
        res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Multiplier = 0;
        res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Unit = din_unitSymbolType_A;
        res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Unit_isUsed = 1;
        res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Value = 0;
        res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Multiplier = 0;
        res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit = din_unitSymbolType_V;
        res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit_isUsed = 1;
        res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Value = 0;
        res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Multiplier = 0;
        res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Unit = din_unitSymbolType_A;
        res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Unit_isUsed = 1;
        res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Value = 0;
        // [V2G-DC-950] EVSEMaximumPowerLimit shall be used
        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Multiplier = 0;
        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Unit = din_unitSymbolType_W;
        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Unit_isUsed = 1;
        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Value = 0;
        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit_isUsed = 1;
        res->DC_EVSEChargeParameter.EVSECurrentRegulationTolerance_isUsed = 0;
        res->DC_EVSEChargeParameter.EVSEEnergyToBeDelivered_isUsed = 0;

        res->SAScheduleList_isUsed = 1;
        res->SAScheduleList.SAScheduleTuple.array[0].SAScheduleTupleID = V2G_SA_SCHEDULE_TUPLE_ID;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleID = 1;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax = 0;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.start = 0;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration = SECONDS_PER_DAY;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration_isUsed = 1;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval_isUsed = 1;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].TimeInterval_isUsed = 0;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.arrayLen = 1;
        res->SAScheduleList.SAScheduleTuple.array[0].SalesTariff_isUsed = 0;
        res->SAScheduleList.SAScheduleTuple.arrayLen = 1;
        res->SASchedules_isUsed = 0;
        res->EVSEChargeParameter_isUsed = 0;
        res->AC_EVSEChargeParameter_isUsed = 0;

        iso15118.common.send_exi(Common::ExiType::Din);
        state = DIN70121State::ChargeParameterDiscovery;
        return;
    }

    if (iso15118.is_read_soc_only() || iso15118.config.get("charge_via_iso15118")->asBool()) {
        soc_read = true;
    }

    // Here we try to get the EV into a loop that calls ChargeParameterDiscoveryReq again and again
    // to be able to continuously read the SoC.

    dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1;

    // [V2G-DC-493] On OK + Ongoing, the EVCC ignores schedule/params and resends ChargeParameterDiscoveryReq.


    // Get EV to send ChargeParameterDiscoveryReq again by using EVSE_Ready with EVSEProcessingType_Ongoing
    res->ResponseCode = din_responseCodeType_OK;

    // [V2G-DC-863/864/1004] Ongoing starts a timer (~10s). If never Finished, EV stops the session.

    // TODO: Does [V2G-DC-863] + [V2G-DC-864] mean that we can only delay with EVSEProcessingType_Ongoing once?
    res->EVSEProcessing = din_EVSEProcessingType_Ongoing;

    // Invalid: An isolation test has not been carried out.
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus = din_isolationLevelType_Invalid;

    // [V2G-DC-500] For DC charging according to this document, the value of EVSENotification shall always be set to “None”.
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification = din_EVSENotificationType_None;
    res->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay = 0;

    // EVSE_Ready: The EVSE is ready for charging.
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode = din_DC_EVSEStatusCodeType_EVSE_Ready;

    // Mandatory charge parameters
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit = din_unitSymbolType_A;
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value = DC_SOC_MAX_CURRENT_A; // 500A
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier = 0;

    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit = din_unitSymbolType_V;
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value = DC_SOC_MAX_VOLTAGE_V; // 800V
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
    res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Value = DC_SOC_PEAK_RIPPLE_A; // 1A
    res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Multiplier = 0;

    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Unit = din_unitSymbolType_W;
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Value = DC_SOC_MAX_POWER_VALUE; // 20000W * 10^1 = 200kW
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Multiplier = DC_SOC_MAX_POWER_EXP;
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit_isUsed = 1;

    res->DC_EVSEChargeParameter_isUsed = 1;

    // Optional charge parameters
    res->DC_EVSEChargeParameter.EVSECurrentRegulationTolerance_isUsed = 0;
    res->DC_EVSEChargeParameter.EVSEEnergyToBeDelivered_isUsed = 0;

    // [V2G-DC-559] Since for DC charging according to this document, the EVCC is not able to provide a planned
    // departure time, SAScheduleList shall provide PMaxSchedule (refer to 9.5.2.10) covering at least 24 hours.
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax = SHRT_MAX;

    // [V2G-DC-338] The value of the duration element shall be defined as period in seconds.
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration = SECONDS_PER_DAY;
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration_isUsed = 1; // Must be used in DIN

    // [V2G-DC-336] The value of the start element shall be defined in seconds from NOW.
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.start = 0; // Start of the interval, in seconds from NOW.
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval_isUsed = 1;

    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].TimeInterval_isUsed = 0; // no content
    res->SAScheduleList.SAScheduleTuple.array[0].SAScheduleTupleID = V2G_SA_SCHEDULE_TUPLE_ID;
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
    state = DIN70121State::ChargeParameterDiscovery;
}

void DIN70121::handle_cable_check_req()
{
    din_CableCheckResType *res = &dinDocEnc->V2G_Message.Body.CableCheckRes;

    dinDocEnc->V2G_Message.Body.CableCheckRes_isUsed = 1;

    // We will reach CableCheck in the SoC-read-only flow when the EV doesn't check
    // EVSEStatusCode in ChargeParameterDiscoveryRes.
    // [V2G-DC-891] If the SECC wants to stop the process, it shall send CableCheckRes with:
    //   ResponseCode = OK, EVSEProcessing = Finished, EVSEStatusCode = EVSE_Shutdown,
    //   EVSENotification = None, EVSEIsolationStatus = Invalid/valid/warning/fault.
    // Note: DIN uses OK here (not FAILED). FAILED is only for isolation faults per [V2G-DC-890].
    // [V2G-DC-901] On Finished + FAILED, the EVCC shall stop the charging session.
    // [V2G-DC-500] EVSENotification shall always be "None" for DC charging per DIN.
    res->ResponseCode = din_responseCodeType_OK;
    res->EVSEProcessing = din_EVSEProcessingType_Finished;

    res->DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
    res->DC_EVSEStatus.EVSEIsolationStatus = din_isolationLevelType_Invalid;
    res->DC_EVSEStatus.EVSENotification = din_EVSENotificationType_None;
    res->DC_EVSEStatus.NotificationMaxDelay = 0;
    res->DC_EVSEStatus.EVSEStatusCode = din_DC_EVSEStatusCodeType_EVSE_Shutdown;

    logger.printfln("DIN70121: CableCheckReq received in SoC-read flow, sending EVSE_Shutdown to terminate");

    iso15118.common.send_exi(Common::ExiType::Din);
    state = DIN70121State::CableCheck;

    // Cancel sequence timeout since we may not get a SessionStopReq.
    if (iso15118.is_read_soc_only() || iso15118.config.get("charge_via_iso15118")->asBool()) {
        cancel_sequence_timeout(next_timeout);
    }
}

void DIN70121::handle_power_delivery_req()
{
    din_PowerDeliveryReqType *req = &dinDocDec->V2G_Message.Body.PowerDeliveryReq;
    din_PowerDeliveryResType *res = &dinDocEnc->V2G_Message.Body.PowerDeliveryRes;

    if (req->DC_EVPowerDeliveryParameter_isUsed) {
        api_state.get("soc")->updateInt(req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVRESSSOC);
    }

    dinDocEnc->V2G_Message.Body.PowerDeliveryRes_isUsed = 1;
    res->ResponseCode = din_responseCodeType_OK;

    // DIN 70121 is DC-only; include DC_EVSEStatus with EVSE_Shutdown so
    // the EV proceeds to SessionStopReq for a clean session teardown.
    res->DC_EVSEStatus_isUsed = 1;
    res->DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
    res->DC_EVSEStatus.EVSEIsolationStatus = din_isolationLevelType_Invalid;
    res->DC_EVSEStatus.EVSENotification = din_EVSENotificationType_None;
    res->DC_EVSEStatus.NotificationMaxDelay = 0;
    res->DC_EVSEStatus.EVSEStatusCode = din_DC_EVSEStatusCodeType_EVSE_Shutdown;

    res->AC_EVSEStatus_isUsed = 0;
    res->EVSEStatus_isUsed = 0;

    logger.printfln("DIN70121: PowerDeliveryReq (ReadyToChargeState=%d), responding OK with EVSE_Shutdown",
                     req->ReadyToChargeState);

    iso15118.common.send_exi(Common::ExiType::Din);
    state = DIN70121State::PowerDelivery;
}

void DIN70121::handle_pre_charge_req()
{
    din_PreChargeResType *res = &dinDocEnc->V2G_Message.Body.PreChargeRes;

    dinDocEnc->V2G_Message.Body.PreChargeRes_isUsed = 1;

    // Safety net: The EV sent PreChargeReq despite our shutdown signals in
    // ChargeParameterDiscoveryRes and CableCheckRes. This is non-compliant EV behavior.
    // [V2G-DC-901] On Finished + FAILED, the EVCC shall stop the charging session.
    res->ResponseCode = din_responseCodeType_FAILED;

    res->DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
    res->DC_EVSEStatus.EVSEIsolationStatus = din_isolationLevelType_Invalid;
    // [V2G-DC-500] EVSENotification shall always be "None" for DC charging per DIN.
    res->DC_EVSEStatus.EVSENotification = din_EVSENotificationType_None;
    res->DC_EVSEStatus.NotificationMaxDelay = 0;
    res->DC_EVSEStatus.EVSEStatusCode = din_DC_EVSEStatusCodeType_EVSE_Shutdown;

    res->EVSEPresentVoltage.Unit = din_unitSymbolType_V;
    res->EVSEPresentVoltage.Unit_isUsed = 1;
    res->EVSEPresentVoltage.Value = 0;
    res->EVSEPresentVoltage.Multiplier = 0;

    logger.printfln("DIN70121: PreChargeReq received in SoC-read flow, sending FAILED to terminate");

    iso15118.common.send_exi(Common::ExiType::Din);

    if (iso15118.is_read_soc_only() || iso15118.config.get("charge_via_iso15118")->asBool()) {
        cancel_sequence_timeout(next_timeout);
    }
}

void DIN70121::handle_current_demand_req()
{
    din_CurrentDemandResType *res = &dinDocEnc->V2G_Message.Body.CurrentDemandRes;

    dinDocEnc->V2G_Message.Body.CurrentDemandRes_isUsed = 1;

    // Safety net: The EV should never reach CurrentDemand in a SoC-read-only flow.
    // [V2G-DC-901] On Finished + FAILED, the EVCC shall stop the charging session.
    res->ResponseCode = din_responseCodeType_FAILED;

    res->DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
    res->DC_EVSEStatus.EVSEIsolationStatus = din_isolationLevelType_Invalid;
    // [V2G-DC-500] EVSENotification shall always be "None" for DC charging per DIN.
    res->DC_EVSEStatus.EVSENotification = din_EVSENotificationType_None;
    res->DC_EVSEStatus.NotificationMaxDelay = 0;
    res->DC_EVSEStatus.EVSEStatusCode = din_DC_EVSEStatusCodeType_EVSE_Shutdown;

    res->EVSEPresentVoltage.Unit = din_unitSymbolType_V;
    res->EVSEPresentVoltage.Unit_isUsed = 1;
    res->EVSEPresentVoltage.Value = 0;
    res->EVSEPresentVoltage.Multiplier = 0;

    res->EVSEPresentCurrent.Unit = din_unitSymbolType_A;
    res->EVSEPresentCurrent.Unit_isUsed = 1;
    res->EVSEPresentCurrent.Value = 0;
    res->EVSEPresentCurrent.Multiplier = 0;

    res->EVSECurrentLimitAchieved = 0;
    res->EVSEVoltageLimitAchieved = 0;
    res->EVSEPowerLimitAchieved = 0;

    res->EVSEMaximumVoltageLimit_isUsed = 0;
    res->EVSEMaximumCurrentLimit_isUsed = 0;
    res->EVSEMaximumPowerLimit_isUsed = 0;

    logger.printfln("DIN70121: CurrentDemandReq received in SoC-read flow, sending FAILED to terminate");

    iso15118.common.send_exi(Common::ExiType::Din);

    if (iso15118.is_read_soc_only() || iso15118.config.get("charge_via_iso15118")->asBool()) {
        cancel_sequence_timeout(next_timeout);
    }
}

void DIN70121::handle_session_stop_req()
{
    din_SessionStopResType *res = &dinDocEnc->V2G_Message.Body.SessionStopRes;

    dinDocEnc->V2G_Message.Body.SessionStopRes_isUsed = 1;
    res->ResponseCode = din_responseCodeType_OK;

    iso15118.common.send_exi(Common::ExiType::Din);
    state = DIN70121State::SessionStop;

    // Begin IEC transition: 100% CP -> 2s delay -> IEC temporary mode.
    // DIN 70121 is DC-only, so we always fall back to IEC.
    if (iso15118.is_read_soc_only() || iso15118.config.get("charge_via_iso15118")->asBool()) {
        iso15118.begin_iec_transition();
        cancel_sequence_timeout(next_timeout);

        // Schedule delayed PLC modem shutdown. If the EV closes TCP early (POLLHUP), the modem is killed immediately.
        if (iso15118.plc_modem_off_task != 0) {
            task_scheduler.cancel(iso15118.plc_modem_off_task);
        }
        iso15118.plc_modem_off_task = task_scheduler.scheduleOnce([this]() {
            logger.printfln("DIN70121: 5s modem-off timer fired, EV did not close TCP in time");
            iso15118.disable_plc_modem();
        }, 5000_ms);

        // Do NOT call reset_active_socket() here. Leave the socket open so the
        // poll loop can detect POLLHUP when the EV closes the connection, which
        // triggers early modem shutdown.
        return;
    }
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
    } else if (dinDocDec->V2G_Message.Body.CableCheckReq_isUsed) {
        din_CableCheckReqType *req = &dinDocDec->V2G_Message.Body.CableCheckReq;

        trace_header(&dinDocDec->V2G_Message.Header, "CableCheck Request");
        iso15118.trace(" Body");
        iso15118.trace("  CableCheckReq");
        iso15118.trace("   DC_EVStatus.EVRESSSOC: %d", req->DC_EVStatus.EVRESSSOC);
        iso15118.trace("   DC_EVStatus.EVReady: %d", req->DC_EVStatus.EVReady);
        iso15118.trace("   DC_EVStatus.EVErrorCode: %d", req->DC_EVStatus.EVErrorCode);
    } else if (dinDocDec->V2G_Message.Body.PowerDeliveryReq_isUsed) {
        din_PowerDeliveryReqType *req = &dinDocDec->V2G_Message.Body.PowerDeliveryReq;

        trace_header(&dinDocDec->V2G_Message.Header, "PowerDelivery Request");
        iso15118.trace(" Body");
        iso15118.trace("  PowerDeliveryReq");
        iso15118.trace("   ReadyToChargeState: %d", req->ReadyToChargeState);
        iso15118.trace("   DC_EVPowerDeliveryParameter_isUsed: %d", req->DC_EVPowerDeliveryParameter_isUsed);
        if (req->DC_EVPowerDeliveryParameter_isUsed) {
            iso15118.trace("    DC_EVStatus.EVRESSSOC: %d", req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVRESSSOC);
            iso15118.trace("    DC_EVStatus.EVReady: %d", req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVReady);
            iso15118.trace("    DC_EVStatus.EVErrorCode: %d", req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVErrorCode);
        }
    } else if (dinDocDec->V2G_Message.Body.PreChargeReq_isUsed) {
        din_PreChargeReqType *req = &dinDocDec->V2G_Message.Body.PreChargeReq;

        trace_header(&dinDocDec->V2G_Message.Header, "PreCharge Request");
        iso15118.trace(" Body");
        iso15118.trace("  PreChargeReq");
        iso15118.trace("   DC_EVStatus.EVRESSSOC: %d", req->DC_EVStatus.EVRESSSOC);
        iso15118.trace("   DC_EVStatus.EVReady: %d", req->DC_EVStatus.EVReady);
        iso15118.trace("   DC_EVStatus.EVErrorCode: %d", req->DC_EVStatus.EVErrorCode);
        iso15118.trace("   EVTargetVoltage.Value: %d", req->EVTargetVoltage.Value);
        iso15118.trace("   EVTargetVoltage.Multiplier: %d", req->EVTargetVoltage.Multiplier);
        iso15118.trace("   EVTargetCurrent.Value: %d", req->EVTargetCurrent.Value);
        iso15118.trace("   EVTargetCurrent.Multiplier: %d", req->EVTargetCurrent.Multiplier);
    } else if (dinDocDec->V2G_Message.Body.CurrentDemandReq_isUsed) {
        din_CurrentDemandReqType *req = &dinDocDec->V2G_Message.Body.CurrentDemandReq;

        trace_header(&dinDocDec->V2G_Message.Header, "CurrentDemand Request");
        iso15118.trace(" Body");
        iso15118.trace("  CurrentDemandReq");
        iso15118.trace("   DC_EVStatus.EVRESSSOC: %d", req->DC_EVStatus.EVRESSSOC);
        iso15118.trace("   DC_EVStatus.EVReady: %d", req->DC_EVStatus.EVReady);
        iso15118.trace("   DC_EVStatus.EVErrorCode: %d", req->DC_EVStatus.EVErrorCode);
        iso15118.trace("   EVTargetVoltage.Value: %d", req->EVTargetVoltage.Value);
        iso15118.trace("   EVTargetVoltage.Multiplier: %d", req->EVTargetVoltage.Multiplier);
        iso15118.trace("   EVTargetCurrent.Value: %d", req->EVTargetCurrent.Value);
        iso15118.trace("   EVTargetCurrent.Multiplier: %d", req->EVTargetCurrent.Multiplier);
        iso15118.trace("   ChargingComplete: %d", req->ChargingComplete);
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
    } else if (dinDocEnc->V2G_Message.Body.CableCheckRes_isUsed) {
        din_CableCheckResType *res = &dinDocEnc->V2G_Message.Body.CableCheckRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "CableCheck Response");
        iso15118.trace(" Body");
        iso15118.trace("  CableCheckRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEProcessing: %d", res->EVSEProcessing);
        iso15118.trace("   DC_EVSEStatus.EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
        iso15118.trace("   DC_EVSEStatus.EVSEIsolationStatus_isUsed: %d", res->DC_EVSEStatus.EVSEIsolationStatus_isUsed);
        iso15118.trace("   DC_EVSEStatus.EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
        iso15118.trace("   DC_EVSEStatus.EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
    } else if (dinDocEnc->V2G_Message.Body.PowerDeliveryRes_isUsed) {
        din_PowerDeliveryResType *res = &dinDocEnc->V2G_Message.Body.PowerDeliveryRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "PowerDelivery Response");
        iso15118.trace(" Body");
        iso15118.trace("  PowerDeliveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   DC_EVSEStatus_isUsed: %d", res->DC_EVSEStatus_isUsed);
        if (res->DC_EVSEStatus_isUsed) {
            iso15118.trace("   DC_EVSEStatus.EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
            iso15118.trace("   DC_EVSEStatus.EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
            iso15118.trace("   DC_EVSEStatus.EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
        }
    } else if (dinDocEnc->V2G_Message.Body.PreChargeRes_isUsed) {
        din_PreChargeResType *res = &dinDocEnc->V2G_Message.Body.PreChargeRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "PreCharge Response");
        iso15118.trace(" Body");
        iso15118.trace("  PreChargeRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   DC_EVSEStatus.EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
        iso15118.trace("   DC_EVSEStatus.EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
        iso15118.trace("   DC_EVSEStatus.EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
        iso15118.trace("   EVSEPresentVoltage.Value: %d", res->EVSEPresentVoltage.Value);
        iso15118.trace("   EVSEPresentVoltage.Multiplier: %d", res->EVSEPresentVoltage.Multiplier);
    } else if (dinDocEnc->V2G_Message.Body.CurrentDemandRes_isUsed) {
        din_CurrentDemandResType *res = &dinDocEnc->V2G_Message.Body.CurrentDemandRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "CurrentDemand Response");
        iso15118.trace(" Body");
        iso15118.trace("  CurrentDemandRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   DC_EVSEStatus.EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
        iso15118.trace("   DC_EVSEStatus.EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
        iso15118.trace("   DC_EVSEStatus.EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
        iso15118.trace("   EVSEPresentVoltage.Value: %d", res->EVSEPresentVoltage.Value);
        iso15118.trace("   EVSEPresentCurrent.Value: %d", res->EVSEPresentCurrent.Value);
    } else if (dinDocEnc->V2G_Message.Body.SessionStopRes_isUsed) {
        din_SessionStopResType *res = &dinDocEnc->V2G_Message.Body.SessionStopRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "SessionStop Response");
        iso15118.trace(" Body");
        iso15118.trace("  SessionStopRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
    }
}
