/* esp32-firmware
 * Copyright (C) 2024-2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include <FS.h> // FIXME: without this include here there is a problem with the IPADDR_NONE define in <lwip/ip4_addr.h>

#include <cstdint>
#include <functional>
#include <source_location>
#include <utility>

#include "config.h"
#include "generated/ship_connection_state.enum.h"
#include "tools/allocator.h"
#include "tools/malloc.h"
#include "tools/tf_websocket_client.h"

#include "esp_transport.h"

// Values and Timeouts as defined by SHIP specification
#define SHIP_CONNECTION_CMI_TIMEOUT 30_s // SHIP 13.4.3 Timneout procedure
#define SHIP_CONNECTION_SME_INIT_TIMEOUT 60_s
#define SHIP_CONNECTION_SME_T_hello_prolong_thr_inc 30_s
#define SHIP_CONNECTION_SME_T_hello_prolong_waiting_gap 15_s
#define SHIP_CONNECTION_PROTOCOL_HANDSHAKE_TIMEOUT 10_s
#define SHIP_CONNECTION_TRUST_CHECK_INTERVAL 10_s // Interval to re-check trust status while in pending state

// Buffers
#define SHIP_CONNECTION_MAX_JSON_SIZE (8192 * 2)
#define SHIP_CONNECTION_MAX_BUFFER_SIZE SHIP_CONNECTION_MAX_JSON_SIZE

// Client Timeouts. These are only needed for when we are and using websockets as a client
#define SHIP_CONNECTION_WS_TIMEOUT_MS 10000

// Stack size of the websocket client task (client role connections).
// Measured peak usage is ~3.95 KiB during the TLS handshake.
#define SHIP_CONNECTION_WS_CLIENT_TASK_STACK 5120

enum class NodeState : uint8_t;
class SpineConnection;
struct ShipNode;

class ShipConnection
{
public:
    // SHIP 13.4.1
    enum class Role : uint8_t {
        Client,
        Server,
    };

    enum class ProtocolState : uint16_t {
        ConnectionHello,
        MessageProtocolHandshake,
        ConnectionPinState,
        AccessMethodsRequest,
        AccessMethods,
        Data,
        Terminate,
        None,
        Unknown,
    };

    struct CMIMessage {
        bool valid;
        uint8_t type;
        uint8_t value;
    };

    // With SHIP/EEBus we only ever hold at most one incoming and one outgoing message,
    // each in a fixed-size buffer allocated in PSRAM.
    struct Message {
        uint8_t data[SHIP_CONNECTION_MAX_BUFFER_SIZE];
        size_t length = 0;
        size_t multipart_index = SIZE_MAX; // SIZE_MAX: no message being assembled
    };

    unique_ptr_any<Message> message_incoming;
    unique_ptr_any<Message> message_outgoing;

    BasicJsonDocument<ArduinoJsonPsramAllocator> incoming_json_doc{SHIP_CONNECTION_MAX_JSON_SIZE};
    BasicJsonDocument<ArduinoJsonPsramAllocator> outgoing_json_doc{SHIP_CONNECTION_MAX_JSON_SIZE};

    int ws_fd = -1;                                 // Server role: fd of the incoming websocket connection
    tf_websocket_client_handle_t ws_server = nullptr;
    esp_transport_handle_t ext_transport = nullptr; ///< Owned external transport, destroyed on close
    Role role;
    ShipNode *peer_node; // Non-owning, owned by ShipPeerHandler (see there for lifetime rules)
    unique_ptr_any<SpineConnection> spine; // Shall only be initialized after SHIP connection has been established
    bool connection_established = false;
    bool closing_scheduled = false;

    /**
     * New ShipConnection where we act as a Server
     * @param ws_fd The socket fd of the incoming websocket connection
     * @param node The ShipNode representing the peer we are connected to
     */
    ShipConnection(int ws_fd, ShipNode *node);
    /**
     * New ShipConnection where we act as a Client
     * @param ws_config The WebSocket server handle to connect to
     * @param node The ShipNode representing the peer we are connecting to
     */
    ShipConnection(tf_websocket_client_config_t ws_config, ShipNode *node);

    ~ShipConnection();

    /// Start the WebSocket client task. Must be called AFTER this ShipConnection
    /// has been added to ship_connections so that event callbacks can find it.
    void start_client();
    // Confirm to the ship Connection that the client has started successfully and it may now begin with CMI
    void start_client_confirm();

    ShipConnection(const ShipConnection &other) = delete;
    const ShipConnection &operator=(const ShipConnection &other) = delete;

    ShipConnectionState state = ShipConnectionState::CmiInitStart;
    ShipConnectionState previous_state = ShipConnectionState::CmiInitStart;
    uint64_t timeout_task = 0;
    uint64_t state_machine_task = 0;

    void reschedule_task(uint64_t &task_id, std::function<void()> &&fn, millis_t delay_ms, const std::source_location &src_location = std::source_location::current());
    void cancel_task(uint64_t &task_id);

    void process_frame(const uint8_t *data, size_t len, bool fin);
    void schedule_close(millis_t delay_ms, const String &reason = "");

    // SHIP 13.4.8: Gracefully terminate the connection by announcing the close
    // to the peer (connectionClose phase=announce, reason=removedConnection),
    // then closing the socket shortly after.
    void initiate_termination(const String &reason);

    void send_raw(const char *payload, size_t payload_len, const char *what);

    void send_cmi_message(uint8_t type, uint8_t value);

    void send_current_outgoing_message();

    void send_string(const char *str, int length, int msg_classifier = 1);

    void send_data_message(JsonVariant payload);

    CMIMessage get_cmi_message();

    ProtocolState get_protocol_state();

    // State machine functions
    void set_state(ShipConnectionState state);

    void set_and_schedule_state(ShipConnectionState state);

    void set_and_schedule_state(ShipConnectionState state, millis_t delay_ms);

    void state_machine_next_step();

    void schedule_state_machine_next_step();

    void state_cme_init_start();

    void state_cmi_client_send();

    void state_cmi_client_wait();

    void state_cmi_client_evaluate();

    void state_cmi_server_wait();

    void state_cmi_server_evaluate();

    void state_sme_connection_data_preparation();

    void state_sme_hello();

    void state_sme_hello_ready_init();

    void state_sme_hello_ready_listen();

    void state_sme_hello_ready_timeout();

    void state_sme_hello_pending_init();

    void state_sme_hello_pending_listen();

    void state_sme_hello_pending_timeout();

    void state_sme_hello_ok();

    void state_sme_hello_abort();

    void state_sme_hello_abort_done();

    void state_sme_hello_remote_abort_done();

    void state_sme_hello_rejected();

    void state_sme_protocol_handshake_server_init();

    void state_sme_protocol_handshake_client_init();

    void state_sme_protocol_handshake_server_listen_proposal();

    void state_sme_protocol_handshake_server_listen_confirm();

    void state_sme_protocol_handshake_client_listen_choice();

    void state_sme_protocol_handshake_timeout();

    void state_sme_protocol_handshake_client_ok();

    void state_sme_protocol_handshake_server_ok();

    void start_handshake_timeout();

    void state_sme_pin_check_init();

    void state_sme_pin_check_listen();

    void state_sme_pin_check_error();

    void state_sme_pin_check_busy_init();

    void state_sme_pin_check_busy_wait();

    void state_sme_pin_check_ok();

    void state_sme_pin_ask_init();

    void state_sme_pin_ask_process();

    void state_sme_pin_ask_restricted();

    void state_sme_pin_ask_ok();

    void state_sme_access_method_request();

    void update_config_state(NodeState state) const;

    void state_done();

    void state_is_not_implemented();

    //--------------------------------------------------------------------------------
    // Hello Phase Specific stuff
    // Types from EEBus SHIP TS TransferProtocol xsd (v1.0.1) with corresponding conversion functions
    // The json_to_type and type_to_json functions operatore on the message_incoming and message_outgoing variables.
    // json_to_type will read from message_incoming (and may modify it while doing so) and write to the Type
    // type_to_json will read from the Type and write to message_outgoing
    class ConnectionHelloPhase
    {
    public:
        enum Type : uint8_t {
            Pending = 0,
            Ready = 1,
            Aborted = 2,
            Unknown = 255,
        };

        static Type from_str(const char *str)
        {
            if (strcmp(str, "pending") == 0) {
                return Type::Pending;
            } else if (strcmp(str, "ready") == 0) {
                return Type::Ready;
            } else if (strcmp(str, "aborted") == 0) {
                return Type::Aborted;
            } else {
                return Type::Unknown;
            }
        }

        static const char *to_str(Type value)
        {
            switch (value) {
                case Type::Pending:
                    return "pending";
                case Type::Ready:
                    return "ready";
                case Type::Aborted:
                    return "aborted";
                default:
                    return "unknown";
            }
        }
    };

    struct ConnectionHelloType {
        ConnectionHelloPhase::Type phase;
        uint32_t waiting;
        bool waiting_valid;
        bool prolongation_request;
        bool prolongation_request_valid;
    };

    void json_to_type_connection_hello(ConnectionHelloType *connection_hello);

    void type_to_json_connection_hello(ConnectionHelloType *connection_hello);

    uint64_t hello_wait_for_ready_timer = 0;
    uint64_t hello_send_prolongation_request_timer = 0;
    uint64_t hello_send_prolongation_reply_timer = 0;
    uint64_t hello_trust_check_timer = 0; // Timer to periodically re-check trust status while in pending state

    /// @brief Which timer expired. 1=wait_for_ready, 2=prolongation_request, 3=prolongation_reply
    uint8_t hello_timer_expiry = 0;

    // Current Peer Hello Phase
    ConnectionHelloType peer_hello_phase = {};
    // Our Hello Phase
    ConnectionHelloType this_hello_phase = {};

    void hello_send_sme_update();

    void hello_set_wait_for_ready_timer(ShipConnectionState target);

    void hello_decide_prolongation();

    void hello_arm_prolongation_request_timer();

    ConnectionHelloPhase::Type hello_check_trust_status();

    void hello_start_trust_check_timer();

    void notify_trust_changed();

    //--------------------------------------------------------------------------------
    // Protocol Handshake Specific stuff

    // What version is supported by the device. Unless some changes are done this should be 1.0
    uint32_t protocol_handshake_version_major = 1;
    uint32_t protocol_handshake_version_minor = 0;

    uint64_t protocol_handshake_timer = 0;

    uint32_t protocol_handshake_version_selected[2] = {1, 0};

    class ProtocolHandshake
    {
    public:
        enum Type : uint8_t {
            AnnounceMax = 0,
            Select = 1,
            Unknown = 255,
        };

        static Type from_str(const char *str)
        {
            if (strcmp(str, "announceMax") == 0) {
                return Type::AnnounceMax;
            } else if (strcmp(str, "select") == 0) {
                return Type::Select;
            } else {
                return Type::Unknown;
            }
        }

        static const char *to_str(Type value)
        {
            switch (value) {
                case Type::AnnounceMax:
                    return "announceMax";
                case Type::Select:
                    return "select";
                default:
                    return "unknown";
            }
        }
    };

    struct ProtocolHandshakeType {
        // Default to Unknown so an unparseable message is never mistaken
        // for a valid announceMax (= 0).
        ProtocolHandshake::Type handshakeType = ProtocolHandshake::Type::Unknown;
        uint32_t version_major = 0;
        uint32_t version_minor = 0;
        // Hint: We ignore "formats" parameter completly since we only support UTF-8 and UTF-8 ist mandatory to support for the receiver
        //       Changing the character enconding in the middle of the communication also seems like a bad idea...
    };

    // Parse message_incoming as an SME "protocol handshake" message.
    // Returns false if the message is not valid.
    bool json_to_type_handshake_type(ProtocolHandshakeType *handshake_type);

    void type_to_json_handshake_type(ProtocolHandshakeType *handshake_type);

    enum ProtocolAbortReason : uint8_t {
        Timeout = 1,
        UnexpectedMessage = 2,
        SelectionMismatch = 3,
        RFU,
    };

    void sme_protocol_abort_procedure(ProtocolAbortReason reason);

    void common_procedure_enable_data_exchange();

    // Builds the accessMethods reply for an accessMethodsRequest
    void to_json_access_methods_type();

    void log_message(const String &state_prefix, Message *msg);

    // True while a complete message is in message_incoming and has not been
    // processed by the state machine. Checked by states that would otherwise
    // get stuck waiting for an already arrived frame.
    bool incoming_message_pending = false;

    // Returns true for states whose handler reads and processes message_incoming
    static bool is_listening_state(ShipConnectionState s);
};
