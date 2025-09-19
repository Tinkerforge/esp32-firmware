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

#include "eebus.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include <chrono>
#include <regex>


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
    feature.description->featureAddress->feature = 0; // Feature 0 is the NodeManagement feature
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

size_t NodeManagementEntity::inform_subscribers(const std::vector<AddressEntityType> &entity, const AddressFeatureType feature, SpineDataTypeHandler *data)
{
    size_t sent_count = 0;
    BasicJsonDocument<ArduinoJsonPsramAllocator> response(SPINE_CONNECTION_MAX_JSON_SIZE);
    JsonVariant dst = response.createNestedObject(data->function_to_string(data->last_cmd));
    data->last_cmd_to_json(dst);
    for (SubscriptionManagementEntryDataType &subscription : subscription_data.subscriptionEntry.get()) {
        if (subscription.serverAddress->entity == entity && subscription.serverAddress->feature == feature) {
            eebus.usecases->send_spine_message(subscription.clientAddress.get(), subscription.serverAddress.get(), response.as<JsonObject>(), CmdClassifierType::notify, false);
            sent_count++;
        }
    }
    data->reset();
    return sent_count;
};

void NodeManagementEntity::set_usecaseManager(EEBusUseCases *new_usecase_interface)
{
    usecase_interface = new_usecase_interface;
}


