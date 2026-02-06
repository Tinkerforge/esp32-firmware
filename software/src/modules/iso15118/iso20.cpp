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

// IPv6/TCP/ISO-15118-20
// This implementation follows the requirements from ISO 15118-20:2022.
// Requirements are referenced using [V2G20-xxx] notation.

#include "iso20.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools/hexdump.h"
#include "tools/malloc.h"

void ISO20::pre_setup()
{
    api_state = Config::Object({
        {"state", Config::Uint8(0)},
        {"session_id", Config::Str("", 0, 16)},  // 8 bytes as hex string (16 chars)
        {"evcc_id", Config::Str("", 0, 20)},     // Up to 20 character string
        // DisplayParameters from AC_ChargeLoopReq (all optional)
        {"present_soc", Config::Int8(-1)},
        {"present_soc_is_used", Config::Bool(false)},
        {"minimum_soc", Config::Int8(-1)},
        {"minimum_soc_is_used", Config::Bool(false)},
        {"target_soc", Config::Int8(-1)},
        {"target_soc_is_used", Config::Bool(false)},
        {"maximum_soc", Config::Int8(-1)},
        {"maximum_soc_is_used", Config::Bool(false)},
        {"remaining_time_to_minimum_soc", Config::Uint32(0)},
        {"remaining_time_to_minimum_soc_is_used", Config::Bool(false)},
        {"remaining_time_to_target_soc", Config::Uint32(0)},
        {"remaining_time_to_target_soc_is_used", Config::Bool(false)},
        {"remaining_time_to_maximum_soc", Config::Uint32(0)},
        {"remaining_time_to_maximum_soc_is_used", Config::Bool(false)},
        {"charging_complete", Config::Bool(false)},
        {"charging_complete_is_used", Config::Bool(false)},
        {"battery_energy_capacity_val", Config::Int16(0)},
        {"battery_energy_capacity_exp", Config::Int8(0)},
        {"battery_energy_capacity_is_used", Config::Bool(false)},
        {"inlet_hot", Config::Bool(false)},
        {"inlet_hot_is_used", Config::Bool(false)},
        // EV present active power from Dynamic control mode
        {"ev_present_active_power_val", Config::Int16(0)},
        {"ev_present_active_power_exp", Config::Int8(0)},
    });
}

void ISO20::handle_bitstream(exi_bitstream *exi, V2GTPPayloadType payload_type)
{
    // Increment state on first call
    if (state == 0) {
        state = 1;
    }

    // Handle AC-specific messages
    if (payload_type == V2GTPPayloadType::ISO20AC) {
        handle_ac_bitstream(exi);
        return;
    }

    // Handle common messages
    // We alloc the iso20 buffers the very first time they are used.
    // This way it is not allocated if ISO15118-20 is not used.
    // If it is used once we can assume that it will be used all the time, so it stays allocated.
    if (iso20DocDec == nullptr) {
        iso20DocDec = static_cast<struct iso20_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct iso20_exiDocument)));
    }
    if (iso20DocEnc == nullptr) {
        iso20DocEnc = static_cast<struct iso20_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct iso20_exiDocument)));
    }
    memset(iso20DocDec, 0, sizeof(struct iso20_exiDocument));
    memset(iso20DocEnc, 0, sizeof(struct iso20_exiDocument));

    int ret = decode_iso20_exiDocument(exi, iso20DocDec);
    if (ret != 0) {
        logger.printfln("ISO20: Could not decode EXI document: %d", ret);
        return;
    }

    cancel_sequence_timeout(next_timeout);

    dispatch_common_messages();

    trace_request_response();

    api_state.get("state")->updateUint(state);

    // [V2G20-435] The SECC shall implement the SECC specific timeouts and performance times
    //             defined in Table 214 and Table 215.
    // [V2G20-441] The SECC shall set V2G_SECC_Sequence_Timeout and start monitoring when
    //             it sends a response message.
    // [V2G20-443] The SECC shall stop waiting for a request message when the timer expires.
    schedule_sequence_timeout(next_timeout, ISO20_SECC_SEQUENCE_TIMEOUT, "ISO20");
}

void ISO20::dispatch_common_messages()
{
    auto &doc = *iso20DocDec;

    // SessionSetupReq - no session validation needed (session is established here)
    V2G_DISPATCH("ISO20", doc, SessionSetupReq, handle_session_setup_req);
    if (doc.SessionSetupReq_isUsed) return;

    // [V2G20-753] All messages after SessionSetup require session ID validation.
    // If the SessionID received does not match the previously communicated SessionID,
    // the SECC shall respond with FAILED_UnknownSession.
    // Note: All request types in the union have Header as their first member,
    // so we can access it directly by casting to the header type.
    auto *header = reinterpret_cast<const struct iso20_MessageHeaderType*>(&doc.AuthorizationSetupReq);
    if (!validate_session_id(header->SessionID.bytes,
                             ISO20_SESSION_ID_LENGTH,
                             session_id,
                             ISO20_SESSION_ID_LENGTH)) {
        logger.printfln("ISO20: Session ID mismatch, sending FAILED_UnknownSession");
        send_failed_unknown_session();
        return;
    }

    // Implemented message handlers (session already validated)
    V2G_DISPATCH("ISO20", doc, AuthorizationSetupReq, handle_authorization_setup_req);
    V2G_DISPATCH("ISO20", doc, AuthorizationReq,      handle_authorization_req);
    V2G_DISPATCH("ISO20", doc, ServiceDiscoveryReq,   handle_service_discovery_req);
    V2G_DISPATCH("ISO20", doc, ServiceDetailReq,      handle_service_detail_req);
    V2G_DISPATCH("ISO20", doc, ServiceSelectionReq,   handle_service_selection_req);
    V2G_DISPATCH("ISO20", doc, ScheduleExchangeReq,   handle_schedule_exchange_req);
    V2G_DISPATCH("ISO20", doc, PowerDeliveryReq,      handle_power_delivery_req);
    V2G_DISPATCH("ISO20", doc, SessionStopReq,        handle_session_stop_req);

    // Not yet implemented

    // This is for Plug&Charge certificate handling, currently not supported
    V2G_NOT_IMPL("ISO20", doc, CertificateInstallationReq);

    // This is to Plug&Charge signed meter data transfer, currently not supported
    V2G_NOT_IMPL("ISO20", doc, MeteringConfirmationReq);

    // These are used for automated/robotic charging system
    // like pantograph chargers or robotic arm chargers.
    // See Section 8.3.4.8.1.1 of ISO 15118-20:2022.
    // We will not support these.
    V2G_NOT_IMPL("ISO20", doc, VehicleCheckInReq);
    V2G_NOT_IMPL("ISO20", doc, VehicleCheckOutReq);
}

void ISO20::send_failed_unknown_session()
{
    auto &doc = *iso20DocDec;

    // Determine which message type was received and send the appropriate error response
    if (false
        || (V2G20_SEND_FAILED_SESSION(doc, iso20DocEnc, AuthorizationSetup, iso20_responseCodeType_FAILED_UnknownSession, prepare_header))
        || (V2G20_SEND_FAILED_SESSION(doc, iso20DocEnc, Authorization,      iso20_responseCodeType_FAILED_UnknownSession, prepare_header))
        || (V2G20_SEND_FAILED_SESSION(doc, iso20DocEnc, ServiceDiscovery,   iso20_responseCodeType_FAILED_UnknownSession, prepare_header))
        || (V2G20_SEND_FAILED_SESSION(doc, iso20DocEnc, ServiceDetail,      iso20_responseCodeType_FAILED_UnknownSession, prepare_header))
        || (V2G20_SEND_FAILED_SESSION(doc, iso20DocEnc, ServiceSelection,   iso20_responseCodeType_FAILED_UnknownSession, prepare_header))
        || (V2G20_SEND_FAILED_SESSION(doc, iso20DocEnc, ScheduleExchange,   iso20_responseCodeType_FAILED_UnknownSession, prepare_header))
        || (V2G20_SEND_FAILED_SESSION(doc, iso20DocEnc, PowerDelivery,      iso20_responseCodeType_FAILED_UnknownSession, prepare_header))
        || (V2G20_SEND_FAILED_SESSION(doc, iso20DocEnc, SessionStop,        iso20_responseCodeType_FAILED_UnknownSession, prepare_header))
    ) {
        iso15118.common.send_exi(Common::ExiType::Iso20);
        return;
    }

    logger.printfln("ISO20: Unknown message type for FAILED_UnknownSession");
}

void ISO20::prepare_header(struct iso20_MessageHeaderType *header)
{
    // Copy session ID (8 bytes for ISO 20)
    for (uint8_t i = 0; i < ISO20_SESSION_ID_LENGTH; i++) {
        header->SessionID.bytes[i] = session_id[i];
    }
    header->SessionID.bytesLen = ISO20_SESSION_ID_LENGTH;

    // Timestamp is Unix timestamp (seconds since epoch)
    timeval now;
    if (!rtc.clock_synced(&now)) {
        now.tv_sec = 0;
    }
    header->TimeStamp = now.tv_sec;

    // Signature not used (no Plug&Charge)
    header->Signature_isUsed = 0;
}

void ISO20::handle_session_setup_req()
{
    // [V2G20-751] The SessionID value returned by the SECC in SessionSetupRes shall not
    //             change as long as the V2G communication session is not terminated.
    // [V2G20-752] The header of any message sent by the SECC during an active V2G
    //             communication session shall contain the same SessionID.
    iso20_SessionSetupReqType *req = &iso20DocDec->SessionSetupReq;
    iso20_SessionSetupResType *res = &iso20DocEnc->SessionSetupRes;

    // Reset soc_read flag for new session
    soc_read = false;
    ev_supports_asymmetric = false;

    // Store EVCCID in API state (up to 20 characters)
    char evcc_id_str[21] = {0};
    size_t evcc_id_len = req->EVCCID.charactersLen;
    if (evcc_id_len > 20) evcc_id_len = 20;
    memcpy(evcc_id_str, req->EVCCID.characters, evcc_id_len);
    evcc_id_str[evcc_id_len] = '\0';
    api_state.get("evcc_id")->updateString(evcc_id_str);

    // Check if session ID is all zeros (new session) or matches existing session (resume)
    bool all_zero = true;
    for (size_t i = 0; i < req->Header.SessionID.bytesLen; i++) {
        if (req->Header.SessionID.bytes[i] != 0x00) {
            all_zero = false;
            break;
        }
    }

    // Check if received session ID matches our stored session ID
    bool different_to_known = false;
    if (req->Header.SessionID.bytesLen == ISO20_SESSION_ID_LENGTH) {
        for (uint16_t i = 0; i < ISO20_SESSION_ID_LENGTH; i++) {
            if (req->Header.SessionID.bytes[i] != session_id[i]) {
                different_to_known = true;
                break;
            }
        }
    } else {
        different_to_known = true;
    }

    // Generate new session ID if requested or session ID doesn't match
    if (all_zero || different_to_known) {
        // Generate new random session ID (8 bytes for ISO 20)
        for (uint8_t i = 0; i < ISO20_SESSION_ID_LENGTH; i++) {
            session_id[i] = static_cast<uint8_t>(random(256));
        }
        res->ResponseCode = iso20_responseCodeType_OK_NewSessionEstablished;
    } else {
        res->ResponseCode = iso20_responseCodeType_OK_OldSessionJoined;
    }

    // Store session ID in API state as hex string
    char session_id_hex[17];
    hexdump(session_id, ISO20_SESSION_ID_LENGTH, session_id_hex, sizeof(session_id_hex), HexdumpCase::Upper);
    api_state.get("session_id")->updateString(session_id_hex);

    iso20DocEnc->SessionSetupRes_isUsed = 1;
    prepare_header(&res->Header);

    // [V2G20-192] The SECC and the EVCC shall use the format for EVSEID as defined in Annex C.
    //             If an SECC cannot provide such ID data, the value of the EVSEID is set to "ZZ00000".
    strcpy(res->EVSEID.characters, "ZZ00000");
    res->EVSEID.charactersLen = strlen("ZZ00000");

    iso15118.common.send_exi(Common::ExiType::Iso20);
    state = 2;
}

