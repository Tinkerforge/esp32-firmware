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

#ifdef EEBUS_ENABLE_OPEV_USECASE

#include "../module_dependencies.h"
#include "entity_data.h"
#include "event_log_prefix.h"
#include "opev.h"
#include "usecase_helpers.h"

extern EEBus eebus;

// ==============================================================================
// OPEV - Overload Protection by EV Charging Current Curtailment Use Case
// Spec: EEBus_UC_TS_OverloadProtectionByEVChargingCurrentCurtailment_V1.0.1b.pdf
//
// Scenarios:
//   1 (3.4.1): EV provides limitations and writes current limit
//   2 (3.4.2): EG sends heartbeat
//   3 (3.4.3): EG sends error state
//
// Entity Type: EV
// Actor: EV
// Features: LoadControl, ElectricalConnection
// ==============================================================================

OpevUsecase::OpevUsecase()
{
    usecase_actor = "EV";
    usecase_name = "overloadProtectionByEvChargingCurrentCurtailment";
    usecase_version = "1.0.1";
    supported_scenarios = {1, 2, 3};
}

MessageReturn OpevUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    AddressFeatureType feature_address = header.addressDestination->feature.get();
    switch (data->last_cmd) {
        // 3.2.1.2.1.1: Function "loadControlLimitDescriptionListData" - Current limit metadata
        case SpineDataTypeHandler::Function::loadControlLimitDescriptionListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::LoadControl)) {
                response["loadControlLimitDescriptionListData"] = EVEntity::get_load_control_limit_description_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        case SpineDataTypeHandler::Function::loadControlLimitConstraintsListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::LoadControl)) {
                response["loadControlLimitConstraintsListData"] = EVEntity::get_load_control_limit_constraints_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        // 3.2.1.2.1.2: Function "loadControlLimitListData" - Per-phase current limits
        case SpineDataTypeHandler::Function::loadControlLimitListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::LoadControl)) {
                if (header.cmdClassifier == CmdClassifierType::read) {
                    response["loadControlLimitListData"] = EVEntity::get_load_control_limit_list_data();
                    return {true, true, CmdClassifierType::reply};
                }
                if (header.cmdClassifier == CmdClassifierType::write) {
                    return write_load_control_limit_list_data(header, data->loadcontrollimitlistdatatype, response);
                }
            }
            break;
        // 3.2.1.2.2: Function "measurementListData" - Current measurements (via EVCEM)
        case SpineDataTypeHandler::Function::measurementListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::Measurement)) {
                response["measurementListData"] = EVEntity::get_measurement_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        // 3.2.1.2.3.1: Function "electricalConnectionParameterDescriptionListData"
        case SpineDataTypeHandler::Function::electricalConnectionParameterDescriptionListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::ElectricalConnection)) {
                response["electricalConnectionParameterDescriptionListData"] = EVEntity::get_electrical_connection_parameter_description_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        // 3.2.1.2.3.2: Function "electricalConnectionPermittedValueSetListData" - Permitted current values
        case SpineDataTypeHandler::Function::electricalConnectionPermittedValueSetListData:
            if (feature_address == feature_addresses.at(FeatureTypeEnumType::ElectricalConnection)) {
                response["electricalConnectionPermittedValueSetListData"] = EVEntity::get_electrical_connection_permitted_list_data();
                return {true, true, CmdClassifierType::reply};
            }
            break;
        default:;
    }
    return {false};
}

NodeManagementDetailedDiscoveryEntityInformationType OpevUsecase::get_detailed_discovery_entity_information() const
{
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return {};
    }
    return build_entity_info(EntityTypeEnumType::EV, "EV");
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> OpevUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return features;
    }

    // The following functions are needed by the LoadControl Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType loadControlFeature = build_feature_information(FeatureTypeEnumType::LoadControl);

    // loadControlLimitDescriptionListData
    loadControlFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::loadControlLimitDescriptionListData));

    // loadControlLimitListData
    loadControlFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::loadControlLimitListData, true));

    features.push_back(loadControlFeature);

    // The following functions are needed by the ElectricalConnection Feature Type
    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnectionFeature = build_feature_information(FeatureTypeEnumType::ElectricalConnection);

    // electricalConnectionParameterDescriptionListData
    electricalConnectionFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::electricalConnectionParameterDescriptionListData));

    // electricalConnectionPermittedValueSetListData
    electricalConnectionFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::electricalConnectionPermittedValueSetListData));

    features.push_back(electricalConnectionFeature);

    return features;
}

