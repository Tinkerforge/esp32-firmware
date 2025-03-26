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

#pragma once

#include "module.h"
#include "config.h"

#include "lwip/sockets.h"

#include "sdp.h"

class DIN70121 final
{
public:
    DIN70121(){}
    void setup_socket();
    void state_machine_loop();
    void pre_setup();

    ConfigRoot api_state;
    Config supported_protocols_prototype;

private:
    enum class ExiType : uint8_t {
        AppHand,
        Din
    };

    void handle_session_setup_req();
    void handle_supported_app_protocol_req();
    void handle_service_discovery_req();
    void handle_service_payment_selection_req();
    void handle_contract_authentication_req();
    void handle_charge_parameter_discovery_req();
    void handle_session_stop_req();

    void decode(uint8_t *data, const size_t length);
    void send_exi(ExiType type);

    int listen_socket = -1;
    int active_socket = -1;
    struct sockaddr_storage source_addr;
    socklen_t addr_len = sizeof(source_addr);

    uint8_t state = 0;

};
