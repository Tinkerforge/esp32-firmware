/* esp32-firmware
 * Copyright (C) 2024 Julius Dill <julius@tinkerforge.com>
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
        CoolString payload{}; //Can be anything so we just shove it into a string and let the handler responsible for the protocol figure it out


        // Optional
        // This is used by Manufacturers to add their own data to the message
        // If a field is to be sent out it has to be set to valid = true
        bool extension_id_valid = false;
        bool extension_binary_valid = false;
        bool extension_string_valid = false;
        CoolString extension_id{};
        std::vector<bool> extension_binary{}; // This technically an array of integers
        CoolString extension_string{};

        DeserializationResult json_to_type(String json);
        String type_to_json();

    };

    } // namespace SHIP_TYPES