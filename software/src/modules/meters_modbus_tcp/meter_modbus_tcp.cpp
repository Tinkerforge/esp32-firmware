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
#include "module_dependencies.h"
#include "meter_modbus_tcp_specs.h"
#include "tools/float.h"
#include "tools/sun_spec.h"

#include "gcc_warnings.h"

// 1P2L, 3P4L and 3P3L use the same addresses, but different amounts of phase voltage registers
namespace SungrowHybridInverterAddress = SungrowHybridInverter1P2LAddress;

// 1P2L, 3P4L and 3P3L use the same addresses, but different amounts of phase voltage registers
namespace SungrowStringInverterAddress = SungrowStringInverter1P2LAddress;

// LowVoltage and HighVoltage use the same addresses, but different scale factors
namespace DeyeHybridInverterPVAddress = DeyeHybridInverterLowVoltagePVAddress;

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

//#define DEBUG_VALUES_TO_TRACE_LOG

#define NUMBER_TO_ADDRESS(number) ((number) - 1u)

#define MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(x) (static_cast<uint8_t>(x) & 0x07)
#define MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(x) ((static_cast<uint8_t>(x) >> 5) & 1)

#define trace(fmt, ...) \
    do { \
        meters_modbus_tcp.trace_timestamp(); \
        logger.tracefln_plain(trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

MeterClassID MeterModbusTCP::get_class() const
{
    return MeterClassID::ModbusTCP;
}

void MeterModbusTCP::setup(Config *ephemeral_config)
{
    host      = ephemeral_config->get("host")->asString();
    port      = static_cast<uint16_t>(ephemeral_config->get("port")->asUint());
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

                custom_specs[i].name = getMeterValueName(value_id);
                custom_specs[i].register_type = registers->get(i)->get("rtype")->asEnum<ModbusRegisterType>();
                custom_specs[i].start_address = registers->get(i)->get("addr")->asUint();
                custom_specs[i].value_type = registers->get(i)->get("vtype")->asEnum<ModbusValueType>();
                custom_specs[i].drop_sign = false; // FIXME: expose in API?
                custom_specs[i].offset = registers->get(i)->get("off")->asFloat();
                custom_specs[i].scale_factor = registers->get(i)->get("scale")->asFloat();

                custom_ids[i] = value_id;

                custom_index[i] = i;
            }

            custom_table->default_location = MeterLocation::Unknown;
            custom_table->specs = custom_specs;
            custom_table->specs_length = registers_count;
            custom_table->ids = custom_ids;
            custom_table->ids_length = registers_count;
            custom_table->index = custom_index;

            table = custom_table;
        }

        break;

    case MeterModbusTCPTableID::SungrowHybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        sungrow_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<SungrowHybridInverterVirtualMeter>();
        sungrow_hybrid_inverter.output_type = -1;
        table = get_sungrow_hybrid_inverter_table(slot, sungrow_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::SungrowStringInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        sungrow_string_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<SungrowStringInverterVirtualMeter>();
        sungrow_string_inverter.output_type = -1;
        table = get_sungrow_string_inverter_table(slot, sungrow_string_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::SolarmaxMaxStorage:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        solarmax_max_storage.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<SolarmaxMaxStorageVirtualMeter>();
        table = get_solarmax_max_storage_table(slot, solarmax_max_storage.virtual_meter);
        break;

    case MeterModbusTCPTableID::VictronEnergyGX:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        victron_energy_gx.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<VictronEnergyGXVirtualMeter>();
        table = get_victron_energy_gx_table(slot, victron_energy_gx.virtual_meter);
        break;

    case MeterModbusTCPTableID::DeyeHybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        deye_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<DeyeHybridInverterVirtualMeter>();
        deye_hybrid_inverter.device_type = -1;
        table = get_deye_hybrid_inverter_table(slot, deye_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::AlphaESSHybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        alpha_ess_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<AlphaESSHybridInverterVirtualMeter>();
        table = get_alpha_ess_hybrid_inverter_table(slot, alpha_ess_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::ShellyProEM:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        shelly_pro_em.monophase_channel = ephemeral_table_config->get("monophase_channel")->asEnum<ShellyEMMonophaseChannel>();
        shelly_pro_em.monophase_mapping = ephemeral_table_config->get("monophase_mapping")->asEnum<ShellyEMMonophaseMapping>();

        switch (shelly_pro_em.monophase_channel) {
        case ShellyEMMonophaseChannel::None:
            logger.printfln_meter("No Shelly Pro EM Monophase Channel selected");
            break;

        case ShellyEMMonophaseChannel::First:
            switch (shelly_pro_em.monophase_mapping) {
            case ShellyEMMonophaseMapping::None:
                logger.printfln_meter("No Shelly Pro EM Monophase Mapping selected");
                break;

            case ShellyEMMonophaseMapping::L1:
                table = &shelly_em_monophase_channel_1_as_l1_table;
                break;

            case ShellyEMMonophaseMapping::L2:
                table = &shelly_em_monophase_channel_1_as_l2_table;
                break;

            case ShellyEMMonophaseMapping::L3:
                table = &shelly_em_monophase_channel_1_as_l3_table;
                break;

            default:
                logger.printfln_meter("Unknown Shelly Pro EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_em.monophase_mapping));
                break;
            }

            break;

        case ShellyEMMonophaseChannel::Second:
            switch (shelly_pro_em.monophase_mapping) {
            case ShellyEMMonophaseMapping::None:
                logger.printfln_meter("No Shelly Pro EM Monophase Mapping selected");
                break;

            case ShellyEMMonophaseMapping::L1:
                table = &shelly_em_monophase_channel_2_as_l1_table;
                break;

            case ShellyEMMonophaseMapping::L2:
                table = &shelly_em_monophase_channel_2_as_l2_table;
                break;

            case ShellyEMMonophaseMapping::L3:
                table = &shelly_em_monophase_channel_2_as_l3_table;
                break;

            default:
                logger.printfln_meter("Unknown Shelly Pro EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_em.monophase_mapping));
                break;
            }

            break;

        case ShellyEMMonophaseChannel::Third:
            logger.printfln_meter("Impossible Shelly Pro EM Monophase Channel selected: Third");
            break;

        default:
            logger.printfln_meter("Unknown Shelly Pro EM Monophase Channel: %u", static_cast<uint8_t>(shelly_pro_em.monophase_channel));
            break;
        }

        break;

    case MeterModbusTCPTableID::ShellyPro3EM:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        shelly_pro_3em.device_profile = ephemeral_table_config->get("device_profile")->asEnum<ShellyPro3EMDeviceProfile>();
        shelly_pro_3em.monophase_channel = ephemeral_table_config->get("monophase_channel")->asEnum<ShellyEMMonophaseChannel>();
        shelly_pro_3em.monophase_mapping = ephemeral_table_config->get("monophase_mapping")->asEnum<ShellyEMMonophaseMapping>();

        switch (shelly_pro_3em.device_profile) {
        case ShellyPro3EMDeviceProfile::Triphase:
            table = &shelly_em_triphase_table;
            break;

        case ShellyPro3EMDeviceProfile::Monophase:
            switch (shelly_pro_3em.monophase_channel) {
            case ShellyEMMonophaseChannel::None:
                logger.printfln_meter("No Shelly Pro 3EM Monophase Channel selected");
                break;

            case ShellyEMMonophaseChannel::First:
                switch (shelly_pro_3em.monophase_mapping) {
                case ShellyEMMonophaseMapping::None:
                    logger.printfln_meter("No Shelly Pro 3EM Monophase Mapping selected");
                    break;

                case ShellyEMMonophaseMapping::L1:
                    table = &shelly_em_monophase_channel_1_as_l1_table;
                    break;

                case ShellyEMMonophaseMapping::L2:
                    table = &shelly_em_monophase_channel_1_as_l2_table;
                    break;

                case ShellyEMMonophaseMapping::L3:
                    table = &shelly_em_monophase_channel_1_as_l3_table;
                    break;

                default:
                    logger.printfln_meter("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_mapping));
                    break;
                }

                break;

            case ShellyEMMonophaseChannel::Second:
                switch (shelly_pro_3em.monophase_mapping) {
                case ShellyEMMonophaseMapping::None:
                    logger.printfln_meter("No Shelly Pro 3EM Monophase Mapping selected");
                    break;

                case ShellyEMMonophaseMapping::L1:
                    table = &shelly_em_monophase_channel_2_as_l1_table;
                    break;

                case ShellyEMMonophaseMapping::L2:
                    table = &shelly_em_monophase_channel_2_as_l2_table;
                    break;

                case ShellyEMMonophaseMapping::L3:
                    table = &shelly_em_monophase_channel_2_as_l3_table;
                    break;

                default:
                    logger.printfln_meter("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_mapping));
                    break;
                }

                break;

            case ShellyEMMonophaseChannel::Third:
                switch (shelly_pro_3em.monophase_mapping) {
                case ShellyEMMonophaseMapping::None:
                    logger.printfln_meter("No Shelly Pro 3EM Monophase Mapping selected");
                    break;

                case ShellyEMMonophaseMapping::L1:
                    table = &shelly_em_monophase_channel_3_as_l1_table;
                    break;

                case ShellyEMMonophaseMapping::L2:
                    table = &shelly_em_monophase_channel_3_as_l2_table;
                    break;

                case ShellyEMMonophaseMapping::L3:
                    table = &shelly_em_monophase_channel_3_as_l3_table;
                    break;

                default:
                    logger.printfln_meter("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_mapping));
                    break;
                }

                break;

            default:
                logger.printfln_meter("Unknown Shelly Pro 3EM Monophase Channel: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_channel));
                break;
            }

            break;

        default:
            logger.printfln_meter("Unknown Shelly Pro 3EM Device Profile: %u", static_cast<uint8_t>(shelly_pro_3em.device_profile));
            break;
        }

        break;

    case MeterModbusTCPTableID::GoodweHybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        goodwe_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<GoodweHybridInverterVirtualMeter>();
        goodwe_hybrid_inverter.battery_1_mode = -1;
        goodwe_hybrid_inverter.battery_2_mode = -1;
        table = get_goodwe_hybrid_inverter_table(slot, goodwe_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::SolaxHybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        solax_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<SolaxHybridInverterVirtualMeter>();
        table = get_solax_hybrid_inverter_table(slot, solax_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::FroniusGEN24Plus:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        fronius_gen24_plus.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<FroniusGEN24PlusVirtualMeter>();
        fronius_gen24_plus.input_id_or_model_id = -1;
        table = get_fronius_gen24_plus_table(slot, fronius_gen24_plus.virtual_meter);
        break;

    case MeterModbusTCPTableID::HaileiHybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        hailei_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<HaileiHybridInverterVirtualMeter>();
        table = get_hailei_hybrid_inverter_table(slot, hailei_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::FoxESSH3AC3HybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        fox_ess_h3_ac3_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<FoxESSH3AC3HybridInverterVirtualMeter>();
        table = get_fox_ess_h3_ac3_hybrid_inverter_table(slot, fox_ess_h3_ac3_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::SiemensPAC2200:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &siemens_pac2200_table;
        break;

    case MeterModbusTCPTableID::SiemensPAC3120:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &siemens_pac3120_and_pac3220_table;
        break;

    case MeterModbusTCPTableID::SiemensPAC3200:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &siemens_pac3200_table;
        break;

    case MeterModbusTCPTableID::SiemensPAC3220:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &siemens_pac3120_and_pac3220_table;
        break;

    case MeterModbusTCPTableID::SiemensPAC4200:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &siemens_pac4200_table;
        break;

    case MeterModbusTCPTableID::SiemensPAC4220:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &siemens_pac4220_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM24DIN:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 11));
        table = &carlo_gavazzi_em24_din_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM24E1:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &carlo_gavazzi_em24_e1_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM100:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        carlo_gavazzi_em100.phase = ephemeral_table_config->get("phase")->asEnum<CarloGavazziPhase>();
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));

        switch (carlo_gavazzi_em100.phase) {
        case CarloGavazziPhase::None:
            logger.printfln_meter("No Carlo Gavazzi EM100 Phase selected");
            break;

        case CarloGavazziPhase::L1:
            table = &carlo_gavazzi_em100_and_et100_at_l1_table;
            break;

        case CarloGavazziPhase::L2:
            table = &carlo_gavazzi_em100_and_et100_at_l2_table;
            break;

        case CarloGavazziPhase::L3:
            table = &carlo_gavazzi_em100_and_et100_at_l3_table;
            break;

        default:
            logger.printfln_meter("Unknown Carlo Gavazzi EM100 Phase: %u", static_cast<uint8_t>(carlo_gavazzi_em100.phase));
            break;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziET100:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        carlo_gavazzi_et100.phase = ephemeral_table_config->get("phase")->asEnum<CarloGavazziPhase>();
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));

        switch (carlo_gavazzi_et100.phase) {
        case CarloGavazziPhase::None:
            logger.printfln_meter("No Carlo Gavazzi ET100 Phase selected");
            break;

        case CarloGavazziPhase::L1:
            table = &carlo_gavazzi_em100_and_et100_at_l1_table;
            break;

        case CarloGavazziPhase::L2:
            table = &carlo_gavazzi_em100_and_et100_at_l2_table;
            break;

        case CarloGavazziPhase::L3:
            table = &carlo_gavazzi_em100_and_et100_at_l3_table;
            break;

        default:
            logger.printfln_meter("Unknown Carlo Gavazzi ET100 Phase: %u", static_cast<uint8_t>(carlo_gavazzi_et100.phase));
            break;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziEM210:
        device_address = static_cast<uint8_t>(ephemeral_table_config->get("device_address")->asUint());
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 61));
        table = &carlo_gavazzi_em210_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM270:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        carlo_gavazzi_em270.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<CarloGavazziEM270VirtualMeter>();
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 18));

        switch (carlo_gavazzi_em270.virtual_meter) {
        case CarloGavazziEM270VirtualMeter::None:
            logger.printfln_meter("No Carlo Gavazzi EM270 Virtual Meter selected");
            break;

        case CarloGavazziEM270VirtualMeter::Meter:
            table = &carlo_gavazzi_em270_and_em280_meter_table;
            break;

        case CarloGavazziEM270VirtualMeter::CurrentTransformer1:
            table = &carlo_gavazzi_em270_and_em280_current_transformer_1_table;
            break;

        case CarloGavazziEM270VirtualMeter::CurrentTransformer2:
            table = &carlo_gavazzi_em270_and_em280_current_transformer_2_table;
            break;

        default:
            logger.printfln_meter("Unknown Carlo Gavazzi EM270 Virtual Meter: %u", static_cast<uint8_t>(carlo_gavazzi_em270.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziEM280:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        carlo_gavazzi_em280.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<CarloGavazziEM280VirtualMeter>();
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 18));

        switch (carlo_gavazzi_em280.virtual_meter) {
        case CarloGavazziEM280VirtualMeter::None:
            logger.printfln_meter("No Carlo Gavazzi EM280 Virtual Meter selected");
            break;

        case CarloGavazziEM280VirtualMeter::Meter:
            table = &carlo_gavazzi_em270_and_em280_meter_table;
            break;

        case CarloGavazziEM280VirtualMeter::CurrentTransformer1:
            table = &carlo_gavazzi_em270_and_em280_current_transformer_1_table;
            break;

        case CarloGavazziEM280VirtualMeter::CurrentTransformer2:
            table = &carlo_gavazzi_em270_and_em280_current_transformer_2_table;
            break;

        default:
            logger.printfln_meter("Unknown Carlo Gavazzi EM280 Virtual Meter: %u", static_cast<uint8_t>(carlo_gavazzi_em280.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziEM300:
        device_address = static_cast<uint8_t>(ephemeral_table_config->get("device_address")->asUint());
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));
        table = &carlo_gavazzi_em300_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziET300:
        device_address = static_cast<uint8_t>(ephemeral_table_config->get("device_address")->asUint());
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));
        table = &carlo_gavazzi_et300_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM510:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        carlo_gavazzi_em510.phase = ephemeral_table_config->get("phase")->asEnum<CarloGavazziPhase>();
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));

        switch (carlo_gavazzi_em510.phase) {
        case CarloGavazziPhase::None:
            logger.printfln_meter("No Carlo Gavazzi EM510 Phase selected");
            break;

        case CarloGavazziPhase::L1:
            table = &carlo_gavazzi_em510_at_l1_table;
            break;

        case CarloGavazziPhase::L2:
            table = &carlo_gavazzi_em510_at_l2_table;
            break;

        case CarloGavazziPhase::L3:
            table = &carlo_gavazzi_em510_at_l3_table;
            break;

        default:
            logger.printfln_meter("Unknown Carlo Gavazzi EM510 Phase: %u", static_cast<uint8_t>(carlo_gavazzi_em510.phase));
            break;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziEM530:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &carlo_gavazzi_em530_and_em540_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM540:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &carlo_gavazzi_em530_and_em540_table;
        break;

    case MeterModbusTCPTableID::SolaredgeInverter:
        solaredge_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<SolaredgeInverterVirtualMeter>();
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = get_solaredge_inverter_table(slot, solaredge_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::EastronSDM630TCP:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));
        table = &eastron_sdm630_tcp_table;
        break;

    case MeterModbusTCPTableID::TinkerforgeWARPCharger:
        device_address = 1;
        table = &tinkerforge_warp_charger_table;
        break;

    case MeterModbusTCPTableID::SAXPowerHomeBasicMode:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        sax_power_home_basic_mode.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<SAXPowerHomeBasicModeVirtualMeter>();
        table = get_sax_power_home_basic_mode_table(slot, sax_power_home_basic_mode.virtual_meter);
        break;

    case MeterModbusTCPTableID::SAXPowerHomeExtendedMode:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        sax_power_home_extended_mode.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<SAXPowerHomeExtendedModeVirtualMeter>();
        table = get_sax_power_home_extended_mode_table(slot, sax_power_home_extended_mode.virtual_meter);
        break;

    case MeterModbusTCPTableID::E3DCHauskraftwerk:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        e3dc_hauskraftwerk.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<E3DCHauskraftwerkVirtualMeter>();
        table = get_e3dc_hauskraftwerk_table(slot, e3dc_hauskraftwerk.virtual_meter);
        break;

    case MeterModbusTCPTableID::HuaweiSUN2000:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        huawei_sun2000.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<HuaweiSUN2000VirtualMeter>();
        huawei_sun2000.energy_storage_product_model = -1;
        huawei_sun2000.number_of_pv_strings = -1;
        table = get_huawei_sun2000_table(slot, huawei_sun2000.virtual_meter);
        break;

    case MeterModbusTCPTableID::HuaweiSUN2000SmartDongle:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        huawei_sun2000_smart_dongle.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<HuaweiSUN2000SmartDongleVirtualMeter>();
        huawei_sun2000_smart_dongle.energy_storage_product_model = -1;
        table = get_huawei_sun2000_smart_dongle_table(slot, huawei_sun2000_smart_dongle.virtual_meter);
        break;

    case MeterModbusTCPTableID::HuaweiEMMA:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        huawei_emma.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<HuaweiEMMAVirtualMeter>();
        table = get_huawei_emma_table(slot, huawei_emma.virtual_meter);
        break;

    case MeterModbusTCPTableID::CarloGavazziEM580:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &carlo_gavazzi_em580_table;
        break;

    case MeterModbusTCPTableID::SolaxStringInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        solax_string_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<SolaxStringInverterVirtualMeter>();
        table = get_solax_string_inverter_table(slot, solax_string_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::FoxESSH3SmartHybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        fox_ess_h3_smart_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<FoxESSH3SmartHybridInverterVirtualMeter>();
        table = get_fox_ess_h3_smart_hybrid_inverter_table(slot, fox_ess_h3_smart_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::FoxESSH3ProHybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        fox_ess_h3_pro_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<FoxESSH3ProHybridInverterVirtualMeter>();
        table = get_fox_ess_h3_pro_hybrid_inverter_table(slot, fox_ess_h3_pro_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::SMAHybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        sma_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<SMAHybridInverterVirtualMeter>();
        table = get_sma_hybrid_inverter_table(slot, sma_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::VARTAElement:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        varta_element.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<VARTAElementVirtualMeter>();
        table = get_varta_element_table(slot, varta_element.virtual_meter);
        break;

    case MeterModbusTCPTableID::VARTAFlex:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        varta_flex.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<VARTAFlexVirtualMeter>();
        table = get_varta_flex_table(slot, varta_flex.virtual_meter);
        break;

    case MeterModbusTCPTableID::ChisageESSHybridInverter:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        chisage_ess_hybrid_inverter.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<ChisageESSHybridInverterVirtualMeter>();
        table = get_chisage_ess_hybrid_inverter_table(slot, chisage_ess_hybrid_inverter.virtual_meter);
        break;

    case MeterModbusTCPTableID::Janitza:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        table = &janitza_table;
        break;

    case MeterModbusTCPTableID::HuaweiSmartLogger3000:
        device_address = ephemeral_table_config->get("device_address")->asUint8();
        huawei_smart_logger_3000.virtual_meter = ephemeral_table_config->get("virtual_meter")->asEnum<HuaweiSmartLogger3000VirtualMeter>();
        table = get_huawei_smart_logger_3000_table(slot, huawei_smart_logger_3000.virtual_meter);
        break;

    default:
        logger.printfln_meter("Unknown table: %u", static_cast<uint8_t>(table_id));
        break;
    }

    if (table == nullptr) {
        return;
    }

    if (ephemeral_config->get("location")->asEnum<MeterLocation>() == MeterLocation::Unknown && table->default_location != MeterLocation::Unknown) {
        ephemeral_config->get("location")->updateEnum(table->default_location);
    }

    if (table->ids_length > 0) {
        meters.declare_value_ids(slot, table->ids, table->ids_length);
    }

    if (table->specs_length > 0) {
        task_scheduler.scheduleUncancelable([this]() {
            if (read_allowed) {
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

void MeterModbusTCP::connect_callback()
{
    GenericModbusTCPClient::connect_callback();

    generic_read_request.data[0] = register_buffer;
    generic_read_request.data[1] = nullptr;
    generic_read_request.read_twice = false;
    generic_read_request.done_callback = [this]{ read_done_callback(); };

    read_index = 0;
    register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE; // this tells read_next() that generic_read_request is not fully initialized

    prepare_read();
    read_next();
}

void MeterModbusTCP::disconnect_callback()
{
    read_allowed = false;
}

bool MeterModbusTCP::prepare_read()
{
    bool overflow = false;

    while (
#ifndef DEBUG_VALUES_TO_TRACE_LOG
        table->index[read_index] == VALUE_INDEX_DEBUG ||
#endif
        table->specs[read_index].start_address == START_ADDRESS_VIRTUAL) {
        read_index = (read_index + 1) % table->specs_length;

        if (read_index == 0) {
            overflow = true;
        }
    }

    return overflow;
}

void MeterModbusTCP::read_next()
{
    read_allowed = false;

    if (register_buffer_index != METER_MODBUS_TCP_REGISTER_BUFFER_SIZE // only use fully initialized generic_read_request from last read
     && register_buffer_index < generic_read_request.register_count
     && generic_read_request.register_type == table->specs[read_index].register_type
     && generic_read_request.start_address + register_buffer_index == table->specs[read_index].start_address
     && register_buffer_index + MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[read_index].value_type) <= generic_read_request.register_count) {
        parse_next();
    }
    else {
        generic_read_request.register_type = table->specs[read_index].register_type;
        generic_read_request.start_address = table->specs[read_index].start_address;
        generic_read_request.register_count = MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[read_index].value_type);

        for (size_t i = read_index + 1; i < table->specs_length; ++i) {
            if (generic_read_request.register_type == table->specs[i].register_type
             && generic_read_request.start_address + generic_read_request.register_count == table->specs[i].start_address
             && generic_read_request.register_count + MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[i].value_type) <= max_register_count) {
                generic_read_request.register_count += MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[i].value_type);
                continue;
            }

            break;
        }

        register_buffer_index = 0;
        register_start_address = generic_read_request.start_address;

        start_generic_read();
    }
}

bool MeterModbusTCP::is_sungrow_hybrid_inverter_meter() const
{
    return table_id == MeterModbusTCPTableID::SungrowHybridInverter
        && sungrow_hybrid_inverter.virtual_meter == SungrowHybridInverterVirtualMeter::Inverter;
}

bool MeterModbusTCP::is_sungrow_hybrid_inverter_grid_meter() const
{
    return table_id == MeterModbusTCPTableID::SungrowHybridInverter
        && sungrow_hybrid_inverter.virtual_meter == SungrowHybridInverterVirtualMeter::Grid;
}

bool MeterModbusTCP::is_sungrow_hybrid_inverter_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::SungrowHybridInverter
        && sungrow_hybrid_inverter.virtual_meter == SungrowHybridInverterVirtualMeter::Battery;
}

bool MeterModbusTCP::is_sungrow_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::SungrowHybridInverter
        && sungrow_hybrid_inverter.virtual_meter == SungrowHybridInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_sungrow_string_inverter_meter() const
{
    return table_id == MeterModbusTCPTableID::SungrowStringInverter
        && sungrow_string_inverter.virtual_meter == SungrowStringInverterVirtualMeter::Inverter;
}

bool MeterModbusTCP::is_sungrow_string_inverter_grid_meter() const
{
    return table_id == MeterModbusTCPTableID::SungrowStringInverter
        && sungrow_string_inverter.virtual_meter == SungrowStringInverterVirtualMeter::Grid;
}

bool MeterModbusTCP::is_sungrow_string_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::SungrowStringInverter
        && sungrow_string_inverter.virtual_meter == SungrowStringInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_victron_energy_gx_load_meter() const
{
    return table_id == MeterModbusTCPTableID::VictronEnergyGX
        && victron_energy_gx.virtual_meter == VictronEnergyGXVirtualMeter::Load;
}

bool MeterModbusTCP::is_victron_energy_gx_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::VictronEnergyGX
        && victron_energy_gx.virtual_meter == VictronEnergyGXVirtualMeter::PV;
}

bool MeterModbusTCP::is_deye_hybrid_inverter_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::DeyeHybridInverter
        && deye_hybrid_inverter.virtual_meter == DeyeHybridInverterVirtualMeter::Battery;
}

bool MeterModbusTCP::is_deye_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::DeyeHybridInverter
        && deye_hybrid_inverter.virtual_meter == DeyeHybridInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_alpha_ess_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::AlphaESSHybridInverter
        && alpha_ess_hybrid_inverter.virtual_meter == AlphaESSHybridInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_shelly_pro_xem_monophase() const
{
    return table_id == MeterModbusTCPTableID::ShellyProEM
        || (table_id == MeterModbusTCPTableID::ShellyPro3EM
            && shelly_pro_3em.device_profile == ShellyPro3EMDeviceProfile::Monophase);
}

bool MeterModbusTCP::is_goodwe_hybrid_inverter_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::GoodweHybridInverter
        && goodwe_hybrid_inverter.virtual_meter == GoodweHybridInverterVirtualMeter::Battery;
}

bool MeterModbusTCP::is_goodwe_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::GoodweHybridInverter
        && goodwe_hybrid_inverter.virtual_meter == GoodweHybridInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_solax_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::SolaxHybridInverter
        && solax_hybrid_inverter.virtual_meter == SolaxHybridInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_fronius_gen24_plus_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::FroniusGEN24Plus
        && fronius_gen24_plus.virtual_meter == FroniusGEN24PlusVirtualMeter::Battery;
}

bool MeterModbusTCP::is_hailei_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::HaileiHybridInverter
        && hailei_hybrid_inverter.virtual_meter == HaileiHybridInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_fox_ess_h3_ac3_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3AC3HybridInverter
        && fox_ess_h3_ac3_hybrid_inverter.virtual_meter == FoxESSH3AC3HybridInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_carlo_gavazzi_em100_or_et100() const
{
    return table_id == MeterModbusTCPTableID::CarloGavazziEM100
        || table_id == MeterModbusTCPTableID::CarloGavazziET100;
}

bool MeterModbusTCP::is_carlo_gavazzi_em510() const
{
    return table_id == MeterModbusTCPTableID::CarloGavazziEM510;
}

bool MeterModbusTCP::is_solaredge_inverter_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::SolaredgeInverter
        && solaredge_inverter.virtual_meter == SolaredgeInverterVirtualMeter::Battery;
}

bool MeterModbusTCP::is_huawei_sun2000_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::HuaweiSUN2000
        && huawei_sun2000.virtual_meter == HuaweiSUN2000VirtualMeter::Battery;
}

bool MeterModbusTCP::is_huawei_sun2000_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::HuaweiSUN2000
        && huawei_sun2000.virtual_meter == HuaweiSUN2000VirtualMeter::PV;
}