EvseEntity::EvseEntity()
{
    update_billing_data(1, 0_s, 0_s, 0, 0, 100, 100, 0, 0);
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
    evcs_usecase_feature_address.entity = entity_address;
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

void EvseEntity::update_billing_data(int id, seconds_t start_time, seconds_t end_time, int energy_wh, uint32_t cost_eur_cent, int grid_energy_percent, int grid_cost_percent, int self_produced_energy_percent, int self_produced_cost_percent)
{

    BillDataType billData{};
    billData.billId = id;
    billData.billType = BillTypeEnumType::chargingSummary;
    billData.total->timePeriod->startTime = std::to_string(start_time.as<int>());
    billData.total->timePeriod->endTime = std::to_string(end_time.as<int>());
    BillValueType total_value;
    total_value.value->number = energy_wh;
    total_value.unit = UnitOfMeasurementEnumType::Wh;
    total_value.value->scale = 0; // The total value is calculated like numer * 10^(scale).
    BillCostType total_cost;
    total_cost.costType = BillCostTypeEnumType::absolutePrice;
    total_cost.cost->number = cost_eur_cent;
    total_cost.currency = CurrencyEnumType::EUR;
    total_cost.cost->scale = -2; // We send the cost in cents, so
    billData.total->value->push_back(total_value);
    billData.total->cost->push_back(total_cost);

    BillPositionType grid_position;
    grid_position.positionId = 1;
    grid_position.positionType = BillPositionTypeEnumType::gridElectricEnergy;
    BillValueType grid_value;
    grid_value.valuePercentage->number = grid_energy_percent;
    grid_value.valuePercentage->scale = 0;
    BillCostType grid_cost;
    grid_cost.costPercentage->number = grid_cost_percent;
    grid_position.cost->push_back(grid_cost);
    grid_position.value->push_back(grid_value);

    BillPositionType self_produced_position;
    self_produced_position.positionId = 2;
    self_produced_position.positionType = BillPositionTypeEnumType::selfProducedElectricEnergy;
    BillValueType self_produced_value;
    self_produced_value.valuePercentage->number = self_produced_energy_percent;
    self_produced_value.valuePercentage->scale = 0;
    BillCostType self_produced_cost;
    self_produced_cost.costPercentage->number = self_produced_energy_percent;
    grid_position.cost->push_back(grid_cost);
    grid_position.value->push_back(grid_value);

    billData.position->push_back(grid_position);
    billData.position->push_back(self_produced_position);

    for (int i = 0; i < bill_list_data.billData->size(); i++) {
        if (bill_list_data.billData->at(i).billId == id) {
            bill_list_data.billData->at(i) = billData;
            return;
        }
    }
    bill_list_data.billData->push_back(billData);

    //eebus.data_handler->billlistdatatype = bill_list_data;
    //eebus.data_handler->last_cmd = SpineDataTypeHandler::Function::billListData;
    //eebus.usecases->node_management.inform_subscribers(this->entity_address, this->bill_feature_address, eebus.data_handler.get());
}


CmdClassifierType EvseEntity::bill_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    //TODO: Support partial reads, writes and deletes
    if (data->last_cmd == SpineDataTypeHandler::Function::billDescriptionListData) {
        // EEBUS_UC_TS_EVCHargingSummary_v1.0.1.pdf 3.2.1.2.2.1 Function "billDescriptionListData"
        BillDescriptionListDataType billDescriptionListData{};
        billDescriptionListData.billDescriptionData.emplace();
        for (BillDataType bill_entry : bill_list_data.billData.get()) {
            BillDescriptionDataType billDescriptionData{};
            billDescriptionData.billWriteable = false; // No bill is writeable for now
            billDescriptionData.billId = bill_entry.billId;
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
        for (BillDataType bill_entry : bill_list_data.billData.get()) {
            BillConstraintsDataType billConstraintsData{};
            billConstraintsData.billId = bill_entry.billId;
            billConstraintsData.positionCountMin = "0";
            billConstraintsData.positionCountMax = std::to_string(bill_entry.position->size());
            billConstraintsListData.billConstraintsData->push_back(billConstraintsData);
        }
        response["billConstraintsListData"] = billConstraintsListData;
        return CmdClassifierType::reply;

    }
    if (data->last_cmd == SpineDataTypeHandler::Function::billListData) {
        // EEBUS_UC_TS_EVCHargingSummary_v1.0.1.pdf 3.2.1.2.2.3 Function "billListData"
        response["billListData"] = bill_list_data;
        return CmdClassifierType::reply;
    }
    return CmdClassifierType::EnumUndefined;
}

ControllableSystemEntity::ControllableSystemEntity()
{
    // Initialize DeviceConfiguration feature
    update_failsafe(EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION, 0_s);

    // Initialize DeviceDiagnosis feature
    task_scheduler.scheduleWithFixedDelay([this]() {
                                              DeviceDiagnosisHeartbeatDataType outgoing_heartbeatData{};
                                              outgoing_heartbeatData.heartbeatCounter = heartbeatCounter;
                                              outgoing_heartbeatData.heartbeatTimeout = EEBUS_USECASE_HELPERS::iso_duration_to_string(60_s);
                                              outgoing_heartbeatData.timestamp = std::to_string(rtc.timestamp_minutes() * 60);

                                              eebus.data_handler->devicediagnosisheartbeatdatatype = outgoing_heartbeatData;
                                              eebus.data_handler->last_cmd = SpineDataTypeHandler::Function::deviceDiagnosisHeartbeatData;
                                              if (eebus.usecases->inform_subscribers(this->entity_address, this->deviceDiagnosis_feature_address, eebus.data_handler.get()) > 0) {
                                                  heartbeatCounter++;
                                              }
                                              if (!heartbeat_received) {
                                                  handle_heartbeat_timeout();
                                              }
                                              heartbeat_received = false;
                                          },
                                          120_s,
                                          60_s);
    // Initialize ElectricalConnection feature
    update_constraints(EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION, EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION);
    update_lpc(false, EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION, 0);
    lpc_state = LPCState::Init;
    switch_state(LPCState::Init);

}

UseCaseInformationDataType ControllableSystemEntity::get_usecase_information()
{
    UseCaseInformationDataType lpc_usecase;
    lpc_usecase.actor = "Controllable System";

    UseCaseSupportType lpc_usecase_support;
    lpc_usecase_support.useCaseName = "limitationOfPowerConsumption";
    lpc_usecase_support.useCaseVersion = "1.0.0";
    lpc_usecase_support.useCaseAvailable = true;
    lpc_usecase_support.scenarioSupport->push_back(1);
    lpc_usecase_support.scenarioSupport->push_back(2);
    lpc_usecase_support.scenarioSupport->push_back(3);
    lpc_usecase_support.scenarioSupport->push_back(4);
    lpc_usecase_support.useCaseDocumentSubRevision = "release";
    lpc_usecase.useCaseSupport->push_back(lpc_usecase_support);

    FeatureAddressType lpc_usecase_feature_address;
    lpc_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    lpc_usecase_feature_address.entity = entity_address;
    lpc_usecase_feature_address.feature = 1;
    lpc_usecase.address = lpc_usecase_feature_address;
    return lpc_usecase;
}

CmdClassifierType ControllableSystemEntity::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.addressDestination->feature.has_value() && header.addressDestination->feature == loadControl_feature_address) {
        return load_control_feature(header, data, response, connection);
    }
    if (header.addressDestination->feature.has_value() && header.addressDestination->feature == deviceConfiguration_feature_address) {
        return deviceConfiguration_feature(header, data, response, connection);
    }
    if (header.addressDestination->feature.has_value() && header.addressDestination->feature == deviceDiagnosis_feature_address) {
        return device_diagnosis_feature(header, data, response, connection);
    }
    if (header.addressDestination->feature.has_value() && header.addressDestination->feature == electricalConnection_feature_address) {
        return electricalConnection_feature(header, data, response, connection);
    }

    return CmdClassifierType::EnumUndefined;
}

