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

// Common functions that are shared between DIN-SPEC-70121, ISO-151118-2 and ISO-15118-20

#include "common.h"
#include <esp_random.h>
#include "sdp.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "esp_netif.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "tools/net.h"
#include "tools/malloc.h"

#include "cbv2g/exi_v2gtp.h"
#include "cbv2g/app_handshake/appHand_Decoder.h"
#include "cbv2g/app_handshake/appHand_Encoder.h"
#include "cbv2g/din/din_msgDefDecoder.h"
#include "cbv2g/din/din_msgDefEncoder.h"
#include "cbv2g/common/exi_bitstream.h"

void Common::pre_setup()
{
    supported_protocols_prototype = Config::Str("", 0, 32);
    api_state = Config::Object({
        {"state", Config::Uint8(0)},
        {"supported_protocols", Config::Array({}, &supported_protocols_prototype, 0, 4, Config::type_id<Config::ConfString>())},
        {"protocol", Config::Str("", 0, 32)},
        {"tls_active", Config::Bool(false)}
    });

    if (exi_data == nullptr) {
        exi_data = static_cast<uint8_t*>(calloc_psram_or_dram(EXI_DATA_SIZE, sizeof(uint8_t)));
    }
}

void Common::setup_socket()
{
    // Close existing socket if any
    if (listen_socket >= 0) {
        close(listen_socket);
        listen_socket = -1;
    }

    listen_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if(listen_socket < 0) {
        logger.printfln("Common: Failed to create socket: %d (errno %d)", listen_socket, errno);
        return;
    }

    if(fcntl(listen_socket, F_SETFL, fcntl(listen_socket, F_GETFL) | O_NONBLOCK) < 0) {
        logger.printfln("Common: Failed to set non-blocking mode (listen socket)");
        reset_active_socket();
        return;
    }

    int opt = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));

    struct sockaddr_in6 dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin6_family = AF_INET6;
    dest_addr.sin6_port = htons(V2G_TCP_DATA_PORT);
    dest_addr.sin6_addr = in6addr_any;

    int err = bind(listen_socket, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if(err < 0) {
        logger.printfln("Common: Failed to bind socket: %d (errno %d)", err, errno);
        return;
    }

    err = listen(listen_socket, 1);
    if(err < 0) {
        logger.printfln("Common: Failed to listen on socket: %d (errno %d)", err, errno);
        return;
    }
}

void Common::close_socket()
{
    reset_active_socket();
    if (listen_socket >= 0) {
        close(listen_socket);
        listen_socket = -1;
    }
}

void Common::state_machine_loop()
{
    int new_socket = accept(listen_socket, (struct sockaddr *)&source_addr, &addr_len);
    if (new_socket > 0) {
        // We only support one socket connection at a time. If there is a new connection and one is currently open we close the old one.
        // Usually this means the EV has reconnected. There can't be multiple EVs connected at the same time.
        if ((active_socket > 0) && (new_socket > 0)) {
            logger.printfln("Common: Replacing socket %d with %d", active_socket, new_socket);
        }

        // Save tls_requested_by_ev before reset (it was set by SDP)
        bool tls_requested = tls_requested_by_ev;

        reset_active_socket();
        active_socket = new_socket;

        // Restore tls_requested_by_ev after reset
        tls_requested_by_ev = tls_requested;
        if(active_socket < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                // No connection available, non-blocking mode
                return;
            }
            logger.printfln("Common: Failed to accept connection: %d (errno %d [%s])", active_socket, errno, strerror(errno));
            return;
        }

        char addr_str[INET6_ADDRSTRLEN];
        tf_ip6addr_ntoa(&source_addr, addr_str, sizeof(addr_str));
        logger.printfln("Common: Accepted connection from %s", addr_str);

        if(fcntl(active_socket, F_SETFL, fcntl(active_socket, F_GETFL) | O_NONBLOCK) < 0) {
            logger.printfln("Common: Failed to set non-blocking mode (active socket)");
            reset_active_socket();
            return;
        }

        // If a new socket is opened we expect a new handshake
        exi_in_use = ExiType::AppHand;

        // Check if TLS should be used for this connection
        if (tls_requested_by_ev) {
            logger.printfln("Common: TLS requested, starting handshake");

            // Setup TLS if not already done
            if (!tls.is_initialized()) {
                if (!tls.setup()) {
                    logger.printfln("Common: TLS setup failed, closing connection");
                    reset_active_socket();
                    return;
                }
            }

            // Start TLS session for this connection
            if (!tls.start_session(active_socket)) {
                logger.printfln("Common: Failed to start TLS session, closing connection");
                reset_active_socket();
                return;
            }
        }
    } else if (active_socket > 0) {
        // Handle TLS handshake in progress
        if (tls.get_handshake_state() == TlsHandshakeState::IN_PROGRESS) {
            if (tls.do_handshake()) {
                // Handshake completed, can now receive data
                api_state.get("tls_active")->updateBool(true);
                logger.printfln("Common: TLS handshake completed, ready for V2G communication");
            } else if (tls.get_handshake_state() == TlsHandshakeState::FAILED) {
                logger.printfln("Common: TLS handshake failed, closing connection");
                reset_active_socket();
            }
            // If still IN_PROGRESS, just return and try again next loop
            return;
        }

        // Read data (either TLS or plain TCP)
        ssize_t length;
        if (tls.is_session_active()) {
            length = tls.read(exi_data, EXI_DATA_SIZE);
        } else {
            length = recv(active_socket, exi_data, EXI_DATA_SIZE, 0);
        }

        if(length < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                // No data available, non-blocking mode
                return;
            } else if(errno == ECONNRESET) {
                logger.printfln("Common: Connection reset by peer");
                reset_active_socket();
                return;
            }
            logger.printfln("Common: Failed to receive data: %zd (errno %d [%s])", length, errno, strerror(errno));
            reset_active_socket();
        } else if(length == 0) {
            logger.printfln("Common: Connection closed");
            reset_active_socket();
        } else {
            decode(exi_data, length);
        }
    }
}

