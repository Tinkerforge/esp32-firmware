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

#ifdef EEBUS_ENABLE_CEVC_USECASE

#include "../module_dependencies.h"
#include "../spine_connection.h"
#include "cevc.h"
#include "usecase_helpers.h"

#include <ctime>
#include <string>

// ==============================================================================
// CEVC - Coordinated EV Charging Use Case
// Spec: EEBus_UC_TS_CoordinatedEVCharging_V1.0.1.pdf
//
// Scenarios:
//   1 (3.4.1): EV sends charging energy demand - TimeSeries singleDemand - COMPLETE
//   2 (3.4.2): Max power limitation - TimeSeries constraints - COMPLETE
//   3 (3.4.3): Incentive table - IncentiveTable for tariff data - COMPLETE
//   4 (3.4.4): Charging plan curve - TimeSeries plan (write) - COMPLETE
//   5 (3.4.5): Energy Guard heartbeat - Via EebusHeartBeat - COMPLETE
//   6 (3.4.6): Energy Broker heartbeat - Via EebusHeartBeat - COMPLETE
//   7 (3.4.7): Energy Guard error state - Via DeviceDiagnosis - COMPLETE
//   8 (3.4.8): Energy Broker error state - Via DeviceDiagnosis - COMPLETE
// ==============================================================================

CevcUsecase::CevcUsecase()
{
    usecase_actor = "EV";
    usecase_name = "coordinatedEvCharging";
    usecase_version = "1.0.1";
    supported_scenarios = {1, 2, 3, 4, 5, 6, 7, 8};
}

MessageReturn CevcUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    CmdClassifierType cmd = header.cmdClassifier.get();

    switch (data->last_cmd) {
        // =====================================================================
        // TimeSeries Feature - 3.2.1.2.1
        // =====================================================================

        // 3.2.1.2.1.1: Function "timeSeriesDescriptionListData" - Time series metadata
        case SpineDataTypeHandler::Function::timeSeriesDescriptionListData:
            if (cmd == CmdClassifierType::read) {
                TimeSeriesDescriptionListDataType ts_desc;
                read_time_series_description(&ts_desc);
                response["timeSeriesDescriptionListData"] = ts_desc;
                return {true, true, CmdClassifierType::reply};
            }
            break;

        // 3.2.1.2.1.2: Function "timeSeriesConstraintsListData" - Time series constraints
        case SpineDataTypeHandler::Function::timeSeriesConstraintsListData:
            if (cmd == CmdClassifierType::read) {
                TimeSeriesConstraintsListDataType ts_constraints;
                read_time_series_constraints(&ts_constraints);
                response["timeSeriesConstraintsListData"] = ts_constraints;
                return {true, true, CmdClassifierType::reply};
            }
            break;

        // 3.2.1.2.1.3: Function "timeSeriesListData" - Actual charging demand/plan data
        case SpineDataTypeHandler::Function::timeSeriesListData:
            if (cmd == CmdClassifierType::read) {
                TimeSeriesListDataType ts_list;
                read_time_series_list(&ts_list);
                response["timeSeriesListData"] = ts_list;
                return {true, true, CmdClassifierType::reply};
            }
            if (cmd == CmdClassifierType::write) {
                return write_time_series_list(header, data->timeserieslistdatatype, response);
            }
            break;

        // =====================================================================
        // IncentiveTable Feature - 3.2.1.2.2
        // =====================================================================

        // 3.2.1.2.2.1: Function "incentiveTableDescriptionData" - Tariff metadata
        case SpineDataTypeHandler::Function::incentiveTableDescriptionData:
            if (cmd == CmdClassifierType::read) {
                IncentiveTableDescriptionDataType it_desc;
                read_incentive_table_description(&it_desc);
                response["incentiveTableDescriptionData"] = it_desc;
                return {true, true, CmdClassifierType::reply};
            }
            if (cmd == CmdClassifierType::write) {
                return write_incentive_table_description(header, data->incentivetabledescriptiondatatype, response);
            }
            break;

        // 3.2.1.2.2.2: Function "incentiveTableConstraintsData" - Tariff constraints
        case SpineDataTypeHandler::Function::incentiveTableConstraintsData:
            if (cmd == CmdClassifierType::read) {
                IncentiveTableConstraintsDataType it_constraints;
                read_incentive_table_constraints(&it_constraints);
                response["incentiveTableConstraintsData"] = it_constraints;
                return {true, true, CmdClassifierType::reply};
            }
            break;

        // 3.2.1.2.2.3: Function "incentiveTableData" - Actual tariff/incentive data
        case SpineDataTypeHandler::Function::incentiveTableData:
            if (cmd == CmdClassifierType::read) {
                IncentiveTableDataType it_data;
                read_incentive_table_data(&it_data);
                response["incentiveTableData"] = it_data;
                return {true, true, CmdClassifierType::reply};
            }
            if (cmd == CmdClassifierType::write) {
                return write_incentive_table_data(header, data->incentivetabledatatype, response);
            }
            break;

        default:
            break;
    }
    return {false};
}