NodeManagementDetailedDiscoveryEntityInformationType ControllableSystemEntity::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EVSE;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "Controllable System"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> ControllableSystemEntity::get_detailed_discovery_feature_information() const
{

    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    // See EEBUS UC TS LimitationOfPowerConsum ption v1.0.0.pdf 3.2.2.2.1

    // The following functions are needed by the LoadControl Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType loadControlFeature{};
    loadControlFeature.description->featureAddress->entity = entity_address;
    loadControlFeature.description->featureAddress->feature = loadControl_feature_address; // Feature IDs are just arbitrary numbers. Just have to be unique within the entity
    loadControlFeature.description->featureType = FeatureTypeEnumType::LoadControl;
    loadControlFeature.description->role = RoleType::server;

    // loadControlLimitDescriptionListData
    FunctionPropertyType loadControlDescriptionList{};
    loadControlDescriptionList.function = FunctionEnumType::loadControlLimitConstraintsListData;
    loadControlDescriptionList.possibleOperations->read = PossibleOperationsReadType{};
    loadControlFeature.description->supportedFunction->push_back(loadControlDescriptionList);

    // loadControlLimitListData
    FunctionPropertyType loadControlLimitListData{};
    loadControlLimitListData.function = FunctionEnumType::loadControlLimitListData;
    loadControlLimitListData.possibleOperations->read = PossibleOperationsReadType{};
    loadControlLimitListData.possibleOperations->write = PossibleOperationsWriteType{};

    loadControlFeature.description->supportedFunction->push_back(loadControlLimitListData);
    features.push_back(loadControlFeature);

    // The following functions are needed by the DeviceConfiguration Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType deviceConfigurationFeature{};
    deviceConfigurationFeature.description->featureAddress->entity = entity_address;
    deviceConfigurationFeature.description->featureAddress->feature = deviceConfiguration_feature_address;
    deviceConfigurationFeature.description->featureType = FeatureTypeEnumType::DeviceConfiguration;
    deviceConfigurationFeature.description->role = RoleType::server;

    //deviceConfigurationKeyValueDescriptionListData
    FunctionPropertyType deviceConfigurationKeyValueDescriptionListData{};
    deviceConfigurationKeyValueDescriptionListData.function = FunctionEnumType::deviceConfigurationKeyValueDescriptionListData;
    deviceConfigurationKeyValueDescriptionListData.possibleOperations->read = PossibleOperationsReadType{};
    deviceConfigurationFeature.description->supportedFunction->push_back(deviceConfigurationKeyValueDescriptionListData);

    //deviceConfigurationKeyValueListData
    FunctionPropertyType deviceConfigurationKeyValueListData{};
    deviceConfigurationKeyValueListData.function = FunctionEnumType::deviceConfigurationKeyValueListData;
    deviceConfigurationKeyValueListData.possibleOperations->read = PossibleOperationsReadType{};
    deviceConfigurationFeature.description->supportedFunction->push_back(deviceConfigurationKeyValueListData);
    features.push_back(deviceConfigurationFeature);

    // The following functions are needed by the DeviceDiagnosis Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType deviceDiagnosisFeature{};
    deviceDiagnosisFeature.description->featureAddress->entity = entity_address;
    deviceDiagnosisFeature.description->featureAddress->feature = deviceDiagnosis_feature_address;
    deviceDiagnosisFeature.description->featureType = FeatureTypeEnumType::DeviceDiagnosis;
    deviceDiagnosisFeature.description->role = RoleType::server;

    //deviceDiagnosisHeartBeatData
    FunctionPropertyType deviceDiagnosisHeartBeatData{};
    deviceDiagnosisHeartBeatData.function = FunctionEnumType::deviceDiagnosisHeartbeatData;
    deviceDiagnosisHeartBeatData.possibleOperations->read = PossibleOperationsReadType{};
    deviceDiagnosisFeature.description->supportedFunction->push_back(deviceDiagnosisHeartBeatData);
    features.push_back(deviceDiagnosisFeature);

    // The following functions are needed by the ElectricalConnection Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnectionFeature{};
    electricalConnectionFeature.description->featureAddress->entity = entity_address;
    electricalConnectionFeature.description->featureAddress->feature = electricalConnection_feature_address;
    electricalConnectionFeature.description->featureType = FeatureTypeEnumType::ElectricalConnection;
    electricalConnectionFeature.description->role = RoleType::server;

    //electricalConnectionCharacteristicsListData
    FunctionPropertyType electricalConnectionCharacteristicsListData{};
    electricalConnectionCharacteristicsListData.function = FunctionEnumType::electricalConnectionCharacteristicListData;
    electricalConnectionCharacteristicsListData.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionCharacteristicsListData);
    features.push_back(electricalConnectionFeature);

    return features;
}

