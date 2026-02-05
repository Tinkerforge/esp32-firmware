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

#include "lwip/sockets.h"

#include "sdp.h"
#include "cbv2g/app_handshake/appHand_Decoder.h"
#include "cbv2g/app_handshake/appHand_Encoder.h"

#include "isotls.h"

#define SESSION_ID_LENGTH 4
#define EXI_DATA_SIZE (10*1024) // TODO: How much do we need here?

#define COMMON_MAC_ADDRESS_LENGTH 6
#define COMMON_SEEN_MAC_COUNT 8

// Result of session ID check in SessionSetupReq handling
enum class SessionIdResult {
    NewSession,    // New session: received ID was all zeros or different from stored
    ResumeSession  // Resume session: received ID matches stored session ID
};

// Check received session ID against stored session ID.
// If new session is needed, generates a new random session ID.
// Returns whether this is a new or resumed session.
SessionIdResult check_session_id(const uint8_t *received_id, size_t received_len, uint8_t *stored_id, size_t stored_len);

// Check received session ID matches stored session ID (for messages after SessionSetup).
// Returns true if session ID matches, false if it's an unknown session.
// This is used to validate that subsequent messages belong to the established session.
bool validate_session_id(const uint8_t *received_id, size_t received_len, const uint8_t *stored_id, size_t stored_len);

// Cancel an existing sequence timeout if active.
// Sets next_timeout to 0 after cancellation.
void cancel_sequence_timeout(uint64_t &next_timeout);

// Schedule a sequence timeout that resets the SLAC state machine on expiry.
// protocol_name: Used for logging (e.g., "ISO2", "ISO20", "ISO20 AC")
// timeout: The timeout duration (e.g., 60_s)
// next_timeout: Reference to store the scheduled task ID (set to 0 on expiry)
void schedule_sequence_timeout(uint64_t &next_timeout, millis_t timeout, const char *protocol_name);

// Common message dispatch macros for all V2G protocols (DIN, ISO2, ISO20)
// prefix: Protocol name for logging (e.g., "ISO2", "DIN70121")
// doc_path: Path to the _isUsed flags (e.g., body, doc)
// msg: Message name without suffix (e.g., SessionSetupReq)
// handler: Handler function to call
#define V2G_DISPATCH(prefix, doc_path, msg, handler) \
    if (doc_path.msg##_isUsed) { handler(); }

#define V2G_NOT_IMPL(prefix, doc_path, msg) \
    if (doc_path.msg##_isUsed) { \
        logger.printfln(prefix ": " #msg " received but not implemented"); \
    }

// Macro to send FAILED_UnknownSession error response for DIN/ISO2
// doc_dec: Decoded document path to check _isUsed flags (e.g., body_dec)
// doc_enc: Encoded document path to set response (e.g., body_enc)
// msg: Message name without Req/Res suffix (e.g., ServiceDiscovery)
// response_code: The FAILED_UnknownSession response code type
// Returns true if this message type matched, false otherwise
#define V2G_SEND_FAILED_SESSION(doc_dec, doc_enc, msg, response_code) \
    (doc_dec.msg##Req_isUsed && ( \
        doc_enc.msg##Res_isUsed = 1, \
        doc_enc.msg##Res.ResponseCode = response_code, \
        true))

// Macro to send FAILED_UnknownSession error response for ISO20
// doc_dec: Decoded document to check _isUsed flags (e.g., *iso20DocDec)
// doc_enc: Encoded document pointer to set response (e.g., iso20DocEnc)
// msg: Message name without Req/Res suffix (e.g., ServiceDiscovery)
// response_code: The FAILED_UnknownSession response code type
// prepare_hdr: Function to prepare header (e.g., prepare_header)
// Returns true if this message type matched, false otherwise
#define V2G20_SEND_FAILED_SESSION(doc_dec, doc_enc, msg, response_code, prepare_hdr) \
    (doc_dec.msg##Req_isUsed && ( \
        doc_enc->msg##Res_isUsed = 1, \
        prepare_hdr(&doc_enc->msg##Res.Header), \
        doc_enc->msg##Res.ResponseCode = response_code, \
        true))

class Common final
{
public:
    Common(){}
    void setup_socket();
    void close_socket();
    void handle_socket();  // Called by central poll when socket has data
    void pre_setup();

    void add_seen_mac_address(const uint8_t mac[COMMON_MAC_ADDRESS_LENGTH]);
    void set_soc(int8_t soc);

    // TLS handler
    ISOTLS tls;

    ConfigRoot api_state;
    Config supported_protocols_prototype;
    Config seen_macs_prototype;

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
    void prepare_din_header(struct din_MessageHeaderType *header);
    void prepare_iso2_header(struct iso2_MessageHeaderType *header);
    void reset_active_socket();
    int get_active_socket() const { return active_socket; }
    int get_listen_socket() const { return listen_socket; }

    // TLS state - set by SDP when EV requests TLS
    bool tls_requested_by_ev = false;   // EV requested TLS in SDP

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

    uint8_t state = 0;
};
