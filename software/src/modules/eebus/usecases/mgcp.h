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
#include "usecase_base.h"

// Forward declarations for ID linking
#ifdef EEBUS_ENABLE_LPC_USECASE
#include "loadcontrol.h"
#endif

#ifdef EEBUS_ENABLE_MGCP_USECASE

/**
 * @brief Monitoring of Grid Connection Point (MGCP) use case.
 *
 * Implements EEBus UC TS - Monitoring of Grid Connection Point V1.0.0.
 * Represents the point where the public electricity grid connects to the building's internal grid.
 * Uses load/passive sign convention: positive = consumption, negative = production/feed-in.
 *
 * Scenarios:
 *   1 (3.4.1): Monitor PV feed-in power limitation factor (Optional) - DeviceConfiguration
 *   2 (3.4.2): Monitor momentary power consumption/production (Mandatory)
 *   3 (3.4.3): Monitor total feed-in energy (Mandatory)
 *   4 (3.4.4): Monitor total consumed energy (Mandatory)
 *   5 (3.4.5): Monitor momentary current phase details (Recommended)
 *   6 (3.4.6): Monitor voltage phase details (Optional)
 *   7 (3.4.7): Monitor frequency (Optional)
 *
 * Entity Type: GridConnectionPointOfPremises
 * Actor: GridConnectionPoint
 * Entity address: {1} (EVSE entity)
 * Features:
 *   - DeviceConfiguration: For PV curtailment limit factor (Scenario 1)
 *   - Measurement: For power, energy, current, voltage, frequency (Scenarios 2-7)
 *   - ElectricalConnection: For electrical connection descriptions
 *
 * @see EEBus_UC_TS_MonitoringOfGridConnectionPoint_V1.0.0.pdf
 */
