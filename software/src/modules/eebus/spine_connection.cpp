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
#include "eebus_usecases.h"
#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "ship_types.h"
#include "tools.h"

#include <algorithm>

SpineConnection::SpineConnection(ShipConnection *ship_conn)
{
    ship_connection = ship_conn;
    ack_check_timer = task_scheduler.scheduleWithFixedDelay(
        [this]() {
            check_ack_expired();
        },
        60_s); // Every 60 seconds we check for expired acks. Worst case an ack expires and it takes 119 seconds to notice.
    initial_peer_discovery_timer = task_scheduler.scheduleOnce(
        [this]() {
            if (!initial_peer_discovery_started) {
                eebus.trace_fmtln("SPINE: Peer has not initiated discovery. Starting discovery ourselves...");
                initial_peer_discovery();
            }
        },
        5_s); // If we haven't started the initial peer discovery within 5 seconds after connection, we force it. This is to attempt to move the connection into "eebus active" state. If the peer has not started discovery it is likely that they are not compatible anyway.
    eebus.trace_fmtln("New SPINE Connection created for peer %s", ship_connection->peer_node->node_name().c_str());
}
SpineConnection::~SpineConnection()
{
    task_scheduler.cancel(ack_check_timer);
    task_scheduler.cancel(update_api_timer);
    task_scheduler.cancel(initial_peer_discovery_timer);

    // Subscriptions and bindings do not survive the connection (peers re-subscribe
    // on reconnect). Purge deferred: this destructor may run while ship_connections
    // is being modified, and another live connection to the device must keep them.
    for (const FeatureAddressType &addr : known_addresses) {
        if (addr.device.isNull() || addr.device.get().empty()) {
            continue;
        }
        task_scheduler.scheduleOnce([device = addr.device.get()]() {
            if (eebus.usecases == nullptr) {
                return;
            }
            for (const auto &conn : eebus.ship.ship_connections) {
                if (conn->spine && conn->spine->knows_device(device)) {
                    return;
                }
            }
            eebus.usecases->node_management.remove_entries_for_device(device);
        });
    }
}
bool SpineConnection::process_datagram(JsonVariant datagram)
{
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("SPINE: Processing datagram:");
    eebus.trace_jsonln(datagram);
#endif
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
        eebus.trace_fmtln("SPINE: No function found for the received payload");
#ifdef EEBUS_TRACE_SUPER_VERBOSE
        eebus.trace_jsonln(received_payload);
#endif
        return false;
    }
    eebus.trace_fmtln("SPINE: Received %s %s from %s", convertToString(received_header.cmdClassifier.get()).c_str(), SpineDataTypeHandler::function_to_string(called_function).c_str(), EEBUS_USECASE_HELPERS::spine_address_to_string(received_header.addressSource.get()).c_str());
    initial_peer_discovery();
    eebus.usecases->process_spine_message(received_header, eebus.data_handler.get(), this);
    return true;
}

