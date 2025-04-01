/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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
#include <esp_http_client.h>
#include <ArduinoJson.h>

#include "module.h"
#include "config.h"
#include "modules/ws/web_sockets.h"

// Values and Timeouts as defined by SHIP document
#define SHIP_CONNECTION_CMI_TIMEOUT 30_s // SHIP 13.4.3 Timneout procedure
#define SHIP_CONNECTION_SME_INIT_TIMEOUT 60_s
#define SHIP_CONNECTION_SME_T_hello_prolong_thr_inc 30_s
#define SHIP_CONNECTION_SME_T_hello_prolong_waiting_gap 15_s

#define SHIP_CONNECTION_MAX_JSON_SIZE 8192 // TODO: What is a sane value here?
#define SHIP_CONNECTION_MAX_BUFFER_SIZE (1024*10) // TODO: What is a sane value here?




class ShipConnection
{
public:
    // SHIP 13.4.1
    enum class Role : uint8_t {
        Client,
        Server
    };

    enum class State : uint8_t {
        // CMI = Connection Mode Initialisation
        // SHIP 13.4.3
        CmiInitStart,
        CmiClientSend,
        CmiClientWait,
        CmiClientEvaluate,
        CmiServerWait,
        CmiServerEvaluate,

        // SME = Ship Message Exchange
        // SHIP 13.4.4.1
        SmeConnectionDataPreparation,
        SmeHello,
        SmeHelloReadyInit,
        SmeHelloReadyListen,
        SmeHelloReadyTimeout,
        SmeHelloPendingInit,
        SmeHelloPendingListen,
        SmeHelloPendingTimeout,
        SmeHelloOk,
        SmeHelloAbort,
        SmeHelloAbortDone,
        SmeHelloRemoteAbortDone,
        SmeHelloRejected,

        // SHIP 13.4.4.2
        SmeProtocolHandshakeServerInit,
        SmeProtocolHandshakeClientInit,
        SmeProtocolHandshakeServerListenProposal,
        SmeProtocolHandshakeServerListenConfirm,
        SmeProtocolHandshakeClientListenChoice,
        SmeProtocolHandshakeTimeout,
        SmeProtocolHandshakeClientOk,
        SmeProtocolHandshakeServerOk,

        // SHIP 13.4.4.3
        SmePinCheckInit,
        SmePinCheckListen,
        SmePinCheckError,
        SmePinCheckBusyInit,
        SmePinCheckBusyWait,
        SmePinCheckOk,
        SmePinAskInit,
        SmePinAskProcess,
        SmePinAskRestricted,
        SmePinAskOk,

        // SHIP 13.4.6
        SmeAccessMethodRequest,

        // Marker for end of SHIP handshake state machine
        Done
    };

    // SHIP 13.4.4.1.3
    enum class SubState : uint8_t {
        Init,
        Listen,
        Timeout
    };

    enum class ProtocolState : uint8_t {
        ConnectionHello,
        MessageProtocolHandshake,
        ConnectionPinState,
        AccessMethodsRequest,
        AccessMethods,
        Data,
        None,
        Unknown
    };

    struct CMIMessage {
        bool    valid;
        uint8_t type;
        uint8_t value;
    };

    // With SHIP/EEBus we will alays only have to hold at most one incoming and one outgoing message
    // For now we just use a fixed size buffer for the messages for each connection
    // This could probably be done more dynamic, but with the heap_caps_calloc_prefer we put it
    // in PSRAM (which we have plenty of) and with the uinque_ptr we make sure that it is automatically
    // freed as soon as it is removed from the ShipConnection list in Ship.
    struct Message {
        uint8_t data[SHIP_CONNECTION_MAX_BUFFER_SIZE]; // TODO: Find good size
        size_t length;
    };
    std::unique_ptr<Message, decltype(std::free) *> message_incoming = std::unique_ptr<Message, decltype(std::free) *>((Message*)heap_caps_calloc_prefer(sizeof(Message), sizeof(char), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL), heap_caps_free);
    std::unique_ptr<Message, decltype(std::free) *> message_outgoing = std::unique_ptr<Message, decltype(std::free) *>((Message*)heap_caps_calloc_prefer(sizeof(Message), sizeof(char), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL), heap_caps_free);