// Spec 3.2.1.2.1.1: Load control limit descriptions for per-phase current limits
// Creates 3 limit IDs (id_x_1, id_x_2, id_x_3) for phases A, B, C
// Limit type: maxValueLimit, Category: obligation, Scope: overloadProtection
void OpevUsecase::get_load_control_limit_description_list_data(LoadControlLimitDescriptionListDataType *data)
{
    const std::array<std::pair<uint8_t, uint8_t>, 3> id_pairs{{
        {id_x_1, id_z_1}, // Phase A: limit id -> measurement id
        {id_x_2, id_z_2}, // Phase B
        {id_x_3, id_z_3}, // Phase C
    }};

    for (const auto &p : id_pairs) {
        LoadControlLimitDescriptionDataType loadControlLimitDescriptionData{};
        loadControlLimitDescriptionData.limitId = p.first;
        loadControlLimitDescriptionData.limitType = LoadControlLimitTypeEnumType::maxValueLimit;
        loadControlLimitDescriptionData.limitCategory = LoadControlCategoryEnumType::obligation;
        loadControlLimitDescriptionData.measurementId = p.second; // Link to current measurement
        loadControlLimitDescriptionData.unit = UnitOfMeasurementEnumType::A;
        loadControlLimitDescriptionData.scopeType = ScopeTypeEnumType::overloadProtection;
        data->loadControlLimitDescriptionData->push_back(loadControlLimitDescriptionData);
    }
}

// Spec 3.2.1.2.1.2: Current limit values per phase (read)
// Returns per-phase current limits in milliamps (scale=-3)
void OpevUsecase::get_load_control_limit_list_data(LoadControlLimitListDataType *data) const
{
    LoadControlLimitConstraintsListDataType loadControlLimitConstraintsListData{};

    constexpr std::array<uint8_t, 3> ids_used{{
        id_x_1, // Phase A
        id_x_2, // Phase B
        id_x_3, // Phase C
    }};
    for (size_t i = 0; i < ids_used.size(); ++i) {
        uint8_t p = ids_used[i];
        LoadControlLimitDataType loadControlLimitData{};
        loadControlLimitData.limitId = p;
        loadControlLimitData.isLimitChangeable = limit_changeable();
        loadControlLimitData.isLimitActive = limit_active;
        loadControlLimitData.value->number = limit_per_phase_milliamps[i];
        loadControlLimitData.value->scale = -3; // Milliamps: value * 10^-3 = A
        data->loadControlLimitData->push_back(loadControlLimitData);
    }
}

void OpevUsecase::get_electrical_connection_parameter_description_list_data(ElectricalConnectionParameterDescriptionListDataType *data)
{
    struct Entrydata {
        uint8_t param_id;
        uint8_t measure_id;
        ElectricalConnectionPhaseNameEnumType phase;
    };

    constexpr std::array<Entrydata, 3> entrydata{{
        {id_i_1, id_z_1, ElectricalConnectionPhaseNameEnumType::a},
        {id_i_2, id_z_2, ElectricalConnectionPhaseNameEnumType::b},
        {id_i_3, id_z_3, ElectricalConnectionPhaseNameEnumType::c},
    }};
    for (const auto &entry : entrydata) {
        ElectricalConnectionParameterDescriptionDataType parameter_description{};
        parameter_description.electricalConnectionId = id_j_1;
        parameter_description.parameterId = entry.param_id;
        parameter_description.measurementId = entry.measure_id;
        parameter_description.acMeasuredPhases = entry.phase;

        data->electricalConnectionParameterDescriptionData->push_back(parameter_description);
    }
}

void OpevUsecase::get_electrical_connection_permitted_list_data(ElectricalConnectionPermittedValueSetListDataType *data) const
{
    constexpr std::array<uint8_t, 3> ids_used{{
        id_i_1,
        id_i_2,
        id_i_3,
    }};
    for (const auto &id : ids_used) {
        ElectricalConnectionPermittedValueSetDataType permittedValueSetData{};
        permittedValueSetData.electricalConnectionId = id_j_1;
        permittedValueSetData.parameterId = id;
        ScaledNumberSetType permittedValueSet{};
        ScaledNumberRangeType range{};
        range.min->number = limit_milliamps_min;
        range.min->scale = -3;
        range.max->number = limit_milliamps_max;
        range.max->scale = -3;
        permittedValueSet.range->push_back(range);
        permittedValueSetData.permittedValueSet->push_back(permittedValueSet);
        data->electricalConnectionPermittedValueSetData->push_back(permittedValueSetData);
    }
}

