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

bool NodeManagementUsecase::handle_binding(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
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
            std::optional<FeatureTypeType> feature_type = data->nodemanagementbindingrequestcalltype->bindingRequest->serverFeatureType;

            if (check_is_bound(binding_entry.clientAddress.value(), binding_entry.serverAddress.value())) {
                logger.printfln("A Binding was requested but is already bound");
            } else {
                binding_entry.bindingId = binding_management_entry_list_.bindingManagementEntryData->size();
                binding_management_entry_list_.bindingManagementEntryData->push_back(binding_entry);
            }
            EEBUS_USECASE_HELPERS::build_result_data(response,
                                                     EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError,
                                                     "Binding request was successful");
            return true;
        }
        logger.printfln("A Binding was requested but failed");
        EEBUS_USECASE_HELPERS::build_result_data(response,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected,
                                                 "Binding request failed");
        return true;
    }
    // Binding Data as defined in EEBus SPINE TS ProtocolSpecification 7.3.3
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingData) {
        NodeManagementBindingDataType binding_data;
        binding_data.bindingEntry = binding_management_entry_list_.bindingManagementEntryData;
        response["nodeManagementBindingData"] = binding_data;
        logger.printfln("List of bindings was requested");
        return true;
    }
    // Binding Release as defined in EEBus SPINE TS ProtocolSpecification 7.3.4
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingDeleteCall) {
        if (!data->nodemanagementbindingdeletecalltype && data->nodemanagementbindingdeletecalltype->bindingDelete
            && data->nodemanagementbindingdeletecalltype->bindingDelete->clientAddress
            && data->nodemanagementbindingdeletecalltype->bindingDelete->serverAddress) {
            logger.printfln("A binding release was requested but no binding delete information was provided or request was malformed");
            EEBUS_USECASE_HELPERS::build_result_data(response,
                                                     EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected,
                                                     "Binding release failed");
            return true;
        };

        // Compares two optionals. If the one has no value, its considered a wildcard and matches anything in the second. If both have value, they are compared and the result returnd
        auto optional_equal_or_undefined = [](const auto &a, const auto &b) {
            if (!a.has_value() || !b.has_value())
                return true;
            return *a == *b;
        };
        NodeManagementBindingDeleteCallType binding_delete_call = data->nodemanagementbindingdeletecalltype.value();
        std::vector<size_t> to_delete_indices{};

        // Iterate throught the list of bindings and find the ones that match the delete request
        for (size_t i = 0; i < binding_management_entry_list_.bindingManagementEntryData->size(); ++i) {
            BindingManagementEntryDataType entry = binding_management_entry_list_.bindingManagementEntryData->at(i);

            // We handle cases where the client or server address is not set which is allowed
            FeatureAddressType client_address = binding_delete_call.bindingDelete->clientAddress.value();
            FeatureAddressType server_address = binding_delete_call.bindingDelete->serverAddress.value();
            if (!binding_delete_call.bindingDelete->clientAddress->device && binding_delete_call.bindingDelete->serverAddress->device) {
                // This implies the client is referencing its own and the servers name as device names
                client_address.device = header.addressSource->device.value();
                server_address.device = header.addressDestination->device.value();
            } else if (binding_delete_call.bindingDelete->clientAddress->device
                       && !binding_delete_call.bindingDelete->serverAddress->device) {
                server_address.device = header.addressSource->device.value();
            } else if (!binding_delete_call.bindingDelete->clientAddress->device
                       && binding_delete_call.bindingDelete->serverAddress->device) {
                client_address.device = header.addressDestination->device.value();
            }
            // If the device does not match to the entry we skip it
            if (client_address.device != entry.clientAddress->device && server_address.device != entry.serverAddress->device) {
                continue;
            }

            // This handles all the cases.
            // If a value of client_address or server_address is empty it is considered a wildcard and anything matches, if it has a value it is compared
            if (optional_equal_or_undefined(client_address.entity, entry.clientAddress->entity)
                && optional_equal_or_undefined(server_address.entity, entry.serverAddress->entity)
                && optional_equal_or_undefined(client_address.feature, entry.clientAddress->feature)
                && optional_equal_or_undefined(server_address.feature, entry.serverAddress->feature)) {
                to_delete_indices.push_back(i);
                continue;
            }
        }

        // delete all the found entries
        std::sort(to_delete_indices.rbegin(), to_delete_indices.rend()); // sort descending
        for (size_t i : to_delete_indices) {
            if (i < binding_management_entry_list_.bindingManagementEntryData.value().size()) {
                // remove the element at i starting from the back. Always do it relative to the beginning so the indices stay valid
                binding_management_entry_list_.bindingManagementEntryData.value().erase(
                    binding_management_entry_list_.bindingManagementEntryData.value().begin() + i);
            }
        }
        EEBUS_USECASE_HELPERS::build_result_data(response,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError,
                                                 "Removed bindings successfully");
        return true;
    }

    return false;
}
bool NodeManagementUsecase::check_is_bound(FeatureAddressType &client, FeatureAddressType &server) const
{
    for (const BindingManagementEntryDataType &binding : binding_management_entry_list_.bindingManagementEntryData.value()) {
        if (binding.clientAddress && binding.serverAddress) {
            if (binding.clientAddress->device == client.device && binding.serverAddress->device == server.device
                && binding.clientAddress->entity == client.entity && binding.serverAddress->entity == server.entity
                && binding.clientAddress->feature == client.feature && binding.serverAddress->feature == server.feature) {
                return true; // The client is bound to the server
            }
        }
    }
    return false;
}
UseCaseInformationDataType NodeManagementUsecase::get_usecase_information()
{
    return UseCaseInformationDataType(); // This should never be used as the NodeManagementUsecase has no usecase information
}
bool NodeManagementUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.cmdClassifier == CmdClassifierType::read && data->last_cmd == SpineDataTypeHandler::Function::nodeManagementUseCaseData) {
        return read_usecase_data(header, data, response);
    }
    if (header.cmdClassifier == CmdClassifierType::read
        && data->last_cmd == SpineDataTypeHandler::Function::nodeManagementDetailedDiscoveryData) {
        return read_detailed_discovery_data(header, data, response);
    }
    return false;
}

