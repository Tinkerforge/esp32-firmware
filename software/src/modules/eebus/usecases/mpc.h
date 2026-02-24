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

// Forward declarations for conditional compilation
#ifdef EEBUS_ENABLE_LPC_USECASE
class LpcUsecase;
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
class LppUsecase;
#endif

#ifdef EEBUS_ENABLE_MPC_USECASE

/**
 * @brief Monitoring of Power Consumption (MPC) usecase.
 *
 * As defined in EEBus UC TS - Monitoring of Power Consumption V1.0.0.
 * This should have the same entity address as other entities with the EVSE.
 *
 * Actor: MonitoredUnit
 * Entity address: {1}
 * Features:
 *   - ElectricalConnection (electricalConnectionDescriptionListData,
 *                           electricalConnectionParameterDescriptionListData)
 *   - Measurement (measurementDescriptionListData, measurementConstraintsListData,
 *                  measurementListData)
 *
 * Scenarios:
 *   1: Monitor Power - Mandatory
 *   2: Monitor Energy - Optional
 *   3: Monitor Current - Recommended
 *   4: Monitor Voltage - Optional
 *   5: Monitor Frequency - Optional
 *
 * @see EEBus_UC_TS_MonitoringOfPowerConsumption_V1.0.0.pdf
 */
