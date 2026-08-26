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

// Common functions that are shared between DIN-SPEC-70121, ISO-15118-2 and ISO-15118-20

#include "common.h"
#include <esp_random.h>
#include "sdp.h"

#include "bindings/bricklet_evse_v2.h"
#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "build.h"
#include "esp_netif.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"

#include "tools/net.h"
#include "tools/malloc.h"

#include "bindings/bricklet_evse_v2.h"

#include "cbv2g/exi_v2gtp.h"
#include "cbv2g/app_handshake/appHand_Decoder.h"
#include "cbv2g/app_handshake/appHand_Encoder.h"
#include "cbv2g/din/din_msgDefDecoder.h"
#include "cbv2g/din/din_msgDefEncoder.h"
#include "cbv2g/common/exi_bitstream.h"

#include "gcc_warnings.h"

// EVCC vendor detection for vendor-specific behavior.
// Note: EVCC vendor may be an EV manufacturer like Tesla, but it may also be
//       a component supplier like Bosch or Continental.
struct EVCCVendorOUI {
    uint8_t oui[3];
    EVCCVendor vendor;
};

static constexpr EVCCVendorOUI evcc_vendor_ouis[] = {
    {{0x44, 0x1D, 0xB1}, EVCCVendor::AptivService}, // OBC seen in CUPRA e-HYBRID
    {{0x70, 0xB3, 0xD6}, EVCCVendor::Audi},
    {{0x18, 0x4C, 0xAE}, EVCCVendor::Aumovio},      // OBC seen in Renault Megane E-Tech
    {{0x00, 0x01, 0xA9}, EVCCVendor::BMW},
    {{0x10, 0xBD, 0x43}, EVCCVendor::Bosch},        // OBC seen in VW ID.7
    {{0x38, 0x1F, 0x26}, EVCCVendor::Bosch},        // OBC seen in Citroën e-C3
    {{0x48, 0x31, 0x33}, EVCCVendor::Bosch},
    {{0xEC, 0xFA, 0x03}, EVCCVendor::FCA},
    {{0x00, 0x26, 0xB4}, EVCCVendor::Ford},
    {{0x00, 0x76, 0xB6}, EVCCVendor::Ford},
    {{0x9C, 0x36, 0xF8}, EVCCVendor::Hyundai},
    {{0xAC, 0x96, 0x5B}, EVCCVendor::LucidMotors},
    {{0x3C, 0xCE, 0x15}, EVCCVendor::MercedesBenz},
    {{0x8C, 0x14, 0x7D}, EVCCVendor::Nio},
    {{0xF0, 0xF6, 0x9C}, EVCCVendor::Nio},
    {{0x00, 0x04, 0xEF}, EVCCVendor::Polestar},
    {{0x0C, 0x29, 0x8F}, EVCCVendor::Tesla},
    {{0x4C, 0xFC, 0xAA}, EVCCVendor::Tesla},
    {{0x54, 0xF8, 0xF0}, EVCCVendor::Tesla},
    {{0x90, 0xE6, 0x43}, EVCCVendor::Tesla},
    {{0x98, 0xED, 0x5C}, EVCCVendor::Tesla},
    {{0xD4, 0x4F, 0x14}, EVCCVendor::Tesla},
    {{0xDC, 0x44, 0x27}, EVCCVendor::Tesla},
    {{0x00, 0x7D, 0xFA}, EVCCVendor::VolkswagenGroup},
    {{0x00, 0x50, 0xC2}, EVCCVendor::Volvo},
};

void Common::set_evcc_vendor(EVCCVendor vendor)
{
    if (get_evcc_vendor() == vendor) {
        return;
    }

    api_state.get("evcc_vendor")->updateEnum(vendor);
    if (vendor != EVCCVendor::Unknown) {
        iso15118.trace("Common: EVCC vendor %s", get_evcc_vendor_name(vendor));
    }
}

void Common::detect_evcc_vendor_from_mac(const uint8_t *mac)
{
    for (const EVCCVendorOUI &entry : evcc_vendor_ouis) {
        if (memcmp(entry.oui, mac, sizeof(entry.oui)) == 0) {
            set_evcc_vendor(entry.vendor);
            return;
        }
    }
}

