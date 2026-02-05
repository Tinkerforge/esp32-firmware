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

SpineConnection::SpineConnection(ShipConnection *ship_conn)
{
    ship_connection = ship_conn;
    ack_check_timer = task_scheduler.scheduleWithFixedDelay(
        [this]() {
            check_ack_expired();
        },
        60_s); // Every 60 seconds we check for expired acks. Worst case an ack expires and it takes 119 seconds to notice.
    eebus.trace_fmtln("New SPINE Connection created for peer %s", ship_connection->peer_node->node_name().c_str());
}
SpineConnection::~SpineConnection()
{
    task_scheduler.cancel(ack_check_timer);
    task_scheduler.cancel(update_api_timer);
}
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
    initial_peer_discovery();
    eebus.usecases->process_spine_message(received_header, eebus.data_handler.get(), this);
    return true;
}

void SpineConnection::send_datagram(JsonVariantConst payload, CmdClassifierType cmd_classifier, const FeatureAddressType &sender, const FeatureAddressType &receiver, const bool require_ack)
{
    eebus.trace_fmtln("SPINE: Sending datagram. cmdClassifier: %s, Content:", convertToString(cmd_classifier).c_str());
    eebus.trace_jsonln(payload);
    if (require_ack) {
        ack_waiting[msg_counter] = millis();
    }

    BasicJsonDocument<ArduinoJsonPsramAllocator> response_doc{payload.memoryUsage() + 512}; // Payload size + header size + some slack as recommended by arduinojson assistant
    HeaderType header{};
    header.ackRequest = require_ack;
    header.cmdClassifier = cmd_classifier;
    header.specificationVersion = SUPPORTED_SPINE_VERSION;
    header.addressSource = sender;

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
    ship_connection->send_data_message(response_doc.as<JsonVariant>());
}

void SpineConnection::check_message_counter()
{
    if (received_header.msgCounterReference.has_value()) {
        ack_waiting.erase(received_header.msgCounterReference.get());
    }
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
    if (detailed_discovery_data_received) {
        for (auto feature_info : detailed_discovery_data.featureInformation.get()) {
            if (feature_info.description->featureAddress.get().device.get() == address.device.get() && feature_info.description->featureAddress.get().feature.get() == address.feature.get() && feature_info.description->featureAddress.get().entity.get() == address.entity.get()) {
                return true;
            }
        }
    }
    return false;
}

void SpineConnection::initial_peer_discovery()
{
    if (initial_peer_discovery_started)
        return;
    initial_peer_discovery_started = true;
    FeatureAddressType address = received_header.addressSource.get();
    address.entity = {0};
    address.feature = 0;
    if (!detailed_discovery_data_received)
        eebus.usecases->node_management.send_full_read(0, address, SpineDataTypeHandler::Function::nodeManagementDetailedDiscoveryData);
    if (!use_case_data_received)
        eebus.usecases->node_management.send_full_read(0, address, SpineDataTypeHandler::Function::nodeManagementUseCaseData);

    update_api_timer = task_scheduler.scheduleOnce(
        [this] { // If the connection gets interrupted and removed, this might cause a crash
            if (!detailed_discovery_data_received || !use_case_data_received) {
                eebus.trace_fmtln("SPINE: WARNING: Initial peer discovery not completed for peer %s", ship_connection->peer_node->node_name().c_str());
                ship_connection->peer_node->state = NodeState::EEBUSDegraded;
                eebus.update_peers_state();
                initial_peer_discovery_started = false;
            } else {
                eebus_active(true);
            }
        },
        10_s);
}
void SpineConnection::eebus_active(bool active) const
{
    if (active) {
        if (ship_connection->peer_node->state != NodeState::EEBUSActive) {
            logger.printfln("Full EEBUS connection established to %s", ship_connection->peer_node->node_name().c_str());
        }
        ship_connection->peer_node->state = NodeState::EEBUSActive;
    } else {
        ship_connection->peer_node->state = NodeState::Connected;
    }
    eebus.update_peers_state();
}
bool SpineConnection::is_subscribed(FeatureAddressType local, FeatureAddressType remote)
{
    if (!subscription_data_received)
        return false;
    for (const auto &subscription : subscription_data.subscriptionEntry.get()) {
        if (EEBUS_USECASE_HELPERS::compare_spine_addresses(subscription.clientAddress.get(), local) && EEBUS_USECASE_HELPERS::compare_spine_addresses(subscription.serverAddress.get(), remote)) {
            return true;
        }
    }
    return false;
}
std::vector<FeatureAddressType> SpineConnection::get_address_of_feature(FeatureTypeEnumType feature, RoleType role, const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor)
{
    if (!detailed_discovery_data_received || !use_case_data_received) {
        eebus.trace_fmtln("SPINE: WARNING: Attempted to get a feature address without full discovery data");
        return {};
    }
    std::vector<FeatureAddressType> feature_addresses{};
    for (auto usecase : use_case_data.useCaseInformation.get()) {
        if (usecase.actor == use_case_actor) {
            for (auto usecase_support : usecase.useCaseSupport.get()) {
                if (usecase_support.useCaseAvailable.get() && usecase_support.useCaseName == use_case_name) {
                    for (auto feature_info : detailed_discovery_data.featureInformation.get()) {
                        if (feature_info.description->featureType == feature && feature_info.description->role == role) {
                            feature_addresses.push_back(feature_info.description->featureAddress.get());
                        }
                    }
                }
            }
        }
    }
    return feature_addresses;
}
FeatureAddressType SpineConnection::get_address_of_feature(const std::vector<AddressEntityType> &entity_target, FeatureTypeEnumType feature, RoleType role)
{
    if (!detailed_discovery_data_received) {
        eebus.trace_fmtln("SPINE: WARNING: Attempted to get a feature address without full discovery data");
        return {};
    }

    for (auto entities : detailed_discovery_data.entityInformation.get()) {
        if (entities.description->entityAddress->entity == entity_target) {
            for (auto feature_info : detailed_discovery_data.featureInformation.get()) {
                if (feature_info.description->featureType == feature && feature_info.description->role == role) {
                    return feature_info.description->featureAddress.get();
                }
            }
        }
    }
    return {};
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
void SpineConnection::check_ack_expired()
{
    if (ack_waiting.empty())
        return;
    for (const auto &[key, value] : ack_waiting) {
        if (millis() - value > 60000) { //30 seconds timeout
            eebus.trace_fmtln("SPINE: WARNING: Acknowledgement for message counter %d not received within 30 seconds", key);
            ack_waiting.erase(key);
        }
    }
}
void SpineConnection::inform_usecases_supported_functionalities()
{
    if (detailed_discovery_data_received && use_case_data_received)
        for (EebusUsecase *uc : eebus.usecases->usecase_list) {
            uc->inform_spineconnection_usecase_update(this);
        }
}