NodeManagementDetailedDiscoveryEntityInformationType CevcUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return entity;
    }
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EV;
    entity.description->label = "EV";
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> CevcUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return features;
    }

    // =========================================================================
    // TimeSeries Feature - See EEBUS UC TS CoordinatedEvCharging v1.0.1.pdf 3.2.1.2.1
    // =========================================================================
    NodeManagementDetailedDiscoveryFeatureInformationType timeseries_feature{};
    timeseries_feature.description->featureAddress->entity = entity_address;
    timeseries_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::TimeSeries);
    timeseries_feature.description->featureType = FeatureTypeEnumType::TimeSeries;
    timeseries_feature.description->role = RoleType::server;

    // timeSeriesDescriptionListData - read only
    FunctionPropertyType timeseries_description{};
    timeseries_description.function = FunctionEnumType::timeSeriesDescriptionListData;
    timeseries_description.possibleOperations->read = PossibleOperationsReadType{};
    timeseries_feature.description->supportedFunction->push_back(timeseries_description);

    // timeSeriesConstraintsListData - read only
    FunctionPropertyType timeseries_constraints{};
    timeseries_constraints.function = FunctionEnumType::timeSeriesConstraintsListData;
    timeseries_constraints.possibleOperations->read = PossibleOperationsReadType{};
    timeseries_feature.description->supportedFunction->push_back(timeseries_constraints);

    // timeSeriesListData - read and write (write for receiving charging plan)
    FunctionPropertyType timeseries_data{};
    timeseries_data.function = FunctionEnumType::timeSeriesListData;
    timeseries_data.possibleOperations->read = PossibleOperationsReadType{};
    timeseries_data.possibleOperations->write = PossibleOperationsWriteType{};
    timeseries_feature.description->supportedFunction->push_back(timeseries_data);
    features.push_back(timeseries_feature);

    // =========================================================================
    // IncentiveTable Feature - See EEBUS UC TS CoordinatedEvCharging v1.0.1.pdf 3.2.1.2.2
    // =========================================================================
    NodeManagementDetailedDiscoveryFeatureInformationType incentive_table_feature{};
    incentive_table_feature.description->featureAddress->entity = entity_address;
    incentive_table_feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::IncentiveTable);
    incentive_table_feature.description->featureType = FeatureTypeEnumType::IncentiveTable;
    incentive_table_feature.description->role = RoleType::server;

    // incentiveTableDescriptionData - read and write
    FunctionPropertyType incentivetable_description{};
    incentivetable_description.function = FunctionEnumType::incentiveTableDescriptionData;
    incentivetable_description.possibleOperations->read = PossibleOperationsReadType{};
    incentivetable_description.possibleOperations->write = PossibleOperationsWriteType{};
    incentive_table_feature.description->supportedFunction->push_back(incentivetable_description);

    // incentiveTableConstraintsData - read only
    FunctionPropertyType incentivetable_constraints{};
    incentivetable_constraints.function = FunctionEnumType::incentiveTableConstraintsData;
    incentivetable_constraints.possibleOperations->read = PossibleOperationsReadType{};
    incentive_table_feature.description->supportedFunction->push_back(incentivetable_constraints);

    // incentiveTableData - read and write
    FunctionPropertyType incentivetable_data{};
    incentivetable_data.function = FunctionEnumType::incentiveTableData;
    incentivetable_data.possibleOperations->read = PossibleOperationsReadType{};
    incentivetable_data.possibleOperations->write = PossibleOperationsWriteType{};
    incentive_table_feature.description->supportedFunction->push_back(incentivetable_data);
    features.push_back(incentive_table_feature);

    return features;
}

