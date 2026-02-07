/* esp32-firmware
 * Copyright (C) 2024-2026 Olaf Lüke <olaf@tinkerforge.com>
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
#include "tools/net.h"
#include "esp_netif.h"
#include <sys/socket.h>
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "qca700x.h"

#include "gcc_warnings.h"

// IPv6 all-nodes multicast address (ff02::1) in network byte order
// ff02::1 = ff02:0000:0000:0000:0000:0000:0000:0001
#define IN6ADDR_ALLNODES_INIT {{{PP_HTONL(0xff020000UL), 0, 0, PP_HTONL(0x00000001UL)}}}

void SDP::pre_setup()
{
    api_state = Config::Object({
        {"state", Config::Uint8(0)},
        {"evse_ip_address", Config::Tuple(4, Config::Uint32(0))},// TODO: why is this not a Config::String("", 0, INET6_ADDRSTRLEN)?
        {"evse_port", Config::Uint16(0)},
        {"evse_security", Config::Int16(-1)},
        {"evse_tranport_protocol", Config::Int16(-1)},
        {"ev_security", Config::Int16(-1)},
        {"ev_tranport_protocol", Config::Int16(-1)}
    });
}

void SDP::setup_socket()
{
    // Close existing socket if any
    if (sdp_socket >= 0) {
        close(sdp_socket);
        sdp_socket = -1;
    }

    // For SDP we need to accept incoming UDP connections from any address on port 15118.
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_addr   = in6addr_any;
    server_addr.sin6_port   = htons(V2G_UDP_SDP_PORT);

    sdp_socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (sdp_socket < 0) {
        logger.printfln("SDP: Failed to create socket: (errno %i [%s])", errno, strerror(errno));
        return;
    }

    int opt = 1;
    setsockopt(sdp_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sdp_socket, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
        logger.printfln("SDP: Failed to bind socket: (errno %i [%s])", errno, strerror(errno));
        close(sdp_socket);
        sdp_socket = -1;
        return;
    }

    // In debug mode, join the IPv6 all-nodes multicast group (ff02::1) on the Ethernet interface
    // This is needed because in debug mode we don't have SLAC to set this up and
    // the EV simulator sends SDP requests to the multicast address
    if (iso15118.debug_mode) {
#if LWIP_IPV6_MLD
        esp_netif_t *eth_netif = esp_netif_get_handle_from_ifkey("ETH_DEF");
        if (eth_netif != NULL) {
            int if_index = esp_netif_get_netif_impl_index(eth_netif);

            struct ipv6_mreq mreq;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
            static const struct in6_addr in6addr_allnodes = IN6ADDR_ALLNODES_INIT;
#pragma GCC diagnostic pop
            memcpy(&mreq.ipv6mr_multiaddr, &in6addr_allnodes, sizeof(struct in6_addr));
            mreq.ipv6mr_interface = static_cast<unsigned int>(if_index);

            if (setsockopt(sdp_socket, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq, sizeof(mreq)) < 0) {
                logger.printfln("SDP: Failed to join IPv6 multicast group ff02::1: (errno %i [%s])", errno, strerror(errno));
            } else {
                logger.printfln("SDP: Joined IPv6 multicast group ff02::1 on interface %d", if_index);
            }
        } else {
            logger.printfln("SDP: Could not get Ethernet interface for multicast join");
        }
#else
        logger.printfln("SDP: LWIP_IPV6_MLD not enabled, cannot join multicast group");
#endif
    }

    const int flags = fcntl(sdp_socket, F_GETFL, 0);
    int ret = fcntl(sdp_socket, F_SETFL, flags | O_NONBLOCK);
    if (ret < 0) {
        logger.printfln("SDP: Failed to set non-blocking mode: (errno %i [%s])", errno, strerror(errno));
        close(sdp_socket);
        sdp_socket = -1;
        return;
    }

    // Register socket in central poll array
    iso15118.set_poll_fd(FDS_SDP_INDEX, sdp_socket);
}

void SDP::close_socket()
{
    if (sdp_socket >= 0) {
        close(sdp_socket);
        sdp_socket = -1;
    }
    iso15118.set_poll_fd(FDS_SDP_INDEX, -1);
}

void SDP::handle_socket()
{
    if (sdp_socket < 0) {
        return;
    }

    uint8_t buffer[64];
    struct sockaddr_in6 recv_addr;
    socklen_t recv_addr_length = sizeof(recv_addr);
    ssize_t length = recvfrom(sdp_socket, buffer, sizeof(buffer), 0, reinterpret_cast<struct sockaddr*>(&recv_addr), &recv_addr_length);
    if (length < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            // No data available, non-blocking mode
            return;
        } else {
            logger.printfln("SDP recv failed: (errno %i [%s])", errno, strerror(errno));
            close(sdp_socket);
            sdp_socket = -1;
            setup_socket();
            return;
        }
    }

    if (length > 0) {
        char straddr[INET6_ADDRSTRLEN];
        tf_ip6addr_ntoa(&recv_addr.sin6_addr, straddr, INET6_ADDRSTRLEN);

        if (length != sizeof(SDP_DiscoveryRequest)) {
            logger.printfln("Invalid SDP_DiscoveryRequest length: %d", length);
            return;
        }

        SDP_DiscoveryRequest *request = reinterpret_cast<SDP_DiscoveryRequest*>(buffer);
        if (request->v2gtp.protocol_version != 0x01 || request->v2gtp.inverse_protocol_version != 0xFE) {
            logger.printfln("Invalid V2GTP protocol version: %02x, %02x", request->v2gtp.protocol_version, request->v2gtp.inverse_protocol_version);
            return;
        }

        if (request->v2gtp.payload_type != htons(static_cast<uint16_t>(V2GTPPayloadType::SDPRequest))) {
            logger.printfln("Invalid V2GTP payload type: %04x", htons(request->v2gtp.payload_type));
            return;
        }

        if (request->v2gtp.payload_length != htonl((sizeof(SDP_DiscoveryRequest) - sizeof(V2GTP_Header)))) {
            logger.printfln("Invalid V2GTP payload length: %lu", htonl(request->v2gtp.payload_length));
            return;
        }

        // Security 0x00 = TLS, 0x10 = no security
        // Transport Protocol 0x00 = TCP, 0x10 = UDP (only TCP allowed here)
        iso15118.trace("Got SDP Discovery Request with security %02x, transport_protocol %02x", request->security, request->tranport_protocol);
        api_state.get("ev_security")->updateInt(request->security);
        api_state.get("ev_tranport_protocol")->updateInt(request->tranport_protocol);

        esp_ip6_addr_t ip6;
        if (!iso15118.qca700x.get_ip6_linklocal(&ip6)) {
            logger.printfln("SDP: Failed to get link-local IPv6 address, cannot send response");
            return;
        }

        // Determine security mode for response
        // TLS is always supported. If the EV requests TLS (security=0x00), respond with TLS.
        // Otherwise respond with no security.
        uint8_t response_security = SDP_SECURITY_NO_TLS;
        if (request->security == SDP_SECURITY_TLS) {
            response_security = SDP_SECURITY_TLS;
            iso15118.common.tls_requested_by_ev = true;
            iso15118.trace("SDP: TLS negotiated - will use TLS for V2G connection");
        } else {
            iso15118.common.tls_requested_by_ev = false;
        }

        SDP_DiscoveryResponse response = {
            .v2gtp = {
                .protocol_version = 0x01,
                .inverse_protocol_version = 0xFE,
                .payload_type = htons(static_cast<uint16_t>(V2GTPPayloadType::SDPResponse)),
                .payload_length = htonl(sizeof(SDP_DiscoveryResponse) - sizeof(V2GTP_Header))
            },
            .secc_ip_address = {ip6.addr[0], ip6.addr[1], ip6.addr[2], ip6.addr[3]},
            .secc_port = htons(V2G_TCP_DATA_PORT),
            .security = response_security,
            .tranport_protocol = SDP_TRANSPORT_TCP
        };
        iso15118.trace("Sending SDP Discovery Response with security %02x, transport_protocol %02x", response.security, response.tranport_protocol);

        int ret = sendto(sdp_socket, &response, sizeof(response), 0, reinterpret_cast<struct sockaddr*>(&recv_addr), recv_addr_length);
        if (ret < 0) {
            logger.printfln("SDP sendto failed: (%i)%s", errno, strerror(errno));
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
