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

//#define DEBUG_LOG_ALL_VALUES

#define SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS                 5002u
#define SUNGROW_INVERTER_GRID_FREQUENCY_ADDRESS              5036u
#define SUNGROW_HYBRID_INVERTER_RUNNING_STATE_ADDRESS        13001u
#define SUNGROW_HYBRID_INVERTER_BATTERY_CURRENT_ADDRESS      13021u
#define SUNGROW_HYBRID_INVERTER_BATTERY_POWER_ADDRESS        13022u
#define SUNGROW_STRING_INVERTER_TOTAL_ACTVE_POWER_ADDRESS    5031u

#define VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L1_ADDRESS 808u
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L2_ADDRESS 809u
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L3_ADDRESS 810u
#define VICTRON_ENERGY_GX_GRID_L1_ADDRESS                    820u
#define VICTRON_ENERGY_GX_GRID_L2_ADDRESS                    821u
#define VICTRON_ENERGY_GX_GRID_L3_ADDRESS                    822u
#define VICTRON_ENERGY_GX_AC_CONSUMPTION_L1_ADDRESS          817u
#define VICTRON_ENERGY_GX_AC_CONSUMPTION_L2_ADDRESS          818u
#define VICTRON_ENERGY_GX_AC_CONSUMPTION_L3_ADDRESS          819u

#define DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS             0u

#define MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(x) (static_cast<uint8_t>(x) & 0x07)
#define MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(x) ((static_cast<uint8_t>(x) >> 5) & 1)

MeterClassID MeterModbusTCP::get_class() const
{
    return MeterClassID::ModbusTCP;
}