void ISO20::handle_authorization_setup_req()
{
    // [V2G20-1220] If the SECC does not offer PnC, it shall use EIM_ASResAuthorizationMode.
    // [V2G20-2567] If the SECC offers only EIM, it shall include EIM_ASResAuthorizationMode.
    iso20_AuthorizationSetupResType *res = &iso20DocEnc->AuthorizationSetupRes;

    iso20DocEnc->AuthorizationSetupRes_isUsed = 1;
    prepare_header(&res->Header);

    res->ResponseCode = iso20_responseCodeType_OK;

    // Offer only EIM (External Identification Means) authorization - no Plug&Charge
    res->AuthorizationServices.array[0] = iso20_authorizationType_EIM;
    res->AuthorizationServices.arrayLen = 1;

    // We don't support certificate installation service
    res->CertificateInstallationService = 0;

    // EIM authorization mode is used (empty structure, but must be marked as used)
    res->EIM_ASResAuthorizationMode_isUsed = 1;
    res->PnC_ASResAuthorizationMode_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Iso20);
    state = 3;
}

void ISO20::handle_authorization_req()
{
    // [V2G20-1200] In case of EIM, the SECC shall send AuthorizationRes with ResponseCode
    //              = "OK" and EVSEProcessing = "Finished" once authorization is complete.
    // [V2G20-2560] In case EIM was selected, elements prefixed with EIM shall be used.
    iso20_AuthorizationReqType *req = &iso20DocDec->AuthorizationReq;
    iso20_AuthorizationResType *res = &iso20DocEnc->AuthorizationRes;

    iso20DocEnc->AuthorizationRes_isUsed = 1;
    prepare_header(&res->Header);

    // Only accept EIM authorization
    if (req->SelectedAuthorizationService == iso20_authorizationType_EIM) {
        res->ResponseCode = iso20_responseCodeType_OK;
    } else {
        // If PnC was selected, respond with warning
        res->ResponseCode = iso20_responseCodeType_WARNING_AuthorizationSelectionInvalid;
        logger.printfln("ISO20: PnC authorization not supported, sending warning");
    }

    // Authorization is immediately finished (no waiting for external authorization)
    res->EVSEProcessing = iso20_processingType_Finished;

    iso15118.common.send_exi(Common::ExiType::Iso20);
    state = 4;
}

void ISO20::handle_service_discovery_req()
{
    // [V2G20-1980] After receiving the ServiceDiscoveryReq, the SECC shall respond with a
    //              ServiceDiscoveryRes within V2G_SECC_Msg_Performance_Time.
    // [V2G20-1931] To indicate that SECC is capable of a ServiceRenegotiation, the SECC shall set
    //              ServiceRenegotiationSupported to "True"/"False" in the first ServiceDiscoveryRes.
    // [V2G20-1249] The EVCC and the SECC shall implement the message elements as defined in Table 39.
    iso20_ServiceDiscoveryResType *res = &iso20DocEnc->ServiceDiscoveryRes;

    iso20DocEnc->ServiceDiscoveryRes_isUsed = 1;
    prepare_header(&res->Header);

    res->ResponseCode = iso20_responseCodeType_OK;

    // [V2G20-1931] To indicate that SECC is capable of a ServiceRenegotiation, the SECC shall set
    //              ServiceRenegotiationSupported to "True" in the first ServiceDiscoveryRes.
    // [V2G20-1932] The value shall be identical throughout the session.
    res->ServiceRenegotiationSupported = 1;

    // Offer AC charging service (ISO 15118-20 Table 203)
    res->EnergyTransferServiceList.Service.array[0].ServiceID = V2G_SERVICE_ID_CHARGING;
    res->EnergyTransferServiceList.Service.array[0].FreeService = 1;  // Free (no payment needed)
    res->EnergyTransferServiceList.Service.arrayLen = 1;

    // No value-added services
    res->VASList_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Iso20);
    state = 5;
}

void ISO20::handle_service_detail_req()
{
    // [V2G20-1981] After receiving the ServiceDetailReq the SECC shall respond with a ServiceDetailRes
    //              within V2G_SECC_Msg_Performance_Time. The allowed next requests shall be either
    //              ServiceDetailReq or ServiceSelectionReq.
    // [V2G20-425]  The SECC shall respond with ResponseCode "FAILED_ServiceIDInvalid" if the EVCC
    //              provided a not previously offered ServiceID in the ServiceDetailReq.
    // [V2G20-1251] The EVCC and the SECC shall implement the message elements as defined in Table 41.
    iso20_ServiceDetailReqType *req = &iso20DocDec->ServiceDetailReq;
    iso20_ServiceDetailResType *res = &iso20DocEnc->ServiceDetailRes;

    iso20DocEnc->ServiceDetailRes_isUsed = 1;
    prepare_header(&res->Header);

    // Check if the requested service is valid (we only offer AC = ServiceID 1)
    if (req->ServiceID != V2G_SERVICE_ID_CHARGING) {
        res->ResponseCode = iso20_responseCodeType_FAILED_ServiceIDInvalid;
        res->ServiceID = req->ServiceID;
        res->ServiceParameterList.ParameterSet.arrayLen = 0;
        iso15118.common.send_exi(Common::ExiType::Iso20);
        return;
    }

    res->ResponseCode = iso20_responseCodeType_OK;
    res->ServiceID = req->ServiceID;

    // Define AC service parameters per ISO 15118-20 Table 205:
    // - Connector: 1=SinglePhase, 2=ThreePhase (usage of the connector)
    // - ControlMode: 1=Scheduled, 2=Dynamic
    // - EVSENominalVoltage: Line voltage in V (between one phase and neutral)
    // - MobilityNeedsMode: 1=ProvidedByEvcc, 2=ProvidedBySecc
    // - Pricing: 0=NoPricing, 1=AbsolutePricing, 2=PriceLevels
    //
    // We offer two ParameterSets to enable ServiceRenegotiation for phase switching:
    //   ParameterSetID 1: ThreePhase, Dynamic, 230V, ProvidedByEvcc, NoPricing
    //   ParameterSetID 2: SinglePhase, Dynamic, 230V, ProvidedByEvcc, NoPricing
    // The EV selects one during ServiceSelection and can switch via ServiceRenegotiation.

    // Helper to fill one ParameterSet with 5 parameters (only Connector differs)
    auto fill_parameter_set = [](iso20_ParameterSetType *ps, uint16_t set_id, int connector_type) {
        ps->ParameterSetID = set_id;

        // Parameter 1: Connector - 1=SinglePhase, 2=ThreePhase per ISO 15118-20 Table 205
        // [V2G20-1815] SinglePhase: only base power elements (L1) shall be used.
        // [V2G20-1816] ThreePhase: _L2 and _L3 suffixed elements shall be allowed.
        auto &p0 = ps->Parameter.array[0];
        strcpy(p0.Name.characters, "Connector");
        p0.Name.charactersLen = strlen("Connector");
        p0.intValue = connector_type;  // ISO20_CONNECTOR_SINGLE_PHASE or ISO20_CONNECTOR_THREE_PHASE
        p0.intValue_isUsed = 1;
        p0.boolValue_isUsed = 0;
        p0.byteValue_isUsed = 0;
        p0.shortValue_isUsed = 0;
        p0.finiteString_isUsed = 0;
        p0.rationalNumber_isUsed = 0;

        // Parameter 2: ControlMode - 1=Scheduled, 2=Dynamic
        auto &p1 = ps->Parameter.array[1];
        strcpy(p1.Name.characters, "ControlMode");
        p1.Name.charactersLen = strlen("ControlMode");
        p1.intValue = ISO20_CONTROL_MODE_DYNAMIC;
        p1.intValue_isUsed = 1;
        p1.boolValue_isUsed = 0;
        p1.byteValue_isUsed = 0;
        p1.shortValue_isUsed = 0;
        p1.finiteString_isUsed = 0;
        p1.rationalNumber_isUsed = 0;

        // Parameter 3: EVSENominalVoltage - line voltage between one phase and neutral
        auto &p2 = ps->Parameter.array[2];
        strcpy(p2.Name.characters, "EVSENominalVoltage");
        p2.Name.charactersLen = strlen("EVSENominalVoltage");
        p2.intValue = V2G_NOMINAL_VOLTAGE_V;
        p2.intValue_isUsed = 1;
        p2.boolValue_isUsed = 0;
        p2.byteValue_isUsed = 0;
        p2.shortValue_isUsed = 0;
        p2.finiteString_isUsed = 0;
        p2.rationalNumber_isUsed = 0;

        // Parameter 4: MobilityNeedsMode - 1=ProvidedByEvcc, 2=ProvidedBySecc
        auto &p3 = ps->Parameter.array[3];
        strcpy(p3.Name.characters, "MobilityNeedsMode");
        p3.Name.charactersLen = strlen("MobilityNeedsMode");
        p3.intValue = ISO20_MOBILITY_NEEDS_PROVIDED_BY_EVCC;
        p3.intValue_isUsed = 1;
        p3.boolValue_isUsed = 0;
        p3.byteValue_isUsed = 0;
        p3.shortValue_isUsed = 0;
        p3.finiteString_isUsed = 0;
        p3.rationalNumber_isUsed = 0;

        // Parameter 5: Pricing - 0=NoPricing, 1=AbsolutePricing, 2=PriceLevels
        auto &p4 = ps->Parameter.array[4];
        strcpy(p4.Name.characters, "Pricing");
        p4.Name.charactersLen = strlen("Pricing");
        p4.intValue = ISO20_PRICING_NONE;
        p4.intValue_isUsed = 1;
        p4.boolValue_isUsed = 0;
        p4.byteValue_isUsed = 0;
        p4.shortValue_isUsed = 0;
        p4.finiteString_isUsed = 0;
        p4.rationalNumber_isUsed = 0;

        ps->Parameter.arrayLen = 5;
    };

    // ParameterSetID 1: ThreePhase (Connector = 2)
    fill_parameter_set(&res->ServiceParameterList.ParameterSet.array[0], ISO20_PARAM_SET_THREE_PHASE, ISO20_CONNECTOR_THREE_PHASE);
    // ParameterSetID 2: SinglePhase (Connector = 1)
    fill_parameter_set(&res->ServiceParameterList.ParameterSet.array[1], ISO20_PARAM_SET_SINGLE_PHASE, ISO20_CONNECTOR_SINGLE_PHASE);
    res->ServiceParameterList.ParameterSet.arrayLen = 2;

    // Send response
    iso15118.common.send_exi(Common::ExiType::Iso20);
    state = 6;
}