int SpineConnection::send_datagram(JsonVariantConst payload, CmdClassifierType cmd_classifier, const FeatureAddressType &sender, const FeatureAddressType &receiver, const bool require_ack)
{
    const char *function_name = "unknown";
    if (payload.is<JsonObjectConst>()) {
        JsonObjectConst obj = payload.as<JsonObjectConst>();
        if (obj.begin() != obj.end()) {
            function_name = obj.begin()->key().c_str();
        }
    }
    eebus.trace_fmtln("SPINE: Sending %s %s to %s", convertToString(cmd_classifier).c_str(), function_name, EEBUS_USECASE_HELPERS::spine_address_to_string(receiver).c_str());
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_jsonln(payload);
#endif

    int msg_id = msg_counter++;
    BasicJsonDocument<ArduinoJsonPsramAllocator> response_doc{payload.memoryUsage() + 512}; // Payload size + header size + some slack as recommended by arduinojson assistant
    HeaderType header{};
    header.ackRequest = require_ack;
    header.cmdClassifier = cmd_classifier;
    header.specificationVersion = SUPPORTED_SPINE_VERSION;
    header.addressSource = sender;
    if (header.addressSource->device.isNull()) {
        header.addressSource->device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    }

    header.addressDestination = receiver;
    header.msgCounter = msg_id;
    if (cmd_classifier == CmdClassifierType::reply || cmd_classifier == CmdClassifierType::result) {
        header.msgCounterReference = received_header.msgCounter; // The message counter of the last received datagram
    }
    response_doc["datagram"][0]["header"] = header;
    if (!response_doc["datagram"][1]["payload"]["cmd"][0].set(payload)) {
        eebus.trace_fmtln("SPINE: ERROR: Could not set payload for the datagram");
        msg_counter--;
        return -1;
    }
    if (require_ack) {
        ack_waiting[msg_id] = now_us() + 60_s;
    }
    ship_connection->send_data_message(response_doc.as<JsonVariant>());
    return msg_id;
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
    if (detailed_discovery_data_received && detailed_discovery_data.featureInformation.has_value()) {
        // This is called for every connection on each lookup of a connection by address, so compare without copying addresses.
        const std::string &wanted_device = *address.device;
        const std::string *peer_device = nullptr;
        bool peer_device_resolved = false;
        for (const auto &feature_info : *detailed_discovery_data.featureInformation) {
            if (!feature_info.description.has_value() || !feature_info.description->featureAddress.has_value()) {
                continue;
            }
            const FeatureAddressType &discovered = *feature_info.description->featureAddress;
            if (*discovered.feature != *address.feature || *discovered.entity != *address.entity) {
                continue;
            }
            // The device is optional in the feature addresses of the detailed discovery. If omitted, it is the device of the peer.
            const std::string *device = discovered.device.has_value() && !discovered.device->empty() ? &*discovered.device : nullptr;
            if (device == nullptr) {
                if (!peer_device_resolved) {
                    peer_device = get_peer_device_name();
                    peer_device_resolved = true;
                }
                device = peer_device;
            }
            if (device == nullptr ? wanted_device.empty() : *device == wanted_device) {
                return true;
            }
        }
    }
    // If we havent gotten a detailed discovery data, we are in inital peer discovery and the targeted address is [0]/0 we just assume the message is for us.
    std::vector<AddressEntityType> nm_ent = {0};
    if (!detailed_discovery_data_received && initial_peer_discovery_started && address.entity.get() == nm_ent && address.feature.get() == 0) {
        return true;
    }
    return false;
}

