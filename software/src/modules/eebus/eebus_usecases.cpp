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

// Disable autoformat as it moves eebus.h below eebus_usecases.h which causes linker errors
// clang-format off
#include "eebus.h"
#include "eebus_usecases.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "ship_types.h"
#include <chrono>
#include <regex>
#include <utility>

// clang-format on

template <typename T> void insert_vector(std::vector<T> &dest, const std::vector<T> &src)
{
    dest.insert(dest.end(), src.begin(), src.end());
}

EebusHeartBeat::EebusHeartBeat()
{
    heartbeat_received_timeout_task = task_scheduler.scheduleOnce(
        [this]() {
            emit_timeout();
        },
        120_s); // Add some buffer time
    // Send out heartbeat
    heartbeat_send_task = task_scheduler.scheduleWithFixedDelay(
        [this]() {
            send_heartbeat_to_subs();
        },
        heartbeat_interval);
}
DeviceDiagnosisHeartbeatDataType EebusHeartBeat::read_heartbeat()
{
    timeval time_v{};
    rtc.clock_synced(&time_v);

    DeviceDiagnosisHeartbeatDataType outgoing_heartbeatData{};
    outgoing_heartbeatData.heartbeatCounter = heartbeat_counter++;
    outgoing_heartbeatData.heartbeatTimeout = EEBUS_USECASE_HELPERS::iso_duration_to_string(heartbeat_interval);
    outgoing_heartbeatData.timestamp = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(time_v.tv_sec).c_str();
    return outgoing_heartbeatData;
}

void EebusHeartBeat::initialize_heartbeat_on_feature(FeatureAddressType &target, Usecases sending_usecase, bool expect_notify)
{
    heartbeat_targets.push_back(target);
    // Subscribe to heartbeat notifications from target
    if (expect_notify) {
        task_scheduler.scheduleOnce(
            [=, this]() mutable {
                const auto connection = EEBusUseCases::get_spine_connection(target);
                if (connection == nullptr)
                    return;
                FeatureAddressType remote_device_diag = connection->get_address_of_feature(target.entity.get(), FeatureTypeEnumType::DeviceDiagnosis, RoleType::server);
                FeatureAddressType local_client = get_feature_address(feature_addresses.at(FeatureTypeEnumType::Generic));
                if (connection->is_subscribed(local_client, remote_device_diag)) {
                    eebus.trace_fmtln("EebusHeartBeat: Already subscribed to heartbeat notifications from target device %s", EEBUS_USECASE_HELPERS::spine_address_to_string(remote_device_diag).c_str());
                    return;
                }
                eebus.trace_fmtln("EebusHeartBeat: Subscribing to heartbeat notifications from target device %s", EEBUS_USECASE_HELPERS::spine_address_to_string(remote_device_diag).c_str());
                send_full_read(feature_addresses.at(FeatureTypeEnumType::Generic), remote_device_diag, SpineDataTypeHandler::Function::deviceDiagnosisHeartbeatData);
                eebus.usecases->node_management.subscribe_to_feature(local_client, remote_device_diag, FeatureTypeEnumType::DeviceDiagnosis);
            },
            0_ms);
    }
}
void EebusHeartBeat::update_heartbeat_interval(seconds_t interval)
{
    heartbeat_interval = interval;
    task_scheduler.cancel(heartbeat_send_task);
    heartbeat_send_task = task_scheduler.scheduleOnce(
        [this]() {
            send_heartbeat_to_subs();
        },
        heartbeat_interval);
}
MessageReturn EebusHeartBeat::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    if (data->last_cmd != SpineDataTypeHandler::Function::deviceDiagnosisHeartbeatData) {
        return {false};
    }
    switch (header.cmdClassifier.get()) {
        case CmdClassifierType::read:
            eebus.trace_fmtln("EebusHeartBeat: Command identified as DeviceDiagnosisHeartbeatData with a read command");
            response["deviceDiagnosisHeartbeatData"] = read_heartbeat();
            return {true, true, CmdClassifierType::reply};
        case CmdClassifierType::notify:
        case CmdClassifierType::reply:
            emit_heartbeat_received(data->devicediagnosisheartbeatdatatype.get());
            return {true, false};
        default:
            return {false, false};
    }
}
NodeManagementDetailedDiscoveryEntityInformationType EebusHeartBeat::get_detailed_discovery_entity_information() const
{
    return {};
}
std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> EebusHeartBeat::get_detailed_discovery_feature_information() const
{
    NodeManagementDetailedDiscoveryFeatureInformationType server_feature{};
    server_feature.description->featureAddress->entity = this->entity_address;
    server_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis);
    server_feature.description->featureType = FeatureTypeEnumType::DeviceDiagnosis;
    server_feature.description->role = RoleType::server;

    FunctionPropertyType heartbeat_property{};
    heartbeat_property.function = FunctionEnumType::deviceDiagnosisHeartbeatData;
    heartbeat_property.possibleOperations->read = PossibleOperationsReadType{};
    server_feature.description->supportedFunction->push_back(heartbeat_property);

    NodeManagementDetailedDiscoveryFeatureInformationType client_feature{};
    client_feature.description->featureAddress->entity = this->entity_address;
    client_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::Generic);
    client_feature.description->featureType = FeatureTypeEnumType::Generic;
    client_feature.description->role = RoleType::client;

    FunctionPropertyType generic_heartbeat_property{};
    generic_heartbeat_property.function = FunctionEnumType::deviceDiagnosisHeartbeatData;
    client_feature.description->supportedFunction->push_back(generic_heartbeat_property);

    return {server_feature, client_feature};
}
void EebusHeartBeat::emit_timeout() const
{
    for (EebusUsecase *uc : registered_usecases) {
        uc->receive_heartbeat_timeout();
    }
}
void EebusHeartBeat::send_heartbeat_to_subs()
{
    DeviceDiagnosisHeartbeatDataType heartbeat_data = read_heartbeat();
    auto subs = eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis), heartbeat_data, "deviceDiagnosisHeartbeatData");
    eebus.trace_fmtln("heartbeat_sent to %d subscribers", subs);
}
void EebusHeartBeat::emit_heartbeat_received(DeviceDiagnosisHeartbeatDataType &heartbeat_data)
{
    task_scheduler.cancel(heartbeat_received_timeout_task);
    for (EebusUsecase *uc : registered_usecases) {
        uc->receive_heartbeat();
    }
    seconds_t timeout = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(heartbeat_data.heartbeatTimeout.get());
    heartbeat_received_timeout_task = task_scheduler.scheduleOnce(
        [this]() {
            emit_timeout();
        },
        timeout + 1_s); // Add some buffer time
}

void EebusUsecase::send_full_read(AddressFeatureType sending_feature, FeatureAddressType receiver, SpineDataTypeHandler::Function function) const
{
    String function_name = SpineDataTypeHandler::function_to_string(function);
    FeatureAddressType sender{};
    sender.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    sender.entity = this->entity_address;
    sender.feature = sending_feature;
    eebus.trace_fmtln("%s sent read of %s to target device %s", EEBUS_USECASE_HELPERS::spine_address_to_string(sender).c_str(), function_name.c_str(), EEBUS_USECASE_HELPERS::spine_address_to_string(receiver).c_str());
    ElementTagType data{};
    BasicJsonDocument<ArduinoJsonPsramAllocator> message(256);
    JsonObject dst = message.to<JsonObject>();
    dst.createNestedObject(function_name);
    eebus.usecases->send_spine_message(receiver, sender, message.as<JsonVariantConst>(), CmdClassifierType::read, true);
}
void EebusUsecase::set_feature_address(AddressFeatureType feature_address, FeatureTypeEnumType feature_type)
{
    // Setting a feature address or feature type twice is illegal behavior and should cause a crash
    for (auto pair : feature_addresses) {
        assert(pair.first != feature_type);
        assert(pair.second != feature_address);
    }
    feature_addresses[feature_type] = feature_address;
}
FeatureTypeEnumType EebusUsecase::get_feature_by_address(AddressFeatureType feature_address) const
{
    for (auto pair : feature_addresses) {
        if (pair.second == feature_address) {
            return pair.first;
        }
    }
    return FeatureTypeEnumType::EnumUndefined;
}

bool NodeManagementEntity::check_is_bound(FeatureAddressType &sending_feature, FeatureAddressType &target_feature) const
{
    for (const BindingManagementEntryDataType &binding : binding_management_entry_list_.bindingManagementEntryData.get()) {

        if (binding.clientAddress && binding.serverAddress) {
            if (binding.clientAddress->device.get() == sending_feature.device.get() && binding.serverAddress->device.get() == target_feature.device.get() && binding.clientAddress->entity.get() == sending_feature.entity.get() && binding.serverAddress->entity.get() == target_feature.entity.get() && binding.clientAddress->feature.get() == sending_feature.feature.get() && binding.serverAddress->feature.get() == target_feature.feature.get()) {
                return true; // The client is bound to the server
            }
        }
    }
    return false;
}

UseCaseInformationDataType NodeManagementEntity::get_usecase_information()
{
    return {};
    // This should never be used as the NodeManagementUsecase has no usecase information
}

MessageReturn NodeManagementEntity::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    if (header.addressDestination->feature.get() != 0)
        return {false};
    const String cmd_classifier = convertToString(header.cmdClassifier.get());
    data->function_to_string(data->last_cmd);
    switch (data->last_cmd) {
        case SpineDataTypeHandler::Function::nodeManagementUseCaseData:
            eebus.trace_fmtln("NodeManagementUsecase: Command identified as NodeManagementUseCaseData with a %s command", cmd_classifier.c_str());
            switch (header.cmdClassifier.get()) {
                case CmdClassifierType::read:
                    response["nodeManagementUseCaseData"] = get_usecase_data();
                    return {true, true, CmdClassifierType::reply};
                case CmdClassifierType::reply:
                case CmdClassifierType::notify:
                    if (const auto conn = EEBusUseCases::get_spine_connection(header.addressSource.get())) {
                        conn->update_use_case_data(data->nodemanagementusecasedatatype.get());
                    }
                    return {true, false};
                default:
                    eebus.trace_fmtln("NodeManagementUsecase: NodeManagementUsecaseData does not support a %s command", cmd_classifier.c_str());
                    // Maybe send a response in this case?
                    return {true, false};
            }
        case SpineDataTypeHandler::Function::nodeManagementDetailedDiscoveryData:
            eebus.trace_fmtln("NodeManagementUsecase: Command identified as NodeManagementDetailedDiscoveryData with a %s command", cmd_classifier.c_str());
            switch (header.cmdClassifier.get()) {
                case CmdClassifierType::read:
                    response["nodeManagementDetailedDiscoveryData"] = get_detailed_discovery_data();
                    //send_detailed_discovery_read(header.addressSource.get());
                    return {true, true, CmdClassifierType::reply};
                case CmdClassifierType::reply:
                case CmdClassifierType::notify:
                    eebus.trace_fmtln("Got a reply to a NodeManagementDetailedDiscoveryData read command as expected");
                    if (const auto conn = EEBusUseCases::get_spine_connection(header.addressSource.get())) {
                        conn->update_detailed_discovery_data(data->nodemanagementdetaileddiscoverydatatype.get());
                    }
                    return {true, false};
                default:
                    eebus.trace_fmtln("NodeManagementUsecase: NodeManagementDetailedDiscoveryData does not support a %s command", cmd_classifier.c_str());
                    return {true, false};
            }

        case SpineDataTypeHandler::Function::nodeManagementSubscriptionData:
        case SpineDataTypeHandler::Function::nodeManagementSubscriptionRequestCall:
        case SpineDataTypeHandler::Function::nodeManagementSubscriptionDeleteCall:
            if (header.cmdClassifier.get() == CmdClassifierType::reply || header.cmdClassifier.get() == CmdClassifierType::notify) {
                if (const auto conn = EEBusUseCases::get_spine_connection(header.addressSource.get())) {
                    conn->update_subscription_data(data->nodemanagementsubscriptiondatatype.get());
                }
                return {true, false};
            }
            eebus.trace_fmtln("NodeManagementUsecase: Command identified as Subscription handling");
            return handle_subscription(header, data, response);

        case SpineDataTypeHandler::Function::nodeManagementBindingData:
        case SpineDataTypeHandler::Function::nodeManagementBindingRequestCall:
        case SpineDataTypeHandler::Function::nodeManagementBindingDeleteCall:
            eebus.trace_fmtln("NodeManagementUsecase: Command identified as Binding handling");
            return handle_binding(header, data, response);
        default:
            return {false};
    }
}

bool NodeManagementEntity::subscribe_to_feature(FeatureAddressType &sending_feature, FeatureAddressType &target_feature, FeatureTypeEnumType feature) const
{
    NodeManagementSubscriptionRequestCallType subscription_request{};
    subscription_request.subscriptionRequest->clientAddress = sending_feature;
    subscription_request.subscriptionRequest->serverAddress = target_feature;
    if (feature != FeatureTypeEnumType::EnumUndefined) {
        subscription_request.subscriptionRequest->serverFeatureType = feature;
    }
    FeatureAddressType sender{};
    sender.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    sender.entity = this->entity_address;
    sender.feature = 0;
    FeatureAddressType target{};
    target.feature = 0;
    target.entity = {0};
    target.device = target_feature.device;
    eebus.trace_fmtln("NodeManagementUsecase: subscribe_to_feature");
    BasicJsonDocument<ArduinoJsonPsramAllocator> message(512);
    JsonObject dst = message.to<JsonObject>();
    if (dst["nodeManagementSubscriptionRequestCall"].set(subscription_request)) {

        eebus.trace_fmtln("NodeManagementUsecase: Built subscription request message successfully");
    } else {
        eebus.trace_fmtln("NodeManagementUsecase: Failed to build subscription request message");
    }

    return eebus.usecases->send_spine_message(target, sender, message.as<JsonVariantConst>(), CmdClassifierType::call, true);
}
void NodeManagementEntity::detailed_discovery_update()
{
    auto detailed_data = get_detailed_discovery_data();
    this->inform_subscribers(entity_address, nodemgmt_feature_address, detailed_data, "nodeManagementDetailedDiscoveryData");
}

NodeManagementUseCaseDataType NodeManagementEntity::get_usecase_data() const
{
    NodeManagementUseCaseDataType node_management_usecase_data;
    for (EebusUsecase *uc : usecase_interface->usecase_list) {
        if (uc->get_usecase_type() != Usecases::NMC && uc->get_usecase_type() != Usecases::HEARTBEAT) {
            node_management_usecase_data.useCaseInformation->push_back(uc->get_usecase_information());
        }
    }
    return node_management_usecase_data;
}

NodeManagementDetailedDiscoveryDataType NodeManagementEntity::get_detailed_discovery_data() const
{
    // Detailed discovery as defined in EEBus SPINE TS ProtocolSpecification 7.1.2
    NodeManagementDetailedDiscoveryDataType node_management_detailed_data = {};
    node_management_detailed_data.specificationVersionList->specificationVersion->emplace_back(SUPPORTED_SPINE_VERSION);

    node_management_detailed_data.deviceInformation->description->description = std::string(OPTIONS_PRODUCT_NAME()) + " by " + OPTIONS_MANUFACTURER_FULL(); // Optional. Shall not be longer than 4096 characters.
    node_management_detailed_data.deviceInformation->description->label = OPTIONS_PRODUCT_NAME();
    // Optional. Shall not be longer than 256 characters.
    node_management_detailed_data.deviceInformation->description->networkFeatureSet = NetworkManagementFeatureSetType::simple;
    // Only simple operation is supported. We dont act as a SPINE router or anything like that.
    node_management_detailed_data.deviceInformation->description->deviceAddress->device = EEBUS_USECASE_HELPERS::get_spine_device_name();
#ifdef EEBUS_MODE_EVSE
    node_management_detailed_data.deviceInformation->description->deviceType = DeviceTypeEnumType::ChargingStation; // Mandatory. String defined in EEBUS SPINE TS ResourceSpecification 4.1
#endif
#ifdef EEBUS_MODE_EM
    node_management_detailed_data.deviceInformation->description->deviceType = DeviceTypeEnumType::EnergyManagementSystem; // Mandatory. String defined in EEBUS SPINE TS ResourceSpecification 4.1
#endif
    for (EebusUsecase *uc : usecase_interface->usecase_list) {
        if (!uc->isActive()) {
            continue;
        }
        bool add_entity_info = true;
        auto entity_info = uc->get_detailed_discovery_entity_information();
        for (auto &existing_entity_info : *(node_management_detailed_data.entityInformation)) {
            // If the entity type matches, we do not add it again and if it has no entity type defined it is inactive
            if (entity_info.description->entityType.has_value() && existing_entity_info.description->entityType == entity_info.description->entityType.get() && entity_info.description->entityAddress->entity.get() == existing_entity_info.description->entityAddress->entity) {
                add_entity_info = false;
                break;
            }
        }

        if (add_entity_info && entity_info.description->entityType.has_value()) {
            node_management_detailed_data.entityInformation->push_back(uc->get_detailed_discovery_entity_information());
        }
        auto features = uc->get_detailed_discovery_feature_information();

        for (auto feature : features) {
            bool has_feature = false;
            for (auto &i : *node_management_detailed_data.featureInformation) {
                if (i.description->featureAddress->feature == feature.description->featureAddress->feature.get() && i.description->featureAddress->entity == feature.description->featureAddress->entity.get()) {
                    has_feature = true;
                    for (auto function : feature.description->supportedFunction.get()) {
                        i.description->supportedFunction->push_back(function);
                        // TODO: Check if this function already exists
                    }
                }
            }
            if (!has_feature) {
                node_management_detailed_data.featureInformation->push_back(feature);
            }
        }
    }
    return node_management_detailed_data;
}

MessageReturn NodeManagementEntity::handle_subscription(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionRequestCall && header.cmdClassifier == CmdClassifierType::call) {
        if (!data->nodemanagementsubscriptionrequestcalltype || !data->nodemanagementsubscriptionrequestcalltype->subscriptionRequest || !data->nodemanagementsubscriptionrequestcalltype->subscriptionRequest->clientAddress || !data->nodemanagementsubscriptionrequestcalltype->subscriptionRequest->serverAddress) {
            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Subscription request failed, no or invalid subscription request data provided");
            return {true, true, CmdClassifierType::result};
        }
        NodeManagementSubscriptionRequestCallType request = data->nodemanagementsubscriptionrequestcalltype.get();
        SubscriptionManagementEntryDataType entry{};
        entry.clientAddress = request.subscriptionRequest->clientAddress;
        entry.serverAddress = request.subscriptionRequest->serverAddress;

        // Check if the subscription already exists. We do not send an error and instead fail semi-silently.
        for (SubscriptionManagementEntryDataType &old_entry : subscription_data.subscriptionEntry.get()) {
            if (old_entry.clientAddress->device.get() == request.subscriptionRequest->clientAddress->device && old_entry.clientAddress->entity.get() == request.subscriptionRequest->clientAddress->entity && old_entry.clientAddress->feature.get() == request.subscriptionRequest->clientAddress->feature && old_entry.serverAddress->device.get() == request.subscriptionRequest->serverAddress->device && old_entry.serverAddress->entity.get() == request.subscriptionRequest->serverAddress->entity
                && old_entry.serverAddress->feature.get() == request.subscriptionRequest->serverAddress->feature) {
                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "Subscription request was successful (already subscribed)");
                return {true, true, CmdClassifierType::result};
            }
        }

        subscription_data.subscriptionEntry->push_back(entry);
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "Subscription request was successful");
        eebus.trace_fmtln("%s successfully subscribed to %s. Featuretype: %s", EEBUS_USECASE_HELPERS::spine_address_to_string(entry.clientAddress.get()).c_str(), EEBUS_USECASE_HELPERS::spine_address_to_string(entry.serverAddress.get()).c_str(), convertToString(request.subscriptionRequest->serverFeatureType.get()).c_str());
        return {true, true, CmdClassifierType::result};
    }
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionData && header.cmdClassifier == CmdClassifierType::read) {
        response["nodeManagementSubscriptionData"] = subscription_data;
        return {true, true, CmdClassifierType::reply};
    }
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionDeleteCall) {
        NodeManagementSubscriptionDeleteCallType subscription_delete_call = data->nodemanagementsubscriptiondeletecalltype.get();
        std::vector<size_t> to_delete_indices{};

        // Iterate throught the list of subscriptions and find the ones that match the delete request
        for (size_t i = 0; i < subscription_data.subscriptionEntry.get().size(); ++i) {
            SubscriptionManagementEntryDataType entry = subscription_data.subscriptionEntry->at(i);

            // Compares two optionals. If the one has no value, its considered a wildcard and matches anything in the second. If both have value, they are compared and the result returnd
            auto optional_equal_or_undefined = [](const auto &a, const auto &b) {
                if (!a.has_value() || !b.has_value())
                    return true;
                return *a == *b;
            };

            // We handle cases where the client or server address is not set which is allowed
            FeatureAddressType client_address = subscription_delete_call.subscriptionDelete->clientAddress.get();
            FeatureAddressType server_address = subscription_delete_call.subscriptionDelete->serverAddress.get();
            if (!subscription_delete_call.subscriptionDelete->clientAddress->device && subscription_delete_call.subscriptionDelete->serverAddress->device) {
                // This implies the client is referencing its own and the servers name as device names
                client_address.device = header.addressSource->device.get();
                server_address.device = header.addressDestination->device.get();
            } else if (subscription_delete_call.subscriptionDelete->clientAddress->device && !subscription_delete_call.subscriptionDelete->serverAddress->device) {
                server_address.device = header.addressSource->device.get();
            } else if (!subscription_delete_call.subscriptionDelete->clientAddress->device && subscription_delete_call.subscriptionDelete->serverAddress->device) {
                client_address.device = header.addressDestination->device.get();
            }
            // If the device does not match to the entry we skip it
            if (client_address.device.get() != entry.clientAddress->device.get() && server_address.device.get() != entry.serverAddress->device.get()) {
                continue;
            }

            // This handles all the cases.
            // If a value of client_address or server_address is empty it is considered a wildcard and anything matches, if it has a value it is compared
            if (optional_equal_or_undefined(client_address.entity, entry.clientAddress->entity) && optional_equal_or_undefined(server_address.entity, entry.serverAddress->entity) && optional_equal_or_undefined(client_address.feature, entry.clientAddress->feature) && optional_equal_or_undefined(server_address.feature, entry.serverAddress->feature)) {
                to_delete_indices.push_back(i);
                continue;
            }
        }

        // delete all the found entries
        std::sort(to_delete_indices.rbegin(), to_delete_indices.rend()); // sort descending
        for (size_t i : to_delete_indices) {
            if (i < subscription_data.subscriptionEntry.get().size()) {
                // remove the element at i starting from the back. Always do it relative to the beginning so the indices stay valid
                subscription_data.subscriptionEntry.get().erase(subscription_data.subscriptionEntry.get().begin() + i);
            }
        }
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "Removed Subscriptions successfully");
        return {true, true, CmdClassifierType::reply};
    }

    return {false};
}

MessageReturn NodeManagementEntity::handle_binding(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    // Binding Request as defined in EEBus SPINE TS ProtocolSpecification 7.3.2
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingRequestCall) {
        if (data->nodemanagementbindingrequestcalltype && data->nodemanagementbindingrequestcalltype->bindingRequest && data->nodemanagementbindingrequestcalltype->bindingRequest->clientAddress && data->nodemanagementbindingrequestcalltype->bindingRequest->serverAddress) {
            BindingManagementEntryDataType binding_entry;
            binding_entry.clientAddress = data->nodemanagementbindingrequestcalltype->bindingRequest->clientAddress;
            binding_entry.serverAddress = data->nodemanagementbindingrequestcalltype->bindingRequest->serverAddress;
            // We are supposed consider the featuretype of the feature
            //SpineOptional<FeatureTypeEnumType> feature_type = data->nodemanagementbindingrequestcalltype->bindingRequest->serverFeatureType;
            if (check_is_bound(binding_entry.clientAddress.get(), binding_entry.serverAddress.get()) && eebus.usecases->get_spine_connection(header.addressSource.get()) != nullptr) {
                eebus.trace_fmtln("Binding requested but is already bound");
            } else {
                binding_entry.bindingId = binding_management_entry_list_.bindingManagementEntryData->size();
                binding_management_entry_list_.bindingManagementEntryData->push_back(binding_entry);
            }
            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "Binding request was successful");
            return {true, true, CmdClassifierType::reply};
        }
        eebus.trace_fmtln("Binding requested but failed");
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Binding request failed");
        return {true, true, CmdClassifierType::reply};
    }
    // Binding Data as defined in EEBus SPINE TS ProtocolSpecification 7.3.3
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingData) {
        NodeManagementBindingDataType binding_data;
        binding_data.bindingEntry = binding_management_entry_list_.bindingManagementEntryData;
        response["nodeManagementBindingData"] = binding_data;
        eebus.trace_fmtln("List of bindings was requested");
        return {true, true, CmdClassifierType::reply};
    }
    // Binding Release as defined in EEBus SPINE TS ProtocolSpecification 7.3.4
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingDeleteCall) {
        if (!data->nodemanagementbindingdeletecalltype && data->nodemanagementbindingdeletecalltype->bindingDelete && data->nodemanagementbindingdeletecalltype->bindingDelete->clientAddress && data->nodemanagementbindingdeletecalltype->bindingDelete->serverAddress) {
            eebus.trace_fmtln("A binding release was requested but no binding delete information was provided or request was malformed");
            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Binding release failed");
            return {true, true, CmdClassifierType::reply};
        };

        // Compares two optionals. If the one has no value, its considered a wildcard and matches anything in the second. If both have value, they are compared and the result returnd
        auto optional_equal_or_undefined = [](const auto &a, const auto &b) {
            if (!a.has_value() || !b.has_value())
                return true;
            return *a == *b;
        };
        NodeManagementBindingDeleteCallType binding_delete_call = data->nodemanagementbindingdeletecalltype.get();
        std::vector<size_t> to_delete_indices{};

        // Iterate throught the list of bindings and find the ones that match the delete request
        for (size_t i = 0; i < binding_management_entry_list_.bindingManagementEntryData->size(); ++i) {
            BindingManagementEntryDataType entry = binding_management_entry_list_.bindingManagementEntryData->at(i);

            // We handle cases where the client or server address is not set which is allowed
            FeatureAddressType client_address = binding_delete_call.bindingDelete->clientAddress.get();
            FeatureAddressType server_address = binding_delete_call.bindingDelete->serverAddress.get();
            if (!binding_delete_call.bindingDelete->clientAddress->device && binding_delete_call.bindingDelete->serverAddress->device) {
                // This implies the client is referencing its own and the servers name as device names
                client_address.device = header.addressSource->device.get();
                server_address.device = header.addressDestination->device.get();
            } else if (binding_delete_call.bindingDelete->clientAddress->device && !binding_delete_call.bindingDelete->serverAddress->device) {
                server_address.device = header.addressSource->device.get();
            } else if (!binding_delete_call.bindingDelete->clientAddress->device && binding_delete_call.bindingDelete->serverAddress->device) {
                client_address.device = header.addressDestination->device.get();
            }
            // If the device does not match to the entry we skip it
            if (client_address.device.get() != entry.clientAddress->device.get() && server_address.device.get() != entry.serverAddress->device.get()) {
                continue;
            }

            // This handles all the cases.
            // If a value of client_address or server_address is empty it is considered a wildcard and anything matches, if it has a value it is compared
            if (optional_equal_or_undefined(client_address.entity, entry.clientAddress->entity) && optional_equal_or_undefined(server_address.entity, entry.serverAddress->entity) && optional_equal_or_undefined(client_address.feature, entry.clientAddress->feature) && optional_equal_or_undefined(server_address.feature, entry.serverAddress->feature)) {
                to_delete_indices.push_back(i);
                continue;
            }
        }

        // delete all the found entries
        std::sort(to_delete_indices.rbegin(), to_delete_indices.rend()); // sort descending
        for (size_t i : to_delete_indices) {
            if (i < binding_management_entry_list_.bindingManagementEntryData.get().size()) {
                // remove the element at i starting from the back. Always do it relative to the beginning so the indices stay valid
                binding_management_entry_list_.bindingManagementEntryData.get().erase(binding_management_entry_list_.bindingManagementEntryData.get().begin() + i);
            }
        }
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "Removed bindings successfully");
        return {true, true, CmdClassifierType::reply};
    }

    return {false};
}

NodeManagementDetailedDiscoveryEntityInformationType NodeManagementEntity::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity = {};
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::DeviceInformation;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.7
    entity.description->label = "Node Management"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> NodeManagementEntity::get_detailed_discovery_feature_information() const
{
    NodeManagementDetailedDiscoveryFeatureInformationType feature = {};
    feature.description->featureAddress->entity = entity_address;
    feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::NodeManagement);
    feature.description->featureType = FeatureTypeEnumType::NodeManagement;
    // The feature type as defined in EEBUS SPINE TS ResourceSpecification 4.3.19
    feature.description->role = RoleType::special;

    // The following functions are supported by the Nodemanagement feature
    // Basic Usecase information
    FunctionPropertyType useCaseData = {};
    useCaseData.function = FunctionEnumType::nodeManagementUseCaseData;
    useCaseData.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(useCaseData);

    // Detailed discovery information
    FunctionPropertyType detailedDiscoveryData = {};
    detailedDiscoveryData.function = FunctionEnumType::nodeManagementDetailedDiscoveryData;
    detailedDiscoveryData.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(detailedDiscoveryData);

    // Information about current bindings
    FunctionPropertyType nodemanagementBindingData = {};
    nodemanagementBindingData.function = FunctionEnumType::nodeManagementBindingData;
    nodemanagementBindingData.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(nodemanagementBindingData);

    // Binding delete calls
    FunctionPropertyType nodemanagementBindingDelete = {};
    nodemanagementBindingDelete.function = FunctionEnumType::nodeManagementBindingDeleteCall;
    nodemanagementBindingDelete.possibleOperations.emplace();
    feature.description->supportedFunction->push_back(nodemanagementBindingDelete);

    // Binding request calls
    FunctionPropertyType nodemanagementBindingRequest = {};
    nodemanagementBindingRequest.function = FunctionEnumType::nodeManagementBindingRequestCall;
    nodemanagementBindingRequest.possibleOperations.emplace();
    feature.description->supportedFunction->push_back(nodemanagementBindingRequest);

    // Information about current Subscriptions
    FunctionPropertyType nodemanagemntSubscriptionData{};
    nodemanagemntSubscriptionData.function = FunctionEnumType::nodeManagementSubscriptionData;
    nodemanagemntSubscriptionData.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(nodemanagemntSubscriptionData);

    // Subscription delete calls
    FunctionPropertyType nodemanagementSubscriptionDelete{};
    nodemanagementSubscriptionDelete.function = FunctionEnumType::nodeManagementSubscriptionDeleteCall;
    nodemanagementSubscriptionDelete.possibleOperations.emplace();
    feature.description->supportedFunction->push_back(nodemanagementSubscriptionDelete);

    // Subscription request calls
    FunctionPropertyType nodemanagementSubscriptionRequest{};
    nodemanagementSubscriptionRequest.function = FunctionEnumType::nodeManagementSubscriptionRequestCall;
    nodemanagementSubscriptionRequest.possibleOperations.emplace();
    feature.description->supportedFunction->push_back(nodemanagementSubscriptionRequest);

    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;
    features.push_back(feature);
    return features;
}

