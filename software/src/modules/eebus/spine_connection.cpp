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
#include "tools.h"

bool SpineConnection::process_datagram(JsonVariant datagram)
{
    // TODO: Handle json formatting weirdness sent by e.g. spine-go
    last_received_time = millis();
    compatiblity_mode = true;
    ArduinoJson::spine_go_compatibility_mode = this->compatiblity_mode;
    //received_header = HeaderType(); // Reset the header to avoid using old values
    logger.printfln("Mark 1: Stack high water mark: %u bytes", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));

    received_header = datagram["datagram"][0]["header"];
    //logger.printfln("SPINE: Received datagram: %s", datagram.as<String>().c_str());
    //logger.printfln("Header: %s", datagram["datagram"][0].as<String>().c_str());

    received_payload =
        datagram["datagram"][1]["payload"][0]["cmd"][0][0]; // The payload should not be in an array but spine-go does these strange things

    if (!received_header.cmdClassifier || !received_header.addressSource || received_payload.isNull()) {
        logger.tracefln(eebus.trace_buffer_index, "SPINE: ERROR: No datagram header or payload found");
        if (!received_header.cmdClassifier) {
            logger.tracefln(eebus.trace_buffer_index, "SPINE: ERROR: No cmdClassifier found in the received header");
        }
        if (!received_header.addressSource) {
            logger.tracefln(eebus.trace_buffer_index, "SPINE: ERROR: No addressSource found in the received header");
        }
        if (received_payload.isNull()) {
            logger.tracefln(eebus.trace_buffer_index, "SPINE: ERROR: No payload found in the received datagram");
            //logger.printfln("SPINE: Payload section: %s", datagram["datagram"][1]["payload"][0].as<String>().c_str());
        } else {
            //logger.printfln("Received Payload: %s", received_payload.as<String>().c_str());
        }
        return false;
    }
    logger.printfln("Mark 2: Stack high water mark: %u bytes", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));

    check_message_counter();
    SpineDataTypeHandler::Function called_function = eebus.data_handler->handle_cmd(received_payload);
    if (called_function == SpineDataTypeHandler::Function::None) {
        logger.tracefln(eebus.trace_buffer_index, "SPINE: No function found for the received payload");
        //logger.printfln("SPINE: Payload: %s", received_payload.as<String>().c_str());
        return false;
    }
    logger.printfln("Mark 3: Stack high water mark: %u bytes", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));

    //response_doc.clear();
    eebus.usecases.handle_message(received_header, eebus.data_handler.get(), this);
    //logger.printfln("Mark 4: Stack high water mark: %u bytes", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));

    //logger.printfln("SPINE: Message processed");

    return true;
}

void SpineConnection::send_datagram(JsonVariantConst payload,
                                    CmdClassifierType cmd_classifier,
                                    const FeatureAddressType &sender,
                                    const FeatureAddressType &receiver,
                                    bool require_ack)
{
    logger.tracefln(eebus.trace_buffer_index,
                    "SPINE: Sending datagram. cmdClassifier: %d, Content: %s",
                    static_cast<int>(cmd_classifier),
                    payload.as<String>().c_str());
    response_doc.clear();
    HeaderType header;
    header.ackRequest = require_ack;
    header.cmdClassifier = cmd_classifier;
    header.specificationVersion = SUPPORTED_SPINE_VERSION;
    header.addressSource = sender;
    header.addressDestination = receiver;
    header.msgCounter = msg_counter++;
    header.msgCounterReference = received_header.msgCounter; // The message counter of the last received datagram

    response_doc["datagram"][0]["header"] = header;
    response_doc["datagram"][1]["payload"]["cmd"][0] = payload;
    ship_connection->send_data_message(response_doc);
    // TODO: Handle acknowledge request
}

void SpineConnection::check_message_counter()
{
    // TODO: Implement a proper message counter check
    if (received_header.msgCounter && received_header.msgCounter.value() < msg_counter_received) {
        logger.tracefln(eebus.trace_buffer_index,
                        "SPINE Message counter is lower than expected. The peer might have technical issues or has been rebooted.");
        msg_counter_received = received_header.msgCounter.value();
        msg_counter_error_count++;
    } else {
        msg_counter_error_count = msg_counter_error_count > 0 ? msg_counter_error_count - 1 : 0;
    }

    // We ignore the message counter received for now as we are not sending messages that warrant a response.
}
bool SpineConnection::check_known_address(const FeatureAddressType &address)
{
    for (FeatureAddressType &known_address : known_addresses) {
        if (known_address.device == address.device && known_address.feature == address.feature && known_address.entity == address.entity) {
            return true; // The address is known
        }
    }
    return false;
}
