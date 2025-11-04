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


#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "ship_types.h"
#include "tools.h"
#include "eebus.h"
#include <chrono>
#include <regex>
#include <utility>
#include "lib/libiso8601/iso8601.h"
#include "ocpp/Types.h"


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

    return CmdClassifierType::EnumUndefined;
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
    return {};
    // This should never be used as the NodeManagementUsecase has no usecase information
}

CmdClassifierType NodeManagementEntity::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.addressDestination->feature == FeatureAddresses::nodemgmt_feature_address) {
        eebus.trace_fmtln("NodeManagementUsecase: Handling message with cmdClassifier %d and command %s",
                          static_cast<int>(header.cmdClassifier.get()),
                          data->function_to_string(data->last_cmd).c_str());
        if (header.cmdClassifier == CmdClassifierType::read && data->last_cmd == SpineDataTypeHandler::Function::nodeManagementUseCaseData) {
            eebus.trace_fmtln("NodeManagementUsecase: Command identified as NodeManagementUseCaseData");
            response["nodeManagementUseCaseData"] = get_usecase_data();
            return CmdClassifierType::reply;

        }
        if (header.cmdClassifier == CmdClassifierType::read && data->last_cmd == SpineDataTypeHandler::Function::nodeManagementDetailedDiscoveryData) {
            eebus.trace_fmtln("NodeManagementUsecase: Command identified as NodeManagementDetailedDiscoveryData");
            response["nodeManagementDetailedDiscoveryData"] = get_detailed_discovery_data();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionData || data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionRequestCall || data->last_cmd == SpineDataTypeHandler::Function::nodeManagementSubscriptionDeleteCall) {
            eebus.trace_fmtln("NodeManagementUsecase: Command identified as Subscription handling");
            return handle_subscription(header, data, response, connection);
        }

        if (data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingData || data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingRequestCall || data->last_cmd == SpineDataTypeHandler::Function::nodeManagementBindingDeleteCall) {
            eebus.trace_fmtln("NodeManagementUsecase: Command identified as Binding handling");
            return handle_binding(header, data, response);
        }
    }
    return CmdClassifierType::EnumUndefined;
}

void NodeManagementEntity::detailed_discovery_update()
{
    auto detailed_data = get_detailed_discovery_data();
    this->inform_subscribers(entity_address, nodemgmt_feature_address, detailed_data, "nodeManagementDetailedDiscoveryData");
}

