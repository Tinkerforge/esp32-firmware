/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#define EVENT_LOG_PREFIX "meters_mbtcp"

#include "meter_modbus_tcp.h"
#include "modbus_tcp_tools.h"
#include "module_dependencies.h"

#include "event_log.h"
#include "task_scheduler.h"

#include "gcc_warnings.h"

enum class ValueType {
    U16,
    S16,
    U32,
    S32,
};

struct ValueSpec {
    MeterValueID value_id;
    const char *name;
    size_t start_address;
    size_t register_count;
    ValueType value_type;
    float scale_factor;
};

static const ValueSpec value_specs[] = {
    {
        MeterValueID::EnergyActiveLSumExport,
        "Total Output Energy [0.1 kWh]",
        5004,
        2,
        ValueType::U32,
        0.1f,
    },
    {
        MeterValueID::TemperatureCabinet,
        "Inside Temperature [0.1 °C]",
        5008,
        1,
        ValueType::S16,
        0.1f,
    },
    {
        MeterValueID::PowerDC,
        "Total DC Power [W]",
        5017,
        2,
        ValueType::U32,
        1.0f,
    },
    {
        MeterValueID::VoltageL1N, // FIXME: depends on [5002]
        "A-B / A-N Voltage [0.1 V]",
        5019,
        1,
        ValueType::U16,
        0.1f,
    },
    {
        MeterValueID::VoltageL2N, // FIXME: depends on [5002]
        "B-C / B-N Voltage [0.1 V]",
        5020,
        1,
        ValueType::U16,
        0.1f,
    },
    {
        MeterValueID::VoltageL3N, // FIXME: depends on [5002]
        "C-A / C-N Voltage [0.1 V]",
        5021,
        1,
        ValueType::U16,
        0.1f,
    },
    {
        MeterValueID::PowerReactiveLSumIndCapDiff,
        "Reactive Power [var]",
        5033,
        2,
        ValueType::S32,
        1.0f,
    },
    {
        MeterValueID::PowerFactorLSumDirectional,
        "Power Factor [0.001]",
        5035,
        1,
        ValueType::S16,
        0.001f,
    },
    {
        MeterValueID::FrequencyLAvg,
        "Grid Frequency [0.01 Hz]",
        5036,
        1,
        ValueType::U16,
        0.01f, // FIXME: spec says 0.1 Hz, but device reports 0.01 Hz
    },
    {
        MeterValueID::NotSupported,
        "Total PV Generation [0.1 kWh]",
        13003,
        2,
        ValueType::U32,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Total PV Energy Export [0.1 kWh]",
        13006,
        2,
        ValueType::U32,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Load Power [W]",
        13008,
        2,
        ValueType::S32,
        1.0f,
    },
    {
        MeterValueID::PowerActiveLSumImExDiff,
        "Export Power [W]",
        13010,
        2,
        ValueType::S32,
        -1.0f,
    },
    {
        MeterValueID::NotSupported,
        "Total PV Energy Battery Charge [0.1 kWh]",
        13013,
        2,
        ValueType::U32,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Total Direct Battery Consumption [0.1 kWh]",
        13018,
        2,
        ValueType::U32,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Battery Voltage [0.1 V]",
        13020,
        1,
        ValueType::U16,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Battery Current [0.1 A]",
        13021,
        1,
        ValueType::U16,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Battery Power [W]",
        13022,
        1,
        ValueType::U16,
        1.0f,
    },
    {
        MeterValueID::StateOfCharge,
        "Battery Level [0.1 %]",
        13023,
        1,
        ValueType::U16,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Battery SoH [0.1 %]",
        13024,
        1,
        ValueType::U16,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Battery Temperature [0.1 °C]",
        13025,
        1,
        ValueType::S16,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Total Battery Energy Discharge [0.1 kWh]",
        13027,
        2,
        ValueType::U32,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Grid State",
        13030,
        1,
        ValueType::U16,
        1.0f,
    },
    {
        MeterValueID::CurrentL1ImExDiff, // FIXME: depends on [5002]
        "Phase A Current [0.1 A]",
        13031,
        1,
        ValueType::S16,
        0.1f,
    },
    {
        MeterValueID::CurrentL2ImExDiff, // FIXME: depends on [5002]
        "Phase B Current [0.1 A]",
        13032,
        1,
        ValueType::S16,
        0.1f,
    },
    {
        MeterValueID::CurrentL3ImExDiff, // FIXME: depends on [5002]
        "Phase C Current [0.1 A]",
        13033,
        1,
        ValueType::S16,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Total Active Power [W]",
        13034,
        2,
        ValueType::S32,
        1.0f,
    },
    {
        MeterValueID::EnergyActiveLSumImport,
        "Total Import Energy [0.1 kWh]",
        13037,
        2,
        ValueType::U32,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Battery Capacity [0.1 kWh | Ah]",
        13039,
        1,
        ValueType::U16,
        0.1f,
    },
    {
        MeterValueID::NotSupported,
        "Total Charge Energy [0.1 kWh]",
        13041,
        2,
        ValueType::U32,
        0.1f,
    },
    {
        MeterValueID::EnergyActiveLSumExport,
        "Total Export Energy [0.1 kWh]",
        13046,
        2,
        ValueType::U32,
        0.1f,
    },
};

