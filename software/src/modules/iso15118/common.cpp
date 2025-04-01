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

// Common functions that are shared between DIN-SPEC-70121, ISO-151118-2 and ISO-15118-20

#include "common.h"

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

void Common::pre_setup()
{
    supported_protocols_prototype = Config::Str("", 0, 32);
    api_state = Config::Object({
        {"state", Config::Uint8(0)},
        {"supported_protocols", Config::Array({}, &supported_protocols_prototype, 0, 4, Config::type_id<Config::ConfString>())},
        {"protocol", Config::Str("", 0, 32)}
    });

    if (exi_data == nullptr) {
        exi_data = (uint8_t*)heap_caps_calloc_prefer(EXI_DATA_SIZE, sizeof(uint8_t), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
    }
}

void Common::setup_socket()
{
    listen_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if(listen_socket < 0) {
        logger.printfln("Common: Failed to create socket: %d", listen_socket);
        return;
    }

    if(fcntl(listen_socket, F_SETFL, fcntl(listen_socket, F_GETFL) | O_NONBLOCK) < 0) {
        logger.printfln("Common: Failed to set non-blocking mode (listen socket)");
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
        logger.printfln("Common: Failed to bind socket: %d", err);
        return;
    }

    err = listen(listen_socket, 1);
    if(err < 0) {
        logger.printfln("Common: Failed to listen on socket: %d", err);
        return;
    }
}

void Common::state_machine_loop()
{
    if (active_socket < 0) {
        active_socket = accept(listen_socket, (struct sockaddr *)&source_addr, &addr_len);
        if(active_socket < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                // No connection available, non-blocking mode
                return;
            }
            logger.printfln("Common: Failed to accept connection: %d (errno %d [%s])", active_socket, errno, strerror_r(errno, nullptr, 0));
            return;
        }

        char addr_str[128];
        inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
        logger.printfln("Common: Accepted connection from %s", addr_str);

        if(fcntl(active_socket, F_SETFL, fcntl(active_socket, F_GETFL) | O_NONBLOCK) < 0) {
            logger.printfln("Common: Failed to set non-blocking mode (active socket)");
            close(active_socket);
            active_socket = -1;
            return;
        }
    } else {
        // TODO: We assume that we always read the whole packet in one go here.
        //       This is of course not necessarily true with TCP.
        //       Instead read the V2GTP header first, then determine the length of the payload,
        //       then read the payload.
        ssize_t length = recv(active_socket, exi_data, EXI_DATA_SIZE, 0);

        if(length < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                // No data available, non-blocking mode
                return;
            } else if(errno == ECONNRESET) {
                logger.printfln("Common: Connection reset by peer");
                close(active_socket);
                active_socket = -1;
                return;
            }
            logger.printfln("Common: Failed to receive data: %d (errno %d [%s])", length, errno, strerror_r(errno, nullptr, 0));
            close(active_socket);
            active_socket = -1;
        } else if(length == 0) {
            logger.printfln("Common: Connection closed");
            close(active_socket);
            active_socket = -1;
        } else {
            decode(exi_data, length);
        }
    }
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
            // TBD
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
    header->payload_type             = htons(0x8001), // 0x8001 = EXI data
    header->payload_length           = htonl(length);

    ssize_t send_ret = send(active_socket, exi_data, length + sizeof(V2GTP_Header), 0);
    logger.printfln("Common: Sent %u bytes", length + sizeof(V2GTP_Header));
    if(send_ret < 0) {
        logger.printfln("Common: Failed to send data: %d (errno %d)", send_ret, errno);
        close(active_socket);
        active_socket = -1;
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
    if(ntohs(header->payload_type) != 0x8001) {
        logger.printfln("Common: Invalid payload type: %d", ntohs(header->payload_type));
        return;
    }

    exi_bitstream exi;
    exi_bitstream_init(&exi, &data[sizeof(V2GTP_Header)], length - sizeof(V2GTP_Header), 0, nullptr);

    if (exi_in_use == ExiType::AppHand) {
        // We alloc the appHand buffers the very first time they are used.
        // This way it is not allocated if ISO15118 is not used.
        // If it is used once we can assume that it will be used all the time, so it stays allocated.
        if (appHandDec == nullptr) {
            appHandDec = (struct appHand_exiDocument*)heap_caps_calloc_prefer(sizeof(struct appHand_exiDocument), 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
        }
        if (appHandEnc == nullptr) {
            appHandEnc = (struct appHand_exiDocument*)heap_caps_calloc_prefer(sizeof(struct appHand_exiDocument), 1, 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
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
        iso15118.iso20.handle_bitstream(&exi);
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
        } else if(strnstr(req->AppProtocol.array[i].ProtocolNamespace.characters, "iso:15118:20:", req->AppProtocol.array[i].ProtocolNamespace.charactersLen) != nullptr) {
            iso20_schema_id = req->AppProtocol.array[i].SchemaID;
            iso20_index     = i;
        }

        logger.printfln_continue("%d: %s", req->AppProtocol.array[i].SchemaID, req->AppProtocol.array[i].ProtocolNamespace.characters);
        iso15118.trace(" found %d: %s", req->AppProtocol.array[i].SchemaID, req->AppProtocol.array[i].ProtocolNamespace.characters);
        api_state.get("supported_protocols")->add()->updateString(req->AppProtocol.array[i].ProtocolNamespace.characters);
    }

    if ((din70121_schema_id == -1) && (iso2_schema_id == -1)) {
        logger.printfln("EV does not support DIN 70121 or ISO 15118-2");
        api_state.get("protocol")->updateString("-");
        return;
    } else {
        // Prefer ISO 15118-2 (if available) over DIN 70121
        uint8_t schema_id = 0;
        uint8_t index     = 0;
        if (iso2_schema_id != -1) {
            schema_id  = iso2_schema_id;
            index      = iso2_index;
            exi_in_use = ExiType::Iso2;
            logger.printfln("Using ISO 15118-2");
        } else {
            schema_id  = din70121_schema_id;
            index      = din70121_index;
            exi_in_use = ExiType::Din;
            logger.printfln("Using DIN 70121");
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