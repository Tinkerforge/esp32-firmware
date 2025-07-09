
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

#include "build.h"

#include "config.h"
#include "spine_connection.h"
#include "spine_types.h"

// Update this as new usecases are added
#define EEBUS_USECASES_ACTIVE 2

class EEBusUseCases; // Forward declaration of EEBusUseCases

enum class UseCaseType : uint8_t {
    NodeManagement,
    ChargingSummary,
    LimitationOfActivePowerConsumption,
    CoordinatedEvCharging,
    EvCommissioningAndConfiguration,
    LimitationOfPowerProduction
};

/**
 * The basic Framework of a EEBUS UseCase.
 */
class UseCase
{
public:
    // Usecase Handlers
    virtual ~UseCase() = default;

    [[nodiscard]] virtual UseCaseType get_usecase_type() const = 0;
    void set_entity_address(const uint8_t address)
    {
        entity_address = address;
    }

    /**
     * \brief Handles a message for a usecase.
     * @param header SPINE header of the message. Contains information about the commandclassifier and the targeted entitiy.
     * @param data The actual Function call
     * @param response Where to write the response to. This is a JsonObject that should be filled with the response data.
     * @param connection The SPINE Connection that sent the message. This is used to send the response back to the correct connection and to identify the connection which bound or subscribed to a function.
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    virtual bool handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection) = 0;

    /**
     * Returns the usecase information for this usecase.
     * @return The UseCaseInformationDataType that contains the information about the usecase.
     */
    virtual UseCaseInformationDataType get_usecase_information() = 0;

    [[nodiscard]] virtual NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const;
    [[nodiscard]] virtual NodeManagementDetailedDiscoveryFeatureInformationType get_detailed_discovery_feature_information() const;

protected:
    uint8_t entity_address =
        0; // The feature address of the usecase. This is used to identify the usecase in the NodeManagementUseCaseDataType.
};

/**
 * The Default Nodemanagement UseCase. Needs to be supported by all EEBUS devices.
 */
class NodeManagementUsecase final : public UseCase
{
public:
    void set_usecaseManager(EEBusUseCases *usecases);

    /**
    * Handles a binding request for a usecase.
    * @param header SPINE header of the message. Might be required for releasing of bindings.
    * @param data The SpineDataTypeHandler that contains the command.
    * @param response The JsonObject to write the response to. This should be filled with the response data.
    * @return true if a response was generated and needs to be sent, false if no response is needed.
    */
    bool handle_binding(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);
    // The list of bindings for this usecase.
    // The SPINE Protocol specification implies in 7.3.6 that this should be stored persistently but it also allows binding information to be discarded in case the device was offline.
    BindingManagementEntryListDataType binding_management_entry_list_{};
    /**
     * Checks if the given client is bound to the given server entity and feature.
     * @param client The client FeatureAddressType to check.
     * @param server The server FeatureAddressType to check.
     * @return true if the client is bound to the server, false otherwise.
     */
    bool check_is_bound(FeatureAddressType &client, FeatureAddressType &server) const;

    UseCaseInformationDataType get_usecase_information() override;
    /**
    * \brief Handles a message for a usecase.
    * @param header SPINE header of the message. Contains information about the commandclassifier and the targeted entitiy.
    * @param data The actual Function call
    * @param response Where to write the response to. This is a JsonObject that should be filled with the response data.
    * @param connection The SPINE Connection that sent the message. This is used to send the response back to the correct connection and to identify the connection which bound or subscribed to a function.
    * @return true if a response was generated and needs to be sent, false if no response is needed.
    */
    bool handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection) override;

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] NodeManagementDetailedDiscoveryFeatureInformationType get_detailed_discovery_feature_information() const override;

    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::NodeManagement;
    }

    void inform_subscribers(int entity, int feature, SpineDataTypeHandler *data);

private:
    EEBusUseCases *usecase_interface{};

    NodeManagementSubscriptionDataType subscription_data{};
    bool read_usecase_data(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) const;

    bool read_detailed_discovery_data(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) const;

    bool handle_subscription(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection);
};

/**
 * The EEBUSChargingSummary UseCase as defined in EEBus UC TS - EV Charging Summary V1.0.1.
 */
class ChargingSummaryUsecase final : public UseCase
{
public:
    // Usecase Management
    ChargingSummaryUsecase() = default;

    /**
    * Builds and returns the UseCaseInformationDataType as defined in EEBus UC TS - EV Charging Summary V1.0.1. 3.1.2.
    * @return
    */
    UseCaseInformationDataType get_usecase_information() override;

    /**
     * \brief Handles a message for a usecase.
     * @param header SPINE header of the message. Contains information about the commandclassifier and the targeted entitiy.
     * @param data The actual Function call and data of the message.
     * @param response Where to write the response to. This is a JsonObject that should be filled with the response data.
     * @param connection The SPINE Connection that sent the message. This is used to send the response back to the correct connection and to identify the connection which bound or subscribed to a function.
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    bool handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection) override;

    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::ChargingSummary;
    }
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] NodeManagementDetailedDiscoveryFeatureInformationType get_detailed_discovery_feature_information() const override;

    // Binding
    std::vector<uint32_t> bound_connections{}; // List of connections that have been bound successfully
};

/**
 * The central Interface for EEBus UseCases.
 */
class EEBusUseCases
{
public:
    EEBusUseCases();

    /**
     * Main interface for the EEBUS UseCases.
     * @param header Spine Header
     * @param data Payload of the message.
     * @param response The response object to fill with the response data.
     * @param connection The SPINE Connection that sent the message. This is used to send the response back to the correct connection and to identify the connection which bound or subscribed to a function.
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    void handle_message(HeaderType &header, SpineDataTypeHandler *data, SpineConnection *connection);

    /**
     * Informs the subscribers of a feature about a change in the data.
     * @param entity the entity address of the feature
     * @param feature the feature address
     * @param data The Data the subscribers should be informed about. This is a SpineDataTypeHandler that contains the data.
     */
    void inform_subscribers(int entity, int feature, SpineDataTypeHandler *data);

    DynamicJsonDocument response{8192}; // The response document to be filled with the response data


    uint8_t feature_address_node_management = 0;
    NodeManagementUsecase node_management{};

    uint8_t feature_address_charging_summary = 1;
    ChargingSummaryUsecase charging_summary{};

    UseCase *usecase_list[EEBUS_USECASES_ACTIVE] = {&node_management, &charging_summary};
};

namespace EEBUS_USECASE_HELPERS
{
const char *get_spine_device_name();

/**
 * The Values specified in the EEBUS SPINE TS ResourceSpecification 3.11 Table 19
 */
enum class ResultErrorNumber {
    NoError = 0,
    GeneralError,
    Timeout,
    Overload,
    DestinationUnknown,
    DestinationUnreachable,
    CommandNotSupported,
    CommandRejected,
    RestrictedFunctionExchangeCombinationNotSupported,
    BindingRequired
};

/**
 * Generate a result data object and writes it to the response object. Generally required when the header demands an acknowledgement or a result.
 * @param response The JsonObject to write the result data to.
 * @param error_number The error number to set in the result data. Default is NoError.
 * @param description The description of the error set in the result data. Default is an empty string.
 */
void build_result_data(JsonObject &response, ResultErrorNumber error_number = ResultErrorNumber::NoError, const char *description = "");

} // namespace EEBUS_USECASE_HELPERS
