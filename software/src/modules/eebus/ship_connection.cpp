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

#include "ship_connection.h"

#include <esp_https_server.h>
#include <time.h>

#include "build.h"
#include "eebus.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "ship_types.h"
#include "tools.h"

extern EEBus eebus;

void ShipConnection::frame_received(httpd_ws_frame_t *ws_pkt)
{

    // TODO: Does the ws_client implement some kind of keepalive? Like sending ping/pong frames? Otherwise we need to implement something like that here
    if (ws_pkt->fragmented) {
        logger.printfln("ShipConnection ws_frame_received: fragmented %d, final %d", ws_pkt->fragmented, ws_pkt->final);
        logger.printfln("Not yet implemented...");
        return;
    }

    if (ws_pkt->len < 2) {
        logger.printfln("ShipConnection ws_frame_received: payload too short: %d", ws_pkt->len);
        return;
    }
    logger.printfln("ShipConnection ws_frame_received: %x %x", ws_pkt->payload[0], ws_pkt->payload[1]);
    // Copy new message and trigger next SHIP state machine step
    memset(message_incoming->data, 0, SHIP_CONNECTION_MAX_BUFFER_SIZE);
    memcpy(message_incoming->data, ws_pkt->payload, ws_pkt->len);
    message_incoming->length = ws_pkt->len;
    state_machine_next_step();
}

void ShipConnection::schedule_close(const millis_t delay_ms)
{
    task_scheduler.scheduleOnce(
        [this]() {
            logger.printfln("Closing socket");
            // Close socket and
            ws_client.close_HTTPThread();
            // remove this ShipConnection from vector of ShipConnections in Ship
            eebus.ship.remove(*this);
        },
        delay_ms);
}

void ShipConnection::send_cmi_message(uint8_t type, uint8_t value)
{
    char payload[2] = {type, value};
    ws_client.sendOwnedNoFreeBlocking_HTTPThread(payload, 2, HTTPD_WS_TYPE_BINARY);
}

void ShipConnection::send_current_outgoing_message()
{
    if (message_outgoing->length == 0) {
        return;
    }
    if (!message_outgoing) {
        logger.printfln("message_outgoing ist NULL!");
        return;
    }
    if (message_outgoing->length > SHIP_CONNECTION_MAX_BUFFER_SIZE) {
        logger.printfln("message_outgoing->length zu groß!");
        return;
    }
    ws_client.sendOwnedNoFreeBlocking_HTTPThread((char *)message_outgoing->data, message_outgoing->length, HTTPD_WS_TYPE_BINARY);
}

void ShipConnection::send_string(const char *str, const int length)
{
    message_outgoing->data[0] = 1;
    memcpy(&message_outgoing->data[1], str, length);
    message_outgoing->length = length + 1;
    send_current_outgoing_message();
}

ShipConnection::CMIMessage ShipConnection::get_cmi_message()
{
    if (message_incoming->length != 2) {
        return {false, 0, 0};
    }

    return {true, message_incoming->data[0], message_incoming->data[1]};
}

ShipConnection::ProtocolState ShipConnection::get_protocol_state()
{
    if (message_incoming->data[0] == 3) {
        return ProtocolState::Terminate;
    }
    if ((message_incoming->data[0] != 1 && message_incoming->data[0] != 2) || (message_incoming->length < 3)) {
        return ProtocolState::None;
    }

    // TODO: Either search the first '"' or use a proper json parser
    // We assume the json message starts with{""
    const int start_pos = 1 + strlen("{\"");

    if (strncmp((char *)&message_incoming->data[start_pos], "connectionHello", strlen("connectionHello")) == 0) {
        return ProtocolState::ConnectionHello;
    } else if (strncmp((char *)&message_incoming->data[start_pos], "messageProtocolHandshake", strlen("messageProtocolHandshake")) == 0) {
        return ProtocolState::MessageProtocolHandshake;
    } else if (strncmp((char *)&message_incoming->data[start_pos], "connectionPinState", strlen("connectionPinState")) == 0) {
        return ProtocolState::ConnectionPinState;
    } else if (strncmp((char *)&message_incoming->data[start_pos], "accessMethodsRequest", strlen("accessMethodsRequest")) == 0) {
        return ProtocolState::AccessMethodsRequest;
    } else if (strncmp((char *)&message_incoming->data[start_pos], "accessMethods", strlen("accessMethods")) == 0) {
        return ProtocolState::AccessMethods;
    } else if (strncmp((char *)&message_incoming->data[start_pos], "data", strlen("data")) == 0) {
        return ProtocolState::Data;
    }

    return ProtocolState::Unknown;
}

void ShipConnection::set_state(State state)
{
    State old_state = this->state;
    logger.printfln("State Change %s(%d) -> %s(%d)",
                    get_state_name(old_state),
                    static_cast<std::underlying_type<State>::type>(old_state),
                    get_state_name(state),
                    static_cast<std::underlying_type<State>::type>(state));

    this->previous_state = old_state;
    this->state = state;
}

void ShipConnection::set_and_schedule_state(State state)
{
    set_state(state);
    schedule_state_machine_next_step();
}

void ShipConnection::set_and_schedule_state(State state, millis_t delay_ms)
{
    task_scheduler.scheduleOnce(
        [this, state]() {
            this->set_and_schedule_state(state);
        },
        delay_ms);
}

void ShipConnection::schedule_state_machine_next_step()
{
    task_scheduler.scheduleOnce(
        [this]() {
            this->state_machine_next_step();
        },
        0_ms);
}

