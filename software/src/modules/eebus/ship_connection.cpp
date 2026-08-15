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

#include "ship_connection.h"

#include "eebus.h"
#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "ship_types.h"
#include "spine_connection.h"
#include "tools.h"
#include <utility>

// Out-of-line destructor: unique_ptr_any<SpineConnection> needs complete SpineConnection type
// for DeleterAny.
ShipConnection::~ShipConnection()
{
    cancel_task(state_machine_task);
    cancel_task(timeout_task);
    cancel_task(hello_wait_for_ready_timer);
    cancel_task(hello_send_prolongation_request_timer);
    cancel_task(hello_send_prolongation_reply_timer);
    cancel_task(hello_trust_check_timer);
    cancel_task(protocol_handshake_timer);
}

void ShipConnection::reschedule_task(uint64_t &task_id, std::function<void()> &&fn, const millis_t delay_ms, const std::source_location &src_location)
{
    task_scheduler.cancel(task_id);
    task_id = task_scheduler.scheduleOnce(std::move(fn), delay_ms, src_location);
}

void ShipConnection::cancel_task(uint64_t &task_id)
{
    task_scheduler.cancel(task_id);
    task_id = 0;
}

// Runs on the websocket client task. Only copy data and push events,
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    const tf_websocket_event_data_t *data = static_cast<tf_websocket_event_data_t *>(event_data);
    Ship::RxEvent event;
    event.role = ShipConnection::Role::Client;
    event.ws_handle = data->client;

    switch (event_id) {
        case WEBSOCKET_EVENT_DATA: {
            if (data->data_len == 0) {
                return;
            }
            // Accept both binary and text frames. Text is allowed after CMI
            ws_transport_opcodes_t opcode = (ws_transport_opcodes_t)(data->op_code & ~WS_TRANSPORT_OPCODES_FIN);
            if (opcode == WS_TRANSPORT_OPCODES_CLOSE) {
                uint16_t close_code = 0;
                String close_reason;
                if (data->data_len >= 2) {
                    close_code = ((uint8_t)data->data_ptr[0] << 8) | (uint8_t)data->data_ptr[1];
                    if (data->data_len > 2) {
                        close_reason = String(data->data_ptr + 2, data->data_len - 2);
                    }
                }
                eebus.trace_fmtln("ShipConnection: Received WebSocket close frame from peer. Code: %u, Reason: %s", close_code, close_reason.c_str());
                event.type = Ship::RxEvent::Type::Disconnected;
                event.reason = "Peer closed WebSocket connection";
                break;
            }
            if (opcode != WS_TRANSPORT_OPCODES_BINARY && opcode != WS_TRANSPORT_OPCODES_TEXT) {
                eebus.trace_fmtln("ShipConnection: Received unexpected opcode 0x%02x from peer", data->op_code);
                return;
            }
            event.type = Ship::RxEvent::Type::Frame;
            const bool last_chunk = (data->payload_offset + data->data_len >= data->payload_len);
            event.fin = data->fin && last_chunk;
            event.data_len = data->data_len;
            event.data.reset(static_cast<uint8_t *>(malloc_psram(data->data_len)));
            if (!event.data) {
                eebus.trace_fmtln("websocket_event_handler: PSRAM allocation failed, dropping frame (len %d)", data->data_len);
                return;
            }
            memcpy(event.data.get(), data->data_ptr, data->data_len);
            break;
        }
        case WEBSOCKET_EVENT_DISCONNECTED:
            event.type = Ship::RxEvent::Type::Disconnected;
            event.reason = "WebSocket disconnected";
            break;
        case WEBSOCKET_EVENT_CONNECTED:
            event.type = Ship::RxEvent::Type::ClientConnected;
            break;
        default:
            return; // Ignore other events for now
    }
    eebus.ship.push_rx_event(std::move(event));
}

ShipConnection::ShipConnection(int ws_fd, ShipNode *node) : ws_fd(ws_fd)
{
    role = Role::Server;
    peer_node = node;
    message_incoming = make_unique_psram<Message>();
    message_outgoing = make_unique_psram<Message>();
    eebus.trace_fmtln("New Shipconnection created for peer %s where we act as server", peer_node->node_name().c_str());
    state_machine_next_step();
}

