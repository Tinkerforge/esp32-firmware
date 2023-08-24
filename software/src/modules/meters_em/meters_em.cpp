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

#include "meters_em.h"
#include "module_dependencies.h"

#include "modules/meters/meter_class_defs.h"

#include "gcc_warnings.h"

void MetersEM::pre_setup()
{
    state_prototype = Config::Object({
        {"type",  Config::Uint(0)}  // 0 - not available, 1 - sdm72, 2 - sdm630, 3 - sdm72v2, ... see meter.h
    });

    meters.register_meter_generator(METER_CLASS_LOCAL_EM, this);
}

_ATTRIBUTE((const))
uint32_t MetersEM::get_class() const
{
    return METER_CLASS_LOCAL_EM;
}

IMeter * MetersEM::new_meter(uint32_t slot, Config *state, Config *config)
{
    if (meter_instance) {
        logger.printfln("meters_em: Cannot create more than one meter of class LOCAL_EM.");
        return nullptr;
    }
    meter_instance = new MeterEM(slot, state);
    return meter_instance;
}

const Config * MetersEM::get_config_prototype()
{
    return config_prototype;
}

_ATTRIBUTE((const))
const Config * MetersEM::get_state_prototype()
{
    return &state_prototype;
}

void MetersEM::update_from_em_all_data(EnergyManagerAllData &all_data)
{
    if (!meter_instance)
        return;

    meter_instance->update_from_em_all_data(all_data);
}
