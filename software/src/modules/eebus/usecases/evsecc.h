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

#ifdef EEBUS_ENABLE_EVSECC_USECASE

/**
 * @brief The EVSE Commissioning and Configuration (EVSECC) use case.
 *
 * Implements EEBus UC TS - EVSE Commissioning and Configuration V1.0.1.
 * Provides EVSE manufacturer information and operating state/error reporting.
 *
 * Scenarios:
 *   1 (2.3.1, 3.4.1): EVSE manufacturer information
 *   2 (2.3.2, 3.4.2): EVSE error state
 *
 * Actor: EVSE
 * Entity address: {1}
 * Features:
 *   - DeviceClassification (deviceClassificationManufacturerData)
 *   - DeviceDiagnosis (deviceDiagnosisStateData)
 *
 * @see EEBus_UC_TS_EVSECommissioningAndConfiguration_V1.0.1.pdf
 */
class EvseccUsecase final : public EebusUsecase
{
public:
    EvseccUsecase();

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::EVSECC;
    }

    /**
     * @brief Handles a message for the EVSECC use case.
     * @param header SPINE header with command classifier and targeted entity info
     * @param data The actual function call and data of the message
     * @param response JsonObject to write the response to
     * @return MessageReturn with handling status and response info
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * @brief Update the operating state of the EVSE.
     *
     * As required by Scenario 2 of the Usecase. This will inform all subscribers
     * of the new operating state.
     *
     * @param failure If the EVSE is in failure state or not. If true, the operating
     *        state will be set to "failure". If false, the operating state will be
     *        set to "normalOperation".
     * @param error_message The message describing the failure. Only used if failure is true.
     */
    void update_operating_state(bool failure = false, const String &error_message = "");

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::DeviceDiagnosis, FeatureTypeEnumType::DeviceClassification};
    }

    // Data generators for SPINE data types
    void get_device_diagnosis_state(DeviceDiagnosisStateDataType *data) const;
    static void get_device_classification_manufacturer(DeviceClassificationManufacturerDataType *data);

private:
    // Server Data
    DeviceDiagnosisOperatingStateEnumType operating_state = DeviceDiagnosisOperatingStateEnumType::normalOperation;
    std::string last_error_message;

    void update_api() const;
};

#endif // EEBUS_ENABLE_EVSECC_USECASE
