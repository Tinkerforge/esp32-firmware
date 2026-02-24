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

#ifdef EEBUS_ENABLE_MPC_USECASE

#include "../module_dependencies.h"
#include "mpc.h"
#include "usecase_helpers.h"

#include <array>

// ==============================================================================
// MPC - Monitoring of Power Consumption Use Case
// Spec: EEBus_UC_TS_MonitoringOfPowerConsumption_V1.0.0.pdf
//
// Scenarios:
//   1 (3.4.1): Monitor Power - Mandatory
//   2 (3.4.2): Monitor Energy - Optional
//   3 (3.4.3): Monitor Current - Recommended
//   4 (3.4.4): Monitor Voltage - Optional
//   5 (3.4.5): Monitor Frequency - Optional
// ==============================================================================

MpcUsecase::MpcUsecase()
{
    // Initialize with default values
    task_scheduler.scheduleOnce(
        [this]() {
            update_api();
        },
        1_s); // Schedule init delayed to allow other entities to initialize first
    usecase_actor = "MonitoredUnit";
    usecase_name = "monitoringOfPowerConsumption";
    usecase_version = "1.0.0";
    supported_scenarios = {1, 2, 3, 4, 5};
}

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

NodeManagementDetailedDiscoveryEntityInformationType MpcUsecase::get_detailed_discovery_entity_information() const
{
    return build_entity_info(EntityTypeEnumType::EVSE, "Monitored Unit");
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> MpcUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    // See EEBUS_UC_TS_MonitoringOfPowerConsumption_v1.0.0.pdf
    // The following functions are needed by the ElectricalConnection Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnectionFeature = build_feature_information(FeatureTypeEnumType::ElectricalConnection);

    // electricalConnectionDescriptionListdata
    electricalConnectionFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::electricalConnectionDescriptionListData));
    electricalConnectionFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::electricalConnectionParameterDescriptionListData));
    features.push_back(electricalConnectionFeature);

    // The following functions are needed by the Measurement Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType measurementFeature = build_feature_information(FeatureTypeEnumType::Measurement);
    measurementFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::measurementDescriptionListData));
    measurementFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::measurementConstraintsListData));
    measurementFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::measurementListData));
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
            bool supported = true;
        };
        const std::array<RelativePhaseAssignment, 6> relative_phase_assignments{{
            {id_p_6_1, id_m_6_1, ElectricalConnectionPhaseNameEnumType::a, ElectricalConnectionPhaseNameEnumType::neutral},
            {id_p_6_2, id_m_6_2, ElectricalConnectionPhaseNameEnumType::b, ElectricalConnectionPhaseNameEnumType::neutral},
            {id_p_6_3, id_m_6_3, ElectricalConnectionPhaseNameEnumType::c, ElectricalConnectionPhaseNameEnumType::neutral},
            {id_p_6_4, id_m_6_4, ElectricalConnectionPhaseNameEnumType::a, ElectricalConnectionPhaseNameEnumType::b, phase_to_phase_available},
            {id_p_6_5, id_m_6_5, ElectricalConnectionPhaseNameEnumType::b, ElectricalConnectionPhaseNameEnumType::c, phase_to_phase_available},
            {id_p_6_6, id_m_6_6, ElectricalConnectionPhaseNameEnumType::c, ElectricalConnectionPhaseNameEnumType::a, phase_to_phase_available},
        }};
        for (const auto &assignment : relative_phase_assignments) {
            if (assignment.supported) {
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
        bool supported = true;
    };
    const std::array<MeasurementDescriptionEntry, 16> measurement_descriptions{{
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
        {id_m_6_2, MeasurementTypeEnumType::voltage, UnitOfMeasurementEnumType::V, ScopeTypeEnumType::acVoltage},
        {id_m_6_3, MeasurementTypeEnumType::voltage, UnitOfMeasurementEnumType::V, ScopeTypeEnumType::acVoltage},
        {id_m_6_4, MeasurementTypeEnumType::voltage, UnitOfMeasurementEnumType::V, ScopeTypeEnumType::acVoltage, phase_to_phase_available},
        {id_m_6_5, MeasurementTypeEnumType::voltage, UnitOfMeasurementEnumType::V, ScopeTypeEnumType::acVoltage, phase_to_phase_available},
        {id_m_6_6, MeasurementTypeEnumType::voltage, UnitOfMeasurementEnumType::V, ScopeTypeEnumType::acVoltage, phase_to_phase_available},
        {id_m_7, MeasurementTypeEnumType::frequency, UnitOfMeasurementEnumType::Hz, ScopeTypeEnumType::acFrequency},
    }};

    for (const auto &entry : measurement_descriptions) {
        if (entry.supported) {
            MeasurementDescriptionDataType measurement_description{};
            measurement_description.measurementId = entry.measurement_id;
            measurement_description.measurementType = entry.measurement_type;
            measurement_description.commodityType = CommodityTypeEnumType::electricity;
            measurement_description.unit = entry.unit;
            measurement_description.scopeType = entry.scope;
            data->measurementDescriptionData->push_back(measurement_description);
        }
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
        bool supported = true;
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
        {id_m_6_4, voltage_limit_min_v, voltage_limit_max_v, voltage_limit_stepsize_v, 0, phase_to_phase_available},
        {id_m_6_5, voltage_limit_min_v, voltage_limit_max_v, voltage_limit_stepsize_v, 0, phase_to_phase_available},
        {id_m_6_6, voltage_limit_min_v, voltage_limit_max_v, voltage_limit_stepsize_v, 0, phase_to_phase_available},
        // Frequency (Hz) - scale -3 (mHz)
        {id_m_7, frequency_limit_min_mhz, frequency_limit_max_mhz, frequency_limit_stepsize_mhz, -3},
    }};

    for (const auto &entry : entries) {
        if (entry.supported) {
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
    if (phase_to_phase_available) {
        for (int i = 0; i < 3; i++) {
            MeasurementDataType m{};
            m.measurementId = id_m_6_4 + i;
            m.valueType = MeasurementValueTypeEnumType::value;
            m.value->number = voltage_phase_to_phase_v[i];
            m.value->scale = 0;
            m.valueSource = MeasurementValueSourceEnumType::measuredValue;
            data->measurementData->push_back(m);
        }
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

    bool phase_to_phase_supported_before = phase_to_phase_available;
    phase_to_phase_available = !(voltage_phase_1_2 == INT32_MIN || voltage_phase_2_3 == INT32_MIN || voltage_phase_3_1 == INT32_MIN);
    voltage_phase_to_phase_v[0] = voltage_phase_1_2;
    voltage_phase_to_phase_v[1] = voltage_phase_2_3;
    voltage_phase_to_phase_v[2] = voltage_phase_3_1;
    if (phase_to_phase_supported_before != phase_to_phase_available) {
        ElectricalConnectionParameterDescriptionListDataType ec_parameter_description_data = EVSEEntity::get_electrical_connection_parameter_description_list_data();
        eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::ElectricalConnection), ec_parameter_description_data, "electricalConnectionParameterDescriptionListData");

        MeasurementConstraintsListDataType constraints_data = EVSEEntity::get_measurement_constraints_list_data();
        eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), constraints_data, "measurementConstraintsListData");

        MeasurementDescriptionListDataType measurement_description_data = EVSEEntity::get_measurement_description_list_data();
        eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::Measurement), measurement_description_data, "measurementDescriptionListData");
    }

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

#endif // EEBUS_ENABLE_MPC_USECASE
