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

// Include eebus.h first to get the EEBUS_MODE_* macros (which define EEBUS_ENABLE_* in eebus_usecases.h)
#include "../eebus.h"
#include "../eebus_usecases.h"

#if defined(EEBUS_ENABLE_LPC_USECASE) || defined(EEBUS_ENABLE_LPP_USECASE)

#include "../module_dependencies.h"
#include "entity_data.h"
#include "event_log_prefix.h"
#include "loadcontrol.h"
#include "usecase_helpers.h"

// ==============================================================================
// LoadPowerLimitUsecase - Base class for LPC and LPP usecases
// ==============================================================================

LoadPowerLimitUsecase::~LoadPowerLimitUsecase()
{
    task_scheduler.cancel(limit_endtime_timer);
    task_scheduler.cancel(failsafe_expiry_timer);
}

// Base class constructor - initializes IDs from config offsets
LoadPowerLimitUsecase::LoadPowerLimitUsecase(const LoadPowerLimitConfig &config) :
    config_(config), id_l_1(config.loadcontrol_limit_id_offset + 1), id_m_1(config.measurement_id_offset + 1), id_k_1(config.device_config_key_id_offset + 1), id_k_2(config.device_config_key_id_offset + 2), id_ec_1(config.electrical_connection_id_offset + 1), id_cc_1(config.electrical_connection_characteristic_id_offset + 1), id_cc_2(config.electrical_connection_characteristic_id_offset + 2), id_p_1(config.electrical_connection_parameter_id_offset + 1), limit_description_id(id_l_1),
    limit_measurement_description_id(id_m_1), failsafe_power_key_id(id_k_1), failsafe_duration_key_id(id_k_2)
{
    task_scheduler.scheduleOnce(
        [this]() {
            // Register for heartbeat (Scenario 3)
            eebus.usecases->evse_heartbeat.register_usecase_for_heartbeat(this);
            eebus.usecases->evse_heartbeat.set_autosubscribe(true);
            update_state(); // Initialize state machine
            update_api();
        },
        1_s); // Schedule all the init stuff a bit delayed to allow other entities to initialize first
}

UseCaseInformationDataType LoadPowerLimitUsecase::get_usecase_information()
{
    UseCaseInformationDataType usecase;
    usecase.actor = "ControllableSystem";

    UseCaseSupportType usecase_support;
    usecase_support.useCaseName = config_.usecase_name;
    usecase_support.useCaseVersion = "1.0.0";
    usecase_support.scenarioSupport->push_back(1);
    usecase_support.scenarioSupport->push_back(2);
    usecase_support.scenarioSupport->push_back(3);
    usecase_support.scenarioSupport->push_back(4);
    usecase_support.useCaseDocumentSubRevision = "release";
    usecase.useCaseSupport->push_back(usecase_support);

    FeatureAddressType usecase_feature_address;
    usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    usecase_feature_address.entity = entity_address;
    usecase.address = usecase_feature_address;
    return usecase;
}

MessageReturn LoadPowerLimitUsecase::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    switch (get_feature_by_address(header.addressDestination->feature.get())) {
        case FeatureTypeEnumType::LoadControl:
            return load_control_feature(header, data, response);
        case FeatureTypeEnumType::DeviceConfiguration:
            return deviceConfiguration_feature(header, data, response);
        case FeatureTypeEnumType::ElectricalConnection:
            return electricalConnection_feature(header, data, response);
        default:;
    }
    return {false};
}

NodeManagementDetailedDiscoveryEntityInformationType LoadPowerLimitUsecase::get_detailed_discovery_entity_information() const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
#ifndef EEBUS_MODE_EM
    entity.description->entityType = EntityTypeEnumType::EVSE;
#else
    entity.description->entityType = EntityTypeEnumType::CEM;