void ShipConnection::state_machine_next_step()
{
    switch (state) {
        case State::CmiInitStart:
            state_cme_init_start();
            break;
        case State::CmiClientSend:
            state_cmi_client_send();
            break;
        case State::CmiClientWait:
            state_cmi_client_wait();
            break;
        case State::CmiClientEvaluate:
            state_cmi_client_evaluate();
            break;
        case State::CmiServerWait:
            state_cmi_server_wait();
            break;
        case State::CmiServerEvaluate:
            state_cmi_server_evaluate();
            break;
        case State::SmeConnectionDataPreparation:
            state_sme_connection_data_preparation();
            break;
        case State::SmeHello:
            state_sme_hello();
            break;
        case State::SmeHelloReadyInit:
            state_sme_hello_ready_init();
            break;
        case State::SmeHelloReadyListen:
            state_sme_hello_ready_listen();
            break;
        case State::SmeHelloReadyTimeout:
            state_sme_hello_ready_timeout();
            break;
        case State::SmeHelloPendingInit:
            state_sme_hello_pending_init();
            break;
        case State::SmeHelloPendingListen:
            state_sme_hello_pending_listen();
            break;
        case State::SmeHelloPendingTimeout:
            state_sme_hello_pending_timeout();
            break;
        case State::SmeHelloOk:
            state_sme_hello_ok();
            break;
        case State::SmeHelloAbort:
            state_sme_hello_abort();
            break;
        case State::SmeHelloAbortDone:
            state_sme_hello_abort_done();
            break;
        case State::SmeHelloRemoteAbortDone:
            state_sme_hello_remote_abort_done();
            break;
        case State::SmeHelloRejected:
            state_sme_hello_rejected();
            break;
        case State::SmeProtocolHandshakeServerInit:
            state_sme_protocol_handshake_server_init();
            break;
        case State::SmeProtocolHandshakeClientInit:
            state_sme_protocol_handshake_client_init();
            break;
        case State::SmeProtocolHandshakeServerListenProposal:
            state_sme_protocol_handshake_server_listen_proposal();
            break;
        case State::SmeProtocolHandshakeServerListenConfirm:
            state_sme_protocol_handshake_server_listen_confirm();
            break;
        case State::SmeProtocolHandshakeClientListenChoice:
            state_sme_protocol_handshake_client_listen_choice();
            break;
        case State::SmeProtocolHandshakeTimeout:
            state_sme_protocol_handshake_timeout();
            break;
        case State::SmeProtocolHandshakeClientOk:
            state_sme_protocol_handshake_client_ok();
            break;
        case State::SmeProtocolHandshakeServerOk:
            state_sme_protocol_handshake_server_ok();
            break;
        case State::SmePinCheckInit:
            state_sme_pin_check_init();
            break;
        case State::SmePinCheckListen:
            state_sme_pin_check_listen();
            break;
        case State::SmePinCheckError:
            state_sme_pin_check_error();
            break;
        case State::SmePinCheckBusyInit:
            state_sme_pin_check_busy_init();
            break;
        case State::SmePinCheckBusyWait:
            state_sme_pin_check_busy_wait();
            break;
        case State::SmePinCheckOk:
            state_sme_pin_check_ok();
            break;
        case State::SmePinAskInit:
            state_sme_pin_ask_init();
            break;
        case State::SmePinAskProcess:
            state_sme_pin_ask_process();
            break;
        case State::SmePinAskRestricted:
            state_sme_pin_ask_restricted();
            break;
        case State::SmePinAskOk:
            state_sme_pin_ask_ok();
            break;
        case State::SmeAccessMethodRequest:
            state_sme_access_method_request();
            break;
        case State::Done:
            state_done();
            break;
        default:
            state_is_not_implemented();
            break;
    }
    int state_id = eebus.get_state_connection_id_by_ski(peer_ski);
    if (state_id == -1) {
        eebus.state.get("connections")->add()->get("ski")->updateString(peer_ski);
        state_id = eebus.get_state_connection_id_by_ski(peer_ski);
    }
    eebus.state.get("connections")->get(state_id)->get("ship_state")->updateString(get_state_name(state));
}

void ShipConnection::state_cme_init_start()
{
    // SHIP 13.4.3
    switch (role) {
        case Role::Client: {
            set_and_schedule_state(State::CmiClientSend);
            break;
        }

        case Role::Server: {
            set_state(State::CmiServerWait);
            timeout_task = task_scheduler.scheduleOnce(
                [this]() {
                    schedule_close(0_ms);
                },
                SHIP_CONNECTION_CMI_TIMEOUT);

            break;
        }
    }
}

void ShipConnection::state_cmi_client_send()
{
    // SHIP 13.4.3 1.1
    send_cmi_message(0, 0);
    set_state(State::CmiClientWait);
    timeout_task = task_scheduler.scheduleOnce(
        [this]() {
            schedule_close(0_ms);
        },
        SHIP_CONNECTION_CMI_TIMEOUT);
}

void ShipConnection::state_cmi_client_wait()
{
    // SHIP 13.4.3 3.1
    task_scheduler.cancel(timeout_task);
    set_and_schedule_state(State::CmiClientEvaluate);
}

void ShipConnection::state_cmi_client_evaluate()
{
    auto cmi_message = get_cmi_message();
    if (cmi_message.type == 0 && cmi_message.value == 0) {
        // SHIP 13.4.3 3.2.2
        set_and_schedule_state(State::SmeConnectionDataPreparation);
    } else {
        // SHIP 13.4.3 3.2.1 and 3.2.3
        schedule_close(0_ms);
    }
}

void ShipConnection::state_cmi_server_wait()
{
    // SHIP 13.4.3 2.1
    task_scheduler.cancel(timeout_task);
    set_and_schedule_state(State::CmiServerEvaluate);
}

