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

// Include eebus.h first to get the EEBUS_MODE_* macros (which define EEBUS_ENABLE_* in eebus_usecases.h)
#include "../eebus.h"
#include "../eebus_usecases.h"

#ifdef EEBUS_ENABLE_EVSECC_USECASE

#include "../module_dependencies.h"
#include "entity_data.h"
#include "evsecc.h"
#include "usecase_helpers.h"

// ==============================================================================
// EVSECC - EVSE Commissioning and Configuration Use Case
// Spec: EEBus_UC_TS_EVSECommissioningAndConfiguration_V1.0.1.pdf
//
// Scenarios:
//   1 (2.3.1, 3.4.1): EVSE manufacturer information
//   2 (2.3.2, 3.4.2): EVSE error state
// ==============================================================================

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

    // deviceDiagnosisStateData
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

#endif // EEBUS_ENABLE_EVSECC_USECASE
