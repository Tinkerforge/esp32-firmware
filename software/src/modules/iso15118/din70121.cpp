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

#include "gcc_warnings.h"

void DIN70121::pre_setup()
{
    api_state = Config::Object({
        {"state", Config::Enum(DIN70121State::Idle)},
        {"session_id", Config::Tuple(SESSION_ID_LENGTH, Config::Uint8(0))},
        {"evcc_id", Config::Array({}, Config::get_prototype_uint8_0(), 0, 8, Config::type_id<Config::ConfUint>())},
        {"soc", Config::Int8(-1)},
    });
}

void DIN70121::handle_bitstream(exi_bitstream *exi)
{
    // Leave Idle once the first bitstream of a session arrives
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
    int ret = decode_din_exiDocument(exi, dinDocDec);
    if (ret != 0) {
        iso15118.trace("DIN70121: Could not decode EXI document: %d", ret);
        return;
    }

    cancel_sequence_timeout(next_timeout);

    dispatch_messages();

#ifdef ISO15118_TRACE_MESSAGES
    trace_request_response();
#endif

    api_state.get("state")->updateEnum(state);

    // DIN TS 70121:2024-11 [V2G-DC-443]: The SECC shall stop waiting for a request message
    // when V2G_SECC_Sequence_Timer >= V2G_SECC_Sequence_Timeout and no request was received.
    if (!iso15118.nonegotiation_pending) {
        schedule_sequence_timeout(next_timeout, V2G_SECC_SEQUENCE_TIMEOUT, "DIN70121");
    }
}

void DIN70121::dispatch_messages()
{
    auto &body = dinDocDec->V2G_Message.Body;

    // SessionSetupReq - no session validation needed (session is established here)
    V2G_DISPATCH(body, SessionSetupReq, handle_session_setup_req);
    if (body.SessionSetupReq_isUsed) return;

    // All messages after SessionSetup require session ID validation.
    // If the SessionID received does not match the previously communicated SessionID,
    // the SECC shall respond with FAILED_UnknownSession.
    if (!validate_session_id(dinDocDec->V2G_Message.Header.SessionID.bytes,
                             dinDocDec->V2G_Message.Header.SessionID.bytesLen,
                             iso15118.common.session_id,
                             SESSION_ID_LENGTH)) {
        iso15118.trace("DIN70121: Session ID mismatch, sending FAILED_UnknownSession");
        send_failed_unknown_session();
        return;
    }

    // Implemented message handlers (session already validated)
    V2G_DISPATCH(body, ServiceDiscoveryReq,         handle_service_discovery_req);
    V2G_DISPATCH(body, ServicePaymentSelectionReq,  handle_service_payment_selection_req);
    V2G_DISPATCH(body, ContractAuthenticationReq,   handle_contract_authentication_req);
    V2G_DISPATCH(body, ChargeParameterDiscoveryReq, handle_charge_parameter_discovery_req);
    V2G_DISPATCH(body, PowerDeliveryReq,            handle_power_delivery_req);
    V2G_DISPATCH(body, SessionStopReq,              handle_session_stop_req);
    V2G_DISPATCH(body, CableCheckReq,               handle_cable_check_req);

    // We handle PreChargeReq and CurrentDemandReq to respond with FAILED + EVSE_Shutdown
    // as safety nets when an EV ignores our shutdown signals, matching ISO2 behavior.
    V2G_DISPATCH(body, PreChargeReq,     handle_pre_charge_req);
    V2G_DISPATCH(body, CurrentDemandReq, handle_current_demand_req);

    // VAS (Value Added Services). Not used in practice.
    V2G_NOT_IMPL("DIN70121", body, ServiceDetailReq);

    // These are for Plug&Charge. We don't support PnC via DIN 70121.
    V2G_NOT_IMPL("DIN70121", body, PaymentDetailsReq);
    V2G_NOT_IMPL("DIN70121", body, CertificateInstallationReq);
    V2G_NOT_IMPL("DIN70121", body, CertificateUpdateReq);
    V2G_NOT_IMPL("DIN70121", body, MeteringReceiptReq);

    // Not needed for our SoC-read-only flow.
    V2G_NOT_IMPL("DIN70121", body, ChargingStatusReq);
    V2G_NOT_IMPL("DIN70121", body, WeldingDetectionReq);
}

// Fill a DC_EVSEStatus with safe "shutdown" values, used by all FAILED/shutdown
// responses and to satisfy the EXI schema in FAILED_UnknownSession responses.
// [V2G-DC-500] EVSENotification shall always be "None" for DC charging per DIN.
static void fill_dc_evse_status_shutdown(struct din_DC_EVSEStatusType *s)
{
    s->EVSEIsolationStatus_isUsed = 1;
    s->EVSEIsolationStatus = din_isolationLevelType_Invalid;
    s->EVSENotification = din_EVSENotificationType_None;
    s->NotificationMaxDelay = 0;
    s->EVSEStatusCode = din_DC_EVSEStatusCodeType_EVSE_Shutdown;
}

static void fill_dc_evse_status_ready(struct din_DC_EVSEStatusType *s)
{
    s->EVSEIsolationStatus_isUsed = 1;
    s->EVSEIsolationStatus = din_isolationLevelType_Invalid; // Invalid = an isolation test has not been carried out
    s->EVSENotification = din_EVSENotificationType_None;
    s->NotificationMaxDelay = 0;
    s->EVSEStatusCode = din_DC_EVSEStatusCodeType_EVSE_Ready;
}

