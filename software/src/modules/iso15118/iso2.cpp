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

// IPv6/TCP/ISO-15118-2
// This implementation follows the requirements from ISO 15118-2:2014.
// Requirements are referenced using [V2G2-xxx] notation.

#include "iso2.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools/malloc.h"
#include "bindings/bricklet_evse_v2.h"

#include "gcc_warnings.h"

void ISO2::pre_setup()
{
    api_state = Config::Object({
        {"state", Config::Enum(ISO2State::Idle)},
        {"session_id", Config::Tuple(4, Config::Uint8(0))},
        {"evcc_id", Config::Array({}, Config::get_prototype_uint8_0(), 0, 8, Config::type_id<Config::ConfUint>())},
        {"soc", Config::Int8(-1)},
        {"energy_capacity", Config::Int32(-1)},
    });
}

void ISO2::handle_bitstream(exi_bitstream *exi)
{
    // Increment state on first call
    if (state == ISO2State::Idle) {
        state = ISO2State::BitstreamReceived;
    }

    // We alloc the iso2 buffers the very first time they are used.
    // This way it is not allocated if ISO15118 is not used.
    // If it is used once we can assume that it will be used all the time, so it stays allocated.
    if (iso2DocDec == nullptr) {
        iso2DocDec = static_cast<struct iso2_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct iso2_exiDocument)));
    }
    if (iso2DocEnc == nullptr) {
        iso2DocEnc = static_cast<struct iso2_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct iso2_exiDocument)));
    }
    memset(iso2DocDec, 0, sizeof(struct iso2_exiDocument));
    memset(iso2DocEnc, 0, sizeof(struct iso2_exiDocument));
    int ret = decode_iso2_exiDocument(exi, iso2DocDec);
    if (ret != 0) {
        logger.printfln("ISO2: Could not decode EXI document: %d", ret);
        return;
    }

    cancel_sequence_timeout(next_timeout);

    dispatch_messages();

    trace_request_response();

    api_state.get("state")->updateEnum(state);

    // [V2G2-443] The SECC shall stop waiting for a request message and stop monitoring the
    //            V2G_SECC_Sequence_Timer when V2G_SECC_Sequence_Timer is equal or larger than
    //            V2G_SECC_Sequence_Timeout and no request message was received. It shall then stop the
    //            V2G Communication Session.
    if (!pause_active) {
        schedule_sequence_timeout(next_timeout, ISO2_SECC_SEQUENCE_TIMEOUT, "ISO2");
    // [V2G2-725] If the SECC received the message SessionStopReq with parameter ChargingSession equal to
    //            "Pause" it shall pause the Data-Link (D-LINK_PAUSE.request()) after sending the message
    //            SessionStopRes and continue with [V2G2-721].
    } else {
        // No timeout in case auf pausing by EVCC
        // We just wait for new SLAC or new SDP message
        cancel_sequence_timeout(next_timeout);
    }
}

void ISO2::dispatch_messages()
{
    auto &body = iso2DocDec->V2G_Message.Body;

    // SessionSetupReq - no session validation needed (session is established here)
    V2G_DISPATCH("ISO2", body, SessionSetupReq, handle_session_setup_req);
    if (body.SessionSetupReq_isUsed) return;

    // [V2G2-741] All messages after SessionSetup require session ID validation.
    // If the SessionID received does not match the previously communicated SessionID,
    // the SECC shall respond with FAILED_UnknownSession.
    if (!validate_session_id(iso2DocDec->V2G_Message.Header.SessionID.bytes,
                             iso2DocDec->V2G_Message.Header.SessionID.bytesLen,
                             iso15118.common.session_id,
                             SESSION_ID_LENGTH)) {
        logger.printfln("ISO2: Session ID mismatch, sending FAILED_UnknownSession");
        send_failed_unknown_session();
        return;
    }

    // Implemented message handlers (session already validated)
    V2G_DISPATCH("ISO2", body, ServiceDiscoveryReq,         handle_service_discovery_req);
    V2G_DISPATCH("ISO2", body, PaymentServiceSelectionReq,  handle_payment_service_selection_req);
    V2G_DISPATCH("ISO2", body, AuthorizationReq,            handle_authorization_req);
    V2G_DISPATCH("ISO2", body, ChargeParameterDiscoveryReq, handle_charge_parameter_discovery_req);
    V2G_DISPATCH("ISO2", body, PowerDeliveryReq,            handle_power_delivery_req);
    V2G_DISPATCH("ISO2", body, ChargingStatusReq,           handle_charging_status_req);
    V2G_DISPATCH("ISO2", body, SessionStopReq,              handle_session_stop_req);
    V2G_DISPATCH("ISO2", body, CableCheckReq,               handle_cable_check_req);

    // Not yet implemented

    // This can return VAS (Value Added Services).
    // As far as i can tell this is not used in practice.
    V2G_NOT_IMPL("ISO2", body, ServiceDetailReq);

    // These are for Plug&Charge. In practice PnC is mostly
    // done via ISO 15118-20, we don't support PnC via ISO 15118-2.
    V2G_NOT_IMPL("ISO2", body, PaymentDetailsReq);
    V2G_NOT_IMPL("ISO2", body, CertificateInstallationReq);
    V2G_NOT_IMPL("ISO2", body, CertificateUpdateReq);
    V2G_NOT_IMPL("ISO2", body, MeteringReceiptReq);

    // These are for DC charging only.
    // We will not support them.
    V2G_NOT_IMPL("ISO2", body, PreChargeReq);
    V2G_NOT_IMPL("ISO2", body, CurrentDemandReq);
    V2G_NOT_IMPL("ISO2", body, WeldingDetectionReq);
}

void ISO2::send_failed_unknown_session()
{
    auto &body_dec = iso2DocDec->V2G_Message.Body;
    auto &body_enc = iso2DocEnc->V2G_Message.Body;

    // Determine which message type was received and send the appropriate error response
    if (false
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, ServiceDiscovery,         iso2_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, PaymentServiceSelection,  iso2_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, Authorization,            iso2_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, ChargeParameterDiscovery, iso2_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, PowerDelivery,            iso2_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, ChargingStatus,           iso2_responseCodeType_FAILED_UnknownSession))
        || (V2G_SEND_FAILED_SESSION(body_dec, body_enc, SessionStop,              iso2_responseCodeType_FAILED_UnknownSession))
    ) {
        iso15118.common.send_exi(Common::ExiType::Iso2);
        return;
    }

    logger.printfln("ISO2: Unknown message type for FAILED_UnknownSession");
}

