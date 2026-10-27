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


#pragma once

#include <WireGuard-ESP32.h>

#define MAX_USER_CONNECTIONS 5

// A tunnel learns its peer from an authenticated ICMPv6 discovery request.
// All address/route state is accessed only in the lwIP TCP/IP context.
class RemoteAccessTunnel final : public WireGuard {
public:
    using PacketFilter = int (*)(struct pbuf *);
    static constexpr uint8_t MANAGEMENT_SLOT = MAX_USER_CONNECTIONS;

    explicit RemoteAccessTunnel(uint8_t slot) : slot(slot) {}
    ~RemoteAccessTunnel();

    bool configure_ipv6();
    void end();

    static PacketFilter input_filter(uint8_t slot);
    static PacketFilter output_filter(uint8_t slot);
    // Copies address state in lwIP context. Peer addresses require discovery.
    static bool get_address(uint8_t slot, bool peer, ip6_addr_t &address);
    static bool is_connected_address(const IPAddress &local, const IPAddress &peer);

private:
    uint8_t slot;
};

// Generate a fresh ULA, without a shared subnet or connection-number encoding.
IPAddress random_remote_access_address();

// Packet filters run in lwIP context. Defer logging to the main task because
// the event log takes WebSocket locks whose owners may be waiting for lwIP.
[[gnu::format(__printf__, 1, 2)]]
void remote_access_log_packet(const char *fmt, ...);