template <typename T> size_t NodeManagementEntity::inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, const T data, const char *function_name)
{
    eebus.trace_fmtln("EEBUS: Informing subscribers of %s", function_name);
    if (!EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS || subscription_data.subscriptionEntry.isNull() || subscription_data.subscriptionEntry.get().empty()) {
        return 0;
    }
    size_t sent_count = 0;
    size_t error_count = 0;
    for (SubscriptionManagementEntryDataType &subscription : subscription_data.subscriptionEntry.get()) {
        if (subscription.serverAddress->entity == entity && subscription.serverAddress->feature == feature) {
            if (usecase_interface->send_spine_message(subscription.clientAddress.get(), subscription.serverAddress.get(), data, CmdClassifierType::notify, function_name, false)) {
                sent_count++;
            } else {
                error_count++;
            }
        }
    }
    eebus.trace_fmtln("EEBUS: Informed %d subscribers of %s, got %d errors", sent_count, function_name, error_count);
    return sent_count;
};

NodeManagementEntity::NodeManagementEntity()
{
    subscription_data.subscriptionEntry.emplace();
}

void NodeManagementEntity::set_usecaseManager(EEBusUseCases *new_usecase_interface)
{
    usecase_interface = new_usecase_interface;
}

// ==============================================================================
// EVCS - EV Charging Summary Use Case
// Spec: EEBus_UC_TS_EVChargingSummary_V1.0.1.pdf
//
// Scenario 1 (2.3.1, 3.4.1): Energy Broker sends charging session summary
//
// NOTE: Write support for billListData is NOT YET IMPLEMENTED (see line 751).
//       This is REQUIRED by spec 3.2.1.2.1.3 for receiving billing data from Energy Broker.
// ==============================================================================
#ifdef EEBUS_ENABLE_EVCS_USECASE
EvcsUsecase::EvcsUsecase()
{
    update_api();
}

UseCaseInformationDataType EvcsUsecase::get_usecase_information()
{
    UseCaseInformationDataType evcs_usecase;
    evcs_usecase.actor = "EVSE"; // Actor can be EVSE or Energy Broker; we implement EVSE role

    UseCaseSupportType evcs_usecase_support;
    evcs_usecase_support.useCaseName = "evChargingSummary";
    evcs_usecase_support.useCaseVersion = "1.0.1";
    evcs_usecase_support.scenarioSupport->push_back(1); // Scenario 1 (2.3.1): Charging summary exchange
    evcs_usecase_support.useCaseDocumentSubRevision = "release";
    evcs_usecase.useCaseSupport->push_back(evcs_usecase_support);

    FeatureAddressType evcs_usecase_feature_address;
    evcs_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    evcs_usecase_feature_address.entity = entity_address;
    evcs_usecase.address = evcs_usecase_feature_address;
    return evcs_usecase;
}

MessageReturn EvcsUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    if (header.addressDestination->feature.has_value() && header.addressDestination->feature == feature_addresses.at(FeatureTypeEnumType::Bill)) {
        switch (data->last_cmd) {
            // 3.2.1.2.1.1: Function "billDescriptionListData" - Bill metadata
            case SpineDataTypeHandler::Function::billDescriptionListData: {
                switch (header.cmdClassifier.get()) {
                    case CmdClassifierType::read: {
                        response["billDescriptionListData"] = EVSEEntity::get_bill_description_list_data();
                        return {true, false, CmdClassifierType::reply};
                    }
                    default:
                        return {false};
                }
            }
            // 3.2.1.2.1.2: Function "billConstraintsListData" - Bill position constraints
            case SpineDataTypeHandler::Function::billConstraintsListData: {
                switch (header.cmdClassifier.get()) {
                    case CmdClassifierType::read: {
                        response["billConstraintsListData"] = EVSEEntity::get_bill_constraints_list_data();
                        return {true, true, CmdClassifierType::reply};
                    }
                    default:
                        return {false};
                }
            }
            // 3.2.1.2.1.3: Function "billListData" - Actual billing data
            case SpineDataTypeHandler::Function::billListData: {
                switch (header.cmdClassifier.get()) {
                    case CmdClassifierType::read: {
                        response["billListData"] = EVSEEntity::get_bill_list_data();
                        return {true, true, CmdClassifierType::reply};
                    }
                    case CmdClassifierType::write: {
                        // Implements write handler with MANDATORY partial write support per spec line 649

                        if (!data->billlistdatatype.has_value() || !data->billlistdatatype->billData.has_value()) {
                            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Invalid bill data");
                            return {true, true, CmdClassifierType::result};
                        }

                        // Check if this is a partial write operation (MANDATORY per spec)
                        bool is_partial = false;
                        if (data->cmdcontroltype.has_value() && data->cmdcontroltype->partial.has_value()) {
                            is_partial = data->cmdcontroltype->partial.has_value();
                        }

                        // If not partial, clear all existing bills (full replacement)
                        if (!is_partial) {
                            for (auto &entry : bill_entries) {
                                entry.id = 0; // Mark as unused
                            }
                        }

                        // Process each incoming bill
                        for (const auto &incoming_bill : data->billlistdatatype->billData.get()) {
                            // Validate billType
                            if (incoming_bill.billType.get() != BillTypeEnumType::chargingSummary) {
                                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Only chargingSummary billType is supported");
                                return {true, true, CmdClassifierType::result};
                            }

                            // Validate billId range (1-8)
                            int bill_id = incoming_bill.billId.get();
                            if (bill_id < 1 || bill_id > 8) {
                                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "billId must be between 1 and 8");
                                return {true, true, CmdClassifierType::result};
                            }

                            int array_idx = bill_id - 1;

                            // Parse total section
                            if (!incoming_bill.total.has_value() || !incoming_bill.total->timePeriod.has_value()) {
                                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Missing total section or time period");
                                return {true, true, CmdClassifierType::result};
                            }

                            time_t start_time = 0, end_time = 0;
                            if (incoming_bill.total->timePeriod->startTime.has_value()) {
                                EEBUS_USECASE_HELPERS::iso_timestamp_to_unix(incoming_bill.total->timePeriod->startTime->c_str(), &start_time);
                            }
                            if (incoming_bill.total->timePeriod->endTime.has_value()) {
                                EEBUS_USECASE_HELPERS::iso_timestamp_to_unix(incoming_bill.total->timePeriod->endTime->c_str(), &end_time);
                            }

                            // Parse total energy (Wh)
                            int energy_wh = 0;
                            if (incoming_bill.total->value.has_value() && !incoming_bill.total->value->empty()) {
                                for (const auto &val : incoming_bill.total->value.get()) {
                                    if (val.unit.has_value() && val.unit.get() == UnitOfMeasurementEnumType::Wh && val.value.has_value()) {
                                        energy_wh = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(val.value.get());
                                        break;
                                    }
                                }
                            }

                            // Parse total cost (EUR cents)
                            uint32_t cost_eur_cent = 0;
                            if (incoming_bill.total->cost.has_value() && !incoming_bill.total->cost->empty()) {
                                for (const auto &cost : incoming_bill.total->cost.get()) {
                                    if (cost.currency.has_value() && cost.currency.get() == CurrencyEnumType::EUR && cost.cost.has_value()) {
                                        // Cost is in ScaledNumber format, typically with scale=-2 (cents)
                                        int cost_value = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(cost.cost.get());
                                        // Adjust for scale: if scale=-2, value is already in cents
                                        int scale = cost.cost->scale.has_value() ? cost.cost->scale.get() : 0;
                                        if (scale == -2) {
                                            cost_eur_cent = cost_value;
                                        } else if (scale == 0) {
                                            cost_eur_cent = cost_value * 100; // Convert EUR to cents
                                        } else {
                                            cost_eur_cent = static_cast<uint32_t>(cost_value * std::pow(10.0, -scale - 2));
                                        }
                                        break;
                                    }
                                }
                            }

                            // Parse positions (MANDATORY: grid and self-produced)
                            int grid_energy_percent = 0, grid_cost_percent = 0;
                            int self_produced_energy_percent = 0, self_produced_cost_percent = 0;

                            if (incoming_bill.position.has_value()) {
                                for (const auto &pos : incoming_bill.position.get()) {
                                    if (!pos.positionType.has_value())
                                        continue;

                                    if (pos.positionType.get() == BillPositionTypeEnumType::gridElectricEnergy) {
                                        // Position 1: Grid energy
                                        if (pos.value.has_value() && !pos.value->empty()) {
                                            for (const auto &val : pos.value.get()) {
                                                if (val.valuePercentage.has_value()) {
                                                    grid_energy_percent = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(val.valuePercentage.get());
                                                }
                                            }
                                        }
                                        if (pos.cost.has_value() && !pos.cost->empty()) {
                                            for (const auto &cost : pos.cost.get()) {
                                                if (cost.costPercentage.has_value()) {
                                                    grid_cost_percent = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(cost.costPercentage.get());
                                                }
                                            }
                                        }
                                    } else if (pos.positionType.get() == BillPositionTypeEnumType::selfProducedElectricEnergy) {
                                        // Position 2: Self-produced energy
                                        if (pos.value.has_value() && !pos.value->empty()) {
                                            for (const auto &val : pos.value.get()) {
                                                if (val.valuePercentage.has_value()) {
                                                    self_produced_energy_percent = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(val.valuePercentage.get());
                                                }
                                            }
                                        }
                                        if (pos.cost.has_value() && !pos.cost->empty()) {
                                            for (const auto &cost : pos.cost.get()) {
                                                if (cost.costPercentage.has_value()) {
                                                    self_produced_cost_percent = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(cost.costPercentage.get());
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // Validate percentages sum to 100% (spec requirement)
                            if (grid_energy_percent + self_produced_energy_percent != 100) {
                                eebus.trace_fmtln("EVCS Warning: Energy percentages do not sum to 100%% (grid: %d%%, self: %d%%)", grid_energy_percent, self_produced_energy_percent);
                            }
                            if (grid_cost_percent + self_produced_cost_percent != 100) {
                                eebus.trace_fmtln("EVCS Warning: Cost percentages do not sum to 100%% (grid: %d%%, self: %d%%)", grid_cost_percent, self_produced_cost_percent);
                            }

                            // Store bill entry
                            bill_entries[array_idx].id = bill_id;
                            bill_entries[array_idx].start_time = start_time;
                            bill_entries[array_idx].end_time = end_time;
                            bill_entries[array_idx].energy_wh = energy_wh;
                            bill_entries[array_idx].cost_eur_cent = cost_eur_cent;
                            bill_entries[array_idx].grid_energy_percent = grid_energy_percent;
                            bill_entries[array_idx].grid_cost_percent = grid_cost_percent;
                            bill_entries[array_idx].self_produced_energy_percent = self_produced_energy_percent;
                            bill_entries[array_idx].self_produced_cost_percent = self_produced_cost_percent;
                        }

                        // Update API state
                        update_api();

                        // Notify subscribers (with partial flag if applicable)
                        BillListDataType bill_list_data = EVSEEntity::get_bill_list_data();
                        eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::Bill), bill_list_data, "billListData");

                        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
                        return {true, true, CmdClassifierType::result};
                    }
                    default:;
                }
            }
            default:;
        }
    }

    return {false};
}

NodeManagementDetailedDiscoveryEntityInformationType EvcsUsecase::get_detailed_discovery_entity_information() const
{

    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EVSE;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "Charging Summary"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> EvcsUsecase::get_detailed_discovery_feature_information() const
{
    NodeManagementDetailedDiscoveryFeatureInformationType feature{};

    feature.description->featureAddress->entity = entity_address;
    feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::Bill);

    feature.description->featureType = FeatureTypeEnumType::Bill;
    // The feature type as defined in EEBUS SPINE TS ResourceSpecification 4.3.19
    feature.description->role = RoleType::server;

    // The following functions are needed by the ChargingSummary usecase
    // Bill description information
    FunctionPropertyType billDescriptionList{};
    billDescriptionList.function = FunctionEnumType::billDescriptionListData;
    billDescriptionList.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(billDescriptionList);

    // Bill constraints information
    FunctionPropertyType billconstraints{};
    billconstraints.function = FunctionEnumType::billConstraintsListData;
    billconstraints.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(billconstraints);

    // Bill list information
    FunctionPropertyType billListData{};
    billListData.function = FunctionEnumType::billListData;
    billListData.possibleOperations->read = PossibleOperationsReadType{};
    billListData.possibleOperations->write = PossibleOperationsWriteType{};
    feature.description->supportedFunction->push_back(billListData);

    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;
    features.push_back(feature);
    return features;
}

void EvcsUsecase::update_billing_data(int id, time_t start_time, time_t end_time, int energy_wh, uint32_t cost_eur_cent, int grid_energy_percent, int grid_cost_percent, int self_produced_energy_percent, int self_produced_cost_percent)
{
    if (id > 8 || id < 1) {
        eebus.trace_fmtln("Billing ID %d is out of range. Must be between 1 and 8", id);
        return;
    }
    int array_ref = id - 1;

    bill_entries[array_ref].id = id;
    bill_entries[array_ref].start_time = start_time;
    bill_entries[array_ref].end_time = end_time;
    bill_entries[array_ref].energy_wh = energy_wh;
    bill_entries[array_ref].cost_eur_cent = cost_eur_cent;
    bill_entries[array_ref].grid_energy_percent = grid_energy_percent;
    bill_entries[array_ref].grid_cost_percent = grid_cost_percent;
    bill_entries[array_ref].self_produced_energy_percent = self_produced_energy_percent;
    bill_entries[array_ref].self_produced_cost_percent = self_produced_cost_percent;

    //eebus.data_handler->billlistdatatype = bill_list_data;
    //eebus.data_handler->last_cmd = SpineDataTypeHandler::Function::billListData;
    BillListDataType bill_list_data = EVSEEntity::get_bill_list_data();
    eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::Bill), bill_list_data, "billListData");
    update_api();
}

// Spec 3.2.1.2.1.1: Builds billDescriptionListData with metadata for each bill
void EvcsUsecase::get_bill_description_list(BillDescriptionListDataType *data) const
{
    if (data == nullptr)
        return;
    data->billDescriptionData.emplace();
    for (BillEntry entry : bill_entries) {
        if (!entry.id)
            continue;
        BillDescriptionDataType billDescriptionData{};
        billDescriptionData.billWriteable = false; // Bills are read-only (managed internally)
        billDescriptionData.billId = entry.id;
        billDescriptionData.supportedBillType->push_back(BillTypeEnumType::chargingSummary);
        data->billDescriptionData->push_back(billDescriptionData);
    }
}

// Spec 3.2.1.2.1.2: Builds billConstraintsListData with position count constraints
void EvcsUsecase::get_bill_constraints_list(BillConstraintsListDataType *data) const
{
    if (data == nullptr)
        return;
    data->billConstraintsData.emplace();
    for (BillEntry entry : bill_entries) {
        if (!entry.id)
            continue;
        BillConstraintsDataType billConstraintsData{};
        billConstraintsData.billId = entry.id;
        billConstraintsData.positionCountMin = "0";
        billConstraintsData.positionCountMax = std::to_string(2); // gridElectricEnergy + selfProducedElectricEnergy
        data->billConstraintsData->push_back(billConstraintsData);
    }
}

// Spec 3.2.1.2.1.3: Builds billListData with total energy/cost and position breakdowns
// Bill structure (Table 6):
//   - Total: energy (Wh), cost (EUR cents, scale=-2), time period
//   - Position 1: gridElectricEnergy (percentage of total)
//   - Position 2: selfProducedElectricEnergy (percentage of total)
void EvcsUsecase::get_bill_list_data(BillListDataType *data) const
{
    if (data == nullptr)
        return;
    for (BillEntry entry : bill_entries) {
        if (!entry.id)
            continue;
        BillDataType billData{};
        billData.billId = entry.id;
        billData.billType = BillTypeEnumType::chargingSummary;

        // Total energy and cost
        billData.total->timePeriod->startTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(entry.start_time).c_str();
        billData.total->timePeriod->endTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(entry.end_time).c_str();
        BillValueType total_value;
        total_value.value->number = entry.energy_wh;
        total_value.unit = UnitOfMeasurementEnumType::Wh;
        total_value.value->scale = 0; // Total value = number * 10^scale
        BillCostType total_cost;
        total_cost.costType = BillCostTypeEnumType::absolutePrice;
        total_cost.cost->number = entry.cost_eur_cent;
        total_cost.currency = CurrencyEnumType::EUR;
        total_cost.cost->scale = -2; // Cost in cents: value * 10^-2 = EUR
        billData.total->value->push_back(total_value);
        billData.total->cost->push_back(total_cost);

        // Position 1: Grid energy breakdown
        BillPositionType grid_position;
        grid_position.positionId = 1;
        grid_position.positionType = BillPositionTypeEnumType::gridElectricEnergy;
        BillValueType grid_value;
        grid_value.valuePercentage->number = entry.grid_energy_percent;
        grid_value.valuePercentage->scale = 0;
        BillCostType grid_cost;
        grid_cost.costPercentage->number = entry.grid_cost_percent;
        grid_position.cost->push_back(grid_cost);
        grid_position.value->push_back(grid_value);

        // Position 2: Self-produced energy breakdown
        BillPositionType self_produced_position;
        self_produced_position.positionId = 2;
        self_produced_position.positionType = BillPositionTypeEnumType::selfProducedElectricEnergy;
        BillValueType self_produced_value;
        self_produced_value.valuePercentage->number = entry.self_produced_energy_percent;
        self_produced_value.valuePercentage->scale = 0;
        BillCostType self_produced_cost;
        self_produced_cost.costPercentage->number = entry.self_produced_cost_percent;
        self_produced_position.cost->push_back(self_produced_cost);
        self_produced_position.value->push_back(self_produced_value);

        billData.position->push_back(grid_position);
        billData.position->push_back(self_produced_position);

        data->billData->push_back(billData);
    }
}

void EvcsUsecase::update_api() const
{
    auto api_entry = eebus.eebus_usecase_state.get("charging_summary");
    api_entry->removeAll();
    int i = 0;
    for (BillEntry bill_entry : bill_entries) {
        if (bill_entry.id < 1)
            continue;
        api_entry->add();
        auto api_bill_entry = api_entry->get(i++);
        api_bill_entry->get("id")->updateUint(bill_entry.id);
        uint32_t charged_kwh = bill_entry.energy_wh;
        api_bill_entry->get("charged_kwh")->updateFloat(static_cast<float>(charged_kwh) / 1000.0f);
        api_bill_entry->get("cost")->updateFloat(static_cast<float>(bill_entry.cost_eur_cent) / 100.0f);
        api_bill_entry->get("start_time")->updateUint(bill_entry.start_time);
        api_bill_entry->get("duration")->updateUint(bill_entry.end_time - bill_entry.start_time);

        api_bill_entry->get("percent_self_produced_energy")->updateUint(bill_entry.self_produced_energy_percent);
        api_bill_entry->get("percent_self_produced_cost")->updateUint(bill_entry.self_produced_cost_percent);
    }
}
#endif

#ifdef EEBUS_ENABLE_EVCEM_USECASE
// EV Charging Electricity Measurement Usecase as defined in EEBus_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf
// This usecase implements scenarios 1-3 for measuring current, power, and energy during EV charging
EvcemUsecase::EvcemUsecase() = default;
MessageReturn EvcemUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    AddressFeatureType feature_address = header.addressDestination->feature.get();
    switch (data->last_cmd) {
        // EEBUS_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf 3.2.1.2.1.1 Scenario 1: Measurement Description
        case SpineDataTypeHandler::Function::measurementDescriptionListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::Measurement)) {
                response["measurementDescriptionListData"] = EVEntity::get_measurement_description_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        // EEBUS_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf 3.2.1.2.1.2 Scenario 1-3: Measurement Constraints
        case SpineDataTypeHandler::Function::measurementConstraintsListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::Measurement)) {
                response["measurementConstraintsListData"] = EVEntity::get_measurement_constraints_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        // EEBUS_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf 3.2.1.2.1.3 Scenario 1-3: Measurement Data (current, power, energy)
        case SpineDataTypeHandler::Function::measurementListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::Measurement)) {
                response["measurementListData"] = EVEntity::get_measurement_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        // EEBUS_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf 3.2.1.2.2.1 Electrical Connection Description
        case SpineDataTypeHandler::Function::electricalConnectionDescriptionListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::ElectricalConnection)) {
                response["electricalConnectionDescriptionListData"] = EVEntity::get_electrical_connection_description_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        // EEBUS_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf 3.2.1.2.2.2 Electrical Connection Parameters
        case SpineDataTypeHandler::Function::electricalConnectionParameterDescriptionListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::ElectricalConnection)) {
                response["electricalConnectionParameterDescriptionListData"] = EVEntity::get_electrical_connection_parameter_description_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        default:;
    }
    return {false};
}

UseCaseInformationDataType EvcemUsecase::get_usecase_information()
{
    UseCaseInformationDataType evcm_usecase;
    evcm_usecase.actor = "EV";

    UseCaseSupportType evcm_usecase_support;
    evcm_usecase_support.useCaseName = "measurementOfElectricityDuringEvCharging";
    evcm_usecase_support.useCaseVersion = "1.0.1";
    evcm_usecase_support.scenarioSupport->insert(evcm_usecase_support.scenarioSupport->end(), {1, 2, 3});

    evcm_usecase_support.useCaseDocumentSubRevision = "release";
    evcm_usecase.useCaseSupport->push_back(evcm_usecase_support);

    FeatureAddressType evcm_usecase_feature_address;
    evcm_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    evcm_usecase_feature_address.entity = entity_address;
    evcm_usecase.address = evcm_usecase_feature_address;
    return evcm_usecase;
}

NodeManagementDetailedDiscoveryEntityInformationType EvcemUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return entity;
    }

    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EV;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "EV";

    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> EvcemUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return features;
    }

    NodeManagementDetailedDiscoveryFeatureInformationType measurement_feature{};
    measurement_feature.description->featureAddress->entity = entity_address;
    measurement_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::Measurement);
    measurement_feature.description->featureType = FeatureTypeEnumType::Measurement;
    // The feature type as defined in EEBUS SPINE TS ResourceSpecification 4.3.19
    measurement_feature.description->role = RoleType::server;

    // measurementDescriptionListData
    FunctionPropertyType measurementDescription{};
    measurementDescription.function = FunctionEnumType::measurementDescriptionListData;
    measurementDescription.possibleOperations->read = PossibleOperationsReadType{};
    measurement_feature.description->supportedFunction->push_back(measurementDescription);

    // measurementConstraintsListData
    FunctionPropertyType measurementConstrains{};
    measurementConstrains.function = FunctionEnumType::measurementConstraintsListData;
    measurementConstrains.possibleOperations->read = PossibleOperationsReadType{};
    measurement_feature.description->supportedFunction->push_back(measurementConstrains);

    // measurementConstraintsListData
    FunctionPropertyType measurementData{};
    measurementData.function = FunctionEnumType::measurementListData;
    measurementData.possibleOperations->read = PossibleOperationsReadType{};
    measurement_feature.description->supportedFunction->push_back(measurementData);
    features.push_back(measurement_feature);

    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnection_feature{};
    electricalConnection_feature.description->featureAddress->entity = entity_address;
    electricalConnection_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::ElectricalConnection);
    electricalConnection_feature.description->featureType = FeatureTypeEnumType::ElectricalConnection;
    // The feature type as defined in EEBUS SPINE TS ResourceSpecification 4.3.19
    electricalConnection_feature.description->role = RoleType::server;

    // electricalConnectionDescriptionListData
    FunctionPropertyType electricalConnectionDescription{};
    electricalConnectionDescription.function = FunctionEnumType::electricalConnectionDescriptionListData;
    electricalConnectionDescription.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnection_feature.description->supportedFunction->push_back(electricalConnectionDescription);

    // electricalConnectionParameterDescriptionListData
    FunctionPropertyType electricalConnectionParameters{};
    electricalConnectionParameters.function = FunctionEnumType::electricalConnectionParameterDescriptionListData;
    electricalConnectionParameters.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnection_feature.description->supportedFunction->push_back(electricalConnectionParameters);
    features.push_back(electricalConnection_feature);

    return features;
}

void EvcemUsecase::update_measurements(const int amps_phase_1, const int amps_phase_2, const int amps_phase_3, const int power_phase_1, const int power_phase_2, const int power_phase_3, const int charged_wh, const bool charged_measured)
{
    bool phases_measured_before[3] = {false, false, false};
    for (int i = 0; i < 3; i++) {
        if (milliamps_draw_phase[i] > 0 || power_draw_phase[i] > 0) {
            phases_measured_before[i] = true;
        }
    }
    milliamps_draw_phase[0] = amps_phase_1;
    milliamps_draw_phase[1] = amps_phase_2;
    milliamps_draw_phase[2] = amps_phase_3;
    power_draw_phase[0] = power_phase_1;
    power_draw_phase[1] = power_phase_2;
    power_draw_phase[2] = power_phase_3;
    power_charged_wh = charged_wh;
    power_charged_measured = charged_measured;

    bool phases_measured_after[3] = {false, false, false};
    for (int i = 0; i < 3; i++) {
        if (milliamps_draw_phase[i] > 0 || power_draw_phase[i] > 0) {
            phases_measured_after[i] = true;
        }
    }

    bool phase_changed = false;
    for (int i = 0; i < 3; i++) {
        if (phases_measured_before[i] != phases_measured_after[i]) {
            phase_changed = true;
            break;
        }
    }
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected())
        return;
    MeasurementListDataType measurement_list_data = EVEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_list_data, "measurementListData");
    if (phase_changed) {
        MeasurementDescriptionListDataType measurement_description = EVEntity::get_measurement_description_list_data();
        MeasurementConstraintsListDataType measurement_constraints_data_elements = EVEntity::get_measurement_constraints_list_data();
        eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_description, "measurementDescriptionListData");
        eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_constraints_data_elements, "measurementConstraintsListData");
    }
    update_api();
}

void EvcemUsecase::update_constraints(const int amps_min, const int amps_max, const int amps_stepsize, const int power_min, const int power_max, const int power_stepsize, const int energy_min, const int energy_max, const int energy_stepsize)
{
    measurement_limit_milliamps_min = amps_min;
    measurement_limit_milliamps_max = amps_max;
    measurement_limit_milliamps_stepsize = amps_stepsize;
    measurement_limit_power_min = power_min;
    measurement_limit_power_max = power_max;
    measurement_limit_power_stepsize = power_stepsize;
    measurement_limit_energy_min = energy_min;
    measurement_limit_energy_max = energy_max;
    measurement_limit_energy_stepsize = energy_stepsize;

    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected())
        return;

    MeasurementConstraintsListDataType constraints = EVEntity::get_measurement_constraints_list_data();
    eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), constraints, "measurementConstraintsListData");

    update_api();
}

void EvcemUsecase::get_measurement_description_list(MeasurementDescriptionListDataType *data) const
{
    struct Entrydata {
        uint8_t id;
        MeasurementTypeEnumType type;
        UnitOfMeasurementEnumType unit;
        ScopeTypeEnumType scope;
    };
    const std::array<Entrydata, 7> entries{{
        {id_x_1, MeasurementTypeEnumType::current, UnitOfMeasurementEnumType::A, ScopeTypeEnumType::acCurrent},
        {id_x_2, MeasurementTypeEnumType::current, UnitOfMeasurementEnumType::A, ScopeTypeEnumType::acCurrent},
        {id_x_3, MeasurementTypeEnumType::current, UnitOfMeasurementEnumType::A, ScopeTypeEnumType::acCurrent},
        {id_x_4, MeasurementTypeEnumType::power, UnitOfMeasurementEnumType::W, ScopeTypeEnumType::acPower},
        {id_x_5, MeasurementTypeEnumType::power, UnitOfMeasurementEnumType::W, ScopeTypeEnumType::acPower},
        {id_x_6, MeasurementTypeEnumType::power, UnitOfMeasurementEnumType::W, ScopeTypeEnumType::acPower},
        {id_x_7, MeasurementTypeEnumType::energy, UnitOfMeasurementEnumType::Wh, ScopeTypeEnumType::charge},
    }};

    for (const auto &entrydata : entries) {
        MeasurementDescriptionDataType measurement_description_data{};
        measurement_description_data.measurementId = entrydata.id;
        measurement_description_data.unit = entrydata.unit;
        measurement_description_data.scopeType = entrydata.scope;
        measurement_description_data.measurementType = entrydata.type;
        measurement_description_data.commodityType = CommodityTypeEnumType::electricity;
        data->measurementDescriptionData->push_back(measurement_description_data);
    }
}

void EvcemUsecase::get_measurement_constraints(MeasurementConstraintsListDataType *data) const
{
    struct Entrydata {
        uint8_t id;
        int min;
        int max;
        int stepsize;
        int scale;
    };

    const std::array<Entrydata, 7> entries{{
        {id_x_1, measurement_limit_milliamps_min, measurement_limit_milliamps_max, measurement_limit_milliamps_stepsize, -3},
        {id_x_2, measurement_limit_milliamps_min, measurement_limit_milliamps_max, measurement_limit_milliamps_stepsize, -3},
        {id_x_3, measurement_limit_milliamps_min, measurement_limit_milliamps_max, measurement_limit_milliamps_stepsize, -3},
        {id_x_4, measurement_limit_power_min, measurement_limit_power_max, measurement_limit_power_stepsize, 0},
        {id_x_5, measurement_limit_power_min, measurement_limit_power_max, measurement_limit_power_stepsize, 0},
        {id_x_6, measurement_limit_power_min, measurement_limit_power_max, measurement_limit_power_stepsize, 0},
        {id_x_7, measurement_limit_energy_min, measurement_limit_energy_max, measurement_limit_energy_stepsize, 0},
    }};

    for (const auto &entrydata : entries) {
        MeasurementConstraintsDataType measurement_constraints_data{};
        measurement_constraints_data.measurementId = entrydata.id;
        measurement_constraints_data.valueRangeMin->number = entrydata.min;
        measurement_constraints_data.valueRangeMin->scale = entrydata.scale;
        measurement_constraints_data.valueRangeMax->number = entrydata.max;
        measurement_constraints_data.valueRangeMax->scale = entrydata.scale;
        measurement_constraints_data.valueStepSize->number = entrydata.stepsize;
        measurement_constraints_data.valueStepSize->scale = entrydata.scale;
        data->measurementConstraintsData->push_back(measurement_constraints_data);
    }
}

void EvcemUsecase::get_measurement_list(MeasurementListDataType *data) const
{
    struct Entrydata {
        uint8_t id;
        int value;
        int scale;
    };

    const std::array<Entrydata, 7> entries{{
        {id_x_1, milliamps_draw_phase[0], -3},
        {id_x_2, milliamps_draw_phase[1], -3},
        {id_x_3, milliamps_draw_phase[2], -3},
        {id_x_4, power_draw_phase[0], 0},
        {id_x_5, power_draw_phase[1], 0},
        {id_x_6, power_draw_phase[2], 0},
        {id_x_7, power_charged_wh, 0},
    }};

    for (const auto &entry : entries) {
        MeasurementDataType measurement_data{};
        measurement_data.measurementId = entry.id;
        measurement_data.valueType = MeasurementValueTypeEnumType::value;
        measurement_data.value->number = entry.value;
        measurement_data.value->scale = entry.scale;
        if (entry.id == id_x_7) {
            if (power_charged_measured) {
                measurement_data.valueSource = MeasurementValueSourceEnumType::measuredValue;
            } else {
                measurement_data.valueSource = MeasurementValueSourceEnumType::calculatedValue;
            }
        }
        data->measurementData->push_back(measurement_data);
    }
}