bool NodeManagementUsecase::read_usecase_data(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) const
{
    NodeManagementUseCaseDataType node_management_usecase_data;
    for (UseCase *uc : usecase_interface->usecase_list) {
        if (uc->get_usecase_type() != UseCaseType::NodeManagement) {
            node_management_usecase_data.useCaseInformation->push_back(uc->get_usecase_information());
        }
    }
    if (!node_management_usecase_data.useCaseInformation->empty()) {
        response["nodeManagementUseCaseData"] = node_management_usecase_data;
        if (response["nodeManagementUseCaseData"].isNull()) {
            logger.printfln("Error while writing NodeManagementUseCaseData to response");
        }
        return true;
    }
    return false;
}
bool NodeManagementUsecase::read_detailed_discovery_data(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) const
{
    // Detailed discovery as defined in EEBus SPINE TS ProtocolSpecification 7.1.2
    NodeManagementDetailedDiscoveryDataType node_management_detailed_data;
    node_management_detailed_data.specificationVersionList->specificationVersion->push_back(SUPPORTED_SPINE_VERSION);

    node_management_detailed_data.deviceInformation->description->description =
        "Tinkerforge WARP Charger"; // Optional. Shall not be longer than 4096 characters.
    node_management_detailed_data.deviceInformation->description->label =
        EEBUS_DEVICE_LABEL; // Optional. Shall not be longer than 256 characters.
    node_management_detailed_data.deviceInformation->description->networkFeatureSet =
        NetworkManagementFeatureSetType::simple; // Only simple operation is supported. We dont act as a SPINE router or anything like that.
    node_management_detailed_data.deviceInformation->description->deviceAddress->device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    node_management_detailed_data.deviceInformation->description->deviceType =
        "ChargingStation"; // Mandatory. String defined in EEBUS SPINE TS ResourceSpecification 4.1

    for (UseCase *uc : usecase_interface->usecase_list) {
        node_management_detailed_data.entityInformation->push_back(uc->get_detailed_discovery_entity_information());
        node_management_detailed_data.featureInformation->push_back(uc->get_detailed_discovery_feature_information());
    }
    return false;
}

