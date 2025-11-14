/* esp32-firmware
 * Copyright (C) 2025 Julius Dill <julius@tinkerforge.com>
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
#include "spine_connection.h"

#include "build.h"
#include "eebus.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "ship_types.h"
#include "tools.h"

bool SpineConnection::process_datagram(JsonVariant datagram)
{
    eebus.trace_fmtln("SPINE: Processing datagram:");
    eebus.trace_jsonln(datagram);
    last_received_time = millis();

    received_header = datagram["datagram"]["header"];
    received_payload = datagram["datagram"]["payload"]["cmd"][0];

    if (validate_header(received_header)) {
        eebus.trace_fmtln("SPINE: ERROR: Received datagram header is invalid");
        return false;
    }
    if (received_payload.isNull()) {
        eebus.trace_fmtln("SPINE: ERROR: No payload found in the received datagram");
        return false;
    }
    if (!check_known_address(received_header.addressSource.get())) {
        known_addresses.push_back(received_header.addressSource.get());
    }

    check_message_counter();
    SpineDataTypeHandler::Function called_function = eebus.data_handler->handle_cmd(received_payload);
    if (called_function == SpineDataTypeHandler::Function::None) {
        eebus.trace_fmtln("SPINE: No function found for the received payload:");
        eebus.trace_jsonln(received_payload);
        return false;
    }
    eebus.usecases->handle_message(received_header, eebus.data_handler.get(), this);
    return true;
}

void SpineConnection::send_datagram(JsonVariantConst payload, CmdClassifierType cmd_classifier, const FeatureAddressType &sender, const FeatureAddressType &receiver, const bool require_ack)
{
    eebus.trace_fmtln("SPINE: Sending datagram. cmdClassifier: %s, Content:", convertToString(cmd_classifier).c_str());
    eebus.trace_jsonln(payload);
    // so i spent 4 hours on this and for some reason the pointers to sender and receivers seem to be nullpointers in about 1/5 restarts but if i print them here its fine mostly.
    logger.printfln("SPINE Connection: This needs to be here otherwise it crashes sometimes. Pointer sender: %p, Pointer Receiver: %p", &sender, &receiver);

    BasicJsonDocument<ArduinoJsonPsramAllocator> response_doc{payload.memoryUsage() + 512}; // Payload size + header size + some slack as recommended by arduinojson assistant
    HeaderType header{};
    header.ackRequest = require_ack;
    header.cmdClassifier = cmd_classifier;
    header.specificationVersion = SUPPORTED_SPINE_VERSION;
    header.addressSource = sender;
    // See if this fixes the memory issue
    //header.addressSource->device = sender.device.get().c_str();
    //header.addressSource->entity = sender.entity.get();
    //header.addressSource->feature = sender.feature.get();

    header.addressDestination = receiver;
    header.msgCounter = msg_counter++;
    if (cmd_classifier == CmdClassifierType::reply || cmd_classifier == CmdClassifierType::result) {
        header.msgCounterReference = received_header.msgCounter; // The message counter of the last received datagram
    }
    response_doc["datagram"][0]["header"] = header;
    if (!response_doc["datagram"][1]["payload"]["cmd"][0].set(payload)) {
        eebus.trace_fmtln("SPINE: ERROR: Could not set payload for the datagram");
        return;
    }
#ifdef EEBUS_DEV_ENABLE_RESPONSE
    ship_connection->send_data_message(response_doc.as<JsonVariant>());
#else
    eebus.trace_fmtln("Sending SPINE messages is disabled. No message sent");
#endif
    // TODO: Handle acknowledge request. Some messages require acknowledgement from the other side. If we send one of those we need to handle this
}

void SpineConnection::check_message_counter()
{
    if (received_header.msgCounter && received_header.msgCounter.get() < msg_counter_received) {
        eebus.trace_fmtln("SPINE Message counter is lower than expected. The peer might have technical issues or has been rebooted.");
        msg_counter_received = received_header.msgCounter.get();
        msg_counter_error_count++;
    } else {
        msg_counter_error_count = msg_counter_error_count > 0 ? msg_counter_error_count - 1 : 0;
    }
}

bool SpineConnection::check_known_address(const FeatureAddressType &address)
{
    for (FeatureAddressType &known_address : known_addresses) {
        if (known_address.device.get() == address.device.get() && known_address.feature.get() == address.feature.get() && known_address.entity.get() == address.entity.get()) {
            return true;
        }
    }
    return false;
}

bool SpineConnection::validate_header(HeaderType &header)
{
    bool error_found = false;
    if (header.cmdClassifier.isNull()) {
        eebus.trace_fmtln("SPINE: ERROR: No cmdClassifier found in the received header");
        error_found = true;
    }
    if (header.addressSource.isNull() || header.addressSource->feature.isNull() || header.addressSource->entity.isNull() || header.addressSource->entity->empty()) {
        eebus.trace_fmtln("SPINE: ERROR: No addressSource found in the received header or existing addressSource is invalid");
        error_found = true;
    }
    if (header.addressDestination.isNull() || header.addressDestination->feature.isNull() || header.addressDestination->entity.isNull() || header.addressDestination->entity->empty()) {
        eebus.trace_fmtln("SPINE: ERROR: No addressDestination found in the received header or existing addressDestination is invalid");
        error_found = true;
    }

    return error_found;
}