bool MeterModbusTCP::is_huawei_sun2000_smart_dongle_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::HuaweiSUN2000SmartDongle
        && huawei_sun2000_smart_dongle.virtual_meter == HuaweiSUN2000SmartDongleVirtualMeter::Battery;
}

bool MeterModbusTCP::is_huawei_emma_load_meter() const
{
    return table_id == MeterModbusTCPTableID::HuaweiEMMA
        && huawei_emma.virtual_meter == HuaweiEMMAVirtualMeter::Load;
}

bool MeterModbusTCP::is_solax_string_inverter_meter() const
{
    return table_id == MeterModbusTCPTableID::SolaxStringInverter
        && solax_string_inverter.virtual_meter == SolaxStringInverterVirtualMeter::Inverter;
}

bool MeterModbusTCP::is_solax_string_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::SolaxStringInverter
        && solax_string_inverter.virtual_meter == SolaxStringInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_fox_ess_h3_smart_hybrid_inverter_battery_1_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3SmartHybridInverter
        && fox_ess_h3_smart_hybrid_inverter.virtual_meter == FoxESSH3SmartHybridInverterVirtualMeter::Battery1;
}

bool MeterModbusTCP::is_fox_ess_h3_smart_hybrid_inverter_battery_2_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3SmartHybridInverter
        && fox_ess_h3_smart_hybrid_inverter.virtual_meter == FoxESSH3SmartHybridInverterVirtualMeter::Battery2;
}