void EvcemUsecase::get_electrical_connection_description(ElectricalConnectionDescriptionListDataType *data) const
{
    ElectricalConnectionDescriptionDataType connection_description_data{};
    connection_description_data.electricalConnectionId = id_y_1;
    connection_description_data.powerSupplyType = ElectricalConnectionVoltageTypeEnumType::ac;
    connection_description_data.positiveEnergyDirection = EnergyDirectionEnumType::consume;
    data->electricalConnectionDescriptionData->push_back(connection_description_data);
}

void EvcemUsecase::get_electrical_connection_parameters(ElectricalConnectionParameterDescriptionListDataType *data) const
{
    struct Entrydata {
        uint8_t parameterId;
        uint8_t measurementId;
        ElectricalConnectionPhaseNameEnumType phases;
        bool to_be_added;
    };
    const std::array<Entrydata, 7> entries{{
        {id_z_1, id_x_1, ElectricalConnectionPhaseNameEnumType::a, milliamps_draw_phase[0] > 0},
        {id_z_2, id_x_2, ElectricalConnectionPhaseNameEnumType::b, milliamps_draw_phase[1] > 0},
        {id_z_3, id_x_3, ElectricalConnectionPhaseNameEnumType::c, milliamps_draw_phase[2] > 0},
        {id_z_4, id_x_4, ElectricalConnectionPhaseNameEnumType::a, power_draw_phase[0] > 0},
        {id_z_5, id_x_5, ElectricalConnectionPhaseNameEnumType::b, power_draw_phase[1] > 0},
        {id_z_6, id_x_6, ElectricalConnectionPhaseNameEnumType::c, power_draw_phase[2] > 0},
        {id_z_7, id_x_7, ElectricalConnectionPhaseNameEnumType::abc, true},
    }};

    for (const auto &entry : entries) {
        if (!entry.to_be_added)
            continue;

        ElectricalConnectionParameterDescriptionDataType connection_parameters_data{};
        connection_parameters_data.parameterId = entry.parameterId;
        connection_parameters_data.measurementId = entry.measurementId;
        connection_parameters_data.electricalConnectionId = id_y_1;
        connection_parameters_data.acMeasuredPhases = entry.phases;
        if (entry.parameterId == id_z_1 || entry.parameterId == id_z_2 || entry.parameterId == id_z_3) {
            connection_parameters_data.acMeasurementVariant = ElectricalConnectionMeasurandVariantEnumType::rms;
        } else if (entry.parameterId == id_z_7) {
            connection_parameters_data.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
            connection_parameters_data.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::real;
        }
        data->electricalConnectionParameterDescriptionData->push_back(connection_parameters_data);
    }
}

void EvcemUsecase::update_api() const
{
    auto api_entry = eebus.eebus_usecase_state.get("ev_charging_electricity_measurement");
    api_entry->get("amps_phase_1")->updateUint(milliamps_draw_phase[0]);
    api_entry->get("amps_phase_2")->updateUint(milliamps_draw_phase[1]);
    api_entry->get("amps_phase_3")->updateUint(milliamps_draw_phase[2]);
    api_entry->get("power_phase_1")->updateUint(power_draw_phase[0]);
    api_entry->get("power_phase_2")->updateUint(power_draw_phase[1]);
    api_entry->get("power_phase_3")->updateUint(power_draw_phase[2]);
    api_entry->get("charged_wh")->updateUint(power_charged_wh);
    api_entry->get("charged_valuesource_measured")->updateBool(power_charged_measured);
}
#endif
// ==============================================================================
// EVCC - EV Commissioning and Configuration Use Case
// Spec: EEBus_UC_TS_EVCommissioningAndConfiguration_V1.0.1.pdf
//
// Scenarios:
//   1 (2.3.1, 3.4.1): EV connected
//   2 (2.3.2, 3.4.2): Communication standard
//   3 (2.3.3, 3.4.3): Asymmetric charging
//   4 (2.3.4, 3.4.4): EV identification
//   5 (2.3.5, 3.4.5): Manufacturer information
//   6 (2.3.6, 3.4.6): Charging power limits
//   7 (2.3.7, 3.4.7): EV sleep mode
//   8 (2.3.8, 3.4.8): EV disconnected
// ==============================================================================
#ifdef EEBUS_ENABLE_EVCC_USECASE
EvccUsecase::EvccUsecase()
{
    entity_active = false; // Disable entity until an EV is connected
    ev_connected = false;
}

UseCaseInformationDataType EvccUsecase::get_usecase_information()
{
    UseCaseInformationDataType evcc_usecase;
    evcc_usecase.actor = "EV";

    UseCaseSupportType evcc_usecase_support;
    evcc_usecase_support.useCaseName = "evCommissioningAndConfiguration";
    evcc_usecase_support.useCaseVersion = "1.0.1";
    // All 8 scenarios supported (see spec chapter 2.3)
    evcc_usecase_support.scenarioSupport->insert(evcc_usecase_support.scenarioSupport->end(), {1, 2, 3, 4, 5, 6, 7, 8});

    evcc_usecase_support.useCaseDocumentSubRevision = "release";
    evcc_usecase.useCaseSupport->push_back(evcc_usecase_support);

    FeatureAddressType evcc_usecase_feature_address;
    evcc_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    evcc_usecase_feature_address.entity = entity_address;
    evcc_usecase.address = evcc_usecase_feature_address;
    return evcc_usecase;
}

MessageReturn EvccUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    AddressFeatureType feature_address = header.addressDestination->feature.get();
    if (header.cmdClassifier == CmdClassifierType::read) {
        switch (data->last_cmd) {
            // 3.2.1.2.1.1: Function "deviceConfigurationKeyValueDescriptionListData"
            case SpineDataTypeHandler::Function::deviceConfigurationKeyValueDescriptionListData:
                if (feature_address == feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration)) {
                    response["deviceConfigurationKeyValueDescriptionListData"] = EVEntity::get_device_configuration_value_description_list();
                    return {true, true, CmdClassifierType::reply};
                }
                break;
            // 3.2.1.2.1.2: Function "deviceConfigurationKeyValueListData"
            case SpineDataTypeHandler::Function::deviceConfigurationKeyValueListData:
                if (feature_address == feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration)) {
                    response["deviceConfigurationKeyValueListData"] = EVEntity::get_device_configuration_value_list();
                    return {true, true, CmdClassifierType::reply};
                }
                break;
            // 3.2.1.2.2.1: Function "identificationListData"
            case SpineDataTypeHandler::Function::identificationListData:
                if (feature_address == feature_addresses.at(FeatureTypeEnumType::Identification)) {
                    response["identificationListData"] = EVEntity::get_identification_list_data();
                    return {true, true, CmdClassifierType::reply};
                }
                break;
            // 3.2.1.2.3.1: Function "deviceClassificationManufacturerData"
            case SpineDataTypeHandler::Function::deviceClassificationManufacturerData:
                if (feature_address == feature_addresses.at(FeatureTypeEnumType::DeviceClassification)) {
                    response["deviceClassificationManufacturerData"] = EVEntity::get_device_classification_manufacturer_data();
                    return {true, true, CmdClassifierType::reply};
                }
                break;
            // 3.2.1.2.4.1: Function "electricalConnectionParameterDescriptionListData"
            case SpineDataTypeHandler::Function::electricalConnectionParameterDescriptionListData:
                if (feature_address == feature_addresses.at(FeatureTypeEnumType::ElectricalConnection)) {
                    response["electricalConnectionParameterDescriptionListData"] = EVEntity::get_electrical_connection_parameter_description_list_data();
                    return {true, true, CmdClassifierType::reply};
                }
                break;
            // 3.2.1.2.4.2: Function "electricalConnectionPermittedValueSetListData"
            case SpineDataTypeHandler::Function::electricalConnectionPermittedValueSetListData:
                if (feature_address == feature_addresses.at(FeatureTypeEnumType::ElectricalConnection)) {
                    response["electricalConnectionPermittedValueSetListData"] = EVEntity::get_electrical_connection_permitted_list_data();
                    return {true, true, CmdClassifierType::reply};
                }
                break;
            // 3.2.1.2.5.1: Function "deviceDiagnosisStateData"
            case SpineDataTypeHandler::Function::deviceDiagnosisStateData:
                if (feature_address == feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis)) {
                    response["deviceDiagnosisStateData"] = EVEntity::get_diagnosis_state_data();
                    return {true, true, CmdClassifierType::reply};
                }
                break;
            default:;
        }
    }
    return {false};
}

NodeManagementDetailedDiscoveryEntityInformationType EvccUsecase::get_detailed_discovery_entity_information() const
{

    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return entity;
    }
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EV;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "EV"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> EvccUsecase::get_detailed_discovery_feature_information() const
{

    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    if (!ev_connected)
        return features;

    // The following functions are needed by the first DeviceDiagnosis Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType device_configuration_feature{};
    device_configuration_feature.description->featureAddress->entity = entity_address;
    device_configuration_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration);
    device_configuration_feature.description->featureType = FeatureTypeEnumType::DeviceConfiguration;
    device_configuration_feature.description->role = RoleType::server;

    // deviceConfigurationKeyValueDescriptionListData
    FunctionPropertyType device_configuration_description{};
    device_configuration_description.function = FunctionEnumType::deviceConfigurationKeyValueDescriptionListData;
    device_configuration_description.possibleOperations->read = PossibleOperationsReadType{};
    device_configuration_feature.description->supportedFunction->push_back(device_configuration_description);

    // deviceConfigurationKeyValueListData
    FunctionPropertyType device_configuration_values{};
    device_configuration_values.function = FunctionEnumType::deviceConfigurationKeyValueListData;
    device_configuration_values.possibleOperations->read = PossibleOperationsReadType{};

    device_configuration_feature.description->supportedFunction->push_back(device_configuration_values);
    features.push_back(device_configuration_feature);

    // The following functions are needed by the Identification Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType identification_feature{};
    identification_feature.description->featureAddress->entity = entity_address;
    identification_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::Identification);
    identification_feature.description->featureType = FeatureTypeEnumType::Identification;
    identification_feature.description->role = RoleType::server;

    //identificationListData
    FunctionPropertyType identificationListData{};
    identificationListData.function = FunctionEnumType::identificationListData;
    identificationListData.possibleOperations->read = PossibleOperationsReadType{};

    identification_feature.description->supportedFunction->push_back(identificationListData);
    features.push_back(identification_feature);

    // The following functions are needed by the DeviceDiagnosis Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType device_classification_feature{};
    device_classification_feature.description->featureAddress->entity = entity_address;
    device_classification_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::DeviceClassification);
    device_classification_feature.description->featureType = FeatureTypeEnumType::DeviceClassification;
    device_classification_feature.description->role = RoleType::server;

    //deviceClassificationManufacturerData
    FunctionPropertyType deviceClassificationManufacturerData{};
    deviceClassificationManufacturerData.function = FunctionEnumType::deviceClassificationManufacturerData;
    deviceClassificationManufacturerData.possibleOperations->read = PossibleOperationsReadType{};
    device_classification_feature.description->supportedFunction->push_back(deviceClassificationManufacturerData);
    features.push_back(device_classification_feature);

    // The following functions are needed by the ElectricalConnection Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnectionFeature{};
    electricalConnectionFeature.description->featureAddress->entity = entity_address;
    electricalConnectionFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::ElectricalConnection);
    electricalConnectionFeature.description->featureType = FeatureTypeEnumType::ElectricalConnection;
    electricalConnectionFeature.description->role = RoleType::server;

    //electricalConnectionCharacteristicsListData
    FunctionPropertyType electricalConnectionParameterDescriptionData{};
    electricalConnectionParameterDescriptionData.function = FunctionEnumType::electricalConnectionParameterDescriptionListData;
    electricalConnectionParameterDescriptionData.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionParameterDescriptionData);

    //electricalConnectionPermittedValueSetListData
    FunctionPropertyType electricalConnectionPermittedValueListData{};
    electricalConnectionPermittedValueListData.function = FunctionEnumType::electricalConnectionPermittedValueSetListData;
    electricalConnectionPermittedValueListData.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionPermittedValueListData);
    features.push_back(electricalConnectionFeature);

    // The following functions are needed by the DeviceDiagnosis Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType deviceDiagnosisFeature{};
    deviceDiagnosisFeature.description->featureAddress->entity = entity_address;
    deviceDiagnosisFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis);
    deviceDiagnosisFeature.description->featureType = FeatureTypeEnumType::DeviceDiagnosis;
    deviceDiagnosisFeature.description->role = RoleType::server;

    // deviceDiagnosisStateData
    FunctionPropertyType deviceDiagnosisState{};
    deviceDiagnosisState.function = FunctionEnumType::deviceDiagnosisStateData;
    deviceDiagnosisState.possibleOperations->read = PossibleOperationsReadType{};
    deviceDiagnosisFeature.description->supportedFunction->push_back(deviceDiagnosisState);
    features.push_back(deviceDiagnosisFeature);

    return features;
}

// Scenario 1 (2.3.1, 3.4.1) & Scenario 8 (2.3.8, 3.4.8): EV connected/disconnected state
void EvccUsecase::ev_connected_state(bool connected)
{
    logger.printfln("EV connected: %d, previous: %d", connected, ev_connected);
    bool changed = (ev_connected != connected);
    entity_active = ev_connected = connected;
    if (changed) {
        eebus.usecases->node_management.detailed_discovery_update();
        update_api();
    }
}

// Scenario 2 (2.3.2, 3.4.2): Communication standard
// Scenario 3 (2.3.3, 3.4.3): Asymmetric charging support
void EvccUsecase::update_device_config(const String &comm_standard, bool asym_supported)
{
    communication_standard = comm_standard;
    asymmetric_supported = asym_supported;
    // Spec 3.2.1.2.1.2: Valid values per Table 6: iso15118-2ed1, iso15118-2ed2, iec61851
    if (communication_standard != "iso15118-2ed1" && communication_standard != "iso15118-2ed2" && communication_standard != "iec61851") {
        eebus.trace_fmtln(R"(Usecase EVCC: Invalid communication standard for EV entity device configuration: %s, should be "iso15118-2ed1","iso15118-2ed1" or "iec61851".)", communication_standard.c_str());
        // We continue on regardless and let the peer deal with incorrect values
    }
    if (!ev_connected)
        return;
    DeviceConfigurationKeyValueDescriptionListDataType generate_dev_desc = EVEntity::get_device_configuration_value_description_list();
    DeviceConfigurationKeyValueListDataType generate_dev_list = EVEntity::get_device_configuration_value_list();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis), generate_dev_desc, "deviceConfigurationKeyValueDescriptionData");
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis), generate_dev_list, "deviceConfigurationKeyValueListData");

    update_api();
}

// Scenario 4 (2.3.4, 3.4.4): EV identification
// Spec 3.2.1.2.2.1: Supports EUI-48 (MAC) and EUI-64 identification types (Table 9)
void EvccUsecase::update_identification(String mac, IdentificationTypeEnumType type)
{
    mac_address = std::move(mac);
    mac_type = type;
    if (!ev_connected)
        return;
    IdentificationListDataType identification_desc = EVEntity::get_identification_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Identification), identification_desc, "identificationListData");
    update_api();
}

// Scenario 5 (2.3.5, 3.4.5): Manufacturer information
// Spec 3.2.1.2.3.1: Manufacturer data fields per Table 12
void EvccUsecase::update_manufacturer(String name, String code, String serial, String software_vers, String hardware_vers, String vendor_n, String vendor_c, String brand, String manufacturer, String manufacturer_description_text)
{
    manufacturer_name = std::move(name);
    manufacturer_code = std::move(code);
    ev_serial_number = std::move(serial);
    ev_sofware_version = std::move(software_vers);
    ev_hardware_version = std::move(hardware_vers);
    vendor_name = std::move(vendor_n);
    vendor_code = std::move(vendor_c);
    manufacturer_label = std::move(manufacturer);
    brand_name = std::move(brand);
    manufacturer_description = std::move(manufacturer_description_text);

    // This is quite a few strings so we shrink them
    manufacturer_name.shrinkToFit();
    manufacturer_code.shrinkToFit();
    ev_serial_number.shrinkToFit();
    ev_sofware_version.shrinkToFit();
    ev_hardware_version.shrinkToFit();
    vendor_name.shrinkToFit();
    vendor_code.shrinkToFit();
    manufacturer_label.shrinkToFit();
    brand_name.shrinkToFit();
    manufacturer_description.shrinkToFit();

    if (!ev_connected)
        return;

    DeviceClassificationManufacturerDataType manufacturer_desc = EVEntity::get_device_classification_manufacturer_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceClassification), manufacturer_desc, "deviceClassificationManufacturerData");
    update_api();
}

// Scenario 6 (2.3.6, 3.4.6): Charging power limits
// Spec 3.2.1.2.4: Min/max/standby power values (Table 14, 15)
void EvccUsecase::update_electrical_connection(int min_power, int max_power, int stby_power)
{
    min_power_draw = min_power;
    max_power_draw = max_power;
    standby_power = stby_power;

    if (!ev_connected)
        return;

    ElectricalConnectionParameterDescriptionListDataType electrical_connection_desc = EVEntity::get_electrical_connection_parameter_description_list_data();
    ElectricalConnectionPermittedValueSetListDataType electrical_connection_values = EVEntity::get_electrical_connection_permitted_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::ElectricalConnection), electrical_connection_desc, "electricalConnectionParameterDescriptionListData");
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::ElectricalConnection), electrical_connection_values, "electricalConnectionPermittedValueSetListData");

    update_api();
}

// Scenario 7 (2.3.7, 3.4.7): EV sleep mode
// Spec 3.2.1.2.5.1: Operating state (standby or normalOperation)
void EvccUsecase::update_operating_state(bool standby)
{
    standby_mode = standby;

    if (!ev_connected)
        return;

    DeviceDiagnosisStateDataType state = EVEntity::get_diagnosis_state_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis), state, "deviceDiagnosisStateData");
    update_api();
}

void EvccUsecase::update_api() const
{
    auto api_entry = eebus.eebus_usecase_state.get("ev_commissioning_and_configuration");
    api_entry->get("ev_connected")->updateBool(ev_connected);

    api_entry->get("communication_standard")->updateString(communication_standard);
    api_entry->get("asymmetric_charging_supported")->updateBool(asymmetric_supported);
    api_entry->get("mac_address")->updateString(mac_address);
    api_entry->get("minimum_power")->updateUint(min_power_draw);
    api_entry->get("maximum_power")->updateUint(max_power_draw);
    api_entry->get("standby_power")->updateUint(standby_power);
    api_entry->get("standby_mode")->updateBool(standby_mode);
}

void EvccUsecase::get_device_config_description(DeviceConfigurationKeyValueDescriptionListDataType *data) const
{
    DeviceConfigurationKeyValueDescriptionDataType comm_standard_description{};
    comm_standard_description.keyId = id_x_1;
    comm_standard_description.keyName = DeviceConfigurationKeyNameEnumType::communicationsStandard;
    comm_standard_description.valueType = DeviceConfigurationKeyValueTypeType::string;
    data->deviceConfigurationKeyValueDescriptionData->push_back(comm_standard_description);

    DeviceConfigurationKeyValueDescriptionDataType asymmetric_description{};
    asymmetric_description.keyId = id_x_2;
    asymmetric_description.keyName = DeviceConfigurationKeyNameEnumType::asymmetricChargingSupported;
    asymmetric_description.valueType = DeviceConfigurationKeyValueTypeType::boolean;
    data->deviceConfigurationKeyValueDescriptionData->push_back(asymmetric_description);
}

void EvccUsecase::get_device_config_list(DeviceConfigurationKeyValueListDataType *data) const
{
    DeviceConfigurationKeyValueDataType comm_standard_value{};
    comm_standard_value.keyId = id_x_1;
    comm_standard_value.value->string = communication_standard.c_str();
    data->deviceConfigurationKeyValueData->push_back(comm_standard_value);

    DeviceConfigurationKeyValueDataType asymmetric_value{};
    asymmetric_value.keyId = id_x_2;
    asymmetric_value.value->boolean = asymmetric_supported;
    data->deviceConfigurationKeyValueData->push_back(asymmetric_value);
}

void EvccUsecase::get_identification_list(IdentificationListDataType *data) const
{

    IdentificationDataType identification_data_entry{};
    identification_data_entry.identificationId = 1;
    identification_data_entry.identificationType = mac_type;
    identification_data_entry.identificationValue = mac_address.c_str();
    data->identificationData->push_back(identification_data_entry);
}

void EvccUsecase::get_device_classification_manufacturer(DeviceClassificationManufacturerDataType *data) const
{
    if (manufacturer_name.length() > 0)
        data->deviceName = manufacturer_name.c_str();
    if (manufacturer_code.length() > 0)
        data->deviceCode = manufacturer_code.c_str();
    if (ev_serial_number.length() > 0)
        data->serialNumber = ev_serial_number.c_str();
    if (ev_sofware_version.length() > 0)
        data->softwareRevision = ev_sofware_version.c_str();
    if (ev_hardware_version.length() > 0)
        data->hardwareRevision = ev_hardware_version.c_str();
    if (vendor_name.length() > 0)
        data->vendorName = vendor_name.c_str();
    if (vendor_code.length() > 0)
        data->vendorCode = vendor_code.c_str();
    if (brand_name.length() > 0)
        data->brandName = brand_name.c_str();
    if (manufacturer_label.length() > 0)
        data->manufacturerLabel = manufacturer_label.c_str();
    if (manufacturer_description.length() > 0)
        data->manufacturerDescription = manufacturer_description.c_str();
}

void EvccUsecase::get_electrical_connection_parameter_description(ElectricalConnectionParameterDescriptionListDataType *data) const
{

    ElectricalConnectionParameterDescriptionDataType power_description{};
    power_description.electricalConnectionId = id_y_1;
    power_description.parameterId = id_z_1;
    power_description.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::abc; // Not sure but the spec has no value here, maybe its what we are capable of measuring?
    power_description.scopeType = ScopeTypeEnumType::acPowerTotal;
    data->electricalConnectionParameterDescriptionData->push_back(power_description);
}

void EvccUsecase::get_electrical_connection_permitted_values(ElectricalConnectionPermittedValueSetListDataType *data) const
{
    ElectricalConnectionPermittedValueSetDataType permitted_values{};
    permitted_values.electricalConnectionId = id_y_1;
    permitted_values.parameterId = id_z_1;

    ScaledNumberSetType minmax_power_value_set{};
    ScaledNumberRangeType power_value_range{};
    power_value_range.min->number = min_power_draw;
    power_value_range.min->scale = 0;
    power_value_range.max->number = max_power_draw;
    power_value_range.max->scale = 0;
    minmax_power_value_set.range->push_back(power_value_range);

    //ScaledNumberSetType standby_power_value_set{};
    ScaledNumberType standby_value_range{};
    standby_value_range.number = standby_power;
    //standby_power_value_set.value->push_back(standby_value_range);
    minmax_power_value_set.value->push_back(standby_value_range);
    permitted_values.permittedValueSet->push_back(minmax_power_value_set);
    //permitted_values.permittedValueSet->push_back(standby_power_value_set);
    data->electricalConnectionPermittedValueSetData->push_back(permitted_values);
}

DeviceDiagnosisStateDataType EvccUsecase::get_device_diagnosis_state() const
{
    DeviceDiagnosisStateDataType state{};
    state.operatingState.emplace();
    if (standby_mode) {
        state.operatingState = DeviceDiagnosisOperatingStateEnumType::standby;
    } else {
        state.operatingState = DeviceDiagnosisOperatingStateEnumType::normalOperation;
    }
    return state;
}
#endif
#ifdef EEBUS_ENABLE_EVSECC_USECASE
// EVSE Commissioning and Configuration Usecase as defined in EEBus_UC_TS_EVSECommissioningAndConfiguration_V1.0.1.pdf
// This usecase implements scenarios 1-2 for EVSE identification and error reporting
EvseccUsecase::EvseccUsecase() = default;

MessageReturn EvseccUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    AddressFeatureType feature_address = header.addressDestination->feature.get();
    if (header.cmdClassifier == CmdClassifierType::read) {
        switch (data->last_cmd) {
            // EEBus_UC_TS_EVSECommissioningAndConfiguration_V1.0.1.pdf 3.2.1.2.1.1 Scenario 1: Manufacturer Data
            case SpineDataTypeHandler::Function::deviceClassificationManufacturerData:
                if (feature_address == feature_addresses.at(FeatureTypeEnumType::DeviceClassification)) {
                    response["deviceClassificationManufacturerData"] = EVSEEntity::get_device_classification_manufacturer_data();
                    return {true, true, CmdClassifierType::reply};
                }
                break;
            // EEBus_UC_TS_EVSECommissioningAndConfiguration_V1.0.1.pdf 3.2.1.2.2.1 Scenario 2: Operating State / Error Reporting
            case SpineDataTypeHandler::Function::deviceDiagnosisStateData:
                if (feature_address == feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis)) {
                    response["deviceDiagnosisStateData"] = EVSEEntity::get_state_data();
                    return {true, true, CmdClassifierType::reply};
                }
                break;
            default:;
        }
    }
    return {false};
}

UseCaseInformationDataType EvseccUsecase::get_usecase_information()
{
    UseCaseInformationDataType evse_usecase;
    evse_usecase.actor = "EVSE";

    UseCaseSupportType evse_usecase_support;
    evse_usecase_support.useCaseName = "evseCommissioningAndConfiguration";
    evse_usecase_support.useCaseVersion = "1.0.1";
    evse_usecase_support.scenarioSupport->push_back(1);
    evse_usecase_support.scenarioSupport->push_back(2);
    evse_usecase_support.useCaseDocumentSubRevision = "release";
    evse_usecase.useCaseSupport->push_back(evse_usecase_support);

    FeatureAddressType evcs_usecase_feature_address;
    evcs_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    evcs_usecase_feature_address.entity = entity_address;
    evse_usecase.address = evcs_usecase_feature_address;
    return evse_usecase;
}

NodeManagementDetailedDiscoveryEntityInformationType EvseccUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EVSE;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "EVSE"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> EvseccUsecase::get_detailed_discovery_feature_information() const
{

    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    // The following functions are needed by the DeviceClassification Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType device_classification{};
    device_classification.description->featureAddress->entity = entity_address;
    device_classification.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::DeviceClassification);
    device_classification.description->featureType = FeatureTypeEnumType::DeviceClassification;
    device_classification.description->role = RoleType::server;

    // deviceClassificationManufacturerData
    FunctionPropertyType device_configuration_description{};
    device_configuration_description.function = FunctionEnumType::deviceClassificationManufacturerData;
    device_configuration_description.possibleOperations->read = PossibleOperationsReadType{};
    device_classification.description->supportedFunction->push_back(device_configuration_description);
    features.push_back(device_classification);

    // The following functions are needed by the DeviceDiagnosis Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType device_diagnosis{};
    device_diagnosis.description->featureAddress->entity = entity_address;
    device_diagnosis.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis);
    device_diagnosis.description->featureType = FeatureTypeEnumType::DeviceDiagnosis;
    device_diagnosis.description->role = RoleType::server;

    //identificationListData
    FunctionPropertyType device_diagnosis_state{};
    device_diagnosis_state.function = FunctionEnumType::deviceDiagnosisStateData;
    device_diagnosis_state.possibleOperations->read = PossibleOperationsReadType{};
    device_diagnosis.description->supportedFunction->push_back(device_diagnosis_state);
    features.push_back(device_diagnosis);

    return features;
}

void EvseccUsecase::update_operating_state(const bool failure, const String &error_message)
{
    if (failure) {
        operating_state = DeviceDiagnosisOperatingStateEnumType::failure;
        last_error_message = error_message.c_str();
    } else {
        operating_state = DeviceDiagnosisOperatingStateEnumType::normalOperation;
        last_error_message = "";
    }
    DeviceDiagnosisStateDataType state = EVSEEntity::get_state_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis), state, "deviceDiagnosisStateData");
    update_api();
}

void EvseccUsecase::get_device_diagnosis_state(DeviceDiagnosisStateDataType *data) const
{
    data->lastErrorCode = last_error_message;
    data->operatingState = operating_state;
}

void EvseccUsecase::get_device_classification_manufacturer(DeviceClassificationManufacturerDataType *data)
{
    data->deviceName = eebus.get_eebus_name().c_str();
    data->deviceCode = OPTIONS_PRODUCT_NAME_USER_AGENT();
    data->vendorName = OPTIONS_MANUFACTURER_FULL();
    data->vendorCode = OPTIONS_HOSTNAME_PREFIX();
    data->brandName = OPTIONS_CONFIG_TYPE();
    data->manufacturerLabel = OPTIONS_MANUAL_URL();
}

void EvseccUsecase::update_api() const
{
    auto api_entry = eebus.eebus_usecase_state.get("evse_commissioning_and_configuration");
    api_entry->get("evse_failure")->updateBool(operating_state != DeviceDiagnosisOperatingStateEnumType::normalOperation);
    api_entry->get("evse_failure_description")->updateString(last_error_message.c_str());
}
#endif

// =============================================================================
// LoadPowerLimitUsecase - Base class for LPC and LPP usecases
// =============================================================================
#if defined(EEBUS_ENABLE_LPC_USECASE) || defined(EEBUS_ENABLE_LPP_USECASE)

// Base class constructor - initializes IDs from config offsets
LoadPowerLimitUsecase::LoadPowerLimitUsecase(const LoadPowerLimitConfig &config) :
    config_(config), id_l_1(config.loadcontrol_limit_id_offset + 1), id_m_1(config.measurement_id_offset + 1), id_k_1(config.device_config_key_id_offset + 1), id_k_2(config.device_config_key_id_offset + 2), id_ec_1(config.electrical_connection_id_offset + 1), id_cc_1(config.electrical_connection_characteristic_id_offset + 1), id_cc_2(config.electrical_connection_characteristic_id_offset + 2), id_p_1(config.electrical_connection_parameter_id_offset + 1), limit_description_id(id_l_1),
    limit_measurement_description_id(id_m_1), failsafe_power_key_id(id_k_1), failsafe_duration_key_id(id_k_2)
{
    task_scheduler.scheduleOnce(
        [this]() {
            // Register for heartbeat (Scenario 3)
            eebus.usecases->evse_heartbeat.register_usecase_for_heartbeat(this);
            eebus.usecases->evse_heartbeat.set_autosubscribe(true);
            update_state(); // Initialize state machine
            update_api();
        },
        1_s); // Schedule all the init stuff a bit delayed to allow other entities to initialize first
}

UseCaseInformationDataType LoadPowerLimitUsecase::get_usecase_information()
{
    UseCaseInformationDataType usecase;
    usecase.actor = "ControllableSystem";

    UseCaseSupportType usecase_support;
    usecase_support.useCaseName = config_.usecase_name;
    usecase_support.useCaseVersion = "1.0.0";
    usecase_support.scenarioSupport->push_back(1);
    usecase_support.scenarioSupport->push_back(2);
    usecase_support.scenarioSupport->push_back(3);
    usecase_support.scenarioSupport->push_back(4);
    usecase_support.useCaseDocumentSubRevision = "release";
    usecase.useCaseSupport->push_back(usecase_support);

    FeatureAddressType usecase_feature_address;
    usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    usecase_feature_address.entity = entity_address;
    usecase.address = usecase_feature_address;
    return usecase;
}