class MgcpUsecase final : public EebusUsecase
{
public:
    MgcpUsecase();

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::MGCP;
    }

    /**
     * @brief Handles a message for the MGCP use case.
     * @param header SPINE header with command classifier and targeted entity info
     * @param data The actual function call and data of the message
     * @param response JsonObject to write the response to
     * @return MessageReturn with handling status and response info
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    /**
     * @brief Builds and returns the use case information.
     *
     * As defined in MGCP spec 3.1.2.
     *
     * @return UseCaseInformationDataType with scenarios 1-7 support
     */
    UseCaseInformationDataType get_usecase_information() override;

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::DeviceConfiguration, FeatureTypeEnumType::Measurement, FeatureTypeEnumType::ElectricalConnection};
    }

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    // ========================================================================
    // Data Generator Methods
    // ========================================================================

    /**
     * @brief Get measurement descriptions for all scenarios.
     * @param data Output parameter for the description list
     */
    void get_measurement_description_list_data(MeasurementDescriptionListDataType *data) const;

    /**
     * @brief Get measurement constraints for all scenarios.
     * @param data Output parameter for the constraints list
     */
    void get_measurement_constraints_list_data(MeasurementConstraintsListDataType *data) const;

    /**
     * @brief Get current measurement values for all scenarios.
     * @param data Output parameter for the measurement list
     */
    void get_measurement_list_data(MeasurementListDataType *data) const;

    /**
     * @brief Get electrical connection descriptions.
     * @param data Output parameter for the description list
     */
    void get_electrical_connection_description_list_data(ElectricalConnectionDescriptionListDataType *data) const;

    /**
     * @brief Get electrical connection parameter descriptions.
     * @param data Output parameter for the parameter description list
     */
    void get_electrical_connection_parameter_description_list_data(ElectricalConnectionParameterDescriptionListDataType *data) const;

    /**
     * @brief Get device configuration key descriptions.
     * @param data Output parameter for the description list
     */
    void get_device_configuration_description_list_data(DeviceConfigurationKeyValueDescriptionListDataType *data) const;

    /**
     * @brief Get device configuration key values.
     * @param data Output parameter for the value list
     */
    void get_device_configuration_value_list_data(DeviceConfigurationKeyValueListDataType *data) const;

    // ========================================================================
    // Scenario 1: PV feed-in power limitation factor (Optional)
    // ========================================================================

    /**
     * @brief Update the PV curtailment limit factor.
     *
     * This represents the percentage of nominal power that a PV system is allowed to feed-in.
     *
     * @param limit_factor_percent PV curtailment limit factor as percentage (0-100, typically 70% in Germany).
     *                             A value of 100 means no curtailment.
     */
    void update_pv_curtailment_limit_factor(float limit_factor_percent);

    // ========================================================================
    // Scenario 2: Monitor momentary power (Mandatory)
    // ========================================================================

    /**
     * @brief Update the total power measurement.
     *
     * Uses load/passive sign convention: positive = consumption, negative = production.
     *
     * @param total_power_w Total active power in watts. Negative values indicate power feed-in.
     */
    void update_power(int total_power_w);

    // ========================================================================
    // Scenario 3: Monitor total feed-in energy (Mandatory)
    // ========================================================================

    /**
     * @brief Update the total energy fed into the grid.
     * @param energy_feed_in_wh Total energy fed into the grid in watt-hours.
     */
    void update_energy_feed_in(uint32_t energy_feed_in_wh);

    // ========================================================================
    // Scenario 4: Monitor total consumed energy (Mandatory)
    // ========================================================================

    /**
     * @brief Update the total energy consumed from the grid.
     * @param energy_consumed_wh Total energy consumed from the grid in watt-hours.
     */
    void update_energy_consumed(uint32_t energy_consumed_wh);

    // ========================================================================
    // Scenario 5: Monitor momentary current (Recommended)
    // ========================================================================

    /**
     * @brief Update the per-phase current measurements.
     *
     * Uses load/passive sign convention: positive = consumption, negative = production.
     *
     * @param current_phase_1_ma Current on phase 1 in milliamps
     * @param current_phase_2_ma Current on phase 2 in milliamps
     * @param current_phase_3_ma Current on phase 3 in milliamps
     */
    void update_current(int current_phase_1_ma, int current_phase_2_ma, int current_phase_3_ma);

    // ========================================================================
    // Scenario 6: Monitor voltage (Optional)
    // ========================================================================

    /**
     * @brief Update the per-phase voltage measurements.
     * @param voltage_phase_1_v Voltage on phase 1 (phase-to-neutral) in volts
     * @param voltage_phase_2_v Voltage on phase 2 (phase-to-neutral) in volts
     * @param voltage_phase_3_v Voltage on phase 3 (phase-to-neutral) in volts
     */
    void update_voltage(int voltage_phase_1_v, int voltage_phase_2_v, int voltage_phase_3_v);

    // ========================================================================
    // Scenario 7: Monitor frequency (Optional)
    // ========================================================================

    /**
     * @brief Update the grid frequency measurement.
     * @param frequency_mhz Grid frequency in millihertz (e.g., 50000 for 50Hz)
     */
    void update_frequency(int frequency_mhz);

    // ========================================================================
    // Constraint Update
    // ========================================================================

    /**
     * @brief Update all measurement constraints.
     * @param power_min Minimum power that can be measured in watts
     * @param power_max Maximum power that can be measured in watts
     * @param current_min_ma Minimum current in milliamps
     * @param current_max_ma Maximum current in milliamps
     * @param energy_max_wh Maximum energy in watt-hours
     * @param voltage_min_v Minimum voltage in volts
     * @param voltage_max_v Maximum voltage in volts
     * @param frequency_min_mhz Minimum frequency in millihertz
     * @param frequency_max_mhz Maximum frequency in millihertz
     */
    void update_constraints(int power_min, int power_max, int current_min_ma, int current_max_ma, uint32_t energy_max_wh, int voltage_min_v, int voltage_max_v, int frequency_min_mhz, int frequency_max_mhz);

    // ========================================================================
    // ID Constants
    // Based on MGCP spec 3.2.2.2
    // ========================================================================

    /// Measurement IDs
    static constexpr uint8_t id_m_1 = EVSEEntity::mgcpMeasurementIdOffset + 1;   ///< Total power
    static constexpr uint8_t id_m_2 = EVSEEntity::mgcpMeasurementIdOffset + 2;   ///< Energy feed-in
    static constexpr uint8_t id_m_3 = EVSEEntity::mgcpMeasurementIdOffset + 3;   ///< Energy consumed
    static constexpr uint8_t id_m_4_1 = EVSEEntity::mgcpMeasurementIdOffset + 4; ///< Current phase A
    static constexpr uint8_t id_m_4_2 = EVSEEntity::mgcpMeasurementIdOffset + 5; ///< Current phase B
    static constexpr uint8_t id_m_4_3 = EVSEEntity::mgcpMeasurementIdOffset + 6; ///< Current phase C
    static constexpr uint8_t id_m_5_1 = EVSEEntity::mgcpMeasurementIdOffset + 7; ///< Voltage phase A
    static constexpr uint8_t id_m_5_2 = EVSEEntity::mgcpMeasurementIdOffset + 8; ///< Voltage phase B
    static constexpr uint8_t id_m_5_3 = EVSEEntity::mgcpMeasurementIdOffset + 9; ///< Voltage phase C
    static constexpr uint8_t id_m_6 = EVSEEntity::mgcpMeasurementIdOffset + 10;  ///< Frequency

    /// ElectricalConnection IDs (linked to LPC/LPP if available)
