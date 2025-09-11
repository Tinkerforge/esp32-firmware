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

#define EVENT_LOG_PREFIX "meters_mbtcp"

#include "meters_modbus_tcp.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "options.h"
#include "meter_modbus_tcp.h"
#include "meter_modbus_tcp_specs.h"
#include "modules/meters/meter_value_id.h"
#include "modules/modbus_tcp_client/modbus_register_address_mode.enum.h"

#include "gcc_warnings.h"

void MetersModbusTCP::pre_setup()
{
    this->trace_buffer_index = logger.alloc_trace_buffer("meters_mbtcp", 8192);

    table_prototypes.push_back({MeterModbusTCPTableID::None, *Config::Null()});

    table_custom_registers_prototype = Config::Object({
        {"rtype", Config::Enum(ModbusRegisterType::HoldingRegister)},
        {"addr", Config::Uint16(0)},
        {"vtype", Config::Enum(ModbusValueType::U16)},
        {"off", Config::Float(0.0f)},
        {"scale", Config::Float(1.0f)},
        {"id", Config::Uint16(static_cast<uint16_t>(MeterValueID::NotSupported))},
    });

    table_prototypes.push_back({MeterModbusTCPTableID::Custom, Config::Object({
        {"device_address", Config::Uint8(1)},
        {"register_address_mode", Config::Enum(ModbusRegisterAddressMode::Address)},
        {"registers", Config::Array({},
            &table_custom_registers_prototype,
            0, OPTIONS_METERS_MODBUS_TCP_MAX_CUSTOM_REGISTERS(), Config::type_id<Config::ConfObject>()
        )}
    })});

    get_meter_modbus_tcp_table_prototypes(&table_prototypes);

    config_prototype = Config::Object({
        {"display_name",   Config::Str("", 0, 32)},
        {"location",       Config::Enum(MeterLocation::Unknown)},
        {"host",           Config::Str("", 0, 64)},
        {"port",           Config::Uint16(502)},
        {"table",          Config::Union<MeterModbusTCPTableID>(
            *Config::Null(),
            MeterModbusTCPTableID::None,
            table_prototypes.data(),
            static_cast<uint8_t>(table_prototypes.size())
        )},
    });

    errors_prototype = Config::Object({
        {"timeout", Config::Uint32(0)},
    });

    meters.register_meter_generator(get_class(), this);
}

[[gnu::const]]
MeterClassID MetersModbusTCP::get_class() const
{
    return MeterClassID::ModbusTCP;
}

IMeter *MetersModbusTCP::new_meter(uint32_t slot, Config *state, Config *errors)
{
    return new MeterModbusTCP(slot, state, errors, modbus_tcp_client.get_pool(), trace_buffer_index);
}

const Config *MetersModbusTCP::get_config_prototype()
{
    return &config_prototype;
}

const Config *MetersModbusTCP::get_state_prototype()
{
    return Config::Null();
}

const Config *MetersModbusTCP::get_errors_prototype()
{
    return &errors_prototype;
}

void MetersModbusTCP::trace_timestamp()
{
    if (last_trace_timestamp < 0_us || deadline_elapsed(last_trace_timestamp + 1_s)) {
        last_trace_timestamp = now_us();
        logger.trace_timestamp(trace_buffer_index);
    }
}