MessageReturn LoadPowerLimitUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    switch (get_feature_by_address(header.addressDestination->feature.get())) {
        case FeatureTypeEnumType::LoadControl:
            return load_control_feature(header, data, response);
        case FeatureTypeEnumType::DeviceConfiguration:
            return deviceConfiguration_feature(header, data, response);
        case FeatureTypeEnumType::ElectricalConnection:
            return electricalConnection_feature(header, data, response);
        default:;
    }
    return {false};
}

NodeManagementDetailedDiscoveryEntityInformationType LoadPowerLimitUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
#ifndef EEBUS_MODE_EM
    entity.description->entityType = EntityTypeEnumType::EVSE;
#else
    entity.description->entityType = EntityTypeEnumType::CEM;
#endif
    entity.description->label = "Controllable System";
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> LoadPowerLimitUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    // LoadControl Feature
    NodeManagementDetailedDiscoveryFeatureInformationType loadControlFeature{};
    loadControlFeature.description->featureAddress->entity = entity_address;
    loadControlFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::LoadControl);
    loadControlFeature.description->featureType = FeatureTypeEnumType::LoadControl;
    loadControlFeature.description->role = RoleType::server;

    FunctionPropertyType loadControlDescriptionList{};
    loadControlDescriptionList.function = FunctionEnumType::loadControlLimitDescriptionListData;
    loadControlDescriptionList.possibleOperations->read = PossibleOperationsReadType{};
    loadControlFeature.description->supportedFunction->push_back(loadControlDescriptionList);

    FunctionPropertyType loadControlLimitListData{};
    loadControlLimitListData.function = FunctionEnumType::loadControlLimitListData;
    loadControlLimitListData.possibleOperations->read = PossibleOperationsReadType{};
    loadControlLimitListData.possibleOperations->write = PossibleOperationsWriteType{};
    loadControlLimitListData.possibleOperations->write->partial = ElementTagType{};
    loadControlFeature.description->supportedFunction->push_back(loadControlLimitListData);
    features.push_back(loadControlFeature);

    // DeviceConfiguration Feature
    NodeManagementDetailedDiscoveryFeatureInformationType deviceConfigurationFeature{};
    deviceConfigurationFeature.description->featureAddress->entity = entity_address;
    deviceConfigurationFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration);
    deviceConfigurationFeature.description->featureType = FeatureTypeEnumType::DeviceConfiguration;
    deviceConfigurationFeature.description->role = RoleType::server;

    FunctionPropertyType deviceConfigurationKeyValueDescriptionListData{};
    deviceConfigurationKeyValueDescriptionListData.function = FunctionEnumType::deviceConfigurationKeyValueDescriptionListData;
    deviceConfigurationKeyValueDescriptionListData.possibleOperations->read = PossibleOperationsReadType{};
    deviceConfigurationFeature.description->supportedFunction->push_back(deviceConfigurationKeyValueDescriptionListData);

    FunctionPropertyType deviceConfigurationKeyValueListData{};
    deviceConfigurationKeyValueListData.function = FunctionEnumType::deviceConfigurationKeyValueListData;
    deviceConfigurationKeyValueListData.possibleOperations->read = PossibleOperationsReadType{};
    deviceConfigurationKeyValueListData.possibleOperations->write = PossibleOperationsWriteType{};
    deviceConfigurationFeature.description->supportedFunction->push_back(deviceConfigurationKeyValueListData);
    features.push_back(deviceConfigurationFeature);

    // ElectricalConnection Feature
    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnectionFeature{};
    electricalConnectionFeature.description->featureAddress->entity = entity_address;
    electricalConnectionFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::ElectricalConnection);
    electricalConnectionFeature.description->featureType = FeatureTypeEnumType::ElectricalConnection;
    electricalConnectionFeature.description->role = RoleType::server;

    FunctionPropertyType electricalConnectionCharacteristicsListData{};
    electricalConnectionCharacteristicsListData.function = FunctionEnumType::electricalConnectionCharacteristicListData;
    electricalConnectionCharacteristicsListData.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionCharacteristicsListData);
    features.push_back(electricalConnectionFeature);

    return features;
}

// LoadControl feature handler
MessageReturn LoadPowerLimitUsecase::load_control_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    if (header.cmdClassifier == CmdClassifierType::read) {
        if (data->last_cmd == SpineDataTypeHandler::Function::loadControlLimitDescriptionListData) {
            response["loadControlLimitDescriptionListData"] = EVSEEntity::get_load_control_limit_description_list_data();
            return {true, true, CmdClassifierType::reply};
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::loadControlLimitListData) {
            response["loadControlLimitListData"] = EVSEEntity::get_load_control_limit_list_data();
            return {true, true, CmdClassifierType::reply};
        }
    }
    if (header.cmdClassifier == CmdClassifierType::write) {
        FeatureAddressType feature_address{};
        feature_address.entity = entity_address;
        feature_address.feature = feature_addresses.at(FeatureTypeEnumType::LoadControl);
        feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
        bool is_bound = eebus.usecases->node_management.check_is_bound(header.addressSource.get(), feature_address);
        if (!is_bound) {
            eebus.trace_fmtln("Received write from an unbound node");
            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::BindingRequired, "Load Control requires binding");
            return {true, true, CmdClassifierType::result};
        }
        switch (data->last_cmd) {
            case SpineDataTypeHandler::Function::loadControlLimitListData:
                if (data->loadcontrollimitlistdatatype.has_value() && !data->loadcontrollimitlistdatatype->loadControlLimitData->empty()) {
                    for (LoadControlLimitDataType load_control_limit_data : data->loadcontrollimitlistdatatype->loadControlLimitData.get()) {
                        if (load_control_limit_data.limitId == id_l_1) {
                            bool limit_enabled = load_control_limit_data.isLimitActive.get();
                            const int new_limit_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(*load_control_limit_data.value);
                            const seconds_t duration_s = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(load_control_limit_data.timePeriod->endTime.get());
                            logger.printfln("Received a Loadcontrol Limit for a %s usecase. Attempting to apply limit. Limit is: %d W, duration: %d s, enabled: %d", get_usecases_name(config_.usecase_type), new_limit_w, duration_s.as<int>(), limit_enabled);
                            if (!update_limit(limit_enabled, new_limit_w, duration_s)) {
                                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Limit not accepted");
                                logger.printfln("Limit not accepted");
                                return {true, true, CmdClassifierType::result};
                            }
                            logger.printfln("Limit accepted");
                            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
                            return {true, true, CmdClassifierType::result};
                        }
                    }
                    return {false};
                }
                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Limit not accepted or invalid data");
                break;
            default:
                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Unknown command");
        }
        return {true, true, CmdClassifierType::result};
    }
    return {false};
}

// DeviceConfiguration feature handler
MessageReturn LoadPowerLimitUsecase::deviceConfiguration_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    switch (data->last_cmd) {
        case SpineDataTypeHandler::Function::deviceConfigurationKeyValueDescriptionListData:
            switch (header.cmdClassifier.get()) {
                case CmdClassifierType::read: {
                    response["deviceConfigurationKeyValueDescriptionListData"] = EVSEEntity::get_device_configuration_list_data();
                    return {true, true, CmdClassifierType::reply};
                }
                default:
                    EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandNotSupported, "This cmdclassifier is not supported on this function");
                    return {true, true, CmdClassifierType::result};
            }
        case SpineDataTypeHandler::Function::deviceConfigurationKeyValueListData:
            switch (header.cmdClassifier.get()) {
                case CmdClassifierType::read: {
                    response["deviceConfigurationKeyValueListData"] = EVSEEntity::get_device_configuration_value_list_data();
                    return {true, true, CmdClassifierType::reply};
                }
                case CmdClassifierType::write:
                    if (eebus.usecases->node_management.check_is_bound(header.addressSource.get(), header.addressDestination.get())) {
                        auto new_config = data->deviceconfigurationkeyvaluelistdatatype.get();
                        int new_failsafe_power = -1;
                        seconds_t new_failsafe_duration = -1_s;
                        for (const auto &list_entry : new_config.deviceConfigurationKeyValueData.get()) {
                            if (list_entry.keyId == failsafe_power_key_id) {
                                new_failsafe_power = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(*list_entry.value->scaledNumber);
                            } else if (list_entry.keyId == failsafe_duration_key_id) {
                                new_failsafe_duration = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(list_entry.value->duration.get());
                            }
                        }
                        update_failsafe(new_failsafe_power, new_failsafe_duration);
                        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "Configuration updated successfully");
                        return {true, true, CmdClassifierType::result};
                    }
                    EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::BindingRequired, "DeviceConfiguration requires binding");
                    return {true, true, CmdClassifierType::result};

                default:
                    EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandNotSupported, "This cmdclassifier is not supported on this function");
                    return {true, true, CmdClassifierType::result};
            }
        default:
            return {false};
    }

    return {false};
}

// ElectricalConnection feature handler
MessageReturn LoadPowerLimitUsecase::electricalConnection_feature(const HeaderType &header, const SpineDataTypeHandler *data, JsonObject response)
{
    if (header.cmdClassifier == CmdClassifierType::read && data->last_cmd == SpineDataTypeHandler::Function::electricalConnectionCharacteristicListData) {
        response["electricalConnectionCharacteristicListData"] = EVSEEntity::get_electrical_connection_characteristic_list_data();
        return {true, true, CmdClassifierType::reply};
    }
    return {false};
}

void LoadPowerLimitUsecase::update_failsafe(int power_limit_w, seconds_t duration)
{
    if (power_limit_w > -1) {
        failsafe_power_limit_w = power_limit_w;
    }
    if (duration > -1_s) {
        failsafe_duration = duration;
    }
    if (power_limit_w > 0 || duration > 0_s) {
        logger.printfln("Updated failsafe to %d W for %d seconds", failsafe_power_limit_w, failsafe_duration.as<int>());
        update_state();
        update_api();
        auto data = EVSEEntity::get_device_configuration_value_list_data();
        eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration), data, "deviceConfigurationKeyValueListData");
    }
}

void LoadPowerLimitUsecase::update_constraints(int power_max, int power_contract_max)
{
    if (power_max > 0) {
        power_max_w = power_max;
    }
    if (power_contract_max > 0) {
        power_contract_max_w = power_contract_max;
    }
    auto data = EVSEEntity::get_electrical_connection_characteristic_list_data();
    eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::ElectricalConnection), data, "electricalConnectionCharacteristicListData");
}

bool LoadPowerLimitUsecase::update_limit(bool limit, int current_limit_w, const seconds_t duration)
{
    // For LPC: limit_received when power > 0; for LPP: when power < 0
    if (config_.limit_is_positive) {
        limit_received = current_limit_w > 0 || limit_received;
    } else {
        limit_received = current_limit_w < 0 || limit_received;
    }

    // Evaluate if the limit can be applied
    if (duration <= 0_s && !limit_active && limit_received && limit) {
        // In case the duration is 0 (meaning until further notice) and the limit is not active, reject the limit
        limit_active = false;
        return false;
    }
    limit_active = limit;

    // For LPC: reject limit < 0; for LPP: reject limit > 0
    if (config_.limit_is_positive) {
        if (current_limit_w < 0) {
            limit_active = false;
            return false;
        }
    } else {
        if (current_limit_w > 0) {
            limit_active = false;
            return false;
        }
    }
    // TODO: Add more limit validation here.
    // For example: Can this limit be applied electrically.

    configured_limit = current_limit_w;

    if (duration > 0_s && limit_active) {
        limit_expired = false;
        timeval time_v{};
        rtc.clock_synced(&time_v);
        limit_endtime = time_v.tv_sec + duration.as<int>();
        task_scheduler.cancel(limit_endtime_timer);
        limit_endtime_timer = task_scheduler.scheduleOnce(
            [this]() {
                if (state_ == LoadcontrolState::Limited) {
                    logger.printfln("Limit duration expired");
                    limit_expired = true;
                    update_state();
                    update_api();
                }
            },
            duration);
    } else if (duration == 0_s && limit_active) {
        // A value of 0 means the limit is valid until further notice
    }
    update_state();
    update_api();

    LoadControlLimitListDataType data{};
    get_loadcontrol_limit_list(&data);
    eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::LoadControl), data, "loadControlLimitListData");
    return true;
}

void LoadPowerLimitUsecase::update_state()
{
    eebus.trace_fmtln("Updating state. Current state: %s. Heartbeat received: %d, Limit received: %d, Limit active: %d, Limit expired: %d", get_loadcontrol_state_name(state_), heartbeat_received, limit_received, limit_active, limit_expired);

    switch (state_) {
        case LoadcontrolState::Startup:
            init_state();
            break;
        case LoadcontrolState::Init:
            if (heartbeat_received && limit_received && !limit_active) {
                unlimited_controlled_state();
            } else if (heartbeat_received && limit_received && limit_active) {
                limited_state();
            } else {
                unlimited_autonomous_state();
            }
            break;
        case LoadcontrolState::UnlimitedControlled:
            if (heartbeat_received && limit_received && limit_active) {
                limited_state();
            } else if (!heartbeat_received) {
                failsafe_state();
            }
            break;
        case LoadcontrolState::Limited:
            if (!heartbeat_received) {
                failsafe_state();
            } else if (limit_expired || (limit_received && !limit_active)) {
                unlimited_controlled_state();
            } else if (state_ == LoadcontrolState::Limited) {
                limited_state();
            }
            break;
        case LoadcontrolState::Failsafe:
            if (heartbeat_received && limit_received && !limit_active) {
                unlimited_controlled_state();
            } else if (heartbeat_received && limit_received && limit_active) {
                limited_state();
            } else if (failsafe_expired) {
                unlimited_autonomous_state();
            }
            break;
        case LoadcontrolState::UnlimitedAutonomous:
            if (heartbeat_received && limit_received && !limit_active) {
                unlimited_controlled_state();
            } else if (heartbeat_received && limit_received && limit_active) {
                limited_state();
            }
            break;
    }
}

void LoadPowerLimitUsecase::receive_heartbeat()
{
    heartbeat_received = true;
    update_state();
    update_api();
}

void LoadPowerLimitUsecase::receive_heartbeat_timeout()
{
    heartbeat_received = false;
    logger.printfln("No Heartbeat received from control box. Switching to failsafe or unlimited/autonomous mode");
    update_state();
    update_api();
}

void LoadPowerLimitUsecase::inform_spineconnection_usecase_update(SpineConnection *conn)
{
    auto peers = conn->get_address_of_feature(FeatureTypeEnumType::DeviceDiagnosis, RoleType::client, config_.usecase_name, "EnergyGuard");
    for (FeatureAddressType &peer : peers) {
        eebus.usecases->evse_heartbeat.initialize_heartbeat_on_feature(peer, config_.usecase_type, true);
    }
}

void LoadPowerLimitUsecase::init_state()
{
    limit_active = false;
    current_active_limit_w = failsafe_power_limit_w;
    state_ = LoadcontrolState::Init;
}

void LoadPowerLimitUsecase::unlimited_controlled_state()
{
    state_ = LoadcontrolState::UnlimitedControlled;
    limit_active = false;
    current_active_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
}

void LoadPowerLimitUsecase::limited_state()
{
    timeval time_v{};
    rtc.clock_synced(&time_v);
    long long duration_left = limit_endtime - time_v.tv_sec;
    if (state_ != LoadcontrolState::Limited) {
        logger.printfln("Received a limit of %d W valid for %lld s", configured_limit, duration_left);
    } else if (current_active_limit_w != configured_limit) {
        logger.printfln("Updating limit to %d W", configured_limit);
    }

    state_ = LoadcontrolState::Limited;
    current_active_limit_w = configured_limit;
    limit_active = true;
}

void LoadPowerLimitUsecase::failsafe_state()
{
    state_ = LoadcontrolState::Failsafe;
    limit_active = false;

    current_active_limit_w = failsafe_power_limit_w;
    task_scheduler.cancel(limit_endtime_timer);

    timeval time_v{};
    rtc.clock_synced(&time_v);
    failsafe_expiry_endtime = time_v.tv_sec + failsafe_duration.as<int>();
    failsafe_expiry_timer = task_scheduler.scheduleOnce(
        [this]() {
            if (state_ == LoadcontrolState::Failsafe) {
                logger.printfln("Failsafe duration expired. Switching to autonomous/unlimited mode");
                failsafe_expired = true;
                update_state();
                update_api();
            }
        },
        failsafe_duration);
}

void LoadPowerLimitUsecase::unlimited_autonomous_state()
{
    state_ = LoadcontrolState::UnlimitedAutonomous;
    limit_active = false;
    current_active_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
}

void LoadPowerLimitUsecase::update_api() const
{
    auto api_entry = eebus.eebus_usecase_state.get(config_.api_key);
    api_entry->get("usecase_state")->updateEnum(state_);
    api_entry->get("limit_active")->updateBool(limit_active);
    // For LPP, we use abs() to display the limit as positive in the UI
    api_entry->get("current_limit")->updateUint(config_.limit_is_positive ? current_active_limit_w : abs(current_active_limit_w));
    api_entry->get("failsafe_limit_power_w")->updateUint(failsafe_power_limit_w);
    api_entry->get("failsafe_limit_duration_s")->updateUint(failsafe_duration.as<uint32_t>());

    timeval now{};
    rtc.clock_synced(&now);
    if (state_ == LoadcontrolState::Limited) {
        const long long duration_left = limit_endtime - now.tv_sec;
        api_entry->get("outstanding_duration_s")->updateUint(duration_left > 0 ? duration_left : 0);
    } else if (state_ == LoadcontrolState::Failsafe) {
        const long long failsafe_time_left = failsafe_expiry_endtime - now.tv_sec;
        api_entry->get("outstanding_duration_s")->updateUint(failsafe_time_left > 0 ? failsafe_time_left : 0);
    } else {
        api_entry->get("outstanding_duration_s")->updateUint(0);
    }

    api_entry->get("constraints_power_maximum")->updateUint(power_max_w);
}

void LoadPowerLimitUsecase::get_loadcontrol_limit_description(LoadControlLimitDescriptionListDataType *data) const
{
    LoadControlLimitDescriptionDataType limit_description{};
    limit_description.limitId = limit_description_id;
    limit_description.limitType = LoadControlLimitTypeEnumType::signDependentAbsValueLimit;
    limit_description.limitCategory = LoadControlCategoryEnumType::obligation;
    limit_description.limitDirection = config_.energy_direction;
    limit_description.measurementId = limit_measurement_description_id;
    limit_description.unit = UnitOfMeasurementEnumType::W;
    limit_description.scopeType = ScopeTypeEnumType::activePowerLimit;
    data->loadControlLimitDescriptionData->push_back(limit_description);
}

void LoadPowerLimitUsecase::get_loadcontrol_limit_list(LoadControlLimitListDataType *data) const
{
    timeval now{};
    rtc.clock_synced(&now);
    const long long duration_left = limit_endtime - now.tv_sec;

    LoadControlLimitDataType limit_data{};
    limit_data.limitId = limit_description_id;
    limit_data.isLimitChangeable = !limit_fixed;
    limit_data.isLimitActive = limit_active;
    if (duration_left > 0) {
        limit_data.timePeriod->endTime = EEBUS_USECASE_HELPERS::iso_duration_to_string(seconds_t(duration_left));
    }
    limit_data.value->number = current_active_limit_w;
    limit_data.value->scale = 0;
    data->loadControlLimitData->push_back(limit_data);
}

void LoadPowerLimitUsecase::get_device_configuration_value(DeviceConfigurationKeyValueListDataType *data) const
{
    DeviceConfigurationKeyValueDataType failsafe_power_key_value{};
    failsafe_power_key_value.isValueChangeable = true;
    failsafe_power_key_value.keyId = failsafe_power_key_id;
    failsafe_power_key_value.value->scaledNumber->number = failsafe_power_limit_w;
    failsafe_power_key_value.value->scaledNumber->scale = 0;
    data->deviceConfigurationKeyValueData->push_back(failsafe_power_key_value);

    DeviceConfigurationKeyValueDataType failsafe_duration_key_value{};
    failsafe_duration_key_value.isValueChangeable = true;
    failsafe_duration_key_value.keyId = failsafe_duration_key_id;
    failsafe_duration_key_value.value->duration = EEBUS_USECASE_HELPERS::iso_duration_to_string(failsafe_duration);
    data->deviceConfigurationKeyValueData->push_back(failsafe_duration_key_value);
}

void LoadPowerLimitUsecase::get_device_configuration_description(DeviceConfigurationKeyValueDescriptionListDataType *data) const
{
    DeviceConfigurationKeyValueDescriptionDataType failsafe_power_description{};
    failsafe_power_description.keyId = failsafe_power_key_id;
    failsafe_power_description.keyName = config_.failsafe_key_name;
    failsafe_power_description.unit = UnitOfMeasurementEnumType::W;
    failsafe_power_description.valueType = DeviceConfigurationKeyValueTypeType::scaledNumber;
    data->deviceConfigurationKeyValueDescriptionData->push_back(failsafe_power_description);

    DeviceConfigurationKeyValueDescriptionDataType failsafe_duration_description{};
    failsafe_duration_description.keyId = failsafe_duration_key_id;
    failsafe_duration_description.keyName = DeviceConfigurationKeyNameEnumType::failsafeDurationMinimum;
    failsafe_duration_description.valueType = DeviceConfigurationKeyValueTypeType::duration;
    data->deviceConfigurationKeyValueDescriptionData->push_back(failsafe_duration_description);
}

void LoadPowerLimitUsecase::get_electrical_connection_characteristic(ElectricalConnectionCharacteristicListDataType *data) const
{
    ElectricalConnectionCharacteristicDataType power_max{};
    power_max.electricalConnectionId = id_ec_1;
    power_max.parameterId = id_p_1;
    power_max.characteristicId = id_cc_1;
    power_max.characteristicContext = ElectricalConnectionCharacteristicContextEnumType::entity;
    power_max.characteristicType = config_.characteristic_type;
    power_max.value->number = power_max_w;
    power_max.unit = UnitOfMeasurementEnumType::W;
    data->electricalConnectionCharacteristicData->push_back(power_max);
}
#endif // defined(EEBUS_ENABLE_LPC_USECASE) || defined(EEBUS_ENABLE_LPP_USECASE)

// =============================================================================
// LpcUsecase - Thin wrapper around LoadPowerLimitUsecase
// =============================================================================
#ifdef EEBUS_ENABLE_LPC_USECASE

// Static configuration for LPC (Limitation of Power Consumption)
const LoadPowerLimitConfig LpcUsecase::lpc_config = {
    .usecase_type = Usecases::LPC,
    .usecase_name = "limitationOfPowerConsumption",
    .api_key = "power_consumption_limitation",
    .energy_direction = EnergyDirectionEnumType::consume,
    .characteristic_type = ElectricalConnectionCharacteristicTypeEnumType::powerConsumptionMax,
    .failsafe_key_name = DeviceConfigurationKeyNameEnumType::failsafeConsumptionActivePowerLimit,
    .limit_is_positive = true,
    .loadcontrol_limit_id_offset = EVSEEntity::lpcLoadcontrolLimitIdOffset,
    .measurement_id_offset = EVSEEntity::lpcMeasurementIdOffset,
    .device_config_key_id_offset = EVSEEntity::lpcDeviceConfigurationKeyIdOffset,
    .electrical_connection_id_offset = EVSEEntity::lpcElectricalConnectionIdOffset,
    .electrical_connection_characteristic_id_offset = EVSEEntity::lpcElectricalConnectionCharacteristicIdOffset,
    .electrical_connection_parameter_id_offset = EVSEEntity::lpcElectricalConnectionParameterIdOffset,
};

LpcUsecase::LpcUsecase() : LoadPowerLimitUsecase(lpc_config)
{
}
#endif // EEBUS_ENABLE_LPC_USECASE

// =============================================================================
// LppUsecase - Thin wrapper around LoadPowerLimitUsecase
// =============================================================================
#ifdef EEBUS_ENABLE_LPP_USECASE

// Static configuration for LPP (Limitation of Power Production)
const LoadPowerLimitConfig LppUsecase::lpp_config = {
    .usecase_type = Usecases::LPP,
    .usecase_name = "limitationOfPowerProduction",
    .api_key = "power_production_limitation",
    .energy_direction = EnergyDirectionEnumType::produce,
    .characteristic_type = ElectricalConnectionCharacteristicTypeEnumType::powerProductionMax,
    .failsafe_key_name = DeviceConfigurationKeyNameEnumType::failsafeProductionActivePowerLimit,
    .limit_is_positive = false,
    .loadcontrol_limit_id_offset = EVSEEntity::lppLoadcontrolLimitIdOffset,
    .measurement_id_offset = EVSEEntity::lppMeasurementIdOffset,
    .device_config_key_id_offset = EVSEEntity::lppDeviceConfigurationKeyIdOffset,
    .electrical_connection_id_offset = EVSEEntity::lppElectricalConnectionIdOffset,
    .electrical_connection_characteristic_id_offset = EVSEEntity::lppElectricalConnectionCharacteristicIdOffset,
    .electrical_connection_parameter_id_offset = EVSEEntity::lppElectricalConnectionParameterIdOffset,
};

LppUsecase::LppUsecase() : LoadPowerLimitUsecase(lpp_config)
{
}
#endif // EEBUS_ENABLE_LPP_USECASE

