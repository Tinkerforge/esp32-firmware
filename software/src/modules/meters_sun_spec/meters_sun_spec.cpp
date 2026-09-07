/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

#include "meters_sun_spec.h"

#include <esp_random.h>
#include <TFJson.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "options.h"
#include "meter_sun_spec.h"
#include "modules/meters/generated/meter_location.enum.h"
#include "generated/dc_port_type.enum.h"

#include "gcc_warnings.h"

void MetersSunSpec::pre_setup()
{
    this->trace_buffer_index = logger.alloc_trace_buffer("meters_sun_spec");

    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 65)}, // 32 chars manufacturer name; space; 32 chars model name
        {"location", Config::Enum(MeterLocation::Unknown)},
        {"excluded", Config::Bool(false)},
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"device_address", Config::Uint8(1)},
        {"manufacturer_name", Config::Str("", 0, 32)},
        {"model_name", Config::Str("", 0, 32)},
        {"serial_number", Config::Str("", 0, 32)},
        {"model_id", Config::Uint16(0)}, // 0 == invalid
        {"model_instance", Config::Uint16(0)},
        {"dc_port_type", Config::Enum(DCPortType::NotImplemented)},
    });

    errors_prototype = Config::Object({
        {"timeout", Config::Uint32(0)},
        {"inconsistency", Config::Uint32(0)},
    });

    meters.register_meter_generator(get_class(), this);
}

[[gnu::const]]
MeterClassID MetersSunSpec::get_class() const
{
    return MeterClassID::SunSpec;
}

IMeter *MetersSunSpec::new_meter(uint32_t slot, Config *state, Config *errors)
{
    return new MeterSunSpec(slot, state, errors, modbus_tcp_client.get_pool());
}

[[gnu::const]]
const Config *MetersSunSpec::get_config_prototype()
{
    return &config_prototype;
}

[[gnu::const]]
const Config *MetersSunSpec::get_state_prototype()
{
    return Config::Null();
}

[[gnu::const]]
const Config *MetersSunSpec::get_errors_prototype()
{
    return &errors_prototype;
}

void MetersSunSpec::trace_timestamp()
{
    if (last_trace_timestamp < 0_us || deadline_elapsed(last_trace_timestamp + 1_s)) {
        last_trace_timestamp = now_us();
        logger.trace_timestamp(trace_buffer_index);
    }
}