void Common::detect_evcc_vendor_from_protocol(const char *protocol_namespace, size_t len)
{
    // Tesla uses "urn:tesla:din:2018:MsgDef" for proprietary protocol namespace.
    if (strnstr(protocol_namespace, "tesla", len) != nullptr) {
        set_evcc_vendor(EVCCVendor::Tesla);
    }
}

void Common::pre_setup()
{
    supported_protocols_prototype = Config::Str("", 0, 32);

    api_state = Config::Object({
        {"state", Config::Enum(CommonState::Idle)},
        {"supported_protocols", Config::Array({}, &supported_protocols_prototype, 0, 4, Config::type_id<Config::ConfString>())},
        {"protocol", Config::Str("", 0, 32)},
        {"encryption", Config::Enum(Encryption::Unencrypted)},
        {"evcc_vendor", Config::Enum(EVCCVendor::Unknown)}
    });

    if (exi_data == nullptr) {
        exi_data = static_cast<uint8_t*>(calloc_psram_or_dram(EXI_DATA_SIZE, sizeof(uint8_t)));
    }
}

void Common::setup_socket()
{
    if (listen_socket >= 0) {
        close(listen_socket);
        listen_socket = -1;
    }

    listen_socket = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if(listen_socket < 0) {
        iso15118.trace("Common: Failed to create socket: %d (errno %d)", listen_socket, errno);
        return;
    }

    if(fcntl(listen_socket, F_SETFL, fcntl(listen_socket, F_GETFL) | O_NONBLOCK) < 0) {
        iso15118.trace("Common: Failed to set non-blocking mode (listen socket)");
        reset_active_socket();
        return;
    }

    int opt = 1;
    setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listen_socket, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));

    struct sockaddr_in6 dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin6_family = AF_INET6;
    dest_addr.sin6_port = htons(V2G_TCP_DATA_PORT);
    dest_addr.sin6_addr = in6addr_any;

    int err = bind(listen_socket, reinterpret_cast<struct sockaddr *>(&dest_addr), sizeof(dest_addr));
    if(err < 0) {
        iso15118.trace("Common: Failed to bind socket: %d (errno %d)", err, errno);
        return;
    }

    err = listen(listen_socket, 1);
    if(err < 0) {
        iso15118.trace("Common: Failed to listen on socket: %d (errno %d)", err, errno);
        return;
    }

    // Register socket in central poll array
    iso15118.set_poll_fd(FDS_LISTEN_INDEX, listen_socket);
}

void Common::close_socket()
{
    reset_active_socket();
    if (listen_socket >= 0) {
        close(listen_socket);
        listen_socket = -1;
    }
    iso15118.set_poll_fd(FDS_LISTEN_INDEX, -1);
}

