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

#include "meter_modbus_tcp.h"
#include "meters_modbus_tcp.h"
#include "module_dependencies.h"

#include "event_log.h"
#include "tools.h"

#include "gcc_warnings.h"

void MetersModbusTCP::pre_setup()
{
    register_element = Config::Object({
        {"ra", Config::Uint16(0)},  // register address
        {"rt", Config::Uint8(0)},   // register type: input, holding, coil, discrete
        {"vt", Config::Uint8(0)},   // value type: uint16, int16, uint32, int32, etc...
        {"s",  Config::Float(1.0)}, // scale
        {"o",  Config::Float(0.0)}, // offset
        {"mv", Config::Uint32(0)},  // MeterValueID
    });

    config_prototype = Config::Object({
        {"host",      Config::Str("", 0, 64)},
        {"port",      Config::Uint16(502)},
        {"address",   Config::Uint8(255)}, // device address
        {"registers", Config::Array(
            {},
            &register_element,
            0,
            METERS_MODBUS_TCP_REGISTER_COUNT_MAX,
            Config::type_id<Config::ConfObject>()
        )},
    });

    state_prototype = Config::Object({
        {"connected", Config::Bool(false)},
    });

    errors_prototype = Config::Object({
        {"connection_failure", Config::Uint32(0)},
    });

    meters.register_meter_generator(get_class(), this);
}

void MetersModbusTCP::setup()
{
    mb.client();
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

IMeter * MetersModbusTCP::new_meter(uint32_t slot, Config *state, Config *errors)
{
    if (instance_count >= MODBUSIP_MAX_CLIENTS) {
        logger.printfln("meters_modbus_tcp: Cannot create more than " MACRO_VALUE_TO_STRING(MODBUSIP_MAX_CLIENTS) " meters of class ModbusTCP.");
        return nullptr;
    }
    instance_count++;
    return new MeterModbusTCP(slot, state, errors, &mb);
}

const Config *MetersModbusTCP::get_config_prototype()
{
    return &config_prototype;
}

const Config *MetersModbusTCP::get_state_prototype()
{
    return &state_prototype;
}

const Config *MetersModbusTCP::get_errors_prototype()
{
    return &errors_prototype;
}

ModbusTCP *MetersModbusTCP::get_modbus_tcp_handle()
{
    return &mb;
}
