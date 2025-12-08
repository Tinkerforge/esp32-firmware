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
EVSECC -> EVSE Commissioning and Configuration. Implemented according to EEBus_UC_TS_EVSECommissioningAndConfiguration_V1.0.0.pdf
NMC -> Node Management and Control. Implemented according to EEBUS_SPINE_TS_ProtocolSpecification.pdf, technically nodemanagement is not a usecase but it behaves like one in many ways and is therefore implemented alongside
EVCEM -> EV Charging Electricity Measurement. Implemented according to EEBUS_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf
Sometimes the following references are used e.g. LPC-905, these refer to rules laid out in the spec and can be found in the according technical spec.
*/

#pragma once

#include "build.h"

#include "config.h"
#include "lpc_state.enum.h"
#include "options.h"
#include "spine_connection.h"
#include "spine_types.h"

// Update this as usecases are enabled. 1 is always active and the nodemanagement Usecase
#define EEBUS_USECASES_ACTIVE 5

// Configuration related to the LPC usecases
// Disable if subscription functionalities shall not be used
#define EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS true
// The power consumption limit at startup in w. Should be the maximum limit of the Warp Charger. Is also used to tell the Energy Broker the maximum consumption limit of the device
#define EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION 22000
#define EEBUS_LPC_AWAIT_HEARTBEAT false

// Feature Addresses for the different features in the usecases. Addresses can repeat accross entities but we make sure they are unique for simplicity
/*enum FeatureAddresses : uint8_t {
    nodemgmt_feature_address = 0,
    chargingsummary_bill = 10,
    chargerate_measurement = 20,
    chargerate_electrical_connection,
    ev_device_configuration = 30,
    ev_identification,
    ev_device_classification,
    ev_electrical_connection,
    ev_device_diagnosis,
    evse_device_classification = 40,
    evse_device_diagnosis,
    lpc_loadcontrol = 50,
    lpc_device_configuration,
    lpc_device_diagnosis_server,
    lpc_device_diagnosis_client,
    lpc_electrical_connection,
    cevc_timeseries = 60,
    cevc_incentive_table,
};*/

class EEBusUseCases; // Forward declaration of EEBusUseCases

enum class UseCaseType : uint8_t { NodeManagement, ChargingSummary, LimitationOfActivePowerConsumption, CoordinatedEvCharging, EvCommissioningAndConfiguration, EvseCommissioningAndConfiguration, LimitationOfPowerProduction, EvChargingElectricityMeasurement, MonitoringOfPowerConsumption };

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
} // namespace EEBUS_USECASE_HELPERS

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

    [[nodiscard]] virtual UseCaseType get_usecase_type() const = 0;

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

    virtual String get_entity_name() const = 0;

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
    virtual CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) = 0;

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