bool NodeManagementUsecase::handle_subscription(HeaderType &header,
                                                SpineDataTypeHandler *data,
                                                JsonObject response,
                                                SpineConnection *connection)
{

    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionRequestCall && header.cmdClassifier == CmdClassifierType::call) {

        if (!data->nodemanagementsubscriptionrequestcalltype || !data->nodemanagementsubscriptionrequestcalltype->subscriptionRequest
            || !data->nodemanagementsubscriptionrequestcalltype->subscriptionRequest->clientAddress
            || !data->nodemanagementsubscriptionrequestcalltype->subscriptionRequest->serverAddress) {
            EEBUS_USECASE_HELPERS::build_result_data(response,
                                                     EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected,
                                                     "Subscription request failed, no or invalid subscription request data provided");
            return true;
        }
        NodeManagementSubscriptionRequestCallType request = data->nodemanagementsubscriptionrequestcalltype.value();
        SubscriptionManagementEntryDataType entry = SubscriptionManagementEntryDataType();
        //TODO: Implement and check trust level of the client
        entry.clientAddress = request.subscriptionRequest->clientAddress;
        entry.serverAddress = request.subscriptionRequest->serverAddress;
        subscription_data.subscriptionEntry->push_back(entry);
        EEBUS_USECASE_HELPERS::build_result_data(response,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError,
                                                 "Subscription request was successful");
        return true;

    }
    if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionData && header.cmdClassifier == CmdClassifierType::read) {
        response["nodeManagementSubscriptionData"] = subscription_data;
        return true;
    } else if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionDeleteCall) {
        //TODO: Implement subscription delete handling
    }

    return false;
}

NodeManagementDetailedDiscoveryEntityInformationType NodeManagementUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity->push_back(0); // Entity 0 is the NodeManagement entity
    entity.description->entityType = "DeviceInformation";    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.7
    entity.description->label = "Node Management";           // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}

NodeManagementDetailedDiscoveryFeatureInformationType NodeManagementUsecase::get_detailed_discovery_feature_information() const
{

    NodeManagementDetailedDiscoveryFeatureInformationType feature{};

    feature.description->featureAddress->entity->push_back(0); // Entity 0 is the NodeManagement entity
    feature.description->featureAddress->feature = 0;          // Feature 0 is the NodeManagement feature

    feature.description->featureType = "NodeManagement"; // The feature type as defined in EEBUS SPINE TS ResourceSpecification 4.3.19
    feature.description->role = RoleType::server;

    // The following functions are supported by the Nodemanagement feature
    // Basic Usecase information
    FunctionPropertyType useCaseData{};
    useCaseData.function = eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::nodeManagementUseCaseData).c_str();
    useCaseData.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(useCaseData);

    // Detailed discovery information
    FunctionPropertyType detailedDiscoveryData{};
    detailedDiscoveryData.function =
        eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::nodeManagementDetailedDiscoveryData).c_str();
    detailedDiscoveryData.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(detailedDiscoveryData);

    // Information about current bindings
    FunctionPropertyType nodemanagementBindingData{};
    nodemanagementBindingData.function =
        eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::nodeManagementBindingData).c_str();
    nodemanagementBindingData.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(nodemanagementBindingData);

    // Binding delete calls
    FunctionPropertyType nodemanagementBindingDelete{};
    nodemanagementBindingDelete.function =
        eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::nodeManagementBindingDeleteCall).c_str();
    nodemanagementBindingDelete.possibleOperations = PossibleOperationsType{};
    feature.description->supportedFunction->push_back(nodemanagementBindingDelete);

    // Binding request calls
    FunctionPropertyType nodemanagementBindingRequest{};
    nodemanagementBindingRequest.function =
        eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::nodeManagementBindingRequestCall).c_str();
    nodemanagementBindingRequest.possibleOperations = PossibleOperationsType{};
    feature.description->supportedFunction->push_back(nodemanagementBindingRequest);

    // Information about current Subscriptions
    FunctionPropertyType nodemanagemntSubscriptionData{};
    nodemanagemntSubscriptionData.function =
        eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::nodeManagementSubscriptionData).c_str();
    nodemanagemntSubscriptionData.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(nodemanagemntSubscriptionData);

    // Subscription delete calls
    FunctionPropertyType nodemanagementSubscriptionDelete{};
    nodemanagementSubscriptionDelete.function =
        eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::nodeManagementSubscriptionDeleteCall).c_str();
    nodemanagementSubscriptionDelete.possibleOperations = PossibleOperationsType{};
    feature.description->supportedFunction->push_back(nodemanagementSubscriptionDelete);

    // Subscription request calls
    FunctionPropertyType nodemanagementSubscriptionRequest{};
    nodemanagementSubscriptionRequest.function =
        eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::nodeManagementSubscriptionRequestCall).c_str();
    nodemanagementSubscriptionRequest.possibleOperations = PossibleOperationsType{};
    feature.description->supportedFunction->push_back(nodemanagementSubscriptionRequest);

    return feature;
}
void NodeManagementUsecase::inform_subscribers(int entity, int feature, SpineDataTypeHandler *data)
{
    std::optional<std::vector<int>> entities{};
    entities->push_back(entity);
    DynamicJsonDocument response(8192); // TODO: Change this to use less memory or move it PSRAM
    JsonVariant dst = response.createNestedObject(data->function_to_string(data->last_cmd));
    data->last_cmd_to_json(dst);
    for (SubscriptionManagementEntryDataType &subscription : subscription_data.subscriptionEntry.value()) {
        if (subscription.serverAddress->entity == entities && subscription.serverAddress->feature == feature) {
            for (ShipConnection &ship_connection : eebus.ship.ship_connections) {
                if (ship_connection.spine.check_known_address(subscription.clientAddress.value())) {
                    ship_connection.spine.send_datagram(response.as<JsonObject>(),
                                                        CmdClassifierType::notify,
                                                        subscription.serverAddress.value(),
                                                        subscription.clientAddress.value(),
                                                        false);
                }
            }
        }
    }
};

