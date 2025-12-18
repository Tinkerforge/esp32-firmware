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

#pragma once

#include "config.h"
#include "module.h"
#include "ship_connection.h"
#include <TFJson.h>

namespace SHIP_TYPES
{
// SHIP Datatypes as defined in SHIP .xsd
enum DeserializationResult { SUCCESS, ERROR, NOT_IMPLEMENTED };

void DeserializeOptionalField(JsonObject *data, const char *field_name, bool *field_valid, String *field_value);

template <typename T> void DeserializeOptionalField(JsonObject *data, const char *field_name, bool *field_valid, std::vector<T> *field_value);

static constexpr uint16_t JSON_TO_EEBUS_MAX_DEPTH = 32;
/**
 * EEBUS wants objects to be arrays with each field being an array element. This function converts a normal json object to the EEBUS format
 * @param src The Json object to convert. Should belong to a different JsonDocument than dst
 * @param dst Target Json object where the converted json will be stored. Should belong to a different JsonDocument than src
 * @param recursion_depth Depth of current recursion. Used to avoid too deep recursion. Should be 0 when called as it might create invalid EEBUS Json
 */
void JsonToEEBusJson(JsonVariantConst src, JsonVariant dst, uint16_t depth = 0);
/**
 * EEBUS wants objects to be arrays with each field being an array element. This function converts EEBUS json to a normal json object
 * @param json_in The EEBUS Json
 * @return Json in regular JSON format
 */
String EEBUSJsonToJson(String json_in);

struct ShipMessageDataType {
    // SHIP 13.4.5.2.1
    // Mandatory
    bool valid = false;
    String protocol_id{};
    JsonVariant payload{};

    // Optional
    // This is used by Manufacturers to add their own data to the message
    // If a field is to be sent out it has to be set to valid = true, otherwise it will be ignored
    bool extension_id_valid = false;
    bool extension_binary_valid = false;
    bool extension_string_valid = false;
    String extension_id{};
    std::vector<bool> extension_binary{}; // This technically an array of integers
    String extension_string{};

    bool eebus_json_compatibility_mode = true; // If true, the json will be converted to EEBUS format during deserialization and back to normal json during serialization
    BasicJsonDocument<ArduinoJsonPsramAllocator> doc{SHIP_CONNECTION_MAX_JSON_SIZE};
    DeserializationResult json_to_type(uint8_t *data, size_t length);
    // Needs a jsondoc otherwise the payload cant be stored
    String type_to_json();
};

struct ShipMessageAccessMethodsRequest {
    String request; // There is no datatype defined for this yet
    DeserializationResult json_to_type(uint8_t *data, size_t length);
    String type_to_json();
};

struct ShipMessageAccessMethods {
    String id;
    std::vector<String> dns_sd_mdns;
    bool dns_sd_mdns_valid = false;
    std::vector<String> dns{};
    bool dns_valid = false;
    String dns_uri{};
    bool dns_uri_valid = false;

    DeserializationResult json_to_type(uint8_t *data, size_t length);
    String type_to_json();
};
} // namespace SHIP_TYPES