void ControllableSystemEntity::update_failsafe(int power_limit_w, seconds_t duration)
{
    device_configuration_key_value_description_list.deviceConfigurationKeyValueDescriptionData.emplace();
    device_configuration_key_value_description_list.deviceConfigurationKeyValueDescriptionData->clear();
    device_configuration_key_value_list.deviceConfigurationKeyValueData.emplace();
    device_configuration_key_value_list.deviceConfigurationKeyValueData->clear();

    DeviceConfigurationKeyValueDescriptionDataType failsafeConsumptionActivePowerLimitDescription{};
    DeviceConfigurationKeyValueDataType failsafeConsumptionActivePowerLimit{};
    failsafeConsumptionActivePowerLimitDescription.keyId = failsafeConsumptionActivePowerLimit.keyId = 1;
    failsafeConsumptionActivePowerLimitDescription.keyName = DeviceConfigurationKeyNameEnumType::failsafeConsumptionActivePowerLimit;
    failsafeConsumptionActivePowerLimitDescription.unit = UnitOfMeasurementEnumType::W;
    failsafeConsumptionActivePowerLimitDescription.valueType = DeviceConfigurationKeyValueTypeType::scaledNumber;
    failsafeConsumptionActivePowerLimit.isValueChangeable = true;
    failsafeConsumptionActivePowerLimit.value->scaledNumber.emplace();
    failsafeConsumptionActivePowerLimit.value->scaledNumber->number = power_limit_w;
    failsafeConsumptionActivePowerLimit.value->scaledNumber->scale = 0;

    DeviceConfigurationKeyValueDescriptionDataType failsafeDurationMinimumDescription{};
    DeviceConfigurationKeyValueDataType failsafeDurationMinimum{};
    failsafeDurationMinimumDescription.keyId = failsafeDurationMinimum.keyId = 2;
    failsafeDurationMinimumDescription.keyName = DeviceConfigurationKeyNameEnumType::failsafeDurationMinimum;
    failsafeDurationMinimumDescription.valueType = DeviceConfigurationKeyValueTypeType::duration;
    failsafeDurationMinimum.isValueChangeable = true;
    failsafeDurationMinimum.value->duration = EEBUS_USECASE_HELPERS::iso_duration_to_string(duration);

    device_configuration_key_value_description_list.deviceConfigurationKeyValueDescriptionData->push_back(failsafeConsumptionActivePowerLimitDescription);
    device_configuration_key_value_description_list.deviceConfigurationKeyValueDescriptionData->push_back(failsafeDurationMinimumDescription);
    device_configuration_key_value_list.deviceConfigurationKeyValueData->push_back(failsafeConsumptionActivePowerLimit);
    device_configuration_key_value_list.deviceConfigurationKeyValueData->push_back(failsafeDurationMinimum);
    // TODO: Inform Subscribers
}

void ControllableSystemEntity::update_constraints(int power_consumption_max_w, int power_consumption_contract_max_w)
{
    electrical_connection_characteristic_list.electricalConnectionCharacteristicData->clear();

    ElectricalConnectionCharacteristicDataType power_consumption_max{};
    power_consumption_max.electricalConnectionId = 1;
    power_consumption_max.characteristicId = 1;
    power_consumption_max.characteristicContext = ElectricalConnectionCharacteristicContextEnumType::entity;
    power_consumption_max.characteristicType = ElectricalConnectionCharacteristicTypeEnumType::powerConsumptionMax;
    power_consumption_max.value->number = power_consumption_max_w;
    power_consumption_max.unit = UnitOfMeasurementEnumType::W;
    electrical_connection_characteristic_list.electricalConnectionCharacteristicData->push_back(power_consumption_max);

    // TODO: Inform Subscribers

    // As stated in EEBUS_UC_TS_LimitationOfPowerConsumption_v1.0.0.pdf 2.6.4.1, the contractual consumption is not something the CS is supposed to handle
    /*
    ElectricalConnectionCharacteristicDataType contractual_power_consumption_max{};
    contractual_power_consumption_max.electricalConnectionId = 1;
    contractual_power_consumption_max.parameterId = 1;
    contractual_power_consumption_max.characteristicId = 2;
    contractual_power_consumption_max.characteristicContext = ElectricalConnectionCharacteristicContextEnumType::entity;
    contractual_power_consumption_max.characteristicType = ElectricalConnectionCharacteristicTypeEnumType::contractualConsumptionNominalMax;
    contractual_power_consumption_max.value->number = power_consumption_contract_max_w;
    contractual_power_consumption_max.unit = UnitOfMeasurementEnumType::W;
    electrical_connection_characteristic_list.electricalConnectionCharacteristicData->push_back(contractual_power_consumption_max);*/

}

