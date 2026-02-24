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

#ifdef EEBUS_ENABLE_MGCP_USECASE

#include "../module_dependencies.h"
#include "entity_data.h"
#include "mgcp.h"
#include "usecase_helpers.h"

extern EEBus eebus;

// ==============================================================================
// MGCP - Monitoring of Grid Connection Point Use Case
// Spec: EEBus_UC_TS_MonitoringOfGridConnectionPoint_V1.0.0.pdf
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

MgcpUsecase::MgcpUsecase()
{
    // Initialize with default values
    task_scheduler.scheduleOnce(
        [this]() {
            update_api();
        },
        1_s); // Schedule init delayed to allow other entities to initialize first

    usecase_actor = "GridConnectionPoint";
    usecase_name = "monitoringOfGridConnectionPoint";
    usecase_version = "1.0.0";
    supported_scenarios = {1, 2, 3, 4, 6, 7};
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
// Data Generator Methods
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
// Update Methods for Each Scenario
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
