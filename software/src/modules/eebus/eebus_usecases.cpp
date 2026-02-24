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

// Disable autoformat as it moves eebus.h below eebus_usecases.h which causes linker errors
// clang-format off
#include "eebus.h"
#include "eebus_usecases.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "ship_types.h"
#include <chrono>
#include <utility>

// clang-format on

template <typename T> void insert_vector(std::vector<T> &dest, const std::vector<T> &src)
{
    dest.insert(dest.end(), src.begin(), src.end());
}

// ============================================================================
// EEBusUseCases Manager Implementation
// ============================================================================

EEBusUseCases::EEBusUseCases()
{
    // Entity Addresses should be consistent so all actors are under the same entity
    usecase_list.push_back(&node_management);
    node_management.set_usecaseManager(this);
    node_management.set_entity_address({0});
    std::vector<Usecases> supported_usecases{};

    // EVSE Actors
    usecase_list.push_back(&evse_heartbeat);
    evse_heartbeat.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(evse_heartbeat.get_usecase_type());
#ifdef EEBUS_ENABLE_EVCS_USECASE
    usecase_list.push_back(&charging_summary);
    charging_summary.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(charging_summary.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_LPC_USECASE
    usecase_list.push_back(&limitation_of_power_consumption);
    limitation_of_power_consumption.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(limitation_of_power_consumption.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_MPC_USECASE
    usecase_list.push_back(&monitoring_of_power_consumption);
    monitoring_of_power_consumption.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(monitoring_of_power_consumption.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    usecase_list.push_back(&limitation_of_power_production);
    limitation_of_power_production.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(limitation_of_power_production.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_EVSECC_USECASE
    usecase_list.push_back(&evse_commissioning_and_configuration);
    evse_commissioning_and_configuration.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(evse_commissioning_and_configuration.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    usecase_list.push_back(&monitoring_of_grid_connection_point);
    monitoring_of_grid_connection_point.set_entity_address(EVSEEntity::entity_address);
    supported_usecases.push_back(monitoring_of_grid_connection_point.get_usecase_type());
#endif

    // EV actors
    usecase_list.push_back(&ev_heartbeat);
    ev_heartbeat.set_entity_address(EVEntity::entity_address);
    supported_usecases.push_back(ev_heartbeat.get_usecase_type());
#ifdef EEBUS_ENABLE_EVCC_USECASE
    usecase_list.push_back(&ev_commissioning_and_configuration);
    ev_commissioning_and_configuration.set_entity_address(EVEntity::entity_address); // EVCC entity is "under" the ChargingSummary entity and therefore the first value
    supported_usecases.push_back(ev_commissioning_and_configuration.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_CEVC_USECASE
    usecase_list.push_back(&coordinate_ev_charging);
    coordinate_ev_charging.set_entity_address({1, 1});
    supported_usecases.push_back(coordinate_ev_charging.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    usecase_list.push_back(&ev_charging_electricity_measurement);
    ev_charging_electricity_measurement.set_entity_address(EVEntity::entity_address);
    supported_usecases.push_back(ev_charging_electricity_measurement.get_usecase_type());
#endif
#ifdef EEBUS_ENABLE_OPEV_USECASE
    usecase_list.push_back(&overload_protection_by_ev_charging_current_curtailment);
    overload_protection_by_ev_charging_current_curtailment.set_entity_address(EVEntity::entity_address);
    supported_usecases.push_back(overload_protection_by_ev_charging_current_curtailment.get_usecase_type());
#endif

    // Map out the features used and assign feature addresses
    std::map<std::pair<std::vector<int>, int>, FeatureTypeEnumType> features;
    constexpr int feature_step_size = 2;
    int feature_index = feature_step_size;
    for (EebusUsecase *uc : usecase_list) {
        auto entity = uc->get_entity_address();
        for (FeatureTypeEnumType feature_type_needed : uc->get_supported_features()) {
            bool assigned = false;
            for (auto &existing_feature_pair : features) {
                if (existing_feature_pair.second == feature_type_needed && existing_feature_pair.first.first == entity) {
                    uc->set_feature_address(existing_feature_pair.first.second, feature_type_needed);
                    assigned = true;
                    break;
                }
            }
            if (!assigned) {
                if (feature_type_needed == FeatureTypeEnumType::NodeManagement) {
                    features[{entity, 0}] = feature_type_needed;
                    uc->set_feature_address(0, feature_type_needed);
                } else {
                    features[{entity, feature_index}] = feature_type_needed;
                    uc->set_feature_address(feature_index, feature_type_needed);
                    feature_index += feature_step_size;
                }
            }
        }
    }
    task_scheduler.scheduleOnce(
        [this, supported_usecases]() {
            // String usecase_names = "";
            eebus.eebus_usecase_state.get("usecases_supported")->removeAll();
            for (const Usecases uc : supported_usecases) {
                //  usecase_names += String(get_usecases_name(uc)) + ",";
                auto entry = eebus.eebus_usecase_state.get("usecases_supported")->add();
                entry->updateEnum(uc);
            }
        },
        1_s);
    // eebus.eebus_usecase_state.get("usecases_supported")->updateString(usecase_names);
    initialized = true; // set to true, otherwise subscriptions will not work
}

void EEBusUseCases::process_spine_message(HeaderType &header, SpineDataTypeHandler *data, SpineConnection *connection)
{
    // Prepare the handle messages
    eebus_commands_received++;
    eebus.eebus_usecase_state.get("commands_received")->updateUint(eebus_commands_received);
    BasicJsonDocument<ArduinoJsonPsramAllocator> response_doc{SPINE_CONNECTION_MAX_JSON_SIZE};
    JsonObject responseObj = response_doc.to<JsonObject>();
    MessageReturn send_response{};
    String entity_name = "Unknown";
    const FeatureAddressType source_address = header.addressSource.get();
    const FeatureAddressType destination_address = header.addressDestination.get();

    // If its a result, no further processing. If the result is an error, log it
    // TODO: send the results back to the usecase that sent the original command?
    if (data->last_cmd == SpineDataTypeHandler::Function::resultData) {
        eebus.trace_fmtln("Usecases: Received resultData, no further processing");
        ResultDataType result_data = data->resultdatatype.get();
        if (result_data.errorNumber.get() != static_cast<uint8_t>(EEBUS_USECASE_HELPERS::ResultErrorNumber::NoError)) {
            logger.printfln("Usecases: An error was received from the communication Partner. Error Number: %s, Description: %s. Error refers to Message: %d", EEBUS_USECASE_HELPERS::get_result_error_number_string(result_data.errorNumber.get()).c_str(), result_data.description.get().c_str(), header.msgCounterReference.get());
        }
        data->reset();
        return;
    }
    // Identify the usecase by matching the entity address and checking if the usecase has that feature. This needs to be this extensive as multiple usecases may share a feature.
    // Currently no two usecases have the same function but once they do, this and the usecases need to be updated
    bool found_dest_entity = false;

    for (EebusUsecase *entity : usecase_list) {
        if (header.addressDestination->entity.has_value() && entity->matches_entity_address(header.addressDestination->entity.get()) && !found_dest_entity) {
            send_response = entity->handle_message(header, data, responseObj);
            if (send_response.is_handled) {
                found_dest_entity = true;
                eebus.trace_fmtln("Usecases: Found entity: %s", get_usecases_name(entity->get_usecase_type()));
            } else {
                eebus.trace_fmtln("Usecases: Entity %s could not handle the message", get_usecases_name(entity->get_usecase_type()));
            }
        }
    }

    // If no usecase was found that could handle the message, we should inform the peer.
    if (!found_dest_entity) {
        eebus.trace_fmtln("Usecases: Received message for unknown entity: %s", EEBUS_USECASE_HELPERS::spine_address_to_string(header.addressDestination.get()).c_str());
        EEBUS_USECASE_HELPERS::build_result_data(responseObj, EEBUS_USECASE_HELPERS::ResultErrorNumber::CommandRejected, "Unknown entity requested");
        // We always send a response if we do not know the entity
        connection->send_datagram(response_doc, CmdClassifierType::result, destination_address, source_address, false);
        data->reset();
        return;
    }
    // Do we need to send a response
    if (send_response.send_response) {
        eebus.trace_fmtln("Usecases: Sending response");
        if (header.ackRequest.has_value() && header.ackRequest.get() && send_response.cmd_classifier != CmdClassifierType::result && header.cmdClassifier != CmdClassifierType::read) {

            eebus.trace_fmtln("Usecases: Header requested an ack, but sending a non-result response: %d", static_cast<int>(send_response.cmd_classifier));
        }
        eebus.eebus_usecase_state.get("commands_sent")->updateUint(eebus_responses_sent++);
        //send_spine_message(*header.addressDestination, *header.addressSource, response_doc, send_response);
        // We should use send_spine_message here but as we have the connection it is much quicker to send it directly back to it
        connection->send_datagram(response_doc, send_response.cmd_classifier, destination_address, source_address, false);
    } else {
        if (header.ackRequest.has_value() && header.ackRequest.get()) {
            eebus.trace_fmtln("Usecases: ERROR: Header requested an ack, but no response was generated");
        }
        eebus.trace_fmtln("Usecases: No response needed. Not sending anything");
    }
    data->reset();
}

template <typename T> size_t EEBusUseCases::inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, T &data, const char *function_name)
{
    if (initialized && EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS)
        return node_management.inform_subscribers(entity, feature, data, function_name);
    if constexpr (EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS)
        eebus.trace_fmtln("Attempted to inform subscribers while Usecases were not yet initialized");
    else
        eebus.trace_fmtln("Attempted to inform subscribers about a change in %s while subscriptions are disabled", function_name);
    return 0;
}
template <typename T> bool EEBusUseCases::send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, T payload, CmdClassifierType cmd_classifier, const char *function_name, bool want_ack)
{
    BasicJsonDocument<ArduinoJsonPsramAllocator> doc{SPINE_CONNECTION_MAX_JSON_SIZE};
    JsonObject obj = doc.to<JsonObject>();
    obj[function_name] = payload;
    return send_spine_message(destination, sender, obj, cmd_classifier, want_ack);
}

bool EEBusUseCases::send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, const JsonVariantConst payload, CmdClassifierType cmd_classifier, const bool want_ack)
{
    eebus_responses_sent++;
    eebus.eebus_usecase_state.get("commands_sent")->updateUint(eebus_responses_sent);
    if (sender.feature.isNull() || sender.entity.isNull()) {
        eebus.trace_fmtln("Usecases: Cannot send spine message, sender entity or feature is null");
        return false;
    }
    if (payload.isNull()) {
        eebus.trace_fmtln("Usecases: Payload is null");
        logger.printfln("payload is null");
    }
    if (sender.device.isNull()) {
        sender.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    }
    bool message_sent = false;
    if (SpineConnection *spine_conn = get_spine_connection(destination)) {
        spine_conn->send_datagram(payload, cmd_classifier, sender, destination, want_ack);
        message_sent = true;
    }

    return message_sent;
}

SpineConnection *EEBusUseCases::get_spine_connection(const FeatureAddressType &spine_address)
{
    for (auto &ship_connection : eebus.ship.ship_connections) {
        if (ship_connection && ship_connection->spine->check_known_address(spine_address)) {
            return ship_connection->spine.get();
        }
    }
    return nullptr;
}

// ============================================================================
// Explicit Template Instantiations
// ============================================================================
// These are required because the template definitions are in this .cpp file
// but used from the separate usecase .cpp files in the usecases/ subdirectory.

// --- inform_subscribers<T> instantiations ---
// Used by heartbeat.cpp
template size_t EEBusUseCases::inform_subscribers<DeviceDiagnosisHeartbeatDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, DeviceDiagnosisHeartbeatDataType &, const char *);
// Used by loadcontrol.cpp
template size_t EEBusUseCases::inform_subscribers<LoadControlLimitListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, LoadControlLimitListDataType &, const char *);
template size_t EEBusUseCases::inform_subscribers<DeviceConfigurationKeyValueListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, DeviceConfigurationKeyValueListDataType &, const char *);
template size_t EEBusUseCases::inform_subscribers<ElectricalConnectionCharacteristicListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, ElectricalConnectionCharacteristicListDataType &, const char *);
// Used by evcc.cpp
template size_t EEBusUseCases::inform_subscribers<DeviceConfigurationKeyValueDescriptionListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, DeviceConfigurationKeyValueDescriptionListDataType &, const char *);
template size_t EEBusUseCases::inform_subscribers<IdentificationListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, IdentificationListDataType &, const char *);
template size_t EEBusUseCases::inform_subscribers<DeviceClassificationManufacturerDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, DeviceClassificationManufacturerDataType &, const char *);
template size_t EEBusUseCases::inform_subscribers<ElectricalConnectionParameterDescriptionListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, ElectricalConnectionParameterDescriptionListDataType &, const char *);
template size_t EEBusUseCases::inform_subscribers<ElectricalConnectionPermittedValueSetListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, ElectricalConnectionPermittedValueSetListDataType &, const char *);
template size_t EEBusUseCases::inform_subscribers<DeviceDiagnosisStateDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, DeviceDiagnosisStateDataType &, const char *);
// Used by evsecc.cpp (DeviceDiagnosisStateDataType already instantiated above)
// Used by evcem.cpp, mpc.cpp, mgcp.cpp
template size_t EEBusUseCases::inform_subscribers<MeasurementListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, MeasurementListDataType &, const char *);
template size_t EEBusUseCases::inform_subscribers<MeasurementConstraintsListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, MeasurementConstraintsListDataType &, const char *);
template size_t EEBusUseCases::inform_subscribers<MeasurementDescriptionListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, MeasurementDescriptionListDataType &, const char *);
// Used by evcs.cpp
template size_t EEBusUseCases::inform_subscribers<BillListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, BillListDataType &, const char *);
// Used by cevc.cpp
template size_t EEBusUseCases::inform_subscribers<TimeSeriesListDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, TimeSeriesListDataType &, const char *);
template size_t EEBusUseCases::inform_subscribers<IncentiveTableDataType>(const std::vector<AddressEntityType> &, AddressFeatureType, IncentiveTableDataType &, const char *);

