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

#include "cool_string.h"
#include "entity_data.h"
#include "usecase_base.h"

#ifdef EEBUS_ENABLE_EVCC_USECASE

/**
 * @brief The EV Commissioning and Configuration (EVCC) use case.
 *
 * Implements EEBus UC TS - EV Commissioning and Configuration V1.0.1.
 * Provides EV identification, manufacturer information, electrical connection
 * parameters, and device diagnosis for connected EVs.
 *
 * Scenarios:
 *   1 (2.3.1, 3.4.1): EV connected
 *   2 (2.3.2, 3.4.2): Communication standard
 *   3 (2.3.3, 3.4.3): Asymmetric charging
 *   4 (2.3.4, 3.4.4): EV identification
 *   5 (2.3.5, 3.4.5): Manufacturer information
 *   6 (2.3.6, 3.4.6): Charging power limits
 *   7 (2.3.7, 3.4.7): EV sleep mode
 *   8 (2.3.8, 3.4.8): EV disconnected
 *
 * Actor: EV
 * Entity address: {1, 1}
 * Features:
 *   - DeviceConfiguration (deviceConfigurationKeyValueDescriptionListData, deviceConfigurationKeyValueListData)
 *   - Identification (identificationListData)
 *   - DeviceClassification (deviceClassificationManufacturerData)
 *   - ElectricalConnection (electricalConnectionParameterDescriptionListData, electricalConnectionPermittedValueSetListData)
 *   - DeviceDiagnosis (deviceDiagnosisStateData)
 *
 * @see EEBus_UC_TS_EVCommissioningAndConfiguration_V1.0.1.pdf
 */
