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
#define TRACE_LOG_PREFIX nullptr

#include "meter_modbus_tcp.h"

#include <float.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "generated/meter_modbus_tcp_specs.h"
#include "tools/float.h"
#include "tools/sun_spec.h"
#include "tools/hexdump.h"

#include "gcc_warnings.h"

// 1P2L, 3P4L and 3P3L use the same addresses, but different amounts of phase voltage registers
namespace SungrowHybridInverterAddress = SungrowHybridInverter1P2LAddress;

// 1P2L, 3P4L and 3P3L use the same addresses, but different amounts of phase voltage registers
namespace SungrowStringInverterAddress = SungrowStringInverter1P2LAddress;

// AtL1, AtL2 and AtL3 use the same addresses, but different value IDs
namespace CarloGavazziEM100AndET100Address = CarloGavazziEM100AndET100AtL1Address;

// AsL1, AsL2 and AsL3 use the same addresses, but different value IDs
namespace ShellyEMMonophaseChannel1Address = ShellyEMMonophaseChannel1AsL1Address;

// AsL1, AsL2 and AsL3 use the same addresses, but different value IDs
namespace ShellyEMMonophaseChannel2Address = ShellyEMMonophaseChannel2AsL1Address;

// AsL1, AsL2 and AsL3 use the same addresses, but different value IDs
namespace ShellyEMMonophaseChannel3Address = ShellyEMMonophaseChannel3AsL1Address;

// AtL1, AtL2 and AtL3 use the same addresses, but different value IDs
namespace CarloGavazziEM510Address = CarloGavazziEM510AtL1Address;

// InputPower uses the same address in all HuaweiSUN2000PV*Address
namespace HuaweiSUN2000PVAddress = HuaweiSUN2000PVNoStringsAddress;

#define NUMBER_TO_ADDRESS(number) ((number) - 1u)

#define MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(x) (static_cast<uint8_t>(x) & 0x07)
#define MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(x) ((static_cast<uint8_t>(x) >> 5) & 1)

#define trace(fmt, ...) \
    do { \
        meters_modbus_tcp.trace_timestamp(); \
        logger.tracefln_plain(meters_modbus_tcp.trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

#define is_deye_hybrid_inverter_low_voltage_pv_meter()  (is_deye_hybrid_inverter_pv_meter() && deye_hybrid_inverter.low_voltage)
#define is_deye_hybrid_inverter_high_voltage_pv_meter() (is_deye_hybrid_inverter_pv_meter() && deye_hybrid_inverter.high_voltage)

MeterClassID MeterModbusTCP::get_class() const
{
    return MeterClassID::ModbusTCP;
}

void MeterModbusTCP::setup(Config *ephemeral_config)
{
    host      = ephemeral_config->get("host")->asString();
    port      = ephemeral_config->get("port")->asUint16();
    table_id  = ephemeral_config->get("table")->getTag<MeterModbusTCPTableID>();

    auto ephemeral_table_config = ephemeral_config->get("table")->get();

    switch (table_id) {
    case MeterModbusTCPTableID::None:
        logger.printfln_meter("No table selected");
        break;

    case MeterModbusTCPTableID::Custom: {
            device_address = ephemeral_table_config->get("device_address")->asUint8();

            const Config *registers = static_cast<const Config *>(ephemeral_table_config->get("registers"));
            size_t registers_count = registers->count();

            // FIXME: leaking this, because as of right now meter instances don't get destroyed
            TableSpec *custom_table = new TableSpec;
            ValueSpec *custom_specs = new ValueSpec[registers_count];
            MeterValueID *custom_ids = new MeterValueID[registers_count];
            uint32_t *custom_index = new uint32_t[registers_count];

            for (size_t i = 0; i < registers_count; ++i) {
                MeterValueID value_id = registers->get(i)->get("id")->asEnum<MeterValueID>();

                custom_specs[i].register_type = registers->get(i)->get("rtype")->asEnum<ModbusRegisterType>();
                custom_specs[i].start_address = registers->get(i)->get("addr")->asUint();
                custom_specs[i].value_type = registers->get(i)->get("vtype")->asEnum<ModbusValueType>();
                custom_specs[i].drop_sign = false; // FIXME: expose in API?
                custom_specs[i].offset = registers->get(i)->get("off")->asFloat();
                custom_specs[i].scale_factor = registers->get(i)->get("scale")->asFloat();

                custom_ids[i] = value_id;

                custom_index[i] = i;
            }

            custom_table->fixed_location = MeterLocation::Unknown;
            custom_table->specs = custom_specs;
            custom_table->specs_length = registers_count;
            custom_table->ids = custom_ids;
            custom_table->ids_length = registers_count;
            custom_table->index = custom_index;

            table = custom_table;
        }

        break;

#include "generated/meter_modbus_tcp_table_lookup_cases.inc"

    default:
        logger.printfln_meter("Unknown table: %u", static_cast<uint8_t>(table_id));
        break;
    }

    if (table == nullptr) {
        return;
    }

    max_register_count = std::min(static_cast<size_t>(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE), max_register_count);

    if (table->fixed_location != MeterLocation::Unknown) {
        ephemeral_config->get("location")->updateEnum(table->fixed_location);
    }

    if (table->ids_length > 0) {
        meters.declare_value_ids(slot, table->ids, table->ids_length);
    }

    if (table->specs_length > 0) {
        task_scheduler.scheduleUncancelable([this]() {
            if (read_allowed) {
                read_allowed = false;
                read_next();
            }
        }, 2_s, 1_s);
    }
}

void MeterModbusTCP::register_events()
{
    if (table == nullptr || table->specs_length == 0) {
        return;
    }

    network.on_network_connected([this](const Config *connected) {
        if (connected->asBool()) {
            start_connection();
        }
        else {
            stop_connection();
        }

        return EventResult::OK;
    });
}

void MeterModbusTCP::pre_reboot()
{
    stop_connection();
}

void MeterModbusTCP::connect_callback(TFGenericTCPClientConnectResult result, TFGenericTCPClientPoolShareLevel share_level)
{
    GenericModbusTCPClient::connect_callback(result, share_level);

    trace("m%lu c%d sl%d", slot, static_cast<int>(result), static_cast<int>(share_level));

    if (result != TFGenericTCPClientConnectResult::Connected) {
        return;
    }

    generic_read_request.data = register_buffer;
    generic_read_request.done_callback = [this]{ read_done(); };

    read_index = 0;
    read_next();
}

void MeterModbusTCP::disconnect_callback(TFGenericTCPClientDisconnectReason reason, TFGenericTCPClientPoolShareLevel share_level)
{
    trace("m%lu d%d sl%d", slot, static_cast<int>(reason), static_cast<int>(share_level));

    read_allowed = false;
}

void MeterModbusTCP::read_next()
{
    while (table->specs[read_index].start_address >= START_ADDRESS_VIRTUAL_BASE) {
        read_index = (read_index + 1) % table->specs_length;

        if (read_index == 0) {
            meters.finish_update(slot);
            read_allowed = true;
            return; // make a little pause after each round trip
        }
    }

    generic_read_request.register_type = table->specs[read_index].register_type;
    generic_read_request.start_address = table->specs[read_index].start_address;
    generic_read_request.register_count = MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[read_index].value_type);

    for (size_t i = read_index + 1; i < table->specs_length; ++i) {
        if (table->specs[i].start_address >= START_ADDRESS_VIRTUAL_BASE) {
            continue;
        }

        if (table->specs[i].register_type == generic_read_request.register_type
         && table->specs[i].start_address >= generic_read_request.start_address
         && table->specs[i].start_address <= generic_read_request.start_address + generic_read_request.register_count) {
            size_t register_count = table->specs[i].start_address + MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[i].value_type) - generic_read_request.start_address;

            if (register_count <= max_register_count) {
                if (register_count > generic_read_request.register_count) {
                    generic_read_request.register_count = register_count;
                }

                continue;
            }
        }

        break;
    }

    start_generic_read();
}

