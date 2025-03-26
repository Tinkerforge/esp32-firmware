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
#include "esp_netif.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
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
        {"session_id", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)
            }, Config::get_prototype_uint8_0(), 4, 4, Config::type_id<Config::ConfUint>())
        },
        {"evcc_id", Config::Array({}, Config::get_prototype_uint8_0(), 0, 8, Config::type_id<Config::ConfUint>())},
        {"soc", Config::Int8(0)},
        {"ev_ready", Config::Int32(0)},
        {"ev_cabin_conditioning", Config::Int32(0)},
        {"ev_cabin_conditioning_is_used", Config::Bool(false)},
        {"ev_ress_conditioning", Config::Int32(0)},
        {"ev_ress_conditioning_is_used", Config::Bool(false)},
        {"ev_error_code", Config::Uint8(0)},
        {"ev_maximum_current_limit_val", Config::Int16(0)},
        {"ev_maximum_current_limit_mul", Config::Int8(0)},
        {"ev_maximum_power_limit_val", Config::Int16(0)},
        {"ev_maximum_power_limit_mul", Config::Int8(0)},
        {"ev_maximum_power_limit_is_used", Config::Bool(0)},
        {"ev_maximum_voltage_limit_val", Config::Int16(0)},
        {"ev_maximum_voltage_limit_mul", Config::Int8(0)},
        {"ev_energy_capacity_val", Config::Int16(0)},
        {"ev_energy_capacity_mul", Config::Int8(0)},
        {"ev_energy_capacity_is_used", Config::Bool(0)},
        {"ev_energy_request_val", Config::Int16(0)},
        {"ev_energy_request_mul", Config::Int8(0)},
        {"ev_energy_request_is_used", Config::Bool(0)},
        {"full_soc", Config::Int8(0)},
        {"full_soc_is_used", Config::Bool(0)},
        {"bulk_soc", Config::Int8(0)},
        {"bulk_soc_is_used", Config::Bool(0)}
    });
}

void DIN70121::handle_bitstream(exi_bitstream *exi)
{
    // Increment state on first call
    if (state == 0) {
        state = 1;
    }

    memset(&dinDocDec, 0, sizeof(dinDocDec));
    memset(&dinDocEnc, 0, sizeof(dinDocEnc));
    int ret = decode_din_exiDocument(exi, &dinDocDec);
    logger.printfln("DIN70121: decode_din_exiDocument: %d", ret);

    if (dinDocDec.V2G_Message.Body.SessionSetupReq_isUsed) {
        iso15118.trace("DIN70121: SessionSetupReq received");
        handle_session_setup_req();
        dinDocDec.V2G_Message.Body.SessionSetupReq_isUsed = 0;
    }

    if (dinDocDec.V2G_Message.Body.ServiceDiscoveryReq_isUsed) {
        iso15118.trace("DIN70121: ServiceDiscoveryReq received");
        handle_service_discovery_req();
        dinDocDec.V2G_Message.Body.ServiceDiscoveryReq_isUsed = 0;
    }

    if (dinDocDec.V2G_Message.Body.ServiceDetailReq_isUsed) {
        iso15118.trace("DIN70121: ServiceDetailReq received (not implemented)");
    }

    if (dinDocDec.V2G_Message.Body.ServicePaymentSelectionReq_isUsed) {
        iso15118.trace("DIN70121: ServicePaymentSelectionReq received");
        handle_service_payment_selection_req();
        dinDocDec.V2G_Message.Body.ServicePaymentSelectionReq_isUsed = 0;
    }

    if (dinDocDec.V2G_Message.Body.PaymentDetailsReq_isUsed) {
        iso15118.trace("DIN70121: PaymentDetailsReq received (not implemented)");
    }

    if (dinDocDec.V2G_Message.Body.ContractAuthenticationReq_isUsed) {
        iso15118.trace("DIN70121: ContractAuthenticationReq received");
        handle_contract_authentication_req();
        dinDocDec.V2G_Message.Body.ContractAuthenticationReq_isUsed = 0;
    }

    if (dinDocDec.V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed) {
        iso15118.trace("DIN70121: ChargeParameterDiscoveryReq received");
        handle_charge_parameter_discovery_req();
        dinDocDec.V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed = 0;
    }

    if (dinDocDec.V2G_Message.Body.PowerDeliveryReq_isUsed) {
        iso15118.trace("DIN70121: PowerDeliveryReq received (not implemented)");
    }

    if (dinDocDec.V2G_Message.Body.ChargingStatusReq_isUsed) {
        iso15118.trace("DIN70121: ChargingStatusReq received (not implemented)");
    }

    if (dinDocDec.V2G_Message.Body.MeteringReceiptReq_isUsed) {
        iso15118.trace("DIN70121: MeteringReceiptReq received (not implemented)");
    }

    if (dinDocDec.V2G_Message.Body.SessionStopReq_isUsed) {
        iso15118.trace("DIN70121: SessionStopReq received");
        handle_session_stop_req();
        dinDocDec.V2G_Message.Body.SessionStopReq_isUsed = 0;
    }

    if (dinDocDec.V2G_Message.Body.CertificateUpdateReq_isUsed) {
        iso15118.trace("DIN70121: CertificateUpdateReq received (not implemented)");
    }

    if (dinDocDec.V2G_Message.Body.CertificateInstallationReq_isUsed) {
        iso15118.trace("DIN70121: CertificateInstallationReq received (not implemented)");
    }

    if (dinDocDec.V2G_Message.Body.CableCheckReq_isUsed) {
        iso15118.trace("DIN70121: CableCheckReq received (not implemented)");
    }

    if (dinDocDec.V2G_Message.Body.PreChargeReq_isUsed) {
        iso15118.trace("DIN70121: PreChargeReq received (not implemented)");
    }

    if (dinDocDec.V2G_Message.Body.CurrentDemandReq_isUsed) {
        iso15118.trace("DIN70121: CurrentDemandReq received (not implemented)");
    }

    if (dinDocDec.V2G_Message.Body.WeldingDetectionReq_isUsed) {
        iso15118.trace("DIN70121: WeldingDetectionReq received (not implemented)");
    }

    api_state.get("state")->updateUint(state);
}

