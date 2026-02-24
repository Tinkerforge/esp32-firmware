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

#include <vector>

#ifdef EEBUS_ENABLE_CEVC_USECASE

/**
 * @brief Coordinated EV Charging (CEVC) usecase.
 *
 * As defined in EEBus UC TS - Coordinated EV Charging V1.0.1.
 * This should have the same entity address as other entities with the EV actor.
 *
 * Actor: EV (Energy Guard)
 * Entity address: {1,1}
 *
 * Scenarios:
 *   1 (3.4.1): EV sends charging energy demand - TimeSeries singleDemand
 *   2 (3.4.2): Max power limitation - TimeSeries constraints
 *   3 (3.4.3): Incentive table - IncentiveTable for tariff data
 *   4 (3.4.4): Charging plan curve - TimeSeries plan (write from Energy Broker)
 *   5 (3.4.5): Energy Guard heartbeat - DeviceDiagnosis (via EebusHeartBeat)
 *   6 (3.4.6): Energy Broker heartbeat - DeviceDiagnosis (via EebusHeartBeat)
 *   7 (3.4.7): Energy Guard error state - DeviceDiagnosis state
 *   8 (3.4.8): Energy Broker error state - DeviceDiagnosis state
 *
 * Features (Functions):
 *   - TimeSeries: timeSeriesDescriptionListData, timeSeriesConstraintsListData, timeSeriesListData
 *   - IncentiveTable: incentiveTableDescriptionData, incentiveTableConstraintsData, incentiveTableData
 *
 * @see EEBus_UC_TS_CoordinatedEVCharging_V1.0.1.pdf
 */
class CevcUsecase final : public EebusUsecase
{
public:
    CevcUsecase();

    [[nodiscard]] Usecases get_usecase_type() const override
    {
        return Usecases::CEVC;
    }

    /**
     * @brief Handles a message for the CEVC usecase.
     * @param header SPINE header of the message.
     * @param data The actual Function call and data of the message.
     * @param response Where to write the response to.
     * @return The MessageReturn object which contains information about the returned message.
     */
    MessageReturn handle_message(HeaderType &header, SpineDataTypeHandler *data, JsonObject response) override;

    /**
     * @brief Builds and returns the UseCaseInformationDataType.
     *
     * As defined in CEVC spec 3.1.2.
     *
     * @return UseCaseInformationDataType with all 8 scenarios support
     */
    UseCaseInformationDataType get_usecase_information() override;

    [[nodiscard]] NodeManagementDetailedDiscoveryEntityInformationType get_detailed_discovery_entity_information() const override;
    [[nodiscard]] std::vector<NodeManagementDetailedDiscoveryFeatureInformationType> get_detailed_discovery_feature_information() const override;

    [[nodiscard]] std::vector<FeatureTypeEnumType> get_supported_features() const override
    {
        return {FeatureTypeEnumType::TimeSeries, FeatureTypeEnumType::IncentiveTable};
    }

    // =========================================================================
    // Public read methods for EVEntity access
    // These methods allow EVEntity to aggregate CEVC data with other EV entity data
    // =========================================================================

    /// @brief Reads the time series description list data.
    void read_time_series_description(TimeSeriesDescriptionListDataType *data) const;

    /// @brief Reads the time series constraints list data.
    void read_time_series_constraints(TimeSeriesConstraintsListDataType *data) const;

    /// @brief Reads the time series list data.
    void read_time_series_list(TimeSeriesListDataType *data) const;

    /// @brief Reads the incentive table description data.
    void read_incentive_table_description(IncentiveTableDescriptionDataType *data) const;

    /// @brief Reads the incentive table constraints data.
    void read_incentive_table_constraints(IncentiveTableConstraintsDataType *data) const;

    /// @brief Reads the incentive table data.
    void read_incentive_table_data(IncentiveTableDataType *data) const;

    /**
     * @brief Called when heartbeat timeout occurs from the Energy Broker.
     *
     * Invalidates the charging plan and incentives (Scenario 8).
     */
    void receive_heartbeat_timeout() override;