void ShipConnection::state_cmi_server_evaluate()
{
    // SHIP 13.4.3 2.2
    auto cmi_message = get_cmi_message();
    if (cmi_message.valid) {
        if ((cmi_message.type == 0) && (cmi_message.value == 0)) {
            // SHIP 13.4.3 2.2.2
            set_and_schedule_state(State::SmeConnectionDataPreparation);
            send_cmi_message(0, 0);
        } else {
            // SHIP 13.4.3 2.2.1 and 2.2.3
            send_cmi_message(0, 0);
            schedule_close(0_ms);
        }
    }
}

void ShipConnection::state_sme_connection_data_preparation()
{
    // SHIP 13.4.4
    set_and_schedule_state(State::SmeHello);
}

void ShipConnection::state_sme_hello()
{
    // Initialize
    this_hello_phase = {
        .phase = ConnectionHelloPhase::Type::Ready, // TODO: Figure out if we are ready
        .waiting = 0,
        .waiting_valid = false,
        .prolongation_request = 0,
        .prolongation_request_valid = false,
    };

    // SHIP 13.4.4.1.2
    // Depending on if we trust the peer we either go pending, ready or abort
    switch (this_hello_phase.phase) {
        case ConnectionHelloPhase::Type::Pending: {
            set_and_schedule_state(State::SmeHelloPendingInit);
            break;
        }
        case ConnectionHelloPhase::Type::Ready: {
            // If we are ready we tell the peer that we are ready
            set_and_schedule_state(State::SmeHelloReadyInit);
            break;
        }
        case ConnectionHelloPhase::Type::Aborted: {
            // This should only be reached if the peer is explicitly not to be connected with
            set_and_schedule_state(State::SmeHelloAbort);
            break;
        }
        case ConnectionHelloPhase::Type::Unknown: {
            set_and_schedule_state(State::SmeHelloAbort);
            break;
        }
    }
}

void ShipConnection::hello_send_sme_update()
{
    // SHIP 13.4.4.1.3 "Common Procedure for Sending an SME "hello" Update Message:"
    uint64_t timer_end = hello_wait_for_ready_timestamp + millis_t{SHIP_CONNECTION_SME_INIT_TIMEOUT}.as<uint64_t>();
    if (timer_end < millis()) {
        this_hello_phase.waiting = 0;
        this_hello_phase.waiting_valid = true;
    } else {
        this_hello_phase.waiting = millis() - (hello_wait_for_ready_timestamp + millis_t{SHIP_CONNECTION_SME_INIT_TIMEOUT}.as<uint64_t>());
    }
    type_to_json_connection_hello(&this_hello_phase);
    send_current_outgoing_message();
}
void ShipConnection::hello_set_wait_for_ready_timer(State target)
{
    task_scheduler.cancel(hello_wait_for_ready_timer);
    hello_wait_for_ready_timer = task_scheduler.scheduleOnce(
        [this, target]() {
            hello_timer_expiry = 1;
            set_and_schedule_state(target);
        },
        SHIP_CONNECTION_SME_INIT_TIMEOUT);
    hello_wait_for_ready_timestamp = millis();
}

void ShipConnection::hello_decide_prolongation()
{
    // SHIP 13.4.4.1.3 "Common Procedure to Decide an Incoming Prolongation Request"
    // We always decide to prolong the connection so we just reset the timer
    if (peer_hello_phase.phase == ConnectionHelloPhase::Type::Pending) {
        if (this_hello_phase.phase == ConnectionHelloPhase::Type::Pending) {
            // We are both pending so we just reset the timer
            hello_set_wait_for_ready_timer(State::SmeHelloPendingTimeout);
        } else {
            // We are ready and the peer is pending so we set the timer to abort
            hello_set_wait_for_ready_timer(State::SmeHelloReadyTimeout);
        }
    }
    // If we are not ready
    if (this_hello_phase.phase == ConnectionHelloPhase::Type::Pending) {
        //TODO: Decide what to do if we are not ready
    }
}

void ShipConnection::state_sme_hello_ready_init()
{
    // 13.4.4.1.3 "Update Message"
    hello_set_wait_for_ready_timer(State::SmeHelloReadyTimeout);
    task_scheduler.cancel(hello_send_prolongation_reply_timer);
    task_scheduler.cancel(hello_send_prolongation_request_timer);
    hello_send_sme_update();
    set_state(State::SmeHelloReadyListen);
}

void ShipConnection::state_sme_hello_ready_listen()
{

    json_to_type_connection_hello(&peer_hello_phase);

    // SHIP 13.4.4.1.3 Sub-state SME_HELLO_STATE_READY_LISTEN

    switch (peer_hello_phase.phase) {
        case ConnectionHelloPhase::Type::Pending: {
            if (peer_hello_phase.prolongation_request && peer_hello_phase.prolongation_request_valid) {
                // We always accept prolongation requests for an infinite time
                hello_decide_prolongation();
                hello_send_sme_update();
            }
            break;
        }
        case ConnectionHelloPhase::Type::Ready: {
            set_and_schedule_state(State::SmeHelloOk);
            break;
        }
        case ConnectionHelloPhase::Type::Aborted: {
            // Peer wants to abort so we abort too
            set_and_schedule_state(State::SmeHelloAbort);
            break;
        }
        case ConnectionHelloPhase::Type::Unknown: {
            set_and_schedule_state(State::SmeHelloAbort);
            break;
        }
    }
}

void ShipConnection::state_sme_hello_ready_timeout()
{
    set_and_schedule_state(State::SmeHelloAbort);
}

void ShipConnection::state_sme_hello_pending_init()
{
    // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_INIT"
    hello_set_wait_for_ready_timer(State::SmeHelloPendingTimeout);
    task_scheduler.cancel(hello_send_prolongation_reply_timer);
    task_scheduler.cancel(hello_send_prolongation_request_timer);
    hello_send_sme_update();
    set_state(State::SmeHelloPendingListen);
}

