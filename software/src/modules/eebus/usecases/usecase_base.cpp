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

#include "usecase_base.h"
#include "../eebus.h"
#include "../eebus_usecases.h"

extern EEBus eebus;

void EebusUsecase::send_full_read(AddressFeatureType sending_feature, FeatureAddressType receiver, SpineDataTypeHandler::Function function) const
{
    String function_name = SpineDataTypeHandler::function_to_string(function);
    FeatureAddressType sender{};
    sender.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    sender.entity = this->entity_address;
    sender.feature = sending_feature;
    eebus.trace_fmtln("%s sent read of %s to target device %s", EEBUS_USECASE_HELPERS::spine_address_to_string(sender).c_str(), function_name.c_str(), EEBUS_USECASE_HELPERS::spine_address_to_string(receiver).c_str());
    ElementTagType data{};
    BasicJsonDocument<ArduinoJsonPsramAllocator> message(256);
    JsonObject dst = message.to<JsonObject>();
    dst.createNestedObject(function_name);
    eebus.usecases->send_spine_message(receiver, sender, message.as<JsonVariantConst>(), CmdClassifierType::read, true);
}
UseCaseInformationDataType EebusUsecase::get_usecase_information() const
{
    if (usecase_actor.empty() || usecase_name.empty() || usecase_version.empty() || supported_scenarios.empty()) {
        return {};
    }
    UseCaseInformationDataType evcc_usecase;
    evcc_usecase.actor = usecase_actor;

    UseCaseSupportType evcc_usecase_support;
    evcc_usecase_support.useCaseName = usecase_name;
    evcc_usecase_support.useCaseVersion = usecase_version;
    // All 8 scenarios supported (see spec chapter 2.3)
    evcc_usecase_support.scenarioSupport = supported_scenarios;

    evcc_usecase_support.useCaseDocumentSubRevision = "release";
    evcc_usecase.useCaseSupport->push_back(evcc_usecase_support);

    FeatureAddressType evcc_usecase_feature_address;
    evcc_usecase_feature_address.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    evcc_usecase_feature_address.entity = entity_address;
    evcc_usecase.address = evcc_usecase_feature_address;
    return evcc_usecase;
}

void EebusUsecase::set_feature_address(AddressFeatureType feature_address, FeatureTypeEnumType feature_type)
{
    // Setting a feature address or feature type twice is illegal behavior and should cause a crash
    for (auto pair : feature_addresses) {
        assert(pair.first != feature_type);
        assert(pair.second != feature_address);
    }
    feature_addresses[feature_type] = feature_address;
}

FeatureTypeEnumType EebusUsecase::get_feature_by_address(AddressFeatureType feature_address) const
{
    for (auto pair : feature_addresses) {
        if (pair.second == feature_address) {
            return pair.first;
        }
    }
    return FeatureTypeEnumType::EnumUndefined;
}
NodeManagementDetailedDiscoveryEntityInformationType EebusUsecase::build_entity_info(const EntityTypeEnumType type, const char *label) const
{
    NodeManagementDetailedDiscoveryEntityInformationType entity{};
    entity.description->entityAddress->entity = entity_address;
    entity.description->entityType = type; // Should be set by the use case
    if (label != nullptr) {
        entity.description->label = label; // The label of the entity. This is optional but recommended.
    }
    return entity;
}
NodeManagementDetailedDiscoveryFeatureInformationType EebusUsecase::build_feature_information(const FeatureTypeEnumType feature_type, const RoleType role) const
{
    NodeManagementDetailedDiscoveryFeatureInformationType feature{};
    feature.description->featureAddress->entity = entity_address;
    feature.description->featureAddress->feature = feature_addresses.at(feature_type);
    feature.description->featureType = feature_type;
    feature.description->role = role;
    return feature;
}
FunctionPropertyType EebusUsecase::build_function_property(const FunctionEnumType function, const bool write, const bool partial_write) const
{
    FunctionPropertyType function_property{};
    function_property.function = function;
    if (write) {
        function_property.possibleOperations->write = PossibleOperationsWriteType{};
        if (partial_write) {
            function_property.possibleOperations->write->partial = ElementTagType{};
        }
    } else {
        function_property.possibleOperations->read = PossibleOperationsReadType{};
    }
    return function_property;
}
