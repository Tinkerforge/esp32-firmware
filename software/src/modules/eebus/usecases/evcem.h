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

#ifdef EEBUS_ENABLE_EVCEM_USECASE

// Forward declaration for EVCC reference
#ifdef EEBUS_ENABLE_EVCC_USECASE
class EvccUsecase;
#endif

/**
 * @brief The EV Charging Electricity Measurement (EVCEM) use case.
 *
 * Implements EEBus UC TS - EV Charging Electricity Measurement V1.0.1.
 * Provides measurement data for current, power, and energy during EV charging.
 *
 * Scenarios:
 *   1 (3.2.1.2.1.1): Current measurement per phase
 *   2 (3.2.1.2.1.2): Power measurement per phase
 *   3 (3.2.1.2.1.3): Energy measurement (total charged)
 *
 * Actor: EV
 * Entity address: {1, 1}
 * Features:
 *   - Measurement (measurementDescriptionListData, measurementConstraintsListData, measurementListData)
 *   - ElectricalConnection (electricalConnectionDescriptionListData, electricalConnectionParameterDescriptionListData)
 *
 * @see EEBus_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf
 */
class EvcemUsecase final : public EebusUsecase
{
public:
    EvcemUsecase();

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::EVCEM;
    }

    /**
     * @brief Handles a message for the EVCEM use case.
     * @param header SPINE header with commandclassifier and targeted entity info
     * @param data The actual function call and data of the message
     * @param response JsonObject to write the response to
     * @return MessageReturn with handling status and response info
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    /**
     * @brief Builds and returns the use case information.
     *
     * As defined in EEBus UC TS - EV Charging Electricity Measurement V1.0.1 section 3.1.2.
     *
     * @return UseCaseInformationDataType with scenarios 1-3 support
     */
    UseCaseInformationDataType get_usecase_information() override;

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * @brief Update the measurements.
     *
     * Updates current, power, and energy measurements and informs all subscribers.
     * Values at or below 0 are ignored and not sent.
     *
     * @param amps_phase_1 Milliamps on phase a
     * @param amps_phase_2 Milliamps on phase b
     * @param amps_phase_3 Milliamps on phase c
     * @param power_phase_1 Total power on phase 1 in watts
     * @param power_phase_2 Total power on phase 2 in watts
     * @param power_phase_3 Total power on phase 3 in watts
     * @param charged_wh Total charged into the EV during the current session in Wh
     * @param charged_measured If the charged_wh value is measured (true) or calculated (false)
     */
    void update_measurements(int amps_phase_1, int amps_phase_2, int amps_phase_3, int power_phase_1, int power_phase_2, int power_phase_3, int charged_wh, bool charged_measured = false);

    /**
     * @brief Update the measurement constraints.
     *
     * Updates the min/max/stepsize constraints for measurements and informs all subscribers.
     * Values set to negative are omitted from the EEBUS information.
     *
     * @param amps_min Minimum amps in mA that can be measured
     * @param amps_max Maximum amps in mA that can be measured
     * @param amps_stepsize Stepsize of the amps measurement in mA
     * @param power_min Minimum power in W that can be measured
     * @param power_max Maximum power in W that can be measured
     * @param power_stepsize Stepsize of the power measurement in W
     * @param energy_min Minimum charged energy in Wh that can be measured
     * @param energy_max Maximum charged energy in Wh that can be measured
     * @param energy_stepsize Stepsize of the charged energy measurement in Wh
     */
    void update_constraints(int amps_min, int amps_max, int amps_stepsize, int power_min, int power_max, int power_stepsize, int energy_min, int energy_max, int energy_stepsize);

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::Measurement, FeatureTypeEnumType::ElectricalConnection};
    }

    // Data generators for SPINE data types
    void get_measurement_description_list(MeasurementDescriptionListDataType *data) const;
    void get_measurement_constraints(MeasurementConstraintsListDataType *data) const;
    void get_measurement_list(MeasurementListDataType *data) const;

    void get_electrical_connection_description(ElectricalConnectionDescriptionListDataType *data) const;
    void get_electrical_connection_parameters(ElectricalConnectionParameterDescriptionListDataType *data) const;

    // ========================================================================
    // ID Constants
    // IDs as defined in EV Charging Electricity Measurement V1.0.1 3.2.1.2
    // and referenced in Overload Protection by EV Charging Current Curtailment V1.0.1b 3.2.1.2
    // ========================================================================

    /// Measurement IDs for current per phase (x_1, x_2, x_3)
    static constexpr uint8_t id_x_1 = EVEntity::evcemMeasurementIdOffset + 1;
    static constexpr uint8_t id_x_2 = EVEntity::evcemMeasurementIdOffset + 2;
    static constexpr uint8_t id_x_3 = EVEntity::evcemMeasurementIdOffset + 3;
    /// Measurement IDs for power per phase (x_4, x_5, x_6)
    static constexpr uint8_t id_x_4 = EVEntity::evcemMeasurementIdOffset + 4;
    static constexpr uint8_t id_x_5 = EVEntity::evcemMeasurementIdOffset + 5;
    static constexpr uint8_t id_x_6 = EVEntity::evcemMeasurementIdOffset + 6;
    /// Measurement ID for total charged energy (x_7)
    static constexpr uint8_t id_x_7 = EVEntity::evcemMeasurementIdOffset + 7;

    /// Electrical connection ID (references EVCC id_y_1)
    static constexpr uint8_t id_y_1 = EVEntity::evccElectricalConnectionIdOffset + 1;

    /// Electrical connection parameter IDs per phase (z_1 to z_7)
    static constexpr uint8_t id_z_1 = EVEntity::evcemElectricalconnectionParameterIdOffset + 1;
    static constexpr uint8_t id_z_2 = EVEntity::evcemElectricalconnectionParameterIdOffset + 2;
    static constexpr uint8_t id_z_3 = EVEntity::evcemElectricalconnectionParameterIdOffset + 3;
    static constexpr uint8_t id_z_4 = EVEntity::evcemElectricalconnectionParameterIdOffset + 4;
    static constexpr uint8_t id_z_5 = EVEntity::evcemElectricalconnectionParameterIdOffset + 5;
    static constexpr uint8_t id_z_6 = EVEntity::evcemElectricalconnectionParameterIdOffset + 6;
    static constexpr uint8_t id_z_7 = EVEntity::evcemElectricalconnectionParameterIdOffset + 7;

private:
    // Current measurement data held about the current charge
    int milliamps_draw_phase[3] = {1, 1, 1}; ///< Milliamp draw per phase. Set to 1 to avoid zero values
    int power_draw_phase[3] = {1, 1, 1};     ///< Power per phase in W. Set to 1 to avoid zero values
    int power_charged_wh = 1;                ///< Total charged into the EV during current session in Wh
    bool power_charged_measured = false;     ///< Whether power_charged_wh is measured (true) or calculated (false)

    // Measurement constraints
    int measurement_limit_milliamps_min = 1;
    int measurement_limit_milliamps_max = 32000;
    int measurement_limit_milliamps_stepsize = 1;
    int measurement_limit_power_min = 1;
    int measurement_limit_power_max = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    int measurement_limit_power_stepsize = 10;
    int measurement_limit_energy_min = 1;
    int measurement_limit_energy_max = 1000000;
    int measurement_limit_energy_stepsize = 10;

    void update_api() const;
};

#endif // EEBUS_ENABLE_EVCEM_USECASE
