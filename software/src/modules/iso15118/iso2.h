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
#include "cbv2g/iso_2/iso2_msgDefDecoder.h"
#include "cbv2g/iso_2/iso2_msgDefEncoder.h"

#include "common.h"

#define ISO2_SECC_SEQUENCE_TIMEOUT 60_s

// When true and both charge_via_iso15118 and read_soc are enabled, run a DC session
// first to read the EV's SoC, then restart as an AC session for actual charging.
// Set to false to skip the DC SoC reading and go directly to AC charging.
// This is an expirimental feature. If this doesn't work with all of the EVs we
// we have access too, we will not offer this.
static constexpr bool ISO2_DC_SOC_BEFORE_AC = true;

inline float physical_value_to_float(const iso2_PhysicalValueType *value)
{
    return physical_value_to_float(value->Value, value->Multiplier);
}

class ISO2 final
{
public:
    ISO2(){}
    void pre_setup();
    void handle_bitstream(exi_bitstream *exi);
    void reset_dc_soc_done() { dc_soc_done = false; }

    ConfigRoot api_state;
    struct iso2_exiDocument *iso2DocDec = nullptr;
    struct iso2_exiDocument *iso2DocEnc = nullptr;

private:
    void dispatch_messages();
    void send_failed_unknown_session();

    void handle_session_setup_req();
    void handle_supported_app_protocol_req();
    void handle_service_discovery_req();
    void handle_payment_service_selection_req();
    void handle_authorization_req();
    void handle_charge_parameter_discovery_req();
    void handle_power_delivery_req();
    void handle_charging_status_req();
    void handle_session_stop_req();

    void trace_header(const struct iso2_MessageHeaderType *header, const char *name);
    void trace_request_response();

    uint8_t state = 0;
    uint64_t next_timeout = 0;
    bool pause_active = false;
    bool soc_read = false;    // Set after first SoC reading in read_soc_only mode
    bool dc_soc_done = false; // Set after DC SoC session completes (for charge_via_iso15118 + read_soc)
};
