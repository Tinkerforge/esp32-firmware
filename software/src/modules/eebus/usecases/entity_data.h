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

#include "../spine_types.h"
#include "config.h"
#include <vector>

/**
 * @brief Data generator functions for the EVSE entity.
 *
 * This class contains static methods that generate SPINE data types for the EVSE
 * entity. Since multiple use cases share features under the same entity, and we
 * only support full reads or notify messages, the full data has to be generated
 * by combining data from all active use cases.
 *
 * Entity Address: {1}
 */
class EVSEEntity
{
public:
    /// Entity address for the EVSE entity
    inline static std::vector<int> entity_address = {1};

    // ========================================================================
    // LoadControl Feature ID Offsets
    // ========================================================================
    static constexpr uint8_t lpcLoadcontrolLimitIdOffset = 0;  ///< LPC load control limit IDs start at 0
    static constexpr uint8_t lppLoadcontrolLimitIdOffset = 10; ///< LPP load control limit IDs start at 10

    /**
     * @brief Get load control limit descriptions from all active use cases.
     * @return Combined LoadControlLimitDescriptionListDataType from LPC and LPP.
     */
    static LoadControlLimitDescriptionListDataType get_load_control_limit_description_list_data();

    /**
     * @brief Get load control limits from all active use cases.
     * @return Combined LoadControlLimitListDataType from LPC and LPP.
     */
    static LoadControlLimitListDataType get_load_control_limit_list_data();

    // ========================================================================
    // DeviceConfiguration Feature ID Offsets
    // ========================================================================
    static constexpr uint8_t lpcDeviceConfigurationKeyIdOffset = 10;  ///< LPC device config keys start at 10
    static constexpr uint8_t lppDeviceConfigurationKeyIdOffset = 20;  ///< LPP device config keys start at 20
    static constexpr uint8_t mgcpDeviceConfigurationKeyIdOffset = 30; ///< MGCP device config keys start at 30

    /**
     * @brief Get device configuration key descriptions from all active use cases.
     * @return Combined DeviceConfigurationKeyValueDescriptionListDataType.
     */
    static DeviceConfigurationKeyValueDescriptionListDataType get_device_configuration_list_data();

    /**
     * @brief Get device configuration key values from all active use cases.
     * @return Combined DeviceConfigurationKeyValueListDataType.
     */
    static DeviceConfigurationKeyValueListDataType get_device_configuration_value_list_data();

    // ========================================================================
    // DeviceDiagnosis Feature
    // ========================================================================

    /**
     * @brief Get device diagnosis state from EVSECC use case.
     * @return DeviceDiagnosisStateDataType with EVSE state.
     */
    static DeviceDiagnosisStateDataType get_state_data();

    // ========================================================================
    // ElectricalConnection Feature ID Offsets
    // ========================================================================
    static constexpr uint8_t lpcElectricalConnectionIdOffset = 0;                ///< LPC electrical connection IDs start at 0
    static constexpr uint8_t lppElectricalConnectionIdOffset = 10;               ///< LPP electrical connection IDs start at 10
    static constexpr uint8_t lpcElectricalConnectionCharacteristicIdOffset = 10; ///< LPC characteristic IDs start at 10
    static constexpr uint8_t lppElectricalConnectionCharacteristicIdOffset = 20; ///< LPP characteristic IDs start at 20
    static constexpr uint8_t lpcElectricalConnectionParameterIdOffset = 0;       ///< LPC parameter IDs start at 0
    static constexpr uint8_t lppElectricalConnectionParameterIdOffset = 0;       ///< LPP parameter IDs start at 0
    static constexpr uint8_t mpcElectricalConnectionParameterIdOffset = 10;      ///< MPC parameter IDs start at 10 (next offset should be +20 at least)
    static constexpr uint8_t mgcpElectricalConnectionIdOffset = 20;              ///< MGCP electrical connection IDs start at 20
    static constexpr uint8_t mgcpElectricalConnectionParameterIdOffset = 30;     ///< MGCP parameter IDs start at 30

    /**
     * @brief Get electrical connection characteristics from all active use cases.
     * @return Combined ElectricalConnectionCharacteristicListDataType.
     */
    static ElectricalConnectionCharacteristicListDataType get_electrical_connection_characteristic_list_data();

    /**
     * @brief Get electrical connection descriptions from all active use cases.
     * @return Combined ElectricalConnectionDescriptionListDataType.
     */
    static ElectricalConnectionDescriptionListDataType get_electrical_connection_description_list_data();