    /**
     * @brief Called when a SPINE connection discovers use cases.
     *
     * Registers for heartbeat monitoring from Energy Broker (Scenarios 5-6).
     */
    void inform_spineconnection_usecase_update(SpineConnection *conn) override;

    // =========================================================================
    // Public data structures
    // =========================================================================

    /**
     * @brief A charging demand slot entry for communicating EV charging demands.
     *
     * Used for Scenario 1: EV sends charging energy demand.
     */
    struct ChargingDemandSlot {
        seconds_t duration;  ///< Duration of this slot
        int min_power_w = 0; ///< Minimum power in this slot (W)
        int max_power_w = 0; ///< Maximum power in this slot (W)
        int opt_power_w = 0; ///< Optimal/expected power in this slot (W)
    };

    /**
     * @brief A charging plan slot entry received from the Energy Broker.
     *
     * Used for Scenario 4: Charging plan curve.
     */
    struct ChargingPlanSlot {
        seconds_t duration; ///< Duration of this slot
        int power_w = 0;    ///< Target power for this slot (W)
    };

    /**
     * @brief A power limit slot entry for communicating constraints.
     *
     * Used for Scenario 2: Max power limitation.
     */
    struct PowerConstraintSlot {
        seconds_t duration;  ///< Duration of this constraint
        int max_power_w = 0; ///< Maximum power limit (W)
    };

    /**
     * @brief An incentive tier within a time slot.
     *
     * Represents a power-based tier with associated cost/incentive value.
     */
    struct IncentiveTier {
        int lower_boundary_w = 0;     ///< Lower power boundary (W)
        int upper_boundary_w = 0;     ///< Upper power boundary (W)
        float incentive_value = 0.0f; ///< Cost or incentive value (cents/kWh or relative)
    };

    /**
     * @brief An incentive slot entry for communicating tariff information.
     *
     * Used for Scenario 3: Incentive table.
     */
    struct IncentiveSlotEntry {
        time_t start_time = 0;            ///< Start time of this slot (Unix timestamp)
        time_t end_time = 0;              ///< End time of this slot (Unix timestamp)
        std::vector<IncentiveTier> tiers; ///< Tiers within this slot
    };

    // =========================================================================
    // Public update methods for charging system integration
    // =========================================================================

    /**
     * @brief Updates the EV charging demand (Scenario 1).
     *
     * Call this when the EV's charging requirements change.
     *
     * @param min_energy_wh Minimum energy needed (Wh)
     * @param opt_energy_wh Optimal energy wanted (Wh)
     * @param max_energy_wh Maximum energy that can be stored (Wh)
     * @param target_time Target departure time (Unix timestamp)
     * @param slots Detailed charging demand slots (optional)
     */
    void update_charging_demand(int min_energy_wh, int opt_energy_wh, int max_energy_wh, time_t target_time, const std::vector<ChargingDemandSlot> &slots = {});

    /**
     * @brief Updates the power constraints (Scenario 2).
     *
     * Call this when the maximum power limits change.
     *
     * @param max_power_w Maximum power the EV can consume (W)
     * @param slots Detailed power constraint slots (optional)
     */
    void update_power_constraints(int max_power_w, const std::vector<PowerConstraintSlot> &slots = {});

    /**
     * @brief Gets the currently active charging plan received from the Energy Broker.
     * @return Vector of charging plan slots, empty if no plan received.
     */
    [[nodiscard]] const std::vector<ChargingPlanSlot> &get_charging_plan() const
    {
        return received_charging_plan;
    }

    /**
     * @brief Gets the current target power from the charging plan for a given time.
     * @param time Unix timestamp to query (0 = now)
     * @return Target power in Watts, or -1 if no plan active.
     */
    [[nodiscard]] int get_target_power_w(time_t time = 0) const;

    /**
     * @brief Gets the currently available incentives received from the Energy Broker.
     * @return Vector of incentive slots, empty if no incentives received.
     */
    [[nodiscard]] const std::vector<IncentiveSlotEntry> &get_incentives() const
    {
        return received_incentives;
    }

    /**
     * @brief Checks if a valid charging plan has been received.
     * @return true if a charging plan is active.
     */
    [[nodiscard]] bool has_charging_plan() const
    {
        return !received_charging_plan.empty() && charging_plan_valid;
    }