void Common::reset_active_socket()
{
    // Close TLS session if active
    tls.end_session();
    tls_requested_by_ev = false;
    api_state.get("tls_active")->updateBool(false);

    if (active_socket >= 0) {
        close(active_socket);
        active_socket = -1;
    }
    state = 0;
    exi_in_use = ExiType::AppHand;
}

void Common::prepare_din_header(struct din_MessageHeaderType *header)
{
    header->Notification_isUsed = 0;
    header->Signature_isUsed = 0;
    for (uint8_t i = 0; i < SESSION_ID_LENGTH; i++) {
        header->SessionID.bytes[i] = session_id[i];
    }
    header->SessionID.bytesLen = SESSION_ID_LENGTH;
}

void Common::prepare_iso2_header(struct iso2_MessageHeaderType *header)
{
    header->Notification_isUsed = 0;
    header->Signature_isUsed = 0;
    for (uint8_t i = 0; i < SESSION_ID_LENGTH; i++) {
        header->SessionID.bytes[i] = session_id[i];
    }
    header->SessionID.bytesLen = SESSION_ID_LENGTH;
}

void Common::send_exi(ExiType type)
{
    memset(exi_data, 0, EXI_DATA_SIZE);

    exi_bitstream exi;
    exi_bitstream_init(&exi, exi_data, EXI_DATA_SIZE, sizeof(V2GTP_Header), nullptr);

    int ret = -1;
    switch(type) {
        case ExiType::AppHand:
            ret = encode_appHand_exiDocument(&exi, appHandEnc);
            break;
        case ExiType::Din:
            prepare_din_header(&iso15118.din70121.dinDocEnc->V2G_Message.Header);
            ret = encode_din_exiDocument(&exi, iso15118.din70121.dinDocEnc);
            break;
        case ExiType::Iso2:
            prepare_iso2_header(&iso15118.iso2.iso2DocEnc->V2G_Message.Header);
            ret = encode_iso2_exiDocument(&exi, iso15118.iso2.iso2DocEnc);
            break;
        case ExiType::Iso20:
            // Note: ISO 20 uses a flat structure, not V2G_Message.Header like ISO 2
            // The header is prepared in the response handler (e.g., handle_session_setup_req)
            ret = encode_iso20_exiDocument(&exi, iso15118.iso20.iso20DocEnc);
            break;
        case ExiType::Iso20Ac:
            // ISO 20 AC-specific messages (AC_ChargeParameterDiscovery, AC_ChargeLoop, etc.)
            // The header is prepared in the response handler
            ret = encode_iso20_ac_exiDocument(&exi, iso15118.iso20.iso20AcDocEnc);
            break;
    }

    if (ret != 0) {
        logger.printfln("Common: Failed to encode EXI document: %d", ret);
        return;
    }

    const size_t length = exi_bitstream_get_length(&exi);

    V2GTP_Header *header = (V2GTP_Header*)exi_data;
    header->protocol_version         = 0x01;
    header->inverse_protocol_version = 0xFE;

    // Select payload type based on EXI type
    V2GTPPayloadType payload_type = V2GTPPayloadType::SAP;
    if (type == ExiType::Iso20) {
        payload_type = V2GTPPayloadType::ISO20Common;
    } else if (type == ExiType::Iso20Ac) {
        payload_type = V2GTPPayloadType::ISO20AC;
    }
    header->payload_type             = htons(static_cast<uint16_t>(payload_type));
    header->payload_length           = htonl(length);

    const size_t total_length = length + sizeof(V2GTP_Header);
    ssize_t send_ret;

    if (tls.is_session_active()) {
        send_ret = tls.write(exi_data, total_length);
    } else {
        send_ret = send(active_socket, exi_data, total_length, 0);
    }

    if(send_ret < 0) {
        logger.printfln("Common: Failed to send data: %zd (errno %d)", send_ret, errno);
        reset_active_socket();
        return;
    }
}