void ISO20::handle_service_selection_req()
{
    // [V2G20-1983] After receiving the ServiceSelectionReq, the SECC shall respond with
    //              ServiceSelectionRes within V2G_SECC_Msg_Performance_Time.
    // [V2G20-433]  The SECC shall respond with ResponseCode "FAILED_ServiceSelectionInvalid" if the
    //              EVCC provided a not previously offered ServiceID, ParameterSetID pair.
    // [V2G20-1253] The EVCC and the SECC shall implement the message elements as defined in Table 43.
    iso20_ServiceSelectionReqType *req = &iso20DocDec->ServiceSelectionReq;
    iso20_ServiceSelectionResType *res = &iso20DocEnc->ServiceSelectionRes;

    iso20DocEnc->ServiceSelectionRes_isUsed = 1;
    prepare_header(&res->Header);

    // Validate the selected service
    // We offer AC = ServiceID 1 with ParameterSetID 1 (ThreePhase) or 2 (SinglePhase)
    if (req->SelectedEnergyTransferService.ServiceID != V2G_SERVICE_ID_CHARGING ||
        (req->SelectedEnergyTransferService.ParameterSetID != ISO20_PARAM_SET_THREE_PHASE &&
         req->SelectedEnergyTransferService.ParameterSetID != ISO20_PARAM_SET_SINGLE_PHASE)) {
        res->ResponseCode = iso20_responseCodeType_FAILED_ServiceSelectionInvalid;
        iso15118.common.send_exi(Common::ExiType::Iso20);
        return;
    }

    // Store the selected service for later use
    selected_service_id = req->SelectedEnergyTransferService.ServiceID;
    selected_parameter_set_id = req->SelectedEnergyTransferService.ParameterSetID;

    res->ResponseCode = iso20_responseCodeType_OK;

    iso15118.common.send_exi(Common::ExiType::Iso20);
    state = 7;
}

void ISO20::handle_schedule_exchange_req()
{
    // [V2G20-1987] After receiving the ScheduleExchangeReq, the SECC shall respond with a
    //              ScheduleExchangeRes with EVSEProcessing set to "Finished" and ResponseCode
    //              starting with "OK" within V2G_SECC_Msg_Performance_Time, if all parameters
    //              are available.
    // [V2G20-1989] (AC) After receiving the ScheduleExchangeReq, the SECC shall respond with a
    //              ScheduleExchangeRes with EVSEProcessing set to "Finished". The next allowed
    //              request shall be PowerDeliveryReq if ServiceName= AC or AC_BPT was selected.
    // [V2G20-1563] If the EVCC did not provide a DepartureTime, the sum of the individual time
    //              intervals in the PowerSchedule and PriceSchedule shall be >= 24h.
    // [V2G20-1267] The EVCC and the SECC shall implement the message elements as defined in Table 44.
    iso20_ScheduleExchangeReqType *req = &iso20DocDec->ScheduleExchangeReq;
    iso20_ScheduleExchangeResType *res = &iso20DocEnc->ScheduleExchangeRes;

    (void)req;  // Currently unused

    iso20DocEnc->ScheduleExchangeRes_isUsed = 1;
    prepare_header(&res->Header);

    res->ResponseCode = iso20_responseCodeType_OK;
    res->EVSEProcessing = iso20_processingType_Finished;
    res->GoToPause_isUsed = 0;

    // Use Dynamic control mode (not Scheduled)
    // In Dynamic mode, the SECC controls charging power in real-time via EVSETargetActivePower
    // in AC_ChargeLoopRes. No PowerSchedule or PriceSchedule is needed.
    res->Scheduled_SEResControlMode_isUsed = 0;
    res->Dynamic_SEResControlMode_isUsed = 1;

    // All fields in Dynamic_SEResControlMode are optional
    res->Dynamic_SEResControlMode.DepartureTime_isUsed = 0;
    res->Dynamic_SEResControlMode.MinimumSOC_isUsed = 0;
    res->Dynamic_SEResControlMode.TargetSOC_isUsed = 0;
    res->Dynamic_SEResControlMode.AbsolutePriceSchedule_isUsed = 0;
    res->Dynamic_SEResControlMode.PriceLevelSchedule_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Iso20);
    state = 8;
}

void ISO20::handle_power_delivery_req()
{
    // [V2G20-1615] The EVCC shall use PowerDeliveryReq with ChargeProgress="Start" to start HLC-C
    //              based energy transfer.
    // [V2G20-1311] The EVCC shall use PowerDeliveryReq with ChargeProgress="Stop" to stop HLC-C
    //              based energy transfer.
    // [V2G20-1385] The SECC shall enter standby after receiving PowerDeliveryReq with ChargeProgress
    //              set to "Standby" by sending PowerDeliveryRes with ResponseCode="OK".
    // [V2G20-1386] The SECC shall exit standby after receiving PowerDeliveryReq with ChargeProgress
    //              set to "Start", "Stop", or "Renegotiate" by sending PowerDeliveryRes with
    //              ResponseCode="OK".
    // [V2G20-1262] The EVCC and the SECC shall implement the message elements as defined in Table 47.
    //
    // PWM note: Unlike ISO 15118-2 [V2G2-866] which requires PWM=100% after PowerDeliveryRes on Stop/error,
    // ISO 15118-20 [V2G20-1408] requires the SECC to keep 5% duty cycle from start of data link setup
    // until end of V2G communication session. The session ends at SessionStopReq(Terminate), not at
    // PowerDeliveryReq(Stop). So we must NOT switch to 100% PWM on Stop in ISO20.
    iso20_PowerDeliveryReqType *req = &iso20DocDec->PowerDeliveryReq;
    iso20_PowerDeliveryResType *res = &iso20DocEnc->PowerDeliveryRes;

    iso20DocEnc->PowerDeliveryRes_isUsed = 1;
    prepare_header(&res->Header);

    res->ResponseCode = iso20_responseCodeType_OK;

    // Handle different charge progress states
    switch (req->ChargeProgress) {
        case iso20_chargeProgressType_Start:
            // Ensure EVSE bricklet is in ISO15118 mode with 5% PWM.
            // [V2G20-1408] SECC shall apply CP duty cycle of 5% from start of data link setup
            //              until end of V2G communication session.
            // This is already set in common.cpp during SupportedAppProtocolRes, but we set it
            // again here defensively in case the state was changed externally.
            evse_v2.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 50);
            state = 9;
            break;

        case iso20_chargeProgressType_Stop:
            // EV wants to stop energy transfer. Session remains active.
            // [V2G20-1408] Keep 5% PWM. Do NOT switch to 100% (unlike ISO2 [V2G2-866]).
            state = 10;
            break;

        case iso20_chargeProgressType_Standby:
            // EV wants to go to standby (power reduced but session active)
            break;

        case iso20_chargeProgressType_ScheduleRenegotiation:
            // EV wants to renegotiate the schedule
            break;
    }

    // Optionally provide EVSE status
    res->EVSEStatus_isUsed = 0;  // TODO: We should probably provide some status here, maybe can be shown in ev display

    iso15118.common.send_exi(Common::ExiType::Iso20);
}

void ISO20::handle_session_stop_req()
{
    // [V2G20-1632] After receiving the SessionStopReq, the SECC shall respond with a SessionStopRes
    //              within V2G_SECC_Msg_Performance_Time.
    // [V2G20-1633] After sending a SessionStopRes, the SECC shall wait at least 5s before closing
    //              the TCP connection.
    // [V2G20-1080] After confirming a SessionStopReq with ChargingSession="Terminate" by a
    //              SessionStopRes with ResponseCode="OK", the SECC shall discard all session related
    //              information (session ID, selected charge service, authorization details).
    // [V2G20-1955] SessionStopRes shall contain ResponseCode "FAILED_NoServiceRenegotiationSupported"
    //              if SessionStopReq with ChargingSession="ServiceRenegotiation" is received and
    //              ServiceRenegotiationSupported was set to "False".
    // [V2G20-1272] The EVCC and the SECC shall implement the message elements as defined in Table 51.
    iso20_SessionStopReqType *req = &iso20DocDec->SessionStopReq;
    iso20_SessionStopResType *res = &iso20DocEnc->SessionStopRes;

    iso20DocEnc->SessionStopRes_isUsed = 1;
    prepare_header(&res->Header);

    // Handle different charging session states
    switch (req->ChargingSession) {
        case iso20_chargingSessionType_Terminate:
            // EV wants to terminate the session completely
            res->ResponseCode = iso20_responseCodeType_OK;
            state = 11;
            break;

        case iso20_chargingSessionType_Pause:
            // EV wants to pause the session (can resume later)
            res->ResponseCode = iso20_responseCodeType_OK;
            state = 12;
            break;

        case iso20_chargingSessionType_ServiceRenegotiation:
            // [V2G20-1476] EVCC sends SessionStopReq(ChargingSession=ServiceRenegotiation)
            // [V2G20-1969] AC flow: PowerDeliveryReq(Stop) -> SessionStopReq(ServiceRenegotiation)
            //              -> ServiceDiscoveryReq. Session ID is preserved.
            // [V2G20-1931] We advertised ServiceRenegotiationSupported=true, so we accept this.
            res->ResponseCode = iso20_responseCodeType_OK;
            // Reset state to ServiceDiscovery (state 4) so the EV can re-select services.
            // The session ID remains valid. This is not a new session.
            state = 4;
            logger.printfln("ISO20: ServiceRenegotiation requested, returning to ServiceDiscovery");
            break;
    }

    // Send response
    iso15118.common.send_exi(Common::ExiType::Iso20);

    // In read_soc_only mode, switch to IEC 61851 after the session ends.
    // The EVSE will control charging via PWM and revert to ISO 15118 on EV disconnect.
    if (iso15118.is_read_soc_only()) {
        iso15118.switch_to_iec_temporary();
    }

    // For Terminate and Pause, reset the session after sending response
    if (req->ChargingSession == iso20_chargingSessionType_Terminate ||
        req->ChargingSession == iso20_chargingSessionType_Pause) {
        // Cancel any pending sequence timeout using the shared helper
        cancel_sequence_timeout(next_timeout);

        // [V2G20-1633] Wait at least 5s before closing TCP connection
        // Schedule delayed socket reset to allow EV to close first
        // Capture the current socket fd to avoid closing a new session's socket
        // if a new connection arrives before the 5s delay expires
        int socket_to_close = iso15118.common.get_active_socket();
        task_scheduler.scheduleOnce([socket_to_close]() {
            // Only close if this is still the same socket (no new session connected)
            if (iso15118.common.get_active_socket() == socket_to_close) {
                iso15118.common.reset_active_socket();
            }
        }, 5_s);
    }
}

void ISO20::trace_header(const struct iso20_MessageHeaderType *header, const char *name)
{
    iso15118.trace("ISO20 Message (%s)", name);
    iso15118.trace(" Header");
    iso15118.trace("  SessionID: %02x%02x%02x%02x%02x%02x%02x%02x",
                   header->SessionID.bytes[0], header->SessionID.bytes[1],
                   header->SessionID.bytes[2], header->SessionID.bytes[3],
                   header->SessionID.bytes[4], header->SessionID.bytes[5],
                   header->SessionID.bytes[6], header->SessionID.bytes[7]);
    iso15118.trace("  SessionID.bytesLen: %d", header->SessionID.bytesLen);
    iso15118.trace("  TimeStamp: %llu", header->TimeStamp);
    iso15118.trace("  Signature_isUsed: %d", header->Signature_isUsed);
}

// ======================================================
// AC-specific message handlers (for payload type 0x8003)
// ======================================================

