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

#include "../generated/module_dependencies.h"
#include "entity_data.h"
#include "event_log_prefix.h"
#include "loadcontrol.h"
#include "usecase_helpers.h"

// ==============================================================================
// LoadPowerLimitUsecase - Base class for LPC and LPP usecases
// ==============================================================================

// LPC-022, LPP-022: The Failsafe Duration Minimum is between 2 and 24 hours
static constexpr seconds_t FAILSAFE_DURATION_MIN = 2_h;
static constexpr seconds_t FAILSAFE_DURATION_MAX = 24_h;

LoadPowerLimitUsecase::~LoadPowerLimitUsecase()
{
    task_scheduler.cancel(limit_endtime_timer);
    task_scheduler.cancel(failsafe_expiry_timer);
    task_scheduler.cancel(init_timer);
    task_scheduler.cancel(heartbeat_timeout_timer);
}

// Base class constructor - initializes IDs from config offsets
LoadPowerLimitUsecase::LoadPowerLimitUsecase(const LoadPowerLimitConfig &config) :
    config_(config), id_l_1(config.loadcontrol_limit_id_offset + 1), id_m_1(config.measurement_id_offset + 1), id_k_1(config.device_config_key_id_offset + 1), id_k_2(config.device_config_key_id_offset + 2), id_ec_1(config.electrical_connection_id_offset + 1), id_cc_1(config.electrical_connection_characteristic_id_offset + 1), id_cc_2(config.electrical_connection_characteristic_id_offset + 2), id_p_1(config.electrical_connection_parameter_id_offset + 1), limit_description_id(id_l_1),
    limit_measurement_description_id(id_m_1), failsafe_power_key_id(id_k_1), failsafe_duration_key_id(id_k_2)
{
    load_persisted_failsafe();
    schedule_once_while_alive(
        [this]() {
            // Register for heartbeat (Scenario 3)
            eebus.usecases->evse_heartbeat.register_usecase_for_heartbeat(this);
            eebus.usecases->evse_heartbeat.set_autosubscribe(true);
            init_state(); // LPC-901: Restart of the Controllable System completed
            update_api();
        },
        1_s); // Schedule all the init stuff a bit delayed to allow other entities to initialize first
    usecase_actor = "ControllableSystem";
    usecase_name = config_.usecase_name;
    usecase_version = "1.0.0";
    supported_scenarios = {1, 2, 3, 4};
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
#ifdef EEBUS_MODE_EVSE
    return build_entity_info(EntityTypeEnumType::EVSE, "Controllable System");
#elifdef EEBUS_MODE_EM
    return build_entity_info(EntityTypeEnumType::CEM, "Controllable System");
#else
    return {};
#endif
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> LoadPowerLimitUsecase::get_detailed_discovery_feature_information() const
{
    std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> features;

    // LoadControl Feature
    NodeManagementDetailedDiscoveryFeatureInformationType loadControlFeature = build_feature_information(FeatureTypeEnumType::LoadControl);
    loadControlFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::loadControlLimitDescriptionListData));
    loadControlFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::loadControlLimitListData, true, true));
    features.push_back(loadControlFeature);

    // DeviceConfiguration Feature
    NodeManagementDetailedDiscoveryFeatureInformationType deviceConfigurationFeature = build_feature_information(FeatureTypeEnumType::DeviceConfiguration);
    deviceConfigurationFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::deviceConfigurationKeyValueDescriptionListData));
    // LPC/LPP Table 21: write and partial write are mandatory. Writes only change the keys they contain.
    deviceConfigurationFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::deviceConfigurationKeyValueListData, true, true));
    features.push_back(deviceConfigurationFeature);

    // ElectricalConnection Feature
    NodeManagementDetailedDiscoveryFeatureInformationType electricalConnectionFeature = build_feature_information(FeatureTypeEnumType::ElectricalConnection);
    electricalConnectionFeature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::electricalConnectionCharacteristicListData));
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
                    for (const LoadControlLimitDataType &load_control_limit_data : data->loadcontrollimitlistdatatype->loadControlLimitData.get()) {
                        if (load_control_limit_data.limitId != id_l_1) {
                            continue;
                        }
                        // Elements that are not part of the write keep their old value ("partial" write, LPC 3.4.1.4)
                        LimitWrite write{};
                        if (load_control_limit_data.isLimitActive.has_value()) {
                            write.active = load_control_limit_data.isLimitActive.get();
                        }
                        if (load_control_limit_data.value.has_value()) {
                            write.value_w = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(load_control_limit_data.value.get());
                        }
                        if (load_control_limit_data.timePeriod.has_value() && load_control_limit_data.timePeriod->endTime.has_value()) {
                            write.duration = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(load_control_limit_data.timePeriod->endTime.get());
                        }
                        // The Energy Guard removes the duration with a partial delete of the endTime (LPC 3.4.1.4)
                        if (SpineConnection *conn = EEBusUseCases::get_spine_connection(header.addressSource.get())) {
                            JsonArrayConst filters = conn->received_payload["filter"].as<JsonArrayConst>();
                            for (JsonVariantConst filter : filters) {
                                if (!filter["cmdControl"].containsKey("delete")) {
                                    continue;
                                }
                                JsonVariantConst selected_limit = filter["loadControlLimitListDataSelectors"]["limitId"];
                                if (!selected_limit.isNull() && selected_limit.as<int>() != id_l_1) {
                                    continue;
                                }
                                // Deleting the whole timePeriod (e.g. eebus-go) or only its endTime removes the duration
                                if (filter["loadControlLimitDataElements"].containsKey("timePeriod")) {
                                    write.delete_duration = true;
                                }
                            }
                        }
                        const String value_str = write.value_w.has_value() ? String(write.value_w.get()) + " W" : String("value unchanged");
                        const char *active_str = write.active.has_value() ? (write.active.get() ? "activated" : "deactivated") : "activation unchanged";
                        const String duration_str = write.duration.has_value() ? String(write.duration->as<int>()) + " s" : String(write.delete_duration ? "removed" : "unchanged");
                        logger.printfln("Received a %s limit: %s, %s, duration: %s", get_usecases_name(config_.usecase_type), value_str.c_str(), active_str, duration_str.c_str());
                        if (!update_limit(write)) {
                            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Limit not accepted");
                            logger.printfln("Limit rejected");
                            return {true, true, CmdClassifierType::result};
                        }
                        logger.printfln("Limit accepted");
                        EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError, "");
                        return {true, true, CmdClassifierType::result};
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
                        const auto &new_config = data->deviceconfigurationkeyvaluelistdatatype.get();
                        SpineOptional<int> new_failsafe_power{};
                        SpineOptional<seconds_t> new_failsafe_duration{};
                        bool found_key = false;
                        if (new_config.deviceConfigurationKeyValueData.has_value()) {
                            for (const auto &list_entry : new_config.deviceConfigurationKeyValueData.get()) {
                                if (list_entry.keyId == failsafe_power_key_id) {
                                    found_key = true;
                                    if (list_entry.value.has_value() && list_entry.value->scaledNumber.has_value()) {
                                        new_failsafe_power = EEBUS_USECASE_HELPERS::scaled_numbertype_to_int(list_entry.value->scaledNumber.get());
                                    }
                                } else if (list_entry.keyId == failsafe_duration_key_id) {
                                    found_key = true;
                                    if (list_entry.value.has_value() && list_entry.value->duration.has_value()) {
                                        new_failsafe_duration = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(list_entry.value->duration.get());
                                    }
                                }
                            }
                        }
                        if (!found_key) {
                            // The DeviceConfiguration feature might be shared with other usecases using different keys
                            return {false};
                        }
                        if (!update_failsafe(new_failsafe_power, new_failsafe_duration)) {
                            EEBUS_USECASE_HELPERS::build_result_data(response, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Failsafe values not accepted");
                            return {true, true, CmdClassifierType::result};
                        }
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

// LPC 2.2, IG-LPC 2.11: In "init", "failsafe" and "unlimited/autonomous" a write on the limit is only evaluated if it follows a heartbeat within 60 seconds
static constexpr seconds_t HEARTBEAT_WRITE_WINDOW = 60_s;
// LPC-906: Leave "init" if no Energy Guard took control within 120 seconds
static constexpr seconds_t INIT_TIMEOUT = 120_s;
// LPC-911, LPC-912: Switch to failsafe if no heartbeat of the Energy Guard was received for 120 seconds
static constexpr seconds_t HEARTBEAT_TIMEOUT = 120_s;

// End times are monotonic, so a wall clock change (e.g. the first NTP sync) does not change remaining durations.
// Returns the seconds left until end, rounded up. 0 if there is no end time or it elapsed.
static seconds_t seconds_left(micros_t end)
{
    if (end == 0_us) {
        return 0_s;
    }
    const int64_t left_us = (end - now_us()).as<int64_t>();
    if (left_us <= 0) {
        return 0_s;
    }
    return seconds_t{(left_us + 999999) / 1000000};
}

bool LoadPowerLimitUsecase::update_failsafe(SpineOptional<int> power_limit_w, SpineOptional<seconds_t> duration)
{
    const char *name = get_usecases_name(config_.usecase_type);
    // IG-LPC 2.11: Writes on the failsafe values are only evaluated after the Energy Guard sent a heartbeat followed by a write on the limit.
    if (!is_controlled()) {
        logger.printfln("Rejected %s failsafe values: The Energy Guard has to send a heartbeat and a limit first", name);
        return false;
    }
    // IG-LPC 3.6: The Failsafe Consumption Active Power Limit is >= 0 W
    if (power_limit_w.has_value() && config_.limit_is_positive && power_limit_w.get() < 0) {
        logger.printfln("Rejected %s failsafe values: Failsafe limit of %d W is out of range", name, power_limit_w.get());
        return false;
    }
    if (duration.has_value()) {
        if (duration.get() > FAILSAFE_DURATION_MAX) {
            // LPC-022/4, LPC-022/5: Reject the value and use our maximum value instead
            logger.printfln("Rejected %s failsafe values: Failsafe duration of %d s is longer than %d s", name, duration->as<int>(), FAILSAFE_DURATION_MAX.as<int>());
            if (failsafe_duration != FAILSAFE_DURATION_MAX) {
                failsafe_duration = FAILSAFE_DURATION_MAX;
                persist_failsafe();
                update_api();
                notify_failsafe_subscribers();
            }
            return false;
        }
        if (duration.get() < FAILSAFE_DURATION_MIN) {
            // IG-LPC 3.1: Values out of the permitted range are rejected
            logger.printfln("Rejected %s failsafe values: Failsafe duration of %d s is shorter than %d s", name, duration->as<int>(), FAILSAFE_DURATION_MIN.as<int>());
            return false;
        }
    }

    if (power_limit_w.has_value()) {
        failsafe_power_limit_w = power_limit_w.get();
        failsafe_power_written = true;
    }
    if (duration.has_value()) {
        failsafe_duration = duration.get();
    }
    logger.printfln("Updated %s failsafe to %d W for %d seconds", name, failsafe_power_limit_w, failsafe_duration.as<int>());
    persist_failsafe();
    update_api();
    notify_failsafe_subscribers();
    return true;
}

void LoadPowerLimitUsecase::load_persisted_failsafe()
{
    auto persisted = eebus.failsafe_config.get(config_.api_key);
    if (persisted->get("power_set")->asBool()) {
        const int power_w = persisted->get("power_w")->asInt();
        // IG-LPC 3.6: The Failsafe Consumption Active Power Limit is >= 0 W
        if (!config_.limit_is_positive || power_w >= 0) {
            failsafe_power_limit_w = power_w;
            failsafe_power_written = true;
        }
    }
    const seconds_t duration{persisted->get("duration_s")->asUint()};
    if (duration >= FAILSAFE_DURATION_MIN && duration <= FAILSAFE_DURATION_MAX) {
        failsafe_duration = duration;
    }
    if (failsafe_power_written || duration != 0_s) {
        logger.printfln("%s: Restored failsafe values: %s%d W for %d s", get_usecases_name(config_.usecase_type), failsafe_power_written ? "" : "Nominal maximum power, ", failsafe_power_limit_w, failsafe_duration.as<int>());
    }
}

void LoadPowerLimitUsecase::persist_failsafe()
{
    auto persisted = eebus.failsafe_config.get(config_.api_key);
    bool changed = false;
    if (failsafe_power_written) {
        changed |= persisted->get("power_set")->updateBool(true);
        changed |= persisted->get("power_w")->updateInt(failsafe_power_limit_w);
    }
    changed |= persisted->get("duration_s")->updateUint(failsafe_duration.as<uint32_t>());
    if (changed) {
        eebus.persist_failsafe_config();
        eebus.trace_fmtln("%s: Stored failsafe values persistently", get_usecases_name(config_.usecase_type));
    }
}

int LoadPowerLimitUsecase::nominal_max_w() const
{
    if (power_max_w > 0) {
        return power_max_w;
    }
    if (power_contract_max_w > 0) {
        return power_contract_max_w;
    }
    return EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
}

void LoadPowerLimitUsecase::update_constraints(int power_max, int power_contract_max)
{
    power_max = std::max(power_max, 0);
    power_contract_max = std::max(power_contract_max, 0);
    if (power_max == power_max_w && power_contract_max == power_contract_max_w) {
        return;
    }
    power_max_w = power_max;
    power_contract_max_w = power_contract_max;
    logger.printfln("%s: Nominal maximum power %d W, contractual nominal maximum power %d W (0: unknown)", get_usecases_name(config_.usecase_type), power_max_w, power_contract_max_w);

    // LPC-021/1: The pre-configured failsafe limit is the nominal maximum power, until the Energy Guard wrote a failsafe limit.
    if (config_.limit_is_positive && !failsafe_power_written && failsafe_power_limit_w != nominal_max_w()) {
        failsafe_power_limit_w = nominal_max_w();
        if (state_ == LoadcontrolState::Init || state_ == LoadcontrolState::Failsafe) {
            current_active_limit_w = failsafe_power_limit_w;
        }
        notify_failsafe_subscribers();
    }

    update_api();
    auto data = EVSEEntity::get_electrical_connection_characteristic_list_data();
    eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::ElectricalConnection), data, "electricalConnectionCharacteristicListData");
}