protected:
    std::vector<int> entity_address{}; // The feature address of the usecase. This is used to identify the usecase in the NodeManagementUseCaseDataType.

    bool entity_active = true;                                             // If the entity is active or not. Inactive entities do not respond to messages or their entity and feature information should not be called.
    std::map<FeatureTypeEnumType, AddressFeatureType> feature_addresses{}; // The feature addresses of the features in this usecase.
    [[nodiscard]] FeatureTypeEnumType get_feature_by_address(AddressFeatureType feature_address) const;

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
    * @param connection The SPINE Connection that sent the message. This is used to send the response back to the correct connection and to identify the connection which bound or subscribed to a function.
    * @return true if a response was generated and needs to be sent, false if no response is needed.
    */
    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
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
    template <typename T> size_t inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, T data, const char *function_name);

    /**
     * Subscribe to a feature. Make sure the sending feature supports notify messages.
     * @param sending_feature The feature that is subscribing to the target feature.
     * @param target_feature The target feature to subscribe to.
     * @param feature The feature type of the target feature. Not mandatory.
     * @return If the subscription request was successful. Note: This only means the subscription request was valid and a destination was found, not that the target feature accepted the subscription.
     */
    bool subscribe_to_feature(FeatureAddressType &sending_feature, FeatureAddressType &target_feature, FeatureTypeEnumType feature = FeatureTypeEnumType::EnumUndefined) const;

    String get_entity_name() const override
    {
        return "NodeManagement";
    };

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

    CmdClassifierType handle_subscription(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);

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
    CmdClassifierType handle_binding(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);
};

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
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::ChargingSummary;
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

    [[nodiscard]] String get_entity_name() const override
    {
        return "EvcsUsecase";
    };
    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::Bill};
    }

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

    [[nodiscard]] BillListDataType get_bill_list_data() const;

    void update_api() const;
};

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

    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::EvChargingElectricityMeasurement;
    }

    [[nodiscard]] String get_entity_name() const override
    {
        return "EvcemUsecase";
    }

    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;
    UseCaseInformationDataType get_usecase_information() override;
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * Update the measurements. This will inform all subscribers of the new measurements.
     * @param amps_phase_1 Amps on phase a
     * @param amps_phase_2 Amps on phase b
     * @param amps_phase_3 Amps on phase c
     * @param power_phase_1 Total power on phase 1 in watts
     * @param power_phase_2 Total power on phase 2 in watts
     * @param power_phase_3 Total power on phase 3 in watts
     * @param charged_wh Total charged into the ev during the current session in wh
     * @param charged_measured If the charged_wh value is measured. If false it is assumed its calculated. Default is false
     */
    void update_measurements(int amps_phase_1, int amps_phase_2, int amps_phase_3, int power_phase_1, int power_phase_2, int power_phase_3, int charged_wh, bool charged_measured = false);

    /**
     * Update the constraints of the system. This will inform all subscribers of the new constraints. Values set to negative values will be omitted from the eebus information
     * @param amps_min Minimum amps in A that can be measured
     * @param amps_max Maximum amps in A that can be measured
     * @param amps_stepsize Stepsize of the amps in A measurement
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

private:
    // Data held about the current charge
    int amps_draw_phase[3]{};  // Amp draw per phase
    int power_draw_phase[3]{}; // Power per phase
    int power_charged_wh = 0;  // Total charged into the ev during the current session in wh
    bool power_charged_measured = false;

    // Constraints
    int measurement_limit_amps_min = 0;
    int measurement_limit_amps_max = 32;
    int measurement_limit_amps_stepsize = 1;
    int measurement_limit_power_min = 0;
    int measurement_limit_power_max = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    int measurement_limit_power_stepsize = 10;
    int measurement_limit_energy_min = 0;
    int measurement_limit_energy_max = 1000000;
    int measurement_limit_energy_stepsize = 10;

    // Generators for data types
    [[nodiscard]] MeasurementDescriptionListDataType generate_measurement_description() const;
    [[nodiscard]] MeasurementConstraintsListDataType generate_measurement_constraints() const;
    [[nodiscard]] MeasurementListDataType generate_measurement_list() const;

    [[nodiscard]] ElectricalConnectionDescriptionListDataType generate_electrical_connection_description() const;
    [[nodiscard]] ElectricalConnectionParameterDescriptionListDataType generate_electrical_connection_parameters() const;

    void update_api() const;
};

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

    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::EvCommissioningAndConfiguration;
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
    * @return true if a response was generated and needs to be sent, false if no response is needed.
    */
    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;
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

    [[nodiscard]] String get_entity_name() const override
    {
        return "EvccUsecase";
    };

    bool is_ev_connected() const
    {
        return ev_connected;
    }
    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::DeviceConfiguration, FeatureTypeEnumType::DeviceDiagnosis, FeatureTypeEnumType::Identification, FeatureTypeEnumType::DeviceClassification, FeatureTypeEnumType::ElectricalConnection};
    }

private:
    void update_api() const;
    bool ev_connected = false;

    // Server Data
    //DeviceDiagnosis
    CoolString communication_standard = "";
    bool asymmetric_supported = false;
    [[nodiscard]] DeviceConfigurationKeyValueDescriptionListDataType generate_device_config_description() const;
    [[nodiscard]] DeviceConfigurationKeyValueListDataType generate_device_config_list() const;
    //Identification
    IdentificationTypeEnumType mac_type = IdentificationTypeEnumType::eui64;
    CoolString mac_address = "";
    [[nodiscard]] IdentificationListDataType generate_identification_description() const;
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
    [[nodiscard]] DeviceClassificationManufacturerDataType generate_manufacturer_description() const;
    //ElectricalConnection
    uint32_t min_power_draw = 0;
    uint32_t max_power_draw = 0;
    uint32_t standby_power = 0;
    [[nodiscard]] ElectricalConnectionParameterDescriptionListDataType generate_electrical_connection_description() const;
    [[nodiscard]] ElectricalConnectionPermittedValueSetListDataType generate_electrical_connection_values() const;
    //DeviceDiagnosis
    bool standby_mode = false;
    [[nodiscard]] DeviceDiagnosisStateDataType generate_state() const;
};

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

    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::EvseCommissioningAndConfiguration;
    }

    [[nodiscard]] String get_entity_name() const override
    {
        return "EvseccUsecase";
    }

    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;
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