    // Set the ws_client, role and start the state machine that will branch into ClientWait or ServerWait depending on the role
    ShipConnection(WebSocketsClient ws_client, Role role) : ws_client(ws_client), role(role) { state_machine_next_step(); }
    WebSocketsClient ws_client;
    Role role;


    State state = State::CmiInitStart;
    State previous_state = State::CmiInitStart;
    SubState sub_state = SubState::Init;
    uint64_t timeout_task = 0;


    // Implement operator== so that we can easily remove ShipConnections from the vector in Ship
    bool operator==(const ShipConnection &rhs) const { return this == &rhs; }

    void frame_received(httpd_ws_frame_t *ws_pkt);
    void schedule_close(const millis_t delay_ms);
    void send_cmi_message(uint8_t type, uint8_t value);
    void send_current_outgoing_message();
    CMIMessage get_cmi_message();
    ProtocolState get_protocol_state();

    // State machine functions
    void set_state(State state);
    void set_and_schedule_state(State state);
    void set_and_schedule_state(State state, millis_t delay_ms);
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
    void state_done();

    void state_is_not_implemented();
    const char *get_state_name(State state);

    // Types from EEBus SHIP TS TransferProtocol xsd (v1.0.1) with corresponding conversion functions
    // The json_to_type and type_to_json functions operatore on the message_incoming and message_outgoing variables.
    // json_to_type will read from message_incoming (and may modify it while doing so) and write to the Type
    // type_to_json will read from the Type and write to message_outgoing
    class ConnectionHelloPhase {
    public:
        enum Type : uint8_t {
            Pending = 0,
            Ready = 1,
            Aborted = 2,
            Unknown = 255
        };

        static Type from_str(const char *str) {
            if(strcmp(str, "pending") == 0) {
                return Type::Pending;
            } else if(strcmp(str, "ready") == 0) {
                return Type::Ready;
            } else if(strcmp(str, "aborted") == 0) {
                return Type::Aborted;
            } else {
                return Type::Unknown;
            }
        }

        static const char *to_str(Type value) {
            switch(value) {
                case Type::Pending: return "pending";
                case Type::Ready:   return "ready";
                case Type::Aborted: return "aborted";
                default:            return "unknown";
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
    uint64_t hello_wait_for_ready_timestamp = 0; // To calculate the waiting part in messages
    uint64_t hello_send_prolongation_request_timer = 0;
    uint64_t hello_send_prolongation_reply_timer = 0;

    /// @brief Which timer expire. 1=wait_for_ready, 2=prolongation_request, 3=prolongation_reply
    uint8_t hello_timer_expiry = 0; 

    // Current Peer Hello Phase
    ConnectionHelloType peer_hello_phase = {};
    // Our Hello Phase
    ConnectionHelloType this_hello_phase = {};

    void hello_send_sme_update();
    void hello_set_wait_for_ready_timer(State target);
    void hello_decide_prolongation();

    class ProtocolHandshake {
    public:
        enum Type : uint8_t {
            AnnounceMax = 0,
            Select = 1,
            Unknown = 255
        };

        static Type from_str(const char *str) {
            if(strcmp(str, "announceMax") == 0) {
                return Type::AnnounceMax;
            } else if(strcmp(str, "select") == 0) {
                return Type::Select;
            } else {
                return Type::Unknown;
            }
        }

        static const char *to_str(Type value) {
            switch(value) {
                case Type::AnnounceMax: return "announceMax";
                case Type::Select:      return "select";
                default:                return "unknown";
            }
        }
    };

    struct ProtocolHandshakeType {
        ProtocolHandshake::Type handshakeType;
        uint32_t version_major;
        uint32_t version_minor;
        // Hint: We ignore "formats" parameter completly since we only support UTF-8 and UTF-8 ist mandatory to support for the receiver
        //       Changing the character enconding in the middle of the communication also seems like a bad idea...
    };
    void json_to_type_handshake_type(ProtocolHandshakeType *handshake_type);
    void type_to_json_handshake_type(ProtocolHandshakeType *handshake_type);

    // We don't intend to use accessMethodsRequest ourself,
    // so we only have to implement a function that
    // generates a proper json for the accessMethods
    // that is returned on accessMethodsRequest
    void to_json_access_methods_type();
};