bool LoadPowerLimitUsecase::update_limit(const LimitWrite &write)
{
    const char *name = get_usecases_name(config_.usecase_type);
    // LPC 2.2, IG-LPC 2.11 and 2.14: Reject without changing the state
    if (!is_controlled() && !heartbeat_in_write_window()) {
        logger.printfln("Rejected %s limit: No heartbeat of the Energy Guard within the last %d seconds", name, HEARTBEAT_WRITE_WINDOW.as<int>());
        return false;
    }

    bool new_active = write.active.has_value() ? write.active.get() : limit_active;
    const int new_value = write.value_w.has_value() ? write.value_w.get() : configured_limit;
    // A write without duration keeps the current duration, unless it already expired
    micros_t new_endtime = seconds_left(limit_endtime) > 0_s ? limit_endtime : 0_us;
    if (write.duration.has_value()) {
        if (write.duration.get() <= 0_s) {
            // IG-LPC 2.2: A duration of 0 deactivates the limit immediately, even if the write requests to activate it.
            // The write is accepted nevertheless (IG-LPC 2.16).
            new_active = false;
            new_endtime = 0_us;
        } else {
            new_endtime = now_us() + static_cast<micros_t>(write.duration.get());
        }
    } else if (write.delete_duration) {
        // LPC 3.4.1.4: The duration was removed, the limit is valid until further notice
        new_endtime = 0_us;
    }

    // The limit has to be in the valid range: LPC >= 0 W, LPP <= 0 W
    const bool valid = config_.limit_is_positive ? new_value >= 0 : new_value <= 0;
    if (!valid) {
        // LPC-003/1: Reject the limit and keep the old limit data.
        // LPC-902, LPC-918, LPC-920: Without control so far, a limit that cannot be applied leads to "unlimited/controlled",
        // as the communication with the Energy Guard is verified (IG-LPC 2.14).
        logger.printfln("Rejected %s limit of %d W: Out of range", name, new_value);
        if (!is_controlled()) {
            unlimited_controlled_state();
            update_api();
            notify_limit_subscribers();
        }
        return false;
    }

    limit_active = new_active;
    configured_limit = new_value;
    limit_endtime = new_endtime;

    // The duration keeps decreasing while the limit is deactivated (LPC 2.6.1.1)
    task_scheduler.cancel(limit_endtime_timer);
    limit_endtime_timer = 0;
    if (limit_endtime != 0_us) {
        const micros_t left = limit_endtime - now_us();
        if (left <= 0_us) {
            limit_active = false;
            limit_endtime = 0_us;
        } else {
            limit_endtime_timer = task_scheduler.scheduleOnce(
                [this]() {
                    limit_duration_expired();
                },
                left.to<millis_t>());
        }
    }

    if (limit_active) {
        limited_state();
    } else {
        unlimited_controlled_state();
    }
    update_api();
    notify_limit_subscribers();
    return true;
}

