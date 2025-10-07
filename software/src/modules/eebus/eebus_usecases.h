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


/*
This file contains the definitions of the EEBUS Usecases as defined in the EEBUS Usecase Technical Specifications.
The usecase names may have been shortened and the spec is referred to as much as possible:
EVCS -> Electric Vehicle Charging Summary. Implemented according to EEBUS_UC_TS_CoordinatedEVCharging_V1.0.1.pdf
LPC -> Limitation of Power Consumption. Implemented according to EEBUS_UC_TS_LimitationOfPowerConsumption_V1.0.0.pdf
EVCC -> EV Commissioning and Configuration. Implemented according to EEBus_UC_TS_EVCommissioningAndConfiguration_V1.0.1.pdf
NMC -> Node Management and Control. Implemented according to EEBUS_SPINE_TS_ProtocolSpecification.pdf, technically nodemanagement is not a usecase but it behaves like one in many ways and is therefore implemented alongside

Sometimes the following references are used e.g. LPC-905, these refer to rules laid out in the spec and can be found in the according technical spec.
*/

#pragma once

#include "build.h"

#include "config.h"
#include "spine_connection.h"
#include "spine_types.h"
#include "lpc_state.enum.h"

// Update this as usecases are enabled. 1 is always active and the nodemanagement Usecase
#define EEBUS_USECASES_ACTIVE 4

// Configuration related to the LPC usecases
// Comment out if subscription functionalities shall be disabled
// TODO: Subscriptions are currently broken, so do not reenable or it will crash at startup
#define EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS false
// The power consumption limit at startup in w. Should be the maximum limit of the Warp Charger. Is also used to tell the Energy Broker the maximum consumption limit of the device
#define EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION 22000


class EEBusUseCases; // Forward declaration of EEBusUseCases

enum class UseCaseType : uint8_t
{
    NodeManagement,
    ChargingSummary,
    LimitationOfActivePowerConsumption,
    CoordinatedEvCharging,
    EvCommissioningAndConfiguration,
    LimitationOfPowerProduction
};

/**
 * The basic Framework of a EEBUS Entity.
 * Each entity has one or multiple features. An entity
 */
class EebusEntity
{
public:
    // Usecase Handlers
    virtual ~EebusEntity() = default;

    [[nodiscard]] virtual UseCaseType get_usecase_type() const = 0;

    void set_entity_address(const std::vector<int> &address)
    {
        entity_address = address;
    }

    [[nodiscard]] bool matches_entity_address(const std::vector<int> &address) const
    {
        return entity_address == address;
    }

    virtual String get_entity_name() const = 0;

    [[nodiscard]] bool isActive() const
    {
        return entity_active;
    }

    /**
     * \brief Handles a message for a usecase.
     * @param header SPINE header of the message. Contains information about the commandclassifier and the targeted entitiy.
     * @param data The actual Function call
     * @param response Where to write the response to. This is a JsonObject that should be filled with the response data.
     * @param connection The SPINE Connection that sent the message. This is used to send the response back to the correct connection and to identify the connection which bound or subscribed to a function.
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    virtual CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection) = 0;

    /**
     * Returns the usecase information for this entity. This gives information about which usecases this entity belongs to. Multiple entities may belong to the same usecase.
     * @return The UseCaseInformationDataType that contains the information about the usecase.
     */
    virtual UseCaseInformationDataType get_usecase_information() = 0;

    [[nodiscard]] virtual std::vector<NodeManagementDetailedDiscoveryEntityInformationType> get_detailed_discovery_entity_information() const = 0; // An entity exists only once but can have multiple features.
    [[nodiscard]] virtual std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const = 0;

protected:
    std::vector<int> entity_address{}; // The feature address of the usecase. This is used to identify the usecase in the NodeManagementUseCaseDataType.

    bool entity_active = true; // If the entity is active or not. Inactive entities do not respond to messages or their entity and feature information should not be called.
};

/**
 * The Default Nodemanagement UseCase. Needs to be supported by all EEBUS devices.
 */
class NodeManagementEntity final : public EebusEntity
{
public:
    NodeManagementEntity();
    void set_usecaseManager(EEBusUseCases *usecases);

