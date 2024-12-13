/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#define EVENT_LOG_PREFIX "batteries_mbtcp"

#include "batteries_modbus_tcp.h"

#include "battery_modbus_tcp.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modules/modbus_tcp_client/modbus_register_address_mode.enum.h"
#include "tools.h"

#include "gcc_warnings.h"

void BatteriesModbusTCP::pre_setup()
{
    table_prototypes.push_back({BatteryModbusTCPTableID::None, *Config::Null()});

    table_custom_registers_prototype = Config::Object({
        {"rtype", Config::Uint8(static_cast<uint8_t>(ModbusRegisterType::HoldingRegister))},
        {"addr", Config::Uint16(0)},
        {"value", Config::Uint16(0)},
    });

    table_prototypes.push_back({BatteryModbusTCPTableID::Custom, Config::Object({
        {"enable_grid_charge", Config::Object({
            {"device_address", Config::Uint(1, 1, 247)},
            {"register_address_mode", Config::Uint8(static_cast<uint8_t>(ModbusRegisterAddressMode::Address))},
            {"registers", Config::Array({},
                &table_custom_registers_prototype,
                0,
                BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTERS,
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"disable_grid_charge", Config::Object({
            {"device_address", Config::Uint(1, 1, 247)},
            {"register_address_mode", Config::Uint8(static_cast<uint8_t>(ModbusRegisterAddressMode::Address))},
            {"registers", Config::Array({},
                &table_custom_registers_prototype,
                0,
                BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTERS,
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"enable_discharge", Config::Object({
            {"device_address", Config::Uint(1, 1, 247)},
            {"register_address_mode", Config::Uint8(static_cast<uint8_t>(ModbusRegisterAddressMode::Address))},
            {"registers", Config::Array({},
                &table_custom_registers_prototype,
                0,
                BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTERS,
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"disable_discharge", Config::Object({
            {"device_address", Config::Uint(1, 1, 247)},
            {"register_address_mode", Config::Uint8(static_cast<uint8_t>(ModbusRegisterAddressMode::Address))},
            {"registers", Config::Array({},
                &table_custom_registers_prototype,
                0,
                BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTERS,
                Config::type_id<Config::ConfObject>()
            )},
        })},
    })});

    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 32)},
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"table", Config::Union<BatteryModbusTCPTableID>(*Config::Null(),
            BatteryModbusTCPTableID::None,
            table_prototypes.data(),
            static_cast<uint8_t>(table_prototypes.size())
        )},
    });

    /*errors_prototype = Config::Object({
        {"timeout", Config::Uint32(0)},
    });*/

    batteries.register_battery_generator(get_class(), this);
}

[[gnu::const]] BatteryClassID BatteriesModbusTCP::get_class() const
{
    return BatteryClassID::ModbusTCP;
}

IBattery *BatteriesModbusTCP::new_battery(uint32_t slot, Config *state, Config *errors)
{
    return new BatteryModbusTCP(slot, state, errors, modbus_tcp_client.get_pool());
}

const Config *BatteriesModbusTCP::get_config_prototype()
{
    return &config_prototype;
}

const Config *BatteriesModbusTCP::get_state_prototype()
{
    return Config::Null();
}

const Config *BatteriesModbusTCP::get_errors_prototype()
{
    return Config::Null();//&errors_prototype;
}