    /**
     * @brief Get electrical connection parameter descriptions from all active use cases.
     * @return Combined ElectricalConnectionParameterDescriptionListDataType.
     */
    static ElectricalConnectionParameterDescriptionListDataType get_electrical_connection_parameter_description_list_data();

    // ========================================================================
    // DeviceClassification Feature
    // ========================================================================

    /**
     * @brief Get device classification manufacturer data from EVSECC use case.
     * @return DeviceClassificationManufacturerDataType with EVSE manufacturer info.
     */
    static DeviceClassificationManufacturerDataType get_device_classification_manufacturer_data();

    // ========================================================================
    // Measurement Feature ID Offsets
    // ========================================================================
    static constexpr uint8_t lpcMeasurementIdOffset = 0;   ///< LPC measurement IDs start at 0
    static constexpr uint8_t lppMeasurementIdOffset = 40;  ///< LPP measurement IDs start at 40
    static constexpr uint8_t mpcMeasurementIdOffset = 10;  ///< MPC measurement IDs start at 10 (next offset should be +20)
    static constexpr uint8_t mgcpMeasurementIdOffset = 40; ///< MGCP uses high offsets to avoid conflicts

    /**
     * @brief Get measurement descriptions from all active use cases.
     * @return Combined MeasurementDescriptionListDataType.
     */
    static MeasurementDescriptionListDataType get_measurement_description_list_data();

    /**
     * @brief Get measurement constraints from all active use cases.
     * @return Combined MeasurementConstraintsListDataType.
     */
    static MeasurementConstraintsListDataType get_measurement_constraints_list_data();

    /**
     * @brief Get measurement values from all active use cases.
     * @return Combined MeasurementListDataType.
     */
    static MeasurementListDataType get_measurement_list_data();

    // ========================================================================
    // Bill Feature (EVCS use case)
    // ========================================================================

    /**
     * @brief Get bill description list from EVCS use case.
     * @return BillDescriptionListDataType.
     */
    static BillDescriptionListDataType get_bill_description_list_data();

    /**
     * @brief Get bill constraints list from EVCS use case.
     * @return BillConstraintsListDataType.
     */
    static BillConstraintsListDataType get_bill_constraints_list_data();

    /**
     * @brief Get bill list from EVCS use case.
     * @return BillListDataType.
     */
    static BillListDataType get_bill_list_data();
};

/**
 * @brief Data generator functions for the EV entity.
 *
 * This class contains static methods that generate SPINE data types for the EV
 * entity. Since multiple use cases share features under the same entity, and we
 * only support full reads or notify messages, the full data has to be generated
 * by combining data from all active use cases.
 *
 * Entity Address: {1, 1}
 */
class EVEntity
{
public:
    /// Entity address for the EV entity
    inline static std::vector<int> entity_address = {1, 1};

    // ========================================================================
    // DeviceConfiguration Feature ID Offsets
    // ========================================================================
    static constexpr uint8_t evccDeviceconfigurationIdOffset = 0; ///< EVCC device config IDs start at 0

    /**
     * @brief Get device configuration key descriptions from EVCC use case.
     * @return DeviceConfigurationKeyValueDescriptionListDataType.
     */
    static DeviceConfigurationKeyValueDescriptionListDataType get_device_configuration_value_description_list();

    /**
     * @brief Get device configuration key values from EVCC use case.
     * @return DeviceConfigurationKeyValueListDataType.
     */
    static DeviceConfigurationKeyValueListDataType get_device_configuration_value_list();

    // ========================================================================
    // Identification Feature
    // ========================================================================

    /**
     * @brief Get identification list from EVCC use case.
     * @return IdentificationListDataType.
     */
    static IdentificationListDataType get_identification_list_data();

    // ========================================================================
    // DeviceClassification Feature
    // ========================================================================

    /**
     * @brief Get device classification manufacturer data from EVCC use case.
     * @return DeviceClassificationManufacturerDataType with EV manufacturer info.
     */
    static DeviceClassificationManufacturerDataType get_device_classification_manufacturer_data();

    // ========================================================================
    // ElectricalConnection Feature ID Offsets
    // ========================================================================
    static constexpr uint8_t evccElectricalConnectionIdOffset = 0;           ///< EVCC electrical connection IDs start at 0
    static constexpr uint8_t evcemElectricalconnectionParameterIdOffset = 0; ///< EVCEM parameter IDs start at 0
    static constexpr uint8_t opevElectricalconnectionParameterIdOffset = 10; ///< OPEV parameter IDs start at 10
    static constexpr uint8_t evccElectricalconnectionParameterIdOffset = 20; ///< EVCC parameter IDs start at 20