NodeManagementUseCaseDataType NodeManagementEntity::get_usecase_data() const
{
    NodeManagementUseCaseDataType node_management_usecase_data;
    for (EebusUsecase *uc : usecase_interface->entity_list) {
        if (uc->get_usecase_type() != UseCaseType::NodeManagement) {
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
#if OPTIONS_PRODUCT_ID_IS_WARP_ANY() == 1
    node_management_detailed_data.deviceInformation->description->deviceType = DeviceTypeEnumType::ChargingStation; // Mandatory. String defined in EEBUS SPINE TS ResourceSpecification 4.1
#elif OPTIONS_PRODUCT_ID_IS_ENERGY_MANAGER() == 1
    node_management_detailed_data.deviceInformation->description->deviceType = DeviceTypeEnumType::EnergyManagementSystem; // Mandatory. String defined in EEBUS SPINE TS ResourceSpecification 4.1
#endif
    for (EebusUsecase *uc : usecase_interface->entity_list) {
        if (!uc->isActive()) {
            continue;
        }
        bool add_entity_info = true;
        auto entity_info = uc->get_detailed_discovery_entity_information();
        for (auto &existing_entity_info : *(node_management_detailed_data.entityInformation)) {
            // If the entity type matches, we do not add it again and if it has no entity type defined it is inactive
            if (entity_info.description->entityType.has_value() && existing_entity_info.description->entityType == entity_info.description->entityType.get()) {
                add_entity_info = false;
                break;
            }
        }

        if (add_entity_info && entity_info.description->entityType.has_value()) {
            node_management_detailed_data.entityInformation->push_back(uc->get_detailed_discovery_entity_information());
        }
        auto features = uc->get_detailed_discovery_feature_information();
        node_management_detailed_data.featureInformation->insert(node_management_detailed_data.featureInformation->end(), features.begin(), features.end());
    }
    return node_management_detailed_data;

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
        return CmdClassifierType::result;
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

template <typename T>
size_t NodeManagementEntity::inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, const T data, const char *function_name)
{
    eebus.trace_fmtln("EEBUS: Informing subscribers of %s", function_name);
    if (!EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS || subscription_data.subscriptionEntry.isNull() || subscription_data.subscriptionEntry.get().empty()) {
        return 0;
    }
    size_t sent_count = 0;
    size_t error_count = 0;
    BasicJsonDocument<ArduinoJsonPsramAllocator> response(SPINE_CONNECTION_MAX_JSON_SIZE);
    JsonObject dst = response.to<JsonObject>();

    dst[function_name] = data;
    for (SubscriptionManagementEntryDataType &subscription : subscription_data.subscriptionEntry.get()) {
        if (subscription.serverAddress->entity == entity && subscription.serverAddress->feature == feature) {
            if (usecase_interface->send_spine_message(subscription.clientAddress.get(), subscription.serverAddress.get(), response.as<JsonVariantConst>(), CmdClassifierType::notify, false)) {
                sent_count++;
            } else {
                error_count++;
                // TODO: If sending fails, we should remove the subscription after a number of failed attempts as the connection is likely dead
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


EvcsUsecase::EvcsUsecase()
{
    update_api();
#ifdef EEBUS_DEV_TEST_ENABLE
    task_scheduler.scheduleOnce([this]() {
                                    logger.printfln("EEBUS Usecase test enabled. Updating ChargingSummary");
                                    update_billing_data(1, 299921, 3242662, 245233, 1242, 75, 90, 25, 10);
                                    update_billing_data(2, 5622123, 5655611, 23677, 1242, 50, 100, 50, 0);

                                },
                                30_s);
#endif
}

UseCaseInformationDataType EvcsUsecase::get_usecase_information()
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
    evcs_usecase.address = evcs_usecase_feature_address;
    return evcs_usecase;
}

CmdClassifierType EvcsUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.addressDestination->feature.has_value() && header.addressDestination->feature == bill_feature_address) {
        return bill_feature(header, data, response, connection);
    }

    return CmdClassifierType::EnumUndefined;
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
    auto bill_list_data = get_bill_list_data();
    eebus.usecases->inform_subscribers(this->entity_address, this->bill_feature_address, bill_list_data, "billListData");
    update_api();
}


BillListDataType EvcsUsecase::get_bill_list_data() const
{
    BillListDataType bill_list_data{};
    for (BillEntry entry : bill_entries) {
        if (!entry.id)
            continue;
        BillDataType billData{};
        billData.billId = entry.id;
        billData.billType = BillTypeEnumType::chargingSummary;

        billData.total->timePeriod->startTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(entry.start_time).c_str();
        billData.total->timePeriod->endTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(entry.start_time).c_str();
        BillValueType total_value;
        total_value.value->number = entry.energy_wh;
        total_value.unit = UnitOfMeasurementEnumType::Wh;
        total_value.value->scale = 0; // The total value is calculated like numer * 10^(scale).
        BillCostType total_cost;
        total_cost.costType = BillCostTypeEnumType::absolutePrice;
        total_cost.cost->number = entry.cost_eur_cent;
        total_cost.currency = CurrencyEnumType::EUR;
        total_cost.cost->scale = -2; // We send the cost in cents, so
        billData.total->value->push_back(total_value);
        billData.total->cost->push_back(total_cost);

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

        BillPositionType self_produced_position;
        self_produced_position.positionId = 2;
        self_produced_position.positionType = BillPositionTypeEnumType::selfProducedElectricEnergy;
        BillValueType self_produced_value;
        self_produced_value.valuePercentage->number = entry.self_produced_energy_percent;
        self_produced_value.valuePercentage->scale = 0;
        BillCostType self_produced_cost;
        self_produced_cost.costPercentage->number = entry.self_produced_energy_percent;
        grid_position.cost->push_back(grid_cost);
        grid_position.value->push_back(grid_value);

        billData.position->push_back(grid_position);
        billData.position->push_back(self_produced_position);

        bill_list_data.billData->push_back(billData);
    }
    return bill_list_data;
}

CmdClassifierType EvcsUsecase::bill_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    //TODO: Support writes and deletes
    if (data->last_cmd == SpineDataTypeHandler::Function::billDescriptionListData && header.cmdClassifier == CmdClassifierType::read) {
        // EEBUS_UC_TS_EVCHargingSummary_v1.0.1.pdf 3.2.1.2.2.1 Function "billDescriptionListData"
        BillDescriptionListDataType billDescriptionListData{};
        billDescriptionListData.billDescriptionData.emplace();
        for (BillEntry bill_entry : bill_entries) {
            BillDescriptionDataType billDescriptionData{};
            billDescriptionData.billWriteable = false; // No bill is writeable for now
            billDescriptionData.billId = bill_entry.id;
            billDescriptionData.supportedBillType->push_back(BillTypeEnumType::chargingSummary);
            billDescriptionListData.billDescriptionData->push_back(billDescriptionData);

        }
        response["billDescriptionListData"] = billDescriptionListData;
        return CmdClassifierType::reply;
    }
    if (data->last_cmd == SpineDataTypeHandler::Function::billConstraintsListData && header.cmdClassifier == CmdClassifierType::read) {
        // EEBUS_UC_TS_EVCHargingSummary_v1.0.1.pdf 3.2.1.2.2.2 Function "billConstraintsListData"
        BillConstraintsListDataType billConstraintsListData{};
        billConstraintsListData.billConstraintsData.emplace();
        for (BillEntry bill_entry : bill_entries) {
            BillConstraintsDataType billConstraintsData{};
            billConstraintsData.billId = bill_entry.id;
            billConstraintsData.positionCountMin = "0";
            billConstraintsData.positionCountMax = std::to_string(2);
            billConstraintsListData.billConstraintsData->push_back(billConstraintsData);
        }
        response["billConstraintsListData"] = billConstraintsListData;
        return CmdClassifierType::reply;

    }
    if (data->last_cmd == SpineDataTypeHandler::Function::billListData && header.cmdClassifier == CmdClassifierType::read) {
        // EEBUS_UC_TS_EVCHargingSummary_v1.0.1.pdf 3.2.1.2.2.3 Function "billListData"
        response["billListData"] = get_bill_list_data();
        return CmdClassifierType::reply;
    }
    if (data->last_cmd == SpineDataTypeHandler::Function::billListData && header.cmdClassifier == CmdClassifierType::write) {
        // EEBUS_UC_TS_EVCHargingSummary_v1.0.1.pdf 3.2.1.2.2.3 Function "billListData"
        //response["billListData"] = get_bill_list_data();
        //TODO: Implement write on this feature
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Writing billing data is not yet supported");
        return CmdClassifierType::result;
    }
    return CmdClassifierType::EnumUndefined;
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

EvcemUsecase::EvcemUsecase()
{
#ifdef EEBUS_DEV_TEST_ENABLE
    task_scheduler.scheduleUncancelable([this]() {
                                              logger.printfln("EEBUS Usecase test enabled. Updating EvcemUsecase");
                                              update_measurements(32, 32, 32, 7000, 7000, 7000, this->power_charged_wh + 100);
                                          },
                                          60_s,
                                          60_s);
#endif
}

CmdClassifierType EvcemUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.cmdClassifier == CmdClassifierType::read) {
        if (data->last_cmd == SpineDataTypeHandler::Function::measurementDescriptionListData) {
            response["measurementDescriptionListData"] = generate_measurement_description();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::measurementConstraintsListData) {
            response["measurementConstraintsListData"] = generate_measurement_constraints();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::measurementListData) {
            response["measurementListData"] = generate_measurement_list();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::electricalConnectionDescriptionListData) {
            response["electricalConnectionDescriptionListData"] = generate_electrical_connection_description();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::electricalConnectionParameterDescriptionListData) {
            response["electricalConnectionParameterDescriptionListData"] = generate_electrical_connection_parameters();
            return CmdClassifierType::reply;
        }
    }
    return CmdClassifierType::EnumUndefined;
}

UseCaseInformationDataType EvcemUsecase::get_usecase_information()
{
    UseCaseInformationDataType evcm_usecase;
    evcm_usecase.actor = "EV";

    UseCaseSupportType evcm_usecase_support;
    evcm_usecase_support.useCaseName = "measurementOfElectricityDuringCharging";
    evcm_usecase_support.useCaseVersion = "1.0.1";
    evcm_usecase_support.useCaseAvailable = true;
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
    measurement_feature.description->featureAddress->feature = feature_address_measurement;
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
    electricalConnection_feature.description->featureAddress->feature = feature_address_electrical_connection;
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
    amps_draw_phase[0] = amps_phase_1;
    amps_draw_phase[1] = amps_phase_2;
    amps_draw_phase[2] = amps_phase_3;
    power_draw_phase[0] = power_phase_1;
    power_draw_phase[1] = power_phase_2;
    power_draw_phase[2] = power_phase_3;
    power_charged_wh = charged_wh;
    power_charged_measured = charged_measured;

    auto measurement_list_data = generate_measurement_list();
    eebus.usecases->inform_subscribers(this->entity_address, this->feature_address_measurement, measurement_list_data, "measurementListData");
    update_api();
}

void EvcemUsecase::update_constraints(const int amps_min, const int amps_max, const int amps_stepsize, const int power_min, const int power_max, const int power_stepsize, const int energy_min, const int energy_max, const int energy_stepsize)
{
    measurement_limit_amps_min = amps_min;
    measurement_limit_amps_max = amps_max;
    measurement_limit_amps_stepsize = amps_stepsize;
    measurement_limit_power_min = power_min;
    measurement_limit_power_max = power_max;
    measurement_limit_power_stepsize = power_stepsize;
    measurement_limit_energy_min = energy_min;
    measurement_limit_energy_max = energy_max;
    measurement_limit_energy_stepsize = energy_stepsize;

    auto constraints = generate_measurement_constraints();
    eebus.usecases->inform_subscribers(this->entity_address, this->feature_address_measurement, constraints, "measurementConstraintsListData");

    update_api();
}

MeasurementDescriptionListDataType EvcemUsecase::generate_measurement_description() const
{
    MeasurementDescriptionListDataType measurement_description{};
    for (uint8_t i = 0; i < 7; i++) {
        MeasurementDescriptionDataType measurement_description_data{};
        measurement_description_data.measurementId = i + 1;
        if (i < 3) {
            measurement_description_data.measurementType = MeasurementTypeEnumType::current;
            measurement_description_data.commodityType = CommodityTypeEnumType::electricity;
            measurement_description_data.unit = UnitOfMeasurementEnumType::A;
            measurement_description_data.scopeType = ScopeTypeEnumType::acCurrent;
        } else if (i < 6) {
            measurement_description_data.measurementType = MeasurementTypeEnumType::power;
            measurement_description_data.commodityType = CommodityTypeEnumType::electricity;
            measurement_description_data.unit = UnitOfMeasurementEnumType::W;
            measurement_description_data.scopeType = ScopeTypeEnumType::acPower;
        } else {
            measurement_description_data.measurementType = MeasurementTypeEnumType::energy;
            measurement_description_data.commodityType = CommodityTypeEnumType::electricity;
            measurement_description_data.unit = UnitOfMeasurementEnumType::Wh;
            measurement_description_data.scopeType = ScopeTypeEnumType::charge;
        }
        measurement_description.measurementDescriptionData->push_back(measurement_description_data);
    }
    return measurement_description;
}

MeasurementConstraintsListDataType EvcemUsecase::generate_measurement_constraints() const
{
    MeasurementConstraintsListDataType measurement_constraints{};
    for (uint8_t i = 0; i < 7; i++) {
        MeasurementConstraintsDataType measurement_constraints_data{};
        measurement_constraints_data.measurementId = i + 1;
        if (i < 3) {
            if (measurement_limit_amps_min >= 0)
                measurement_constraints_data.valueRangeMin->number = measurement_limit_amps_min;
            if (measurement_limit_amps_max >= 0)
                measurement_constraints_data.valueRangeMax->number = measurement_limit_amps_max;
            if (measurement_limit_amps_stepsize)
                measurement_constraints_data.valueStepSize->number = measurement_limit_amps_stepsize;
        } else if (i < 6) {
            if (measurement_limit_power_min >= 0)
                measurement_constraints_data.valueRangeMin->number = measurement_limit_power_min;
            if (measurement_limit_power_max >= 0)
                measurement_constraints_data.valueRangeMax->number = measurement_limit_power_max;
            if (measurement_limit_power_stepsize)
                measurement_constraints_data.valueStepSize->number = measurement_limit_power_stepsize;
        } else {
            if (measurement_limit_energy_min >= 0)
                measurement_constraints_data.valueRangeMin->number = measurement_limit_power_min;
            if (measurement_limit_power_max >= 0)
                measurement_constraints_data.valueRangeMax->number = measurement_limit_power_max;
            if (measurement_limit_power_stepsize)
                measurement_constraints_data.valueStepSize->number = measurement_limit_power_stepsize;
        }
        measurement_constraints.measurementConstraintsData->push_back(measurement_constraints_data);
    }
    return measurement_constraints;
}

MeasurementListDataType EvcemUsecase::generate_measurement_list() const
{
    MeasurementListDataType measurement_list{};
    for (uint8_t i = 0; i < 7; i++) {
        MeasurementDataType measurement_data{};
        measurement_data.measurementId = i + 1;
        measurement_data.valueType = MeasurementValueTypeEnumType::value;
        if (i < 3) {
            measurement_data.value->number = amps_draw_phase[i];
        } else if (i < 6) {
            measurement_data.value->number = power_draw_phase[i - 3];
        } else {
            measurement_data.value->number = power_charged_wh;
            if (power_charged_measured) {
                measurement_data.valueSource = MeasurementValueSourceEnumType::measuredValue;
            } else {
                measurement_data.valueSource = MeasurementValueSourceEnumType::calculatedValue;
            }
        }
        measurement_list.measurementData->push_back(measurement_data);
    }
    return measurement_list;
}

ElectricalConnectionDescriptionListDataType EvcemUsecase::generate_electrical_connection_description() const
{
    ElectricalConnectionDescriptionListDataType electrical_connection_description{};
    ElectricalConnectionDescriptionDataType connection_description_data{};
    connection_description_data.electricalConnectionId = 1;
    connection_description_data.powerSupplyType = ElectricalConnectionVoltageTypeEnumType::ac;
    connection_description_data.positiveEnergyDirection = EnergyDirectionEnumType::consume;
    electrical_connection_description.electricalConnectionDescriptionData->push_back(connection_description_data);

    return electrical_connection_description;
}

ElectricalConnectionParameterDescriptionListDataType EvcemUsecase::generate_electrical_connection_parameters() const
{
    ElectricalConnectionParameterDescriptionListDataType electrical_connection_parameters{};
    for (uint8_t i = 0; i < 7; i++) {
        ElectricalConnectionParameterDescriptionDataType connection_parameters_data{};
        connection_parameters_data.parameterId = i + 1; // This is a new ID
        connection_parameters_data.measurementId = i + 1; // This should be the same as the measurement IDs in other function
        connection_parameters_data.electricalConnectionId = 1; // This refers to the electrical connection in the connection description function
        if (i < 6) {
            switch (i) {
                case 0:
                    connection_parameters_data.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::a;
                    break;
                case 1:
                    connection_parameters_data.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::b;
                    break;
                case 2:
                    connection_parameters_data.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::c;
                    break;
                case 3:
                    connection_parameters_data.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::a;
                    break;
                case 4:
                    connection_parameters_data.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::b;
                    break;
                case 5:
                    connection_parameters_data.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::c;
                    break;
                default:
                    break;
            }
            if (i < 3) {
                connection_parameters_data.acMeasurementVariant = ElectricalConnectionMeasurandVariantEnumType::rms;
            }
        } else {
            connection_parameters_data.voltageType = ElectricalConnectionVoltageTypeEnumType::ac;
            connection_parameters_data.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::abc;
            connection_parameters_data.acMeasurementType = ElectricalConnectionAcMeasurementTypeEnumType::real;

        }
        electrical_connection_parameters.electricalConnectionParameterDescriptionData->push_back(connection_parameters_data);
    }
    return electrical_connection_parameters;
}

void EvcemUsecase::update_api() const
{
    // TODO: Update API
}

EvccUsecase::EvccUsecase()
{
    entity_active = false; // Disable entity until an EV is connected
#ifdef EEBUS_DEV_TEST_ENABLE
    task_scheduler.scheduleOnce([this]() {
                                    logger.printfln("EEBUS Usecase test enabled. Updating EvccUsecase");
                                    ev_connected_state(true);
                                    update_device_config("iso15118-2ed1", true);
                                    update_identification("12:34:56:78:9a:bc");
                                    update_manufacturer("VW", "0", "00001", "1.0", "0.1", "Volkswagen", "1", "Skoda", "VW", "");
                                    update_electrical_connection(0, EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION, 800);
                                    update_operating_state(false);

                                },
                                40_s);
#endif

}

UseCaseInformationDataType EvccUsecase::get_usecase_information()
{
    UseCaseInformationDataType evcc_usecase;
    evcc_usecase.actor = "EV";

    UseCaseSupportType evcc_usecase_support;
    evcc_usecase_support.useCaseName = "evCommissioningAndConfiguration";
    evcc_usecase_support.useCaseVersion = "1.0.1";
    evcc_usecase_support.useCaseAvailable = true;
    evcc_usecase_support.scenarioSupport->insert(evcc_usecase_support.scenarioSupport->end(), {1, 2, 3, 4, 5, 6, 7, 8});

    evcc_usecase_support.useCaseDocumentSubRevision = "release";
    evcc_usecase.useCaseSupport->push_back(evcc_usecase_support);

    FeatureAddressType evcc_usecase_feature_address;
    evcc_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    evcc_usecase_feature_address.entity = entity_address;
    evcc_usecase.address = evcc_usecase_feature_address;
    return evcc_usecase;
}

CmdClassifierType EvccUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.cmdClassifier == CmdClassifierType::read) {
        if (data->last_cmd == SpineDataTypeHandler::Function::deviceConfigurationKeyValueDescriptionListData) {
            response["deviceConfigurationKeyValueDescriptionListData"] = generate_device_config_description();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::deviceConfigurationKeyValueListData) {
            response["deviceConfigurationKeyValueListData"] = generate_device_config_list();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::identificationListData) {
            response["identificationListData"] = generate_identification_description();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::deviceClassificationManufacturerData) {
            response["deviceClassificationManufacturerData"] = generate_manufacturer_description();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::electricalConnectionParameterDescriptionListData) {
            response["electricalConnectionParameterDescriptionListData"] = generate_electrical_connection_description();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::electricalConnectionPermittedValueSetListData) {
            response["electricalConnectionPermittedValueSetListData"] = generate_electrical_connection_values();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::deviceDiagnosisStateData) {
            response["deviceDiagnosisStateData"] = generate_state();
            return CmdClassifierType::reply;
        }

    } else if (header.cmdClassifier == CmdClassifierType::write || header.cmdClassifier == CmdClassifierType::call) {
        EEBUS_USECASE_HELPERS::build_result_data(response,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected,
                                                 "Only Reads supported on this entity");
        return CmdClassifierType::reply;
    }
    return CmdClassifierType::EnumUndefined;
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

    // The following functions are needed by the DeviceDiagnosis Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType device_configuration_feature{};
    device_configuration_feature.description->featureAddress->entity = entity_address;
    device_configuration_feature.description->featureAddress->feature = feature_address_device_configuration; // Feature IDs are just arbitrary numbers. Just have to be unique within the entity
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
    identification_feature.description->featureAddress->feature = feature_address_identification;
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
    device_classification_feature.description->featureAddress->feature = feature_address_device_classification;
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
    electricalConnectionFeature.description->featureAddress->feature = feature_address_electrical_connection;
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
    deviceDiagnosisFeature.description->featureAddress->feature = feature_address_device_diagnosis;
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

