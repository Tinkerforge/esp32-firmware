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
#include "cbv2g/app_handshake/appHand_Decoder.h"
#include "cbv2g/app_handshake/appHand_Encoder.h"

#define SESSION_ID_LENGTH 4

class Common final
{
public:
    Common(){}
    void setup_socket();
    void state_machine_loop();
    void pre_setup();


    ConfigRoot api_state;
    Config supported_protocols_prototype;

    uint8_t session_id[SESSION_ID_LENGTH];
    enum class ExiType : uint8_t {
        AppHand,
        Din
    };

    void send_exi(ExiType type);
    void prepare_din_header(struct din_MessageHeaderType *header);

private:
    void handle_session_setup_req();
    void handle_supported_app_protocol_req();

    void decode(uint8_t *data, const size_t length);

    struct appHand_exiDocument appHandDec;
    struct appHand_exiDocument appHandEnc;

    int listen_socket = -1;
    int active_socket = -1;
    struct sockaddr_storage source_addr;
    socklen_t addr_len = sizeof(source_addr);

    uint8_t exi_data[1024] = {0};

    uint8_t state = 0;

};
