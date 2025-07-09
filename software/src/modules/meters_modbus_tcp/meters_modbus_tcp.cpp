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
#include "modules/meters/meter_location.enum.h"
#include "modules/meters/meter_value_id.h"
#include "tools.h"
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

    table_prototypes.push_back({MeterModbusTCPTableID::SungrowHybridInverter, Config::Object({
        {"virtual_meter", Config::Enum(SungrowHybridInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SungrowStringInverter, Config::Object({
        {"virtual_meter", Config::Enum(SungrowStringInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SolarmaxMaxStorage, Config::Object({
        {"virtual_meter", Config::Enum(SolarmaxMaxStorageVirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::VictronEnergyGX, Config::Object({
        {"virtual_meter", Config::Enum(VictronEnergyGXVirtualMeter::None)},
        {"device_address", Config::Uint8(100)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::DeyeHybridInverter, Config::Object({
        {"virtual_meter", Config::Enum(DeyeHybridInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::AlphaESSHybridInverter, Config::Object({
        {"virtual_meter", Config::Enum(AlphaESSHybridInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(85)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::ShellyProEM, Config::Object({
        {"device_address", Config::Uint8(1)},
        {"monophase_channel", Config::Enum(ShellyEMMonophaseChannel::None)},
        {"monophase_mapping", Config::Enum(ShellyEMMonophaseMapping::None)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::ShellyPro3EM, Config::Object({
        {"device_address", Config::Uint8(1)},
        {"device_profile", Config::Enum(ShellyPro3EMDeviceProfile::Triphase)},
        {"monophase_channel", Config::Enum(ShellyEMMonophaseChannel::None)},
        {"monophase_mapping", Config::Enum(ShellyEMMonophaseMapping::None)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::GoodweHybridInverter, Config::Object({
        {"virtual_meter", Config::Enum(GoodweHybridInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SolaxHybridInverter, Config::Object({
        {"virtual_meter", Config::Enum(SolaxHybridInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::FroniusGEN24Plus, Config::Object({
        {"virtual_meter", Config::Enum(FroniusGEN24PlusVirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::HaileiHybridInverter, Config::Object({
        {"virtual_meter", Config::Enum(HaileiHybridInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(85)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::FoxESSH3HybridInverter, Config::Object({
        {"virtual_meter", Config::Enum(FoxESSH3HybridInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(247)},
    })});

    Config siemens = Config::Object({
        {"device_address", Config::Uint8(1)},
    });

    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC2200, siemens});
    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC3120, siemens});
    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC3200, siemens});
    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC3220, siemens});
    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC4200, siemens});
    table_prototypes.push_back({MeterModbusTCPTableID::SiemensPAC4220, siemens});

    Config carlo_gavazzi = Config::Object({
        {"device_address", Config::Uint8(1)},
    });

    Config carlo_gavazzi_single_phase = Config::Object({
        {"device_address", Config::Uint8(1)},
        {"phase", Config::Enum(CarloGavazziPhase::None)},
    });

    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM24DIN, carlo_gavazzi});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM24E1, carlo_gavazzi});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM100, carlo_gavazzi_single_phase});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziET100, carlo_gavazzi_single_phase});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM210, carlo_gavazzi});

    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM270, Config::Object({
        {"virtual_meter", Config::Enum(CarloGavazziEM270VirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM280, Config::Object({
        {"virtual_meter", Config::Enum(CarloGavazziEM280VirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM300, carlo_gavazzi});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziET300, carlo_gavazzi});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM510, carlo_gavazzi_single_phase});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM530, carlo_gavazzi});
    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM540, carlo_gavazzi});

    table_prototypes.push_back({MeterModbusTCPTableID::Solaredge, Config::Object({
        {"virtual_meter", Config::Enum(SolaredgeVirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::EastronSDM630TCP, Config::Object({
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::TinkerforgeWARPCharger, *Config::Null()});

    table_prototypes.push_back({MeterModbusTCPTableID::SAXPowerHomeBasicMode, Config::Object({
        {"virtual_meter", Config::Enum(SAXPowerVirtualMeter::None)},
        {"device_address", Config::Uint8(64)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::SAXPowerHomeExtendedMode, Config::Object({
        {"virtual_meter", Config::Enum(SAXPowerVirtualMeter::None)},
        {"device_address", Config::Uint8(40)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::E3DC, Config::Object({
        {"virtual_meter", Config::Enum(E3DCVirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::HuaweiSUN2000, Config::Object({
        {"virtual_meter", Config::Enum(HuaweiSUN2000VirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::HuaweiSUN2000SmartDongle, Config::Object({
        {"virtual_meter", Config::Enum(HuaweiSUN2000SmartDongleVirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::HuaweiEMMA, Config::Object({
        {"virtual_meter", Config::Enum(HuaweiEMMAVirtualMeter::None)},
        {"device_address", Config::Uint8(0)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::CarloGavazziEM580, carlo_gavazzi});

    table_prototypes.push_back({MeterModbusTCPTableID::SolaxStringInverter, Config::Object({
        {"virtual_meter", Config::Enum(SolaxStringInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(1)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::FoxESSH3SmartHybridInverter, Config::Object({
        {"virtual_meter", Config::Enum(FoxESSH3SmartHybridInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(247)},
    })});

    table_prototypes.push_back({MeterModbusTCPTableID::FoxESSH3ProHybridInverter, Config::Object({
        {"virtual_meter", Config::Enum(FoxESSH3ProHybridInverterVirtualMeter::None)},
        {"device_address", Config::Uint8(247)},
    })});

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