void EvccUsecase::ev_connected_state(bool connected)
{
    entity_active = ev_connected = connected;
    eebus.usecases->node_management.detailed_discovery_update();
    update_api();
}

void EvccUsecase::update_device_config(const String &comm_standard, bool asym_supported)
{
    communication_standard = comm_standard;
    asymmetric_supported = asym_supported;
    // As defined in EEBUS_UC_TS_EVCommissioningAndConfiguration_v1.0.1.pdf 3.2.1.2.2.2.
    if (communication_standard != "iso15118-2ed1" && communication_standard != "iso15118-2ed2" && communication_standard != "iec61851") {
        eebus.trace_fmtln(R"(Usecase EVCC: Invalid communication standard for EV entity device configuration: %s, should be "iso15118-2ed1","iso15118-2ed1" or "iec61851".)", communication_standard.c_str());
        // We continue on regardless and let the peer deal with incorrect values
    }
    auto generate_dev_desc = generate_device_config_description();
    auto generate_dev_list = generate_device_config_list();
    eebus.usecases->inform_subscribers(entity_address, feature_address_device_configuration, generate_dev_desc, "deviceConfigurationKeyValueDescriptionData");
    eebus.usecases->inform_subscribers(entity_address, feature_address_device_configuration, generate_dev_list, "deviceConfigurationKeyValueListData");

    update_api();
}