    /**
    * Handles a binding request for a usecase.
    * @param header SPINE header of the message. Might be required for releasing of bindings.
    * @param data The SpineDataTypeHandler that contains the command.
    * @param response The JsonObject to write the response to. This should be filled with the response data.
    * @return true if a response was generated and needs to be sent, false if no response is needed.
    */
    CmdClassifierType handle_binding(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);
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
    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection) override;

    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryEntityInformationType> get_detailed_discovery_entity_information() const override;
    /**
     * The NodeManagement Usecase only has one features which is the NodeManagement feature itself.
     * @return The lis of features supported by this entity.
     */
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::NodeManagement;
    }

    /**
     * Inform all subscribers of the given entity. Sends out a Notify SPINE message with the given content
     * @param entity The entity emitting the notification
     * @param feature The feature emitting the notification
     * @param data The Data to be sent. The last_cmd value needs to be set to the appropriate function and the corresponding data field needs to be set.
     * @param function_name Name of the function that is informing the subscribers.
     * @return The number of subscribers that have been informed. 0 if no subscribers have been found.
     */
    template <typename T>
    size_t inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, T data, String function_name);

    String get_entity_name() const override
    {
        return "NodeManagement";
    };

private:
    EEBusUseCases *usecase_interface{};

    NodeManagementSubscriptionDataType subscription_data{};
    bool read_usecase_data(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) const;

    bool read_detailed_discovery_data(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) const;

    CmdClassifierType handle_subscription(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection);


};

/**
 * The EEBUSChargingSummary Entity as defined in EEBus UC TS - EV Charging Summary V1.0.1.
 * TODO: This cannot be fully used until EVCS usecase is implemented. And need to figure out how this usecases works as documentation seems contratictory in parts.
 */
class EvseEntity final : public EebusEntity
{
public:
    // TODO: Instead of pulling data from API, API pushes data to this usecases maybe? So we can trigger updates to subscribers?
    EvseEntity();

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
    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection) override;


    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::ChargingSummary;
    }

    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryEntityInformationType> get_detailed_discovery_entity_information() const override;
    /**
     * Returns the supported features. EVSE supports only one feature which is the Bill feature.
     * @return a list of the supported features.
     */
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * Update the billing data. This will update the bill entry with the given data and inform all subscribers of the new bill entry.
     * @param id The ID of the bill entry to update. If it matches an existing entry, the entry will be updated. If it does not match an existing entry, a new entry will be created.
     * @param start_time Start time of billing period
     * @param end_time End time of billing period
     * @param energy_wh The amount of energy charged in Wh
     * @param cost_eur_cent The cost of the charged energy in Euro cents
     * @param grid_energy_percent The percentage of energy that was drawn from the grid
     * @param grid_cost_percent The percentage of cost that was due to grid energy
     * @param self_produced_energy_percent The amount of self produced energy used for charging in Wh
     * @param self_produced_cost_percent The cost of the self produced energy used for charging in Euro cents
     */
    void update_billing_data(int id, seconds_t start_time, seconds_t end_time, int energy_wh, uint32_t cost_eur_cent, int grid_energy_percent = 100, int grid_cost_percent = 100, int self_produced_energy_percent = 0, int self_produced_cost_percent = 0);

    String get_entity_name() const override
    {
        return "EvseEntity";
    };

private:
    int bill_feature_address = 1;
    BillListDataType bill_list_data{};
    /**
     * Handle the Bill Feature.
     * @param header
     * @param data
     * @param response
     * @param connection
     * @return
     */
    CmdClassifierType bill_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection);

    void update_api() const;
};