void MeterModbusTCP::read_done()
{
    read_allowed = true;

    if (generic_read_request.result != TFModbusTCPClientTransactionResult::Success) {
        trace("m%lu a%zu c%zu e%lu",
              slot,
              generic_read_request.start_address,
              generic_read_request.register_count,
              static_cast<uint32_t>(generic_read_request.result));

        if (generic_read_request.result == TFModbusTCPClientTransactionResult::Aborted) {
            // an abort is triggered before a connection close or before a forced reconnect,
            // stop reading. in both cases the reading will be restarted by the automatic reconnect
            read_allowed = false;
        }
        else if (generic_read_request.result == TFModbusTCPClientTransactionResult::Timeout) {
            auto timeout = errors->get("timeout");
            timeout->updateUint(timeout->asUint() + 1);
        }
        else if (generic_read_request.result == TFModbusTCPClientTransactionResult::ModbusIllegalDataAddress
              && is_goodwe_inverter_grid_meter()
              && goodwe_inverter.grid_detect_state == GoodweGridDetectState::Reading
              && generic_read_request.start_address == GoodweInverterGridDetectAddress::ActiveEnergyTotalBuyTotal) {
            goodwe_inverter.grid_detect_state = GoodweGridDetectState::Detected32bitEnergy;
            table = &goodwe_inverter_grid_32bit_energy_table;
            read_index = 0;

            logger.printfln_meter("Goodwe inverter with 32-bit grid energy registers detected (reading 64-bit grid energy register returns error)");
            meters.declare_value_ids(slot, table->ids, table->ids_length);
        }

        return; // make a little pause after each error or restart
    }

    trace("m%lu a%zu c%zu",
          slot,
          generic_read_request.start_address,
          generic_read_request.register_count);

    char data_buf[125 * 4 + 1]; // 4 nibble per register for 125 registers plus \n
    size_t data_buf_used = hexdump(generic_read_request.data, generic_read_request.register_count, data_buf, ARRAY_SIZE(data_buf), HexdumpCase::Lower);
    data_buf[data_buf_used] = '\n';
    ++data_buf_used;

    logger.trace_plain(meters_modbus_tcp.trace_buffer_index, data_buf, data_buf_used);

    while (true) {
        if (!parse()) {
            read_index = 0;
            return; // make a little pause after each restart
        }

        read_index = (read_index + 1) % table->specs_length;

        if (read_index == 0) {
            meters.finish_update(slot);
            return; // make a little pause after each round trip
        }

        if (table->specs[read_index].start_address >= START_ADDRESS_VIRTUAL_BASE) {
            continue;
        }

        if (table->specs[read_index].register_type == generic_read_request.register_type
         && table->specs[read_index].start_address >= generic_read_request.start_address
         && table->specs[read_index].start_address + MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[read_index].value_type) <= generic_read_request.start_address + generic_read_request.register_count) {
            continue;
        }

        break;
    }

    read_allowed = false;
    read_next();
}