ShipConnection::ShipConnection(const tf_websocket_client_config_t ws_config, ShipNode *node)
{
    peer_node = node;
    role = Role::Client;
    message_incoming = make_unique_psram<Message>();
    message_outgoing = make_unique_psram<Message>();

    ws_server = tf_websocket_client_init(&ws_config);
    eebus.trace_fmtln("New Shipconnection created for peer %s where we act as client", peer_node->node_name().c_str());

    tf_websocket_register_events(ws_server, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)ws_server);
    // NOTE: Do NOT call tf_websocket_client_start() here.
    // The caller must first add this ShipConnection to ship_connections,
    // then call start_client() so that websocket_event_handler can find us.
}

void ShipConnection::start_client()
{
    esp_err_t err = tf_websocket_client_start(ws_server);
    if (err != ESP_OK) {
        schedule_close(0_ms, "Error connecting to peer");
        return;
    }
#if MODULE_DEBUG_AVAILABLE()
    debug.register_task(tf_websocket_client_get_task_handle(ws_server), SHIP_CONNECTION_WS_CLIENT_TASK_STACK);
#endif
}

void ShipConnection::start_client_confirm()
{
    if (closing_scheduled) {
        return;
    }
    reschedule_task(
        state_machine_task,
        [this]() {
            state_machine_next_step();
        },
        1_s);
}

void ShipConnection::process_frame(const uint8_t *data, const size_t len, const bool fin)
{
    if (closing_scheduled) {
        return;
    }
    if (len < 2) {
        eebus.trace_fmtln("ShipConnection process_frame: payload too short: %zu", len);
        return;
    }
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    if (state != ShipConnectionState::Done) {
        eebus.trace_fmtln("ShipConnection frame received from %s during connection establishment", peer_node->node_name().c_str());
    }
#endif

    // Copy new message, await further parts if its not final and trigger the next SHIP state machine step
    if (message_incoming->multipart_index == SIZE_MAX) {
        memset(message_incoming->data, 0, SHIP_CONNECTION_MAX_BUFFER_SIZE);
        message_incoming->multipart_index = 0;
        message_incoming->length = 0;
    }
    if (message_incoming->multipart_index + len >= SHIP_CONNECTION_MAX_BUFFER_SIZE) {
        eebus.trace_fmtln("process_frame: ws frame too big for the buffer. Current index: %zu, incoming length: %zu, max buffer size: %d", message_incoming->multipart_index, len, SHIP_CONNECTION_MAX_BUFFER_SIZE);
        message_incoming->multipart_index = SIZE_MAX;
        message_incoming->length = 0;
        return;
    }
    memcpy(message_incoming->data + message_incoming->multipart_index, data, len);

    message_incoming->length += len;
    message_incoming->multipart_index += len;

    if (fin) {
        message_incoming->multipart_index = SIZE_MAX;
        incoming_message_pending = true;
        state_machine_next_step();
    }
}

