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

#pragma once

#include "module.h"
#include "config.h"


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
    void state_machine_loop();
    void pre_setup();

    ConfigRoot api_state;

private:
    int sdp_socket = -1;
};