// =============================================================================
// TimeSeries Feature Implementation
// =============================================================================

// CEVC Spec 3.2.1.2.1.1: Time series description - describes available time series
void CevcUsecase::read_time_series_description(TimeSeriesDescriptionListDataType *data) const
{
    // We provide three time series:
    // 1. singleDemand - EV's charging energy demand (Scenario 1)
    // 2. constraints - Maximum power limits (Scenario 2)
    // 3. plan - Charging plan from Energy Broker (Scenario 4, writable)

    // Time Series 1: Single Demand (Scenario 1)
    // Describes the EV's overall charging energy demand
    TimeSeriesDescriptionDataType demand_desc{};
    demand_desc.timeSeriesId = TIME_SERIES_ID_DEMAND;
    demand_desc.timeSeriesType = TimeSeriesTypeEnumType::singleDemand;
    demand_desc.timeSeriesWriteable = false; // EV provides this, not writable by broker
    demand_desc.updateRequired = false;
    demand_desc.unit = UnitOfMeasurementEnumType::Wh;
    demand_desc.label = "Charging Demand";
    demand_desc.description = "EV charging energy demand";
    data->timeSeriesDescriptionData->push_back(demand_desc);

    // Time Series 2: Constraints (Scenario 2)
    // Describes the maximum power the EV can consume over time
    TimeSeriesDescriptionDataType constraints_desc{};
    constraints_desc.timeSeriesId = TIME_SERIES_ID_CONSTRAINTS;
    constraints_desc.timeSeriesType = TimeSeriesTypeEnumType::constraints;
    constraints_desc.timeSeriesWriteable = false; // EV provides this
    constraints_desc.updateRequired = false;
    constraints_desc.unit = UnitOfMeasurementEnumType::W;
    constraints_desc.label = "Power Constraints";
    constraints_desc.description = "Maximum power consumption constraints";
    data->timeSeriesDescriptionData->push_back(constraints_desc);

    // Time Series 3: Plan (Scenario 4)
    // The charging plan received from the Energy Broker
    TimeSeriesDescriptionDataType plan_desc{};
    plan_desc.timeSeriesId = TIME_SERIES_ID_PLAN;
    plan_desc.timeSeriesType = TimeSeriesTypeEnumType::plan;
    plan_desc.timeSeriesWriteable = true; // Broker can write the plan
    plan_desc.updateRequired = true;      // Plan should be updated regularly
    plan_desc.unit = UnitOfMeasurementEnumType::W;
    plan_desc.label = "Charging Plan";
    plan_desc.description = "Charging power plan from Energy Broker";
    data->timeSeriesDescriptionData->push_back(plan_desc);
}

// CEVC Spec 3.2.1.2.1.2: Time series constraints - slot count, duration, value ranges
void CevcUsecase::read_time_series_constraints(TimeSeriesConstraintsListDataType *data) const
{
    // Constraints for singleDemand time series
    TimeSeriesConstraintsDataType demand_constraints{};
    demand_constraints.timeSeriesId = TIME_SERIES_ID_DEMAND;
    demand_constraints.slotCountMin = "1";
    demand_constraints.slotCountMax = "1"; // Single demand is just one value
    // Value constraints for energy in Wh
    demand_constraints.slotValueMin->number = 0;
    demand_constraints.slotValueMin->scale = 0;
    demand_constraints.slotValueMax->number = 100000; // 100 kWh max
    demand_constraints.slotValueMax->scale = 0;
    data->timeSeriesConstraintsData->push_back(demand_constraints);

    // Constraints for constraints time series
    TimeSeriesConstraintsDataType power_constraints{};
    power_constraints.timeSeriesId = TIME_SERIES_ID_CONSTRAINTS;
    power_constraints.slotCountMin = "1";
    power_constraints.slotCountMax = std::to_string(MAX_TIME_SERIES_SLOTS);
    power_constraints.slotDurationMin = "PT1M";  // Minimum 1 minute per slot
    power_constraints.slotDurationMax = "PT24H"; // Maximum 24 hours per slot
    // Value constraints for power in W
    power_constraints.slotValueMin->number = 0;
    power_constraints.slotValueMin->scale = 0;
    power_constraints.slotValueMax->number = 22000; // 22 kW max
    power_constraints.slotValueMax->scale = 0;
    data->timeSeriesConstraintsData->push_back(power_constraints);

    // Constraints for plan time series
    TimeSeriesConstraintsDataType plan_constraints{};
    plan_constraints.timeSeriesId = TIME_SERIES_ID_PLAN;
    plan_constraints.slotCountMin = "1";
    plan_constraints.slotCountMax = std::to_string(MAX_TIME_SERIES_SLOTS);
    plan_constraints.slotDurationMin = "PT1M";  // Minimum 1 minute per slot
    plan_constraints.slotDurationMax = "PT24H"; // Maximum 24 hours per slot
    // Value constraints for power in W
    plan_constraints.slotValueMin->number = 0;
    plan_constraints.slotValueMin->scale = 0;
    plan_constraints.slotValueMax->number = 22000; // 22 kW max
    plan_constraints.slotValueMax->scale = 0;
    data->timeSeriesConstraintsData->push_back(plan_constraints);
}

