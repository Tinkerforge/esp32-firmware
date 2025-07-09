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

#include "ship_types.h"

#include "build.h"
#include "eebus.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "ship_connection.h"
#include "tools.h"

namespace SHIP_TYPES
{

DeserializationResult ShipMessageDataType::json_to_type(uint8_t *incoming_data, size_t length, bool compatiblity_mode)
{
    //DynamicJsonDocument doc{SHIP_TYPES_MAX_JSON_SIZE};

    String incoming_data_str(reinterpret_cast<const char *>(incoming_data), length);
    if (compatiblity_mode) {
        incoming_data_str.replace("{}", "[]");
        incoming_data_str.replace("}", "}]");
        incoming_data_str.replace(",", "},{");
        incoming_data_str.replace("{", "[{");
    }
    int nesting_limit = 20;
    DeserializationError error = deserializeJson(doc, incoming_data_str, DeserializationOption::NestingLimit(nesting_limit));

    //doc.shrinkToFit(); // Make this a bit smaller
    if (error) {
        logger.printfln("J2T ShipMessageData Error during JSON deserialization : %s", error.c_str());
        return DeserializationResult::ERROR;
    }

    JsonObject data = doc["data"][0];

    if (data.isNull()) {
        logger.printfln("J2T ShipMessageData Error: No data object found");

        return DeserializationResult::ERROR;
    }
    if (doc["data"][0]["header"][0]["protocolId"] == nullptr || doc["data"][1]["payload"] == nullptr) {
        logger.printfln("J2T ShipMessageData Error: Data invalid");
        valid = false;
        return DeserializationResult::ERROR;
    }
    protocol_id = String(doc["data"][0]["header"][0]["protocolId"]);
    payload = doc["data"][1]["payload"];
    valid = true;

    JsonObject data_extension = data["extension"];
    // Optional fields

    DeserializeOptionalField(&data_extension, "extensionId", &extension_id_valid, &extension_id);
    DeserializeOptionalField(&data_extension, "binary", &extension_binary_valid, &extension_binary);
    DeserializeOptionalField(&data_extension, "string", &extension_string_valid, &extension_string);

    return DeserializationResult::SUCCESS;
}

String ShipMessageDataType::type_to_json(ShipConnection::Message &message_outgoing)
{
    DynamicJsonDocument doc{SHIP_TYPES_MAX_JSON_SIZE};

    JsonObject data = doc["data"].to<JsonObject>();
    data["header"]["protocolId"] = protocol_id;
    bool payload_loaded = data["payload"].set(payload);
    logger.printfln("Payload: %s", payload.as<String>().c_str());
    if (!payload_loaded) {
        logger.printfln("J2T ShipMessageData Error: Payload invalid");
        return "";
    }

    if (extension_id_valid || extension_binary_valid || extension_string_valid) {
        JsonObject data_extension = data.createNestedObject("extension");
        if (extension_id_valid) {
            data_extension["extensionId"] = extension_id;
        }
        if (extension_binary_valid) {
            JsonArray data_extension_binary = data_extension.createNestedArray("binary");
            for (const auto &value : extension_binary) {
                data_extension_binary.add(String(value));
            }
        }
        if (extension_string_valid) {
            data_extension["string"] = extension_string;
        }
    }

    message_outgoing.data[0] = 2;
    String message_outgoing_data;
    message_outgoing_data.reserve(SHIP_TYPES_MAX_JSON_SIZE - 1); // Reserve space for the JSON data
    serializeJson(doc, message_outgoing_data);
    //message_outgoing_data.replace("[{", "[[{"); // spine-go expects a double array for some reason
    //message_outgoing_data.replace("}]", "}]]");

    //size_t size = serializeJson(doc, &message_outgoing.data[1], SHIP_TYPES_MAX_JSON_SIZE - 1);
    memcpy(&message_outgoing.data[1], message_outgoing_data.c_str(), message_outgoing_data.length());
    message_outgoing.length = message_outgoing_data.length() + 1;
    return "";
}

void DeserializeOptionalField(JsonObject *data, const char *field_name, bool *field_valid, String *field_value)
{
    if (data->containsKey(field_name)) {
        *field_value = (*data)[field_name].as<String>();
        *field_valid = true;
    } else {
        *field_valid = false;
    }
}

template <typename T>
void DeserializeOptionalField(JsonObject *data, const char *field_name, bool *field_valid, std::vector<T> *field_value)
{
    if (data->containsKey(field_name)) {
        for (JsonVariant value : (*data)[field_name].as<JsonArray>()) {
            field_value->push_back(value.as<T>());
        }
        *field_valid = true;
    } else {
        *field_valid = false;
    }
}
DeserializationResult ShipMessageAccessMethodsRequest::json_to_type(uint8_t *data, size_t length)

{
    DynamicJsonDocument doc{SHIP_TYPES_MAX_JSON_SIZE};
    logger.printfln("J2T ShipMessageAccessMethodsRequest json: %s", data);
    DeserializationError error = deserializeJson(doc, data, length);
    //doc.shrinkToFit(); // Make this a bit smaller
    if (error) {
        logger.printfln("J2T ShipMessageAccessMethodsRequest Error during JSON deserialization : %s", error.c_str());
        return DeserializationResult::ERROR;
    }
    JsonObject accessMethodsRequest = doc["accessMethodsRequest"];
    if (accessMethodsRequest.isNull()) {
        logger.printfln("J2T ShipMessageAccessMethodsShipMessageAccessMethodsRequest Error: Invalid accessMethodsRequest");
        return DeserializationResult::ERROR;
    }
    request = accessMethodsRequest["request"].as<String>();
    return DeserializationResult::SUCCESS;
}
String ShipMessageAccessMethodsRequest::type_to_json()
{
    DynamicJsonDocument doc{SHIP_TYPES_MAX_JSON_SIZE};
    JsonObject accessMethodsRequest = doc["accessMethodsRequest"].to<JsonObject>();
    accessMethodsRequest["request"] = request;
    String output;
    //doc.shrinkToFit();
    serializeJson(doc, output);
    logger.printfln("T2J ShipMessageAccessMethods json: %s", output.c_str());
    return output;
}

DeserializationResult ShipMessageAccessMethods::json_to_type(uint8_t *data, size_t length)
{
    DynamicJsonDocument doc{SHIP_TYPES_MAX_JSON_SIZE};
    DeserializationError error = deserializeJson(doc, data, length);
    //doc.shrinkToFit(); // Make this a bit smaller
    if (error) {
        logger.printfln("J2T ShipMessageAccessMethods Error during JSON deserialization : %s. Data: %s", error.c_str(), data);
        return DeserializationResult::ERROR;
    }
    JsonObject accessMethods = doc["accessMethods"];
    if (accessMethods.isNull() || accessMethods["id"] == nullptr) {
        logger.printfln("J2T ShipMessageAccessMethods Error: Invalid accessMethods");
        return DeserializationResult::ERROR;
    }
    id = accessMethods["id"].as<String>();
    DeserializeOptionalField(&accessMethods, "dns_sd_mdns", &dns_sd_mdns_valid, &dns_sd_mdns);
    DeserializeOptionalField(&accessMethods, "dns", &dns_valid, &dns);
    DeserializeOptionalField(&accessMethods, "dns_uri", &dns_uri_valid, &dns_uri);
    return DeserializationResult::SUCCESS;
}
String ShipMessageAccessMethods::type_to_json()
{
    DynamicJsonDocument doc{SHIP_TYPES_MAX_JSON_SIZE};
    JsonArray json_am = doc.createNestedArray("accessMethods");
    JsonObject access_methods = json_am.createNestedObject();
    access_methods["id"] = id;

    json_am.createNestedObject().createNestedArray("dnsSd_mDns");
    for (auto &value : dns_sd_mdns) {
        access_methods["dns_sd_mdns"].add(value);
    }
    /* This is standard conform, but ship-go does not accept it. remove it for now..
    JsonArray dns = json_am.createNestedObject().createNestedArray("dns");
    for (const auto &value : dns) {
        access_methods["dns"].add(value);
    }
    JsonObject uri = dns.createNestedObject();
    uri["uri"] =  dns_uri; //"wss://192.168.0.33:4712/ship/"; // TODO
    */
    String output;
    //doc.shrinkToFit();
    serializeJson(doc, output);
    logger.printfln("T2J ShipMessageAccessMethods json: %s", output.c_str());
    return output;
}
} // namespace SHIP_TYPES