void LoadPowerLimitUsecase::limit_duration_expired()
{
    limit_endtime_timer = 0;
    // LPC-007: Deactivate the limit when the duration expired. The duration MAY be removed.
    limit_active = false;
    limit_endtime = 0_us;
    if (state_ == LoadcontrolState::Limited) {
        logger.printfln("%s limit duration expired", get_usecases_name(config_.usecase_type));
        unlimited_controlled_state();
    }
    update_api();
    notify_limit_subscribers();
}

void LoadPowerLimitUsecase::notify_limit_subscribers()
{
    schedule_once_while_alive([this]() {
        LoadControlLimitListDataType data = EVSEEntity::get_load_control_limit_list_data();
        eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::LoadControl), data, "loadControlLimitListData");
    });
}

void LoadPowerLimitUsecase::notify_failsafe_subscribers()
{
    schedule_once_while_alive([this]() {
        DeviceConfigurationKeyValueListDataType data = EVSEEntity::get_device_configuration_value_list_data();
        eebus.usecases->inform_subscribers(this->entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration), data, "deviceConfigurationKeyValueListData");
    });
}

bool LoadPowerLimitUsecase::heartbeat_in_write_window() const
{
    return last_heartbeat != 0_us && !deadline_elapsed(last_heartbeat + HEARTBEAT_WRITE_WINDOW);
}

