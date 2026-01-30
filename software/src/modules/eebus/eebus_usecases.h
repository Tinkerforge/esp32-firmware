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
NMC -> Node Management and Control. Implemented according to EEBUS_SPINE_TS_ProtocolSpecification.pdf, technically nodemanagement is not a usecase but it behaves like one in many ways and is therefore implemented alongside
EVCS -> Electric Vehicle Charging Summary. Implemented according to EEBUS_UC_TS_CoordinatedEVCharging_V1.0.1.pdf
EVCEM -> EV Charging Electricity Measurement. Implemented according to EEBUS_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf
EVCC -> EV Commissioning and Configuration. Implemented according to EEBus_UC_TS_EVCommissioningAndConfiguration_V1.0.1.pdf
EVSECC -> EVSE Commissioning and Configuration. Implemented according to EEBus_UC_TS_EVSECommissioningAndConfiguration_V1.0.0.pdf
LPC -> Limitation of Power Consumption. Implemented according to EEBUS_UC_TS_LimitationOfPowerConsumption_V1.0.0.pdf
CEVC -> Coordinate EV Charging. Implemented according to EEBUS_UC_TS_CoordinatedEVCharging_V1.0.1.pdf
OPEV -> Overload Protection by EV Charging Current curtailment. Implemented According to EEBUS_UC_TS_OverloadProtectionByEVChargingCurrentCurtailment_V1.0.1b.pdf
Sometimes the following references are used e.g. LPC-905, these refer to rules laid out in the spec and can be found in the according technical spec.
*/

#pragma once

#include "build.h"

#include "config.h"
#include "eebus_usecases.h"
#include "lpc_state.enum.h"
#include "options.h"
#include "spine_connection.h"
#include "spine_types.h"
#include "usecases.enum.h"

// Update this as usecases are enabled. 1 is always active and the nodemanagement Usecase
//#define EEBUS_ENABLE_EVCS_USECASE
#define EEBUS_ENABLE_EVCEM_USECASE
#define EEBUS_ENABLE_EVCC_USECASE
#define EEBUS_ENABLE_EVSECC_USECASE
#define EEBUS_ENABLE_LPC_USECASE
//#define EEBUS_ENABLE_CEVC_USECASE
#define EEBUS_ENABLE_MPC_USECASE
//#define EEBUS_ENABLE_LPP_USECASE
//#define EEBUS_ENABLE_OPEV_USECASE

// Configuration related to the LPC usecases
// Disable if subscription functionalities shall not be used
#define EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS true
// The power consumption limit at startup in w. Should be the maximum limit of the Warp Charger. Is also used to tell the Energy Broker the maximum consumption limit of the device
#define EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION 22000
// Enable LPC heartbeat
#define EEBUS_LPC_ENABLE_HEARTBEAT true
#define EEBUS_LPC_HEARTBEAT_INTERVAL 30_s // Must be more than 10 seconds and less or equal to 60 seconds

struct MessageReturn {
    /**
     * If this is true, the usecase has handled the message and no further processing is required.
     */
    bool is_handled = false;
    /**
     * If true a response should be sent back to the sender.
     */
    bool send_response = false;
    /**
     * If a response should be sent, this contains the command classifier to use for the response.
     */
    CmdClassifierType cmd_classifier = CmdClassifierType::EnumUndefined;
};

class EEBusUseCases; // Forward declaration of EEBusUseCases

//enum class UseCaseType : uint8_t { NodeManagement, ChargingSummary, LimitationOfActivePowerConsumption, CoordinatedEvCharging, EvCommissioningAndConfiguration, EvseCommissioningAndConfiguration, LimitationOfPowerProduction, EvChargingElectricityMeasurement, MonitoringOfPowerConsumption };

namespace EEBUS_USECASE_HELPERS
{
const char *get_spine_device_name();

/**
* The Values specified in the EEBUS SPINE TS ResourceSpecification 3.11 Table 19
*/
enum class ResultErrorNumber { NoError = 0, GeneralError, Timeout, Overload, DestinationUnknown, DestinationUnreachable, CommandNotSupported, CommandRejected, RestrictedFunctionExchangeCombinationNotSupported, BindingRequired };

String get_result_error_number_string(int error_number);

/**
* Generate a result data object and writes it to the response object. Generally required when the header demands an acknowledgement or a result.
* @param response The JsonObject to write the result data to.
* @param error_number The error number to set in the result data. Default is NoError.
* @param description The description of the error set in the result data. Default is an empty string.
*/
void build_result_data(const JsonObject &response, ResultErrorNumber error_number = ResultErrorNumber::NoError, const char *description = "");

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
seconds_t iso_duration_to_seconds(const std::string &iso_duration);

time_t iso_timestamp_to_unix(const char *iso_timestamp, time_t *t);

String unix_to_iso_timestamp(time_t unix_time);

String spine_address_to_string(const FeatureAddressType &address);

bool compare_spine_addresses(const FeatureAddressType &addr1, const FeatureAddressType &addr2);

float scaled_numbertype_to_float(const ScaledNumberType &number);
int scaled_numbertype_to_int(const ScaledNumberType &number);

} // namespace EEBUS_USECASE_HELPERS

// These classes contain the data generator functions for all usecase entities.
// As some usecases share a function under the same entity and feature, and since we only support full reads or notify messages, the full data has to be generated.
class EVSEEntity
{
public:
    inline static std::vector<int> entity_address = {1};
    // LoadControl Feature
    static constexpr uint8_t lpcLoadcontrolLimitIdOffset = 0;
    static LoadControlLimitDescriptionListDataType get_load_control_limit_description_list_data();
    static LoadControlLimitListDataType get_load_control_limit_list_data();

    // DeviceConfiguration
    static constexpr uint8_t lpcDeviceConfigurationKeyIdOffset = 10;
    static DeviceConfigurationKeyValueDescriptionListDataType get_device_configuration_list_data();
    static DeviceConfigurationKeyValueListDataType get_device_configuration_value_list_data();

    // DeviceDiagnosis
    static DeviceDiagnosisStateDataType get_state_data();

    // ElectricalConnection
    static constexpr uint8_t lpcElectricalConnectionIdOffset = 0;
    static constexpr uint8_t lpcElectricalConnectionCharacteristicIdOffset = 10;
    static constexpr uint8_t lpcElectricalConnectionParameterIdOffset = 0;
    static constexpr uint8_t mpcElectricalConnectionParameterIdOffset = 10; // next offset should be +20 atleast
    static ElectricalConnectionCharacteristicListDataType get_electrical_connection_characteristic_list_data();
    static ElectricalConnectionDescriptionListDataType get_electrical_connection_description_list_data();
    static ElectricalConnectionParameterDescriptionListDataType get_electrical_connection_parameter_description_list_data();

    // DeviceClassification
    static DeviceClassificationManufacturerDataType get_device_classification_manufacturer_data();

    // Measurement
    static constexpr uint8_t lpcMeasurementIdOffset = 0;
    static constexpr uint8_t mpcMeasurementIdOffset = 10; // next offset should be +20
    static MeasurementDescriptionListDataType get_measurement_description_list_data();
    static MeasurementConstraintsListDataType get_measurement_constraints_list_data();
    static MeasurementListDataType get_measurement_list_data();

    // Bill
    BillDescriptionListDataType get_bill_description_list_data();
    BillConstraintsListDataType get_bill_constraints_list_data();
    BillListDataType get_bill_list_data();
};

class EVEntity
{
public:
    inline static std::vector<int> entity_address = {1, 1};
    // DeviceConfiguration
    static constexpr uint8_t evccDeviceconfigurationIdOffset = 0;
    static DeviceConfigurationKeyValueDescriptionListDataType get_device_configuration_value_description_list();
    static DeviceConfigurationKeyValueListDataType get_device_configuration_value_list();

    // Identification
    static IdentificationListDataType get_identification_list_data();

    // DeviceClassification
    static DeviceClassificationManufacturerDataType get_device_classification_manufacturer_data();

