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

#include "eebus_usecases.h"

#include "build.h"
#include "eebus.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"

CmdClassifierType NodeManagementEntity::handle_binding(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    // Binding Request as defined in EEBus SPINE TS ProtocolSpecification 7.3.2
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingRequestCall) {
        if (data->nodemanagementbindingrequestcalltype && data->nodemanagementbindingrequestcalltype->bindingRequest
            && data->nodemanagementbindingrequestcalltype->bindingRequest->clientAddress
            && data->nodemanagementbindingrequestcalltype->bindingRequest->serverAddress) {
            BindingManagementEntryDataType binding_entry;
            binding_entry.clientAddress = data->nodemanagementbindingrequestcalltype->bindingRequest->clientAddress;
            binding_entry.serverAddress = data->nodemanagementbindingrequestcalltype->bindingRequest->serverAddress;
            // We are supposed consider the featuretype of the feature
            //SpineOptional<FeatureTypeEnumType> feature_type = data->nodemanagementbindingrequestcalltype->bindingRequest->serverFeatureType;

            if (check_is_bound(binding_entry.clientAddress.get(), binding_entry.serverAddress.get())) {
                eebus.trace_fmtln("Binding requested but is already bound");
            } else {
                binding_entry.bindingId = binding_management_entry_list_.bindingManagementEntryData->size();
                binding_management_entry_list_.bindingManagementEntryData->push_back(binding_entry);
            }
            EEBUS_USECASE_HELPERS::build_result_data(response,
                                                     EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError,
                                                     "Binding request was successful");
            return CmdClassifierType::reply;
        }
        eebus.trace_fmtln("Binding requested but failed");
        EEBUS_USECASE_HELPERS::build_result_data(response,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected,
                                                 "Binding request failed");
        return CmdClassifierType::reply;
    }
    // Binding Data as defined in EEBus SPINE TS ProtocolSpecification 7.3.3
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingData) {
        NodeManagementBindingDataType binding_data;
        binding_data.bindingEntry = binding_management_entry_list_.bindingManagementEntryData;
        response["nodeManagementBindingData"] = binding_data;
        eebus.trace_fmtln("List of bindings was requested");
        return CmdClassifierType::reply;
    }
    // Binding Release as defined in EEBus SPINE TS ProtocolSpecification 7.3.4
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingDeleteCall) {
        if (!data->nodemanagementbindingdeletecalltype && data->nodemanagementbindingdeletecalltype->bindingDelete
            && data->nodemanagementbindingdeletecalltype->bindingDelete->clientAddress
            && data->nodemanagementbindingdeletecalltype->bindingDelete->serverAddress) {
            eebus.trace_fmtln("A binding release was requested but no binding delete information was provided or request was malformed");
            EEBUS_USECASE_HELPERS::build_result_data(response,
                                                     EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected,
                                                     "Binding release failed");
            return CmdClassifierType::reply;
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
        EEBUS_USECASE_HELPERS::build_result_data(response,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError,
                                                 "Removed bindings successfully");
        return CmdClassifierType::reply;
    }

    return CmdClassifierType::reply;
}

bool NodeManagementEntity::check_is_bound(FeatureAddressType &client, FeatureAddressType &server) const
{
    for (const BindingManagementEntryDataType &binding : binding_management_entry_list_.bindingManagementEntryData.get()) {
        if (binding.clientAddress && binding.serverAddress) {
            if (binding.clientAddress->device.get() == client.device.get() && binding.serverAddress->device.get() == server.device.get() && binding.clientAddress->entity.get() == client.entity.get() && binding.serverAddress->entity.get() == server.entity.get() && binding.clientAddress->feature.get() == client.feature.get() && binding.serverAddress->feature.get() == server.feature.get()) {
                return true; // The client is bound to the server
            }
        }
    }
    return false;
}

UseCaseInformationDataType NodeManagementEntity::get_usecase_information()
{
    return UseCaseInformationDataType();
    // This should never be used as the NodeManagementUsecase has no usecase information
}

