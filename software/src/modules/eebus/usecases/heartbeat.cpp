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
#include "../generated/module_dependencies.h"

// No heartbeat of any peer for this long calls receive_heartbeat_timeout() (e.g. CEVC). LPC/LPP have their own timeout.
// The heartbeatTimeout announced by the peer is not used: The CS does not have to evaluate it (LPC 3.2.2.2, Table 28).
static constexpr seconds_t HEARTBEAT_RECEIVE_TIMEOUT = 120_s;

// LPC 3.3.4: If the subscription fails, poll. Half of the 60 s heartbeat interval (LPC-031), so one lost read is tolerated.
static constexpr seconds_t HEARTBEAT_POLL_INTERVAL = 30_s;

// Read the heartbeat if a subscribed peer did not notify for this long, before the 120 s timeout is reached.
static constexpr seconds_t HEARTBEAT_NOTIFY_MISSING_INTERVAL = 75_s;

static constexpr seconds_t HEARTBEAT_POLL_CHECK_INTERVAL = 10_s;

EebusHeartBeat::EebusHeartBeat()
{
    heartbeat_received_timeout_task = task_scheduler.scheduleOnce(
        [this]() {
            emit_timeout();
        },
        HEARTBEAT_RECEIVE_TIMEOUT);
    // Send out heartbeat
    heartbeat_send_task = task_scheduler.scheduleWithFixedDelay(
        [this]() {
            send_heartbeat_to_subs();
        },
        heartbeat_interval);
    heartbeat_poll_task = task_scheduler.scheduleWithFixedDelay(
        [this]() {
            poll_heartbeats();
        },
        HEARTBEAT_POLL_CHECK_INTERVAL);
}