void ISO2::handle_session_setup_req()
{
    iso2_SessionSetupReqType *req = &iso2DocDec->V2G_Message.Body.SessionSetupReq;
    iso2_SessionSetupResType *res = &iso2DocEnc->V2G_Message.Body.SessionSetupRes;

    // Reset soc_read flag for new session
    soc_read = false;

    api_state.get("evcc_id")->removeAll();
    for (uint16_t i = 0; i < std::min(static_cast<uint16_t>(sizeof(req->EVCCID.bytes)), req->EVCCID.bytesLen); i++) {
        api_state.get("evcc_id")->add()->updateUint(req->EVCCID.bytes[i]);
    }

    // [V2G2-750] When receiving the SessionSetupReq with the parameter SessionID equal to zero (0), the
    //            SECC shall generate a new (not stored) SessionID value different from zero (0) and return this
    //            value in the SessionSetupRes message header.
    // [V2G2-753] If an EVCC chooses to resume a charging session by sending a SesstionSetupReq with a
    //            message header including the SessionID value from the previously paused V2G
    //            Communication Session, the SECC shall compare this value to the value stored from the
    //            preceding V2G Communication Session.
    SessionIdResult result = check_session_id(
        iso2DocDec->V2G_Message.Header.SessionID.bytes,
        iso2DocDec->V2G_Message.Header.SessionID.bytesLen,
        iso15118.common.session_id,
        SESSION_ID_LENGTH
    );

    if (result == SessionIdResult::NewSession) {
        // [V2G2-462] The message 'SessionSetupRes' shall contain the specific ResponseCode
        //            'OK_NewSessionEstablished' if processing of the SessionSetupReq message was successful
        //            and a different SessionID is contained in the response message than the SessionID in the
        //            request message.
        res->ResponseCode = iso2_responseCodeType_OK_NewSessionEstablished;
    } else {
        // [V2G2-754] If the SessionID value received in the current SessionSetupReq is equal to the value stored
        //            from the preceding V2G Communication Session, the SECC shall confirm the continuation of
        //            the charging session by sending a SessionSetupRes message including the stored SessionID
        //            value and indicating the resumed V2G Communication Session with the ResponseCode set to
        //            "OK_OldSessionJoined" (refer also to [V2G2-463] for selecting the appropriate response
        //            code).
        res->ResponseCode = iso2_responseCodeType_OK_OldSessionJoined;
    }

    for (uint16_t i = 0; i < SESSION_ID_LENGTH; i++) {
        api_state.get("session_id")->get(i)->updateUint(iso15118.common.session_id[i]);
    }

    iso2DocEnc->V2G_Message.Body.SessionSetupRes_isUsed = 1;

    // The EVSEID shall match the following structure (the notation corresponds to the augmented Backus-Naur
    // Form (ABNF) as defined in IETF RFC 5234):
    // <EVSEID> = <Country Code> <S> <EVSE Operator ID> <S> <ID Type> <Power Outlet ID>

    // If an SECC cannot provide such ID data, the value of the EVSEID is set to zero ("ZZ00000").
    strcpy(res->EVSEID.characters, "ZZ00000");
    res->EVSEID.charactersLen = strlen("ZZ00000");
    // ^ note: This is completely different to EVSEID syntax in DIN SPEC 70121

    // Timestamp of the current SECC time. Format is “Unix Time Stamp”.
    // This message element may be used by the
    // EVCC to decide whether a specific contract
    // certificate can be used for contract based
    // charging during the current communication session.
    timeval now;
    if (!rtc.clock_synced(&now)) {
        now.tv_sec = 0;
    }
    if (now.tv_sec != 0) {
        res->EVSETimeStamp_isUsed = 1;
        res->EVSETimeStamp = now.tv_sec;
    } else {
        res->EVSETimeStamp_isUsed = 0;
    }

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = ISO2State::SessionSetup;
}

void ISO2::handle_service_discovery_req()
{
    iso2_ServiceDiscoveryReqType *req = &iso2DocDec->V2G_Message.Body.ServiceDiscoveryReq;
    iso2_ServiceDiscoveryResType *res = &iso2DocEnc->V2G_Message.Body.ServiceDiscoveryRes;

    if (req->ServiceCategory_isUsed) {
        switch (req->ServiceCategory) {
            case iso2_serviceCategoryType_EVCharging:
                break;
            case iso2_serviceCategoryType_Internet:
                logger.printfln("ISO2: ServiceCategory Internet unsupported");
                break;
            case iso2_serviceCategoryType_ContractCertificate:
                logger.printfln("ISO2: ServiceCategory ContractCertificate unsupported");
                break;
            case iso2_serviceCategoryType_OtherCustom:
                logger.printfln("ISO2: ServiceCategory OtherCustom unsupported");
                break;
            default:
                logger.printfln("ISO2: ServiceCategory Unknown");
                break;
        }
    }

    // We just assume EVCharging as service here

    iso2DocEnc->V2G_Message.Body.ServiceDiscoveryRes_isUsed = 1;
    res->ResponseCode = iso2_responseCodeType_OK;

    // One payment option: EVSE handles payment
    res->PaymentOptionList.PaymentOption.array[0] = iso2_paymentOptionType_ExternalPayment;
    res->PaymentOptionList.PaymentOption.arrayLen = 1;

    // No other services then charging
    res->ServiceList_isUsed = 0;

    // Available energy transfer modes (add DC to be able to read SoC)
#if 0
    res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[0] = iso2_EnergyTransferModeType_DC_extended;
    res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[1] = iso2_EnergyTransferModeType_AC_single_phase_core;
    res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[2] = iso2_EnergyTransferModeType_AC_three_phase_core;
    res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.arrayLen = 3;
#endif

    const bool charge_via_iso15118 = iso15118.config.get("charge_via_iso15118")->asBool();
    const bool read_soc = iso15118.config.get("read_soc")->asBool();

    if (ISO2_DC_SOC_BEFORE_AC && charge_via_iso15118 && read_soc && !dc_soc_done) {
        // Offer DC (for SoC reading) and AC (for charging).
        // If the EV supports DC, it picks DC and we read SoC first.
        // If the EV only supports AC, it picks AC and we charge directly.
        res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[0] = iso2_EnergyTransferModeType_DC_extended;
        const ChargingInformation ci = iso15118.get_charging_information();
        res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[1] = ci.three_phase
            ? iso2_EnergyTransferModeType_AC_three_phase_core
            : iso2_EnergyTransferModeType_AC_single_phase_core;
        res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.arrayLen = 2;
    } else if (charge_via_iso15118) {
        // AC charging mode (either read_soc=false, or dc_soc_done=true after SoC was read)
        const ChargingInformation ci = iso15118.get_charging_information();
        if (ci.three_phase) {
            res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[0] = iso2_EnergyTransferModeType_AC_three_phase_core;
        } else {
            res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[0] = iso2_EnergyTransferModeType_AC_single_phase_core;
        }
        res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.arrayLen = 1;
    } else if (read_soc) {
        // DC mode for SoC reading only (no ISO15118 charging)
        res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[0] = iso2_EnergyTransferModeType_DC_extended;
        res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.arrayLen = 1;
    }

    // Unique identifier of the service
    res->ChargeService.ServiceID = V2G_SERVICE_ID_CHARGING;

    res->ChargeService.ServiceName_isUsed = 0;
    res->ChargeService.ServiceScope_isUsed = 0;

    // EV can use offered service without payment
    res->ChargeService.FreeService = 1;

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = ISO2State::ServiceDiscovery;
}

void ISO2::handle_payment_service_selection_req()
{
    iso2_PaymentServiceSelectionReqType *req = &iso2DocDec->V2G_Message.Body.PaymentServiceSelectionReq;
    iso2_PaymentServiceSelectionResType *res = &iso2DocEnc->V2G_Message.Body.PaymentServiceSelectionRes;

    if (req->SelectedPaymentOption == iso2_paymentOptionType_ExternalPayment) {
        iso2DocEnc->V2G_Message.Body.PaymentServiceSelectionRes_isUsed = 1;
        res->ResponseCode = iso2_responseCodeType_OK;

        iso15118.common.send_exi(Common::ExiType::Iso2);
        state = ISO2State::PaymentServiceSelection;
    }
}

void ISO2::handle_authorization_req()
{
    iso2_AuthorizationResType *res = &iso2DocEnc->V2G_Message.Body.AuthorizationRes;

    iso2DocEnc->V2G_Message.Body.AuthorizationRes_isUsed = 1;

    // Set Authorisation to Finished here.
    // We want to go on ChargeParameteryDiscovery to read the SoC and then use Ongoing.
    res->ResponseCode = iso2_responseCodeType_OK;
    res->EVSEProcessing = iso2_EVSEProcessingType_Finished;
    iso15118.common.send_exi(Common::ExiType::Iso2);

    state = ISO2State::Authorization;
}