static uint32_t value_index[ARRAY_SIZE(value_specs)];

MeterClassID MeterModbusTCP::get_class() const
{
    return MeterClassID::ModbusTCP;
}

void MeterModbusTCP::setup(const Config &ephemeral_config)
{
    MeterValueID value_ids[ARRAY_SIZE(value_specs)];
    uint32_t value_ids_used = 0;

    for (size_t i = 0; i < ARRAY_SIZE(value_specs); ++i) {
        if (value_specs[i].value_id != MeterValueID::NotSupported) {
            value_ids[value_ids_used] = value_specs[i].value_id;
            value_index[i] = value_ids_used;

            ++value_ids_used;
        }
        else {
            value_index[i] = UINT32_MAX;
        }
    }

    meters.declare_value_ids(slot, value_ids, value_ids_used);

    host_name      = ephemeral_config.get("host")->asString();
    port           = static_cast<uint16_t>(ephemeral_config.get("port")->asUint());
    device_address = static_cast<uint8_t>(ephemeral_config.get("device_address")->asUint());
    preset         = ephemeral_config.get("preset")->asEnum<Preset>();

    task_scheduler.scheduleOnce([this]() {
        this->read_allowed = false;
        this->start_connection();
    }, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (this->read_allowed) {
            this->read_allowed = false;
            this->start_generic_read();
        };
    }, 2000, 1000);
}

void MeterModbusTCP::connect_callback()
{
    read_index = 0;

    generic_read_request.register_type = TAddress::RegType::IREG;
    generic_read_request.start_address = value_specs[read_index].start_address - 1;
    generic_read_request.register_count = value_specs[read_index].register_count;

    generic_read_request.data[0] = register_buffer;
    generic_read_request.data[1] = nullptr;
    generic_read_request.read_twice = false;
    generic_read_request.done_callback = [this]{ read_done_callback(); };

    start_generic_read();
}

void MeterModbusTCP::disconnect_callback()
{
    read_allowed = false;
}

void MeterModbusTCP::read_done_callback()
{
    if (generic_read_request.result_code != Modbus::ResultCode::EX_SUCCESS) {
        read_allowed = true;

        logger.printfln("Error reading %s (%zu): %s [%d]",
                        value_specs[read_index].name,
                        value_specs[read_index].start_address,
                        get_modbus_result_code_name(generic_read_request.result_code),
                        generic_read_request.result_code);
        return;
    }

    union {
        uint32_t v;
        uint16_t r[2];
    } u;

    u.r[0] = register_buffer[0];
    u.r[1] = register_buffer[1];

    float value = NAN;

    switch (value_specs[read_index].value_type) {
    case ValueType::U16:
        logger.printfln("%s (%zu): %u", value_specs[read_index].name, value_specs[read_index].start_address, register_buffer[0]);
        value = static_cast<float>(register_buffer[0]);
        break;

    case ValueType::S16:
        logger.printfln("%s (%zu): %d", value_specs[read_index].name, value_specs[read_index].start_address, static_cast<int16_t>(register_buffer[0]));
        value = static_cast<float>(static_cast<int16_t>(register_buffer[0]));
        break;

    case ValueType::U32:
        logger.printfln("%s (%zu): %u (%u %u)", value_specs[read_index].name, value_specs[read_index].start_address, u.v, register_buffer[0], register_buffer[1]);
        value = static_cast<float>(u.v);
        break;

    case ValueType::S32:
        logger.printfln("%s (%zu): %d (%u %u)", value_specs[read_index].name, value_specs[read_index].start_address, static_cast<int32_t>(u.v), register_buffer[0], register_buffer[1]);
        value = static_cast<float>(static_cast<int32_t>(u.v));
        break;

    default:
        break;
    }

    if (value_index[read_index] != UINT32_MAX) {
        meters.update_value(slot, value_index[read_index], value * value_specs[read_index].scale_factor);
    }

    read_index = (read_index + 1) % ARRAY_SIZE(value_specs);
    generic_read_request.start_address = value_specs[read_index].start_address - 1;
    generic_read_request.register_count = value_specs[read_index].register_count;

    start_generic_read();
}