    // ElectricalConnection
    static constexpr uint8_t evccElectricalConnectionIdOffset = 0;
    static constexpr uint8_t evcemElectricalconnectionParameterIdOffset = 0;
    static constexpr uint8_t opevElectricalconnectionParameterIdOffset = 10;
    static constexpr uint8_t evccElectricalconnectionParameterIdOffset = 20;
    static ElectricalConnectionParameterDescriptionListDataType get_electrical_connection_parameter_description_list_data();
    static ElectricalConnectionPermittedValueSetListDataType get_electrical_connection_permitted_list_data();
    static ElectricalConnectionDescriptionListDataType get_electrical_connection_description_list_data();

    // DeviceDiagnosis
    static DeviceDiagnosisStateDataType get_diagnosis_state_data();

    // Measurement
    static constexpr uint8_t evcemMeasurementIdOffset = 0;
    static MeasurementDescriptionListDataType get_measurement_description_list_data();
    static MeasurementConstraintsListDataType get_measurement_constraints_list_data();
    static MeasurementListDataType get_measurement_list_data();

    // LoadControl
    static constexpr uint8_t opevLoadcontrolIdOffset = 0;
    static constexpr uint8_t opevMeasurementIdOffset = 10;
    static LoadControlLimitDescriptionListDataType get_load_control_limit_description_list_data();
    static LoadControlLimitListDataType get_load_control_limit_list_data();
    static LoadControlLimitConstraintsListDataType get_load_control_limit_constraints_list_data();
};
/**
 * The basic Framework of a EEBUS Usecase.
 * Each usecase has one or multiple features and belongs to an entity
 * Should only be inherited and never be used directly.
 */
class EebusUsecase
{
public:
    // Usecase Handlers
    virtual ~EebusUsecase() = default;

    [[nodiscard]] virtual Usecases get_usecase_type() const = 0;

    void set_entity_address(const std::vector<int> &address)
    {
        entity_address = address;
    }
    [[nodiscard]] std::vector<int> get_entity_address() const
    {
        return entity_address;
    }

    [[nodiscard]] bool matches_entity_address(const std::vector<int> &address) const
    {
        return entity_address == address;
    }

    [[nodiscard]] bool isActive() const
    {
        return entity_active;
    }

    /**
     * Sends a full read to the given receiver for the given feature and function. Make sure the feature can handle the response.
     * @param sending_feature
     * @param receiver
     * @param function
     */
    void send_full_read(AddressFeatureType sending_feature, FeatureAddressType receiver, SpineDataTypeHandler::Function function) const;

    /**
     * \brief Handles a message for a usecase.
     * @param header SPINE header of the message. Contains information about the commandclassifier and the targeted entitiy.
     * @param data The actual Function call
     * @param response Where to write the response to. This is a JsonObject that should be filled with the response data.
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    virtual MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) = 0;

    /**
     * Returns the usecase information for this entity. This gives information about which usecases this entity belongs to. Multiple entities may belong to the same usecase.
     * @return The UseCaseInformationDataType that contains the information about the usecase.
     */
    virtual UseCaseInformationDataType get_usecase_information() = 0;

    [[nodiscard]] virtual std::vector<FeatureTypeEnumType> get_supported_features() const = 0;
    /**
     * Set a feature address for a given feature type.
     * @param feature_address The feature address. This must be unique within the usecase.
     * @param feature_type The feature type. This must also be unique within the usecase and must be one of the features returned by get_supported_features().
     */
    void set_feature_address(AddressFeatureType feature_address, FeatureTypeEnumType feature_type);

    [[nodiscard]] virtual NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const = 0; // An entity exists only once but can have multiple features.
    [[nodiscard]] virtual std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const = 0;

    /**
     * If the usecase utilizes heartbeats, this function is called when a heartbeat is received and should be implemented on the usecase.
     */
    virtual void receive_heartbeat()
    {
    }

    /**
    * If the usecase utilizes heartbeats, this function is called when a heartbeat timeout occurs and should be implemented on the usecase.
    */
    virtual void receive_heartbeat_timeout()
    {
    }

    /**
     * When the supported usecases or supported entities change, this function is called to inform the usecase about the change.
     * @param conn
     */
    virtual void inform_spineconnection_usecase_update(SpineConnection *conn)
    {
    }

protected:
    std::vector<int> entity_address{}; // The feature address of the usecase. This is used to identify the usecase in the NodeManagementUseCaseDataType.

    bool entity_active = true;                                             // If the entity is active or not. Inactive entities do not respond to messages or their entity and feature information should not be called.
    std::map<FeatureTypeEnumType, AddressFeatureType> feature_addresses{}; // The feature addresses of the features in this usecase.
    [[nodiscard]] FeatureTypeEnumType get_feature_by_address(AddressFeatureType feature_address) const;

    /**
     * Get a FeatureAddressType for the given feature type for the current usecase.
     * @param feature
     * @return
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

/**
 * The Default Nodemanagement UseCase. Needs to be supported by all EEBUS devices.
 */
class NodeManagementEntity final : public EebusUsecase
{
public:
    NodeManagementEntity();
    void set_usecaseManager(EEBusUseCases *usecases);

    /**
     * Checks if the given client is bound to the given server entity and feature.
     * @param sending_feature The client FeatureAddressType to check.
     * @param target_feature The server FeatureAddressType to check.
     * @return true if the client is bound to the server, false otherwise.
     */
    bool check_is_bound(FeatureAddressType &sending_feature, FeatureAddressType &target_feature) const;

    UseCaseInformationDataType get_usecase_information() override;
    /**
    * \brief Handles a message for a usecase.
    * @param header SPINE header of the message. Contains information about the commandclassifier and the targeted entitiy.
    * @param data The actual Function call
    * @param response Where to write the response to. This is a JsonObject that should be filled with the response data.
    * @return The MessageReturn object which contains information about the returned message.
    */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    /**
     * The NodeManagement Usecase only has one features which is the NodeManagement feature itself.
     * @return The lis of features supported by this entity.
     */
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::NMC;
    }

    /**
     * Inform all subscribers of the given entity. Sends out a Notify SPINE message with the given content
     * @param entity The entity emitting the notification
     * @param feature The feature emitting the notification
     * @param data The Data to be sent. The last_cmd value needs to be set to the appropriate function and the corresponding data field needs to be set.
     * @param function_name Name of the function that is informing the subscribers.
     * @return The number of subscribers that have been informed. 0 if no subscribers have been found.
     */
    template <typename T> size_t inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, T data, const char *function_name);

    /**
     * Subscribe to a feature. Make sure the sending feature supports notify messages.
     * @param sending_feature The feature that is subscribing to the target feature.
     * @param target_feature The target feature to subscribe to.
     * @param feature The feature type of the target feature. Not mandatory.
     * @return If the subscription request was successful. Note: This only means the subscription request was valid and a destination was found, not that the target feature accepted the subscription.
     */
    bool subscribe_to_feature(FeatureAddressType &sending_feature, FeatureAddressType &target_feature, FeatureTypeEnumType feature = FeatureTypeEnumType::EnumUndefined) const;

    /**
     * Informs the nodemanagement usecase that the detailed discovery data has changed and all subscribers should be informed.
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
    // The SPINE Protocol specification implies in 7.3.6 that this should be stored persistently but it also allows binding information to be discarded in case the device was offline.
    BindingManagementEntryListDataType binding_management_entry_list_{};
    /**
    * Handles a binding request for a usecase.
    * @param header SPINE header of the message. Might be required for releasing of bindings.
    * @param data The SpineDataTypeHandler that contains the command.
    * @param response The JsonObject to write the response to. This should be filled with the response data.
    * @return true if a response was generated and needs to be sent, false if no response is needed.
    */
    MessageReturn handle_binding(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);
};

#ifdef EEBUS_ENABLE_EVCS_USECASE
/**
 * The EEBUSChargingSummary Entity as defined in EEBus UC TS - EV Charging Summary V1.0.1.
 * This should have the same entity address as other entities with the EVSE actor <br>
 * Actor: EVSE <br>
 * Features: Bill (billDescriptionListData, billConstraintsListData, billListData) <br>
*/
class EvcsUsecase final : public EebusUsecase
{
public:
    EvcsUsecase();

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
     * @return The MessageReturn object which contains information about the returned message.
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::EVCS;
    }

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
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
    void update_billing_data(int id, time_t start_time, time_t end_time, int energy_wh, uint32_t cost_eur_cent, int grid_energy_percent = 100, int grid_cost_percent = 100, int self_produced_energy_percent = 0, int self_produced_cost_percent = 0);

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::Bill};
    }

    void get_bill_list_data(BillListDataType *data) const;

