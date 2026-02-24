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

/*
This file contains the definitions of the EEBUS Usecases as defined in the EEBUS Usecase Technical Specifications.
The usecase names may have been shortened and the spec is referred to as much as possible:
NMC -> Node Management and Control. Implemented according to EEBUS_SPINE_TS_ProtocolSpecification.pdf, technically nodemanagement is not a usecase but it behaves like one in many ways and is therefore implemented alongside
EVCS -> Electric Vehicle Charging Summary. Implemented according to EEBUS_UC_TS_CoordinatedEVCharging_V1.0.1.pdf
EVCEM -> EV Charging Electricity Measurement. Implemented according to EEBUS_UC_TS_EVChargingElectricityMeasurement_V1.0.1.pdf
EVCC -> EV Commissioning and Configuration. Implemented according to EEBus_UC_TS_EVCommissioningAndConfiguration_V1.0.1.pdf
EVSECC -> EVSE Commissioning and Configuration. Implemented according to EEBus_UC_TS_EVSECommissioningAndConfiguration_V1.0.0.pdf
LPC -> Limitation of Power Consumption. Implemented according to EEBUS_UC_TS_LimitationOfPowerConsumption_V1.0.0.pdf
CEVC -> Coordinate EV Charging. Implemented according to EEBUS_UC_TS_CoordinatedEVCharging_V1.0.1.pdf
OPEV -> Overload Protection by EV Charging Current curtailment. Implemented According to EEBUS_UC_TS_OverloadProtectionByEVChargingCurrentCurtailment_V1.0.1b.pdf
Sometimes the following references are used e.g. LPC-905, these refer to rules laid out in the spec and can be found in the according technical spec.
*/

#pragma once
#include "config.h"
#include "loadcontrol_state.enum.h"
#include "spine_connection.h"
#include "spine_types.h"
#include "usecases.enum.h"

// What usecases are enabled depends on if this is a WARP Charger or an Energy Manager
#ifdef EEBUS_MODE_EVSE
// To disabled a usecase simply comment the respective line
//#define EEBUS_ENABLE_EVCS_USECASE
#define EEBUS_ENABLE_EVCEM_USECASE
#define EEBUS_ENABLE_EVCC_USECASE
#define EEBUS_ENABLE_EVSECC_USECASE
#define EEBUS_ENABLE_LPC_USECASE
//#define EEBUS_ENABLE_CEVC_USECASE
#define EEBUS_ENABLE_MPC_USECASE
//#define EEBUS_ENABLE_OPEV_USECASE
#endif
#ifdef EEBUS_MODE_EM
#define EEBUS_ENABLE_LPP_USECASE
#define EEBUS_ENABLE_LPC_USECASE
#define EEBUS_ENABLE_MGCP_USECASE
#endif

#ifdef EEBUS_DEV_TEST_ENABLE
#define EEBUS_ENABLE_EVCEM_USECASE
#define EEBUS_ENABLE_EVCC_USECASE
#define EEBUS_ENABLE_EVSECC_USECASE
#define EEBUS_ENABLE_LPC_USECASE
#define EEBUS_ENABLE_MPC_USECASE
#define EEBUS_ENABLE_LPP_USECASE
#define EEBUS_ENABLE_EVCS_USECASE
#define EEBUS_ENABLE_CEVC_USECASE
#define EEBUS_ENABLE_OPEV_USECASE
#define EEBUS_ENABLE_MGCP_USECASE
#endif

// Configuration related to the LPC usecases
// Disable if subscription functionalities shall not be used
#define EEBUS_NODEMGMT_ENABLE_SUBSCRIPTIONS true
// The power consumption limit at startup in w. Should be the maximum limit of the Warp Charger. Is also used to tell the Energy Broker the maximum consumption limit of the device
#define EEBUS_LPC_INITIAL_ACTIVE_POWER_CONSUMPTION 22000
// Enable LPC heartbeat
#define EEBUS_LPC_ENABLE_HEARTBEAT true
#define EEBUS_LPC_HEARTBEAT_INTERVAL 30_s // Must be more than 10 seconds and less or equal to 60 seconds

using LPCState = LoadcontrolState;
using LPPState = LoadcontrolState;

class EEBusUseCases; // Forward declaration of EEBusUseCases

// Include all usecase headers - these contain the class declarations that were previously in this file
#include "usecases/entity_data.h"
#include "usecases/heartbeat.h"
#include "usecases/node_management.h"
#include "usecases/usecase_base.h"
#include "usecases/usecase_helpers.h"

#ifdef EEBUS_ENABLE_EVCS_USECASE
#include "usecases/evcs.h"
#endif

#ifdef EEBUS_ENABLE_EVCC_USECASE
#include "usecases/evcc.h"
#endif

#ifdef EEBUS_ENABLE_EVCEM_USECASE
#include "usecases/evcem.h"
#endif

#ifdef EEBUS_ENABLE_EVSECC_USECASE
#include "usecases/evsecc.h"
#endif

#if defined(EEBUS_ENABLE_LPC_USECASE) || defined(EEBUS_ENABLE_LPP_USECASE)
#include "usecases/loadcontrol.h"
#endif

#ifdef EEBUS_ENABLE_CEVC_USECASE
#include "usecases/cevc.h"
#endif

