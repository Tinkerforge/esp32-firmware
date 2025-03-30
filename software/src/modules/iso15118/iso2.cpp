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

// IPv6/TCP/ISO-15118-2

#include "iso2.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"

void ISO2::pre_setup()
{
    api_state = Config::Object({
        {"state", Config::Uint8(0)},
        {"session_id", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)
            }, Config::get_prototype_uint8_0(), 4, 4, Config::type_id<Config::ConfUint>())
        },
        {"evcc_id", Config::Array({}, Config::get_prototype_uint8_0(), 0, 8, Config::type_id<Config::ConfUint>())},
        {"max_entries_sa_schedule_tuple", Config::Uint16(0)},
        {"requested_energy_transfer_mode", Config::Uint8(0)},
        {"ac", Config::Object({
            {"is_used", Config::Bool(false)},
            {"departure_time", Config::Uint32(0)},
            {"departure_time_is_used", Config::Bool(false)},
            {"eamount_val", Config::Int16(0)},
            {"eamount_mul", Config::Int8(0)},
            {"ev_max_voltage_val", Config::Int16(0)},
            {"ev_max_voltage_mul", Config::Int8(0)},
            {"ev_max_current_val", Config::Int16(0)},
            {"ev_max_current_mul", Config::Int8(0)},
            {"ev_min_current_val", Config::Int16(0)},
            {"ev_min_current_mul", Config::Int8(0)}
        })},
        {"dc", Config::Object({
            {"is_used", Config::Bool(false)},
            {"departure_time", Config::Uint32(0)},
            {"departure_time_is_used", Config::Bool(false)},
            {"soc", Config::Int8(0)},
            {"ev_ready", Config::Int32(0)},
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
            {"ev_energy_capacity_is_used", Config::Bool(false)},
            {"ev_energy_request_val", Config::Int16(0)},
            {"ev_energy_request_mul", Config::Int8(0)},
            {"ev_energy_request_is_used", Config::Bool(false)},
            {"full_soc", Config::Int8(0)},
            {"full_soc_is_used", Config::Bool(false)},
            {"bulk_soc", Config::Int8(0)},
            {"bulk_soc_is_used", Config::Bool(false)}
        })},
    });
}