#ifdef EEBUS_ENABLE_MPC_USECASE
MessageReturn MpcUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    if (header.cmdClassifier == CmdClassifierType::read) {
        switch (get_feature_by_address(header.addressDestination->feature.get())) {
            case FeatureTypeEnumType::ElectricalConnection:
                switch (data->last_cmd) {
                    case SpineDataTypeHandler::Function::electricalConnectionDescriptionListData:
                        response["electricalConnectionDescriptionListData"] = EVSEEntity::get_electrical_connection_description_list_data();
                        return {true, true, CmdClassifierType::reply};
                    case SpineDataTypeHandler::Function::electricalConnectionParameterDescriptionListData:
                        response["electricalConnectionParameterDescriptionListData"] = EVSEEntity::get_electrical_connection_parameter_description_list_data();
                        return {true, true, CmdClassifierType::reply};
                    default:;
                }
            case FeatureTypeEnumType::Measurement:
                switch (data->last_cmd) {
                    case SpineDataTypeHandler::Function::measurementDescriptionListData:
                        response["measurementDescriptionListData"] = EVSEEntity::get_measurement_description_list_data();
                        return {true, true, CmdClassifierType::reply};
                    case SpineDataTypeHandler::Function::measurementConstraintsListData:
                        response["measurementConstraintsListData"] = EVSEEntity::get_measurement_constraints_list_data();
                        return {true, true, CmdClassifierType::reply};
                    case SpineDataTypeHandler::Function::measurementListData:
                        response["measurementListData"] = EVSEEntity::get_measurement_list_data();
                        return {true, true, CmdClassifierType::reply};
                    default:;
                }
            default:;
        }
    }
    return {false};
}
UseCaseInformationDataType MpcUsecase::get_usecase_information()
{
    UseCaseInformationDataType mpc_usecase;
    mpc_usecase.actor = "MonitoredUnit";

    UseCaseSupportType mpc_usecase_support;
    mpc_usecase_support.useCaseName = "monitoringOfPowerConsumption";
    mpc_usecase_support.useCaseVersion = "1.0.0";
    // Scenario 1: Monitor Power - Mandatory
    mpc_usecase_support.scenarioSupport->push_back(1);
    // Scenario 2: Monior Energy - Optional
    mpc_usecase_support.scenarioSupport->push_back(2);
    // Scenario 3: Monitor Current - Recommend
    mpc_usecase_support.scenarioSupport->push_back(3);
    // Scenario 4: Monitor Voltage - Optional
    mpc_usecase_support.scenarioSupport->push_back(4);
    // Scenario 5: Monitor frequency - Optional
    mpc_usecase_support.scenarioSupport->push_back(5);
    mpc_usecase_support.useCaseDocumentSubRevision = "release";
    mpc_usecase.useCaseSupport->push_back(mpc_usecase_support);

    FeatureAddressType mpc_usecase_feature_address{};
    mpc_usecase_feature_address.entity = entity_address;
    mpc_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    mpc_usecase.address = mpc_usecase_feature_address;

    return mpc_usecase;
}
NodeManagementDetailedDiscoveryEntityInformationType MpcUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EVSE;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "Monitored Unit"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}
std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> MpcUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    // See EEBUS_UC_TS_MonitoringOfPowerConsumption_v1.0.0.pdf
    // The following functions are needed by the ElectricalConection Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnectionFeature{};
    electricalConnectionFeature.description->featureAddress->entity = entity_address;
    electricalConnectionFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::ElectricalConnection);
    electricalConnectionFeature.description->featureType = FeatureTypeEnumType::ElectricalConnection;
    electricalConnectionFeature.description->role = RoleType::server;

    // electricalConnectionDescriptionListdata
    FunctionPropertyType electricalConnectionDescriptionListData{};
    electricalConnectionDescriptionListData.function = FunctionEnumType::electricalConnectionDescriptionListData;
    electricalConnectionDescriptionListData.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionDescriptionListData);

    // electricalConnectionParameterDescriptionListData
    FunctionPropertyType electricalConnectionParameterDescriptionListData{};
    electricalConnectionParameterDescriptionListData.function = FunctionEnumType::electricalConnectionParameterDescriptionListData;
    electricalConnectionParameterDescriptionListData.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionParameterDescriptionListData);
    features.push_back(electricalConnectionFeature);

    // The following functions are needed by the Measurement Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType measurementFeature{};
    measurementFeature.description->featureAddress->entity = entity_address;
    measurementFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::Measurement);
    measurementFeature.description->featureType = FeatureTypeEnumType::Measurement;
    measurementFeature.description->role = RoleType::server;

    // measurementDescriptionListData
    FunctionPropertyType measurementDescriptionListData{};
    measurementDescriptionListData.function = FunctionEnumType::measurementDescriptionListData;
    measurementDescriptionListData.possibleOperations->read = PossibleOperationsReadType{};
    measurementFeature.description->supportedFunction->push_back(measurementDescriptionListData);

    // measurementConstraintsListData
    FunctionPropertyType measurementConstraintsListData{};
    measurementConstraintsListData.function = FunctionEnumType::measurementConstraintsListData;
    measurementConstraintsListData.possibleOperations->read = PossibleOperationsReadType{};
    measurementFeature.description->supportedFunction->push_back(measurementConstraintsListData);

    // measurementListData
    FunctionPropertyType measurementListData{};
    measurementListData.function = FunctionEnumType::measurementListData;
    measurementListData.possibleOperations->read = PossibleOperationsReadType{};
    measurementFeature.description->supportedFunction->push_back(measurementListData);
    features.push_back(measurementFeature);

    return features;
}
void MpcUsecase::get_electricalConnection_description_list_data(ElectricalConnectionDescriptionListDataType *data)
{
    ElectricalConnectionDescriptionDataType ec_description{};
    ec_description.electricalConnectionId = id_ec_1;
    ec_description.powerSupplyType = ElectricalConnectionVoltageTypeEnumType::ac;
    ec_description.positiveEnergyDirection = EnergyDirectionEnumType::consume; // If this is set to consume, it means positive values are consumption, negative values production (i assume)
    data->electricalConnectionDescriptionData->push_back(ec_description);
}
void MpcUsecase::get_electricalConnection_parameter_description_list_data(ElectricalConnectionParameterDescriptionListDataType *data) const
{
    {
        // This links the parameter, electrical connection to the measurement ID for the measuring of total wattage
        ElectricalConnectionParameterDescriptionDataType w_description_data{};
        w_description_data.electricalConnectionId = id_ec_1;
        w_description_data.parameterId = id_p_1;
        w_description_data.measurementId = id_m_1;
        w_description_data.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
        w_description_data.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::abc;
        w_description_data.acMeasuredInReferenceTo = ElectricalConnectionPhaseNameEnumType::neutral;
        w_description_data.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::real;
        w_description_data.acMeasurementVariant = ElectricalConnectionMeasurandVariantEnumType::rms;
        data->electricalConnectionParameterDescriptionData->push_back(w_description_data);
    }
    {

        constexpr std::array<std::pair<std::pair<uint8_t, uint8_t>, ElectricalConnectionPhaseNameEnumType>, 6> id_triples{{
            // This links the parameter, electrical connection to the measurement ID for the measuring of wattage per phase
            {{id_p_2_1, id_m_2_1}, ElectricalConnectionPhaseNameEnumType::a},
            {{id_p_2_2, id_m_2_2}, ElectricalConnectionPhaseNameEnumType::b},
            {{id_p_2_3, id_m_2_3}, ElectricalConnectionPhaseNameEnumType::c},
            // This links the parameter, electrical connection to the measurement ID for the measuring of current per phase
            {{id_p_5_1, id_m_5_1}, ElectricalConnectionPhaseNameEnumType::a},
            {{id_p_5_2, id_m_5_2}, ElectricalConnectionPhaseNameEnumType::b},
            {{id_p_5_3, id_m_5_3}, ElectricalConnectionPhaseNameEnumType::c},
        }};
        for (const auto &id_pair : id_triples) {

            ElectricalConnectionParameterDescriptionDataType a_description_data{};
            a_description_data.electricalConnectionId = id_ec_1;
            a_description_data.parameterId = id_pair.first.first;
            a_description_data.measurementId = id_pair.first.second;
            a_description_data.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
            a_description_data.acMeasuredPhases = id_pair.second;
            a_description_data.acMeasuredInReferenceTo = ElectricalConnectionPhaseNameEnumType::neutral;
            a_description_data.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::real;
            a_description_data.acMeasurementVariant = ElectricalConnectionMeasurandVariantEnumType::rms;
            data->electricalConnectionParameterDescriptionData->push_back(a_description_data);
        }
    }
    {
        // This links the parameter, electrical connection to the measurement ID for the measuring of energy consumed (watt hours)
        ElectricalConnectionParameterDescriptionDataType w_description_data{};
        w_description_data.electricalConnectionId = id_ec_1;
        w_description_data.parameterId = id_p_3;
        w_description_data.measurementId = id_m_3;
        w_description_data.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
        w_description_data.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::real;
        data->electricalConnectionParameterDescriptionData->push_back(w_description_data);
    }
    {
        // This links the parameter, electrical connection to the measurement ID for the measuring of energy produced (watt hours)
        ElectricalConnectionParameterDescriptionDataType w_description_data{};
        w_description_data.electricalConnectionId = id_ec_1;
        w_description_data.parameterId = id_p_4;
        w_description_data.measurementId = id_m_4;
        w_description_data.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
        w_description_data.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::real;
        data->electricalConnectionParameterDescriptionData->push_back(w_description_data);
    }
    {
        // This links the parameter, electrical connection to the measurement ID for the measuring of voltage per phase relative to other phases
        struct RelativePhaseAssignment {
            uint8_t parameter_id;
            uint8_t measurement_id;
            ElectricalConnectionPhaseNameEnumType measured_phase;
            ElectricalConnectionPhaseNameEnumType measured_in_reference_to;
        };
        constexpr std::array<RelativePhaseAssignment, 6> relative_phase_assignments{{
            {id_p_6_1, id_m_6_1, ElectricalConnectionPhaseNameEnumType::a, ElectricalConnectionPhaseNameEnumType::neutral},
            {id_p_6_2, id_m_6_2, ElectricalConnectionPhaseNameEnumType::b, ElectricalConnectionPhaseNameEnumType::neutral},
            {id_p_6_3, id_m_6_3, ElectricalConnectionPhaseNameEnumType::c, ElectricalConnectionPhaseNameEnumType::neutral},
            {id_p_6_4, id_m_6_4, ElectricalConnectionPhaseNameEnumType::a, ElectricalConnectionPhaseNameEnumType::b},
            {id_p_6_5, id_m_6_5, ElectricalConnectionPhaseNameEnumType::b, ElectricalConnectionPhaseNameEnumType::c},
            {id_p_6_6, id_m_6_6, ElectricalConnectionPhaseNameEnumType::c, ElectricalConnectionPhaseNameEnumType::a},
        }};
        for (const auto &assignment : relative_phase_assignments) {
            ElectricalConnectionParameterDescriptionDataType current_description_data{};
            current_description_data.electricalConnectionId = id_ec_1;
            current_description_data.parameterId = assignment.parameter_id;
            current_description_data.measurementId = assignment.measurement_id;
            current_description_data.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
            current_description_data.acMeasuredPhases = assignment.measured_phase;
            current_description_data.acMeasuredInReferenceTo = assignment.measured_in_reference_to;
            current_description_data.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::apparent;
            current_description_data.acMeasurementVariant = ElectricalConnectionMeasurandVariantEnumType::rms;
            data->electricalConnectionParameterDescriptionData->push_back(current_description_data);
        }
    }
    {
        // This links the parameter, electrical connection to the measurement ID for the measuring of frequency
        ElectricalConnectionParameterDescriptionDataType w_description_data{};
        w_description_data.electricalConnectionId = id_ec_1;
        w_description_data.parameterId = id_p_7;
        w_description_data.measurementId = id_m_7;
        w_description_data.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
        data->electricalConnectionParameterDescriptionData->push_back(w_description_data);
    }
}
void MpcUsecase::get_measurement_description_list_data(MeasurementDescriptionListDataType *data)
{
    struct MeasurementDescriptionEntry {
        uint8_t measurement_id;
        MeasurementTypeEnumType measurement_type;
        UnitOfMeasurementEnumType unit;
        ScopeTypeEnumType scope;
    };
    constexpr std::array<MeasurementDescriptionEntry, 11> measurement_descriptions{{
        {id_m_1, MeasurementTypeEnumType::power, UnitOfMeasurementEnumType::W, ScopeTypeEnumType::acPowerTotal},
        {id_m_2_1, MeasurementTypeEnumType::power, UnitOfMeasurementEnumType::W, ScopeTypeEnumType::acPower},
        {id_m_2_2, MeasurementTypeEnumType::power, UnitOfMeasurementEnumType::W, ScopeTypeEnumType::acPower},
        {id_m_2_3, MeasurementTypeEnumType::power, UnitOfMeasurementEnumType::W, ScopeTypeEnumType::acPower},
        {id_m_3, MeasurementTypeEnumType::energy, UnitOfMeasurementEnumType::Wh, ScopeTypeEnumType::acEnergyConsumed},
        {id_m_4, MeasurementTypeEnumType::energy, UnitOfMeasurementEnumType::Wh, ScopeTypeEnumType::acEnergyProduced},
        {id_m_5_1, MeasurementTypeEnumType::current, UnitOfMeasurementEnumType::A, ScopeTypeEnumType::acCurrent},
        {id_m_5_2, MeasurementTypeEnumType::current, UnitOfMeasurementEnumType::A, ScopeTypeEnumType::acCurrent},
        {id_m_5_3, MeasurementTypeEnumType::current, UnitOfMeasurementEnumType::A, ScopeTypeEnumType::acCurrent},
        {id_m_6_1, MeasurementTypeEnumType::voltage, UnitOfMeasurementEnumType::V, ScopeTypeEnumType::acVoltage},
        {id_m_7, MeasurementTypeEnumType::frequency, UnitOfMeasurementEnumType::Hz, ScopeTypeEnumType::acFrequency},
    }};

    for (const auto &entry : measurement_descriptions) {
        MeasurementDescriptionDataType measurement_description{};
        measurement_description.measurementId = entry.measurement_id;
        measurement_description.measurementType = entry.measurement_type;
        measurement_description.commodityType = CommodityTypeEnumType::electricity;
        measurement_description.unit = entry.unit;
        measurement_description.scopeType = entry.scope;
        data->measurementDescriptionData->push_back(measurement_description);
    }
}
void MpcUsecase::get_measurement_constraints_list_data(MeasurementConstraintsListDataType *data) const
{
    // Constraint entries for all measurements
    // Format: {measurement_id, min, max, stepsize, scale}
    struct ConstraintEntry {
        uint8_t measurement_id;
        int32_t min;
        int32_t max;
        int32_t stepsize;
        int8_t scale;
    };

    // Use non-constexpr array since we access member variables
    const std::array<ConstraintEntry, 17> entries{{
        // Power measurements (W) - scale 0
        {id_m_1, power_limit_min_w, power_limit_max_w, power_limit_stepsize_w, 0},
        {id_m_2_1, power_limit_min_w, power_limit_max_w, power_limit_stepsize_w, 0},
        {id_m_2_2, power_limit_min_w, power_limit_max_w, power_limit_stepsize_w, 0},
        {id_m_2_3, power_limit_min_w, power_limit_max_w, power_limit_stepsize_w, 0},
        // Energy measurements (Wh) - scale 0
        {id_m_3, static_cast<int32_t>(energy_limit_min_wh), static_cast<int32_t>(energy_limit_max_wh), static_cast<int32_t>(energy_limit_stepsize_wh), 0},
        {id_m_4, static_cast<int32_t>(energy_limit_min_wh), static_cast<int32_t>(energy_limit_max_wh), static_cast<int32_t>(energy_limit_stepsize_wh), 0},
        // Current measurements (A) - scale -3 (mA)
        {id_m_5_1, current_limit_min_ma, current_limit_max_ma, current_limit_stepsize_ma, -3},
        {id_m_5_2, current_limit_min_ma, current_limit_max_ma, current_limit_stepsize_ma, -3},
        {id_m_5_3, current_limit_min_ma, current_limit_max_ma, current_limit_stepsize_ma, -3},
        // Voltage measurements (V) - scale 0
        {id_m_6_1, voltage_limit_min_v, voltage_limit_max_v, voltage_limit_stepsize_v, 0},
        {id_m_6_2, voltage_limit_min_v, voltage_limit_max_v, voltage_limit_stepsize_v, 0},
        {id_m_6_3, voltage_limit_min_v, voltage_limit_max_v, voltage_limit_stepsize_v, 0},
        {id_m_6_4, voltage_limit_min_v, voltage_limit_max_v, voltage_limit_stepsize_v, 0},
        {id_m_6_5, voltage_limit_min_v, voltage_limit_max_v, voltage_limit_stepsize_v, 0},
        {id_m_6_6, voltage_limit_min_v, voltage_limit_max_v, voltage_limit_stepsize_v, 0},
        // Frequency (Hz) - scale -3 (mHz)
        {id_m_7, frequency_limit_min_mhz, frequency_limit_max_mhz, frequency_limit_stepsize_mhz, -3},
    }};

    for (const auto &entry : entries) {
        MeasurementConstraintsDataType constraint{};
        constraint.measurementId = entry.measurement_id;
        constraint.valueRangeMin->number = entry.min;
        constraint.valueRangeMin->scale = entry.scale;
        constraint.valueRangeMax->number = entry.max;
        constraint.valueRangeMax->scale = entry.scale;
        constraint.valueStepSize->number = entry.stepsize;
        constraint.valueStepSize->scale = entry.scale;
        data->measurementConstraintsData->push_back(constraint);
    }
}

void MpcUsecase::get_measurement_list_data(MeasurementListDataType *data) const
{
    // Measurement entries - using individual entries instead of array
    // to avoid 'this' in constant expression
    // NOTE: MPC allows all values including 0 and negative (e.g., power feed-in)

    // Total power (id_m_1) - always included, negative values allowed (power feed-in)
    MeasurementDataType m_total{};
    m_total.measurementId = id_m_1;
    m_total.valueType = MeasurementValueTypeEnumType::value;
    m_total.value->number = total_power_w;
    m_total.value->scale = 0;
    m_total.valueSource = MeasurementValueSourceEnumType::measuredValue;
    data->measurementData->push_back(m_total);

    // Power per phase (id_m_2_1, id_m_2_2, id_m_2_3) - all values allowed
    for (int i = 0; i < 3; i++) {
        MeasurementDataType m{};
        m.measurementId = id_m_2_1 + i;
        m.valueType = MeasurementValueTypeEnumType::value;
        m.value->number = power_phase_w[i];
        m.value->scale = 0;
        m.valueSource = MeasurementValueSourceEnumType::measuredValue;
        data->measurementData->push_back(m);
    }

    // Energy consumed (id_m_3) - all values allowed
    MeasurementDataType m_consumed{};
    m_consumed.measurementId = id_m_3;
    m_consumed.valueType = MeasurementValueTypeEnumType::value;
    m_consumed.value->number = static_cast<int32_t>(energy_consumed_wh);
    m_consumed.value->scale = 0;
    m_consumed.valueSource = MeasurementValueSourceEnumType::measuredValue;
    data->measurementData->push_back(m_consumed);

    // Energy produced (id_m_4) - all values allowed
    MeasurementDataType m_produced{};
    m_produced.measurementId = id_m_4;
    m_produced.valueType = MeasurementValueTypeEnumType::value;
    m_produced.value->number = static_cast<int32_t>(energy_produced_wh);
    m_produced.value->scale = 0;
    m_produced.valueSource = MeasurementValueSourceEnumType::measuredValue;
    data->measurementData->push_back(m_produced);

    // Current per phase (id_m_5_1, id_m_5_2, id_m_5_3) - scale -3 (mA), all values allowed
    for (int i = 0; i < 3; i++) {
        MeasurementDataType m{};
        m.measurementId = id_m_5_1 + i;
        m.valueType = MeasurementValueTypeEnumType::value;
        m.value->number = current_phase_ma[i];
        m.value->scale = -3;
        m.valueSource = MeasurementValueSourceEnumType::measuredValue;
        data->measurementData->push_back(m);
    }

    // Voltage phase-to-neutral (id_m_6_1, id_m_6_2, id_m_6_3) - all values allowed
    for (int i = 0; i < 3; i++) {
        MeasurementDataType m{};
        m.measurementId = id_m_6_1 + i;
        m.valueType = MeasurementValueTypeEnumType::value;
        m.value->number = voltage_phase_to_neutral_v[i];
        m.value->scale = 0;
        m.valueSource = MeasurementValueSourceEnumType::measuredValue;
        data->measurementData->push_back(m);
    }

    // Voltage phase-to-phase (id_m_6_4, id_m_6_5, id_m_6_6) - all values allowed
    for (int i = 0; i < 3; i++) {
        MeasurementDataType m{};
        m.measurementId = id_m_6_4 + i;
        m.valueType = MeasurementValueTypeEnumType::value;
        m.value->number = voltage_phase_to_phase_v[i];
        m.value->scale = 0;
        m.valueSource = MeasurementValueSourceEnumType::measuredValue;
        data->measurementData->push_back(m);
    }

    // Frequency (id_m_7) - scale -3 (mHz), all values allowed
    MeasurementDataType m_freq{};
    m_freq.measurementId = id_m_7;
    m_freq.valueType = MeasurementValueTypeEnumType::value;
    m_freq.value->number = frequency_mhz;
    m_freq.value->scale = -3;
    m_freq.valueSource = MeasurementValueSourceEnumType::measuredValue;
    data->measurementData->push_back(m_freq);
}

void MpcUsecase::update_api() const
{
    // Update API state for monitoring
    auto api_entry = eebus.eebus_usecase_state.get("monitoring_of_power_consumption");
    api_entry->get("total_power_w")->updateInt(total_power_w);
    api_entry->get("power_phase_1_w")->updateInt(power_phase_w[0]);
    api_entry->get("power_phase_2_w")->updateInt(power_phase_w[1]);
    api_entry->get("power_phase_3_w")->updateInt(power_phase_w[2]);
    api_entry->get("energy_consumed_wh")->updateInt(energy_consumed_wh);
    api_entry->get("energy_produced_wh")->updateInt(energy_produced_wh);
    api_entry->get("current_phase_1_ma")->updateInt(current_phase_ma[0]);
    api_entry->get("current_phase_2_ma")->updateInt(current_phase_ma[1]);
    api_entry->get("current_phase_3_ma")->updateInt(current_phase_ma[2]);
    api_entry->get("voltage_phase_1_v")->updateInt(voltage_phase_to_neutral_v[0]);
    api_entry->get("voltage_phase_2_v")->updateInt(voltage_phase_to_neutral_v[1]);
    api_entry->get("voltage_phase_3_v")->updateInt(voltage_phase_to_neutral_v[2]);
    api_entry->get("voltage_phase_1_2_v")->updateInt(voltage_phase_to_phase_v[0]);
    api_entry->get("voltage_phase_2_3_v")->updateInt(voltage_phase_to_phase_v[1]);
    api_entry->get("voltage_phase_3_1_v")->updateInt(voltage_phase_to_phase_v[2]);
    api_entry->get("frequency_mhz")->updateUint(frequency_mhz);
}

MpcUsecase::MpcUsecase()
{
    // Initialize with default values
    task_scheduler.scheduleOnce(
        [this]() {
            update_api();
        },
        1_s); // Schedule init delayed to allow other entities to initialize first
}

void MpcUsecase::update_power(int total_power, int power_phase_1, int power_phase_2, int power_phase_3)
{
    // Update all values including 0 and negative (negative = power feed-in)
    total_power_w = total_power;
    power_phase_w[0] = power_phase_1;
    power_phase_w[1] = power_phase_2;
    power_phase_w[2] = power_phase_3;

    // Inform subscribers of measurement data
    MeasurementListDataType measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MpcUsecase::update_energy(uint32_t energy_consumed, uint32_t energy_produced)
{
    // Update all values including 0
    energy_consumed_wh = energy_consumed;
    energy_produced_wh = energy_produced;

    // Inform subscribers of measurement data
    MeasurementListDataType measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MpcUsecase::update_current(int current_phase_1, int current_phase_2, int current_phase_3)
{
    // Update all values including 0 and negative (negative = reverse current)
    current_phase_ma[0] = current_phase_1;
    current_phase_ma[1] = current_phase_2;
    current_phase_ma[2] = current_phase_3;

    // Inform subscribers of measurement data
    MeasurementListDataType measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MpcUsecase::update_voltage(int voltage_phase_1, int voltage_phase_2, int voltage_phase_3, int voltage_phase_1_2, int voltage_phase_2_3, int voltage_phase_3_1)
{
    // Update all values including 0
    voltage_phase_to_neutral_v[0] = voltage_phase_1;
    voltage_phase_to_neutral_v[1] = voltage_phase_2;
    voltage_phase_to_neutral_v[2] = voltage_phase_3;
    voltage_phase_to_phase_v[0] = voltage_phase_1_2;
    voltage_phase_to_phase_v[1] = voltage_phase_2_3;
    voltage_phase_to_phase_v[2] = voltage_phase_3_1;

    // Inform subscribers of measurement data
    MeasurementListDataType measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MpcUsecase::update_frequency(int frequency_millihertz)
{
    // Update value including 0
    frequency_mhz = frequency_millihertz;

    // Inform subscribers of measurement data
    MeasurementListDataType measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MpcUsecase::update_constraints(int power_min, int power_max, int power_stepsize, int current_min_ma, int current_max_ma, int current_stepsize_ma, uint32_t energy_min_wh, uint32_t energy_max_wh, uint32_t energy_stepsize_wh, int voltage_min, int voltage_max, int voltage_stepsize, int frequency_min_mhz, int frequency_max_mhz, int frequency_stepsize_mhz)
{
    // Update power constraints
    if (power_min >= 0) {
        power_limit_min_w = power_min;
    }
    if (power_max > 0) {
        power_limit_max_w = power_max;
    }
    if (power_stepsize > 0) {
        power_limit_stepsize_w = power_stepsize;
    }

    // Update current constraints
    if (current_min_ma >= 0) {
        current_limit_min_ma = current_min_ma;
    }
    if (current_max_ma > 0) {
        current_limit_max_ma = current_max_ma;
    }
    if (current_stepsize_ma > 0) {
        current_limit_stepsize_ma = current_stepsize_ma;
    }

    // Update energy constraints
    if (energy_min_wh > 0) {
        energy_limit_min_wh = energy_min_wh;
    }
    if (energy_max_wh > 0) {
        energy_limit_max_wh = energy_max_wh;
    }
    if (energy_stepsize_wh > 0) {
        energy_limit_stepsize_wh = energy_stepsize_wh;
    }

    // Update voltage constraints
    if (voltage_min >= 0) {
        voltage_limit_min_v = voltage_min;
    }
    if (voltage_max > 0) {
        voltage_limit_max_v = voltage_max;
    }
    if (voltage_stepsize > 0) {
        voltage_limit_stepsize_v = voltage_stepsize;
    }

    // Update frequency constraints
    if (frequency_min_mhz > 0) {
        frequency_limit_min_mhz = frequency_min_mhz;
    }
    if (frequency_max_mhz > 0) {
        frequency_limit_max_mhz = frequency_max_mhz;
    }
    if (frequency_stepsize_mhz > 0) {
        frequency_limit_stepsize_mhz = frequency_stepsize_mhz;
    }

    // Inform subscribers of constraint changes
    MeasurementConstraintsListDataType constraints_data = EVSEEntity::get_measurement_constraints_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), constraints_data, "measurementConstraintsListData");

    update_api();
}

#endif
// ==============================================================================
// CEVC - Coordinated EV Charging Use Case
// Spec: EEBus_UC_TS_CoordinatedEVCharging_V1.0.1.pdf
//
// STATUS: IMPLEMENTED
//
// Scenarios:
//   1 (3.4.1): EV sends charging energy demand - TimeSeries singleDemand - COMPLETE
//   2 (3.4.2): Max power limitation - TimeSeries constraints - COMPLETE
//   3 (3.4.3): Incentive table - IncentiveTable for tariff data - COMPLETE
//   4 (3.4.4): Charging plan curve - TimeSeries plan (write) - COMPLETE
//   5 (3.4.5): Energy Guard heartbeat - Via EebusHeartBeat - COMPLETE
//   6 (3.4.6): Energy Broker heartbeat - Via EebusHeartBeat - COMPLETE
//   7 (3.4.7): Energy Guard error state - Via DeviceDiagnosis - COMPLETE
//   8 (3.4.8): Energy Broker error state - Via DeviceDiagnosis - COMPLETE
// ==============================================================================
#ifdef EEBUS_ENABLE_CEVC_USECASE
CevcUsecase::CevcUsecase() = default;

MessageReturn CevcUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    CmdClassifierType cmd = header.cmdClassifier.get();

    switch (data->last_cmd) {
        // =====================================================================
        // TimeSeries Feature - 3.2.1.2.1
        // =====================================================================

        // 3.2.1.2.1.1: Function "timeSeriesDescriptionListData" - Time series metadata
        case SpineDataTypeHandler::Function::timeSeriesDescriptionListData:
            if (cmd == CmdClassifierType::read) {
                TimeSeriesDescriptionListDataType ts_desc;
                read_time_series_description(&ts_desc);
                response["timeSeriesDescriptionListData"] = ts_desc;
                return {true, true, CmdClassifierType::reply};
            }
            break;

        // 3.2.1.2.1.2: Function "timeSeriesConstraintsListData" - Time series constraints
        case SpineDataTypeHandler::Function::timeSeriesConstraintsListData:
            if (cmd == CmdClassifierType::read) {
                TimeSeriesConstraintsListDataType ts_constraints;
                read_time_series_constraints(&ts_constraints);
                response["timeSeriesConstraintsListData"] = ts_constraints;
                return {true, true, CmdClassifierType::reply};
            }
            break;

        // 3.2.1.2.1.3: Function "timeSeriesListData" - Actual charging demand/plan data
        case SpineDataTypeHandler::Function::timeSeriesListData:
            if (cmd == CmdClassifierType::read) {
                TimeSeriesListDataType ts_list;
                read_time_series_list(&ts_list);
                response["timeSeriesListData"] = ts_list;
                return {true, true, CmdClassifierType::reply};
            }
            if (cmd == CmdClassifierType::write) {
                return write_time_series_list(header, data->timeserieslistdatatype, response);
            }
            break;

        // =====================================================================
        // IncentiveTable Feature - 3.2.1.2.2
        // =====================================================================

        // 3.2.1.2.2.1: Function "incentiveTableDescriptionData" - Tariff metadata
        case SpineDataTypeHandler::Function::incentiveTableDescriptionData:
            if (cmd == CmdClassifierType::read) {
                IncentiveTableDescriptionDataType it_desc;
                read_incentive_table_description(&it_desc);
                response["incentiveTableDescriptionData"] = it_desc;
                return {true, true, CmdClassifierType::reply};
            }
            if (cmd == CmdClassifierType::write) {
                return write_incentive_table_description(header, data->incentivetabledescriptiondatatype, response);
            }
            break;

        // 3.2.1.2.2.2: Function "incentiveTableConstraintsData" - Tariff constraints
        case SpineDataTypeHandler::Function::incentiveTableConstraintsData:
            if (cmd == CmdClassifierType::read) {
                IncentiveTableConstraintsDataType it_constraints;
                read_incentive_table_constraints(&it_constraints);
                response["incentiveTableConstraintsData"] = it_constraints;
                return {true, true, CmdClassifierType::reply};
            }
            break;

        // 3.2.1.2.2.3: Function "incentiveTableData" - Actual tariff/incentive data
        case SpineDataTypeHandler::Function::incentiveTableData:
            if (cmd == CmdClassifierType::read) {
                IncentiveTableDataType it_data;
                read_incentive_table_data(&it_data);
                response["incentiveTableData"] = it_data;
                return {true, true, CmdClassifierType::reply};
            }
            if (cmd == CmdClassifierType::write) {
                return write_incentive_table_data(header, data->incentivetabledatatype, response);
            }
            break;

        default:
            break;
    }
    return {false};
}

UseCaseInformationDataType CevcUsecase::get_usecase_information()
{
    UseCaseInformationDataType cevc_usecase;
    cevc_usecase.actor = "EV";

    UseCaseSupportType cevc_usecase_support;
    cevc_usecase_support.useCaseName = "coordinatedEvCharging";
    cevc_usecase_support.useCaseVersion = "1.0.1";
    cevc_usecase_support.scenarioSupport->insert(cevc_usecase_support.scenarioSupport->end(), {1, 2, 3, 4, 5, 6, 7, 8});
    cevc_usecase_support.useCaseDocumentSubRevision = "release";
    cevc_usecase.useCaseSupport->push_back(cevc_usecase_support);

    FeatureAddressType cevc_usecase_feature_address;
    cevc_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    cevc_usecase_feature_address.entity = entity_address;
    cevc_usecase.address = cevc_usecase_feature_address;
    return cevc_usecase;
}

NodeManagementDetailedDiscoveryEntityInformationType CevcUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return entity;
    }
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EV;
    entity.description->label = "EV";
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> CevcUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return features;
    }

    // =========================================================================
    // TimeSeries Feature - See EEBUS UC TS CoordinatedEvCharging v1.0.1.pdf 3.2.1.2.1
    // =========================================================================
    NodeManagementDetailedDiscoveryFeatureInformationType timeseries_feature{};
    timeseries_feature.description->featureAddress->entity = entity_address;
    timeseries_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::TimeSeries);
    timeseries_feature.description->featureType = FeatureTypeEnumType::TimeSeries;
    timeseries_feature.description->role = RoleType::server;

    // timeSeriesDescriptionListData - read only
    FunctionPropertyType timeseries_description{};
    timeseries_description.function = FunctionEnumType::timeSeriesDescriptionListData;
    timeseries_description.possibleOperations->read = PossibleOperationsReadType{};
    timeseries_feature.description->supportedFunction->push_back(timeseries_description);

    // timeSeriesConstraintsListData - read only
    FunctionPropertyType timeseries_constraints{};
    timeseries_constraints.function = FunctionEnumType::timeSeriesConstraintsListData;
    timeseries_constraints.possibleOperations->read = PossibleOperationsReadType{};
    timeseries_feature.description->supportedFunction->push_back(timeseries_constraints);

    // timeSeriesListData - read and write (write for receiving charging plan)
    FunctionPropertyType timeseries_data{};
    timeseries_data.function = FunctionEnumType::timeSeriesListData;
    timeseries_data.possibleOperations->read = PossibleOperationsReadType{};
    timeseries_data.possibleOperations->write = PossibleOperationsWriteType{};
    timeseries_feature.description->supportedFunction->push_back(timeseries_data);
    features.push_back(timeseries_feature);

    // =========================================================================
    // IncentiveTable Feature - See EEBUS UC TS CoordinatedEvCharging v1.0.1.pdf 3.2.1.2.2
    // =========================================================================
    NodeManagementDetailedDiscoveryFeatureInformationType incentive_table_feature{};
    incentive_table_feature.description->featureAddress->entity = entity_address;
    incentive_table_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::IncentiveTable);
    incentive_table_feature.description->featureType = FeatureTypeEnumType::IncentiveTable;
    incentive_table_feature.description->role = RoleType::server;

    // incentiveTableDescriptionData - read and write
    FunctionPropertyType incentivetable_description{};
    incentivetable_description.function = FunctionEnumType::incentiveTableDescriptionData;
    incentivetable_description.possibleOperations->read = PossibleOperationsReadType{};
    incentivetable_description.possibleOperations->write = PossibleOperationsWriteType{};
    incentive_table_feature.description->supportedFunction->push_back(incentivetable_description);

    // incentiveTableConstraintsData - read only
    FunctionPropertyType incentivetable_constraints{};
    incentivetable_constraints.function = FunctionEnumType::incentiveTableConstraintsData;
    incentivetable_constraints.possibleOperations->read = PossibleOperationsReadType{};
    incentive_table_feature.description->supportedFunction->push_back(incentivetable_constraints);

    // incentiveTableData - read and write
    FunctionPropertyType incentivetable_data{};
    incentivetable_data.function = FunctionEnumType::incentiveTableData;
    incentivetable_data.possibleOperations->read = PossibleOperationsReadType{};
    incentivetable_data.possibleOperations->write = PossibleOperationsWriteType{};
    incentive_table_feature.description->supportedFunction->push_back(incentivetable_data);
    features.push_back(incentive_table_feature);

    return features;
}

// =============================================================================
// TimeSeries Feature Implementation
// =============================================================================