private:
    struct BillEntry {
        uint8_t id = 0; // ID of the bill entry. 0 means unused entry
        time_t start_time;
        time_t end_time;
        uint16_t energy_wh;
        uint32_t cost_eur_cent;
        uint8_t grid_energy_percent;
        uint8_t grid_cost_percent;
        uint8_t self_produced_energy_percent;
        uint8_t self_produced_cost_percent;
    };

    BillEntry bill_entries[8]{};

    void update_api() const;
};
#endif

#ifdef EEBUS_ENABLE_EVCC_USECASE
/**
 * The Evcc Entity as defined in EEBus UC TS - EV Commissioning and Configuration V1.0.1.
 * This should have the same entity address as other entities with the EV actor <br>
 * Actor: EV <br>
 * Features (Functions): DeviceDiagnosis (deviceConfigurationKeyValueDescriptionListData, deviceConfigurationKeyValueListData), Identification (identificationListData), DeviceClassification (deviceClassificationManufacturerData), ElectricalConnection (electricalConnectionParameterDescriptionListData, electricalConnectionPermittedValueSetListData), DeviceDiagnosis (deviceDiagnosisStateData) <br>
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
    * Builds and returns the UseCaseInformationDataType as defined in EEBus UC TS - EV Commissioning and Configuration V1.0.0. 3.1.2.
    * @return
    */
    UseCaseInformationDataType get_usecase_information() override;

    /**
    * \brief Handles a message for a usecase.
    * @param header SPINE header of the message. Contains information about the commandclassifier and the targeted entitiy.
    * @param data The actual Function call and data of the message.
    * @param response Where to write the response to. This is a JsonObject that should be filled with the response data.
    * @return The MessageReturn object which contains information about the message handling.
    */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;
    /**
     * The entity information as defined in EEBus UC TS - EV Commissioning and Configuration V1.0.1. 3.2.1.
     * @return The entity information.
     */
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
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
     * @param communication_standard The communication standard. Should only be "iso15118-2ed1","iso15118-2ed2" or "iec61851". Will log a tracelog if another value is given.
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

    [[nodiscard]] bool is_ev_connected() const
    {
        return ev_connected;
    }
    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::DeviceConfiguration, FeatureTypeEnumType::DeviceDiagnosis, FeatureTypeEnumType::Identification, FeatureTypeEnumType::DeviceClassification, FeatureTypeEnumType::ElectricalConnection};
    }
    void get_device_config_description(DeviceConfigurationKeyValueDescriptionListDataType *data) const;
    void get_device_config_list(DeviceConfigurationKeyValueListDataType *data) const;

    void get_identification_list(IdentificationListDataType *data) const;
    void get_device_classification_manufacturer(DeviceClassificationManufacturerDataType *data) const;
    void get_electrical_connection_parameter_description(ElectricalConnectionParameterDescriptionListDataType *data) const;
    void get_electrical_connection_permitted_values(ElectricalConnectionPermittedValueSetListDataType *data) const;

    [[nodiscard]] DeviceDiagnosisStateDataType get_device_diagnosis_state() const;

    // IDs as they are used in EV Commissioning and Configuration V1.0.1 3.2.1.2 in the definition of the features
    static constexpr uint16_t id_x_1 = EVEntity::evccDeviceconfigurationIdOffset + 1;
    static constexpr uint16_t id_x_2 = EVEntity::evccDeviceconfigurationIdOffset + 2;
    static constexpr uint16_t id_y_1 = EVEntity::evccElectricalConnectionIdOffset + 1;
    static constexpr uint16_t id_z_1 = EVEntity::evccElectricalconnectionParameterIdOffset + 1;

private:
    void update_api() const;
    bool ev_connected = false;

    // Server Data
    //DeviceDiagnosis
    CoolString communication_standard = "";
    bool asymmetric_supported = false;

    //Identification
    IdentificationTypeEnumType mac_type = IdentificationTypeEnumType::eui64;
    CoolString mac_address = "";
    //DeviceClassification
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
    //ElectricalConnection
    int min_power_draw = 0;
    int max_power_draw = 0;
    int standby_power = 0;
    //DeviceDiagnosis
    bool standby_mode = false;
};
#endif

#ifdef EEBUS_ENABLE_EVCEM_USECASE
/**
 * The EvcemUsecase Entity as defined in EEBus UC TS - EV Charging Electricity Measurement V1.0.1.
 * This should have the same entity address as other entities with the EV actor <br>
 * Actor: EV <br>
 * Features: Measurement (measurementDescriptionListData, measurementConstraintsListData, measurementListData), ElectricalConnection (electricalConnectionDescriptionListData, electricalConnectionParameterDescriptionListData) <br>
 */
class EvcemUsecase final : public EebusUsecase
{
public:
    EvcemUsecase();

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::EVCEM;
    }

    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;
    UseCaseInformationDataType get_usecase_information() override;
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * Update the measurements. This will inform all subscribers of the new measurements. Amps or power values at or below 0 are ignored and not send.
     * @param amps_phase_1 Milliamps on phase a
     * @param amps_phase_2 Milliamps on phase b
     * @param amps_phase_3 Milliamps on phase c
     * @param power_phase_1 Total power on phase 1 in watts
     * @param power_phase_2 Total power on phase 2 in watts
     * @param power_phase_3 Total power on phase 3 in watts
     * @param charged_wh Total charged into the ev during the current session in wh
     * @param charged_measured If the charged_wh value is measured. If false it is assumed its calculated. Default is false
     */
    void update_measurements(int amps_phase_1, int amps_phase_2, int amps_phase_3, int power_phase_1, int power_phase_2, int power_phase_3, int charged_wh, bool charged_measured = false);

    /**
     * Update the constraints of the system. This will inform all subscribers of the new constraints. Values set to negative values will be omitted from the eebus information
     * @param amps_min Minimum amps in mA that can be measured
     * @param amps_max Maximum amps in mA that can be measured
     * @param amps_stepsize Stepsize of the amps in mA measurement
     * @param power_min Minimum power in w that can be measured
     * @param power_max Maximum power in w  that can be measured
     * @param power_stepsize Stepsize of the power in w  measurement
     * @param energy_min Minimum charged power in wh that can be measured
     * @param energy_max Maximum charged power in wh that can be measured
     * @param energy_stepsize Stepsize of the charged power in wh measurement
     */
    void update_constraints(int amps_min, int amps_max, int amps_stepsize, int power_min, int power_max, int power_stepsize, int energy_min, int energy_max, int energy_stepsize);

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::Measurement, FeatureTypeEnumType::ElectricalConnection};
    }

    // Generators for data types
    void get_measurement_description_list(MeasurementDescriptionListDataType *data) const;
    void get_measurement_constraints(MeasurementConstraintsListDataType *data) const;
    void get_measurement_list(MeasurementListDataType *data) const;

    void get_electrical_connection_description(ElectricalConnectionDescriptionListDataType *data) const;
    void get_electrical_connection_parameters(ElectricalConnectionParameterDescriptionListDataType *data) const;

    // IDs as they are used in Overload Protection by EV Charging Current Curtailment V1.0.1b 3.2.1.2 in the definition of the features
    static constexpr uint8_t id_x_1 = EVEntity::evcemMeasurementIdOffset + 1;
    static constexpr uint8_t id_x_2 = EVEntity::evcemMeasurementIdOffset + 2;
    static constexpr uint8_t id_x_3 = EVEntity::evcemMeasurementIdOffset + 3;
    static constexpr uint8_t id_x_4 = EVEntity::evcemMeasurementIdOffset + 4;
    static constexpr uint8_t id_x_5 = EVEntity::evcemMeasurementIdOffset + 5;
    static constexpr uint8_t id_x_6 = EVEntity::evcemMeasurementIdOffset + 6;
    static constexpr uint8_t id_x_7 = EVEntity::evcemMeasurementIdOffset + 7;
    static constexpr uint8_t id_y_1 = EvccUsecase::id_y_1;
    static constexpr uint8_t id_z_1 = EVEntity::evcemElectricalconnectionParameterIdOffset + 1;
    static constexpr uint8_t id_z_2 = EVEntity::evcemElectricalconnectionParameterIdOffset + 2;
    static constexpr uint8_t id_z_3 = EVEntity::evcemElectricalconnectionParameterIdOffset + 3;
    static constexpr uint8_t id_z_4 = EVEntity::evcemElectricalconnectionParameterIdOffset + 4;
    static constexpr uint8_t id_z_5 = EVEntity::evcemElectricalconnectionParameterIdOffset + 5;
    static constexpr uint8_t id_z_6 = EVEntity::evcemElectricalconnectionParameterIdOffset + 6;
    static constexpr uint8_t id_z_7 = EVEntity::evcemElectricalconnectionParameterIdOffset + 7;