class MpcUsecase final : public EebusUsecase
{
public:
    MpcUsecase();

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::MPC;
    }

    /**
     * @brief Handles a message for the usecase.
     * @param header SPINE header of the message. Contains information about the
     *               commandclassifier and the targeted entity.
     * @param data The actual Function call and data of the message.
     * @param response Where to write the response to. This is a JsonObject that
     *                 should be filled with the response data.
     * @return The MessageReturn object which contains information about the returned message.
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    /**
     * @brief Builds and returns the UseCaseInformationDataType.
     *
     * As defined in EEBus UC TS - Monitoring of Power Consumption V1.0.0 3.1.2.
     *
     * @return UseCaseInformationDataType with all 5 scenarios support
     */
    UseCaseInformationDataType get_usecase_information() override;

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::ElectricalConnection, FeatureTypeEnumType::Measurement};
    }

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    // =========================================================================
    // Data retrieval methods
    // =========================================================================
    void get_electricalConnection_description_list_data(ElectricalConnectionDescriptionListDataType *data);
    void get_electricalConnection_parameter_description_list_data(ElectricalConnectionParameterDescriptionListDataType *data) const;
    void get_measurement_description_list_data(MeasurementDescriptionListDataType *data);
    void get_measurement_constraints_list_data(MeasurementConstraintsListDataType *data) const;
    void get_measurement_list_data(MeasurementListDataType *data) const;

    // =========================================================================
    // Update methods - Power measurements
    // =========================================================================
    /**
     * @brief Update the power measurements.
     *
     * This will inform all subscribers of the new measurements.
     * All values are accepted including 0 and negative values (negative = power feed-in).
     *
     * @param total_power Total power consumption in watts (id_m_1) - Linked to LPC.
     *                    Negative values indicate power feed-in.
     * @param power_phase_1 Power on phase 1 in watts (id_m_2_1). Negative values indicate power feed-in.
     * @param power_phase_2 Power on phase 2 in watts (id_m_2_2). Negative values indicate power feed-in.
     * @param power_phase_3 Power on phase 3 in watts (id_m_2_3). Negative values indicate power feed-in.
     */
    void update_power(int total_power, int power_phase_1, int power_phase_2, int power_phase_3);

    // =========================================================================
    // Update methods - Energy measurements
    // =========================================================================
    /**
     * @brief Update the energy measurements.
     *
     * This will inform all subscribers of the new measurements.
     * All values are accepted including 0.
     *
     * @param energy_consumed Energy consumed in watt-hours (id_m_3)
     * @param energy_produced Energy produced in watt-hours (id_m_4)
     */
    void update_energy(uint32_t energy_consumed, uint32_t energy_produced);

    // =========================================================================
    // Update methods - Current measurements
    // =========================================================================
    /**
     * @brief Update the current measurements.
     *
     * This will inform all subscribers of the new measurements.
     * All values are accepted including 0 and negative values (negative = reverse current flow).
     *
     * @param current_phase_1 Current on phase 1 in milliamps (id_m_5_1). Negative values indicate reverse flow.
     * @param current_phase_2 Current on phase 2 in milliamps (id_m_5_2). Negative values indicate reverse flow.
     * @param current_phase_3 Current on phase 3 in milliamps (id_m_5_3). Negative values indicate reverse flow.
     */
    void update_current(int current_phase_1, int current_phase_2, int current_phase_3);

    // =========================================================================
    // Update methods - Voltage measurements
    // =========================================================================
    /**
     * @brief Update the voltage measurements.
     *
     * This will inform all subscribers of the new measurements.
     * All values are accepted including 0. If INT32_MIN is passed to any of the
     * phase_to_phase voltage parameters, it is assumed that the phase-to-phase
     * voltage is not measured and shall therefore be omitted from the data.
     *
     * @param voltage_phase_1 Voltage on phase 1 (phase-to-neutral) in volts (id_m_6_1)
     * @param voltage_phase_2 Voltage on phase 2 (phase-to-neutral) in volts (id_m_6_2)
     * @param voltage_phase_3 Voltage on phase 3 (phase-to-neutral) in volts (id_m_6_3)
     * @param voltage_phase_1_2 Voltage between phase 1 and 2 in volts (id_m_6_4)
     * @param voltage_phase_2_3 Voltage between phase 2 and 3 in volts (id_m_6_5)
     * @param voltage_phase_3_1 Voltage between phase 3 and 1 in volts (id_m_6_6)
     */
    void update_voltage(int voltage_phase_1, int voltage_phase_2, int voltage_phase_3, int voltage_phase_1_2, int voltage_phase_2_3, int voltage_phase_3_1);

    // =========================================================================
    // Update methods - Frequency measurement
    // =========================================================================
    /**
     * @brief Update the frequency measurement.
     *
     * This will inform all subscribers of the new measurement.
     * All values are accepted including 0.
     *
     * @param frequency_millihertz Grid frequency in millihertz (id_m_7), default is 50000 (50Hz)
     */
    void update_frequency(int frequency_millihertz);

    // =========================================================================
    // Update methods - Constraints
    // =========================================================================
    /**
     * @brief Update all measurement constraints.
     *
     * This will inform all subscribers of the new constraints.
     * Values set to negative or 0 will use defaults.
     *
     * @param power_min Minimum power that can be measured in watts
     * @param power_max Maximum power that can be measured in watts
     * @param power_stepsize Step size for power measurements in watts
     * @param current_min_ma Minimum current that can be measured in milliamps
     * @param current_max_ma Maximum current that can be measured in milliamps
     * @param current_stepsize_ma Step size for current measurements in milliamps
     * @param energy_min_wh Minimum energy that can be measured in watt-hours
     * @param energy_max_wh Maximum energy that can be measured in watt-hours
     * @param energy_stepsize_wh Step size for energy measurements in watt-hours
     * @param voltage_min Minimum voltage that can be measured in volts
     * @param voltage_max Maximum voltage that can be measured in volts
     * @param voltage_stepsize Step size for voltage measurements in volts
     * @param frequency_min_mhz Minimum frequency in millihertz
     * @param frequency_max_mhz Maximum frequency in millihertz
     * @param frequency_stepsize_mhz Step size for frequency in millihertz
     */
    void update_constraints(int power_min, int power_max, int power_stepsize, int current_min_ma, int current_max_ma, int current_stepsize_ma, uint32_t energy_min_wh, uint32_t energy_max_wh, uint32_t energy_stepsize_wh, int voltage_min, int voltage_max, int voltage_stepsize, int frequency_min_mhz, int frequency_max_mhz, int frequency_stepsize_mhz);

    // =========================================================================
    // IDs as used in Monitoring of Power Consumption V1.0.0 3.2.2.2
    // Linked IDs are described in the spec for the LPC usecase
    // =========================================================================
