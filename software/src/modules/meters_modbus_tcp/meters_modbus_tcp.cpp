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

#include "meter_modbus_tcp.h"
#include "meters_modbus_tcp.h"
#include "module_dependencies.h"
#include "../meters/meter_value_id.h"

#include "event_log.h"
#include "tools.h"

#include "gcc_warnings.h"

void MetersModbusTCP::pre_setup()
{
    table_prototypes.push_back({MeterModbusTCPTableID::None, *Config::Null()});

    table_prototypes.push_back({MeterModbusTCPTableID::Custom, Config::Object({
        {"device_address", Config::Uint(1, 1, 247)},
        {"register_address_mode", Config::Uint8(static_cast<uint8_t>(ModbusRegisterAddressMode::Address))},
        {"registers", Config::Array({},
            new Config{Config::Object({
                {"register_type", Config::Uint8(static_cast<uint8_t>(ModbusRegisterType::HoldingRegister))},
                {"start_address", Config::Uint32(0)},
                {"value_type", Config::Uint8(static_cast<uint8_t>(ModbusValueType::U16))},
                {"offset", Config::Float(0.0f)},
                {"scale_factor", Config::Float(1.0f)},
                {"value_id", Config::Uint16(static_cast<uint8_t>(MeterValueID::NotSupported))},
            })},
            0, METERS_MODBUS_TCP_MAX_CUSTOM_REGISTERS, Config::type_id<Config::ConfObject>()
        )}
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SungrowHybridInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(SungrowHybridInverterVirtualMeterID::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SungrowStringInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(SungrowStringInverterVirtualMeterID::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SolarmaxMaxStorage, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(SolarmaxMaxStorageVirtualMeterID::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::VictronEnergyGX, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(VictronEnergyGXVirtualMeterID::None))},
        {"device_address", Config::Uint(100, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::DeyeHybridInverterLowVoltage, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(DeyeHybridInverterVirtualMeterID::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::DeyeHybridInverterHighVoltage, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(DeyeHybridInverterVirtualMeterID::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    Config table_union = Config::Union<MeterModbusTCPTableID>(
        *Config::Null(),
        MeterModbusTCPTableID::None,
        table_prototypes.data(),
        static_cast<uint8_t>(table_prototypes.size()));

    config_prototype = Config::Object({
        {"display_name",   Config::Str("", 0, 32)},
        {"host",           Config::Str("", 0, 64)},
        {"port",           Config::Uint16(502)},
        {"table",          table_union},
    });

    meters.register_meter_generator(get_class(), this);
}

void MetersModbusTCP::setup()
{
    mb.client();

    initialized = true;
}

void MetersModbusTCP::loop()
{
    mb.task();
}

[[gnu::const]]
MeterClassID MetersModbusTCP::get_class() const
{
    return MeterClassID::ModbusTCP;
}

IMeter *MetersModbusTCP::new_meter(uint32_t slot, Config *state, Config *errors)
{
    return new MeterModbusTCP(slot, state, errors, &mb);
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
    return Config::Null();
}

ModbusTCP *MetersModbusTCP::get_modbus_tcp_handle()
{
    return &mb;
}
