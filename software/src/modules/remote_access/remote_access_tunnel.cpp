/* esp32-firmware
 * Copyright (C) 2026 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "remote_access_tunnel.h"

#include <array>
#include <cstdarg>
#include <cstdio>
#include <memory>
#include <utility>
#include <esp_netif.h>
#include <esp_random.h>
#include <lwip/inet_chksum.h>
#include <lwip/ip6.h>
#include <lwip/prot/ip6.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tools/net.h"

void remote_access_log_packet(const char *fmt, ...)
{
    char message[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(message, sizeof(message), fmt, args);
    va_end(args);

    // Copy formatted values before the packet and stack-local addresses expire.
    // Never enter the event log (and its WebSocket locks) from the TCP/IP task.
    task_scheduler.scheduleOnce([message = String(message)]() {
        logger.printfln("%s", message.c_str());
    });
}

namespace {
struct TunnelState {
    RemoteAccessTunnel *tunnel = nullptr;
    ip6_addr_t local = {};
    ip6_addr_t peer = {};
    bool discovered = false;
};

constexpr size_t TUNNEL_COUNT = MAX_USER_CONNECTIONS + 1;
TunnelState tunnels[TUNNEL_COUNT];
constexpr uint16_t DISCOVERY_PAYLOAD_LENGTH = 24; // Echo header plus 16-byte session nonce.

bool valid_peer(const ip6_addr_t &address)
{
    return !ip6_addr_isany_val(address) && !ip6_addr_ismulticast(&address) &&
           !ip6_addr_islinklocal(&address) && !ip6_addr_isloopback(&address) &&
           !ip6_addr_isipv4mappedipv6(&address);
}

void log_packet(size_t slot, const struct pbuf *packet, bool incoming, const char *reason)
{
    const char *kind = slot == RemoteAccessTunnel::MANAGEMENT_SLOT ? "management" : "user";
    uint8_t header[42];
    const uint16_t copied = pbuf_copy_partial(packet, header, sizeof(header), 0);
    if (copied >= IP6_HLEN && (header[0] >> 4) == 6) {
        ip6_addr_t source = {}, destination = {};
        memcpy(source.addr, header + 8, sizeof(source.addr));
        memcpy(destination.addr, header + 24, sizeof(destination.addr));
        char source_str[IP6ADDR_STRLEN_MAX], destination_str[IP6ADDR_STRLEN_MAX];
        tf_ip6addr_ntoa(&source, source_str, sizeof(source_str));
        tf_ip6addr_ntoa(&destination, destination_str, sizeof(destination_str));
        if (header[6] == IP6_NEXTH_ICMP6 && copied == sizeof(header)) {
            remote_access_log_packet("%s packet on %s tunnel slot %zu (%hu bytes, ICMPv6 type %hhu, code %hhu): %s -> %s: %s",
                                    incoming ? "incoming" : "outgoing", kind, slot, packet->tot_len, header[40], header[41],
                                    source_str, destination_str, reason);
        } else {
            remote_access_log_packet("%s packet on %s tunnel slot %zu (%hu bytes, IPv6 next header %hhu): %s -> %s: %s",
                                    incoming ? "incoming" : "outgoing", kind, slot, packet->tot_len, header[6],
                                    source_str, destination_str, reason);
        }
    } else {
        remote_access_log_packet("%s packet on %s tunnel slot %zu (%hu bytes): %s",
                                incoming ? "incoming" : "outgoing", kind, slot, packet->tot_len, reason);
    }
}

// The IPv6 header and total length have already been checked by filter_packet.
// Walk extension headers before reading ports, including across pbuf segments.
const char *management_packet_error(const struct pbuf *packet, uint8_t protocol, bool incoming)
{
    uint16_t offset = IP6_HLEN;
    while (protocol == IP6_NEXTH_HOPBYHOP || protocol == IP6_NEXTH_ROUTING ||
           protocol == IP6_NEXTH_DESTOPTS || protocol == IP6_NEXTH_FRAGMENT) {
        uint8_t extension[8];
        if (pbuf_copy_partial(packet, extension, sizeof(extension), offset) != sizeof(extension)) {
            return "truncated IPv6 extension header";
        }
        const uint16_t length = protocol == IP6_NEXTH_FRAGMENT ? sizeof(extension) : (extension[1] + 1) * 8;
        // Later fragments cannot be checked for a permitted destination port.
        if (protocol == IP6_NEXTH_FRAGMENT && (extension[2] != 0 || extension[3] != 0)) {
            return "fragmented traffic is not allowed";
        }
        if (offset + length > packet->tot_len) {
            return "IPv6 extension header exceeds packet length";
        }
        offset += length;
        protocol = extension[0];
    }

    // ICMPv6 has no ports; it is required for echo and Packet Too Big handling.
    if (protocol != IP6_NEXTH_ICMP6 && protocol != IP6_NEXTH_TCP && protocol != IP6_NEXTH_UDP) {
        return "unsupported transport protocol";
    }
    if (offset + (protocol == IP6_NEXTH_TCP ? 20 : 8) > packet->tot_len) {
        return "truncated transport header";
    }
    if (incoming && protocol != IP6_NEXTH_ICMP6) {
        uint8_t ports[4];
        if (pbuf_copy_partial(packet, ports, sizeof(ports), offset) != sizeof(ports)) {
            return "cannot read transport ports";
        }
        const uint16_t port = (static_cast<uint16_t>(ports[2]) << 8) | ports[3];
        if (port != 12345 && (remote_access.send_port == 0 || port != remote_access.send_port)) {
            return "destination port is not allowed";
        }
    }
    return nullptr;
}

int filter_packet(size_t slot, struct pbuf *packet, bool incoming)
{
    const auto filtered = [slot, packet, incoming](const char *reason, int result = ERR_VAL) {
        log_packet(slot, packet, incoming, reason);
        return result;
    };
    if (slot == RemoteAccessTunnel::MANAGEMENT_SLOT && incoming) {
        log_packet(slot, packet, incoming, "received");
    }

    TunnelState &state = tunnels[slot];
    uint8_t header[IP6_HLEN];
    if (state.tunnel == nullptr) {
        return filtered("tunnel is not configured");
    }
    if (pbuf_copy_partial(packet, header, sizeof(header), 0) != sizeof(header)) {
        return filtered("truncated IPv6 header");
    }
    if ((header[0] >> 4) != 6) {
        return filtered("expected IPv6");
    }
    const uint16_t payload_length = static_cast<uint16_t>((header[4] << 8) | header[5]);
    if (sizeof(header) + payload_length != packet->tot_len) {
        return filtered("IPv6 payload length does not match packet length");
    }
    if (slot == RemoteAccessTunnel::MANAGEMENT_SLOT) {
        if (const char *error = management_packet_error(packet, header[6], incoming)) {
            return filtered(error);
        }
    }
    ip6_addr_t source = {}, destination = {};
    memcpy(source.addr, header + 8, sizeof(source.addr));
    memcpy(destination.addr, header + 24, sizeof(destination.addr));

    if (incoming && ip6_addr_isallnodes_linklocal(&destination)) {
        // Each authenticated tunnel has one peer. Answer all-nodes multicast
        // echo from our service ULA, before lwIP's multicast echo policy and
        // source selection, so the peer learns the usable random address.
        if (header[6] != IP6_NEXTH_ICMP6) {
            return filtered("discovery requires ICMPv6");
        }
        if (payload_length != DISCOVERY_PAYLOAD_LENGTH) {
            return filtered("invalid discovery payload length");
        }
        if (!valid_peer(source) || ip6_addr_cmp_zoneless(&source, &state.local)) {
            return filtered("invalid discovery source address");
        }
        // The authenticated management server can restart independently and
        // choose a new address. User peers remain pinned until their tunnel ends.
        if (slot != RemoteAccessTunnel::MANAGEMENT_SLOT && state.discovered &&
            !ip6_addr_cmp_zoneless(&state.peer, &source)) {
            return filtered("discovery source does not match the learned peer");
        }
        std::unique_ptr<struct pbuf, decltype(&pbuf_free)> reply(pbuf_alloc(PBUF_IP, payload_length, PBUF_RAM), pbuf_free);
        if (reply == nullptr) {
            return filtered("cannot allocate discovery reply", ERR_MEM);
        }
        if (pbuf_copy_partial(packet, reply->payload, payload_length, sizeof(header)) != payload_length) {
            return filtered("truncated discovery payload");
        }
        auto *echo = static_cast<uint8_t *>(reply->payload);
        if (echo[0] != 128 || echo[1] != 0) {
            return filtered("discovery requires an echo request with code zero");
        }
        const uint16_t discovery_ident = slot == RemoteAccessTunnel::MANAGEMENT_SLOT ? 0x4d47 : 0x5757;
        if (((echo[4] << 8) | echo[5]) != discovery_ident) {
            return filtered("invalid discovery identifier");
        }
        if (ip6_chksum_pseudo(reply.get(), IP6_NEXTH_ICMP6, payload_length, &source, &destination) != 0) {
            return filtered("invalid discovery checksum");
        }
        struct netif *interface = state.tunnel->routeIPv6(&source);
        if (interface == nullptr) {
            return filtered("no route for discovery reply", ERR_RTE);
        }
        // Pin the authenticated peer for this session. The output filter and
        // routing hook already need the learned address while sending the reply.
        state.peer = source;
        state.discovered = true;
        echo[0] = 129;
        echo[2] = 0;
        echo[3] = 0;
        const uint16_t checksum = ip6_chksum_pseudo(reply.get(), IP6_NEXTH_ICMP6, payload_length, &state.local, &source);
        memcpy(echo + 2, &checksum, sizeof(checksum));
        const err_t result = ip6_output_if_src(reply.get(), &state.local, &source, 64, 0, IP6_NEXTH_ICMP6, interface);
        if (result != ERR_OK) {
            remote_access_log_packet("Discovery reply failed on tunnel slot %zu: lwIP error %hhi", slot, result);
            return filtered("cannot send discovery reply");
        }
        char local_str[IP6ADDR_STRLEN_MAX], peer_str[IP6ADDR_STRLEN_MAX];
        tf_ip6addr_ntoa(&state.local, local_str, sizeof(local_str));
        tf_ip6addr_ntoa(&state.peer, peer_str, sizeof(peer_str));
        remote_access_log_packet("Handled incoming discovery packet on %s tunnel slot %zu (%hu bytes): %s -> %s reply sent",
                       slot == RemoteAccessTunnel::MANAGEMENT_SLOT ? "management" : "user",
                       slot, packet->tot_len, local_str, peer_str);
        // WireGuard retains ownership and frees the consumed request.
        return ERR_VAL;
    }

    if (!state.discovered) {
        return filtered("peer discovery is incomplete");
    }
    const ip6_addr_t &expected_source = incoming ? state.peer : state.local;
    const ip6_addr_t &expected_destination = incoming ? state.local : state.peer;
    if (!ip6_addr_cmp_zoneless(&source, &expected_source)) {
        return filtered("source address does not match the discovered pair");
    }
    if (!ip6_addr_cmp_zoneless(&destination, &expected_destination)) {
        return filtered("destination address does not match the discovered pair");
    }
    return ERR_OK;
}

template<size_t Slot, bool Incoming> int filter(struct pbuf *packet) { return filter_packet(Slot, packet, Incoming); }

template<bool Incoming, size_t... Slots> constexpr auto make_filters(std::index_sequence<Slots...>)
{
    return std::array<RemoteAccessTunnel::PacketFilter, sizeof...(Slots)>{&filter<Slots, Incoming>...};
}
constexpr auto input_filters = make_filters<true>(std::make_index_sequence<TUNNEL_COUNT>{});
constexpr auto output_filters = make_filters<false>(std::make_index_sequence<TUNNEL_COUNT>{});
} // namespace

IPAddress random_remote_access_address()
{
    uint8_t bytes[16];
    esp_fill_random(bytes, sizeof(bytes));
    bytes[0] = 0xfd;
    return IPAddress(IPv6, bytes);
}

RemoteAccessTunnel::~RemoteAccessTunnel()
{
    end();
}

bool RemoteAccessTunnel::configure_ipv6()
{
    if (slot >= TUNNEL_COUNT) {
        return false;
    }
    const IPAddress address = random_remote_access_address();
    // The remote address is initially unknown. The input/output filters restrict
    // this authenticated peer to discovery and the subsequently learned pair.
    if (!setIPv6Address(address.toString().c_str()) || !addAllowedIP("::", 0)) {
        return false;
    }
    struct {
        RemoteAccessTunnel *tunnel;
        uint8_t slot;
        ip_addr_t address;
    } parameters = {this, slot, {}};
    address.to_ip_addr_t(&parameters.address);
    return esp_netif_tcpip_exec([](void *arg) -> esp_err_t {
        const auto *data = static_cast<decltype(parameters) *>(arg);
        tunnels[data->slot] = {};
        tunnels[data->slot].tunnel = data->tunnel;
        tunnels[data->slot].local = *ip_2_ip6(&data->address);
        return ESP_OK;
    }, &parameters) == ESP_OK;
}

void RemoteAccessTunnel::end()
{
    esp_netif_tcpip_exec([](void *arg) -> esp_err_t {
        auto *tunnel = static_cast<RemoteAccessTunnel *>(arg);
        if (tunnel->slot < TUNNEL_COUNT && tunnels[tunnel->slot].tunnel == tunnel) {
            tunnels[tunnel->slot] = {};
        }
        return ESP_OK;
    }, this);
    WireGuard::end();
}

RemoteAccessTunnel::PacketFilter RemoteAccessTunnel::input_filter(uint8_t slot)
{
    return slot < input_filters.size() ? input_filters[slot] : nullptr;
}

RemoteAccessTunnel::PacketFilter RemoteAccessTunnel::output_filter(uint8_t slot)
{
    return slot < output_filters.size() ? output_filters[slot] : nullptr;
}

bool RemoteAccessTunnel::get_address(uint8_t slot, bool peer, ip6_addr_t &address)
{
    if (slot >= TUNNEL_COUNT) {
        return false;
    }
    struct {
        uint8_t slot;
        bool peer;
        ip6_addr_t address;
    } parameters = {slot, peer, {}};
    const auto result = esp_netif_tcpip_exec([](void *arg) -> esp_err_t {
        auto *data = static_cast<decltype(parameters) *>(arg);
        const auto &state = tunnels[data->slot];
        if (state.tunnel == nullptr || (data->peer && !state.discovered)) {
            return ESP_FAIL;
        }
        data->address = data->peer ? state.peer : state.local;
        return ESP_OK;
    }, &parameters);
    address = parameters.address;
    return result == ESP_OK;
}

bool RemoteAccessTunnel::is_connected_address(const IPAddress &local, const IPAddress &peer)
{
    if (local.type() != IPv6 || peer.type() != IPv6) {
        return false;
    }
    struct {
        ip_addr_t local;
        ip_addr_t peer;
        bool connected;
    } parameters = {};
    local.to_ip_addr_t(&parameters.local);
    peer.to_ip_addr_t(&parameters.peer);
    const auto result = esp_netif_tcpip_exec([](void *arg) -> esp_err_t {
        auto *data = static_cast<decltype(parameters) *>(arg);
        // Management peers do not authenticate user HTTP requests.
        for (size_t i = 0; i < MAX_USER_CONNECTIONS; ++i) {
            const auto &state = tunnels[i];
            if (state.tunnel != nullptr && state.discovered &&
                ip6_addr_cmp_zoneless(&state.local, ip_2_ip6(&data->local)) &&
                ip6_addr_cmp_zoneless(&state.peer, ip_2_ip6(&data->peer))) {
                data->connected = true;
                break;
            }
        }
        return ESP_OK;
    }, &parameters);
    return result == ESP_OK && parameters.connected;
}

// Random peers may be in unrelated /64s. Match this tunnel's source address,
// rather than installing a default route for its bootstrap ::/0 allowed prefix.
extern "C" struct netif *lwip_hook_ip6_route(const ip6_addr_t *source, const ip6_addr_t *destination)
{
    if (source == nullptr || ip6_addr_isany(source)) {
        return nullptr;
    }
    for (const auto &state : tunnels) {
        if (state.tunnel != nullptr && state.discovered &&
            ip6_addr_cmp_zoneless(source, &state.local) &&
            ip6_addr_cmp_zoneless(destination, &state.peer)) {
            return state.tunnel->routeIPv6(destination);
        }
    }
    return nullptr;
}
