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

#include "meters_evse_v2.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

#include "gcc_warnings.h"

void MetersEVSEV2::pre_setup()
{
    config_prototype = Config::Object({
        {"display_name", Config::Str("WARP Charger", 0, 32)}
    });

    state_prototype = Config::Object({
        {"type",  Config::Uint(0)}  // 0 - not available, 1 - sdm72, 2 - sdm630, 3 - sdm72v2, ... see meter.h
    });

    errors_prototype = Config::Object({
        {"local_timeout",        Config::Uint32(0)},
        {"global_timeout",       Config::Uint32(0)},
        {"illegal_function",     Config::Uint32(0)},
        {"illegal_data_access",  Config::Uint32(0)},
        {"illegal_data_value",   Config::Uint32(0)},
        {"slave_device_failure", Config::Uint32(0)},
    });

    meters.register_meter_generator(get_class(), this);
}

[[gnu::const]]
MeterClassID MetersEVSEV2::get_class() const
{
    return MeterClassID::EVSEV2;
}

IMeter *MetersEVSEV2::new_meter(uint32_t slot, Config *state, Config * errors)
{
    if (meter_instance) {
        logger.printfln("Cannot create more than one meter of class EVSEV2.");
        return nullptr;
    }
    meter_instance = new MeterEVSEV2(slot, state, errors);
    return meter_instance;
}

[[gnu::const]]
const Config *MetersEVSEV2::get_config_prototype()
{
    return &config_prototype;
}

[[gnu::const]]
const Config *MetersEVSEV2::get_state_prototype()
{
    return &state_prototype;
}

const Config *MetersEVSEV2::get_errors_prototype()
{
    return &errors_prototype;
}

void MetersEVSEV2::update_from_evse_v2_all_data(EVSEV2MeterData *meter_data)
{
    if (!meter_instance)
        return;

    meter_instance->update_from_evse_v2_all_data(meter_data);
}

void MetersEVSEV2::energy_meter_values_callback(float power, float current[3])
{
    if (!meter_instance)
        return;

    meter_instance->energy_meter_values_callback(power, current);
}