bool LoadPowerLimitUsecase::is_power_limited() const
{
    switch (state_) {
        case LoadcontrolState::Limited:
            return true;
        case LoadcontrolState::Init:
        case LoadcontrolState::Failsafe:
            // LPC-901: The Failsafe Active Power Limit applies. It only limits if it is below the nominal maximum power.
            return !config_.limit_is_positive || failsafe_power_limit_w < nominal_max_w();
        default:
            return false;
    }
}

std::string LoadPowerLimitUsecase::bound_energy_guard_device() const
{
    for (FeatureTypeEnumType feature : {FeatureTypeEnumType::LoadControl, FeatureTypeEnumType::DeviceConfiguration}) {
        for (const FeatureAddressType &bound : eebus.usecases->node_management.get_bound_clients(get_feature_address(feature_addresses.at(feature)))) {
            if (bound.device.has_value() && !bound.device.get().empty()) {
                return bound.device.get();
            }
        }
    }
    return {};
}

void LoadPowerLimitUsecase::receive_heartbeat(const FeatureAddressType &source)
{
    // IG-LPC 3.5: Only the heartbeat of the binding partner counts. Another device must not keep the use case out of failsafe.
    // Before any binding, every heartbeat counts: Writes are only accepted from a binding partner anyway.
    const std::string bound_device = bound_energy_guard_device();
    if (!bound_device.empty() && source.device.has_value() && !source.device.get().empty() && source.device.get() != bound_device) {
        eebus.trace_fmtln("%s: Ignoring heartbeat of %s, the Energy Guard is %s", get_usecases_name(config_.usecase_type), EEBUS_USECASE_HELPERS::spine_address_to_string(source).c_str(), bound_device.c_str());
        return;
    }
    heartbeat_received = true;
    last_heartbeat = now_us();
    task_scheduler.cancel(heartbeat_timeout_timer);
    heartbeat_timeout_timer = task_scheduler.scheduleOnce(
        [this]() {
            heartbeat_timeout_timer = 0;
            heartbeat_timed_out();
        },
        HEARTBEAT_TIMEOUT);
    // A heartbeat alone does not change the state. The state changes with a following write on the limit (LPC 2.2).
    update_api();
}

