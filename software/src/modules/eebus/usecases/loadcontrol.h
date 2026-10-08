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

#if defined(EEBUS_ENABLE_LPC_USECASE) || defined(EEBUS_ENABLE_LPP_USECASE)

/**
 * @brief Configuration struct for LoadPowerLimitUsecase parameterization.
 *
 * Used to differentiate between LPC (power consumption) and LPP (power production)
 * usecases. Both share the same base implementation but with different parameters.
 */
struct LoadPowerLimitConfig {
    Usecases usecase_type;                                              ///< LPC or LPP
    const char *usecase_name;                                           ///< "limitationOfPowerConsumption" or "limitationOfPowerProduction"
    const char *api_key;                                                ///< "lpc" or "lpp"
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
 * @brief Base class for Load Power Limit usecases (LPC and LPP).
 *
 * Implements the common logic for Limitation of Power Consumption and
 * Limitation of Power Production. Both usecases share nearly identical
 * structure and differ only in:
 * - Energy direction (consume vs produce)
 * - Limit validation logic (positive vs negative power values)
 * - API keys and usecase names
 * - ID offsets for SPINE data types
 *
 * The base class is parameterized via LoadPowerLimitConfig to handle
 * these differences.
 *
 * Scenarios (both LPC and LPP):
 *   1: Control active power limit
 *   2: Failsafe values
 *   3: Heartbeat
 *   4: Constraints
 *
 * Actor: Controllable System
 * Entity address: {1}
 * Features:
 *   - LoadControl (loadControlLimitDescriptionListData, loadControlLimitListData)
 *   - DeviceConfiguration (deviceConfigurationKeyValueDescriptionListData, deviceConfigurationKeyValueListData)
 *   - DeviceDiagnosis (via heartbeat)
 *   - ElectricalConnection (electricalConnectionCharacteristicListData)
 *
 * @see EEBus_UC_TS_LimitationOfPowerConsumption_V1.0.0.pdf
 * @see EEBus_UC_TS_LimitationOfPowerProduction_V1.0.0.pdf
 */
class LoadPowerLimitUsecase : public EebusUsecase
{
public:
    explicit LoadPowerLimitUsecase(const LoadPowerLimitConfig &config);
    ~LoadPowerLimitUsecase() override;

    /**
     * @brief Handles a message for the usecase.
     * @param header SPINE header of the message
     * @param data The actual Function call and data of the message
     * @param response Where to write the response to
     * @return The MessageReturn object
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return config_.usecase_type;
    }

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * @brief A write command on the Active Power Limit.
     *
     * Elements that are not set keep their old value (partial write).
     */
    struct LimitWrite {
        SpineOptional<bool> active{};      ///< isLimitActive
        SpineOptional<int> value_w{};
        SpineOptional<seconds_t> duration; ///< timePeriod.endTime as duration from now
        bool delete_duration = false;      ///< The write contains a partial delete of timePeriod.endTime
    };

    /**
     * @brief Process a write on the Active Power Limit.
     * @param write The written data
     * @return true if the write was accepted (ACK), false if it was rejected (NACK)
     */
    bool update_limit(const LimitWrite &write);

    /**
     * @brief Process a write on the failsafe values.
     * @param power_limit_w The Failsafe Active Power Limit in W, if written
     * @param duration The Failsafe Duration Minimum, if written
     * @return true if the write was accepted (ACK), false if it was rejected (NACK)
     */
    bool update_failsafe(SpineOptional<int> power_limit_w, SpineOptional<seconds_t> duration);

