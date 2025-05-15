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
    node_management_usecase_data_type = NodeManagementUseCaseDataType();
    node_management_usecase_data_type.useCaseInformation = UseCaseInformationDataType();
    node_management_usecase_data_type.useCaseInformation->address = FeatureAddressType();
    node_management_usecase_data_type.useCaseInformation->actor = UseCaseActorType();
    node_management_usecase_data_type.useCaseInformation->useCaseSupport = UseCaseSupportType();
    node_management_usecase_data_type.useCaseInformation->address->device = AddressDeviceType();
    node_management_usecase_data_type.useCaseInformation->address->entity = std::vector<AddressEntityType>();
    node_management_usecase_data_type.useCaseInformation->address->feature = AddressFeatureType();
    node_management_usecase_data_type.useCaseInformation->actor->usecaseactorenumtype = UseCaseActorEnumType();
    node_management_usecase_data_type.useCaseInformation->actor->enumextendtype = EnumExtendType();
    node_management_usecase_data_type.useCaseInformation->useCaseSupport->useCaseName = UseCaseNameType();
    node_management_usecase_data_type.useCaseInformation->useCaseSupport->useCaseVersion = SpecificationVersionType();
    node_management_usecase_data_type.useCaseInformation->useCaseSupport->useCaseAvailable = bool();
    node_management_usecase_data_type.useCaseInformation->useCaseSupport->scenarioSupport = UseCaseScenarioSupportType();
    node_management_usecase_data_type.useCaseInformation->useCaseSupport->useCaseDocumentSubRevision = std::string();
}