// Not EebusHeartBeat's receive_heartbeat_timeout(): It is restarted by the heartbeat of any device (IG-LPC 3.5).
void LoadPowerLimitUsecase::heartbeat_timed_out()
{
    heartbeat_received = false;
    if (is_controlled()) {
        // LPC-911, LPC-912
        logger.printfln("No heartbeat received from the Energy Guard for 120 seconds. Switching to failsafe state");
        failsafe_state();
    }
    update_api();
}

void LoadPowerLimitUsecase::inform_spineconnection_usecase_update(SpineConnection *conn)
{
    // IG-LPC 3.8: With multiple Energy Guard instances, the one that binds LoadControl and DeviceConfiguration is in control.
    // Subscribe to its heartbeat only after the bindings.
    if (has_multiple_energy_guards(conn)) {
        eebus.trace_fmtln("%s: Peer announces multiple Energy Guard instances. Waiting for bindings before subscribing to the heartbeat", get_usecases_name(config_.usecase_type));
        // The SMA Sunny Home Manager 2.0 writes its first limit about 2 s after connecting, before any heartbeat, and does not retry
        // a rejected write. Read all its heartbeats once now, so the first write follows a heartbeat (LPC 2.2, IG-LPC 2.11).
        for (const std::vector<AddressEntityType> &entity : conn->get_use_case_actor_entities(config_.usecase_name, "EnergyGuard")) {
            const FeatureAddressType device_diagnosis = conn->get_address_of_feature(entity, FeatureTypeEnumType::DeviceDiagnosis, RoleType::server);
            if (device_diagnosis.feature.has_value()) {
                eebus.usecases->evse_heartbeat.read_heartbeat_once(device_diagnosis);
            }
        }
        // The bindings might already exist if they were requested before the use case data was received
        subscribe_heartbeat_of_bound_energy_guard(conn);
        return;
    }
    // Scenario 3
    eebus.usecases->evse_heartbeat.subscribe_to_actor_heartbeat(conn, config_.usecase_name, "EnergyGuard", config_.usecase_type);
}

