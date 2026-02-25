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

#include "entity_data.h"
#include "evcem.h" // Required for EvcemUsecase ID references
#include "usecase_base.h"

#ifdef EEBUS_ENABLE_OPEV_USECASE

// Dependency checks
#ifndef EEBUS_ENABLE_EVCC_USECASE
#error "OPEV Usecase requires EVCC Usecase to be enabled"
#endif
#ifndef EEBUS_ENABLE_EVCEM_USECASE
#error "OPEV Usecase requires EVCEM Usecase to be enabled"
#endif

/**
 * @brief Overload Protection by EV Charging Current Curtailment (OPEV) use case.
 *
 * Implements EEBus UC TS - Overload Protection by EV Charging Current Curtailment V1.0.1b.
 * Allows limiting the charging current per phase to protect against overload conditions.
 *
 * Scenarios:
 *   1 (3.4.1): EV provides limitations and writes current limit
 *   2 (3.4.2): EG sends heartbeat
 *   3 (3.4.3): EG sends error state
 *
 * Actor: EV
 * Entity address: {1, 1}
 * Features:
 *   - LoadControl (loadControlLimitDescriptionListData, loadControlLimitListData)
 *   - ElectricalConnection (electricalConnectionParameterDescriptionListData)
 *
 * @see EEBus_UC_TS_OverloadProtectionByEVChargingCurrentCurtailment_V1.0.1b.pdf
 */
class OpevUsecase final : public EebusUsecase
{
public:
    OpevUsecase();

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::OPEV;
    }

    /**
     * @brief Handles a message for the OPEV use case.
     * @param header SPINE header with command classifier and targeted entity info
     * @param data The actual function call and data of the message
     * @param response JsonObject to write the response to
     * @return MessageReturn with handling status and response info
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::LoadControl, FeatureTypeEnumType::ElectricalConnection};
    }

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    // ========================================================================
    // Data Generators
    // ========================================================================

    /**
     * @brief Get load control limit descriptions.
     *
     * Spec 3.2.1.2.1.1: Creates 3 limit IDs (id_x_1, id_x_2, id_x_3) for phases A, B, C.
     * Limit type: maxValueLimit, Category: obligation, Scope: overloadProtection.
     *
     * @param data Output parameter for the description list
     */
    void get_load_control_limit_description_list_data(LoadControlLimitDescriptionListDataType *data);

    /**
     * @brief Get current load control limits.
     *
     * Spec 3.2.1.2.1.2: Returns per-phase current limits in milliamps (scale=-3).
     *
     * @param data Output parameter for the limit list
     */
    void get_load_control_limit_list_data(LoadControlLimitListDataType *data) const;

    /**
     * @brief Get electrical connection parameter descriptions.
     * @param data Output parameter for the parameter description list
     */
    static void get_electrical_connection_parameter_description_list_data(ElectricalConnectionParameterDescriptionListDataType *data);

    /**
     * @brief Get permitted values for electrical connection parameters.
     * @param data Output parameter for the permitted value set list
     */
    void get_electrical_connection_permitted_list_data(ElectricalConnectionPermittedValueSetListDataType *data) const;

    // ========================================================================
    // Limit Control
    // ========================================================================

    /**
     * @brief Update the per-phase current limits.
     *
     * Updates the limits and informs all subscribers of the new values.
     *
     * @param limit_phase_1_milliamps Phase A limit in milliamps (-1 to keep unchanged)
     * @param limit_phase_2_milliamps Phase B limit in milliamps (-1 to keep unchanged)
     * @param limit_phase_3_milliamps Phase C limit in milliamps (-1 to keep unchanged)
     * @param active Whether the limit shall be activated. Default is false.
     */
    void update_limits(int limit_phase_1_milliamps, int limit_phase_2_milliamps, int limit_phase_3_milliamps, bool active = false);

    /**
     * Updates the minimum and maximum amps that can be set.
     * @param min_limit_milliamps
     * @param max_limit_milliamps
     */
    void update_min_max_limits(int min_limit_milliamps, int max_limit_milliamps);

    /**
     * @brief Update the API state with current OPEV data.
     *
     * Updates the eebus/usecases API endpoint with current limit values.
     */
    void update_api() const;

    /**
     * @brief Allow or disallow limit changes.
     * @param allowed Whether limit changes are allowed
     */
    inline void allow_limitation(bool allowed)
    {
        limit_changeable_allowed = allowed;
    }

    /**
     * @brief Check if limits can be changed.
     *
     * Returns false if LPC limit is active (takes precedence) or if manually disallowed.
     *
     * @return true if limits can be changed
     */
    [[nodiscard]] bool limit_changeable() const;

    /**
     * @brief Get current per-phase limits.
     * @return Array of 3 limits in milliamps [phase_a, phase_b, phase_c]
     */
    [[nodiscard]] std::array<int, 3> get_limit_milliamps() const
    {
        return {limit_per_phase_milliamps[0], limit_per_phase_milliamps[1], limit_per_phase_milliamps[2]};
    }

    /**
     * @brief Check if limit is currently active.
     * @return true if limit is active
     */
    [[nodiscard]] bool limit_is_active() const
    {
        return limit_active;
    }

    // ========================================================================
    // ID Constants
    // IDs as defined in Overload Protection by EV Charging Current Curtailment V1.0.1b 3.2.1.2
    // May be used to link to other IDs in other use cases
    // ========================================================================

    /// Load control limit IDs per phase (link to measurement IDs)
    static constexpr uint8_t id_x_1 = EVEntity::opevLoadcontrolIdOffset + 1;
    static constexpr uint8_t id_x_2 = EVEntity::opevLoadcontrolIdOffset + 2;
    static constexpr uint8_t id_x_3 = EVEntity::opevLoadcontrolIdOffset + 3;

    /// Measurement IDs (reference EVCEM measurement IDs)
    static constexpr uint8_t id_z_1 = EvcemUsecase::id_x_1;
    static constexpr uint8_t id_z_2 = EvcemUsecase::id_x_2;
    static constexpr uint8_t id_z_3 = EvcemUsecase::id_x_3;

    /// Electrical connection parameter IDs per phase
    static constexpr uint8_t id_i_1 = EVEntity::opevElectricalconnectionParameterIdOffset + 1;
    static constexpr uint8_t id_i_2 = EVEntity::opevElectricalconnectionParameterIdOffset + 2;
    static constexpr uint8_t id_i_3 = EVEntity::opevElectricalconnectionParameterIdOffset + 3;

    /// Electrical connection ID (reference EVCEM electrical connection)
    static constexpr uint8_t id_j_1 = EvcemUsecase::id_y_1;

private:
    // Current limit values
    int limit_per_phase_milliamps[3] = {32000, 32000, 32000};
    bool limit_active = false;
    bool limit_changeable_allowed = true;
    int limit_milliamps_min = 0;
    int limit_milliamps_max = 32000;

    /**
     * @brief Handle write request for load control limits.
     * @param header SPINE header
     * @param data Load control limit list data
     * @param response JsonObject to write the response to
     * @return MessageReturn with handling status
     */
    MessageReturn write_load_control_limit_list_data(HeaderType &header, SpineOptional<LoadControlLimitListDataType> data, JsonObject response);
};

#endif // EEBUS_ENABLE_OPEV_USECASE
