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

#include <math.h>

#include "module.h"
#include "config.h"

#include "lwip/sockets.h"

#include "sdp.h"
#include "cbv2g/app_handshake/appHand_Decoder.h"
#include "cbv2g/app_handshake/appHand_Encoder.h"

#include "isotls.h"

#include "generated/common_state.enum.h"
#include "generated/encryption.enum.h"
#include "generated/evcc_vendor.enum.h"

#define SESSION_ID_LENGTH 8
#define EXI_DATA_SIZE (10*1024) // TODO: How much do we need here?

// Convert value * 10^exponent to float
float physical_value_to_float(int16_t value, int8_t exponent);

// Result of session ID check in SessionSetupReq handling
enum class SessionIdResult {
    NewSession,
    ResumeSession
};

// Check received session ID against stored session ID (SessionSetupReq handling).
// Received ID all zeros or mismatch: generates a new random stored ID, returns NewSession.
// Received ID matches stored ID: returns ResumeSession.
SessionIdResult check_session_id(const uint8_t *received_id, size_t received_len, uint8_t *stored_id, size_t stored_len);

// Check that a message after SessionSetup belongs to the established session.
// Returns true if the received session ID matches the stored one.
bool validate_session_id(const uint8_t *received_id, size_t received_len, const uint8_t *stored_id, size_t stored_len);

// Cancel an existing sequence timeout if active.
// Sets next_timeout to 0 after cancellation.
void cancel_sequence_timeout(uint64_t &next_timeout);

// Schedule a sequence timeout that resets the SLAC state machine on expiry.
// protocol_name: Used for logging (e.g., "ISO2", "ISO20", "ISO20 AC")
// timeout: The timeout duration (e.g., 60_s)
// next_timeout: Reference to store the scheduled task ID (set to 0 on expiry)
void schedule_sequence_timeout(uint64_t &next_timeout, millis_t timeout, const char *protocol_name);

// Common message dispatch macros for all V2G protocols (DIN, ISO2, ISO20).
// doc_path: Path to the _isUsed flags (e.g. body, doc)
// msg:      Message name without suffix (e.g. SessionSetupReq)

// Call handler if the message is present in the decoded document.
#define V2G_DISPATCH(doc_path, msg, handler) \
    if (doc_path.msg##_isUsed) { handler(); }

// Trace an unimplemented message.
// prefix: Protocol name for logging (e.g. "ISO2", "DIN70121")
#define V2G_NOT_IMPL(prefix, doc_path, msg) \
    if (doc_path.msg##_isUsed) { \
        iso15118.trace(prefix ": " #msg " received but not implemented"); \
    }

class Common final
{
public:
    Common(){}
    void setup_socket();
    void close_socket();
    void handle_socket();  // Called by central poll when socket has data
    void pre_setup();

    ISOTLS tls;

    ConfigRoot api_state;
    Config supported_protocols_prototype;

    uint8_t session_id[SESSION_ID_LENGTH];
    enum class ExiType : uint8_t {
        AppHand,
        Din,
        Iso2,
        Iso20,
        Iso20Ac  // ISO 15118-20 AC-specific messages
    };
    ExiType exi_in_use = ExiType::AppHand;

    void send_exi(ExiType type);
    void send_exi_raw(const uint8_t *exi, size_t exi_len, ExiType type);
    const uint8_t *get_rx_exi(size_t *len_out) const { *len_out = rx_exi_len; return rx_exi; }
    void prepare_din_header(struct din_MessageHeaderType *header);
    void prepare_iso2_header(struct iso2_MessageHeaderType *header);
    void reset_active_socket();
    int get_active_socket() const { return active_socket; }
    int get_listen_socket() const { return listen_socket; }

    bool tls_requested_by_ev = false;   // EV requested TLS in SDP

    // EVCC vendor detection
    void detect_evcc_vendor_from_mac(const uint8_t *mac);
    void detect_evcc_vendor_from_protocol(const char *protocol_namespace, size_t len);
    void set_evcc_vendor(EVCCVendor vendor);
    void reset_evcc_vendor() { set_evcc_vendor(EVCCVendor::Unknown); }
    EVCCVendor get_evcc_vendor() const { return api_state.get("evcc_vendor")->asEnum<EVCCVendor>(); }

private:
    void handle_session_setup_req();
    void handle_supported_app_protocol_req();

    void decode(uint8_t *data, const size_t length);

    struct appHand_exiDocument *appHandDec;
    struct appHand_exiDocument *appHandEnc;

    int listen_socket = -1;
    int active_socket = -1;
    struct sockaddr_storage source_addr;
    socklen_t addr_len = sizeof(source_addr);

    uint8_t *exi_data = nullptr;
    const uint8_t *rx_exi = nullptr;
    size_t rx_exi_len = 0;

    CommonState state = CommonState::Idle;
};


// Encode a milliwatt value into (Value, Exponent) form for EXI physical values.
// Starts at exponent -3 (milliwatts) and divides by 10 until the value fits in int16_t,
// maximizing resolution. Used for ISO2 PhysicalValueType and ISO20 RationalNumberType.
struct ScaledPower {
    int16_t value;
    int8_t exponent;
};
static inline ScaledPower encode_milliwatts(uint32_t milliwatts)
{
    int8_t exponent = -3;
    while (milliwatts > INT16_MAX) {
        milliwatts /= 10;
        exponent++;
    }
    return {static_cast<int16_t>(milliwatts), exponent};
}

// ServiceID for EV Charging (DIN Table 105, ISO2 Table 105, ISO20 Table 203)
static constexpr uint16_t V2G_SERVICE_ID_CHARGING = 1;

// ServiceID for the ContractCertificate service (ISO2 Table 105)
static constexpr uint16_t V2G_SERVICE_ID_CERTIFICATE = 2;

// V2G_SECC_Sequence_Timeout = 60s (DIN TS 70121:2024-11 Table 76, ISO 15118-2 Table 109)
static constexpr auto V2G_SECC_SEQUENCE_TIMEOUT = 60_s;

// SAScheduleTupleID: We always only offer one schedule tuple (DIN/ISO2)
static constexpr uint16_t V2G_SA_SCHEDULE_TUPLE_ID = 1;
static constexpr uint16_t V2G_NOMINAL_VOLTAGE_V    = 230;
static constexpr uint8_t  V2G_NOMINAL_FREQUENCY_HZ = 50;
static constexpr uint32_t SECONDS_PER_DAY = 86400;

// These values are advertised to the EV to initiate a DC session
// purely for reading the EV's State of Charge.
static constexpr int16_t DC_SOC_MAX_CURRENT_A   = 500;
static constexpr int16_t DC_SOC_MAX_VOLTAGE_V   = 800;
static constexpr int16_t DC_SOC_PEAK_RIPPLE_A   = 1;
static constexpr int16_t DC_SOC_MAX_POWER_VALUE = 20000;
static constexpr int8_t  DC_SOC_MAX_POWER_EXP   = 1;