void ISO2::handle_bitstream(exi_bitstream *exi)
{
    // Increment state on first call
    if (state == 0) {
        state = 1;
    }

    // We alloc the iso2 buffers the very first time they are used.
    // This way it is not allocated if ISO15118 is not used.
    // If it is used once we can assume that it will be used all the time, so it stays allocated.
    if (iso2DocDec == nullptr) {
        iso2DocDec = (struct iso2_exiDocument*)heap_caps_calloc_prefer(sizeof(struct iso2_exiDocument), 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    }
    if (iso2DocEnc == nullptr) {
        iso2DocEnc = (struct iso2_exiDocument*)heap_caps_calloc_prefer(sizeof(struct iso2_exiDocument), 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    }
    memset(iso2DocDec, 0, sizeof(struct iso2_exiDocument));
    memset(iso2DocEnc, 0, sizeof(struct iso2_exiDocument));
    int ret = decode_iso2_exiDocument(exi, iso2DocDec);
    if (ret != 0) {
        logger.printfln("ISO2: Could not decode EXI document: %d", ret);
        return;
    }

    if (iso2DocDec->V2G_Message.Body.SessionSetupReq_isUsed) {
        handle_session_setup_req();
    }

    if (iso2DocDec->V2G_Message.Body.ServiceDiscoveryReq_isUsed) {
        handle_service_discovery_req();
    }

    if (iso2DocDec->V2G_Message.Body.ServiceDetailReq_isUsed) {
        logger.printfln("ISO2: ServiceDetailReq received but not implemented");
    }

    if (iso2DocDec->V2G_Message.Body.PaymentServiceSelectionReq_isUsed) {
        handle_payment_service_selection_req();
    }

    if (iso2DocDec->V2G_Message.Body.PaymentDetailsReq_isUsed) {
        logger.printfln("ISO2: PaymentDetailsReq received but not implemented");
    }

    if (iso2DocDec->V2G_Message.Body.CertificateInstallationReq_isUsed) {
        logger.printfln("ISO2: CertificateInstallationReq received but not implemented");
    }

    if (iso2DocDec->V2G_Message.Body.CertificateUpdateReq_isUsed) {
        logger.printfln("ISO2: CertificateUpdateReq received but not implemented");
    }

    if (iso2DocDec->V2G_Message.Body.AuthorizationReq_isUsed) {
        handle_authorization_req();
    }

    if (iso2DocDec->V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed) {
        handle_charge_parameter_discovery_req();
    }

    if (iso2DocDec->V2G_Message.Body.PowerDeliveryReq_isUsed) {
        handle_power_delivery_req();
    }

    if (iso2DocDec->V2G_Message.Body.ChargingStatusReq_isUsed) {
        handle_charging_status_req();
    }

    if (iso2DocDec->V2G_Message.Body.MeteringReceiptReq_isUsed) {
        logger.printfln("ISO2: MeteringReceiptReq received but not implemented");
    }

    if (iso2DocDec->V2G_Message.Body.SessionStopReq_isUsed) {
        handle_session_stop_req();
    }

    if (iso2DocDec->V2G_Message.Body.CableCheckReq_isUsed) {
        logger.printfln("ISO2: CableCheckReq received but not implemented");
    }

    if (iso2DocDec->V2G_Message.Body.PreChargeReq_isUsed) {
        logger.printfln("ISO2: PreChargeReq received but not implemented");
    }

    if (iso2DocDec->V2G_Message.Body.CurrentDemandReq_isUsed) {
        logger.printfln("ISO2: CurrentDemandReq received but not implemented");
    }

    if (iso2DocDec->V2G_Message.Body.WeldingDetectionReq_isUsed) {
        logger.printfln("ISO2: WeldingDetectionReq received but not implemented");
    }

    trace_request_response();

    api_state.get("state")->updateUint(state);
}

void ISO2::handle_session_setup_req()
{
    iso2_SessionSetupReqType *req = &iso2DocDec->V2G_Message.Body.SessionSetupReq;
    iso2_SessionSetupResType *res = &iso2DocEnc->V2G_Message.Body.SessionSetupRes;

    api_state.get("evcc_id")->removeAll();
    for (uint16_t i = 0; i < std::min(static_cast<uint16_t>(sizeof(req->EVCCID.bytes)), req->EVCCID.bytesLen); i++) {
        api_state.get("evcc_id")->add()->updateUint(req->EVCCID.bytes[i]);
    }

    // [V2G2-750] When receiving the SessionSetupReq with the parameter SessionID equal to zero (0), the
    // SECC shall generate a new (not stored) SessionID value different from zero (0) and return this
    // value in the SessionSetupRes message header.
    bool all_zero = true;
    for (size_t i = 0; i < iso2DocDec->V2G_Message.Header.SessionID.bytesLen; i++) {
        if (iso2DocDec->V2G_Message.Header.SessionID.bytes[i] != 0x00) {
            all_zero = false;
            break;
        }
    }

    // [V2G2-753] If an EVCC chooses to resume a charging session by sending a SesstionSetupReq with a
    // message header including the SessionID value from the previously paused V2G
    // Communication Session, the SECC shall compare this value to the value stored from the
    // preceding V2G Communication Session.
    bool different_to_known = true;
    if (iso2DocDec->V2G_Message.Header.SessionID.bytesLen == SESSION_ID_LENGTH) {
        for (uint16_t i = 0; i < SESSION_ID_LENGTH; i++) {
            if (iso2DocDec->V2G_Message.Header.SessionID.bytes[i] == iso15118.common.session_id[i]) {
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

        // [V2G2-462] The message 'SessionSetupRes' shall contain the specific ResponseCode
        // 'OK_NewSessionEstablished' if processing of the SessionSetupReq message was successful
        // and a different SessionID is contained in the response message than the SessionID in the
        // request message.
        res->ResponseCode = iso2_responseCodeType_OK_NewSessionEstablished;
    } else {
        // [V2G2-754]If the SessionID value received in the current SessionSetupReq is equal to the value stored
        // from the preceding V2G Communication Session, the SECC shall confirm the continuation of
        // the charging session by sending a SessionSetupRes message including the stored SessionID
        // value and indicating the resumed V2G Communication Session with the ResponseCode set to
        // "OK_OldSessionJoined" (refer also to [V2G2-463] for selecting the appropriate response
        // code).
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
    state = 2;
}

void ISO2::handle_service_discovery_req()
{
    iso2_ServiceDiscoveryReqType *req = &iso2DocDec->V2G_Message.Body.ServiceDiscoveryReq;
    iso2_ServiceDiscoveryResType *res = &iso2DocEnc->V2G_Message.Body.ServiceDiscoveryRes;

    // TODO: Stop session if session ID does not match?
    //       Or just keep going? For now we log it and keep going.
    if (iso2DocDec->V2G_Message.Header.SessionID.bytesLen != SESSION_ID_LENGTH) {
        logger.printfln("ISO2: Session ID length mismatch");
    }
    if (memcmp(iso2DocDec->V2G_Message.Header.SessionID.bytes, iso15118.common.session_id, SESSION_ID_LENGTH) != 0) {
        logger.printfln("ISO2: Session ID mismatch");
    }

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
    if ((iso15118.charge_type == ISO15118::ChargeType::DC_ReadSocOnce) || (iso15118.charge_type == ISO15118::ChargeType::DC_ReadSocInLoop)) {
        res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[0] = iso2_EnergyTransferModeType_DC_extended;
        res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.arrayLen = 1;
    } else {
        res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.array[0] = iso2_EnergyTransferModeType_AC_single_phase_core;
        res->ChargeService.SupportedEnergyTransferMode.EnergyTransferMode.arrayLen = 1;
    }

    // Unique identifier of the service
    res->ChargeService.ServiceID = 1;

    res->ChargeService.ServiceName_isUsed = 0;
    res->ChargeService.ServiceScope_isUsed = 0;

    // EV can use offered service without payment
    res->ChargeService.FreeService = 1;

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = 3;
}

void ISO2::handle_payment_service_selection_req()
{
    iso2_PaymentServiceSelectionReqType *req = &iso2DocDec->V2G_Message.Body.PaymentServiceSelectionReq;
    iso2_PaymentServiceSelectionResType *res = &iso2DocEnc->V2G_Message.Body.PaymentServiceSelectionRes;

    if (req->SelectedPaymentOption == iso2_paymentOptionType_ExternalPayment) {
        iso2DocEnc->V2G_Message.Body.PaymentServiceSelectionRes_isUsed = 1;
        res->ResponseCode = iso2_responseCodeType_OK;

        iso15118.common.send_exi(Common::ExiType::Iso2);
        state = 4;
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

    state = 5;
}

void ISO2::handle_charge_parameter_discovery_req()
{
    iso2_ChargeParameterDiscoveryReqType* req = &iso2DocDec->V2G_Message.Body.ChargeParameterDiscoveryReq;
    iso2_ChargeParameterDiscoveryResType* res = &iso2DocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes;

    api_state.get("ac")->get("is_used")->updateInt(req->AC_EVChargeParameter_isUsed);
    if (req->AC_EVChargeParameter_isUsed) {
        api_state.get("ac")->get("departure_time")->updateInt(req->AC_EVChargeParameter.DepartureTime);
        api_state.get("ac")->get("departure_time_is_used")->updateBool(req->AC_EVChargeParameter.DepartureTime_isUsed);
        api_state.get("ac")->get("eamount_val")->updateInt(req->AC_EVChargeParameter.EAmount.Value);
        api_state.get("ac")->get("eamount_mul")->updateInt(req->AC_EVChargeParameter.EAmount.Multiplier);
        api_state.get("ac")->get("ev_max_voltage_val")->updateInt(req->AC_EVChargeParameter.EVMaxVoltage.Value);
        api_state.get("ac")->get("ev_max_voltage_mul")->updateInt(req->AC_EVChargeParameter.EVMaxVoltage.Multiplier);
        api_state.get("ac")->get("ev_max_current_val")->updateInt(req->AC_EVChargeParameter.EVMaxCurrent.Value);
        api_state.get("ac")->get("ev_max_current_mul")->updateInt(req->AC_EVChargeParameter.EVMaxCurrent.Multiplier);
        api_state.get("ac")->get("ev_min_current_val")->updateInt(req->AC_EVChargeParameter.EVMinCurrent.Value);
        api_state.get("ac")->get("ev_min_current_mul")->updateInt(req->AC_EVChargeParameter.EVMinCurrent.Multiplier);
    }

    api_state.get("dc")->get("is_used")->updateInt(req->DC_EVChargeParameter_isUsed);
    if (req->DC_EVChargeParameter_isUsed) {
        api_state.get("dc")->get("departure_time")->updateInt(req->DC_EVChargeParameter.DepartureTime);
        api_state.get("dc")->get("departure_time_is_used")->updateBool(req->DC_EVChargeParameter.DepartureTime_isUsed);
        api_state.get("dc")->get("soc")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
        api_state.get("dc")->get("ev_ready")->updateInt(req->DC_EVChargeParameter.DC_EVStatus.EVReady);
        api_state.get("dc")->get("ev_error_code")->updateUint(req->DC_EVChargeParameter.DC_EVStatus.EVErrorCode);
        api_state.get("dc")->get("ev_max_current_limit_val")->updateInt(req->DC_EVChargeParameter.EVMaximumCurrentLimit.Value);
        api_state.get("dc")->get("ev_max_current_limit_mul")->updateInt(req->DC_EVChargeParameter.EVMaximumCurrentLimit.Multiplier);
        api_state.get("dc")->get("ev_max_power_limit_val")->updateInt(req->DC_EVChargeParameter.EVMaximumPowerLimit.Value);
        api_state.get("dc")->get("ev_max_power_limit_mul")->updateInt(req->DC_EVChargeParameter.EVMaximumPowerLimit.Multiplier);
        api_state.get("dc")->get("ev_max_power_limit_is_used")->updateBool(req->DC_EVChargeParameter.EVMaximumPowerLimit_isUsed);
        api_state.get("dc")->get("ev_max_voltage_limit_val")->updateInt(req->DC_EVChargeParameter.EVMaximumVoltageLimit.Value);
        api_state.get("dc")->get("ev_max_voltage_limit_mul")->updateInt(req->DC_EVChargeParameter.EVMaximumVoltageLimit.Multiplier);
        api_state.get("dc")->get("ev_energy_capacity_val")->updateInt(req->DC_EVChargeParameter.EVEnergyCapacity.Value);
        api_state.get("dc")->get("ev_energy_capacity_mul")->updateInt(req->DC_EVChargeParameter.EVEnergyCapacity.Multiplier);
        api_state.get("dc")->get("ev_energy_capacity_is_used")->updateBool(req->DC_EVChargeParameter.EVEnergyCapacity_isUsed);
        api_state.get("dc")->get("ev_energy_request_val")->updateInt(req->DC_EVChargeParameter.EVEnergyRequest.Value);
        api_state.get("dc")->get("ev_energy_request_mul")->updateInt(req->DC_EVChargeParameter.EVEnergyRequest.Multiplier);
        api_state.get("dc")->get("ev_energy_request_is_used")->updateBool(req->DC_EVChargeParameter.EVEnergyRequest_isUsed);
        api_state.get("dc")->get("full_soc")->updateInt(req->DC_EVChargeParameter.FullSOC);
        api_state.get("dc")->get("full_soc_is_used")->updateBool(req->DC_EVChargeParameter.FullSOC_isUsed);
        api_state.get("dc")->get("bulk_soc")->updateInt(req->DC_EVChargeParameter.BulkSOC);
        api_state.get("dc")->get("bulk_soc_is_used")->updateBool(req->DC_EVChargeParameter.BulkSOC_isUsed);
    }


    // TODO: Check if EnergyTransferMode is as configured

    if (req->DC_EVChargeParameter_isUsed) {
        logger.printfln("ISO2: Current SoC %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);
    }

    iso2DocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1;

    // Here we try to get the EV into a loop that calls ChargeParameterDiscoveryReq again and again
    // so we are able to continously read the SoC.
    if (iso15118.charge_type == ISO15118::ChargeType::DC_ReadSocInLoop) {
        res->ResponseCode = iso2_responseCodeType_OK;
        res->EVSEProcessing = iso2_EVSEProcessingType_Ongoing;

        res->AC_EVSEChargeParameter_isUsed = 0;
        res->DC_EVSEChargeParameter_isUsed = 1;

        // Invalid = An isolation test has not been carried out.
        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus = iso2_isolationLevelType_Invalid;

        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification = iso2_EVSENotificationType_ReNegotiation;
        res->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay = 10;

        // EVSE_Ready here seems to work, maybe test iso2_DC_EVSEStatusCodeType_EVSE_IsolationMonitoringActive if other EVs send StopRequest after this.
        res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode = iso2_DC_EVSEStatusCodeType_EVSE_Ready;

        res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit = iso2_unitSymbolType_A;
        res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value = 500; // 500A
        res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier = 0;

        res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit = iso2_unitSymbolType_V;
        res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value = 800; // 800V
        res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Multiplier = 0;

        res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Unit = iso2_unitSymbolType_A;
        res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Value = 0; // 0A
        res->DC_EVSEChargeParameter.EVSEMinimumCurrentLimit.Multiplier = 0;

        res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit = iso2_unitSymbolType_V;
        res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Value = 0; // 0V
        res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Multiplier = 0;

        res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Unit = iso2_unitSymbolType_A;
        res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Value = 1; // 1A
        res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Multiplier = 0;

        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Unit = iso2_unitSymbolType_W;
        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Value = 20000; // 20000W * 10^1 = 200kW
        res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit.Multiplier = 1;

        res->DC_EVSEChargeParameter_isUsed = 1;

        // Optinal charge parameters
        res->DC_EVSEChargeParameter.EVSECurrentRegulationTolerance_isUsed = 0;
        res->DC_EVSEChargeParameter.EVSEEnergyToBeDelivered_isUsed = 0;
    } else {
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
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax.Value = 1500; // TODO: Use user configuration
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax.Multiplier = 0;
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax.Unit = iso2_unitSymbolType_W;

        // [V2G2-330] The value of the duration element shall be defined as period of time in seconds.
        res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration = 86400; // One day
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
        res->SAScheduleList.SAScheduleTuple.array[0].SAScheduleTupleID = 1; // [V2G2-773] 1-255 OK, 0 not allowed

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
        res->AC_EVSEChargeParameter.EVSEMaxCurrent.Value = 32;
        res->AC_EVSEChargeParameter.EVSEMaxCurrent.Multiplier = 0;
        res->AC_EVSEChargeParameter.EVSEMaxCurrent.Unit = iso2_unitSymbolType_A;

        // Line voltage supported by the EVSE. This is the voltage measured between
        // one phases and neutral. If the EVSE supports multiple phase charging the EV
        // might easily calculate the voltage between phases.
        res->AC_EVSEChargeParameter.EVSENominalVoltage.Value = 230;
        res->AC_EVSEChargeParameter.EVSENominalVoltage.Multiplier = 0;
        res->AC_EVSEChargeParameter.EVSENominalVoltage.Unit = iso2_unitSymbolType_V;
    }

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = 6;
}

void ISO2::handle_power_delivery_req()
{
    iso2_PowerDeliveryReqType *req = &iso2DocDec->V2G_Message.Body.PowerDeliveryReq;
    iso2_PowerDeliveryResType *res = &iso2DocEnc->V2G_Message.Body.PowerDeliveryRes;

    if (req->DC_EVPowerDeliveryParameter_isUsed) {
        api_state.get("dc")->get("soc")->updateInt(req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVRESSSOC);
        api_state.get("dc")->get("ev_ready")->updateInt(req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVReady);
        api_state.get("dc")->get("ev_error_code")->updateUint(req->DC_EVPowerDeliveryParameter.DC_EVStatus.EVErrorCode);
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
            evse_v2.set_charging_protocol(1, 50);
            break;
        case iso2_chargeProgressType_Stop:
            evse_v2.set_charging_protocol(1, 1000); // TODO: should we go to 100% duty cycle here or not?
            break;
        case iso2_chargeProgressType_Renegotiate:
            break;
    }

    // Unique identifier within a charging session for a SAScheduleTuple
    // element. An SAID remains a unique identifier for one schedule throughout
    // a charging session.
    if (req->SAScheduleTupleID != 1) {
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
    state = 7;
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

    res->SAScheduleTupleID = 1;

    res->ReceiptRequired_isUsed = 0;
    res->MeterInfo_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = 8;
}

void ISO2::handle_session_stop_req()
{
    iso2_SessionStopResType *res = &iso2DocEnc->V2G_Message.Body.SessionStopRes;

    iso2DocEnc->V2G_Message.Body.SessionStopRes_isUsed = 1;
    res->ResponseCode = iso2_responseCodeType_OK;

    iso15118.common.send_exi(Common::ExiType::Iso2);
    state = 9;
}

void ISO2::trace_header(struct iso2_MessageHeaderType *header)
{
    iso15118.trace("V2G_Message");
    iso15118.trace(" Header");
    iso15118.trace("  SessionID.bytes: %02x%02x%02x%02x", header->SessionID.bytes[0], header->SessionID.bytes[1], header->SessionID.bytes[2], header->SessionID.bytes[3]);
    iso15118.trace("  SessionID.bytesLen: %d", header->SessionID.bytesLen);
    iso15118.trace("  Notification: %d", header->Notification);
    iso15118.trace("  Notification_isUsed: %d", header->Notification_isUsed);
    iso15118.trace("  Signature: %d", header->Notification);
    iso15118.trace("  Signature_isUsed: %d", header->Signature_isUsed);
};

void ISO2::trace_request_response()
{
    if (iso2DocDec->V2G_Message.Body.SessionSetupReq_isUsed) {
        iso2_SessionSetupReqType *req = &iso2DocDec->V2G_Message.Body.SessionSetupReq;

        trace_header(&iso2DocDec->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  SessionSetupReq");
        iso15118.trace("   EVCCID: %02x%02x%02x%02x%02x%02x", req->EVCCID.bytes[0], req->EVCCID.bytes[1], req->EVCCID.bytes[2], req->EVCCID.bytes[3], req->EVCCID.bytes[4], req->EVCCID.bytes[5]);
        iso15118.trace("   EVCCID.bytesLen: %d", req->EVCCID.bytesLen);
    } else if (iso2DocDec->V2G_Message.Body.ServiceDiscoveryReq_isUsed) {
        iso2_ServiceDiscoveryReqType *req = &iso2DocDec->V2G_Message.Body.ServiceDiscoveryReq;

        trace_header(&iso2DocDec->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  ServiceDiscoveryReq");
        iso15118.trace("   ServiceCategory: %d", req->ServiceCategory);
        iso15118.trace("   ServiceScope: %s", req->ServiceScope.characters);
        iso15118.trace("   ServiceCategory_isUsed: %d", req->ServiceCategory_isUsed);
    } else if (iso2DocDec->V2G_Message.Body.PaymentServiceSelectionReq_isUsed) {
        iso2_PaymentServiceSelectionReqType *req = &iso2DocDec->V2G_Message.Body.PaymentServiceSelectionReq;

        trace_header(&iso2DocDec->V2G_Message.Header);
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

        trace_header(&iso2DocDec->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  AuthorizationReq");
        iso15118.trace("  GenChallenge_isUsed: %d", req->GenChallenge_isUsed);
        if (req->GenChallenge_isUsed) {
            iso15118.trace("   GenChallenge: %02x%02x%02x%02x...", req->GenChallenge.bytes[0], req->GenChallenge.bytes[1], req->GenChallenge.bytes[2], req->GenChallenge.bytes[3]);
        }
        if (req->Id_isUsed) {
            iso15118.trace("   Id: %s", req->Id.characters);
        }
    } else if (iso2DocDec->V2G_Message.Body.ChargeParameterDiscoveryReq_isUsed) {
        iso2_ChargeParameterDiscoveryReqType *req = &iso2DocDec->V2G_Message.Body.ChargeParameterDiscoveryReq;

        trace_header(&iso2DocDec->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  ChargeParameterDiscoveryReq");
        iso15118.trace("   MaxEntriesSAScheduleTuple: %d", req->MaxEntriesSAScheduleTuple);
        iso15118.trace("   MaxEntriesSAScheduleTuple_isUsed: %d", req->MaxEntriesSAScheduleTuple_isUsed);
        iso15118.trace("   RequestedEnergyTransferMode: %d", req->RequestedEnergyTransferMode);
        iso15118.trace("   AC_EVChargeParameter_isUsed: %d", req->AC_EVChargeParameter_isUsed);
        if (req->AC_EVChargeParameter_isUsed) {
            iso15118.trace("    DepartureTime: %d", req->AC_EVChargeParameter.DepartureTime);
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
            iso15118.trace("    DepartureTime: %d", req->DC_EVChargeParameter.DepartureTime);
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
        }
    } else if (iso2DocDec->V2G_Message.Body.PowerDeliveryReq_isUsed) {
        iso2_PowerDeliveryReqType *req = &iso2DocDec->V2G_Message.Body.PowerDeliveryReq;

        trace_header(&iso2DocDec->V2G_Message.Header);
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
                iso15118.trace("     ChargingProfileEntryStart: %d", req->ChargingProfile.ProfileEntry.array[i].ChargingProfileEntryStart);
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
        trace_header(&iso2DocDec->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  ChargingStatusReq");
    } else if (iso2DocDec->V2G_Message.Body.SessionStopReq_isUsed) {
        iso2_SessionStopReqType *req = &iso2DocDec->V2G_Message.Body.SessionStopReq;

        trace_header(&iso2DocDec->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  SessionStopReq");
        iso15118.trace("   ChargingSession: %d", req->ChargingSession);
    }

    if (iso2DocEnc->V2G_Message.Body.SessionSetupRes_isUsed) {
        iso2_SessionSetupResType *res = &iso2DocEnc->V2G_Message.Body.SessionSetupRes;

        trace_header(&iso2DocEnc->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  SessionSetupRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEID: %s", res->EVSEID.characters);
        iso15118.trace("   EVSEID.charactersLen: %d", res->EVSEID.charactersLen);
        iso15118.trace("   EVSETimeStamp: %ld", res->EVSETimeStamp);
        iso15118.trace("   EVSETimeStamp_isUsed: %ld", res->EVSETimeStamp_isUsed);
    } else if (iso2DocEnc->V2G_Message.Body.ServiceDiscoveryRes_isUsed) {
        iso2_ServiceDiscoveryResType *res = &iso2DocEnc->V2G_Message.Body.ServiceDiscoveryRes;

        trace_header(&iso2DocEnc->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  ServiceDiscoveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   PaymentOptionList: %d", res->PaymentOptionList.PaymentOption.array[0]);
        iso15118.trace("   ChargeService.ServiceID: %d", res->ChargeService.ServiceID);
    } else if (iso2DocEnc->V2G_Message.Body.PaymentServiceSelectionRes_isUsed) {
        iso2_PaymentServiceSelectionResType *res = &iso2DocEnc->V2G_Message.Body.PaymentServiceSelectionRes;

        trace_header(&iso2DocEnc->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  PaymentServiceSelectionRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
    } else if (iso2DocEnc->V2G_Message.Body.AuthorizationRes_isUsed) {
        iso2_AuthorizationResType *res = &iso2DocEnc->V2G_Message.Body.AuthorizationRes;

        trace_header(&iso2DocEnc->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  AuthorizationRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEProcessing: %d", res->EVSEProcessing);
    } else if (iso2DocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed) {
        iso2_ChargeParameterDiscoveryResType *res = &iso2DocEnc->V2G_Message.Body.ChargeParameterDiscoveryRes;

        trace_header(&iso2DocEnc->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  ChargeParameterDiscoveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEProcessing: %d", res->EVSEProcessing);
        iso15118.trace("   AC_EVSEChargeParameter_isUsed: %d", res->AC_EVSEChargeParameter_isUsed);
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

                        if (res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval_isUsed) {
                            iso15118.trace("       RelativeTimeInterval");
                            iso15118.trace("        start: %d", res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.start);
                            iso15118.trace("        duration: %d", res->SAScheduleList.SAScheduleTuple.array[i].PMaxSchedule.PMaxScheduleEntry.array[j].RelativeTimeInterval.duration);
                        }
                    }
                }
                iso15118.trace("     SalesTariff_isUsed: %d", res->SAScheduleList.SAScheduleTuple.array[i].SalesTariff_isUsed);
            }
        }

        if (res->AC_EVSEChargeParameter_isUsed) {
            iso15118.trace("    EVSEMaxCurrent.Value: %d", res->AC_EVSEChargeParameter.EVSEMaxCurrent.Value);
            iso15118.trace("    EVSEMaxCurrent.Multiplier: %d", res->AC_EVSEChargeParameter.EVSEMaxCurrent.Multiplier);
            iso15118.trace("    EVSEMaxCurrent.Unit: %d", res->AC_EVSEChargeParameter.EVSEMaxCurrent.Unit);
            iso15118.trace("    EVSENominalVoltage.Value: %d", res->AC_EVSEChargeParameter.EVSENominalVoltage.Value);
            iso15118.trace("    EVSENominalVoltage.Multiplier: %d", res->AC_EVSEChargeParameter.EVSENominalVoltage.Multiplier);
            iso15118.trace("    EVSENominalVoltage.Unit: %d", res->AC_EVSEChargeParameter.EVSENominalVoltage.Unit);
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
        }
    } else if (iso2DocEnc->V2G_Message.Body.PowerDeliveryRes_isUsed) {
        iso2_PowerDeliveryResType *res = &iso2DocEnc->V2G_Message.Body.PowerDeliveryRes;

        trace_header(&iso2DocEnc->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  PowerDeliveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        if (res->AC_EVSEStatus_isUsed) {
            iso15118.trace("   AC_EVSEStatus");
            iso15118.trace("    RCD: %d", res->AC_EVSEStatus.RCD);
            iso15118.trace("    NotificationMaxDelay: %d", res->AC_EVSEStatus.NotificationMaxDelay);
            iso15118.trace("    EVSENotification: %d", res->AC_EVSEStatus.EVSENotification);
        }

        if (res->DC_EVSEStatus_isUsed) {
            iso15118.trace("   DC_EVSEStatus");
            iso15118.trace("    EVSEIsolationStatus: %d", res->DC_EVSEStatus.EVSEIsolationStatus);
            iso15118.trace("    EVSENotification: %d", res->DC_EVSEStatus.EVSENotification);
            iso15118.trace("    NotificationMaxDelay: %d", res->DC_EVSEStatus.NotificationMaxDelay);
            iso15118.trace("    EVSEStatusCode: %d", res->DC_EVSEStatus.EVSEStatusCode);
        }
    } else if (iso2DocEnc->V2G_Message.Body.ChargingStatusRes_isUsed) {
        iso2_ChargingStatusResType *res = &iso2DocEnc->V2G_Message.Body.ChargingStatusRes;

        trace_header(&iso2DocEnc->V2G_Message.Header);
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

        trace_header(&iso2DocEnc->V2G_Message.Header);
        iso15118.trace(" Body");
        iso15118.trace("  SessionStopRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
    }
}