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
#include "exi/projectExiConnector.h"

void DIN70121::pre_setup()
{
    supported_protocols_prototype = Config::Str("", 0, 32);
    api_state = Config::Object({
        {"state", Config::Uint8(0)},
        {"session_id", Config::Array({
                Config::Uint8(0), Config::Uint8(0), Config::Uint8(0), Config::Uint8(0)
            }, Config::get_prototype_uint8_0(), 4, 4, Config::type_id<Config::ConfUint>())
        },
        {"supported_protocols", Config::Array({}, &supported_protocols_prototype, 0, 4, Config::type_id<Config::ConfString>())},
        {"protocol", Config::Str("", 0, 32)},
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

void DIN70121::setup_socket()
{
    listen_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if(listen_socket < 0) {
        logger.printfln("DIN70121: Failed to create socket: %d", listen_socket);
        return;
    }

    if(fcntl(listen_socket, F_SETFL, fcntl(listen_socket, F_GETFL) | O_NONBLOCK) < 0) {
        logger.printfln("DIN70121: Failed to set non-blocking mode (listen socket)");
        close(listen_socket);
        active_socket = -1;
        return;
    }

    int opt = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));

    struct sockaddr_storage dest_addr;
    struct sockaddr_in6 *dest_addr_ip6 = (struct sockaddr_in6 *)&dest_addr;
    bzero(&dest_addr_ip6->sin6_addr.un, sizeof(dest_addr_ip6->sin6_addr.un));
    dest_addr_ip6->sin6_family = AF_INET6;
    dest_addr_ip6->sin6_port = htons(15118);

    int err = bind(listen_socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if(err < 0) {
        logger.printfln("DIN70121: Failed to bind socket: %d", err);
        return;
    }

    err = listen(listen_socket, 1);
    if(err < 0) {
        logger.printfln("DIN70121: Failed to listen on socket: %d", err);
        return;
    }
}

void DIN70121::state_machine_loop()
{
    if (active_socket < 0) {
        active_socket = accept(listen_socket, (struct sockaddr *)&source_addr, &addr_len);
        if(active_socket < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                // No connection available, non-blocking mode
                return;
            }
            logger.printfln("DIN70121: Failed to accept connection: %d (errno %d)", active_socket, errno);
            return;
        }

        char addr_str[128];
        inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
        logger.printfln("DIN70121: Accepted connection from %s", addr_str);

        if(fcntl(active_socket, F_SETFL, fcntl(active_socket, F_GETFL) | O_NONBLOCK) < 0) {
            logger.printfln("DIN70121: Failed to set non-blocking mode (active socket)");
            close(active_socket);
            active_socket = -1;
            return;
        }
    } else {
        uint8_t data[1024] = {0};
        ssize_t length = recv(active_socket, data, sizeof(data), 0);

        if(length < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                // No data available, non-blocking mode
                return;
            } else if(errno == ECONNRESET) {
                logger.printfln("DIN70121: Connection reset by peer");
                close(active_socket);
                active_socket = -1;
                return;
            }
            logger.printfln("DIN70121: Failed to receive data: %d (errno %d)", length, errno);
            close(active_socket);
            active_socket = -1;
        } else if(length == 0) {
            logger.printfln("DIN70121: Connection closed");
            close(active_socket);
            active_socket = -1;
        } else {
            decode(data, length);
        }
    }
}

void DIN70121::send_exi(ExiType type)
{
    if (type == ExiType::Din) {
        global_streamEncPos = 0;
        projectExiConnector_encode_DinExiDocument();
    }

    const uint8_t *data = global_streamEnc.data;
    const size_t length = global_streamEncPos;

    uint8_t exi[length + sizeof(V2GTP_Header)];
    V2GTP_Header *header = (V2GTP_Header*)exi;
    header->protocol_version         = 0x01;
    header->inverse_protocol_version = 0xFE;
    header->payload_type             = htons(0x8001), // 0x8001 = EXI data
    header->payload_length           = htonl(length);
    memcpy(&exi[sizeof(V2GTP_Header)], data, length);

    ssize_t ret = send(active_socket, exi, length + sizeof(V2GTP_Header), 0);
    if(ret < 0) {
        logger.printfln("DIN70121: Failed to send data: %d (errno %d)", ret, errno);
        close(active_socket);
        active_socket = -1;
        return;
    }
}

