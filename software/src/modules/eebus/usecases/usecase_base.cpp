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