void EvccUsecase::update_identification(String mac, IdentificationTypeEnumType type)
{
    mac_address = std::move(mac);
    mac_type = type;
    auto identification_desc = generate_identification_description();
    eebus.usecases->inform_subscribers(entity_address, feature_address_identification, identification_desc, "identificationListData");
    update_api();
}

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

    auto manufacturer_desc = generate_manufacturer_description();
    eebus.usecases->inform_subscribers(entity_address, feature_address_device_classification, manufacturer_desc, "deviceClassificationManufacturerData");
    update_api();
}

void EvccUsecase::update_electrical_connection(int min_power, int max_power, int stby_power)
{
    min_power_draw = min_power;
    max_power_draw = max_power;
    standby_power = stby_power;
    auto electrical_connection_desc = generate_electrical_connection_description();
    auto electrical_connection_values = generate_electrical_connection_values();
    eebus.usecases->inform_subscribers(entity_address, feature_address_electrical_connection, electrical_connection_desc, "electricalConnectionParameterDescriptionListData");
    eebus.usecases->inform_subscribers(entity_address, feature_address_electrical_connection, electrical_connection_values, "electricalConnectionPermittedValueSetListData");

    update_api();
}

void EvccUsecase::update_operating_state(bool standby)
{
    standby_mode = standby;
    auto state = generate_state();
    eebus.usecases->inform_subscribers(entity_address, feature_address_device_diagnosis, state, "deviceDiagnosisStateData");
    update_api();
}


