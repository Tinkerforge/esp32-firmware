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

#ifdef EEBUS_ENABLE_EVCEM_USECASE

#include "../module_dependencies.h"
#include "entity_data.h"
#include "evcem.h"
#include "usecase_helpers.h"

#include <array>

// ==============================================================================
// EVCEM - EV Charging Electricity Measurement Use Case
// Spec: EEBus_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf
//
// Scenarios:
//   1 (3.2.1.2.1.1): Current measurement per phase
//   2 (3.2.1.2.1.2): Power measurement per phase
//   3 (3.2.1.2.1.3): Energy measurement (total charged)
// ==============================================================================

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

    // measurementListData
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

#endif // EEBUS_ENABLE_EVCEM_USECASE