void ShipConnection::schedule_close(const millis_t delay_ms, const String &reason)
{
    if (closing_scheduled)
        return;
    closing_scheduled = true;
    cancel_task(state_machine_task);
    cancel_task(timeout_task);
    cancel_task(hello_wait_for_ready_timer);
    cancel_task(hello_send_prolongation_request_timer);
    cancel_task(hello_send_prolongation_reply_timer);
    cancel_task(hello_trust_check_timer);
    cancel_task(protocol_handshake_timer);

    // Only mark the node as disconnected if this is the last active connection to it,
    // otherwise closing a duplicate connection (SHIP 13.4.6.2) would wrongly flip an
    // active peer to Disconnected.
    if (peer_node) {
        bool other_connection_active = false;
        for (const auto &conn : eebus.ship.ship_connections) {
            if (conn.get() != this && !conn->closing_scheduled && conn->peer_node == peer_node) {
                other_connection_active = true;
                break;
            }
        }
        if (!other_connection_active) {
            peer_node->state = NodeState::Disconnected;
        }
    }

    task_scheduler.scheduleOnce(
        [this, reason]() {
            logger.printfln("Closing connection to %s", peer_node ? peer_node->node_name().c_str() : "<unknown>");
            eebus.trace_fmtln("Closing connection to %s. Reason: %s", peer_node ? peer_node->node_name().c_str() : "<unknown>", reason.c_str());
            if (role == Role::Server) {
                if (ws_fd >= 0) {
                    eebus.ship.ws_close(ws_fd);
                    ws_fd = -1;
                }
            } else if (role == Role::Client) {
#if MODULE_DEBUG_AVAILABLE()
                TaskHandle_t ws_task = tf_websocket_client_get_task_handle(ws_server);
                if (ws_task != nullptr) {
                    debug.deregister_task(ws_task);
                }
#endif
                tf_websocket_client_close(ws_server, pdMS_TO_TICKS(SHIP_CONNECTION_WS_TIMEOUT_MS));
                tf_websocket_client_destroy(ws_server);
                // Destroy the external transport that we own (not managed by tf_websocket_client)
                if (ext_transport != nullptr) {
                    esp_transport_destroy(ext_transport);
                    ext_transport = nullptr;
                }
            }
            eebus.ship.remove(*this);
            eebus.update_peers_state();
        },
        delay_ms);
}

void ShipConnection::initiate_termination(const String &reason)
{
    // SHIP 13.4.8: Graceful connection termination. Announce the close to the
    // peer, then close the socket after giving it time to process the message.
    if (closing_scheduled) {
        return;
    }

    outgoing_json_doc.clear();
    JsonArray connection_close = outgoing_json_doc.createNestedArray("connectionClose");
    connection_close.createNestedObject()["phase"] = "announce";
    connection_close.createNestedObject()["maxTime"] = 500;
    // reason is an enum: "unspecific" or "removedConnection". Currently only
    // used for duplicate connection removal (SHIP 12.2.3).
    connection_close.createNestedObject()["reason"] = "removedConnection";

    message_outgoing->data[0] = 3; // SHIP message type: end (connection termination)
    const size_t length = serializeJson(outgoing_json_doc, &message_outgoing->data[1], SHIP_CONNECTION_MAX_JSON_SIZE - 1);
    message_outgoing->length = length + 1;
    send_current_outgoing_message();

    schedule_close(500_ms, reason);
}

void ShipConnection::send_raw(const char *payload, const size_t payload_len, const char *what)
{
    if (role == Role::Server) {
        if (ws_fd < 0) {
            eebus.trace_fmtln("%s: attempted send on closed connection", what);
        } else if (!eebus.ship.ws_send(ws_fd, payload, payload_len)) {
            eebus.trace_fmtln("%s: send failed", what);
            schedule_close(0_ms, "Failed to send message");
        }
    } else if (role == Role::Client) {
        tf_websocket_client_send_bin(ws_server, payload, payload_len, pdMS_TO_TICKS(SHIP_CONNECTION_WS_TIMEOUT_MS));
    }
}

void ShipConnection::send_cmi_message(uint8_t type, uint8_t value)
{
    char payload[2] = {static_cast<char>(type), static_cast<char>(value)};
    send_raw(payload, 2, "send_cmi_message");
}

void ShipConnection::send_current_outgoing_message()
{
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("Sending Websocket message");
#endif

    if (!message_outgoing) {
        eebus.trace_fmtln("message_outgoing is NULL!");
        logger.printfln("an error occurred while sending a message. Check tracelog for details.");
        return;
    }
    if (message_outgoing->length == 0) {
        return;
    }
    if (message_outgoing->length > SHIP_CONNECTION_MAX_BUFFER_SIZE) {
        eebus.trace_fmtln("Error: Message being sent exceeds maximum buffer size of %d bytes: %d bytes", SHIP_CONNECTION_MAX_BUFFER_SIZE, message_outgoing->length);
        logger.printfln("an error occurred while sending a message. Check tracelog for details.");
        return;
    }

    log_message("send_current_outgoing", message_outgoing.get());
    send_raw(reinterpret_cast<const char *>(message_outgoing->data), message_outgoing->length, "send_current_outgoing_message");
}