private:
    // Data held about the current charge
    int milliamps_draw_phase[3] = {1, 1, 1}; // Milliamp draw per phase. Set to 1 to avoid zero values
    int power_draw_phase[3] = {1, 1, 1};     // Power per phase. Set to 1 to avoid zero values
    int power_charged_wh = 1;                // Total charged into the ev during the current session in wh
    bool power_charged_measured = false;

    // Constraints
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
#endif

#ifdef EEBUS_ENABLE_EVSECC_USECASE
/**
 * The EVSE Entity as defined in EEBus UC TS - EVSE Commissioning and Configuration V1.0.1.
 * This should have the same entity address as other entities with the EVSE actor <br>
 * Actor: EVSE <br>
 * Features (Functions): DeviceClassification (deviceClassificationManufacturerData), DeviceDiagnosis (deviceDiagnosisStateData) <br>
 */
class EvseccUsecase final : public EebusUsecase
{
public:
    EvseccUsecase();

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::EVSECC;
    }

    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;
    /**
    * Builds and returns the UseCaseInformationDataType as defined in EEBus UC TS - EVSE Commissioning and Configuration V1.0.0. 3.1.2.
    * @return
    */
    UseCaseInformationDataType get_usecase_information() override;
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * Update the operating state of the EVSE. As required by Scenario 2 of the Usecase. This will inform all subscribers of the new operating state.
     * @param failure If the evse is in failure state or not. If true, the operating state will be set to "failure". If false, the operating state will be set to "normalOperation".
     * @param error_message The message describing the failure. Only used if failure is true.
     */
    void update_operating_state(bool failure = false, const String &error_message = "");
    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::DeviceDiagnosis, FeatureTypeEnumType::DeviceClassification};
    }

    void get_device_diagnosis_state(DeviceDiagnosisStateDataType *data) const;
    static void get_device_classification_manufacturer(DeviceClassificationManufacturerDataType *data);

private:
    // Server Data
    DeviceDiagnosisOperatingStateEnumType operating_state = DeviceDiagnosisOperatingStateEnumType::normalOperation;
    std::string last_error_message;

    void update_api() const;
};
#endif
#ifdef EEBUS_ENABLE_LPC_USECASE

/**
 * The LpcUsecase as defined in EEBus UC TS - EV Limitation Of Power Consumption V1.0.0.
 * This should have the same entity address as other entities with the Controllable System actor <br>
 * Actor: Controllable System <br>
 * Features (Functions): LoadControl (loadControlLimitDescriptionListData, loadControlLimitListData), DeviceConfiguration (deviceConfigurationKeyValueDescriptionListData, deviceConfigurationKeyValueListData), DeviceDiagnosis (deviceDiagnosisHeartbeatData), ElectricalConnection (electricalConnectionCharacteristicListData)   <br>
*/

class LpcUsecase final : public EebusUsecase
{
public:
    LpcUsecase();

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
     * @return The MessageReturn object which contains information about the returned message.
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::LPC;
    }

    /**
     * The entity information as defined in EEBus UC TS - EV Limitation Of Power Consumption V1.0.0. 3.2.2.
     * @return The entity information.
     */
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    /**
     * Returns the supported features. As defined in EEBus UC TS - EV Limitation Of Power Consumption V1.0.0. 3.2.2.2.1.
     * @return a list of the supported features.
     */
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * Update the limit the system is supposed to be consuming.
     * @param limit If the limit is active or not.
     * @param current_limit_w The limit in W.
     * @param duration For how long the limit shall be active from now
     * @return true if the processing of the limit was successful.
     */
    bool update_lpc(bool limit, int current_limit_w, seconds_t duration);

    /**
     * Update the maximum power the system is currently capable of consuming. This will inform all subscribers of the new power limit. Implemented according to LPC UC TS v1.0.0 3.2.2.2.3.1
     * @param power_limit_w Power limit in watts. Below -1 the value is ignored
     * @param duration Duration in seconds. Below or equal to -1 the value is ignored.
     */
    void update_failsafe(int power_limit_w = -1, seconds_t duration = -1_s);

    /**
     * Update the constraints of the system. This will inform all subscribers of the new constraints. Implemented according to LPC UC TS v1.0.0 3.2.2.2.5.1
     * @param power_consumption_max Maximum power consumption the system is capable of consuming. This is the physical limit of the device and should only be set if the device is not an energy manager. Set to 0 to omit it.
     * @param power_consumption_contract_max Maximum power consumption the contract allows. This should only be set if the device is an energy manager.
     */
    void update_constraints(int power_consumption_max = 0, int power_consumption_contract_max = 0);

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        // The feature "DeviceDiagnosis" is handled by a separate heartbeat handler as it can be shared accross multiple usecases
        return {FeatureTypeEnumType::LoadControl, FeatureTypeEnumType::DeviceConfiguration, FeatureTypeEnumType::ElectricalConnection};
    }

    void get_loadcontrol_limit_description(LoadControlLimitDescriptionListDataType *data) const;
    void get_loadcontrol_limit_list(LoadControlLimitListDataType *data) const;

    void get_device_configuration_value(DeviceConfigurationKeyValueListDataType *data) const;
    void get_device_configuration_description(DeviceConfigurationKeyValueDescriptionListDataType *data) const;

    void get_electrical_connection_characteristic(ElectricalConnectionCharacteristicListDataType *data) const;

    [[nodiscard]] bool limit_is_active() const
    {
        return limit_active;
    }
    [[nodiscard]] int get_current_limit_w() const
    {
        return current_active_consumption_limit_w;
    }
    void receive_heartbeat() override;
    void receive_heartbeat_timeout() override;

    void inform_spineconnection_usecase_update(SpineConnection *conn) override;

    // IDs as they are used in EV Limitation Of Power Consumption V1.0.0 3.2.2.2
    static constexpr uint8_t id_l_1 = EVSEEntity::lpcLoadcontrolLimitIdOffset + 1;
    static constexpr uint8_t id_m_1 = EVSEEntity::lpcMeasurementIdOffset + 1;
    static constexpr uint16_t id_k_1 = EVSEEntity::lpcDeviceConfigurationKeyIdOffset + 1;
    static constexpr uint16_t id_k_2 = EVSEEntity::lpcDeviceConfigurationKeyIdOffset + 2;
    static constexpr uint16_t id_ec_1 = EVSEEntity::lpcElectricalConnectionIdOffset + 1;
    static constexpr uint16_t id_cc_1 = EVSEEntity::lpcElectricalConnectionCharacteristicIdOffset + 1;
    static constexpr uint16_t id_cc_2 = EVSEEntity::lpcElectricalConnectionCharacteristicIdOffset + 2;
    static constexpr uint16_t id_p_1 = EVSEEntity::lpcElectricalConnectionParameterIdOffset + 1;