void ShipConnection::state_sme_hello_pending_listen()
{
    // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN"
    json_to_type_connection_hello(&peer_hello_phase);
    switch (peer_hello_phase.phase) {
        case ConnectionHelloPhase::Type::Ready: {
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 1.
            if (!peer_hello_phase.waiting_valid) {
                // Peer is not waiting anymore so connection is aborted
                set_and_schedule_state(State::SmeHelloAbort);
                break;
            }
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 2.
            if (peer_hello_phase.waiting_valid) {

                // Peer is still waiting for us
                task_scheduler.cancel(hello_wait_for_ready_timer);
                task_scheduler.cancel(hello_send_prolongation_reply_timer);
                if (peer_hello_phase.waiting >= static_cast<millis_t>(SHIP_CONNECTION_SME_T_hello_prolong_thr_inc).as<uint64_t>()) {
                    hello_send_prolongation_request_timer = task_scheduler.scheduleOnce(
                        [this]() {
                            set_and_schedule_state(State::SmeHelloPendingTimeout);
                            hello_timer_expiry = 2;
                        },
                        millis_t(peer_hello_phase.waiting) - SHIP_CONNECTION_SME_T_hello_prolong_waiting_gap);
                }
                break;
            }
            break;
        }
        case ConnectionHelloPhase::Type::Pending: {
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 3.
            if (!peer_hello_phase.prolongation_request_valid && peer_hello_phase.waiting_valid) {
                task_scheduler.cancel(hello_send_prolongation_reply_timer);
                if (peer_hello_phase.waiting >= static_cast<millis_t>(SHIP_CONNECTION_SME_T_hello_prolong_thr_inc).as<uint64_t>()) {
                    hello_send_prolongation_request_timer = task_scheduler.scheduleOnce(
                        [this]() {
                            set_and_schedule_state(State::SmeHelloPendingTimeout);
                            hello_timer_expiry = 2;
                        },
                        millis_t(peer_hello_phase.waiting) - SHIP_CONNECTION_SME_T_hello_prolong_waiting_gap);
                }
                break;
            }
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 4.
            if (peer_hello_phase.prolongation_request_valid && !peer_hello_phase.waiting_valid) {
                hello_decide_prolongation();
                hello_send_sme_update();
                break;
            }
            break;
        }
        case ConnectionHelloPhase::Type::Aborted: {
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 5.
            set_and_schedule_state(State::SmeHelloAbort);
            break;
        }
        case ConnectionHelloPhase::Type::Unknown: {
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 6.
            set_and_schedule_state(State::SmeHelloAbort);
            break;
        }
    }
}

void ShipConnection::state_sme_hello_pending_timeout()
{
    // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_TIMEOUT" 2.
    if (hello_timer_expiry == 2) {

        this_hello_phase.phase = ConnectionHelloPhase::Type::Pending;
        this_hello_phase.waiting_valid = false;
        this_hello_phase.prolongation_request = true;
        this_hello_phase.prolongation_request_valid = true;
        hello_send_sme_update();

        uint64_t waiting_time =
            peer_hello_phase.waiting_valid ?
                peer_hello_phase.waiting :
                SHIP_CONNECTION_SME_INIT_TIMEOUT.as<
                    uint64_t>(); // Using SHIP_CONNECTION_SME_INIT_TIMEOUT here is not 100% to the spec but its close enough and saves a bit of calculation time

        hello_send_prolongation_reply_timer = task_scheduler.scheduleOnce(
            [this]() {
                hello_timer_expiry = 3;
                set_and_schedule_state(State::SmeHelloPendingTimeout);
            },
            millis_t(waiting_time));
        set_state(this->previous_state);

    } else {
        // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_TIMEOUT" 1.
        // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_TIMEOUT" 3.
        set_and_schedule_state(State::SmeHelloAbort);
    }
}

void ShipConnection::state_sme_hello_ok()
{
    task_scheduler.cancel(hello_wait_for_ready_timer);
    task_scheduler.cancel(hello_send_prolongation_request_timer);
    task_scheduler.cancel(hello_send_prolongation_reply_timer);
    if (role == Role::Client) {
        set_and_schedule_state(State::SmeProtocolHandshakeClientInit);
    } else {
        set_and_schedule_state(State::SmeProtocolHandshakeServerInit);
    }
}

void ShipConnection::state_sme_hello_abort()
{
    // SHIP 13.4.4.1.3 Common "abort" procedure
    task_scheduler.cancel(hello_wait_for_ready_timer);
    task_scheduler.cancel(hello_send_prolongation_request_timer);
    task_scheduler.cancel(hello_send_prolongation_reply_timer);
    ConnectionHelloType abort_msg = {
        .phase = ConnectionHelloPhase::Type::Aborted,
        .waiting = 0,
        .waiting_valid = false,
        .prolongation_request = 0,
        .prolongation_request_valid = false,
    };
    type_to_json_connection_hello(&abort_msg);
    send_current_outgoing_message();
    set_and_schedule_state(State::SmeHelloAbortDone);
}

void ShipConnection::state_sme_hello_abort_done()
{
    schedule_close(0_ms);
}

void ShipConnection::state_sme_hello_remote_abort_done()
{
    set_and_schedule_state(State::SmeHelloAbortDone);
}

void ShipConnection::state_sme_hello_rejected()
{
    set_and_schedule_state(State::SmeHelloAbort);
}

void ShipConnection::state_sme_protocol_handshake_server_init()
{
    // SHIP 13.4.4.2.3 State SME_PROT_H_STATE_SERVER_INIT
    protocol_handshake_timer = task_scheduler.scheduleOnce(
        [this]() {
            set_and_schedule_state(State::SmeProtocolHandshakeTimeout);
        },
        SHIP_CONNECTION_PROTOCOL_HANDSHAKE_TIMEOUT);

    set_state(State::SmeProtocolHandshakeServerListenProposal);
}