// CEVC Spec 3.2.1.2.1.3: Time series data - actual charging demand and constraints
void CevcUsecase::read_time_series_list(TimeSeriesListDataType *data) const
{
    // =========================================================================
    // Time Series 1: Single Demand (Scenario 1)
    // =========================================================================
    TimeSeriesDataType demand_data{};
    demand_data.timeSeriesId = TIME_SERIES_ID_DEMAND;

    // Set the time period for the demand (now until target departure)
    if (target_departure_time > 0) {
        time_t now = time(nullptr);
        demand_data.timePeriod->startTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(now).c_str();
        demand_data.timePeriod->endTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(target_departure_time).c_str();
    }

    // Create demand slot with min/opt/max energy values
    TimeSeriesSlotType demand_slot{};
    demand_slot.timeSeriesSlotId = 1;
    // Use minValue for minimum energy, value for optimal, maxValue for maximum
    demand_slot.minValue->number = minimum_energy_wh;
    demand_slot.minValue->scale = 0;
    demand_slot.value->number = optimal_energy_wh;
    demand_slot.value->scale = 0;
    demand_slot.maxValue->number = maximum_energy_wh;
    demand_slot.maxValue->scale = 0;
    demand_data.timeSeriesSlot->push_back(demand_slot);
    data->timeSeriesData->push_back(demand_data);

    // =========================================================================
    // Time Series 2: Constraints (Scenario 2)
    // =========================================================================
    TimeSeriesDataType constraints_data{};
    constraints_data.timeSeriesId = TIME_SERIES_ID_CONSTRAINTS;

    if (power_constraint_slots.empty()) {
        // If no detailed slots, provide a single constraint with max power
        TimeSeriesSlotType constraint_slot{};
        constraint_slot.timeSeriesSlotId = 1;
        constraint_slot.maxValue->number = max_power_constraint_w;
        constraint_slot.maxValue->scale = 0;
        constraints_data.timeSeriesSlot->push_back(constraint_slot);
    } else {
        // Provide detailed constraint slots
        int slot_id = 1;
        for (const auto &slot : power_constraint_slots) {
            TimeSeriesSlotType constraint_slot{};
            constraint_slot.timeSeriesSlotId = slot_id++;
            constraint_slot.duration = EEBUS_USECASE_HELPERS::iso_duration_to_string(slot.duration);
            constraint_slot.maxValue->number = slot.max_power_w;
            constraint_slot.maxValue->scale = 0;
            constraints_data.timeSeriesSlot->push_back(constraint_slot);
        }
    }
    data->timeSeriesData->push_back(constraints_data);

    // =========================================================================
    // Time Series 3: Plan (Scenario 4) - only if we have received a plan
    // =========================================================================
    if (charging_plan_valid && !received_charging_plan.empty()) {
        TimeSeriesDataType plan_data{};
        plan_data.timeSeriesId = TIME_SERIES_ID_PLAN;

        if (charging_plan_start_time > 0) {
            plan_data.timePeriod->startTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(charging_plan_start_time).c_str();
        }

        int slot_id = 1;
        for (const auto &slot : received_charging_plan) {
            TimeSeriesSlotType plan_slot{};
            plan_slot.timeSeriesSlotId = slot_id++;
            plan_slot.duration = EEBUS_USECASE_HELPERS::iso_duration_to_string(slot.duration);
            plan_slot.value->number = slot.power_w;
            plan_slot.value->scale = 0;
            plan_data.timeSeriesSlot->push_back(plan_slot);
        }
        data->timeSeriesData->push_back(plan_data);
    }
}