void Common::handle_socket()
{
    int new_socket = accept(listen_socket, reinterpret_cast<struct sockaddr *>(&source_addr), &addr_len);
    if (new_socket > 0) {
        // We only support one socket connection at a time. If there is a new connection and one is currently open we close the old one.
        // Usually this means the EV has reconnected. There can't be multiple EVs connected at the same time.
        if (active_socket > 0) {
            iso15118.trace("Common: Replacing socket %d with %d", active_socket, new_socket);
        }

        // Preserve tls_requested_by_ev across the reset (it was set by SDP)
        bool tls_requested = tls_requested_by_ev;

        reset_active_socket();
        active_socket = new_socket;

        // Register active socket in central poll array
        iso15118.set_poll_fd(FDS_ACTIVE_INDEX, active_socket);

        tls_requested_by_ev = tls_requested;

        char addr_str[INET6_ADDRSTRLEN];
        tf_ip6addr_ntoa(&source_addr, addr_str, sizeof(addr_str));
        iso15118.trace("Common: Accepted connection from %s", addr_str);

        if(fcntl(active_socket, F_SETFL, fcntl(active_socket, F_GETFL) | O_NONBLOCK) < 0) {
            iso15118.trace("Common: Failed to set non-blocking mode (active socket)");
            reset_active_socket();
            return;
        }

        // If a new socket is opened we expect a new handshake
        exi_in_use = ExiType::AppHand;

        // Check if TLS should be used for this connection
        if (tls_requested_by_ev) {
            iso15118.trace("Common: TLS requested, starting handshake");

            // The certificate source changed, reload before this connection
            if (tls.certs_dirty) {
                tls.certs_dirty = false;
                if (tls.is_initialized()) {
                    iso15118.trace("Common: Certificates changed, reloading TLS setup");
                    tls.cleanup();
                }
            }

            // Setup TLS if not already done
            if (!tls.is_initialized()) {
                if (!tls.setup()) {
                    iso15118.trace("Common: TLS setup failed, closing connection");
                    reset_active_socket();
                    return;
                }
            }

            // Start TLS session for this connection
            if (!tls.start_session(active_socket)) {
                iso15118.trace("Common: Failed to start TLS session, closing connection");
                reset_active_socket();
                return;
            }
        }
    } else if (active_socket > 0) {
        // Handle TLS handshake in progress
        if (tls.get_handshake_state() == TlsHandshakeState::IN_PROGRESS) {
            if (tls.do_handshake()) {
                // Handshake completed, can now receive data
                if (tls.is_tls13_active()) {
                    api_state.get("encryption")->updateEnum(Encryption::TLS13);
                } else {
                    api_state.get("encryption")->updateEnum(Encryption::TLS12);
                }
                iso15118.trace("Common: TLS handshake completed, ready for V2G communication");
            } else if (tls.get_handshake_state() == TlsHandshakeState::FAILED) {
                iso15118.trace("Common: TLS handshake failed, closing connection");
                reset_active_socket();
            }
            // If still IN_PROGRESS, just return and try again next loop
            return;
        }

        // Read data (either TLS or plain TCP)
        ssize_t length;
        if (tls.is_session_active()) {
            length = tls.read(exi_data, EXI_DATA_SIZE);
        } else {
            length = recv(active_socket, exi_data, EXI_DATA_SIZE, 0);
        }

        if(length < 0) {
            if ((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
                // No data available, non-blocking mode
                return;
            } else if(errno == ECONNRESET) {
                iso15118.trace("Common: Connection reset by peer");
                reset_active_socket();
                return;
            }
            iso15118.trace("Common: Failed to receive data: %zd (errno %d [%s])", length, errno, strerror(errno));
            reset_active_socket();
        } else if(length == 0) {
            iso15118.trace("Common: Connection closed");
            reset_active_socket();
        } else {
            decode(exi_data, static_cast<size_t>(length));
        }
    }
}

void Common::reset_active_socket()
{
    tls.end_session();
    tls_requested_by_ev = false;
    api_state.get("encryption")->updateEnum(Encryption::Unencrypted);

    if (active_socket >= 0) {
        close(active_socket);
        active_socket = -1;
    }
    iso15118.set_poll_fd(FDS_ACTIVE_INDEX, -1);
    state = CommonState::Idle;
    exi_in_use = ExiType::AppHand;
    cancel_sequence_timeout(iso15118.iso2.next_timeout);
    cancel_sequence_timeout(iso15118.din70121.next_timeout);
    cancel_sequence_timeout(iso15118.iso20.next_timeout);
}

void Common::prepare_din_header(struct din_MessageHeaderType *header)
{
    header->Notification_isUsed = 0;
    header->Signature_isUsed = 0;
    for (uint8_t i = 0; i < SESSION_ID_LENGTH; i++) {
        header->SessionID.bytes[i] = session_id[i];
    }
    header->SessionID.bytesLen = SESSION_ID_LENGTH;
}

void Common::prepare_iso2_header(struct iso2_MessageHeaderType *header)
{
    header->Notification_isUsed = 0;
    header->Signature_isUsed = 0;
    for (uint8_t i = 0; i < SESSION_ID_LENGTH; i++) {
        header->SessionID.bytes[i] = session_id[i];
    }
    header->SessionID.bytesLen = SESSION_ID_LENGTH;
}

void Common::send_exi(ExiType type)
{
    memset(exi_data, 0, EXI_DATA_SIZE);
    rx_exi = nullptr;
    rx_exi_len = 0;

    exi_bitstream exi;
    exi_bitstream_init(&exi, exi_data, EXI_DATA_SIZE, sizeof(V2GTP_Header), nullptr);

    int ret = -1;
    switch(type) {
        case ExiType::AppHand:
            ret = encode_appHand_exiDocument(&exi, appHandEnc);
            break;
        case ExiType::Din:
            prepare_din_header(&iso15118.din70121.dinDocEnc->V2G_Message.Header);
            ret = encode_din_exiDocument(&exi, iso15118.din70121.dinDocEnc);
            break;
        case ExiType::Iso2:
            prepare_iso2_header(&iso15118.iso2.iso2DocEnc->V2G_Message.Header);
            ret = encode_iso2_exiDocument(&exi, iso15118.iso2.iso2DocEnc);
            break;
        case ExiType::Iso20:
            // Note: ISO 20 uses a flat structure, not V2G_Message.Header like ISO 2
            // The header is prepared in the response handler (e.g., handle_session_setup_req)
            ret = encode_iso20_exiDocument(&exi, iso15118.iso20.iso20DocEnc);
            break;
        case ExiType::Iso20Ac:
            // ISO 20 AC-specific messages (AC_ChargeParameterDiscovery, AC_ChargeLoop, etc.)
            // The header is prepared in the response handler
            ret = encode_iso20_ac_exiDocument(&exi, iso15118.iso20.iso20AcDocEnc);
            break;
        default:
            break;
    }

    if (ret != 0) {
        iso15118.trace("Common: Failed to encode EXI document: %d", ret);
        return;
    }

    const size_t length = exi_bitstream_get_length(&exi);

    V2GTP_Header *header = reinterpret_cast<V2GTP_Header*>(exi_data);
    header->protocol_version         = 0x01;
    header->inverse_protocol_version = 0xFE;

    // Select payload type based on EXI type
    V2GTPPayloadType payload_type = V2GTPPayloadType::SAP;
    if (type == ExiType::Iso20) {
        payload_type = V2GTPPayloadType::ISO20Common;
    } else if (type == ExiType::Iso20Ac) {
        payload_type = V2GTPPayloadType::ISO20AC;
    }
    header->payload_type             = htons(static_cast<uint16_t>(payload_type));
    header->payload_length           = htonl(length);

    const size_t total_length = length + sizeof(V2GTP_Header);
    ssize_t send_ret;

    if (tls.is_session_active()) {
        send_ret = tls.write(exi_data, total_length);
    } else {
        send_ret = send(active_socket, exi_data, total_length, 0);
    }

    if(send_ret < 0) {
        iso15118.trace("Common: Failed to send data: %zd (errno %d)", send_ret, errno);
        reset_active_socket();
        return;
    }
}

// Send an externally produced EXI document (e.g. a CSMS provided
// CertificateInstallationRes) verbatim. We need to add the V2GTP framing.
void Common::send_exi_raw(const uint8_t *exi, size_t exi_len, ExiType type)
{
    if (exi_len + sizeof(V2GTP_Header) > EXI_DATA_SIZE) {
        iso15118.trace("Common: Raw EXI payload too large: %zu", exi_len);
        return;
    }

    rx_exi = nullptr;
    rx_exi_len = 0;
    memmove(exi_data + sizeof(V2GTP_Header), exi, exi_len);

    V2GTP_Header *header = reinterpret_cast<V2GTP_Header*>(exi_data);
    header->protocol_version         = 0x01;
    header->inverse_protocol_version = 0xFE;

    V2GTPPayloadType payload_type = V2GTPPayloadType::SAP;
    if (type == ExiType::Iso20) {
        payload_type = V2GTPPayloadType::ISO20Common;
    } else if (type == ExiType::Iso20Ac) {
        payload_type = V2GTPPayloadType::ISO20AC;
    }
    header->payload_type             = htons(static_cast<uint16_t>(payload_type));
    header->payload_length           = htonl(exi_len);

    const size_t total_length = exi_len + sizeof(V2GTP_Header);
    ssize_t send_ret;

    if (tls.is_session_active()) {
        send_ret = tls.write(exi_data, total_length);
    } else {
        send_ret = send(active_socket, exi_data, total_length, 0);
    }

    if (send_ret < 0) {
        iso15118.trace("Common: Failed to send data: %zd (errno %d)", send_ret, errno);
        reset_active_socket();
        return;
    }
}

void Common::decode(uint8_t *data, const size_t length)
{
    // A short frame would underflow the payload size below (exi_bitstream_init).
    // The header checks that follow might not catch it as exi_data is a shared buffer
    // and the bytes might hold valid data from a previous response we sent.
    if (length < sizeof(V2GTP_Header)) {
        iso15118.trace("Common: Truncated V2GTP header: %zu bytes", length);
        return;
    }

    V2GTP_Header *header = reinterpret_cast<V2GTP_Header*>(data);
    if(header->protocol_version != 0x01) {
        iso15118.trace("Common: Invalid protocol version: %d", header->protocol_version);
        return;
    }
    if(header->inverse_protocol_version != 0xFE) {
        iso15118.trace("Common: Invalid inverse protocol version: %d", header->inverse_protocol_version);
        return;
    }

    const V2GTPPayloadType payload_type = static_cast<V2GTPPayloadType>(ntohs(header->payload_type));
    if (payload_type != V2GTPPayloadType::SAP &&
        payload_type != V2GTPPayloadType::ISO20Common &&
        payload_type != V2GTPPayloadType::ISO20AC) {
        iso15118.trace("Common: Invalid payload type: 0x%04x", static_cast<uint16_t>(payload_type));
        return;
    }

    exi_bitstream exi;
    exi_bitstream_init(&exi, &data[sizeof(V2GTP_Header)], length - sizeof(V2GTP_Header), 0, nullptr);
    rx_exi = &data[sizeof(V2GTP_Header)];
    rx_exi_len = length - sizeof(V2GTP_Header);

    if (exi_in_use == ExiType::AppHand) {
        // Lazy-alloc appHand buffers on first use; they stay allocated for the session lifetime.
        if (appHandDec == nullptr) {
            appHandDec = static_cast<struct appHand_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct appHand_exiDocument)));
        }
        if (appHandEnc == nullptr) {
            appHandEnc = static_cast<struct appHand_exiDocument*>(calloc_psram_or_dram(1, sizeof(struct appHand_exiDocument)));
        }
        memset(appHandDec, 0, sizeof(struct appHand_exiDocument));
        memset(appHandEnc, 0, sizeof(struct appHand_exiDocument));
        int ret = decode_appHand_exiDocument(&exi, appHandDec);
        if (ret != 0) {
            iso15118.trace("Common: Failed to decode EXI document: %d", ret);
            return;
        }

        if (appHandDec->supportedAppProtocolReq_isUsed) {
            trace_iso("Common: SupportedAppProtocolReq received");
            handle_supported_app_protocol_req();
            appHandDec->supportedAppProtocolReq_isUsed = 0;
        }
    } else if (exi_in_use == ExiType::Din) {
        iso15118.din70121.handle_bitstream(&exi);
    } else if (exi_in_use == ExiType::Iso2) {
        iso15118.iso2.handle_bitstream(&exi);
    } else if (exi_in_use == ExiType::Iso20) {
        iso15118.iso20.handle_bitstream(&exi, payload_type);
    }

    api_state.get("state")->updateEnum(state);
}

