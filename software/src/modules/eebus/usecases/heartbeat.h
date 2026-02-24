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
     * @param target Target entity address of the heartbeat target
     * @param sending_usecase The usecase that is requesting the heartbeat to be sent
     * @param expect_notify If true, a subscription will be created to receive heartbeat notifications
     */
    void initialize_heartbeat_on_feature(FeatureAddressType &target, Usecases sending_usecase, bool expect_notify = true);

    /**
     * @brief Update the heartbeat interval.
     *
     * This resets the notify timer and timeout timer.
     *
     * @param interval New interval in seconds (default 30s)
     */
    void update_heartbeat_interval(seconds_t interval = 30_s);

    /**
     * @brief Get list of targets the heartbeat is sent to.
     * @return Vector of feature addresses for heartbeat targets
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
    /** @brief Notify all registered usecases of heartbeat timeout */
    void emit_timeout() const;

    /** @brief Send heartbeat to all subscribers */
    void send_heartbeat_to_subs();

    /** @brief Notify all registered usecases of heartbeat reception */
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