// CEVC Spec 3.2.1.2.1.1: Time series description - describes available time series
void CevcUsecase::read_time_series_description(TimeSeriesDescriptionListDataType *data) const
{
    // We provide three time series:
    // 1. singleDemand - EV's charging energy demand (Scenario 1)
    // 2. constraints - Maximum power limits (Scenario 2)
    // 3. plan - Charging plan from Energy Broker (Scenario 4, writable)

    // Time Series 1: Single Demand (Scenario 1)
    // Describes the EV's overall charging energy demand
    TimeSeriesDescriptionDataType demand_desc{};
    demand_desc.timeSeriesId = TIME_SERIES_ID_DEMAND;
    demand_desc.timeSeriesType = TimeSeriesTypeEnumType::singleDemand;
    demand_desc.timeSeriesWriteable = false; // EV provides this, not writable by broker
    demand_desc.updateRequired = false;
    demand_desc.unit = UnitOfMeasurementEnumType::Wh;
    demand_desc.label = "Charging Demand";
    demand_desc.description = "EV charging energy demand";
    data->timeSeriesDescriptionData->push_back(demand_desc);

    // Time Series 2: Constraints (Scenario 2)
    // Describes the maximum power the EV can consume over time
    TimeSeriesDescriptionDataType constraints_desc{};
    constraints_desc.timeSeriesId = TIME_SERIES_ID_CONSTRAINTS;
    constraints_desc.timeSeriesType = TimeSeriesTypeEnumType::constraints;
    constraints_desc.timeSeriesWriteable = false; // EV provides this
    constraints_desc.updateRequired = false;
    constraints_desc.unit = UnitOfMeasurementEnumType::W;
    constraints_desc.label = "Power Constraints";
    constraints_desc.description = "Maximum power consumption constraints";
    data->timeSeriesDescriptionData->push_back(constraints_desc);

    // Time Series 3: Plan (Scenario 4)
    // The charging plan received from the Energy Broker
    TimeSeriesDescriptionDataType plan_desc{};
    plan_desc.timeSeriesId = TIME_SERIES_ID_PLAN;
    plan_desc.timeSeriesType = TimeSeriesTypeEnumType::plan;
    plan_desc.timeSeriesWriteable = true; // Broker can write the plan
    plan_desc.updateRequired = true;      // Plan should be updated regularly
    plan_desc.unit = UnitOfMeasurementEnumType::W;
    plan_desc.label = "Charging Plan";
    plan_desc.description = "Charging power plan from Energy Broker";
    data->timeSeriesDescriptionData->push_back(plan_desc);
}

// CEVC Spec 3.2.1.2.1.2: Time series constraints - slot count, duration, value ranges
void CevcUsecase::read_time_series_constraints(TimeSeriesConstraintsListDataType *data) const
{
    // Constraints for singleDemand time series
    TimeSeriesConstraintsDataType demand_constraints{};
    demand_constraints.timeSeriesId = TIME_SERIES_ID_DEMAND;
    demand_constraints.slotCountMin = "1";
    demand_constraints.slotCountMax = "1"; // Single demand is just one value
    // Value constraints for energy in Wh
    demand_constraints.slotValueMin->number = 0;
    demand_constraints.slotValueMin->scale = 0;
    demand_constraints.slotValueMax->number = 100000; // 100 kWh max
    demand_constraints.slotValueMax->scale = 0;
    data->timeSeriesConstraintsData->push_back(demand_constraints);

    // Constraints for constraints time series
    TimeSeriesConstraintsDataType power_constraints{};
    power_constraints.timeSeriesId = TIME_SERIES_ID_CONSTRAINTS;
    power_constraints.slotCountMin = "1";
    power_constraints.slotCountMax = std::to_string(MAX_TIME_SERIES_SLOTS);
    power_constraints.slotDurationMin = "PT1M";  // Minimum 1 minute per slot
    power_constraints.slotDurationMax = "PT24H"; // Maximum 24 hours per slot
    // Value constraints for power in W
    power_constraints.slotValueMin->number = 0;
    power_constraints.slotValueMin->scale = 0;
    power_constraints.slotValueMax->number = 22000; // 22 kW max
    power_constraints.slotValueMax->scale = 0;
    data->timeSeriesConstraintsData->push_back(power_constraints);

    // Constraints for plan time series
    TimeSeriesConstraintsDataType plan_constraints{};
    plan_constraints.timeSeriesId = TIME_SERIES_ID_PLAN;
    plan_constraints.slotCountMin = "1";
    plan_constraints.slotCountMax = std::to_string(MAX_TIME_SERIES_SLOTS);
    plan_constraints.slotDurationMin = "PT1M";  // Minimum 1 minute per slot
    plan_constraints.slotDurationMax = "PT24H"; // Maximum 24 hours per slot
    // Value constraints for power in W
    plan_constraints.slotValueMin->number = 0;
    plan_constraints.slotValueMin->scale = 0;
    plan_constraints.slotValueMax->number = 22000; // 22 kW max
    plan_constraints.slotValueMax->scale = 0;
    data->timeSeriesConstraintsData->push_back(plan_constraints);
}

// CEVC Spec 3.2.1.2.1.3: Time series data - actual charging demand and constraints
void CevcUsecase::read_time_series_list(TimeSeriesListDataType *data) const
{
    // =========================================================================
    // Time Series 1: Single Demand (Scenario 1)
    // =========================================================================
    TimeSeriesDataType demand_data{};
    demand_data.timeSeriesId = TIME_SERIES_ID_DEMAND;

    // Set the time period for the demand (now until target departure)
    if (target_departure_time > 0) {
        time_t now = time(nullptr);
        demand_data.timePeriod->startTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(now).c_str();
        demand_data.timePeriod->endTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(target_departure_time).c_str();
    }

    // Create demand slot with min/opt/max energy values
    TimeSeriesSlotType demand_slot{};
    demand_slot.timeSeriesSlotId = 1;
    // Use minValue for minimum energy, value for optimal, maxValue for maximum
    demand_slot.minValue->number = minimum_energy_wh;
    demand_slot.minValue->scale = 0;
    demand_slot.value->number = optimal_energy_wh;
    demand_slot.value->scale = 0;
    demand_slot.maxValue->number = maximum_energy_wh;
    demand_slot.maxValue->scale = 0;
    demand_data.timeSeriesSlot->push_back(demand_slot);
    data->timeSeriesData->push_back(demand_data);

    // =========================================================================
    // Time Series 2: Constraints (Scenario 2)
    // =========================================================================
    TimeSeriesDataType constraints_data{};
    constraints_data.timeSeriesId = TIME_SERIES_ID_CONSTRAINTS;

    if (power_constraint_slots.empty()) {
        // If no detailed slots, provide a single constraint with max power
        TimeSeriesSlotType constraint_slot{};
        constraint_slot.timeSeriesSlotId = 1;
        constraint_slot.maxValue->number = max_power_constraint_w;
        constraint_slot.maxValue->scale = 0;
        constraints_data.timeSeriesSlot->push_back(constraint_slot);
    } else {
        // Provide detailed constraint slots
        int slot_id = 1;
        for (const auto &slot : power_constraint_slots) {
            TimeSeriesSlotType constraint_slot{};
            constraint_slot.timeSeriesSlotId = slot_id++;
            constraint_slot.duration = EEBUS_USECASE_HELPERS::iso_duration_to_string(slot.duration);
            constraint_slot.maxValue->number = slot.max_power_w;
            constraint_slot.maxValue->scale = 0;
            constraints_data.timeSeriesSlot->push_back(constraint_slot);
        }
    }
    data->timeSeriesData->push_back(constraints_data);

    // =========================================================================
    // Time Series 3: Plan (Scenario 4) - only if we have received a plan
    // =========================================================================
    if (charging_plan_valid && !received_charging_plan.empty()) {
        TimeSeriesDataType plan_data{};
        plan_data.timeSeriesId = TIME_SERIES_ID_PLAN;

        if (charging_plan_start_time > 0) {
            plan_data.timePeriod->startTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(charging_plan_start_time).c_str();
        }

        int slot_id = 1;
        for (const auto &slot : received_charging_plan) {
            TimeSeriesSlotType plan_slot{};
            plan_slot.timeSeriesSlotId = slot_id++;
            plan_slot.duration = EEBUS_USECASE_HELPERS::iso_duration_to_string(slot.duration);
            plan_slot.value->number = slot.power_w;
            plan_slot.value->scale = 0;
            plan_data.timeSeriesSlot->push_back(plan_slot);
        }
        data->timeSeriesData->push_back(plan_data);
    }
}

// CEVC Spec 3.4.4: Receive charging plan from Energy Broker
MessageReturn CevcUsecase::write_time_series_list(HeaderType &header, SpineOptional<TimeSeriesListDataType> data, JsonObject response)
{
    if (!data.has_value() || !data->timeSeriesData.has_value()) {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Invalid time series data");
        return {true, true, CmdClassifierType::result};
    }

    bool plan_updated = false;

    for (const auto &ts : data->timeSeriesData.get()) {
        // We only accept writes to the plan time series (ID 3)
        if (!ts.timeSeriesId.has_value()) {
            continue;
        }

        int ts_id = ts.timeSeriesId.get();
        if (ts_id != TIME_SERIES_ID_PLAN) {
            eebus.trace_fmtln("CEVC: Ignoring write to time series ID %d (only plan ID %d is writable)", ts_id, TIME_SERIES_ID_PLAN);
            continue;
        }

        // Parse the charging plan
        received_charging_plan.clear();
        charging_plan_valid = false;

        // Extract start time if provided
        if (ts.timePeriod.has_value() && ts.timePeriod->startTime.has_value()) {
            EEBUS_USECASE_HELPERS::iso_timestamp_to_unix(ts.timePeriod->startTime->c_str(), &charging_plan_start_time);
        } else {
            charging_plan_start_time = time(nullptr);
        }

        // Parse each slot
        if (ts.timeSeriesSlot.has_value()) {
            for (const auto &slot : ts.timeSeriesSlot.get()) {
                ChargingPlanSlot plan_slot{};

                // Parse duration
                if (slot.duration.has_value()) {
                    plan_slot.duration = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(slot.duration.get());
                } else {
                    plan_slot.duration = 900_s; // Default 15 minutes if not specified
                }

                // Parse power value
                if (slot.value.has_value()) {
                    plan_slot.power_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(slot.value.get());
                } else if (slot.maxValue.has_value()) {
                    // If no value, use maxValue as the target
                    plan_slot.power_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(slot.maxValue.get());
                }

                received_charging_plan.push_back(plan_slot);

                // Limit to max slots
                if (received_charging_plan.size() >= MAX_TIME_SERIES_SLOTS) {
                    break;
                }
            }
        }

        if (!received_charging_plan.empty()) {
            charging_plan_valid = true;
            plan_updated = true;
            eebus.trace_fmtln("CEVC: Received charging plan with %d slots", received_charging_plan.size());
        }
    }

    if (plan_updated) {
        notify_subscribers_time_series();
        update_api_state();
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
    } else {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "No valid plan data found");
    }

    return {true, true, CmdClassifierType::result};
}

// =============================================================================
// IncentiveTable Feature Implementation
// =============================================================================

// CEVC Spec 3.2.1.2.2.1: Incentive table description - tariff metadata
void CevcUsecase::read_incentive_table_description(IncentiveTableDescriptionDataType *data) const
{
    IncentiveTableDescriptionType desc{};

    // Tariff description
    TariffDescriptionDataType tariff_desc{};
    tariff_desc.tariffId = TARIFF_ID;
    tariff_desc.tariffWriteable = true; // Broker can update tariffs
    tariff_desc.updateRequired = true;  // Should be updated regularly
    tariff_desc.scopeType = ScopeTypeEnumType::simpleIncentiveTable;
    tariff_desc.label = "Energy Tariff";
    tariff_desc.description = "Energy pricing for charging optimization";
    desc.tariffDescription = tariff_desc;

    // Tier description - we support one dynamic cost tier
    IncentiveTableDescriptionTierType tier{};

    TierDescriptionDataType tier_desc{};
    tier_desc.tierId = TIER_ID;
    tier_desc.tierType = TierTypeEnumType::dynamicCost;
    tier_desc.label = "Dynamic Pricing";
    tier.tierDescription = tier_desc;

    // Boundary description - power-based boundaries
    TierBoundaryDescriptionDataType boundary_desc{};
    boundary_desc.boundaryId = 1;
    boundary_desc.boundaryType = TierBoundaryTypeEnumType::powerBoundary;
    boundary_desc.boundaryUnit = UnitOfMeasurementEnumType::W;
    boundary_desc.label = "Power Boundary";
    tier.boundaryDescription->push_back(boundary_desc);

    // Incentive description - absolute cost
    IncentiveDescriptionDataType incentive_desc{};
    incentive_desc.incentiveId = INCENTIVE_ID;
    incentive_desc.incentiveType = IncentiveTypeEnumType::absoluteCost;
    incentive_desc.currency = incentive_currency;
    incentive_desc.unit = UnitOfMeasurementEnumType::Wh;
    incentive_desc.label = "Energy Cost";
    tier.incentiveDescription->push_back(incentive_desc);

    desc.tier->push_back(tier);
    data->incentiveTableDescription->push_back(desc);
}

// CEVC Spec 3.2.1.2.2.1: Write incentive table description from Energy Broker
MessageReturn CevcUsecase::write_incentive_table_description(HeaderType &header, SpineOptional<IncentiveTableDescriptionDataType> data, JsonObject response)
{
    if (!data.has_value() || !data->incentiveTableDescription.has_value()) {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Invalid incentive table description");
        return {true, true, CmdClassifierType::result};
    }

    // Parse the incentive table description to extract currency if provided
    // Only fields present in the incoming data are updated (partial write support)
    for (const auto &desc : data->incentiveTableDescription.get()) {
        if (desc.tier.has_value()) {
            for (const auto &tier : desc.tier.get()) {
                if (tier.incentiveDescription.has_value()) {
                    for (const auto &incentive : tier.incentiveDescription.get()) {
                        if (incentive.currency.has_value()) {
                            incentive_currency = incentive.currency.get();
                            eebus.trace_fmtln("CEVC: Updated incentive currency");
                        }
                    }
                }
            }
        }
    }

    EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
    return {true, true, CmdClassifierType::result};
}

// CEVC Spec 3.2.1.2.2.2: Incentive table constraints
void CevcUsecase::read_incentive_table_constraints(IncentiveTableConstraintsDataType *data) const
{
    IncentiveTableConstraintsType constraints{};

    // Tariff reference
    TariffDataType tariff{};
    tariff.tariffId = TARIFF_ID;
    constraints.tariff = tariff;

    // Slot constraints - how many incentive slots we support
    TimeTableConstraintsDataType slot_constraints{};
    slot_constraints.slotCountMin = "1";
    slot_constraints.slotCountMax = std::to_string(MAX_INCENTIVE_SLOTS);
    slot_constraints.slotDurationMin = "PT1M";  // Minimum 1 minute
    slot_constraints.slotDurationMax = "PT24H"; // Maximum 24 hours
    constraints.incentiveSlotConstraints = slot_constraints;

    data->incentiveTableConstraints->push_back(constraints);
}

// CEVC Spec 3.2.1.2.2.3: Incentive table data - current tariff information
void CevcUsecase::read_incentive_table_data(IncentiveTableDataType *data) const
{
    if (!incentives_valid || received_incentives.empty()) {
        // Return empty data if no incentives available
        return;
    }

    IncentiveTableType table{};

    // Tariff reference
    TariffDataType tariff{};
    tariff.tariffId = TARIFF_ID;
    tariff.activeTierId->push_back(TIER_ID);
    table.tariff = tariff;

    // Convert our internal incentive slots to SPINE format
    for (const auto &slot : received_incentives) {
        IncentiveTableIncentiveSlotType spine_slot{};

        // Time interval
        TimeTableDataType time_interval{};
        time_interval.startTime->dateTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(slot.start_time).c_str();
        time_interval.endTime->dateTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(slot.end_time).c_str();
        spine_slot.timeInterval = time_interval;

        // Tiers within this slot
        for (const auto &tier : slot.tiers) {
            IncentiveTableTierType spine_tier{};

            // Tier data
            TierDataType tier_data{};
            tier_data.tierId = TIER_ID;
            spine_tier.tier = tier_data;

            // Boundary data
            TierBoundaryDataType boundary{};
            boundary.boundaryId = 1;
            boundary.lowerBoundaryValue->number = tier.lower_boundary_w;
            boundary.lowerBoundaryValue->scale = 0;
            boundary.upperBoundaryValue->number = tier.upper_boundary_w;
            boundary.upperBoundaryValue->scale = 0;
            spine_tier.boundary->push_back(boundary);

            // Incentive data (cost value)
            IncentiveDataType incentive{};
            incentive.incentiveId = INCENTIVE_ID;
            // Store as cents with scale -2 for proper decimal representation
            incentive.value->number = static_cast<int>(tier.incentive_value * 100);
            incentive.value->scale = -2;
            spine_tier.incentive->push_back(incentive);

            spine_slot.tier->push_back(spine_tier);
        }

        table.incentiveSlot->push_back(spine_slot);
    }

    data->incentiveTable->push_back(table);
}

// CEVC Spec 3.4.3: Receive incentive table from Energy Broker
MessageReturn CevcUsecase::write_incentive_table_data(HeaderType &header, SpineOptional<IncentiveTableDataType> data, JsonObject response)
{
    if (!data.has_value() || !data->incentiveTable.has_value()) {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Invalid incentive table data");
        return {true, true, CmdClassifierType::result};
    }

    // Process incoming incentive slots
    int updated_slots = 0;
    int new_slots = 0;

    for (const auto &table : data->incentiveTable.get()) {
        if (!table.incentiveSlot.has_value()) {
            continue;
        }

        for (const auto &slot : table.incentiveSlot.get()) {
            IncentiveSlotEntry entry{};

            // Parse time interval
            if (slot.timeInterval.has_value()) {
                if (slot.timeInterval->startTime.has_value() && slot.timeInterval->startTime->dateTime.has_value()) {
                    EEBUS_USECASE_HELPERS::iso_timestamp_to_unix(slot.timeInterval->startTime->dateTime->c_str(), &entry.start_time);
                }
                if (slot.timeInterval->endTime.has_value() && slot.timeInterval->endTime->dateTime.has_value()) {
                    EEBUS_USECASE_HELPERS::iso_timestamp_to_unix(slot.timeInterval->endTime->dateTime->c_str(), &entry.end_time);
                }
            }

            // Parse tiers
            if (slot.tier.has_value()) {
                for (const auto &tier : slot.tier.get()) {
                    IncentiveTier t{};

                    // Parse boundary
                    if (tier.boundary.has_value() && !tier.boundary->empty()) {
                        const auto &boundary = tier.boundary->at(0);
                        if (boundary.lowerBoundaryValue.has_value()) {
                            t.lower_boundary_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(boundary.lowerBoundaryValue.get());
                        }
                        if (boundary.upperBoundaryValue.has_value()) {
                            t.upper_boundary_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(boundary.upperBoundaryValue.get());
                        }
                    }

                    // Parse incentive value
                    if (tier.incentive.has_value() && !tier.incentive->empty()) {
                        const auto &incentive = tier.incentive->at(0);
                        if (incentive.value.has_value()) {
                            t.incentive_value = EEBUS_USECASE_HELPERS::scaled_numbertype_to_float(incentive.value.get());
                        }
                    }

                    entry.tiers.push_back(t);
                }
            }

            // Try to find existing slot by matching time interval (startTime + endTime)
            bool found = false;
            for (auto &existing : received_incentives) {
                // Match by time interval (start_time and end_time must match)
                if (existing.start_time == entry.start_time && existing.end_time == entry.end_time) {
                    // Update existing slot with new tier data
                    existing.tiers = entry.tiers;
                    found = true;
                    updated_slots++;
                    break;
                }
            }

            if (!found) {
                // Add new slot if not found and within limit
                if (received_incentives.size() < MAX_INCENTIVE_SLOTS) {
                    received_incentives.push_back(entry);
                    new_slots++;
                }
            }
        }
    }

    if (!received_incentives.empty()) {
        incentives_valid = true;
        eebus.trace_fmtln("CEVC: Updated incentive table (%d updated, %d new, total %d slots)", updated_slots, new_slots, received_incentives.size());
        notify_subscribers_incentives();
        update_api_state();
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
    } else {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "No valid incentive data found");
    }

    return {true, true, CmdClassifierType::result};
}

// =============================================================================
// Public Update Methods
// =============================================================================

void CevcUsecase::update_charging_demand(int min_energy_wh, int opt_energy_wh, int max_energy_wh, time_t target_time, const std::vector<ChargingDemandSlot> &slots)
{
    minimum_energy_wh = min_energy_wh;
    optimal_energy_wh = opt_energy_wh;
    maximum_energy_wh = max_energy_wh;
    target_departure_time = target_time;
    charging_demand_slots = slots;

    notify_subscribers_time_series();
    update_api_state();
}

void CevcUsecase::update_power_constraints(int max_power_w, const std::vector<PowerConstraintSlot> &slots)
{
    max_power_constraint_w = max_power_w;
    power_constraint_slots = slots;

    notify_subscribers_time_series();
    update_api_state();
}

int CevcUsecase::get_target_power_w(time_t query_time) const
{
    if (!charging_plan_valid || received_charging_plan.empty()) {
        return -1;
    }

    if (query_time == 0) {
        query_time = time(nullptr);
    }

    // Find the slot that contains the query time
    time_t slot_start = charging_plan_start_time;
    for (const auto &slot : received_charging_plan) {
        time_t slot_end = slot_start + slot.duration.as<int>();
        if (query_time >= slot_start && query_time < slot_end) {
            return slot.power_w;
        }
        slot_start = slot_end;
    }

    // Query time is after all slots - plan has expired
    return -1;
}

// =============================================================================
// Helper Methods
// =============================================================================

void CevcUsecase::notify_subscribers_time_series() const
{
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return;
    }
    // Build the time series data to send to subscribers
    TimeSeriesListDataType ts_data;
    read_time_series_list(&ts_data);
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::TimeSeries), ts_data, "timeSeriesListData");
}

void CevcUsecase::notify_subscribers_incentives() const
{
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return;
    }
    // Build the incentive data to send to subscribers
    IncentiveTableDataType it_data;
    read_incentive_table_data(&it_data);
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::IncentiveTable), it_data, "incentiveTableData");
}

void CevcUsecase::update_api_state() const
{
    auto api_entry = eebus.eebus_usecase_state.get("coordinated_ev_charging");
    api_entry->get("has_charging_plan")->updateBool(has_charging_plan());
    api_entry->get("charging_plan_start_time")->updateUint(charging_plan_start_time);
    api_entry->get("target_power_w")->updateInt(get_target_power_w());
    api_entry->get("has_incentives")->updateBool(has_incentives());
    api_entry->get("energy_broker_connected")->updateBool(energy_broker_connected);
    api_entry->get("energy_broker_heartbeat_ok")->updateBool(energy_broker_heartbeat_ok);
}

// =============================================================================
// Error State Handling (Scenarios 7-8)
// =============================================================================

void CevcUsecase::receive_heartbeat_timeout()
{
    // Scenario 8: Energy Broker error state
    // When we lose heartbeat from the Energy Broker, treat it as an error state
    // and invalidate all received data (charging plan, incentives)
    eebus.trace_fmtln("CEVC: Energy Broker heartbeat timeout - invalidating broker data");
    energy_broker_heartbeat_ok = false;
    invalidate_broker_data();
}

void CevcUsecase::inform_spineconnection_usecase_update(SpineConnection *conn)
{
    // Look for Energy Broker actors that support the CEVC use case
    // and register for heartbeat monitoring (Scenarios 5-6)
    auto peers = conn->get_address_of_feature(FeatureTypeEnumType::DeviceDiagnosis, RoleType::client, "coordinatedEvCharging", "EnergyBroker");

    for (FeatureAddressType &peer : peers) {
        eebus.trace_fmtln("CEVC: Found Energy Broker at %s, registering for heartbeat", EEBUS_USECASE_HELPERS::spine_address_to_string(peer).c_str());
        eebus.usecases->evse_heartbeat.initialize_heartbeat_on_feature(peer, Usecases::CEVC, true);
        energy_broker_connected = true;
        energy_broker_heartbeat_ok = true;
    }
}

void CevcUsecase::invalidate_broker_data()
{
    // Clear charging plan (Scenario 4 data)
    if (charging_plan_valid) {
        charging_plan_valid = false;
        received_charging_plan.clear();
        charging_plan_start_time = 0;
        eebus.trace_fmtln("CEVC: Charging plan invalidated due to Energy Broker error");
    }

    // Clear incentives (Scenario 3 data)
    if (incentives_valid) {
        incentives_valid = false;
        received_incentives.clear();
        eebus.trace_fmtln("CEVC: Incentives invalidated due to Energy Broker error");
    }

    // Notify subscribers that data has changed
    notify_subscribers_time_series();
    notify_subscribers_incentives();
    update_api_state();
}

#endif

// ==============================================================================
// OPEV - Overload Protection by EV Charging Current Curtailment Use Case
// Spec: EEBus_UC_TS_OverloadProtectionByEVChargingCurrentCurtailment_V1.0.1b.pdf
//
// STATUS: PARTIALLY IMPLEMENTED (~60% complete)
//
// Scenarios:
//   1 (3.4.1): EV reports limitations and writes current limit - COMPLETE
//   2 (3.4.2): EG sends heartbeat - Not complete
//   3 (3.4.3): EG sends error state - Not complete
//
// Known issues:
//   - Limit validation missing: No check if limits can be applied
//   - Missing update_api
//   - eebus-go reports that no current limits are received.
//   - Does not check if EG is okay.
// ==============================================================================
#ifdef EEBUS_ENABLE_OPEV_USECASE
MessageReturn OpevUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    AddressFeatureType feature_address = header.addressDestination->feature.get();
    switch (data->last_cmd) {
        // 3.2.1.2.1.1: Function "loadControlLimitDescriptionListData" - Current limit metadata
        case SpineDataTypeHandler::Function::loadControlLimitDescriptionListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::LoadControl)) {
                response["loadControlLimitDescriptionListData"] = EVEntity::get_load_control_limit_description_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        case SpineDataTypeHandler::Function::loadControlLimitConstraintsListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::LoadControl)) {
                response["loadControlLimitConstraintsListData"] = EVEntity::get_load_control_limit_constraints_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        // 3.2.1.2.1.2: Function "loadControlLimitListData" - Per-phase current limits
        case SpineDataTypeHandler::Function::loadControlLimitListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::LoadControl)) {
                if (header.cmdClassifier == CmdClassifierType::read) {
                    response["loadControlLimitListData"] = EVEntity::get_load_control_limit_list_data();
                    return {true, true, CmdClassifierType::reply};
                }
                if (header.cmdClassifier == CmdClassifierType::write) {
                    return write_load_control_limit_list_data(header, data->loadcontrollimitlistdatatype, response);
                }
            }
            break;
        // 3.2.1.2.2: Function "measurementListData" - Current measurements (via EVCEM)
        case SpineDataTypeHandler::Function::measurementListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::Measurement)) {
                response["measurementListData"] = EVEntity::get_measurement_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        // 3.2.1.2.3.1: Function "electricalConnectionParameterDescriptionListData"
        case SpineDataTypeHandler::Function::electricalConnectionParameterDescriptionListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::ElectricalConnection)) {
                response["electricalConnectionParameterDescriptionListData"] = EVEntity::get_electrical_connection_parameter_description_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        // 3.2.1.2.3.2: Function "electricalConnectionPermittedValueSetListData" - Permitted current values
        case SpineDataTypeHandler::Function::electricalConnectionPermittedValueSetListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::ElectricalConnection)) {
                response["electricalConnectionPermittedValueSetListData"] = EVEntity::get_electrical_connection_permitted_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        default:;
    }
    return {false};
}

UseCaseInformationDataType OpevUsecase::get_usecase_information()
{
    UseCaseInformationDataType opev_usecase;
    opev_usecase.actor = "EV";

    UseCaseSupportType opev_usecase_support;
    opev_usecase_support.useCaseName = "overloadProtectionByEvChargingCurrentCurtailment";
    opev_usecase_support.useCaseVersion = "1.0.1";
    // Scenario 1 (3.4.1): EV provides limitations and writes current limit
    opev_usecase_support.scenarioSupport->push_back(1);
    // Scenario 2 (3.4.2): EG sends heartbeat
    opev_usecase_support.scenarioSupport->push_back(2);
    // Scenario 3 (3.4.3): EG sends error state
    opev_usecase_support.scenarioSupport->push_back(3);
    opev_usecase_support.useCaseDocumentSubRevision = "release";
    opev_usecase.useCaseSupport->push_back(opev_usecase_support);

    FeatureAddressType opev_usecase_feature_address;
    opev_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    opev_usecase_feature_address.entity = entity_address;
    opev_usecase.address = opev_usecase_feature_address;
    return opev_usecase;
}

NodeManagementDetailedDiscoveryEntityInformationType OpevUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return entity;
    }
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EV;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "EV"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> OpevUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return features;
    }

    // The following functions are needed by the LoadControl Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType loadControlFeature{};
    loadControlFeature.description->featureAddress->entity = entity_address;
    loadControlFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::LoadControl);
    loadControlFeature.description->featureType = FeatureTypeEnumType::LoadControl;
    loadControlFeature.description->role = RoleType::server;

    // loadControlLimitDescriptionListData
    FunctionPropertyType loadControlDescriptionList{};
    loadControlDescriptionList.function = FunctionEnumType::loadControlLimitDescriptionListData;
    loadControlDescriptionList.possibleOperations->read = PossibleOperationsReadType{};
    loadControlFeature.description->supportedFunction->push_back(loadControlDescriptionList);

    // loadControlLimitListData
    FunctionPropertyType loadControlLimitListData{};
    loadControlLimitListData.function = FunctionEnumType::loadControlLimitListData;
    loadControlLimitListData.possibleOperations->read = PossibleOperationsReadType{};
    loadControlLimitListData.possibleOperations->write = PossibleOperationsWriteType{};

    loadControlFeature.description->supportedFunction->push_back(loadControlLimitListData);
    features.push_back(loadControlFeature);

    // The following functions are needed by the ElectricalConnection Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnectionFeature{};
    electricalConnectionFeature.description->featureAddress->entity = entity_address;
    electricalConnectionFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::ElectricalConnection);
    electricalConnectionFeature.description->featureType = FeatureTypeEnumType::ElectricalConnection;
    electricalConnectionFeature.description->role = RoleType::server;

    // electricalConnectionParameterDescriptionListData
    FunctionPropertyType electricalConnectionDescriptionList{};
    electricalConnectionDescriptionList.function = FunctionEnumType::electricalConnectionParameterDescriptionListData;
    electricalConnectionDescriptionList.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionDescriptionList);

    // electricalConnectionPermittedValueSetListData
    FunctionPropertyType electricalConnectionPermittedValueSetList{};
    electricalConnectionPermittedValueSetList.function = FunctionEnumType::electricalConnectionPermittedValueSetListData;
    electricalConnectionPermittedValueSetList.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionPermittedValueSetList);

    features.push_back(electricalConnectionFeature);

    return features;
}

// Spec 3.2.1.2.1.1: Load control limit descriptions for per-phase current limits
// Creates 3 limit IDs (id_x_1, id_x_2, id_x_3) for phases A, B, C
// Limit type: maxValueLimit, Category: obligation, Scope: overloadProtection
void OpevUsecase::get_load_control_limit_description_list_data(LoadControlLimitDescriptionListDataType *data)
{
    const std::array<std::pair<uint8_t, uint8_t>, 3> id_pairs{{
        {id_x_1, id_z_1}, // Phase A: limit id -> measurement id
        {id_x_2, id_z_2}, // Phase B
        {id_x_3, id_z_3}, // Phase C
    }};

    for (const auto &p : id_pairs) {
        LoadControlLimitDescriptionDataType loadControlLimitDescriptionData{};
        loadControlLimitDescriptionData.limitId = p.first;
        loadControlLimitDescriptionData.limitType = LoadControlLimitTypeEnumType::maxValueLimit;
        loadControlLimitDescriptionData.limitCategory = LoadControlCategoryEnumType::obligation;
        loadControlLimitDescriptionData.measurementId = p.second; // Link to current measurement
        loadControlLimitDescriptionData.unit = UnitOfMeasurementEnumType::A;
        loadControlLimitDescriptionData.scopeType = ScopeTypeEnumType::overloadProtection;
        data->loadControlLimitDescriptionData->push_back(loadControlLimitDescriptionData);
    }
}

