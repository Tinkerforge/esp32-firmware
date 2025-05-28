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

#include "eebus_usecases.h"

#include "build.h"
#include "eebus.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"

NodeManagementUsecase::NodeManagementUsecase()
{
}

UseCaseInformationDataType NodeManagementUsecase::get_usecase_information()
{
    return UseCaseInformationDataType(); // This should never be used
}

JsonVariant NodeManagementUsecase::read()
{
    return JsonVariant();
}
void NodeManagementUsecase::subscribe()
{
}
NodeManagementUseCaseDataType NodeManagementUsecase::get_usecases()
{
    NodeManagementUseCaseDataType data;
    std::vector<UseCaseInformationDataType> usecases;

    // This is done for testing. For the final implementation each usecase should report this information themselves
    UseCaseInformationDataType evcs_usecase;
    evcs_usecase.actor = "EVCS";


    UseCaseSupportType evcs_usecase_support;
    evcs_usecase_support.useCaseName = "evChargingSummary";
    evcs_usecase_support.useCaseVersion = "1.0.1";
    evcs_usecase_support.useCaseAvailable = true;
    evcs_usecase_support.scenarioSupport->push_back(1);
    evcs_usecase_support.useCaseDocumentSubRevision = "release";
    evcs_usecase.useCaseSupport->push_back(evcs_usecase_support);

    FeatureAddressType evcs_usecase_feature_address;
    evcs_usecase_feature_address.device = "d:_i:123456_WARP3";
    evcs_usecase_feature_address.entity->push_back(1);
    evcs_usecase_feature_address.feature = 1;
    evcs_usecase.address = evcs_usecase_feature_address;

    usecases.push_back(evcs_usecase);
    data.useCaseInformation = usecases;
    return data;
}

bool EEBusUseCases::handle_message(SpineHeader &header, SpineDataTypeHandler &data, JsonVariant response)
{
    if (header.destination_feature == 0 && header.destination_entity[0] == 0) {
        logger.printfln("EEBus: Received message for NodeManagementUsecase");
        logger.printfln("Function called: %s", data.function_to_string(data.last_cmd).c_str());

        if (!response["nodeManagementUseCaseData"].set(node_management.get_usecases())) {
            logger.printfln("Could not set response");
        }


        return true;
    }

    return false;
}
