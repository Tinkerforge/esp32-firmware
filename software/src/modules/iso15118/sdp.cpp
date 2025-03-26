/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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

// SECC Discovery Protocol (SDP) according to ISO 15118-2:2014 7.10.1

#include "sdp.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "esp_netif.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include "lwip/ip_addr.h"

#include "qca700x.h"

void SDP::pre_setup()
{
    api_state = Config::Object({
        {"state", Config::Uint8(0)},
        {"evse_ip_address", Config::Array({
            Config::Uint32(0), Config::Uint32(0), Config::Uint32(0), Config::Uint32(0)
        }, Config::get_prototype_uint32_0(), 4, 4, Config::type_id<Config::ConfUint>())},
        {"evse_port", Config::Uint16(0)},
        {"evse_security", Config::Int16(-1)},
        {"evse_tranport_protocol", Config::Int16(-1)},
        {"ev_security", Config::Int16(-1)},
        {"ev_tranport_protocol", Config::Int16(-1)}
    });
}

void SDP::setup_socket()
{
    if (sdp_socket > 0) {
        return;
    }

    // For SDP we need to accept incoming UDP connections from any address on port 15118.
    struct sockaddr_in6 server_addr;
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr   = in6addr_any;
    server_addr.sin6_port   = htons(15118);

    sdp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sdp_socket < 0) {
        logger.printfln("Failed to create socket: (%i)%s", errno, strerror_r(errno, nullptr, 0));
        return;
    }

    if (bind(sdp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        logger.printfln("Failed to bind socket: (%i)%s", errno, strerror_r(errno, nullptr, 0));
        close(sdp_socket);
        sdp_socket = -1;
        return;
    }

    const int flags = fcntl(sdp_socket, F_GETFL, 0);
    int ret = fcntl(sdp_socket, F_SETFL, flags | O_NONBLOCK);
    if (ret < 0) {
        logger.printfln("Failed to set non-blocking mode: (%i)%s", errno, strerror_r(errno, nullptr, 0));
        close(sdp_socket);
        sdp_socket = -1;
        return;
    }
}

void SDP::state_machine_loop()
{
    if (sdp_socket < 0) {
        return;
    }

    uint8_t buffer[64];
    struct sockaddr_in6 recv_addr;
    socklen_t recv_addr_length = sizeof(recv_addr);
    ssize_t length = recvfrom(sdp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&recv_addr, &recv_addr_length);
    if (length < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // No data available, non-blocking mode
            return;
        } else {
            logger.printfln("SDP recv failed: (%i)%s", errno, strerror_r(errno, nullptr, 0));
            close(sdp_socket);
            sdp_socket = -1;
            setup_socket();
            return;
        }
    }

    if (length > 0) {
        char straddr[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &recv_addr.sin6_addr, straddr, INET6_ADDRSTRLEN);

        if (length != sizeof(SDP_DiscoveryRequest)) {
            logger.printfln("Invalid SDP_DiscoveryRequest length: %d", length);
            return;
        }

        SDP_DiscoveryRequest *request = reinterpret_cast<SDP_DiscoveryRequest*>(buffer);
        if (request->v2gtp.protocol_version != 0x01 || request->v2gtp.inverse_protocol_version != 0xFE) {
            logger.printfln("Invalid V2GTP protocol version: %02x, %02x", request->v2gtp.protocol_version, request->v2gtp.inverse_protocol_version);
            return;
        }

        if (request->v2gtp.payload_type != htons(0x9000)) {
            logger.printfln("Invalid V2GTP payload type: %04x", htons(request->v2gtp.payload_type));
            return;
        }

        if (request->v2gtp.payload_length != htonl((sizeof(SDP_DiscoveryRequest) - sizeof(V2GTP_Header)))) {
            logger.printfln("Invalid V2GTP payload length: %ld", htonl(request->v2gtp.payload_length));
            return;
        }

        // Security 0x00 = TLS, 0x10 = no security
        // Transport Protocol 0x00 = TCP, 0x10 = UDP (only TCP allowed here)
        iso15118.trace("Got SDP Discovery Request with security %02x, transport_protocol %02x", request->security, request->tranport_protocol);
        api_state.get("ev_security")->updateInt(request->security);
        api_state.get("ev_tranport_protocol")->updateInt(request->tranport_protocol);

        esp_ip6_addr_t ip6;
        qca700x.get_ip6_linklocal(&ip6);

        SDP_DiscoveryResponse response = {
            .v2gtp = {
                .protocol_version = 0x01,
                .inverse_protocol_version = 0xFE,
                .payload_type = htons(0x9001),
                .payload_length = htonl(sizeof(SDP_DiscoveryResponse) - sizeof(V2GTP_Header))
            },
            .secc_ip_address = {ip6.addr[0], ip6.addr[1], ip6.addr[2], ip6.addr[3]},
            .secc_port = htons(15118),
            .security = 0x10,
            .tranport_protocol = 0x00
        };
        iso15118.trace("Sending SDP Discovery Response with security %02x, transport_protocol %02x", response.security, response.tranport_protocol);

        int ret = sendto(sdp_socket, &response, sizeof(response), 0, (struct sockaddr*)&recv_addr, recv_addr_length);
        if (ret < 0) {
            logger.printfln("SDP sendto failed: (%i)%s", errno, strerror_r(errno, nullptr, 0));
            close(sdp_socket);
            sdp_socket = -1;
            setup_socket();
            return;
        }

        for (uint8_t i = 0; i < 4; i++) {
            api_state.get("evse_ip_address")->get(i)->updateUint(response.secc_ip_address[i]);
        }
        api_state.get("evse_port")->updateUint(ntohs(response.secc_port));
        api_state.get("evse_security")->updateInt(response.security);
        api_state.get("evse_tranport_protocol")->updateInt(response.tranport_protocol);
    }

}