// Spec 3.2.1.2.1.2: Current limit values per phase (read)
// Returns per-phase current limits in milliamps (scale=-3)
void OpevUsecase::get_load_control_limit_list_data(LoadControlLimitListDataType *data) const
{
    LoadControlLimitConstraintsListDataType loadControlLimitConstraintsListData{};

    constexpr std::array<uint8_t, 3> ids_used{{
        id_x_1, // Phase A
        id_x_2, // Phase B
        id_x_3, // Phase C
    }};
    for (size_t i = 0; i < ids_used.size(); ++i) {
        uint8_t p = ids_used[i];
        LoadControlLimitDataType loadControlLimitData{};
        loadControlLimitData.limitId = p;
        loadControlLimitData.isLimitChangeable = limit_changeable();
        loadControlLimitData.isLimitActive = limit_active;
        loadControlLimitData.value->number = limit_per_phase_milliamps[i];
        loadControlLimitData.value->scale = -3; // Milliamps: value * 10^-3 = A
        data->loadControlLimitData->push_back(loadControlLimitData);
    }
}

void OpevUsecase::get_electrical_connection_parameter_description_list_data(ElectricalConnectionParameterDescriptionListDataType *data)
{
    struct Entrydata {
        uint8_t param_id;
        uint8_t measure_id;
        ElectricalConnectionPhaseNameEnumType phase;
    };

    constexpr std::array<Entrydata, 3> entrydata{{
        {id_i_1, id_z_1, ElectricalConnectionPhaseNameEnumType::a},
        {id_i_2, id_z_2, ElectricalConnectionPhaseNameEnumType::b},
        {id_i_3, id_z_3, ElectricalConnectionPhaseNameEnumType::c},
    }};
    for (const auto &entry : entrydata) {
        ElectricalConnectionParameterDescriptionDataType parameter_description{};
        parameter_description.electricalConnectionId = id_j_1;
        parameter_description.parameterId = entry.param_id;
        parameter_description.measurementId = entry.measure_id;
        parameter_description.acMeasuredPhases = entry.phase;

        data->electricalConnectionParameterDescriptionData->push_back(parameter_description);
    }
}

void OpevUsecase::get_electrical_connection_permitted_list_data(ElectricalConnectionPermittedValueSetListDataType *data) const
{
    constexpr std::array<uint8_t, 3> ids_used{{
        id_i_1,
        id_i_2,
        id_i_3,
    }};
    for (const auto &id : ids_used) {
        ElectricalConnectionPermittedValueSetDataType permittedValueSetData{};
        permittedValueSetData.electricalConnectionId = id_j_1;
        permittedValueSetData.parameterId = id;
        ScaledNumberSetType permittedValueSet{};
        ScaledNumberRangeType range{};
        // TODO: We can actually dictate which values can be set here. For now we allow everything between min and max but maybe in future only allow full amp values between 0 and 32A?
        range.min->number = limit_milliamps_min;
        range.min->scale = -3;
        range.max->number = limit_milliamps_max;
        range.max->scale = -3;
        permittedValueSet.range->push_back(range);
        permittedValueSetData.permittedValueSet->push_back(permittedValueSet);
        data->electricalConnectionPermittedValueSetData->push_back(permittedValueSetData);
    }
}
void OpevUsecase::update_limits(int limit_phase_1_milliamps, int limit_phase_2_milliamps, int limit_phase_3_milliamps, bool active)
{
    // TODO: check if limits can be applied and apply them to the evse
    bool notify_subs = (limit_active != active);
    // Phase A
    if (limit_phase_1_milliamps >= 0) {
        if (limit_per_phase_milliamps[0] != limit_phase_1_milliamps) {
            notify_subs = true;
        }
        limit_per_phase_milliamps[0] = limit_phase_1_milliamps;
    }
    // Phase B
    if (limit_phase_2_milliamps >= 0) {
        if (limit_per_phase_milliamps[1] != limit_phase_2_milliamps) {
            notify_subs = true;
        }
        limit_per_phase_milliamps[1] = limit_phase_2_milliamps;
    }
    // Phase C
    if (limit_phase_3_milliamps >= 0) {
        if (limit_per_phase_milliamps[2] != limit_phase_3_milliamps) {
            notify_subs = true;
        }
        limit_per_phase_milliamps[2] = limit_phase_3_milliamps;
    }

    limit_active = active;

    if (notify_subs) {
        auto limit_list_data = EVEntity::get_load_control_limit_list_data();
        auto limit_permitted_data = EVEntity::get_electrical_connection_permitted_list_data();
        logger.printfln("OPEV: New limits received: L1: %d mA, L2: %d mA, L3: %d mA, active: %s", limit_per_phase_milliamps[0], limit_per_phase_milliamps[1], limit_per_phase_milliamps[2], limit_active ? "true" : "false");
        eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::LoadControl), limit_list_data, "loadControlLimitListData");
        eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::ElectricalConnection), limit_permitted_data, "electricalConnectionPermittedValueSetListData");
    }
}
bool OpevUsecase::limit_changeable() const
{
#ifdef EEBUS_ENABLE_LPC_USECASE
    if (eebus.usecases->limitation_of_power_consumption.limit_is_active())
        return false;
#endif
    return limit_changeable_allowed;
}
MessageReturn OpevUsecase::write_load_control_limit_list_data(HeaderType &header, SpineOptional<LoadControlLimitListDataType> data, JsonObject response)
{
    if (!data.has_value()) {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "No load control limit data provided");
        return {true, true, CmdClassifierType::result};
    }
    int limit_phase_a = -1;
    int limit_phase_b = -1;
    int limit_phase_c = -1;
    bool limit_enabled = false;

    for (const auto &limit_data : data->loadControlLimitData.get()) {
        int limit_value_milliamps = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(limit_data.value.get()) * 1000;

        switch (limit_data.limitId.get()) {
            case id_x_1:
                limit_phase_a = limit_value_milliamps;
                limit_enabled = limit_active || limit_data.isLimitActive.get();
                break;
            case id_x_2:
                limit_phase_b = limit_value_milliamps;
                limit_enabled = limit_active || limit_data.isLimitActive.get();
                break;
            case id_x_3:
                limit_phase_c = limit_value_milliamps;
                limit_enabled = limit_active || limit_data.isLimitActive.get();
                break;
            default:
                break;
        }
    }
    // Check if received limits are within min/max defined in electricalConnectionPermittedValueSetListData
    auto check_limit = [this](int limit_milliamps) -> bool {
        return (limit_milliamps < 0) || // -1 means "not set", always valid
               (limit_milliamps >= limit_milliamps_min && limit_milliamps <= limit_milliamps_max);
    };

    if (!check_limit(limit_phase_a) || !check_limit(limit_phase_b) || !check_limit(limit_phase_c)) {
        logger.printfln("OPEV: Received limit out of range (min: %d mA, max: %d mA). L1: %d mA, L2: %d mA, L3: %d mA", limit_milliamps_min, limit_milliamps_max, limit_phase_a, limit_phase_b, limit_phase_c);
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Limit value outside permitted range");
        return {true, true, CmdClassifierType::result};
    }
    update_limits(limit_phase_a, limit_phase_b, limit_phase_c, limit_enabled);
    return {true, false, CmdClassifierType::reply};
}
#endif

// ==============================================================================
// MGCP - Monitoring of Grid Connection Point Use Case
// Spec: EEBus_UC_TS_MonitoringOfGridConnectionPoint_V1.0.0.pdf
//
// STATUS: IMPLEMENTED
//
// Scenarios:
//   1 (3.4.1): Monitor PV feed-in power limitation factor (Optional) - DeviceConfiguration
//   2 (3.4.2): Monitor momentary power (Mandatory) - Measurement
//   3 (3.4.3): Monitor total feed-in energy (Mandatory) - Measurement
//   4 (3.4.4): Monitor total consumed energy (Mandatory) - Measurement
//   5 (3.4.5): Monitor momentary current (Recommended) - Measurement
//   6 (3.4.6): Monitor voltage (Optional) - Measurement
//   7 (3.4.7): Monitor frequency (Optional) - Measurement
//
// Entity Type: GridConnectionPointOfPremises
// Actor: GridConnectionPoint
// Sign Convention: Load/passive - positive = consumption, negative = production/feed-in
// ==============================================================================
#ifdef EEBUS_ENABLE_MGCP_USECASE

MgcpUsecase::MgcpUsecase()
{
    // Initialize with default values
    task_scheduler.scheduleOnce(
        [this]() {
            update_api();
        },
        1_s); // Schedule init delayed to allow other entities to initialize first
}

MessageReturn MgcpUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    if (header.cmdClassifier == CmdClassifierType::read) {
        switch (get_feature_by_address(header.addressDestination->feature.get())) {
            case FeatureTypeEnumType::DeviceConfiguration:
                switch (data->last_cmd) {
                    case SpineDataTypeHandler::Function::deviceConfigurationKeyValueDescriptionListData: {
                        response["deviceConfigurationKeyValueDescriptionListData"] = EVSEEntity::get_device_configuration_list_data();
                        ;
                        return {true, true, CmdClassifierType::reply};
                    }
                    case SpineDataTypeHandler::Function::deviceConfigurationKeyValueListData: {
                        response["deviceConfigurationKeyValueListData"] = EVSEEntity::get_device_configuration_value_list_data();
                        return {true, true, CmdClassifierType::reply};
                    }
                    default:;
                }
                break;
            case FeatureTypeEnumType::ElectricalConnection:
                switch (data->last_cmd) {
                    case SpineDataTypeHandler::Function::electricalConnectionDescriptionListData: {
                        response["electricalConnectionDescriptionListData"] = EVSEEntity::get_electrical_connection_description_list_data();
                        return {true, true, CmdClassifierType::reply};
                    }
                    case SpineDataTypeHandler::Function::electricalConnectionParameterDescriptionListData: {
                        response["electricalConnectionParameterDescriptionListData"] = EVSEEntity::get_electrical_connection_parameter_description_list_data();
                        return {true, true, CmdClassifierType::reply};
                    }
                    default:;
                }
                break;
            case FeatureTypeEnumType::Measurement:
                switch (data->last_cmd) {
                    case SpineDataTypeHandler::Function::measurementDescriptionListData: {
                        response["measurementDescriptionListData"] = EVSEEntity::get_measurement_description_list_data();
                        return {true, true, CmdClassifierType::reply};
                    }
                    case SpineDataTypeHandler::Function::measurementConstraintsListData: {
                        response["measurementConstraintsListData"] = EVSEEntity::get_measurement_constraints_list_data();
                        return {true, true, CmdClassifierType::reply};
                    }
                    case SpineDataTypeHandler::Function::measurementListData: {
                        response["measurementListData"] = EVSEEntity::get_measurement_list_data();
                        return {true, true, CmdClassifierType::reply};
                    }
                    default:;
                }
                break;
            default:;
        }
    }
    return {false};
}

UseCaseInformationDataType MgcpUsecase::get_usecase_information()
{
    UseCaseInformationDataType mgcp_usecase;
    mgcp_usecase.actor = "GridConnectionPoint";

    UseCaseSupportType mgcp_usecase_support;
    mgcp_usecase_support.useCaseName = "monitoringOfGridConnectionPoint";
    mgcp_usecase_support.useCaseVersion = "1.0.0";
    // Scenario 1: Monitor PV feed-in power limitation factor (Optional)
    mgcp_usecase_support.scenarioSupport->push_back(1);
    // Scenario 2: Monitor momentary power (Mandatory)
    mgcp_usecase_support.scenarioSupport->push_back(2);
    // Scenario 3: Monitor total feed-in energy (Mandatory)
    mgcp_usecase_support.scenarioSupport->push_back(3);
    // Scenario 4: Monitor total consumed energy (Mandatory)
    mgcp_usecase_support.scenarioSupport->push_back(4);
    // Scenario 5: Monitor momentary current (Recommended)
    mgcp_usecase_support.scenarioSupport->push_back(5);
    // Scenario 6: Monitor voltage (Optional)
    mgcp_usecase_support.scenarioSupport->push_back(6);
    // Scenario 7: Monitor frequency (Optional)
    mgcp_usecase_support.scenarioSupport->push_back(7);
    mgcp_usecase_support.useCaseDocumentSubRevision = "release";
    mgcp_usecase.useCaseSupport->push_back(mgcp_usecase_support);

    FeatureAddressType mgcp_usecase_feature_address{};
    mgcp_usecase_feature_address.entity = entity_address;
    mgcp_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    mgcp_usecase.address = mgcp_usecase_feature_address;

    return mgcp_usecase;
}

NodeManagementDetailedDiscoveryEntityInformationType MgcpUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::CEM;
    entity.description->label = "Grid Connection Point";

    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> MgcpUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    // DeviceConfiguration Feature (Scenario 1: PV curtailment limit factor)
    NodeManagementDetailedDiscoveryFeatureInformationType deviceConfigFeature{};
    deviceConfigFeature.description->featureAddress->entity = entity_address;
    deviceConfigFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration);
    deviceConfigFeature.description->featureType = FeatureTypeEnumType::DeviceConfiguration;
    deviceConfigFeature.description->role = RoleType::server;

    FunctionPropertyType deviceConfigDescListData{};
    deviceConfigDescListData.function = FunctionEnumType::deviceConfigurationKeyValueDescriptionListData;
    deviceConfigDescListData.possibleOperations->read = PossibleOperationsReadType{};
    deviceConfigFeature.description->supportedFunction->push_back(deviceConfigDescListData);

    FunctionPropertyType deviceConfigValueListData{};
    deviceConfigValueListData.function = FunctionEnumType::deviceConfigurationKeyValueListData;
    deviceConfigValueListData.possibleOperations->read = PossibleOperationsReadType{};
    deviceConfigFeature.description->supportedFunction->push_back(deviceConfigValueListData);
    features.push_back(deviceConfigFeature);

    // ElectricalConnection Feature
    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnectionFeature{};
    electricalConnectionFeature.description->featureAddress->entity = entity_address;
    electricalConnectionFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::ElectricalConnection);
    electricalConnectionFeature.description->featureType = FeatureTypeEnumType::ElectricalConnection;
    electricalConnectionFeature.description->role = RoleType::server;

    FunctionPropertyType electricalConnectionDescriptionListData{};
    electricalConnectionDescriptionListData.function = FunctionEnumType::electricalConnectionDescriptionListData;
    electricalConnectionDescriptionListData.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionDescriptionListData);

    FunctionPropertyType electricalConnectionParameterDescriptionListData{};
    electricalConnectionParameterDescriptionListData.function = FunctionEnumType::electricalConnectionParameterDescriptionListData;
    electricalConnectionParameterDescriptionListData.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionParameterDescriptionListData);
    features.push_back(electricalConnectionFeature);

    // Measurement Feature (Scenarios 2-7)
    NodeManagementDetailedDiscoveryFeatureInformationType measurementFeature{};
    measurementFeature.description->featureAddress->entity = entity_address;
    measurementFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::Measurement);
    measurementFeature.description->featureType = FeatureTypeEnumType::Measurement;
    measurementFeature.description->role = RoleType::server;

    FunctionPropertyType measurementDescriptionListData{};
    measurementDescriptionListData.function = FunctionEnumType::measurementDescriptionListData;
    measurementDescriptionListData.possibleOperations->read = PossibleOperationsReadType{};
    measurementFeature.description->supportedFunction->push_back(measurementDescriptionListData);

    FunctionPropertyType measurementConstraintsListData{};
    measurementConstraintsListData.function = FunctionEnumType::measurementConstraintsListData;
    measurementConstraintsListData.possibleOperations->read = PossibleOperationsReadType{};
    measurementFeature.description->supportedFunction->push_back(measurementConstraintsListData);

    FunctionPropertyType measurementListData{};
    measurementListData.function = FunctionEnumType::measurementListData;
    measurementListData.possibleOperations->read = PossibleOperationsReadType{};
    measurementFeature.description->supportedFunction->push_back(measurementListData);
    features.push_back(measurementFeature);

    return features;
}

// ==============================================================================
// Data generator methods
// ==============================================================================

void MgcpUsecase::get_device_configuration_description_list_data(DeviceConfigurationKeyValueDescriptionListDataType *data) const
{
    // Scenario 1: PV curtailment limit factor key
    DeviceConfigurationKeyValueDescriptionDataType pv_curtailment_desc{};
    pv_curtailment_desc.keyId = id_k_1;
    pv_curtailment_desc.keyName = DeviceConfigurationKeyNameEnumType::pvCurtailmentLimitFactor;
    pv_curtailment_desc.valueType = DeviceConfigurationKeyValueTypeType::scaledNumber;
    pv_curtailment_desc.unit = UnitOfMeasurementEnumType::pct;
    data->deviceConfigurationKeyValueDescriptionData->push_back(pv_curtailment_desc);
}

void MgcpUsecase::get_device_configuration_value_list_data(DeviceConfigurationKeyValueListDataType *data) const
{
    // Scenario 1: PV curtailment limit factor value
    DeviceConfigurationKeyValueDataType pv_curtailment_value{};
    pv_curtailment_value.keyId = id_k_1;
    pv_curtailment_value.value->scaledNumber->number = static_cast<int32_t>(pv_curtailment_limit_factor_percent * 10); // scale -1
    pv_curtailment_value.value->scaledNumber->scale = -1;
    pv_curtailment_value.isValueChangeable = false; // Read-only for monitoring
    data->deviceConfigurationKeyValueData->push_back(pv_curtailment_value);
}

void MgcpUsecase::get_electrical_connection_description_list_data(ElectricalConnectionDescriptionListDataType *data) const
{
    ElectricalConnectionDescriptionDataType ec_description{};
    ec_description.electricalConnectionId = id_ec_1;
    ec_description.powerSupplyType = ElectricalConnectionVoltageTypeEnumType::ac;
    // Load/passive sign convention: positive = consumption, negative = feed-in
    ec_description.positiveEnergyDirection = EnergyDirectionEnumType::consume;
    data->electricalConnectionDescriptionData->push_back(ec_description);
}

void MgcpUsecase::get_electrical_connection_parameter_description_list_data(ElectricalConnectionParameterDescriptionListDataType *data) const
{
    // Total power parameter (Scenario 2)
    {
        ElectricalConnectionParameterDescriptionDataType param{};
        param.electricalConnectionId = id_ec_1;
        param.parameterId = id_p_1;
        param.measurementId = id_m_1;
        param.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
        param.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::abc;
        param.acMeasuredInReferenceTo = ElectricalConnectionPhaseNameEnumType::neutral;
        param.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::real;
        param.acMeasurementVariant = ElectricalConnectionMeasurandVariantEnumType::rms;
        data->electricalConnectionParameterDescriptionData->push_back(param);
    }

    // Energy feed-in parameter (Scenario 3)
    {
        ElectricalConnectionParameterDescriptionDataType param{};
        param.electricalConnectionId = id_ec_1;
        param.parameterId = id_p_2;
        param.measurementId = id_m_2;
        param.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
        param.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::real;
        data->electricalConnectionParameterDescriptionData->push_back(param);
    }

    // Energy consumed parameter (Scenario 4)
    {
        ElectricalConnectionParameterDescriptionDataType param{};
        param.electricalConnectionId = id_ec_1;
        param.parameterId = id_p_3;
        param.measurementId = id_m_3;
        param.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
        param.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::real;
        data->electricalConnectionParameterDescriptionData->push_back(param);
    }

    // Per-phase current parameters (Scenario 5)
    constexpr std::array<std::pair<std::pair<uint8_t, uint8_t>, ElectricalConnectionPhaseNameEnumType>, 3> current_params{{
        {{id_p_4_1, id_m_4_1}, ElectricalConnectionPhaseNameEnumType::a},
        {{id_p_4_2, id_m_4_2}, ElectricalConnectionPhaseNameEnumType::b},
        {{id_p_4_3, id_m_4_3}, ElectricalConnectionPhaseNameEnumType::c},
    }};
    for (const auto &cp : current_params) {
        ElectricalConnectionParameterDescriptionDataType param{};
        param.electricalConnectionId = id_ec_1;
        param.parameterId = cp.first.first;
        param.measurementId = cp.first.second;
        param.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
        param.acMeasuredPhases = cp.second;
        param.acMeasuredInReferenceTo = ElectricalConnectionPhaseNameEnumType::neutral;
        param.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::real;
        param.acMeasurementVariant = ElectricalConnectionMeasurandVariantEnumType::rms;
        data->electricalConnectionParameterDescriptionData->push_back(param);
    }

    // Per-phase voltage parameters (Scenario 6) - phase to neutral only
    constexpr std::array<std::pair<std::pair<uint8_t, uint8_t>, ElectricalConnectionPhaseNameEnumType>, 3> voltage_params{{
        {{id_p_5_1, id_m_5_1}, ElectricalConnectionPhaseNameEnumType::a},
        {{id_p_5_2, id_m_5_2}, ElectricalConnectionPhaseNameEnumType::b},
        {{id_p_5_3, id_m_5_3}, ElectricalConnectionPhaseNameEnumType::c},
    }};
    for (const auto &vp : voltage_params) {
        ElectricalConnectionParameterDescriptionDataType param{};
        param.electricalConnectionId = id_ec_1;
        param.parameterId = vp.first.first;
        param.measurementId = vp.first.second;
        param.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
        param.acMeasuredPhases = vp.second;
        param.acMeasuredInReferenceTo = ElectricalConnectionPhaseNameEnumType::neutral;
        param.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::apparent;
        param.acMeasurementVariant = ElectricalConnectionMeasurandVariantEnumType::rms;
        data->electricalConnectionParameterDescriptionData->push_back(param);
    }

    // Frequency parameter (Scenario 7)
    {
        ElectricalConnectionParameterDescriptionDataType param{};
        param.electricalConnectionId = id_ec_1;
        param.parameterId = id_p_6;
        param.measurementId = id_m_6;
        param.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
        data->electricalConnectionParameterDescriptionData->push_back(param);
    }
}

void MgcpUsecase::get_measurement_description_list_data(MeasurementDescriptionListDataType *data) const
{
    struct MeasurementDescriptionEntry {
        uint8_t measurement_id;
        MeasurementTypeEnumType measurement_type;
        UnitOfMeasurementEnumType unit;
        ScopeTypeEnumType scope;
    };
    constexpr std::array<MeasurementDescriptionEntry, 10> measurement_descriptions{{
        // Scenario 2: Total power
        {id_m_1, MeasurementTypeEnumType::power, UnitOfMeasurementEnumType::W, ScopeTypeEnumType::acPowerTotal},
        // Scenario 3: Energy feed-in (energy produced and fed into grid)
        {id_m_2, MeasurementTypeEnumType::energy, UnitOfMeasurementEnumType::Wh, ScopeTypeEnumType::gridFeedIn},
        // Scenario 4: Energy consumed from grid
        {id_m_3, MeasurementTypeEnumType::energy, UnitOfMeasurementEnumType::Wh, ScopeTypeEnumType::gridConsumption},
        // Scenario 5: Per-phase current
        {id_m_4_1, MeasurementTypeEnumType::current, UnitOfMeasurementEnumType::A, ScopeTypeEnumType::acCurrent},
        {id_m_4_2, MeasurementTypeEnumType::current, UnitOfMeasurementEnumType::A, ScopeTypeEnumType::acCurrent},
        {id_m_4_3, MeasurementTypeEnumType::current, UnitOfMeasurementEnumType::A, ScopeTypeEnumType::acCurrent},
        // Scenario 6: Per-phase voltage
        {id_m_5_1, MeasurementTypeEnumType::voltage, UnitOfMeasurementEnumType::V, ScopeTypeEnumType::acVoltage},
        {id_m_5_2, MeasurementTypeEnumType::voltage, UnitOfMeasurementEnumType::V, ScopeTypeEnumType::acVoltage},
        {id_m_5_3, MeasurementTypeEnumType::voltage, UnitOfMeasurementEnumType::V, ScopeTypeEnumType::acVoltage},
        // Scenario 7: Frequency
        {id_m_6, MeasurementTypeEnumType::frequency, UnitOfMeasurementEnumType::Hz, ScopeTypeEnumType::acFrequency},
    }};

    for (const auto &entry : measurement_descriptions) {
        MeasurementDescriptionDataType measurement_description{};
        measurement_description.measurementId = entry.measurement_id;
        measurement_description.measurementType = entry.measurement_type;
        measurement_description.commodityType = CommodityTypeEnumType::electricity;
        measurement_description.unit = entry.unit;
        measurement_description.scopeType = entry.scope;
        data->measurementDescriptionData->push_back(measurement_description);
    }
}

void MgcpUsecase::get_measurement_constraints_list_data(MeasurementConstraintsListDataType *data) const
{
    struct ConstraintEntry {
        uint8_t measurement_id;
        int32_t min;
        int32_t max;
        int32_t stepsize;
        int8_t scale;
    };

    const std::array<ConstraintEntry, 10> entries{{
        // Power measurement (W) - scale 0, can be negative (production)
        {id_m_1, power_limit_min_w, power_limit_max_w, 1, 0},
        // Energy measurements (Wh) - scale 0, always positive
        {id_m_2, 0, static_cast<int32_t>(energy_limit_max_wh), 1, 0},
        {id_m_3, 0, static_cast<int32_t>(energy_limit_max_wh), 1, 0},
        // Current measurements (A) - scale -3 (mA), can be negative
        {id_m_4_1, current_limit_min_ma, current_limit_max_ma, 1, -3},
        {id_m_4_2, current_limit_min_ma, current_limit_max_ma, 1, -3},
        {id_m_4_3, current_limit_min_ma, current_limit_max_ma, 1, -3},
        // Voltage measurements (V) - scale 0
        {id_m_5_1, voltage_limit_min_v, voltage_limit_max_v, 1, 0},
        {id_m_5_2, voltage_limit_min_v, voltage_limit_max_v, 1, 0},
        {id_m_5_3, voltage_limit_min_v, voltage_limit_max_v, 1, 0},
        // Frequency (Hz) - scale -3 (mHz)
        {id_m_6, frequency_limit_min_mhz, frequency_limit_max_mhz, 1, -3},
    }};

    for (const auto &entry : entries) {
        MeasurementConstraintsDataType constraint{};
        constraint.measurementId = entry.measurement_id;
        constraint.valueRangeMin->number = entry.min;
        constraint.valueRangeMin->scale = entry.scale;
        constraint.valueRangeMax->number = entry.max;
        constraint.valueRangeMax->scale = entry.scale;
        constraint.valueStepSize->number = entry.stepsize;
        constraint.valueStepSize->scale = entry.scale;
        data->measurementConstraintsData->push_back(constraint);
    }
}

void MgcpUsecase::get_measurement_list_data(MeasurementListDataType *data) const
{
    // Scenario 2: Total power (negative = feed-in)
    MeasurementDataType m_power{};
    m_power.measurementId = id_m_1;
    m_power.valueType = MeasurementValueTypeEnumType::value;
    m_power.value->number = total_power_w;
    m_power.value->scale = 0;
    m_power.valueSource = MeasurementValueSourceEnumType::measuredValue;
    data->measurementData->push_back(m_power);

    // Scenario 3: Energy feed-in
    MeasurementDataType m_feed_in{};
    m_feed_in.measurementId = id_m_2;
    m_feed_in.valueType = MeasurementValueTypeEnumType::value;
    m_feed_in.value->number = static_cast<int32_t>(energy_feed_in_wh);
    m_feed_in.value->scale = 0;
    m_feed_in.valueSource = MeasurementValueSourceEnumType::measuredValue;
    data->measurementData->push_back(m_feed_in);

    // Scenario 4: Energy consumed
    MeasurementDataType m_consumed{};
    m_consumed.measurementId = id_m_3;
    m_consumed.valueType = MeasurementValueTypeEnumType::value;
    m_consumed.value->number = static_cast<int32_t>(energy_consumed_wh);
    m_consumed.value->scale = 0;
    m_consumed.valueSource = MeasurementValueSourceEnumType::measuredValue;
    data->measurementData->push_back(m_consumed);

    // Scenario 5: Per-phase current (mA, scale -3)
    for (int i = 0; i < 3; i++) {
        MeasurementDataType m{};
        m.measurementId = id_m_4_1 + i;
        m.valueType = MeasurementValueTypeEnumType::value;
        m.value->number = current_phase_ma[i];
        m.value->scale = -3;
        m.valueSource = MeasurementValueSourceEnumType::measuredValue;
        data->measurementData->push_back(m);
    }

    // Scenario 6: Per-phase voltage (V, scale 0)
    for (int i = 0; i < 3; i++) {
        MeasurementDataType m{};
        m.measurementId = id_m_5_1 + i;
        m.valueType = MeasurementValueTypeEnumType::value;
        m.value->number = voltage_phase_v[i];
        m.value->scale = 0;
        m.valueSource = MeasurementValueSourceEnumType::measuredValue;
        data->measurementData->push_back(m);
    }

    // Scenario 7: Frequency (mHz, scale -3)
    MeasurementDataType m_freq{};
    m_freq.measurementId = id_m_6;
    m_freq.valueType = MeasurementValueTypeEnumType::value;
    m_freq.value->number = frequency_mhz;
    m_freq.value->scale = -3;
    m_freq.valueSource = MeasurementValueSourceEnumType::measuredValue;
    data->measurementData->push_back(m_freq);
}

// ==============================================================================
// Update methods for each scenario
// ==============================================================================

void MgcpUsecase::update_pv_curtailment_limit_factor(float limit_factor_percent)
{
    pv_curtailment_limit_factor_percent = limit_factor_percent;

    // Inform subscribers of device configuration changes
    auto config_data = EVSEEntity::get_device_configuration_value_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration), config_data, "deviceConfigurationKeyValueListData");

    update_api();
}