private:
    /**
     * The Load Control feature as required for Scenario 1 - Control active power consumption.
     * It is the primary feature of the entity and major part of the LPC Usecases.
     * Takes in a limit, if its enabled and a duration and attempts to apply it.
     * Requires a binding to write.
     * Supports subscriptions.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.1 and 3.4.1
     */
    MessageReturn load_control_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);

    /**
     * The Device Configuration feature as required for Scenario 2 - Failsafe Values.
     * Reports the current failsafe values and allows them to be updated.
     * A failsafe value is the maximum power that can be consumed if no contact to the energy guard can be established and always comes with a duration for which the failsafe will be active.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.2 and 3.4.2
     */
    MessageReturn deviceConfiguration_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);

    /**
     * The Device Diagnosis feature as required for Scenario 3 - Heartbeat.
     * Implements a heartbeat mechanism to ensure that the energy guard is still online and reachable, if not it enters the failsafe state.
     * If no heartbeat is received for a certain time, the system will switch to failsafe mode.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.3 and 3.4.3
     */
    // Handled in the common heartbeat handler

    /**
     * The Electrical Connection feature as required for Scenario 4 - Constraints.
     * Reports the current constraints of the system and is read only.
     * Constraints are the maximum power the system is capable of consuming.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.4 and 3.4.4
     */
    static MessageReturn electricalConnection_feature(const HeaderType &header, const SpineDataTypeHandler *data, JsonObject response);

    // State handling
    // State machine as described in LPC UC TS v1.0.0 2.3
    void update_state();
    LPCState lpc_state = LPCState::Startup;
    uint64_t state_change_timeout_task = 0;
    bool heartbeat_received = false;

    /**
     * Switch to or update the Init state. As described in LPC UC TS v1.0.0 2.3.2
     * Triggered on startup.
     * Disables the active consumption limit and applies the failsafe limit.
     */
    void init_state();

    /**
     * Switch to or update the Unlimited/controlled state. As described in LPC UC TS v1.0.0 2.3.2
     * Entered when a heartbeat and limit is received but no limit is enabled.
     * Disables the active consumption limit.
     */
    void unlimited_controlled_state();

    /**
     * Switch or update the Limited state. As described in LPC UC TS v1.0.0 2.3.2
     * Entered when a heartbeat, limit and activation of the limit is received.
     * Enables the active consumption limit and applies the value to the charging system.
     */
    void limited_state();

    /**
     * Switch or update the Failsafe state. As described in LPC UC TS v1.0.0 2.3.2
     * Entered when no heartbeat is received.
     * Applies the failsafe limit.
     */
    void failsafe_state();
    bool failsafe_expired = false;

    /**
     * Switch or update the Unlimited/autonomous state. As described in LPC UC TS v1.0.0 2.3.2
     * Entered when no heartbeat and write is received after startup or after expiry of the failsafe duration.
     * Disables the active consumption limit.
     */
    void unlimited_autonomous_state();

    void update_api() const;

    // LoadControl configuration as required for scenario 1 - Control Active Power
    // If a limit was received since startup
    bool limit_received = false;
    // While the limit is active, this shall be set to true
    bool limit_active = false;
    // The current limit in effect
    int current_active_consumption_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    // The configured limit as received from the energy manager
    int configured_limit = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    // If the limit is changeable, this shall be set to false
    constexpr static bool limit_fixed = false;
    // The description ID of the limit so its consistent across description and limit list data
    int limit_description_id = id_l_1;
    int limit_measurement_description_id = id_m_1;
    // Time when the limit shall end
    time_t limit_endtime = 0;
    // If the limit is expired
    bool limit_expired = false;
    // If in limited mode, this shall
    uint64_t limit_endtime_timer = 0;

    // Device Configuration Data as required for Scenario 2 - Failsafe values
    int failsafe_power_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    seconds_t failsafe_duration = 2_h; // Default to 2 hours
    uint64_t failsafe_expiry_timer = 0;
    time_t failsafe_expiry_endtime = 0;
    uint8_t failsafe_consumption_key_id = id_k_1;
    uint8_t failsafe_duration_key_id = id_k_2;

    // Electrical Connection Data as required for Scenario 4 - Constraints
    int power_consumption_max_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;          // This device shall only be used if the device is a consumer
    int power_consumption_contract_max_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION; // This value shall only be used if the device is an enery manager
};
#endif

#ifdef EEBUS_ENABLE_CEVC_USECASE
/**
 * The CevcUsecase as defined in EEBus UC TS - Coordinated EV Charging V1.0.1.
 * This should have the same entity address as other entities with the EV actor.
 *
 * Actor: EV (Energy Guard)
 *
 * Scenarios:
 *   1 (3.4.1): EV sends charging energy demand - TimeSeries singleDemand
 *   2 (3.4.2): Max power limitation - TimeSeries constraints
 *   3 (3.4.3): Incentive table - IncentiveTable for tariff data
 *   4 (3.4.4): Charging plan curve - TimeSeries plan (write from Energy Broker)
 *   5 (3.4.5): Energy Guard heartbeat - DeviceDiagnosis (via EebusHeartBeat)
 *   6 (3.4.6): Energy Broker heartbeat - DeviceDiagnosis (via EebusHeartBeat)
 *   7 (3.4.7): Energy Guard error state - DeviceDiagnosis state
 *   8 (3.4.8): Energy Broker error state - DeviceDiagnosis state
 *
 * Features (Functions):
 *   - TimeSeries: timeSeriesDescriptionListData, timeSeriesConstraintsListData, timeSeriesListData
 *   - IncentiveTable: incentiveTableDescriptionData, incentiveTableConstraintsData, incentiveTableData
 */
class CevcUsecase final : public EebusUsecase
{
public:
    CevcUsecase();

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::CEVC;
    }

    /**
     * Handles a message for the CEVC usecase.
     * @param header SPINE header of the message.
     * @param data The actual Function call and data of the message.
     * @param response Where to write the response to.
     * @return The MessageReturn object which contains information about the returned message.
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    /**
     * Builds and returns the UseCaseInformationDataType as defined in CEVC spec 3.1.2.
     */
    UseCaseInformationDataType get_usecase_information() override;
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::TimeSeries, FeatureTypeEnumType::IncentiveTable};
    }

    /**
     * Called when heartbeat timeout occurs from the Energy Broker.
     * Invalidates the charging plan and incentives (Scenario 8).
     */
    void receive_heartbeat_timeout() override;

    /**
     * Called when a SPINE connection discovers use cases.
     * Registers for heartbeat monitoring from Energy Broker (Scenarios 5-6).
     */
    void inform_spineconnection_usecase_update(SpineConnection *conn) override;

    // =====================================================================
    // Public data structures
    // =====================================================================

    /**
     * A charging demand slot entry for communicating EV charging demands.
     * Used for Scenario 1: EV sends charging energy demand.
     */
    struct ChargingDemandSlot {
        seconds_t duration;  ///< Duration of this slot
        int min_power_w = 0; ///< Minimum power in this slot (W)
        int max_power_w = 0; ///< Maximum power in this slot (W)
        int opt_power_w = 0; ///< Optimal/expected power in this slot (W)
    };

    /**
     * A charging plan slot entry received from the Energy Broker.
     * Used for Scenario 4: Charging plan curve.
     */
    struct ChargingPlanSlot {
        seconds_t duration; ///< Duration of this slot
        int power_w = 0;    ///< Target power for this slot (W)
    };

    /**
     * A power limit slot entry for communicating constraints.
     * Used for Scenario 2: Max power limitation.
     */
    struct PowerConstraintSlot {
        seconds_t duration;  ///< Duration of this constraint
        int max_power_w = 0; ///< Maximum power limit (W)
    };

    /**
     * An incentive tier within a time slot.
     * Represents a power-based tier with associated cost/incentive value.
     */
    struct IncentiveTier {
        int lower_boundary_w = 0;     ///< Lower power boundary (W)
        int upper_boundary_w = 0;     ///< Upper power boundary (W)
        float incentive_value = 0.0f; ///< Cost or incentive value (cents/kWh or relative)
    };

    /**
     * An incentive slot entry for communicating tariff information.
     * Used for Scenario 3: Incentive table.
     */
    struct IncentiveSlotEntry {
        time_t start_time = 0;            ///< Start time of this slot (Unix timestamp)
        time_t end_time = 0;              ///< End time of this slot (Unix timestamp)
        std::vector<IncentiveTier> tiers; ///< Tiers within this slot
    };

    // =====================================================================
    // Public update methods for charging system integration
    // =====================================================================

    /**
     * Updates the EV charging demand (Scenario 1).
     * Call this when the EV's charging requirements change.
     * @param min_energy_wh Minimum energy needed (Wh)
     * @param opt_energy_wh Optimal energy wanted (Wh)
     * @param max_energy_wh Maximum energy that can be stored (Wh)
     * @param target_time Target departure time (Unix timestamp)
     * @param slots Detailed charging demand slots (optional)
     */
    void update_charging_demand(int min_energy_wh, int opt_energy_wh, int max_energy_wh, time_t target_time, const std::vector<ChargingDemandSlot> &slots = {});

    /**
     * Updates the power constraints (Scenario 2).
     * Call this when the maximum power limits change.
     * @param max_power_w Maximum power the EV can consume (W)
     * @param slots Detailed power constraint slots (optional)
     */
    void update_power_constraints(int max_power_w, const std::vector<PowerConstraintSlot> &slots = {});

    /**
     * Gets the currently active charging plan received from the Energy Broker.
     * @return Vector of charging plan slots, empty if no plan received.
     */
    [[nodiscard]] const std::vector<ChargingPlanSlot> &get_charging_plan() const
    {
        return received_charging_plan;
    }

    /**
     * Gets the current target power from the charging plan for a given time.
     * @param time Unix timestamp to query (0 = now)
     * @return Target power in Watts, or -1 if no plan active.
     */
    [[nodiscard]] int get_target_power_w(time_t time = 0) const;

    /**
     * Gets the currently available incentives received from the Energy Broker.
     * @return Vector of incentive slots, empty if no incentives received.
     */
    [[nodiscard]] const std::vector<IncentiveSlotEntry> &get_incentives() const
    {
        return received_incentives;
    }

    /**
     * Checks if a valid charging plan has been received.
     * @return true if a charging plan is active.
     */
    [[nodiscard]] bool has_charging_plan() const
    {
        return !received_charging_plan.empty() && charging_plan_valid;
    }

    /**
     * Checks if valid incentive data has been received.
     * @return true if incentive data is available.
     */
    [[nodiscard]] bool has_incentives() const
    {
        return !received_incentives.empty() && incentives_valid;
    }

    // =====================================================================
    // ID Constants (for consistent IDs across the use case)
    // =====================================================================

    /// TimeSeries ID for single charging demand (Scenario 1)
    static constexpr int TIME_SERIES_ID_DEMAND = 1;
    /// TimeSeries ID for power constraints (Scenario 2)
    static constexpr int TIME_SERIES_ID_CONSTRAINTS = 2;
    /// TimeSeries ID for charging plan from broker (Scenario 4)
    static constexpr int TIME_SERIES_ID_PLAN = 3;
    /// Tariff ID for incentive table
    static constexpr int TARIFF_ID = 1;
    /// Tier ID for dynamic cost tier
    static constexpr int TIER_ID = 1;
    /// Incentive ID for absolute cost
    static constexpr int INCENTIVE_ID = 1;
    /// Maximum number of time series slots
    static constexpr int MAX_TIME_SERIES_SLOTS = 48;
    /// Maximum number of incentive slots
    static constexpr int MAX_INCENTIVE_SLOTS = 48;