#endif
    entity.description->label = "Controllable System";
    return entity;
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> LoadPowerLimitUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    // LoadControl Feature
    NodeManagementDetailedDiscoveryFeatureInformationType loadControlFeature{};
    loadControlFeature.description->featureAddress->entity = entity_address;
    loadControlFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::LoadControl);
    loadControlFeature.description->featureType = FeatureTypeEnumType::LoadControl;
    loadControlFeature.description->role = RoleType::server;

    FunctionPropertyType loadControlDescriptionList{};
    loadControlDescriptionList.function = FunctionEnumType::loadControlLimitDescriptionListData;
    loadControlDescriptionList.possibleOperations->read = PossibleOperationsReadType{};
    loadControlFeature.description->supportedFunction->push_back(loadControlDescriptionList);

    FunctionPropertyType loadControlLimitListData{};
    loadControlLimitListData.function = FunctionEnumType::loadControlLimitListData;
    loadControlLimitListData.possibleOperations->read = PossibleOperationsReadType{};
    loadControlLimitListData.possibleOperations->write = PossibleOperationsWriteType{};
    loadControlLimitListData.possibleOperations->write->partial = ElementTagType{};
    loadControlFeature.description->supportedFunction->push_back(loadControlLimitListData);
    features.push_back(loadControlFeature);

    // DeviceConfiguration Feature
    NodeManagementDetailedDiscoveryFeatureInformationType deviceConfigurationFeature{};
    deviceConfigurationFeature.description->featureAddress->entity = entity_address;
    deviceConfigurationFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration);
    deviceConfigurationFeature.description->featureType = FeatureTypeEnumType::DeviceConfiguration;
    deviceConfigurationFeature.description->role = RoleType::server;

    FunctionPropertyType deviceConfigurationKeyValueDescriptionListData{};
    deviceConfigurationKeyValueDescriptionListData.function = FunctionEnumType::deviceConfigurationKeyValueDescriptionListData;
    deviceConfigurationKeyValueDescriptionListData.possibleOperations->read = PossibleOperationsReadType{};
    deviceConfigurationFeature.description->supportedFunction->push_back(deviceConfigurationKeyValueDescriptionListData);

    FunctionPropertyType deviceConfigurationKeyValueListData{};
    deviceConfigurationKeyValueListData.function = FunctionEnumType::deviceConfigurationKeyValueListData;
    deviceConfigurationKeyValueListData.possibleOperations->read = PossibleOperationsReadType{};
    deviceConfigurationKeyValueListData.possibleOperations->write = PossibleOperationsWriteType{};
    deviceConfigurationFeature.description->supportedFunction->push_back(deviceConfigurationKeyValueListData);
    features.push_back(deviceConfigurationFeature);

    // ElectricalConnection Feature
    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnectionFeature{};
    electricalConnectionFeature.description->featureAddress->entity = entity_address;
    electricalConnectionFeature.description->featureAddress->feature = feature_addresses.at(FeatureTypeEnumType::ElectricalConnection);
    electricalConnectionFeature.description->featureType = FeatureTypeEnumType::ElectricalConnection;
    electricalConnectionFeature.description->role = RoleType::server;

    FunctionPropertyType electricalConnectionCharacteristicsListData{};
    electricalConnectionCharacteristicsListData.function = FunctionEnumType::electricalConnectionCharacteristicListData;
    electricalConnectionCharacteristicsListData.possibleOperations->read = PossibleOperationsReadType{};
    electricalConnectionFeature.description->supportedFunction->push_back(electricalConnectionCharacteristicsListData);
    features.push_back(electricalConnectionFeature);

    return features;
}