bool LoadPowerLimitUsecase::has_multiple_energy_guards(SpineConnection *conn) const
{
    return conn != nullptr && conn->get_use_case_actor_entities(config_.usecase_name, "EnergyGuard").size() > 1;
}

bool LoadPowerLimitUsecase::is_energy_guard_binding_target(const FeatureAddressType &server) const
{
    if (!server.feature.has_value() || !server.entity.has_value() || server.entity.get() != entity_address) {
        return false;
    }
    if (server.device.has_value() && server.device.get() != EEBUS_USECASE_HELPERS::get_spine_device_name()) {
        return false;
    }
    return server.feature.get() == feature_addresses.at(FeatureTypeEnumType::LoadControl) || server.feature.get() == feature_addresses.at(FeatureTypeEnumType::DeviceConfiguration);
}

bool LoadPowerLimitUsecase::validate_binding_request(const FeatureAddressType &client, const FeatureAddressType &server)
{
    if (!is_energy_guard_binding_target(server)) {
        return true;
    }
    // IG-LPC 3.5: While a device is bound to LoadControl or DeviceConfiguration, no other device may bind to either.
    // Bindings are removed when the device disconnects.
    for (FeatureTypeEnumType feature : {FeatureTypeEnumType::LoadControl, FeatureTypeEnumType::DeviceConfiguration}) {
        for (const FeatureAddressType &bound : eebus.usecases->node_management.get_bound_clients(get_feature_address(feature_addresses.at(feature)))) {
            if (bound.device.has_value() && client.device.has_value() && bound.device.get() != client.device.get()) {
                logger.printfln("Rejected EEBUS binding of %s: %s is already bound by %s", EEBUS_USECASE_HELPERS::spine_address_to_string(client).c_str(), get_usecases_name(config_.usecase_type), EEBUS_USECASE_HELPERS::spine_address_to_string(bound).c_str());
                return false;
            }
        }
    }
    SpineConnection *conn = EEBusUseCases::get_spine_connection(client);
    if (!has_multiple_energy_guards(conn)) {
        return true;
    }
    // IG-LPC 3.8: The bindings on LoadControl and DeviceConfiguration have to originate from the same entity
    for (FeatureTypeEnumType feature : {FeatureTypeEnumType::LoadControl, FeatureTypeEnumType::DeviceConfiguration}) {
        for (const FeatureAddressType &bound : eebus.usecases->node_management.get_bound_clients(get_feature_address(feature_addresses.at(feature)))) {
            if (EEBusUseCases::get_spine_connection(bound) == conn && bound.entity.get() != client.entity.get()) {
                logger.printfln("Rejected EEBUS binding of %s: The Energy Guard is already bound from %s", EEBUS_USECASE_HELPERS::spine_address_to_string(client).c_str(), EEBUS_USECASE_HELPERS::spine_address_to_string(bound).c_str());
                return false;
            }
        }
    }
    return true;
}

void LoadPowerLimitUsecase::inform_binding_added(const FeatureAddressType &client, const FeatureAddressType &server)
{
    if (!is_energy_guard_binding_target(server)) {
        return;
    }
    SpineConnection *conn = EEBusUseCases::get_spine_connection(client);
    // With a single Energy Guard instance the heartbeat was already subscribed after discovery
    if (has_multiple_energy_guards(conn)) {
        subscribe_heartbeat_of_bound_energy_guard(conn);
    }
}