static void fill_physical_value(struct din_PhysicalValueType *pv, int16_t value, int8_t multiplier, din_unitSymbolType unit)
{
    pv->Value = value;
    pv->Multiplier = multiplier;
    pv->Unit = unit;
    pv->Unit_isUsed = 1; // Unit is optional in the DIN schema, but we always provide it
}

// Fill a single-tuple SAScheduleList with the given PMax covering 24 hours.
// [V2G-DC-559] SAScheduleList shall provide a PMaxSchedule covering at least 24 hours.
// [V2G-DC-336] start: seconds from NOW. [V2G-DC-338] duration: period in seconds.
// [V2G-DC-554] SalesTariff shall not be used.
static void fill_sa_schedule_24h(struct din_SAScheduleListType *list, int16_t pmax)
{
    list->SAScheduleTuple.array[0].SAScheduleTupleID = V2G_SA_SCHEDULE_TUPLE_ID;
    list->SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleID = 1;
    list->SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax = pmax;
    list->SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.start = 0;
    list->SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration = SECONDS_PER_DAY;
    list->SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration_isUsed = 1;
    list->SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval_isUsed = 1;
    list->SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].TimeInterval_isUsed = 0;
    list->SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.arrayLen = 1;
    list->SAScheduleTuple.array[0].SalesTariff_isUsed = 0;
    list->SAScheduleTuple.arrayLen = 1;
}

void DIN70121::send_failed_unknown_session()
{
    auto &body_dec = dinDocDec->V2G_Message.Body;
    auto &body_enc = dinDocEnc->V2G_Message.Body;

    constexpr auto rc = din_responseCodeType_FAILED_UnknownSession;

    // Respond with the matching message type and populate all mandatory child fields.
    if (body_dec.ServiceDiscoveryReq_isUsed) {
        auto *res = &body_enc.ServiceDiscoveryRes;
        body_enc.ServiceDiscoveryRes_isUsed = 1;
        res->ResponseCode = rc;
        res->PaymentOptions.PaymentOption.array[0] = din_paymentOptionType_ExternalPayment;
        res->PaymentOptions.PaymentOption.arrayLen = 1;
        res->ChargeService.ServiceTag.ServiceID = V2G_SERVICE_ID_CHARGING;
        res->ChargeService.ServiceTag.ServiceCategory = din_serviceCategoryType_EVCharging;
        res->ChargeService.ServiceTag.ServiceName_isUsed = 0;
        res->ChargeService.ServiceTag.ServiceScope_isUsed = 0;
        res->ChargeService.FreeService = 1;
        res->ChargeService.EnergyTransferType = din_EVSESupportedEnergyTransferType_DC_extended;
        res->ServiceList_isUsed = 0;
    } else if (body_dec.ServicePaymentSelectionReq_isUsed) {
        body_enc.ServicePaymentSelectionRes_isUsed = 1;
        body_enc.ServicePaymentSelectionRes.ResponseCode = rc;
    } else if (body_dec.ContractAuthenticationReq_isUsed) {
        body_enc.ContractAuthenticationRes_isUsed = 1;
        body_enc.ContractAuthenticationRes.ResponseCode = rc;
        body_enc.ContractAuthenticationRes.EVSEProcessing = din_EVSEProcessingType_Finished;
    } else if (body_dec.ChargeParameterDiscoveryReq_isUsed) {
        auto *res = &body_enc.ChargeParameterDiscoveryRes;
        body_enc.ChargeParameterDiscoveryRes_isUsed = 1;
        res->ResponseCode = rc;
        res->EVSEProcessing = din_EVSEProcessingType_Finished;
        res->SAScheduleList_isUsed = 0;
        res->SASchedules_isUsed = 0;
        res->AC_EVSEChargeParameter_isUsed = 0;
        res->DC_EVSEChargeParameter_isUsed = 0;
        res->EVSEChargeParameter_isUsed = 0;
    } else if (body_dec.PowerDeliveryReq_isUsed) {
        auto *res = &body_enc.PowerDeliveryRes;
        body_enc.PowerDeliveryRes_isUsed = 1;
        res->ResponseCode = rc;
        res->AC_EVSEStatus_isUsed = 0;
        res->EVSEStatus_isUsed = 0;
        res->DC_EVSEStatus_isUsed = 1;
        fill_dc_evse_status_shutdown(&res->DC_EVSEStatus);
    } else if (body_dec.CableCheckReq_isUsed) {
        auto *res = &body_enc.CableCheckRes;
        body_enc.CableCheckRes_isUsed = 1;
        res->ResponseCode = rc;
        res->EVSEProcessing = din_EVSEProcessingType_Finished;
        fill_dc_evse_status_shutdown(&res->DC_EVSEStatus);
    } else if (body_dec.PreChargeReq_isUsed) {
        auto *res = &body_enc.PreChargeRes;
        body_enc.PreChargeRes_isUsed = 1;
        res->ResponseCode = rc;
        fill_dc_evse_status_shutdown(&res->DC_EVSEStatus);
        fill_physical_value(&res->EVSEPresentVoltage, 0, 0, din_unitSymbolType_V);
    } else if (body_dec.CurrentDemandReq_isUsed) {
        auto *res = &body_enc.CurrentDemandRes;
        body_enc.CurrentDemandRes_isUsed = 1;
        res->ResponseCode = rc;
        fill_dc_evse_status_shutdown(&res->DC_EVSEStatus);
        fill_physical_value(&res->EVSEPresentVoltage, 0, 0, din_unitSymbolType_V);
        fill_physical_value(&res->EVSEPresentCurrent, 0, 0, din_unitSymbolType_A);
        res->EVSECurrentLimitAchieved = 0;
        res->EVSEVoltageLimitAchieved = 0;
        res->EVSEPowerLimitAchieved = 0;
        res->EVSEMaximumVoltageLimit_isUsed = 0;
        res->EVSEMaximumCurrentLimit_isUsed = 0;
        res->EVSEMaximumPowerLimit_isUsed = 0;
    } else if (body_dec.SessionStopReq_isUsed) {
        body_enc.SessionStopRes_isUsed = 1;
        body_enc.SessionStopRes.ResponseCode = rc;
    } else {
        iso15118.trace("DIN70121: Unknown message type for FAILED_UnknownSession");
        return;
    }

    iso15118.common.send_exi(Common::ExiType::Din);
}