CmdClassifierType NodeManagementEntity::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    eebus.trace_fmtln("NodeManagementUsecase: Handling message with cmdClassifier %d and command %s",
                      static_cast<int>(header.cmdClassifier.get()),
                      data->function_to_string(data->last_cmd).c_str());
    if (header.cmdClassifier == CmdClassifierType::read && data->last_cmd == SpineDataTypeHandler::Function::nodeManagementUseCaseData) {
        eebus.trace_fmtln("NodeManagementUsecase: Command identified as NodeManagementUseCaseData");
        if (read_usecase_data(header, data, response)) {
            return CmdClassifierType::reply;
        }
        return CmdClassifierType::EnumUndefined;

    }
    if (header.cmdClassifier == CmdClassifierType::read && data->last_cmd == SpineDataTypeHandler::Function::nodeManagementDetailedDiscoveryData) {
        eebus.trace_fmtln("NodeManagementUsecase: Command identified as NodeManagementDetailedDiscoveryData");
        if (read_detailed_discovery_data(header, data, response)) {
            return CmdClassifierType::reply;
        }
        return CmdClassifierType::EnumUndefined;
    }
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionData || data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionRequestCall || data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionDeleteCall) {
        eebus.trace_fmtln("NodeManagementUsecase: Command identified as Subscription handling");
        return handle_subscription(header, data, response, connection);
    }

    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingData || data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingRequestCall || data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingDeleteCall) {
        eebus.trace_fmtln("NodeManagementUsecase: Command identified as Binding handling");
        return handle_binding(header, data, response);
    }

    eebus.trace_fmtln("NodeManagementUsecase: Unknown. Command not handled");
    EEBUS_USECASE_HELPERS::build_result_data(response,
                                             EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandNotSupported,
                                             "Unknown. Command not supported. Support may be pending.");

    return CmdClassifierType::EnumUndefined;
}

bool NodeManagementEntity::read_usecase_data(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) const
{
    NodeManagementUseCaseDataType node_management_usecase_data;
    for (EebusEntity *uc : usecase_interface->entity_list) {
        if (uc->get_usecase_type() != UseCaseType::NodeManagement) {
            node_management_usecase_data.useCaseInformation->push_back(uc->get_usecase_information());
        }
    }
    if (!node_management_usecase_data.useCaseInformation->empty()) {
        response["nodeManagementUseCaseData"] = node_management_usecase_data;
        if (response["nodeManagementUseCaseData"].isNull()) {
            eebus.trace_fmtln("Error while writing NodeManagementUseCaseData to response");
        }
        return true;
    }
    eebus.trace_fmtln("An error occurred while trying to read the NodeManagementUseCaseData");
    return false;
}

bool NodeManagementEntity::read_detailed_discovery_data(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) const
{
    // Detailed discovery as defined in EEBus SPINE TS ProtocolSpecification 7.1.2
    NodeManagementDetailedDiscoveryDataType node_management_detailed_data = {};
    node_management_detailed_data.specificationVersionList->specificationVersion->push_back(SUPPORTED_SPINE_VERSION);

    node_management_detailed_data.deviceInformation->description->description = api.getState("info/display_name")->get("display_name")->asEphemeralCStr(); // Optional. Shall not be longer than 4096 characters.
    node_management_detailed_data.deviceInformation->description->label = api.getState("info/name")->get("display_type")->asEphemeralCStr();
    // Optional. Shall not be longer than 256 characters.
    node_management_detailed_data.deviceInformation->description->networkFeatureSet = NetworkManagementFeatureSetType::simple;
    // Only simple operation is supported. We dont act as a SPINE router or anything like that.
    node_management_detailed_data.deviceInformation->description->deviceAddress->device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    node_management_detailed_data.deviceInformation->description->deviceType = DeviceTypeEnumType::ChargingStation; // Mandatory. String defined in EEBUS SPINE TS ResourceSpecification 4.1

    for (EebusEntity *uc : usecase_interface->entity_list) {
        node_management_detailed_data.entityInformation->push_back(uc->get_detailed_discovery_entity_information());
        auto features = uc->get_detailed_discovery_feature_information();
        node_management_detailed_data.featureInformation->insert(node_management_detailed_data.featureInformation->end(), features.begin(), features.end());
    }
    return response["nodeManagementDetailedDiscoveryData"].set(node_management_detailed_data);

}