void EvccUsecase::update_api() const
{
    auto api_entry = eebus.eebus_usecase_state.get("ev_commissioning_and_configuration");
    api_entry->get("ev_connected")->updateBool(ev_connected);
    if (ev_connected) {
        api_entry->get("communication_standard")->updateString(communication_standard);
        api_entry->get("asymmetric_charging_supported")->updateBool(asymmetric_supported);
        api_entry->get("mac_address")->updateString(mac_address);
        api_entry->get("minimum_power")->updateUint(min_power_draw);
        api_entry->get("maximum_power")->updateUint(max_power_draw);
        api_entry->get("standby_power")->updateUint(standby_power);
        api_entry->get("standby_mode")->updateBool(standby_mode);
    }

}

DeviceConfigurationKeyValueDescriptionListDataType EvccUsecase::generate_device_config_description() const
{
    DeviceConfigurationKeyValueDescriptionListDataType device_configuration{};
    device_configuration.deviceConfigurationKeyValueDescriptionData.emplace();
    DeviceConfigurationKeyValueDescriptionDataType comm_standard_description{};
    comm_standard_description.keyId = 1;
    comm_standard_description.keyName = DeviceConfigurationKeyNameEnumType::communicationsStandard;
    comm_standard_description.valueType = DeviceConfigurationKeyValueTypeType::string;
    device_configuration.deviceConfigurationKeyValueDescriptionData->push_back(comm_standard_description);

    DeviceConfigurationKeyValueDescriptionDataType asymmetric_description{};
    asymmetric_description.keyId = 2;
    asymmetric_description.keyName = DeviceConfigurationKeyNameEnumType::asymmetricChargingSupported;
    asymmetric_description.valueType = DeviceConfigurationKeyValueTypeType::boolean;
    device_configuration.deviceConfigurationKeyValueDescriptionData->push_back(asymmetric_description);

    return device_configuration;
}

DeviceConfigurationKeyValueListDataType EvccUsecase::generate_device_config_list() const
{
    DeviceConfigurationKeyValueListDataType device_configuration{};
    device_configuration.deviceConfigurationKeyValueData.emplace();
    DeviceConfigurationKeyValueDataType comm_standard_value{};
    comm_standard_value.keyId = 1;
    comm_standard_value.value->string = communication_standard.c_str();
    device_configuration.deviceConfigurationKeyValueData->push_back(comm_standard_value);

    DeviceConfigurationKeyValueDataType asymmetric_value{};
    asymmetric_value.keyId = 2;
    asymmetric_value.value->boolean = asymmetric_supported;
    device_configuration.deviceConfigurationKeyValueData->push_back(asymmetric_value);
    return device_configuration;
}

IdentificationListDataType EvccUsecase::generate_identification_description() const
{
    IdentificationListDataType identification_data{};
    identification_data.identificationData.emplace();

    IdentificationDataType identification_data_entry{};
    identification_data_entry.identificationId = 1;
    identification_data_entry.identificationType = mac_type;
    identification_data_entry.identificationValue = mac_address.c_str();
    identification_data.identificationData->push_back(identification_data_entry);
    return identification_data;
}


DeviceClassificationManufacturerDataType EvccUsecase::generate_manufacturer_description() const
{
    DeviceClassificationManufacturerDataType manufacturer_data{};
    if (manufacturer_name.length() > 0)
        manufacturer_data.deviceName = manufacturer_name.c_str();
    if (manufacturer_code.length() > 0)
        manufacturer_data.deviceCode = manufacturer_code.c_str();
    if (ev_serial_number.length() > 0)
        manufacturer_data.serialNumber = ev_serial_number.c_str();
    if (ev_sofware_version.length() > 0)
        manufacturer_data.softwareRevision = ev_sofware_version.c_str();
    if (ev_hardware_version.length() > 0)
        manufacturer_data.hardwareRevision = ev_hardware_version.c_str();
    if (vendor_name.length() > 0)
        manufacturer_data.vendorName = vendor_name.c_str();
    if (vendor_code.length() > 0)
        manufacturer_data.vendorCode = vendor_code.c_str();
    if (brand_name.length() > 0)
        manufacturer_data.brandName = brand_name.c_str();
    if (manufacturer_label.length() > 0)
        manufacturer_data.manufacturerLabel = manufacturer_label.c_str();
    if (manufacturer_description.length() > 0)
        manufacturer_data.manufacturerDescription = manufacturer_description.c_str();

    return manufacturer_data;
}

ElectricalConnectionParameterDescriptionListDataType EvccUsecase::generate_electrical_connection_description() const
{
    ElectricalConnectionParameterDescriptionListDataType electrical_connection_description{};
    electrical_connection_description.electricalConnectionParameterDescriptionData.emplace();

    ElectricalConnectionParameterDescriptionDataType power_description{};
    power_description.parameterId = 1;
    power_description.acMeasuredPhases = ElectricalConnectionPhaseNameEnumType::neutral; // Not sure but the spec has no value here, maybe its what we are capable of measuring?
    power_description.scopeType = ScopeTypeEnumType::acPowerTotal;
    electrical_connection_description.electricalConnectionParameterDescriptionData->push_back(power_description);
    return electrical_connection_description;

}

ElectricalConnectionPermittedValueSetListDataType EvccUsecase::generate_electrical_connection_values() const
{

    ElectricalConnectionPermittedValueSetListDataType electrical_connection_values{};
    electrical_connection_values.electricalConnectionPermittedValueSetData.emplace();

    ElectricalConnectionPermittedValueSetDataType permitted_values{};
    permitted_values.electricalConnectionId = 1;
    permitted_values.parameterId = 1;
    ScaledNumberSetType minmax_power_value_set{};
    ScaledNumberRangeType power_value_range{};
    power_value_range.min->number = min_power_draw;
    power_value_range.max->number = max_power_draw;
    minmax_power_value_set.range->push_back(power_value_range);
    permitted_values.permittedValueSet->push_back(minmax_power_value_set);

    ScaledNumberSetType standby_power_value_set{};
    ScaledNumberType standby_value_range{};
    standby_value_range.number = standby_power;
    standby_power_value_set.value->push_back(standby_value_range);
    permitted_values.permittedValueSet->push_back(standby_power_value_set);
    electrical_connection_values.electricalConnectionPermittedValueSetData->push_back(permitted_values);
    return electrical_connection_values;
}