void ISO2::handle_charge_parameter_discovery_req()
{
    iso2_ChargeParameterDiscoveryReqType* req = &iso2DocDec->V2G_Message.Body.ChargeParameterDiscoveryReq;
    iso2_ChargeParameterDiscoveryResType* res = &iso2DocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes;

    if (req->DC_EVChargeParameter_isUsed) {
        api_state.get("soc")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
        if (req->DC_EVChargeParameter.EVEnergyCapacity_isUsed) {
            api_state.get("energy_capacity")->updateInt(static_cast<int32_t>(physical_value_to_float(&req->DC_EVChargeParameter.EVEnergyCapacity)));
        }
    }


    // TODO: Check if EnergyTransferMode is as configured

    if (req->DC_EVChargeParameter_isUsed) {
        logger.printfln("ISO2: Current SoC %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
    }

    // Update EV data for meters module
    {
        EVData ev_data;

        if (req->DC_EVChargeParameter_isUsed) {
            ev_data.soc_present = static_cast<float>(req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
        } else if (req->AC_EVChargeParameter_isUsed) {
            ev_data.max_voltage = physical_value_to_float(&req->AC_EVChargeParameter.EVMaxVoltage);
            ev_data.max_current = physical_value_to_float(&req->AC_EVChargeParameter.EVMaxCurrent);
            ev_data.min_current = physical_value_to_float(&req->AC_EVChargeParameter.EVMinCurrent);
            ev_data.energy_request_kwh = physical_value_to_float(&req->AC_EVChargeParameter.EAmount) / 1000.0f;
        }

        iso15118.common.update_ev_data(ev_data, EVDataProtocol::ISO2);
    }

    iso2DocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1;

    const bool charge_via_iso15118 = iso15118.config.get("charge_via_iso15118")->asBool();
    const bool read_soc = iso15118.config.get("read_soc")->asBool();

    // DC SoC reading mode: read SoC on first request, then send EVSE_Shutdown on second request
    // to make the EVCC send SessionStopReq.
    // This applies to both:
    //   - read_soc_only mode (read_soc=true, charge_via_iso15118=false)
    //   - charge_via_iso15118 + read_soc mode before DC SoC is done
    // Only enter DC SoC mode if the EV actually requested a DC energy transfer mode.
    // If we offered both DC and AC but the EV chose AC (e.g. because it doesn't support DC),
    // we skip DC SoC reading and go straight to AC charging.
    const bool ev_requested_dc = (req->RequestedEnergyTransferMode == iso2_EnergyTransferModeType_DC_extended
                               || req->RequestedEnergyTransferMode == iso2_EnergyTransferModeType_DC_core
                               || req->RequestedEnergyTransferMode == iso2_EnergyTransferModeType_DC_unique);
    current_session_is_dc = ev_requested_dc;

    const bool dc_soc_session = ev_requested_dc &&
                                ((read_soc && !charge_via_iso15118) ||
                                 (ISO2_DC_SOC_BEFORE_AC && charge_via_iso15118 && read_soc && !dc_soc_done));

    if (dc_soc_session) {
        if (soc_read) {
            // SoC already read. Send OK + Finished + EVSE_Shutdown to end the session.
            // We use ResponseCode=OK with EVSEStatusCode=EVSE_Shutdown + EVSEProcessing=Finished.
            logger.printfln("ISO2: SoC already read, sending EVSE_Shutdown to end session");
            res->ResponseCode = iso2_responseCodeType_OK;
            res->EVSEProcessing = iso2_EVSEProcessingType_Finished;

            // Mandatory: DC_EVSEChargeParameter with EVSE_Shutdown status
            res->DC_EVSEChargeParameter_isUsed = 1;
            res->AC_EVSEChargeParameter_isUsed = 0;

            res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
            res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus = iso2_isolationLevelType_Invalid;
            res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification = iso2_EVSENotificationType_None;
            res->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay = 0;
            res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode = iso2_DC_EVSEStatusCodeType_EVSE_Shutdown;

            res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit = iso2_unitSymbolType_A;
            res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value = 0;
            res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier = 0;

            res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit = iso2_unitSymbolType_V;
            res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value = 0;
            res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Multiplier = 0;

            res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Unit = iso2_unitSymbolType_A;
            res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Value = 0;
            res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Multiplier = 0;

            res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit = iso2_unitSymbolType_V;
            res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Value = 0;
            res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Multiplier = 0;

            res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Unit = iso2_unitSymbolType_A;
            res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Value = 0;
            res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Multiplier = 0;

            res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Unit = iso2_unitSymbolType_W;
            res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Value = 0;
            res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Multiplier = 0;

            res->DC_EVSEChargeParameter.EVSECurrentRegulationTolerance_isUsed = 0;
            res->DC_EVSEChargeParameter.EVSEEnergyToBeDelivered_isUsed = 0;

            // SAScheduleList is required when EVSEProcessing=Finished
            res->SAScheduleList_isUsed = 1;
            res->SAScheduleList.SAScheduleTuple.array[0].SAScheduleTupleID = V2G_SA_SCHEDULE_TUPLE_ID;
            res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax.Value = 0;
            res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax.Multiplier = 0;
            res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax.Unit = iso2_unitSymbolType_W;
            res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.start = 0;
            res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration = SECONDS_PER_DAY;
            res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration_isUsed = 1;
            res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval_isUsed = 1;
            res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].TimeInterval_isUsed = 0;
            res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.arrayLen = 1;
            res->SAScheduleList.SAScheduleTuple.array[0].SalesTariff_isUsed = 0;
            res->SAScheduleList.SAScheduleTuple.arrayLen = 1;

            iso15118.common.send_exi(Common::ExiType::Iso2);
            state = ISO2State::ChargeParameterDiscovery;
            return;
        }

        soc_read = true;

        res->ResponseCode = iso2_responseCodeType_OK;
        res->EVSEProcessing = iso2_EVSEProcessingType_Ongoing;

        res->AC_EVSEChargeParameter_isUsed = 0;
        res->DC_EVSEChargeParameter_isUsed = 1;

        // Invalid = An isolation test has not been carried out.
        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus = iso2_isolationLevelType_Invalid;

        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification = iso2_EVSENotificationType_None;
        res->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay = 0;

        // EVSE_Ready here seems to work, maybe test iso2_DC_EVSEStatusCodeType_EVSE_IsolationMonitoringActive if other EVs send StopRequest after this.
        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode = iso2_DC_EVSEStatusCodeType_EVSE_Ready;

        res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit = iso2_unitSymbolType_A;
        res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value = DC_SOC_MAX_CURRENT_A;
        res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier = 0;

        res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit = iso2_unitSymbolType_V;
        res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value = DC_SOC_MAX_VOLTAGE_V;
        res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Multiplier = 0;

        res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Unit = iso2_unitSymbolType_A;
        res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Value = 0;
        res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Multiplier = 0;

        res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit = iso2_unitSymbolType_V;
        res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Value = 0;
        res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Multiplier = 0;

        res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Unit = iso2_unitSymbolType_A;
        res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Value = DC_SOC_PEAK_RIPPLE_A;
        res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Multiplier = 0;

        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Unit = iso2_unitSymbolType_W;
        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Value = DC_SOC_MAX_POWER_VALUE;
        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Multiplier = DC_SOC_MAX_POWER_EXP;

        // Optinal charge parameters
        res->DC_EVSEChargeParameter.EVSECurrentRegulationTolerance_isUsed = 0;
        res->DC_EVSEChargeParameter.EVSEEnergyToBeDelivered_isUsed = 0;

        iso15118.common.send_exi(Common::ExiType::Iso2);
        state = ISO2State::ChargeParameterDiscovery;
    } else if (charge_via_iso15118) {
        // AC Charging mode
        const ChargingInformation ci = iso15118.get_charging_information();

        // Calculate minimum possible power from EV's minimum current
        uint16_t minimum_power = static_cast<uint16_t>(physical_value_to_float(&req->AC_EVChargeParameter.EVMinCurrent)*static_cast<float>(V2G_NOMINAL_VOLTAGE_V) + 100.0f);
        minimum_power = minimum_power - (minimum_power % 100); // round up to 100W

        // Calculate maximum power from our current limit
        // current_ma is per-phase, total power depends on phases
        // Compute in milliwatts and find optimal encoding for maximum resolution.
        uint32_t max_power_mw = static_cast<uint32_t>(ci.current_ma) * V2G_NOMINAL_VOLTAGE_V * (ci.three_phase ? 3 : 1);
        const ScaledPower pmax = encode_milliwatts(max_power_mw);

        res->ResponseCode = iso2_responseCodeType_OK;
        res->EVSEProcessing = iso2_EVSEProcessingType_Finished;

        // Includes several tuples of schedules from secondary actors. The SECC shall
        // only omit the parameter 'SAScheduleList' in case EVSEProcessing is set to 'Ongoing'
        res->SAScheduleList_isUsed = 1;

        // [V2G2-297] The first SAScheduleTuple element in the SAScheduleListType shall be defined as default SASchedule.
        // [V2G2-298] If the EVCC is not capable of comparing different SAScheduleTuple elements or comparison
        // fails, the EVCC shall choose the default SAScheduleTuple according to [V2G2-297].

        // PMax: Defines maximum amount of power for a time interval to be drawn from the EVSE power
        //       outlet the vehicle is connected to. This value represents the total power over all selected phases.
        // V2G2-315] The PMax element shall define the maximum amount of power to be drawn from the EVSE
        //           power outlet when the element of type PMaxScheduleEntryType is active.

        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax.Value = pmax.value;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax.Multiplier = pmax.exponent;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax.Unit = iso2_unitSymbolType_W;

        // [V2G2-330] The value of the duration element shall be defined as period of time in seconds.
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration = SECONDS_PER_DAY; // One day
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration_isUsed = 1;

        // [V2G2-328] The value of the start element shall be defined in seconds from NOW.
        // [V2G2-329] The value of the start element shall simultaneously define the start time of this interval and the
        //            stop time of the previous interval.
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.start = 0;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval_isUsed = 1;

        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].TimeInterval_isUsed = 0; // no content
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.arrayLen = 1;

        // [V2G2-300] The SAScheduleTupleID element shall be unique within all SAScheduleTuple elements in the
        //            SAScheduleListType and uniquely identifies a tuple of PMaxSchedule and SalesTariff elements
        //            during the entire charging session.
        res->SAScheduleList.SAScheduleTuple.array[0].SAScheduleTupleID = V2G_SA_SCHEDULE_TUPLE_ID; // [V2G2-773] 1-255 OK, 0 not allowed

        // SalesTariff: Optional: Encapsulating element describing all relevant details for one SalesTariff from the secondary actor
        res->SAScheduleList.SAScheduleTuple.array[0].SalesTariff_isUsed = 0;
        res->SAScheduleList.SAScheduleTuple.arrayLen = 1;

        res->DC_EVSEChargeParameter_isUsed = 0;
        res->AC_EVSEChargeParameter_isUsed = 1;
        res->AC_EVSEChargeParameter.AC_EVSEStatus.RCD = 0;
        res->AC_EVSEChargeParameter.AC_EVSEStatus.NotificationMaxDelay = 0;
        res->AC_EVSEChargeParameter.AC_EVSEStatus.EVSENotification = iso2_EVSENotificationType_None;

        // Maximum allowed line current restriction set by the EVSE per phase. If the PWM
        // ratio is set to 5% ratio then this is the only line current restriction processed by
        // the EVCC. Otherwise the EVCC applies the smaller current constraint from the
        // EVSEMaxCurrent value and the PWM ratio information.
        res->AC_EVSEChargeParameter.EVSEMaxCurrent.Value = static_cast<int16_t>(ci.current_ma); // e.g. 6123mA -> 6123
        res->AC_EVSEChargeParameter.EVSEMaxCurrent.Multiplier = -3;    // 6123 * 10^-3 = 6.123A (1mA resolution)
        res->AC_EVSEChargeParameter.EVSEMaxCurrent.Unit = iso2_unitSymbolType_A;

        // Line voltage supported by the EVSE. This is the voltage measured between
        // one phases and neutral. If the EVSE supports multiple phase charging the EV
        // might easily calculate the voltage between phases.
        res->AC_EVSEChargeParameter.EVSENominalVoltage.Value = V2G_NOMINAL_VOLTAGE_V;
        res->AC_EVSEChargeParameter.EVSENominalVoltage.Multiplier = 0;
        res->AC_EVSEChargeParameter.EVSENominalVoltage.Unit = iso2_unitSymbolType_V;

        iso15118.common.send_exi(Common::ExiType::Iso2);
        state = ISO2State::ChargeParameterDiscovery;
    }
}