    /**
     * @brief Update the constraints of the system.
     * @param power_max Maximum power consumption the system is capable of
     * @param power_contract_max Maximum power the contract allows
     */
    void update_constraints(int power_max = 0, int power_contract_max = 0);

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::LoadControl, FeatureTypeEnumType::DeviceConfiguration, FeatureTypeEnumType::ElectricalConnection};
    }

    // Data generators for SPINE data types
    void get_loadcontrol_limit_description(LoadControlLimitDescriptionListDataType *data) const;
    void get_loadcontrol_limit_list(LoadControlLimitListDataType *data) const;
    void get_device_configuration_value(DeviceConfigurationKeyValueListDataType *data) const;
    void get_device_configuration_description(DeviceConfigurationKeyValueDescriptionListDataType *data) const;
    void get_electrical_connection_characteristic(ElectricalConnectionCharacteristicListDataType *data) const;

    /**
     * @brief Whether the Active Power Limit written by the Energy Guard is activated.
     *
     * This is the limit data point of the use case. It is not set in failsafe state,
     * use is_power_limited() to check if the power has to be limited.
     */
    [[nodiscard]] bool limit_is_active() const
    {
        return limit_active;
    }

    /**
     * @brief Whether the power currently has to be limited to get_current_limit_w().
     *
     * True in state "limited", and in states "init" and "failsafe" if the
     * Failsafe Active Power Limit is below the nominal maximum power (LPC-901).
     */
    [[nodiscard]] bool is_power_limited() const;

    [[nodiscard]] int get_current_limit_w() const
    {
        return current_active_limit_w;
    }

    void receive_heartbeat() override;
    void receive_heartbeat_timeout() override;
    void inform_spineconnection_usecase_update(SpineConnection *conn) override;

protected:
    const LoadPowerLimitConfig &config_;

    // IDs computed from config offsets
    uint8_t id_l_1;   // LoadControl limit ID
    uint8_t id_m_1;   // Measurement ID
    uint16_t id_k_1;  // Device config key ID for failsafe power
    uint16_t id_k_2;  // Device config key ID for failsafe duration
    uint16_t id_ec_1; // Electrical connection ID
    uint16_t id_cc_1; // Electrical connection characteristic ID
    uint16_t id_cc_2; // Electrical connection characteristic ID 2
    uint16_t id_p_1;  // Electrical connection parameter ID

private:
    MessageReturn load_control_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);
    MessageReturn deviceConfiguration_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response);
    MessageReturn electricalConnection_feature(const HeaderType &header, const SpineDataTypeHandler *data, JsonObject response);

    // State handling (LPC/LPP 2.3)
    LoadcontrolState state_ = LoadcontrolState::Startup;
    bool heartbeat_received = false; ///< A heartbeat of the Energy Guard was received within the last 120 s
    micros_t last_heartbeat = 0_us;  ///< Time of the last heartbeat of the Energy Guard, 0 if none was received yet
    uint64_t init_timer = 0;

    /** @brief True in the states "unlimited/controlled" and "limited", i.e. an Energy Guard is in control. */
    [[nodiscard]] bool is_controlled() const
    {
        return state_ == LoadcontrolState::UnlimitedControlled || state_ == LoadcontrolState::Limited;
    }
    /** @brief True if a heartbeat of the Energy Guard was received within the last 60 seconds (LPC 2.2, IG-LPC 2.11). */
    [[nodiscard]] bool heartbeat_in_write_window() const;
    void limit_duration_expired();
    /** @brief Inform subscribers about the Active Power Limit data. Deferred, so the result of a write is sent first. */
    void notify_limit_subscribers();
    /** @brief Inform subscribers about the failsafe values. Deferred, so the result of a write is sent first. */
    void notify_failsafe_subscribers();

    void init_state();
    void unlimited_controlled_state();
    void limited_state();
    void failsafe_state();
    void unlimited_autonomous_state();

    void update_api() const;

    // LoadControl configuration
    bool limit_active = false;
    int current_active_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    int configured_limit = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    static constexpr bool limit_fixed = false;
    int limit_description_id;
    int limit_measurement_description_id;
    micros_t limit_endtime = 0_us; ///< End of the duration of the Active Power Limit (monotonic, now_us()), 0 if the limit has no duration
    uint64_t limit_endtime_timer = 0;

    // Device Configuration Data (Failsafe)
    int failsafe_power_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    seconds_t failsafe_duration = 2_h;
    uint64_t failsafe_expiry_timer = 0;
    micros_t failsafe_expiry_endtime = 0_us; ///< End of the Failsafe Duration Minimum in failsafe state (monotonic, now_us())
    uint8_t failsafe_power_key_id;
    uint8_t failsafe_duration_key_id;

    // Electrical Connection Data (Constraints)
    int power_max_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
    int power_contract_max_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
};

#endif // defined(EEBUS_ENABLE_LPC_USECASE) || defined(EEBUS_ENABLE_LPP_USECASE)

#ifdef EEBUS_ENABLE_LPC_USECASE