bool MeterModbusTCP::is_fox_ess_h3_smart_hybrid_inverter_battery_1_and_2_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3SmartHybridInverter
        && fox_ess_h3_smart_hybrid_inverter.virtual_meter == FoxESSH3SmartHybridInverterVirtualMeter::Battery1And2;
}

bool MeterModbusTCP::is_fox_ess_h3_smart_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3SmartHybridInverter
        && fox_ess_h3_smart_hybrid_inverter.virtual_meter == FoxESSH3SmartHybridInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_fox_ess_h3_pro_hybrid_inverter_battery_1_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3ProHybridInverter
        && fox_ess_h3_pro_hybrid_inverter.virtual_meter == FoxESSH3ProHybridInverterVirtualMeter::Battery1;
}

bool MeterModbusTCP::is_fox_ess_h3_pro_hybrid_inverter_battery_2_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3ProHybridInverter
        && fox_ess_h3_pro_hybrid_inverter.virtual_meter == FoxESSH3ProHybridInverterVirtualMeter::Battery2;
}

bool MeterModbusTCP::is_fox_ess_h3_pro_hybrid_inverter_battery_1_and_2_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3ProHybridInverter
        && fox_ess_h3_pro_hybrid_inverter.virtual_meter == FoxESSH3ProHybridInverterVirtualMeter::Battery1And2;
}

bool MeterModbusTCP::is_fox_ess_h3_pro_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3ProHybridInverter
        && fox_ess_h3_pro_hybrid_inverter.virtual_meter == FoxESSH3ProHybridInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_sma_hybrid_inverter_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::SMAHybridInverter
        && sma_hybrid_inverter.virtual_meter == SMAHybridInverterVirtualMeter::Battery;
}

bool MeterModbusTCP::is_varta_flex_grid_meter() const
{
    return table_id == MeterModbusTCPTableID::VARTAFlex
        && varta_flex.virtual_meter == VARTAFlexVirtualMeter::Grid;
}

bool MeterModbusTCP::is_varta_flex_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::VARTAFlex
        && varta_flex.virtual_meter == VARTAFlexVirtualMeter::Battery;
}

bool MeterModbusTCP::is_chisage_ess_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::ChisageESSHybridInverter
        && chisage_ess_hybrid_inverter.virtual_meter == ChisageESSHybridInverterVirtualMeter::PV;
}

void MeterModbusTCP::read_done_callback()
{
    if (generic_read_request.result != TFModbusTCPClientTransactionResult::Success) {
        trace("m%lu t%u i%zu a%zu:%x c%zu e%lu",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              generic_read_request.start_address,
              generic_read_request.start_address,
              generic_read_request.register_count,
              static_cast<uint32_t>(generic_read_request.result));

        if (generic_read_request.result == TFModbusTCPClientTransactionResult::Timeout) {
            auto timeout = errors->get("timeout");
            timeout->updateUint(timeout->asUint() + 1);
        }

        read_allowed = true;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;
        return;
    }

    parse_next();
}