#ifdef EEBUS_ENABLE_LPC_USECASE
    static constexpr uint8_t id_ec_1 = EVSEEntity::lpcElectricalConnectionIdOffset + 1;         // Linked to LPC usecase electrical connection description
    static constexpr uint8_t id_m_1 = EVSEEntity::lpcMeasurementIdOffset + 1;                   // Linked to LPC usecase measurement description
    static constexpr uint8_t id_p_1 = EVSEEntity::lpcElectricalConnectionParameterIdOffset + 1; // Linked to LPC usecase electrical connection parameter description
#elifdef EEBUS_ENABLE_LPP_USECASE
    static constexpr uint8_t id_ec_1 = EVSEEntity::lppElectricalConnectionIdOffset + 1;         // Linked to LPP usecase electrical connection description
    static constexpr uint8_t id_m_1 = EVSEEntity::lppMeasurementIdOffset + 1;                   // Linked to LPP usecase measurement description
    static constexpr uint8_t id_p_1 = EVSEEntity::lppElectricalConnectionParameterIdOffset + 1; // Linked to LPP usecase electrical connection parameter description
#endif

    // MPC-specific measurement IDs
    static constexpr uint8_t id_m_2_1 = EVSEEntity::mpcMeasurementIdOffset + 2;
    static constexpr uint8_t id_m_2_2 = EVSEEntity::mpcMeasurementIdOffset + 3;
    static constexpr uint8_t id_m_2_3 = EVSEEntity::mpcMeasurementIdOffset + 4;
    static constexpr uint8_t id_m_3 = EVSEEntity::mpcMeasurementIdOffset + 5;
    static constexpr uint8_t id_m_4 = EVSEEntity::mpcMeasurementIdOffset + 6;
    static constexpr uint8_t id_m_5_1 = EVSEEntity::mpcMeasurementIdOffset + 7;
    static constexpr uint8_t id_m_5_2 = EVSEEntity::mpcMeasurementIdOffset + 8;
    static constexpr uint8_t id_m_5_3 = EVSEEntity::mpcMeasurementIdOffset + 9;
    static constexpr uint8_t id_m_6_1 = EVSEEntity::mpcMeasurementIdOffset + 10;
    static constexpr uint8_t id_m_6_2 = EVSEEntity::mpcMeasurementIdOffset + 11;
    static constexpr uint8_t id_m_6_3 = EVSEEntity::mpcMeasurementIdOffset + 12;
    static constexpr uint8_t id_m_6_4 = EVSEEntity::mpcMeasurementIdOffset + 13;
    static constexpr uint8_t id_m_6_5 = EVSEEntity::mpcMeasurementIdOffset + 14;
    static constexpr uint8_t id_m_6_6 = EVSEEntity::mpcMeasurementIdOffset + 15;
    static constexpr uint8_t id_m_7 = EVSEEntity::mpcMeasurementIdOffset + 16;

    // MPC-specific electrical connection parameter IDs
    static constexpr uint8_t id_p_2 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 2;
    static constexpr uint8_t id_p_2_1 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 3;
    static constexpr uint8_t id_p_2_2 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 4;
    static constexpr uint8_t id_p_2_3 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 5;
    static constexpr uint8_t id_p_3 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 6;
    static constexpr uint8_t id_p_4 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 7;
    static constexpr uint8_t id_p_5_1 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 8;
    static constexpr uint8_t id_p_5_2 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 9;
    static constexpr uint8_t id_p_5_3 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 10;
    static constexpr uint8_t id_p_6_1 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 11;
    static constexpr uint8_t id_p_6_2 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 12;
    static constexpr uint8_t id_p_6_3 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 13;
    static constexpr uint8_t id_p_6_4 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 14;
    static constexpr uint8_t id_p_6_5 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 15;
    static constexpr uint8_t id_p_6_6 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 16;
    static constexpr uint8_t id_p_7 = EVSEEntity::mpcElectricalConnectionParameterIdOffset + 17;