DeviceDiagnosisStateDataType EvccUsecase::generate_state() const
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

EvseccUsecase::EvseccUsecase()
{
#ifdef EEBUS_DEV_TEST_ENABLE
    task_scheduler.scheduleOnce([this]() {
                                    logger.printfln("EEBUS Usecase test enabled. Updating EvseccUsecase with a test error");
                                    update_operating_state(true, "This is a test error message");

                                },
                                50_s);
    task_scheduler.scheduleOnce([this]() {
                                    logger.printfln("EEBUS Usecase test enabled. Updating EvseccUsecase to normal operation");
                                    update_operating_state(false, "This is a test error message. It should not be shown");

                                },
                                200_s);
#endif
}

CmdClassifierType EvseccUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.cmdClassifier == CmdClassifierType::read) {
        if (data->last_cmd == SpineDataTypeHandler::Function::deviceClassificationManufacturerData) {
            response["deviceClassificationManufacturerData"] = generate_manufacturer_description();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::deviceDiagnosisStateData) {
            response["deviceDiagnosisStateData"] = generate_state();
            return CmdClassifierType::reply;
        }
        EEBUS_USECASE_HELPERS::build_result_data(response,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandNotSupported,
                                                 "Command not supported on this entity");
        return CmdClassifierType::result;
    }
    EEBUS_USECASE_HELPERS::build_result_data(response,
                                             EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected,
                                             "Only Reads supported on this entity");
    return CmdClassifierType::result;
}

UseCaseInformationDataType EvseccUsecase::get_usecase_information()
{
    UseCaseInformationDataType evse_usecase;
    evse_usecase.actor = "EVSE";

    UseCaseSupportType evse_usecase_support;
    evse_usecase_support.useCaseName = "evseCommissioningAndConfiguration";
    evse_usecase_support.useCaseVersion = "1.0.1";
    evse_usecase_support.useCaseAvailable = true;
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
    device_classification.description->featureAddress->feature = feature_address_device_classification;
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
    device_diagnosis.description->featureAddress->feature = feature_address_device_diagnosis;
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

void EvseccUsecase::update_operating_state(bool failure, const String &error_message)
{
    if (failure) {
        operating_state = DeviceDiagnosisOperatingStateEnumType::failure;
        last_error_message = error_message.c_str();
    } else {
        operating_state = DeviceDiagnosisOperatingStateEnumType::normalOperation;
        last_error_message = "";
    }
    auto state = generate_state();
    eebus.usecases->inform_subscribers(entity_address, feature_address_device_diagnosis, state, "deviceDiagnosisStateData");
    update_api();
}

DeviceDiagnosisStateDataType EvseccUsecase::generate_state() const
{
    DeviceDiagnosisStateDataType state{};
    state.lastErrorCode = last_error_message;
    state.operatingState = operating_state;
    return state;
}

DeviceClassificationManufacturerDataType EvseccUsecase::generate_manufacturer_description()
{

    DeviceClassificationManufacturerDataType manufacturer{};
    manufacturer.deviceName = eebus.get_eebus_name().c_str();
    manufacturer.deviceCode = OPTIONS_PRODUCT_NAME_USER_AGENT();
    manufacturer.vendorName = OPTIONS_MANUFACTURER_FULL();
    manufacturer.vendorCode = OPTIONS_HOSTNAME_PREFIX();
    manufacturer.brandName = OPTIONS_CONFIG_TYPE();
    manufacturer.manufacturerLabel = OPTIONS_MANUAL_URL();
    return manufacturer;
}

void EvseccUsecase::update_api() const
{
    auto api_entry = eebus.eebus_usecase_state.get("evse_commissioning_and_configuration");
    api_entry->get("evse_failure")->updateBool(operating_state != DeviceDiagnosisOperatingStateEnumType::normalOperation);
    api_entry->get("evse_failure_description")->updateString(last_error_message.c_str());
}

LpcUsecase::LpcUsecase()
{
    task_scheduler.scheduleOnce([this]() {
                                    // Initialize DeviceConfiguration feature
                                    update_failsafe(EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION, 0_s);

                                    // Initialize DeviceDiagnosis feature
                                    task_scheduler.scheduleUncancelable([this]() {
                                                                              if constexpr (EEBUS_LPC_AWAIT_HEARTBEAT) {
                                                                                  return;
                                                                              }
                                                                              DeviceDiagnosisHeartbeatDataType outgoing_heartbeatData{};
                                                                              outgoing_heartbeatData.heartbeatCounter = heartbeatCounter;
                                                                              outgoing_heartbeatData.heartbeatTimeout = EEBUS_USECASE_HELPERS::iso_duration_to_string(60_s);
                                                                              outgoing_heartbeatData.timestamp = std::to_string(rtc.timestamp_minutes() * 60);

                                                                              eebus.data_handler->devicediagnosisheartbeatdatatype = outgoing_heartbeatData;
                                                                              eebus.data_handler->last_cmd = SpineDataTypeHandler::Function::deviceDiagnosisHeartbeatData;
                                                                              if (eebus.usecases->inform_subscribers(this->entity_address, this->deviceDiagnosis_feature_address, outgoing_heartbeatData, "deviceDiagnosisHeartBeatData") > 0) {
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
                                },
                                1_s); // Schedule all the init stuff a bit delayed to allow other entities to initialize first

}

UseCaseInformationDataType LpcUsecase::get_usecase_information()
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
    lpc_usecase.address = lpc_usecase_feature_address;
    return lpc_usecase;
}

CmdClassifierType LpcUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
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

NodeManagementDetailedDiscoveryEntityInformationType LpcUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EVSE;
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "Controllable System"; // The label of the entity. This is optional but recommended.

    // We focus on returning the mandatory fields.
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> LpcUsecase::get_detailed_discovery_feature_information() const
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

void LpcUsecase::update_failsafe(int power_limit_w, seconds_t duration)
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

void LpcUsecase::update_constraints(int power_consumption_max_w, int power_consumption_contract_max_w)
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

bool LpcUsecase::update_lpc(bool limit_active, int current_limit_w, uint64_t endtime)
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

    eebus.usecases->inform_subscribers(this->entity_address, this->loadControl_feature_address, load_control_limit_description_list, "loadControlLimitDescriptionListData");
    eebus.usecases->inform_subscribers(this->entity_address, this->loadControl_feature_address, load_control_limit_list, "loadControlLimitListData");

    return limit_accepted;
}

CmdClassifierType LpcUsecase::load_control_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
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

CmdClassifierType LpcUsecase::deviceConfiguration_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
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

CmdClassifierType LpcUsecase::device_diagnosis_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
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

CmdClassifierType LpcUsecase::electricalConnection_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.cmdClassifier == CmdClassifierType::read && data->last_cmd == SpineDataTypeHandler::Function::electricalConnectionCharacteristicListData) {
        response["electricalConnectionCharacteristicListData"] = electrical_connection_characteristic_list;
        return CmdClassifierType::reply;
    }
    return CmdClassifierType::EnumUndefined;
}