// CEVC Spec 3.4.4: Receive charging plan from Energy Broker
MessageReturn CevcUsecase::write_time_series_list(HeaderType &header, SpineOptional<TimeSeriesListDataType> data, JsonObject response)
{
    if (!data.has_value() || !data->timeSeriesData.has_value()) {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Invalid time series data");
        return {true, true, CmdClassifierType::result};
    }

    bool plan_updated = false;

    for (const auto &ts : data->timeSeriesData.get()) {
        // We only accept writes to the plan time series (ID 3)
        if (!ts.timeSeriesId.has_value()) {
            continue;
        }

        int ts_id = ts.timeSeriesId.get();
        if (ts_id != TIME_SERIES_ID_PLAN) {
            eebus.trace_fmtln("CEVC: Ignoring write to time series ID %d (only plan ID %d is writable)", ts_id, TIME_SERIES_ID_PLAN);
            continue;
        }

        // Parse the charging plan
        received_charging_plan.clear();
        charging_plan_valid = false;

        // Extract start time if provided
        if (ts.timePeriod.has_value() && ts.timePeriod->startTime.has_value()) {
            EEBUS_USECASE_HELPERS::iso_timestamp_to_unix(ts.timePeriod->startTime->c_str(), &charging_plan_start_time);
        } else {
            charging_plan_start_time = time(nullptr);
        }

        // Parse each slot
        if (ts.timeSeriesSlot.has_value()) {
            for (const auto &slot : ts.timeSeriesSlot.get()) {
                ChargingPlanSlot plan_slot{};

                // Parse duration
                if (slot.duration.has_value()) {
                    plan_slot.duration = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(slot.duration.get());
                } else {
                    plan_slot.duration = 900_s; // Default 15 minutes if not specified
                }

                // Parse power value
                if (slot.value.has_value()) {
                    plan_slot.power_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(slot.value.get());
                } else if (slot.maxValue.has_value()) {
                    // If no value, use maxValue as the target
                    plan_slot.power_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(slot.maxValue.get());
                }

                received_charging_plan.push_back(plan_slot);

                // Limit to max slots
                if (received_charging_plan.size() >= MAX_TIME_SERIES_SLOTS) {
                    break;
                }
            }
        }

        if (!received_charging_plan.empty()) {
            charging_plan_valid = true;
            plan_updated = true;
            eebus.trace_fmtln("CEVC: Received charging plan with %d slots", received_charging_plan.size());
        }
    }

    if (plan_updated) {
        notify_subscribers_time_series();
        update_api_state();
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
    } else {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "No valid plan data found");
    }

    return {true, true, CmdClassifierType::result};
}

// =============================================================================
// IncentiveTable Feature Implementation
// =============================================================================

// CEVC Spec 3.2.1.2.2.1: Incentive table description - tariff metadata
void CevcUsecase::read_incentive_table_description(IncentiveTableDescriptionDataType *data) const
{
    IncentiveTableDescriptionType desc{};

    // Tariff description
    TariffDescriptionDataType tariff_desc{};
    tariff_desc.tariffId = TARIFF_ID;
    tariff_desc.tariffWriteable = true; // Broker can update tariffs
    tariff_desc.updateRequired = true;  // Should be updated regularly
    tariff_desc.scopeType = ScopeTypeEnumType::simpleIncentiveTable;
    tariff_desc.label = "Energy Tariff";
    tariff_desc.description = "Energy pricing for charging optimization";
    desc.tariffDescription = tariff_desc;

    // Tier description - we support one dynamic cost tier
    IncentiveTableDescriptionTierType tier{};

    TierDescriptionDataType tier_desc{};
    tier_desc.tierId = TIER_ID;
    tier_desc.tierType = TierTypeEnumType::dynamicCost;
    tier_desc.label = "Dynamic Pricing";
    tier.tierDescription = tier_desc;

    // Boundary description - power-based boundaries
    TierBoundaryDescriptionDataType boundary_desc{};
    boundary_desc.boundaryId = 1;
    boundary_desc.boundaryType = TierBoundaryTypeEnumType::powerBoundary;
    boundary_desc.boundaryUnit = UnitOfMeasurementEnumType::W;
    boundary_desc.label = "Power Boundary";
    tier.boundaryDescription->push_back(boundary_desc);

    // Incentive description - absolute cost
    IncentiveDescriptionDataType incentive_desc{};
    incentive_desc.incentiveId = INCENTIVE_ID;
    incentive_desc.incentiveType = IncentiveTypeEnumType::absoluteCost;
    incentive_desc.currency = incentive_currency;
    incentive_desc.unit = UnitOfMeasurementEnumType::Wh;
    incentive_desc.label = "Energy Cost";
    tier.incentiveDescription->push_back(incentive_desc);

    desc.tier->push_back(tier);
    data->incentiveTableDescription->push_back(desc);
}

