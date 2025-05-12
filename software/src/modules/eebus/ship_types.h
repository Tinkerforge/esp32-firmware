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

#include "module.h"
#include "config.h"
#include <TFJson.h>

#define SHIP_TYPES_MAX_JSON_SIZE 8192 // TODO: What is a sane value here?

namespace SHIP_TYPES {


// SHIP Datatypes as defined in SHIP .xsd

    enum DeserializationResult {
        SUCCESS,
        ERROR,
        NOT_IMPLEMENTED
    };

    void DeserializeOptionalField(JsonObject *data, const char *field_name, bool *field_valid, String *field_value);
    
    template <typename T>
    void DeserializeOptionalField(JsonObject *data, const char *field_name, bool *field_valid, std::vector<T> *field_value);

    struct ShipMessageDataType {
        // SHIP 13.4.5.2.1
        // Mandatory
        bool valid = false;
        CoolString protocol_id{};
        JsonVariant payload{}; 

        // Optional
        // This is used by Manufacturers to add their own data to the message
        // If a field is to be sent out it has to be set to valid = true, otherwise it will be ignored
        bool extension_id_valid = false;
        bool extension_binary_valid = false;
        bool extension_string_valid = false;
        CoolString extension_id{};
        std::vector<bool> extension_binary{}; // This technically an array of integers
        CoolString extension_string{};

        DeserializationResult json_to_type(uint8_t *data, size_t length);
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