private:
    // Server Data
    DeviceDiagnosisOperatingStateEnumType operating_state = DeviceDiagnosisOperatingStateEnumType::normalOperation;
    std::string last_error_message;
    [[nodiscard]] DeviceDiagnosisStateDataType generate_state() const;
    static DeviceClassificationManufacturerDataType generate_manufacturer_description();

    void update_api() const;
};

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
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::LimitationOfActivePowerConsumption;
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
     * @param endtime Timestamp until which the limit shall be set
     * @return true if the limit is accepted and set.
     */
    bool update_lpc(bool limit, int current_limit_w, time_t endtime);

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
        return "LpcUsecase";
    }
    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::LoadControl, FeatureTypeEnumType::DeviceConfiguration, FeatureTypeEnumType::DeviceDiagnosis, FeatureTypeEnumType::ElectricalConnection, FeatureTypeEnumType::Generic};
    }

private:
    /**
     * The Load Control feature as required for Scenario 1 - Control active power consumption.
     * It is the primary feature of the entity and major part of the LPC Usecases.
     * Takes in a limit, if its enabled and a duration and attempts to apply it.
     * Requires a binding to write.
     * Supports subscriptions.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.1 and 3.4.1
     */
    CmdClassifierType load_control_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);

    /**
     * The Device Configuration feature as required for Scenario 2 - Failsafe Values.
     * Reports the current failsafe values and allows them to be updated.
     * A failsafe value is the maximum power that can be consumed if no contact to the energy guard can be established and always comes with a duration for which the failsafe will be active.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.2 and 3.4.2
     */
    CmdClassifierType deviceConfiguration_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);

    /**
     * The Device Diagnosis feature as required for Scenario 3 - Heartbeat.
     * Implements a heartbeat mechanism to ensure that the energy guard is still online and reachable, if not it enters the failsafe state.
     * If no heartbeat is received for a certain time, the system will switch to failsafe mode.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.3 and 3.4.3
     */
    CmdClassifierType device_diagnosis_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);

    /**
     * The Electrical Connection feature as required for Scenario 4 - Constraints.
     * Reports the current constraints of the system and is read only.
     * Constraints are the maximum power the system is capable of consuming.
     * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.4 and 3.4.4
     */
    CmdClassifierType electricalConnection_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);

    /**
    * The Device Diagnosis feature as required for Scenario 3 - Heartbeat.
    * Implements a heartbeat mechanism to ensure that the energy guard is still online and reachable, if not it enters the failsafe state.
    * If no heartbeat is received for a certain time, the system will switch to failsafe mode.
    * This is the client part of that scenario and will request heartbeats from the energy guard aswell as handle notify messages.
    * As described in EEBUS UC TS - EV Limitation Of Power Consumption V1.0.0. 2.6.3 and 3.4.3
    */
    CmdClassifierType generic_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);

    // State handling
    // State machine as described in LPC UC TS v1.0.0 2.3
    void update_state();
    void got_heartbeat(seconds_t timeout = 60_s);
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

    void update_api();

    // LoadControl configuration as required for scenario 1 - Control Active Power
    bool limit_received = false;
    int current_active_consumption_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    // While the limit is active, this shall be set to true
    bool limit_active = false;
    int configured_limit = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    // If the limit is changeable, this shall be set to false
    bool limit_fixed = false;
    // The description ID of the limit so its consistent across description and limit list data
    int limit_description_id = 1;
    int limit_measurement_description_id = 1;
    time_t limit_endtime = 0;
    LoadControlLimitDescriptionListDataType get_loadcontrol_limit_description() const;
    LoadControlLimitListDataType get_loadcontrol_limit_list() const;

    // Device Configuration Data as required for Scenario 2 - Failsafe values
    // TODO: switch these to type generating functions
    int failsafe_power_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    seconds_t failsafe_duration_min = 3600_s * 24_h; // Default to 24 hours
    DeviceConfigurationKeyValueListDataType get_device_configuration_value() const;
    DeviceConfigurationKeyValueDescriptionListDataType get_device_configuration_description() const;
    //DeviceConfigurationKeyValueListDataType device_configuration_key_value_list{};
    //DeviceConfigurationKeyValueDescriptionListDataType device_configuration_key_value_description_list{};

    // Heartbeat Data as required for Scenario 3 - Hearbeat
    bool heartbeatEnabled = false;
    uint64_t heartbeatCounter = 0;
    uint64_t heartbeat_timeout_task = 0;
    void broadcast_heartbeat();

    // Electrical Connection Data as required for Scenario 4 - Constraints
    // TODO: switch this to type generating functions
    ElectricalConnectionCharacteristicListDataType electrical_connection_characteristic_list{};
};

