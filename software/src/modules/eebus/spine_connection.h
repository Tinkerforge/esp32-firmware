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
#include "module.h"
#include "ship_connection.h"
#include "spine_types.h"
#include <TFJson.h>
#include <TFTools/Micros.h>
#include <map>

#define SPINE_CONNECTION_MAX_JSON_SIZE 8192 // 8192 should be enough for a start
#define SPINE_CONNECTION_MAX_DEPTH 30       // Maximum depth of serialization of a json document

class ShipConnection; // Need to forward declare this here so it can be included in ship_connection.h

class SpineConnection
{
public:
    ShipConnection *ship_connection = nullptr;

    explicit SpineConnection(ShipConnection *ship_connection);
    ~SpineConnection();

    // Disallow copying of SpineConnection
    SpineConnection(const SpineConnection &other) = delete;
    const SpineConnection &operator=(const SpineConnection &other) = delete;

    /**
    * Process a received SPINE datagram and passes the data to the EEBUS Usecase.
    * @param datagram JsonVariant containing the datagram process
    * @return true if a response was created and needs to be sent, false if no response is needed or an error occurred
    */
    bool process_datagram(JsonVariant datagram);

    /**
     * Send a SPINE datagram to the peer.
     * @param payload The Data to be sent
     * @param cmd_classifier The Command classifier of the datagram
     * @param sender The FeatureAddressType of the sender of the datagram.
     * @param receiver The FeatureAddressType of the destination of the datagram.
     * @param require_ack Request an acknowledgement for the datagram. This is used to ensure that the peer received the datagram and can be used to detect if the peer is still alive.
     * @return The message counter of the sent datagram. This can be used to identify result data sent by the peer. -1 if an error occurred and the datagram was not sent.
    */
    int send_datagram(JsonVariantConst payload, CmdClassifierType cmd_classifier, const FeatureAddressType &sender, const FeatureAddressType &receiver, bool require_ack = false);

    /**
    * Check if the message counter is correct and log it if it isnt. Its not actually a problem if the message counter is lower than expected but indicates that the peer might have technical issues or has been rebooted.
    */
    void check_message_counter();

    /**
     * Check if the peer of this connection has used this address as sender before or if its otherwise known to this connection.
     * @param address The address to check if it is known.
     * @return True if the address is known, false if it is not known.
     */
    bool check_known_address(const FeatureAddressType &address);

    // SPINE TS 5.2.3.1
    // Specification recommends these be stored in non-volatile memory
    // But for now we dont do that as it is not needed
    int msg_counter = 0;          // Our message counter
    int msg_counter_received = 0; // The message counter of the last received datagram

    /**
    * The last received header of a SPINE datagram.
    */
    HeaderType received_header{};

    /**
    * The Payload of the last received SPINE datagram.
    */
    JsonVariant received_payload;

    /**
    * The response datagram to be retrieved by the SPINE Connection and sent back to the peer.
    */
    JsonVariant response_datagram;

    void update_detailed_discovery_data(const NodeManagementDetailedDiscoveryDataType &data)
    {
        detailed_discovery_data_received = true;
        detailed_discovery_data = data;
        // After receiving the discovery reply, immediately subscribe to the
        // peer's NodeManagement feature and read the use case data.
        // This matches the message ordering of working peers (e.g. devices-app)
        // and keeps aggressive peers (e.g. SMA SHM 2.0) from closing before
        // we have a chance to respond.
        subscribe_to_peer_node_management();
        send_use_case_read();
        inform_usecases_supported_functionalities();
    }
    void update_use_case_data(const NodeManagementUseCaseDataType &data)
    {
        use_case_data_received = true;
        use_case_data = data;
        trace_use_case_data();
        inform_usecases_supported_functionalities();
    }
    void update_subscription_data(const NodeManagementSubscriptionDataType &data)
    {
        subscription_data_received = true;
        subscription_data = data;
    }