CmdClassifierType NodeManagementEntity::handle_subscription(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionRequestCall && header.cmdClassifier == CmdClassifierType::call) {
        if (!data->nodemanagementsubscriptionrequestcalltype || !data->nodemanagementsubscriptionrequestcalltype->subscriptionRequest || !data->nodemanagementsubscriptionrequestcalltype->subscriptionRequest->clientAddress || !data->nodemanagementsubscriptionrequestcalltype->subscriptionRequest->serverAddress) {
            EEBUS_USECASE_HELPERS::build_result_data(response,
                                                     EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected,
                                                     "Subscription request failed, no or invalid subscription request data provided");
            return CmdClassifierType::result;
        }
        NodeManagementSubscriptionRequestCallType request = data->nodemanagementsubscriptionrequestcalltype.get();
        SubscriptionManagementEntryDataType entry = SubscriptionManagementEntryDataType();
        //TODO: Implement and check trust level of the client
        entry.clientAddress = request.subscriptionRequest->clientAddress;
        entry.serverAddress = request.subscriptionRequest->serverAddress;
        subscription_data.subscriptionEntry->push_back(entry);
        EEBUS_USECASE_HELPERS::build_result_data(response,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError,
                                                 "Subscription request was successful");
        return CmdClassifierType::EnumUndefined;
    }
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionData && header.cmdClassifier == CmdClassifierType::read) {
        response["nodeManagementSubscriptionData"] = subscription_data;
        return CmdClassifierType::reply;;
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
                subscription_data.subscriptionEntry.get().erase(
                    subscription_data.subscriptionEntry.get().begin() + i);
            }
        }
        EEBUS_USECASE_HELPERS::build_result_data(response,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError,
                                                 "Removed Subscriptions successfully");
        return CmdClassifierType::reply;
    }

    return CmdClassifierType::reply;
}

NodeManagementDetailedDiscoveryEntityInformationType NodeManagementEntity::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity = {};
    entity.description->entityAddress->entity->push_back(0); // Entity 0 is the NodeManagement entity
    entity.description->entityType = EntityTypeEnumType::DeviceInformation;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.7
    entity.description->label = "Node Management"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> NodeManagementEntity::get_detailed_discovery_feature_information() const
{
    NodeManagementDetailedDiscoveryFeatureInformationType feature = {};
    feature.description->featureAddress->entity->push_back(0); // Entity 0 is the NodeManagement entity
    feature.description->featureAddress->feature = 0; // Feature 0 is the NodeManagement feature
    feature.description->featureType = FeatureTypeEnumType::NodeManagement;
    // The feature type as defined in EEBUS SPINE TS ResourceSpecification 4.3.19
    feature.description->role = RoleType::server;

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

void NodeManagementEntity::inform_subscribers(int entity, int feature, SpineDataTypeHandler *data)
{
    std::optional<std::vector<int>> entities{};
    entities->push_back(entity);
    BasicJsonDocument<ArduinoJsonPsramAllocator> response(SPINE_CONNECTION_MAX_JSON_SIZE);
    JsonVariant dst = response.createNestedObject(data->function_to_string(data->last_cmd));
    data->last_cmd_to_json(dst);
    for (SubscriptionManagementEntryDataType &subscription : subscription_data.subscriptionEntry.get()) {
        if (subscription.serverAddress->entity == entities && subscription.serverAddress->feature == feature) {
            for (auto &ship_connection : eebus.ship.ship_connections) {
                if (ship_connection && ship_connection->spine->check_known_address(subscription.clientAddress.get())) {
                    ship_connection->spine->send_datagram(response.as<JsonObject>(), CmdClassifierType::notify, subscription.serverAddress.get(), subscription.clientAddress.get(), false);
                }
            }
        }
    }
};

void NodeManagementEntity::set_usecaseManager(EEBusUseCases *new_usecase_interface)
{
    usecase_interface = new_usecase_interface;
}

UseCaseInformationDataType EvseEntity::get_usecase_information()
{
    UseCaseInformationDataType evcs_usecase;
    evcs_usecase.actor = "EVSE"; // The actor can either be EVSE or Energy Broker but we support only EVSE

    UseCaseSupportType evcs_usecase_support;
    evcs_usecase_support.useCaseName = "evChargingSummary";
    evcs_usecase_support.useCaseVersion = "1.0.1";
    evcs_usecase_support.useCaseAvailable = true;
    evcs_usecase_support.scenarioSupport->push_back(1); //We support only scenario 1 which is defined in Chapter 2.3.1
    evcs_usecase_support.useCaseDocumentSubRevision = "release";
    evcs_usecase.useCaseSupport->push_back(evcs_usecase_support);

    FeatureAddressType evcs_usecase_feature_address;
    evcs_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    evcs_usecase_feature_address.entity->push_back(1);
    evcs_usecase_feature_address.feature = 1;
    evcs_usecase.address = evcs_usecase_feature_address;
    return evcs_usecase;
}

CmdClassifierType EvseEntity::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.addressDestination->feature.has_value() && header.addressDestination->feature == bill_feature_address) {
        return bill_feature(header, data, response, connection);
    }

    return CmdClassifierType::EnumUndefined;
}