void ShipConnection::state_sme_protocol_handshake_client_init()
{
    // SHIP 13.4.4.2.3 State SME_PROT_H_STATE_CLIENT_INIT
    ProtocolHandshakeType handshake = {.handshakeType = ProtocolHandshake::Type::AnnounceMax,
                                       .version_major = protocol_handshake_version_major,
                                       .version_minor = protocol_handshake_version_minor};
    type_to_json_handshake_type(&handshake);
    send_current_outgoing_message();
    set_state(State::SmeProtocolHandshakeClientListenChoice);
    protocol_handshake_timer = task_scheduler.scheduleOnce(
        [this]() {
            set_and_schedule_state(State::SmeProtocolHandshakeTimeout);
        },
        SHIP_CONNECTION_PROTOCOL_HANDSHAKE_TIMEOUT);
}

void ShipConnection::state_sme_protocol_handshake_server_listen_proposal()
{
    logger.printfln("state_sme_protocol_handshake_server_listen_proposal: %d (len %d)-> %s",
                    message_incoming->data[0],
                    message_incoming->length,
                    &message_incoming->data[1]);

    // 13.4.4.2.3 "State SME_PROT_H_STATE_SERVER_LISTEN_PROPOSAL"
    auto handshake = ProtocolHandshakeType();
    json_to_type_handshake_type(&handshake);
    switch (handshake.handshakeType) {
        case ProtocolHandshake::Type::AnnounceMax: {
            task_scheduler.cancel(protocol_handshake_timer);

            protocol_handshake_version_selected[0] = min(protocol_handshake_version_major, handshake.version_major);
            protocol_handshake_version_selected[1] = min(protocol_handshake_version_minor, handshake.version_minor);
            // We always select 1.0 as the protocol version. JSON-UTF8 has to be supported by all participants so its always selected by default.
            ProtocolHandshakeType hst = {.handshakeType = ProtocolHandshake::Type::Select,
                                         .version_major = protocol_handshake_version_selected[0],
                                         .version_minor = protocol_handshake_version_selected[1]};

            type_to_json_handshake_type(&hst);
            set_state(State::SmeProtocolHandshakeServerListenConfirm);
            send_current_outgoing_message();
            protocol_handshake_timer = task_scheduler.scheduleOnce(
                [this]() {
                    set_and_schedule_state(State::SmeProtocolHandshakeTimeout);
                },
                SHIP_CONNECTION_PROTOCOL_HANDSHAKE_TIMEOUT);
            break;
        }
        case ProtocolHandshake::Type::Select:
        case ProtocolHandshake::Type::Unknown: {
            sme_protocol_abort_procedure(ProtocolAbortReason::UnexpectedMessage);
            break;
        }
    }
}

void ShipConnection::state_sme_protocol_handshake_server_listen_confirm()
{
    // 13.4.4.2.3 "State SME_PROT_H_STATE_SERVER_LISTEN_CONFIRM"
    task_scheduler.cancel(protocol_handshake_timer);
    auto handshake = ProtocolHandshakeType();
    json_to_type_handshake_type(&handshake);

    // Maybe verify format aswell?
    if ((handshake.handshakeType == ProtocolHandshake::Type::Select) && (handshake.version_major == protocol_handshake_version_selected[0])
        && (handshake.version_minor == protocol_handshake_version_selected[1])) {
        set_state(State::SmeProtocolHandshakeServerOk);
    } else if ((handshake.version_major == 1) || (handshake.version_minor == 0)) {
        sme_protocol_abort_procedure(ProtocolAbortReason::SelectionMismatch);
    } else {
        sme_protocol_abort_procedure(ProtocolAbortReason::UnexpectedMessage);
    }
}

void ShipConnection::state_sme_protocol_handshake_client_listen_choice()
{
    auto handshake = ProtocolHandshakeType();
    json_to_type_handshake_type(&handshake);
    switch (handshake.handshakeType) {
        case ProtocolHandshake::Type::Select: {
            task_scheduler.cancel(protocol_handshake_timer);
            if ((handshake.version_major <= protocol_handshake_version_major)
                && (handshake.version_minor <= protocol_handshake_version_minor)) {
                // TODO: Check format. This is not done yet because the format is not parsed by the json parser
                type_to_json_handshake_type(&handshake);
                send_current_outgoing_message();
                set_state(State::SmeProtocolHandshakeClientOk);
            } else {
                sme_protocol_abort_procedure(ProtocolAbortReason::SelectionMismatch);
            }
            break;
        }
        case ProtocolHandshake::Type::AnnounceMax:
        case ProtocolHandshake::Type::Unknown: {
            sme_protocol_abort_procedure(ProtocolAbortReason::UnexpectedMessage);
            break;
        }
    }
}

void ShipConnection::state_sme_protocol_handshake_timeout()
{
    sme_protocol_abort_procedure(ProtocolAbortReason::Timeout);
}

void ShipConnection::state_sme_protocol_handshake_client_ok()
{
    set_and_schedule_state(State::SmePinCheckInit);
}

void ShipConnection::state_sme_protocol_handshake_server_ok()
{
    set_and_schedule_state(State::SmePinCheckInit);
}

void ShipConnection::state_sme_pin_check_init()
{
    // Currently we don't support the PIN verification.
    // So we just report that wie don't support it and move on.
    const char *pin_not_suported = "{\"connectionPinState\":[{\"pinState\":\"none\"}]}";
    message_outgoing->data[0] = 1;
    memcpy(&message_outgoing->data[1], pin_not_suported, strlen(pin_not_suported));
    message_outgoing->length = strlen(pin_not_suported) + 1;

    set_state(State::Done);
    send_current_outgoing_message();
}