#ifdef EEBUS_ENABLE_LPC_USECASE
    static constexpr uint8_t id_ec_1 = LpcUsecase::id_ec_1;
#elifdef EEBUS_ENABLE_LPP_USECASE
    static constexpr uint8_t id_ec_1 = LppUsecase::id_ec_1;
#else
    static constexpr uint8_t id_ec_1 = EVSEEntity::mgcpElectricalConnectionIdOffset + 1;
#endif

    /// ElectricalConnection Parameter IDs
    static constexpr uint8_t id_p_1 = EVSEEntity::mgcpElectricalConnectionParameterIdOffset + 1;   ///< Total power param
    static constexpr uint8_t id_p_2 = EVSEEntity::mgcpElectricalConnectionParameterIdOffset + 2;   ///< Energy feed-in param
    static constexpr uint8_t id_p_3 = EVSEEntity::mgcpElectricalConnectionParameterIdOffset + 3;   ///< Energy consumed param
    static constexpr uint8_t id_p_4_1 = EVSEEntity::mgcpElectricalConnectionParameterIdOffset + 4; ///< Current phase A param
    static constexpr uint8_t id_p_4_2 = EVSEEntity::mgcpElectricalConnectionParameterIdOffset + 5; ///< Current phase B param
    static constexpr uint8_t id_p_4_3 = EVSEEntity::mgcpElectricalConnectionParameterIdOffset + 6; ///< Current phase C param
    static constexpr uint8_t id_p_5_1 = EVSEEntity::mgcpElectricalConnectionParameterIdOffset + 7; ///< Voltage phase A param
    static constexpr uint8_t id_p_5_2 = EVSEEntity::mgcpElectricalConnectionParameterIdOffset + 8; ///< Voltage phase B param
    static constexpr uint8_t id_p_5_3 = EVSEEntity::mgcpElectricalConnectionParameterIdOffset + 9; ///< Voltage phase C param
    static constexpr uint8_t id_p_6 = EVSEEntity::mgcpElectricalConnectionParameterIdOffset + 10;  ///< Frequency param

    /// DeviceConfiguration Key IDs
    static constexpr uint8_t id_k_1 = EVSEEntity::mgcpDeviceConfigurationKeyIdOffset + 1; ///< PV curtailment limit factor

private:
    // ========================================================================
    // Scenario 1: PV curtailment limit factor
    // ========================================================================
    float pv_curtailment_limit_factor_percent = 100.0f; ///< PV curtailment limit (100% = no curtailment)

    // ========================================================================
    // Scenario 2: Power measurement
    // ========================================================================
    int total_power_w = 0; ///< Total power (positive = consumption, negative = feed-in)

    // ========================================================================
    // Scenario 3 & 4: Energy measurements
    // ========================================================================
    uint32_t energy_feed_in_wh = 0;  ///< Total energy fed into grid
    uint32_t energy_consumed_wh = 0; ///< Total energy consumed from grid

    // ========================================================================
    // Scenario 5: Current measurements
    // ========================================================================
    int current_phase_ma[3] = {0, 0, 0}; ///< Per-phase current in mA

    // ========================================================================
    // Scenario 6: Voltage measurements
    // ========================================================================
    int voltage_phase_v[3] = {230, 230, 230}; ///< Per-phase voltage in V (default 230V)

    // ========================================================================
    // Scenario 7: Frequency measurement
    // ========================================================================
    int frequency_mhz = 50000; ///< Grid frequency in mHz (default 50Hz)

    // ========================================================================
    // Constraints
    // ========================================================================
    int power_limit_min_w = -1000000;          ///< Min power (negative for production)
    int power_limit_max_w = 1000000;           ///< Max power
    int current_limit_min_ma = -100000;        ///< Min current (negative for reverse flow)
    int current_limit_max_ma = 100000;         ///< Max current
    uint32_t energy_limit_max_wh = 1000000000; ///< Max energy (1 TWh)
    int voltage_limit_min_v = 0;               ///< Min voltage
    int voltage_limit_max_v = 500;             ///< Max voltage
    int frequency_limit_min_mhz = 45000;       ///< Min frequency (45 Hz)
    int frequency_limit_max_mhz = 65000;       ///< Max frequency (65 Hz)

    /**
     * @brief Update the API state with current values.
     */
    void update_api() const;
};

#endif // EEBUS_ENABLE_MGCP_USECASE