NodeManagementDetailedDiscoveryEntityInformationType EvseEntity::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EVSE;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "Charging Summary"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> EvseEntity::get_detailed_discovery_feature_information() const
{
    NodeManagementDetailedDiscoveryFeatureInformationType feature{};

    feature.description->featureAddress->entity = entity_address;
    feature.description->featureAddress->feature = 1; // Feature 1 is the Bill feature

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
    feature.description->supportedFunction->push_back(billListData);

    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;
    features.push_back(feature);
    return features;
}


CmdClassifierType EvseEntity::bill_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (data->last_cmd == SpineDataTypeHandler::Function::billDescriptionListData) {
        // EEBUS_UC_TS_EVCHargingSummary_v1.0.1.pdf 3.2.1.2.2.1 Function "billDescriptionListData"
        BillDescriptionListDataType billDescriptionListData{};
        billDescriptionListData.billDescriptionData.emplace();
        size_t count_charges = eebus.state.get("charge_state")->count();

        for (size_t i = 0; i < count_charges; i++) {
            BillDescriptionDataType billDescriptionData{};
            billDescriptionData.billWriteable = false;
            int id = eebus.state.get("charge_state")->get(i)->get("id")->asUint16();
            billDescriptionData.billId = id;
            billDescriptionData.supportedBillType.emplace();
            billDescriptionData.supportedBillType->push_back(BillTypeEnumType::chargingSummary);
            billDescriptionListData.billDescriptionData->push_back(billDescriptionData);

        }
        response["billDescriptionListData"] = billDescriptionListData;
        return CmdClassifierType::reply;
    }
    if (data->last_cmd == SpineDataTypeHandler::Function::billConstraintsListData) {
        // EEBUS_UC_TS_EVCHargingSummary_v1.0.1.pdf 3.2.1.2.2.2 Function "billConstraintsListData"
        BillConstraintsListDataType billConstraintsListData{};
        billConstraintsListData.billConstraintsData.emplace();
        size_t count_charges = eebus.state.get("charge_state")->count();
        for (size_t i = 0; i < count_charges; i++) {
            BillConstraintsDataType billConstraintsData{};
            int id = eebus.state.get("charge_state")->get(i)->get("id")->asUint16();
            billConstraintsData.billId = id;
            billConstraintsData.positionCountMin = "0";
            billConstraintsData.positionCountMax = "0";
            billConstraintsListData.billConstraintsData->push_back(billConstraintsData);
        }
        response["billConstraintsListData"] = billConstraintsListData;
        return CmdClassifierType::reply;

    }
    if (data->last_cmd == SpineDataTypeHandler::Function::billListData) {
        // EEBUS_UC_TS_EVCHargingSummary_v1.0.1.pdf 3.2.1.2.2.3 Function "billListData"
        BillListDataType billListData{};
        billListData.billData.emplace();
        size_t count_charges = eebus.state.get("charge_state")->count();

        for (size_t i = 0; i < count_charges; i++) {
            // TODO: Add more entries billlistdata. Maybe get them from charge tracker.
            int id = eebus.state.get("charge_state")->get(i)->get("id")->asUint16();
            float charged_kwh = eebus.state.get("charge_state")->get(i)->get("charged_kwh")->asFloat();
            uint64_t start_time = eebus.state.get("charge_state")->get(i)->get("start_time")->asUint();
            uint32_t duration = eebus.state.get("charge_state")->get(i)->get("duration")->asUint();
            float cost = eebus.state.get("charge_state")->get(i)->get("cost")->asFloat();

            BillDataType billData{};
            billData.billId = id;
            billData.billType = BillTypeEnumType::chargingSummary;
            billData.total->timePeriod->startTime = std::to_string(start_time); // TODO: maybe use iso time instead of timestamp
            billData.total->timePeriod->endTime = std::to_string(start_time + duration);

            BillCostType billCost{};
            billCost.cost->number = static_cast<int>(cost / 100.0);
            billCost.cost->scale = 2; // We send the cost in cents, so scale 2 means the number is divided by 10^2 to get EUR
            billCost.currency = CurrencyEnumType::EUR;
            BillValueType billValue{};
            billValue.unit = UnitOfMeasurementEnumType::Wh;
            billValue.value->number = static_cast<int>(charged_kwh * 1000.0);

            billData.total->cost.emplace();
            billData.total->value.emplace();
            billData.total->value->push_back(billValue);
            billData.total->cost.get().push_back(billCost);

            billListData.billData->push_back(billData);
        }
        response["billListData"] = billListData;
        return CmdClassifierType::reply;
    }
    return CmdClassifierType::EnumUndefined;
}