// LoadControl feature handler
MessageReturn LoadPowerLimitUsecase::load_control_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    if (header.cmdClassifier == CmdClassifierType::read) {
        if (data->last_cmd == SpineDataTypeHandler::Function::loadControlLimitDescriptionListData) {
            response["loadControlLimitDescriptionListData"] = EVSEEntity::get_load_control_limit_description_list_data();
            return {true, true, CmdClassifierType::reply};
        }
        if (data->last_cmd == SpineDataTypeHandler::Function::loadControlLimitListData) {
            response["loadControlLimitListData"] = EVSEEntity::get_load_control_limit_list_data();
            return {true, true, CmdClassifierType::reply};
        }
    }
    if (header.cmdClassifier == CmdClassifierType::write) {
        FeatureAddressType feature_address{};
        feature_address.entity = entity_address;
        feature_address.feature = feature_addresses.at(FeatureTypeEnumType::LoadControl);
        feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
        bool is_bound = eebus.usecases->node_management.check_is_bound(header.addressSource.get(), feature_address);
        if (!is_bound) {
            eebus.trace_fmtln("Received write from an unbound node");
            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::BindingRequired, "Load Control requires binding");
            return {true, true, CmdClassifierType::result};
        }
        switch (data->last_cmd) {
            case SpineDataTypeHandler::Function::loadControlLimitListData:
                if (data->loadcontrollimitlistdatatype.has_value() && !data->loadcontrollimitlistdatatype->loadControlLimitData->empty()) {
                    for (LoadControlLimitDataType load_control_limit_data : data->loadcontrollimitlistdatatype->loadControlLimitData.get()) {
                        if (load_control_limit_data.limitId == id_l_1) {
                            bool limit_enabled = load_control_limit_data.isLimitActive.get();
                            const int new_limit_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(*load_control_limit_data.value);
                            const seconds_t duration_s = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(load_control_limit_data.timePeriod->endTime.get());
                            logger.printfln("Received a Loadcontrol Limit for a %s usecase. Attempting to apply limit. Limit is: %d W, duration: %d s, enabled: %d", get_usecases_name(config_.usecase_type), new_limit_w, duration_s.as<int>(), limit_enabled);
                            if (!update_limit(limit_enabled, new_limit_w, duration_s)) {
                                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Limit not accepted");
                                logger.printfln("Limit not accepted");
                                return {true, true, CmdClassifierType::result};
                            }
                            logger.printfln("Limit accepted");
                            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
                            return {true, true, CmdClassifierType::result};
                        }
                    }
                    return {false};
                }
                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Limit not accepted or invalid data");
                break;
            default:
                EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Unknown command");
        }
        return {true, true, CmdClassifierType::result};
    }
    return {false};
}

// DeviceConfiguration feature handler
MessageReturn LoadPowerLimitUsecase::deviceConfiguration_feature(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    switch (data->last_cmd) {
        case SpineDataTypeHandler::Function::deviceConfigurationKeyValueDescriptionListData:
            switch (header.cmdClassifier.get()) {
                case CmdClassifierType::read: {
                    response["deviceConfigurationKeyValueDescriptionListData"] = EVSEEntity::get_device_configuration_list_data();
                    return {true, true, CmdClassifierType::reply};
                }
                default:
                    EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandNotSupported, "This cmdclassifier is not supported on this function");
                    return {true, true, CmdClassifierType::result};
            }
        case SpineDataTypeHandler::Function::deviceConfigurationKeyValueListData:
            switch (header.cmdClassifier.get()) {
                case CmdClassifierType::read: {
                    response["deviceConfigurationKeyValueListData"] = EVSEEntity::get_device_configuration_value_list_data();
                    return {true, true, CmdClassifierType::reply};
                }
                case CmdClassifierType::write:
                    if (eebus.usecases->node_management.check_is_bound(header.addressSource.get(), header.addressDestination.get())) {
                        auto new_config = data->deviceconfigurationkeyvaluelistdatatype.get();
                        int new_failsafe_power = -1;
                        seconds_t new_failsafe_duration = -1_s;
                        for (const auto &list_entry : new_config.deviceConfigurationKeyValueData.get()) {
                            if (list_entry.keyId == failsafe_power_key_id) {
                                new_failsafe_power = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(*list_entry.value->scaledNumber);
                            } else if (list_entry.keyId == failsafe_duration_key_id) {
                                new_failsafe_duration = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(list_entry.value->duration.get());
                            }
                        }
                        update_failsafe(new_failsafe_power, new_failsafe_duration);
                        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "Configuration updated successfully");
                        return {true, true, CmdClassifierType::result};
                    }
                    EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::BindingRequired, "DeviceConfiguration requires binding");
                    return {true, true, CmdClassifierType::result};

                default:
                    EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandNotSupported, "This cmdclassifier is not supported on this function");
                    return {true, true, CmdClassifierType::result};
            }
        default:
            return {false};
    }

    return {false};
}