// We do not support PIN verification so these will not be implemented
void ShipConnection::state_sme_pin_check_listen()
{
    state_is_not_implemented();
}

void ShipConnection::state_sme_pin_check_error()
{
    state_is_not_implemented();
}

void ShipConnection::state_sme_pin_check_busy_init()
{
    state_is_not_implemented();
}

void ShipConnection::state_sme_pin_check_busy_wait()
{
    state_is_not_implemented();
}

void ShipConnection::state_sme_pin_check_ok()
{
    state_is_not_implemented();
}

void ShipConnection::state_sme_pin_ask_init()
{
    state_is_not_implemented();
}

void ShipConnection::state_sme_pin_ask_process()
{
    state_is_not_implemented();
}

void ShipConnection::state_sme_pin_ask_restricted()
{
    state_is_not_implemented();
}

void ShipConnection::state_sme_pin_ask_ok()
{
    state_is_not_implemented();
}

void ShipConnection::state_sme_access_method_request()
{
    logger.printfln("state_sme_access_method_request: %d (len %d)-> %s",
                    message_incoming->data[0],
                    message_incoming->length,
                    &message_incoming->data[1]);
    auto protocol_state = get_protocol_state();
    if (protocol_state == ProtocolState::AccessMethodsRequest) {
        to_json_access_methods_type();

        // Invalidate incoming message (otherwise we would end up in a loop, since the done state could interpret it again)
        message_incoming->length = 0;
        set_state(State::Done);
        send_current_outgoing_message();
    }
}

void ShipConnection::send_data_message(JsonVariant payload)
{
    // Technically we should only send data messages if the state is done but for some reason the done state is not set correctly
    if (/*state == State::Done*/ true) {
        SHIP_TYPES::ShipMessageDataType data = SHIP_TYPES::ShipMessageDataType();
        // SHIP 14.
        data.protocol_id = "ee1.0"; // We only speak ee1.0
        [[maybe_unused]] auto tmp = data.payload = payload;

        data.type_to_json(*message_outgoing);

        logger.printfln("Data: sending message with total length of %d", message_outgoing->length -1);
        send_current_outgoing_message();
    } else {
        logger.printfln("send_data_message: Connection not in done state. Actual State: %d", (int)state);
    }
}

void ShipConnection::state_done()
{
    logger.printfln("state_done: %d (len %d)-> %s", message_incoming->data[0], message_incoming->length, &message_incoming->data[1]);
    // If we are done we can still get the PIN Request (which we currently don't support)
    // and the SME Access Method Request. In that case we jump to the corresponding state,
    // which will then come back here.
    if (!connection_established) {
        for (int i = 0; i < eebus.config.get("peers")->count(); i++) {
            if (eebus.config.get("peers")->get(i)->get("ski")->asString() == peer_ski) {
                eebus.config.get("peers")->get(i)->get("state")->updateUint((uint32_t)NodeState::Connected);
            }
        }
    }
    connection_established = true;

    auto protocol_state = get_protocol_state();

    logger.printfln("state_done: protocol_state %d", (int)protocol_state);
    switch (protocol_state) {
        case ProtocolState::Data: {
            SHIP_TYPES::ShipMessageDataType data = SHIP_TYPES::ShipMessageDataType();
            if (data.json_to_type(&message_incoming->data[1], message_incoming->length - 1) == SHIP_TYPES::DeserializationResult::SUCCESS) {
                logger.printfln("DATA received: %d (len %d)-> %s",
                                message_incoming->data[0],
                                message_incoming->length,
                                &message_incoming->data[1]);
                bool response = spine.process_datagram(data.payload);
                if (response) {
// logger.printfln("Response datagram: %s", spine.response_doc.as<String>().c_str());
                    send_data_message(spine.response_doc.as<JsonVariant>());
                } else {
                    logger.printfln("No response to data message");
                }
            } else {
                logger.printfln("Error while trying to deserialize data message");
            }
            break;
        }
        case ProtocolState::MessageProtocolHandshake: {
            set_and_schedule_state(State::SmeProtocolHandshakeClientInit);
            break;
        }
        case ProtocolState::ConnectionPinState: {
            set_and_schedule_state(State::SmePinCheckInit);
            break;
        }
        case ProtocolState::AccessMethodsRequest: {
            logger.printfln("AccessMethodsRequest received: %d (len %d)-> %s",
                            message_incoming->data[0],
                            message_incoming->length,
                            &message_incoming->data[1]);
            SHIP_TYPES::ShipMessageAccessMethods access_methods = SHIP_TYPES::ShipMessageAccessMethods();
            access_methods.id = DNS_SD_UUID;
            String json = access_methods.type_to_json();
            // TOD: optimize this so it doesnt need to copy the string
            send_string(json.c_str(), json.length());
            break;
        }
        case ProtocolState::AccessMethods: {

            break;
        }
        case ProtocolState::Terminate: {
            logger.printfln("SHIP Connection Close requested. Closing connection.");
            // TODO: Move all this json stuff into another function/type
            DynamicJsonDocument doc{1024};

            JsonArray connectionClose = doc["connectionClose"].to<JsonArray>();
            connectionClose[0]["phase"] = "announce";
            connectionClose[1]["maxTime"] = 500;
            connectionClose[2]["reason"] = "User close";

            String output;
            serializeJson(doc, output);
            message_outgoing->data[0] = 3;
            memcpy(&message_outgoing->data[1], output.c_str(), output.length());
            message_outgoing->length = output.length() + 1;
            send_current_outgoing_message();
            schedule_close(0_ms);
        }
        default:
            break;
    }
    logger.printfln("After state done state: %s", get_state_name(state));
}

