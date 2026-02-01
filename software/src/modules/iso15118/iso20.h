/* esp32-firmware
 * Copyright (C) 2025-2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "cbv2g/common/exi_bitstream.h"
#include "cbv2g/iso_20/iso20_CommonMessages_Datatypes.h"
#include "cbv2g/iso_20/iso20_CommonMessages_Decoder.h"
#include "cbv2g/iso_20/iso20_CommonMessages_Encoder.h"
#include "cbv2g/iso_20/iso20_AC_Datatypes.h"
#include "cbv2g/iso_20/iso20_AC_Decoder.h"
#include "cbv2g/iso_20/iso20_AC_Encoder.h"

#include "sdp.h"  // For V2GTPPayloadType

#define ISO20_SECC_SEQUENCE_TIMEOUT 60_s
#define ISO20_SESSION_ID_LENGTH 8

class ISO20 final
{
public:
    ISO20(){}
    void pre_setup();
    void handle_bitstream(exi_bitstream *exi, V2GTPPayloadType payload_type);

    ConfigRoot api_state;

    // EXI document structures for common messages (allocated on first use)
    struct iso20_exiDocument *iso20DocDec = nullptr;
    struct iso20_exiDocument *iso20DocEnc = nullptr;

    // EXI document structures for AC-specific messages (allocated on first use)
    struct iso20_ac_exiDocument *iso20AcDocDec = nullptr;
    struct iso20_ac_exiDocument *iso20AcDocEnc = nullptr;

    // Session ID (8 bytes for ISO 20)
    uint8_t session_id[ISO20_SESSION_ID_LENGTH];

private:
    void dispatch_common_messages();
    void dispatch_ac_messages();

    // Common message handlers
    void handle_session_setup_req();
    void handle_authorization_setup_req();
    void handle_authorization_req();
    void handle_service_discovery_req();
    void handle_service_detail_req();
    void handle_service_selection_req();
    void handle_schedule_exchange_req();
    void handle_power_delivery_req();
    void handle_session_stop_req();
    void prepare_header(struct iso20_MessageHeaderType *header);

    // AC-specific message handlers
    void handle_ac_bitstream(exi_bitstream *exi);
    void handle_ac_charge_parameter_discovery_req();
    void handle_ac_charge_loop_req();
    void prepare_ac_header(struct iso20_ac_MessageHeaderType *header);

    void trace_header(const struct iso20_MessageHeaderType *header, const char *name);
    void trace_request_response();
    void trace_ac_request_response();

    uint8_t state = 0;
    uint64_t next_timeout = 0;

    // Selected service from ServiceSelectionReq
    uint16_t selected_service_id = 0;
    uint16_t selected_parameter_set_id = 0;
};