void Common::decode(uint8_t *data, const size_t length)
{
    V2GTP_Header *header = (V2GTP_Header*)data;
    if(header->protocol_version != 0x01) {
        logger.printfln("Common: Invalid protocol version: %d", header->protocol_version);
        return;
    }
    if(header->inverse_protocol_version != 0xFE) {
        logger.printfln("Common: Invalid inverse protocol version: %d", header->inverse_protocol_version);
        return;
    }

    const V2GTPPayloadType payload_type = static_cast<V2GTPPayloadType>(ntohs(header->payload_type));
    if (payload_type != V2GTPPayloadType::SAP &&
        payload_type != V2GTPPayloadType::ISO20Common &&
        payload_type != V2GTPPayloadType::ISO20AC) {
        logger.printfln("Common: Invalid payload type: 0x%04x", static_cast<uint16_t>(payload_type));
        return;
    }

    exi_bitstream exi;
    exi_bitstream_init(&exi, &data[sizeof(V2GTP_Header)], length - sizeof(V2GTP_Header), 0, nullptr);

    if (exi_in_use == ExiType::AppHand) {
        // We alloc the appHand buffers the very first time they are used.
        // This way it is not allocated if ISO15118 is not used.
        // If it is used once we can assume that it will be used all the time, so it stays allocated.
        if (appHandDec == nullptr) {
            appHandDec = static_cast<struct appHand_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct appHand_exiDocument)));
        }
        if (appHandEnc == nullptr) {
            appHandEnc = static_cast<struct appHand_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct appHand_exiDocument)));
        }
        memset(appHandDec, 0, sizeof(struct appHand_exiDocument));
        memset(appHandEnc, 0, sizeof(struct appHand_exiDocument));
        int ret = decode_appHand_exiDocument(&exi, appHandDec);
        if (ret != 0) {
            logger.printfln("Common: Failed to decode EXI document: %d", ret);
            return;
        }

        if (appHandDec->supportedAppProtocolReq_isUsed) {
            iso15118.trace("Common: SupportedAppProtocolReq received");
            handle_supported_app_protocol_req();
            appHandDec->supportedAppProtocolReq_isUsed = 0;
        }
    } else if (exi_in_use == ExiType::Din) {
        iso15118.din70121.handle_bitstream(&exi);
    } else if (exi_in_use == ExiType::Iso2) {
        iso15118.iso2.handle_bitstream(&exi);
    } else if (exi_in_use == ExiType::Iso20) {
        iso15118.iso20.handle_bitstream(&exi, payload_type);
    }

    api_state.get("state")->updateUint(state);
}