void OpevUsecase::update_limits(int limit_phase_1_milliamps, int limit_phase_2_milliamps, int limit_phase_3_milliamps, bool active)
{
    // TODO: check if limits can be applied and apply them to the evse
    bool notify_subs = (limit_active != active);
    // Phase A
    if (limit_phase_1_milliamps >= 0) {
        if (limit_per_phase_milliamps[0] != limit_phase_1_milliamps) {
            notify_subs = true;
        }
        limit_per_phase_milliamps[0] = limit_phase_1_milliamps;
    }
    // Phase B
    if (limit_phase_2_milliamps >= 0) {
        if (limit_per_phase_milliamps[1] != limit_phase_2_milliamps) {
            notify_subs = true;
        }
        limit_per_phase_milliamps[1] = limit_phase_2_milliamps;
    }
    // Phase C
    if (limit_phase_3_milliamps >= 0) {
        if (limit_per_phase_milliamps[2] != limit_phase_3_milliamps) {
            notify_subs = true;
        }
        limit_per_phase_milliamps[2] = limit_phase_3_milliamps;
    }

    limit_active = active;

    if (notify_subs) {
        auto limit_list_data = EVEntity::get_load_control_limit_list_data();
        auto limit_permitted_data = EVEntity::get_electrical_connection_permitted_list_data();
        logger.printfln("OPEV: New limits received: L1: %d mA, L2: %d mA, L3: %d mA, active: %s", limit_per_phase_milliamps[0], limit_per_phase_milliamps[1], limit_per_phase_milliamps[2], limit_active ? "true" : "false");
        eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::LoadControl), limit_list_data, "loadControlLimitListData");
        eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::ElectricalConnection), limit_permitted_data, "electricalConnectionPermittedValueSetListData");
    }
}

bool OpevUsecase::limit_changeable() const
{
#ifdef EEBUS_ENABLE_LPC_USECASE
    if (eebus.usecases->limitation_of_power_consumption.limit_is_active())
        return false;
#endif
    return limit_changeable_allowed;
}

MessageReturn OpevUsecase::write_load_control_limit_list_data(HeaderType &header, SpineOptional<LoadControlLimitListDataType> data, JsonObject response)
{
    if (!data.has_value()) {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "No load control limit data provided");
        return {true, true, CmdClassifierType::result};
    }
    int limit_phase_a = -1;
    int limit_phase_b = -1;
    int limit_phase_c = -1;
    bool limit_enabled = false;

    for (const auto &limit_data : data->loadControlLimitData.get()) {
        int limit_value_milliamps = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(limit_data.value.get()) * 1000;

        switch (limit_data.limitId.get()) {
            case id_x_1:
                limit_phase_a = limit_value_milliamps;
                limit_enabled = limit_active || limit_data.isLimitActive.get();
                break;
            case id_x_2:
                limit_phase_b = limit_value_milliamps;
                limit_enabled = limit_active || limit_data.isLimitActive.get();
                break;
            case id_x_3:
                limit_phase_c = limit_value_milliamps;
                limit_enabled = limit_active || limit_data.isLimitActive.get();
                break;
            default:
                break;
        }
    }
    // Check if received limits are within min/max defined in electricalConnectionPermittedValueSetListData
    auto check_limit = [this](int limit_milliamps) -> bool {
        return (limit_milliamps < 0) || // -1 means "not set", always valid
               (limit_milliamps >= limit_milliamps_min && limit_milliamps <= limit_milliamps_max);
    };

    if (!check_limit(limit_phase_a) || !check_limit(limit_phase_b) || !check_limit(limit_phase_c)) {
        logger.printfln("OPEV: Received limit out of range (min: %d mA, max: %d mA). L1: %d mA, L2: %d mA, L3: %d mA", limit_milliamps_min, limit_milliamps_max, limit_phase_a, limit_phase_b, limit_phase_c);
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Limit value outside permitted range");
        return {true, true, CmdClassifierType::result};
    }
    update_limits(limit_phase_a, limit_phase_b, limit_phase_c, limit_enabled);
    return {true, false, CmdClassifierType::reply};
}

#endif // EEBUS_ENABLE_OPEV_USECASE