    /**
     * Check if the local feature is subscribed to the remote feature, either according to the
     * subscription data of the peer or because we already requested the subscription on this connection.
     */
    bool is_subscribed(FeatureAddressType local, FeatureAddressType remote);

    /**
     * Remember that we requested a subscription from local to remote on this connection, to avoid duplicate requests.
     */
    void mark_subscription_requested(const FeatureAddressType &local, const FeatureAddressType &remote);

    [[nodiscard]] bool knows_device(const std::string &device) const;
    /**
     * Gets the addresses of a feature implemented by the remote actor of a use case.
     * Only features on the entity announced in the use case information of the actor are returned.
     * If the actor announces no entity, all matching features of the device are returned.
     * @param feature The feature to get the address for.
     * @param role What role the feature should have.
     * @param use_case_name The usecase being targeted.
     * @param use_case_actor The usecase actor being targeted.
     * @return The addresses of the matching features. Empty if none was found.
     */
    std::vector<FeatureAddressType> get_address_of_feature(FeatureTypeEnumType feature, RoleType role, const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor);

    /**
    * Gets the address of a feature under an entity.
    * @param entity_target The entity the feature has to be located on.
    * @param feature The feature to get the address for.
    * @param role What role the feature should have. Defaults to server.
    * @return The address of the feature. The feature part is unset if no feature was found.
    */
    FeatureAddressType get_address_of_feature(const std::vector<AddressEntityType> &entity_target, FeatureTypeEnumType feature, RoleType role = RoleType::server);

    /**
    * Gets the addresses of all features of a type and role on any entity of the peer device.
    * @param feature The feature to get the addresses for.
    * @param role What role the features should have.
    * @return The addresses of the matching features.
    */
    std::vector<FeatureAddressType> get_addresses_of_feature(FeatureTypeEnumType feature, RoleType role);

    /**
    * Check if the peer announced the given use case with the given actor in its use case data.
    */
    bool peer_supports_use_case(const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor);

    /**
    * Get the entities on which the peer implements the given actor of a use case, as announced in its use case data.
    * Use case information without an entity address is ignored.
    * @return The distinct entity addresses. Empty if the use case data was not received yet.
    */
    std::vector<std::vector<AddressEntityType>> get_use_case_actor_entities(const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor);

    // Subscription state

private:
    // This is to hold the state of the connection regarding discovery and usecase information.
    NodeManagementDetailedDiscoveryDataType detailed_discovery_data{};
    NodeManagementUseCaseDataType use_case_data{};
    NodeManagementSubscriptionDataType subscription_data{};
    bool detailed_discovery_data_received = false;
    bool use_case_data_received = false;
    bool subscription_data_received = false;
    bool initial_peer_discovery_started = false;
    uint32_t initial_peer_discovery_timer = 0;
    void initial_peer_discovery();

    void eebus_active(bool active) const;

    std::vector<FeatureAddressType> known_addresses;
    // Subscriptions (local client, remote server) we requested on this connection
    std::vector<std::pair<FeatureAddressType, FeatureAddressType>> requested_subscriptions;
    [[nodiscard]] FeatureAddressType complete_peer_feature_address(const FeatureAddressType &address) const;
    /** The device name of the peer from its detailed discovery data or its first message. nullptr if unknown. */
    [[nodiscard]] const std::string *get_peer_device_name() const;
    static bool use_case_information_matches(const UseCaseInformationDataType &usecase, const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor);
    uint16_t msg_counter_error_count = 0; // The number of message counter errors that have occurred. This is used to detect if the peer is still alive and if it has technical issues.
    static bool validate_header(HeaderType &header);

    // Message counter to acknowledgement deadline.
    std::map<int, micros_t> ack_waiting{};
    void check_ack_expired();
    uint64_t ack_check_timer = 0;

    uint64_t update_api_timer = 0;

    void inform_usecases_supported_functionalities();
    void trace_use_case_data() const;
    void subscribe_to_peer_node_management();
    void send_use_case_read();
};
