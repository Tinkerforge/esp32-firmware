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

// Enable subscription mechanism for nodemanagement
#define EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS true

/**
 * @brief The default NodeManagement UseCase.
 *
 * Required by all EEBUS devices. Handles device discovery, subscription management,
 * and binding management as defined in SPINE specification sections 7.1-7.3.
 *
 * Entity address: {0}
 * Feature: NodeManagement (special role)
 *
 * @see SPINE TS ProtocolSpecification sections 7.1 (Discovery), 7.2 (Subscription), 7.3 (Binding)
 */
class NodeManagementEntity final : public EebusUsecase
{
public:
    NodeManagementEntity();

    /**
     * @brief Set the parent usecase manager reference.
     * @param usecases Pointer to the EEBusUseCases manager
     */
    void set_usecaseManager(EEBusUseCases *usecases);

    /**
     * @brief Checks if the given client is bound to the given server entity and feature.
     * @param sending_feature The client FeatureAddressType to check
     * @param target_feature The server FeatureAddressType to check
     * @return true if the client is bound to the server, false otherwise
     */
    bool check_is_bound(FeatureAddressType &sending_feature, FeatureAddressType &target_feature) const;

    UseCaseInformationDataType get_usecase_information() override;

    /**
     * @brief Handles a message for the NodeManagement usecase.
     * @param header SPINE header containing commandclassifier and targeted entity info
     * @param data The actual function call data
     * @param response JsonObject to write the response to
     * @return MessageReturn with handling status and response info
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;

    /**
     * @brief Get feature discovery information.
     *
     * The NodeManagement usecase only has one feature which is the NodeManagement feature itself.
     *
     * @return List of features supported by this entity
     */
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::NMC;
    }

    /**
     * @brief Inform all subscribers of the given entity.
     *
     * Sends out a Notify SPINE message with the given content to all subscribers.
     *
     * @tparam T Data type to send
     * @param entity The entity emitting the notification
     * @param feature The feature emitting the notification
     * @param data The data to be sent
     * @param function_name Name of the function informing the subscribers
     * @return Number of subscribers informed (0 if none found)
     */
    template <typename T> size_t inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, T data, const char *function_name);

    /**
     * @brief Subscribe to a feature.
     *
     * Make sure the sending feature supports notify messages.
     *
     * @param sending_feature The feature that is subscribing to the target feature
     * @param target_feature The target feature to subscribe to
     * @param feature The feature type of the target feature (optional)
     * @return true if subscription request was valid and destination found (not whether target accepted)
     */
    bool subscribe_to_feature(FeatureAddressType &sending_feature, FeatureAddressType &target_feature, FeatureTypeEnumType feature = FeatureTypeEnumType::EnumUndefined) const;

    /**
     * @brief Informs that detailed discovery data has changed.
     *
     * All subscribers will be notified of the update.
     */
    void detailed_discovery_update();

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::NodeManagement};
    }

private:
    AddressFeatureType nodemgmt_feature_address = 0;

    EEBusUseCases *usecase_interface{};

    NodeManagementSubscriptionDataType subscription_data{};

    NodeManagementUseCaseDataType get_usecase_data() const;
    NodeManagementDetailedDiscoveryDataType get_detailed_discovery_data() const;

    MessageReturn handle_subscription(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);

    // The list of bindings for this usecase.
    // The SPINE Protocol specification implies in 7.3.6 that this should be stored persistently
    // but it also allows binding information to be discarded in case the device was offline.
    BindingManagementEntryListDataType binding_management_entry_list_{};

    /**
     * @brief Handles a binding request for a usecase.
     * @param header SPINE header (may be required for releasing bindings)
     * @param data The SpineDataTypeHandler containing the command
     * @param response JsonObject to write the response to
     * @return MessageReturn indicating if response needs to be sent
     */
    MessageReturn handle_binding(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);
};
