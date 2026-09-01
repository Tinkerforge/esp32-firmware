/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "eebus.h"
#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "ship_types.h"
#include "spine_connection.h"
#include "tools.h"

void ShipConnection::set_state(ShipConnectionState state)
{
    // Ignore same-state transitions, they would spam the log (especially in Done)
    if (state == this->state) {
        return;
    }
    ShipConnectionState old_state = this->state;
    eebus.trace_fmtln(" SHIP State Change %s(%d) -> %s(%d)", get_ship_connection_state_name(old_state), static_cast<std::underlying_type<ShipConnectionState>::type>(old_state), get_ship_connection_state_name(state), static_cast<std::underlying_type<ShipConnectionState>::type>(state));
    this->previous_state = old_state;
    this->state = state;
}

void ShipConnection::set_and_schedule_state(ShipConnectionState state)
{
    set_state(state);
    schedule_state_machine_next_step();
}

void ShipConnection::set_and_schedule_state(ShipConnectionState state, millis_t delay_ms)
{
    if (closing_scheduled) {
        return;
    }
    reschedule_task(
        state_machine_task,
        [this, state]() {
            this->set_and_schedule_state(state);
        },
        delay_ms);
}

void ShipConnection::schedule_state_machine_next_step()
{
    if (closing_scheduled) {
        return;
    }
    reschedule_task(
        state_machine_task,
        [this]() {
            this->state_machine_next_step();
        },
        0_ms);
}

void ShipConnection::state_machine_next_step()
{
    if (closing_scheduled) {
        return;
    }

    const bool was_listening = is_listening_state(state);
    const ShipConnectionState state_at_entry = state;

    if (was_listening && !incoming_message_pending && (state != ShipConnectionState::Done)) {
        return;
    }

    switch (state) {
        case ShipConnectionState::CmiInitStart:
            state_cme_init_start();
            break;
        case ShipConnectionState::CmiClientSend:
            state_cmi_client_send();
            break;
        case ShipConnectionState::CmiClientWait:
            state_cmi_client_wait();
            break;
        case ShipConnectionState::CmiClientEvaluate:
            state_cmi_client_evaluate();
            break;
        case ShipConnectionState::CmiServerWait:
            state_cmi_server_wait();
            break;
        case ShipConnectionState::CmiServerEvaluate:
            state_cmi_server_evaluate();
            break;
        case ShipConnectionState::SmeConnectionDataPreparation:
            state_sme_connection_data_preparation();
            break;
        case ShipConnectionState::SmeHello:
            state_sme_hello();
            break;
        case ShipConnectionState::SmeHelloReadyInit:
            state_sme_hello_ready_init();
            break;
        case ShipConnectionState::SmeHelloReadyListen:
            state_sme_hello_ready_listen();
            break;
        case ShipConnectionState::SmeHelloReadyTimeout:
            state_sme_hello_ready_timeout();
            break;
        case ShipConnectionState::SmeHelloPendingInit:
            state_sme_hello_pending_init();
            break;
        case ShipConnectionState::SmeHelloPendingListen:
            state_sme_hello_pending_listen();
            break;
        case ShipConnectionState::SmeHelloPendingTimeout:
            state_sme_hello_pending_timeout();
            break;
        case ShipConnectionState::SmeHelloOk:
            state_sme_hello_ok();
            break;
        case ShipConnectionState::SmeHelloAbort:
            state_sme_hello_abort();
            break;
        case ShipConnectionState::SmeHelloAbortDone:
            state_sme_hello_abort_done();
            break;
        case ShipConnectionState::SmeHelloRemoteAbortDone:
            state_sme_hello_remote_abort_done();
            break;
        case ShipConnectionState::SmeHelloRejected:
            state_sme_hello_rejected();
            break;
        case ShipConnectionState::SmeProtocolHandshakeServerInit:
            state_sme_protocol_handshake_server_init();
            break;
        case ShipConnectionState::SmeProtocolHandshakeClientInit:
            state_sme_protocol_handshake_client_init();
            break;
        case ShipConnectionState::SmeProtocolHandshakeServerListenProposal:
            state_sme_protocol_handshake_server_listen_proposal();
            break;
        case ShipConnectionState::SmeProtocolHandshakeServerListenConfirm:
            state_sme_protocol_handshake_server_listen_confirm();
            break;
        case ShipConnectionState::SmeProtocolHandshakeClientListenChoice:
            state_sme_protocol_handshake_client_listen_choice();
            break;
        case ShipConnectionState::SmeProtocolHandshakeTimeout:
            state_sme_protocol_handshake_timeout();
            break;
        case ShipConnectionState::SmeProtocolHandshakeClientOk:
            state_sme_protocol_handshake_client_ok();
            break;
        case ShipConnectionState::SmeProtocolHandshakeServerOk:
            state_sme_protocol_handshake_server_ok();
            break;
        case ShipConnectionState::SmePinCheckInit:
            state_sme_pin_check_init();
            break;
        case ShipConnectionState::SmePinCheckListen:
            state_sme_pin_check_listen();
            break;
        case ShipConnectionState::SmePinCheckError:
            state_sme_pin_check_error();
            break;
        case ShipConnectionState::SmePinCheckBusyInit:
            state_sme_pin_check_busy_init();
            break;
        case ShipConnectionState::SmePinCheckBusyWait:
            state_sme_pin_check_busy_wait();
            break;
        case ShipConnectionState::SmePinCheckOk:
            state_sme_pin_check_ok();
            break;
        case ShipConnectionState::SmePinAskInit:
            state_sme_pin_ask_init();
            break;
        case ShipConnectionState::SmePinAskProcess:
            state_sme_pin_ask_process();
            break;
        case ShipConnectionState::SmePinAskRestricted:
            state_sme_pin_ask_restricted();
            break;
        case ShipConnectionState::SmePinAskOk:
            state_sme_pin_ask_ok();
            break;
        case ShipConnectionState::SmeAccessMethodRequest:
            state_sme_access_method_request();
            break;
        case ShipConnectionState::Done:
            state_done();
            break;
        default:
            state_is_not_implemented();
            break;
    }

    if (was_listening) {
        incoming_message_pending = false;
    }

    // A pending message was not processed by the state at entry (a frame arrival and a
    // scheduled step can coalesce into one invocation). Keep stepping while the handlers
    // transition, so wait states (e.g. CmiServerWait) get to process the message.
    if (!was_listening && incoming_message_pending && state != state_at_entry) {
        state_machine_next_step();
    }
}