void DIN70121::handle_session_setup_req()
{
    din_SessionSetupReqType *req = &dinDocDec->V2G_Message.Body.SessionSetupReq;
    din_SessionSetupResType *res = &dinDocEnc->V2G_Message.Body.SessionSetupRes;

    // Reset SoC tracking for the new session
    soc_read = false;
    soc_shutdown_retries = 0;

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

    memcpy(res->EVSEID.bytes, iso15118.evseid_din, iso15118.evseid_din_len);
    res->EVSEID.bytesLen = iso15118.evseid_din_len;

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

    if (req->DC_EVChargeParameter_isUsed) {
        api_state.get("soc")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);

        iso15118.trace("DIN70121: Current SoC %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
    }

    // Update EV data: only SOC, all other values would be for DC charging.
#if MODULE_EV_AVAILABLE()
    if (req->DC_EVChargeParameter_isUsed) {
        ev.set_soc(static_cast<float>(req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC));
    }
    ev.session_updated(EVDataSource::DIN);
#endif

    dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1;

    // In read_soc_only mode: read SoC on first request, then signal EVSE_Shutdown on second request.
    // Also applies when charge_via_iso15118 is set: DIN 70121 is DC-only, so we can't do
    // AC charging via DIN. Read SoC if configured, then end session to fall back to IEC 61851.
    //
    // We use ResponseCode=OK with EVSEStatusCode=EVSE_Shutdown + EVSEProcessing=Ongoing.
    // Some EVs react to EVSE_Shutdown right away and send SessionStopReq.
    //
    // Other EVs (e.g. Tesla) ignore the EVSEStatusCode during ChargeParameterDiscovery
    // and keep polling until their internal Ongoing timeout expires.
    // To avoid this, after a few Ongoing+EVSE_Shutdown retries we escalate to
    // EVSEProcessing=Finished. The EV then either stops on the EVSE_Shutdown status or proceeds
    // to CableCheckReq, where we perform the official SECC-initiated stop per [V2G-DC-891]
    // (OK + Finished + EVSE_Shutdown), resulting in a graceful session teardown without an
    // EV-side error.
    if ((iso15118.is_read_soc_only() || iso15118.config.get("charge_via_iso15118")->asBool()) && soc_read) {
        if (soc_shutdown_retries < 255) {
            soc_shutdown_retries++;
        }

        // NOTE: We use OK here, while after 10 retries and FAILED in iso2. This is on purpose.
        //       The meaning is slightly different between the two. With iso2 a new charge can
        //       explicitely be started after FAILED, which is not true with din.
        const bool end_at_cpd = iso15118.is_read_soc_only() && iso15118.opt_nonegotiation_after_soc;

        res->ResponseCode = din_responseCodeType_OK;
        if (soc_shutdown_retries > 10 && !end_at_cpd) {
            iso15118.trace("DIN70121: SoC shutdown ignored after %d retries, sending Finished", soc_shutdown_retries);
            res->EVSEProcessing = din_EVSEProcessingType_Finished;
        } else {
            iso15118.trace("DIN70121: SoC already read, sending EVSE_Shutdown to end session");
            res->EVSEProcessing = din_EVSEProcessingType_Ongoing;
        }

        // Mandatory fields: DC_EVSEChargeParameter with valid DC_EVSEStatus and limits
        // must be present for a valid ChargeParameterDiscoveryRes EXI encoding.
        res->DC_EVSEChargeParameter_isUsed = 1;
        fill_dc_evse_status_shutdown(&res->DC_EVSEChargeParameter.DC_EVSEStatus);
        fill_physical_value(&res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit, 0, 0, din_unitSymbolType_A);
        fill_physical_value(&res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit, 0, 0, din_unitSymbolType_V);
        fill_physical_value(&res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit, 0, 0, din_unitSymbolType_A);
        fill_physical_value(&res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit, 0, 0, din_unitSymbolType_V);
        fill_physical_value(&res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple, 0, 0, din_unitSymbolType_A);
        // [V2G-DC-950] EVSEMaximumPowerLimit shall be used
        fill_physical_value(&res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit, 0, 0, din_unitSymbolType_W);
        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit_isUsed = 1;
        res->DC_EVSEChargeParameter.EVSECurrentRegulationTolerance_isUsed = 0;
        res->DC_EVSEChargeParameter.EVSEEnergyToBeDelivered_isUsed = 0;

        res->SAScheduleList_isUsed = 1;
        fill_sa_schedule_24h(&res->SAScheduleList, 0);
        res->SASchedules_isUsed = 0;
        res->EVSEChargeParameter_isUsed = 0;
        res->AC_EVSEChargeParameter_isUsed = 0;

        iso15118.common.send_exi(Common::ExiType::Din);
        state = DIN70121State::ChargeParameterDiscovery;

        if (soc_shutdown_retries == 11) {
            if (end_at_cpd && !iso15118.nonegotiation_pending) {
                iso15118.trace("DIN70121: SoC shutdown ignored after %d retries, ending session at ChargeParameterDiscovery", soc_shutdown_retries);
                iso15118.begin_reslac_for_nonegotiation();
            } else {
                iso15118.schedule_delayed_modem_off();
            }
        }
        return;
    }

    if (iso15118.is_read_soc_only() || iso15118.config.get("charge_via_iso15118")->asBool()) {
        soc_read = true;
    }

    dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1;

    // [V2G-DC-493] On OK + Ongoing, the EVCC ignores schedule/params and resends ChargeParameterDiscoveryReq.

    // Get EV to send ChargeParameterDiscoveryReq again by using EVSE_Ready with EVSEProcessingType_Ongoing
    res->ResponseCode = din_responseCodeType_OK;

    // [V2G-DC-863/864/1004] Ongoing starts a timer (~10s). If never Finished, EV stops the session.

    // TODO: Does [V2G-DC-863] + [V2G-DC-864] mean that we can only delay with EVSEProcessingType_Ongoing once?
    res->EVSEProcessing = din_EVSEProcessingType_Ongoing;

    // EVSE_Ready: The EVSE is ready for charging.
    fill_dc_evse_status_ready(&res->DC_EVSEChargeParameter.DC_EVSEStatus);

    // Mandatory charge parameters
    fill_physical_value(&res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit, DC_SOC_MAX_CURRENT_A, 0, din_unitSymbolType_A); // 500A
    fill_physical_value(&res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit, DC_SOC_MAX_VOLTAGE_V, 0, din_unitSymbolType_V); // 800V
    fill_physical_value(&res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit, 0, 0, din_unitSymbolType_A);
    fill_physical_value(&res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit, 0, 0, din_unitSymbolType_V);
    fill_physical_value(&res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple, DC_SOC_PEAK_RIPPLE_A, 0, din_unitSymbolType_A); // 1A
    fill_physical_value(&res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit, DC_SOC_MAX_POWER_VALUE, DC_SOC_MAX_POWER_EXP, din_unitSymbolType_W); // 20000W * 10^1 = 200kW
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit_isUsed = 1;

    res->DC_EVSEChargeParameter_isUsed = 1;

    // Optional charge parameters
    res->DC_EVSEChargeParameter.EVSECurrentRegulationTolerance_isUsed = 0;
    res->DC_EVSEChargeParameter.EVSEEnergyToBeDelivered_isUsed = 0;

    // [V2G-DC-559] Since for DC charging according to this document, the EVCC is not able to provide a planned
    // departure time, SAScheduleList shall provide PMaxSchedule (refer to 9.5.2.10) covering at least 24 hours.
    fill_sa_schedule_24h(&res->SAScheduleList, SHRT_MAX);
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

// Shared epilogue for handlers that end the session outside of SessionStop
void DIN70121::abort_soc_session()
{
    if (iso15118.is_read_soc_only() || iso15118.config.get("charge_via_iso15118")->asBool()) {
        cancel_sequence_timeout(next_timeout);
        iso15118.schedule_delayed_modem_off();
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
    fill_dc_evse_status_shutdown(&res->DC_EVSEStatus);

    res->AC_EVSEStatus_isUsed = 0;
    res->EVSEStatus_isUsed = 0;

    iso15118.trace("DIN70121: PowerDeliveryReq (ReadyToChargeState=%d), responding OK with EVSE_Shutdown",
                     req->ReadyToChargeState);

    iso15118.common.send_exi(Common::ExiType::Din);
    state = DIN70121State::PowerDelivery;

    abort_soc_session();
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

    fill_dc_evse_status_shutdown(&res->DC_EVSEStatus);

    iso15118.trace("DIN70121: CableCheckReq received in SoC-read flow, sending EVSE_Shutdown to terminate");

    iso15118.common.send_exi(Common::ExiType::Din);
    state = DIN70121State::CableCheck;

    abort_soc_session();
}

void DIN70121::handle_pre_charge_req()
{
    din_PreChargeResType *res = &dinDocEnc->V2G_Message.Body.PreChargeRes;

    dinDocEnc->V2G_Message.Body.PreChargeRes_isUsed = 1;

    // Safety net: The EV sent PreChargeReq despite our shutdown signals in
    // ChargeParameterDiscoveryRes and CableCheckRes. This is non-compliant EV behavior.
    // [V2G-DC-901] On Finished + FAILED, the EVCC shall stop the charging session.
    res->ResponseCode = din_responseCodeType_FAILED;

    fill_dc_evse_status_shutdown(&res->DC_EVSEStatus);
    fill_physical_value(&res->EVSEPresentVoltage, 0, 0, din_unitSymbolType_V);

    iso15118.trace("DIN70121: PreChargeReq received in SoC-read flow, sending FAILED to terminate");

    iso15118.common.send_exi(Common::ExiType::Din);

    abort_soc_session();
}

void DIN70121::handle_current_demand_req()
{
    din_CurrentDemandResType *res = &dinDocEnc->V2G_Message.Body.CurrentDemandRes;

    dinDocEnc->V2G_Message.Body.CurrentDemandRes_isUsed = 1;

    // Safety net: The EV should never reach CurrentDemand in a SoC-read-only flow.
    // [V2G-DC-901] On Finished + FAILED, the EVCC shall stop the charging session.
    res->ResponseCode = din_responseCodeType_FAILED;

    fill_dc_evse_status_shutdown(&res->DC_EVSEStatus);
    fill_physical_value(&res->EVSEPresentVoltage, 0, 0, din_unitSymbolType_V);
    fill_physical_value(&res->EVSEPresentCurrent, 0, 0, din_unitSymbolType_A);

    res->EVSECurrentLimitAchieved = 0;
    res->EVSEVoltageLimitAchieved = 0;
    res->EVSEPowerLimitAchieved = 0;

    res->EVSEMaximumVoltageLimit_isUsed = 0;
    res->EVSEMaximumCurrentLimit_isUsed = 0;
    res->EVSEMaximumPowerLimit_isUsed = 0;

    iso15118.trace("DIN70121: CurrentDemandReq received in SoC-read flow, sending FAILED to terminate");

    iso15118.common.send_exi(Common::ExiType::Din);

    abort_soc_session();
}

void DIN70121::handle_session_stop_req()
{
    din_SessionStopResType *res = &dinDocEnc->V2G_Message.Body.SessionStopRes;

    dinDocEnc->V2G_Message.Body.SessionStopRes_isUsed = 1;
    res->ResponseCode = din_responseCodeType_OK;

    iso15118.common.send_exi(Common::ExiType::Din);
    state = DIN70121State::SessionStop;

    // DIN 70121 is DC-only, so we always fall back to IEC after the session.
    if (iso15118.is_read_soc_only() || iso15118.config.get("charge_via_iso15118")->asBool()) {
        if (iso15118.end_hlc_after_session_stop(next_timeout)) {
            // Re-SLAC round started. Reset for the second session.
            reset_session();
        }
    }
}

void DIN70121::trace_header(const struct din_MessageHeaderType *header, const char *name)
{
    trace_iso("V2G_Message (%s)", name);
    trace_iso(" Header");
    trace_iso("  SessionID.bytes: %02x%02x%02x%02x%02x%02x%02x%02x",
              header->SessionID.bytes[0], header->SessionID.bytes[1], header->SessionID.bytes[2], header->SessionID.bytes[3],
              header->SessionID.bytes[4], header->SessionID.bytes[5], header->SessionID.bytes[6], header->SessionID.bytes[7]);
    trace_iso("  SessionID.bytesLen: %d", header->SessionID.bytesLen);
    trace_iso("  Notification_isUsed: %d", header->Notification_isUsed);
    if (header->Notification_isUsed) {
        trace_iso("  Notification.FaultCode: %d", header->Notification.FaultCode);
        trace_iso("  Notification.FaultMsg_isUsed: %d", header->Notification.FaultMsg_isUsed);
        if (header->Notification.FaultMsg_isUsed) {
            trace_iso("  Notification.FaultMsg.characters: %s", header->Notification.FaultMsg.characters);
            trace_iso("  Notification.FaultMsg.charactersLen: %d", header->Notification.FaultMsg.charactersLen);
        }
    }
    trace_iso("  Signature_isUsed: %d", header->Signature_isUsed);
}

void DIN70121::trace_request_response()
{
    // Trace requests
    if (dinDocDec->V2G_Message.Body.SessionSetupReq_isUsed) {
        din_SessionSetupReqType *req = &dinDocDec->V2G_Message.Body.SessionSetupReq;

        trace_header(&dinDocDec->V2G_Message.Header, "SessionSetup Request");
        trace_iso(" Body");
        trace_iso("  SessionSetupReq");
        trace_iso("   EVCCID: %02x%02x%02x%02x%02x%02x%02x%02x",
                       req->EVCCID.bytes[0], req->EVCCID.bytes[1], req->EVCCID.bytes[2], req->EVCCID.bytes[3],
                       req->EVCCID.bytes[4], req->EVCCID.bytes[5], req->EVCCID.bytes[6], req->EVCCID.bytes[7]);
        trace_iso("   EVCCID.bytesLen: %d", req->EVCCID.bytesLen);
    } else if (dinDocDec->V2G_Message.Body.ServiceDiscoveryReq_isUsed) {
        din_ServiceDiscoveryReqType *req = &dinDocDec->V2G_Message.Body.ServiceDiscoveryReq;

        trace_header(&dinDocDec->V2G_Message.Header, "ServiceDiscovery Request");
        trace_iso(" Body");
        trace_iso("  ServiceDiscoveryReq");
        trace_iso("   ServiceCategory_isUsed: %d", req->ServiceCategory_isUsed);
        if (req->ServiceCategory_isUsed) {
            trace_iso("   ServiceCategory: %d", req->ServiceCategory);
        }
        trace_iso("   ServiceScope_isUsed: %d", req->ServiceScope_isUsed);
        if (req->ServiceScope_isUsed) {
            trace_iso("   ServiceScope: %s", req->ServiceScope.characters);
        }
    } else if (dinDocDec->V2G_Message.Body.ServicePaymentSelectionReq_isUsed) {
        din_ServicePaymentSelectionReqType *req = &dinDocDec->V2G_Message.Body.ServicePaymentSelectionReq;

        trace_header(&dinDocDec->V2G_Message.Header, "ServicePaymentSelection Request");
        trace_iso(" Body");
        trace_iso("  ServicePaymentSelectionReq");
        trace_iso("   SelectedPaymentOption: %d", req->SelectedPaymentOption);
        for (uint16_t i = 0; i < req->SelectedServiceList.SelectedService.arrayLen; i++) {
            trace_iso("   SelectedService[%d]", i);
            trace_iso("    ServiceID: %d", req->SelectedServiceList.SelectedService.array[i].ServiceID);
            trace_iso("    ParameterSetID_isUsed: %d", req->SelectedServiceList.SelectedService.array[i].ParameterSetID_isUsed);
            if (req->SelectedServiceList.SelectedService.array[i].ParameterSetID_isUsed) {
                trace_iso("     ParameterSetID: %d", req->SelectedServiceList.SelectedService.array[i].ParameterSetID);
            }
        }
    } else if (dinDocDec->V2G_Message.Body.ContractAuthenticationReq_isUsed) {
        trace_header(&dinDocDec->V2G_Message.Header, "ContractAuthentication Request");
        trace_iso(" Body");
        trace_iso("  ContractAuthenticationReq");
        // [V2G-DC-550] In the scope of this document, the element "GenChallenge" shall not be used.
        // [V2G-DC-545] In the scope of this document, the element "Id" shall not be used.
    } else if (dinDocDec->V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed) {
        din_ChargeParameterDiscoveryReqType *req = &dinDocDec->V2G_Message.Body.ChargeParameterDiscoveryReq;

        trace_header(&dinDocDec->V2G_Message.Header, "ChargeParameterDiscovery Request");
        trace_iso(" Body");
        trace_iso("  ChargeParameterDiscoveryReq");
        trace_iso("   EVRequestedEnergyTransferType: %d", req->EVRequestedEnergyTransferType);
        trace_iso("   DC_EVChargeParameter_isUsed: %d", req->DC_EVChargeParameter_isUsed);
        if (req->DC_EVChargeParameter_isUsed) {
            trace_iso("    DC_EVStatus.EVRESSSOC: %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
            trace_iso("    DC_EVStatus.EVReady: %d", req->DC_EVChargeParameter.DC_EVStatus.EVReady);
            trace_iso("    DC_EVStatus.EVCabinConditioning: %d", req->DC_EVChargeParameter.DC_EVStatus.EVCabinConditioning);
            trace_iso("    DC_EVStatus.EVCabinConditioning_isUsed: %d", req->DC_EVChargeParameter.DC_EVStatus.EVCabinConditioning_isUsed);
            trace_iso("    DC_EVStatus.EVRESSConditioning: %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSConditioning);
            trace_iso("    DC_EVStatus.EVRESSConditioning_isUsed: %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSConditioning_isUsed);
            trace_iso("    DC_EVStatus.EVErrorCode: %d", req->DC_EVChargeParameter.DC_EVStatus.EVErrorCode);
            trace_iso("    EVMaximumCurrentLimit.Value: %d", req->DC_EVChargeParameter.EVMaximumCurrentLimit.Value);
            trace_iso("    EVMaximumCurrentLimit.Multiplier: %d", req->DC_EVChargeParameter.EVMaximumCurrentLimit.Multiplier);
            trace_iso("    EVMaximumPowerLimit.Value: %d", req->DC_EVChargeParameter.EVMaximumPowerLimit.Value);
            trace_iso("    EVMaximumPowerLimit.Multiplier: %d", req->DC_EVChargeParameter.EVMaximumPowerLimit.Multiplier);
            trace_iso("    EVMaximumPowerLimit_isUsed: %d", req->DC_EVChargeParameter.EVMaximumPowerLimit_isUsed);
            trace_iso("    EVMaximumVoltageLimit.Value: %d", req->DC_EVChargeParameter.EVMaximumVoltageLimit.Value);
            trace_iso("    EVMaximumVoltageLimit.Multiplier: %d", req->DC_EVChargeParameter.EVMaximumVoltageLimit.Multiplier);
            trace_iso("    EVEnergyCapacity.Value: %d", req->DC_EVChargeParameter.EVEnergyCapacity.Value);
            trace_iso("    EVEnergyCapacity.Multiplier: %d", req->DC_EVChargeParameter.EVEnergyCapacity.Multiplier);
            trace_iso("    EVEnergyCapacity_isUsed: %d", req->DC_EVChargeParameter.EVEnergyCapacity_isUsed);
            trace_iso("    EVEnergyRequest.Value: %d", req->DC_EVChargeParameter.EVEnergyRequest.Value);
            trace_iso("    EVEnergyRequest.Multiplier: %d", req->DC_EVChargeParameter.EVEnergyRequest.Multiplier);
            trace_iso("    EVEnergyRequest_isUsed: %d", req->DC_EVChargeParameter.EVEnergyRequest_isUsed);
            trace_iso("    FullSOC: %d", req->DC_EVChargeParameter.FullSOC);
            trace_iso("    FullSOC_isUsed: %d", req->DC_EVChargeParameter.FullSOC_isUsed);
            trace_iso("    BulkSOC: %d", req->DC_EVChargeParameter.BulkSOC);
            trace_iso("    BulkSOC_isUsed: %d", req->DC_EVChargeParameter.BulkSOC_isUsed);
        }
    } else if (dinDocDec->V2G_Message.Body.CableCheckReq_isUsed) {
        din_CableCheckReqType *req = &dinDocDec->V2G_Message.Body.CableCheckReq;

        trace_header(&dinDocDec->V2G_Message.Header, "CableCheck Request");
        trace_iso(" Body");
        trace_iso("  CableCheckReq");
        trace_iso("   DC_EVStatus.EVRESSSOC: %d", req->DC_EVStatus.EVRESSSOC);
        trace_iso("   DC_EVStatus.EVReady: %d", req->DC_EVStatus.EVReady);
        trace_iso("   DC_EVStatus.EVErrorCode: %d", req->DC_EVStatus.EVErrorCode);
    } else if (dinDocDec->V2G_Message.Body.PowerDeliveryReq_isUsed) {
        din_PowerDeliveryReqType *req = &dinDocDec->V2G_Message.Body.PowerDeliveryReq;

        trace_header(&dinDocDec->V2G_Message.Header, "PowerDelivery Request");
        trace_iso(" Body");
        trace_iso("  PowerDeliveryReq");
        trace_iso("   ReadyToChargeState: %d", req->ReadyToChargeState);
        trace_iso("   DC_EVPowerDeliveryParameter_isUsed: %d", req->DC_EVPowerDeliveryParameter_isUsed);
        if (req->DC_EVPowerDeliveryParameter_isUsed) {
            trace_iso("    DC_EVStatus.EVRESSSOC: %d", req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVRESSSOC);
            trace_iso("    DC_EVStatus.EVReady: %d", req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVReady);
            trace_iso("    DC_EVStatus.EVErrorCode: %d", req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVErrorCode);
        }
    } else if (dinDocDec->V2G_Message.Body.PreChargeReq_isUsed) {
        din_PreChargeReqType *req = &dinDocDec->V2G_Message.Body.PreChargeReq;

        trace_header(&dinDocDec->V2G_Message.Header, "PreCharge Request");
        trace_iso(" Body");
        trace_iso("  PreChargeReq");
        trace_iso("   DC_EVStatus.EVRESSSOC: %d", req->DC_EVStatus.EVRESSSOC);
        trace_iso("   DC_EVStatus.EVReady: %d", req->DC_EVStatus.EVReady);
        trace_iso("   DC_EVStatus.EVErrorCode: %d", req->DC_EVStatus.EVErrorCode);
        trace_iso("   EVTargetVoltage.Value: %d", req->EVTargetVoltage.Value);
        trace_iso("   EVTargetVoltage.Multiplier: %d", req->EVTargetVoltage.Multiplier);
        trace_iso("   EVTargetCurrent.Value: %d", req->EVTargetCurrent.Value);
        trace_iso("   EVTargetCurrent.Multiplier: %d", req->EVTargetCurrent.Multiplier);
    } else if (dinDocDec->V2G_Message.Body.CurrentDemandReq_isUsed) {
        din_CurrentDemandReqType *req = &dinDocDec->V2G_Message.Body.CurrentDemandReq;

        trace_header(&dinDocDec->V2G_Message.Header, "CurrentDemand Request");
        trace_iso(" Body");
        trace_iso("  CurrentDemandReq");
        trace_iso("   DC_EVStatus.EVRESSSOC: %d", req->DC_EVStatus.EVRESSSOC);
        trace_iso("   DC_EVStatus.EVReady: %d", req->DC_EVStatus.EVReady);
        trace_iso("   DC_EVStatus.EVErrorCode: %d", req->DC_EVStatus.EVErrorCode);
        trace_iso("   EVTargetVoltage.Value: %d", req->EVTargetVoltage.Value);
        trace_iso("   EVTargetVoltage.Multiplier: %d", req->EVTargetVoltage.Multiplier);
        trace_iso("   EVTargetCurrent.Value: %d", req->EVTargetCurrent.Value);
        trace_iso("   EVTargetCurrent.Multiplier: %d", req->EVTargetCurrent.Multiplier);
        trace_iso("   ChargingComplete: %d", req->ChargingComplete);
    } else if (dinDocDec->V2G_Message.Body.SessionStopReq_isUsed) {
        trace_header(&dinDocDec->V2G_Message.Header, "SessionStop Request");
        trace_iso(" Body");
        trace_iso("  SessionStopReq");
    }

    // Trace responses
    if (dinDocEnc->V2G_Message.Body.SessionSetupRes_isUsed) {
        din_SessionSetupResType *res = &dinDocEnc->V2G_Message.Body.SessionSetupRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "SessionSetup Response");
        trace_iso(" Body");
        trace_iso("  SessionSetupRes");
        trace_iso("   ResponseCode: %d", res->ResponseCode);
        trace_iso("   EVSEID: %02x", res->EVSEID.bytes[0]);
        trace_iso("   EVSEID.bytesLen: %d", res->EVSEID.bytesLen);
        trace_iso("   DateTimeNow_isUsed: %d", res->DateTimeNow_isUsed);
    } else if (dinDocEnc->V2G_Message.Body.ServiceDiscoveryRes_isUsed) {
        din_ServiceDiscoveryResType *res = &dinDocEnc->V2G_Message.Body.ServiceDiscoveryRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "ServiceDiscovery Response");
        trace_iso(" Body");
        trace_iso("  ServiceDiscoveryRes");
        trace_iso("   ResponseCode: %d", res->ResponseCode);
        trace_iso("   PaymentOptions.PaymentOption: %d", res->PaymentOptions.PaymentOption.array[0]);
        trace_iso("   ChargeService.ServiceTag.ServiceID: %d", res->ChargeService.ServiceTag.ServiceID);
        trace_iso("   ChargeService.ServiceTag.ServiceCategory: %d", res->ChargeService.ServiceTag.ServiceCategory);
        trace_iso("   ChargeService.FreeService: %d", res->ChargeService.FreeService);
        trace_iso("   ChargeService.EnergyTransferType: %d", res->ChargeService.EnergyTransferType);
    } else if (dinDocEnc->V2G_Message.Body.ServicePaymentSelectionRes_isUsed) {
        din_ServicePaymentSelectionResType *res = &dinDocEnc->V2G_Message.Body.ServicePaymentSelectionRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "ServicePaymentSelection Response");
        trace_iso(" Body");
        trace_iso("  ServicePaymentSelectionRes");
        trace_iso("   ResponseCode: %d", res->ResponseCode);
    } else if (dinDocEnc->V2G_Message.Body.ContractAuthenticationRes_isUsed) {
        din_ContractAuthenticationResType *res = &dinDocEnc->V2G_Message.Body.ContractAuthenticationRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "ContractAuthentication Response");
        trace_iso(" Body");
        trace_iso("  ContractAuthenticationRes");
        trace_iso("   ResponseCode: %d", res->ResponseCode);
        trace_iso("   EVSEProcessing: %d", res->EVSEProcessing);
    } else if (dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed) {
        din_ChargeParameterDiscoveryResType *res = &dinDocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "ChargeParameterDiscovery Response");
        trace_iso(" Body");
        trace_iso("  ChargeParameterDiscoveryRes");
        trace_iso("   ResponseCode: %d", res->ResponseCode);
        trace_iso("   EVSEProcessing: %d", res->EVSEProcessing);
        trace_iso("   DC_EVSEChargeParameter_isUsed: %d", res->DC_EVSEChargeParameter_isUsed);
        if (res->DC_EVSEChargeParameter_isUsed) {
            trace_iso("    DC_EVSEStatus.EVSEIsolationStatus: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus);
            trace_iso("    DC_EVSEStatus.EVSEIsolationStatus_isUsed: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed);
            trace_iso("    DC_EVSEStatus.EVSENotification: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification);
            trace_iso("    DC_EVSEStatus.NotificationMaxDelay: %lu", res->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay);
            trace_iso("    DC_EVSEStatus.EVSEStatusCode: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode);
            trace_iso("    EVSEMaximumCurrentLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value);
            trace_iso("    EVSEMaximumCurrentLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier);
            trace_iso("    EVSEMaximumVoltageLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value);
            trace_iso("    EVSEMaximumVoltageLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Multiplier);
            trace_iso("    EVSEMinimumCurrentLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Value);
            trace_iso("    EVSEMinimumCurrentLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Multiplier);
            trace_iso("    EVSEMinimumVoltageLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Value);
            trace_iso("    EVSEMinimumVoltageLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Multiplier);
            trace_iso("    EVSEPeakCurrentRipple.Value: %d", res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Value);
            trace_iso("    EVSEPeakCurrentRipple.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Multiplier);
            trace_iso("    EVSEMaximumPowerLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Value);
            trace_iso("    EVSEMaximumPowerLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Multiplier);
            trace_iso("    EVSEMaximumPowerLimit_isUsed: %d", res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit_isUsed);
        }
    } else if (dinDocEnc->V2G_Message.Body.CableCheckRes_isUsed) {
        din_CableCheckResType *res = &dinDocEnc->V2G_Message.Body.CableCheckRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "CableCheck Response");
        trace_iso(" Body");
        trace_iso("  CableCheckRes");
        trace_iso("   ResponseCode: %d", res->ResponseCode);
        trace_iso("   EVSEProcessing: %d", res->EVSEProcessing);
        trace_iso("   DC_EVSEStatus.EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
        trace_iso("   DC_EVSEStatus.EVSEIsolationStatus_isUsed: %d", res->DC_EVSEStatus.EVSEIsolationStatus_isUsed);
        trace_iso("   DC_EVSEStatus.EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
        trace_iso("   DC_EVSEStatus.EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
    } else if (dinDocEnc->V2G_Message.Body.PowerDeliveryRes_isUsed) {
        din_PowerDeliveryResType *res = &dinDocEnc->V2G_Message.Body.PowerDeliveryRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "PowerDelivery Response");
        trace_iso(" Body");
        trace_iso("  PowerDeliveryRes");
        trace_iso("   ResponseCode: %d", res->ResponseCode);
        trace_iso("   DC_EVSEStatus_isUsed: %d", res->DC_EVSEStatus_isUsed);
        if (res->DC_EVSEStatus_isUsed) {
            trace_iso("   DC_EVSEStatus.EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
            trace_iso("   DC_EVSEStatus.EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
            trace_iso("   DC_EVSEStatus.EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
        }
    } else if (dinDocEnc->V2G_Message.Body.PreChargeRes_isUsed) {
        din_PreChargeResType *res = &dinDocEnc->V2G_Message.Body.PreChargeRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "PreCharge Response");
        trace_iso(" Body");
        trace_iso("  PreChargeRes");
        trace_iso("   ResponseCode: %d", res->ResponseCode);
        trace_iso("   DC_EVSEStatus.EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
        trace_iso("   DC_EVSEStatus.EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
        trace_iso("   DC_EVSEStatus.EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
        trace_iso("   EVSEPresentVoltage.Value: %d", res->EVSEPresentVoltage.Value);
        trace_iso("   EVSEPresentVoltage.Multiplier: %d", res->EVSEPresentVoltage.Multiplier);
    } else if (dinDocEnc->V2G_Message.Body.CurrentDemandRes_isUsed) {
        din_CurrentDemandResType *res = &dinDocEnc->V2G_Message.Body.CurrentDemandRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "CurrentDemand Response");
        trace_iso(" Body");
        trace_iso("  CurrentDemandRes");
        trace_iso("   ResponseCode: %d", res->ResponseCode);
        trace_iso("   DC_EVSEStatus.EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
        trace_iso("   DC_EVSEStatus.EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
        trace_iso("   DC_EVSEStatus.EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
        trace_iso("   EVSEPresentVoltage.Value: %d", res->EVSEPresentVoltage.Value);
        trace_iso("   EVSEPresentCurrent.Value: %d", res->EVSEPresentCurrent.Value);
    } else if (dinDocEnc->V2G_Message.Body.SessionStopRes_isUsed) {
        din_SessionStopResType *res = &dinDocEnc->V2G_Message.Body.SessionStopRes;

        trace_header(&dinDocEnc->V2G_Message.Header, "SessionStop Response");
        trace_iso(" Body");
        trace_iso("  SessionStopRes");
        trace_iso("   ResponseCode: %d", res->ResponseCode);
    }
}