void DIN70121::decode(uint8_t *data, const size_t length)
{
    global_streamDec.data = &data[sizeof(V2GTP_Header)];
    global_streamDec.size = length - sizeof(V2GTP_Header);

    V2GTP_Header *header = (V2GTP_Header*)data;
    if(header->protocol_version != 0x01) {
        logger.printfln("DIN70121: Invalid protocol version: %d", header->protocol_version);
        return;
    }
    if(header->inverse_protocol_version != 0xFE) {
        logger.printfln("DIN70121: Invalid inverse protocol version: %d", header->inverse_protocol_version);
        return;
    }
    if(ntohs(header->payload_type) != 0x8001) {
        logger.printfln("DIN70121: Invalid payload type: %d", ntohs(header->payload_type));
        return;
    }

    if (state == 0) {
        projectExiConnector_decode_appHandExiDocument(); // Decode handshake EXI on first packet
    } else {
        projectExiConnector_decode_DinExiDocument(); // Decode DIN-SPEC-70121 EXI on subsequent packets
    }

    if (aphsDoc.supportedAppProtocolReq_isUsed) {
        iso15118.trace("DIN70121: SupportedAppProtocolReq received");
        handle_supported_app_protocol_req();
        aphsDoc.supportedAppProtocolReq_isUsed = 0;
    }

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

void DIN70121::handle_supported_app_protocol_req()
{
    appHandAnonType_supportedAppProtocolReq *req = &aphsDoc.supportedAppProtocolReq;

    // process data when no errors occured during decoding
    if (g_errn == 0) {
        int8_t schema_id = -1;
        uint8_t index = 0;

        // check all schemas for DIN
        logger.printfln("EV supports %u protocols", req->AppProtocol.arrayLen);
        api_state.get("supported_protocols")->removeAll();

        for(uint16_t i = 0; i < req->AppProtocol.arrayLen; i++) {
            if (strnstr(req->AppProtocol.array[i].ProtocolNamespace.characters, ":din:70121:", req->AppProtocol.array[i].ProtocolNamespace.charactersLen) != nullptr) {
                schema_id = req->AppProtocol.array[i].SchemaID;
                index = i;
            }
            logger.printfln_continue("%d: %s", req->AppProtocol.array[i].SchemaID, req->AppProtocol.array[i].ProtocolNamespace.characters);
            api_state.get("supported_protocols")->add()->updateString(req->AppProtocol.array[i].ProtocolNamespace.characters);
        }

        if (schema_id == -1) {
            logger.printfln("EV does not support DIN 70121");
            api_state.get("protocol")->updateString("-");
            return;
        } else {
            logger.printfln("Using DIN 70121");
            api_state.get("protocol")->updateString(req->AppProtocol.array[index].ProtocolNamespace.characters);

            projectExiConnector_encode_appHandExiDocument(schema_id);
            send_exi(DIN70121::ExiType::AppHand);
            state = 1;
        }
    }
}

void DIN70121::handle_session_setup_req()
{
    dinSessionSetupReqType *req = &dinDocDec.V2G_Message.Body.SessionSetupReq;
    dinSessionSetupResType *res = &dinDocEnc.V2G_Message.Body.SessionSetupRes;

    api_state.get("evcc_id")->removeAll();
    for (uint16_t i = 0; i < std::min(static_cast<uint16_t>(sizeof(req->EVCCID.bytes)), req->EVCCID.bytesLen); i++) {
        api_state.get("evcc_id")->add()->updateUint(req->EVCCID.bytes[i]);
    }

    // TODO: Check session id. If not 0 use it, if it is known [V2G-DC-872]

    // The SessionId is set up here, and used by projectExiConnector_prepare_DinExiDocument
    // it is used by the EV in future communication
    sessionId[0] = static_cast<uint8_t>(random(256));
    sessionId[1] = static_cast<uint8_t>(random(256));
    sessionId[2] = static_cast<uint8_t>(random(256));
    sessionId[3] = static_cast<uint8_t>(random(256));
    sessionIdLen = 4;

    for (uint16_t i = 0; i < sessionIdLen; i++) {
        api_state.get("session_id")->get(i)->updateUint(sessionId[i]);
    }

    projectExiConnector_prepare_DinExiDocument();

    dinDocEnc.V2G_Message.Body.SessionSetupRes_isUsed = 1;
    init_dinSessionSetupResType(res);
    res->ResponseCode = dinresponseCodeType_OK_NewSessionEstablished;

    // EVSEID needs to be according to DIN SPEC 91286, it can be 0x00 if not available
    // Example EVSEID according to DIN SPEC 91286: +49*810*000*438

    res->EVSEID.bytes[0] = 0;
    res->EVSEID.bytesLen = 1;

    send_exi(ExiType::Din);
    state = 2;
}

void DIN70121::handle_service_discovery_req()
{
    dinServiceDiscoveryReqType *req = &dinDocDec.V2G_Message.Body.ServiceDiscoveryReq;
    dinServiceDiscoveryResType *res = &dinDocEnc.V2G_Message.Body.ServiceDiscoveryRes;

    // TODO: Stop session if session ID does not match?
    if (dinDocDec.V2G_Message.Header.SessionID.bytesLen != sessionIdLen) {
        logger.printfln("DIN70121: Session ID length mismatch");
    }
    if (memcmp(dinDocDec.V2G_Message.Header.SessionID.bytes, sessionId, sessionIdLen) != 0) {
        logger.printfln("DIN70121: Session ID mismatch");
    }

    projectExiConnector_prepare_DinExiDocument();

    dinDocEnc.V2G_Message.Body.ServiceDiscoveryRes_isUsed = 1;
    init_dinServiceDiscoveryResType(res);
    res->ResponseCode = dinresponseCodeType_OK;

    // One payment option: EVSE handles payment
    res->PaymentOptions.PaymentOption.array[0] = dinpaymentOptionType_ExternalPayment;
    res->PaymentOptions.PaymentOption.arrayLen = 1;

    // One service available: EV charging
    res->ChargeService.ServiceTag.ServiceID = 1;
    res->ChargeService.ServiceTag.ServiceCategory = dinserviceCategoryType_EVCharging;

    // EV can use offered service without payment
    res->ChargeService.FreeService = 1;

    // DC charging with CCS connector (normal DC charging)
    res->ChargeService.EnergyTransferType = dinEVSESupportedEnergyTransferType_DC_extended;

    send_exi(ExiType::Din);
    state = 3;
}

void DIN70121::handle_service_payment_selection_req()
{
    dinServicePaymentSelectionReqType *req = &dinDocDec.V2G_Message.Body.ServicePaymentSelectionReq;
    dinServicePaymentSelectionResType *res = &dinDocEnc.V2G_Message.Body.ServicePaymentSelectionRes;

    if (req->SelectedPaymentOption == dinpaymentOptionType_ExternalPayment) {
        projectExiConnector_prepare_DinExiDocument();

        dinDocEnc.V2G_Message.Body.ServicePaymentSelectionRes_isUsed = 1;
        init_dinServicePaymentSelectionResType(res);
        res->ResponseCode = dinresponseCodeType_OK;

        send_exi(ExiType::Din);
        state = 4;
    }
}

void DIN70121::handle_contract_authentication_req()
{
    dinContractAuthenticationResType *res = &dinDocEnc.V2G_Message.Body.ContractAuthenticationRes;

    projectExiConnector_prepare_DinExiDocument();

    dinDocEnc.V2G_Message.Body.ContractAuthenticationRes_isUsed = 1;
    init_dinContractAuthenticationResType(res);

    // Set Authorisation to Finished
    res->EVSEProcessing = dinEVSEProcessingType_Finished;
    send_exi(ExiType::Din);

    state = 5;
}

void DIN70121::handle_charge_parameter_discovery_req()
{
    dinChargeParameterDiscoveryReqType* req = &dinDocDec.V2G_Message.Body.ChargeParameterDiscoveryReq;
    dinChargeParameterDiscoveryResType* res = &dinDocDec.V2G_Message.Body.ChargeParameterDiscoveryRes;

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
    logger.printfln("DIN70121: Energy Request unit: %d", req->DC_EVChargeParameter.EVEnergyRequest.Unit);
    api_state.get("ev_energy_request_is_used")->updateBool(req->DC_EVChargeParameter.EVEnergyRequest_isUsed);
    api_state.get("full_soc")->updateInt(req->DC_EVChargeParameter.FullSOC);
    api_state.get("full_soc_is_used")->updateBool(req->DC_EVChargeParameter.FullSOC_isUsed);
    api_state.get("bulk_soc")->updateInt(req->DC_EVChargeParameter.BulkSOC);
    api_state.get("bulk_soc_is_used")->updateBool(req->DC_EVChargeParameter.BulkSOC_isUsed);

    logger.printfln("DIN70121: Current SoC %d", req->DC_EVChargeParameter.DC_EVStatus.EVRESSSOC);

    projectExiConnector_prepare_DinExiDocument();

    dinDocEnc.V2G_Message.Body.ChargeParameterDiscoveryRes_isUsed = 1;
    init_dinChargeParameterDiscoveryResType(&dinDocEnc.V2G_Message.Body.ChargeParameterDiscoveryRes);

    // [V2G-DC-493] After the EVCC has successfully processed a received ChargeParameterDiscoveryRes
    // message with ResponseCode equal to “OK” and EVSEProcessing equal to
    // “Ongoing”, the EVCC shall ignore the values of SAScheduleList and DC_EVSEChar-
    // geParameter contained in this ChargeParameterDiscoveryRes message, and shall
    // send another ChargeParameterDiscoveryReq message and shall then wait for a
    // ChargeParameterDiscoveryRes message. This following ChargeParameterDisco-
    // veryReq message, if any, may contain different values for the contained parame-
    // ters than the preceding ChargeParameterDiscoveryReq message.


    // Get EV to send ChargeParameterDiscoveryReq again by using EVSE_IsolationMonitoringActive with EVSEProcessingType_Ongoing
    // See DIN/TS:70121:2024 [V2G-DC-966]
    res->ResponseCode = dinresponseCodeType_OK;
	res->EVSEProcessing = dinEVSEProcessingType_Ongoing;
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus_isUsed = 1;
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEIsolationStatus = dinisolationLevelType_Invalid;
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSENotification = dinEVSENotificationType_None;
    res->DC_EVSEChargeParameter.DC_EVSEStatus.NotificationMaxDelay = 0;
    res->DC_EVSEChargeParameter.DC_EVSEStatus.EVSEStatusCode = dinDC_EVSEStatusCodeType_EVSE_IsolationMonitoringActive;

    // Mandatory charge parameters
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit = dinunitSymbolType_A;
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value = 32; // 32A
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier = 1;

    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit = dinunitSymbolType_V;
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Value = 400; // 400V
    res->DC_EVSEChargeParameter.EVSEMaximumVoltageLimit.Multiplier = 1;

    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit = dinunitSymbolType_A;
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Value = 6; // 6A
    res->DC_EVSEChargeParameter.EVSEMaximumCurrentLimit.Multiplier = 1;

    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit = dinunitSymbolType_V;
    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Value = 200; // 200V
    res->DC_EVSEChargeParameter.EVSEMinimumVoltageLimit.Multiplier = 1;

    res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Unit = dinunitSymbolType_A;
    res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Unit_isUsed = 1;
    res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Value = 1; // 1A
    res->DC_EVSEChargeParameter.EVSEPeakCurrentRipple.Multiplier = 1;

	res->DC_EVSEChargeParameter_isUsed = 1;

    // Optinal charge parameters
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit_isUsed = 0;
    res->DC_EVSEChargeParameter.EVSECurrentRegulationTolerance_isUsed = 0;
    res->DC_EVSEChargeParameter.EVSEEnergyToBeDelivered_isUsed = 0;
    res->DC_EVSEChargeParameter.EVSEMaximumPowerLimit_isUsed = 0;
    res->DC_EVSEChargeParameter.EVSEEnergyToBeDelivered_isUsed = 0;

    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].PMax = SHRT_MAX;
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration = 86400; // Must cover 24 hours
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.duration_isUsed = 1; // Must be used in DIN
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval.start = 0;
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].RelativeTimeInterval_isUsed = 1;
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.array[0].TimeInterval_isUsed = 0; // no content
    res->SAScheduleList.SAScheduleTuple.array[0].SalesTariff_isUsed = 0; // In the scope of DIN 70121, this optional element shall not be used. DIN/TS:70121:2024 [V2G-DC-554]
    res->SAScheduleList.SAScheduleTuple.array[0].SAScheduleTupleID = 1;
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleEntry.arrayLen = 1;
    res->SAScheduleList.SAScheduleTuple.array[0].PMaxSchedule.PMaxScheduleID = 1;
    res->SAScheduleList.SAScheduleTuple.arrayLen = 1;
    res->SAScheduleList_isUsed = 1;


    // [V2G-DC-882] The EV shall ignore the SASchedule received in ChargeParameterDiscoveryRes
    res->SASchedules_isUsed = 0;

    // Has no content
    res->EVSEChargeParameter_isUsed = 0;

    // [V2G-DC-552] In the scope of this document, the element “AC_EVSEChargeParameter” shall not be used
	res->AC_EVSEChargeParameter_isUsed = 0;


    send_exi(ExiType::Din);
    state = 6;
}

void DIN70121::handle_session_stop_req()
{
    dinSessionStopResType *res = &dinDocEnc.V2G_Message.Body.SessionStopRes;

    projectExiConnector_prepare_DinExiDocument();
    dinDocEnc.V2G_Message.Body.SessionStopRes_isUsed = 1;
    init_dinSessionStopResType(res);
    res->ResponseCode = dinresponseCodeType_OK;

    send_exi(ExiType::Din);
    state = 7;
}
