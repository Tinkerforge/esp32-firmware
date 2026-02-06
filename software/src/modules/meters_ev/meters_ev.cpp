/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "meters_ev.h"
#include "meter_ev.h"
#include "ev_data_protocol.enum.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modules/meters/meter_location.enum.h"
#include "modules/iso15118/common.h"

#include "gcc_warnings.h"

void MetersEV::pre_setup()
{
    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 32)},
        {"location", Config::Enum(MeterLocation::EV)},
    });

    state_prototype = Config::Object({
        {"protocol", Config::Enum(EVDataProtocol::None)}
    });

    errors_prototype = Config::Object({
        // No error tracking for now
    });

    meters.register_meter_generator(get_class(), this);
}

[[gnu::const]]
MeterClassID MetersEV::get_class() const
{
    return MeterClassID::ISO15118;
}

IMeter *MetersEV::new_meter(uint32_t slot, Config *state, Config *errors)
{
    if (meter_instance) {
        logger.printfln("Meter %lu: Cannot create more than one meter of class ISO15118", slot);
        return nullptr;
    }
    meter_instance = new MeterEV(slot, state, errors);
    return meter_instance;
}

[[gnu::const]]
const Config *MetersEV::get_config_prototype()
{
    return &config_prototype;
}

[[gnu::const]]
const Config *MetersEV::get_state_prototype()
{
    return &state_prototype;
}

[[gnu::const]]
const Config *MetersEV::get_errors_prototype()
{
    return &errors_prototype;
}

void MetersEV::update_from_ev_data(const EVData &data, EVDataProtocol protocol)
{
    if (!meter_instance) {
        return;
    }

    if (protocol == EVDataProtocol::None) {
        meter_instance->clear_all_values();
    } else {
        meter_instance->update_all_values(
            data.soc_present,
            data.soc_target,
            data.soc_min,
            data.soc_max,
            data.max_voltage,
            data.max_current,
            data.max_power,
            data.capacity_kwh,
            data.present_power,
            data.energy_request_kwh,
            data.remaining_time_to_target_soc,
            data.min_power,
            data.min_current
        );
        meter_instance->set_protocol(protocol);
    }
}

void MetersEV::clear_values()
{
    if (meter_instance) {
        meter_instance->clear_all_values();
    }
}
