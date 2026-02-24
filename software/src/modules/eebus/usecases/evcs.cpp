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

#ifdef EEBUS_ENABLE_EVCS_USECASE

#include "../module_dependencies.h"
#include "entity_data.h"
#include "evcs.h"

#include <cmath>

// ==============================================================================
// EVCS - EV Charging Summary Use Case
// Spec: EEBus_UC_TS_EVChargingSummary_V1.0.1.pdf
//
// Scenario 1 (2.3.1, 3.4.1): Energy Broker sends charging session summary
//
// NOTE: Write support for billListData is implemented per spec 3.2.1.2.1.3
//       for receiving billing data from Energy Broker.
// ==============================================================================

EvcsUsecase::EvcsUsecase()
{
    update_api();
}

UseCaseInformationDataType EvcsUsecase::get_usecase_information()
{
    UseCaseInformationDataType evcs_usecase;
    evcs_usecase.actor = "EVSE"; // Actor can be EVSE or Energy Broker; we implement EVSE role

    UseCaseSupportType evcs_usecase_support;
    evcs_usecase_support.useCaseName = "evChargingSummary";
    evcs_usecase_support.useCaseVersion = "1.0.1";
    evcs_usecase_support.scenarioSupport->push_back(1); // Scenario 1 (2.3.1): Charging summary exchange
    evcs_usecase_support.useCaseDocumentSubRevision = "release";
    evcs_usecase.useCaseSupport->push_back(evcs_usecase_support);

    FeatureAddressType evcs_usecase_feature_address;
    evcs_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    evcs_usecase_feature_address.entity = entity_address;
    evcs_usecase.address = evcs_usecase_feature_address;
    return evcs_usecase;
}