void LoadPowerLimitUsecase::subscribe_heartbeat_of_bound_energy_guard(SpineConnection *conn)
{
    if (conn == nullptr) {
        return;
    }
    auto bound_clients_of_peer = [this, conn](FeatureTypeEnumType feature) {
        std::vector<FeatureAddressType> clients{};
        for (const FeatureAddressType &bound : eebus.usecases->node_management.get_bound_clients(get_feature_address(feature_addresses.at(feature)))) {
            if (EEBusUseCases::get_spine_connection(bound) == conn) {
                clients.push_back(bound);
            }
        }
        return clients;
    };
    const std::vector<FeatureAddressType> load_control_clients = bound_clients_of_peer(FeatureTypeEnumType::LoadControl);
    const std::vector<FeatureAddressType> device_configuration_clients = bound_clients_of_peer(FeatureTypeEnumType::DeviceConfiguration);

    for (const FeatureAddressType &load_control_client : load_control_clients) {
        for (const FeatureAddressType &device_configuration_client : device_configuration_clients) {
            if (load_control_client.entity.get() != device_configuration_client.entity.get()) {
                continue;
            }
            // Both bindings from the same entity: This is the Energy Guard instance in control
            FeatureAddressType device_diagnosis = conn->get_address_of_feature(load_control_client.entity.get(), FeatureTypeEnumType::DeviceDiagnosis, RoleType::server);
            if (!device_diagnosis.feature.has_value()) {
                eebus.trace_fmtln("%s: Energy Guard bound from %s has no DeviceDiagnosis server on its entity. Cannot receive its heartbeat", get_usecases_name(config_.usecase_type), EEBUS_USECASE_HELPERS::spine_address_to_string(load_control_client).c_str());
                return;
            }
            eebus.trace_fmtln("%s: Energy Guard bound from %s. Using its heartbeat", get_usecases_name(config_.usecase_type), EEBUS_USECASE_HELPERS::spine_address_to_string(load_control_client).c_str());
            eebus.usecases->evse_heartbeat.initialize_heartbeat_on_feature(device_diagnosis, config_.usecase_type, true);
            return;
        }
    }
}

void LoadPowerLimitUsecase::init_state()
{
    state_ = LoadcontrolState::Init;
    limit_active = false;
    // LPC-901: In "init" the Failsafe Active Power Limit applies
    current_active_limit_w = failsafe_power_limit_w;

    task_scheduler.cancel(init_timer);
    init_timer = task_scheduler.scheduleOnce(
        [this]() {
            init_timer = 0;
            if (state_ == LoadcontrolState::Init) {
                // LPC-906
                logger.printfln("No Energy Guard took control within %d seconds. Switching to unlimited/autonomous state", INIT_TIMEOUT.as<int>());
                unlimited_autonomous_state();
                update_api();
            }
        },
        INIT_TIMEOUT);
}

void LoadPowerLimitUsecase::unlimited_controlled_state()
{
    if (state_ != LoadcontrolState::UnlimitedControlled) {
        logger.printfln("%s: Controlled by the Energy Guard, power not limited", get_usecases_name(config_.usecase_type));
    }
    state_ = LoadcontrolState::UnlimitedControlled;
    limit_active = false;
    current_active_limit_w = EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION;
}

void LoadPowerLimitUsecase::limited_state()
{
    if (state_ != LoadcontrolState::Limited || current_active_limit_w != configured_limit) {
        if (limit_endtime == 0_us) {
            logger.printfln("%s: Limiting power to %d W until further notice", get_usecases_name(config_.usecase_type), configured_limit);
        } else {
            logger.printfln("%s: Limiting power to %d W for %d s", get_usecases_name(config_.usecase_type), configured_limit, seconds_left(limit_endtime).as<int>());
        }
    }

    state_ = LoadcontrolState::Limited;
    current_active_limit_w = configured_limit;
    limit_active = true;
}