/**
 * @brief The LPC (Limitation of Power Consumption) use case.
 *
 * Implements EEBus UC TS - Limitation Of Power Consumption V1.0.0.
 * This is a thin wrapper around LoadPowerLimitUsecase with LPC-specific configuration.
 *
 * Actor: Controllable System
 * Entity address: {1}
 * Features: LoadControl, DeviceConfiguration, DeviceDiagnosis (via heartbeat), ElectricalConnection
 *
 * @see EEBus_UC_TS_LimitationOfPowerConsumption_V1.0.0.pdf
 */
class LpcUsecase final : public LoadPowerLimitUsecase
{
public:
    LpcUsecase();

    /**
     * @brief Update the constraints.
     *
     * Wrapper for base class.
     *
     * @param power_consumption_max Maximum power consumption
     * @param power_consumption_contract_max Maximum power per contract
     */
    void update_constraints(int power_consumption_max = 0, int power_consumption_contract_max = 0)
    {
        LoadPowerLimitUsecase::update_constraints(power_consumption_max, power_consumption_contract_max);
    }

    // Static IDs for external reference (e.g., MpcUsecase)
    static constexpr uint8_t id_l_1 = EVSEEntity::lpcLoadcontrolLimitIdOffset + 1;
    static constexpr uint8_t id_m_1 = EVSEEntity::lpcMeasurementIdOffset + 1;
    static constexpr uint16_t id_k_1 = EVSEEntity::lpcDeviceConfigurationKeyIdOffset + 1;
    static constexpr uint16_t id_k_2 = EVSEEntity::lpcDeviceConfigurationKeyIdOffset + 2;
    static constexpr uint16_t id_ec_1 = EVSEEntity::lpcElectricalConnectionIdOffset + 1;
    static constexpr uint16_t id_cc_1 = EVSEEntity::lpcElectricalConnectionCharacteristicIdOffset + 1;
    static constexpr uint16_t id_cc_2 = EVSEEntity::lpcElectricalConnectionCharacteristicIdOffset + 2;
    static constexpr uint16_t id_p_1 = EVSEEntity::lpcElectricalConnectionParameterIdOffset + 1;

private:
    static const LoadPowerLimitConfig lpc_config;
};

#endif // EEBUS_ENABLE_LPC_USECASE

#ifdef EEBUS_ENABLE_LPP_USECASE

/**
 * @brief The LPP (Limitation of Power Production) use case.
 *
 * Implements EEBus UC TS - Limitation Of Power Production V1.0.0.
 * This is a thin wrapper around LoadPowerLimitUsecase with LPP-specific configuration.
 *
 * Actor: Controllable System
 * Entity address: {1}
 * Features: LoadControl, DeviceConfiguration, DeviceDiagnosis (via heartbeat), ElectricalConnection
 *
 * @see EEBus_UC_TS_LimitationOfPowerProduction_V1.0.0.pdf
 */
class LppUsecase final : public LoadPowerLimitUsecase
{
public:
    LppUsecase();

    /**
     * @brief Update the constraints.
     *
     * Wrapper for base class.
     *
     * @param power_production_max Maximum power production
     * @param power_production_contract_max Maximum power per contract
     */
    void update_constraints(int power_production_max = 0, int power_production_contract_max = 0)
    {
        LoadPowerLimitUsecase::update_constraints(power_production_max, power_production_contract_max);
    }

    // Static IDs for external reference (e.g., MpcUsecase)
    static constexpr uint8_t id_l_1 = EVSEEntity::lppLoadcontrolLimitIdOffset + 1;
    static constexpr uint8_t id_m_1 = EVSEEntity::lppMeasurementIdOffset + 1;
    static constexpr uint16_t id_k_1 = EVSEEntity::lppDeviceConfigurationKeyIdOffset + 1;
    static constexpr uint16_t id_k_2 = EVSEEntity::lppDeviceConfigurationKeyIdOffset + 2;
    static constexpr uint16_t id_ec_1 = EVSEEntity::lppElectricalConnectionIdOffset + 1;
    static constexpr uint16_t id_cc_1 = EVSEEntity::lppElectricalConnectionCharacteristicIdOffset + 1;
    static constexpr uint16_t id_cc_2 = EVSEEntity::lppElectricalConnectionCharacteristicIdOffset + 2;
    static constexpr uint16_t id_p_1 = EVSEEntity::lppElectricalConnectionParameterIdOffset + 1;

private:
    static const LoadPowerLimitConfig lpp_config;
};

#endif // EEBUS_ENABLE_LPP_USECASE