EebusHeartBeat::~EebusHeartBeat()
{
    task_scheduler.cancel(heartbeat_received_timeout_task);
    task_scheduler.cancel(heartbeat_send_task);
    task_scheduler.cancel(heartbeat_poll_task);
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

EebusHeartBeat::HeartbeatTarget *EebusHeartBeat::find_heartbeat_target(const FeatureAddressType &address)
{
    for (HeartbeatTarget &target : heartbeat_targets) {
        if (EEBUS_USECASE_HELPERS::compare_spine_addresses(target.address, address)) {
            return &target;
        }
    }
    return nullptr;
}

void EebusHeartBeat::request_heartbeat(HeartbeatTarget &target)
{
    target.last_read = now_us();
    send_full_read(feature_addresses.at(FeatureTypeEnumType::Generic), target.address, SpineDataTypeHandler::Function::deviceDiagnosisHeartbeatData);
}

void EebusHeartBeat::initialize_heartbeat_on_feature(FeatureAddressType &target, Usecases sending_usecase, bool expect_notify)
{
    if (find_heartbeat_target(target) == nullptr) {
        HeartbeatTarget new_target{};
        new_target.address = target;
        heartbeat_targets.push_back(new_target);
    }
    // Subscribe to heartbeat notifications from target
    if (expect_notify) {
        schedule_once_while_alive(
            [=, this]() mutable {
                const auto connection = EEBusUseCases::get_spine_connection(target);
                if (connection == nullptr) {
                    eebus.trace_fmtln("EebusHeartBeat: No connection found for heartbeat source %s", EEBUS_USECASE_HELPERS::spine_address_to_string(target).c_str());
                    return;
                }
                HeartbeatTarget *heartbeat_target = find_heartbeat_target(target);
                if (heartbeat_target == nullptr)
                    return;
                FeatureAddressType local_client = get_feature_address(feature_addresses.at(FeatureTypeEnumType::Generic));
                if (connection->is_subscribed(local_client, target)) {
                    eebus.trace_fmtln("EebusHeartBeat: Already subscribed to heartbeat notifications from target device %s", EEBUS_USECASE_HELPERS::spine_address_to_string(target).c_str());
                    return;
                }
                eebus.trace_fmtln("EebusHeartBeat: Subscribing to heartbeat notifications from target device %s", EEBUS_USECASE_HELPERS::spine_address_to_string(target).c_str());
                // This is a new connection or a new target: Start without polling until we know the result of the subscription.
                heartbeat_target->polling = false;
                eebus.usecases->node_management.subscribe_to_feature(local_client, target, FeatureTypeEnumType::DeviceDiagnosis, [this, target](bool successful) {
                    HeartbeatTarget *t = find_heartbeat_target(target);
                    if (t == nullptr) {
                        return;
                    }
                    t->polling = !successful;
                    if (!successful) {
                        eebus.trace_fmtln("EebusHeartBeat: Subscription to heartbeat of %s failed. Polling the heartbeat every %d seconds instead", EEBUS_USECASE_HELPERS::spine_address_to_string(target).c_str(), HEARTBEAT_POLL_INTERVAL.as<int>());
                    }
                });
                connection->mark_subscription_requested(local_client, target);
                // Initial read of the heartbeat (LPC 3.4.3.2)
                request_heartbeat(*heartbeat_target);
            },
            0_ms);
    }
}

void EebusHeartBeat::read_heartbeat_once(const FeatureAddressType &target)
{
    send_full_read(feature_addresses.at(FeatureTypeEnumType::Generic), target, SpineDataTypeHandler::Function::deviceDiagnosisHeartbeatData);
}

bool EebusHeartBeat::is_new_heartbeat(HeartbeatTarget &target, const DeviceDiagnosisHeartbeatDataType &data)
{
    const bool has_counter = data.heartbeatCounter.has_value();
    const bool has_timestamp = data.timestamp.has_value();
    if (!has_counter && !has_timestamp) {
        // Nothing to compare. The data is not compliant (IG-LPC 3.7), accept it as before.
        return true;
    }
    const uint64_t counter = has_counter ? data.heartbeatCounter.get() : 0;
    const std::string timestamp = has_timestamp ? data.timestamp.get() : std::string{};
    const bool is_new = !target.data_received || (has_counter && counter != target.last_counter) || (has_timestamp && timestamp != target.last_timestamp);
    target.data_received = true;
    target.last_counter = counter;
    target.last_timestamp = timestamp;
    return is_new;
}

void EebusHeartBeat::poll_heartbeats()
{
    for (auto it = heartbeat_targets.begin(); it != heartbeat_targets.end();) {
        // Forget targets whose connection is gone. They are added again when the peer reconnects.
        if (EEBusUseCases::get_spine_connection(it->address) == nullptr) {
            it = heartbeat_targets.erase(it);
            continue;
        }
        const micros_t last_activity = it->last_received > it->last_read ? it->last_received : it->last_read;
        const micros_t interval = it->polling ? HEARTBEAT_POLL_INTERVAL : HEARTBEAT_NOTIFY_MISSING_INTERVAL;
        if (deadline_elapsed(last_activity + interval)) {
            if (!it->polling) {
                eebus.trace_fmtln("EebusHeartBeat: No recent heartbeat notification from %s. Reading heartbeat", EEBUS_USECASE_HELPERS::spine_address_to_string(it->address).c_str());
            }
#ifdef EEBUS_TRACE_SUPER_VERBOSE
            else {
                eebus.trace_fmtln("EebusHeartBeat: Polling heartbeat from %s", EEBUS_USECASE_HELPERS::spine_address_to_string(it->address).c_str());
            }
#endif
            request_heartbeat(*it);
        }
        ++it;
    }
}

size_t EebusHeartBeat::subscribe_to_actor_heartbeat(SpineConnection *conn, const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor, Usecases sending_usecase)
{
    // Not a DeviceDiagnosis client: The actor may use a Generic client instead (e.g. SMA Sunny Home Manager 2.0)
    std::vector<FeatureAddressType> servers = conn->get_address_of_feature(FeatureTypeEnumType::DeviceDiagnosis, RoleType::server, use_case_name, use_case_actor);

    if (servers.empty() && conn->peer_supports_use_case(use_case_name, use_case_actor)) {
        // Not compliant: No DeviceDiagnosis server on the entity of the actor. Use the first one of the device, like eebus-go.
        std::vector<FeatureAddressType> device_servers = conn->get_addresses_of_feature(FeatureTypeEnumType::DeviceDiagnosis, RoleType::server);
        if (!device_servers.empty()) {
            eebus.trace_fmtln("EebusHeartBeat: %s of %s has no DeviceDiagnosis server on its entity. Falling back to %s", use_case_actor.c_str(), use_case_name.c_str(), EEBUS_USECASE_HELPERS::spine_address_to_string(device_servers.front()).c_str());
            servers.push_back(device_servers.front());
        }
    }

    if (servers.empty() && conn->peer_supports_use_case(use_case_name, use_case_actor)) {
        eebus.trace_fmtln("EebusHeartBeat: Peer announces %s of %s, but no heartbeat source was found", use_case_actor.c_str(), use_case_name.c_str());
    }

    for (FeatureAddressType &server : servers) {
        initialize_heartbeat_on_feature(server, sending_usecase, true);
    }
    return servers.size();
}

void EebusHeartBeat::update_heartbeat_interval(seconds_t interval)
{
    if (interval <= 0_s) {
        eebus.trace_fmtln("EebusHeartBeat: Ignoring invalid heartbeat interval of %d seconds", interval.as<int>());
        return;
    }
    heartbeat_interval = interval;
    task_scheduler.cancel(heartbeat_send_task);
    heartbeat_send_task = task_scheduler.scheduleWithFixedDelay(
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
#ifdef EEBUS_TRACE_SUPER_VERBOSE
            eebus.trace_fmtln("EebusHeartBeat: Command identified as DeviceDiagnosisHeartbeatData with a read command");
#endif
            response["deviceDiagnosisHeartbeatData"] = read_heartbeat();
            return {true, true, CmdClassifierType::reply};
        case CmdClassifierType::notify:
        case CmdClassifierType::reply: {
            const DeviceDiagnosisHeartbeatDataType &heartbeat_data = data->devicediagnosisheartbeatdatatype.get();
            if (header.addressSource.has_value()) {
                if (HeartbeatTarget *target = find_heartbeat_target(header.addressSource.get())) {
                    if (!is_new_heartbeat(*target, heartbeat_data)) {
                        eebus.trace_fmtln("EebusHeartBeat: Ignoring stale heartbeat from %s", EEBUS_USECASE_HELPERS::spine_address_to_string(target->address).c_str());
                        return {true, false};
                    }
                    target->last_received = now_us();
                }
            }
            emit_heartbeat_received(header.addressSource.has_value() ? header.addressSource.get() : FeatureAddressType{});
            return {true, false};
        }
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
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    auto subs = eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis), heartbeat_data, "deviceDiagnosisHeartbeatData");
    if (subs > 0) {
        eebus.trace_fmtln("heartbeat_sent to %d subscribers", subs);
    }
#else
    eebus.usecases->inform_subscribers(entity_address, feature_addresses.at(FeatureTypeEnumType::DeviceDiagnosis), heartbeat_data, "deviceDiagnosisHeartbeatData");
#endif
}

void EebusHeartBeat::emit_heartbeat_received(const FeatureAddressType &source)
{
    task_scheduler.cancel(heartbeat_received_timeout_task);
    for (EebusUsecase *uc : registered_usecases) {
        uc->receive_heartbeat(source);
    }
    heartbeat_received_timeout_task = task_scheduler.scheduleOnce(
        [this]() {
            emit_timeout();
        },
        HEARTBEAT_RECEIVE_TIMEOUT);
}
