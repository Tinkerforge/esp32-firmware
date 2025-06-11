
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

#include "build.h"

#include "config.h"
#include "module.h"
#include <TFJson.h>
#include "spine_types.h"

#define SPINE_CONNECTION_MAX_JSON_SIZE 8192 // TODO: What is a sane value here?
#define SPINE_CONNECTION_MAX_DEPTH 30       // Maximum depth of serialization of a json document

#define SPINE_DEVICE_ID "d:_i:123456_WARP3" // TODO: This should be based on the device UUID



struct SpineHeader {
    CoolString version; // Version of SPINE the node is using
    CoolString source_device_id;
    bool source_device_id_valid = false; // True if device is present and not empty or not present. False if present and empty
    std::vector<uint8_t> source_entity;
    uint16_t source_feature{};

    CoolString destination_device_id;
    bool destination_device_id_valid = false;
    std::vector<uint8_t> destination_entity;
    uint16_t destination_feature{};
    uint64_t msg_counter{};
    uint64_t msg_counter_received{};
    CoolString cmd_classifier;
    bool wants_response{};

    void from_json(String json);
    String to_json();
};

class ShipConnection; // Need to forward declare this here so it can be included in ship_connection.h

class SpineConnection
{

public:
    ShipConnection *ship_connection = nullptr;
    explicit SpineConnection(ShipConnection *ship_connection) : ship_connection(ship_connection), received_header() {};
  

    /**
    * Process a received SPINE datagram and passes the data to the EEBUS Usecase.
    * @param datagram JsonVariant containing the datagram process
    * @return true if a response was created and needs to be sent, false if no response is needed or an error occurred
    */
    bool process_datagram(JsonVariant datagram);
    /**
    * Check if the message counter is correct and log it if it isnt. Its not actually a problem if the message counter is lower than expected but indicates that the peer might have technical issues or has been rebooted.
    */
    void check_message_counter();

    // SPINE TS 5.2.3.1
    // Specification recommends these be stored in non-volatile memory
    // But for now we dont do that as it is not needed
    uint64_t msg_counter = 0;          // Our message counter
    uint64_t msg_counter_received = 0; // The message counter of the last received datagram

    /**
     * The ID of this device
     */
    CoolString device_id = SPINE_DEVICE_ID;

    /**
    * The last received header of a SPINE datagram.
    */
    SpineHeader received_header;

    /**
    * The Payload of the last received SPINE datagram.
    */
    JsonVariant received_payload;

    /**
    * The JSON Document used for holding the response
    */
    DynamicJsonDocument response_doc{SPINE_CONNECTION_MAX_JSON_SIZE}; // TODO: Maybe allocate this in PSRAM
    /**
    * The response datagram to be retrieved by the SPINE Connection and sent back to the peer.
    */
    JsonVariant response_datagram;

};