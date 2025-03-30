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

#include "cbv2g/common/exi_bitstream.h"
#include "cbv2g/din/din_msgDefDecoder.h"
#include "cbv2g/din/din_msgDefEncoder.h"

class DIN70121 final
{
public:
    DIN70121(){}
    void pre_setup();
    void handle_bitstream(exi_bitstream *exi);

    ConfigRoot api_state;

    struct din_exiDocument *dinDocDec;
    struct din_exiDocument *dinDocEnc;

private:
    void handle_session_setup_req();
    void handle_supported_app_protocol_req();
    void handle_service_discovery_req();
    void handle_service_payment_selection_req();
    void handle_contract_authentication_req();
    void handle_charge_parameter_discovery_req();
    void handle_session_stop_req();

    uint8_t state = 0;
};