void MeterModbusTCP::parse_next()
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
    ModbusValueType value_type = table->specs[read_index].value_type;
    size_t register_count = MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(value_type);
    const char *endian = "?";

    switch (register_count) {
    case 1:
        c16.r = register_buffer[register_buffer_index];
        break;

    case 2:
        if (MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(value_type)) {
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
        if (MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(value_type)) {
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
        logger.printfln_meter("%s / %s has unsupported register count: %zu", get_meter_modbus_tcp_table_id_name(table_id), table->specs[read_index].name, register_count);
        return;
    }

    switch (value_type) {
    case ModbusValueType::None:
        trace("m%lu t%u i%zu n a%zu:%x",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address);
        break;

    case ModbusValueType::U16:
        trace("m%lu t%u i%zu u16 a%zu:%x r%u v%u",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address,
              register_buffer[register_buffer_index],
              c16.u);

        value = static_cast<float>(c16.u);
        break;

    case ModbusValueType::S16:
        trace("m%lu t%u i%zu s16 a%zu:%x r%u v%d",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address,
              register_buffer[register_buffer_index],
              static_cast<int16_t>(c16.u));

        value = static_cast<float>(static_cast<int16_t>(c16.u));
        break;

    case ModbusValueType::U32BE:
    case ModbusValueType::U32LE:
        trace("m%lu t%u i%zu u32%s a%zu:%x r%u,%u v%lu",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              endian,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address,
              register_buffer[register_buffer_index + 0],
              register_buffer[register_buffer_index + 1],
              c32.u);

        value = static_cast<float>(c32.u);
        break;

    case ModbusValueType::S32BE:
    case ModbusValueType::S32LE:
        trace("m%lu t%u i%zu s32%s a%zu:%x r%u,%u v%ld",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              endian,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address,
              register_buffer[register_buffer_index + 0],
              register_buffer[register_buffer_index + 1],
              static_cast<int32_t>(c32.u));

        value = static_cast<float>(static_cast<int32_t>(c32.u));
        break;

    case ModbusValueType::F32BE:
    case ModbusValueType::F32LE:
        trace("m%lu t%u i%zu f32%s a%zu:%x r%u,%u v%f",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              endian,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address,
              register_buffer[register_buffer_index + 0],
              register_buffer[register_buffer_index + 1],
              static_cast<double>(c32.f));

        if (!table->f32_negative_max_as_nan || !is_exactly_equal(c32.f, -FLT_MAX)) {
            value = c32.f;
        }

        break;

    case ModbusValueType::U64BE:
    case ModbusValueType::U64LE:
        trace("m%lu t%u i%zu u64%s a%zu:%x r%u,%u,%u,%u v%llu",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              endian,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address,
              register_buffer[register_buffer_index + 0],
              register_buffer[register_buffer_index + 1],
              register_buffer[register_buffer_index + 2],
              register_buffer[register_buffer_index + 3],
              c64.u);

        value = static_cast<float>(c64.u);
        break;

    case ModbusValueType::S64BE:
    case ModbusValueType::S64LE:
        trace("m%lu t%u i%zu s64%s a%zu:%x r%u,%u,%u,%u v%lld",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              endian,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address,
              register_buffer[register_buffer_index + 0],
              register_buffer[register_buffer_index + 1],
              register_buffer[register_buffer_index + 2],
              register_buffer[register_buffer_index + 3],
              static_cast<int64_t>(c64.u));

        value = static_cast<float>(static_cast<int64_t>(c64.u));
        break;

    case ModbusValueType::F64BE:
    case ModbusValueType::F64LE:
        trace("m%lu t%u i%zu f64%s a%zu:%x r%u,%u,%u,%u v%f",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              endian,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address,
              register_buffer[register_buffer_index + 0],
              register_buffer[register_buffer_index + 1],
              register_buffer[register_buffer_index + 2],
              register_buffer[register_buffer_index + 3],
              c64.f);

        value = static_cast<float>(c64.f);
        break;

    default:
        break;
    }

    if (table->specs[read_index].drop_sign) {
        value = fabs(value);
    }

    value += table->specs[read_index].offset;

    if (!is_exactly_zero(value)) {
        // Don't convert 0.0f into -0.0f if the scale factor is negative
        value *= table->specs[read_index].scale_factor;
    }

    if (is_sungrow_hybrid_inverter_meter()
     && generic_read_request.start_address == SungrowHybridInverterOutputTypeAddress::OutputType) {
        if (sungrow_hybrid_inverter.output_type < 0) {
            bool success = true;

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
                success = false;
                logger.printfln_meter("Sungrow Hybrid Inverter has unknown output type: %u", c16.u);
                break;
            }

            if (success) {
               sungrow_hybrid_inverter.output_type = c16.u;

                meters.declare_value_ids(slot, table->ids, table->ids_length);
            }
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

    if (is_sungrow_string_inverter_meter()
     && generic_read_request.start_address == SungrowStringInverterOutputTypeAddress::OutputType) {
        if (sungrow_string_inverter.output_type < 0) {
            bool success = true;

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
                success = false;
                logger.printfln_meter("Sungrow String Inverter has unknown output type: %u", c16.u);
                break;
            }

            if (success) {
                sungrow_string_inverter.output_type = c16.u;

                meters.declare_value_ids(slot, table->ids, table->ids_length);
            }
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

    if (is_deye_hybrid_inverter_battery_meter()
     && generic_read_request.start_address == DeyeHybridInverterBatteryDeviceTypeAddress::DeviceType) {
        if (deye_hybrid_inverter.device_type < 0) {
            bool success = true;

            switch (c16.u) {
            case 0x0002:
            case 0x0003:
            case 0x0004:
                logger.printfln_meter("Deye hybrid inverter has unsupported device type: 0x%04x", c16.u);
                return;

            case 0x0005:
                table = &deye_hybrid_inverter_low_voltage_battery_table;
                logger.printfln_meter("Deye hybrid inverter with low-voltage battery detected");
                break;

            case 0x0006:
            case 0x0106:
                table = &deye_hybrid_inverter_high_voltage_battery_table;
                logger.printfln_meter("Deye hybrid inverter with high-voltage battery detected: 0x%04x", c16.u);
                break;

            default:
                success = false;
                logger.printfln_meter("Deye hybrid inverter has unknown device type: 0x%04x", c16.u);
                break;
            }

            if (success) {
                deye_hybrid_inverter.device_type = c16.u;
                meters.declare_value_ids(slot, table->ids, table->ids_length);
            }
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

    if (is_deye_hybrid_inverter_pv_meter()
     && generic_read_request.start_address == DeyeHybridInverterPVDeviceTypeAddress::DeviceType) {
        if (deye_hybrid_inverter.device_type < 0) {
            bool success = true;

            switch (c16.u) {
            case 0x0002:
            case 0x0003:
            case 0x0004:
                logger.printfln_meter("Deye hybrid inverter has unsupported device type: 0x%04x", c16.u);
                return;

            case 0x0005:
                table = &deye_hybrid_inverter_low_voltage_pv_table;
                logger.printfln_meter("Deye hybrid inverter with low-voltage battery detected");
                break;

            case 0x0006:
            case 0x0106:
                table = &deye_hybrid_inverter_high_voltage_pv_table;
                logger.printfln_meter("Deye hybrid inverter with high-voltage battery detected: 0x%04x", c16.u);
                break;

            default:
                success = false;
                logger.printfln_meter("Deye hybrid inverter has unknown device type: 0x%04x", c16.u);
                break;
            }

            if (success) {
                deye_hybrid_inverter.device_type = c16.u;
                meters.declare_value_ids(slot, table->ids, table->ids_length);
            }
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

    if (is_goodwe_hybrid_inverter_battery_meter()) {
        if (generic_read_request.start_address == GoodweHybridInverterBatteryModesAddress::Battery1Mode) {
            if (goodwe_hybrid_inverter.battery_1_mode < 0) {
                goodwe_hybrid_inverter.battery_1_mode = c16.u;
            }
        }
        else if (generic_read_request.start_address == GoodweHybridInverterBatteryModesAddress::Battery2Mode) {
            if (goodwe_hybrid_inverter.battery_2_mode < 0) {
                goodwe_hybrid_inverter.battery_2_mode = c16.u;

                bool success = true;

                if (goodwe_hybrid_inverter.battery_1_mode != 0 && goodwe_hybrid_inverter.battery_2_mode != 0) {
                    table = &goodwe_hybrid_inverter_battery_1_and_2_table;
                    logger.printfln_meter("Goodwe hybrid inverter with battery 1 and 2 detected");
                }
                else if (goodwe_hybrid_inverter.battery_1_mode != 0 && goodwe_hybrid_inverter.battery_2_mode == 0) {
                    table = &goodwe_hybrid_inverter_battery_1_table;
                    logger.printfln_meter("Goodwe hybrid inverter with battery 1 detected");
                }
                else if (goodwe_hybrid_inverter.battery_1_mode == 0 && goodwe_hybrid_inverter.battery_2_mode != 0) {
                    table = &goodwe_hybrid_inverter_battery_2_table;
                    logger.printfln_meter("Goodwe hybrid inverter with battery 2 detected");
                }
                else {
                    success = false;
                    logger.printfln_meter("Goodwe hybrid inverter without battery detected");
                }

                if (success) {
                    meters.declare_value_ids(slot, table->ids, table->ids_length);
                }
                else {
                    goodwe_hybrid_inverter.battery_1_mode = -1;
                    goodwe_hybrid_inverter.battery_2_mode = -1;
                }
            }

            read_allowed = true;
            read_index = 0;
            register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

            prepare_read();
            return;
        }
    }

    if (is_fronius_gen24_plus_battery_meter()
     && generic_read_request.start_address == FroniusGEN24PlusBatteryTypeAddress::InputIDOrModelID) {
        if (fronius_gen24_plus.input_id_or_model_id < 0) {
            bool success = true;

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
                success = false;
                logger.printfln_meter("Fronius GEN24 Plus inverter has malformed MPPT model: %u", c16.u);
                break;
            }

            if (success) {
                fronius_gen24_plus.input_id_or_model_id = c16.u;
                meters.declare_value_ids(slot, table->ids, table->ids_length);
            }
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

    if (is_huawei_sun2000_battery_meter()
     && generic_read_request.start_address == HuaweiSUN2000BatteryProductModelAddress::EnergyStorageProductModel) {
        if (huawei_sun2000.energy_storage_product_model < 0) {
            bool success = true;

            switch (c16.u) {
            case 0: // None
                success = false;
                logger.printfln_meter("Huawei SUN2000 inverter has no battery connected");
                return;

            case 1: // LG RESU
                table = &huawei_sun2000_battery_lg_resu_table;
                logger.printfln_meter("Huawei SUN2000 inverter with LG RESU battery detected");
                break;

            case 2: // Huawei LUNA2000
                table = &huawei_sun2000_battery_huawei_luna2000_table;
                logger.printfln_meter("Huawei SUN2000 inverter with Huawei LUNA2000 battery detected");
                break;

            default:
                success = false;
                logger.printfln_meter("Huawei SUN2000 inverter has unknown battery model: %u", c16.u);
                break;
            }

            if (success) {
                huawei_sun2000.energy_storage_product_model = c16.u;
                meters.declare_value_ids(slot, table->ids, table->ids_length);
            }
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

    if (is_huawei_sun2000_pv_meter()
     && generic_read_request.start_address == HuaweiSUN2000PVStringCountAddress::NumberOfPVStrings) {
        if (huawei_sun2000.number_of_pv_strings < 0) {
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

            huawei_sun2000.number_of_pv_strings = c16.u;
            logger.printfln_meter("Huawei SUN2000 inverter has %u PV string%s", c16.u, c16.u != 1 ? "s" : "");
            meters.declare_value_ids(slot, table->ids, table->ids_length);
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

    if (is_huawei_sun2000_smart_dongle_battery_meter()
     && generic_read_request.start_address == HuaweiSUN2000SmartDongleBatteryProductModelAddress::EnergyStorageProductModel) {
        if (huawei_sun2000_smart_dongle.energy_storage_product_model < 0) {
            bool success = true;

            switch (c16.u) {
            case 0: // None
                success = false;
                logger.printfln_meter("Huawei SUN2000 inverter has no battery connected");
                return;

            case 1: // LG RESU
                table = &huawei_sun2000_smart_dongle_battery_lg_resu_table;
                logger.printfln_meter("Huawei SUN2000 inverter with LG RESU battery detected");
                break;

            case 2: // Huawei LUNA2000
                table = &huawei_sun2000_smart_dongle_battery_huawei_luna2000_table;
                logger.printfln_meter("Huawei SUN2000 inverter with Huawei LUNA2000 battery detected");
                break;

            default:
                success = false;
                logger.printfln_meter("Huawei SUN2000 inverter has unknown battery model: %u", c16.u);
                return;
            }

            if (success) {
                huawei_sun2000_smart_dongle.energy_storage_product_model = c16.u;
                meters.declare_value_ids(slot, table->ids, table->ids_length);
            }
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

    if (is_sungrow_hybrid_inverter_meter() || is_sungrow_string_inverter_meter()) {
        if (register_start_address == SungrowHybridInverterAddress::TotalActivePower
         || register_start_address == SungrowStringInverterAddress::TotalActivePower) {
            meters.update_value(slot, table->index[read_index + 1], zero_safe_negation(value));
        }
    }
    else if (is_sungrow_hybrid_inverter_grid_meter() || is_sungrow_string_inverter_grid_meter()) {
        if (register_start_address == SungrowHybridInverterGridAddress::GridFrequency
         || register_start_address == SungrowStringInverterGridAddress::GridFrequency) {
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
        if (register_start_address == SungrowHybridInverterBatteryAddress::RunningState) {
            sungrow_hybrid_inverter.running_state = c16.u;
        }
        else if (register_start_address == SungrowHybridInverterBatteryAddress::BatteryCurrent) {
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
        else if (register_start_address == SungrowHybridInverterBatteryAddress::BatteryPower) {
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
    else if (is_sungrow_hybrid_inverter_pv_meter()) {
        if (register_start_address == SungrowHybridInverterPVAddress::MPPT1Voltage) {
            sungrow_hybrid_inverter.mppt1_voltage = value;
        }
        else if (register_start_address == SungrowHybridInverterPVAddress::MPPT1Current) {
            sungrow_hybrid_inverter.mppt1_current = value;
        }
        else if (register_start_address == SungrowHybridInverterPVAddress::MPPT2Voltage) {
            sungrow_hybrid_inverter.mppt2_voltage = value;
        }
        else if (register_start_address == SungrowHybridInverterPVAddress::MPPT2Current) {
            sungrow_hybrid_inverter.mppt2_current = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(sungrow_hybrid_inverter.mppt1_voltage)) {
                voltage_sum += sungrow_hybrid_inverter.mppt1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(sungrow_hybrid_inverter.mppt2_voltage)) {
                voltage_sum += sungrow_hybrid_inverter.mppt2_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = sungrow_hybrid_inverter.mppt1_current
                              + sungrow_hybrid_inverter.mppt2_current;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
        }
    }
    else if (is_sungrow_string_inverter_pv_meter()) {
        if (register_start_address == SungrowStringInverterPVAddress::MPPT1Voltage) {
            sungrow_string_inverter.mppt1_voltage = value;
        }
        else if (register_start_address == SungrowStringInverterPVAddress::MPPT1Current) {
            sungrow_string_inverter.mppt1_current = value;
        }
        else if (register_start_address == SungrowStringInverterPVAddress::MPPT2Voltage) {
            sungrow_string_inverter.mppt2_voltage = value;
        }
        else if (register_start_address == SungrowStringInverterPVAddress::MPPT2Current) {
            sungrow_string_inverter.mppt2_current = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(sungrow_string_inverter.mppt1_voltage)) {
                voltage_sum += sungrow_string_inverter.mppt1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(sungrow_string_inverter.mppt2_voltage)) {
                voltage_sum += sungrow_string_inverter.mppt2_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = sungrow_string_inverter.mppt1_current
                              + sungrow_string_inverter.mppt2_current;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
        }
    }
    else if (is_victron_energy_gx_load_meter()) {
        if (register_start_address == VictronEnergyGXLoadAddress::ACConsumptionL1) {
            victron_energy_gx.ac_consumption_l1 = value;
        }
        else if (register_start_address == VictronEnergyGXLoadAddress::ACConsumptionL2) {
            victron_energy_gx.ac_consumption_l2 = value;
        }
        else if (register_start_address == VictronEnergyGXLoadAddress::ACConsumptionL3) {
            victron_energy_gx.ac_consumption_l3 = value;

            float power_sum = victron_energy_gx.ac_consumption_l1
                            + victron_energy_gx.ac_consumption_l2
                            + victron_energy_gx.ac_consumption_l3;

            meters.update_value(slot, table->index[read_index + 1], power_sum);
            meters.update_value(slot, table->index[read_index + 2], power_sum);
        }
    }
    else if (is_victron_energy_gx_pv_meter()) {
        if (register_start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnOutputL1) {
            victron_energy_gx.ac_coupled_pv_power_on_output_l1 = value;
        }
        else if (register_start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnOutputL2) {
            victron_energy_gx.ac_coupled_pv_power_on_output_l2 = value;
        }
        else if (register_start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnOutputL3) {
            victron_energy_gx.ac_coupled_pv_power_on_output_l3 = value;
        }
        else if (register_start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnInputL1) {
            victron_energy_gx.ac_coupled_pv_power_on_input_l1 = value;
        }
        else if (register_start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnInputL2) {
            victron_energy_gx.ac_coupled_pv_power_on_input_l2 = value;
        }
        else if (register_start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnInputL3) {
            victron_energy_gx.ac_coupled_pv_power_on_input_l3 = value;
        }
        else if (register_start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnGeneratorL1) {
            victron_energy_gx.ac_coupled_pv_power_on_generator_l1 = value;
        }
        else if (register_start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnGeneratorL2) {
            victron_energy_gx.ac_coupled_pv_power_on_generator_l2 = value;
        }
        else if (register_start_address == VictronEnergyGXPVAddress::ACCoupledPVPowerOnGeneratorL3) {
            victron_energy_gx.ac_coupled_pv_power_on_generator_l3 = value;
        }
        else if (register_start_address == VictronEnergyGXPVAddress::DCCoupledPVPower) {
            victron_energy_gx.dc_coupled_pv_power = value;

            float power_sum = victron_energy_gx.ac_coupled_pv_power_on_output_l1
                            + victron_energy_gx.ac_coupled_pv_power_on_output_l2
                            + victron_energy_gx.ac_coupled_pv_power_on_output_l3
                            + victron_energy_gx.ac_coupled_pv_power_on_input_l1
                            + victron_energy_gx.ac_coupled_pv_power_on_input_l2
                            + victron_energy_gx.ac_coupled_pv_power_on_input_l3
                            + victron_energy_gx.ac_coupled_pv_power_on_generator_l1
                            + victron_energy_gx.ac_coupled_pv_power_on_generator_l2
                            + victron_energy_gx.ac_coupled_pv_power_on_generator_l3
                            + victron_energy_gx.dc_coupled_pv_power;

            meters.update_value(slot, table->index[read_index + 1], power_sum);
        }
    }
    else if (is_deye_hybrid_inverter_pv_meter()) {
        if (register_start_address == DeyeHybridInverterPVAddress::PV1Power) {
            deye_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV2Power) {
            deye_hybrid_inverter.pv2_power = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV3Power) {
            deye_hybrid_inverter.pv3_power = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV4Power) {
            deye_hybrid_inverter.pv4_power = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV1Voltage) {
            deye_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV1Current) {
            deye_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV2Voltage) {
            deye_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV2Current) {
            deye_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV3Voltage) {
            deye_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV3Current) {
            deye_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV4Voltage) {
            deye_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == DeyeHybridInverterPVAddress::PV4Current) {
            deye_hybrid_inverter.pv4_current = value;

            float power_sum = deye_hybrid_inverter.pv1_power
                            + deye_hybrid_inverter.pv2_power
                            + deye_hybrid_inverter.pv3_power
                            + deye_hybrid_inverter.pv4_power;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(deye_hybrid_inverter.pv1_voltage)) {
                voltage_sum += deye_hybrid_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(deye_hybrid_inverter.pv2_voltage)) {
                voltage_sum += deye_hybrid_inverter.pv2_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(deye_hybrid_inverter.pv3_voltage)) {
                voltage_sum += deye_hybrid_inverter.pv3_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(deye_hybrid_inverter.pv4_voltage)) {
                voltage_sum += deye_hybrid_inverter.pv4_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = deye_hybrid_inverter.pv1_current
                              + deye_hybrid_inverter.pv2_current
                              + deye_hybrid_inverter.pv3_current
                              + deye_hybrid_inverter.pv4_current;

            meters.update_value(slot, table->index[read_index + 1], power_sum);
            meters.update_value(slot, table->index[read_index + 2], voltage_avg);
            meters.update_value(slot, table->index[read_index + 3], current_sum);
        }
    }
    else if (is_alpha_ess_hybrid_inverter_pv_meter()) {
        if (register_start_address == AlphaESSHybridInverterPVAddress::PV1Voltage) {
            alpha_ess_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV1Current) {
            alpha_ess_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV1Power) {
            alpha_ess_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV2Voltage) {
            alpha_ess_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV2Current) {
            alpha_ess_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV2Power) {
            alpha_ess_hybrid_inverter.pv2_power = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV3Voltage) {
            alpha_ess_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV3Current) {
            alpha_ess_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV3Power) {
            alpha_ess_hybrid_inverter.pv3_power = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV4Voltage) {
            alpha_ess_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV4Current) {
            alpha_ess_hybrid_inverter.pv4_current = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV4Power) {
            alpha_ess_hybrid_inverter.pv4_power = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV5Voltage) {
            alpha_ess_hybrid_inverter.pv5_voltage = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV5Current) {
            alpha_ess_hybrid_inverter.pv5_current = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV5Power) {
            alpha_ess_hybrid_inverter.pv5_power = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV6Voltage) {
            alpha_ess_hybrid_inverter.pv6_voltage = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV6Current) {
            alpha_ess_hybrid_inverter.pv6_current = value;
        }
        else if (register_start_address == AlphaESSHybridInverterPVAddress::PV6Power) {
            alpha_ess_hybrid_inverter.pv6_power = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(alpha_ess_hybrid_inverter.pv1_voltage)) {
                voltage_sum += alpha_ess_hybrid_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(alpha_ess_hybrid_inverter.pv2_voltage)) {
                voltage_sum += alpha_ess_hybrid_inverter.pv2_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(alpha_ess_hybrid_inverter.pv3_voltage)) {
                voltage_sum += alpha_ess_hybrid_inverter.pv3_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(alpha_ess_hybrid_inverter.pv4_voltage)) {
                voltage_sum += alpha_ess_hybrid_inverter.pv4_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(alpha_ess_hybrid_inverter.pv5_voltage)) {
                voltage_sum += alpha_ess_hybrid_inverter.pv5_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(alpha_ess_hybrid_inverter.pv6_voltage)) {
                voltage_sum += alpha_ess_hybrid_inverter.pv6_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = alpha_ess_hybrid_inverter.pv1_current
                              + alpha_ess_hybrid_inverter.pv2_current
                              + alpha_ess_hybrid_inverter.pv3_current
                              + alpha_ess_hybrid_inverter.pv4_current
                              + alpha_ess_hybrid_inverter.pv5_current
                              + alpha_ess_hybrid_inverter.pv6_current;

            float power_sum = alpha_ess_hybrid_inverter.pv1_power
                            + alpha_ess_hybrid_inverter.pv2_power
                            + alpha_ess_hybrid_inverter.pv3_power
                            + alpha_ess_hybrid_inverter.pv4_power
                            + alpha_ess_hybrid_inverter.pv5_power
                            + alpha_ess_hybrid_inverter.pv6_power;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
        }
    }
    else if (is_shelly_pro_xem_monophase()) {
        if (register_start_address == ShellyEMMonophaseChannel1Address::Channel1ActivePower
         || register_start_address == ShellyEMMonophaseChannel1Address::Channel1TotalActiveEnergyPerpetualCount
         || register_start_address == ShellyEMMonophaseChannel1Address::Channel1TotalActiveReturnedEnergyPerpetualCount
         || register_start_address == ShellyEMMonophaseChannel2Address::Channel2ActivePower
         || register_start_address == ShellyEMMonophaseChannel2Address::Channel2TotalActiveEnergyPerpetualCount
         || register_start_address == ShellyEMMonophaseChannel2Address::Channel2TotalActiveReturnedEnergyPerpetualCount
         || register_start_address == ShellyEMMonophaseChannel3Address::Channel3ActivePower
         || register_start_address == ShellyEMMonophaseChannel3Address::Channel3TotalActiveEnergyPerpetualCount
         || register_start_address == ShellyEMMonophaseChannel3Address::Channel3TotalActiveReturnedEnergyPerpetualCount) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_goodwe_hybrid_inverter_battery_meter() && goodwe_hybrid_inverter.battery_1_mode != 0 && goodwe_hybrid_inverter.battery_2_mode != 0) {
        if (register_start_address == GoodweHybridInverterBattery1And2Address::Battery1Voltage) {
            goodwe_hybrid_inverter.battery_1_voltage = value;
        }
        else if (register_start_address == GoodweHybridInverterBattery1And2Address::Battery1Current) {
            goodwe_hybrid_inverter.battery_1_current = value;
        }
        else if (register_start_address == GoodweHybridInverterBattery1And2Address::Battery1Power) {
            goodwe_hybrid_inverter.battery_1_power = value;
        }
        else if (register_start_address == GoodweHybridInverterBattery1And2Address::BMS1PackTemperature) {
            goodwe_hybrid_inverter.bms_1_pack_temperature = value;
        }
        else if (register_start_address == GoodweHybridInverterBattery1And2Address::Battery1Capacity) {
            goodwe_hybrid_inverter.battery_1_capacity = value;
        }
        else if (register_start_address == GoodweHybridInverterBattery1And2Address::Battery2Voltage) {
            goodwe_hybrid_inverter.battery_2_voltage = value;
        }
        else if (register_start_address == GoodweHybridInverterBattery1And2Address::Battery2Current) {
            goodwe_hybrid_inverter.battery_2_current = value;
        }
        else if (register_start_address == GoodweHybridInverterBattery1And2Address::Battery2Power) {
            goodwe_hybrid_inverter.battery_2_power = value;
        }
        else if (register_start_address == GoodweHybridInverterBattery1And2Address::BMS2PackTemperature) {
            goodwe_hybrid_inverter.bms_2_pack_temperature = value;
        }
        else if (register_start_address == GoodweHybridInverterBattery1And2Address::Battery2Capacity) {
            goodwe_hybrid_inverter.battery_2_capacity = value;

            float voltage_avg = (goodwe_hybrid_inverter.battery_1_voltage
                               + goodwe_hybrid_inverter.battery_2_voltage) / 2.0f;

            float current_sum = goodwe_hybrid_inverter.battery_1_current
                              + goodwe_hybrid_inverter.battery_2_current;

            float power_sum = goodwe_hybrid_inverter.battery_1_power
                            + goodwe_hybrid_inverter.battery_2_power;

            float temperature_avg = (goodwe_hybrid_inverter.bms_1_pack_temperature
                                   + goodwe_hybrid_inverter.bms_2_pack_temperature) / 2.0f;

            float state_of_charge_avg = (goodwe_hybrid_inverter.battery_1_capacity
                                       + goodwe_hybrid_inverter.battery_2_capacity) / 2.0f;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
            meters.update_value(slot, table->index[read_index + 4], temperature_avg);
            meters.update_value(slot, table->index[read_index + 5], state_of_charge_avg);
        }
    }
    else if (is_solax_hybrid_inverter_pv_meter()) {
        if (register_start_address == SolaxHybridInverterPVAddress::PV1Voltage) {
            solax_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == SolaxHybridInverterPVAddress::PV2Voltage) {
            solax_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == SolaxHybridInverterPVAddress::PV1Current) {
            solax_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == SolaxHybridInverterPVAddress::PV2Current) {
            solax_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == SolaxHybridInverterPVAddress::PV1Power) {
            solax_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == SolaxHybridInverterPVAddress::PV2Power) {
            solax_hybrid_inverter.pv2_power = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(solax_hybrid_inverter.pv1_voltage)) {
                voltage_sum += solax_hybrid_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(solax_hybrid_inverter.pv2_voltage)) {
                voltage_sum += solax_hybrid_inverter.pv2_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = solax_hybrid_inverter.pv1_current
                              + solax_hybrid_inverter.pv2_current;

            float power_sum = solax_hybrid_inverter.pv1_power
                            + solax_hybrid_inverter.pv2_power;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
        }
    }
    else if (is_goodwe_hybrid_inverter_pv_meter()) {
        if (register_start_address == GoodweHybridInverterPVAddress::PVMode) {
            goodwe_hybrid_inverter.pv_mode = c32.u;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV1Voltage) {
            if (((goodwe_hybrid_inverter.pv_mode >> 0) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV1Current) {
            if (((goodwe_hybrid_inverter.pv_mode >> 0) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV1Power) {
            if (((goodwe_hybrid_inverter.pv_mode >> 0) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV2Voltage) {
            if (((goodwe_hybrid_inverter.pv_mode >> 8) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV2Current) {
            if (((goodwe_hybrid_inverter.pv_mode >> 8) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV2Power) {
            if (((goodwe_hybrid_inverter.pv_mode >> 8) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv2_power = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV3Voltage) {
            if (((goodwe_hybrid_inverter.pv_mode >> 16) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV3Current) {
            if (((goodwe_hybrid_inverter.pv_mode >> 16) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV3Power) {
            if (((goodwe_hybrid_inverter.pv_mode >> 16) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv3_power = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV4Voltage) {
            if (((goodwe_hybrid_inverter.pv_mode >> 24) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV4Current) {
            if (((goodwe_hybrid_inverter.pv_mode >> 24) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv4_current = value;
        }
        else if (register_start_address == GoodweHybridInverterPVAddress::PV4Power) {
            if (((goodwe_hybrid_inverter.pv_mode >> 24) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv4_power = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(goodwe_hybrid_inverter.pv1_voltage)) {
                voltage_sum += goodwe_hybrid_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(goodwe_hybrid_inverter.pv2_voltage)) {
                voltage_sum += goodwe_hybrid_inverter.pv2_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(goodwe_hybrid_inverter.pv3_voltage)) {
                voltage_sum += goodwe_hybrid_inverter.pv3_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(goodwe_hybrid_inverter.pv4_voltage)) {
                voltage_sum += goodwe_hybrid_inverter.pv4_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = goodwe_hybrid_inverter.pv1_current
                              + goodwe_hybrid_inverter.pv2_current
                              + goodwe_hybrid_inverter.pv3_current
                              + goodwe_hybrid_inverter.pv4_current;

            float power_sum = goodwe_hybrid_inverter.pv1_power
                            + goodwe_hybrid_inverter.pv2_power
                            + goodwe_hybrid_inverter.pv3_power
                            + goodwe_hybrid_inverter.pv4_power;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
        }
    }
    else if (is_fronius_gen24_plus_battery_meter()) {
        size_t start_address = register_start_address - fronius_gen24_plus.start_address_shift;

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

            float dca_scale_factor = get_sun_spec_scale_factor(fronius_gen24_plus.dca_sf);
            float dcv_scale_factor = get_sun_spec_scale_factor(fronius_gen24_plus.dcv_sf);
            float dcw_scale_factor = get_sun_spec_scale_factor(fronius_gen24_plus.dcw_sf);
            float dcwh_scale_factor = get_sun_spec_scale_factor(fronius_gen24_plus.dcwh_sf);
            float chastate_scale_factor = get_sun_spec_scale_factor(fronius_gen24_plus.chastate_sf);

            float charge_dca = fronius_gen24_plus.charge_dca * dca_scale_factor;
            float charge_dcv = fronius_gen24_plus.charge_dcv * dcv_scale_factor;
            float charge_dcw = fronius_gen24_plus.charge_dcw * dcw_scale_factor;

            float discharge_dca = fronius_gen24_plus.discharge_dca * dca_scale_factor;
            float discharge_dcv = fronius_gen24_plus.discharge_dcv * dcv_scale_factor;
            float discharge_dcw = fronius_gen24_plus.discharge_dcw * dcw_scale_factor;

            float current_charge_discharge_diff = charge_dca - discharge_dca; // One of the two value is always 0A
            float voltage = std::max(charge_dcv, discharge_dcv); // In firmware versions <= 1.30 one of the two values is always 0V. In firmware versions >= 1.31 they are the same
            float power_charge_discharge_diff = charge_dcw - discharge_dcw; // One of the two value is always 0W
            float state_of_charge = fronius_gen24_plus.chastate * chastate_scale_factor;
            float energy_charge = fronius_gen24_plus.charge_dcwh * dcwh_scale_factor * 0.001f;
            float energy_discharge = fronius_gen24_plus.discharge_dcwh * dcwh_scale_factor * 0.001f;

            meters.update_value(slot, table->index[read_index + 1], current_charge_discharge_diff);
            meters.update_value(slot, table->index[read_index + 2], voltage);
            meters.update_value(slot, table->index[read_index + 3], power_charge_discharge_diff);
            meters.update_value(slot, table->index[read_index + 4], state_of_charge);
            meters.update_value(slot, table->index[read_index + 5], energy_charge);
            meters.update_value(slot, table->index[read_index + 6], energy_discharge);
        }
    }
    else if (is_hailei_hybrid_inverter_pv_meter()) {
        if (register_start_address == HaileiHybridInverterPVAddress::PV1Voltage) {
            hailei_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV1Current) {
            hailei_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV1Power) {
            hailei_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV2Voltage) {
            hailei_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV2Current) {
            hailei_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV2Power) {
            hailei_hybrid_inverter.pv2_power = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV3Voltage) {
            hailei_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV3Current) {
            hailei_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV3Power) {
            hailei_hybrid_inverter.pv3_power = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV4Voltage) {
            hailei_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV4Current) {
            hailei_hybrid_inverter.pv4_current = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV4Power) {
            hailei_hybrid_inverter.pv4_power = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV5Voltage) {
            hailei_hybrid_inverter.pv5_voltage = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV5Current) {
            hailei_hybrid_inverter.pv5_current = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV5Power) {
            hailei_hybrid_inverter.pv5_power = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV6Voltage) {
            hailei_hybrid_inverter.pv6_voltage = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV6Current) {
            hailei_hybrid_inverter.pv6_current = value;
        }
        else if (register_start_address == HaileiHybridInverterPVAddress::PV6Power) {
            hailei_hybrid_inverter.pv6_power = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(hailei_hybrid_inverter.pv1_voltage)) {
                voltage_sum += hailei_hybrid_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(hailei_hybrid_inverter.pv2_voltage)) {
                voltage_sum += hailei_hybrid_inverter.pv2_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(hailei_hybrid_inverter.pv3_voltage)) {
                voltage_sum += hailei_hybrid_inverter.pv3_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(hailei_hybrid_inverter.pv4_voltage)) {
                voltage_sum += hailei_hybrid_inverter.pv4_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(hailei_hybrid_inverter.pv5_voltage)) {
                voltage_sum += hailei_hybrid_inverter.pv5_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(hailei_hybrid_inverter.pv6_voltage)) {
                voltage_sum += hailei_hybrid_inverter.pv6_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = hailei_hybrid_inverter.pv1_current
                              + hailei_hybrid_inverter.pv2_current
                              + hailei_hybrid_inverter.pv3_current
                              + hailei_hybrid_inverter.pv4_current
                              + hailei_hybrid_inverter.pv5_current
                              + hailei_hybrid_inverter.pv6_current;

            float power_sum = hailei_hybrid_inverter.pv1_power
                            + hailei_hybrid_inverter.pv2_power
                            + hailei_hybrid_inverter.pv3_power
                            + hailei_hybrid_inverter.pv4_power
                            + hailei_hybrid_inverter.pv5_power
                            + hailei_hybrid_inverter.pv6_power;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
        }
    }
    else if (is_fox_ess_h3_ac3_hybrid_inverter_pv_meter()) {
        if (register_start_address == FoxESSH3AC3HybridInverterPVAddress::PV1Voltage) {
            fox_ess_h3_ac3_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == FoxESSH3AC3HybridInverterPVAddress::PV1Current) {
            fox_ess_h3_ac3_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == FoxESSH3AC3HybridInverterPVAddress::PV1Power) {
            fox_ess_h3_ac3_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == FoxESSH3AC3HybridInverterPVAddress::PV2Voltage) {
            fox_ess_h3_ac3_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == FoxESSH3AC3HybridInverterPVAddress::PV2Current) {
            fox_ess_h3_ac3_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == FoxESSH3AC3HybridInverterPVAddress::PV2Power) {
            fox_ess_h3_ac3_hybrid_inverter.pv2_power = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(fox_ess_h3_ac3_hybrid_inverter.pv1_voltage)) {
                voltage_sum += fox_ess_h3_ac3_hybrid_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(fox_ess_h3_ac3_hybrid_inverter.pv2_voltage)) {
                voltage_sum += fox_ess_h3_ac3_hybrid_inverter.pv2_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = fox_ess_h3_ac3_hybrid_inverter.pv1_current
                              + fox_ess_h3_ac3_hybrid_inverter.pv2_current;

            float power_sum = fox_ess_h3_ac3_hybrid_inverter.pv1_power
                            + fox_ess_h3_ac3_hybrid_inverter.pv2_power;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
        }
    }
    else if (is_carlo_gavazzi_em100_or_et100()) {
        if (register_start_address == CarloGavazziEM100AndET100Address::W
         || register_start_address == CarloGavazziEM100AndET100Address::KWhPositiveTotal
         || register_start_address == CarloGavazziEM100AndET100Address::KWhNegativeTotal) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_carlo_gavazzi_em510()) {
        if (register_start_address == CarloGavazziEM510Address::W
         || register_start_address == CarloGavazziEM510Address::KWhPositiveTotal
         || register_start_address == CarloGavazziEM510Address::KWhNegativeTotal) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_solaredge_inverter_battery_meter()) {
        if (register_start_address == SolaredgeInverterBatteryAddress::Battery1AverageTemperature) {
            solaredge_inverter.battery_1_temperature = value;
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery1InstantaneousVoltage) {
            solaredge_inverter.battery_1_voltage = value;
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery1InstantaneousCurrent) {
            solaredge_inverter.battery_1_current = value;
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery1InstantaneousPower) {
            solaredge_inverter.battery_1_power = value;
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery1LifetimeExportEnergyCounter) {
            solaredge_inverter.battery_1_export_energy = value;
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery1LifetimeImportEnergyCounter) {
            solaredge_inverter.battery_1_import_energy = value;
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery1StateOfEnergy) {
            solaredge_inverter.battery_1_state_of_charge = value;
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery2AverageTemperature) {
            value = nan_safe_sum(solaredge_inverter.battery_1_temperature, value);
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery2InstantaneousVoltage) {
            value = nan_safe_avg(solaredge_inverter.battery_1_voltage, value);
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery2InstantaneousCurrent) {
            value = nan_safe_sum(solaredge_inverter.battery_1_current, value);
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery2InstantaneousPower) {
            value = nan_safe_sum(solaredge_inverter.battery_1_power, value);
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery2LifetimeExportEnergyCounter) {
            value = nan_safe_sum(solaredge_inverter.battery_1_export_energy, value);
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery2LifetimeImportEnergyCounter) {
            value = nan_safe_sum(solaredge_inverter.battery_1_import_energy, value);
        }
        else if (register_start_address == SolaredgeInverterBatteryAddress::Battery2StateOfEnergy) {
            value = nan_safe_avg(solaredge_inverter.battery_1_state_of_charge, value);
        }
    }
    else if (is_huawei_emma_load_meter()) {
        if (register_start_address == HuaweiEMMALoadAddress::LoadPower) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_solax_string_inverter_meter()) {
        if (register_start_address == SolaxStringInverterAddress::OutputPower) {
            meters.update_value(slot, table->index[read_index + 1], zero_safe_negation(value));
        }
    }
    else if (is_solax_string_inverter_pv_meter()) {
        if (register_start_address == SolaxStringInverterPVAddress::PV1Voltage) {
            solax_string_inverter.pv1_voltage = value;
        }
        else if (register_start_address == SolaxStringInverterPVAddress::PV2Voltage) {
            solax_string_inverter.pv2_voltage = value;
        }
        else if (register_start_address == SolaxStringInverterPVAddress::PV1Current) {
            solax_string_inverter.pv1_current = value;
        }
        else if (register_start_address == SolaxStringInverterPVAddress::PV2Current) {
            solax_string_inverter.pv2_current = value;
        }
        else if (register_start_address == SolaxStringInverterPVAddress::PV1Power) {
            solax_string_inverter.pv1_power = value;
        }
        else if (register_start_address == SolaxStringInverterPVAddress::PV2Power) {
            solax_string_inverter.pv2_power = value;
        }
        else if (register_start_address == SolaxStringInverterPVAddress::PV3Voltage) {
            solax_string_inverter.pv3_voltage = value;
        }
        else if (register_start_address == SolaxStringInverterPVAddress::PV3Current) {
            solax_string_inverter.pv3_current = value;
        }
        else if (register_start_address == SolaxStringInverterPVAddress::PV3Power) {
            solax_string_inverter.pv3_power = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(solax_string_inverter.pv1_voltage)) {
                voltage_sum += solax_string_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(solax_string_inverter.pv2_voltage)) {
                voltage_sum += solax_string_inverter.pv2_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(solax_string_inverter.pv3_voltage)) {
                voltage_sum += solax_string_inverter.pv3_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = solax_string_inverter.pv1_current
                              + solax_string_inverter.pv2_current
                              + solax_string_inverter.pv3_current;

            float power_sum = solax_string_inverter.pv1_power
                            + solax_string_inverter.pv2_power
                            + solax_string_inverter.pv3_power;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
        }
    }
    else if (is_fox_ess_h3_smart_hybrid_inverter_battery_1_meter()) {
        if (register_start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1Voltage) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1Current) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1Temperature) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1SoC) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS1DesignEnergy) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1Address::BMS2ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1Address::TotalDischargeEnergy) {
            // this is the combined discharge energy of both batteries, only show if battery 2 is offline
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status != 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1Address::TotalChargeEnergy) {
            // this is the combined charge energy of both batteries, only show if battery 2 is offline
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status != 0) {
                value = NAN;
            }
        }
    }
    else if (is_fox_ess_h3_smart_hybrid_inverter_battery_2_meter()) {
        if (register_start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2Voltage) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2Current) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2Temperature) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2SoC) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS2DesignEnergy) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery2Address::BMS1ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery2Address::TotalDischargeEnergy) {
            // this is the combined discharge energy of both batteries, only show if battery 1 is offline
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status != 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery2Address::TotalChargeEnergy) {
            // this is the combined charge energy of both batteries, only show if battery 1 is offline
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status != 0) {
                value = NAN;
            }
        }
    }
    else if (is_fox_ess_h3_smart_hybrid_inverter_battery_1_and_2_meter()) {
        if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1Voltage) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_smart_hybrid_inverter.bms_1_voltage = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1Current) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_smart_hybrid_inverter.bms_1_current = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1Temperature) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_smart_hybrid_inverter.bms_1_temperature = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1SoC) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_smart_hybrid_inverter.bms_1_soc = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS1DesignEnergy) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_smart_hybrid_inverter.bms_1_design_energy = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2ConnectionStatus) {
            fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2Voltage) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_smart_hybrid_inverter.bms_1_voltage, value);
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2Current) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_sum(fox_ess_h3_smart_hybrid_inverter.bms_1_current, value);
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2Temperature) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_smart_hybrid_inverter.bms_1_temperature, value);
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2SoC) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_smart_hybrid_inverter.bms_1_soc, value);
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterBattery1And2Address::BMS2DesignEnergy) {
            if (fox_ess_h3_smart_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_sum(fox_ess_h3_smart_hybrid_inverter.bms_1_design_energy, value);
        }
    }
    else if (is_fox_ess_h3_smart_hybrid_inverter_pv_meter()) {
        if (register_start_address == FoxESSH3SmartHybridInverterPVAddress::PV1Voltage) {
            fox_ess_h3_smart_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterPVAddress::PV1Current) {
            fox_ess_h3_smart_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterPVAddress::PV2Voltage) {
            fox_ess_h3_smart_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterPVAddress::PV2Current) {
            fox_ess_h3_smart_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterPVAddress::PV3Voltage) {
            fox_ess_h3_smart_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterPVAddress::PV3Current) {
            fox_ess_h3_smart_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterPVAddress::PV4Voltage) {
            fox_ess_h3_smart_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == FoxESSH3SmartHybridInverterPVAddress::PV4Current) {
            fox_ess_h3_smart_hybrid_inverter.pv4_current = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(fox_ess_h3_smart_hybrid_inverter.pv1_voltage)) {
                voltage_sum += fox_ess_h3_smart_hybrid_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(fox_ess_h3_smart_hybrid_inverter.pv2_voltage)) {
                voltage_sum += fox_ess_h3_smart_hybrid_inverter.pv2_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(fox_ess_h3_smart_hybrid_inverter.pv3_voltage)) {
                voltage_sum += fox_ess_h3_smart_hybrid_inverter.pv3_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(fox_ess_h3_smart_hybrid_inverter.pv4_voltage)) {
                voltage_sum += fox_ess_h3_smart_hybrid_inverter.pv4_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = fox_ess_h3_smart_hybrid_inverter.pv1_current
                              + fox_ess_h3_smart_hybrid_inverter.pv2_current
                              + fox_ess_h3_smart_hybrid_inverter.pv3_current
                              + fox_ess_h3_smart_hybrid_inverter.pv4_current;

            meters.update_value(slot, table->index[read_index + 2], voltage_avg);
            meters.update_value(slot, table->index[read_index + 3], current_sum);
        }
    }
    else if (is_fox_ess_h3_pro_hybrid_inverter_battery_1_meter()) {
        if (register_start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1Voltage) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1Current) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1Temperature) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1SoC) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1Address::BMS1DesignEnergy) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1Address::BMS2ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1Address::TotalDischargeEnergy) {
            // this is the combined discharge energy of both batteries, only show if battery 2 is offline
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status != 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1Address::TotalChargeEnergy) {
            // this is the combined charge energy of both batteries, only show if battery 2 is offline
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status != 0) {
                value = NAN;
            }
        }
    }
    else if (is_fox_ess_h3_pro_hybrid_inverter_battery_2_meter()) {
        if (register_start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2Voltage) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2Current) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2Temperature) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2SoC) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery2Address::BMS2DesignEnergy) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery2Address::BMS1ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery2Address::TotalDischargeEnergy) {
            // this is the combined discharge energy of both batteries, only show if battery 1 is offline
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status != 0) {
                value = NAN;
            }
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery2Address::TotalChargeEnergy) {
            // this is the combined charge energy of both batteries, only show if battery 1 is offline
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status != 0) {
                value = NAN;
            }
        }
    }
    else if (is_fox_ess_h3_pro_hybrid_inverter_battery_1_and_2_meter()) {
        if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1Voltage) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_pro_hybrid_inverter.bms_1_voltage = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1Current) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_pro_hybrid_inverter.bms_1_current = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1Temperature) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_pro_hybrid_inverter.bms_1_temperature = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1SoC) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_pro_hybrid_inverter.bms_1_soc = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS1DesignEnergy) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_1_connection_status == 0) {
                value = NAN;
            }

            fox_ess_h3_pro_hybrid_inverter.bms_1_design_energy = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2ConnectionStatus) {
            fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status = c16.u;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2Voltage) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_pro_hybrid_inverter.bms_1_voltage, value);
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2Current) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_sum(fox_ess_h3_pro_hybrid_inverter.bms_1_current, value);
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2Temperature) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_pro_hybrid_inverter.bms_1_temperature, value);
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2SoC) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_avg(fox_ess_h3_pro_hybrid_inverter.bms_1_soc, value);
        }
        else if (register_start_address == FoxESSH3ProHybridInverterBattery1And2Address::BMS2DesignEnergy) {
            if (fox_ess_h3_pro_hybrid_inverter.bms_2_connection_status == 0) {
                value = NAN;
            }

            value = nan_safe_sum(fox_ess_h3_pro_hybrid_inverter.bms_1_design_energy, value);
        }
    }
    else if (is_fox_ess_h3_pro_hybrid_inverter_pv_meter()) {
        if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV1Voltage) {
            fox_ess_h3_pro_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV1Current) {
            fox_ess_h3_pro_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV2Voltage) {
            fox_ess_h3_pro_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV2Current) {
            fox_ess_h3_pro_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV3Voltage) {
            fox_ess_h3_pro_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV3Current) {
            fox_ess_h3_pro_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV4Voltage) {
            fox_ess_h3_pro_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV4Current) {
            fox_ess_h3_pro_hybrid_inverter.pv4_current = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV5Voltage) {
            fox_ess_h3_pro_hybrid_inverter.pv5_voltage = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV5Current) {
            fox_ess_h3_pro_hybrid_inverter.pv5_current = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV6Voltage) {
            fox_ess_h3_pro_hybrid_inverter.pv6_voltage = value;
        }
        else if (register_start_address == FoxESSH3ProHybridInverterPVAddress::PV6Current) {
            fox_ess_h3_pro_hybrid_inverter.pv6_current = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(fox_ess_h3_pro_hybrid_inverter.pv1_voltage)) {
                voltage_sum += fox_ess_h3_pro_hybrid_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(fox_ess_h3_pro_hybrid_inverter.pv2_voltage)) {
                voltage_sum += fox_ess_h3_pro_hybrid_inverter.pv2_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(fox_ess_h3_pro_hybrid_inverter.pv3_voltage)) {
                voltage_sum += fox_ess_h3_pro_hybrid_inverter.pv3_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(fox_ess_h3_pro_hybrid_inverter.pv4_voltage)) {
                voltage_sum += fox_ess_h3_pro_hybrid_inverter.pv4_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(fox_ess_h3_pro_hybrid_inverter.pv5_voltage)) {
                voltage_sum += fox_ess_h3_pro_hybrid_inverter.pv5_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(fox_ess_h3_pro_hybrid_inverter.pv6_voltage)) {
                voltage_sum += fox_ess_h3_pro_hybrid_inverter.pv6_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = fox_ess_h3_pro_hybrid_inverter.pv1_current
                              + fox_ess_h3_pro_hybrid_inverter.pv2_current
                              + fox_ess_h3_pro_hybrid_inverter.pv3_current
                              + fox_ess_h3_pro_hybrid_inverter.pv4_current
                              + fox_ess_h3_pro_hybrid_inverter.pv5_current
                              + fox_ess_h3_pro_hybrid_inverter.pv6_current;

            meters.update_value(slot, table->index[read_index + 2], voltage_avg);
            meters.update_value(slot, table->index[read_index + 3], current_sum);
        }
    }
    else if (is_sma_hybrid_inverter_battery_meter()) {
        if (register_start_address == SMAHybridInverterBatteryAddress::BatChrgCurBatCha) {
            sma_hybrid_inverter.battery_charge_power = value;
        }
        else if (register_start_address == SMAHybridInverterBatteryAddress::BatDschCurBatDsch) {
            sma_hybrid_inverter.battery_discharge_power = value;

            float power = sma_hybrid_inverter.battery_charge_power - sma_hybrid_inverter.battery_discharge_power;

            meters.update_value(slot, table->index[read_index + 1], power);
        }
    }
    else if (is_varta_flex_grid_meter()) {
        if (register_start_address == VARTAFlexGridAddress::GridPower) {
            varta_flex.grid_power = value;
        }
        else if (register_start_address == VARTAFlexGridAddress::GridPowerSF) {
            varta_flex.grid_power_sf = static_cast<int16_t>(c16.u);

            float grid_power = varta_flex.grid_power * get_sun_spec_scale_factor(varta_flex.grid_power_sf);

            meters.update_value(slot, table->index[read_index + 1], grid_power);
        }
    }
    else if (is_varta_flex_battery_meter()) {
        if (register_start_address == VARTAFlexBatteryAddress::ActivePower) {
            varta_flex.battery_active_power = value;
        }
        else if (register_start_address == VARTAFlexBatteryAddress::ApparentPower) {
            varta_flex.battery_apparent_power = value;
        }
        else if (register_start_address == VARTAFlexBatteryAddress::TotalChargeEnergy) {
            varta_flex.battery_total_charge_energy = value;
        }
        else if (register_start_address == VARTAFlexBatteryAddress::ActivePowerSF) {
            varta_flex.battery_active_power_sf = static_cast<int16_t>(c16.u);
        }
        else if (register_start_address == VARTAFlexBatteryAddress::ApparentPowerSF) {
            varta_flex.battery_apparent_power_sf = static_cast<int16_t>(c16.u);
        }
        else if (register_start_address ==VARTAFlexBatteryAddress::TotalChargeEnergySF) {
            varta_flex.battery_total_charge_energy_sf = static_cast<int16_t>(c16.u);

            float active_power = varta_flex.battery_active_power * get_sun_spec_scale_factor(varta_flex.battery_active_power_sf);
            float apparent_power = varta_flex.battery_apparent_power * get_sun_spec_scale_factor(varta_flex.battery_apparent_power_sf);
            float total_charge_energy = varta_flex.battery_total_charge_energy * get_sun_spec_scale_factor(varta_flex.battery_total_charge_energy_sf);

            meters.update_value(slot, table->index[read_index + 1], active_power);
            meters.update_value(slot, table->index[read_index + 2], apparent_power);
            meters.update_value(slot, table->index[read_index + 3], total_charge_energy);
        }
    }
    else if (is_chisage_ess_hybrid_inverter_pv_meter()) {
        if (register_start_address == ChisageESSHybridInverterPVAddress::PV1Voltage) {
            chisage_ess_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == ChisageESSHybridInverterPVAddress::PV1Current) {
            chisage_ess_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == ChisageESSHybridInverterPVAddress::PV2Voltage) {
            chisage_ess_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == ChisageESSHybridInverterPVAddress::PV2Current) {
            chisage_ess_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == ChisageESSHybridInverterPVAddress::PV1Power) {
            chisage_ess_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == ChisageESSHybridInverterPVAddress::PV2Power) {
            chisage_ess_hybrid_inverter.pv2_power = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(chisage_ess_hybrid_inverter.pv1_voltage)) {
                voltage_sum += chisage_ess_hybrid_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(chisage_ess_hybrid_inverter.pv2_voltage)) {
                voltage_sum += chisage_ess_hybrid_inverter.pv2_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = chisage_ess_hybrid_inverter.pv1_current
                              + chisage_ess_hybrid_inverter.pv2_current;

            float power_sum = chisage_ess_hybrid_inverter.pv1_power
                            + chisage_ess_hybrid_inverter.pv2_power;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
        }
    }

    if (table->index[read_index] != VALUE_INDEX_META && table->index[read_index] != VALUE_INDEX_DEBUG) {
        meters.update_value(slot, table->index[read_index], value);
    }

    register_buffer_index += register_count;
    register_start_address += register_count;
    read_index = (read_index + 1) % table->specs_length;

    bool overflow = read_index == 0;

    if (prepare_read()) {
        overflow = true;
    }

    if (overflow) {
        // make a little pause after each round trip
        meters.finish_update(slot);
        read_allowed = true;
    }
    else {
        read_next();
    }
}
