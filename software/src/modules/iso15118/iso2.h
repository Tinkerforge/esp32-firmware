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
#include "pnc.h"
#include "generated/iso2_state.enum.h"

// When true and both charge_via_iso15118 and read_soc are enabled, run a DC session
// first to read the EV's SoC, then restart as an AC session for actual charging.
// Set to false to skip the DC SoC reading and go directly to AC charging.
// This is an experimental feature. If this doesn't work with all EVs we have access to, we will not offer this.
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

    bool soc_was_read() const { return soc_read; }

    void reset_session()
    {
        state = ISO2State::Idle;
        soc_read = false;
        soc_shutdown_retries = 0;
        reset_pnc_session();
    }

    ConfigRoot api_state;
    struct iso2_exiDocument *iso2DocDec = nullptr;
    struct iso2_exiDocument *iso2DocEnc = nullptr;
    uint64_t next_timeout = 0;

private:
    void dispatch_messages();
    void send_failed_unknown_session();

    void handle_session_setup_req();
    void handle_service_discovery_req();
    void handle_payment_service_selection_req();
    void handle_authorization_req();
    void handle_charge_parameter_discovery_req();
    void handle_power_delivery_req();
    void handle_charging_status_req();
    void handle_cable_check_req();
    void handle_pre_charge_req();
    void handle_current_demand_req();
    void handle_session_stop_req();

    void finish_or_abort_dc_soc_session(const char *via);

    void trace_header(const struct iso2_MessageHeaderType *header, const char *name);
    void trace_request_response();

    ISO2State state = ISO2State::Idle;
    bool pause_active = false;
    bool soc_read = false;              // Set after first SoC reading in read_soc_only mode
    uint8_t soc_shutdown_retries = 0;   // Count of ChargeParameterDiscoveryReq received after SoC was already read
    bool dc_soc_done = false;           // Set after DC SoC session completes (for charge_via_iso15118 + read_soc)
    bool current_session_is_dc = false; // Tracks whether the current session uses DC (set in ChargeParameterDiscovery)

    // PnC session state (-2 contract payment, PnC only offered over TLS)
    static constexpr size_t ISO2_CERT_MAX = 800;
    bool pnc_offered = false;
    bool cert_service_offered = false;
    bool contract_selected = false;
    bool contract_validated = false;
    uint8_t contract_leaf[ISO2_CERT_MAX];
    uint16_t contract_leaf_len = 0;
    uint8_t gen_challenge[PNC_CHALLENGE_LEN];

    // Plug and Charge, implemented in iso2_pnc.cpp
    void reset_pnc_session();
    void offer_pnc(struct iso2_ServiceDiscoveryResType *res);
    void authorize_pnc(const struct iso2_AuthorizationReqType *req, struct iso2_AuthorizationResType *res);
    void handle_service_detail_req();
    void handle_payment_details_req();
    void handle_certificate_installation_req();
    void handle_certificate_update_req();
    void start_cert_forward(bool update);
    void poll_cert_forward(bool update, int socket, uint8_t attempts);
    void send_cert_forward_result(bool update, bool failed);
    bool verify_authorization_signature(const struct iso2_AuthorizationReqType *req, const struct iso2_MessageHeaderType *header);
    void fill_cert_installation_res_dummy(struct iso2_CertificateInstallationResType *res, iso2_responseCodeType rc);
    void fill_cert_update_res_dummy(struct iso2_CertificateUpdateResType *res, iso2_responseCodeType rc);
};
