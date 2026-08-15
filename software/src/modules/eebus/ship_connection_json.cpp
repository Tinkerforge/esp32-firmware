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

void ShipConnection::json_to_type_connection_hello(ConnectionHelloType *connection_hello)
{
    incoming_json_doc.clear();
    DeserializationError error = deserializeJson(incoming_json_doc, &message_incoming->data[1], message_incoming->length - 1);
    if (error) {
        eebus.trace_fmtln("ConnectionHello: Error during JSON deserialization: %s", error.c_str());
    } else {
        // Initialize non-mandatory fields
        connection_hello->waiting = 0;
        connection_hello->waiting_valid = false;
        connection_hello->prolongation_request = false;
        connection_hello->prolongation_request_valid = false;

        // Parse all fields, overwriting the optional ones if present
        for (JsonObject obj : incoming_json_doc["connectionHello"].as<JsonArray>()) {
            if (obj.containsKey("phase")) {
                connection_hello->phase = static_cast<ConnectionHelloPhase::Type>(ConnectionHelloPhase::from_str(obj["phase"].as<String>().c_str()));
            } else if (obj.containsKey("waiting")) {
                connection_hello->waiting = obj["waiting"].as<uint32_t>();
                connection_hello->waiting_valid = true;
            } else if (obj.containsKey("prolongationRequest")) {
                connection_hello->prolongation_request = obj["prolongationRequest"].as<bool>();
                connection_hello->prolongation_request_valid = true;
            }
        }

#ifdef EEBUS_TRACE_SUPER_VERBOSE
        eebus.trace_fmtln("J2T ConnectionHello Type: phase %d, waiting %ld(%d), prolongation_request %d(%d)'", static_cast<std::underlying_type<ConnectionHelloPhase::Type>::type>(connection_hello->phase), connection_hello->waiting, connection_hello->waiting_valid, connection_hello->prolongation_request, connection_hello->prolongation_request_valid);
#endif
    }
}

void ShipConnection::type_to_json_connection_hello(ConnectionHelloType *connection_hello)
{
    outgoing_json_doc.clear();
    JsonArray json_hello = outgoing_json_doc.createNestedArray("connectionHello");

    JsonObject phase = json_hello.createNestedObject();
    phase["phase"] = ConnectionHelloPhase::to_str(connection_hello->phase);

    if (connection_hello->waiting_valid) {
        JsonObject waiting = json_hello.createNestedObject();
        waiting["waiting"] = connection_hello->waiting;
    }

    if (connection_hello->prolongation_request_valid) {
        JsonObject prolongation_request = json_hello.createNestedObject();
        prolongation_request["prolongationRequest"] = connection_hello->prolongation_request;
    }

    message_outgoing->data[0] = 1;
    size_t length = serializeJson(outgoing_json_doc, &message_outgoing->data[1], SHIP_CONNECTION_MAX_JSON_SIZE - 1);
    message_outgoing->length = length + 1;

#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("T2J ConnectionHello json: %s", &message_outgoing->data[1]);
#endif
}

bool ShipConnection::json_to_type_handshake_type(ProtocolHandshakeType *handshake_type)
{
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("J2T ProtocolHandshakeType json: %s", &message_incoming->data[1]);
#endif
    incoming_json_doc.clear();
    DeserializationError error = deserializeJson(incoming_json_doc, &message_incoming->data[1], message_incoming->length - 1);
    if (error) {
        eebus.trace_fmtln("Protocolhandshake: Error during JSON deserialization: %s", error.c_str());
        return false;
    }

    JsonArray handshake_array = incoming_json_doc["messageProtocolHandshake"].as<JsonArray>();
    if (handshake_array.isNull()) {
        eebus.trace_fmtln("Protocolhandshake: Received message is not a messageProtocolHandshake message");
        return false;
    }

    for (JsonObject obj : handshake_array) {
        if (obj.containsKey("handshakeType")) {
            handshake_type->handshakeType = static_cast<ProtocolHandshake::Type>(ProtocolHandshake::from_str(obj["handshakeType"].as<String>().c_str()));
        } else if (obj.containsKey("version")) {
            for (JsonObject version : obj["version"].as<JsonArray>()) {
                if (version.containsKey("major")) {
                    handshake_type->version_major = version["major"].as<uint32_t>();
                } else if (version.containsKey("minor")) {
                    handshake_type->version_minor = version["minor"].as<uint32_t>();
                }
            }
        }
    }
    eebus.trace_fmtln("J2T ProtocolHandshakeType Type: handshakeType %d, version %ld.%ld", static_cast<std::underlying_type<ProtocolHandshake::Type>::type>(handshake_type->handshakeType), handshake_type->version_major, handshake_type->version_minor);
    return true;
}

void ShipConnection::type_to_json_handshake_type(ProtocolHandshakeType *handshake_type)
{
    outgoing_json_doc.clear();
    JsonArray json_handshake = outgoing_json_doc.createNestedArray("messageProtocolHandshake");

    JsonObject ht = json_handshake.createNestedObject();
    ht["handshakeType"] = ProtocolHandshake::to_str(handshake_type->handshakeType);

    JsonObject v = json_handshake.createNestedObject();
    JsonArray version = v.createNestedArray("version");
    JsonObject major = version.createNestedObject();
    major["major"] = handshake_type->version_major;
    JsonObject minor = version.createNestedObject();
    minor["minor"] = handshake_type->version_minor;

    JsonObject f = json_handshake.createNestedObject();
    JsonArray formats = f.createNestedArray("formats");
    JsonObject format = formats.createNestedObject();
    JsonArray format_list = format.createNestedArray("format");
    format_list.add("JSON-UTF8");

    message_outgoing->data[0] = 1;
    size_t length = serializeJson(outgoing_json_doc, &message_outgoing->data[1], SHIP_CONNECTION_MAX_JSON_SIZE - 1);
    message_outgoing->length = length + 1;

#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("T2J ProtocolHandshakeType json: %s", &message_outgoing->data[1]);
#endif
}

void ShipConnection::to_json_access_methods_type()
{
    outgoing_json_doc.clear();
    JsonArray json_am = outgoing_json_doc.createNestedArray("accessMethods");

    JsonObject access_methods = json_am.createNestedObject();
    access_methods["id"] = eebus.get_eebus_name();

    json_am.createNestedObject().createNestedArray("dnsSd_mDns");
    // Note: as of SHIP 1.0.1 this array is empty but shall be provided if the service is announced via mdns.
    // Subsequent versions may required sub elements to be provided
    // Since we are not providing our SHIP service via unicast DNS, we do not need the "dns" element

    message_outgoing->data[0] = 1;
    size_t length = serializeJson(outgoing_json_doc, &message_outgoing->data[1], SHIP_CONNECTION_MAX_JSON_SIZE - 1);
    message_outgoing->length = length + 1;

#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("T2J ShipMessageAccessMethods json: %s", &message_outgoing->data[1]);
#endif
}
