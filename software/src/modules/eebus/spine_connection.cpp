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

void SpineConnection::process_datagram(JsonVariant datagram)
{
    JsonVariant datagram_header = datagram["datagram"][0]["header"];

    received_payload = datagram["datagram"][1]["payload"][0]["cmd"][0][0];
    if (datagram_header.isNull() || received_payload.isNull()) {
        logger.printfln("Error: No datagram header or payload found");
        return;
    }

    received_header.from_json(datagram_header.as<String>());

    check_message_counter();
    SpineDataTypeHandler::Function called_function = eebus.data_handler.handle_cmd(received_payload);

    if (called_function == SpineDataTypeHandler::Function::None) {
        logger.printfln("SPINE: No function found for the received payload");
        logger.printfln("SPINE: Payload: %s", received_payload.as<String>().c_str());
        return;
    }
    DynamicJsonDocument doc{10000};
    JsonVariant response = doc.to<JsonVariant>();
    const bool has_response = eebus.usecases.handle_message(received_header, eebus.data_handler, response);

    //logger.printfln("SPINE HEADER: %s", received_header.to_json().c_str());

    //logger.printfln("SPINE Process payload: %s", received_payload.as<String>().c_str());

    if (has_response) {

        // TODO: Build response datagram
        DynamicJsonDocument responseJson{10000};
        JsonVariant responseObject = responseJson.to<JsonVariant>();
        responseObject["datagram"]["header"]["specificationVersion"] = "1.3.0";
        responseObject["datagram"]["header"]["addressSource"]["device"] = "d:_i:123456_warp3";
        responseObject["datagram"]["header"]["addressSource"]["entity"][0] = 0;
        responseObject["datagram"]["header"]["addressSource"]["feature"] = 0;
        responseObject["datagram"]["header"]["addressDestination"]["device"] = received_header.source_device_id;
        responseObject["datagram"]["header"]["addressDestination"]["entity"][0] = 0;
        responseObject["datagram"]["header"]["addressDestination"]["feature"] = 0;
        responseObject["datagram"]["header"]["msgCounter"] = msg_counter++;
        responseObject["datagram"]["header"]["msgCounterReference"] = received_header.msg_counter;
        responseObject["datagram"]["header"]["cmdClassifier"] = "reply";
        responseObject["datagram"]["header"]["ackRequest"] = false;
        responseObject["datagram"]["payload"]["cmd"][0][0] = response;;


        String responseString;
        responseJson.shrinkToFit();
        serializeJson(responseJson, responseString);

        logger.printfln("SPINE: Sending response: %s", responseString.c_str());
        ship_connection->send_string(responseString.c_str(), responseString.length());

    } else if (!has_response && received_header.wants_response) {
        logger.printfln("SPINE: No response availabe but one was requested.");
        logger.printfln("Payload: %s", received_payload.as<String>().c_str());
    } else {
        logger.printfln("SPINE: No response needed");
    }
}

void SpineConnection::check_message_counter()
{
    if (received_header.msg_counter < msg_counter_received) {
        logger.printfln("SPINE Message counter is lower than expected. The peer might have technical issues or has been rebooted.");
        msg_counter_received = received_header.msg_counter;
    }

    // We ignore the message counter received for now as we are not sending messages that warrant a response.
}

void SpineHeader::from_json(String json)
{
    DynamicJsonDocument json_doc{SPINE_CONNECTION_MAX_JSON_SIZE};

    DeserializationError error = deserializeJson(json_doc, json);

    if (error) {
        Serial.print("Failed to deserialize SPINE Datagram Header: ");
        Serial.println(error.c_str());
        return;
    }

    version = json_doc[0]["specificationVersion"].as<String>();
    source_device_id = json_doc[1]["addressSource"][0]["device"].as<String>(); // "device_id_0"
    source_device_id_valid = json_doc[1]["addressSource"][0]["device"].isNull() || source_device_id.length() > 0;

    int source_entity_size = json_doc[1]["addressSource"][0]["entity"].size(); // 2
    for (int i = 0; i < source_entity_size; i++) {
        source_entity.push_back(json_doc[1]["addressSource"][0]["entity"][i]);
    }
    source_feature = json_doc[1]["addressSource"][0]["feature"];
    destination_device_id = json_doc[2]["addressDestination"][0]["device"].as<String>(); // "device_id_0"
    destination_device_id_valid = json_doc[2]["addressDestination"][0]["device"].isNull() || destination_device_id.length() > 0;

    int destination_entity_size = json_doc[2]["addressDestination"][0]["entity"].size(); // 2
    for (int i = 0; i < destination_entity_size; i++) {
        destination_entity.push_back(json_doc[2]["addressDestination"][0]["entity"][i]);
    }
    destination_feature = json_doc[2]["addressDestination"][0]["feature"];
    msg_counter = json_doc[3]["msgCounter"];
    msg_counter_received = json_doc[3]["msgCounterReceived"];
    cmd_classifier = json_doc[4]["cmdClassifier"].as<String>(); // "read"
    wants_response = json_doc[5]["wantsResponse"];
}

String SpineHeader::to_json()
{
    DynamicJsonDocument json_doc{SPINE_CONNECTION_MAX_JSON_SIZE};
    JsonArray datagram = json_doc.createNestedArray("datagram");
    JsonObject header = datagram.createNestedObject();
    header["specificationVersion"] = version;
    JsonObject address_source = header.createNestedObject("addressSource");

    if (source_device_id_valid) {
        JsonObject address_source_device = address_source.createNestedObject("device");
        address_source_device["device"] = source_device_id;
    }
    JsonArray address_source_entity = address_source.createNestedArray("entity");
    for (const auto &entity : source_entity) {
        address_source_entity.add(entity);
    }
    address_source["feature"] = source_feature;
    JsonObject address_destination = header.createNestedObject("addressDestination");

    if (destination_device_id_valid) {
        JsonObject address_destination_device = address_destination.createNestedObject("device");
        address_destination_device["device"] = destination_device_id;
    }

    JsonArray address_destination_entity = address_destination.createNestedArray("entity");
    for (const auto &entity : destination_entity) {
        address_destination_entity.add(entity);
    }
    address_destination["feature"] = destination_feature;
    JsonObject msg_counter_obj = header.createNestedObject("msgCounter");
    msg_counter_obj["msgCounter"] = msg_counter;
    msg_counter_obj["msgCounterReceived"] = msg_counter_received;
    header["cmdClassifier"] = cmd_classifier;
    header["wantsResponse"] = wants_response;
    String output;
    //json_doc.shrinkToFit();
    serializeJson(json_doc, output);
    logger.printfln("SPINE Datagram Header JSON: %s", output.c_str());
    return output;
}
