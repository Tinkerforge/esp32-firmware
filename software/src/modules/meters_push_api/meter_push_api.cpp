/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "meter_push_api.h"
#include "module_dependencies.h"

#include "api.h"
#include "modules/meters/meter_value_id.h"

#include "gcc_warnings.h"

_ATTRIBUTE((const))
MeterClassID MeterPushAPI::get_class() const
{
    return MeterClassID::PushAPI;
}

void MeterPushAPI::setup()
{
    Config *value_ids = static_cast<Config *>(config->get("value_ids"));
    ssize_t id_count = value_ids->count();
    if (id_count < 0) {
        logger.printfln("meter_push_api: Invalid ID count: %i", id_count);
        return;
    }

    value_count = static_cast<uint32_t>(id_count);
    uint16_t value_count_u16 = static_cast<uint16_t>(value_count);

    MeterValueID *ids = static_cast<MeterValueID *>(malloc(value_count * sizeof(MeterValueID)));
    for (uint16_t i = 0; i < value_count_u16; i++) {
        ids[i] = static_cast<MeterValueID>(value_ids->get(i)->asUint());
    }
    meters.declare_value_ids(slot, ids, value_count);
    free(ids);

    push_values = Config::Array({},
        meters.get_config_float_nan_prototype(),
        value_count_u16, value_count_u16, Config::type_id<Config::ConfFloat>()
    );
}

void MeterPushAPI::register_urls(const String &base_url)
{
    api.addCommand(base_url + "push_values", &push_values, {}, [this](){
        meters.update_all_values(this->slot, &push_values);
    }, false);
}