class EvEntity final : public EebusEntity
{
public:
    EvEntity();

    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::EvCommissioningAndConfiguration;
    }

    /**
    * Builds and returns the UseCaseInformationDataType as defined in EEBus UC TS - EV Commissioning and Configuration V1.0.0. 3.1.2.
    * @return
    */
    UseCaseInformationDataType get_usecase_information() override;

    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection) override;
    /**
     * The entity information as defined in EEBus UC TS - EV Commissioning and Configuration V1.0.1. 3.2.1.
     * @return The entity information.
     */
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryEntityInformationType> get_detailed_discovery_entity_information() const override;
    /**
     * Returns the supported features. As defined in EEBus UC TS - EV Commissioning and Configuration V1.0.1. 3.2.1.2.1.
     * @return a list of the supported features.
     */
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * Inform the usecases a EV has been connected or disconnected. This will inform the subscribers of the NodeManagement entity about the change.
     * @param connected If a EV is connected or not.
     */
    void ev_connected_state(bool connected);
    /**
     * Update the device configuration. As required by Scenario 2 and 3 of the EVCC Usecase. This will inform all subscribers of the new configuration.
     * @param communication_standard The communication standard. Should only be "iso15118-2ed1","iso15118-2ed1" or "iec61851". Will log a tracelog if another value is given.
     * @param asymmetric_supported If asymmetric charging is supported or not.
     */
    void update_device_config(const String &communication_standard, bool asymmetric_supported = false);

    /**
     * Update the identification of the EV. As required by scenarion 4 of the Usecase. This will inform all subscribers of the new identification.
     * @param mac_address Mac address of the ev. Should be in the format given as mac_type.
     * @param mac_type Either eui48 or eui64. Default is eui64.
     */
    void update_identification(String mac_address, IdentificationTypeEnumType mac_type = IdentificationTypeEnumType::eui64);

    /**
     * Updates the manufacturer data of the EV. As required by scenario 5 of the Usecase. This will inform all subscribers of the new manufacturer data. Entries with a length of <1 will not be added.
     * @param name Device name. Recommended to be set.
     * @param code Device code. Recommended to be set.
     * @param serial Serial Number of the device. Optional.
     * @param software_vers Software revision of the device. Optional.
     * @param hardware_vers Hardware revision of the device. Optional.
     * @param vendor_name Vendor Name. Recommended to be set.
     * @param vendor_code Vendor code. Recommended to be set.
     * @param brand_name Brand name. Recommended to be set.
     * @param manufacturer Manufacturer Label. Recommended to be set.
     * @param manufacturer_description Manufacturer Description. Optional.
     */
    void update_manufacturer(String name = "", String code = "", String serial = "", String software_vers = "", String hardware_vers = "", String vendor_name = "", String vendor_code = "", String brand_name = "", String manufacturer = "", String manufacturer_description = "");

    /**
     * The Electrical Connection feature as required by Scenario 6 of the Usecase. This will inform all subscribers of the new electrical connection data.
     * @param min_power In Watts. Minimum power at which the EV can still charge. Default is 0W.
     * @param max_power In Watts. Maximum power at which the EV can charge. Default is EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION.
     * @param standby_power In Watts. If the EV has a standby mode and how much power it is capable of consuming in it.
     */
    void update_electrical_connection(int min_power = 0, int max_power = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION, int standby_power = 0);

    /**
     * Update the DeviceDiagnosis state. As required by Scenario 7 of the Usecase. This will inform all subscribers of the new device diagnosis state.
     * @param standby If the device is in standby mode or not.
     */
    void update_operating_state(bool standby);

    String get_entity_name() const override
    {
        return "EvEntity";
    };

private:
    void update_api() const;
    bool ev_connected = false;

    // These can be freely assigned but need to be unique within the entity.
    int feature_address_device_configuration = 101;
    int feature_address_identification = 102;
    int feature_address_device_classification = 103;
    int feature_address_electrical_connection = 104;
    int feature_address_device_diagnosis = 105;

    // Server Data
    //DeviceDiagnosis
    String communication_standard = "";
    bool asymmetric_supported = false;
    DeviceConfigurationKeyValueDescriptionListDataType generate_device_config_description() const;
    DeviceConfigurationKeyValueListDataType generate_device_config_list() const;
    //Identification
    IdentificationTypeEnumType mac_type = IdentificationTypeEnumType::eui64;
    String mac_address = "";
    IdentificationListDataType generate_identification_description() const;
    //DeviceClassification
    String manufacturer_name = "";
    String manufacturer_code = "";
    String ev_serial_number = "";
    String ev_sofware_version = "";
    String ev_hardware_version = "";
    String vendor_name = "";
    String vendor_code = "";
    String brand_name = "";
    String manufacturer_label = "";
    String manufacturer_description = "";
    DeviceClassificationManufacturerDataType generate_manufacturer_description() const;
    //ElectricalConnection
    uint32_t min_power_draw = 0;
    uint32_t max_power_draw = 0;
    uint32_t standby_power = 0;
    ElectricalConnectionParameterDescriptionListDataType generate_electrical_connection_description() const;
    ElectricalConnectionPermittedValueSetListDataType generate_electrical_connection_values() const;
    //DeviceDiagnosis
    bool standby_mode = false;
    DeviceDiagnosisStateDataType generate_state() const;

};

