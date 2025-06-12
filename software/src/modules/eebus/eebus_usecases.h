
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
#include <TFJson.h>

#include "config.h"
#include "module.h"
#include "spine_connection.h"
#include "spine_types.h"

class EEBusUseCases; // Forward declaration of EEBusUseCases

/**
 * The basic Framework of a EEBUS UseCase.
 */
class UseCase
{
public:
    virtual ~UseCase() = default;
    /**
     * Handles a message for a usecase.
     * @param header SPINE header of the message. Contains information about the commandclassifier and the targeted entitiy.
     * @param data The actual Function call
     * @param response Where to write the response to. This is a JsonObject that should be filled with the response data.
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    virtual bool handle_message(SpineHeader &header, SpineDataTypeHandler &data, JsonObject response) = 0;
    virtual UseCaseInformationDataType get_usecase_information() = 0;
};

/**
 * The Default Nodemanagement UseCase. Needs to be supported by all EEBUS devices.
 */
class NodeManagementUsecase final : public UseCase
{
public:
    NodeManagementUsecase() = default;

    UseCaseInformationDataType get_usecase_information() override;

    bool handle_message(SpineHeader &header, SpineDataTypeHandler &data, JsonObject response) override;

    void set_usecaseManager(EEBusUseCases *usecases);

private:
    EEBusUseCases *usecase_interface{};
};

/**
 * The EEBUSChargingSummary UseCase as defined in EEBus UC TS - EV Charging Summary V1.0.1.
 */
class ChargingSummaryUsecase final : public UseCase
{
public:
    ChargingSummaryUsecase() = default;
    UseCaseInformationDataType get_usecase_information() override;

    bool handle_message(SpineHeader &header, SpineDataTypeHandler &data, JsonObject response) override;

private:
    UseCaseInformationDataType use_case_information;
};

/**
 * The central Interface for EEBus UseCases.
 */
class EEBusUseCases
{
public:
    EEBusUseCases();

    /**
     * Main interface for the EEBUS UseCases.
     * @param header Spine Header
     * @param data Payload of the message.
     * @param response The response object to fill with the response data.
     * @return true if a response was generated and needs to be sent, false if no response is needed.
     */
    bool handle_message(SpineHeader &header, SpineDataTypeHandler &data, JsonObject response);

    std::vector<UseCase *> usecases;
    uint8_t feature_address_node_management = 0;
    NodeManagementUsecase node_management;

    uint8_t feature_address_charging_summary = 1;
    ChargingSummaryUsecase charging_summary;
};