EEBusUseCases::EEBusUseCases()
{
    node_management = NodeManagementEntity();
    node_management.set_usecaseManager(this);
    node_management.set_entity_address({0});
    charging_summary = EvseEntity();
    charging_summary.set_entity_address({1});
    // TODO: Set entity address of entities
}

void EEBusUseCases::handle_message(HeaderType &header, SpineDataTypeHandler *data, SpineConnection *connection)
{
    //TODO: Implement a mutex with waiting so only one message can be processed at a time

    //response.clear();
    //connection->ship_connection->outgoing_json_doc.clear();
    BasicJsonDocument<ArduinoJsonPsramAllocator> response_doc{8182};
    JsonObject responseObj = response_doc.to<JsonObject>();
    // TODO: Fix the addressing of the usecases. Maybe better address them by entity?
    CmdClassifierType send_response = CmdClassifierType::EnumUndefined;
    String entity_name = "Unknown";

    bool found_dest_entity = false;
    for (EebusEntity *entity : entity_list) {
        if (header.addressDestination->entity.has_value() && entity->matches_entity_address(header.addressDestination->entity.get())) {
            found_dest_entity = true;
            eebus.trace_fmtln("Usecases: Found entity: %s", entity->get_entity_name().c_str());
            send_response = entity->handle_message(header, data, responseObj, connection);
            break;
        }
    }

    if (!found_dest_entity) {
        eebus.trace_fmtln("Usecases: Received message for unknown entity %d", entity_name.c_str());
        EEBUS_USECASE_HELPERS::build_result_data(responseObj,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected,
                                                 "Unknown entity requested");
        send_response = CmdClassifierType::reply; // We always send a response if we do not know the entity
    }
    if (send_response != CmdClassifierType::EnumUndefined) {
        eebus.trace_fmtln("Usecases: Sending response");
        if (header.ackRequest.has_value() && header.ackRequest.get() && send_response != CmdClassifierType::result) {

            eebus.trace_fmtln("Usecases: Header requested an ack, but sending a non-result response: %d", static_cast<int>(send_response));
        }
        connection->send_datagram(response_doc, send_response, *header.addressSource, *header.addressDestination, false);
    } else {
        if (header.ackRequest.has_value() && header.ackRequest.get()) {
            eebus.trace_fmtln("Usecases: Header requested an ack, but no response was generated");
        }
        eebus.trace_fmtln("Usecases: No response needed. Not sending anything");
    }
}

void EEBusUseCases::inform_subscribers(int entity, int feature, SpineDataTypeHandler *data)
{
    node_management.inform_subscribers(entity, feature, data);
}

namespace EEBUS_USECASE_HELPERS
{
const char *get_spine_device_name()
{
    // This returns the device name as defined in EEBUS SPINE TS ProtocolSpecification
    return ("d:_n:" + eebus.get_eebus_name()).c_str();
}

void build_result_data(JsonObject &response, ResultErrorNumber error_number, const char *description)
{
    ResultDataType result{};
    result.description = description;
    result.errorNumber = static_cast<uint8_t>(error_number);
    response["result"] = result;
}
} // namespace EEBUS_USECASE_HELPERS