private:
    // =========================================================================
    // Measurement values - Scenario 1: Monitor Power
    // =========================================================================
    int total_power_w = 0;            ///< Total power (id_m_1) - Linked to LPC
    int power_phase_w[3] = {0, 0, 0}; ///< Power per phase (id_m_2_1, id_m_2_2, id_m_2_3)

    // =========================================================================
    // Measurement values - Scenario 2: Monitor Energy
    // =========================================================================
    uint32_t energy_consumed_wh = 0; ///< Energy consumed (id_m_3)
    uint32_t energy_produced_wh = 0; ///< Energy produced (id_m_4)

    // =========================================================================
    // Measurement values - Scenario 3: Monitor Current
    // =========================================================================
    int current_phase_ma[3] = {0, 0, 0}; ///< Current per phase in mA (id_m_5_1, id_m_5_2, id_m_5_3)

    // =========================================================================
    // Measurement values - Scenario 4: Monitor Voltage
    // =========================================================================
    int voltage_phase_to_neutral_v[3] = {0, 0, 0}; ///< Voltage phase-to-neutral (id_m_6_1, id_m_6_2, id_m_6_3)
    int voltage_phase_to_phase_v[3] = {0, 0, 0};   ///< Voltage phase-to-phase (id_m_6_4, id_m_6_5, id_m_6_6)
    bool phase_to_phase_available = true;          ///< Whether phase-to-phase voltage measurement is available

    // =========================================================================
    // Measurement values - Scenario 5: Monitor Frequency
    // =========================================================================
    int frequency_mhz = 50000; ///< Grid frequency in millihertz (id_m_7), default 50Hz

    // =========================================================================
    // Constraint values - Power
    // =========================================================================
    int power_limit_min_w = 0;                                          ///< Minimum power that can be measured
    int power_limit_max_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION; ///< Maximum power that can be measured
    int power_limit_stepsize_w = 1;                                     ///< Step size for power measurements

    // =========================================================================
    // Constraint values - Current
    // =========================================================================
    int current_limit_min_ma = 0;      ///< Minimum current that can be measured (mA)
    int current_limit_max_ma = 32000;  ///< Maximum current that can be measured (mA)
    int current_limit_stepsize_ma = 1; ///< Step size for current measurements (mA)

    // =========================================================================
    // Constraint values - Energy
    // =========================================================================
    uint32_t energy_limit_min_wh = 0;         ///< Minimum energy that can be measured
    uint32_t energy_limit_max_wh = 100000000; ///< Maximum energy that can be measured (100MWh)
    uint32_t energy_limit_stepsize_wh = 1;    ///< Step size for energy measurements

    // =========================================================================
    // Constraint values - Voltage
    // =========================================================================
    int voltage_limit_min_v = 0;      ///< Minimum voltage that can be measured
    int voltage_limit_max_v = 500;    ///< Maximum voltage that can be measured
    int voltage_limit_stepsize_v = 1; ///< Step size for voltage measurements

    // =========================================================================
    // Constraint values - Frequency
    // =========================================================================
    int frequency_limit_min_mhz = 45000;   ///< Minimum frequency in mHz (45Hz)
    int frequency_limit_max_mhz = 65000;   ///< Maximum frequency in mHz (65Hz)
    int frequency_limit_stepsize_mhz = 10; ///< Step size for frequency in mHz (0.01Hz)

    // =========================================================================
    // Helper methods
    // =========================================================================
    void update_api() const;
};

#endif // EEBUS_ENABLE_MPC_USECASE