// CEVC Spec 3.2.1.2.2.1: Write incentive table description from Energy Broker
MessageReturn CevcUsecase::write_incentive_table_description(HeaderType &header, SpineOptional<IncentiveTableDescriptionDataType> data, JsonObject response)
{
    if (!data.has_value() || !data->incentiveTableDescription.has_value()) {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Invalid incentive table description");
        return {true, true, CmdClassifierType::result};
    }

    // Parse the incentive table description to extract currency if provided
    // Only fields present in the incoming data are updated (partial write support)
    for (const auto &desc : data->incentiveTableDescription.get()) {
        if (desc.tier.has_value()) {
            for (const auto &tier : desc.tier.get()) {
                if (tier.incentiveDescription.has_value()) {
                    for (const auto &incentive : tier.incentiveDescription.get()) {
                        if (incentive.currency.has_value()) {
                            incentive_currency = incentive.currency.get();
                            eebus.trace_fmtln("CEVC: Updated incentive currency");
                        }
                    }
                }
            }
        }
    }

    EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
    return {true, true, CmdClassifierType::result};
}

// CEVC Spec 3.2.1.2.2.2: Incentive table constraints
void CevcUsecase::read_incentive_table_constraints(IncentiveTableConstraintsDataType *data) const
{
    IncentiveTableConstraintsType constraints{};

    // Tariff reference
    TariffDataType tariff{};
    tariff.tariffId = TARIFF_ID;
    constraints.tariff = tariff;

    // Slot constraints - how many incentive slots we support
    TimeTableConstraintsDataType slot_constraints{};
    slot_constraints.slotCountMin = "1";
    slot_constraints.slotCountMax = std::to_string(MAX_INCENTIVE_SLOTS);
    slot_constraints.slotDurationMin = "PT1M";  // Minimum 1 minute
    slot_constraints.slotDurationMax = "PT24H"; // Maximum 24 hours
    constraints.incentiveSlotConstraints = slot_constraints;

    data->incentiveTableConstraints->push_back(constraints);
}

// CEVC Spec 3.2.1.2.2.3: Incentive table data - current tariff information
void CevcUsecase::read_incentive_table_data(IncentiveTableDataType *data) const
{
    if (!incentives_valid || received_incentives.empty()) {
        // Return empty data if no incentives available
        return;
    }

    IncentiveTableType table{};

    // Tariff reference
    TariffDataType tariff{};
    tariff.tariffId = TARIFF_ID;
    tariff.activeTierId->push_back(TIER_ID);
    table.tariff = tariff;

    // Convert our internal incentive slots to SPINE format
    for (const auto &slot : received_incentives) {
        IncentiveTableIncentiveSlotType spine_slot{};

        // Time interval
        TimeTableDataType time_interval{};
        time_interval.startTime->dateTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(slot.start_time).c_str();
        time_interval.endTime->dateTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(slot.end_time).c_str();
        spine_slot.timeInterval = time_interval;

        // Tiers within this slot
        for (const auto &tier : slot.tiers) {
            IncentiveTableTierType spine_tier{};

            // Tier data
            TierDataType tier_data{};
            tier_data.tierId = TIER_ID;
            spine_tier.tier = tier_data;

            // Boundary data
            TierBoundaryDataType boundary{};
            boundary.boundaryId = 1;
            boundary.lowerBoundaryValue->number = tier.lower_boundary_w;
            boundary.lowerBoundaryValue->scale = 0;
            boundary.upperBoundaryValue->number = tier.upper_boundary_w;
            boundary.upperBoundaryValue->scale = 0;
            spine_tier.boundary->push_back(boundary);

            // Incentive data (cost value)
            IncentiveDataType incentive{};
            incentive.incentiveId = INCENTIVE_ID;
            // Store as cents with scale -2 for proper decimal representation
            incentive.value->number = static_cast<int>(tier.incentive_value * 100);
            incentive.value->scale = -2;
            spine_tier.incentive->push_back(incentive);

            spine_slot.tier->push_back(spine_tier);
        }

        table.incentiveSlot->push_back(spine_slot);
    }

    data->incentiveTable->push_back(table);
}