// ElectricalConnection feature handler
MessageReturn LoadPowerLimitUsecase::electricalConnection_feature(const HeaderType &header, const SpineDataTypeHandler *data, JsonObject response)
{
    if (header.cmdClassifier == CmdClassifierType::read && data->last_cmd == SpineDataTypeHandler::Function::electricalConnectionCharacteristicListData) {
        response["electricalConnectionCharacteristicListData"] = EVSEEntity::get_electrical_connection_characteristic_list_data();
        return {true, true, CmdClassifierType::reply};
    }
    return {false};
}

void LoadPowerLimitUsecase::update_failsafe(int power_limit_w, seconds_t duration)
{
    if (power_limit_w > -1) {
        failsafe_power_limit_w = power_limit_w;
    }
    if (duration > -1_s) {
        failsafe_duration = duration;
    }
    if (power_limit_w > 0 || duration > 0_s) {
        logger.printfln("Updated failsafe to %d W for %d seconds", failsafe_power_limit_w, failsafe_duration.as<int>());
        update_state();
        update_api();
        auto data = EVSEEntity::get_device_configuration_value_list_data();
        eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration), data, "deviceConfigurationKeyValueListData");
    }
}

void LoadPowerLimitUsecase::update_constraints(int power_max, int power_contract_max)
{
    if (power_max > 0) {
        power_max_w = power_max;
    }
    if (power_contract_max > 0) {
        power_contract_max_w = power_contract_max;
    }
    auto data = EVSEEntity::get_electrical_connection_characteristic_list_data();
    eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::ElectricalConnection), data, "electricalConnectionCharacteristicListData");
}

bool LoadPowerLimitUsecase::update_limit(bool limit, int current_limit_w, const seconds_t duration)
{
    // For LPC: limit_received when power > 0; for LPP: when power < 0
    if (config_.limit_is_positive) {
        limit_received = current_limit_w > 0 || limit_received;
    } else {
        limit_received = current_limit_w < 0 || limit_received;
    }

    // Evaluate if the limit can be applied
    if (duration <= 0_s && !limit_active && limit_received && limit) {
        // In case the duration is 0 (meaning until further notice) and the limit is not active, reject the limit
        limit_active = false;
        return false;
    }
    limit_active = limit;

    // For LPC: reject limit < 0; for LPP: reject limit > 0
    if (config_.limit_is_positive) {
        if (current_limit_w < 0) {
            limit_active = false;
            return false;
        }
    } else {
        if (current_limit_w > 0) {
            limit_active = false;
            return false;
        }
    }
    // TODO: Add more limit validation here.
    // For example: Can this limit be applied electrically.

    configured_limit = current_limit_w;

    if (duration > 0_s && limit_active) {
        limit_expired = false;
        timeval time_v{};
        rtc.clock_synced(&time_v);
        limit_endtime = time_v.tv_sec + duration.as<int>();
        task_scheduler.cancel(limit_endtime_timer);
        limit_endtime_timer = task_scheduler.scheduleOnce(
            [this]() {
                if (state_ == LoadcontrolState::Limited) {
                    logger.printfln("Limit duration expired");
                    limit_expired = true;
                    update_state();
                    update_api();
                }
            },
            duration);
    } else if (duration == 0_s && limit_active) {
        // A value of 0 means the limit is valid until further notice
    }
    update_state();
    update_api();

    LoadControlLimitListDataType data = EVSEEntity::get_load_control_limit_list_data();
    eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::LoadControl), data, "loadControlLimitListData");
    return true;
}

void LoadPowerLimitUsecase::update_state()
{
    eebus.trace_fmtln("Updating state. Current state: %s. Heartbeat received: %d, Limit received: %d, Limit active: %d, Limit expired: %d", get_loadcontrol_state_name(state_), heartbeat_received, limit_received, limit_active, limit_expired);

    switch (state_) {
        case LoadcontrolState::Startup:
            init_state();
            break;
        case LoadcontrolState::Init:
            if (heartbeat_received && limit_received && !limit_active) {
                unlimited_controlled_state();
            } else if (heartbeat_received && limit_received && limit_active) {
                limited_state();
            } else {
                unlimited_autonomous_state();
            }
            break;
        case LoadcontrolState::UnlimitedControlled:
            if (heartbeat_received && limit_received && limit_active) {
                limited_state();
            } else if (!heartbeat_received) {
                failsafe_state();
            }
            break;
        case LoadcontrolState::Limited:
            if (!heartbeat_received) {
                failsafe_state();
            } else if (limit_expired || (limit_received && !limit_active)) {
                unlimited_controlled_state();
            } else if (state_ == LoadcontrolState::Limited) {
                limited_state();
            }
            break;
        case LoadcontrolState::Failsafe:
            if (heartbeat_received && limit_received && !limit_active) {
                unlimited_controlled_state();
            } else if (heartbeat_received && limit_received && limit_active) {
                limited_state();
            } else if (failsafe_expired) {
                unlimited_autonomous_state();
            }
            break;
        case LoadcontrolState::UnlimitedAutonomous:
            if (heartbeat_received && limit_received && !limit_active) {
                unlimited_controlled_state();
            } else if (heartbeat_received && limit_received && limit_active) {
                limited_state();
            }
            break;
    }
}