void SpineConnection::initial_peer_discovery()
{
    if (initial_peer_discovery_started)
        return;
    initial_peer_discovery_started = true;
    FeatureAddressType address{};
    address = received_header.addressSource.get();
    address.entity = {0};
    address.feature = 0;

    if (!detailed_discovery_data_received)
        eebus.usecases->node_management.send_full_read(0, address, SpineDataTypeHandler::Function::nodeManagementDetailedDiscoveryData);
    // Use case data read is deferred until we receive the discovery reply.
    // After receiving the discovery reply we first subscribe to the peer's
    // NodeManagement feature and then read the use case data as this is the behavior of e.g. eebus-go and ensure reliable connection establishment with tested peers

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
bool SpineConnection::knows_device(const std::string &device) const
{
    for (const FeatureAddressType &addr : known_addresses) {
        if (addr.device.get() == device) {
            return true;
        }
    }
    return false;
}

bool SpineConnection::is_subscribed(FeatureAddressType local, FeatureAddressType remote)
{
    for (const auto &requested : requested_subscriptions) {
        if (EEBUS_USECASE_HELPERS::compare_spine_addresses(requested.first, local) && EEBUS_USECASE_HELPERS::compare_spine_addresses(requested.second, remote)) {
            return true;
        }
    }
    if (!subscription_data_received || !subscription_data.subscriptionEntry.has_value())
        return false;
    for (const auto &subscription : subscription_data.subscriptionEntry.get()) {
        if (EEBUS_USECASE_HELPERS::compare_spine_addresses(subscription.clientAddress.get(), local) && EEBUS_USECASE_HELPERS::compare_spine_addresses(subscription.serverAddress.get(), remote)) {
            return true;
        }
    }
    return false;
}

void SpineConnection::mark_subscription_requested(const FeatureAddressType &local, const FeatureAddressType &remote)
{
    if (!is_subscribed(local, remote)) {
        requested_subscriptions.emplace_back(local, remote);
    }
}

const std::string *SpineConnection::get_peer_device_name() const
{
    const auto &device_information = detailed_discovery_data.deviceInformation;
    if (device_information.has_value() && device_information->description.has_value() && device_information->description->deviceAddress.has_value() && device_information->description->deviceAddress->device.has_value()) {
        return &*device_information->description->deviceAddress->device;
    }
    if (!known_addresses.empty() && known_addresses[0].device.has_value()) {
        return &*known_addresses[0].device;
    }
    return nullptr;
}

FeatureAddressType SpineConnection::complete_peer_feature_address(const FeatureAddressType &address) const
{
    FeatureAddressType result = address;
    if (result.device.has_value() && !result.device->empty()) {
        return result;
    }
    // The device part of a feature address may be omitted in the detailed discovery.
    // Fill it in so the address can be used as destination and for connection lookups.
    if (const std::string *device = get_peer_device_name()) {
        result.device = *device;
    }
    return result;
}

bool SpineConnection::use_case_information_matches(const UseCaseInformationDataType &usecase, const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor)
{
    if (!usecase.actor.has_value() || usecase.actor.get() != use_case_actor || !usecase.useCaseSupport.has_value()) {
        return false;
    }
    for (const auto &usecase_support : usecase.useCaseSupport.get()) {
        // useCaseAvailable is optional. Only skip the use case if the peer explicitly marks it as unavailable.
        const bool available = !usecase_support.useCaseAvailable.has_value() || usecase_support.useCaseAvailable.get();
        if (available && usecase_support.useCaseName.has_value() && usecase_support.useCaseName.get() == use_case_name) {
            return true;
        }
    }
    return false;
}

bool SpineConnection::peer_supports_use_case(const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor)
{
    if (!use_case_data_received || !use_case_data.useCaseInformation.has_value()) {
        return false;
    }
    for (const auto &usecase : use_case_data.useCaseInformation.get()) {
        if (use_case_information_matches(usecase, use_case_name, use_case_actor)) {
            return true;
        }
    }
    return false;
}

std::vector<std::vector<AddressEntityType>> SpineConnection::get_use_case_actor_entities(const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor)
{
    std::vector<std::vector<AddressEntityType>> entities{};
    if (!use_case_data_received || !use_case_data.useCaseInformation.has_value()) {
        return entities;
    }
    for (const auto &usecase : use_case_data.useCaseInformation.get()) {
        if (!use_case_information_matches(usecase, use_case_name, use_case_actor)) {
            continue;
        }
        if (!usecase.address.has_value() || !usecase.address->entity.has_value() || usecase.address->entity->empty()) {
            continue;
        }
        const std::vector<AddressEntityType> &entity = usecase.address->entity.get();
        if (std::find(entities.begin(), entities.end(), entity) == entities.end()) {
            entities.push_back(entity);
        }
    }
    return entities;
}

std::vector<FeatureAddressType> SpineConnection::get_address_of_feature(FeatureTypeEnumType feature, RoleType role, const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor)
{
    if (!detailed_discovery_data_received || !use_case_data_received) {
        eebus.trace_fmtln("SPINE: WARNING: Attempted to get a feature address without full discovery data");
        return {};
    }
    std::vector<FeatureAddressType> feature_addresses{};
    auto add_unique = [&feature_addresses](const FeatureAddressType &address) {
        for (const FeatureAddressType &existing : feature_addresses) {
            if (EEBUS_USECASE_HELPERS::compare_spine_addresses(existing, address)) {
                return;
            }
        }
        feature_addresses.push_back(address);
    };

    if (!use_case_data.useCaseInformation.has_value()) {
        return {};
    }
    for (const auto &usecase : use_case_data.useCaseInformation.get()) {
        if (!use_case_information_matches(usecase, use_case_name, use_case_actor)) {
            continue;
        }

        // SPINE TS 7.5.2: The use case functionality of the actor is accessible behind useCaseInformation.address.
        // E.g. LPC IG 3.3: An actor implements its client and server features in the same entity.
        // So only features on the actor's entity belong to this actor.
        if (usecase.address.has_value() && usecase.address->entity.has_value() && !usecase.address->entity->empty()) {
            const FeatureAddressType address = get_address_of_feature(usecase.address->entity.get(), feature, role);
            if (address.feature.has_value()) {
                add_unique(address);
            }
        } else {
            // No entity given: Any entity of the device could implement the actor.
            for (const FeatureAddressType &address : get_addresses_of_feature(feature, role)) {
                add_unique(address);
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
    if (!detailed_discovery_data.featureInformation.has_value()) {
        return {};
    }

    for (const auto &feature_info : detailed_discovery_data.featureInformation.get()) {
        if (!feature_info.description.has_value()) {
            continue;
        }
        const auto &description = feature_info.description.get();
        if (!description.featureAddress.has_value() || !description.featureAddress->entity.has_value()) {
            continue;
        }
        if (description.featureAddress->entity.get() == entity_target && description.featureType == feature && description.role == role) {
            return complete_peer_feature_address(description.featureAddress.get());
        }
    }
    return {};
}

std::vector<FeatureAddressType> SpineConnection::get_addresses_of_feature(FeatureTypeEnumType feature, RoleType role)
{
    if (!detailed_discovery_data_received) {
        eebus.trace_fmtln("SPINE: WARNING: Attempted to get a feature address without full discovery data");
        return {};
    }
    if (!detailed_discovery_data.featureInformation.has_value()) {
        return {};
    }

    std::vector<FeatureAddressType> feature_addresses{};
    for (const auto &feature_info : detailed_discovery_data.featureInformation.get()) {
        if (!feature_info.description.has_value()) {
            continue;
        }
        const auto &description = feature_info.description.get();
        if (description.featureAddress.has_value() && description.featureType == feature && description.role == role) {
            feature_addresses.push_back(complete_peer_feature_address(description.featureAddress.get()));
        }
    }
    return feature_addresses;
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
    for (auto it = ack_waiting.begin(); it != ack_waiting.end();) {
        if (deadline_elapsed(it->second)) {
            eebus.trace_fmtln("SPINE: WARNING: Acknowledgement for message counter %d not received within 60 seconds", it->first);
            it = ack_waiting.erase(it);
        } else {
            ++it;
        }
    }
}
void SpineConnection::send_use_case_read()
{
    if (use_case_data_received)
        return;
    FeatureAddressType peer_nm{};
    if (detailed_discovery_data.deviceInformation.has_value() && detailed_discovery_data.deviceInformation->description.has_value() && detailed_discovery_data.deviceInformation->description->deviceAddress.has_value()) {
        peer_nm.device = detailed_discovery_data.deviceInformation->description->deviceAddress->device;
    } else if (!known_addresses.empty()) {
        peer_nm.device = known_addresses[0].device;
    }
    peer_nm.entity = {0};
    peer_nm.feature = 0;

    eebus.usecases->node_management.send_full_read(0, peer_nm, SpineDataTypeHandler::Function::nodeManagementUseCaseData);
}

void SpineConnection::subscribe_to_peer_node_management()
{
    // Build our local NodeManagement address (entity [0], feature 0)
    FeatureAddressType local_nm{};
    local_nm.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    local_nm.entity = {0};
    local_nm.feature = 0;

    // Build the peer's NodeManagement address from the discovery data
    FeatureAddressType peer_nm{};
    if (detailed_discovery_data.deviceInformation.has_value() && detailed_discovery_data.deviceInformation->description.has_value() && detailed_discovery_data.deviceInformation->description->deviceAddress.has_value()) {
        peer_nm.device = detailed_discovery_data.deviceInformation->description->deviceAddress->device;
    } else if (!known_addresses.empty()) {
        peer_nm.device = known_addresses[0].device;
    }
    peer_nm.entity = {0};
    peer_nm.feature = 0;

    eebus.trace_fmtln("SPINE: Subscribing to peer NodeManagement feature (%s)", peer_nm.device.get().c_str());
    eebus.usecases->node_management.subscribe_to_feature(local_nm, peer_nm, FeatureTypeEnumType::NodeManagement);
}

void SpineConnection::inform_usecases_supported_functionalities()
{
    if (detailed_discovery_data_received && use_case_data_received)
        for (EebusUsecase *uc : eebus.usecases->usecase_list) {
            uc->inform_spineconnection_usecase_update(this);
        }
}

void SpineConnection::trace_use_case_data() const
{
    if (!use_case_data.useCaseInformation.has_value()) {
        eebus.trace_fmtln("SPINE: Peer announced no use cases");
        return;
    }
    for (const auto &usecase : use_case_data.useCaseInformation.get()) {
        String names;
        if (usecase.useCaseSupport.has_value()) {
            for (const auto &support : usecase.useCaseSupport.get()) {
                if (names.length() > 0) {
                    names += ", ";
                }
                names += support.useCaseName.has_value() ? support.useCaseName.get().c_str() : "?";
                if (support.useCaseAvailable.has_value() && !support.useCaseAvailable.get()) {
                    names += " (unavailable)";
                }
            }
        }
        eebus.trace_fmtln("SPINE: Peer use cases of actor %s at %s: %s", usecase.actor.has_value() ? usecase.actor.get().c_str() : "?", usecase.address.has_value() ? EEBUS_USECASE_HELPERS::spine_address_to_string(usecase.address.get()).c_str() : "?", names.c_str());
    }
}