void DIN70121::handle_session_setup_req()
{
    din_SessionSetupReqType *req = &dinDocDec.V2G_Message.Body.SessionSetupReq;
    din_SessionSetupResType *res = &dinDocEnc.V2G_Message.Body.SessionSetupRes;

    api_state.get("evcc_id")->removeAll();
    for (uint16_t i = 0; i < std::min(static_cast<uint16_t>(sizeof(req->EVCCID.bytes)), req->EVCCID.bytesLen); i++) {
        api_state.get("evcc_id")->add()->updateUint(req->EVCCID.bytes[i]);
    }

    // [V2G-DC-993] When receiving the SessionSetupReq with the parameter SessionID equal to zero (0), the
    // SECC shall generate a new (not stored) SessionID value different from zero (0) and return
    // this value in the SessionSetupRes message header.
    bool all_zero = true;
    for (size_t i = 0; i < dinDocDec.V2G_Message.Header.SessionID.bytesLen; i++) {
        if (dinDocDec.V2G_Message.Header.SessionID.bytes[i] != 0x00) {
            all_zero = false;
            break;
        }
    }

    // [V2G-DC-872] If the SECC receives a SessionSetupReq including a SessionID value which is not equal
    // to zero (0) and not equal to the SessionID value stored from the preceding V2G commu-
    // nication session, it shall send a SessionID value in the SessionSetupRes message that is
    // unequal to ʺ0ʺ and unequal to the SessionID value stored from the preceding V2G com-
    // munication session and indicate the new V2G communication session with the Respon-
    // seCode set to ʺOK_NewSessionEstablishedʺ (refer also to [V2G-DC-393] for applicability
    // of this response code).
    bool different_to_known = true;
    // [V2G-DC-934] If the SessionID is checked during the V2G communication session, the EVCC shall first
    // compare the length and then the actual value.
    if (dinDocDec.V2G_Message.Header.SessionID.bytesLen == SESSION_ID_LENGTH) {
        for (uint16_t i = 0; i < SESSION_ID_LENGTH; i++) {
            if (dinDocDec.V2G_Message.Header.SessionID.bytes[i] == iso15118.common.session_id[i]) {
                different_to_known = false;
                break;
            }
        }
    }

    // The SessionId is set up here it is used by the EV in future communication
    if (all_zero || different_to_known) {
        iso15118.common.session_id[0] = static_cast<uint8_t>(random(256));
        iso15118.common.session_id[1] = static_cast<uint8_t>(random(256));
        iso15118.common.session_id[2] = static_cast<uint8_t>(random(256));
        iso15118.common.session_id[3] = static_cast<uint8_t>(random(256));
        res->ResponseCode = din_responseCodeType_OK_NewSessionEstablished;
    } else {
        // keep saved session id, nothing to do here.
        // TODO: Strangely i can't find "OK_OldSessionJoined" implemented anywhere else.
        //       Should we always use "OK_NewSessionEstablished"?
        res->ResponseCode = din_responseCodeType_OK_OldSessionJoined;
    }

    for (uint16_t i = 0; i < SESSION_ID_LENGTH; i++) {
        api_state.get("session_id")->get(i)->updateUint(iso15118.common.session_id[i]);
    }

    dinDocEnc.V2G_Message.Body.SessionSetupRes_isUsed = 1;

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
    din_ServiceDiscoveryReqType *req = &dinDocDec.V2G_Message.Body.ServiceDiscoveryReq;
    din_ServiceDiscoveryResType *res = &dinDocEnc.V2G_Message.Body.ServiceDiscoveryRes;

    // TODO: Stop session if session ID does not match?
    //       Or just keep going? For now we log it and keep going.
    if (dinDocDec.V2G_Message.Header.SessionID.bytesLen != SESSION_ID_LENGTH) {
        logger.printfln("DIN70121: Session ID length mismatch");
    }
    if (memcmp(dinDocDec.V2G_Message.Header.SessionID.bytes, iso15118.common.session_id, SESSION_ID_LENGTH) != 0) {
        logger.printfln("DIN70121: Session ID mismatch");
    }

    dinDocEnc.V2G_Message.Body.ServiceDiscoveryRes_isUsed = 1;
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
    din_ServicePaymentSelectionReqType *req = &dinDocDec.V2G_Message.Body.ServicePaymentSelectionReq;
    din_ServicePaymentSelectionResType *res = &dinDocEnc.V2G_Message.Body.ServicePaymentSelectionRes;

    if (req->SelectedPaymentOption == din_paymentOptionType_ExternalPayment) {
        dinDocEnc.V2G_Message.Body.ServicePaymentSelectionRes_isUsed = 1;
        res->ResponseCode = din_responseCodeType_OK;

        iso15118.common.send_exi(Common::ExiType::Din);
        state = 4;
    }
}

