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

#include "heartbeat.h"

#include "../eebus.h"
#include "../eebus_usecases.h"
#include "../module_dependencies.h"

EebusHeartBeat::EebusHeartBeat()
{
    heartbeat_received_timeout_task = task_scheduler.scheduleOnce(
        [this]() {
            emit_timeout();
        },
        120_s); // Add some buffer time
    // Send out heartbeat
    heartbeat_send_task = task_scheduler.scheduleWithFixedDelay(
        [this]() {
            send_heartbeat_to_subs();
        },
        heartbeat_interval);
}

EebusHeartBeat::~EebusHeartBeat()
{
    task_scheduler.cancel(heartbeat_received_timeout_task);
    task_scheduler.cancel(heartbeat_send_task);
}

DeviceDiagnosisHeartbeatDataType EebusHeartBeat::read_heartbeat()
{
    timeval time_v{};
    rtc.clock_synced(&time_v);

    DeviceDiagnosisHeartbeatDataType outgoing_heartbeatData{};
    outgoing_heartbeatData.heartbeatCounter = heartbeat_counter++;
    outgoing_heartbeatData.heartbeatTimeout = EEBUS_USECASE_HELPERS::iso_duration_to_string(heartbeat_interval);
    outgoing_heartbeatData.timestamp = EEBUS_USECASE_HELPERS::unix_to_iso_timestamp(time_v.tv_sec).c_str();
    return outgoing_heartbeatData;
}

void EebusHeartBeat::initialize_heartbeat_on_feature(FeatureAddressType &target, Usecases sending_usecase, bool expect_notify)
{
    heartbeat_targets.push_back(target);
    // Subscribe to heartbeat notifications from target
    if (expect_notify) {
        task_scheduler.scheduleOnce(
            [=, this]() mutable {
                const auto connection = EEBusUseCases::get_spine_connection(target);
                if (connection == nullptr)
                    return;
                FeatureAddressType remote_device_diag = connection->get_address_of_feature(target.entity.get(), FeatureTypeEnumType::DeviceDiagnosis, RoleType::server);
                FeatureAddressType local_client = get_feature_address(feature_addresses.at(FeatureTypeEnumType::Generic));
                if (connection->is_subscribed(local_client, remote_device_diag)) {
                    eebus.trace_fmtln("EebusHeartBeat: Already subscribed to heartbeat notifications from target device %s", EEBUS_USECASE_HELPERS::spine_address_to_string(remote_device_diag).c_str());
                    return;
                }
                eebus.trace_fmtln("EebusHeartBeat: Subscribing to heartbeat notifications from target device %s", EEBUS_USECASE_HELPERS::spine_address_to_string(remote_device_diag).c_str());
                send_full_read(feature_addresses.at(FeatureTypeEnumType::Generic), remote_device_diag, SpineDataTypeHandler::Function::deviceDiagnosisHeartbeatData);
                eebus.usecases->node_management.subscribe_to_feature(local_client, remote_device_diag, FeatureTypeEnumType::DeviceDiagnosis);
            },
            0_ms);
    }
}

void EebusHeartBeat::update_heartbeat_interval(seconds_t interval)
{
    heartbeat_interval = interval;
    task_scheduler.cancel(heartbeat_send_task);
    heartbeat_send_task = task_scheduler.scheduleOnce(
        [this]() {
            send_heartbeat_to_subs();
        },
        heartbeat_interval);
}

MessageReturn EebusHeartBeat::handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response)
{
    if (data->last_cmd != SpineDataTypeHandler::Function::deviceDiagnosisHeartbeatData) {
        return {false};
    }
    switch (header.cmdClassifier.get()) {
        case CmdClassifierType::read:
            eebus.trace_fmtln("EebusHeartBeat: Command identified as DeviceDiagnosisHeartbeatData with a read command");
            response["deviceDiagnosisHeartbeatData"] = read_heartbeat();
            return {true, true, CmdClassifierType::reply};
        case CmdClassifierType::notify:
        case CmdClassifierType::reply:
            emit_heartbeat_received(data->devicediagnosisheartbeatdatatype.get());
            return {true, false};
        default:
            return {false, false};
    }
}

NodeManagementDetailedDiscoveryEntityInformationType EebusHeartBeat::get_detailed_discovery_entity_information() const
{
    return {};
}

std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> EebusHeartBeat::get_detailed_discovery_feature_information() const
{
    NodeManagementDetailedDiscoveryFeatureInformationType server_feature = build_feature_information(FeatureTypeEnumType::DeviceDiagnosis, RoleType::server);
    server_feature.description->supportedFunction->push_back(build_function_property(FunctionEnumType::deviceDiagnosisHeartbeatData));

    NodeManagementDetailedDiscoveryFeatureInformationType client_feature = build_feature_information(FeatureTypeEnumType::Generic, RoleType::client);

    FunctionPropertyType generic_heartbeat_property{};
    generic_heartbeat_property.function = FunctionEnumType::deviceDiagnosisHeartbeatData;
    client_feature.description->supportedFunction->push_back(generic_heartbeat_property);

    return {server_feature, client_feature};
}

void EebusHeartBeat::emit_timeout() const
{
    for (EebusUsecase *uc : registered_usecases) {
        uc->receive_heartbeat_timeout();
    }
}

void EebusHeartBeat::send_heartbeat_to_subs()
{
    DeviceDiagnosisHeartbeatDataType heartbeat_data = read_heartbeat();
    auto subs = eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis), heartbeat_data, "deviceDiagnosisHeartbeatData");
    eebus.trace_fmtln("heartbeat_sent to %d subscribers", subs);
}

void EebusHeartBeat::emit_heartbeat_received(DeviceDiagnosisHeartbeatDataType &heartbeat_data)
{
    task_scheduler.cancel(heartbeat_received_timeout_task);
    for (EebusUsecase *uc : registered_usecases) {
        uc->receive_heartbeat();
    }
    seconds_t timeout = EEBUS_USECASE_HELPERS::iso_duration_to_seconds(heartbeat_data.heartbeatTimeout.get());
    heartbeat_received_timeout_task = task_scheduler.scheduleOnce(
        [this]() {
            emit_timeout();
        },
        timeout + 1_s); // Add some buffer time
}
