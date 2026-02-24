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

#ifdef EEBUS_ENABLE_EVCC_USECASE

#include "../module_dependencies.h"
#include "entity_data.h"
#include "evcc.h"
#include "event_log_prefix.h"
#include "usecase_helpers.h"

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

EvccUsecase::EvccUsecase()
{
    entity_active = false; // Disable entity until an EV is connected
    ev_connected = false;

    usecase_actor = "EV";
    usecase_name = "evCommissioningAndConfiguration";
    usecase_version = "1.0.1";
    // All 8 scenarios supported (see spec chapter 2.3)
    supported_scenarios = {1, 2, 3, 4, 5, 6, 7, 8};
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

#endif // EEBUS_ENABLE_EVCC_USECASE
