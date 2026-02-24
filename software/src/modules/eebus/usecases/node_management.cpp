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

#include "node_management.h"

#include "../eebus.h"
#include "../eebus_usecases.h"
#include "../module_dependencies.h"

NodeManagementEntity::NodeManagementEntity()
{
    subscription_data.subscriptionEntry.emplace();
}

void NodeManagementEntity::set_usecaseManager(EEBusUseCases *new_usecase_interface)
{
    usecase_interface = new_usecase_interface;
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

    node_management_detailed_data.deviceInformation->description->description = std::string(OPTIONS_PRODUCT_NAME()) + " by " + OPTIONS_MANUFACTURER_FULL();
    node_management_detailed_data.deviceInformation->description->label = OPTIONS_PRODUCT_NAME();
    node_management_detailed_data.deviceInformation->description->networkFeatureSet = NetworkManagementFeatureSetType::simple;
    node_management_detailed_data.deviceInformation->description->deviceAddress->device = EEBUS_USECASE_HELPERS::get_spine_device_name();
#ifdef EEBUS_MODE_EVSE
    node_management_detailed_data.deviceInformation->description->deviceType = DeviceTypeEnumType::ChargingStation;
#endif
#ifdef EEBUS_MODE_EM
    node_management_detailed_data.deviceInformation->description->deviceType = DeviceTypeEnumType::EnergyManagementSystem;
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

        // Iterate through the list of subscriptions and find the ones that match the delete request
        for (size_t i = 0; i < subscription_data.subscriptionEntry.get().size(); ++i) {
            SubscriptionManagementEntryDataType entry = subscription_data.subscriptionEntry->at(i);

            // Compares two optionals. If one has no value, it's considered a wildcard and matches anything
            auto optional_equal_or_undefined = [](const auto &a, const auto &b) {
                if (!a.has_value() || !b.has_value())
                    return true;
                return *a == *b;
            };

            // We handle cases where the client or server address is not set which is allowed
            FeatureAddressType client_address = subscription_delete_call.subscriptionDelete->clientAddress.get();
            FeatureAddressType server_address = subscription_delete_call.subscriptionDelete->serverAddress.get();
            if (!subscription_delete_call.subscriptionDelete->clientAddress->device && subscription_delete_call.subscriptionDelete->serverAddress->device) {
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

            if (optional_equal_or_undefined(client_address.entity, entry.clientAddress->entity) && optional_equal_or_undefined(server_address.entity, entry.serverAddress->entity) && optional_equal_or_undefined(client_address.feature, entry.clientAddress->feature) && optional_equal_or_undefined(server_address.feature, entry.serverAddress->feature)) {
                to_delete_indices.push_back(i);
                continue;
            }
        }

        // Delete all the found entries
        std::sort(to_delete_indices.rbegin(), to_delete_indices.rend()); // sort descending
        for (size_t i : to_delete_indices) {
            if (i < subscription_data.subscriptionEntry.get().size()) {
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

        auto optional_equal_or_undefined = [](const auto &a, const auto &b) {
            if (!a.has_value() || !b.has_value())
                return true;
            return *a == *b;
        };
        NodeManagementBindingDeleteCallType binding_delete_call = data->nodemanagementbindingdeletecalltype.get();
        std::vector<size_t> to_delete_indices{};

        for (size_t i = 0; i < binding_management_entry_list_.bindingManagementEntryData->size(); ++i) {
            BindingManagementEntryDataType entry = binding_management_entry_list_.bindingManagementEntryData->at(i);

            FeatureAddressType client_address = binding_delete_call.bindingDelete->clientAddress.get();
            FeatureAddressType server_address = binding_delete_call.bindingDelete->serverAddress.get();
            if (!binding_delete_call.bindingDelete->clientAddress->device && binding_delete_call.bindingDelete->serverAddress->device) {
                client_address.device = header.addressSource->device.get();
                server_address.device = header.addressDestination->device.get();
            } else if (binding_delete_call.bindingDelete->clientAddress->device && !binding_delete_call.bindingDelete->serverAddress->device) {
                server_address.device = header.addressSource->device.get();
            } else if (!binding_delete_call.bindingDelete->clientAddress->device && binding_delete_call.bindingDelete->serverAddress->device) {
                client_address.device = header.addressDestination->device.get();
            }
            if (client_address.device.get() != entry.clientAddress->device.get() && server_address.device.get() != entry.serverAddress->device.get()) {
                continue;
            }

            if (optional_equal_or_undefined(client_address.entity, entry.clientAddress->entity) && optional_equal_or_undefined(server_address.entity, entry.serverAddress->entity) && optional_equal_or_undefined(client_address.feature, entry.clientAddress->feature) && optional_equal_or_undefined(server_address.feature, entry.serverAddress->feature)) {
                to_delete_indices.push_back(i);
                continue;
            }
        }

        std::sort(to_delete_indices.rbegin(), to_delete_indices.rend());
        for (size_t i : to_delete_indices) {
            if (i < binding_management_entry_list_.bindingManagementEntryData.get().size()) {
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
    entity.description->label = "Node Management";

    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> NodeManagementEntity::get_detailed_discovery_feature_information() const
{
    NodeManagementDetailedDiscoveryFeatureInformationType feature = {};
    feature.description->featureAddress->entity = entity_address;
    feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::NodeManagement);
    feature.description->featureType = FeatureTypeEnumType::NodeManagement;
    feature.description->role = RoleType::special;

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
}

// Explicit template instantiations for commonly used types
// Used by heartbeat.cpp
template size_t NodeManagementEntity::inform_subscribers<DeviceDiagnosisHeartbeatDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const DeviceDiagnosisHeartbeatDataType, const char *);
// Used internally by node_management.cpp
template size_t NodeManagementEntity::inform_subscribers<NodeManagementDetailedDiscoveryDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const NodeManagementDetailedDiscoveryDataType, const char *);
// Used by loadcontrol.cpp
template size_t NodeManagementEntity::inform_subscribers<LoadControlLimitListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const LoadControlLimitListDataType, const char *);
template size_t NodeManagementEntity::inform_subscribers<DeviceConfigurationKeyValueListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const DeviceConfigurationKeyValueListDataType, const char *);
template size_t NodeManagementEntity::inform_subscribers<ElectricalConnectionCharacteristicListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const ElectricalConnectionCharacteristicListDataType, const char *);
// Used by evcem.cpp, mpc.cpp, mgcp.cpp
template size_t NodeManagementEntity::inform_subscribers<MeasurementListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const MeasurementListDataType, const char *);
template size_t NodeManagementEntity::inform_subscribers<MeasurementConstraintsListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const MeasurementConstraintsListDataType, const char *);
template size_t NodeManagementEntity::inform_subscribers<MeasurementDescriptionListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const MeasurementDescriptionListDataType, const char *);
// Used by evcc.cpp, evsecc.cpp
template size_t NodeManagementEntity::inform_subscribers<DeviceDiagnosisStateDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const DeviceDiagnosisStateDataType, const char *);
template size_t NodeManagementEntity::inform_subscribers<DeviceConfigurationKeyValueDescriptionListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const DeviceConfigurationKeyValueDescriptionListDataType, const char *);
template size_t NodeManagementEntity::inform_subscribers<IdentificationListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const IdentificationListDataType, const char *);
template size_t NodeManagementEntity::inform_subscribers<DeviceClassificationManufacturerDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const DeviceClassificationManufacturerDataType, const char *);
template size_t NodeManagementEntity::inform_subscribers<ElectricalConnectionParameterDescriptionListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const ElectricalConnectionParameterDescriptionListDataType, const char *);
// Used by evcc.cpp, opev.cpp
template size_t NodeManagementEntity::inform_subscribers<ElectricalConnectionPermittedValueSetListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const ElectricalConnectionPermittedValueSetListDataType, const char *);
// Used by evcs.cpp
template size_t NodeManagementEntity::inform_subscribers<BillListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const BillListDataType, const char *);
// Used by cevc.cpp
template size_t NodeManagementEntity::inform_subscribers<TimeSeriesListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const TimeSeriesListDataType, const char *);
template size_t NodeManagementEntity::inform_subscribers<IncentiveTableDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, const IncentiveTableDataType, const char *);
