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
#include "meter_modbus_tcp.h"
#include "modules/meters/meter_value_id.h"
#include "tools.h"
#include "modbus_register_address_mode.enum.h"

#include "gcc_warnings.h"

void MetersModbusTCP::pre_setup()
{
    table_prototypes.push_back({MeterModbusTCPTableID::None, *Config::Null()});

    table_custom_registers_prototype = Config::Object({
        {"rtype", Config::Uint8(static_cast<uint8_t>(ModbusRegisterType::HoldingRegister))},
        {"addr", Config::Uint16(0)},
        {"vtype", Config::Uint8(static_cast<uint8_t>(ModbusValueType::U16))},
        {"off", Config::Float(0.0f)},
        {"scale", Config::Float(1.0f)},
        {"id", Config::Uint16(static_cast<uint8_t>(MeterValueID::NotSupported))},
    });

    table_prototypes.push_back({MeterModbusTCPTableID::Custom, Config::Object({
        {"device_address", Config::Uint(1, 1, 247)},
        {"register_address_mode", Config::Uint8(static_cast<uint8_t>(ModbusRegisterAddressMode::Address))},
        {"registers", Config::Array({},
            &table_custom_registers_prototype,
            0, METERS_MODBUS_TCP_MAX_CUSTOM_REGISTERS, Config::type_id<Config::ConfObject>()
        )}
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SungrowHybridInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(SungrowHybridInverterVirtualMeter::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SungrowStringInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(SungrowStringInverterVirtualMeter::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SolarmaxMaxStorage, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(SolarmaxMaxStorageVirtualMeter::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::VictronEnergyGX, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(VictronEnergyGXVirtualMeter::None))},
        {"device_address", Config::Uint(100, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::DeyeHybridInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(DeyeHybridInverterVirtualMeter::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::AlphaESSHybridInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(AlphaESSHybridInverterVirtualMeter::None))},
        {"device_address", Config::Uint(85, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::ShellyProEM, Config::Object({
        {"device_address", Config::Uint(1, 1, 247)},
        {"monophase_channel", Config::Uint8(static_cast<uint8_t>(ShellyEMMonophaseChannel::None))},
        {"monophase_mapping", Config::Uint8(static_cast<uint8_t>(ShellyEMMonophaseMapping::None))},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::ShellyPro3EM, Config::Object({
        {"device_address", Config::Uint(1, 1, 247)},
        {"device_profile", Config::Uint8(static_cast<uint8_t>(ShellyPro3EMDeviceProfile::Triphase))},
        {"monophase_channel", Config::Uint8(static_cast<uint8_t>(ShellyEMMonophaseChannel::None))},
        {"monophase_mapping", Config::Uint8(static_cast<uint8_t>(ShellyEMMonophaseMapping::None))},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::GoodweHybridInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(GoodweHybridInverterVirtualMeter::None))},
        {"device_address", Config::Uint(247, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SolaxHybridInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(SolaxHybridInverterVirtualMeter::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::FroniusGEN24PlusHybridInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(FroniusGEN24PlusHybridInverterVirtualMeter::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::HaileiHybridInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(HaileiHybridInverterVirtualMeter::None))},
        {"device_address", Config::Uint(85, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::FoxESSH3HybridInverter, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(FoxESSH3HybridInverterVirtualMeter::None))},
        {"device_address", Config::Uint(247, 1, 247)},
    })});

    Config siemens = Config::Object({
        {"device_address", Config::Uint(1, 1, 247)},
    });

    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC2200, siemens});
    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC3120, siemens});
    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC3200, siemens});
    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC3220, siemens});
    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC4200, siemens});
    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC4220, siemens});

    Config carlo_gavazzi = Config::Object({
        {"device_address", Config::Uint(1, 1, 247)},
    });

    Config carlo_gavazzi_single_phase = Config::Object({
        {"device_address", Config::Uint(1, 1, 247)},
        {"phase", Config::Uint8(static_cast<uint8_t>(CarloGavazziPhase::None))},
    });

    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM24DIN, carlo_gavazzi});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM24E1, carlo_gavazzi});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM100, carlo_gavazzi_single_phase});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziET100, carlo_gavazzi_single_phase});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM210, carlo_gavazzi});

    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM270, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(CarloGavazziEM270VirtualMeter::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM280, Config::Object({
        {"virtual_meter", Config::Uint8(static_cast<uint8_t>(CarloGavazziEM280VirtualMeter::None))},
        {"device_address", Config::Uint(1, 1, 247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM300, carlo_gavazzi});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziET300, carlo_gavazzi});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM510, carlo_gavazzi_single_phase});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM530, carlo_gavazzi});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM540, carlo_gavazzi});

    config_prototype = Config::Object({
        {"display_name",   Config::Str("", 0, 32)},
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
    return new MeterModbusTCP(slot, state, errors, modbus_tcp_client.get_pool());
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