void LoadPowerLimitUsecase::failsafe_state()
{
    state_ = LoadcontrolState::Failsafe;
    limit_active = false;

    // LPC-901: In "failsafe state" the Failsafe Active Power Limit applies
    current_active_limit_w = failsafe_power_limit_w;
    // The duration of the Active Power Limit keeps running (LPC 2.6.1.1). limit_endtime_timer removes it when it expires,
    // so a later write without duration does not inherit an expired duration.

    failsafe_expiry_endtime = now_us() + static_cast<micros_t>(failsafe_duration);
    task_scheduler.cancel(failsafe_expiry_timer);
    failsafe_expiry_timer = task_scheduler.scheduleOnce(
        [this]() {
            failsafe_expiry_timer = 0;
            if (state_ == LoadcontrolState::Failsafe) {
                // LPC-922
                logger.printfln("Failsafe duration expired. Switching to unlimited/autonomous state");
                unlimited_autonomous_state();
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

// The power values in the API are Uint16. Saturate instead of letting larger values wrap around (e.g. 70000 W -> 4464 W).
static uint32_t api_power_w(int power_w)
{
    return static_cast<uint32_t>(std::clamp(power_w, 0, static_cast<int>(UINT16_MAX)));
}

void LoadPowerLimitUsecase::update_api() const
{
    auto api_entry = eebus.eebus_usecase_state.get(config_.api_key);
    api_entry->get("usecase_state")->updateEnum(state_);
    api_entry->get("limit_active")->updateBool(limit_active);
    // For LPP, we use abs() to display the limit as positive in the UI
    api_entry->get("current_limit")->updateUint(api_power_w(config_.limit_is_positive ? current_active_limit_w : abs(current_active_limit_w)));
    api_entry->get("failsafe_limit_power_w")->updateUint(api_power_w(failsafe_power_limit_w));
    api_entry->get("failsafe_limit_duration_s")->updateUint(failsafe_duration.as<uint32_t>());

    if (state_ == LoadcontrolState::Limited) {
        api_entry->get("outstanding_duration_s")->updateUint(seconds_left(limit_endtime).as<uint32_t>());
    } else if (state_ == LoadcontrolState::Failsafe) {
        api_entry->get("outstanding_duration_s")->updateUint(seconds_left(failsafe_expiry_endtime).as<uint32_t>());
    } else {
        api_entry->get("outstanding_duration_s")->updateUint(0);
    }

    api_entry->get("constraints_power_maximum")->updateUint(api_power_w(power_max_w > 0 ? power_max_w : power_contract_max_w));
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
    const seconds_t duration_left = seconds_left(limit_endtime);

    LoadControlLimitDataType limit_data{};
    limit_data.limitId = limit_description_id;
    limit_data.isLimitChangeable = !limit_fixed;
    limit_data.isLimitActive = limit_active;
    if (duration_left > 0_s) {
        limit_data.timePeriod->endTime = EEBUS_USECASE_HELPERS::iso_duration_to_string(duration_left);
    }
    // The Active Power Limit data point as written by the Energy Guard, not the currently effective limit (e.g. the failsafe limit)
    limit_data.value->number = configured_limit;
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
    // Values that are not known are not reported. A device reports only LPC-041, an energy manager only LPC-042 (LPC 2.6.4.1).
    if (power_max_w > 0) {
        ElectricalConnectionCharacteristicDataType power_max{};
        power_max.electricalConnectionId = id_ec_1;
        power_max.parameterId = id_p_1;
        power_max.characteristicId = id_cc_1;
        power_max.characteristicContext = ElectricalConnectionCharacteristicContextEnumType::entity;
        power_max.characteristicType = config_.nominal_max_type;
        power_max.value->number = power_max_w;
        power_max.value->scale = 0;
        power_max.unit = UnitOfMeasurementEnumType::W;
        data->electricalConnectionCharacteristicData->push_back(power_max);
    }
    if (power_contract_max_w > 0) {
        ElectricalConnectionCharacteristicDataType contract_max{};
        contract_max.electricalConnectionId = id_ec_1;
        contract_max.parameterId = id_p_1;
        contract_max.characteristicId = id_cc_2;
        contract_max.characteristicContext = ElectricalConnectionCharacteristicContextEnumType::entity;
        contract_max.characteristicType = config_.contractual_max_type;
        contract_max.value->number = power_contract_max_w;
        contract_max.value->scale = 0;
        contract_max.unit = UnitOfMeasurementEnumType::W;
        data->electricalConnectionCharacteristicData->push_back(contract_max);
    }
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
    .api_key = "lpc",
    .energy_direction = EnergyDirectionEnumType::consume,
    .nominal_max_type = ElectricalConnectionCharacteristicTypeEnumType::powerConsumptionNominalMax,
    .contractual_max_type = ElectricalConnectionCharacteristicTypeEnumType::contractualConsumptionNominalMax,
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
    .api_key = "lpp",
    .energy_direction = EnergyDirectionEnumType::produce,
    .nominal_max_type = ElectricalConnectionCharacteristicTypeEnumType::powerProductionNominalMax,
    .contractual_max_type = ElectricalConnectionCharacteristicTypeEnumType::contractualProductionNominalMax,
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