// CEVC Spec 3.4.3: Receive incentive table from Energy Broker
MessageReturn CevcUsecase::write_incentive_table_data(HeaderType &header, SpineOptional<IncentiveTableDataType> data, JsonObject response)
{
    if (!data.has_value() || !data->incentiveTable.has_value()) {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Invalid incentive table data");
        return {true, true, CmdClassifierType::result};
    }

    // Process incoming incentive slots
    int updated_slots = 0;
    int new_slots = 0;

    for (const auto &table : data->incentiveTable.get()) {
        if (!table.incentiveSlot.has_value()) {
            continue;
        }

        for (const auto &slot : table.incentiveSlot.get()) {
            IncentiveSlotEntry entry{};

            // Parse time interval
            if (slot.timeInterval.has_value()) {
                if (slot.timeInterval->startTime.has_value() && slot.timeInterval->startTime->dateTime.has_value()) {
                    EEBUS_USECASE_HELPERS::iso_timestamp_to_unix(slot.timeInterval->startTime->dateTime->c_str(), &entry.start_time);
                }
                if (slot.timeInterval->endTime.has_value() && slot.timeInterval->endTime->dateTime.has_value()) {
                    EEBUS_USECASE_HELPERS::iso_timestamp_to_unix(slot.timeInterval->endTime->dateTime->c_str(), &entry.end_time);
                }
            }

            // Parse tiers
            if (slot.tier.has_value()) {
                for (const auto &tier : slot.tier.get()) {
                    IncentiveTier t{};

                    // Parse boundary
                    if (tier.boundary.has_value() && !tier.boundary->empty()) {
                        const auto &boundary = tier.boundary->at(0);
                        if (boundary.lowerBoundaryValue.has_value()) {
                            t.lower_boundary_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(boundary.lowerBoundaryValue.get());
                        }
                        if (boundary.upperBoundaryValue.has_value()) {
                            t.upper_boundary_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(boundary.upperBoundaryValue.get());
                        }
                    }

                    // Parse incentive value
                    if (tier.incentive.has_value() && !tier.incentive->empty()) {
                        const auto &incentive = tier.incentive->at(0);
                        if (incentive.value.has_value()) {
                            t.incentive_value = EEBUS_USECASE_HELPERS::scaled_numbertype_to_float(incentive.value.get());
                        }
                    }

                    entry.tiers.push_back(t);
                }
            }

            // Try to find existing slot by matching time interval (startTime + endTime)
            bool found = false;
            for (auto &existing : received_incentives) {
                // Match by time interval (start_time and end_time must match)
                if (existing.start_time == entry.start_time && existing.end_time == entry.end_time) {
                    // Update existing slot with new tier data
                    existing.tiers = entry.tiers;
                    found = true;
                    updated_slots++;
                    break;
                }
            }

            if (!found) {
                // Add new slot if not found and within limit
                if (received_incentives.size() < MAX_INCENTIVE_SLOTS) {
                    received_incentives.push_back(entry);
                    new_slots++;
                }
            }
        }
    }

    if (!received_incentives.empty()) {
        incentives_valid = true;
        eebus.trace_fmtln("CEVC: Updated incentive table (%d updated, %d new, total %d slots)", updated_slots, new_slots, received_incentives.size());
        notify_subscribers_incentives();
        update_api_state();
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
    } else {
        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "No valid incentive data found");
    }

    return {true, true, CmdClassifierType::result};
}

// =============================================================================
// Public Update Methods
// =============================================================================

void CevcUsecase::update_charging_demand(int min_energy_wh, int opt_energy_wh, int max_energy_wh, time_t target_time, const std::vector<ChargingDemandSlot> &slots)
{
    minimum_energy_wh = min_energy_wh;
    optimal_energy_wh = opt_energy_wh;
    maximum_energy_wh = max_energy_wh;
    target_departure_time = target_time;
    charging_demand_slots = slots;

    notify_subscribers_time_series();
    update_api_state();
}