void ShipConnection::send_string(const char *str, const int length, const int msg_classifier)
{
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("ShipConnection::send_string: Sending Message classified as %d with length %d:", msg_classifier, length);
    eebus.trace_strln(str, length);
#endif

    char *buffer = static_cast<char *>(ps_malloc(length + 1));

    buffer[0] = static_cast<char>(msg_classifier);
    memcpy(buffer + 1, str, length);

    send_raw(buffer, length + 1, "send_string");
    free_any(buffer);
}

void ShipConnection::send_data_message(JsonVariant payload)
{
    // Technically we should only send data messages if the state is done but for some reason the done state is not set correctly
    if (/*state == ShipConnectionState::Done*/ true) {
        SHIP_TYPES::ShipMessageDataType data = SHIP_TYPES::ShipMessageDataType();
        data.protocol_id = "ee1.0"; // We only speak ee1.0
        [[maybe_unused]] auto tmp = data.payload = payload;

        String data_to_send = data.type_to_json();
        send_string(data_to_send.c_str(), data_to_send.length(), 2);
    } else {
        eebus.trace_fmtln("send_data_message: Connection not in done state. Actual State: %d", (int)state);
    }
}

ShipConnection::CMIMessage ShipConnection::get_cmi_message()
{
    // SHIP 13.4.3: A CMI message consists of the MessageType byte and at least
    // the CmiHead byte. Only CmiHead is analysed; any additional CmiRemainder
    // bytes are reserved for future use and SHALL be ignored.
    if (message_incoming->length < 2) {
        return {false, 0, 0};
    }

    return {true, message_incoming->data[0], message_incoming->data[1]};
}

ShipConnection::ProtocolState ShipConnection::get_protocol_state()
{
    // If Messagetype is %x03 then this is a termination message
    // SHIP Specification 13.4.7.1
    if (message_incoming->data[0] == 3) {
        return ProtocolState::Terminate;
    }
    if ((message_incoming->data[0] != 1 && message_incoming->data[0] != 2) || (message_incoming->length < 3)) {
        return ProtocolState::None;
    }

    const char *buf = reinterpret_cast<const char *>(message_incoming->data) + 1;
    const size_t buf_len = message_incoming->length - 1;

    // Find first " in the string, use that as a start point
    const char *key_start = (const char *)memchr(buf, '"', buf_len);
    if (!key_start)
        return ProtocolState::Unknown;
    key_start++;

    // find second " in the string. That should signal the end of the first json key
    const char *key_end = (const char *)memchr(key_start, '"', buf_len - (key_start - buf));
    if (!key_end)
        return ProtocolState::Unknown;

    const size_t key_len = (key_end - key_start);

#define KEY_IS(literal) (key_len == (sizeof(literal) - 1) && memcmp(key_start, literal, key_len) == 0)

    switch (key_start[0]) {
        case 'c':
            if (KEY_IS("connectionHello"))
                return ProtocolState::ConnectionHello;
            if (KEY_IS("connectionPinState"))
                return ProtocolState::ConnectionPinState;
            break;
        case 'm':
            if (KEY_IS("messageHello"))
                return ProtocolState::ConnectionHello;
            break;
        case 'a':
            if (KEY_IS("accessMethodsRequest"))
                return ProtocolState::AccessMethodsRequest;
            if (KEY_IS("accessMethods"))
                return ProtocolState::AccessMethods;
            break;
        case 'd':
            if (KEY_IS("data"))
                return ProtocolState::Data;
            break;
        default:
            return ProtocolState::Unknown;
    }
#undef KEY_IS

    return ProtocolState::Unknown;
}


void ShipConnection::log_message(const String &state_prefix, Message *msg)
{
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("SHIP: flag(%s) Message (%d) (len %d)", state_prefix.c_str(), static_cast<int>(msg->data[0]), msg->length);
    eebus.trace_strln(reinterpret_cast<const char *>(&msg->data[1]), msg->length - 1);
#endif
}