bool ControllableSystemEntity::update_lpc(bool limit_active, int current_limit_w, uint64_t endtime)
{

    load_control_limit_description_list.loadControlLimitDescriptionData.reset();
    LoadControlLimitDescriptionDataType limit_description{};
    limit_description.limitId = 1;
    limit_description.limitType = LoadControlLimitTypeEnumType::signDependentAbsValueLimit;
    limit_description.limitCategory = LoadControlCategoryEnumType::obligation;
    limit_description.limitDirection = EnergyDirectionEnumType::consume;
    limit_description.measurementId = 1;
    limit_description.unit = UnitOfMeasurementEnumType::W;
    limit_description.scopeType = ScopeTypeEnumType::activePowerLimit;
    load_control_limit_description_list.loadControlLimitDescriptionData->push_back(limit_description);

    // Evaluate if the limit can be applied according to EEBUS_UC_TS_LimitationOfPowerConsumption_v1.0.0.pdf 2.2 Line 311
    bool limit_accepted = true;
    // A limit lower than 0W shall be rejected
    if (current_limit_w < 0) {
        limit_accepted = false;
    } else {
        // TODO: check if the limit can be applied
        // The limit shall apply the limit unless the rejection of the limit is required by: Self-protection, safety related activities, legal or regulatory specifications

        // A limit MAY be larger than the devices possible maximum consumption. If this limit too large to be stored, the System may alter the value to the highest possible value.

        if (current_limit_w > EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION) {
            current_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
        }
    }
    LoadControlLimitDataType old_limit{};
    load_control_limit_list.loadControlLimitData.emplace();

    LoadControlLimitDataType limit_data{};
    limit_data.limitId = 1;
    limit_data.isLimitChangeable = true;
    limit_data.isLimitActive = limit_active && limit_accepted;
    if (endtime > 0) {
        limit_data.timePeriod->endTime = std::to_string(endtime);
    } else if (old_limit.timePeriod.has_value()) {
        // A value of 0 means the limit is valid until further notice. see 3.4.1.4
        limit_data.timePeriod = old_limit.timePeriod;
    } else {
        limit_data.timePeriod.reset();
    }
    limit_data.value->number = current_limit_w;
    limit_data.value->scale = 0;
    load_control_limit_list.loadControlLimitData->push_back(limit_data);

    if (limit_accepted && heartbeat_received && limit_active) {
        // LPC-902, LPC-904
        switch_state(LPCState::Limited);
    } else if (heartbeat_received && !limit_active) {
        //LPC-905
        switch_state(LPCState::UnlimitedControlled);
    } else {
        // LPC-906
        switch_state(LPCState::UnlimitedControlled);
    }
    //eebus.data_handler->loadcontrollimitdescriptionlistdatatype = load_control_limit_description_list;
    //eebus.data_handler->last_cmd = SpineDataTypeHandler::Function::loadControlLimitDescriptionListData;
    //eebus.usecases->node_management.inform_subscribers(entity_address, loadControl_feature_address, eebus.data_handler.get());

    //eebus.data_handler->loadcontrollimitlistdatatype = load_control_limit_list;
    //eebus.data_handler->last_cmd = SpineDataTypeHandler::Function::loadControlLimitListData;
    //eebus.usecases->node_management.inform_subscribers(entity_address, loadControl_feature_address, eebus.data_handler.get());

    return limit_accepted;
}

CmdClassifierType ControllableSystemEntity::load_control_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.cmdClassifier == CmdClassifierType::read) {
        if (data->last_cmd == SpineDataTypeHandler::Function::loadControlLimitDescriptionListData) {
            response["loadControlLimitConstraintsListData"] = load_control_limit_description_list;
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::loadControlLimitListData) {
            response["loadControlLimitConstraintsList"] = load_control_limit_list;
            return CmdClassifierType::reply;
        }
    }
    if (header.cmdClassifier == CmdClassifierType::write) {
        FeatureAddressType feature_address{};
        feature_address.entity = entity_address;
        feature_address.feature = loadControl_feature_address;
        feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();

        if (eebus.usecases->node_management.check_is_bound(header.addressSource.get(), feature_address) && data->last_cmd == SpineDataTypeHandler::Function::loadControlLimitData && data->loadcontrollimitdatatype.has_value()) {
            LoadControlLimitDataType load_control_limit_data = data->loadcontrollimitdatatype.get();
            if (!update_lpc(load_control_limit_data.isLimitActive.get(), load_control_limit_data.value->number.get() * 10 * *(load_control_limit_data.value->scale), load_control_limit_data.timePeriod->endTime.has_value() ? std::stoull(load_control_limit_data.timePeriod->endTime.get()) : 0)) {
                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Limit not accepted");
            }
            return CmdClassifierType::result;

        }
        EEBUS_USECASE_HELPERS::build_result_data(response,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::BindingRequired,
                                                 "Load Control requires binding");
        return CmdClassifierType::result;
    }
    return CmdClassifierType::EnumUndefined;
}