void ISO2::handle_power_delivery_req()
{
    iso2_PowerDeliveryReqType *req = &iso2DocDec->V2G_Message.Body.PowerDeliveryReq;
    iso2_PowerDeliveryResType *res = &iso2DocEnc->V2G_Message.Body.PowerDeliveryRes;

    if (req->DC_EVPowerDeliveryParameter_isUsed) {
        api_state.get("soc")->updateInt(req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVRESSSOC);
    }

    #if 0
    // Optional:
    // Allows an EV to reserve a specific charging profile for the current
    // charging session (i.e. maximum amount of power drawn over time).
    if (req->ChargingProfile_isUsed) {
        req->ChargingProfile.ProfileEntry.array[0].ChargingProfileEntryMaxNumberOfPhasesInUse;
        req->ChargingProfile.ProfileEntry.array[0].ChargingProfileEntryMaxNumberOfPhasesInUse_isUsed;
        req->ChargingProfile.ProfileEntry.array[0].ChargingProfileEntryMaxPower.Multiplier;
        req->ChargingProfile.ProfileEntry.array[0].ChargingProfileEntryMaxPower.Value;
        req->ChargingProfile.ProfileEntry.array[0].ChargingProfileEntryStart;
        req->ChargingProfile.ProfileEntry.arrayLen;
    }
    #endif

    switch (req->ChargeProgress) {
        case iso2_chargeProgressType_Start:
            evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 50);
            break;
        case iso2_chargeProgressType_Stop:
            // Go to 100% PWM to signal to the ev that we accepted the stop,
            // but we need to go back to 5% PWM, so the ev can resume charging
            // if it wants to.
            // TODO: The timing here is unclear, are 5 seconds OK?
            evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 1000);
            task_scheduler.scheduleOnce([this]() {
                evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 50);
            }, 5_s);
            break;
        case iso2_chargeProgressType_Renegotiate:
            break;
        default:
            break;
    }

    // Unique identifier within a charging session for a SAScheduleTuple
    // element. An SAID remains a unique identifier for one schedule throughout
    // a charging session.
    if (req->SAScheduleTupleID != V2G_SA_SCHEDULE_TUPLE_ID) {
        logger.printfln("ISO2: Unexpected SAScheduleTupleID %d", req->SAScheduleTupleID);
    }


    iso2DocEnc->V2G_Message.Body.PowerDeliveryRes_isUsed = 1;
    res->ResponseCode = iso2_responseCodeType_OK;

    // Indicates the current status of the Residual Current Device (RCD). If RCD is equal to true,
    // the RCD has detected an error. If RCD is equal to false, the RCD has not detected an error. This
    // status flag is for informational purpose only.
    res->AC_EVSEStatus.RCD = 0;

    // The SECC uses the NotificationMaxDelay element in the EVSEStatus to indicate the time
    // until it expects the EVCC to react on the action request indicated in EVSENotification.
    res->AC_EVSEStatus.NotificationMaxDelay = 0;

    // This value is used by the SECC to influence the behaviour of the EVCC. The EVSENotification
    // contains an action that the SECC wants the EVCC to perform. The requested action is
    // expected by the EVCC until the time provided in NotificationMaxDelay. If the target time is not in
    // the future, the EVCC is expected to perform the action immediately.
    res->AC_EVSEStatus.EVSENotification = iso2_EVSENotificationType_None; // We can request stop and renegotiation here.
    res->AC_EVSEStatus_isUsed = 1;

    res->DC_EVSEStatus_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = ISO2State::PowerDelivery;
}