/**
 * The Controllable System Entity as defined in EEBus UC TS - EV Limitation Of Power Consumption V1.0.0.
 */
class ControllableSystemEntity final : public EebusEntity
{
public:
    ControllableSystemEntity();

    /**
    * Builds and returns the UseCaseInformationDataType as defined in EEBus UC TS - EV Limitation Of Power Consumption V1.0.0. 3.1.2.
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
    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection) override;


    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::LimitationOfActivePowerConsumption;
    }

    /**
     * The entity information as defined in EEBus UC TS - EV Limitation Of Power Consumption V1.0.0. 3.2.2.
     * @return The entity information.
     */
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryEntityInformationType> get_detailed_discovery_entity_information() const override;
    /**
     * Returns the supported features. As defined in EEBus UC TS - EV Limitation Of Power Consumption V1.0.0. 3.2.2.2.1.
     * @return a list of the supported features.
     */
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;


    /**
     * Update the limit the system is supposed to be consuming.
     * @param limit_active If the limit is active or not.
     * @param current_limit_w The limit in W.
     * @param endtime Timestamp until which the limit shall be set
     * @return true if the limit is accepted and set.
     */
    bool update_lpc(bool limit_active, int current_limit_w, uint64_t endtime);

    /**
     * Update the maximum power the system is currently capable of consuming. This will inform all subscribers of the new power limit. Implemented according to LPC UC TS v1.0.0 3.2.2.2.3.1
     * @param power_limit_w Power limit in watts.
     * @param duration Duration in seconds.
     */
    void update_failsafe(int power_limit_w, seconds_t duration);

    /**
     * Update the constraints of the system. This will inform all subscribers of the new constraints. Implemented according to LPC UC TS v1.0.0 3.2.2.2.5.1
     * @param power_consumption_max_w
     * @param power_consumption_contract_max_w
     */
    void update_constraints(int power_consumption_max_w, int power_consumption_contract_max_w);

    String get_entity_name() const override
    {
        return "ControllableSystemEntity";
    };

private:
    /**
     * The Load Control feature as required for Scenario 1 - Control active power consumption.
     * It is the primary feature of the entity and major part of the LPC Usecases.
     * Takes in a limit, if its enabled and a duration and attempts to apply it.
     * Requires a binding to write.
     * Supports subscriptions.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.1 and 3.4.1
     */
    CmdClassifierType load_control_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection);

    /**
     * The Device Configuration feature as required for Scenario 2 - Failsafe Values.
     * Reports the current failsafe values and allows them to be updated.
     * A failsafe value is the maximum power that can be consumed if no contact to the energy guard can be established and always comes with a duration for which the failsafe will be active.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.2 and 3.4.2
     */
    CmdClassifierType deviceConfiguration_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection);

    /**
     * The Device Diagnosis feature as required for Scenario 3 - Heartbeat.
     * Implements a heartbeat mechanism to ensure that the energy guard is still online and reachable, if not it enters the failsafe state.
     * If no heartbeat is received for a certain time, the system will switch to failsafe mode.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.3 and 3.4.3
     */
    CmdClassifierType device_diagnosis_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection);

    /**
     * The Electrical Connection feature as required for Scenario 4 - Constraints.
     * Reports the current constraints of the system and is read only.
     * Constraints are the maximum power the system is capable of consuming.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.4 and 3.4.4
     */
    CmdClassifierType electricalConnection_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response, SpineConnection *connection);

    // State handling
    // State machine as described in LPC UC TS v1.0.0 2.3
    LPCState lpc_state;
    uint64_t state_change_timeout = 0;
    bool switch_state(LPCState state);
    bool init_state();
    bool unlimited_controlled_state();
    bool limited_state();
    bool failsafe_state();
    bool unlimited_autonomous_state();

    void update_api();

    // These can be freely assigned but need to be unique within the entity.
    int loadControl_feature_address = 10;
    int deviceConfiguration_feature_address = 20;
    int deviceDiagnosis_feature_address = 30;
    int electricalConnection_feature_address = 40;

    // LoadControl configuration as required for scenario 1 - Control Active Power
    int current_active_consumption_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    bool limit_engaged = false;

    // Device Configuration Data as required for Scenario 2 - Device Configuration
    DeviceConfigurationKeyValueListDataType device_configuration_key_value_list{};
    DeviceConfigurationKeyValueDescriptionListDataType device_configuration_key_value_description_list{};


    // Heartbeat Data as required for Scenario 3 - Hearbeat
    bool heartbeatEnabled = false;
    uint64_t heartbeatCounter = 0;
    uint64_t heartbeat_timeout_task = 0;
    void handle_heartbeat_timeout();
    bool heartbeat_received = false;

    // Electrical Connection Data as required for Scenario 4 - Constraints
    ElectricalConnectionCharacteristicListDataType electrical_connection_characteristic_list{};
    LoadControlLimitDescriptionListDataType load_control_limit_description_list{};
    LoadControlLimitListDataType load_control_limit_list{};
};


