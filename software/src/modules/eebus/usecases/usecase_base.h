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

#include "../spine_connection.h"
#include "../spine_types.h"
#include "../usecases.enum.h"
#include "config.h"
#include "usecase_helpers.h"
#include <map>
#include <vector>

/**
 * @brief Return structure for usecase message handlers.
 *
 * Indicates how a usecase handled an incoming message and whether
 * a response should be sent.
 */
struct MessageReturn {
    /**
     * If true, the usecase has handled the message and no further processing is required.
     */
    bool is_handled = false;
    /**
     * If true, a response should be sent back to the sender.
     */
    bool send_response = false;
    /**
     * If a response should be sent, contains the command classifier to use.
     */
    CmdClassifierType cmd_classifier = CmdClassifierType::EnumUndefined;
};

// Forward declaration
class EEBusUseCases;

/**
 * @brief Configuration struct for LoadPowerLimitUsecase parameterization.
 *
 * Used to differentiate between LPC (power consumption) and LPP (power production)
 * usecases. Both share the same base implementation but with different parameters.
 */
struct LoadPowerLimitConfig {
    Usecases usecase_type;                                              ///< LPC or LPP
    const char *usecase_name;                                           ///< "limitationOfPowerConsumption" or "limitationOfPowerProduction"
    const char *api_key;                                                ///< "power_consumption_limitation" or "power_production_limitation"
    EnergyDirectionEnumType energy_direction;                           ///< consume or produce
    ElectricalConnectionCharacteristicTypeEnumType characteristic_type; ///< powerConsumptionMax or powerProductionMax
    DeviceConfigurationKeyNameEnumType failsafe_key_name;               ///< failsafeConsumptionActivePowerLimit or failsafeProductionActivePowerLimit
    bool limit_is_positive;                                             ///< true for LPC (power > 0), false for LPP (power < 0)
    // ID offsets from EVSEEntity
    uint8_t loadcontrol_limit_id_offset;
    uint8_t measurement_id_offset;
    uint8_t device_config_key_id_offset;
    uint8_t electrical_connection_id_offset;
    uint8_t electrical_connection_characteristic_id_offset;
    uint8_t electrical_connection_parameter_id_offset;
};

/**
 * @brief Base class for all EEBUS use cases.
 *
 * Provides the common framework for implementing EEBUS use cases as defined
 * in the EEBUS Use Case Technical Specifications. Each use case has one or
 * more features and belongs to an entity.
 *
 * This class should only be inherited from, never instantiated directly.
 */
class EebusUsecase
{
public:
    virtual ~EebusUsecase() = default;

    /**
     * @brief Get the type identifier for this use case.
     * @return The Usecases enum value for this use case.
     */
    [[nodiscard]] virtual Usecases get_usecase_type() const = 0;

    /**
     * @brief Set the entity address for this use case.
     * @param address Vector of integers representing the SPINE entity address.
     */
    void set_entity_address(const std::vector<int> &address)
    {
        entity_address = address;
    }

    /**
     * @brief Get the entity address for this use case.
     * @return Vector of integers representing the SPINE entity address.
     */
    [[nodiscard]] std::vector<int> get_entity_address() const
    {
        return entity_address;
    }

    /**
     * @brief Check if the given address matches this use case's entity address.
     * @param address Address to compare against.
     * @return True if the addresses match.
     */
    [[nodiscard]] bool matches_entity_address(const std::vector<int> &address) const
    {
        return entity_address == address;
    }

    /**
     * @brief Check if this use case entity is active.
     * @return True if the entity is active.
     */
    [[nodiscard]] bool isActive() const
    {
        return entity_active;
    }

    /**
     * @brief Send a full read request for a specific function.
     *
     * Sends a SPINE read message to the given receiver requesting the
     * complete data for the specified function.
     *
     * @param sending_feature The feature address of the sender (this device).
     * @param receiver The target feature address to send the read to.
     * @param function The function to read.
     */
    void send_full_read(AddressFeatureType sending_feature, FeatureAddressType receiver, SpineDataTypeHandler::Function function) const;

    /**
     * @brief Handle an incoming SPINE message.
     *
     * Each use case must implement this to handle messages targeted at its features.
     *
     * @param header SPINE header containing command classifier and target info.
     * @param data The function call data.
     * @param response JsonObject to write the response data to.
     * @return MessageReturn indicating if/how the message was handled.
     */
    virtual MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) = 0;

    /**
     * @brief Get the use case information for discovery.
     *
     * Returns information about which use cases this entity supports,
     * used during SPINE node management discovery.
     *
     * @return UseCaseInformationDataType containing use case details.
     */
    virtual UseCaseInformationDataType get_usecase_information() = 0;

    /**
     * @brief Get the list of feature types supported by this use case.
     * @return Vector of FeatureTypeEnumType values.
     */
    [[nodiscard]] virtual std::vector<FeatureTypeEnumType> get_supported_features() const = 0;

    /**
     * @brief Set a feature address for a given feature type.
     *
     * Associates a feature address with a feature type for this use case.
     * Both the address and type must be unique within the use case.
     *
     * @param feature_address The feature address (must be unique).
     * @param feature_type The feature type (must be unique).
     */
    void set_feature_address(AddressFeatureType feature_address, FeatureTypeEnumType feature_type);

    /**
     * @brief Get detailed discovery entity information for this use case.
     * @return Entity information for SPINE discovery.
     */
    [[nodiscard]] virtual NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const = 0;

    /**
     * @brief Get detailed discovery feature information for all features.
     * @return Vector of feature information for SPINE discovery.
     */
    [[nodiscard]] virtual std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const = 0;

    /**
     * @brief Called when a heartbeat is received.
     *
     * Override this in use cases that utilize heartbeats.
     */
    virtual void receive_heartbeat()
    {
    }

    /**
     * @brief Called when a heartbeat timeout occurs.
     *
     * Override this in use cases that utilize heartbeats.
     */
    virtual void receive_heartbeat_timeout()
    {
    }

    /**
     * @brief Called when supported use cases or entities change.
     *
     * Override this to respond to changes in the remote device's capabilities.
     *
     * @param conn The SpineConnection that was updated.
     */
    virtual void inform_spineconnection_usecase_update(SpineConnection *conn)
    {
    }

protected:
    std::vector<int> entity_address{}; ///< The entity address for this use case.
    bool entity_active = true;         ///< Whether the entity is active.

    /// Map of feature types to their addresses.
    std::map<FeatureTypeEnumType, AddressFeatureType> feature_addresses{};

    /**
     * @brief Get the feature type for a given feature address.
     * @param feature_address The feature address to look up.
     * @return The feature type, or EnumUndefined if not found.
     */
    [[nodiscard]] FeatureTypeEnumType get_feature_by_address(AddressFeatureType feature_address) const;

    /**
     * @brief Build a FeatureAddressType for the given feature.
     * @param feature The feature address.
     * @return Complete FeatureAddressType with device and entity info.
     */
    [[nodiscard]] FeatureAddressType get_feature_address(AddressFeatureType feature) const
    {
        FeatureAddressType address{};
        address.feature = feature;
        address.entity = entity_address;
        address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
        return address;
    }
};
