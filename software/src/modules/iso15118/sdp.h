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

#pragma once

#include "module.h"
#include "config.h"
#include "sdp_state.enum.h"

// ISO 15118-2:2014 port definitions
// Table 15: V2G_UDP_SDP_SERVER - UDP port for SDP discovery
#define V2G_UDP_SDP_PORT 15118

// Table 8: V2G_DST_TCP_DATA - TCP port for V2G communication
// Must be in the Dynamic Ports range (49152-65535) as defined in IETF RFC 6335
#define V2G_TCP_DATA_PORT 49152

// SDP Security field values (ISO 15118-2:2014 Table 11)
#define SDP_SECURITY_TLS     0x00
#define SDP_SECURITY_NO_TLS  0x10

// SDP Transport Protocol field values (ISO 15118-2:2014 Table 11)
#define SDP_TRANSPORT_TCP    0x00
#define SDP_TRANSPORT_UDP    0x10

// V2GTP Payload Types (ISO 15118-2:2014 Table 10, ISO 15118-20:2022 Table 212)
enum class V2GTPPayloadType : uint16_t {
    // SDP payload types (UDP)
    SDPRequest                  = 0x9000,  // SDP Request (SECC Discovery Request)
    SDPResponse                 = 0x9001,  // SDP Response (SECC Discovery Response)

    // EXI payload types (TCP)
    SAP                         = 0x8001,  // Supported App Protocol / ISO 15118-2 / DIN 70121
    ISO20Common                 = 0x8002,  // ISO 15118-20 Common Messages
    ISO20AC                     = 0x8003,  // ISO 15118-20 AC Messages
    ISO20DC                     = 0x8004,  // ISO 15118-20 DC Messages
    ISO20ACDP                   = 0x8005,  // ISO 15118-20 ACDP Messages
    ISO20WPT                    = 0x8006,  // ISO 15118-20 WPT Messages
    ScheduleRenegotiation       = 0x8101,  // ISO 15118-20 Schedule Renegotiation
    MeteringConfirmation        = 0x8102,  // ISO 15118-20 Metering Confirmation
    ACDPSystemStatus            = 0x8103,  // ISO 15118-20 ACDP System Status
};


struct [[gnu::packed]] V2GTP_Header {
    uint8_t protocol_version;
    uint8_t inverse_protocol_version;
    uint16_t payload_type;
    uint32_t payload_length;
};

struct [[gnu::packed]] SDP_DiscoveryRequest {
    V2GTP_Header v2gtp;

    uint8_t security;
    uint8_t tranport_protocol;
};

struct [[gnu::packed]] SDP_DiscoveryResponse {
    V2GTP_Header v2gtp;

    uint32_t secc_ip_address[4];
    uint16_t secc_port;
    uint8_t security;
    uint8_t tranport_protocol;
};

class SDP final
{
public:
    SDP(){}
    void setup_socket();
    void close_socket();
    void handle_socket();  // Called by central poll when socket has data
    void pre_setup();

    int get_socket() const { return sdp_socket; }

    ConfigRoot api_state;

private:
    int sdp_socket = -1;
    SDPState state = SDPState::Idle;
};