void Common::handle_supported_app_protocol_req()
{
    struct appHand_supportedAppProtocolReq *req = &appHandDec->supportedAppProtocolReq;
    struct appHand_supportedAppProtocolRes *res = &appHandEnc->supportedAppProtocolRes;

    // check all schemas for DIN, ISO2 and ISO20
    logger.printfln("EV supports %u protocols", req->AppProtocol.arrayLen);
    api_state.get("supported_protocols")->removeAll();

    int8_t  din70121_schema_id = -1;
    uint8_t din70121_index     =  0;
    int8_t  iso2_schema_id     = -1;
    uint8_t iso2_index         =  0;
    int8_t  iso20_schema_id    = -1;
    uint8_t iso20_index        =  0;

    // TODO: Differentiate between iso:151118:2:2010 and iso:151118:2:2013
    //       iso:151118:2:2010 is the same as din:70121?
    for(uint16_t i = 0; i < req->AppProtocol.arrayLen; i++) {
        if (strnstr(req->AppProtocol.array[i].ProtocolNamespace.characters, ":din:70121:", req->AppProtocol.array[i].ProtocolNamespace.charactersLen) != nullptr) {
            din70121_schema_id = req->AppProtocol.array[i].SchemaID;
            din70121_index     = i;
        } else if (strnstr(req->AppProtocol.array[i].ProtocolNamespace.characters, "iso:15118:2:", req->AppProtocol.array[i].ProtocolNamespace.charactersLen) != nullptr) {
            iso2_schema_id = req->AppProtocol.array[i].SchemaID;
            iso2_index     = i;
        } else if(strnstr(req->AppProtocol.array[i].ProtocolNamespace.characters, "iso:15118:-20:AC", req->AppProtocol.array[i].ProtocolNamespace.charactersLen) != nullptr) {
            iso20_schema_id = req->AppProtocol.array[i].SchemaID;
            iso20_index     = i;
        }

        logger.printfln_continue("%d: %s", req->AppProtocol.array[i].SchemaID, req->AppProtocol.array[i].ProtocolNamespace.characters);
        iso15118.trace(" found %d: %s", req->AppProtocol.array[i].SchemaID, req->AppProtocol.array[i].ProtocolNamespace.characters);
        api_state.get("supported_protocols")->add()->updateString(req->AppProtocol.array[i].ProtocolNamespace.characters);
    }

    if ((din70121_schema_id == -1) && (iso2_schema_id == -1) && (iso20_schema_id == -1)) {
        logger.printfln("EV does not support DIN 70121, ISO 15118-2 or ISO 15118-20:AC");
        api_state.get("protocol")->updateString("-");
        return;
    } else {
        // Priority: ISO 15118-2 > DIN 70121 > ISO 15118-20
        // ISO 15118-2 is preferred as it is more widely supported.
        // ISO 15118-20 is currently for testing only and selected if neither ISO 2 nor DIN 70121 is available.
        uint8_t schema_id = 0;
        uint8_t index     = 0;
        if (iso2_schema_id != -1) {
            schema_id  = iso2_schema_id;
            index      = iso2_index;
            exi_in_use = ExiType::Iso2;
            logger.printfln("Using ISO 15118-2");
        } else if (din70121_schema_id != -1) {
            schema_id  = din70121_schema_id;
            index      = din70121_index;
            exi_in_use = ExiType::Din;
            logger.printfln("Using DIN 70121");
        } else {
            schema_id  = iso20_schema_id;
            index      = iso20_index;
            exi_in_use = ExiType::Iso20;
            logger.printfln("Using ISO 15118-20:AC");
        }
        api_state.get("protocol")->updateString(req->AppProtocol.array[index].ProtocolNamespace.characters);

        appHandEnc->supportedAppProtocolRes_isUsed = 1;
        res->ResponseCode = appHand_responseCodeType_OK_SuccessfulNegotiation;

        res->SchemaID = schema_id;
        res->SchemaID_isUsed = 1;

        send_exi(Common::ExiType::AppHand);
        state = 1;

        evse_v2.set_charging_protocol(1, 50);

        iso15118.trace("SupportedAppProtocolRes sent");
        iso15118.trace(" use %d: %s", schema_id, req->AppProtocol.array[index].ProtocolNamespace.characters);
    }
}

void cancel_sequence_timeout(uint64_t &next_timeout)
{
    if (next_timeout != 0) {
        task_scheduler.cancel(next_timeout);
        next_timeout = 0;
    }
}

void schedule_sequence_timeout(uint64_t &next_timeout, millis_t timeout, const char *protocol_name)
{
    next_timeout = task_scheduler.scheduleOnce([&next_timeout, protocol_name]() {
        iso15118.qca700x.link_down();
        iso15118.slac.state = SLAC::State::ModemReset;
        logger.printfln("%s Timeout: Link down, SLAC reset", protocol_name);
        next_timeout = 0;
    }, timeout);
}

SessionIdResult check_session_id(const uint8_t *received_id, size_t received_len, uint8_t *stored_id, size_t stored_len)
{
    // Check if received session ID is all zeros (new session requested)
    bool all_zero = true;
    for (size_t i = 0; i < received_len; i++) {
        if (received_id[i] != 0x00) {
            all_zero = false;
            break;
        }
    }

    // Check if received session ID matches stored session ID (resume session)
    // First compare length, then compare bytes
    bool matches_stored = false;
    if (received_len == stored_len) {
        matches_stored = true;
        for (size_t i = 0; i < stored_len; i++) {
            if (received_id[i] != stored_id[i]) {
                matches_stored = false;
                break;
            }
        }
    }

    // Generate new session ID if needed
    if (all_zero || !matches_stored) {
        for (size_t i = 0; i < stored_len; i++) {
            stored_id[i] = static_cast<uint8_t>(esp_random());
        }
        return SessionIdResult::NewSession;
    }

    return SessionIdResult::ResumeSession;
}