void Common::handle_supported_app_protocol_req()
{
    struct appHand_supportedAppProtocolReq *req = &appHandDec->supportedAppProtocolReq;
    struct appHand_supportedAppProtocolRes *res = &appHandEnc->supportedAppProtocolRes;

    // V2G communication has started
    iso15118.communication_setup_deadline = 0_us;

    // One EVSEID per session
    iso15118.refresh_evseid();

    // Check all schemas for DIN, ISO2 and ISO20
    iso15118.trace("EV supports %u protocols", req->AppProtocol.arrayLen);
    api_state.get("supported_protocols")->removeAll();

    uint8_t din70121_schema_id = UINT8_MAX;
    uint8_t din70121_index     = 0;
    uint8_t iso2_schema_id     = UINT8_MAX;
    uint8_t iso2_index         = 0;
    uint8_t iso20_schema_id    = UINT8_MAX;
    uint8_t iso20_index        = 0;

    // TODO: Differentiate between iso:15118:2:2010 and iso:15118:2:2013
    //       iso:15118:2:2010 is the same as din:70121?
    for(uint8_t i = 0; i < static_cast<uint8_t>(req->AppProtocol.arrayLen); i++) {
        if (strnstr(req->AppProtocol.array[i].ProtocolNamespace.characters, ":din:70121:", req->AppProtocol.array[i].ProtocolNamespace.charactersLen) != nullptr) {
            din70121_schema_id = req->AppProtocol.array[i].SchemaID;
            din70121_index     = i;
        } else if (strnstr(req->AppProtocol.array[i].ProtocolNamespace.characters, "iso:15118:2:", req->AppProtocol.array[i].ProtocolNamespace.charactersLen) != nullptr) {
            iso2_schema_id = req->AppProtocol.array[i].SchemaID;
            iso2_index     = i;
        } else if(strnstr(req->AppProtocol.array[i].ProtocolNamespace.characters, "iso:15118:-20:AC", req->AppProtocol.array[i].ProtocolNamespace.charactersLen) != nullptr) {
            // We implement ISO 15118-20 version 1 (VersionNumberMajor 1)
            if (req->AppProtocol.array[i].VersionNumberMajor == 1) {
                iso20_schema_id = req->AppProtocol.array[i].SchemaID;
                iso20_index     = i;
            }
        }

        iso15118.trace("%d: %s", req->AppProtocol.array[i].SchemaID, req->AppProtocol.array[i].ProtocolNamespace.characters);
        trace_iso(" found %d: %s", req->AppProtocol.array[i].SchemaID, req->AppProtocol.array[i].ProtocolNamespace.characters);
        api_state.get("supported_protocols")->add()->updateString(req->AppProtocol.array[i].ProtocolNamespace.characters);

        detect_evcc_vendor_from_protocol(req->AppProtocol.array[i].ProtocolNamespace.characters, req->AppProtocol.array[i].ProtocolNamespace.charactersLen);
    }

    // [V2G20-2356] After a TLS 1.2 handshake ISO 15118-20 must not be selected
    const bool tls13_active = tls.is_session_active() && tls.is_tls13_active();
    if (tls.is_session_active() && !tls13_active && iso20_schema_id != UINT8_MAX) {
        iso15118.trace("Not selecting ISO 15118-20 after TLS 1.2 handshake [V2G20-2356]");
        iso20_schema_id = UINT8_MAX;
    }

    // [V2G20-2677] Only full-handshake TLS carries V2G communication.
    // A session resumed via a ticket PSK could only serve VAS, which we do not offer.
    if (tls.is_session_active() && tls.is_resumed_session()) {
        iso15118.trace("No V2G on a resumed TLS session [V2G20-2677]");
        din70121_schema_id = UINT8_MAX;
        iso2_schema_id = UINT8_MAX;
        iso20_schema_id = UINT8_MAX;
    }

    const bool no_protocol_match = (din70121_schema_id == UINT8_MAX) && (iso2_schema_id == UINT8_MAX) && (iso20_schema_id == UINT8_MAX);
    const bool force_nonegotiation = iso15118.nonegotiation_pending ||
                                     (iso15118.is_autocharge_only() && iso15118.opt_nonegotiation_autocharge);

    if (no_protocol_match || force_nonegotiation) {
        if (no_protocol_match) {
            iso15118.trace("EV does not support DIN 70121, ISO 15118-2 or ISO 15118-20:AC");
        } else {
            iso15118.trace("Forcing Failed_NoNegotiation (%s)", iso15118.nonegotiation_pending ? "after SoC" : "autocharge");
        }
        api_state.get("protocol")->updateString("-");

        iso15118.nonegotiation_pending = false;
        iso15118.reslac_guard_deadline = 0_us;

        appHandEnc->supportedAppProtocolRes_isUsed = 1;
        res->ResponseCode = appHand_responseCodeType_Failed_NoNegotiation;
        res->SchemaID_isUsed = 0;
        send_exi(Common::ExiType::AppHand);
        state = CommonState::Idle;
        trace_iso("SupportedAppProtocolRes sent with Failed_NoNegotiation");

        // Offer basic charging per IEC 61851-1 Table A.7 ("digital communication
        // could not be established"). Socket is left open so the poll loop can
        // detect the EV closing TCP, which triggers early modem shutdown.
        iso15118.begin_iec_transition(ISO15118::ModemOff::Delayed);
    } else {
        // Priority after a TLS 1.3 handshake: ISO20 > ISO2 > DIN, the EV
        // negotiated TLS 1.3 for ISO 15118-20. Otherwise (plaintext or
        // TLS 1.2): ISO2 > DIN > ISO20, ISO20 without TLS 1.3 is testing-only.
        uint8_t schema_id = 0;
        uint8_t index     = 0;
        if (tls13_active && iso20_schema_id != UINT8_MAX) {
            schema_id  = iso20_schema_id;
            index      = iso20_index;
            exi_in_use = ExiType::Iso20;
            iso15118.trace("Using ISO 15118-20:AC (TLS 1.3)");
        } else if (iso2_schema_id != UINT8_MAX) {
            schema_id  = iso2_schema_id;
            index      = iso2_index;
            exi_in_use = ExiType::Iso2;
            iso15118.trace("Using ISO 15118-2");
        } else if (din70121_schema_id != UINT8_MAX) {
            schema_id  = din70121_schema_id;
            index      = din70121_index;
            exi_in_use = ExiType::Din;
            iso15118.trace("Using DIN 70121");
        } else {
            schema_id  = iso20_schema_id;
            index      = iso20_index;
            exi_in_use = ExiType::Iso20;
            iso15118.trace("Using ISO 15118-20:AC");
        }
        api_state.get("protocol")->updateString(req->AppProtocol.array[index].ProtocolNamespace.characters);

        appHandEnc->supportedAppProtocolRes_isUsed = 1;
        res->ResponseCode = appHand_responseCodeType_OK_SuccessfulNegotiation;

        res->SchemaID = schema_id;
        res->SchemaID_isUsed = 1;

        send_exi(Common::ExiType::AppHand);
        state = CommonState::ProtocolNegotiated;

        iso15118.set_charging_protocol(TF_EVSE_V2_CHARGING_PROTOCOL_ISO15118, 50);

        trace_iso("SupportedAppProtocolRes sent");
        trace_iso(" use %d: %s", schema_id, req->AppProtocol.array[index].ProtocolNamespace.characters);
    }
}