void ISO20::handle_ac_bitstream(exi_bitstream *exi)
{
    // Allocate AC document buffers on first use
    if (iso20AcDocDec == nullptr) {
        iso20AcDocDec = static_cast<struct iso20_ac_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct iso20_ac_exiDocument)));
    }
    if (iso20AcDocEnc == nullptr) {
        iso20AcDocEnc = static_cast<struct iso20_ac_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct iso20_ac_exiDocument)));
    }
    memset(iso20AcDocDec, 0, sizeof(struct iso20_ac_exiDocument));
    memset(iso20AcDocEnc, 0, sizeof(struct iso20_ac_exiDocument));

    int ret = decode_iso20_ac_exiDocument(exi, iso20AcDocDec);
    if (ret != 0) {
        logger.printfln("ISO20 AC: Could not decode EXI document: %d", ret);
        return;
    }

    cancel_sequence_timeout(next_timeout);

    dispatch_ac_messages();

    trace_ac_request_response();

    api_state.get("state")->updateUint(state);

    // [V2G20-435] [V2G20-441] [V2G20-443] Sequence timer handling (see common messages above)
    schedule_sequence_timeout(next_timeout, ISO20_SECC_SEQUENCE_TIMEOUT, "ISO20 AC");
}

void ISO20::dispatch_ac_messages()
{
    auto &doc = *iso20AcDocDec;

    // [V2G20-753] All AC messages require session ID validation.
    // Note: All request types in the union have Header as their first member,
    // so we can access it directly by casting to the header type.
    auto *header = reinterpret_cast<const struct iso20_ac_MessageHeaderType*>(&doc.AC_ChargeParameterDiscoveryReq);
    if (!validate_session_id(header->SessionID.bytes,
                             ISO20_SESSION_ID_LENGTH,
                             session_id,
                             ISO20_SESSION_ID_LENGTH)) {
        logger.printfln("ISO20 AC: Session ID mismatch, sending FAILED_UnknownSession");
        send_ac_failed_unknown_session();
        return;
    }

    // AC-specific message dispatch (session already validated)
    V2G_DISPATCH("ISO20 AC", doc, AC_ChargeParameterDiscoveryReq, handle_ac_charge_parameter_discovery_req);
    V2G_DISPATCH("ISO20 AC", doc, AC_ChargeLoopReq,               handle_ac_charge_loop_req);
}

void ISO20::send_ac_failed_unknown_session()
{
    auto &doc = *iso20AcDocDec;

    // Determine which AC message type was received and send the appropriate error response
    if (false
        || (V2G20_SEND_FAILED_SESSION(doc, iso20AcDocEnc, AC_ChargeParameterDiscovery, iso20_ac_responseCodeType_FAILED_UnknownSession, prepare_ac_header))
        || (V2G20_SEND_FAILED_SESSION(doc, iso20AcDocEnc, AC_ChargeLoop,               iso20_ac_responseCodeType_FAILED_UnknownSession, prepare_ac_header))
    ) {
        iso15118.common.send_exi(Common::ExiType::Iso20Ac);
        return;
    }

    logger.printfln("ISO20 AC: Unknown message type for FAILED_UnknownSession");
}

void ISO20::prepare_ac_header(struct iso20_ac_MessageHeaderType *header)
{
    // Copy session ID (8 bytes for ISO 20)
    for (uint8_t i = 0; i < ISO20_SESSION_ID_LENGTH; i++) {
        header->SessionID.bytes[i] = session_id[i];
    }
    header->SessionID.bytesLen = ISO20_SESSION_ID_LENGTH;

    // Timestamp is Unix timestamp (seconds since epoch)
    timeval now;
    if (!rtc.clock_synced(&now)) {
        now.tv_sec = 0;
    }
    header->TimeStamp = now.tv_sec;

    // Signature not used (no Plug&Charge)
    header->Signature_isUsed = 0;
}

void ISO20::handle_ac_charge_parameter_discovery_req()
{
    // [V2G20-2314] Requirements for ChargeParameterDiscoveryReq/Res apply to all energy
    //              transfer mode specific extensions, e.g. AC_ChargeParameterDiscoveryReq/Res.
    iso20_ac_AC_ChargeParameterDiscoveryReqType *req = &iso20AcDocDec->AC_ChargeParameterDiscoveryReq;
    iso20_ac_AC_ChargeParameterDiscoveryResType *res = &iso20AcDocEnc->AC_ChargeParameterDiscoveryRes;

    // Extract EV minimum charge power if available
    if (req->AC_CPDReqEnergyTransferMode_isUsed) {
        EVData ev_data;
        ev_data.min_power = physical_value_to_float(&req->AC_CPDReqEnergyTransferMode.EVMinimumChargePower);
        ev_data.max_power = physical_value_to_float(&req->AC_CPDReqEnergyTransferMode.EVMaximumChargePower);
        iso15118.common.update_ev_data(ev_data, EVDataProtocol::ISO20);

        // [V2G20-1821] If the EV is capable of asymmetric energy transfer, it communicates
        //              EVMaximumChargePower with phase-specific power values (_L2/_L3) in CPDReq.
        // [V2G20-1820] If the EV only sends the base elements (sum), it is symmetric-only.
        // [V2G20-1822] SECC shall not send _L2/_L3 unless the EV declared this capability.
        ev_supports_asymmetric = req->AC_CPDReqEnergyTransferMode.EVMaximumChargePower_L2_isUsed
                              || req->AC_CPDReqEnergyTransferMode.EVMaximumChargePower_L3_isUsed;
        if (ev_supports_asymmetric) {
            logger.printfln("ISO20 AC: EV supports asymmetric power (per-phase control available)");
        }
    }

    iso20AcDocEnc->AC_ChargeParameterDiscoveryRes_isUsed = 1;
    prepare_ac_header(&res->Header);

    res->ResponseCode = iso20_ac_responseCodeType_OK;

    // Provide AC charge parameters (non-BPT mode)
    res->AC_CPDResEnergyTransferMode_isUsed = 1;
    res->BPT_AC_CPDResEnergyTransferMode_isUsed = 0;

    // EVSEMaximumChargePower and EVSEMinimumChargePower from current charging parameters
    const ChargingInformation ci = iso15118.get_charging_information();

    // Compute per-phase power in milliwatts, then find the smallest exponent fitting int16_t.
    uint32_t per_phase_power_mw = static_cast<uint32_t>(ci.current_ma) * V2G_NOMINAL_VOLTAGE_V;

    if (ev_supports_asymmetric && is_selected_three_phase()) {
        // [V2G20-1818] When _L2/_L3 are present, the base element becomes L1-specific.
        // [V2G20-1819] Same rules apply to _L2 and _L3 as to the base element.
        // Report per-phase max power on each phase (same value = even distribution).
        const ScaledPower sp = encode_milliwatts(per_phase_power_mw);
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower.Value = sp.value;
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower.Exponent = sp.exponent;

        // L2 and L3 get the same per-phase power (even distribution)
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower_L2_isUsed = 1;
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower_L2.Value = sp.value;
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower_L2.Exponent = sp.exponent;
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower_L3_isUsed = 1;
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower_L3.Value = sp.value;
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower_L3.Exponent = sp.exponent;

        // EVSEMinimumChargePower: 0W per phase (no minimum)
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower.Exponent = 0;
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower.Value = 0;
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower_L2_isUsed = 1;
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower_L2.Exponent = 0;
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower_L2.Value = 0;
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower_L3_isUsed = 1;
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower_L3.Exponent = 0;
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower_L3.Value = 0;
    } else {
        // Symmetric mode: base element = sum of all phases per [V2G20-1817].
        // Phase multiplier is determined by the selected ParameterSet (1=ThreePhase, 2=SinglePhase),
        // NOT by get_charging_information().three_phase — the ParameterSet is the contract with the EV.
        // [V2G20-1817] For ThreePhase, when _L2/_L3 are not present, the base element represents the
        //              sum of all three lines. An even distribution across all three phases shall be applied.
        // [V2G20-1815] For SinglePhase, only the base element (L1) shall be used.
        uint32_t total_power_mw = per_phase_power_mw * (is_selected_three_phase() ? 3 : 1);
        const ScaledPower sp = encode_milliwatts(total_power_mw);
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower.Value = sp.value;
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower.Exponent = sp.exponent;

        // [V2G20-1822] SECC shall not send _L2, _L3 values unless the EV declared asymmetric capability.
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower_L2_isUsed = 0;
        res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower_L3_isUsed = 0;

        // EVSEMinimumChargePower: 0W (no minimum)
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower.Exponent = 0;
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower.Value = 0;
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower_L2_isUsed = 0;
        res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower_L3_isUsed = 0;
    }

    // [V2G20-1828] MaximumPowerAsymmetry: if not present, no asymmetric limitation applies.
    // We intentionally omit it so the SECC can set any per-phase power distribution.

    // EVSENominalFrequency: 50Hz (Europe) = 50 * 10^0
    res->AC_CPDResEnergyTransferMode.EVSENominalFrequency.Exponent = 0;
    res->AC_CPDResEnergyTransferMode.EVSENominalFrequency.Value = V2G_NOMINAL_FREQUENCY_HZ;

    // Optional fields not used
    res->AC_CPDResEnergyTransferMode.MaximumPowerAsymmetry_isUsed = 0;
    res->AC_CPDResEnergyTransferMode.EVSEPowerRampLimitation_isUsed = 0;
    res->AC_CPDResEnergyTransferMode.EVSEPresentActivePower_isUsed = 0;
    res->AC_CPDResEnergyTransferMode.EVSEPresentActivePower_L2_isUsed = 0;
    res->AC_CPDResEnergyTransferMode.EVSEPresentActivePower_L3_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Iso20Ac);
    state = 13;
}