void ShipConnection::state_cme_init_start()
{
    eebus.trace_fmtln("Starting SHIP Connection Mode Initialisation (CMI) for %s", peer_node->node_name().c_str());
    // SHIP 13.4.3
    switch (role) {
        case Role::Client: {
            set_and_schedule_state(ShipConnectionState::CmiClientSend);
            break;
        }
        case Role::Server: {
            set_state(ShipConnectionState::CmiServerWait);
            reschedule_task(
                timeout_task,
                [this]() {
                    schedule_close(0_ms, "Timeout during connection initialization. No CMI message received.");
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
    set_state(ShipConnectionState::CmiClientWait);
    reschedule_task(
        timeout_task,
        [this]() {
            schedule_close(0_ms, "Timeout during connection initialization. No CMI response received.");
        },
        SHIP_CONNECTION_CMI_TIMEOUT);
}

void ShipConnection::state_cmi_client_wait()
{
    // SHIP 13.4.3 3.1
    cancel_task(timeout_task);

    set_and_schedule_state(ShipConnectionState::CmiClientEvaluate);
}

void ShipConnection::state_cmi_client_evaluate()
{
    auto cmi_message = get_cmi_message();
    if (cmi_message.valid && cmi_message.type == 0 && cmi_message.value == 0) {
        // SHIP 13.4.3 3.2.2
        set_and_schedule_state(ShipConnectionState::SmeConnectionDataPreparation);
    } else {
        // SHIP 13.4.3 3.2.1 and 3.2.3
        schedule_close(0_ms, "CMI negotiation failed. Received invalid message from server.");
    }
}

void ShipConnection::state_cmi_server_wait()
{
    // SHIP 13.4.3 2.1
    cancel_task(timeout_task);

    set_and_schedule_state(ShipConnectionState::CmiServerEvaluate);
}

void ShipConnection::state_cmi_server_evaluate()
{
    // SHIP 13.4.3 2.2
    auto cmi_message = get_cmi_message();
    if (cmi_message.valid && (cmi_message.type == 0) && (cmi_message.value == 0)) {
        // SHIP 13.4.3 2.2.2
        set_and_schedule_state(ShipConnectionState::SmeConnectionDataPreparation);
        send_cmi_message(0, 0);
    } else {
        // SHIP 13.4.3 2.2.1 and 2.2.3: On any invalid CMI message the server
        // sends a CMI message with MessageValue = 0 and closes the connection.
        send_cmi_message(0, 0);
        schedule_close(0_ms, "CMI negotiation failed. Received invalid message from client.");
    }
}

void ShipConnection::state_sme_connection_data_preparation()
{
    // SHIP 13.4.4
    set_and_schedule_state(ShipConnectionState::SmeHello);
}

void ShipConnection::state_sme_hello()
{
    // SHIP 13.4.4.1.2: Check trust status to determine if we're ready or pending
    ConnectionHelloPhase::Type phase = hello_check_trust_status();

    this_hello_phase = {
        .phase = phase,
        .waiting = 0,
        .waiting_valid = false,
        .prolongation_request = phase == ConnectionHelloPhase::Type::Pending, // If we're pending we expect a prolongation request to keep waiting
        .prolongation_request_valid = false,
    };

    // SHIP 13.4.4.1.2
    switch (this_hello_phase.phase) {
        case ConnectionHelloPhase::Type::Pending: {
            set_and_schedule_state(ShipConnectionState::SmeHelloPendingInit);
            break;
        }
        case ConnectionHelloPhase::Type::Ready: {
            set_and_schedule_state(ShipConnectionState::SmeHelloReadyInit);
            break;
        }
        case ConnectionHelloPhase::Type::Aborted: {
            // This should only be reached if the peer is explicitly not to be connected with
            set_and_schedule_state(ShipConnectionState::SmeHelloAbort);
            break;
        }
        case ConnectionHelloPhase::Type::Unknown: {
            set_and_schedule_state(ShipConnectionState::SmeHelloAbort);
            break;
        }
    }
}

void ShipConnection::hello_send_sme_update()
{
    // SHIP 13.4.4.1.3 "Common Procedure for Sending an SME "hello" Update Message:"
    // We always report the maximum waiting time to give the peer sufficient time.
    this_hello_phase.waiting = millis_t{SHIP_CONNECTION_SME_INIT_TIMEOUT}.as<uint64_t>();
    this_hello_phase.waiting_valid = true;
    type_to_json_connection_hello(&this_hello_phase);
    send_current_outgoing_message();
}

void ShipConnection::hello_set_wait_for_ready_timer(ShipConnectionState target)
{
    reschedule_task(
        hello_wait_for_ready_timer,
        [this, target]() {
            hello_timer_expiry = 1;
            set_and_schedule_state(target);
        },
        SHIP_CONNECTION_SME_INIT_TIMEOUT);
}

void ShipConnection::hello_decide_prolongation()
{
    // SHIP 13.4.4.1.3 "Common Procedure to Decide an Incoming Prolongation Request"
    // We always decide to prolong the connection so we just reset the timer
    if (peer_hello_phase.phase == ConnectionHelloPhase::Type::Pending) {
        if (this_hello_phase.phase == ConnectionHelloPhase::Type::Pending) {
            // We are both pending so we just reset the timer
            hello_set_wait_for_ready_timer(ShipConnectionState::SmeHelloPendingTimeout);
        } else {
            // We are ready and the peer is pending so we set the timer to abort
            hello_set_wait_for_ready_timer(ShipConnectionState::SmeHelloReadyTimeout);
        }
    }
    if (this_hello_phase.phase == ConnectionHelloPhase::Type::Pending) {
        eebus.trace_fmtln("Reached hello pending state that should not be reached and is not properly handled!");
    }
}

ShipConnection::ConnectionHelloPhase::Type ShipConnection::hello_check_trust_status()
{
    // SHIP 13.4.4.1.2
    if (peer_node->trusted) {
        eebus.trace_fmtln("Peer %s is trusted, proceeding with ready phase", peer_node->node_name().c_str());
        return ConnectionHelloPhase::Type::Ready;
    } else {
        eebus.trace_fmtln("Peer %s is NOT trusted, entering pending state awaiting approval", peer_node->node_name().c_str());
        logger.printfln("Peer %s is not trusted. Please approve the connection in the UI to proceed.", peer_node->node_name().c_str());
        peer_node->state = NodeState::AwaitingApproval;
        eebus.update_peers_state();
        return ConnectionHelloPhase::Type::Pending;
    }
}

void ShipConnection::hello_start_trust_check_timer()
{
    reschedule_task(
        hello_trust_check_timer,
        [this]() {
            if (peer_node->trusted) {
                eebus.trace_fmtln("Peer %s trust status changed to trusted, transitioning to ready", peer_node->node_name().c_str());
                this_hello_phase.phase = ConnectionHelloPhase::Type::Ready;
                set_and_schedule_state(ShipConnectionState::SmeHelloReadyInit);
            } else {
                hello_start_trust_check_timer();
            }
        },
        SHIP_CONNECTION_TRUST_CHECK_INTERVAL);
}

void ShipConnection::notify_trust_changed()
{
    // Only relevant if we are in the hello pending state waiting for trust approval
    if (state != ShipConnectionState::SmeHelloPendingListen && state != ShipConnectionState::SmeHelloPendingInit) {
        return;
    }

    if (peer_node->trusted) {
        eebus.trace_fmtln("Peer %s trust changed to trusted (external notification), transitioning to ready", peer_node->node_name().c_str());
        cancel_task(hello_trust_check_timer);
        this_hello_phase.phase = ConnectionHelloPhase::Type::Ready;
        set_and_schedule_state(ShipConnectionState::SmeHelloReadyInit);
    }
}

void ShipConnection::state_sme_hello_ready_init()
{
    // 13.4.4.1.3 "Update Message"
    hello_set_wait_for_ready_timer(ShipConnectionState::SmeHelloReadyTimeout);
    cancel_task(hello_send_prolongation_reply_timer);
    cancel_task(hello_send_prolongation_request_timer);
    hello_send_sme_update();
    if (peer_hello_phase.phase == ConnectionHelloPhase::Type::Ready) {
        // Peer is already known to be ready (likely because we were in pending state).
        // Proceed directly to the ok state: there is no new message to process, so
        // stepping into the listen state would only re-parse the stale hello message.
        set_and_schedule_state(ShipConnectionState::SmeHelloOk);
    } else {
        set_state(ShipConnectionState::SmeHelloReadyListen);
    }
}

void ShipConnection::state_sme_hello_ready_listen()
{
    json_to_type_connection_hello(&peer_hello_phase);

    // SHIP 13.4.4.1.3 Sub-state SME_HELLO_STATE_READY_LISTEN
    switch (peer_hello_phase.phase) {
        case ConnectionHelloPhase::Type::Pending: {
            // Peer has not yet trusted us or is waiting for user approval
            if (peer_hello_phase.prolongation_request && peer_hello_phase.prolongation_request_valid) {
                hello_decide_prolongation();
            }
            hello_send_sme_update();
            break;
        }
        case ConnectionHelloPhase::Type::Ready: {
            set_and_schedule_state(ShipConnectionState::SmeHelloOk);
            break;
        }
        case ConnectionHelloPhase::Type::Aborted:
        case ConnectionHelloPhase::Type::Unknown: {
            set_and_schedule_state(ShipConnectionState::SmeHelloAbort);
            break;
        }
    }
}

void ShipConnection::state_sme_hello_ready_timeout()
{
    set_and_schedule_state(ShipConnectionState::SmeHelloAbort);
}

void ShipConnection::state_sme_hello_pending_init()
{
    // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_INIT"
    hello_set_wait_for_ready_timer(ShipConnectionState::SmeHelloPendingTimeout);
    cancel_task(hello_send_prolongation_reply_timer);
    cancel_task(hello_send_prolongation_request_timer);
    // The user may approve the peer via UI at any time
    hello_start_trust_check_timer();
    hello_send_sme_update();
    set_state(ShipConnectionState::SmeHelloPendingListen);
}

void ShipConnection::hello_arm_prolongation_request_timer()
{
    cancel_task(hello_send_prolongation_reply_timer);
    if (peer_hello_phase.waiting >= static_cast<millis_t>(SHIP_CONNECTION_SME_T_hello_prolong_thr_inc).as<uint64_t>()) {
        reschedule_task(
            hello_send_prolongation_request_timer,
            [this]() {
                set_and_schedule_state(ShipConnectionState::SmeHelloPendingTimeout);
                hello_timer_expiry = 2;
            },
            millis_t(peer_hello_phase.waiting) - SHIP_CONNECTION_SME_T_hello_prolong_waiting_gap);
    }
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
                set_and_schedule_state(ShipConnectionState::SmeHelloAbort);
                break;
            }
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 2.
            // Peer is still waiting for us
            cancel_task(hello_wait_for_ready_timer);
            hello_arm_prolongation_request_timer();
            break;
        }
        case ConnectionHelloPhase::Type::Pending: {
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 3.
            if (!peer_hello_phase.prolongation_request_valid && peer_hello_phase.waiting_valid) {
                hello_arm_prolongation_request_timer();
                break;
            }
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 4.
            if (peer_hello_phase.prolongation_request_valid && !peer_hello_phase.waiting_valid) {
                hello_decide_prolongation();
                hello_send_sme_update();
            }
            break;
        }
        case ConnectionHelloPhase::Type::Aborted: {
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 5.
            set_and_schedule_state(ShipConnectionState::SmeHelloAbort);
            break;
        }
        case ConnectionHelloPhase::Type::Unknown: {
            // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_LISTEN" 6.
            set_and_schedule_state(ShipConnectionState::SmeHelloAbort);
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

        uint64_t waiting_time = peer_hello_phase.waiting_valid ? peer_hello_phase.waiting : SHIP_CONNECTION_SME_INIT_TIMEOUT.as<uint64_t>();
        // Using SHIP_CONNECTION_SME_INIT_TIMEOUT here is not 100% to the spec but its close enough

        reschedule_task(
            hello_send_prolongation_reply_timer,
            [this]() {
                hello_timer_expiry = 3;
                set_and_schedule_state(ShipConnectionState::SmeHelloPendingTimeout);
            },
            millis_t(waiting_time));
        set_state(this->previous_state);
    } else {
        // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_TIMEOUT" 1.
        // SHIP 13.4.4.1.3 "Sub-state SME_HELLO_STATE_PENDING_TIMEOUT" 3.
        set_and_schedule_state(ShipConnectionState::SmeHelloAbort);
    }
}

void ShipConnection::state_sme_hello_ok()
{
    cancel_task(hello_wait_for_ready_timer);
    cancel_task(hello_send_prolongation_request_timer);
    cancel_task(hello_send_prolongation_reply_timer);
    cancel_task(hello_trust_check_timer);
    if (role == Role::Client) {
        set_and_schedule_state(ShipConnectionState::SmeProtocolHandshakeClientInit);
    } else {
        set_and_schedule_state(ShipConnectionState::SmeProtocolHandshakeServerInit);
    }
}

void ShipConnection::state_sme_hello_abort()
{
    // SHIP 13.4.4.1.3 Common "abort" procedure
    cancel_task(hello_wait_for_ready_timer);
    cancel_task(hello_send_prolongation_request_timer);
    cancel_task(hello_send_prolongation_reply_timer);
    cancel_task(hello_trust_check_timer);
    ConnectionHelloType abort_msg = {
        .phase = ConnectionHelloPhase::Type::Aborted,
        .waiting = 0,
        .waiting_valid = false,
        .prolongation_request = 0,
        .prolongation_request_valid = false,
    };
    type_to_json_connection_hello(&abort_msg);
    send_current_outgoing_message();
    set_and_schedule_state(ShipConnectionState::SmeHelloAbortDone);
}

void ShipConnection::state_sme_hello_abort_done()
{
    schedule_close(0_ms, "SHIP Hello aborted.");
}

void ShipConnection::state_sme_hello_remote_abort_done()
{
    set_and_schedule_state(ShipConnectionState::SmeHelloAbortDone);
}

void ShipConnection::state_sme_hello_rejected()
{
    set_and_schedule_state(ShipConnectionState::SmeHelloAbort);
}

void ShipConnection::start_handshake_timeout()
{
    reschedule_task(
        protocol_handshake_timer,
        [this]() {
            set_and_schedule_state(ShipConnectionState::SmeProtocolHandshakeTimeout);
        },
        SHIP_CONNECTION_PROTOCOL_HANDSHAKE_TIMEOUT);
}

void ShipConnection::state_sme_protocol_handshake_server_init()
{
    // SHIP 13.4.4.2.3 State SME_PROT_H_STATE_SERVER_INIT
    start_handshake_timeout();

    set_state(ShipConnectionState::SmeProtocolHandshakeServerListenProposal);
}

void ShipConnection::state_sme_protocol_handshake_client_init()
{
    // SHIP 13.4.4.2.3 State SME_PROT_H_STATE_CLIENT_INIT
    ProtocolHandshakeType handshake = {.handshakeType = ProtocolHandshake::Type::AnnounceMax, .version_major = protocol_handshake_version_major, .version_minor = protocol_handshake_version_minor};
    type_to_json_handshake_type(&handshake);
    send_current_outgoing_message();
    set_state(ShipConnectionState::SmeProtocolHandshakeClientListenChoice);
    start_handshake_timeout();
}

void ShipConnection::state_sme_protocol_handshake_server_listen_proposal()
{
    eebus.trace_fmtln("state_sme_protocol_handshake_server_listen_proposal: %d (len %d)-> %s", message_incoming->data[0], message_incoming->length, &message_incoming->data[1]);

    // 13.4.4.2.3 "State SME_PROT_H_STATE_SERVER_LISTEN_PROPOSAL"
    auto handshake = ProtocolHandshakeType();
    if (!json_to_type_handshake_type(&handshake)) {
        // Not a valid SME "protocol handshake" message
        sme_protocol_abort_procedure(ProtocolAbortReason::UnexpectedMessage);
        return;
    }
    switch (handshake.handshakeType) {
        case ProtocolHandshake::Type::AnnounceMax: {
            cancel_task(protocol_handshake_timer);

            protocol_handshake_version_selected[0] = min(protocol_handshake_version_major, handshake.version_major);
            protocol_handshake_version_selected[1] = min(protocol_handshake_version_minor, handshake.version_minor);
            // We always select 1.0 as the protocol version. JSON-UTF8 has to be supported by all participants so its always selected by default.
            ProtocolHandshakeType hst = {.handshakeType = ProtocolHandshake::Type::Select, .version_major = protocol_handshake_version_selected[0], .version_minor = protocol_handshake_version_selected[1]};

            type_to_json_handshake_type(&hst);
            set_state(ShipConnectionState::SmeProtocolHandshakeServerListenConfirm);
            send_current_outgoing_message();
            start_handshake_timeout();
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
    cancel_task(protocol_handshake_timer);
    auto handshake = ProtocolHandshakeType();
    if (!json_to_type_handshake_type(&handshake)) {
        // Not a valid SME "protocol handshake" message
        sme_protocol_abort_procedure(ProtocolAbortReason::UnexpectedMessage);
        return;
    }

    // The received message must be identical to the "select" message we sent in
    // the listen proposal state. (The formats sub-element is not verified: we
    // only ever offer the mandatory JSON-UTF8 format, see ProtocolHandshakeType.)
    if ((handshake.handshakeType == ProtocolHandshake::Type::Select) && (handshake.version_major == protocol_handshake_version_selected[0]) && (handshake.version_minor == protocol_handshake_version_selected[1])) {
        set_state(ShipConnectionState::SmeProtocolHandshakeServerOk);
    } else {
        sme_protocol_abort_procedure(ProtocolAbortReason::SelectionMismatch);
    }
}

void ShipConnection::state_sme_protocol_handshake_client_listen_choice()
{
    auto handshake = ProtocolHandshakeType();
    if (!json_to_type_handshake_type(&handshake)) {
        // Not a valid SME "protocol handshake" message
        sme_protocol_abort_procedure(ProtocolAbortReason::UnexpectedMessage);
        return;
    }
    switch (handshake.handshakeType) {
        case ProtocolHandshake::Type::Select: {
            cancel_task(protocol_handshake_timer);
            if ((handshake.version_major <= protocol_handshake_version_major) && (handshake.version_minor <= protocol_handshake_version_minor)) {
                // TODO: Check format. This is not done yet because the format is not parsed by the json parser
                type_to_json_handshake_type(&handshake);
                send_current_outgoing_message();
                set_and_schedule_state(ShipConnectionState::SmeProtocolHandshakeClientOk);
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
    set_and_schedule_state(ShipConnectionState::SmePinCheckInit);
}

void ShipConnection::state_sme_protocol_handshake_server_ok()
{
    set_and_schedule_state(ShipConnectionState::SmePinCheckInit);
}

void ShipConnection::state_sme_pin_check_init()
{
    // Currently we don't support the PIN verification.
    // So we just report that we don't support it and move on.
    const char *pin_not_suported = "{\"connectionPinState\":[{\"pinState\":\"none\"}]}";
    message_outgoing->data[0] = 1;
    memcpy(&message_outgoing->data[1], pin_not_suported, strlen(pin_not_suported));
    message_outgoing->length = strlen(pin_not_suported) + 1;

    set_state(ShipConnectionState::Done);
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
    eebus.trace_fmtln("state_sme_access_method_request: %d (len %d)-> %s", message_incoming->data[0], message_incoming->length, &message_incoming->data[1]);
    auto protocol_state = get_protocol_state();
    if (protocol_state == ProtocolState::AccessMethodsRequest) {
        to_json_access_methods_type();

        // Invalidate incoming message (otherwise we would end up in a loop, since the done state could interpret it again)
        message_incoming->length = 0;
        set_state(ShipConnectionState::Done);
        send_current_outgoing_message();
    }
}

void ShipConnection::update_config_state(NodeState state) const
{
    task_scheduler.scheduleOnce([this, state]() {
        peer_node->state = state;
        eebus.update_peers_state();
    });
}

void ShipConnection::state_done()
{
    log_message("state_done", message_incoming.get());
    if (!connection_established) {
        task_scheduler.scheduleOnce([this]() {
            // Don't downgrade a peer that is already fully active via another connection
            if (peer_node->state != NodeState::EEBUSActive && peer_node->state != NodeState::EEBUSDegraded) {
                update_config_state(NodeState::Connected);
            }

            // SHIP v1.1.0 12.2.3: Safety net in case a duplicate connection was
            // not already resolved when the TLS connection was established.
            eebus.ship.resolve_duplicate_connections(peer_node);
        });

        // SHIP 12.2.5: After entering the data exchange state (Done),
        // send an accessMethodsRequest before processing any data.
        const char *request = "{\"accessMethodsRequest\":[]}";
        send_string(request, strlen(request));
    }

    connection_established = true;

    // Only process message_incoming if there actually is a new message.
    // state_done can also be invoked without one (e.g. scheduled step on first
    // entry); the buffer then contains a stale, already-consumed message.
    if (!incoming_message_pending) {
        return;
    }

    auto protocol_state = get_protocol_state();

    // protocol_state logged only for non-Data states to reduce noise
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("SHIP: state_done: protocol_state %d", static_cast<int>(protocol_state));
#else
    if (protocol_state != ProtocolState::Data) {
        eebus.trace_fmtln("SHIP: state_done: protocol_state %d", static_cast<int>(protocol_state));
    }
#endif
    if (!spine) {
        spine = make_unique_psram<SpineConnection>(this);
    }
    switch (protocol_state) {
        case ProtocolState::Data: {
            SHIP_TYPES::ShipMessageDataType data = SHIP_TYPES::ShipMessageDataType();

            if (data.json_to_type(&message_incoming->data[1], message_incoming->length - 1) == SHIP_TYPES::DeserializationResult::SUCCESS) {
                task_scheduler.scheduleOnce([this, data = std::move(data)]() {
                    spine->process_datagram(data.payload);
                });
            } else {
                eebus.trace_fmtln("Received a Data Message but encountered an error while trying to deserialize the message");
            }

            break;
        }
        case ProtocolState::MessageProtocolHandshake: {
            set_and_schedule_state(ShipConnectionState::SmeProtocolHandshakeClientInit);
            break;
        }
        case ProtocolState::ConnectionPinState: {
            // SHIP 13.4.4.3: Peer's pin state received. We already sent ours
            // during the initial SmePinCheckInit transition, so just consume
            // this message without replying to avoid an infinite ping-pong loop.
#ifdef EEBUS_TRACE_SUPER_VERBOSE
            eebus.trace_fmtln("SHIP: Received peer pin state in Done state (no reply needed)");
#endif
            break;
        }
        case ProtocolState::AccessMethodsRequest: {
#ifdef EEBUS_TRACE_SUPER_VERBOSE
            eebus.trace_fmtln("AccessMethodsRequest received");
#endif
            SHIP_TYPES::ShipMessageAccessMethods access_methods = SHIP_TYPES::ShipMessageAccessMethods();
            access_methods.id = eebus.get_eebus_name();
            String json = access_methods.type_to_json();
            send_string(json.c_str(), json.length());
            break;
        }
        case ProtocolState::AccessMethods: {
            break;
        }
        case ProtocolState::Terminate: {
            logger.printfln("SHIP Connection Close requested. Closing connection.");

            // SHIP 13.4.8.1.2 rules for DEV-B: Reply to a received close announce
            // with phase "confirm" and no other sub-element set, then close.
            outgoing_json_doc.clear();
            JsonArray connection_close = outgoing_json_doc.createNestedArray("connectionClose");
            connection_close.createNestedObject()["phase"] = "confirm";

            message_outgoing->data[0] = 3; // SHIP message type: end (connection termination)
            const size_t length = serializeJson(outgoing_json_doc, &message_outgoing->data[1], SHIP_CONNECTION_MAX_JSON_SIZE - 1);
            message_outgoing->length = length + 1;
            send_current_outgoing_message();
            schedule_close(0_ms, "SHIP Connection closed by peer request.");
            break;
        }
        default:
            break;
    }
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("SHIP: After state done state: %s", get_ship_connection_state_name(state));
#else
    if (state != ShipConnectionState::Done) {
        eebus.trace_fmtln("SHIP: After state done state: %s", get_ship_connection_state_name(state));
    }
#endif
}

void ShipConnection::state_is_not_implemented()
{
    eebus.trace_fmtln("State %s(%d) was triggered, but is not implemented yet", get_ship_connection_state_name(state), static_cast<std::underlying_type<ShipConnectionState>::type>(state));

    schedule_close(0_ms, "Invalid state reached.");
}

void ShipConnection::sme_protocol_abort_procedure(ProtocolAbortReason reason)
{
    cancel_task(protocol_handshake_timer);

    DynamicJsonDocument json_doc{1024}; // 1kB should be enough for the error message
    JsonArray json_handshake = json_doc.createNestedArray("messageProtocolHandshakeError");

    JsonObject ht = json_handshake.createNestedObject();

    ht["error"] = static_cast<int>(reason);
    message_outgoing->data[0] = 1;
    size_t length = serializeJson(json_doc, &message_outgoing->data[1], SHIP_CONNECTION_MAX_JSON_SIZE - 1);
    message_outgoing->length = length + 1;

    eebus.trace_fmtln("T2J ProtocolHandshakeError json: %s", &message_outgoing->data[1]);
    send_current_outgoing_message();
    schedule_close(0_ms, "SHIP Protocol Handshake aborted. ");
}

void ShipConnection::common_procedure_enable_data_exchange()
{
}

bool ShipConnection::is_listening_state(ShipConnectionState s)
{
    // States that consume the content of message_incoming
    switch (s) {
        case ShipConnectionState::CmiClientEvaluate:
        case ShipConnectionState::CmiServerEvaluate:
        case ShipConnectionState::SmeHelloReadyListen:
        case ShipConnectionState::SmeHelloPendingListen:
        case ShipConnectionState::SmeProtocolHandshakeServerListenProposal:
        case ShipConnectionState::SmeProtocolHandshakeServerListenConfirm:
        case ShipConnectionState::SmeProtocolHandshakeClientListenChoice:
        case ShipConnectionState::SmeAccessMethodRequest:
        case ShipConnectionState::Done:
            return true;
        default:
            return false;
    }
}