/**
 * The CevcUsecase as defined in EEBus UC TS - Coordinate EV Charging V1.0.1.
 * This should have the same entity address as other entities with the EV actor <br>
 * Actor: EV <br>
 * Features (Functions): TimeSeries (timeSeriesDescriptionListData, timeSeriesConstraintsListData, timeSeriesListData), IncentiveTable (incentiveTableDescriptionData, incentiveTableConstraintsData, incentiveTableData), DeviceDiagnosis (deviceDiagnosisHeartbeatData), ElectricalConnection (electricalConnectionCharacteristicListData)   <br>
*/

class CevcUsecase final : public EebusUsecase
{
public:
    CevcUsecase();

    [[nodiscard]] UseCaseType get_usecase_type() const override
    {
        return UseCaseType::CoordinatedEvCharging;
    }

    String get_entity_name() const override
    {
        return "CevcUsecase";
    }

    /**
    * \brief Handles a message for a usecase.
    * @param header SPINE header of the message. Contains information about the commandclassifier and the targeted entitiy.
    * @param data The actual Function call and data of the message.
    * @param response Where to write the response to. This is a JsonObject that should be filled with the response data.
    * @return true if a response was generated and needs to be sent, false if no response is needed.
    */
    CmdClassifierType handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    /**
    * Builds and returns the UseCaseInformationDataType as defined in EEBus UC TS - EV Commissioning and Configuration V1.0.0. 3.1.2.
    * @return
    */
    UseCaseInformationDataType get_usecase_information() override;
    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * A charging Plan entry as needed to communicate the EV charging demands.
     */
    struct ChargingPlanEntry {
        seconds_t duration;
        int max_power_expected;
        int min_power_expected;
        int opt_power_wanted;
    };

    /**
     * A PowerLimit entry as needed to communicate the power limits set by the grid operator or energy manager.
     */
    using PowerLimitEntry = ChargingPlanEntry;

    /**
     * The Incentive Slot entry as needed to communicate the incentive slots to the EV.
     */
    struct IncentiveSlotEntry {
        /**
         * The tier of the incentive.
         */
        struct IncentiveTier {
            int lower_boundary_w;
            int upper_boundary_w;
            float incentive_value;
        };

        time_t start_time;
        time_t end_time;
        std::vector<IncentiveTier> tiers;
    };

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::TimeSeries, FeatureTypeEnumType::IncentiveTable};
    }

private:
    // Functions
    // Timeseries Feature
    [[nodiscard]] TimeSeriesDescriptionListDataType read_time_series_description() const;
    [[nodiscard]] TimeSeriesConstraintsListDataType read_time_series_constraints() const;
    [[nodiscard]] TimeSeriesListDataType read_time_series_list() const;
    CmdClassifierType write_time_series_list(HeaderType &header, SpineOptional<TimeSeriesListDataType> data, JsonObject response);
    // IncentiveTable Feature
    [[nodiscard]] IncentiveTableDescriptionDataType read_incentive_table_description() const;
    CmdClassifierType write_incentive_table_description(HeaderType &header, SpineOptional<IncentiveTableDescriptionDataType> data, JsonObject response);
    [[nodiscard]] IncentiveTableConstraintsDataType read_incentive_table_constraints() const;
    [[nodiscard]] IncentiveTableDataType read_incentive_table_data() const;
    CmdClassifierType write_incentive_table_data(HeaderType &header, SpineOptional<IncentiveTableDataType> data, JsonObject response);

    // Data held and transmitted
    time_t arrival_time{};
    time_t departure_time{};
    int minimum_energy_wh = 0;
    int maximum_energy_wh = 0;
    int optimal_energy_wh = 0;
    std::vector<ChargingPlanEntry> charging_plan{};
    std::vector<PowerLimitEntry> power_limits{};
    std::vector<IncentiveSlotEntry> incentives_available{};
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
#if OPTIONS_PRODUCT_ID_IS_WARP_ANY() == 1

    EvseccUsecase evse_commissioning_and_configuration{};
    EvcsUsecase charging_summary{};
    LpcUsecase limitation_of_power_consumption{};

    EvccUsecase ev_commissioning_and_configuration{};
    EvcemUsecase ev_charging_electricity_measurement{};
    CevcUsecase coordinate_ev_charging{};

    std::vector<EebusUsecase *> usecase_list{&node_management, &charging_summary, &limitation_of_power_consumption, &ev_commissioning_and_configuration, &evse_commissioning_and_configuration, &coordinate_ev_charging, &ev_charging_electricity_measurement};
#elif OPTIONS_PRODUCT_ID_IS_ENERGY_MANAGER() == 1
    // TODO: Implement the power production limitation usecase

    std::vector<EebusUsecase *> entity_list{&node_management, &limitation_of_power_production};
#endif

private:
    bool initialized = false;
    uint16_t eebus_commands_received = 0;
    uint16_t eebus_responses_sent = 0;
};