// returns true for continue, returns false for restart
bool MeterModbusTCP::parse()
{
    union {
        uint16_t u;
        uint16_t r;
    } c16;

    c16.r = 0; // avoid compiler warning about unintialized value

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
    const ValueSpec *spec = &table->specs[read_index];
    size_t register_count = MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(spec->value_type);
    const char *endian = "?";
    size_t register_buffer_index = 0;

    if (spec->start_address < START_ADDRESS_VIRTUAL_BASE) {
        register_buffer_index = spec->start_address - generic_read_request.start_address;
    }

    if (table->index[read_index] == VALUE_INDEX_GAP_FILLER) {
        trace("m%lu i%zu a%zu o%zu gf",
              slot,
              read_index,
              spec->start_address,
              register_buffer_index);

        return true;
    }

    switch (register_count) {
    case 0:
        break;

    case 1:
        c16.r = register_buffer[register_buffer_index];
        break;

    case 2:
        if (MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(spec->value_type)) {
            endian = "le";
            c32.r[0] = register_buffer[register_buffer_index + 0];
            c32.r[1] = register_buffer[register_buffer_index + 1];
        }
        else {
            endian = "be";
            c32.r[0] = register_buffer[register_buffer_index + 1];
            c32.r[1] = register_buffer[register_buffer_index + 0];
        }

        break;

    case 4:
        if (MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(spec->value_type)) {
            endian = "le";
            c64.r[0] = register_buffer[register_buffer_index + 0];
            c64.r[1] = register_buffer[register_buffer_index + 1];
            c64.r[2] = register_buffer[register_buffer_index + 2];
            c64.r[3] = register_buffer[register_buffer_index + 3];
        }
        else {
            endian = "be";
            c64.r[0] = register_buffer[register_buffer_index + 3];
            c64.r[1] = register_buffer[register_buffer_index + 2];
            c64.r[2] = register_buffer[register_buffer_index + 1];
            c64.r[3] = register_buffer[register_buffer_index + 0];
        }

        break;

    default:
        esp_system_abort("Value has unsupported register count");
    }

    switch (spec->value_type) {
    case ModbusValueType::None:
        break;

    case ModbusValueType::U16:
        trace("m%lu i%zu a%zu o%zu u16 %u",
              slot,
              read_index,
              spec->start_address,
              register_buffer_index,
              c16.u);

        value = static_cast<float>(c16.u);
        break;

    case ModbusValueType::S16:
        trace("m%lu i%zu a%zu o%zu s16 %d",
              slot,
              read_index,
              spec->start_address,
              register_buffer_index,
              static_cast<int16_t>(c16.u));

        value = static_cast<float>(static_cast<int16_t>(c16.u));
        break;

    case ModbusValueType::U32BE:
    case ModbusValueType::U32LE:
        trace("m%lu i%zu a%zu o%zu u32%s %lu",
              slot,
              read_index,
              spec->start_address,
              register_buffer_index,
              endian,
              c32.u);

        value = static_cast<float>(c32.u);
        break;

    case ModbusValueType::S32BE:
    case ModbusValueType::S32LE:
        trace("m%lu i%zu a%zu o%zu s32%s %ld",
              slot,
              read_index,
              spec->start_address,
              register_buffer_index,
              endian,
              static_cast<int32_t>(c32.u));

        value = static_cast<float>(static_cast<int32_t>(c32.u));
        break;

    case ModbusValueType::F32BE:
    case ModbusValueType::F32LE:
        trace("m%lu i%zu a%zu o%zu f32%s %f",
              slot,
              read_index,
              spec->start_address,
              register_buffer_index,
              endian,
              static_cast<double>(c32.f));

        if (!table->f32_negative_max_as_nan || !is_exactly_equal(c32.f, -FLT_MAX)) {
            value = c32.f;
        }

        break;

    case ModbusValueType::U64BE:
    case ModbusValueType::U64LE:
        trace("m%lu i%zu a%zu o%zu u64%s %llu",
              slot,
              read_index,
              spec->start_address,
              register_buffer_index,
              endian,
              c64.u);

        value = static_cast<float>(c64.u);
        break;

    case ModbusValueType::S64BE:
    case ModbusValueType::S64LE:
        trace("m%lu i%zu a%zu o%zu s64%s %lld",
              slot,
              read_index,
              spec->start_address,
              register_buffer_index,
              endian,
              static_cast<int64_t>(c64.u));

        value = static_cast<float>(static_cast<int64_t>(c64.u));
        break;

    case ModbusValueType::F64BE:
    case ModbusValueType::F64LE:
        trace("m%lu i%zu a%zu o%zu f64%s %f",
              slot,
              read_index,
              spec->start_address,
              register_buffer_index,
              endian,
              c64.f);

        value = static_cast<float>(c64.f);
        break;

    default:
        esp_system_abort("Value has unsupported type");
    }

    if (spec->drop_sign) {
        value = fabs(value);
    }

    value += spec->offset;

    if (!is_exactly_zero(value)) {
        // Don't convert 0.0f into -0.0f if the scale factor is negative
        value *= spec->scale_factor;
    }

    if (is_sungrow_hybrid_inverter_inverter_meter()
     && spec->start_address == SungrowHybridInverterOutputTypeAddress::OutputType
     && sungrow_hybrid_inverter.output_type < 0) {
        switch (c16.u) {
        case 0:
            table = &sungrow_hybrid_inverter_1p2l_table;
            logger.printfln_meter("Sungrow 1P2L Hybrid Inverter detected");
            break;

        case 1:
            table = &sungrow_hybrid_inverter_3p4l_table;
            logger.printfln_meter("Sungrow 3P4L Hybrid Inverter detected");
            break;

        case 2:
            table = &sungrow_hybrid_inverter_3p3l_table;
            logger.printfln_meter("Sungrow 3P3L Hybrid Inverter detected");
            break;

        default:
            logger.printfln_meter("Sungrow Hybrid Inverter has unknown output type: %u", c16.u);
            return false;
        }

        sungrow_hybrid_inverter.output_type = c16.u;
        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_sungrow_string_inverter_inverter_meter()
     && spec->start_address == SungrowStringInverterOutputTypeAddress::OutputType
     && sungrow_string_inverter.output_type < 0) {
        switch (c16.u) {
        case 0:
            table = &sungrow_string_inverter_1p2l_table;
            logger.printfln_meter("Sungrow 1P2L String Inverter detected");
            break;

        case 1:
            table = &sungrow_string_inverter_3p4l_table;
            logger.printfln_meter("Sungrow 3P4L String Inverter detected");
            break;

        case 2:
            table = &sungrow_string_inverter_3p3l_table;
            logger.printfln_meter("Sungrow 3P3L String Inverter detected");
            break;

        default:
            logger.printfln_meter("Sungrow String Inverter has unknown output type: %u", c16.u);
            return false;
        }

        sungrow_string_inverter.output_type = c16.u;
        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_deye_hybrid_inverter_battery_meter()
     && spec->start_address == DeyeHybridInverterBatteryDeviceTypeAddress::DeviceType
     && deye_hybrid_inverter.device_type < 0) {
        switch (c16.u) {
        case 0x0002:
        case 0x0003:
        case 0x0004:
            logger.printfln_meter("Deye hybrid inverter has unsupported device type: 0x%04x", c16.u);
            return false;

        case 0x0005:
            deye_hybrid_inverter.low_voltage = true;
            table = &deye_hybrid_inverter_low_voltage_battery_table;
            logger.printfln_meter("Deye hybrid inverter with low-voltage battery detected");
            break;

        case 0x0006:
        case 0x0106:
            deye_hybrid_inverter.high_voltage = true;
            table = &deye_hybrid_inverter_high_voltage_battery_table;
            logger.printfln_meter("Deye hybrid inverter with high-voltage battery detected: 0x%04x", c16.u);
            break;

        default:
            logger.printfln_meter("Deye hybrid inverter has unknown device type: 0x%04x", c16.u);
            return false;
        }

        deye_hybrid_inverter.device_type = c16.u;
        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_deye_hybrid_inverter_pv_meter()
     && spec->start_address == DeyeHybridInverterPVDeviceTypeAddress::DeviceType
     && deye_hybrid_inverter.device_type < 0) {
        switch (c16.u) {
        case 0x0002:
        case 0x0003:
        case 0x0004:
            logger.printfln_meter("Deye hybrid inverter has unsupported device type: 0x%04x", c16.u);
            return false;

        case 0x0005:
            deye_hybrid_inverter.low_voltage = true;
            table = &deye_hybrid_inverter_low_voltage_pv_table;
            logger.printfln_meter("Deye hybrid inverter with low-voltage battery detected");
            break;

        case 0x0006:
        case 0x0106:
            deye_hybrid_inverter.high_voltage = true;
            table = &deye_hybrid_inverter_high_voltage_pv_table;
            logger.printfln_meter("Deye hybrid inverter with high-voltage battery detected: 0x%04x", c16.u);
            break;

        default:
            logger.printfln_meter("Deye hybrid inverter has unknown device type: 0x%04x", c16.u);
            return false;
        }

        deye_hybrid_inverter.device_type = c16.u;
        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_goodwe_inverter_grid_meter()
     && spec->start_address == GoodweInverterGridDetectAddress::ActiveEnergyTotalBuyTotal
     && goodwe_inverter.grid_detect_state == GoodweGridDetectState::Reading) {
        if (c64.u == UINT64_MAX) {
            table = &goodwe_inverter_grid_32bit_energy_table;
            goodwe_inverter.grid_detect_state = GoodweGridDetectState::Detected32bitEnergy;
            logger.printfln_meter("Goodwe inverter with 32-bit grid energy registers detected (reading 64-bit grid energy register returns invalid value)");
        }
        else {
            table = &goodwe_inverter_grid_64bit_energy_table;
            goodwe_inverter.grid_detect_state = GoodweGridDetectState::Detected64bitEnergy;
            logger.printfln_meter("Goodwe inverter with 64-bit grid energy registers detected");
        }

        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_goodwe_inverter_battery_meter()
     && spec->start_address == GoodweInverterBatteryModesAddress::Battery1Mode
     && goodwe_inverter.battery_1_mode < 0) {
        goodwe_inverter.battery_1_mode = c16.u;

        if (goodwe_inverter.battery_1_mode == UINT16_MAX) {
            // The undocumented value 65535 has been observed in the wild. Treat it as "no battery"
            goodwe_inverter.battery_1_mode = 0;
        }

        return true;
    }

    if (is_goodwe_inverter_battery_meter()
     && spec->start_address == GoodweInverterBatteryModesAddress::Battery2Mode
     && goodwe_inverter.battery_2_mode < 0) {
        goodwe_inverter.battery_2_mode = c16.u;

        if (goodwe_inverter.battery_2_mode == UINT16_MAX) {
            // The undocumented value 65535 has been observed in the wild. Treat it as "no battery"
            goodwe_inverter.battery_2_mode = 0;
        }

        if (goodwe_inverter.battery_1_mode != 0 && goodwe_inverter.battery_2_mode != 0) {
            table = &goodwe_inverter_battery_1_and_2_table;
            logger.printfln_meter("Goodwe inverter with battery 1 and 2 detected");
        }
        else if (goodwe_inverter.battery_1_mode != 0 && goodwe_inverter.battery_2_mode == 0) {
            table = &goodwe_inverter_battery_1_table;
            logger.printfln_meter("Goodwe inverter with battery 1 detected");
        }
        else if (goodwe_inverter.battery_1_mode == 0 && goodwe_inverter.battery_2_mode != 0) {
            table = &goodwe_inverter_battery_2_table;
            logger.printfln_meter("Goodwe inverter with battery 2 detected");
        }
        else {
            goodwe_inverter.battery_1_mode = -1;
            goodwe_inverter.battery_2_mode = -1;
            logger.printfln_meter("Goodwe inverter without battery detected");
            return false;
        }

        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_fronius_gen24_plus_battery_meter()
     && spec->start_address == FroniusGEN24PlusBatteryTypeAddress::InputIDOrModelID
     && fronius_gen24_plus.input_id_or_model_id < 0) {
        switch (c16.u) {
        case 1: // module/1/ID: Input ID
            table = &fronius_gen24_plus_battery_integer_table;
            fronius_gen24_plus.start_address_shift = 0;
            logger.printfln_meter("Fronius GEN24 Plus inverter with integer MPPT model detected");
            break;

        case 160: // ID: SunSpec Model ID
            table = &fronius_gen24_plus_battery_float_table;
            fronius_gen24_plus.start_address_shift = 10;
            logger.printfln_meter("Fronius GEN24 Plus inverter with float MPPT model detected");
            break;

        default:
            logger.printfln_meter("Fronius GEN24 Plus inverter has malformed MPPT model: %u", c16.u);
            return false;
        }

        fronius_gen24_plus.input_id_or_model_id = c16.u;
        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_huawei_sun2000_battery_meter()
     && spec->start_address == HuaweiSUN2000BatteryProductModelAddress::EnergyStorageProductModel
     && huawei_sun2000.energy_storage_product_model < 0) {
        switch (c16.u) {
        case 0: // None
            logger.printfln_meter("Huawei SUN2000 inverter has no battery connected");
            return false;

        case 1: // LG RESU
            table = &huawei_sun2000_battery_lg_resu_table;
            logger.printfln_meter("Huawei SUN2000 inverter with LG RESU battery detected");
            break;

        case 2: // Huawei LUNA2000
            table = &huawei_sun2000_battery_huawei_luna2000_table;
            logger.printfln_meter("Huawei SUN2000 inverter with Huawei LUNA2000 battery detected");
            break;

        default:
            logger.printfln_meter("Huawei SUN2000 inverter has unknown battery model: %u", c16.u);
            return false;
        }

        huawei_sun2000.energy_storage_product_model = c16.u;
        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_huawei_sun2000_pv_meter()
     && spec->start_address == HuaweiSUN2000PVStringCountAddress::NumberOfPVStrings
     && huawei_sun2000.number_of_pv_strings < 0) {
        switch (c16.u) {
        case 0:
            table = &huawei_sun2000_pv_no_strings_table;
            break;

        case 1:
            table = &huawei_sun2000_pv_1_string_table;
            break;

        case 2:
            table = &huawei_sun2000_pv_2_strings_table;
            break;

        case 3:
            table = &huawei_sun2000_pv_3_strings_table;
            break;

        case 4:
            table = &huawei_sun2000_pv_4_strings_table;
            break;

        case 5:
            table = &huawei_sun2000_pv_5_strings_table;
            break;

        case 6:
            table = &huawei_sun2000_pv_6_strings_table;
            break;

        case 7:
            table = &huawei_sun2000_pv_7_strings_table;
            break;

        case 8:
            table = &huawei_sun2000_pv_8_strings_table;
            break;

        case 9:
        default:
            table = &huawei_sun2000_pv_9_strings_table;
            break;
        }

        logger.printfln_meter("Huawei SUN2000 inverter has %u PV string%s", c16.u, c16.u != 1 ? "s" : "");
        huawei_sun2000.number_of_pv_strings = c16.u;
        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_huawei_sun2000_smart_dongle_battery_meter()
     && spec->start_address == HuaweiSUN2000SmartDongleBatteryProductModelAddress::EnergyStorageProductModel
     && huawei_sun2000_smart_dongle.energy_storage_product_model < 0) {
        switch (c16.u) {
        case 0: // None
            logger.printfln_meter("Huawei SUN2000 inverter has no battery connected");
            return false;

        case 1: // LG RESU
            table = &huawei_sun2000_smart_dongle_battery_lg_resu_table;
            logger.printfln_meter("Huawei SUN2000 inverter with LG RESU battery detected");
            break;

        case 2: // Huawei LUNA2000
            table = &huawei_sun2000_smart_dongle_battery_huawei_luna2000_table;
            logger.printfln_meter("Huawei SUN2000 inverter with Huawei LUNA2000 battery detected");
            break;

        default:
            logger.printfln_meter("Huawei SUN2000 inverter has unknown battery model: %u", c16.u);
            return false;
        }

        huawei_sun2000_smart_dongle.energy_storage_product_model = c16.u;
        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_fronius_verto_plus_battery_meter()
     && spec->start_address == FroniusVertoPlusBatteryTypeAddress::InputIDOrModelID
     && fronius_verto_plus.input_id_or_model_id < 0) {
        switch (c16.u) {
        case 1: // module/1/ID: Input ID
            table = &fronius_verto_plus_battery_integer_table;
            fronius_verto_plus.start_address_shift = 0;
            logger.printfln_meter("Fronius Verto Plus inverter with integer MPPT model detected");
            break;

        case 160: // ID: SunSpec Model ID
            table = &fronius_verto_plus_battery_float_table;
            fronius_verto_plus.start_address_shift = 10;
            logger.printfln_meter("Fronius Verto Plus inverter with float MPPT model detected");
            break;

        default:
            logger.printfln_meter("Fronius Verto Plus inverter has malformed MPPT model: %u", c16.u);
            return false;
        }

        fronius_verto_plus.input_id_or_model_id = c16.u;
        meters.declare_value_ids(slot, table->ids, table->ids_length);
        return false;
    }

    if (is_sungrow_hybrid_inverter_grid_meter()) {
        if (spec->start_address == SungrowHybridInverterGridAddress::GridFrequency) {
            if (value > 100) {
                // according to the spec the grid frequency is given
                // as 0.1 Hz, but some sungrow inverters report it as 0.01 Hz
                value /= 10;
            }
        }
    }
    else if (is_sungrow_string_inverter_grid_meter()) {
        if (spec->start_address == SungrowStringInverterGridAddress::GridFrequency) {
            if (value > 100) {
                // according to the spec the grid frequency is given
                // as 0.1 Hz, but some sungrow inverters report it as 0.01 Hz
                value /= 10;
            }
        }
    }
    else if (is_sungrow_hybrid_inverter_battery_meter()) {
        // older sungrow inverter firmwares report the battery current and battery power as unsigned
        // values, along with two status flags that indicate charging/discharging mode. newer sungrow
        // inverter firmwares report the battery current as signed value (negative while charging),
        // but the battery power is still reported as an unsigned value. the two status flags are still
        // reported in general but sometimes the status register is all zero. in case the status flags
        // are available use them to derive the charging/discharging mode. in case the status flags are
        // not available derive the charging/discharging mode from the sign of the battery current
        //
        // https://github.com/evcc-io/evcc/issues/18270
        // https://github.com/evcc-io/evcc/pull/18473
        // https://github.com/mkaiser/Sungrow-SHx-Inverter-Modbus-Home-Assistant/blob/a3b82db9a3b33bdda5c35108578d4c7685f82f7d/modbus_sungrow.yaml#L1281
        if (spec->start_address == SungrowHybridInverterBatteryAddress::RunningState) {
            sungrow_hybrid_inverter.running_state = c16.u;
        }
        else if (spec->start_address == SungrowHybridInverterBatteryAddress::BatteryCurrent) {
            // raw battery current might be negative while charging
            sungrow_hybrid_inverter.battery_current = static_cast<int16_t>(c16.u);

            if ((sungrow_hybrid_inverter.running_state & (1 << 1)) != 0) {
                // charging flag is set, force positive battery current
                value = fabs(value);
            }
            else if ((sungrow_hybrid_inverter.running_state & (1 << 2)) != 0) {
                // discharging flag is set, force negative battery current
                value = zero_safe_negation(fabs(value));
            }
            else {
                // neither charging nor discharging flag is set, assume raw battery current is negative
                // while charging. invert it to get positive battery current for charging and negative
                // battery current for discharging
                value = zero_safe_negation(value);
            }
        }
        else if (spec->start_address == SungrowHybridInverterBatteryAddress::BatteryPower) {
            if ((sungrow_hybrid_inverter.running_state & (1 << 1)) != 0) {
                // charging flag is set, force positive battery power
                value = fabs(value);
            }
            else if ((sungrow_hybrid_inverter.running_state & (1 << 2)) != 0) {
                // discharging flag is set, force negative battery power
                value = zero_safe_negation(fabs(value));
            }
            else {
                // neither charging nor discharging flag is set, assume raw battery current is negative while charging
                if (sungrow_hybrid_inverter.battery_current < 0) {
                    // raw battery current is negative (charging), force positive battery power
                    value = fabs(value);
                }
                else {
                    // raw battery current is positive (discharging), force negative battery power
                    value = zero_safe_negation(fabs(value));
                }
            }
        }
    }
    else if (is_victron_energy_gx_load_meter()) {
        if (spec->start_address == VictronEnergyGXLoadAddress::ACConsumptionL1) {
            victron_energy_gx.ac_consumption_l1 = value;
        }
        else if (spec->start_address == VictronEnergyGXLoadAddress::ACConsumptionL2) {
            victron_energy_gx.ac_consumption_l2 = value;
        }
        else if (spec->start_address == VictronEnergyGXLoadAddress::ACConsumptionL3) {
            victron_energy_gx.ac_consumption_l3 = value;
        }
        else if (spec->start_address == VictronEnergyGXLoadAddress::Virtual_ACConsumptionL1AndL2AndL3
              || spec->start_address == VictronEnergyGXLoadAddress::Virtual_ACConsumptionL1AndL2AndL3Signed) {
            value = victron_energy_gx.ac_consumption_l1
                  + victron_energy_gx.ac_consumption_l2
                  + victron_energy_gx.ac_consumption_l3;
        }
    }
    else if (is_victron_energy_gx_pv_meter()) {
        if (spec->start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnOutputL1) {
            victron_energy_gx.ac_coupled_pv_power_on_output_l1 = value;
        }
        else if (spec->start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnOutputL2) {
            victron_energy_gx.ac_coupled_pv_power_on_output_l2 = value;
        }
        else if (spec->start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnOutputL3) {
            victron_energy_gx.ac_coupled_pv_power_on_output_l3 = value;
        }
        else if (spec->start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnInputL1) {
            victron_energy_gx.ac_coupled_pv_power_on_input_l1 = value;
        }
        else if (spec->start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnInputL2) {
            victron_energy_gx.ac_coupled_pv_power_on_input_l2 = value;
        }
        else if (spec->start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnInputL3) {
            victron_energy_gx.ac_coupled_pv_power_on_input_l3 = value;
        }
        else if (spec->start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnGeneratorL1) {
            victron_energy_gx.ac_coupled_pv_power_on_generator_l1 = value;
        }
        else if (spec->start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnGeneratorL2) {
            victron_energy_gx.ac_coupled_pv_power_on_generator_l2 = value;
        }
        else if (spec->start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnGeneratorL3) {
            victron_energy_gx.ac_coupled_pv_power_on_generator_l3 = value;
        }
        else if (spec->start_address == VictronEnergyGXPVAddress::DCCoupledPVPower) {
            victron_energy_gx.dc_coupled_pv_power = value;
        }
        else if (spec->start_address == VictronEnergyGXPVAddress::Virtual_TotalPVPower) {
            value = victron_energy_gx.ac_coupled_pv_power_on_output_l1
                  + victron_energy_gx.ac_coupled_pv_power_on_output_l2
                  + victron_energy_gx.ac_coupled_pv_power_on_output_l3
                  + victron_energy_gx.ac_coupled_pv_power_on_input_l1
                  + victron_energy_gx.ac_coupled_pv_power_on_input_l2
                  + victron_energy_gx.ac_coupled_pv_power_on_input_l3
                  + victron_energy_gx.ac_coupled_pv_power_on_generator_l1
                  + victron_energy_gx.ac_coupled_pv_power_on_generator_l2
                  + victron_energy_gx.ac_coupled_pv_power_on_generator_l3
                  + victron_energy_gx.dc_coupled_pv_power;
        }
    }
    else if (is_goodwe_inverter_battery_meter() && goodwe_inverter.battery_1_mode != 0 && goodwe_inverter.battery_2_mode != 0) {
        if (spec->start_address == GoodweInverterBattery1And2Address::Battery1Voltage) {
            goodwe_inverter.battery_1_voltage = value;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Battery1Current) {
            goodwe_inverter.battery_1_current = value;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Battery1Power) {
            goodwe_inverter.battery_1_power = value;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::BMS1PackTemperature) {
            goodwe_inverter.bms_1_pack_temperature = value;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Battery1Capacity) {
            goodwe_inverter.battery_1_capacity = value;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Battery2Voltage) {
            goodwe_inverter.battery_2_voltage = value;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Battery2Current) {
            goodwe_inverter.battery_2_current = value;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Battery2Power) {
            goodwe_inverter.battery_2_power = value;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::BMS2PackTemperature) {
            goodwe_inverter.bms_2_pack_temperature = value;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Battery2Capacity) {
            goodwe_inverter.battery_2_capacity = value;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Virtual_BatteryVoltageAvg) {
            value = (goodwe_inverter.battery_1_voltage
                  +  goodwe_inverter.battery_2_voltage) / 2.0f;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Virtual_BatteryCurrentSum) {
            value = goodwe_inverter.battery_1_current
                  + goodwe_inverter.battery_2_current;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Virtual_BatteryPowerSum) {
            value = goodwe_inverter.battery_1_power
                  + goodwe_inverter.battery_2_power;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Virtual_BMSPackTemperatureAvg) {
            value = (goodwe_inverter.bms_1_pack_temperature
                  +  goodwe_inverter.bms_2_pack_temperature) / 2.0f;
        }
        else if (spec->start_address == GoodweInverterBattery1And2Address::Virtual_BatteryCapacityAvg) {
            value = (goodwe_inverter.battery_1_capacity
                  +  goodwe_inverter.battery_2_capacity) / 2.0f;
        }
    }
    else if (is_goodwe_inverter_pv_meter()) {
        if (spec->start_address == GoodweInverterPVAddress::PVMode) {
            goodwe_inverter.pv_mode = c32.u;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV1Voltage) {
            if (((goodwe_inverter.pv_mode >> 0) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv1_voltage = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV1Current) {
            if (((goodwe_inverter.pv_mode >> 0) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv1_current = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV1Power) {
            if (((goodwe_inverter.pv_mode >> 0) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv1_power = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV2Voltage) {
            if (((goodwe_inverter.pv_mode >> 8) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv2_voltage = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV2Current) {
            if (((goodwe_inverter.pv_mode >> 8) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv2_current = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV2Power) {
            if (((goodwe_inverter.pv_mode >> 8) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv2_power = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV3Voltage) {
            if (((goodwe_inverter.pv_mode >> 16) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv3_voltage = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV3Current) {
            if (((goodwe_inverter.pv_mode >> 16) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv3_current = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV3Power) {
            if (((goodwe_inverter.pv_mode >> 16) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv3_power = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV4Voltage) {
            if (((goodwe_inverter.pv_mode >> 24) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv4_voltage = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV4Current) {
            if (((goodwe_inverter.pv_mode >> 24) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv4_current = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::PV4Power) {
            if (((goodwe_inverter.pv_mode >> 24) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_inverter.pv4_power = value;
        }
        else if (spec->start_address == GoodweInverterPVAddress::Virtual_AveragePVVoltage) {
            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(goodwe_inverter.pv1_voltage)) {
                voltage_sum += goodwe_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(goodwe_inverter.pv2_voltage)) {
                voltage_sum += goodwe_inverter.pv2_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(goodwe_inverter.pv3_voltage)) {
                voltage_sum += goodwe_inverter.pv3_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(goodwe_inverter.pv4_voltage)) {
                voltage_sum += goodwe_inverter.pv4_voltage;
                ++voltage_count;
            }

            value = voltage_sum / voltage_count;
        }
        else if (spec->start_address == GoodweInverterPVAddress::Virtual_TotalPVCurrent) {
            value = goodwe_inverter.pv1_current
                  + goodwe_inverter.pv2_current
                  + goodwe_inverter.pv3_current
                  + goodwe_inverter.pv4_current;
        }
        else if (spec->start_address == GoodweInverterPVAddress::Virtual_TotalPVPower) {
            value = goodwe_inverter.pv1_power
                  + goodwe_inverter.pv2_power
                  + goodwe_inverter.pv3_power
                  + goodwe_inverter.pv4_power;
        }
    }
    else if (is_fronius_gen24_plus_battery_meter()) {
        size_t start_address = spec->start_address - fronius_gen24_plus.start_address_shift;

        if (start_address == FroniusGEN24PlusBatteryIntegerAddress::DCA_SF) {
            fronius_gen24_plus.dca_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::DCV_SF) {
            fronius_gen24_plus.dcv_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::DCW_SF) {
            fronius_gen24_plus.dcw_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::DCWH_SF) {
            fronius_gen24_plus.dcwh_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::ChargeDCA) {
            fronius_gen24_plus.charge_dca = value; // SunSpec: uint16
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::ChargeDCV) {
            fronius_gen24_plus.charge_dcv = value; // SunSpec: uint16
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::ChargeDCW) {
            fronius_gen24_plus.charge_dcw = value; // SunSpec: uint16
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::ChargeDCWH) {
            // Is 0 in firmware versions <= 1.30 while discharging. As this is an acc32 map 0 to NaN. This will make the
            // meters framework ignore this value during discharging and keep the pervious value
            fronius_gen24_plus.charge_dcwh = c32.u == 0 ? NAN : value; // SunSpec: acc32
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::DischargeDCA) {
            fronius_gen24_plus.discharge_dca = value; // SunSpec: uint16
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::DischargeDCV) {
            fronius_gen24_plus.discharge_dcv = value; // SunSpec: uint16
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::DischargeDCW) {
            fronius_gen24_plus.discharge_dcw = value; // SunSpec: uint16
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::DischargeDCWH) {
            // Is 0 in firmware versions <= 1.30 while charging. As this is an acc32 map 0 to NaN. This will make the
            // meters framework ignore this value during charging and keep the pervious value
            fronius_gen24_plus.discharge_dcwh = c32.u == 0 ? NAN : value; // SunSpec: acc32
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::ChaState) {
            fronius_gen24_plus.chastate = value; // SunSpec: uint16
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::ChaState_SF) {
            fronius_gen24_plus.chastate_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::Virtual_BatteryCurrent) {
            float dca_scale_factor = get_sun_spec_scale_factor(fronius_gen24_plus.dca_sf);
            float charge_dca       = fronius_gen24_plus.charge_dca    * dca_scale_factor;
            float discharge_dca    = fronius_gen24_plus.discharge_dca * dca_scale_factor;

            value = charge_dca - discharge_dca; // One of the two value is always 0A
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::Virtual_BatteryVoltage) {
            float dcv_scale_factor = get_sun_spec_scale_factor(fronius_gen24_plus.dcv_sf);
            float charge_dcv       = fronius_gen24_plus.charge_dcv    * dcv_scale_factor;
            float discharge_dcv    = fronius_gen24_plus.discharge_dcv * dcv_scale_factor;

            value = std::max(charge_dcv, discharge_dcv); // In firmware versions <= 1.30 one of the two values is always 0V. In firmware versions >= 1.31 they are the same
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::Virtual_BatteryPower) {
            float dcw_scale_factor = get_sun_spec_scale_factor(fronius_gen24_plus.dcw_sf);
            float charge_dcw       = fronius_gen24_plus.charge_dcw    * dcw_scale_factor;
            float discharge_dcw    = fronius_gen24_plus.discharge_dcw * dcw_scale_factor;

            value = charge_dcw - discharge_dcw; // One of the two value is always 0W
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::Virtual_BatteryStateOfCharge) {
            value = fronius_gen24_plus.chastate * get_sun_spec_scale_factor(fronius_gen24_plus.chastate_sf);
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::Virtual_BatteryChargeEnergy) {
            value = fronius_gen24_plus.charge_dcwh * get_sun_spec_scale_factor(fronius_gen24_plus.dcwh_sf) * 0.001f;
        }
        else if (start_address == FroniusGEN24PlusBatteryIntegerAddress::Virtual_BatteryDischargeEnergy) {
            value = fronius_gen24_plus.discharge_dcwh * get_sun_spec_scale_factor(fronius_gen24_plus.dcwh_sf) * 0.001f;
        }
    }
    else if (is_solaredge_inverter_battery_meter()) {
        if (spec->start_address == SolaredgeInverterBatteryAddress::Battery1RatedEnergy) {
            solaredge_inverter.battery_1_rated_energy = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery1AverageTemperature) {
            solaredge_inverter.battery_1_temperature = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery1InstantaneousVoltage) {
            solaredge_inverter.battery_1_voltage = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery1InstantaneousCurrent) {
            solaredge_inverter.battery_1_current = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery1InstantaneousPower) {
            solaredge_inverter.battery_1_power = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery1LifetimeExportEnergyCounter) {
            solaredge_inverter.battery_1_export_energy = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery1LifetimeImportEnergyCounter) {
            solaredge_inverter.battery_1_import_energy = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery1StateOfEnergy) {
            solaredge_inverter.battery_1_state_of_charge = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery1Status) {
            solaredge_inverter.battery_1_status = c32.u;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery2RatedEnergy) {
            solaredge_inverter.battery_2_rated_energy = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery2AverageTemperature) {
            solaredge_inverter.battery_2_temperature = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery2InstantaneousVoltage) {
            solaredge_inverter.battery_2_voltage = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery2InstantaneousCurrent) {
            solaredge_inverter.battery_2_current = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery2InstantaneousPower) {
            solaredge_inverter.battery_2_power = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery2LifetimeExportEnergyCounter) {
            solaredge_inverter.battery_2_export_energy = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery2LifetimeImportEnergyCounter) {
            solaredge_inverter.battery_2_import_energy = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery2StateOfEnergy) {
            solaredge_inverter.battery_2_state_of_charge = value;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Battery2Status) {
            solaredge_inverter.battery_2_status = c32.u;
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Virtual_Battery1And2RatedEnergy) {
            value = nan_safe_sum(solaredge_inverter.battery_1_status == 0 /* off */ ? NAN : solaredge_inverter.battery_1_rated_energy,
                                 solaredge_inverter.battery_2_status == 0 /* off */ ? NAN : solaredge_inverter.battery_2_rated_energy);
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Virtual_Battery1And2AverageTemperature) {
            value = nan_safe_avg(solaredge_inverter.battery_1_status == 0 /* off */ ? NAN : solaredge_inverter.battery_1_temperature,
                                 solaredge_inverter.battery_2_status == 0 /* off */ ? NAN : solaredge_inverter.battery_2_temperature);
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Virtual_Battery1And2InstantaneousVoltage) {
            value = nan_safe_avg(solaredge_inverter.battery_1_status == 0 /* off */ ? NAN : solaredge_inverter.battery_1_voltage,
                                 solaredge_inverter.battery_2_status == 0 /* off */ ? NAN : solaredge_inverter.battery_2_voltage);
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Virtual_Battery1And2InstantaneousCurrent) {
            value = nan_safe_sum(solaredge_inverter.battery_1_status == 0 /* off */ ? NAN : solaredge_inverter.battery_1_current,
                                 solaredge_inverter.battery_2_status == 0 /* off */ ? NAN : solaredge_inverter.battery_2_current);
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Virtual_Battery1And2InstantaneousPower) {
            value = nan_safe_sum(solaredge_inverter.battery_1_status == 0 /* off */ ? NAN : solaredge_inverter.battery_1_power,
                                 solaredge_inverter.battery_2_status == 0 /* off */ ? NAN : solaredge_inverter.battery_2_power);
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Virtual_Battery1And2LifetimeExportEnergyCounter) {
            value = nan_safe_sum(solaredge_inverter.battery_1_status == 0 /* off */ ? NAN : solaredge_inverter.battery_1_export_energy,
                                 solaredge_inverter.battery_2_status == 0 /* off */ ? NAN : solaredge_inverter.battery_2_export_energy);
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Virtual_Battery1And2LifetimeImportEnergyCounter) {
            value = nan_safe_sum(solaredge_inverter.battery_1_status == 0 /* off */ ? NAN : solaredge_inverter.battery_1_import_energy,
                                 solaredge_inverter.battery_2_status == 0 /* off */ ? NAN : solaredge_inverter.battery_2_import_energy);
        }
        else if (spec->start_address == SolaredgeInverterBatteryAddress::Virtual_Battery1And2StateOfEnergy) {
            value = nan_safe_avg(solaredge_inverter.battery_1_status == 0 /* off */ ? NAN : solaredge_inverter.battery_1_state_of_charge,
                                 solaredge_inverter.battery_2_status == 0 /* off */ ? NAN : solaredge_inverter.battery_2_state_of_charge);
        }
    }
    else if (is_solaredge_inverter_pv_meter()) {
        if (spec->start_address == SolaredgeInverterPVAddress::Battery1InstantaneousPower) {
            solaredge_inverter.battery_1_power = value;
        }
        else if (spec->start_address == SolaredgeInverterPVAddress::Battery1Status) {
            solaredge_inverter.battery_1_status = c32.u;
        }
        else if (spec->start_address == SolaredgeInverterPVAddress::Battery2InstantaneousPower) {
            solaredge_inverter.battery_2_power = value;
        }
        else if (spec->start_address == SolaredgeInverterPVAddress::Battery2Status) {
            solaredge_inverter.battery_2_status = c32.u;
        }
        else if (spec->start_address == SolaredgeInverterPVAddress::DCPower) {
            solaredge_inverter.dc_power = value;
        }
        else if (spec->start_address == SolaredgeInverterPVAddress::DCPower_SF) {
            solaredge_inverter.dc_power_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (spec->start_address == SolaredgeInverterPVAddress::Virtual_PVPower) {
            float battery_power = nan_safe_sum(solaredge_inverter.battery_1_status == 0 /* off */ ? NAN : solaredge_inverter.battery_1_power,
                                               solaredge_inverter.battery_2_status == 0 /* off */ ? NAN : solaredge_inverter.battery_2_power);

            if (isnan(battery_power)) {
                battery_power = 0.0f;
            }

            float dc_power_scale_factor = get_sun_spec_scale_factor(solaredge_inverter.dc_power_sf);
            float dc_power = solaredge_inverter.dc_power * dc_power_scale_factor * -1.0f;

            // inverter import (+), battery charge (+):    P_PV = P_bat - P_inv
            // inverter import (+), battery discharge (-): P_PV = 0 [should be impossible]
            // inverter export (-), battery charge (+):    P_PV = abs(P_inv) + P_bat
            // inverter export (-), battery discharge (-): P_PV = abs(P_inv) - abs(P_bat)
            //
            // this formula covers all cases:              P_PV = max(P_bat - P_inv, 0)
            //
            // the maximum with 0 is there to avoid negative PV power in case of
            // inaccurate measurements or the import/discharge actually happending

            value = std::max(battery_power - dc_power, 0.0f);
        }
    }
    else if (is_fox_ess_h3_smart_hybrid_inverter_battery_1_meter()) {
        if (spec->start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1Voltage) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1Current) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1Temperature) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1SoC) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1DesignEnergy) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS2ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1Address::TotalDischargeEnergy) {
            // this is the combined discharge energy of both batteries, only show if battery 2 is offline
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status != 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1Address::TotalChargeEnergy) {
            // this is the combined charge energy of both batteries, only show if battery 2 is offline
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status != 0) {
                value = NAN;
            }
        }
    }
    else if (is_fox_ess_h3_smart_hybrid_inverter_battery_2_meter()) {
        if (spec->start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2Voltage) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2Current) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2Temperature) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2SoC) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2DesignEnergy) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS1ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery2Address::TotalDischargeEnergy) {
            // this is the combined discharge energy of both batteries, only show if battery 1 is offline
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status != 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery2Address::TotalChargeEnergy) {
            // this is the combined charge energy of both batteries, only show if battery 1 is offline
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status != 0) {
                value = NAN;
            }
        }
    }
    else if (is_fox_ess_h3_smart_hybrid_inverter_battery_1_and_2_meter()) {
        if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1Voltage) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_smart_hybrid_inverter.bms_1_voltage = value;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1Current) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_smart_hybrid_inverter.bms_1_current = value;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1Temperature) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_smart_hybrid_inverter.bms_1_temperature = value;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1SoC) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_smart_hybrid_inverter.bms_1_soc = value;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1DesignEnergy) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_smart_hybrid_inverter.bms_1_design_energy = value;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2Voltage) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_smart_hybrid_inverter.bms_1_voltage, value);
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2Current) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_sum(fox_ess_h3_smart_hybrid_inverter.bms_1_current, value);
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2Temperature) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_smart_hybrid_inverter.bms_1_temperature, value);
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2SoC) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_smart_hybrid_inverter.bms_1_soc, value);
        }
        else if (spec->start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2DesignEnergy) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_sum(fox_ess_h3_smart_hybrid_inverter.bms_1_design_energy, value);
        }
    }
    else if (is_fox_ess_h3_pro_hybrid_inverter_battery_1_meter()) {
        if (spec->start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1Voltage) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1Current) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1Temperature) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1SoC) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1DesignEnergy) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1Address::BMS2ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1Address::TotalDischargeEnergy) {
            // this is the combined discharge energy of both batteries, only show if battery 2 is offline
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status != 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1Address::TotalChargeEnergy) {
            // this is the combined charge energy of both batteries, only show if battery 2 is offline
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status != 0) {
                value = NAN;
            }
        }
    }
    else if (is_fox_ess_h3_pro_hybrid_inverter_battery_2_meter()) {
        if (spec->start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2Voltage) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2Current) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2Temperature) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2SoC) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2DesignEnergy) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery2Address::BMS1ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery2Address::TotalDischargeEnergy) {
            // this is the combined discharge energy of both batteries, only show if battery 1 is offline
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status != 0) {
                value = NAN;
            }
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery2Address::TotalChargeEnergy) {
            // this is the combined charge energy of both batteries, only show if battery 1 is offline
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status != 0) {
                value = NAN;
            }
        }
    }
    else if (is_fox_ess_h3_pro_hybrid_inverter_battery_1_and_2_meter()) {
        if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1Voltage) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_pro_hybrid_inverter.bms_1_voltage = value;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1Current) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_pro_hybrid_inverter.bms_1_current = value;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1Temperature) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_pro_hybrid_inverter.bms_1_temperature = value;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1SoC) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_pro_hybrid_inverter.bms_1_soc = value;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1DesignEnergy) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_pro_hybrid_inverter.bms_1_design_energy = value;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2Voltage) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_pro_hybrid_inverter.bms_1_voltage, value);
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2Current) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_sum(fox_ess_h3_pro_hybrid_inverter.bms_1_current, value);
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2Temperature) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_pro_hybrid_inverter.bms_1_temperature, value);
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2SoC) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_pro_hybrid_inverter.bms_1_soc, value);
        }
        else if (spec->start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2DesignEnergy) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_sum(fox_ess_h3_pro_hybrid_inverter.bms_1_design_energy, value);
        }
    }
    else if (is_sma_hybrid_inverter_battery_meter()) {
        if (spec->start_address == SMAHybridInverterBatteryAddress::BatChrgCurBatCha) {
            sma_hybrid_inverter.battery_charge_power = value;
        }
        else if (spec->start_address == SMAHybridInverterBatteryAddress::BatDschCurBatDsch) {
            sma_hybrid_inverter.battery_discharge_power = value;
        }
        else if (spec->start_address == SMAHybridInverterBatteryAddress::Virtual_BatteryPower) {
            value = sma_hybrid_inverter.battery_charge_power - sma_hybrid_inverter.battery_discharge_power;
        }
    }
    else if (is_varta_flex_grid_meter()) {
        if (spec->start_address == VARTAFlexGridAddress::GridPower) {
            varta_flex.grid_power = value;
        }
        else if (spec->start_address == VARTAFlexGridAddress::GridPowerSF) {
            varta_flex.grid_power_sf = static_cast<int16_t>(c16.u);
        }
        else if (spec->start_address == VARTAFlexGridAddress::Virtual_ScaledGridPower) {
            value = varta_flex.grid_power * get_sun_spec_scale_factor(varta_flex.grid_power_sf);
        }
    }
    else if (is_varta_flex_battery_meter()) {
        if (spec->start_address == VARTAFlexBatteryAddress::ActivePower) {
            varta_flex.battery_active_power = value;
        }
        else if (spec->start_address == VARTAFlexBatteryAddress::ApparentPower) {
            varta_flex.battery_apparent_power = value;
        }
        else if (spec->start_address == VARTAFlexBatteryAddress::TotalChargeEnergy) {
            varta_flex.battery_total_charge_energy = value;
        }
        else if (spec->start_address == VARTAFlexBatteryAddress::ActivePowerSF) {
            varta_flex.battery_active_power_sf = static_cast<int16_t>(c16.u);
        }
        else if (spec->start_address == VARTAFlexBatteryAddress::ApparentPowerSF) {
            varta_flex.battery_apparent_power_sf = static_cast<int16_t>(c16.u);
        }
        else if (spec->start_address == VARTAFlexBatteryAddress::TotalChargeEnergySF) {
            varta_flex.battery_total_charge_energy_sf = static_cast<int16_t>(c16.u);
        }
        else if (spec->start_address == VARTAFlexBatteryAddress::Virtual_ScaledActivePower) {
            value = varta_flex.battery_active_power * get_sun_spec_scale_factor(varta_flex.battery_active_power_sf);
        }
        else if (spec->start_address == VARTAFlexBatteryAddress::Virtual_ScaledApparentPower) {
            value = varta_flex.battery_apparent_power * get_sun_spec_scale_factor(varta_flex.battery_apparent_power_sf);
        }
        else if (spec->start_address == VARTAFlexBatteryAddress::Virtual_ScaledTotalChargeEnergy) {
            value = varta_flex.battery_total_charge_energy * get_sun_spec_scale_factor(varta_flex.battery_total_charge_energy_sf);
        }
    }
    else if (is_solis_hybrid_inverter_battery_meter()) {
        if (spec->start_address == SolisHybridInverterBatteryAddress::BatteryCurrent) {
            solis_hybrid_inverter.battery_current = value;
        }
        else if (spec->start_address == SolisHybridInverterBatteryAddress::BatteryCurrentDirection) {
            solis_hybrid_inverter.battery_current_direction = c16.u;
        }
        else if (spec->start_address == SolisHybridInverterBatteryAddress::Virtual_BatteryCurrentSigned) {
            value = solis_hybrid_inverter.battery_current;

            if (solis_hybrid_inverter.battery_current_direction != 0) { // 0 = charge, 1 = discharge
                value = zero_safe_negation(value);
            }
        }
        else if (spec->start_address == SolisHybridInverterBatteryAddress::BatteryPower) {
            solis_hybrid_inverter.battery_power = value;
        }
        else if (spec->start_address == SolisHybridInverterBatteryAddress::Virtual_BatteryPowerSigned) {
            value = solis_hybrid_inverter.battery_power;

            if (solis_hybrid_inverter.battery_current_direction != 0) { // 0 = charge, 1 = discharge
                value = zero_safe_negation(value);
            }
        }
    }
    else if (is_solis_hybrid_inverter_load_meter()) {
        if (spec->start_address == SolisHybridInverterLoadAddress::HouseLoadPower) {
            solis_hybrid_inverter.house_load_power = value;
        }
        else if (spec->start_address == SolisHybridInverterLoadAddress::BackupLoadPower) {
            solis_hybrid_inverter.backup_load_power = value;
        }
        else if (spec->start_address == SolisHybridInverterLoadAddress::Virtual_TotalLoadPower
              || spec->start_address == SolisHybridInverterLoadAddress::Virtual_TotalLoadPowerSigned) {
            value = solis_hybrid_inverter.house_load_power + solis_hybrid_inverter.backup_load_power;
        }
    }
    else if (is_solis_hybrid_inverter_pv_meter()) {
        if (spec->start_address == SolisHybridInverterPVAddress::DCVoltage1) {
            solis_hybrid_inverter.dc_voltage_1 = value;
        }
        else if (spec->start_address == SolisHybridInverterPVAddress::DCCurrent1) {
            solis_hybrid_inverter.dc_current_1 = value;
        }
        else if (spec->start_address == SolisHybridInverterPVAddress::DCVoltage2) {
            solis_hybrid_inverter.dc_voltage_2 = value;
        }
        else if (spec->start_address == SolisHybridInverterPVAddress::DCCurrent2) {
            solis_hybrid_inverter.dc_current_2 = value;
        }
        else if (spec->start_address == SolisHybridInverterPVAddress::DCVoltage3) {
            solis_hybrid_inverter.dc_voltage_3 = value;
        }
        else if (spec->start_address == SolisHybridInverterPVAddress::DCCurrent3) {
            solis_hybrid_inverter.dc_current_3 = value;
        }
        else if (spec->start_address == SolisHybridInverterPVAddress::DCVoltage4) {
            solis_hybrid_inverter.dc_voltage_4 = value;
        }
        else if (spec->start_address == SolisHybridInverterPVAddress::DCCurrent4) {
            solis_hybrid_inverter.dc_current_4 = value;
        }
        else if (spec->start_address == SolisHybridInverterPVAddress::Virtual_AverageDCVoltage) {
            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            // Use current != 0 as indicator to avoid phantom voltage
            if (!is_exactly_zero(solis_hybrid_inverter.dc_current_1)) {
                voltage_sum += solis_hybrid_inverter.dc_voltage_1;
                ++voltage_count;
            }

            // Use current != 0 as indicator to avoid phantom voltage
            if (!is_exactly_zero(solis_hybrid_inverter.dc_current_2)) {
                voltage_sum += solis_hybrid_inverter.dc_voltage_2;
                ++voltage_count;
            }

            // Use current != 0 as indicator to avoid phantom voltage
            if (!is_exactly_zero(solis_hybrid_inverter.dc_current_3)) {
                voltage_sum += solis_hybrid_inverter.dc_voltage_3;
                ++voltage_count;
            }

            // Use current != 0 as indicator to avoid phantom voltage
            if (!is_exactly_zero(solis_hybrid_inverter.dc_current_4)) {
                voltage_sum += solis_hybrid_inverter.dc_voltage_4;
                ++voltage_count;
            }

            value = voltage_sum / voltage_count;
        }
        else if (spec->start_address == SolisHybridInverterPVAddress::Virtual_TotalDCCurrent) {
            value = solis_hybrid_inverter.dc_current_1
                  + solis_hybrid_inverter.dc_current_2
                  + solis_hybrid_inverter.dc_current_3
                  + solis_hybrid_inverter.dc_current_4;
        }
    }
    else if (is_growatt_hybrid_inverter_grid_meter()) {
        if (spec->start_address == GrowattHybridInverterGridAddress::PtouserTotalTotalForwardPower) {
            growatt_hybrid_inverter.grid_forward_power = value;
        }
        else if (spec->start_address == GrowattHybridInverterGridAddress::PtogridTotalTotalReversePower) {
            growatt_hybrid_inverter.grid_reverse_power = value;
        }
        else if (spec->start_address == GrowattHybridInverterGridAddress::Virtual_TotalPowerSigned) {
            value = growatt_hybrid_inverter.grid_forward_power - growatt_hybrid_inverter.grid_reverse_power;
        }
    }
    else if (is_growatt_hybrid_inverter_battery_meter()) {
        if (spec->start_address == GrowattHybridInverterBatteryAddress::IbatBatteryCurrent) {
            growatt_hybrid_inverter.battery_current = value;
        }
        else if (spec->start_address == GrowattHybridInverterBatteryAddress::PdischrDischargePower) {
            growatt_hybrid_inverter.battery_discharge_power = value;
        }
        else if (spec->start_address == GrowattHybridInverterBatteryAddress::PchrChargePower) {
            growatt_hybrid_inverter.battery_power_signed = value - growatt_hybrid_inverter.battery_discharge_power;
        }
        else if (spec->start_address == GrowattHybridInverterBatteryAddress::Virtual_BatteryCurrentSigned) {
            value = growatt_hybrid_inverter.battery_current;

            if (growatt_hybrid_inverter.battery_power_signed < 0.0f) {
                value = zero_safe_negation(value);
            }
        }
        else if (spec->start_address == GrowattHybridInverterBatteryAddress::Virtual_BatteryPowerSigned) {
            value = growatt_hybrid_inverter.battery_power_signed;
        }
    }
    else if (is_growatt_hybrid_inverter_load_meter()) {
        if (spec->start_address == GrowattHybridInverterLoadAddress::PtoloadTotalTotalLoadPower) {
            growatt_hybrid_inverter.total_load_power = value;
        }
        else if (spec->start_address == GrowattHybridInverterLoadAddress::Virtual_TotalLoadPowerSigned) {
            value = growatt_hybrid_inverter.total_load_power;
        }
    }
    else if (is_fronius_verto_plus_battery_meter()) {
        size_t start_address = spec->start_address - fronius_verto_plus.start_address_shift;

        if (start_address == FroniusVertoPlusBatteryIntegerAddress::DCA_SF) {
            fronius_verto_plus.dca_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::DCV_SF) {
            fronius_verto_plus.dcv_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::DCW_SF) {
            fronius_verto_plus.dcw_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::DCWH_SF) {
            fronius_verto_plus.dcwh_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::ChargeDCA) {
            fronius_verto_plus.charge_dca = value; // SunSpec: uint16
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::ChargeDCV) {
            fronius_verto_plus.charge_dcv = value; // SunSpec: uint16
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::ChargeDCW) {
            fronius_verto_plus.charge_dcw = value; // SunSpec: uint16
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::ChargeDCWH) {
            // Is 0 in firmware versions <= 1.30 while discharging. As this is an acc32 map 0 to NaN. This will make the
            // meters framework ignore this value during discharging and keep the pervious value
            fronius_verto_plus.charge_dcwh = c32.u == 0 ? NAN : value; // SunSpec: acc32
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::DischargeDCA) {
            fronius_verto_plus.discharge_dca = value; // SunSpec: uint16
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::DischargeDCV) {
            fronius_verto_plus.discharge_dcv = value; // SunSpec: uint16
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::DischargeDCW) {
            fronius_verto_plus.discharge_dcw = value; // SunSpec: uint16
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::DischargeDCWH) {
            // Is 0 in firmware versions <= 1.30 while charging. As this is an acc32 map 0 to NaN. This will make the
            // meters framework ignore this value during charging and keep the pervious value
            fronius_verto_plus.discharge_dcwh = c32.u == 0 ? NAN : value; // SunSpec: acc32
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::ChaState) {
            fronius_verto_plus.chastate = value; // SunSpec: uint16
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::ChaState_SF) {
            fronius_verto_plus.chastate_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::Virtual_BatteryCurrent) {
            float dca_scale_factor = get_sun_spec_scale_factor(fronius_verto_plus.dca_sf);
            float charge_dca       = fronius_verto_plus.charge_dca    * dca_scale_factor;
            float discharge_dca    = fronius_verto_plus.discharge_dca * dca_scale_factor;

            value = charge_dca - discharge_dca; // One of the two value is always 0A
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::Virtual_BatteryVoltage) {
            float dcv_scale_factor = get_sun_spec_scale_factor(fronius_verto_plus.dcv_sf);
            float charge_dcv       = fronius_verto_plus.charge_dcv    * dcv_scale_factor;
            float discharge_dcv    = fronius_verto_plus.discharge_dcv * dcv_scale_factor;

            value = std::max(charge_dcv, discharge_dcv); // In firmware versions <= 1.30 one of the two values is always 0V. In firmware versions >= 1.31 they are the same
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::Virtual_BatteryPower) {
            float dcw_scale_factor = get_sun_spec_scale_factor(fronius_verto_plus.dcw_sf);
            float charge_dcw       = fronius_verto_plus.charge_dcw    * dcw_scale_factor;
            float discharge_dcw    = fronius_verto_plus.discharge_dcw * dcw_scale_factor;

            value = charge_dcw - discharge_dcw; // One of the two value is always 0W
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::Virtual_BatteryStateOfCharge) {
            value = fronius_verto_plus.chastate * get_sun_spec_scale_factor(fronius_verto_plus.chastate_sf);
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::Virtual_BatteryChargeEnergy) {
            value = fronius_verto_plus.charge_dcwh * get_sun_spec_scale_factor(fronius_verto_plus.dcwh_sf) * 0.001f;
        }
        else if (start_address == FroniusVertoPlusBatteryIntegerAddress::Virtual_BatteryDischargeEnergy) {
            value = fronius_verto_plus.discharge_dcwh * get_sun_spec_scale_factor(fronius_verto_plus.dcwh_sf) * 0.001f;
        }
    }

#include "generated/meter_modbus_tcp_calculate_values.inc"

    if (spec->start_address >= START_ADDRESS_VIRTUAL_BASE) {
        trace("m%lu i%zu v %f",
              slot,
              read_index,
              static_cast<double>(value));
    }

    if (table->index[read_index] < VALUE_INDEX_CUT_OFF) {
        meters.update_value(slot, table->index[read_index], value);
    }

    return true;
}