void ShipConnection::state_is_not_implemented()
{
    logger.printfln("State %s(%d) was triggered, but is not implemented yet",
                    get_state_name(state),
                    static_cast<std::underlying_type<State>::type>(state));

    schedule_close(0_ms);
}

const char *ShipConnection::get_state_name(State state)
{
    switch (state) {
        case State::CmiInitStart:
            return "CmiInitStart";
        case State::CmiClientSend:
            return "CmiClientSend";
        case State::CmiClientWait:
            return "CmiClientWait";
        case State::CmiClientEvaluate:
            return "CmiClientEvaluate";
        case State::CmiServerWait:
            return "CmiServerWait";
        case State::CmiServerEvaluate:
            return "CmiServerEvaluate";
        case State::SmeConnectionDataPreparation:
            return "SmeConnectionDataPreparation";
        case State::SmeHello:
            return "SmeHello";
        case State::SmeHelloReadyInit:
            return "SmeHelloReadyInit";
        case State::SmeHelloReadyListen:
            return "SmeHelloReadyListen";
        case State::SmeHelloReadyTimeout:
            return "SmeHelloReadyTimeout";
        case State::SmeHelloPendingInit:
            return "SmeHelloPendingInit";
        case State::SmeHelloPendingListen:
            return "SmeHelloPendingListen";
        case State::SmeHelloPendingTimeout:
            return "SmeHelloPendingTimeout";
        case State::SmeHelloOk:
            return "SmeHelloOk";
        case State::SmeHelloAbort:
            return "SmeHelloAbort";
        case State::SmeHelloAbortDone:
            return "SmeHelloAbortDone";
        case State::SmeHelloRemoteAbortDone:
            return "SmeHelloRemoteAbortDone";
        case State::SmeHelloRejected:
            return "SmeHelloRejected";
        case State::SmeProtocolHandshakeServerInit:
            return "SmeProtocolHandshakeServerInit";
        case State::SmeProtocolHandshakeClientInit:
            return "SmeProtocolHandshakeClientInit";
        case State::SmeProtocolHandshakeServerListenProposal:
            return "SmeProtocolHandshakeServerListenProposal";
        case State::SmeProtocolHandshakeServerListenConfirm:
            return "SmeProtocolHandshakeServerListenConfirm";
        case State::SmeProtocolHandshakeClientListenChoice:
            return "SmeProtocolHandshakeClientListenChoice";
        case State::SmeProtocolHandshakeTimeout:
            return "SmeProtocolHandshakeTimeout";
        case State::SmeProtocolHandshakeClientOk:
            return "SmeProtocolHandshakeClientOk";
        case State::SmeProtocolHandshakeServerOk:
            return "SmeProtocolHandshakeServerOk";
        case State::SmePinCheckInit:
            return "SmePinCheckInit";
        case State::SmePinCheckListen:
            return "SmePinCheckListen";
        case State::SmePinCheckError:
            return "SmePinCheckError";
        case State::SmePinCheckBusyInit:
            return "SmePinCheckBusyInit";
        case State::SmePinCheckBusyWait:
            return "SmePinCheckBusyWait";
        case State::SmePinCheckOk:
            return "SmePinCheckOk";
        case State::SmePinAskInit:
            return "SmePinAskInit";
        case State::SmePinAskProcess:
            return "SmePinAskProcess";
        case State::SmePinAskRestricted:
            return "SmePinAskRestricted";
        case State::SmePinAskOk:
            return "SmePinAskOk";
        case State::SmeAccessMethodRequest:
            return "SmeAccessMethodRequest";
        case State::Done:
            return "Done";
        default:
            return "Unknown";
    }
}

void ShipConnection::json_to_type_connection_hello(ConnectionHelloType *connection_hello)
{
    logger.printfln("J2T ConnectionHello json: %s", &message_incoming->data[1]);

    DynamicJsonDocument json_doc{SHIP_CONNECTION_MAX_JSON_SIZE};
    DeserializationError error = deserializeJson(json_doc, &message_incoming->data[1], message_incoming->length - 1);
    if (error) {
        logger.printfln("Error during JSON deserialization: %s", error.c_str());
    } else {
        // Initialize non-mandatory fields
        connection_hello->waiting = 0;
        connection_hello->waiting_valid = false;
        connection_hello->prolongation_request = false;
        connection_hello->prolongation_request_valid = false;

        // Go through array of objects and parse all fields
        // This will overwrite the optional fields if they are present
        for (JsonObject obj : json_doc["connectionHello"].as<JsonArray>()) {
            if (obj.containsKey("phase")) {
                connection_hello->phase =
                    static_cast<ConnectionHelloPhase::Type>(ConnectionHelloPhase::from_str(obj["phase"].as<String>().c_str()));
            } else if (obj.containsKey("waiting")) {
                connection_hello->waiting = obj["waiting"].as<uint32_t>();
                connection_hello->waiting_valid = true;
            } else if (obj.containsKey("prolongation_request")) {
                connection_hello->prolongation_request = obj["prolongation_request"].as<bool>();
                connection_hello->prolongation_request_valid = true;
            }
        }

        logger.printfln("J2T ConnectionHello Type: phase %d, waiting %ld(%d), prolongation_request %d(%d)'",
                        static_cast<std::underlying_type<ConnectionHelloPhase::Type>::type>(connection_hello->phase),
                        connection_hello->waiting,
                        connection_hello->waiting_valid,
                        connection_hello->prolongation_request,
                        connection_hello->prolongation_request_valid);
    }
}