private:
    // =====================================================================
    // Private feature handler methods
    // =====================================================================

    // TimeSeries Feature
    void read_time_series_description(TimeSeriesDescriptionListDataType *data) const;
    void read_time_series_constraints(TimeSeriesConstraintsListDataType *data) const;
    void read_time_series_list(TimeSeriesListDataType *data) const;
    MessageReturn write_time_series_list(HeaderType &header, SpineOptional<TimeSeriesListDataType> data, JsonObject response);

    // IncentiveTable Feature
    void read_incentive_table_description(IncentiveTableDescriptionDataType *data) const;
    MessageReturn write_incentive_table_description(HeaderType &header, SpineOptional<IncentiveTableDescriptionDataType> data, JsonObject response);
    void read_incentive_table_constraints(IncentiveTableConstraintsDataType *data) const;
    void read_incentive_table_data(IncentiveTableDataType *data) const;
    MessageReturn write_incentive_table_data(HeaderType &header, SpineOptional<IncentiveTableDataType> data, JsonObject response);

    // Helper methods
    void notify_subscribers_time_series() const;
    void notify_subscribers_incentives() const;
    void update_api_state() const;

    // =====================================================================
    // Internal state - Charging demand (EV -> Broker, Scenario 1)
    // =====================================================================
    int minimum_energy_wh = 0;                             ///< Minimum energy needed
    int optimal_energy_wh = 0;                             ///< Optimal energy wanted
    int maximum_energy_wh = 0;                             ///< Maximum storable energy
    time_t target_departure_time = 0;                      ///< Target departure time
    std::vector<ChargingDemandSlot> charging_demand_slots; ///< Detailed demand slots

    // =====================================================================
    // Internal state - Power constraints (EV -> Broker, Scenario 2)
    // =====================================================================
    int max_power_constraint_w = 22000;                      ///< Maximum power constraint
    std::vector<PowerConstraintSlot> power_constraint_slots; ///< Detailed constraint slots

    // =====================================================================
    // Internal state - Charging plan (Broker -> EV, Scenario 4)
    // =====================================================================
    time_t charging_plan_start_time = 0;                  ///< Start time of received plan
    std::vector<ChargingPlanSlot> received_charging_plan; ///< Received charging plan
    bool charging_plan_valid = false;                     ///< Whether the plan is valid/active

    // =====================================================================
    // Internal state - Incentives (Broker -> EV, Scenario 3)
    // =====================================================================
    std::vector<IncentiveSlotEntry> received_incentives;         ///< Received incentive table
    bool incentives_valid = false;                               ///< Whether incentives are valid
    CurrencyEnumType incentive_currency = CurrencyEnumType::EUR; ///< Currency for incentives

    // =====================================================================
    // Internal state - Energy Broker connection (Scenarios 5-8)
    // =====================================================================
    bool energy_broker_connected = false;    ///< Whether Energy Broker is connected
    bool energy_broker_heartbeat_ok = false; ///< Whether Energy Broker heartbeat is healthy

    /**
     * Invalidates all data received from the Energy Broker.
     * Called when the Energy Broker goes into error state or heartbeat times out.
     */
    void invalidate_broker_data();
};
#endif