CmdClassifierType ControllableSystemEntity::deviceConfiguration_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.cmdClassifier == CmdClassifierType::read) {
        if (data->last_cmd == SpineDataTypeHandler::Function::deviceConfigurationKeyValueDescriptionData) {
            response["deviceConfigurationKeyValueDescriptionListData"] = device_configuration_key_value_description_list;
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::deviceConfigurationKeyValueListData) {
            response["deviceConfigurationKeyValueListData"] = device_configuration_key_value_list;
            return CmdClassifierType::reply;
        }
    }
    if (header.cmdClassifier == CmdClassifierType::write && data->deviceconfigurationkeyvaluelistdatatype.has_value() && data->last_cmd == SpineDataTypeHandler::Function::deviceConfigurationKeyValueListData) {
        // We only accept writes from nodes we are bound to and we only do full writes
        if (eebus.usecases->node_management.check_is_bound(header.addressSource.get(), header.addressDestination.get())) {
            device_configuration_key_value_list = data->deviceconfigurationkeyvaluelistdatatype.get();
            EEBUS_USECASE_HELPERS::build_result_data(response,
                                                     EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError,
                                                     "Configuration updated successfully");
            update_api();
            return CmdClassifierType::result;
        }
    }
    return CmdClassifierType::EnumUndefined;
}

CmdClassifierType ControllableSystemEntity::device_diagnosis_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (data->last_cmd == SpineDataTypeHandler::Function::deviceDiagnosisHeartbeatData && data->devicediagnosisheartbeatdatatype.has_value()) {
        DeviceDiagnosisHeartbeatDataType incoming_heartbeatData = data->devicediagnosisheartbeatdatatype.get();
        if (header.cmdClassifier == CmdClassifierType::read) {

            // Prepare our own hearbeat information
            DeviceDiagnosisHeartbeatDataType outgoing_heartbeatData{};
            outgoing_heartbeatData.heartbeatCounter = heartbeatCounter;
            outgoing_heartbeatData.heartbeatTimeout = EEBUS_USECASE_HELPERS::iso_duration_to_string(60_s);
            outgoing_heartbeatData.timestamp = std::to_string(rtc.timestamp_minutes());
            response["deviceDiagnosisHeartbeatData"] = outgoing_heartbeatData;

            // Initialize  read on their heartbeat
            DeviceDiagnosisHeartbeatDataType read_heartbeat{};
            DynamicJsonDocument doc{256};
            JsonObject obj = doc.to<JsonObject>();
            obj["deviceDiagnosisHeartbeatData"] = read_heartbeat;
            FeatureAddressType read_destination = header.addressSource.get();
            task_scheduler.scheduleOnce([this, read_destination, obj]() {
                                            FeatureAddressType read_source{};
                                            read_source.feature = deviceDiagnosis_feature_address;
                                            read_source.entity = entity_address;
                                            eebus.usecases->send_spine_message(read_destination, read_source, obj, CmdClassifierType::read, false);
                                        },
                                        100_ms);
            if (heartbeat_timeout_task) {
                heartbeat_timeout_task = task_scheduler.scheduleOnce(
                    [this]() {
                        this->handle_heartbeat_timeout();
                    },
                    60_s);

            }
            // If we get a read from someone we just try to subscribe to them. They can then deny or accept it. Should be able to handle double requests.
            task_scheduler.scheduleOnce([this, header, obj]() {
                                            NodeManagementSubscriptionRequestCallType subscription_request_call{};
                                            subscription_request_call.subscriptionRequest->clientAddress->device = EEBUS_USECASE_HELPERS::get_spine_device_name();
                                            subscription_request_call.subscriptionRequest->clientAddress->entity = entity_address;
                                            subscription_request_call.subscriptionRequest->clientAddress->feature = deviceDiagnosis_feature_address;

                                            subscription_request_call.subscriptionRequest->serverAddress = header.addressSource.get();
                                            subscription_request_call.subscriptionRequest->serverFeatureType = FeatureTypeEnumType::DeviceDiagnosis;

                                            FeatureAddressType subscription_request_destination = header.addressDestination.get();
                                            subscription_request_destination.entity = {0};
                                            subscription_request_destination.feature = 0;
                                            FeatureAddressType subscription_request_source = subscription_request_call.subscriptionRequest->clientAddress.get();

                                            BasicJsonDocument<ArduinoJsonPsramAllocator> subscription_request_doc(2048);
                                            JsonObject sub_obj = subscription_request_doc.to<JsonObject>();
                                            obj["subscriptionRequestCall"] = subscription_request_call;
                                            eebus.usecases->send_spine_message(subscription_request_destination, subscription_request_source, sub_obj, CmdClassifierType::call);
                                        },
                                        200_ms);
            heartbeatEnabled = true;
            heartbeat_received = true;
            return CmdClassifierType::reply;
        }
        if (header.cmdClassifier == CmdClassifierType::reply || header.cmdClassifier == CmdClassifierType::notify) {
            // Just reset timeout here. Resetting on replies is not quite conform but its still possible
            heartbeat_received = true;
            task_scheduler.cancel(heartbeat_timeout_task);
            heartbeat_timeout_task = task_scheduler.scheduleOnce(
                [this]() {
                    this->handle_heartbeat_timeout();
                },
                60_s);
        }

    }

    return CmdClassifierType::EnumUndefined;
}