void ShipConnection::type_to_json_connection_hello(ConnectionHelloType *connection_hello)
{
    DynamicJsonDocument json_doc{SHIP_CONNECTION_MAX_JSON_SIZE};
    JsonArray json_hello = json_doc.createNestedArray("connectionHello");

    JsonObject phase = json_hello.createNestedObject();
    phase["phase"] = ConnectionHelloPhase::to_str(connection_hello->phase);

    if (connection_hello->waiting_valid) {
        JsonObject waiting = json_hello.createNestedObject();
        waiting["waiting"] = connection_hello->waiting;
    }

    if (connection_hello->prolongation_request_valid) {
        JsonObject prolongation_request = json_hello.createNestedObject();
        prolongation_request["prolongation_request"] = connection_hello->prolongation_request;
    }

    message_outgoing->data[0] = 1;
    size_t length = serializeJson(json_doc, &message_outgoing->data[1], SHIP_CONNECTION_MAX_JSON_SIZE - 1);
    message_outgoing->length = length + 1;

    logger.printfln("T2J ConnectionHello json: %s", &message_outgoing->data[1]);
}

void ShipConnection::json_to_type_handshake_type(ProtocolHandshakeType *handshake_type)
{
    logger.printfln("J2T ProtocolHandshakeType json: %s", &message_incoming->data[1]);

    DynamicJsonDocument json_doc{SHIP_CONNECTION_MAX_JSON_SIZE};
    DeserializationError error = deserializeJson(json_doc, &message_incoming->data[1], message_incoming->length - 1);
    if (error) {
        logger.printfln("Error during JSON deserialization: %s", error.c_str());
    } else {
        for (JsonObject obj : json_doc["messageProtocolHandshake"].as<JsonArray>()) {
            if (obj.containsKey("handshakeType")) {
                handshake_type->handshakeType =
                    static_cast<ProtocolHandshake::Type>(ProtocolHandshake::from_str(obj["handshakeType"].as<String>().c_str()));
            } else if (obj.containsKey("version")) {
                for (JsonObject version : obj["version"].as<JsonArray>()) {
                    if (version.containsKey("major")) {
                        handshake_type->version_major = version["major"].as<uint32_t>();
                    } else if (version.containsKey("minor")) {
                        handshake_type->version_minor = version["minor"].as<uint32_t>();
                    }
                }
            }
        }
        logger.printfln("J2T ProtocolHandshakeType Type: handshakeType %d, version %ld.%ld",
                        static_cast<std::underlying_type<ProtocolHandshake::Type>::type>(handshake_type->handshakeType),
                        handshake_type->version_major,
                        handshake_type->version_minor);
    }
}

void ShipConnection::type_to_json_handshake_type(ProtocolHandshakeType *handshake_type)
{
    DynamicJsonDocument json_doc{SHIP_CONNECTION_MAX_JSON_SIZE};
    JsonArray json_handshake = json_doc.createNestedArray("messageProtocolHandshake");

    JsonObject ht = json_handshake.createNestedObject();
    ht["handshakeType"] = ProtocolHandshake::to_str(handshake_type->handshakeType);

    JsonObject v = json_handshake.createNestedObject();
    JsonArray version = v.createNestedArray("version");
    JsonObject major = version.createNestedObject();
    major["major"] = handshake_type->version_major;
    JsonObject minor = version.createNestedObject();
    minor["minor"] = handshake_type->version_minor;

    JsonObject f = json_handshake.createNestedObject();
    JsonArray formats = f.createNestedArray("formats");
    JsonObject format = formats.createNestedObject();
    JsonArray format_list = format.createNestedArray("format");
    format_list.add("JSON-UTF8");

    message_outgoing->data[0] = 1;
    size_t length = serializeJson(json_doc, &message_outgoing->data[1], SHIP_CONNECTION_MAX_JSON_SIZE - 1);
    message_outgoing->length = length + 1;

    logger.printfln("T2J ProtocolHandshakeType json: %s", &message_outgoing->data[1]);
}

void ShipConnection::sme_protocol_abort_procedure(ProtocolAbortReason reason)
{
    task_scheduler.cancel(protocol_handshake_timer);

    DynamicJsonDocument json_doc{SHIP_CONNECTION_MAX_JSON_SIZE};
    JsonArray json_handshake = json_doc.createNestedArray("messageProtocolHandshakeError");

    JsonObject ht = json_handshake.createNestedObject();

    ht["error"] = static_cast<int>(reason);
    message_outgoing->data[0] = 1;
    size_t length = serializeJson(json_doc, &message_outgoing->data[1], SHIP_CONNECTION_MAX_JSON_SIZE - 1);
    message_outgoing->length = length + 1;

    logger.printfln("T2J ProtocolHandshakeError json: %s", &message_outgoing->data[1]);
    send_current_outgoing_message();
    schedule_close(0_ms);
}

void ShipConnection::to_json_access_methods_type()
{
    DynamicJsonDocument json_doc{SHIP_CONNECTION_MAX_JSON_SIZE};
    JsonArray json_am = json_doc.createNestedArray("accessMethods");

    JsonObject access_methods = json_am.createNestedObject();
    access_methods["id"] = "Tinkerforge-WARP3-12345"; // TODO

    json_am.createNestedObject().createNestedArray("dnsSd_mDns");

#if 0
    // This is standard conform, but ship-go does not accept it. remove it for now..
    JsonArray dns = json_am.createNestedObject().createNestedArray("dns");
    JsonObject uri = dns.createNestedObject();
    uri["uri"] = "wss://192.168.0.33:4712/ship/"; // TODO
#endif

    message_outgoing->data[0] = 1;
    size_t length = serializeJson(json_doc, &message_outgoing->data[1], SHIP_CONNECTION_MAX_JSON_SIZE - 1);
    message_outgoing->length = length + 1;

    logger.printfln("2J AccessMethods json: %s", &message_outgoing->data[1]);
}

void ShipConnection::common_procedure_enable_data_exchange()
{
}