bool LpcUsecase::switch_state(LPCState state)
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

bool LpcUsecase::init_state()
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

bool LpcUsecase::unlimited_controlled_state()
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

bool LpcUsecase::limited_state()
{
    if (lpc_state == LPCState::Init || lpc_state == LPCState::UnlimitedControlled || lpc_state == LPCState::UnlimitedAutonomous || lpc_state == LPCState::Failsafe) {
        lpc_state = LPCState::Limited;
        task_scheduler.cancel(state_change_timeout);
        current_active_consumption_limit_w = load_control_limit_list.loadControlLimitData->at(0).value->number.get();
        return true;
    }

    return false;
}

bool LpcUsecase::failsafe_state()
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

bool LpcUsecase::unlimited_autonomous_state()
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

void LpcUsecase::update_api()
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

void LpcUsecase::handle_heartbeat_timeout()
{
    if (heartbeat_received || !EEBUS_LPC_AWAIT_HEARTBEAT)
        return;
    eebus.trace_fmtln("Usecase: LPC: Heartbeat Timeout");
    if (lpc_state != LPCState::Failsafe && lpc_state != LPCState::UnlimitedAutonomous)
        switch_state(LPCState::Failsafe);
}

CevcUsecase::CevcUsecase()
{
}

CmdClassifierType CevcUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection)
{
    if (header.cmdClassifier == CmdClassifierType::read) {
        if (data->last_cmd == SpineDataTypeHandler::Function::timeSeriesDescriptionListData) {
            response["timeSeriesDescriptionListData"] = read_time_series_description();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::timeSeriesConstraintsListData) {
            response["timeSeriesConstraintsListData"] = read_time_series_constraints();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::timeSeriesListData) {
            response["timeSeriesListData"] = read_time_series_list();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::incentiveTableDescriptionData) {
            response["incentiveTableDescriptionData"] = read_incentive_table_description();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::incentiveTableConstraintsData) {
            response["incentiveTableConstraintsData"] = read_incentive_table_constraints();
            return CmdClassifierType::reply;
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::incentiveTableData) {
            response["incentiveTableData"] = read_incentive_table_data();;
            return CmdClassifierType::reply;
        }
    }
    // TODO: Implement the rest of this usecase
    return CmdClassifierType::EnumUndefined;
}

UseCaseInformationDataType CevcUsecase::get_usecase_information()
{
    UseCaseInformationDataType cevc_usecase;
    cevc_usecase.actor = "EV";

    UseCaseSupportType cevc_usecase_support;
    cevc_usecase_support.useCaseName = "coordinatedEvCharging";
    cevc_usecase_support.useCaseVersion = "1.0.1";
    cevc_usecase_support.useCaseAvailable = false; // TODO: Set to true when implemented
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
    // The entity type as defined in EEBUS SPINE TS ResourceSpecification 4.2.17
    entity.description->label = "EV"; // The label of the entity. This is optional but recommended.

    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> CevcUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return features;
    }

    // See EEBUS UC TS CoordinatedEvCharging v1.0.1.pdf 3.2.1.2.1

    // The following functions are needed by the TimeSeries Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType timeseries_feature{};
    timeseries_feature.description->featureAddress->entity = entity_address;
    timeseries_feature.description->featureAddress->feature = feature_address_timeseries; // Feature IDs are just arbitrary numbers. Just have to be unique within the entity
    timeseries_feature.description->featureType = FeatureTypeEnumType::TimeSeries;
    timeseries_feature.description->role = RoleType::server;

    // timeSeriesDescriptionListData
    FunctionPropertyType timeseries_description{};
    timeseries_description.function = FunctionEnumType::timeSeriesDescriptionListData;
    timeseries_description.possibleOperations->read = PossibleOperationsReadType{};
    timeseries_feature.description->supportedFunction->push_back(timeseries_description);

    // timeSeriesConstraintsListData
    FunctionPropertyType timeseries_constraints{};
    timeseries_constraints.function = FunctionEnumType::timeSeriesConstraintsListData;
    timeseries_constraints.possibleOperations->read = PossibleOperationsReadType{};
    timeseries_feature.description->supportedFunction->push_back(timeseries_constraints);

    // timeSeriesListData
    FunctionPropertyType timeseries_data{};
    timeseries_data.function = FunctionEnumType::timeSeriesListData;
    timeseries_data.possibleOperations->read = PossibleOperationsReadType{};
    timeseries_data.possibleOperations->write = PossibleOperationsWriteType{};
    timeseries_feature.description->supportedFunction->push_back(timeseries_description);
    features.push_back(timeseries_feature);

    NodeManagementDetailedDiscoveryFeatureInformationType incentive_table_feature{};
    incentive_table_feature.description->featureAddress->entity = entity_address;
    incentive_table_feature.description->featureAddress->feature = feature_address_incentive_table;
    incentive_table_feature.description->featureType = FeatureTypeEnumType::IncentiveTable;
    incentive_table_feature.description->role = RoleType::server;

    // incentiveTableDescriptionData
    FunctionPropertyType incentivetable_description{};
    incentivetable_description.function = FunctionEnumType::incentiveDescriptionListData;
    incentivetable_description.possibleOperations->read = PossibleOperationsReadType{};
    incentivetable_description.possibleOperations->write = PossibleOperationsWriteType{};
    incentive_table_feature.description->supportedFunction->push_back(incentivetable_description);

    // incentiveTableConstraintsData
    FunctionPropertyType incentivetable_constraints{};
    incentivetable_constraints.function = FunctionEnumType::incentiveTableConstraintsData;
    incentivetable_constraints.possibleOperations->read = PossibleOperationsReadType{};
    incentive_table_feature.description->supportedFunction->push_back(incentivetable_constraints);

    // incentiveTableData
    FunctionPropertyType incentivetable_data{};
    incentivetable_data.function = FunctionEnumType::incentiveTableData;
    incentivetable_data.possibleOperations->read = PossibleOperationsReadType{};
    incentivetable_data.possibleOperations->write = PossibleOperationsWriteType{};
    incentive_table_feature.description->supportedFunction->push_back(incentivetable_data);

    features.push_back(incentive_table_feature);

    return features;
}

TimeSeriesDescriptionListDataType CevcUsecase::read_time_series_description() const
{
    // TODO: Implement read_time_series_description
    return {};
}

TimeSeriesConstraintsListDataType CevcUsecase::read_time_series_constraints() const
{
    // TODO: Implement read_time_series_constraints
    return {};
}