void ISO20::handle_ac_charge_loop_req()
{
    // [V2G20-2315] Requirements for ChargeLoopReq/Res apply to all energy transfer mode
    //              specific extensions, e.g. AC_ChargeLoopReq/Res.
    // [V2G20-902] The MeterInfo in ChargeLoopRes shall be the amount of energy charged
    //             during the current service session.
    iso20_ac_AC_ChargeLoopReqType *req = &iso20AcDocDec->AC_ChargeLoopReq;
    iso20_ac_AC_ChargeLoopResType *res = &iso20AcDocEnc->AC_ChargeLoopRes;

    // Extract DisplayParameters if provided (this is unique to ISO 20 AC)
    if (req->DisplayParameters_isUsed) {
        // PresentSOC - Current battery state of charge
        api_state.get("present_soc_is_used")->updateBool(req->DisplayParameters.PresentSOC_isUsed);
        if (req->DisplayParameters.PresentSOC_isUsed) {
            api_state.get("present_soc")->updateInt(req->DisplayParameters.PresentSOC);
        }

        // MinimumSOC - Minimum acceptable SOC
        api_state.get("minimum_soc_is_used")->updateBool(req->DisplayParameters.MinimumSOC_isUsed);
        if (req->DisplayParameters.MinimumSOC_isUsed) {
            api_state.get("minimum_soc")->updateInt(req->DisplayParameters.MinimumSOC);
        }

        // TargetSOC - Desired target SOC
        api_state.get("target_soc_is_used")->updateBool(req->DisplayParameters.TargetSOC_isUsed);
        if (req->DisplayParameters.TargetSOC_isUsed) {
            api_state.get("target_soc")->updateInt(req->DisplayParameters.TargetSOC);
        }

        // MaximumSOC - Maximum SOC limit
        api_state.get("maximum_soc_is_used")->updateBool(req->DisplayParameters.MaximumSOC_isUsed);
        if (req->DisplayParameters.MaximumSOC_isUsed) {
            api_state.get("maximum_soc")->updateInt(req->DisplayParameters.MaximumSOC);
        }

        // RemainingTimeToMinimumSOC
        api_state.get("remaining_time_to_minimum_soc_is_used")->updateBool(req->DisplayParameters.RemainingTimeToMinimumSOC_isUsed);
        if (req->DisplayParameters.RemainingTimeToMinimumSOC_isUsed) {
            api_state.get("remaining_time_to_minimum_soc")->updateUint(req->DisplayParameters.RemainingTimeToMinimumSOC);
        }

        // RemainingTimeToTargetSOC
        api_state.get("remaining_time_to_target_soc_is_used")->updateBool(req->DisplayParameters.RemainingTimeToTargetSOC_isUsed);
        if (req->DisplayParameters.RemainingTimeToTargetSOC_isUsed) {
            api_state.get("remaining_time_to_target_soc")->updateUint(req->DisplayParameters.RemainingTimeToTargetSOC);
        }

        // RemainingTimeToMaximumSOC
        api_state.get("remaining_time_to_maximum_soc_is_used")->updateBool(req->DisplayParameters.RemainingTimeToMaximumSOC_isUsed);
        if (req->DisplayParameters.RemainingTimeToMaximumSOC_isUsed) {
            api_state.get("remaining_time_to_maximum_soc")->updateUint(req->DisplayParameters.RemainingTimeToMaximumSOC);
        }

        // ChargingComplete
        api_state.get("charging_complete_is_used")->updateBool(req->DisplayParameters.ChargingComplete_isUsed);
        if (req->DisplayParameters.ChargingComplete_isUsed) {
            api_state.get("charging_complete")->updateBool(req->DisplayParameters.ChargingComplete);
        }

        // BatteryEnergyCapacity (RationalNumber)
        api_state.get("battery_energy_capacity_is_used")->updateBool(req->DisplayParameters.BatteryEnergyCapacity_isUsed);
        if (req->DisplayParameters.BatteryEnergyCapacity_isUsed) {
            api_state.get("battery_energy_capacity_val")->updateInt(req->DisplayParameters.BatteryEnergyCapacity.Value);
            api_state.get("battery_energy_capacity_exp")->updateInt(req->DisplayParameters.BatteryEnergyCapacity.Exponent);
        }

        // InletHot is a safety alert
        api_state.get("inlet_hot_is_used")->updateBool(req->DisplayParameters.InletHot_isUsed);
        if (req->DisplayParameters.InletHot_isUsed) {
            api_state.get("inlet_hot")->updateBool(req->DisplayParameters.InletHot);
            if (req->DisplayParameters.InletHot) {
                logger.printfln("ISO20 AC: WARNING: InletHot = true!");
            }
        }
    }

    // Extract EV present power if using Dynamic mode
    if (req->Dynamic_AC_CLReqControlMode_isUsed) {
        api_state.get("ev_present_active_power_val")->updateInt(req->Dynamic_AC_CLReqControlMode.EVPresentActivePower.Value);
        api_state.get("ev_present_active_power_exp")->updateInt(req->Dynamic_AC_CLReqControlMode.EVPresentActivePower.Exponent);
    }

    // Update EV data for meters module
    {
        EVData ev_data;

        if (req->DisplayParameters_isUsed) {
            if (req->DisplayParameters.PresentSOC_isUsed) {
                ev_data.soc_present = static_cast<float>(req->DisplayParameters.PresentSOC);
            }
            if (req->DisplayParameters.TargetSOC_isUsed) {
                ev_data.soc_target = static_cast<float>(req->DisplayParameters.TargetSOC);
            }
            if (req->DisplayParameters.MinimumSOC_isUsed) {
                ev_data.soc_min = static_cast<float>(req->DisplayParameters.MinimumSOC);
            }
            if (req->DisplayParameters.MaximumSOC_isUsed) {
                ev_data.soc_max = static_cast<float>(req->DisplayParameters.MaximumSOC);
            }
            if (req->DisplayParameters.BatteryEnergyCapacity_isUsed) {
                // Convert from rational number (value * 10^exponent) to kWh
                // The original value is in Wh.
                ev_data.capacity_kwh = physical_value_to_float(&req->DisplayParameters.BatteryEnergyCapacity) / 1000.0f;
            }
            if (req->DisplayParameters.RemainingTimeToTargetSOC_isUsed) {
                ev_data.remaining_time_to_target_soc = static_cast<float>(req->DisplayParameters.RemainingTimeToTargetSOC);
            }
            if (req->DisplayParameters.ChargingComplete_isUsed) {
                ev_data.charging_complete = req->DisplayParameters.ChargingComplete;
            }
        }

        // Extract present power from Dynamic mode if available
        if (req->Dynamic_AC_CLReqControlMode_isUsed) {
            ev_data.present_power = physical_value_to_float(&req->Dynamic_AC_CLReqControlMode.EVPresentActivePower);
        }

        iso15118.common.update_ev_data(ev_data, EVDataProtocol::ISO20);
    }

    iso20AcDocEnc->AC_ChargeLoopRes_isUsed = 1;
    prepare_ac_header(&res->Header);

    res->ResponseCode = iso20_ac_responseCodeType_OK;

    // In read_soc_only mode, signal the EV to terminate after the first SoC reading.
    // [V2G20-1414] SECC sets EVSENotification to "Terminate" to request session termination.
    // [V2G20-1416] EVCC shall stop charging within NotificationMaxDelay seconds.
    // [V2G20-2645] EVCC shall initiate shutdown and send SessionStopReq with Terminate.
    if (iso15118.is_read_soc_only() && soc_read) {
        res->EVSEStatus_isUsed = 1;
        res->EVSEStatus.NotificationMaxDelay = 0;
        res->EVSEStatus.EVSENotification = iso20_ac_evseNotificationType_Terminate;
    }
    // Check if the charge manager wants a different phase mode than currently selected.
    // If the EV supports asymmetric power, we handle this via per-phase EVSETargetActivePower
    // below. No ServiceRenegotiation needed.
    // If the EV does NOT support asymmetric, request ServiceRenegotiation so the EV can
    // re-select the appropriate ParameterSet (ThreePhase vs SinglePhase).
    // [V2G20-1964] EVCC shall initiate ServiceRenegotiation when EVSENotification=ServiceRenegotiation.
    else if (!ev_supports_asymmetric && iso15118.get_charging_information().three_phase != is_selected_three_phase()) {
        res->EVSEStatus_isUsed = 1;
        res->EVSEStatus.NotificationMaxDelay = 10;  // EV must initiate renegotiation within 10 seconds
        res->EVSEStatus.EVSENotification = iso20_ac_evseNotificationType_ServiceRenegotiation;
    } else {
        // EVSE status not provided
        res->EVSEStatus_isUsed = 0;

        // Track first SoC reading in read_soc_only mode
        if (iso15118.is_read_soc_only()) {
            soc_read = true;
        }
    }

    // Metering info not provided (even if requested)
    res->MeterInfo_isUsed = 0;

    // Receipt not provided
    res->Receipt_isUsed = 0;

    // Target frequency not provided
    res->EVSETargetFrequency_isUsed = 0;

    // Control mode response: Use Dynamic mode.
    // [V2G20-1825] The EV shall adjust its energy transfer behavior to the new setpoints
    // as fast as technically feasible.
    res->Scheduled_AC_CLResControlMode_isUsed = 0;
    res->Dynamic_AC_CLResControlMode_isUsed = 1;
    res->BPT_Scheduled_AC_CLResControlMode_isUsed = 0;
    res->BPT_Dynamic_AC_CLResControlMode_isUsed = 0;
    res->CLResControlMode_isUsed = 0;

    // EVSETargetActivePower is MANDATORY in dynamic mode. This is the primary power control mechanism.
    // [V2G20-1823] EVSETargetActivePower shall always be based on EVSENominalVoltage.
    const ChargingInformation ci = iso15118.get_charging_information();

    if (ev_supports_asymmetric && is_selected_three_phase()) {
        // Asymmetric per-phase power control path.
        // [V2G20-1818] When _L2/_L3 are present, the base element becomes L1-specific (not a sum).
        // [V2G20-1825] The EV shall adjust to new setpoints as fast as technically feasible.
        // This allows dynamic switching between 1-phase and 3-phase without ServiceRenegotiation.

        // L1 always gets the per-phase power
        uint32_t l1_power_mw = static_cast<uint32_t>(ci.current_ma) * V2G_NOMINAL_VOLTAGE_V;
        const ScaledPower l1 = encode_milliwatts(l1_power_mw);
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower.Value = l1.value;
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower.Exponent = l1.exponent;

        // L2 and L3: same as L1 when charge manager wants 3-phase, zero when it wants 1-phase.
        // [V2G20-1819] Same rules apply to _L2/_L3 as to the base element.
        uint32_t l23_power_mw = ci.three_phase ? l1_power_mw : 0;
        const ScaledPower l23 = encode_milliwatts(l23_power_mw);
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower_L2_isUsed = 1;
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower_L2.Value = l23.value;
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower_L2.Exponent = l23.exponent;
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower_L3_isUsed = 1;
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower_L3.Value = l23.value;
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower_L3.Exponent = l23.exponent;
    } else {
        // Symmetric power control path.
        // [V2G20-1817] For ThreePhase without _L2/_L3, base element = total power summed across
        //              all three lines. The EV shall apply an even distribution across all phases.
        // [V2G20-1815] For SinglePhase, base element = power on L1 only.
        // [V2G20-1822] SECC shall not send _L2/_L3 unless the EV declared asymmetric capability.
        uint32_t power_mw = static_cast<uint32_t>(ci.current_ma) * V2G_NOMINAL_VOLTAGE_V * (is_selected_three_phase() ? 3 : 1);
        const ScaledPower sp = encode_milliwatts(power_mw);
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower.Value = sp.value;
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower.Exponent = sp.exponent;

        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower_L2_isUsed = 0;
        res->Dynamic_AC_CLResControlMode.EVSETargetActivePower_L3_isUsed = 0;
    }

    // Reactive power control not used
    res->Dynamic_AC_CLResControlMode.EVSETargetReactivePower_isUsed = 0;
    res->Dynamic_AC_CLResControlMode.EVSETargetReactivePower_L2_isUsed = 0;
    res->Dynamic_AC_CLResControlMode.EVSETargetReactivePower_L3_isUsed = 0;

    // Present active power not provided (we don't have metering at this level)
    res->Dynamic_AC_CLResControlMode.EVSEPresentActivePower_isUsed = 0;
    res->Dynamic_AC_CLResControlMode.EVSEPresentActivePower_L2_isUsed = 0;
    res->Dynamic_AC_CLResControlMode.EVSEPresentActivePower_L3_isUsed = 0;

    // Inherited Dynamic_CLResControlMode fields (all optional)
    res->Dynamic_AC_CLResControlMode.DepartureTime_isUsed = 0;
    res->Dynamic_AC_CLResControlMode.MinimumSOC_isUsed = 0;
    res->Dynamic_AC_CLResControlMode.TargetSOC_isUsed = 0;
    res->Dynamic_AC_CLResControlMode.AckMaxDelay_isUsed = 0;

    iso15118.common.send_exi(Common::ExiType::Iso20Ac);
    state = 14;
}

// =============================
// Trace functions for debugging
// =============================