/**
 * The central Interface for EEBus UseCases.
 */
class EEBusUseCases
{
public:
    EEBusUseCases();

    /**
     * Main interface for the EEBUS UseCases. All EEBUS Messages are passed here and forwarded to the correct usecase entity.
     * @param header Spine Header
     * @param data Payload of the message.
     * @param connection The SPINE Connection that sent the message. This is used to send the response back to the correct connection and to identify the connection which bound or subscribed to a function.
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    void handle_message(HeaderType &header, SpineDataTypeHandler *data, SpineConnection *connection);

    /**
     * Informs the subscribers of a feature about a change in the data.
     * @param entity the entity address of the feature
     * @param feature the feature address
     * @param data The Data the subscribers should be informed about. This is a SpineDataTypeHandler that contains the data.
     * @return The number of subscribers that have been informed. 0 if no subscribers were informed.
     */
    template <typename T>
    size_t inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, T data, String function_name);

    /**
     * Send a message to a spine destination.
     * @param destination Spine FeatureAddressType of the destination. A message from the destination has to have been received before.
     * @param sender Sender FeatureAddressType of the sender. The entity and feature must be set. The device can be empty and will be filled automatically if so.
     * @param payload The payload to be sent.
     * @param cmd_classifier The command classifier of the message.
     * @param want_ack If we want an acknowledgement for the message. This is used to ensure that the peer received the message and can be used to detect if the peer is still alive.
     */
    bool send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, JsonVariantConst payload, CmdClassifierType cmd_classifier, bool want_ack = false);

    BasicJsonDocument<ArduinoJsonPsramAllocator> temporary_json_doc{SPINE_CONNECTION_MAX_JSON_SIZE}; // If a temporary doc is needed, use this one.
    BasicJsonDocument<ArduinoJsonPsramAllocator> response{SPINE_CONNECTION_MAX_JSON_SIZE}; // The response document to be filled with the response data

    NodeManagementEntity node_management{};
    EvseEntity charging_summary{};
    ControllableSystemEntity limitation_of_power_consumption{};
    EvEntity ev_commissioning_and_configuration{};

    EebusEntity *entity_list[EEBUS_USECASES_ACTIVE] = {&node_management, &charging_summary, &limitation_of_power_consumption, &ev_commissioning_and_configuration};

private:
    bool initialized = false;
    uint16_t eebus_commands_received = 0;
    uint16_t eebus_responses_sent = 0;
};

namespace EEBUS_USECASE_HELPERS
{
const char *get_spine_device_name();

/**
* The Values specified in the EEBUS SPINE TS ResourceSpecification 3.11 Table 19
*/
enum class ResultErrorNumber
{
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

/**
 * Converts a duration in seconds to an ISO 8601 duration string. Will be automatically converted to full minutes or hours if possible.
 * @param duration Duration in seconds.
 * @return The ISO 8601 duration string.
 */
std::string iso_duration_to_string(seconds_t duration);
/**
 * Convert a ISO 8601 duration string to a duration in seconds.
 * @param iso_duration The ISO 8601 duration string
 * @return The duration in seconds.
 */
seconds_t iso_duration_to_seconds(std::string iso_duration);
} // namespace EEBUS_USECASE_HELPERS
