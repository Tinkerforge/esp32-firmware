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

#pragma once

#include "usecase_base.h"

#ifdef EEBUS_ENABLE_EVCS_USECASE

/**
 * @brief The EV Charging Summary (EVCS) use case.
 *
 * Implements EEBus UC TS - EV Charging Summary V1.0.1.
 * Provides billing data for EV charging sessions including energy consumed,
 * cost breakdown, and source breakdown (grid vs self-produced).
 *
 * Actor: EVSE
 * Entity address: {1}
 * Features: Bill (billDescriptionListData, billConstraintsListData, billListData)
 *
 * @see EEBus_UC_TS_EVChargingSummary_V1.0.1.pdf
 */
class EvcsUsecase final : public EebusUsecase
{
public:
    EvcsUsecase();

    /**
     * @brief Handles a message for the EVCS use case.
     * @param header SPINE header with commandclassifier and targeted entity info
     * @param data The actual function call and data of the message
     * @param response JsonObject to write the response to
     * @return MessageReturn with handling status and response info
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::EVCS;
    }

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;

    /**
     * @brief Returns the supported features.
     *
     * EVCS supports only one feature which is the Bill feature.
     *
     * @return List of supported features
     */
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * @brief Update the billing data.
     *
     * Updates the bill entry with the given data and informs all subscribers of the new bill entry.
     *
     * @param id Bill entry ID (1-8). If matches existing entry, it's updated; otherwise new entry created
     * @param start_time Start time of billing period
     * @param end_time End time of billing period
     * @param energy_wh Amount of energy charged in Wh
     * @param cost_eur_cent Cost of the charged energy in Euro cents
     * @param grid_energy_percent Percentage of energy drawn from grid (default 100)
     * @param grid_cost_percent Percentage of cost due to grid energy (default 100)
     * @param self_produced_energy_percent Percentage of self-produced energy used (default 0)
     * @param self_produced_cost_percent Percentage of cost from self-produced energy (default 0)
     */
    void update_billing_data(int id, time_t start_time, time_t end_time, int energy_wh, uint32_t cost_eur_cent, int grid_energy_percent = 100, int grid_cost_percent = 100, int self_produced_energy_percent = 0, int self_produced_cost_percent = 0);

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::Bill};
    }

    void get_bill_description_list(BillDescriptionListDataType *data) const;
    void get_bill_constraints_list(BillConstraintsListDataType *data) const;
    void get_bill_list_data(BillListDataType *data) const;

private:
    /**
     * @brief Internal bill entry storage structure.
     *
     * Stores billing data in a compact format for memory efficiency.
     */
    struct BillEntry {
        uint8_t id = 0; ///< ID of the bill entry. 0 means unused entry
        time_t start_time;
        time_t end_time;
        uint16_t energy_wh;
        uint32_t cost_eur_cent;
        uint8_t grid_energy_percent;
        uint8_t grid_cost_percent;
        uint8_t self_produced_energy_percent;
        uint8_t self_produced_cost_percent;
    };

    BillEntry bill_entries[8]{};

    void update_api() const;
};

#endif // EEBUS_ENABLE_EVCS_USECASE