void ISO2::handle_charging_status_req()
{
    iso2_ChargingStatusResType *res = &iso2DocEnc->V2G_Message.Body.ChargingStatusRes;

    iso2DocEnc->V2G_Message.Body.ChargingStatusRes_isUsed = 1;
    res->ResponseCode = iso2_responseCodeType_OK;

    res->AC_EVSEStatus.RCD = 0;
    res->AC_EVSEStatus.NotificationMaxDelay = 0;
    res->AC_EVSEStatus.EVSENotification = iso2_EVSENotificationType_None;

    strcpy(res->EVSEID.characters, "ZZ00000");
    res->EVSEID.charactersLen = strlen("ZZ00000");

    res->SAScheduleTupleID = V2G_SA_SCHEDULE_TUPLE_ID;

    // [V2G2-844] EVSEMaxCurrent is the real-time current limit per phase.
    // With 5% CP duty (ISO15118 mode), this is the only current constraint the EV processes.
    // Read on every ChargingStatus cycle to allow dynamic current control.
    const ChargingInformation ci = iso15118.get_charging_information();
    res->EVSEMaxCurrent_isUsed = 1;
    res->EVSEMaxCurrent.Value = static_cast<int16_t>(ci.current_ma); // e.g. 6123mA -> 6123
    res->EVSEMaxCurrent.Multiplier = -3;       // 6123 * 10^-3 = 6.123A (1mA resolution)
    res->EVSEMaxCurrent.Unit = iso2_unitSymbolType_A;

    res->MeterInfo_isUsed = 0;
    //res->MeterInfo.MeterID.characters[0] = '1';
    //res->MeterInfo.MeterID.charactersLen = 1;
    res->MeterInfo.MeterReading_isUsed = 0; // Meter reading in Wh
    res->MeterInfo.TMeter_isUsed = 0; // Unix timestamp
    res->MeterInfo.SigMeterReading_isUsed = 0;


    res->ReceiptRequired_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = ISO2State::ChargingStatus;
}

void ISO2::handle_cable_check_req()
{
    iso2_CableCheckResType *res = &iso2DocEnc->V2G_Message.Body.CableCheckRes;

    iso2DocEnc->V2G_Message.Body.CableCheckRes_isUsed = 1;

    // We will reach CableCheck in the SoC-read-only flow when the EV doesn't check
    // EVSEStatusCode in ChargeParameterDiscoveryRes.
    // We respond with EVSE_Shutdown + Invalid isolation so the CableCheck handler
    // in the EV has to stop the state machine
    res->ResponseCode = iso2_responseCodeType_OK;
    res->EVSEProcessing = iso2_EVSEProcessingType_Finished;

    res->DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
    res->DC_EVSEStatus.EVSEIsolationStatus = iso2_isolationLevelType_Invalid;
    res->DC_EVSEStatus.EVSENotification = iso2_EVSENotificationType_None;
    res->DC_EVSEStatus.NotificationMaxDelay = 0;
    res->DC_EVSEStatus.EVSEStatusCode = iso2_DC_EVSEStatusCodeType_EVSE_Shutdown;

    logger.printfln("ISO2: CableCheckReq received in SoC-read flow, sending EVSE_Shutdown to terminate");

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = ISO2State::CableCheck;

    const bool charge_via_iso15118 = iso15118.config.get("charge_via_iso15118")->asBool();
    const bool read_soc = iso15118.config.get("read_soc")->asBool();

    // Handle the dc_soc_done transition here since we may not get a SessionStopReq.
    if (ISO2_DC_SOC_BEFORE_AC && charge_via_iso15118 && read_soc && !dc_soc_done && current_session_is_dc) {
        dc_soc_done = true;
        soc_read = false;
        state = ISO2State::Idle;
        logger.printfln("ISO2: DC SoC session complete (via CableCheck), waiting for AC session");
    } else if (iso15118.is_read_soc_only() || charge_via_iso15118) {
        // Trigger IEC fallback since we may not get a SessionStopReq.
        iso15118.switch_to_iec_temporary();
    }
}

void ISO2::handle_session_stop_req()
{
    iso2_SessionStopReqType *req = &iso2DocDec->V2G_Message.Body.SessionStopReq;
    iso2_SessionStopResType *res = &iso2DocEnc->V2G_Message.Body.SessionStopRes;

    switch (req->ChargingSession) {
        case iso2_chargingSessionType_Terminate:
            pause_active = false;
            break;
        case iso2_chargingSessionType_Pause:
            pause_active = true;
            break;
        default: break;
    }

    iso2DocEnc->V2G_Message.Body.SessionStopRes_isUsed = 1;
    res->ResponseCode = iso2_responseCodeType_OK;

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = ISO2State::SessionStop;

    const bool charge_via_iso15118 = iso15118.config.get("charge_via_iso15118")->asBool();
    const bool read_soc = iso15118.config.get("read_soc")->asBool();

    if (ISO2_DC_SOC_BEFORE_AC && charge_via_iso15118 && read_soc && !dc_soc_done && current_session_is_dc) {
        // DC SoC session is complete. Mark it done and prepare for the AC charging session.
        // Keep PLC link alive — only close TCP so EV can reconnect for AC.
        dc_soc_done = true;
        soc_read = false;
        state = ISO2State::Idle; // Reset state machine for next session
        logger.printfln("ISO2: DC SoC session complete, waiting for AC session");
    } else if (iso15118.is_read_soc_only()) {
        // In read_soc_only mode, switch to IEC 61851 after the session ends.
        // The EVSE will control charging via PWM and revert to ISO 15118 on EV disconnect.
        iso15118.switch_to_iec_temporary();
    }

    // Reset the socket, so the ev can reconnect when it wants to resume from the pause
    // If the ev wants to terminate we reset the active socket anyway.
    // Reset after data has been sent.
    iso15118.common.reset_active_socket();
}

void ISO2::trace_header(const struct iso2_MessageHeaderType *header, const char *name)
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
    if (header->Signature_isUsed) {
        iso15118.trace("  Signature.Id_isUsed: %d", header->Signature.Id_isUsed);
        if (header->Signature.Id_isUsed) {
            iso15118.trace("  Signature.Id.characters: %s", header->Signature.Id.characters);
            iso15118.trace("  Signature.Id.charactersLen: %d", header->Signature.Id.charactersLen);
        }
        iso15118.trace("  Signature.SignatureValue.Id_isUsed: %d", header->Signature.SignatureValue.Id_isUsed);
        if (header->Signature.SignatureValue.Id_isUsed) {
            iso15118.trace("  Signature.SignatureValue.Id.characters: %s", header->Signature.SignatureValue.Id.characters);
            iso15118.trace("  Signature.SignatureValue.Id.charactersLen: %d", header->Signature.SignatureValue.Id.charactersLen);
        }
        iso15118.trace("  Signature.KeyInfo_isUsed: %d", header->Signature.KeyInfo_isUsed);
        if (header->Signature.KeyInfo_isUsed) {
            iso15118.trace("  Signature.KeyInfo.Id_isUsed: %d", header->Signature.KeyInfo.Id_isUsed);
            if (header->Signature.KeyInfo.Id_isUsed) {
                iso15118.trace("  Signature.KeyInfo.Id.characters: %s", header->Signature.KeyInfo.Id.characters);
                iso15118.trace("  Signature.KeyInfo.Id.charactersLen: %d", header->Signature.KeyInfo.Id.charactersLen);
            }
            iso15118.trace("  Signature.KeyInfo.KeyValue_isUsed: %d", header->Signature.KeyInfo.KeyValue_isUsed);
            if (header->Signature.KeyInfo.KeyValue_isUsed) {
                iso15118.trace("  Signature.KeyInfo.KeyValue.RSAKeyValue_isUsed: %d", header->Signature.KeyInfo.KeyValue.RSAKeyValue_isUsed);
                if (header->Signature.KeyInfo.KeyValue.RSAKeyValue_isUsed) {
                    iso15118.trace("  Signature.KeyInfo.KeyValue.RSAKeyValue.Modulus.bytes: %02x%02x%02x%02x...",
                                   header->Signature.KeyInfo.KeyValue.RSAKeyValue.Modulus.bytes[0],
                                   header->Signature.KeyInfo.KeyValue.RSAKeyValue.Modulus.bytes[1],
                                   header->Signature.KeyInfo.KeyValue.RSAKeyValue.Modulus.bytes[2],
                                   header->Signature.KeyInfo.KeyValue.RSAKeyValue.Modulus.bytes[3]);
                    iso15118.trace("  Signature.KeyInfo.KeyValue.RSAKeyValue.Modulus.bytesLen: %d", header->Signature.KeyInfo.KeyValue.RSAKeyValue.Modulus.bytesLen);
                    iso15118.trace("  Signature.KeyInfo.KeyValue.RSAKeyValue.Exponent.bytes: %02x%02x%02x%02x...",
                                   header->Signature.KeyInfo.KeyValue.RSAKeyValue.Exponent.bytes[0],
                                   header->Signature.KeyInfo.KeyValue.RSAKeyValue.Exponent.bytes[1],
                                   header->Signature.KeyInfo.KeyValue.RSAKeyValue.Exponent.bytes[2],
                                   header->Signature.KeyInfo.KeyValue.RSAKeyValue.Exponent.bytes[3]);
                    iso15118.trace("  Signature.KeyInfo.KeyValue.RSAKeyValue.Exponent.bytesLen: %d", header->Signature.KeyInfo.KeyValue.RSAKeyValue.Exponent.bytesLen);
                }
            }
        }
    }
};