void DIN70121::handle_contract_authentication_req()
{
    din_ContractAuthenticationResType *res = &dinDocEnc.V2G_Message.Body.ContractAuthenticationRes;

    // [V2G-DC-550] In the scope of this document, the element “GenChallenge” shall not be used.
    // [V2G-DC-545] In the scope of this document, the element “Id” shall not be used.
    // -> None of the request parameters are used in DIN SPEC 70121.

    dinDocEnc.V2G_Message.Body.ContractAuthenticationRes_isUsed = 1;

    // Set Authorisation to Finished here.
    // We want to go on ChargeParameteryDiscovery to read the SoC and then use Ongoing.
    res->EVSEProcessing = din_EVSEProcessingType_Finished;
    iso15118.common.send_exi(Common::ExiType::Din);

    state = 5;
}

void DIN70121::handle_charge_parameter_discovery_req()
{
    din_ChargeParameterDiscoveryReqType* req = &dinDocDec.V2G_Message.Body.ChargeParameterDiscoveryReq;
    din_ChargeParameterDiscoveryResType* res = &dinDocDec.V2G_Message.Body.ChargeParameterDiscoveryRes;

    api_state.get("soc")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
    api_state.get("ev_ready")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVReady);
    api_state.get("ev_cabin_conditioning")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVCabinConditioning);
    api_state.get("ev_cabin_conditioning_is_used")->updateBool(req->DC_EVChargeParameter.DC_EVStatus.EVCabinConditioning_isUsed);
    api_state.get("ev_ress_conditioning")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVRESSConditioning);
    api_state.get("ev_ress_conditioning_is_used")->updateBool(req->DC_EVChargeParameter.DC_EVStatus.EVRESSConditioning_isUsed);
    api_state.get("ev_error_code")->updateUint(req->DC_EVChargeParameter.DC_EVStatus.EVErrorCode);
    api_state.get("ev_maximum_current_limit_val")->updateInt(req->DC_EVChargeParameter.EVMaximumCurrentLimit.Value);
    api_state.get("ev_maximum_current_limit_mul")->updateInt(req->DC_EVChargeParameter.EVMaximumCurrentLimit.Multiplier);
    api_state.get("ev_maximum_power_limit_val")->updateInt(req->DC_EVChargeParameter.EVMaximumPowerLimit.Value);
    api_state.get("ev_maximum_power_limit_mul")->updateInt(req->DC_EVChargeParameter.EVMaximumPowerLimit.Multiplier);
    api_state.get("ev_maximum_power_limit_is_used")->updateBool(req->DC_EVChargeParameter.EVMaximumPowerLimit_isUsed);
    api_state.get("ev_maximum_voltage_limit_val")->updateInt(req->DC_EVChargeParameter.EVMaximumVoltageLimit.Value);
    api_state.get("ev_maximum_voltage_limit_mul")->updateInt(req->DC_EVChargeParameter.EVMaximumVoltageLimit.Multiplier);
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

    dinDocEnc.V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1;

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
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode = din_DC_EVSEStatusCodeType_EVSE_IsolationMonitoringActive;

    // Mandatory charge parameters
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit = din_unitSymbolType_A;
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value = 500; // 500A
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier = 0;

    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit = din_unitSymbolType_V;
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value = 400; // 400V
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Multiplier = 0;

    res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Unit = din_unitSymbolType_A;
    res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Value = 6; // 6A
    res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Multiplier = 0;

    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit = din_unitSymbolType_V;
    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Value = 200; // 200V
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
    din_SessionStopResType *res = &dinDocEnc.V2G_Message.Body.SessionStopRes;

    dinDocEnc.V2G_Message.Body.SessionStopRes_isUsed = 1;
    res->ResponseCode = din_responseCodeType_OK;

    iso15118.common.send_exi(Common::ExiType::Din);
    state = 7;
}