// --- send_spine_message<T> instantiations ---
// Used by NodeManagementEntity::inform_subscribers which forwards to EEBusUseCases::send_spine_message
// These must match all types used in NodeManagementEntity::inform_subscribers instantiations
template bool EEBusUseCases::send_spine_message<DeviceDiagnosisHeartbeatDataType>(const FeatureAddressType &, FeatureAddressType &, DeviceDiagnosisHeartbeatDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<NodeManagementDetailedDiscoveryDataType>(const FeatureAddressType &, FeatureAddressType &, NodeManagementDetailedDiscoveryDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<LoadControlLimitListDataType>(const FeatureAddressType &, FeatureAddressType &, LoadControlLimitListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<DeviceConfigurationKeyValueListDataType>(const FeatureAddressType &, FeatureAddressType &, DeviceConfigurationKeyValueListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<ElectricalConnectionCharacteristicListDataType>(const FeatureAddressType &, FeatureAddressType &, ElectricalConnectionCharacteristicListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<MeasurementListDataType>(const FeatureAddressType &, FeatureAddressType &, MeasurementListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<MeasurementConstraintsListDataType>(const FeatureAddressType &, FeatureAddressType &, MeasurementConstraintsListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<MeasurementDescriptionListDataType>(const FeatureAddressType &, FeatureAddressType &, MeasurementDescriptionListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<DeviceDiagnosisStateDataType>(const FeatureAddressType &, FeatureAddressType &, DeviceDiagnosisStateDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<DeviceConfigurationKeyValueDescriptionListDataType>(const FeatureAddressType &, FeatureAddressType &, DeviceConfigurationKeyValueDescriptionListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<IdentificationListDataType>(const FeatureAddressType &, FeatureAddressType &, IdentificationListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<DeviceClassificationManufacturerDataType>(const FeatureAddressType &, FeatureAddressType &, DeviceClassificationManufacturerDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<ElectricalConnectionParameterDescriptionListDataType>(const FeatureAddressType &, FeatureAddressType &, ElectricalConnectionParameterDescriptionListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<ElectricalConnectionPermittedValueSetListDataType>(const FeatureAddressType &, FeatureAddressType &, ElectricalConnectionPermittedValueSetListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<BillListDataType>(const FeatureAddressType &, FeatureAddressType &, BillListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<TimeSeriesListDataType>(const FeatureAddressType &, FeatureAddressType &, TimeSeriesListDataType, CmdClassifierType, const char *, bool);
template bool EEBusUseCases::send_spine_message<IncentiveTableDataType>(const FeatureAddressType &, FeatureAddressType &, IncentiveTableDataType, CmdClassifierType, const char *, bool);