void ISO2::trace_request_response()
{
    if (iso2DocDec->V2G_Message.Body.SessionSetupReq_isUsed) {
        iso2_SessionSetupReqType *req = &iso2DocDec->V2G_Message.Body.SessionSetupReq;

        trace_header(&iso2DocDec->V2G_Message.Header, "SessionSetup Request");
        iso15118.trace(" Body");
        iso15118.trace("  SessionSetupReq");
        iso15118.trace("   EVCCID: %02x%02x%02x%02x%02x%02x", req->EVCCID.bytes[0], req->EVCCID.bytes[1], req->EVCCID.bytes[2], req->EVCCID.bytes[3], req->EVCCID.bytes[4], req->EVCCID.bytes[5]);
        iso15118.trace("   EVCCID.bytesLen: %d", req->EVCCID.bytesLen);
    } else if (iso2DocDec->V2G_Message.Body.ServiceDiscoveryReq_isUsed) {
        iso2_ServiceDiscoveryReqType *req = &iso2DocDec->V2G_Message.Body.ServiceDiscoveryReq;

        trace_header(&iso2DocDec->V2G_Message.Header, "ServiceDiscovery Request");
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
    } else if (iso2DocDec->V2G_Message.Body.PaymentServiceSelectionReq_isUsed) {
        iso2_PaymentServiceSelectionReqType *req = &iso2DocDec->V2G_Message.Body.PaymentServiceSelectionReq;

        trace_header(&iso2DocDec->V2G_Message.Header, "PaymentServiceSelection Request");
        iso15118.trace(" Body");
        iso15118.trace("  PaymentServiceSelectionReq");
        iso15118.trace("   SelectedPaymentOption: %d", req->SelectedPaymentOption);
        for (int i = 0; i < req->SelectedServiceList.SelectedService.arrayLen; i++) {
            iso15118.trace("    SelectedService[%d]", i);
            iso15118.trace("     ServiceID: %d", req->SelectedServiceList.SelectedService.array[i].ServiceID);
            iso15118.trace("     ParameterSetID_isUsed: %d", req->SelectedServiceList.SelectedService.array[i].ParameterSetID_isUsed);
            if (req->SelectedServiceList.SelectedService.array[i].ParameterSetID_isUsed) {
                iso15118.trace("      ParameterSetID: %d", req->SelectedServiceList.SelectedService.array[i].ParameterSetID);
            }
        }
    } else if (iso2DocDec->V2G_Message.Body.AuthorizationReq_isUsed) {
        iso2_AuthorizationReqType *req = &iso2DocDec->V2G_Message.Body.AuthorizationReq;

        trace_header(&iso2DocDec->V2G_Message.Header, "Authorization Request");
        iso15118.trace(" Body");
        iso15118.trace("  AuthorizationReq");
        iso15118.trace("  GenChallenge_isUsed: %d", req->GenChallenge_isUsed);
        if (req->GenChallenge_isUsed) {
            iso15118.trace("   GenChallenge: %02x%02x%02x%02x...", req->GenChallenge.bytes[0], req->GenChallenge.bytes[1], req->GenChallenge.bytes[2], req->GenChallenge.bytes[3]);
        }
        iso15118.trace("   Id_isUsed: %d", req->Id_isUsed);
        if (req->Id_isUsed) {
            iso15118.trace("   Id: %s", req->Id.characters);
        }
    } else if (iso2DocDec->V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed) {
        iso2_ChargeParameterDiscoveryReqType *req = &iso2DocDec->V2G_Message.Body.ChargeParameterDiscoveryReq;

        trace_header(&iso2DocDec->V2G_Message.Header, "ChargeParameterDiscovery Request");
        iso15118.trace(" Body");
        iso15118.trace("  ChargeParameterDiscoveryReq");
        iso15118.trace("   MaxEntriesSAScheduleTuple_isUsed: %d", req->MaxEntriesSAScheduleTuple_isUsed);
        if (req->MaxEntriesSAScheduleTuple_isUsed) {
            iso15118.trace("   MaxEntriesSAScheduleTuple: %d", req->MaxEntriesSAScheduleTuple);
        }
        iso15118.trace("   RequestedEnergyTransferMode: %d", req->RequestedEnergyTransferMode);
        iso15118.trace("   AC_EVChargeParameter_isUsed: %d", req->AC_EVChargeParameter_isUsed);
        if (req->AC_EVChargeParameter_isUsed) {
            iso15118.trace("    DepartureTime: %lu", req->AC_EVChargeParameter.DepartureTime);
            iso15118.trace("    DepartureTime_isUsed: %d", req->AC_EVChargeParameter.DepartureTime_isUsed);
            iso15118.trace("    EAmount.Value: %d", req->AC_EVChargeParameter.EAmount.Value);
            iso15118.trace("    EAmount.Multiplier: %d", req->AC_EVChargeParameter.EAmount.Multiplier);
            iso15118.trace("    EVMaxVoltage.Value: %d", req->AC_EVChargeParameter.EVMaxVoltage.Value);
            iso15118.trace("    EVMaxVoltage.Multiplier: %d", req->AC_EVChargeParameter.EVMaxVoltage.Multiplier);
            iso15118.trace("    EVMaxCurrent.Value: %d", req->AC_EVChargeParameter.EVMaxCurrent.Value);
            iso15118.trace("    EVMaxCurrent.Multiplier: %d", req->AC_EVChargeParameter.EVMaxCurrent.Multiplier);
            iso15118.trace("    EVMinCurrent.Value: %d", req->AC_EVChargeParameter.EVMinCurrent.Value);
            iso15118.trace("    EVMinCurrent.Multiplier: %d", req->AC_EVChargeParameter.EVMinCurrent.Multiplier);
        }

        iso15118.trace("   DC_EVChargeParameter_isUsed: %d", req->DC_EVChargeParameter_isUsed);
        if (req->DC_EVChargeParameter_isUsed) {
            iso15118.trace("    DepartureTime: %lu", req->DC_EVChargeParameter.DepartureTime);
            iso15118.trace("    DepartureTime_isUsed: %d", req->DC_EVChargeParameter.DepartureTime_isUsed);
            iso15118.trace("    DC_EVStatus.EVRESSSOC: %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
            iso15118.trace("    DC_EVStatus.EVReady: %d", req->DC_EVChargeParameter.DC_EVStatus.EVReady);
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
    } else if (iso2DocDec->V2G_Message.Body.PowerDeliveryReq_isUsed) {
        iso2_PowerDeliveryReqType *req = &iso2DocDec->V2G_Message.Body.PowerDeliveryReq;

        trace_header(&iso2DocDec->V2G_Message.Header, "PowerDelivery Request");
        iso15118.trace(" Body");
        iso15118.trace("  PowerDeliveryReq");
        iso15118.trace("   ChargeProgress: %d", req->ChargeProgress);
        iso15118.trace("   SAScheduleTupleID: %d", req->SAScheduleTupleID);
        iso15118.trace("   ChargingProfile_isUsed: %d", req->ChargingProfile_isUsed);
        if (req->ChargingProfile_isUsed) {
            for (int i = 0; i < req->ChargingProfile.ProfileEntry.arrayLen; i++) {
                iso15118.trace("    ProfileEntry[%d]", i);
                iso15118.trace("     ChargingProfileEntryMaxNumberOfPhasesInUse: %d", req->ChargingProfile.ProfileEntry.array[i].ChargingProfileEntryMaxNumberOfPhasesInUse);
                iso15118.trace("     ChargingProfileEntryMaxNumberOfPhasesInUse_isUsed: %d", req->ChargingProfile.ProfileEntry.array[i].ChargingProfileEntryMaxNumberOfPhasesInUse_isUsed);
                iso15118.trace("     ChargingProfileEntryMaxPower.Value: %d", req->ChargingProfile.ProfileEntry.array[i].ChargingProfileEntryMaxPower.Value);
                iso15118.trace("     ChargingProfileEntryMaxPower.Multiplier: %d", req->ChargingProfile.ProfileEntry.array[i].ChargingProfileEntryMaxPower.Multiplier);
                iso15118.trace("     ChargingProfileEntryMaxPower.Unit: %d", req->ChargingProfile.ProfileEntry.array[i].ChargingProfileEntryMaxPower.Unit);
                iso15118.trace("     ChargingProfileEntryStart: %lu", req->ChargingProfile.ProfileEntry.array[i].ChargingProfileEntryStart);
            }
        }

        iso15118.trace("   DC_EVPowerDeliveryParameter_isUsed: %d", req->DC_EVPowerDeliveryParameter_isUsed);
        if (req->DC_EVPowerDeliveryParameter_isUsed) {
            iso15118.trace("    DC_EVStatus");
            iso15118.trace("     EVRESSSOC: %d", req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVRESSSOC);
            iso15118.trace("     EVReady: %d", req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVReady);
            iso15118.trace("     EVErrorCode: %d", req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVErrorCode);
            iso15118.trace("    BulkChargingComplete: %d", req->DC_EVPowerDeliveryParameter.BulkChargingComplete);
            iso15118.trace("    BulkChargingComplete_isUsed: %d", req->DC_EVPowerDeliveryParameter.BulkChargingComplete_isUsed);
            iso15118.trace("    ChargingComplete: %d", req->DC_EVPowerDeliveryParameter.ChargingComplete);
        }
    } else if (iso2DocDec->V2G_Message.Body.ChargingStatusReq_isUsed) {
        trace_header(&iso2DocDec->V2G_Message.Header, "ChargingStatus Request");
        iso15118.trace(" Body");
        iso15118.trace("  ChargingStatusReq");

    } else if (iso2DocDec->V2G_Message.Body.CableCheckReq_isUsed) {
        iso2_CableCheckReqType *req = &iso2DocDec->V2G_Message.Body.CableCheckReq;

        trace_header(&iso2DocDec->V2G_Message.Header, "CableCheck Request");
        iso15118.trace(" Body");
        iso15118.trace("  CableCheckReq");
        iso15118.trace("   DC_EVStatus.EVRESSSOC: %d", req->DC_EVStatus.EVRESSSOC);
        iso15118.trace("   DC_EVStatus.EVReady: %d", req->DC_EVStatus.EVReady);
        iso15118.trace("   DC_EVStatus.EVErrorCode: %d", req->DC_EVStatus.EVErrorCode);

    } else if (iso2DocDec->V2G_Message.Body.SessionStopReq_isUsed) {
        iso2_SessionStopReqType *req = &iso2DocDec->V2G_Message.Body.SessionStopReq;

        trace_header(&iso2DocDec->V2G_Message.Header, "SessionStop Request");
        iso15118.trace(" Body");
        iso15118.trace("  SessionStopReq");
        iso15118.trace("   ChargingSession: %d", req->ChargingSession);
    }

    if (iso2DocEnc->V2G_Message.Body.SessionSetupRes_isUsed) {
        iso2_SessionSetupResType *res = &iso2DocEnc->V2G_Message.Body.SessionSetupRes;

        trace_header(&iso2DocEnc->V2G_Message.Header, "SessionSetup Response");
        iso15118.trace(" Body");
        iso15118.trace("  SessionSetupRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEID: %s", res->EVSEID.characters);
        iso15118.trace("   EVSEID.charactersLen: %d", res->EVSEID.charactersLen);
        iso15118.trace("   EVSETimeStamp: %lld", res->EVSETimeStamp);
        iso15118.trace("   EVSETimeStamp_isUsed: %d", res->EVSETimeStamp_isUsed);
    } else if (iso2DocEnc->V2G_Message.Body.ServiceDiscoveryRes_isUsed) {
        iso2_ServiceDiscoveryResType *res = &iso2DocEnc->V2G_Message.Body.ServiceDiscoveryRes;

        trace_header(&iso2DocEnc->V2G_Message.Header, "ServiceDiscovery Response");
        iso15118.trace(" Body");
        iso15118.trace("  ServiceDiscoveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   PaymentOptionList: %d", res->PaymentOptionList.PaymentOption.array[0]);
        iso15118.trace("   ChargeService.ServiceID: %d", res->ChargeService.ServiceID);
    } else if (iso2DocEnc->V2G_Message.Body.PaymentServiceSelectionRes_isUsed) {
        iso2_PaymentServiceSelectionResType *res = &iso2DocEnc->V2G_Message.Body.PaymentServiceSelectionRes;

        trace_header(&iso2DocEnc->V2G_Message.Header, "PaymentServiceSelection Response");
        iso15118.trace(" Body");
        iso15118.trace("  PaymentServiceSelectionRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
    } else if (iso2DocEnc->V2G_Message.Body.AuthorizationRes_isUsed) {
        iso2_AuthorizationResType *res = &iso2DocEnc->V2G_Message.Body.AuthorizationRes;

        trace_header(&iso2DocEnc->V2G_Message.Header, "Authorization Response");
        iso15118.trace(" Body");
        iso15118.trace("  AuthorizationRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEProcessing: %d", res->EVSEProcessing);
    } else if (iso2DocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed) {
        iso2_ChargeParameterDiscoveryResType *res = &iso2DocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes;

        trace_header(&iso2DocEnc->V2G_Message.Header, "ChargeParameterDiscovery Response");
        iso15118.trace(" Body");
        iso15118.trace("  ChargeParameterDiscoveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEProcessing: %d", res->EVSEProcessing);
        iso15118.trace("   SAScheduleList_isUsed: %d", res->SAScheduleList_isUsed);
        if (res->SAScheduleList_isUsed) {
            iso15118.trace("   SAScheduleList");
            for (int i = 0; i < res->SAScheduleList.SAScheduleTuple.arrayLen; i++) {
                iso15118.trace("    SAScheduleTuple[%d]", i);
                iso15118.trace("     SAScheduleTupleID: %d", res->SAScheduleList.SAScheduleTuple.array[i].SAScheduleTupleID);

                if (res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.arrayLen > 0) {
                    iso15118.trace("     PMaxSchedule");
                    for (int j = 0; j < res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.arrayLen; j++) {
                        iso15118.trace("      PMaxScheduleEntry[%d]", j);
                        iso15118.trace("       PMax.Value: %d", res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].PMax.Value);
                        iso15118.trace("       PMax.Multiplier: %d", res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].PMax.Multiplier);
                        iso15118.trace("       PMax.Unit: %d", res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].PMax.Unit);

                        iso15118.trace("       RelativeTimeInterval_isUsed: %d", res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval_isUsed);
                        if (res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval_isUsed) {
                            iso15118.trace("       RelativeTimeInterval");
                            iso15118.trace("        start: %lu", res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.start);
                            iso15118.trace("        duration: %lu", res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.duration);
                        }
                    }
                }
                iso15118.trace("     SalesTariff_isUsed: %d", res->SAScheduleList.SAScheduleTuple.array[i].SalesTariff_isUsed);
            }
        }

        iso15118.trace("   AC_EVSEChargeParameter_isUsed: %d", res->AC_EVSEChargeParameter_isUsed);
        if (res->AC_EVSEChargeParameter_isUsed) {
            iso15118.trace("    EVSEMaxCurrent.Value: %d", res->AC_EVSEChargeParameter.EVSEMaxCurrent.Value);
            iso15118.trace("    EVSEMaxCurrent.Multiplier: %d", res->AC_EVSEChargeParameter.EVSEMaxCurrent.Multiplier);
            iso15118.trace("    EVSEMaxCurrent.Unit: %d", res->AC_EVSEChargeParameter.EVSEMaxCurrent.Unit);
            iso15118.trace("    EVSENominalVoltage.Value: %d", res->AC_EVSEChargeParameter.EVSENominalVoltage.Value);
            iso15118.trace("    EVSENominalVoltage.Multiplier: %d", res->AC_EVSEChargeParameter.EVSENominalVoltage.Multiplier);
            iso15118.trace("    EVSENominalVoltage.Unit: %d", res->AC_EVSEChargeParameter.EVSENominalVoltage.Unit);
            iso15118.trace("    AC_EVSEStatus");
            iso15118.trace("     RCD: %d", res->AC_EVSEChargeParameter.AC_EVSEStatus.RCD);
            iso15118.trace("     NotificationMaxDelay: %d", res->AC_EVSEChargeParameter.AC_EVSEStatus.NotificationMaxDelay);
            iso15118.trace("     EVSENotification: %d", res->AC_EVSEChargeParameter.AC_EVSEStatus.EVSENotification);
        }

        iso15118.trace("   DC_EVSEChargeParameter_isUsed: %d", res->DC_EVSEChargeParameter_isUsed);
        if (res->DC_EVSEChargeParameter_isUsed) {
            iso15118.trace("    EVSEMaximumCurrentLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value);
            iso15118.trace("    EVSEMaximumCurrentLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier);
            iso15118.trace("    EVSEMaximumCurrentLimit.Unit: %d", res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit);
            iso15118.trace("    EVSEMaximumVoltageLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value);
            iso15118.trace("    EVSEMaximumVoltageLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Multiplier);
            iso15118.trace("    EVSEMaximumVoltageLimit.Unit: %d", res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit);
            iso15118.trace("    EVSEMinimumCurrentLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Value);
            iso15118.trace("    EVSEMinimumCurrentLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Multiplier);
            iso15118.trace("    EVSEMinimumCurrentLimit.Unit: %d", res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Unit);
            iso15118.trace("    EVSEMinimumVoltageLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Value);
            iso15118.trace("    EVSEMinimumVoltageLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Multiplier);
            iso15118.trace("    EVSEMinimumVoltageLimit.Unit: %d", res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit);
            iso15118.trace("    EVSEPeakCurrentRipple.Value: %d", res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Value);
            iso15118.trace("    EVSEPeakCurrentRipple.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Multiplier);
            iso15118.trace("    EVSEPeakCurrentRipple.Unit: %d", res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Unit);
            iso15118.trace("    EVSEMaximumPowerLimit.Value: %d", res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Value);
            iso15118.trace("    EVSEMaximumPowerLimit.Multiplier: %d", res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Multiplier);
            iso15118.trace("    EVSEMaximumPowerLimit.Unit: %d", res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Unit);
            iso15118.trace("    DC_EVSEStatus");
            iso15118.trace("     EVSEIsolationStatus_isUsed: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed);
            if (res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed) {
                iso15118.trace("      EVSEIsolationStatus: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus);
            }
            iso15118.trace("      EVSENotification: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification);
            iso15118.trace("      NotificationMaxDelay: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay);
            iso15118.trace("      EVSEStatusCode: %d", res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode);
        }
    } else if (iso2DocEnc->V2G_Message.Body.CableCheckRes_isUsed) {
        iso2_CableCheckResType *res = &iso2DocEnc->V2G_Message.Body.CableCheckRes;

        trace_header(&iso2DocEnc->V2G_Message.Header, "CableCheck Response");
        iso15118.trace(" Body");
        iso15118.trace("  CableCheckRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEProcessing: %d", res->EVSEProcessing);
        iso15118.trace("   DC_EVSEStatus");
        iso15118.trace("    EVSEIsolationStatus_isUsed: %d", res->DC_EVSEStatus.EVSEIsolationStatus_isUsed);
        if (res->DC_EVSEStatus.EVSEIsolationStatus_isUsed) {
            iso15118.trace("     EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
        }
        iso15118.trace("    EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
        iso15118.trace("    NotificationMaxDelay: %d", res->DC_EVSEStatus.NotificationMaxDelay);
        iso15118.trace("    EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
    } else if (iso2DocEnc->V2G_Message.Body.PowerDeliveryRes_isUsed) {
        iso2_PowerDeliveryResType *res = &iso2DocEnc->V2G_Message.Body.PowerDeliveryRes;

        trace_header(&iso2DocEnc->V2G_Message.Header, "PowerDelivery Response");
        iso15118.trace(" Body");
        iso15118.trace("  PowerDeliveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   AC_EVSEStatus_isUsed: %d", res->AC_EVSEStatus_isUsed);
        if (res->AC_EVSEStatus_isUsed) {
            iso15118.trace("   AC_EVSEStatus");
            iso15118.trace("    RCD: %d", res->AC_EVSEStatus.RCD);
            iso15118.trace("    NotificationMaxDelay: %d", res->AC_EVSEStatus.NotificationMaxDelay);
            iso15118.trace("    EVSENotification: %d", res->AC_EVSEStatus.EVSENotification);
        }

        iso15118.trace("   DC_EVSEStatus_isUsed: %d", res->DC_EVSEStatus_isUsed);
        if (res->DC_EVSEStatus_isUsed) {
            iso15118.trace("   DC_EVSEStatus");
            iso15118.trace("    EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
            iso15118.trace("    EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
            iso15118.trace("    NotificationMaxDelay: %d", res->DC_EVSEStatus.NotificationMaxDelay);
            iso15118.trace("    EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
        }
    } else if (iso2DocEnc->V2G_Message.Body.ChargingStatusRes_isUsed) {
        iso2_ChargingStatusResType *res = &iso2DocEnc->V2G_Message.Body.ChargingStatusRes;

        trace_header(&iso2DocEnc->V2G_Message.Header, "ChargingStatus Response");
        iso15118.trace(" Body");
        iso15118.trace("  ChargingStatusRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEID.characters: %s", res->EVSEID.characters);
        iso15118.trace("   EVSEID.charactersLen: %d", res->EVSEID.charactersLen);
        iso15118.trace("   SAScheduleTupleID: %d", res->SAScheduleTupleID);
        iso15118.trace("   EVSEMaxCurrent.Value: %d", res->EVSEMaxCurrent.Value);
        iso15118.trace("   EVSEMaxCurrent.Multiplier: %d", res->EVSEMaxCurrent.Multiplier);
        iso15118.trace("   EVSEMaxCurrent.Unit: %d", res->EVSEMaxCurrent.Unit);
        iso15118.trace("   AC_EVSEStatus.RCD: %d", res->AC_EVSEStatus.RCD);
        iso15118.trace("   AC_EVSEStatus.NotificationMaxDelay: %d", res->AC_EVSEStatus.NotificationMaxDelay);
        iso15118.trace("   AC_EVSEStatus.EVSENotification: %d", res->AC_EVSEStatus.EVSENotification);
        iso15118.trace("   MeterInfo_isUsed: %d", res->MeterInfo_isUsed);
        iso15118.trace("   ReceiptRequired_isUsed: %d", res->ReceiptRequired_isUsed);
    } else if (iso2DocEnc->V2G_Message.Body.SessionStopRes_isUsed) {
        iso2_SessionStopResType *res = &iso2DocEnc->V2G_Message.Body.SessionStopRes;

        trace_header(&iso2DocEnc->V2G_Message.Header, "SessionStop Response");
        iso15118.trace(" Body");
        iso15118.trace("  SessionStopRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
    }
}