void MeterModbusTCP::setup(const Config &ephemeral_config)
{
    host_name      = ephemeral_config.get("host")->asString();
    port           = static_cast<uint16_t>(ephemeral_config.get("port")->asUint());
    table_id       = ephemeral_config.get("table")->getTag<MeterModbusTCPTableID>();

    switch (table_id) {
    case MeterModbusTCPTableID::None:
        logger.printfln("No table selected");
        return;

    case MeterModbusTCPTableID::Custom: {
            generic_read_request.start_address_offset = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("register_address_mode")->asUint());
            device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

            const Config *registers = static_cast<const Config *>(ephemeral_config.get("table")->get()->get("registers"));
            uint16_t registers_count = static_cast<uint16_t>(registers->count());

            // FIXME: leaking this, because as of right now meter instances don't get destroied
            ValueSpec *customs_specs = new ValueSpec[registers_count];
            MeterValueID *customs_ids = new MeterValueID[registers_count];
            uint32_t *customs_index = new uint32_t[registers_count];

            for (uint16_t i = 0; i < registers_count; ++i) {
                customs_specs[i].name = "Custom";
                customs_specs[i].register_type = registers->get(i)->get("register_type")->asEnum<ModbusRegisterType>();
                customs_specs[i].start_address = registers->get(i)->get("start_address")->asUint();
                customs_specs[i].value_type = registers->get(i)->get("value_type")->asEnum<ModbusValueType>();
                customs_specs[i].offset = registers->get(i)->get("offset")->asFloat();
                customs_specs[i].scale_factor = registers->get(i)->get("scale_factor")->asFloat();

                customs_ids[i] = registers->get(i)->get("value_id")->asEnum<MeterValueID>();

                customs_index[i] = i;
            }

            custom_table = new ValueTable;
            custom_table->specs = customs_specs;
            custom_table->specs_length = registers_count;
            custom_table->ids = customs_ids;
            custom_table->ids_length = registers_count;
            custom_table->index = customs_index;

            table = custom_table;
        }

        break;

    case MeterModbusTCPTableID::SungrowHybridInverter:
        generic_read_request.start_address_offset = 1; // register number mode
        sungrow_hybrid_inverter_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<SungrowHybridInverterVirtualMeterID>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (sungrow_hybrid_inverter_virtual_meter) {
        case SungrowHybridInverterVirtualMeterID::None:
            logger.printfln("No Sungrow Hybrid Inverter Virtual Meter selected");
            return;

        case SungrowHybridInverterVirtualMeterID::Inverter:
            // will be set after output type discovery
            table = nullptr;
            break;

        case SungrowHybridInverterVirtualMeterID::Grid:
            table = &sungrow_hybrid_inverter_grid_table;
            break;

        case SungrowHybridInverterVirtualMeterID::Battery:
            table = &sungrow_hybrid_inverter_battery_table;
            break;

        case SungrowHybridInverterVirtualMeterID::Load:
            table = &sungrow_hybrid_inverter_load_table;
            break;

        default:
            logger.printfln("Unknown Sungrow Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(sungrow_hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::SungrowStringInverter:
        generic_read_request.start_address_offset = 1; // register number mode
        sungrow_string_inverter_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<SungrowStringInverterVirtualMeterID>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (sungrow_string_inverter_virtual_meter) {
        case SungrowStringInverterVirtualMeterID::None:
            logger.printfln("No Sungrow String Inverter Virtual Meter selected");
            return;

        case SungrowStringInverterVirtualMeterID::Inverter:
            // will be set after output type discovery
            table = nullptr;
            break;

        case SungrowStringInverterVirtualMeterID::Grid:
            table = &sungrow_string_inverter_grid_table;
            break;

        case SungrowStringInverterVirtualMeterID::Load:
            table = &sungrow_string_inverter_load_table;
            break;

        default:
            logger.printfln("Unknown Sungrow String Inverter Virtual Meter: %u", static_cast<uint8_t>(sungrow_string_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::SolarmaxMaxStorage:
        generic_read_request.start_address_offset = 0; // register address mode
        solarmax_max_storage_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<SolarmaxMaxStorageVirtualMeterID>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (solarmax_max_storage_virtual_meter) {
        case SolarmaxMaxStorageVirtualMeterID::None:
            logger.printfln("No Solarmax Max Storage Virtual Meter selected");
            return;

        case SolarmaxMaxStorageVirtualMeterID::Inverter:
            table = &solarmax_max_storage_inverter_table;
            break;

        case SolarmaxMaxStorageVirtualMeterID::Grid:
            table = &solarmax_max_storage_grid_table;
            break;

        case SolarmaxMaxStorageVirtualMeterID::Battery:
            table = &solarmax_max_storage_battery_table;
            break;

        default:
            logger.printfln("Unknown Solarmax Max Storage Virtual Meter: %u", static_cast<uint8_t>(solarmax_max_storage_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::VictronEnergyGX:
        generic_read_request.start_address_offset = 0; // register address mode
        victron_energy_gx_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<VictronEnergyGXVirtualMeterID>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (victron_energy_gx_virtual_meter) {
        case VictronEnergyGXVirtualMeterID::None:
            logger.printfln("No Victron Energy GX Virtual Meter selected");
            return;

        case VictronEnergyGXVirtualMeterID::Inverter:
            table = &victron_energy_gx_inverter_table;
            break;

        case VictronEnergyGXVirtualMeterID::Grid:
            table = &victron_energy_gx_grid_table;
            break;

        case VictronEnergyGXVirtualMeterID::Battery:
            table = &victron_energy_gx_battery_table;
            break;

        case VictronEnergyGXVirtualMeterID::Load:
            table = &victron_energy_gx_load_table;
            break;

        default:
            logger.printfln("Unknown Victron Energy GX Virtual Meter: %u", static_cast<uint8_t>(victron_energy_gx_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::DeyeHybridInverter:
        generic_read_request.start_address_offset = 0; // register address mode
        deye_hybrid_inverter_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<DeyeHybridInverterVirtualMeterID>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (deye_hybrid_inverter_virtual_meter) {
        case DeyeHybridInverterVirtualMeterID::None:
            logger.printfln("No Deye Hybrid Inverter Virtual Meter selected");
            return;

        case DeyeHybridInverterVirtualMeterID::Inverter:
            table = &deye_hybrid_inverter_table;
            break;

        case DeyeHybridInverterVirtualMeterID::Grid:
            table = &deye_hybrid_inverter_grid_table;
            break;

        case DeyeHybridInverterVirtualMeterID::Battery:
            // will be set after device type discovery
            table = nullptr;
            break;

        case DeyeHybridInverterVirtualMeterID::Load:
            table = &deye_hybrid_inverter_load_table;
            break;

        default:
            logger.printfln("Unknown Deye Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(deye_hybrid_inverter_virtual_meter));
            return;
        }

        break;

    default:
        logger.printfln("Unknown table: %u", static_cast<uint8_t>(table_id));
        return;
    }

    if (table != nullptr) {
        meters.declare_value_ids(slot, table->ids, table->ids_length);
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
    generic_read_request.data[0] = register_buffer;
    generic_read_request.data[1] = nullptr;
    generic_read_request.read_twice = false;
    generic_read_request.done_callback = [this]{ read_done_callback(); };

    if (is_sungrow_inverter_meter()) {
        if (sungrow_inverter_output_type < 0) {
            generic_read_request.register_type = ModbusRegisterType::InputRegister;
            generic_read_request.start_address = SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS; // read output type
            generic_read_request.register_count = 1;
        }
    }
    else if (is_deye_hybrid_inverter_battery_meter()) {
        if (deye_hybrid_inverter_device_type < 0) {
            generic_read_request.register_type = ModbusRegisterType::HoldingRegister;
            generic_read_request.start_address = DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS; // read device type
            generic_read_request.register_count = 1;
        }
    }
    else {
        read_index = 0;

        prepare_read();
    }

    start_generic_read();
}

void MeterModbusTCP::disconnect_callback()
{
    read_allowed = false;
}

bool MeterModbusTCP::prepare_read()
{
    bool overflow = false;

    while (
#ifndef DEBUG_LOG_ALL_VALUES
        table->index[read_index] == VALUE_INDEX_DEBUG ||
#endif
        table->specs[read_index].start_address == START_ADDRESS_VIRTUAL) {
        read_index = (read_index + 1) % table->specs_length;

        if (read_index == 0) {
            overflow = true;
        }
    }

    generic_read_request.register_type = table->specs[read_index].register_type;
    generic_read_request.start_address = table->specs[read_index].start_address;
    generic_read_request.register_count = MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[read_index].value_type);

    return overflow;
}

bool MeterModbusTCP::is_sungrow_inverter_meter() const
{
    return (table_id == MeterModbusTCPTableID::SungrowHybridInverter
         && sungrow_hybrid_inverter_virtual_meter == SungrowHybridInverterVirtualMeterID::Inverter)
        || (table_id == MeterModbusTCPTableID::SungrowStringInverter
         && sungrow_string_inverter_virtual_meter == SungrowStringInverterVirtualMeterID::Inverter);
}

bool MeterModbusTCP::is_sungrow_grid_meter() const
{
    return (table_id == MeterModbusTCPTableID::SungrowHybridInverter
         && sungrow_hybrid_inverter_virtual_meter == SungrowHybridInverterVirtualMeterID::Grid)
        || (table_id == MeterModbusTCPTableID::SungrowStringInverter
         && sungrow_string_inverter_virtual_meter == SungrowStringInverterVirtualMeterID::Grid);
}

bool MeterModbusTCP::is_sungrow_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::SungrowHybridInverter
        && sungrow_hybrid_inverter_virtual_meter == SungrowHybridInverterVirtualMeterID::Battery;
}

bool MeterModbusTCP::is_victron_energy_gx_inverter_meter() const
{
    return table_id == MeterModbusTCPTableID::VictronEnergyGX
        && victron_energy_gx_virtual_meter == VictronEnergyGXVirtualMeterID::Inverter;
}

bool MeterModbusTCP::is_victron_energy_gx_grid_meter() const
{
    return table_id == MeterModbusTCPTableID::VictronEnergyGX
        && victron_energy_gx_virtual_meter == VictronEnergyGXVirtualMeterID::Grid;
}

bool MeterModbusTCP::is_victron_energy_gx_load_meter() const
{
    return table_id == MeterModbusTCPTableID::VictronEnergyGX
        && victron_energy_gx_virtual_meter == VictronEnergyGXVirtualMeterID::Load;
}

bool MeterModbusTCP::is_deye_hybrid_inverter_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::DeyeHybridInverter
        && deye_hybrid_inverter_virtual_meter == DeyeHybridInverterVirtualMeterID::Battery;
}

void MeterModbusTCP::read_done_callback()
{
    if (generic_read_request.result_code != Modbus::ResultCode::EX_SUCCESS) {
        read_allowed = true;

        if (is_sungrow_inverter_meter()
         && generic_read_request.start_address == SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS) {
            logger.printfln("Error reading %s / Output Type (%u): %s [%d]",
                            get_table_name(table_id),
                            SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS,
                            get_modbus_result_code_name(generic_read_request.result_code),
                            generic_read_request.result_code);
        }
        else if (is_deye_hybrid_inverter_battery_meter()
              && generic_read_request.start_address == DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS) {
            logger.printfln("Error reading %s / Device Type (%u): %s [%d]",
                            get_table_name(table_id),
                            DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS,
                            get_modbus_result_code_name(generic_read_request.result_code),
                            generic_read_request.result_code);
        }
        else {
            logger.printfln("Error reading %s / %s (%zu): %s [%d]",
                            get_table_name(table_id),
                            table->specs[read_index].name,
                            table->specs[read_index].start_address,
                            get_modbus_result_code_name(generic_read_request.result_code),
                            generic_read_request.result_code);
        }

        return;
    }

    if (is_sungrow_inverter_meter()
     && generic_read_request.start_address == SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS) {
        if (sungrow_inverter_output_type < 0) {
            switch (register_buffer[0]) {
            case 0:
                if (table_id == MeterModbusTCPTableID::SungrowHybridInverter) {
                    table = &sungrow_hybrid_inverter_1p2l_table;
                }
                else { // MeterModbusTCPTableID::SungrowStringInverter
                    table = &sungrow_string_inverter_1p2l_table;
                }

                break;

            case 1:
                if (table_id == MeterModbusTCPTableID::SungrowHybridInverter) {
                    table = &sungrow_hybrid_inverter_3p4l_table;
                }
                else { // MeterModbusTCPTableID::SungrowStringInverter
                    table = &sungrow_string_inverter_3p4l_table;
                }

                break;

            case 2:
                if (table_id == MeterModbusTCPTableID::SungrowHybridInverter) {
                    table = &sungrow_hybrid_inverter_3p3l_table;
                }
                else { // MeterModbusTCPTableID::SungrowStringInverter
                    table = &sungrow_string_inverter_3p3l_table;
                }

                break;

            default:
                logger.printfln("%s has unknown Output Type: %u", get_table_name(table_id), register_buffer[0]);
                return;
            }

            sungrow_inverter_output_type = register_buffer[0];

#ifdef DEBUG_LOG_ALL_VALUES
            logger.printfln("%s / Output Type (%u): %d",
                            get_table_name(table_id),
                            SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS,
                            sungrow_inverter_output_type);
#endif

            meters.declare_value_ids(slot, table->ids, table->ids_length);
        }

        read_allowed = true;
        read_index = 0;

        prepare_read();

        return;
    }

    if (is_deye_hybrid_inverter_battery_meter()
     && generic_read_request.start_address == DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS) {
        if (deye_hybrid_inverter_device_type < 0) {
            switch (register_buffer[0]) {
            case 0x200:
            case 0x300:
            case 0x400:
                logger.printfln("%s has unsupported Device Type: %u", get_table_name(table_id), register_buffer[0]);
                return;

            case 0x500:
                table = &deye_hybrid_inverter_low_voltage_battery_table;
                break;

            case 0x600:
            case 0x601:
                table = &deye_hybrid_inverter_high_voltage_battery_table;
                break;

            default:
                logger.printfln("%s has unknown Device Type: %u", get_table_name(table_id), register_buffer[0]);
                return;
            }

            deye_hybrid_inverter_device_type = register_buffer[0];

#ifdef DEBUG_LOG_ALL_VALUES
            logger.printfln("%s / Device Type (%u): %d",
                            get_table_name(table_id),
                            DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS,
                            deye_hybrid_inverter_device_type);
#endif

            meters.declare_value_ids(slot, table->ids, table->ids_length);
        }

        read_allowed = true;
        read_index = 0;

        prepare_read();

        return;
    }

    union {
        uint32_t u;
        float f;
        uint16_t r[2];
    } c32;

    union {
        uint64_t u;
        double f;
        uint16_t r[4];
    } c64;

    float value = NAN;
    ModbusValueType value_type = table->specs[read_index].value_type;
    size_t register_count = MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(value_type);

    switch (register_count) {
    case 1:
        break;

    case 2:
        if (MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(value_type)) {
            c32.r[0] = register_buffer[0];
            c32.r[1] = register_buffer[1];
        }
        else {
            c32.r[0] = register_buffer[1];
            c32.r[1] = register_buffer[0];
        }

        break;

    case 4:
        if (MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(value_type)) {
            c64.r[0] = register_buffer[0];
            c64.r[1] = register_buffer[1];
            c64.r[2] = register_buffer[2];
            c64.r[3] = register_buffer[3];
        }
        else {
            c64.r[0] = register_buffer[3];
            c64.r[1] = register_buffer[2];
            c64.r[2] = register_buffer[1];
            c64.r[3] = register_buffer[0];
        }

        break;

    default:
        logger.printfln("%s / %s has unsupported register count: %u", get_table_name(table_id), table->specs[read_index].name, register_count);
        return;
    }

    switch (value_type) {
    case ModbusValueType::None:
        break;

    case ModbusValueType::U16:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %u",
                        get_table_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        register_buffer[0]);
#endif

        value = static_cast<float>(register_buffer[0]);
        break;

    case ModbusValueType::S16:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %d",
                        get_table_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        static_cast<int16_t>(register_buffer[0]));
#endif

        value = static_cast<float>(static_cast<int16_t>(register_buffer[0]));
        break;

    case ModbusValueType::U32BE:
    case ModbusValueType::U32LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %u (%u %u)",
                        get_table_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        c32.u,
                        register_buffer[0],
                        register_buffer[1]);
#endif

        value = static_cast<float>(c32.u);
        break;

    case ModbusValueType::S32BE:
    case ModbusValueType::S32LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %d (%u %u)",
                        get_table_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        static_cast<int32_t>(c32.u),
                        register_buffer[0],
                        register_buffer[1]);
#endif

        value = static_cast<float>(static_cast<int32_t>(c32.u));
        break;

    case ModbusValueType::F32BE:
    case ModbusValueType::F32LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %f (%u %u)",
                        get_table_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        static_cast<double>(c32.f),
                        register_buffer[0],
                        register_buffer[1]);
#endif

        value = c32.f;
        break;

    case ModbusValueType::U64BE:
    case ModbusValueType::U64LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %llu (%u %u %u %u)",
                        get_table_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        c64.u,
                        register_buffer[0],
                        register_buffer[1],
                        register_buffer[2],
                        register_buffer[3]);
#endif

        value = static_cast<float>(c64.u);
        break;

    case ModbusValueType::S64BE:
    case ModbusValueType::S64LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %lld (%u %u %u %u)",
                        get_table_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        static_cast<int64_t>(c64.u),
                        register_buffer[0],
                        register_buffer[1],
                        register_buffer[2],
                        register_buffer[3]);
#endif

        value = static_cast<float>(static_cast<int64_t>(c64.u));
        break;

    case ModbusValueType::F64BE:
    case ModbusValueType::F64LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %f (%u %u %u %u)",
                        get_table_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        c64.f,
                        register_buffer[0],
                        register_buffer[1],
                        register_buffer[2],
                        register_buffer[3]);
#endif

        value = static_cast<float>(c64.f);
        break;

    default:
        break;
    }

    value = (value + table->specs[read_index].offset) * table->specs[read_index].scale_factor;

    if (is_sungrow_grid_meter()) {
        if (generic_read_request.start_address == SUNGROW_INVERTER_GRID_FREQUENCY_ADDRESS) {
            if (value > 100) {
                // according to the spec the grid frequency is given
                // as 0.1 Hz, but some inverters report it as 0.01 Hz
                value /= 10;
            }
        }
    }
    else if (is_sungrow_inverter_meter()) {
        if (generic_read_request.start_address == SUNGROW_STRING_INVERTER_TOTAL_ACTVE_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], -value);
        }
    }
    else if (is_sungrow_battery_meter()) {
        if (generic_read_request.start_address == SUNGROW_HYBRID_INVERTER_RUNNING_STATE_ADDRESS) {
            sungrow_hybrid_inverter_running_state = register_buffer[0];
        }
        else if (generic_read_request.start_address == SUNGROW_HYBRID_INVERTER_BATTERY_CURRENT_ADDRESS) {
            if ((sungrow_hybrid_inverter_running_state & (1 << 2)) != 0) {
                value = -value;
            }
        }
        else if (generic_read_request.start_address == SUNGROW_HYBRID_INVERTER_BATTERY_POWER_ADDRESS) {
            if ((sungrow_hybrid_inverter_running_state & (1 << 2)) != 0) {
                value = -value;
            }
        }
    }
    else if (is_victron_energy_gx_inverter_meter()) {
        if (generic_read_request.start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L1_ADDRESS) {
            victron_energy_gx_ac_coupled_pv_on_output_l1_power = value;
        }
        else if (generic_read_request.start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L2_ADDRESS) {
            victron_energy_gx_ac_coupled_pv_on_output_l2_power = value;
        }
        else if (generic_read_request.start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L3_ADDRESS) {
            victron_energy_gx_ac_coupled_pv_on_output_l3_power = value;

            float power = victron_energy_gx_ac_coupled_pv_on_output_l1_power
                        + victron_energy_gx_ac_coupled_pv_on_output_l2_power
                        + victron_energy_gx_ac_coupled_pv_on_output_l3_power;

            meters.update_value(slot, table->index[read_index + 1], -power);
        }
    }
    else if (is_victron_energy_gx_grid_meter()) {
        if (generic_read_request.start_address == VICTRON_ENERGY_GX_GRID_L1_ADDRESS) {
            victron_energy_gx_grid_l1_power = value;
        }
        else if (generic_read_request.start_address == VICTRON_ENERGY_GX_GRID_L2_ADDRESS) {
            victron_energy_gx_grid_l2_power = value;
        }
        else if (generic_read_request.start_address == VICTRON_ENERGY_GX_GRID_L3_ADDRESS) {
            victron_energy_gx_grid_l3_power = value;

            float power = victron_energy_gx_grid_l1_power
                        + victron_energy_gx_grid_l2_power
                        + victron_energy_gx_grid_l3_power;

            meters.update_value(slot, table->index[read_index + 1], power);
        }
    }
    else if (is_victron_energy_gx_load_meter()) {
        if (generic_read_request.start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L1_ADDRESS) {
            victron_energy_gx_ac_consumption_l1_power = value;
        }
        else if (generic_read_request.start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L2_ADDRESS) {
            victron_energy_gx_ac_consumption_l2_power = value;
        }
        else if (generic_read_request.start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L3_ADDRESS) {
            victron_energy_gx_ac_consumption_l3_power = value;

            float power = victron_energy_gx_ac_consumption_l1_power
                        + victron_energy_gx_ac_consumption_l2_power
                        + victron_energy_gx_ac_consumption_l3_power;

            meters.update_value(slot, table->index[read_index + 1], power);
        }
    }

    if (table->index[read_index] != VALUE_INDEX_META && table->index[read_index] != VALUE_INDEX_DEBUG) {
        meters.update_value(slot, table->index[read_index], value);
    }

    read_index = (read_index + 1) % table->specs_length;

    bool overflow = read_index == 0;

    if (prepare_read()) {
        overflow = true;
    }

    if (overflow) {
        // make a little pause after each round trip
        read_allowed = true;
    }
    else {
        start_generic_read();
    }
}