CmdClassifierType ControllableSystemEntity::electricalConnection_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.cmdClassifier == CmdClassifierType::read && data->last_cmd == SpineDataTypeHandler::Function::electricalConnectionCharacteristicListData) {
        response["electricalConnectionCharacteristicListData"] = electrical_connection_characteristic_list;
        return CmdClassifierType::reply;
    }
    return CmdClassifierType::EnumUndefined;
}

bool ControllableSystemEntity::switch_state(LPCState state)
{
    bool state_chaged = false;
    switch (state) {
        case LPCState::Init:
            state_chaged = init_state();
            break;
        case LPCState::UnlimitedControlled:
            state_chaged = unlimited_controlled_state();
            break;
        case LPCState::Limited:
            state_chaged = limited_state();
            break;
        case LPCState::Failsafe:
            state_chaged = failsafe_state();
            break;
        case LPCState::UnlimitedAutonomous:
            state_chaged = unlimited_autonomous_state();
            break;
    }
    if (state_chaged) {
        update_api();
    } else {
        eebus.trace_fmtln("Usecases: LPC: Illegal state change request. From %d to %d", lpc_state, state);

    }
    return state_chaged;
}

bool ControllableSystemEntity::init_state()
{
    task_scheduler.cancel(state_change_timeout);
    state_change_timeout = task_scheduler.scheduleOnce(
        [this]() {
            this->switch_state(LPCState::UnlimitedAutonomous);
        },
        120_s);
    limit_engaged = false;
    current_active_consumption_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    return true;

}

bool ControllableSystemEntity::unlimited_controlled_state()
{
    if (lpc_state == LPCState::Init || lpc_state == LPCState::Limited || lpc_state == LPCState::Failsafe || lpc_state == LPCState::UnlimitedAutonomous) {
        lpc_state = LPCState::UnlimitedControlled;
        task_scheduler.cancel(state_change_timeout);
        limit_engaged = false;
        current_active_consumption_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
        return true;
    }

    return false;
}

bool ControllableSystemEntity::limited_state()
{
    if (lpc_state == LPCState::Init || lpc_state == LPCState::UnlimitedControlled || lpc_state == LPCState::UnlimitedAutonomous || lpc_state == LPCState::Failsafe) {
        lpc_state = LPCState::Limited;
        task_scheduler.cancel(state_change_timeout);
        current_active_consumption_limit_w = load_control_limit_list.loadControlLimitData->at(0).value->number.get();
        return true;
    }

    return false;
}

bool ControllableSystemEntity::failsafe_state()
{
    if (lpc_state == LPCState::UnlimitedControlled || lpc_state == LPCState::Limited) {
        lpc_state = LPCState::Failsafe;
        task_scheduler.cancel(state_change_timeout);
        current_active_consumption_limit_w = device_configuration_key_value_list.deviceConfigurationKeyValueData->at(0).value->scaledNumber->number.get();
        seconds_t failsafe_duration = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(device_configuration_key_value_list.deviceConfigurationKeyValueData->at(1).value->duration.get());
        state_change_timeout = task_scheduler.scheduleOnce(
            [this]() {
                this->switch_state(LPCState::UnlimitedAutonomous);
            },
            failsafe_duration);
        return true;
    }
    return false;
}