#ifdef EEBUS_ENABLE_MPC_USECASE
/**
 * The MPC usecase as defined in EEBus UC TS - Monitoring of Power Consumption V1.0.0
 * This should have the same entity address as other entities with the EVSE <br>
 * Actor: MonitoredUnit <br>
 * Features (Functions): ElectricalConnection (electricalConnectionDescriptionListData, electricalConnectionParameterDescriptionLostData), Measurement (measurementDescriptionListData, measurementConstraintsListData, measurementListData)   <br>
*/
class MpcUsecase final : public EebusUsecase
{
public:
    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::MPC;
    }

    /**
     * \brief Handles a message for a usecase.
     * @param header SPINE header of the message. Contains information about the commandclassifier and the targeted entitiy.
     * @param data The actual Function call and data of the message.
     * @param response Where to write the response to. This is a JsonObject that should be filled with the response data.
     * @return The MessageReturn object which contains information about the returned message.
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    /**
    * Builds and returns the UseCaseInformationDataType as defined in EEBus UC TS - EV Limitation Of Power Consumption V1.0.0. 3.1.2.
    * @return
    */
    UseCaseInformationDataType get_usecase_information() override;
    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::ElectricalConnection, FeatureTypeEnumType::Measurement};
    }
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    MpcUsecase();

    static void get_electricalConnection_description_list_data(ElectricalConnectionDescriptionListDataType *data);
    void get_electricalConnection_parameter_description_list_data(ElectricalConnectionParameterDescriptionListDataType *data) const;
    static void get_measurement_description_list_data(MeasurementDescriptionListDataType *data);
    void get_measurement_constraints_list_data(MeasurementConstraintsListDataType *data) const;
    void get_measurement_list_data(MeasurementListDataType *data) const;

    /**
     * Update the power measurements. This will inform all subscribers of the new measurements.
     * All values are accepted including 0 and negative values (negative = power feed-in).
     * @param total_power Total power consumption in watts (id_m_1) - Linked to LPC. Negative values indicate power feed-in.
     * @param power_phase_1 Power on phase 1 in watts (id_m_2_1). Negative values indicate power feed-in.
     * @param power_phase_2 Power on phase 2 in watts (id_m_2_2). Negative values indicate power feed-in.
     * @param power_phase_3 Power on phase 3 in watts (id_m_2_3). Negative values indicate power feed-in.
     */
    void update_power(int total_power, int power_phase_1, int power_phase_2, int power_phase_3);

    /**
     * Update the energy measurements. This will inform all subscribers of the new measurements.
     * All values are accepted including 0.
     * @param energy_consumed Energy consumed in watt-hours (id_m_3)
     * @param energy_produced Energy produced in watt-hours (id_m_4)
     */
    void update_energy(uint32_t energy_consumed, uint32_t energy_produced);

    /**
     * Update the current measurements. This will inform all subscribers of the new measurements.
     * All values are accepted including 0 and negative values (negative = reverse current flow).
     * @param current_phase_1 Current on phase 1 in milliamps (id_m_5_1). Negative values indicate reverse flow.
     * @param current_phase_2 Current on phase 2 in milliamps (id_m_5_2). Negative values indicate reverse flow.
     * @param current_phase_3 Current on phase 3 in milliamps (id_m_5_3). Negative values indicate reverse flow.
     */
    void update_current(int current_phase_1, int current_phase_2, int current_phase_3);

    /**
     * Update the voltage measurements. This will inform all subscribers of the new measurements.
     * All values are accepted including 0.
     * @param voltage_phase_1 Voltage on phase 1 (phase-to-neutral) in volts (id_m_6_1)
     * @param voltage_phase_2 Voltage on phase 2 (phase-to-neutral) in volts (id_m_6_2)
     * @param voltage_phase_3 Voltage on phase 3 (phase-to-neutral) in volts (id_m_6_3)
     * @param voltage_phase_1_2 Voltage between phase 1 and 2 in volts (id_m_6_4)
     * @param voltage_phase_2_3 Voltage between phase 2 and 3 in volts (id_m_6_5)
     * @param voltage_phase_3_1 Voltage between phase 3 and 1 in volts (id_m_6_6)
     */
    void update_voltage(int voltage_phase_1, int voltage_phase_2, int voltage_phase_3, int voltage_phase_1_2, int voltage_phase_2_3, int voltage_phase_3_1);

    /**
     * Update the frequency measurement. This will inform all subscribers of the new measurement.
     * All values are accepted including 0.
     * @param frequency_millihertz Grid frequency in millihertz (id_m_7), default is 50000 (50Hz)
     */
    void update_frequency(int frequency_millihertz);

    /**
     * Update all measurement constraints. This will inform all subscribers of the new constraints.
     * Values set to negative or 0 will use defaults.
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

    // IDs as used in Monitoring of Power Consumption V1.0.0 3.2.2.2 in the definition of the features
    // Linked IDs are described in the spec for the LPC usecase
    static constexpr uint8_t id_ec_1 = LpcUsecase::id_ec_1; // Linked to LPC usecase electrical connection description
    static constexpr uint8_t id_m_1 = LpcUsecase::id_m_1;   // Linked to LPC usecase measurement description
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
    static constexpr uint8_t id_p_1 = LpcUsecase::id_p_1; // Linked to LPC usecase electrical connection parameter description
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
    // =====================================================================
    // Measurement values - Scenario 1: Monitor Power
    // =====================================================================
    int total_power_w = 0;            ///< Total power (id_m_1) - Linked to LPC
    int power_phase_w[3] = {0, 0, 0}; ///< Power per phase (id_m_2_1, id_m_2_2, id_m_2_3)

    // =====================================================================
    // Measurement values - Scenario 2: Monitor Energy
    // =====================================================================
    uint32_t energy_consumed_wh = 0; ///< Energy consumed (id_m_3)
    uint32_t energy_produced_wh = 0; ///< Energy produced (id_m_4)

    // =====================================================================
    // Measurement values - Scenario 3: Monitor Current
    // =====================================================================
    int current_phase_ma[3] = {0, 0, 0}; ///< Current per phase in mA (id_m_5_1, id_m_5_2, id_m_5_3)

    // =====================================================================
    // Measurement values - Scenario 4: Monitor Voltage
    // =====================================================================
    int voltage_phase_to_neutral_v[3] = {0, 0, 0}; ///< Voltage phase-to-neutral (id_m_6_1, id_m_6_2, id_m_6_3)
    int voltage_phase_to_phase_v[3] = {0, 0, 0};   ///< Voltage phase-to-phase (id_m_6_4, id_m_6_5, id_m_6_6)

    // =====================================================================
    // Measurement values - Scenario 5: Monitor Frequency
    // =====================================================================
    int frequency_mhz = 50000; ///< Grid frequency in millihertz (id_m_7), default 50Hz

    // =====================================================================
    // Constraint values - Power
    // =====================================================================
    int power_limit_min_w = 0;                                          ///< Minimum power that can be measured
    int power_limit_max_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION; ///< Maximum power that can be measured
    int power_limit_stepsize_w = 1;                                     ///< Step size for power measurements

    // =====================================================================
    // Constraint values - Current
    // =====================================================================
    int current_limit_min_ma = 0;      ///< Minimum current that can be measured (mA)
    int current_limit_max_ma = 32000;  ///< Maximum current that can be measured (mA)
    int current_limit_stepsize_ma = 1; ///< Step size for current measurements (mA)

    // =====================================================================
    // Constraint values - Energy
    // =====================================================================
    uint32_t energy_limit_min_wh = 0;         ///< Minimum energy that can be measured
    uint32_t energy_limit_max_wh = 100000000; ///< Maximum energy that can be measured (100MWh)
    uint32_t energy_limit_stepsize_wh = 1;    ///< Step size for energy measurements

    // =====================================================================
    // Constraint values - Voltage
    // =====================================================================
    int voltage_limit_min_v = 0;      ///< Minimum voltage that can be measured
    int voltage_limit_max_v = 500;    ///< Maximum voltage that can be measured
    int voltage_limit_stepsize_v = 1; ///< Step size for voltage measurements

    // =====================================================================
    // Constraint values - Frequency
    // =====================================================================
    int frequency_limit_min_mhz = 45000;   ///< Minimum frequency in mHz (45Hz)
    int frequency_limit_max_mhz = 65000;   ///< Maximum frequency in mHz (65Hz)
    int frequency_limit_stepsize_mhz = 10; ///< Step size for frequency in mHz (0.01Hz)

    // =====================================================================
    // Helper methods
    // =====================================================================
    void update_api() const;
};
#endif

#ifdef EEBUS_ENABLE_OPEV_USECASE
#ifndef EEBUS_ENABLE_EVCC_USECASE
#error "OPEV Usecase requires EVCC Usecase to be enabled"
#endif
#ifndef EEBUS_ENABLE_EVCEM_USECASE
#error "OPEV Usecase requires EVCEM Usecase to be enabled"
#endif

/**
 * The OpevUsecase as defined in EEBus UC TS - Overload Protection by EV Charging Current Curtailment V1.0.1b
 * This should have the same entity address as other entities with the EV <br>
 * Actor: EV <br>
 * Features (Functions): LoadControl (loadControlLimitDescriptionListData, loadControlLimitListData),ElectricalConnection (electricalConnectionParameterDescriptionLostData)   <br>
*/
class OpevUsecase final : public EebusUsecase
{
public:
    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::OPEV;
    };
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    /**
    * Builds and returns the UseCaseInformationDataType as defined in EEBus UC TS - EV Limitation Of Power Consumption V1.0.0. 3.1.2.
    * @return
    */
    UseCaseInformationDataType get_usecase_information() override;

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::LoadControl, FeatureTypeEnumType::ElectricalConnection};
    }
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    void get_load_control_limit_description_list_data(LoadControlLimitDescriptionListDataType *data);
    void get_load_control_limit_list_data(LoadControlLimitListDataType *data) const;
    // The load control constraints are not actually in the spec but some implementations use them so this is here for compatibility sake.
    //void get_load_control_constraints_list_data(LoadControlLimitConstraintsListDataType *data) const;

    static void get_electrical_connection_parameter_description_list_data(ElectricalConnectionParameterDescriptionListDataType *data);
    void get_electrical_connection_permitted_list_data(ElectricalConnectionPermittedValueSetListDataType *data) const;

    /**
     * Update the limits possible with the OPEV usecase. This will inform all subscribers of the new limits.
     * @param limit_phase_1_milliamps
     * @param limit_phase_2_milliamps
     * @param limit_phase_3_milliamps
     * @param active if the limit shall be activated right away. Default is false.
     */
    void update_limits(int limit_phase_1_milliamps, int limit_phase_2_milliamps, int limit_phase_3_milliamps, bool active = false);
    inline void allow_limitation(bool allowed)
    {
        limit_changeable_allowed = allowed;
    }
    [[nodiscard]] bool limit_changeable() const;
    [[nodiscard]] std::array<int, 3> get_limit_milliamps() const
    {
        return {limit_per_phase_milliamps[0], limit_per_phase_milliamps[1], limit_per_phase_milliamps[2]};
    }
    [[nodiscard]] bool limit_is_active() const
    {
        return limit_active;
    }

    // IDs as they are used in Overload Protection by EV Charging Current Curtailment V1.0.1b 3.2.1.2 in the definition of the features
    // May be used to link to other ids in other usecases
    static constexpr uint8_t id_x_1 = EVEntity::opevLoadcontrolIdOffset + 1;
    static constexpr uint8_t id_x_2 = EVEntity::opevLoadcontrolIdOffset + 2;
    static constexpr uint8_t id_x_3 = EVEntity::opevLoadcontrolIdOffset + 3;
    static constexpr uint8_t id_z_1 = EvcemUsecase::id_x_1; // Should be the measurement ID entry on EVCEM
    static constexpr uint8_t id_z_2 = EvcemUsecase::id_x_2;
    static constexpr uint8_t id_z_3 = EvcemUsecase::id_x_3;
    static constexpr uint8_t id_i_1 = EVEntity::opevElectricalconnectionParameterIdOffset + 1;
    static constexpr uint8_t id_i_2 = EVEntity::opevElectricalconnectionParameterIdOffset + 2;
    static constexpr uint8_t id_i_3 = EVEntity::opevElectricalconnectionParameterIdOffset + 3;
    static constexpr uint8_t id_j_1 = EvcemUsecase::id_y_1;