void ISO20::trace_request_response()
{
    // === REQUEST MESSAGES ===

    if (iso20DocDec->SessionSetupReq_isUsed) {
        iso20_SessionSetupReqType *req = &iso20DocDec->SessionSetupReq;

        trace_header(&req->Header, "SessionSetupReq");
        iso15118.trace(" Body");
        iso15118.trace("  SessionSetupReq");
        iso15118.trace("   EVCCID: %.*s", req->EVCCID.charactersLen, req->EVCCID.characters);
        iso15118.trace("   EVCCID.charactersLen: %d", req->EVCCID.charactersLen);
    }

    if (iso20DocDec->AuthorizationSetupReq_isUsed) {
        iso20_AuthorizationSetupReqType *req = &iso20DocDec->AuthorizationSetupReq;

        trace_header(&req->Header, "AuthorizationSetupReq");
        iso15118.trace(" Body");
        iso15118.trace("  AuthorizationSetupReq (empty body)");
    }

    if (iso20DocDec->AuthorizationReq_isUsed) {
        iso20_AuthorizationReqType *req = &iso20DocDec->AuthorizationReq;

        trace_header(&req->Header, "AuthorizationReq");
        iso15118.trace(" Body");
        iso15118.trace("  AuthorizationReq");
        iso15118.trace("   SelectedAuthorizationService: %d", req->SelectedAuthorizationService);
        iso15118.trace("   EIM_AReqAuthorizationMode_isUsed: %d", req->EIM_AReqAuthorizationMode_isUsed);
        iso15118.trace("   PnC_AReqAuthorizationMode_isUsed: %d", req->PnC_AReqAuthorizationMode_isUsed);
    }

    if (iso20DocDec->ServiceDiscoveryReq_isUsed) {
        iso20_ServiceDiscoveryReqType *req = &iso20DocDec->ServiceDiscoveryReq;

        trace_header(&req->Header, "ServiceDiscoveryReq");
        iso15118.trace(" Body");
        iso15118.trace("  ServiceDiscoveryReq");
        iso15118.trace("   SupportedServiceIDs_isUsed: %d", req->SupportedServiceIDs_isUsed);
        if (req->SupportedServiceIDs_isUsed) {
            for (int i = 0; i < req->SupportedServiceIDs.ServiceID.arrayLen; i++) {
                iso15118.trace("   SupportedServiceIDs.ServiceID[%d]: %d", i, req->SupportedServiceIDs.ServiceID.array[i]);
            }
        }
    }

    if (iso20DocDec->ServiceDetailReq_isUsed) {
        iso20_ServiceDetailReqType *req = &iso20DocDec->ServiceDetailReq;

        trace_header(&req->Header, "ServiceDetailReq");
        iso15118.trace(" Body");
        iso15118.trace("  ServiceDetailReq");
        iso15118.trace("   ServiceID: %d", req->ServiceID);
    }

    if (iso20DocDec->ServiceSelectionReq_isUsed) {
        iso20_ServiceSelectionReqType *req = &iso20DocDec->ServiceSelectionReq;

        trace_header(&req->Header, "ServiceSelectionReq");
        iso15118.trace(" Body");
        iso15118.trace("  ServiceSelectionReq");
        iso15118.trace("   SelectedEnergyTransferService.ServiceID: %d", req->SelectedEnergyTransferService.ServiceID);
        iso15118.trace("   SelectedEnergyTransferService.ParameterSetID: %d", req->SelectedEnergyTransferService.ParameterSetID);
        iso15118.trace("   SelectedVASList_isUsed: %d", req->SelectedVASList_isUsed);
        if (req->SelectedVASList_isUsed) {
            for (int i = 0; i < req->SelectedVASList.SelectedService.arrayLen; i++) {
                iso15118.trace("   SelectedVASList.SelectedService[%d].ServiceID: %d", i, req->SelectedVASList.SelectedService.array[i].ServiceID);
                iso15118.trace("   SelectedVASList.SelectedService[%d].ParameterSetID: %d", i, req->SelectedVASList.SelectedService.array[i].ParameterSetID);
            }
        }
    }

    if (iso20DocDec->ScheduleExchangeReq_isUsed) {
        iso20_ScheduleExchangeReqType *req = &iso20DocDec->ScheduleExchangeReq;

        trace_header(&req->Header, "ScheduleExchangeReq");
        iso15118.trace(" Body");
        iso15118.trace("  ScheduleExchangeReq");
        iso15118.trace("   MaximumSupportingPoints: %d", req->MaximumSupportingPoints);
        iso15118.trace("   Dynamic_SEReqControlMode_isUsed: %d", req->Dynamic_SEReqControlMode_isUsed);
        iso15118.trace("   Scheduled_SEReqControlMode_isUsed: %d", req->Scheduled_SEReqControlMode_isUsed);
        if (req->Scheduled_SEReqControlMode_isUsed) {
            iso15118.trace("    Scheduled_SEReqControlMode");
            iso15118.trace("     DepartureTime_isUsed: %d", req->Scheduled_SEReqControlMode.DepartureTime_isUsed);
            if (req->Scheduled_SEReqControlMode.DepartureTime_isUsed) {
                iso15118.trace("     DepartureTime: %lu", (unsigned long)req->Scheduled_SEReqControlMode.DepartureTime);
            }
            iso15118.trace("     EVTargetEnergyRequest_isUsed: %d", req->Scheduled_SEReqControlMode.EVTargetEnergyRequest_isUsed);
            if (req->Scheduled_SEReqControlMode.EVTargetEnergyRequest_isUsed) {
                iso15118.trace("     EVTargetEnergyRequest: %d * 10^%d", req->Scheduled_SEReqControlMode.EVTargetEnergyRequest.Value, req->Scheduled_SEReqControlMode.EVTargetEnergyRequest.Exponent);
            }
            iso15118.trace("     EVMaximumEnergyRequest_isUsed: %d", req->Scheduled_SEReqControlMode.EVMaximumEnergyRequest_isUsed);
            if (req->Scheduled_SEReqControlMode.EVMaximumEnergyRequest_isUsed) {
                iso15118.trace("     EVMaximumEnergyRequest: %d * 10^%d", req->Scheduled_SEReqControlMode.EVMaximumEnergyRequest.Value, req->Scheduled_SEReqControlMode.EVMaximumEnergyRequest.Exponent);
            }
            iso15118.trace("     EVMinimumEnergyRequest_isUsed: %d", req->Scheduled_SEReqControlMode.EVMinimumEnergyRequest_isUsed);
            if (req->Scheduled_SEReqControlMode.EVMinimumEnergyRequest_isUsed) {
                iso15118.trace("     EVMinimumEnergyRequest: %d * 10^%d", req->Scheduled_SEReqControlMode.EVMinimumEnergyRequest.Value, req->Scheduled_SEReqControlMode.EVMinimumEnergyRequest.Exponent);
            }
            iso15118.trace("     EVEnergyOffer_isUsed: %d", req->Scheduled_SEReqControlMode.EVEnergyOffer_isUsed);
        }
    }

    if (iso20DocDec->PowerDeliveryReq_isUsed) {
        iso20_PowerDeliveryReqType *req = &iso20DocDec->PowerDeliveryReq;

        trace_header(&req->Header, "PowerDeliveryReq");
        iso15118.trace(" Body");
        iso15118.trace("  PowerDeliveryReq");
        iso15118.trace("   EVProcessing: %d", req->EVProcessing);
        iso15118.trace("   ChargeProgress: %d", req->ChargeProgress);
        iso15118.trace("   EVPowerProfile_isUsed: %d", req->EVPowerProfile_isUsed);
        if (req->EVPowerProfile_isUsed) {
            iso15118.trace("    EVPowerProfile");
            iso15118.trace("     TimeAnchor: %llu", req->EVPowerProfile.TimeAnchor);
            for (int i = 0; i < req->EVPowerProfile.EVPowerProfileEntries.EVPowerProfileEntry.arrayLen; i++) {
                iso15118.trace("     EVPowerProfileEntry[%d]", i);
                iso15118.trace("      Duration: %lu", (unsigned long)req->EVPowerProfile.EVPowerProfileEntries.EVPowerProfileEntry.array[i].Duration);
                iso15118.trace("      Power: %d * 10^%d",
                               req->EVPowerProfile.EVPowerProfileEntries.EVPowerProfileEntry.array[i].Power.Value,
                               req->EVPowerProfile.EVPowerProfileEntries.EVPowerProfileEntry.array[i].Power.Exponent);
            }
            iso15118.trace("     Dynamic_EVPPTControlMode_isUsed: %d", req->EVPowerProfile.Dynamic_EVPPTControlMode_isUsed);
            iso15118.trace("     Scheduled_EVPPTControlMode_isUsed: %d", req->EVPowerProfile.Scheduled_EVPPTControlMode_isUsed);
            if (req->EVPowerProfile.Scheduled_EVPPTControlMode_isUsed) {
                iso15118.trace("      SelectedScheduleTupleID: %lu", (unsigned long)req->EVPowerProfile.Scheduled_EVPPTControlMode.SelectedScheduleTupleID);
                iso15118.trace("      PowerToleranceAcceptance_isUsed: %d", req->EVPowerProfile.Scheduled_EVPPTControlMode.PowerToleranceAcceptance_isUsed);
            }
        }
        iso15118.trace("   BPT_ChannelSelection_isUsed: %d", req->BPT_ChannelSelection_isUsed);
    }

    if (iso20DocDec->SessionStopReq_isUsed) {
        iso20_SessionStopReqType *req = &iso20DocDec->SessionStopReq;

        trace_header(&req->Header, "SessionStopReq");
        iso15118.trace(" Body");
        iso15118.trace("  SessionStopReq");
        iso15118.trace("   ChargingSession: %d", req->ChargingSession);
        iso15118.trace("   EVTerminationCode_isUsed: %d", req->EVTerminationCode_isUsed);
        if (req->EVTerminationCode_isUsed) {
            iso15118.trace("   EVTerminationCode: %.*s", req->EVTerminationCode.charactersLen, req->EVTerminationCode.characters);
        }
        iso15118.trace("   EVTerminationExplanation_isUsed: %d", req->EVTerminationExplanation_isUsed);
        if (req->EVTerminationExplanation_isUsed) {
            iso15118.trace("   EVTerminationExplanation: %.*s", req->EVTerminationExplanation.charactersLen, req->EVTerminationExplanation.characters);
        }
    }

    // === RESPONSE MESSAGES ===

    if (iso20DocEnc->SessionSetupRes_isUsed) {
        iso20_SessionSetupResType *res = &iso20DocEnc->SessionSetupRes;

        trace_header(&res->Header, "SessionSetupRes");
        iso15118.trace(" Body");
        iso15118.trace("  SessionSetupRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEID: %.*s", res->EVSEID.charactersLen, res->EVSEID.characters);
        iso15118.trace("   EVSEID.charactersLen: %d", res->EVSEID.charactersLen);
    }

    if (iso20DocEnc->AuthorizationSetupRes_isUsed) {
        iso20_AuthorizationSetupResType *res = &iso20DocEnc->AuthorizationSetupRes;

        trace_header(&res->Header, "AuthorizationSetupRes");
        iso15118.trace(" Body");
        iso15118.trace("  AuthorizationSetupRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        for (int i = 0; i < res->AuthorizationServices.arrayLen; i++) {
            iso15118.trace("   AuthorizationServices[%d]: %d", i, res->AuthorizationServices.array[i]);
        }
        iso15118.trace("   CertificateInstallationService: %d", res->CertificateInstallationService);
        iso15118.trace("   EIM_ASResAuthorizationMode_isUsed: %d", res->EIM_ASResAuthorizationMode_isUsed);
        iso15118.trace("   PnC_ASResAuthorizationMode_isUsed: %d", res->PnC_ASResAuthorizationMode_isUsed);
    }

    if (iso20DocEnc->AuthorizationRes_isUsed) {
        iso20_AuthorizationResType *res = &iso20DocEnc->AuthorizationRes;

        trace_header(&res->Header, "AuthorizationRes");
        iso15118.trace(" Body");
        iso15118.trace("  AuthorizationRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEProcessing: %d", res->EVSEProcessing);
    }

    if (iso20DocEnc->ServiceDiscoveryRes_isUsed) {
        iso20_ServiceDiscoveryResType *res = &iso20DocEnc->ServiceDiscoveryRes;

        trace_header(&res->Header, "ServiceDiscoveryRes");
        iso15118.trace(" Body");
        iso15118.trace("  ServiceDiscoveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   ServiceRenegotiationSupported: %d", res->ServiceRenegotiationSupported);
        iso15118.trace("   EnergyTransferServiceList");
        for (int i = 0; i < res->EnergyTransferServiceList.Service.arrayLen; i++) {
            iso15118.trace("    Service[%d].ServiceID: %d", i, res->EnergyTransferServiceList.Service.array[i].ServiceID);
            iso15118.trace("    Service[%d].FreeService: %d", i, res->EnergyTransferServiceList.Service.array[i].FreeService);
        }
        iso15118.trace("   VASList_isUsed: %d", res->VASList_isUsed);
        if (res->VASList_isUsed) {
            for (int i = 0; i < res->VASList.Service.arrayLen; i++) {
                iso15118.trace("    VASList.Service[%d].ServiceID: %d", i, res->VASList.Service.array[i].ServiceID);
                iso15118.trace("    VASList.Service[%d].FreeService: %d", i, res->VASList.Service.array[i].FreeService);
            }
        }
    }

    if (iso20DocEnc->ServiceDetailRes_isUsed) {
        iso20_ServiceDetailResType *res = &iso20DocEnc->ServiceDetailRes;

        trace_header(&res->Header, "ServiceDetailRes");
        iso15118.trace(" Body");
        iso15118.trace("  ServiceDetailRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   ServiceID: %d", res->ServiceID);
        iso15118.trace("   ServiceParameterList");
        for (int i = 0; i < res->ServiceParameterList.ParameterSet.arrayLen; i++) {
            iso15118.trace("    ParameterSet[%d].ParameterSetID: %d", i, res->ServiceParameterList.ParameterSet.array[i].ParameterSetID);
            for (int j = 0; j < res->ServiceParameterList.ParameterSet.array[i].Parameter.arrayLen; j++) {
                iso15118.trace("     Parameter[%d].Name: %.*s", j,
                               res->ServiceParameterList.ParameterSet.array[i].Parameter.array[j].Name.charactersLen,
                               res->ServiceParameterList.ParameterSet.array[i].Parameter.array[j].Name.characters);
                if (res->ServiceParameterList.ParameterSet.array[i].Parameter.array[j].intValue_isUsed) {
                    iso15118.trace("     Parameter[%d].intValue: %ld", j, (long)res->ServiceParameterList.ParameterSet.array[i].Parameter.array[j].intValue);
                }
                if (res->ServiceParameterList.ParameterSet.array[i].Parameter.array[j].boolValue_isUsed) {
                    iso15118.trace("     Parameter[%d].boolValue: %d", j, res->ServiceParameterList.ParameterSet.array[i].Parameter.array[j].boolValue);
                }
                if (res->ServiceParameterList.ParameterSet.array[i].Parameter.array[j].finiteString_isUsed) {
                    iso15118.trace("     Parameter[%d].finiteString: %.*s", j,
                                   res->ServiceParameterList.ParameterSet.array[i].Parameter.array[j].finiteString.charactersLen,
                                   res->ServiceParameterList.ParameterSet.array[i].Parameter.array[j].finiteString.characters);
                }
            }
        }
    }

    if (iso20DocEnc->ServiceSelectionRes_isUsed) {
        iso20_ServiceSelectionResType *res = &iso20DocEnc->ServiceSelectionRes;

        trace_header(&res->Header, "ServiceSelectionRes");
        iso15118.trace(" Body");
        iso15118.trace("  ServiceSelectionRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
    }

    if (iso20DocEnc->ScheduleExchangeRes_isUsed) {
        iso20_ScheduleExchangeResType *res = &iso20DocEnc->ScheduleExchangeRes;

        trace_header(&res->Header, "ScheduleExchangeRes");
        iso15118.trace(" Body");
        iso15118.trace("  ScheduleExchangeRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEProcessing: %d", res->EVSEProcessing);
        iso15118.trace("   GoToPause_isUsed: %d", res->GoToPause_isUsed);
        if (res->GoToPause_isUsed) {
            iso15118.trace("   GoToPause: %d", res->GoToPause);
        }
        iso15118.trace("   Dynamic_SEResControlMode_isUsed: %d", res->Dynamic_SEResControlMode_isUsed);
        iso15118.trace("   Scheduled_SEResControlMode_isUsed: %d", res->Scheduled_SEResControlMode_isUsed);
        if (res->Scheduled_SEResControlMode_isUsed) {
            iso15118.trace("    Scheduled_SEResControlMode");
            for (int i = 0; i < res->Scheduled_SEResControlMode.ScheduleTuple.arrayLen; i++) {
                iso15118.trace("     ScheduleTuple[%d]", i);
                iso15118.trace("      ScheduleTupleID: %lu", (unsigned long)res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ScheduleTupleID);
                iso15118.trace("      ChargingSchedule.PowerSchedule.TimeAnchor: %llu", res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.PowerSchedule.TimeAnchor);
                for (int j = 0; j < res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.PowerSchedule.PowerScheduleEntries.PowerScheduleEntry.arrayLen; j++) {
                    iso15118.trace("       PowerScheduleEntry[%d].Duration: %lu", j,
                                   (unsigned long)res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.PowerSchedule.PowerScheduleEntries.PowerScheduleEntry.array[j].Duration);
                    iso15118.trace("       PowerScheduleEntry[%d].Power: %d * 10^%d", j,
                                   res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.PowerSchedule.PowerScheduleEntries.PowerScheduleEntry.array[j].Power.Value,
                                   res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.PowerSchedule.PowerScheduleEntries.PowerScheduleEntry.array[j].Power.Exponent);
                }
                iso15118.trace("      AbsolutePriceSchedule_isUsed: %d", res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.AbsolutePriceSchedule_isUsed);
                if (res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.AbsolutePriceSchedule_isUsed) {
                    iso15118.trace("      AbsolutePriceSchedule.TimeAnchor: %llu", res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.AbsolutePriceSchedule.TimeAnchor);
                    iso15118.trace("      AbsolutePriceSchedule.PriceScheduleID: %lu", (unsigned long)res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.AbsolutePriceSchedule.PriceScheduleID);
                    iso15118.trace("      AbsolutePriceSchedule.Currency: %.*s",
                                   res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.AbsolutePriceSchedule.Currency.charactersLen,
                                   res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.AbsolutePriceSchedule.Currency.characters);
                    iso15118.trace("      AbsolutePriceSchedule.Language: %.*s",
                                   res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.AbsolutePriceSchedule.Language.charactersLen,
                                   res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.AbsolutePriceSchedule.Language.characters);
                    iso15118.trace("      AbsolutePriceSchedule.PriceAlgorithm: %.*s",
                                   res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.AbsolutePriceSchedule.PriceAlgorithm.charactersLen,
                                   res->Scheduled_SEResControlMode.ScheduleTuple.array[i].ChargingSchedule.AbsolutePriceSchedule.PriceAlgorithm.characters);
                }
            }
        }
    }

    if (iso20DocEnc->PowerDeliveryRes_isUsed) {
        iso20_PowerDeliveryResType *res = &iso20DocEnc->PowerDeliveryRes;

        trace_header(&res->Header, "PowerDeliveryRes");
        iso15118.trace(" Body");
        iso15118.trace("  PowerDeliveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEStatus_isUsed: %d", res->EVSEStatus_isUsed);
        if (res->EVSEStatus_isUsed) {
            iso15118.trace("   EVSEStatus.NotificationMaxDelay: %d", res->EVSEStatus.NotificationMaxDelay);
            iso15118.trace("   EVSEStatus.EVSENotification: %d", res->EVSEStatus.EVSENotification);
        }
    }

    if (iso20DocEnc->SessionStopRes_isUsed) {
        iso20_SessionStopResType *res = &iso20DocEnc->SessionStopRes;

        trace_header(&res->Header, "SessionStopRes");
        iso15118.trace(" Body");
        iso15118.trace("  SessionStopRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
    }
}

void ISO20::trace_ac_request_response()
{
    // === AC REQUEST MESSAGES ===

    if (iso20AcDocDec->AC_ChargeParameterDiscoveryReq_isUsed) {
        iso20_ac_AC_ChargeParameterDiscoveryReqType *req = &iso20AcDocDec->AC_ChargeParameterDiscoveryReq;

        // Cast AC header to common header type for tracing (compatible layout)
        trace_header((const struct iso20_MessageHeaderType *)&req->Header, "AC_ChargeParameterDiscoveryReq");
        iso15118.trace(" Body");
        iso15118.trace("  AC_ChargeParameterDiscoveryReq");
        iso15118.trace("   AC_CPDReqEnergyTransferMode_isUsed: %d", req->AC_CPDReqEnergyTransferMode_isUsed);
        if (req->AC_CPDReqEnergyTransferMode_isUsed) {
            iso15118.trace("    AC_CPDReqEnergyTransferMode");
            iso15118.trace("     EVMaximumChargePower: %d * 10^%d",
                           req->AC_CPDReqEnergyTransferMode.EVMaximumChargePower.Value,
                           req->AC_CPDReqEnergyTransferMode.EVMaximumChargePower.Exponent);
            iso15118.trace("     EVMaximumChargePower_L2_isUsed: %d", req->AC_CPDReqEnergyTransferMode.EVMaximumChargePower_L2_isUsed);
            iso15118.trace("     EVMaximumChargePower_L3_isUsed: %d", req->AC_CPDReqEnergyTransferMode.EVMaximumChargePower_L3_isUsed);
            iso15118.trace("     EVMinimumChargePower: %d * 10^%d",
                           req->AC_CPDReqEnergyTransferMode.EVMinimumChargePower.Value,
                           req->AC_CPDReqEnergyTransferMode.EVMinimumChargePower.Exponent);
            iso15118.trace("     EVMinimumChargePower_L2_isUsed: %d", req->AC_CPDReqEnergyTransferMode.EVMinimumChargePower_L2_isUsed);
            iso15118.trace("     EVMinimumChargePower_L3_isUsed: %d", req->AC_CPDReqEnergyTransferMode.EVMinimumChargePower_L3_isUsed);
        }
        iso15118.trace("   BPT_AC_CPDReqEnergyTransferMode_isUsed: %d", req->BPT_AC_CPDReqEnergyTransferMode_isUsed);
    }

    if (iso20AcDocDec->AC_ChargeLoopReq_isUsed) {
        iso20_ac_AC_ChargeLoopReqType *req = &iso20AcDocDec->AC_ChargeLoopReq;

        trace_header((const struct iso20_MessageHeaderType *)&req->Header, "AC_ChargeLoopReq");
        iso15118.trace(" Body");
        iso15118.trace("  AC_ChargeLoopReq");
        iso15118.trace("   MeterInfoRequested: %d", req->MeterInfoRequested);
        iso15118.trace("   DisplayParameters_isUsed: %d", req->DisplayParameters_isUsed);
        if (req->DisplayParameters_isUsed) {
            iso15118.trace("    DisplayParameters");
            iso15118.trace("     PresentSOC_isUsed: %d", req->DisplayParameters.PresentSOC_isUsed);
            if (req->DisplayParameters.PresentSOC_isUsed) {
                iso15118.trace("     PresentSOC: %d", req->DisplayParameters.PresentSOC);
            }
            iso15118.trace("     MinimumSOC_isUsed: %d", req->DisplayParameters.MinimumSOC_isUsed);
            if (req->DisplayParameters.MinimumSOC_isUsed) {
                iso15118.trace("     MinimumSOC: %d", req->DisplayParameters.MinimumSOC);
            }
            iso15118.trace("     TargetSOC_isUsed: %d", req->DisplayParameters.TargetSOC_isUsed);
            if (req->DisplayParameters.TargetSOC_isUsed) {
                iso15118.trace("     TargetSOC: %d", req->DisplayParameters.TargetSOC);
            }
            iso15118.trace("     MaximumSOC_isUsed: %d", req->DisplayParameters.MaximumSOC_isUsed);
            if (req->DisplayParameters.MaximumSOC_isUsed) {
                iso15118.trace("     MaximumSOC: %d", req->DisplayParameters.MaximumSOC);
            }
            iso15118.trace("     RemainingTimeToMinimumSOC_isUsed: %d", req->DisplayParameters.RemainingTimeToMinimumSOC_isUsed);
            if (req->DisplayParameters.RemainingTimeToMinimumSOC_isUsed) {
                iso15118.trace("     RemainingTimeToMinimumSOC: %lu", (unsigned long)req->DisplayParameters.RemainingTimeToMinimumSOC);
            }
            iso15118.trace("     RemainingTimeToTargetSOC_isUsed: %d", req->DisplayParameters.RemainingTimeToTargetSOC_isUsed);
            if (req->DisplayParameters.RemainingTimeToTargetSOC_isUsed) {
                iso15118.trace("     RemainingTimeToTargetSOC: %lu", (unsigned long)req->DisplayParameters.RemainingTimeToTargetSOC);
            }
            iso15118.trace("     RemainingTimeToMaximumSOC_isUsed: %d", req->DisplayParameters.RemainingTimeToMaximumSOC_isUsed);
            if (req->DisplayParameters.RemainingTimeToMaximumSOC_isUsed) {
                iso15118.trace("     RemainingTimeToMaximumSOC: %lu", (unsigned long)req->DisplayParameters.RemainingTimeToMaximumSOC);
            }
            iso15118.trace("     ChargingComplete_isUsed: %d", req->DisplayParameters.ChargingComplete_isUsed);
            if (req->DisplayParameters.ChargingComplete_isUsed) {
                iso15118.trace("     ChargingComplete: %d", req->DisplayParameters.ChargingComplete);
            }
            iso15118.trace("     BatteryEnergyCapacity_isUsed: %d", req->DisplayParameters.BatteryEnergyCapacity_isUsed);
            if (req->DisplayParameters.BatteryEnergyCapacity_isUsed) {
                iso15118.trace("     BatteryEnergyCapacity: %d * 10^%d",
                               req->DisplayParameters.BatteryEnergyCapacity.Value,
                               req->DisplayParameters.BatteryEnergyCapacity.Exponent);
            }
            iso15118.trace("     InletHot_isUsed: %d", req->DisplayParameters.InletHot_isUsed);
            if (req->DisplayParameters.InletHot_isUsed) {
                iso15118.trace("     InletHot: %d", req->DisplayParameters.InletHot);
            }
        }
        iso15118.trace("   CLReqControlMode_isUsed: %d", req->CLReqControlMode_isUsed);
        iso15118.trace("   Dynamic_AC_CLReqControlMode_isUsed: %d", req->Dynamic_AC_CLReqControlMode_isUsed);
        iso15118.trace("   Scheduled_AC_CLReqControlMode_isUsed: %d", req->Scheduled_AC_CLReqControlMode_isUsed);
        if (req->Scheduled_AC_CLReqControlMode_isUsed) {
            iso15118.trace("    Scheduled_AC_CLReqControlMode");
            iso15118.trace("     EVPresentActivePower: %d * 10^%d",
                           req->Scheduled_AC_CLReqControlMode.EVPresentActivePower.Value,
                           req->Scheduled_AC_CLReqControlMode.EVPresentActivePower.Exponent);
            iso15118.trace("     EVPresentActivePower_L2_isUsed: %d", req->Scheduled_AC_CLReqControlMode.EVPresentActivePower_L2_isUsed);
            iso15118.trace("     EVPresentActivePower_L3_isUsed: %d", req->Scheduled_AC_CLReqControlMode.EVPresentActivePower_L3_isUsed);
            iso15118.trace("     EVTargetEnergyRequest_isUsed: %d", req->Scheduled_AC_CLReqControlMode.EVTargetEnergyRequest_isUsed);
            iso15118.trace("     EVMaximumEnergyRequest_isUsed: %d", req->Scheduled_AC_CLReqControlMode.EVMaximumEnergyRequest_isUsed);
            iso15118.trace("     EVMinimumEnergyRequest_isUsed: %d", req->Scheduled_AC_CLReqControlMode.EVMinimumEnergyRequest_isUsed);
            iso15118.trace("     EVMaximumChargePower_isUsed: %d", req->Scheduled_AC_CLReqControlMode.EVMaximumChargePower_isUsed);
            iso15118.trace("     EVMinimumChargePower_isUsed: %d", req->Scheduled_AC_CLReqControlMode.EVMinimumChargePower_isUsed);
            iso15118.trace("     EVPresentReactivePower_isUsed: %d", req->Scheduled_AC_CLReqControlMode.EVPresentReactivePower_isUsed);
        }
    }

    // === AC RESPONSE MESSAGES ===

    if (iso20AcDocEnc->AC_ChargeParameterDiscoveryRes_isUsed) {
        iso20_ac_AC_ChargeParameterDiscoveryResType *res = &iso20AcDocEnc->AC_ChargeParameterDiscoveryRes;

        trace_header((const struct iso20_MessageHeaderType *)&res->Header, "AC_ChargeParameterDiscoveryRes");
        iso15118.trace(" Body");
        iso15118.trace("  AC_ChargeParameterDiscoveryRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   AC_CPDResEnergyTransferMode_isUsed: %d", res->AC_CPDResEnergyTransferMode_isUsed);
        if (res->AC_CPDResEnergyTransferMode_isUsed) {
            iso15118.trace("    AC_CPDResEnergyTransferMode");
            iso15118.trace("     EVSEMaximumChargePower: %d * 10^%d",
                           res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower.Value,
                           res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower.Exponent);
            iso15118.trace("     EVSEMaximumChargePower_L2_isUsed: %d", res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower_L2_isUsed);
            iso15118.trace("     EVSEMaximumChargePower_L3_isUsed: %d", res->AC_CPDResEnergyTransferMode.EVSEMaximumChargePower_L3_isUsed);
            iso15118.trace("     EVSEMinimumChargePower: %d * 10^%d",
                           res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower.Value,
                           res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower.Exponent);
            iso15118.trace("     EVSEMinimumChargePower_L2_isUsed: %d", res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower_L2_isUsed);
            iso15118.trace("     EVSEMinimumChargePower_L3_isUsed: %d", res->AC_CPDResEnergyTransferMode.EVSEMinimumChargePower_L3_isUsed);
            iso15118.trace("     EVSENominalFrequency: %d * 10^%d",
                           res->AC_CPDResEnergyTransferMode.EVSENominalFrequency.Value,
                           res->AC_CPDResEnergyTransferMode.EVSENominalFrequency.Exponent);
            iso15118.trace("     MaximumPowerAsymmetry_isUsed: %d", res->AC_CPDResEnergyTransferMode.MaximumPowerAsymmetry_isUsed);
            iso15118.trace("     EVSEPowerRampLimitation_isUsed: %d", res->AC_CPDResEnergyTransferMode.EVSEPowerRampLimitation_isUsed);
            iso15118.trace("     EVSEPresentActivePower_isUsed: %d", res->AC_CPDResEnergyTransferMode.EVSEPresentActivePower_isUsed);
        }
        iso15118.trace("   BPT_AC_CPDResEnergyTransferMode_isUsed: %d", res->BPT_AC_CPDResEnergyTransferMode_isUsed);
    }

    if (iso20AcDocEnc->AC_ChargeLoopRes_isUsed) {
        iso20_ac_AC_ChargeLoopResType *res = &iso20AcDocEnc->AC_ChargeLoopRes;

        trace_header((const struct iso20_MessageHeaderType *)&res->Header, "AC_ChargeLoopRes");
        iso15118.trace(" Body");
        iso15118.trace("  AC_ChargeLoopRes");
        iso15118.trace("   ResponseCode: %d", res->ResponseCode);
        iso15118.trace("   EVSEStatus_isUsed: %d", res->EVSEStatus_isUsed);
        if (res->EVSEStatus_isUsed) {
            iso15118.trace("    EVSEStatus");
            iso15118.trace("     NotificationMaxDelay: %d", res->EVSEStatus.NotificationMaxDelay);
            iso15118.trace("     EVSENotification: %d", res->EVSEStatus.EVSENotification);
        }
        iso15118.trace("   MeterInfo_isUsed: %d", res->MeterInfo_isUsed);
        iso15118.trace("   Receipt_isUsed: %d", res->Receipt_isUsed);
        iso15118.trace("   EVSETargetFrequency_isUsed: %d", res->EVSETargetFrequency_isUsed);
        iso15118.trace("   CLResControlMode_isUsed: %d", res->CLResControlMode_isUsed);
        iso15118.trace("   Dynamic_AC_CLResControlMode_isUsed: %d", res->Dynamic_AC_CLResControlMode_isUsed);
        iso15118.trace("   Scheduled_AC_CLResControlMode_isUsed: %d", res->Scheduled_AC_CLResControlMode_isUsed);
        if (res->Scheduled_AC_CLResControlMode_isUsed) {
            iso15118.trace("    Scheduled_AC_CLResControlMode");
            iso15118.trace("     EVSETargetActivePower_isUsed: %d", res->Scheduled_AC_CLResControlMode.EVSETargetActivePower_isUsed);
            iso15118.trace("     EVSETargetActivePower_L2_isUsed: %d", res->Scheduled_AC_CLResControlMode.EVSETargetActivePower_L2_isUsed);
            iso15118.trace("     EVSETargetActivePower_L3_isUsed: %d", res->Scheduled_AC_CLResControlMode.EVSETargetActivePower_L3_isUsed);
            iso15118.trace("     EVSETargetReactivePower_isUsed: %d", res->Scheduled_AC_CLResControlMode.EVSETargetReactivePower_isUsed);
            iso15118.trace("     EVSETargetReactivePower_L2_isUsed: %d", res->Scheduled_AC_CLResControlMode.EVSETargetReactivePower_L2_isUsed);
            iso15118.trace("     EVSETargetReactivePower_L3_isUsed: %d", res->Scheduled_AC_CLResControlMode.EVSETargetReactivePower_L3_isUsed);
            iso15118.trace("     EVSEPresentActivePower_isUsed: %d", res->Scheduled_AC_CLResControlMode.EVSEPresentActivePower_isUsed);
            if (res->Scheduled_AC_CLResControlMode.EVSEPresentActivePower_isUsed) {
                iso15118.trace("     EVSEPresentActivePower: %d * 10^%d",
                               res->Scheduled_AC_CLResControlMode.EVSEPresentActivePower.Value,
                               res->Scheduled_AC_CLResControlMode.EVSEPresentActivePower.Exponent);
            }
            iso15118.trace("     EVSEPresentActivePower_L2_isUsed: %d", res->Scheduled_AC_CLResControlMode.EVSEPresentActivePower_L2_isUsed);
            iso15118.trace("     EVSEPresentActivePower_L3_isUsed: %d", res->Scheduled_AC_CLResControlMode.EVSEPresentActivePower_L3_isUsed);
        }
    }
}
