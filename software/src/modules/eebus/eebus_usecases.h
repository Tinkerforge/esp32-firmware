
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
#include "spine_types.h"
#include "spine_connection.h"

// The basic skeleton of a UseCase
class UseCase
{
public:
    virtual ~UseCase() = default;
    virtual JsonVariant read() = 0;
    virtual void subscribe() = 0;
    virtual UseCaseInformationDataType get_usecase_information() = 0;
};

// NodeManagement not quite a full usecase but it is required
class NodeManagementUsecase final : public UseCase
{
public:
    NodeManagementUsecase();

    JsonVariant read() override;
    void subscribe() override;
    UseCaseInformationDataType get_usecase_information() override;

    NodeManagementUseCaseDataType get_usecases();
};

class ChargingSummaryUsecase final : public UseCase
{
public:
    ChargingSummaryUsecase() = default;
    JsonVariant read() override;
    void subscribe() override;
    UseCaseInformationDataType get_usecase_information() override;

private:
    UseCaseInformationDataType use_case_information;
};

// Handles all usecases and is the primary interface to all usecases
class EEBusUseCases
{
public:
    EEBusUseCases() = default;

    bool handle_message(SpineHeader &header, SpineDataTypeHandler &data, JsonObject response);

    std::vector<UseCase *> usecases;
    NodeManagementUsecase node_management;


};