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

#include "meters_modbus_tcp_defs.inc"

#define DEBUG_LOG_ALL_READS

MeterClassID MeterModbusTCP::get_class() const
{
    return MeterClassID::ModbusTCP;
}

void MeterModbusTCP::setup(const Config &ephemeral_config)
{
    host_name      = ephemeral_config.get("host")->asString();
    port           = static_cast<uint16_t>(ephemeral_config.get("port")->asUint());
    device_address = static_cast<uint8_t>(ephemeral_config.get("device_address")->asUint());
    preset         = ephemeral_config.get("preset")->asEnum<MeterModbusTCPPreset>();

    uint32_t value_ids_length;

    switch (preset) {
    //case MeterModbusTCPPreset::Custom:

    case MeterModbusTCPPreset::SungrowHybridInverter:
        // will be set after output type discovery
        value_specs        = nullptr;
        value_specs_length = 0;
        value_ids          = nullptr;
        value_ids_length   = 0;
        value_index        = nullptr;
        break;

    case MeterModbusTCPPreset::SungrowHybridInverterGrid:
        value_specs        = sungrow_hybrid_inverter_grid_specs;
        value_specs_length = ARRAY_SIZE(sungrow_hybrid_inverter_grid_specs);
        value_ids          = sungrow_hybrid_inverter_grid_ids;
        value_ids_length   = ARRAY_SIZE(sungrow_hybrid_inverter_grid_ids);
        value_index        = sungrow_hybrid_inverter_grid_index;
        break;

    case MeterModbusTCPPreset::SungrowHybridInverterBattery:
        value_specs        = sungrow_hybrid_inverter_battery_specs;
        value_specs_length = ARRAY_SIZE(sungrow_hybrid_inverter_battery_specs);
        value_ids          = sungrow_hybrid_inverter_battery_ids;
        value_ids_length   = ARRAY_SIZE(sungrow_hybrid_inverter_battery_ids);
        value_index        = sungrow_hybrid_inverter_battery_index;
        break;

    case MeterModbusTCPPreset::SungrowHybridInverterLoad:
        value_specs        = sungrow_hybrid_inverter_load_specs;
        value_specs_length = ARRAY_SIZE(sungrow_hybrid_inverter_load_specs);
        value_ids          = sungrow_hybrid_inverter_load_ids;
        value_ids_length   = ARRAY_SIZE(sungrow_hybrid_inverter_load_ids);
        value_index        = sungrow_hybrid_inverter_load_index;
        break;

    /*
    case MeterModbusTCPPreset::SungrowStringInverter:
    case MeterModbusTCPPreset::SungrowStringInverterGrid:
    case MeterModbusTCPPreset::SolarmaxMaxStorageInverter:
    case MeterModbusTCPPreset::SolarmaxMaxStorageGrid:
    case MeterModbusTCPPreset::SolarmaxMaxStorageBattery:*/
    default:
        logger.printfln("Unknown preset %u", static_cast<uint8_t>(preset));
        return;
    }

    if (value_ids != nullptr) {
        meters.declare_value_ids(slot, value_ids, value_ids_length);
    }

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

    if (preset == MeterModbusTCPPreset::SungrowHybridInverter) {
        if (sungrow_hybrid_inverter_output_type < 0) {
            generic_read_request.start_address = 5002 - 1; // read output type
            generic_read_request.register_count = 1;
        }
    }
    else {
        generic_read_request.start_address = value_specs[read_index].start_address - 1;
        generic_read_request.register_count = static_cast<uint8_t>(value_specs[read_index].value_type) % 10;
    }

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

        if (preset == MeterModbusTCPPreset::SungrowHybridInverter && generic_read_request.start_address == 5002 - 1) {
            logger.printfln("Error reading %s / Output Type (5002): %s [%d]",
                            get_preset_name(preset),
                            get_modbus_result_code_name(generic_read_request.result_code),
                            generic_read_request.result_code);
        }
        else {
            logger.printfln("Error reading %s / %s (%zu): %s [%d]",
                            get_preset_name(preset),
                            value_specs[read_index].name,
                            value_specs[read_index].start_address,
                            get_modbus_result_code_name(generic_read_request.result_code),
                            generic_read_request.result_code);
        }

        return;
    }

    if (preset == MeterModbusTCPPreset::SungrowHybridInverter && generic_read_request.start_address == 5002 - 1) {
        if (sungrow_hybrid_inverter_output_type < 0) {
            uint32_t value_ids_length;

            switch (register_buffer[0]) {
            case 0:
                value_specs = sungrow_hybrid_inverter_1p2l_specs;
                value_specs_length = ARRAY_SIZE(sungrow_hybrid_inverter_1p2l_specs);
                value_ids = sungrow_hybrid_inverter_1p2l_ids;
                value_ids_length   = ARRAY_SIZE(sungrow_hybrid_inverter_1p2l_ids);
                value_index = sungrow_hybrid_inverter_1p2l_index;
                break;

            case 1:
                value_specs = sungrow_hybrid_inverter_3p4l_specs;
                value_specs_length = ARRAY_SIZE(sungrow_hybrid_inverter_3p4l_specs);
                value_ids = sungrow_hybrid_inverter_3p4l_ids;
                value_ids_length   = ARRAY_SIZE(sungrow_hybrid_inverter_3p4l_ids);
                value_index = sungrow_hybrid_inverter_3p4l_index;
                break;

            case 2:
                value_specs = sungrow_hybrid_inverter_3p3l_specs;
                value_specs_length = ARRAY_SIZE(sungrow_hybrid_inverter_3p3l_specs);
                value_ids = sungrow_hybrid_inverter_3p3l_ids;
                value_ids_length   = ARRAY_SIZE(sungrow_hybrid_inverter_3p3l_ids);
                value_index = sungrow_hybrid_inverter_3p3l_index;
                break;

            default:
                logger.printfln("%s has unknown Output Type: %u", get_preset_name(preset), register_buffer[0]);
                return;
            }

            sungrow_hybrid_inverter_output_type = register_buffer[0];

#ifdef DEBUG_LOG_ALL_READS
            logger.printfln("%s / Output Type (5002): %d", get_preset_name(preset), sungrow_hybrid_inverter_output_type);
#endif

            meters.declare_value_ids(slot, value_ids, value_ids_length);
        }

        read_allowed = true;
        read_index = 0;

        generic_read_request.start_address = value_specs[read_index].start_address - 1;
        generic_read_request.register_count = static_cast<uint8_t>(value_specs[read_index].value_type) % 10;

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
#ifdef DEBUG_LOG_ALL_READS
        logger.printfln("%s / %s (%zu): %u", get_preset_name(preset), value_specs[read_index].name, value_specs[read_index].start_address, register_buffer[0]);
#endif
        value = static_cast<float>(register_buffer[0]);
        break;

    case ValueType::S16:
#ifdef DEBUG_LOG_ALL_READS
        logger.printfln("%s / %s (%zu): %d", get_preset_name(preset), value_specs[read_index].name, value_specs[read_index].start_address, static_cast<int16_t>(register_buffer[0]));
#endif
        value = static_cast<float>(static_cast<int16_t>(register_buffer[0]));
        break;

    case ValueType::U32:
#ifdef DEBUG_LOG_ALL_READS
        logger.printfln("%s / %s (%zu): %u (%u %u)", get_preset_name(preset), value_specs[read_index].name, value_specs[read_index].start_address, u.v, register_buffer[0], register_buffer[1]);
#endif
        value = static_cast<float>(u.v);
        break;

    case ValueType::S32:
#ifdef DEBUG_LOG_ALL_READS
        logger.printfln("%s / %s (%zu): %d (%u %u)", get_preset_name(preset), value_specs[read_index].name, value_specs[read_index].start_address, static_cast<int32_t>(u.v), register_buffer[0], register_buffer[1]);
#endif
        value = static_cast<float>(static_cast<int32_t>(u.v));
        break;

    default:
        break;
    }

    if (value_index[read_index] != UINT32_MAX) {
        value *= value_specs[read_index].scale_factor;
    }

    if (preset == MeterModbusTCPPreset::SungrowHybridInverterGrid) {
        if (generic_read_request.start_address == 5036 - 1) { // grid frequency
            if (value > 100) {
                // according to the spec the grid frequency is given
                // as 0.1 Hz, but some inverters report it as 0.01 Hz
                value /= 10;
            }
        }
    }
    else if (preset == MeterModbusTCPPreset::SungrowHybridInverterBattery) {
        if (generic_read_request.start_address == 13001 - 1) { // running state
            sungrow_hybrid_inverter_battery_running_state = register_buffer[0];
        }
        else if (generic_read_request.start_address == 13021 - 1) { // battery current
            if ((sungrow_hybrid_inverter_battery_running_state & (1 << 2)) != 0) {
                value = -value;
            }
        }
        else if (generic_read_request.start_address == 13022 - 1) { // battery power
            if ((sungrow_hybrid_inverter_battery_running_state & (1 << 2)) != 0) {
                value = -value;
            }
        }
    }

    if (value_index[read_index] != UINT32_MAX) {
        meters.update_value(slot, value_index[read_index], value);
    }

    read_index = (read_index + 1) % value_specs_length;
    generic_read_request.start_address = value_specs[read_index].start_address - 1;
    generic_read_request.register_count = static_cast<uint8_t>(value_specs[read_index].value_type) % 10;

    if (read_index == 0) {
        // make a little pause after each round trip
        read_allowed = true;
    }
    else {
        start_generic_read();
    }
}