#ifdef EEBUS_ENABLE_MPC_USECASE
#include "usecases/mpc.h"
#endif

#ifdef EEBUS_ENABLE_MGCP_USECASE
#include "usecases/mgcp.h"
#endif

#ifdef EEBUS_ENABLE_OPEV_USECASE
#include "usecases/opev.h"
#endif

/**
 * The central Interface for EEBus UseCases.
 */
class EEBusUseCases
{
public:
    EEBusUseCases();

    /**
     * Main interface for the EEBUS UseCases. All EEBUS Messages are passed here and forwarded to the correct usecase entity.
     * @param header Spine Header
     * @param data Payload of the message.
     * @param connection The SPINE Connection that sent the message. This is used to send the response back to the correct connection and to identify the connection which bound or subscribed to a function.
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    void process_spine_message(HeaderType &header, SpineDataTypeHandler *data, SpineConnection *connection);

    /**
     * Informs the subscribers of a feature about a change in the data.
     * @param entity the entity address of the feature
     * @param feature the feature address
     * @param data The Data the subscribers should be informed about. This is a SpineDataTypeHandler that contains the data.
     * @param function_name Name of the function this is notifying about. Must be the spine datatype name
     * @return The number of subscribers that have been informed. 0 if no subscribers were informed.
     */
    template <typename T> size_t inform_subscribers(const std::vector<AddressEntityType> &entity, AddressFeatureType feature, T &data, const char *function_name);

    /**
     * Send a message to a spine destination.
     * @param destination Spine FeatureAddressType of the destination. A message from the destination has to have been received before.
     * @param sender Sender FeatureAddressType of the sender. The entity and feature must be set. The device can be empty and will be filled automatically if so.
     * @param payload The payload to be sent.
     * @param cmd_classifier The command classifier of the message.
     * @param want_ack If we want an acknowledgement for the message. This is used to ensure that the peer received the message and can be used to detect if the peer is still alive.
     */
    bool send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, JsonVariantConst payload, CmdClassifierType cmd_classifier, bool want_ack = false);

    template <typename T> bool send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, T payload, CmdClassifierType cmd_classifier, const char *function_name, bool want_ack = false);

    /**
     * Get a SpineConnection for a given spine address.
     * @param spine_address
     * @return Pointer to the spine connection. nullptr if no connection exists.
     */
    static SpineConnection *get_spine_connection(const FeatureAddressType &spine_address);

    BasicJsonDocument<ArduinoJsonPsramAllocator> temporary_json_doc{SPINE_CONNECTION_MAX_JSON_SIZE}; // If a temporary doc is needed, use this one.
    BasicJsonDocument<ArduinoJsonPsramAllocator> response{SPINE_CONNECTION_MAX_JSON_SIZE};           // The response document to be filled with the response data

    NodeManagementEntity node_management{};
#ifdef EEBUS_ENABLE_EVSECC_USECASE
    EvseccUsecase evse_commissioning_and_configuration{};
    EvseccUsecase *evsecc = &evse_commissioning_and_configuration;
#endif
#ifdef EEBUS_ENABLE_EVCS_USECASE
    EvcsUsecase charging_summary{};
    EvcsUsecase *evcs = &charging_summary;
#endif
#ifdef EEBUS_ENABLE_LPC_USECASE
    LpcUsecase limitation_of_power_consumption{};
    LpcUsecase *lpc = &limitation_of_power_consumption;
#endif
#ifdef EEBUS_ENABLE_LPP_USECASE
    LppUsecase limitation_of_power_production{};
    LppUsecase *lpp = &limitation_of_power_production;
#endif
#ifdef EEBUS_ENABLE_MPC_USECASE
    MpcUsecase monitoring_of_power_consumption{};
    MpcUsecase *mpc = &monitoring_of_power_consumption;
#endif
#ifdef EEBUS_ENABLE_EVCC_USECASE
    EvccUsecase ev_commissioning_and_configuration{};
    EvccUsecase *evcc = &ev_commissioning_and_configuration;
#endif
#ifdef EEBUS_ENABLE_EVCEM_USECASE
    EvcemUsecase ev_charging_electricity_measurement{};
    EvcemUsecase *evcem = &ev_charging_electricity_measurement;
#endif
#ifdef EEBUS_ENABLE_CEVC_USECASE
    CevcUsecase coordinate_ev_charging{};
    CevcUsecase *cevc = &coordinate_ev_charging;
#endif
#ifdef EEBUS_ENABLE_OPEV_USECASE
    OpevUsecase overload_protection_by_ev_charging_current_curtailment{};
    OpevUsecase *opev = &overload_protection_by_ev_charging_current_curtailment;
#endif
#ifdef EEBUS_ENABLE_MGCP_USECASE
    MgcpUsecase monitoring_of_grid_connection_point{};
    MgcpUsecase *mgcp = &monitoring_of_grid_connection_point;
#endif

    std::vector<EebusUsecase *> usecase_list{};

    EVSEEntity evse_entity{};
    EVEntity ev_entity{};
    EebusHeartBeat evse_heartbeat{};
    EebusHeartBeat ev_heartbeat{};

private:
    bool initialized = false;
    uint16_t eebus_commands_received = 0;
    uint16_t eebus_responses_sent = 0;
};