void LoadPowerLimitUsecase::receive_heartbeat()
{
    heartbeat_received = true;
    update_state();
    update_api();
}

void LoadPowerLimitUsecase::receive_heartbeat_timeout()
{
    heartbeat_received = false;
    logger.printfln("No Heartbeat received from control box. Switching to failsafe or unlimited/autonomous mode");
    update_state();
    update_api();
}

void LoadPowerLimitUsecase::inform_spineconnection_usecase_update(SpineConnection *conn)
{
    auto peers = conn->get_address_of_feature(FeatureTypeEnumType::DeviceDiagnosis, RoleType::client, config_.usecase_name, "EnergyGuard");
    for (FeatureAddressType &peer : peers) {
        eebus.usecases->evse_heartbeat.initialize_heartbeat_on_feature(peer, config_.usecase_type, true);
    }
}

void LoadPowerLimitUsecase::init_state()
{
    limit_active = false;
    current_active_limit_w = failsafe_power_limit_w;
    state_ = LoadcontrolState::Init;
}

void LoadPowerLimitUsecase::unlimited_controlled_state()
{
    state_ = LoadcontrolState::UnlimitedControlled;
    limit_active = false;
    current_active_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
}

void LoadPowerLimitUsecase::limited_state()
{
    timeval time_v{};
    rtc.clock_synced(&time_v);
    long long duration_left = limit_endtime - time_v.tv_sec;
    if (state_ != LoadcontrolState::Limited) {
        logger.printfln("Received a limit of %d W valid for %lld s", configured_limit, duration_left);
    } else if (current_active_limit_w != configured_limit) {
        logger.printfln("Updating limit to %d W", configured_limit);
    }

    state_ = LoadcontrolState::Limited;
    current_active_limit_w = configured_limit;
    limit_active = true;
}

void LoadPowerLimitUsecase::failsafe_state()
{
    state_ = LoadcontrolState::Failsafe;
    limit_active = false;

    current_active_limit_w = failsafe_power_limit_w;
    task_scheduler.cancel(limit_endtime_timer);

    timeval time_v{};
    rtc.clock_synced(&time_v);
    failsafe_expiry_endtime = time_v.tv_sec + failsafe_duration.as<int>();
    failsafe_expiry_timer = task_scheduler.scheduleOnce(
        [this]() {
            if (state_ == LoadcontrolState::Failsafe) {
                logger.printfln("Failsafe duration expired. Switching to autonomous/unlimited mode");
                failsafe_expired = true;
                update_state();
                update_api();
            }
        },
        failsafe_duration);
}

void LoadPowerLimitUsecase::unlimited_autonomous_state()
{
    state_ = LoadcontrolState::UnlimitedAutonomous;
    limit_active = false;
    current_active_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
}

