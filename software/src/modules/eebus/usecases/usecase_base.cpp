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
#include "../generated/module_dependencies.h"

extern EEBus eebus;

void EebusUsecase::send_full_read(AddressFeatureType sending_feature, FeatureAddressType receiver, SpineDataTypeHandler::Function function) const
{
    String function_name = SpineDataTypeHandler::function_to_string(function);
    FeatureAddressType sender{};
    sender.device = EEBUS_USECASE_HELPERS::get_spine_device_name();
    sender.entity = this->entity_address;
    sender.feature = sending_feature;
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("%s sent read of %s to target device %s", EEBUS_USECASE_HELPERS::spine_address_to_string(sender).c_str(), function_name.c_str(), EEBUS_USECASE_HELPERS::spine_address_to_string(receiver).c_str());
#endif
    ElementTagType data{};
    BasicJsonDocument<ArduinoJsonPsramAllocator> message(256);
    JsonObject dst = message.to<JsonObject>();
    dst.createNestedObject(function_name);
    // TODO: Handle the returned messagecounter to allow handling of results
    send_spine_message(receiver, sender, message.as<JsonVariantConst>(), CmdClassifierType::read, true);
}

void EebusUsecase::entities_updated() const
{
    eebus.usecases->node_management.detailed_discovery_update();
}
void EebusUsecase::usecase_updated() const
{
    eebus.usecases->node_management.usecase_data_update();
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

void EebusUsecase::handle_result(const HeaderType &header, ResultDataType &result)
{
    if (header.msgCounterReference.isNull()) {
        eebus.trace_fmtln("Got Result without msgcounter reference. Peer %s does not seem to compliant", EEBUS_USECASE_HELPERS::spine_address_to_string(header.addressSource.get()).c_str());
        return;
    }
    const int msg_counter = header.msgCounterReference.get();
    for (auto &awaited_ack : awaited_acks) {
        if (awaited_ack.msg_counter == msg_counter) {
            if (result.errorNumber != 0) {
                awaited_ack.successful = false;
                eebus.trace_fmtln("Got Result with error %d from peer %s for msgcounter %d", result.errorNumber.get(), EEBUS_USECASE_HELPERS::spine_address_to_string(header.addressSource.get()).c_str(), msg_counter);
            } else {
                awaited_ack.successful = true;
            }
            awaited_ack.ack_received = true;

            break;
        }
    }
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
    }
    function_property.possibleOperations->read = PossibleOperationsReadType{};
    return function_property;
}

int EebusUsecase::send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, JsonVariantConst payload, CmdClassifierType cmd_classifier, bool want_ack)
{
    int msg_counter = eebus.usecases->send_spine_message(destination, sender, payload, cmd_classifier, want_ack);
    // TODO: add ack handling
    if (want_ack) {
        AwaitedAcks awaited_ack{.function = FunctionEnumType::nodeManagementSubscriptionRequestCall, .target_feature = destination, .cmd_type = CmdClassifierType::call, .msg_counter = msg_counter};
        awaited_acks.push_back(awaited_ack);
        // Clean up the awaited_ack if the usecase doesnt handle it by itself
        task_scheduler.scheduleOnce(
            [this, msg_counter] {
                for (auto it = awaited_acks.begin(); it != awaited_acks.end(); ++it) {
                    if (it->msg_counter == msg_counter) {
                        awaited_acks.erase(it);
                        break;
                    }
                }
            },
            20_s);
    }
    return msg_counter;
}

template <typename T> int EebusUsecase::send_spine_message(const FeatureAddressType &destination, FeatureAddressType &sender, T payload, CmdClassifierType cmd_classifier, const char *function_name, bool want_ack)
{
    BasicJsonDocument<ArduinoJsonPsramAllocator> doc{SPINE_CONNECTION_MAX_JSON_SIZE};
    JsonObject obj = doc.to<JsonObject>();
    obj[function_name] = payload;
    return send_spine_message(destination, sender, obj, cmd_classifier, want_ack);
}