TimeSeriesListDataType CevcUsecase::read_time_series_list() const
{
    // TODO: Implement read_time_series_list
    return {};
}

CmdClassifierType CevcUsecase::write_time_series_list(HeaderType &header, TimeSeriesListDataType data, JsonObject response)
{
    // TODO: Implement write_time_series_list
    return CmdClassifierType::EnumUndefined;
}

IncentiveTableDescriptionDataType CevcUsecase::read_incentive_table_description() const
{
    // TODO: Implement read_incentive_table_description
    return {};
}

CmdClassifierType CevcUsecase::write_incentive_table_description(HeaderType &header, IncentiveTableDataType data, JsonObject response)
{
    // TODO: Implement write_incentive_table_description
    return CmdClassifierType::EnumUndefined;
}

IncentiveTableConstraintsDataType CevcUsecase::read_incentive_table_constraints() const
{
    // TODO: Implement read_incentive_table_constraints
    return {};
}

IncentiveTableDataType CevcUsecase::read_incentive_table_data() const
{
    // TODO: Implement read_incentive_table_data
    return {};
}

CmdClassifierType CevcUsecase::write_incentive_table_data(HeaderType &header, IncentiveTableDataType data, JsonObject response)
{
    // TODO: Implement write_incentive_table_data
    return CmdClassifierType::EnumUndefined;
}


EEBusUseCases::EEBusUseCases()
{
    // Entity Addresses should be consistent so all actors are under the same entity
    node_management.set_usecaseManager(this);
    node_management.set_entity_address({0});
    // EVSE Actors
    charging_summary.set_entity_address({1});
    evse_commissioning_and_configuration.set_entity_address({1});
    // EV actors
    ev_commissioning_and_configuration.set_entity_address({1, 1}); // EVCC entity is "under" the ChargingSummary entity and therefore the first value
    coordinate_ev_charging.set_entity_address({1, 1});
    ev_charging_electricity_measurement.set_entity_address({1, 1});
    // Controllable System Actors
    limitation_of_power_consumption.set_entity_address({2});

    initialized = true; // set to true, otherwise subscriptions will not work
}

void EEBusUseCases::handle_message(HeaderType &header, SpineDataTypeHandler *data, SpineConnection *connection)
{
    //TODO: Implement a mutex with waiting so only one message can be processed at a time

    eebus_commands_received++;
    eebus.eebus_usecase_state.get("commands_received")->updateUint(eebus_commands_received);
    BasicJsonDocument<ArduinoJsonPsramAllocator> response_doc{SHIP_TYPES_MAX_JSON_SIZE};
    JsonObject responseObj = response_doc.to<JsonObject>();
    CmdClassifierType send_response = CmdClassifierType::EnumUndefined;
    String entity_name = "Unknown";
    FeatureAddressType source_address = header.addressSource.get();
    FeatureAddressType destination_address = header.addressDestination.get();

    if (data->last_cmd == SpineDataTypeHandler::Function::resultData) {
        eebus.trace_fmtln("Usecases: Received resultData, no further processing");
        ResultDataType result_data = data->resultdatatype.get();
        if (result_data.errorNumber.get() != static_cast<uint8_t>(EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError)) {
            logger.printfln("Usecases: An error was received from the communication Partner. Error Number: %s, Description: %s", EEBUS_USECASE_HELPERS::get_result_error_number_string(result_data.errorNumber.get()).c_str(), result_data.description.get().c_str());
        }
        data->reset();
        return;
    }

    bool found_dest_entity = false;
    for (EebusUsecase *entity : entity_list) {
        if (header.addressDestination->entity.has_value() && entity->matches_entity_address(header.addressDestination->entity.get()) && !found_dest_entity) {
            send_response = entity->handle_message(header, data, responseObj, connection);
            if (send_response != CmdClassifierType::EnumUndefined) {
                found_dest_entity = true;
                eebus.trace_fmtln("Usecases: Found entity: %s", entity->get_entity_name().c_str());
            } else {
                eebus.trace_fmtln("Usecases: Entity %s could not handle the message", entity->get_entity_name().c_str());
            }
        }
    }

    if (!found_dest_entity) {
        eebus.trace_fmtln("Usecases: Received message for unknown entity");
        EEBUS_USECASE_HELPERS::build_result_data(responseObj,
                                                 EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected,
                                                 "Unknown entity requested");
        send_response = CmdClassifierType::result; // We always send a response if we do not know the entity
    }
    if (send_response != CmdClassifierType::EnumUndefined) {
        eebus.trace_fmtln("Usecases: Sending response");
        if (header.ackRequest.has_value() && header.ackRequest.get() && send_response != CmdClassifierType::result) {

            eebus.trace_fmtln("Usecases: Header requested an ack, but sending a non-result response: %d", static_cast<int>(send_response));
        }
        eebus_responses_sent++;
        eebus.eebus_usecase_state.get("commands_sent")->updateUint(eebus_responses_sent);
        //send_spine_message(*header.addressDestination, *header.addressSource, response_doc, send_response);
        connection->send_datagram(response_doc, send_response, destination_address, source_address, false);
    } else {
        if (header.ackRequest.has_value() && header.ackRequest.get()) {
            eebus.trace_fmtln("Usecases: Header requested an ack, but no response was generated");
        }
        eebus.trace_fmtln("Usecases: No response needed. Not sending anything");
    }
    data->reset();
}

template <typename T>
size_t EEBusUseCases::inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, T &data, const char *function_name)
{
    if (initialized && EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS)
        return node_management.inform_subscribers(entity, feature, data, function_name);
    if constexpr (EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS)
        eebus.trace_fmtln("Attempted to inform subscribers while Usecases were not yet initialized");
    else
        eebus.trace_fmtln("Attempted to inform subscribers about a change in %s while subscriptions are disabled", function_name);
    return 0;
}

bool EEBusUseCases::send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, const JsonVariantConst payload, CmdClassifierType cmd_classifier, const bool want_ack)
{
    eebus_responses_sent++;
    eebus.eebus_usecase_state.get("commands_sent")->updateUint(eebus_responses_sent);
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

void build_result_data(JsonObject &response, ResultErrorNumber error_number, const char *description)
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

time_t iso_timestamp_to_unix(const char *iso_timestamp, time_t *t)
{
    iso8601_time time;
    if (iso8601_parse(iso_timestamp, &time) != 0) {
        return false;
    }
    iso8601_to_time_t(&time, t);
    return true;
}

String unix_to_iso_timestamp(time_t unix_time)
{
    tm t;
    gmtime_r(&unix_time, &t);
    char buf[OCPP_ISO_8601_MAX_LEN];
    strftime(buf, OCPP_ISO_8601_MAX_LEN, "%FT%TZ", &t);
    return buf;
}
} // namespace EEBUS_USECASE_HELPERS