void LoadPowerLimitUsecase::update_api() const
{
    auto api_entry = eebus.eebus_usecase_state.get(config_.api_key);
    api_entry->get("usecase_state")->updateEnum(state_);
    api_entry->get("limit_active")->updateBool(limit_active);
    // For LPP, we use abs() to display the limit as positive in the UI
    api_entry->get("current_limit")->updateUint(config_.limit_is_positive ? current_active_limit_w : abs(current_active_limit_w));
    api_entry->get("failsafe_limit_power_w")->updateUint(failsafe_power_limit_w);
    api_entry->get("failsafe_limit_duration_s")->updateUint(failsafe_duration.as<uint32_t>());

    timeval now{};
    rtc.clock_synced(&now);
    if (state_ == LoadcontrolState::Limited) {
        const long long duration_left = limit_endtime - now.tv_sec;
        api_entry->get("outstanding_duration_s")->updateUint(duration_left > 0 ? duration_left : 0);
    } else if (state_ == LoadcontrolState::Failsafe) {
        const long long failsafe_time_left = failsafe_expiry_endtime - now.tv_sec;
        api_entry->get("outstanding_duration_s")->updateUint(failsafe_time_left > 0 ? failsafe_time_left : 0);
    } else {
        api_entry->get("outstanding_duration_s")->updateUint(0);
    }

    api_entry->get("constraints_power_maximum")->updateUint(power_max_w);
}

void LoadPowerLimitUsecase::get_loadcontrol_limit_description(LoadControlLimitDescriptionListDataType *data) const
{
    LoadControlLimitDescriptionDataType limit_description{};
    limit_description.limitId = limit_description_id;
    limit_description.limitType = LoadControlLimitTypeEnumType::signDependentAbsValueLimit;
    limit_description.limitCategory = LoadControlCategoryEnumType::obligation;
    limit_description.limitDirection = config_.energy_direction;
    limit_description.measurementId = limit_measurement_description_id;
    limit_description.unit = UnitOfMeasurementEnumType::W;
    limit_description.scopeType = ScopeTypeEnumType::activePowerLimit;
    data->loadControlLimitDescriptionData->push_back(limit_description);
}

void LoadPowerLimitUsecase::get_loadcontrol_limit_list(LoadControlLimitListDataType *data) const
{
    timeval now{};
    rtc.clock_synced(&now);
    const long long duration_left = limit_endtime - now.tv_sec;

    LoadControlLimitDataType limit_data{};
    limit_data.limitId = limit_description_id;
    limit_data.isLimitChangeable = !limit_fixed;
    limit_data.isLimitActive = limit_active;
    if (duration_left > 0) {
        limit_data.timePeriod->endTime = EEBUS_USECASE_HELPERS::iso_duration_to_string(seconds_t(duration_left));
    }
    limit_data.value->number = current_active_limit_w;
    limit_data.value->scale = 0;
    data->loadControlLimitData->push_back(limit_data);
}

void LoadPowerLimitUsecase::get_device_configuration_value(DeviceConfigurationKeyValueListDataType *data) const
{
    DeviceConfigurationKeyValueDataType failsafe_power_key_value{};
    failsafe_power_key_value.isValueChangeable = true;
    failsafe_power_key_value.keyId = failsafe_power_key_id;
    failsafe_power_key_value.value->scaledNumber->number = failsafe_power_limit_w;
    failsafe_power_key_value.value->scaledNumber->scale = 0;
    data->deviceConfigurationKeyValueData->push_back(failsafe_power_key_value);

    DeviceConfigurationKeyValueDataType failsafe_duration_key_value{};
    failsafe_duration_key_value.isValueChangeable = true;
    failsafe_duration_key_value.keyId = failsafe_duration_key_id;
    failsafe_duration_key_value.value->duration = EEBUS_USECASE_HELPERS::iso_duration_to_string(failsafe_duration);
    data->deviceConfigurationKeyValueData->push_back(failsafe_duration_key_value);
}

void LoadPowerLimitUsecase::get_device_configuration_description(DeviceConfigurationKeyValueDescriptionListDataType *data) const
{
    DeviceConfigurationKeyValueDescriptionDataType failsafe_power_description{};
    failsafe_power_description.keyId = failsafe_power_key_id;
    failsafe_power_description.keyName = config_.failsafe_key_name;
    failsafe_power_description.unit = UnitOfMeasurementEnumType::W;
    failsafe_power_description.valueType = DeviceConfigurationKeyValueTypeType::scaledNumber;
    data->deviceConfigurationKeyValueDescriptionData->push_back(failsafe_power_description);

    DeviceConfigurationKeyValueDescriptionDataType failsafe_duration_description{};
    failsafe_duration_description.keyId = failsafe_duration_key_id;
    failsafe_duration_description.keyName = DeviceConfigurationKeyNameEnumType::failsafeDurationMinimum;
    failsafe_duration_description.valueType = DeviceConfigurationKeyValueTypeType::duration;
    data->deviceConfigurationKeyValueDescriptionData->push_back(failsafe_duration_description);
}