void MgcpUsecase::update_power(int total_power)
{
    total_power_w = total_power;

    // Inform subscribers of measurement changes
    auto measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MgcpUsecase::update_energy_feed_in(uint32_t energy_wh)
{
    energy_feed_in_wh = energy_wh;

    // Inform subscribers of measurement changes
    auto measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MgcpUsecase::update_energy_consumed(uint32_t energy_wh)
{
    energy_consumed_wh = energy_wh;

    // Inform subscribers of measurement changes
    auto measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MgcpUsecase::update_current(int current_phase_1_ma, int current_phase_2_ma, int current_phase_3_ma)
{
    current_phase_ma[0] = current_phase_1_ma;
    current_phase_ma[1] = current_phase_2_ma;
    current_phase_ma[2] = current_phase_3_ma;

    // Inform subscribers of measurement changes
    auto measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MgcpUsecase::update_voltage(int voltage_phase_1_v, int voltage_phase_2_v, int voltage_phase_3_v)
{
    voltage_phase_v[0] = voltage_phase_1_v;
    voltage_phase_v[1] = voltage_phase_2_v;
    voltage_phase_v[2] = voltage_phase_3_v;

    // Inform subscribers of measurement changes
    auto measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MgcpUsecase::update_frequency(int freq_mhz)
{
    frequency_mhz = freq_mhz;

    // Inform subscribers of measurement changes
    auto measurement_data = EVSEEntity::get_measurement_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_data, "measurementListData");

    update_api();
}

void MgcpUsecase::update_constraints(int power_min, int power_max, int current_min_ma, int current_max_ma, uint32_t energy_max_wh, int voltage_min_v, int voltage_max_v, int frequency_min_mhz, int frequency_max_mhz)
{
    power_limit_min_w = power_min;
    power_limit_max_w = power_max;
    current_limit_min_ma = current_min_ma;
    current_limit_max_ma = current_max_ma;
    energy_limit_max_wh = energy_max_wh;
    voltage_limit_min_v = voltage_min_v;
    voltage_limit_max_v = voltage_max_v;
    frequency_limit_min_mhz = frequency_min_mhz;
    frequency_limit_max_mhz = frequency_max_mhz;

    // Inform subscribers of constraint changes
    auto constraints_data = EVSEEntity::get_measurement_constraints_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), constraints_data, "measurementConstraintsListData");

    update_api();
}

void MgcpUsecase::update_api() const
{
    auto api_entry = eebus.eebus_usecase_state.get("monitoring_of_grid_connection_point");
    api_entry->get("pv_curtailment_limit_factor_percent")->updateFloat(pv_curtailment_limit_factor_percent);
    api_entry->get("total_power_w")->updateInt(total_power_w);
    api_entry->get("energy_feed_in_wh")->updateUint(energy_feed_in_wh);
    api_entry->get("energy_consumed_wh")->updateUint(energy_consumed_wh);
    api_entry->get("current_phase_1_ma")->updateInt(current_phase_ma[0]);
    api_entry->get("current_phase_2_ma")->updateInt(current_phase_ma[1]);
    api_entry->get("current_phase_3_ma")->updateInt(current_phase_ma[2]);
    api_entry->get("voltage_phase_1_v")->updateInt(voltage_phase_v[0]);
    api_entry->get("voltage_phase_2_v")->updateInt(voltage_phase_v[1]);
    api_entry->get("voltage_phase_3_v")->updateInt(voltage_phase_v[2]);
    api_entry->get("frequency_mhz")->updateInt(frequency_mhz);
}

#endif // EEBUS_ENABLE_MGCP_USECASE

EEBusUseCases::EEBusUseCases()
{
    // Entity Addresses should be consistent so all actors are under the same entity
    usecase_list.push_back(&node_management);
    node_management.set_usecaseManager(this);
    node_management.set_entity_address({0});
    std::vector<Usecases> supported_usecases{};

    // EVSE Actors
    usecase_list.push_back(&evse_heartbeat);
    evse_heartbeat.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(evse_heartbeat.get_usecase_type());
#ifdef EEBUS_ENABLE_EVCS_USECASE
    usecase_list.push_back(&charging_summary);
    charging_summary.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(charging_summary.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_LPC_USECASE
    usecase_list.push_back(&limitation_of_power_consumption);
    limitation_of_power_consumption.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(limitation_of_power_consumption.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_MPC_USECASE
    usecase_list.push_back(&monitoring_of_power_consumption);
    monitoring_of_power_consumption.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(monitoring_of_power_consumption.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    usecase_list.push_back(&limitation_of_power_production);
    limitation_of_power_production.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(limitation_of_power_production.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_EVSECC_USECASE
    usecase_list.push_back(&evse_commissioning_and_configuration);
    evse_commissioning_and_configuration.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(evse_commissioning_and_configuration.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    usecase_list.push_back(&monitoring_of_grid_connection_point);
    monitoring_of_grid_connection_point.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(monitoring_of_grid_connection_point.get_usecase_type());
#endif

    // EV actors
    usecase_list.push_back(&ev_heartbeat);
    ev_heartbeat.set_entity_address(EVEntity::entity_address);
    supported_usecases.push_back(ev_heartbeat.get_usecase_type());
#ifdef EEBUS_ENABLE_EVCC_USECASE
    usecase_list.push_back(&ev_commissioning_and_configuration);
    ev_commissioning_and_configuration.set_entity_address(EVEntity::entity_address); // EVCC entity is "under" the ChargingSummary entity and therefore the first value
    supported_usecases.push_back(ev_commissioning_and_configuration.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_CEVC_USECASE
    usecase_list.push_back(&coordinate_ev_charging);
    coordinate_ev_charging.set_entity_address({1, 1});
    supported_usecases.push_back(coordinate_ev_charging.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    usecase_list.push_back(&ev_charging_electricity_measurement);
    ev_charging_electricity_measurement.set_entity_address(EVEntity::entity_address);
    supported_usecases.push_back(ev_charging_electricity_measurement.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_OPEV_USECASE
    usecase_list.push_back(&overload_protection_by_ev_charging_current_curtailment);
    overload_protection_by_ev_charging_current_curtailment.set_entity_address(EVEntity::entity_address);
    supported_usecases.push_back(overload_protection_by_ev_charging_current_curtailment.get_usecase_type());
#endif

    // Map out the features used and assign feature addresses
    std::map<std::pair<std::vector<int>, int>, FeatureTypeEnumType> features;
    constexpr int feature_step_size = 2;
    int feature_index = feature_step_size;
    for (EebusUsecase *uc : usecase_list) {
        auto entity = uc->get_entity_address();
        for (FeatureTypeEnumType feature_type_needed : uc->get_supported_features()) {
            bool assigned = false;
            for (auto &existing_feature_pair : features) {
                if (existing_feature_pair.second == feature_type_needed && existing_feature_pair.first.first == entity) {
                    uc->set_feature_address(existing_feature_pair.first.second, feature_type_needed);
                    assigned = true;
                    break;
                }
            }
            if (!assigned) {
                if (feature_type_needed == FeatureTypeEnumType::NodeManagement) {
                    features[{entity, 0}] = feature_type_needed;
                    uc->set_feature_address(0, feature_type_needed);
                } else {
                    features[{entity, feature_index}] = feature_type_needed;
                    uc->set_feature_address(feature_index, feature_type_needed);
                    feature_index += feature_step_size;
                }
            }
        }
    }
    task_scheduler.scheduleOnce(
        [this, supported_usecases]() {
            // String usecase_names = "";
            for (const Usecases uc : supported_usecases) {
                //  usecase_names += String(get_usecases_name(uc)) + ",";
                auto entry = eebus.eebus_usecase_state.get("usecases_supported")->add();
                entry->updateEnum(uc);
            }
        },
        1_s);
    // eebus.eebus_usecase_state.get("usecases_supported")->updateString(usecase_names);
    initialized = true; // set to true, otherwise subscriptions will not work
}

void EEBusUseCases::process_spine_message(HeaderType &header, SpineDataTypeHandler *data, SpineConnection *connection)
{
    // Prepare the handle messages
    eebus_commands_received++;
    eebus.eebus_usecase_state.get("commands_received")->updateUint(eebus_commands_received);
    BasicJsonDocument<ArduinoJsonPsramAllocator> response_doc{SPINE_CONNECTION_MAX_JSON_SIZE};
    JsonObject responseObj = response_doc.to<JsonObject>();
    MessageReturn send_response{};
    String entity_name = "Unknown";
    const FeatureAddressType source_address = header.addressSource.get();
    const FeatureAddressType destination_address = header.addressDestination.get();

    // If its a result, no further processing. If the result is an error, log it
    // TODO: send the results back to the usecase that sent the original command?
    if (data->last_cmd == SpineDataTypeHandler::Function::resultData) {
        eebus.trace_fmtln("Usecases: Received resultData, no further processing");
        ResultDataType result_data = data->resultdatatype.get();
        if (result_data.errorNumber.get() != static_cast<uint8_t>(EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError)) {
            logger.printfln("Usecases: An error was received from the communication Partner. Error Number: %s, Description: %s. Error refers to Message: %d", EEBUS_USECASE_HELPERS::get_result_error_number_string(result_data.errorNumber.get()).c_str(), result_data.description.get().c_str(), header.msgCounterReference.get());
        }
        data->reset();
        return;
    }
    // Identify the usecase by matching the entity address and checking if the usecase has that feature. This needs to be this extensive as multiple usecases may share a feature.
    // Currently no two usecases have the same function but once they do, this and the usecases need to be updated
    bool found_dest_entity = false;

    for (EebusUsecase *entity : usecase_list) {
        if (header.addressDestination->entity.has_value() && entity->matches_entity_address(header.addressDestination->entity.get()) && !found_dest_entity) {
            send_response = entity->handle_message(header, data, responseObj);
            if (send_response.is_handled) {
                found_dest_entity = true;
                eebus.trace_fmtln("Usecases: Found entity: %s", get_usecases_name(entity->get_usecase_type()));
            } else {
                eebus.trace_fmtln("Usecases: Entity %s could not handle the message", get_usecases_name(entity->get_usecase_type()));
            }
        }
    }

    // If no usecase was found that could handle the message, we should inform the peer.
    if (!found_dest_entity) {
        eebus.trace_fmtln("Usecases: Received message for unknown entity: %s", EEBUS_USECASE_HELPERS::spine_address_to_string(header.addressDestination.get()).c_str());
        EEBUS_USECASE_HELPERS::build_result_data(responseObj, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Unknown entity requested");
        // We always send a response if we do not know the entity
        connection->send_datagram(response_doc, CmdClassifierType::result, destination_address, source_address, false);
        data->reset();
        return;
    }
    // Do we need to send a response
    if (send_response.send_response) {
        eebus.trace_fmtln("Usecases: Sending response");
        if (header.ackRequest.has_value() && header.ackRequest.get() && send_response.cmd_classifier != CmdClassifierType::result && header.cmdClassifier != CmdClassifierType::read) {

            eebus.trace_fmtln("Usecases: Header requested an ack, but sending a non-result response: %d", static_cast<int>(send_response.cmd_classifier));
        }
        eebus.eebus_usecase_state.get("commands_sent")->updateUint(eebus_responses_sent++);
        //send_spine_message(*header.addressDestination, *header.addressSource, response_doc, send_response);
        // We should use send_spine_message here but as we have the connection it is much quicker to send it directly back to it
        connection->send_datagram(response_doc, send_response.cmd_classifier, destination_address, source_address, false);
    } else {
        if (header.ackRequest.has_value() && header.ackRequest.get()) {
            eebus.trace_fmtln("Usecases: ERROR: Header requested an ack, but no response was generated");
        }
        eebus.trace_fmtln("Usecases: No response needed. Not sending anything");
    }
    data->reset();
}

template <typename T> size_t EEBusUseCases::inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, T &data, const char *function_name)
{
    if (initialized && EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS)
        return node_management.inform_subscribers(entity, feature, data, function_name);
    if constexpr (EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS)
        eebus.trace_fmtln("Attempted to inform subscribers while Usecases were not yet initialized");
    else
        eebus.trace_fmtln("Attempted to inform subscribers about a change in %s while subscriptions are disabled", function_name);
    return 0;
}
template <typename T> bool EEBusUseCases::send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, T payload, CmdClassifierType cmd_classifier, const char *function_name, bool want_ack)
{
    BasicJsonDocument<ArduinoJsonPsramAllocator> doc{SPINE_CONNECTION_MAX_JSON_SIZE};
    JsonObject obj = doc.to<JsonObject>();
    obj[function_name] = payload;
    return send_spine_message(destination, sender, obj, cmd_classifier, want_ack);
}

bool EEBusUseCases::send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, const JsonVariantConst payload, CmdClassifierType cmd_classifier, const bool want_ack)
{
    eebus_responses_sent++;
    eebus.eebus_usecase_state.get("commands_sent")->updateUint(eebus_responses_sent);
    if (sender.feature.isNull() || sender.entity.isNull()) {
        eebus.trace_fmtln("Usecases: Cannot send spine message, sender entity or feature is null");
        return false;
    }
    if (payload.isNull()) {
        eebus.trace_fmtln("Usecases: Payload is null");
        logger.printfln("payload is null");
    }
    if (sender.device.isNull()) {
        sender.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    }
    bool message_sent = false;
    if (SpineConnection *spine_conn = get_spine_connection(destination)) {
        spine_conn->send_datagram(payload, cmd_classifier, sender, destination, want_ack);
        message_sent = true;
    }

    return message_sent;
}

SpineConnection *EEBusUseCases::get_spine_connection(const FeatureAddressType &spine_address)
{
    for (auto &ship_connection : eebus.ship.ship_connections) {
        if (ship_connection && ship_connection->spine->check_known_address(spine_address)) {
            return ship_connection->spine.get();
        }
    }
    return nullptr;
}

LoadControlLimitDescriptionListDataType EVSEEntity::get_load_control_limit_description_list_data()
{
    LoadControlLimitDescriptionListDataType load_control_limit_description_list_data;
#ifdef EEBUS_ENABLE_LPC_USECASE
    eebus.usecases->limitation_of_power_consumption.get_loadcontrol_limit_description(&load_control_limit_description_list_data);
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    eebus.usecases->limitation_of_power_production.get_loadcontrol_limit_description(&load_control_limit_description_list_data);
#endif
    return load_control_limit_description_list_data;
}

LoadControlLimitListDataType EVSEEntity::get_load_control_limit_list_data()
{
    LoadControlLimitListDataType load_control_limit_list_data;
#ifdef EEBUS_ENABLE_LPC_USECASE
    eebus.usecases->limitation_of_power_consumption.get_loadcontrol_limit_list(&load_control_limit_list_data);
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    eebus.usecases->limitation_of_power_production.get_loadcontrol_limit_list(&load_control_limit_list_data);
#endif
    return load_control_limit_list_data;
}

DeviceConfigurationKeyValueDescriptionListDataType EVSEEntity::get_device_configuration_list_data()
{
    DeviceConfigurationKeyValueDescriptionListDataType device_configuration_description_list_data;
#ifdef EEBUS_ENABLE_LPC_USECASE
    eebus.usecases->limitation_of_power_consumption.get_device_configuration_description(&device_configuration_description_list_data);
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    eebus.usecases->limitation_of_power_production.get_device_configuration_description(&device_configuration_description_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_device_configuration_description_list_data(&device_configuration_description_list_data);
#endif
    return device_configuration_description_list_data;
}
DeviceConfigurationKeyValueListDataType EVSEEntity::get_device_configuration_value_list_data()
{
    DeviceConfigurationKeyValueListDataType device_configuration_value_list_data;
#ifdef EEBUS_ENABLE_LPC_USECASE
    eebus.usecases->limitation_of_power_consumption.get_device_configuration_value(&device_configuration_value_list_data);
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    eebus.usecases->limitation_of_power_production.get_device_configuration_value(&device_configuration_value_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_device_configuration_value_list_data(&device_configuration_value_list_data);
#endif
    return device_configuration_value_list_data;
}

DeviceDiagnosisStateDataType EVSEEntity::get_state_data()
{
    DeviceDiagnosisStateDataType state_data;
#ifdef EEBUS_ENABLE_EVSECC_USECASE
    eebus.usecases->evse_commissioning_and_configuration.get_device_diagnosis_state(&state_data);
#endif
    return state_data;
}
ElectricalConnectionCharacteristicListDataType EVSEEntity::get_electrical_connection_characteristic_list_data()
{
    ElectricalConnectionCharacteristicListDataType electrical_connection_characteristic_list_data;
#ifdef EEBUS_ENABLE_LPC_USECASE
    eebus.usecases->limitation_of_power_consumption.get_electrical_connection_characteristic(&electrical_connection_characteristic_list_data);
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    eebus.usecases->limitation_of_power_production.get_electrical_connection_characteristic(&electrical_connection_characteristic_list_data);
#endif
    return electrical_connection_characteristic_list_data;
}
ElectricalConnectionDescriptionListDataType EVSEEntity::get_electrical_connection_description_list_data()
{
    ElectricalConnectionDescriptionListDataType electrical_connection_description_list_data;
#ifdef EEBUS_ENABLE_MPC_USECASE
    eebus.usecases->monitoring_of_power_consumption.get_electricalConnection_description_list_data(&electrical_connection_description_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_electrical_connection_description_list_data(&electrical_connection_description_list_data);
#endif
    return electrical_connection_description_list_data;
}
ElectricalConnectionParameterDescriptionListDataType EVSEEntity::get_electrical_connection_parameter_description_list_data()
{
    ElectricalConnectionParameterDescriptionListDataType electrical_connection_parameter_description_list_data;
#ifdef EEBUS_ENABLE_MPC_USECASE
    eebus.usecases->monitoring_of_power_consumption.get_electricalConnection_parameter_description_list_data(&electrical_connection_parameter_description_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_electrical_connection_parameter_description_list_data(&electrical_connection_parameter_description_list_data);
#endif
    return electrical_connection_parameter_description_list_data;
}
DeviceClassificationManufacturerDataType EVSEEntity::get_device_classification_manufacturer_data()
{
    DeviceClassificationManufacturerDataType manufacturer_data;
#ifdef EEBUS_ENABLE_EVSECC_USECASE
    EvseccUsecase::get_device_classification_manufacturer(&manufacturer_data);
#endif
    return manufacturer_data;
}
MeasurementDescriptionListDataType EVSEEntity::get_measurement_description_list_data()
{
    MeasurementDescriptionListDataType measurement_description_list_data;
#ifdef EEBUS_ENABLE_MPC_USECASE
    eebus.usecases->monitoring_of_power_consumption.get_measurement_description_list_data(&measurement_description_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_measurement_description_list_data(&measurement_description_list_data);
#endif
    return measurement_description_list_data;
}
MeasurementConstraintsListDataType EVSEEntity::get_measurement_constraints_list_data()
{
    MeasurementConstraintsListDataType measurement_constraints_list_data;
#ifdef EEBUS_ENABLE_MPC_USECASE
    eebus.usecases->monitoring_of_power_consumption.get_measurement_constraints_list_data(&measurement_constraints_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_measurement_constraints_list_data(&measurement_constraints_list_data);
#endif
    return measurement_constraints_list_data;
}
MeasurementListDataType EVSEEntity::get_measurement_list_data()
{
    MeasurementListDataType measurement_list_data;
#ifdef EEBUS_ENABLE_MPC_USECASE
    eebus.usecases->monitoring_of_power_consumption.get_measurement_list_data(&measurement_list_data);
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    eebus.usecases->monitoring_of_grid_connection_point.get_measurement_list_data(&measurement_list_data);
#endif
    return measurement_list_data;
}

// Bill entity functions for EVCS usecase
BillDescriptionListDataType EVSEEntity::get_bill_description_list_data()
{
    BillDescriptionListDataType bill_description_list_data;
#ifdef EEBUS_ENABLE_EVCS_USECASE
    eebus.usecases->charging_summary.get_bill_description_list(&bill_description_list_data);
#endif
    return bill_description_list_data;
}

BillConstraintsListDataType EVSEEntity::get_bill_constraints_list_data()
{
    BillConstraintsListDataType bill_constraints_list_data;
#ifdef EEBUS_ENABLE_EVCS_USECASE
    eebus.usecases->charging_summary.get_bill_constraints_list(&bill_constraints_list_data);
#endif
    return bill_constraints_list_data;
}

BillListDataType EVSEEntity::get_bill_list_data()
{
    BillListDataType bill_list_data;
#ifdef EEBUS_ENABLE_EVCS_USECASE
    eebus.usecases->charging_summary.get_bill_list_data(&bill_list_data);
#endif
    return bill_list_data;
}

DeviceConfigurationKeyValueDescriptionListDataType EVEntity::get_device_configuration_value_description_list()
{
    DeviceConfigurationKeyValueDescriptionListDataType device_configuration_value_list_data;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_device_config_description(&device_configuration_value_list_data);
#endif
    return device_configuration_value_list_data;
}
DeviceConfigurationKeyValueListDataType EVEntity::get_device_configuration_value_list()
{
    DeviceConfigurationKeyValueListDataType device_configuration_value_list;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_device_config_list(&device_configuration_value_list);
#endif
    return device_configuration_value_list;
}
IdentificationListDataType EVEntity::get_identification_list_data()
{
    IdentificationListDataType identification_list_data;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_identification_list(&identification_list_data);
#endif
    return identification_list_data;
}
DeviceClassificationManufacturerDataType EVEntity::get_device_classification_manufacturer_data()
{
    DeviceClassificationManufacturerDataType manufacturer_data;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_device_classification_manufacturer(&manufacturer_data);
#endif
    return manufacturer_data;
}
ElectricalConnectionParameterDescriptionListDataType EVEntity::get_electrical_connection_parameter_description_list_data()
{
    ElectricalConnectionParameterDescriptionListDataType electrical_connection_parameter_description_list_data;
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    eebus.usecases->ev_charging_electricity_measurement.get_electrical_connection_parameters(&electrical_connection_parameter_description_list_data);
#endif
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_electrical_connection_parameter_description(&electrical_connection_parameter_description_list_data);
#endif
    return electrical_connection_parameter_description_list_data;
}
ElectricalConnectionPermittedValueSetListDataType EVEntity::get_electrical_connection_permitted_list_data()
{
    ElectricalConnectionPermittedValueSetListDataType electrical_connection_permitted_value_set_list_data;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    eebus.usecases->ev_commissioning_and_configuration.get_electrical_connection_permitted_values(&electrical_connection_permitted_value_set_list_data);
#endif
#ifdef EEBUS_ENABLE_OPEV_USECASE
    eebus.usecases->overload_protection_by_ev_charging_current_curtailment.get_electrical_connection_permitted_list_data(&electrical_connection_permitted_value_set_list_data);
#endif
    return electrical_connection_permitted_value_set_list_data;
}
ElectricalConnectionDescriptionListDataType EVEntity::get_electrical_connection_description_list_data()
{
    ElectricalConnectionDescriptionListDataType electrical_connection_description_list_data;
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    eebus.usecases->ev_charging_electricity_measurement.get_electrical_connection_description(&electrical_connection_description_list_data);
#endif
    return electrical_connection_description_list_data;
}
DeviceDiagnosisStateDataType EVEntity::get_diagnosis_state_data()
{
    DeviceDiagnosisStateDataType diagnosis_state_data;
#ifdef EEBUS_ENABLE_EVCC_USECASE
    diagnosis_state_data = eebus.usecases->ev_commissioning_and_configuration.get_device_diagnosis_state();
#endif
    return diagnosis_state_data;
}
MeasurementDescriptionListDataType EVEntity::get_measurement_description_list_data()
{
    MeasurementDescriptionListDataType measurement_description_list_data;
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    eebus.usecases->ev_charging_electricity_measurement.get_measurement_description_list(&measurement_description_list_data);
#endif
    return measurement_description_list_data;
}
MeasurementConstraintsListDataType EVEntity::get_measurement_constraints_list_data()
{
    MeasurementConstraintsListDataType measurement_constraints_list_data;
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    eebus.usecases->ev_charging_electricity_measurement.get_measurement_constraints(&measurement_constraints_list_data);
#endif
    return measurement_constraints_list_data;
}
MeasurementListDataType EVEntity::get_measurement_list_data()
{
    MeasurementListDataType measurement_list_data;
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    eebus.usecases->ev_charging_electricity_measurement.get_measurement_list(&measurement_list_data);
#endif
    return measurement_list_data;
}
LoadControlLimitDescriptionListDataType EVEntity::get_load_control_limit_description_list_data()
{
    LoadControlLimitDescriptionListDataType load_control_limit_description_list_data;

#ifdef EEBUS_ENABLE_OPEV_USECASE
    eebus.usecases->overload_protection_by_ev_charging_current_curtailment.get_load_control_limit_description_list_data(&load_control_limit_description_list_data);
#endif

    return load_control_limit_description_list_data;
}
LoadControlLimitListDataType EVEntity::get_load_control_limit_list_data()
{
    LoadControlLimitListDataType load_control_limit_list_data;
#ifdef EEBUS_ENABLE_OPEV_USECASE
    eebus.usecases->overload_protection_by_ev_charging_current_curtailment.get_load_control_limit_list_data(&load_control_limit_list_data);
#endif
    return load_control_limit_list_data;
}
LoadControlLimitConstraintsListDataType EVEntity::get_load_control_limit_constraints_list_data()
{
    LoadControlLimitConstraintsListDataType load_control_limit_constraints_list_data;
#ifdef EEBUS_ENABLE_OPEV_USECASE
    //eebus.usecases->overload_protection_by_ev_charging_current_curtailment.get_load_control_constraints_list_data(&load_control_limit_constraints_list_data);
#endif
    return load_control_limit_constraints_list_data;
}

namespace EEBUS_USECASE_HELPERS
{
std::string get_spine_device_name()
{
    // This returns the device name as defined in EEBUS SPINE TS ProtocolSpecification
    std::string name = ("d:_n:" + eebus.get_eebus_name()).c_str();
    return name;
}

String get_result_error_number_string(const int error_number)
{
    ResultErrorNumber error = static_cast<ResultErrorNumber>(error_number);
    switch (error) {
        case ResultErrorNumber::NoError:
            return "NoError";
        case ResultErrorNumber::GeneralError:
            return "GeneralError";
        case ResultErrorNumber::Timeout:
            return "Timeout";
        case ResultErrorNumber::Overload:
            return "Overload";
        case ResultErrorNumber::DestinationUnknown:
            return "DestinationUnknown";
        case ResultErrorNumber::DestinationUnreachable:
            return "DestinationUnreachable";
        case ResultErrorNumber::CommandNotSupported:
            return "CommandNotSupported";
        case ResultErrorNumber::CommandRejected:
            return "CommandRejected";
        case ResultErrorNumber::RestrictedFunctionExchangeCombinationNotSupported:
            return "RestrictedFunctionExchangeCombinationNotSupported";
        case ResultErrorNumber::BindingRequired:
            return "BindingRequired";
    }
    return "UnknownError";
}

void build_result_data(const JsonObject &response, ResultErrorNumber error_number, const char *description)
{
    if (error_number != ResultErrorNumber::NoError) {
        eebus.trace_fmtln("Usecases: Building result data with error number %s and description: %s", get_result_error_number_string(static_cast<int>(error_number)).c_str(), description);
    }
    ResultDataType result{};
    result.description = description;
    result.errorNumber = static_cast<uint8_t>(error_number);
    response["resultData"] = result;
}

std::string iso_duration_to_string(seconds_t duration)
{
    uint64_t duration_uint = duration.as<uint64_t>();
    std::string unit = "S";
    if (duration_uint % 60 == 0) {
        duration_uint = duration_uint / 60;
        unit = "M";
    }
    if (duration_uint % 60 == 0) {
        duration_uint = duration_uint / 60;
        unit = "H";
    }
    return "PT" + std::to_string(duration_uint) + unit;
}

seconds_t iso_duration_to_seconds(const std::string &iso_duration)
{
    int64_t duration_seconds = 0;
    size_t p_pos = iso_duration.find('P');
    size_t t_pos = iso_duration.find('T');
    std::string between_p_t = (p_pos != std::string::npos && t_pos != std::string::npos && t_pos > p_pos + 1) ? iso_duration.substr(p_pos + 1, t_pos - p_pos - 1) : "";
    std::string after_t = (t_pos != std::string::npos) ? iso_duration.substr(t_pos + 1) : "";

    std::regex first_part_regex("([0-9]+[YMD])");
    std::sregex_iterator first_it(between_p_t.begin(), between_p_t.end(), first_part_regex);
    std::sregex_iterator first_end;
    for (; first_it != first_end; ++first_it) {
        std::string match = first_it->str();
        int value = std::stoi(match.substr(0, match.size() - 1));
        char unit = match.back();
        if (unit == 'Y') {
            duration_seconds += value * 31536000; // Approximate, not accounting for leap years
        } else if (unit == 'M') {
            duration_seconds += value * 2592000; // Approximate, assuming 30 days in a month
        } else if (unit == 'D') {
            duration_seconds += value * 86400;
        }
    }
    std::regex second_part_regex("([0-9]+[HMS])");

    std::sregex_iterator second_it(after_t.begin(), after_t.end(), second_part_regex);
    for (std::sregex_iterator second_end; second_it != second_end; ++second_it) {
        std::string match = second_it->str();
        int value = std::stoi(match.substr(0, match.size() - 1));
        char unit = match.back();
        if (unit == 'H') {
            duration_seconds += value * 3600;
        } else if (unit == 'M') {
            duration_seconds += value * 60;
        } else if (unit == 'S') {
            duration_seconds += value;
        }
    }
    return seconds_t(duration_seconds);
}

time_t iso_timestamp_to_unix(const char *iso_timestamp, time_t *t)
{
    struct tm tm_time{};
    // Try parsing ISO 8601 format: YYYY-MM-DDTHH:MM:SSZ or YYYY-MM-DDTHH:MM:SS+HH:MM
    const char *remaining = strptime(iso_timestamp, "%Y-%m-%dT%H:%M:%S", &tm_time);
    if (remaining == nullptr) {
        return false;
    }
    // Use mktime to convert. Note: mktime interprets tm_time as local time.
    // For proper UTC handling, we need to adjust for timezone.
    tm_time.tm_isdst = 0;
    *t = mktime(&tm_time);
    // Adjust for local timezone offset to get UTC
    struct tm gm_tm{};
    gmtime_r(t, &gm_tm);
    time_t gm_t = mktime(&gm_tm);
    *t += (*t - gm_t); // Adjust for timezone difference
    return true;
}

String unix_to_iso_timestamp(time_t unix_time)
{
    tm t{};
    gmtime_r(&unix_time, &t);
    constexpr int ISO_8601_MAX_LEN = 36;
    char buf[ISO_8601_MAX_LEN];
    strftime(buf, ISO_8601_MAX_LEN, "%FT%TZ", &t);
    return buf;
}
String spine_address_to_string(const FeatureAddressType &address)
{
    std::string out;
    if (address.device.has_value()) {
        out += address.device.get();
    }
    if (address.entity.has_value()) {
        if (!out.empty()) {
            out += "/";
        }
        out += "[";
        const auto &entities = address.entity.get();
        for (size_t i = 0; i < entities.size(); ++i) {
            out += std::to_string(entities[i]);
            if (i + 1 < entities.size()) {
                out += ",";
            }
        }
        out += "]";
    }
    if (address.feature.has_value()) {
        if (!out.empty()) {
            out += "/";
        }
        out += std::to_string(address.feature.get());
    }
    return {out.c_str()};
}
bool compare_spine_addresses(const FeatureAddressType &addr1, const FeatureAddressType &addr2)
{
    if (addr1.device.get() != addr2.device) {
        return false;
    }
    if (addr1.entity.get() != addr2.entity) {
        return false;
    }
    if (addr1.feature.get() != addr2.feature) {
        return false;
    }
    return true;
}
float scaled_numbertype_to_float(const ScaledNumberType &number)
{
    if (number.scale.has_value()) {
        return static_cast<float>(number.number.get()) * std::pow(10.0f, static_cast<float>(number.scale.get()));
    }
    return static_cast<float>(number.number.get());
}
int scaled_numbertype_to_int(const ScaledNumberType &number)
{
    if (number.scale.has_value()) {
        return static_cast<int>(static_cast<float>(number.number.get()) * std::pow(10.0f, static_cast<float>(number.scale.get())));
    }
    return static_cast<int>(number.number.get());
}
} // namespace EEBUS_USECASE_HELPERS