UseCaseInformationDataType ChargingSummaryUsecase::get_usecase_information()
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
bool ChargingSummaryUsecase::handle_message(HeaderType &header,
                                            SpineDataTypeHandler *data,
                                            JsonObject response,
                                            SpineConnection *connection)
{
    // TODO: implement messagehandling for the ChargingSummary usecase

    return false;
}
NodeManagementDetailedDiscoveryEntityInformationType ChargingSummaryUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity->push_back(entity_address);
    entity.description->entityType = "EVSE";        // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "Charging Summary"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}
NodeManagementDetailedDiscoveryFeatureInformationType ChargingSummaryUsecase::get_detailed_discovery_feature_information() const
{

    NodeManagementDetailedDiscoveryFeatureInformationType feature{};

    feature.description->featureAddress->entity->push_back(entity_address);
    feature.description->featureAddress->feature = 1; // Feature 1 is the Bill feature

    feature.description->featureType = "Bill"; // The feature type as defined in EEBUS SPINE TS ResourceSpecification 4.3.19
    feature.description->role = RoleType::server;

    // The following functions are needed by the ChargingSummary usecase
    // Bill description information
    FunctionPropertyType billDescriptionList{};
    billDescriptionList.function = eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::billDescriptionListData).c_str();
    billDescriptionList.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(billDescriptionList);

    // Bill constraints information
    FunctionPropertyType billconstraints{};
    billconstraints.function = eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::billConstraintsListData).c_str();
    billconstraints.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(billconstraints);

    // Bill list information
    FunctionPropertyType billListData{};
    billListData.function = eebus.data_handler->function_to_string(SpineDataTypeHandler::Function::billListData).c_str();
    billListData.possibleOperations->read = PossibleOperationsReadType{};
    feature.description->supportedFunction->push_back(billListData);

    return feature;
}

void NodeManagementUsecase::set_usecaseManager(EEBusUseCases *new_usecase_interface)
{
    usecase_interface = new_usecase_interface;
}

EEBusUseCases::EEBusUseCases()
{
    node_management = NodeManagementUsecase();
    node_management.set_usecaseManager(this);
    charging_summary = ChargingSummaryUsecase();

    for (uint8_t i = 0; i < EEBUS_USECASES_ACTIVE; i++) {
        usecase_list[i]->set_entity_address(i);
    }
}
void EEBusUseCases::handle_message(HeaderType &header, SpineDataTypeHandler *data, SpineConnection *connection)
{
    //TODO: Implement a mutex so only one message can be processed at a time
    DynamicJsonDocument response = DynamicJsonDocument(8192); // The response document to be filled with the response data
    // TODO: Fix the addressing of the usecases. Maybe better address them by entity?
    bool send_response = false;
    if (header.addressDestination->feature == feature_address_node_management) {
        logger.printfln("EEBus: Received message for NodeManagementUsecase");
        send_response = node_management.handle_message(header, data, response.as<JsonObject>(), connection);
    }
    if (header.addressDestination->feature == feature_address_charging_summary) {
        logger.printfln("EEBus: Received message for ChargingSummaryUsecase");
        send_response = charging_summary.handle_message(header, data, response.as<JsonObject>(), connection);
    }
    if (send_response) {
        connection->send_datagram(response.as<JsonVariant>(),
                                  CmdClassifierType::reply,
                                  *header.addressSource,
                                  *header.addressDestination,
                                  false);
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