bool ControllableSystemEntity::unlimited_autonomous_state()
{
    if (lpc_state == LPCState::Init || lpc_state == LPCState::Failsafe) {
        lpc_state = LPCState::UnlimitedAutonomous;
        task_scheduler.cancel(state_change_timeout);
        current_active_consumption_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
        return true;
    }
    eebus.trace_fmtln("Usecases: LPC: Illegal state change request. From %d to %d", lpc_state, LPCState::UnlimitedControlled);
    return false;
}

void ControllableSystemEntity::update_api()
{
    auto api_entry = eebus.eebus_usecase_state.get("power_consumption_limitation");
    api_entry->get("usecase_state")->updateEnum(lpc_state);
    api_entry->get("limit_active")->updateBool(limit_engaged);
    api_entry->get("current_limit")->updateUint(current_active_consumption_limit_w);
    api_entry->get("failsafe_limit_power_w")->updateUint(device_configuration_key_value_list.deviceConfigurationKeyValueData->at(0).value->scaledNumber->number.get());
    api_entry->get("failsafe_limit_duration_s")->updateUint(EEBUS_USECASE_HELPERS::iso_duration_to_seconds(device_configuration_key_value_list.deviceConfigurationKeyValueData->at(1).value->duration.get()).as<uint64_t>());
    api_entry->get("constraints_power_maximum")->updateUint(electrical_connection_characteristic_list.electricalConnectionCharacteristicData->at(0).value->number.get());
    //api_entry->get("constraints_power_maximum_contractual")->updateUint(electrical_connection_characteristic_list.electricalConnectionCharacteristicData->at(1).value->number.get());

}

void ControllableSystemEntity::handle_heartbeat_timeout()
{
    if (heartbeat_received)
        return;
    eebus.trace_fmtln("Usecase: LPC: Heartbeat Timeout");
    if (lpc_state != LPCState::Failsafe)
        switch_state(LPCState::Failsafe);
}

EEBusUseCases::EEBusUseCases()
{
    // Entity Addresses have to be unique
    node_management = NodeManagementEntity();
    node_management.set_usecaseManager(this);
    node_management.set_entity_address({0});
    charging_summary = EvseEntity();
    charging_summary.set_entity_address({1});
    limitation_of_power_consumption = ControllableSystemEntity();
    limitation_of_power_consumption.set_entity_address({2});

}

void EEBusUseCases::handle_message(HeaderType &header, SpineDataTypeHandler *data, SpineConnection *connection)
{
    //TODO: Implement a mutex with waiting so only one message can be processed at a time

    //response.clear();
    //connection->ship_connection->outgoing_json_doc.clear();
    BasicJsonDocument<ArduinoJsonPsramAllocator> response_doc{8182};
    JsonObject responseObj = response_doc.to<JsonObject>();
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
    data->reset();
}

size_t EEBusUseCases::inform_subscribers(const std::vector<AddressEntityType> &entity, const AddressFeatureType feature, SpineDataTypeHandler *data)
{
    return node_management.inform_subscribers(entity, feature, data);
}

bool EEBusUseCases::send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, const JsonVariantConst payload, CmdClassifierType cmd_classifier, const bool want_ack)
{
    if (sender.feature.isNull() || sender.entity.isNull()) {
        eebus.trace_fmtln("Usecases: Cannot send spine message, sender entity or feature is null");
        return false;
    }
    if (sender.device.isNull()) {
        sender.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    }
    bool message_sent = false;
    for (auto &ship_connection : eebus.ship.ship_connections) {
        if (ship_connection && ship_connection->spine->check_known_address(destination)) {
            ship_connection->spine->send_datagram(payload, cmd_classifier, sender, destination, want_ack);
            message_sent = true;
        }
    }
    return message_sent;
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

seconds_t iso_duration_to_seconds(std::string iso_duration)
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
    std::sregex_iterator second_end;
    for (; second_it != second_end; ++second_it) {
        std::string match = second_it->str();
        int value = std::stoi(match.substr(0, match.size() - 1));
        char unit = match.back();
        if (unit == 'H') {
            duration_seconds += value * 3600; // Approximate, not accounting for leap years
        } else if (unit == 'M') {
            duration_seconds += value * 60; // Approximate, assuming 30 days in a month
        } else if (unit == 'D') {
            duration_seconds += value;
        }
    }
    return seconds_t(duration_seconds);
}
} // namespace EEBUS_USECASE_HELPERS