    /**
     * @brief Checks if valid incentive data has been received.
     * @return true if incentive data is available.
     */
    [[nodiscard]] bool has_incentives() const
    {
        return !received_incentives.empty() && incentives_valid;
    }

    // =========================================================================
    // ID Constants (for consistent IDs across the use case)
    // =========================================================================

    /// TimeSeries ID for single charging demand (Scenario 1)
    static constexpr int TIME_SERIES_ID_DEMAND = 1;
    /// TimeSeries ID for power constraints (Scenario 2)
    static constexpr int TIME_SERIES_ID_CONSTRAINTS = 2;
    /// TimeSeries ID for charging plan from broker (Scenario 4)
    static constexpr int TIME_SERIES_ID_PLAN = 3;
    /// Tariff ID for incentive table
    static constexpr int TARIFF_ID = 1;
    /// Tier ID for dynamic cost tier
    static constexpr int TIER_ID = 1;
    /// Incentive ID for absolute cost
    static constexpr int INCENTIVE_ID = 1;
    /// Maximum number of time series slots
    static constexpr int MAX_TIME_SERIES_SLOTS = 48;
    /// Maximum number of incentive slots
    static constexpr int MAX_INCENTIVE_SLOTS = 48;

private:
    // =========================================================================
    // Private feature handler methods
    // =========================================================================

    // TimeSeries Feature - Write handlers
    MessageReturn write_time_series_list(HeaderType &header, SpineOptional<TimeSeriesListDataType> data, JsonObject response);

    // IncentiveTable Feature - Write handlers
    MessageReturn write_incentive_table_description(HeaderType &header, SpineOptional<IncentiveTableDescriptionDataType> data, JsonObject response);
    MessageReturn write_incentive_table_data(HeaderType &header, SpineOptional<IncentiveTableDataType> data, JsonObject response);

    // Helper methods
    void notify_subscribers_time_series() const;
    void notify_subscribers_incentives() const;
    void update_api_state() const;

    // =========================================================================
    // Internal state - Charging demand (EV -> Broker, Scenario 1)
    // =========================================================================
    int minimum_energy_wh = 0;                             ///< Minimum energy needed
    int optimal_energy_wh = 0;                             ///< Optimal energy wanted
    int maximum_energy_wh = 0;                             ///< Maximum storable energy
    time_t target_departure_time = 0;                      ///< Target departure time
    std::vector<ChargingDemandSlot> charging_demand_slots; ///< Detailed demand slots

    // =========================================================================
    // Internal state - Power constraints (EV -> Broker, Scenario 2)
    // =========================================================================
    int max_power_constraint_w = 22000;                      ///< Maximum power constraint
    std::vector<PowerConstraintSlot> power_constraint_slots; ///< Detailed constraint slots

    // =========================================================================
    // Internal state - Charging plan (Broker -> EV, Scenario 4)
    // =========================================================================
    time_t charging_plan_start_time = 0;                  ///< Start time of received plan
    std::vector<ChargingPlanSlot> received_charging_plan; ///< Received charging plan
    bool charging_plan_valid = false;                     ///< Whether the plan is valid/active

    // =========================================================================
    // Internal state - Incentives (Broker -> EV, Scenario 3)
    // =========================================================================
    std::vector<IncentiveSlotEntry> received_incentives;         ///< Received incentive table
    bool incentives_valid = false;                               ///< Whether incentives are valid
    CurrencyEnumType incentive_currency = CurrencyEnumType::EUR; ///< Currency for incentives

    // =========================================================================
    // Internal state - Energy Broker connection (Scenarios 5-8)
    // =========================================================================
    bool energy_broker_connected = false;    ///< Whether Energy Broker is connected
    bool energy_broker_heartbeat_ok = false; ///< Whether Energy Broker heartbeat is healthy

    /**
     * @brief Invalidates all data received from the Energy Broker.
     *
     * Called when the Energy Broker goes into error state or heartbeat times out.
     */
    void invalidate_broker_data();
};

#endif // EEBUS_ENABLE_CEVC_USECASE