MessageReturn EvcsUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    if (header.addressDestination->feature.has_value() && header.addressDestination->feature == feature_addresses.at(FeatureTypeEnumType::Bill)) {
        switch (data->last_cmd) {
            // 3.2.1.2.1.1: Function "billDescriptionListData" - Bill metadata
            case SpineDataTypeHandler::Function::billDescriptionListData: {
                switch (header.cmdClassifier.get()) {
                    case CmdClassifierType::read: {
                        response["billDescriptionListData"] = EVSEEntity::get_bill_description_list_data();
                        return {true, false, CmdClassifierType::reply};
                    }
                    default:
                        return {false};
                }
            }
            // 3.2.1.2.1.2: Function "billConstraintsListData" - Bill position constraints
            case SpineDataTypeHandler::Function::billConstraintsListData: {
                switch (header.cmdClassifier.get()) {
                    case CmdClassifierType::read: {
                        response["billConstraintsListData"] = EVSEEntity::get_bill_constraints_list_data();
                        return {true, true, CmdClassifierType::reply};
                    }
                    default:
                        return {false};
                }
            }
            // 3.2.1.2.1.3: Function "billListData" - Actual billing data
            case SpineDataTypeHandler::Function::billListData: {
                switch (header.cmdClassifier.get()) {
                    case CmdClassifierType::read: {
                        response["billListData"] = EVSEEntity::get_bill_list_data();
                        return {true, true, CmdClassifierType::reply};
                    }
                    case CmdClassifierType::write: {
                        // Implements write handler with MANDATORY partial write support per spec line 649
                        // Partial write is supported via identifier matching: incoming bills are matched
                        // by billId and updated in-place, preserving unaffected entries.

                        if (!data->billlistdatatype.has_value() || !data->billlistdatatype->billData.has_value()) {
                            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Invalid bill data");
                            return {true, true, CmdClassifierType::result};
                        }

                        // Process each incoming bill (partial write via identifier matching)
                        for (const auto &incoming_bill : data->billlistdatatype->billData.get()) {
                            // Validate billType
                            if (incoming_bill.billType.get() != BillTypeEnumType::chargingSummary) {
                                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Only chargingSummary billType is supported");
                                return {true, true, CmdClassifierType::result};
                            }

                            // Validate billId range (1-8)
                            int bill_id = incoming_bill.billId.get();
                            if (bill_id < 1 || bill_id > 8) {
                                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "billId must be between 1 and 8");
                                return {true, true, CmdClassifierType::result};
                            }

                            int array_idx = bill_id - 1;

                            // Parse total section
                            if (!incoming_bill.total.has_value() || !incoming_bill.total->timePeriod.has_value()) {
                                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Missing total section or time period");
                                return {true, true, CmdClassifierType::result};
                            }

                            time_t start_time = 0, end_time = 0;
                            if (incoming_bill.total->timePeriod->startTime.has_value()) {
                                EEBUS_USECASE_HELPERS::iso_timestamp_to_unix(incoming_bill.total->timePeriod->startTime->c_str(), &start_time);
                            }
                            if (incoming_bill.total->timePeriod->endTime.has_value()) {
                                EEBUS_USECASE_HELPERS::iso_timestamp_to_unix(incoming_bill.total->timePeriod->endTime->c_str(), &end_time);
                            }

                            // Parse total energy (Wh)
                            int energy_wh = 0;
                            if (incoming_bill.total->value.has_value() && !incoming_bill.total->value->empty()) {
                                for (const auto &val : incoming_bill.total->value.get()) {
                                    if (val.unit.has_value() && val.unit.get() == UnitOfMeasurementEnumType::Wh && val.value.has_value()) {
                                        energy_wh = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(val.value.get());
                                        break;
                                    }
                                }
                            }

                            // Parse total cost (EUR cents)
                            uint32_t cost_eur_cent = 0;
                            if (incoming_bill.total->cost.has_value() && !incoming_bill.total->cost->empty()) {
                                for (const auto &cost : incoming_bill.total->cost.get()) {
                                    if (cost.currency.has_value() && cost.currency.get() == CurrencyEnumType::EUR && cost.cost.has_value()) {
                                        // Cost is in ScaledNumber format, typically with scale=-2 (cents)
                                        int cost_value = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(cost.cost.get());
                                        // Adjust for scale: if scale=-2, value is already in cents
                                        int scale = cost.cost->scale.has_value() ? cost.cost->scale.get() : 0;
                                        if (scale == -2) {
                                            cost_eur_cent = cost_value;
                                        } else if (scale == 0) {
                                            cost_eur_cent = cost_value * 100; // Convert EUR to cents
                                        } else {
                                            cost_eur_cent = static_cast<uint32_t>(cost_value * std::pow(10.0, -scale - 2));
                                        }
                                        break;
                                    }
                                }
                            }

                            // Parse positions (MANDATORY: grid and self-produced)
                            int grid_energy_percent = 0, grid_cost_percent = 0;
                            int self_produced_energy_percent = 0, self_produced_cost_percent = 0;

                            if (incoming_bill.position.has_value()) {
                                for (const auto &pos : incoming_bill.position.get()) {
                                    if (!pos.positionType.has_value())
                                        continue;

                                    if (pos.positionType.get() == BillPositionTypeEnumType::gridElectricEnergy) {
                                        // Position 1: Grid energy
                                        if (pos.value.has_value() && !pos.value->empty()) {
                                            for (const auto &val : pos.value.get()) {
                                                if (val.valuePercentage.has_value()) {
                                                    grid_energy_percent = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(val.valuePercentage.get());
                                                }
                                            }
                                        }
                                        if (pos.cost.has_value() && !pos.cost->empty()) {
                                            for (const auto &cost : pos.cost.get()) {
                                                if (cost.costPercentage.has_value()) {
                                                    grid_cost_percent = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(cost.costPercentage.get());
                                                }
                                            }
                                        }
                                    } else if (pos.positionType.get() == BillPositionTypeEnumType::selfProducedElectricEnergy) {
                                        // Position 2: Self-produced energy
                                        if (pos.value.has_value() && !pos.value->empty()) {
                                            for (const auto &val : pos.value.get()) {
                                                if (val.valuePercentage.has_value()) {
                                                    self_produced_energy_percent = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(val.valuePercentage.get());
                                                }
                                            }
                                        }
                                        if (pos.cost.has_value() && !pos.cost->empty()) {
                                            for (const auto &cost : pos.cost.get()) {
                                                if (cost.costPercentage.has_value()) {
                                                    self_produced_cost_percent = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(cost.costPercentage.get());
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // Validate percentages sum to 100% (spec requirement)
                            if (grid_energy_percent + self_produced_energy_percent != 100) {
                                eebus.trace_fmtln("EVCS Warning: Energy percentages do not sum to 100%% (grid: %d%%, self: %d%%)", grid_energy_percent, self_produced_energy_percent);
                            }
                            if (grid_cost_percent + self_produced_cost_percent != 100) {
                                eebus.trace_fmtln("EVCS Warning: Cost percentages do not sum to 100%% (grid: %d%%, self: %d%%)", grid_cost_percent, self_produced_cost_percent);
                            }

                            // Store bill entry
                            bill_entries[array_idx].id = bill_id;
                            bill_entries[array_idx].start_time = start_time;
                            bill_entries[array_idx].end_time = end_time;
                            bill_entries[array_idx].energy_wh = energy_wh;
                            bill_entries[array_idx].cost_eur_cent = cost_eur_cent;
                            bill_entries[array_idx].grid_energy_percent = grid_energy_percent;
                            bill_entries[array_idx].grid_cost_percent = grid_cost_percent;
                            bill_entries[array_idx].self_produced_energy_percent = self_produced_energy_percent;
                            bill_entries[array_idx].self_produced_cost_percent = self_produced_cost_percent;
                        }

                        // Update API state
                        update_api();

                        // Notify subscribers (with partial flag if applicable)
                        BillListDataType bill_list_data = EVSEEntity::get_bill_list_data();
                        eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::Bill), bill_list_data, "billListData");

                        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
                        return {true, true, CmdClassifierType::result};
                    }
                    default:;
                }
            }
            default:;
        }
    }

    return {false};
}

NodeManagementDetailedDiscoveryEntityInformationType EvcsUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = EntityTypeEnumType::EVSE;
    entity.description->label = "Charging Summary";

    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> EvcsUsecase::get_detailed_discovery_feature_information() const
{
    NodeManagementDetailedDiscoveryFeatureInformationType feature{};

    feature.description->featureAddress->entity = entity_address;
    feature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::Bill);

    feature.description->featureType = FeatureTypeEnumType::Bill;
    feature.description->role = RoleType::server;

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

    BillListDataType bill_list_data = EVSEEntity::get_bill_list_data();
    eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::Bill), bill_list_data, "billListData");
    update_api();
}