void CevcUsecase::update_power_constraints(int max_power_w, const std::vector<PowerConstraintSlot> &slots)
{
    max_power_constraint_w = max_power_w;
    power_constraint_slots = slots;

    notify_subscribers_time_series();
    update_api_state();
}

int CevcUsecase::get_target_power_w(time_t query_time) const
{
    if (!charging_plan_valid || received_charging_plan.empty()) {
        return -1;
    }

    if (query_time == 0) {
        query_time = time(nullptr);
    }

    // Find the slot that contains the query time
    time_t slot_start = charging_plan_start_time;
    for (const auto &slot : received_charging_plan) {
        time_t slot_end = slot_start + slot.duration.as<int>();
        if (query_time >= slot_start && query_time < slot_end) {
            return slot.power_w;
        }
        slot_start = slot_end;
    }

    // Query time is after all slots - plan has expired
    return -1;
}

// =============================================================================
// Helper Methods
// =============================================================================

void CevcUsecase::notify_subscribers_time_series() const
{
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return;
    }
    // Build the time series data to send to subscribers
    TimeSeriesListDataType ts_data = EVEntity::get_time_series_list_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::TimeSeries), ts_data, "timeSeriesListData");
}

void CevcUsecase::notify_subscribers_incentives() const
{
    if (!eebus.usecases->ev_commissioning_and_configuration.is_ev_connected()) {
        return;
    }
    // Build the incentive data to send to subscribers
    IncentiveTableDataType it_data = EVEntity::get_incentive_table_data();
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::IncentiveTable), it_data, "incentiveTableData");
}

void CevcUsecase::update_api_state() const
{
    auto api_entry = eebus.eebus_usecase_state.get("coordinated_ev_charging");
    api_entry->get("has_charging_plan")->updateBool(has_charging_plan());
    api_entry->get("charging_plan_start_time")->updateUint(charging_plan_start_time);
    api_entry->get("target_power_w")->updateInt(get_target_power_w());
    api_entry->get("has_incentives")->updateBool(has_incentives());
    api_entry->get("energy_broker_connected")->updateBool(energy_broker_connected);
    api_entry->get("energy_broker_heartbeat_ok")->updateBool(energy_broker_heartbeat_ok);
}

// =============================================================================
// Error State Handling (Scenarios 7-8)
// =============================================================================

void CevcUsecase::receive_heartbeat_timeout()
{
    // Scenario 8: Energy Broker error state
    // When we lose heartbeat from the Energy Broker, treat it as an error state
    // and invalidate all received data (charging plan, incentives)
    eebus.trace_fmtln("CEVC: Energy Broker heartbeat timeout - invalidating broker data");
    energy_broker_heartbeat_ok = false;
    invalidate_broker_data();
}

void CevcUsecase::inform_spineconnection_usecase_update(SpineConnection *conn)
{
    // Look for Energy Broker actors that support the CEVC use case
    // and register for heartbeat monitoring (Scenarios 5-6)
    auto peers = conn->get_address_of_feature(FeatureTypeEnumType::DeviceDiagnosis, RoleType::client, "coordinatedEvCharging", "EnergyBroker");

    for (FeatureAddressType &peer : peers) {
        eebus.trace_fmtln("CEVC: Found Energy Broker at %s, registering for heartbeat", EEBUS_USECASE_HELPERS::spine_address_to_string(peer).c_str());
        eebus.usecases->evse_heartbeat.initialize_heartbeat_on_feature(peer, Usecases::CEVC, true);
        energy_broker_connected = true;
        energy_broker_heartbeat_ok = true;
    }
}

void CevcUsecase::invalidate_broker_data()
{
    // Clear charging plan (Scenario 4 data)
    if (charging_plan_valid) {
        charging_plan_valid = false;
        received_charging_plan.clear();
        charging_plan_start_time = 0;
        eebus.trace_fmtln("CEVC: Charging plan invalidated due to Energy Broker error");
    }

    // Clear incentives (Scenario 3 data)
    if (incentives_valid) {
        incentives_valid = false;
        received_incentives.clear();
        eebus.trace_fmtln("CEVC: Incentives invalidated due to Energy Broker error");
    }

    // Notify subscribers that data has changed
    notify_subscribers_time_series();
    notify_subscribers_incentives();
    update_api_state();
}

#endif // EEBUS_ENABLE_CEVC_USECASE
