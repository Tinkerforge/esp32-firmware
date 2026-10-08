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

#include "usecase_base.h"

/**
 * @brief Heartbeat handler for EEBUS entities.
 *
 * Manages heartbeat functionality for EEBUS connections. Each entity that requires
 * heartbeat support has its own EebusHeartBeat instance. Usecases that utilize
 * heartbeat must register with the heartbeat assigned to their entity.
 *
 * The heartbeat mechanism ensures connection liveness detection:
 * - Sends periodic heartbeat notifications to subscribers
 * - Monitors incoming heartbeats and triggers timeout callbacks
 * - Supports multiple usecases with the lowest configured interval
 *
 * @see SPINE specification section on DeviceDiagnosis feature
 */
class EebusHeartBeat : public EebusUsecase
{
public:
    EebusHeartBeat();
    ~EebusHeartBeat();

    /**
     * @brief Read the current heartbeat information.
     * @return DeviceDiagnosisHeartbeatDataType containing counter, timeout, and timestamp
     */
    DeviceDiagnosisHeartbeatDataType read_heartbeat();

    /**
     * @brief Initialize a heartbeat on a feature.
     *
     * This triggers a subscription to the heartbeat feature on the target.
     * If a target wants a heartbeat from us, it has to create a subscription.
     *
     * @param target Address of the remote DeviceDiagnosis server feature providing the heartbeat
     * @param sending_usecase The usecase that is requesting the heartbeat to be sent
     * @param expect_notify If true, a subscription will be created to receive heartbeat notifications
     */
    void initialize_heartbeat_on_feature(FeatureAddressType &target, Usecases sending_usecase, bool expect_notify = true);

    /**
     * @brief Read the heartbeat of a remote DeviceDiagnosis server once, without subscribing or supervising it.
     *
     * The reply is a sign of life of the peer like any other heartbeat.
     */
    void read_heartbeat_once(const FeatureAddressType &target);

    /**
     * @brief Subscribe to the heartbeat of a remote use case actor.
     *
     * Looks up the DeviceDiagnosis server feature on the entity of the remote actor
     * (e.g. the Energy Guard for LPC/LPP) and subscribes to its heartbeat.
     * The remote actor is not required to have a DeviceDiagnosis client feature,
     * it may use a Generic client feature instead (LPC IG 3.3, SPINE resource spec 4.3.11.6).
     *
     * @param conn The SPINE connection to the peer
     * @param use_case_name Name of the use case, e.g. "limitationOfPowerConsumption"
     * @param use_case_actor The remote actor, e.g. "EnergyGuard"
     * @param sending_usecase The usecase that is requesting the heartbeat
     * @return Number of heartbeat sources found
     */
    size_t subscribe_to_actor_heartbeat(SpineConnection *conn, const UseCaseNameType &use_case_name, const UseCaseActorType &use_case_actor, Usecases sending_usecase);

    /**
     * @brief Update the interval our own heartbeat is sent to subscribers with.
     *
     * Restarts the periodic notify timer with the new interval. The interval is also
     * announced as heartbeatTimeout in our heartbeat data. The timeout for heartbeats
     * received from peers is not affected.
     *
     * @param interval New interval in seconds (default 30s). Must be greater than 0.
     */
    void update_heartbeat_interval(seconds_t interval = 30_s);

    /**
     * @brief Get list of targets the heartbeat is sent to.
     * @return Vector of feature addresses for heartbeat targets
     */
    [[nodiscard]] std::vector<FeatureAddressType> get_heartbeat_targets() const
    {
        std::vector<FeatureAddressType> addresses;
        addresses.reserve(heartbeat_targets.size());
        for (const HeartbeatTarget &target : heartbeat_targets) {
            addresses.push_back(target.address);
        }
        return addresses;
    }

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::HEARTBEAT;
    }

    /**
     * @brief Handle messages designated for devicediagnosis:heartbeat feature.
     * @param header SPINE message header
     * @param data Parsed SPINE data
     * @param response JSON object to build response into
     * @return MessageReturn indicating handling status
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::DeviceDiagnosis, FeatureTypeEnumType::Generic}; // Generic is the client feature needed for reads
    }

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    /**
     * @brief Register a usecase to receive heartbeat events.
     *
     * Registered usecases will receive callbacks for heartbeat reception and timeout.
     *
     * @param usecase Pointer to usecase to register
     */
    void register_usecase_for_heartbeat(EebusUsecase *usecase)
    {
        registered_usecases.push_back(usecase);
    }

    /**
     * @brief Enable/disable automatic subscription to heartbeat reads on new connections.
     * @param enable If true, automatically subscribe to heartbeat reads
     */
    void set_autosubscribe(bool enable)
    {
        autosubscribe = enable;
    }

private:
    /** @brief A remote DeviceDiagnosis server we receive heartbeats from */
    struct HeartbeatTarget {
        FeatureAddressType address{};
        micros_t last_received = 0_us;    ///< Last new heartbeat (notify or reply) received from this target
        micros_t last_read = 0_us;        ///< Last heartbeat read request sent to this target
        bool polling = false;             ///< Subscription failed, poll the heartbeat periodically
        bool data_received = false;       ///< last_counter and last_timestamp are valid
        uint64_t last_counter = 0;
        std::string last_timestamp{};
    };

    /**
     * @brief Check if heartbeat data of a target is new.
     *
     * A reply to a read returns the last heartbeat again, even if the peer stopped sending heartbeats.
     * Such a stale heartbeat must not be treated as a sign of life (IG-LPC 3.7: the timestamp has to be checked).
     * A heartbeat is new if its counter or timestamp differ from the last one.
     */
    static bool is_new_heartbeat(HeartbeatTarget &target, const DeviceDiagnosisHeartbeatDataType &data);

    /** @brief Notify all registered usecases of heartbeat timeout */
    void emit_timeout() const;

    /** @brief Send heartbeat to all subscribers */
    void send_heartbeat_to_subs();

    /** @brief Notify all registered usecases of heartbeat reception */
    void emit_heartbeat_received(const FeatureAddressType &source);

    /** @brief Find a heartbeat target by its address. Returns nullptr if unknown. */
    HeartbeatTarget *find_heartbeat_target(const FeatureAddressType &address);

    void request_heartbeat(HeartbeatTarget &target);

    /**
     * @brief Read the heartbeat of targets that do not notify us.
     *
     * Polls targets whose subscription failed (LPC 3.3.4) and reads the heartbeat of
     * subscribed targets that did not send a notification for an unusually long time.
     * Removes targets whose connection is gone.
     */
    void poll_heartbeats();

    std::vector<HeartbeatTarget> heartbeat_targets{};
    std::vector<Usecases> usecases_enabled{};
    std::vector<EebusUsecase *> registered_usecases{};

    seconds_t heartbeat_interval = 30_s;
    uint32_t heartbeat_counter = 0;

    uint64_t heartbeat_received_timeout_task = 0;
    uint64_t heartbeat_send_task = 0;
    uint64_t heartbeat_poll_task = 0;

    bool autosubscribe = false;
};