void LoadPowerLimitUsecase::get_electrical_connection_characteristic(ElectricalConnectionCharacteristicListDataType *data) const
{
    ElectricalConnectionCharacteristicDataType power_max{};
    power_max.electricalConnectionId = id_ec_1;
    power_max.parameterId = id_p_1;
    power_max.characteristicId = id_cc_1;
    power_max.characteristicContext = ElectricalConnectionCharacteristicContextEnumType::entity;
    power_max.characteristicType = config_.characteristic_type;
    power_max.value->number = power_max_w;
    power_max.unit = UnitOfMeasurementEnumType::W;
    data->electricalConnectionCharacteristicData->push_back(power_max);
}

#endif // defined(EEBUS_ENABLE_LPC_USECASE) || defined(EEBUS_ENABLE_LPP_USECASE)

// =============================================================================
// LpcUsecase - Thin wrapper around LoadPowerLimitUsecase
// =============================================================================
#ifdef EEBUS_ENABLE_LPC_USECASE

// Static configuration for LPC (Limitation of Power Consumption)
const LoadPowerLimitConfig LpcUsecase::lpc_config = {
    .usecase_type = Usecases::LPC,
    .usecase_name = "limitationOfPowerConsumption",
    .api_key = "power_consumption_limitation",
    .energy_direction = EnergyDirectionEnumType::consume,
    .characteristic_type = ElectricalConnectionCharacteristicTypeEnumType::powerConsumptionMax,
    .failsafe_key_name = DeviceConfigurationKeyNameEnumType::failsafeConsumptionActivePowerLimit,
    .limit_is_positive = true,
    .loadcontrol_limit_id_offset = EVSEEntity::lpcLoadcontrolLimitIdOffset,
    .measurement_id_offset = EVSEEntity::lpcMeasurementIdOffset,
    .device_config_key_id_offset = EVSEEntity::lpcDeviceConfigurationKeyIdOffset,
    .electrical_connection_id_offset = EVSEEntity::lpcElectricalConnectionIdOffset,
    .electrical_connection_characteristic_id_offset = EVSEEntity::lpcElectricalConnectionCharacteristicIdOffset,
    .electrical_connection_parameter_id_offset = EVSEEntity::lpcElectricalConnectionParameterIdOffset,
};

LpcUsecase::LpcUsecase() : LoadPowerLimitUsecase(lpc_config)
{
}

#endif // EEBUS_ENABLE_LPC_USECASE

// =============================================================================
// LppUsecase - Thin wrapper around LoadPowerLimitUsecase
// =============================================================================
#ifdef EEBUS_ENABLE_LPP_USECASE

// Static configuration for LPP (Limitation of Power Production)
const LoadPowerLimitConfig LppUsecase::lpp_config = {
    .usecase_type = Usecases::LPP,
    .usecase_name = "limitationOfPowerProduction",
    .api_key = "power_production_limitation",
    .energy_direction = EnergyDirectionEnumType::produce,
    .characteristic_type = ElectricalConnectionCharacteristicTypeEnumType::powerProductionMax,
    .failsafe_key_name = DeviceConfigurationKeyNameEnumType::failsafeProductionActivePowerLimit,
    .limit_is_positive = false,
    .loadcontrol_limit_id_offset = EVSEEntity::lppLoadcontrolLimitIdOffset,
    .measurement_id_offset = EVSEEntity::lppMeasurementIdOffset,
    .device_config_key_id_offset = EVSEEntity::lppDeviceConfigurationKeyIdOffset,
    .electrical_connection_id_offset = EVSEEntity::lppElectricalConnectionIdOffset,
    .electrical_connection_characteristic_id_offset = EVSEEntity::lppElectricalConnectionCharacteristicIdOffset,
    .electrical_connection_parameter_id_offset = EVSEEntity::lppElectricalConnectionParameterIdOffset,
};

LppUsecase::LppUsecase() : LoadPowerLimitUsecase(lpp_config)
{
}

#endif // EEBUS_ENABLE_LPP_USECASE