class EvccUsecase final : public EebusUsecase
{
public:
    EvccUsecase();

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::EVCC;
    }

    /**
     * @brief Handles a message for the EVCC use case.
     * @param header SPINE header with commandclassifier and targeted entity info
     * @param data The actual function call and data of the message
     * @param response JsonObject to write the response to
     * @return MessageReturn with handling status and response info
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * @brief Inform the usecases when an EV has been connected or disconnected.
     *
     * This will inform the subscribers of the NodeManagement entity about the change.
     * Implements Scenario 1 (2.3.1, 3.4.1) and Scenario 8 (2.3.8, 3.4.8).
     *
     * @param connected True if EV is connected, false if disconnected
     */
    void ev_connected_state(bool connected);

    /**
     * @brief Update the device configuration.
     *
     * As required by Scenario 2 and 3 of the EVCC Usecase.
     * This will inform all subscribers of the new configuration.
     *
     * @param communication_standard The communication standard. Should only be "iso15118-2ed1", "iso15118-2ed2", or "iec61851"
     * @param asymmetric_supported If asymmetric charging is supported or not
     */
    void update_device_config(const String &communication_standard, bool asymmetric_supported = false);

    /**
     * @brief Update the identification of the EV.
     *
     * As required by Scenario 4 of the Usecase.
     * This will inform all subscribers of the new identification.
     *
     * @param mac_address MAC address of the EV. Should be in the format given as mac_type
     * @param mac_type Either eui48 or eui64. Default is eui64
     */
    void update_identification(String mac_address, IdentificationTypeEnumType mac_type = IdentificationTypeEnumType::eui64);

    /**
     * @brief Updates the manufacturer data of the EV.
     *
     * As required by Scenario 5 of the Usecase.
     * This will inform all subscribers of the new manufacturer data.
     * Entries with a length < 1 will not be added.
     *
     * @param name Device name. Recommended to be set
     * @param code Device code. Recommended to be set
     * @param serial Serial Number of the device. Optional
     * @param software_vers Software revision of the device. Optional
     * @param hardware_vers Hardware revision of the device. Optional
     * @param vendor_name Vendor Name. Recommended to be set
     * @param vendor_code Vendor code. Recommended to be set
     * @param brand_name Brand name. Recommended to be set
     * @param manufacturer Manufacturer Label. Recommended to be set
     * @param manufacturer_description Manufacturer Description. Optional
     */
    void update_manufacturer(String name = "", String code = "", String serial = "", String software_vers = "", String hardware_vers = "", String vendor_name = "", String vendor_code = "", String brand_name = "", String manufacturer = "", String manufacturer_description = "");

    /**
     * @brief Update the Electrical Connection feature.
     *
     * As required by Scenario 6 of the Usecase.
     * This will inform all subscribers of the new electrical connection data.
     *
     * @param min_power In Watts. Minimum power at which the EV can still charge. Default is 0W
     * @param max_power In Watts. Maximum power at which the EV can charge. Default is EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION
     * @param standby_power In Watts. If the EV has a standby mode and how much power it is capable of consuming in it
     */
    void update_electrical_connection(int min_power = 0, int max_power = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION, int standby_power = 0);

    /**
     * @brief Update the DeviceDiagnosis state.
     *
     * As required by Scenario 7 of the Usecase.
     * This will inform all subscribers of the new device diagnosis state.
     *
     * @param standby If the device is in standby mode or not
     */
    void update_operating_state(bool standby);

    [[nodiscard]] bool is_ev_connected() const
    {
        return ev_connected;
    }

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::DeviceConfiguration, FeatureTypeEnumType::DeviceDiagnosis, FeatureTypeEnumType::Identification, FeatureTypeEnumType::DeviceClassification, FeatureTypeEnumType::ElectricalConnection};
    }

    // Data generators for SPINE data types
    void get_device_config_description(DeviceConfigurationKeyValueDescriptionListDataType *data) const;
    void get_device_config_list(DeviceConfigurationKeyValueListDataType *data) const;
    void get_identification_list(IdentificationListDataType *data) const;
    void get_device_classification_manufacturer(DeviceClassificationManufacturerDataType *data) const;
    void get_electrical_connection_parameter_description(ElectricalConnectionParameterDescriptionListDataType *data) const;
    void get_electrical_connection_permitted_values(ElectricalConnectionPermittedValueSetListDataType *data) const;
    [[nodiscard]] DeviceDiagnosisStateDataType get_device_diagnosis_state() const;

    // ========================================================================
    // ID Constants
    // IDs as defined in EV Commissioning and Configuration V1.0.1 3.2.1.2
    // ========================================================================

    /// Device configuration key IDs
    static constexpr uint16_t id_x_1 = EVEntity::evccDeviceconfigurationIdOffset + 1;
    static constexpr uint16_t id_x_2 = EVEntity::evccDeviceconfigurationIdOffset + 2;
    /// Electrical connection ID
    static constexpr uint16_t id_y_1 = EVEntity::evccElectricalConnectionIdOffset + 1;
    /// Electrical connection parameter ID
    static constexpr uint16_t id_z_1 = EVEntity::evccElectricalconnectionParameterIdOffset + 1;

private:
    void update_api() const;
    bool ev_connected = false;

    // Server Data
    // DeviceConfiguration
    CoolString communication_standard = "";
    bool asymmetric_supported = false;

    // Identification
    IdentificationTypeEnumType mac_type = IdentificationTypeEnumType::eui64;
    CoolString mac_address = "";

    // DeviceClassification
    CoolString manufacturer_name = "";
    CoolString manufacturer_code = "";
    CoolString ev_serial_number = "";
    CoolString ev_sofware_version = "";
    CoolString ev_hardware_version = "";
    CoolString vendor_name = "";
    CoolString vendor_code = "";
    CoolString brand_name = "";
    CoolString manufacturer_label = "";
    CoolString manufacturer_description = "";

    // ElectricalConnection
    int min_power_draw = 0;
    int max_power_draw = 0;
    int standby_power = 0;

    // DeviceDiagnosis
    bool standby_mode = false;
};

#endif // EEBUS_ENABLE_EVCC_USECASE
