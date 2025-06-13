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

UseCaseInformationDataType NodeManagementUsecase::get_usecase_information()
{
    return UseCaseInformationDataType(); // This should never be used
}
bool NodeManagementUsecase::handle_message(SpineHeader &header, SpineDataTypeHandler &data, JsonObject response)
{
    if (header.cmd_classifier == CmdClassifierType::read && data.last_cmd == SpineDataTypeHandler::Function::nodeManagementUseCaseData) {
        NodeManagementUseCaseDataType node_management_usecase_data;
        for (UseCase *uc : usecase_interface->usecase_list) {
            if (uc->get_usecase_type() != UseCaseType::NodeManagement) {
                node_management_usecase_data.useCaseInformation->push_back(uc->get_usecase_information());
            }
        }
        if (node_management_usecase_data.useCaseInformation->size() > 0) {
            response["nodeManagementUseCaseData"] = node_management_usecase_data;
            if (response["nodeManagementUseCaseData"].isNull()) {
                logger.printfln("Error while writing NodeManagementUseCaseData to response");
            }
            return true;
        }
    }
    return false;
}

UseCaseInformationDataType ChargingSummaryUsecase::get_usecase_information()
{

    UseCaseInformationDataType evcs_usecase;
    evcs_usecase.actor = "EVSE"; // The actor can either be EVSE or Energy Broker but we support only EVSE

    UseCaseSupportType evcs_usecase_support;
    evcs_usecase_support.useCaseName = "evChargingSummary";
    evcs_usecase_support.useCaseVersion = "1.0.1";
    evcs_usecase_support.useCaseAvailable = true;
    evcs_usecase_support.scenarioSupport->push_back(1); //We support only scenario 1 which is defined in Chapter 2.3.1
    evcs_usecase_support.useCaseDocumentSubRevision = "release";
    evcs_usecase.useCaseSupport->push_back(evcs_usecase_support);

    FeatureAddressType evcs_usecase_feature_address;
    evcs_usecase_feature_address.device = "d:_i:123456_WARP3"; //TODO: Pull this name from some global variable
    evcs_usecase_feature_address.entity->push_back(1);
    evcs_usecase_feature_address.feature = 1;
    evcs_usecase.address = evcs_usecase_feature_address;
    return evcs_usecase;
}
bool ChargingSummaryUsecase::handle_message(SpineHeader &header, SpineDataTypeHandler &data, JsonObject response)
{
    return false;
}

void NodeManagementUsecase::set_usecaseManager(EEBusUseCases *new_usecase_interface)
{
    usecase_interface = new_usecase_interface;
}

EEBusUseCases::EEBusUseCases()
{
    node_management = NodeManagementUsecase();
    node_management.set_usecaseManager(this);
    usecase_list.push_back(&node_management);

    charging_summary = ChargingSummaryUsecase();
    usecase_list.push_back(&charging_summary);
}
bool EEBusUseCases::handle_message(SpineHeader &header, SpineDataTypeHandler &data, JsonObject response)
{
    if (header.destination_feature == feature_address_node_management) {

        logger.printfln("EEBus: Received message for NodeManagementUsecase");
        //logger.printfln("Function called: %s", data.function_to_string(data.last_cmd).c_str());
        return node_management.handle_message(header, data, response);
    }
    if (header.destination_feature == feature_address_charging_summary) {
        logger.printfln("EEBus: Received message for ChargingSummaryUsecase");
        //logger.printfln("Function called: %s", data.function_to_string(data.last_cmd).c_str());
        return charging_summary.handle_message(header, data, response);
    }

    return false;
}