// Spec 3.2.1.2.1.1: Builds billDescriptionListData with metadata for each bill
void EvcsUsecase::get_bill_description_list(BillDescriptionListDataType *data) const
{
    if (data == nullptr)
        return;
    data->billDescriptionData.emplace();
    for (BillEntry entry : bill_entries) {
        if (!entry.id)
            continue;
        BillDescriptionDataType billDescriptionData{};
        billDescriptionData.billWriteable = false; // Bills are read-only (managed internally)
        billDescriptionData.billId = entry.id;
        billDescriptionData.supportedBillType->push_back(BillTypeEnumType::chargingSummary);
        data->billDescriptionData->push_back(billDescriptionData);
    }
}

// Spec 3.2.1.2.1.2: Builds billConstraintsListData with position count constraints
void EvcsUsecase::get_bill_constraints_list(BillConstraintsListDataType *data) const
{
    if (data == nullptr)
        return;
    data->billConstraintsData.emplace();
    for (BillEntry entry : bill_entries) {
        if (!entry.id)
            continue;
        BillConstraintsDataType billConstraintsData{};
        billConstraintsData.billId = entry.id;
        billConstraintsData.positionCountMin = "0";
        billConstraintsData.positionCountMax = std::to_string(2); // gridElectricEnergy + selfProducedElectricEnergy
        data->billConstraintsData->push_back(billConstraintsData);
    }
}

// Spec 3.2.1.2.1.3: Builds billListData with total energy/cost and position breakdowns
// Bill structure (Table 6):
//   - Total: energy (Wh), cost (EUR cents, scale=-2), time period
//   - Position 1: gridElectricEnergy (percentage of total)
//   - Position 2: selfProducedElectricEnergy (percentage of total)
void EvcsUsecase::get_bill_list_data(BillListDataType *data) const
{
    if (data == nullptr)
        return;
    for (BillEntry entry : bill_entries) {
        if (!entry.id)
            continue;
        BillDataType billData{};
        billData.billId = entry.id;
        billData.billType = BillTypeEnumType::chargingSummary;

        // Total energy and cost
        billData.total->timePeriod->startTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(entry.start_time).c_str();
        billData.total->timePeriod->endTime = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(entry.end_time).c_str();
        BillValueType total_value;
        total_value.value->number = entry.energy_wh;
        total_value.unit = UnitOfMeasurementEnumType::Wh;
        total_value.value->scale = 0; // Total value = number * 10^scale
        BillCostType total_cost;
        total_cost.costType = BillCostTypeEnumType::absolutePrice;
        total_cost.cost->number = entry.cost_eur_cent;
        total_cost.currency = CurrencyEnumType::EUR;
        total_cost.cost->scale = -2; // Cost in cents: value * 10^-2 = EUR
        billData.total->value->push_back(total_value);
        billData.total->cost->push_back(total_cost);

        // Position 1: Grid energy breakdown
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

        // Position 2: Self-produced energy breakdown
        BillPositionType self_produced_position;
        self_produced_position.positionId = 2;
        self_produced_position.positionType = BillPositionTypeEnumType::selfProducedElectricEnergy;
        BillValueType self_produced_value;
        self_produced_value.valuePercentage->number = entry.self_produced_energy_percent;
        self_produced_value.valuePercentage->scale = 0;
        BillCostType self_produced_cost;
        self_produced_cost.costPercentage->number = entry.self_produced_cost_percent;
        self_produced_position.cost->push_back(self_produced_cost);
        self_produced_position.value->push_back(self_produced_value);

        billData.position->push_back(grid_position);
        billData.position->push_back(self_produced_position);

        data->billData->push_back(billData);
    }
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

#endif // EEBUS_ENABLE_EVCS_USECASE