void cancel_sequence_timeout(uint64_t &next_timeout)
{
    if (next_timeout != 0) {
        task_scheduler.cancel(next_timeout);
        next_timeout = 0;
    }
}

void schedule_sequence_timeout(uint64_t &next_timeout, millis_t timeout, const char *protocol_name)
{
    next_timeout = task_scheduler.scheduleOnce([&next_timeout, protocol_name]() {
        iso15118.qca700x.link_down();
        iso15118.slac.state = SLACState::ModemReset;
        iso15118.trace("%s Timeout: Link down, SLAC reset", protocol_name);
        next_timeout = 0;
    }, timeout);
}

SessionIdResult check_session_id(const uint8_t *received_id, size_t received_len, uint8_t *stored_id, size_t stored_len)
{
    // All-zero received ID means the EV requests a new session
    bool all_zero = true;
    for (size_t i = 0; i < received_len; i++) {
        if (received_id[i] != 0x00) {
            all_zero = false;
            break;
        }
    }

    const bool matches_stored = validate_session_id(received_id, received_len, stored_id, stored_len);

    if (all_zero || !matches_stored) {
        for (size_t i = 0; i < stored_len; i++) {
            stored_id[i] = static_cast<uint8_t>(esp_random());
        }
        return SessionIdResult::NewSession;
    }

    return SessionIdResult::ResumeSession;
}