    /**
     * @brief Get electrical connection parameter descriptions from all active use cases.
     * @return Combined ElectricalConnectionParameterDescriptionListDataType.
     */
    static ElectricalConnectionParameterDescriptionListDataType get_electrical_connection_parameter_description_list_data();

    /**
     * @brief Get electrical connection permitted values from all active use cases.
     * @return Combined ElectricalConnectionPermittedValueSetListDataType.
     */
    static ElectricalConnectionPermittedValueSetListDataType get_electrical_connection_permitted_list_data();

    /**
     * @brief Get electrical connection descriptions from EVCEM use case.
     * @return ElectricalConnectionDescriptionListDataType.
     */
    static ElectricalConnectionDescriptionListDataType get_electrical_connection_description_list_data();

    // ========================================================================
    // DeviceDiagnosis Feature
    // ========================================================================

    /**
     * @brief Get device diagnosis state from EVCC use case.
     * @return DeviceDiagnosisStateDataType with EV state.
     */
    static DeviceDiagnosisStateDataType get_diagnosis_state_data();

    // ========================================================================
    // Measurement Feature ID Offsets
    // ========================================================================
    static constexpr uint8_t evcemMeasurementIdOffset = 0; ///< EVCEM measurement IDs start at 0

    /**
     * @brief Get measurement descriptions from EVCEM use case.
     * @return MeasurementDescriptionListDataType.
     */
    static MeasurementDescriptionListDataType get_measurement_description_list_data();

    /**
     * @brief Get measurement constraints from EVCEM use case.
     * @return MeasurementConstraintsListDataType.
     */
    static MeasurementConstraintsListDataType get_measurement_constraints_list_data();

    /**
     * @brief Get measurement values from EVCEM use case.
     * @return MeasurementListDataType.
     */
    static MeasurementListDataType get_measurement_list_data();

    // ========================================================================
    // LoadControl Feature ID Offsets (OPEV use case)
    // ========================================================================
    static constexpr uint8_t opevLoadcontrolIdOffset = 0;  ///< OPEV load control IDs start at 0
    static constexpr uint8_t opevMeasurementIdOffset = 10; ///< OPEV measurement IDs start at 10

    /**
     * @brief Get load control limit descriptions from OPEV use case.
     * @return LoadControlLimitDescriptionListDataType.
     */
    static LoadControlLimitDescriptionListDataType get_load_control_limit_description_list_data();

    /**
     * @brief Get load control limits from OPEV use case.
     * @return LoadControlLimitListDataType.
     */
    static LoadControlLimitListDataType get_load_control_limit_list_data();

    /**
     * @brief Get load control limit constraints from OPEV use case.
     * @return LoadControlLimitConstraintsListDataType.
     */
    static LoadControlLimitConstraintsListDataType get_load_control_limit_constraints_list_data();

    // ========================================================================
    // TimeSeries Feature (CEVC use case)
    // ========================================================================

    /**
     * @brief Get time series descriptions from CEVC use case.
     * @return TimeSeriesDescriptionListDataType.
     */
    static TimeSeriesDescriptionListDataType get_time_series_description_list_data();

    /**
     * @brief Get time series constraints from CEVC use case.
     * @return TimeSeriesConstraintsListDataType.
     */
    static TimeSeriesConstraintsListDataType get_time_series_constraints_list_data();

    /**
     * @brief Get time series data from CEVC use case.
     * @return TimeSeriesListDataType.
     */
    static TimeSeriesListDataType get_time_series_list_data();

    // ========================================================================
    // IncentiveTable Feature (CEVC use case)
    // ========================================================================

    /**
     * @brief Get incentive table descriptions from CEVC use case.
     * @return IncentiveTableDescriptionDataType.
     */
    static IncentiveTableDescriptionDataType get_incentive_table_description_data();

    /**
     * @brief Get incentive table constraints from CEVC use case.
     * @return IncentiveTableConstraintsDataType.
     */
    static IncentiveTableConstraintsDataType get_incentive_table_constraints_data();

    /**
     * @brief Get incentive table data from CEVC use case.
     * @return IncentiveTableDataType.
     */
    static IncentiveTableDataType get_incentive_table_data();
};