private:
    // Values of the usecase
    int limit_per_phase_milliamps[3] = {32000, 32000, 32000};
    bool limit_active = false;
    bool limit_changeable_allowed = true;
    int limit_milliamps_min = 100;
    int limit_milliamps_max = 32000;

    //write functions
    MessageReturn write_load_control_limit_list_data(HeaderType &header, SpineOptional<LoadControlLimitListDataType> data, JsonObject response);
};
#endif

class EebusHeartBeat : public EebusUsecase
{
public:
    EebusHeartBeat();
    /**
     * Read the current heartbeat information.
     * @return
     */
    DeviceDiagnosisHeartbeatDataType read_heartbeat();

    /**
     * Initialize a heartbeat on a feature. This triggers a subscribtion to the heartbeat feature on the target.
     * If a target wants a heartbeat from us, it has to create a subscription.
     * @param target Target entity address of the heartbeat target.
     * @param sending_usecase The usecase that is requesting the heartbeat to be sent.
     * @param expect_notify If true, a subscription will be created to receive heartbeat notifications.
     */
    void initialize_heartbeat_on_feature(FeatureAddressType &target, Usecases sending_usecase, bool expect_notify = true);

    /**
     * Update the interval. This resets the notify timer and timeout timer.
     * @param interval
     */
    void update_heartbeat_interval(seconds_t interval = 30_s);

    /**
     * List of targets the heartbeat is sent to.
     * @return
     */
    [[nodiscard]] std::vector<FeatureAddressType> get_heartbeat_targets() const
    {
        return heartbeat_targets;
    }
    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::HEARTBEAT;
    }
    /**
     * Handle messages designated for devicediagnosis:heartbeat feature.
     * @param header
     * @param data
     * @param response
     * @return
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    UseCaseInformationDataType get_usecase_information() override
    {
        return {};
    };
    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::DeviceDiagnosis, FeatureTypeEnumType::Generic}; // Generic is the client feature that is needed for reads
    }
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    void register_usecase_for_heartbeat(EebusUsecase *usecase)
    {
        registered_usecases.push_back(usecase);
    }

    /**
     * If autosubscribe is enabled, the heartbeat handler will automatically subscribe to heartbeats reads on a new connection
     * @param enable
     */
    void set_autosubscribe(bool enable)
    {
        autosubscribe = enable;
    }

private:
    void emit_timeout() const;
    void send_heartbeat_to_subs();
    void emit_heartbeat_received(DeviceDiagnosisHeartbeatDataType &heartbeat_data);

    std::vector<FeatureAddressType> heartbeat_targets{};
    std::vector<Usecases> usecases_enabled{};
    std::vector<EebusUsecase *> registered_usecases{};

    seconds_t heartbeat_interval = 30_s;

    uint32_t heartbeat_counter = 0;

    uint64_t heartbeat_received_timeout_task = 0;
    uint64_t heartbeat_send_task = 0;

    bool autosubscribe = false;
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
    void process_spine_message(HeaderType &header, SpineDataTypeHandler *data, SpineConnection *connection);

    /**
     * Informs the subscribers of a feature about a change in the data.
     * @param entity the entity address of the feature
     * @param feature the feature address
     * @param data The Data the subscribers should be informed about. This is a SpineDataTypeHandler that contains the data.
     * @param function_name Name of the function this is notifying about. Must be the spine datatype name
     * @return The number of subscribers that have been informed. 0 if no subscribers were informed.
     */
    template <typename T> size_t inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, T &data, const char *function_name);

    /**
     * Send a message to a spine destination.
     * @param destination Spine FeatureAddressType of the destination. A message from the destination has to have been received before.
     * @param sender Sender FeatureAddressType of the sender. The entity and feature must be set. The device can be empty and will be filled automatically if so.
     * @param payload The payload to be sent.
     * @param cmd_classifier The command classifier of the message.
     * @param want_ack If we want an acknowledgement for the message. This is used to ensure that the peer received the message and can be used to detect if the peer is still alive.
     */
    bool send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, JsonVariantConst payload, CmdClassifierType cmd_classifier, bool want_ack = false);

    template <typename T> bool send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, T payload, CmdClassifierType cmd_classifier, const char *function_name, bool want_ack = false);

    /**
     * Get a SpineConnection for a given spine address.
     * @param spine_address
     * @return Pointer to the spine connection. nullptr if no connection exists.
     */
    static SpineConnection *get_spine_connection(const FeatureAddressType &spine_address);

    BasicJsonDocument<ArduinoJsonPsramAllocator> temporary_json_doc{SPINE_CONNECTION_MAX_JSON_SIZE}; // If a temporary doc is needed, use this one.
    BasicJsonDocument<ArduinoJsonPsramAllocator> response{SPINE_CONNECTION_MAX_JSON_SIZE};           // The response document to be filled with the response data

    NodeManagementEntity node_management{};
#ifdef EEBUS_ENABLE_EVSECC_USECASE
    EvseccUsecase evse_commissioning_and_configuration{};
    EvseccUsecase *evsecc = &evse_commissioning_and_configuration;
#endif
#ifdef EEBUS_ENABLE_EVCS_USECASE
    EvcsUsecase charging_summary{};
    EvcsUsecase *evcs = &charging_summary;
#endif
#ifdef EEBUS_ENABLE_LPC_USECASE
    LpcUsecase limitation_of_power_consumption{};
    LpcUsecase *lpc = &limitation_of_power_consumption;
#endif
#ifdef EEBUS_ENABLE_MPC_USECASE
    MpcUsecase monitoring_of_power_consumption{};
    MpcUsecase *mpc = &monitoring_of_power_consumption;
#endif
#ifdef EEBUS_ENABLE_EVCC_USECASE
    EvccUsecase ev_commissioning_and_configuration{};
    EvccUsecase *evcc = &ev_commissioning_and_configuration;
#endif
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    EvcemUsecase ev_charging_electricity_measurement{};
    EvcemUsecase *evcem = &ev_charging_electricity_measurement;
#endif
#ifdef EEBUS_ENABLE_CEVC_USECASE
    CevcUsecase coordinate_ev_charging{};
    CevcUsecase *cevc = &coordinate_ev_charging;
#endif
#ifdef EEBUS_ENABLE_OPEV_USECASE
    OpevUsecase overload_protection_by_ev_charging_current_curtailment{};
    OpevUsecase *opev = &overload_protection_by_ev_charging_current_curtailment;
#endif

    std::vector<EebusUsecase *> usecase_list{};

    EVSEEntity evse_entity{};
    EVEntity ev_entity{};
    EebusHeartBeat evse_heartbeat{};
    EebusHeartBeat ev_heartbeat{};

private:
    bool initialized = false;
    uint16_t eebus_commands_received = 0;
    uint16_t eebus_responses_sent = 0;
};