bool validate_session_id(const uint8_t *received_id, size_t received_len, const uint8_t *stored_id, size_t stored_len)
{
    return (received_len == stored_len) && (memcmp(received_id, stored_id, stored_len) == 0);
}

// Converts a physical value with exponent to float: result = value * 10^exponent
//
// The exponent/multiplier range of -3 to +3 is defined in:
//   - DIN 70121 Table 55: unitMultiplierType is "byte (range: -3 to +3)"
//   - ISO 15118-2 Table 67: unitMultiplierType is "byte (range: -3..+3)"
//   - ISO 15118-20 Table 102: ExponentType is "xs:byte" (no explicit range constraint...)
//
float physical_value_to_float(int16_t value, int8_t exponent)
{
    switch (exponent) {
        case -4: return static_cast<float>(value) / 10000.0f;
        case -3: return static_cast<float>(value) / 1000.0f;
        case -2: return static_cast<float>(value) / 100.0f;
        case -1: return static_cast<float>(value) / 10.0f;
        case  0: return static_cast<float>(value);
        case  1: return static_cast<float>(value) * 10.0f;
        case  2: return static_cast<float>(value) * 100.0f;
        case  3: return static_cast<float>(value) * 1000.0f;
        case  4: return static_cast<float>(value) * 10000.0f;
        default: break;
    }

    // Fallback
    return static_cast<float>(value) * powf(10.0f, static_cast<float>(exponent));
}
