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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modbus_tcp_tools.h"
#include "modbus_register_address_mode.enum.h"
#include "meters_modbus_tcp_defs.inc"

#include "gcc_warnings.h"

//#define DEBUG_LOG_ALL_VALUES

#define NUMBER_TO_ADDRESS(number) ((number) - 1u)

#define SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS                 NUMBER_TO_ADDRESS(5002u)
#define SUNGROW_INVERTER_GRID_FREQUENCY_ADDRESS              NUMBER_TO_ADDRESS(5036u)
#define SUNGROW_HYBRID_INVERTER_RUNNING_STATE_ADDRESS        NUMBER_TO_ADDRESS(13001u)
#define SUNGROW_HYBRID_INVERTER_BATTERY_CURRENT_ADDRESS      NUMBER_TO_ADDRESS(13021u)
#define SUNGROW_HYBRID_INVERTER_BATTERY_POWER_ADDRESS        NUMBER_TO_ADDRESS(13022u)
#define SUNGROW_STRING_INVERTER_TOTAL_ACTVE_POWER_ADDRESS    NUMBER_TO_ADDRESS(5031u)

#define VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L1_ADDRESS 808u
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L2_ADDRESS 809u
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L3_ADDRESS 810u
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
            device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

            const Config *registers = static_cast<const Config *>(ephemeral_config.get("table")->get()->get("registers"));
            uint16_t registers_count = static_cast<uint16_t>(registers->count());

            // FIXME: leaking this, because as of right now meter instances don't get destroied
            ValueSpec *customs_specs = new ValueSpec[registers_count];
            MeterValueID *customs_ids = new MeterValueID[registers_count];
            uint32_t *customs_index = new uint32_t[registers_count];

            for (uint16_t i = 0; i < registers_count; ++i) {
                MeterValueID value_id = registers->get(i)->get("id")->asEnum<MeterValueID>();

                customs_specs[i].name = getMeterValueName(value_id);
                customs_specs[i].register_type = registers->get(i)->get("rtype")->asEnum<ModbusRegisterType>();
                customs_specs[i].start_address = registers->get(i)->get("addr")->asUint();
                customs_specs[i].value_type = registers->get(i)->get("vtype")->asEnum<ModbusValueType>();
                customs_specs[i].offset = registers->get(i)->get("off")->asFloat();
                customs_specs[i].scale_factor = registers->get(i)->get("scale")->asFloat();

                customs_ids[i] = value_id;

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
        sungrow_hybrid_inverter_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<SungrowHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (sungrow_hybrid_inverter_virtual_meter) {
        case SungrowHybridInverterVirtualMeter::None:
            logger.printfln("No Sungrow Hybrid Inverter Virtual Meter selected");
            return;

        case SungrowHybridInverterVirtualMeter::Inverter:
            table = &sungrow_hybrid_inverter_output_type_table;
            break;

        case SungrowHybridInverterVirtualMeter::Grid:
            table = &sungrow_hybrid_inverter_grid_table;
            break;

        case SungrowHybridInverterVirtualMeter::Battery:
            table = &sungrow_hybrid_inverter_battery_table;
            break;

        case SungrowHybridInverterVirtualMeter::Load:
            table = &sungrow_hybrid_inverter_load_table;
            break;

        default:
            logger.printfln("Unknown Sungrow Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(sungrow_hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::SungrowStringInverter:
        sungrow_string_inverter_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<SungrowStringInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (sungrow_string_inverter_virtual_meter) {
        case SungrowStringInverterVirtualMeter::None:
            logger.printfln("No Sungrow String Inverter Virtual Meter selected");
            return;

        case SungrowStringInverterVirtualMeter::Inverter:
            table = &sungrow_string_inverter_output_type_table;
            break;

        case SungrowStringInverterVirtualMeter::Grid:
            table = &sungrow_string_inverter_grid_table;
            break;

        case SungrowStringInverterVirtualMeter::Load:
            table = &sungrow_string_inverter_load_table;
            break;

        default:
            logger.printfln("Unknown Sungrow String Inverter Virtual Meter: %u", static_cast<uint8_t>(sungrow_string_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::SolarmaxMaxStorage:
        solarmax_max_storage_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<SolarmaxMaxStorageVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (solarmax_max_storage_virtual_meter) {
        case SolarmaxMaxStorageVirtualMeter::None:
            logger.printfln("No Solarmax Max Storage Virtual Meter selected");
            return;

        case SolarmaxMaxStorageVirtualMeter::Inverter:
            table = &solarmax_max_storage_inverter_table;
            break;

        case SolarmaxMaxStorageVirtualMeter::Grid:
            table = &solarmax_max_storage_grid_table;
            break;

        case SolarmaxMaxStorageVirtualMeter::Battery:
            table = &solarmax_max_storage_battery_table;
            break;

        default:
            logger.printfln("Unknown Solarmax Max Storage Virtual Meter: %u", static_cast<uint8_t>(solarmax_max_storage_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::VictronEnergyGX:
        victron_energy_gx_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<VictronEnergyGXVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (victron_energy_gx_virtual_meter) {
        case VictronEnergyGXVirtualMeter::None:
            logger.printfln("No Victron Energy GX Virtual Meter selected");
            return;

        case VictronEnergyGXVirtualMeter::Inverter:
            table = &victron_energy_gx_inverter_table;
            break;

        case VictronEnergyGXVirtualMeter::Grid:
            table = &victron_energy_gx_grid_table;
            break;

        case VictronEnergyGXVirtualMeter::Battery:
            table = &victron_energy_gx_battery_table;
            break;

        case VictronEnergyGXVirtualMeter::Load:
            table = &victron_energy_gx_load_table;
            break;

        default:
            logger.printfln("Unknown Victron Energy GX Virtual Meter: %u", static_cast<uint8_t>(victron_energy_gx_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::DeyeHybridInverter:
        deye_hybrid_inverter_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<DeyeHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (deye_hybrid_inverter_virtual_meter) {
        case DeyeHybridInverterVirtualMeter::None:
            logger.printfln("No Deye Hybrid Inverter Virtual Meter selected");
            return;

        case DeyeHybridInverterVirtualMeter::Inverter:
            table = &deye_hybrid_inverter_table;
            break;

        case DeyeHybridInverterVirtualMeter::Grid:
            table = &deye_hybrid_inverter_grid_table;
            break;

        case DeyeHybridInverterVirtualMeter::Battery:
            table = &deye_hybrid_inverter_device_type_table;
            break;

        case DeyeHybridInverterVirtualMeter::Load:
            table = &deye_hybrid_inverter_load_table;
            break;

        default:
            logger.printfln("Unknown Deye Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(deye_hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::AlphaESSHybridInverter:
        alpha_ess_hybrid_inverter_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<AlphaESSHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (alpha_ess_hybrid_inverter_virtual_meter) {
        case AlphaESSHybridInverterVirtualMeter::None:
            logger.printfln("No Alpha ESS Hybrid Inverter Virtual Meter selected");
            return;

        case AlphaESSHybridInverterVirtualMeter::Inverter:
            table = &alpha_ess_hybrid_inverter_table;
            break;

        case AlphaESSHybridInverterVirtualMeter::Grid:
            table = &alpha_ess_hybrid_inverter_grid_table;
            break;

        case AlphaESSHybridInverterVirtualMeter::Battery:
            table = &alpha_ess_hybrid_inverter_battery_table;
            break;

        default:
            logger.printfln("Unknown Alpha ESS Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(alpha_ess_hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::ShellyProEM:
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());
        shelly_pro_em_monophase_channel = ephemeral_config.get("table")->get()->get("monophase_channel")->asEnum<ShellyEMMonophaseChannel>();
        shelly_pro_em_monophase_mapping = ephemeral_config.get("table")->get()->get("monophase_mapping")->asEnum<ShellyEMMonophaseMapping>();

        switch (shelly_pro_em_monophase_channel) {
        case ShellyEMMonophaseChannel::None:
            logger.printfln("No Shelly Pro EM Monophase Channel selected");
            return;

        case ShellyEMMonophaseChannel::First:
            switch (shelly_pro_em_monophase_mapping) {
            case ShellyEMMonophaseMapping::None:
                logger.printfln("No Shelly Pro EM Monophase Mapping selected");
                return;

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
                logger.printfln("Unknown Shelly Pro EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_em_monophase_mapping));
                return;
            }

            break;

        case ShellyEMMonophaseChannel::Second:
            switch (shelly_pro_em_monophase_mapping) {
            case ShellyEMMonophaseMapping::None:
                logger.printfln("No Shelly Pro EM Monophase Mapping selected");
                return;

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
                logger.printfln("Unknown Shelly Pro EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_em_monophase_mapping));
                return;
            }

            break;

        case ShellyEMMonophaseChannel::Third:
            logger.printfln("Impossible Shelly Pro EM Monophase Channel selected: Third");
            return;

        default:
            logger.printfln("Unknown Shelly Pro EM Monophase Channel: %u", static_cast<uint8_t>(shelly_pro_em_monophase_channel));
            return;
        }

        break;

    case MeterModbusTCPTableID::ShellyPro3EM:
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());
        shelly_pro_3em_device_profile = ephemeral_config.get("table")->get()->get("device_profile")->asEnum<ShellyPro3EMDeviceProfile>();
        shelly_pro_3em_monophase_channel = ephemeral_config.get("table")->get()->get("monophase_channel")->asEnum<ShellyEMMonophaseChannel>();
        shelly_pro_3em_monophase_mapping = ephemeral_config.get("table")->get()->get("monophase_mapping")->asEnum<ShellyEMMonophaseMapping>();

        switch (shelly_pro_3em_device_profile) {
        case ShellyPro3EMDeviceProfile::Triphase:
            table = &shelly_em_triphase_table;
            break;

        case ShellyPro3EMDeviceProfile::Monophase:
            switch (shelly_pro_3em_monophase_channel) {
            case ShellyEMMonophaseChannel::None:
                logger.printfln("No Shelly Pro 3EM Monophase Channel selected");
                return;

            case ShellyEMMonophaseChannel::First:
                switch (shelly_pro_3em_monophase_mapping) {
                case ShellyEMMonophaseMapping::None:
                    logger.printfln("No Shelly Pro 3EM Monophase Mapping selected");
                    return;

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
                    logger.printfln("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em_monophase_mapping));
                    return;
                }

                break;

            case ShellyEMMonophaseChannel::Second:
                switch (shelly_pro_3em_monophase_mapping) {
                case ShellyEMMonophaseMapping::None:
                    logger.printfln("No Shelly Pro 3EM Monophase Mapping selected");
                    return;

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
                    logger.printfln("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em_monophase_mapping));
                    return;
                }

                break;

            case ShellyEMMonophaseChannel::Third:
                switch (shelly_pro_3em_monophase_mapping) {
                case ShellyEMMonophaseMapping::None:
                    logger.printfln("No Shelly Pro 3EM Monophase Mapping selected");
                    return;

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
                    logger.printfln("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em_monophase_mapping));
                    return;
                }

                break;

            default:
                logger.printfln("Unknown Shelly Pro 3EM Monophase Channel: %u", static_cast<uint8_t>(shelly_pro_3em_monophase_channel));
                return;
            }

            break;

        default:
            logger.printfln("Unknown Shelly Pro 3EM Device Profile: %u", static_cast<uint8_t>(shelly_pro_3em_device_profile));
            return;
        }

        break;

    case MeterModbusTCPTableID::GoodweHybridInverter:
        goodwe_hybrid_inverter_virtual_meter = ephemeral_config.get("table")->get()->get("virtual_meter")->asEnum<GoodweHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config.get("table")->get()->get("device_address")->asUint());

        switch (goodwe_hybrid_inverter_virtual_meter) {
        case GoodweHybridInverterVirtualMeter::None:
            logger.printfln("No Goodwe Hybrid Inverter Virtual Meter selected");
            return;

        case GoodweHybridInverterVirtualMeter::Inverter:
            table = &goodwe_hybrid_inverter_table;
            break;

        case GoodweHybridInverterVirtualMeter::Grid:
            table = &goodwe_hybrid_inverter_grid_table;
            break;

        case GoodweHybridInverterVirtualMeter::Battery:
            table = &goodwe_hybrid_inverter_battery_table;
            break;

        case GoodweHybridInverterVirtualMeter::Load:
            table = &goodwe_hybrid_inverter_load_table;
            break;

        case GoodweHybridInverterVirtualMeter::BackupLoad:
            table = &goodwe_hybrid_inverter_backup_load_table;
            break;

        case GoodweHybridInverterVirtualMeter::Meter:
            table = &goodwe_hybrid_inverter_meter_table;
            break;

        default:
            logger.printfln("Unknown Goodwe Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(goodwe_hybrid_inverter_virtual_meter));
            return;
        }

        break;

    default:
        logger.printfln("Unknown table: %u", static_cast<uint8_t>(table_id));
        return;
    }

    if (table->ids_length > 0) {
        meters.declare_value_ids(slot, table->ids, table->ids_length);
    }

    task_scheduler.scheduleOnce([this]() {
        this->read_allowed = false;
        this->start_connection();
    }, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (this->read_allowed) {
            this->read_allowed = false;
            this->read_next();
        };
    }, 2000, 1000);
}

void MeterModbusTCP::pre_reboot()
{
    stop_connection();
}

void MeterModbusTCP::connect_callback()
{
    generic_read_request.data[0] = register_buffer;
    generic_read_request.data[1] = nullptr;
    generic_read_request.read_twice = false;
    generic_read_request.done_callback = [this]{ read_done_callback(); };

    read_index = 0;
    register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

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
#ifndef DEBUG_LOG_ALL_VALUES
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
    if (register_buffer_index < generic_read_request.register_count
     && generic_read_request.register_type == table->specs[read_index].register_type
     && generic_read_request.start_address + register_buffer_index == table->specs[read_index].start_address
     && register_buffer_index + MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[read_index].value_type) <= generic_read_request.register_count) {
        read_done_callback();
    }
    else {
        generic_read_request.register_type = table->specs[read_index].register_type;
        generic_read_request.start_address = table->specs[read_index].start_address;
        generic_read_request.register_count = MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[read_index].value_type);

        for (size_t i = read_index + 1; i < table->specs_length; ++i) {
            if (generic_read_request.register_type == table->specs[i].register_type
             && generic_read_request.start_address + generic_read_request.register_count == table->specs[i].start_address
             && generic_read_request.register_count + MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(table->specs[i].value_type) <= METER_MODBUS_TCP_REGISTER_BUFFER_SIZE) {
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

bool MeterModbusTCP::is_sungrow_inverter_meter() const
{
    return (table_id == MeterModbusTCPTableID::SungrowHybridInverter
         && sungrow_hybrid_inverter_virtual_meter == SungrowHybridInverterVirtualMeter::Inverter)
        || (table_id == MeterModbusTCPTableID::SungrowStringInverter
         && sungrow_string_inverter_virtual_meter == SungrowStringInverterVirtualMeter::Inverter);
}

bool MeterModbusTCP::is_sungrow_grid_meter() const
{
    return (table_id == MeterModbusTCPTableID::SungrowHybridInverter
         && sungrow_hybrid_inverter_virtual_meter == SungrowHybridInverterVirtualMeter::Grid)
        || (table_id == MeterModbusTCPTableID::SungrowStringInverter
         && sungrow_string_inverter_virtual_meter == SungrowStringInverterVirtualMeter::Grid);
}

bool MeterModbusTCP::is_sungrow_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::SungrowHybridInverter
        && sungrow_hybrid_inverter_virtual_meter == SungrowHybridInverterVirtualMeter::Battery;
}

bool MeterModbusTCP::is_victron_energy_gx_inverter_meter() const
{
    return table_id == MeterModbusTCPTableID::VictronEnergyGX
        && victron_energy_gx_virtual_meter == VictronEnergyGXVirtualMeter::Inverter;
}

bool MeterModbusTCP::is_victron_energy_gx_load_meter() const
{
    return table_id == MeterModbusTCPTableID::VictronEnergyGX
        && victron_energy_gx_virtual_meter == VictronEnergyGXVirtualMeter::Load;
}

bool MeterModbusTCP::is_deye_hybrid_inverter_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::DeyeHybridInverter
        && deye_hybrid_inverter_virtual_meter == DeyeHybridInverterVirtualMeter::Battery;
}

void MeterModbusTCP::read_done_callback()
{
    if (generic_read_request.result_code != Modbus::ResultCode::EX_SUCCESS) {
        logger.printfln("Error reading %s / %s (address: %zu, number: %zu): %s [%d]",
                        get_meter_modbus_tcp_table_id_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        table->specs[read_index].start_address + 1,
                        get_modbus_result_code_name(generic_read_request.result_code),
                        generic_read_request.result_code);

        read_allowed = true;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        return;
    }

    if (is_sungrow_inverter_meter()
     && generic_read_request.start_address == SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS) {
        if (sungrow_inverter_output_type < 0) {
            switch (register_buffer[register_buffer_index]) {
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
                logger.printfln("%s has unknown Output Type: %u", get_meter_modbus_tcp_table_id_name(table_id), register_buffer[0]);
                return;
            }

            sungrow_inverter_output_type = register_buffer[register_buffer_index];

#ifdef DEBUG_LOG_ALL_VALUES
            logger.printfln("%s / Output Type (%u): %d",
                            get_meter_modbus_tcp_table_id_name(table_id),
                            SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS,
                            sungrow_inverter_output_type);
#endif

            meters.declare_value_ids(slot, table->ids, table->ids_length);
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();

        return;
    }

    if (is_deye_hybrid_inverter_battery_meter()
     && generic_read_request.start_address == DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS) {
        if (deye_hybrid_inverter_device_type < 0) {
            switch (register_buffer[register_buffer_index]) {
            case 0x0002:
            case 0x0003:
            case 0x0004:
                logger.printfln("%s has unsupported Device Type: 0x%04x", get_meter_modbus_tcp_table_id_name(table_id), register_buffer[0]);
                return;

            case 0x0005:
                table = &deye_hybrid_inverter_low_voltage_battery_table;
                break;

            case 0x0006:
            case 0x0106:
                table = &deye_hybrid_inverter_high_voltage_battery_table;
                break;

            default:
                logger.printfln("%s has unknown Device Type: 0x%04x", get_meter_modbus_tcp_table_id_name(table_id), register_buffer[0]);
                return;
            }

            deye_hybrid_inverter_device_type = register_buffer[register_buffer_index];

#ifdef DEBUG_LOG_ALL_VALUES
            logger.printfln("%s / Device Type (%u): %d",
                            get_meter_modbus_tcp_table_id_name(table_id),
                            DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS,
                            deye_hybrid_inverter_device_type);
#endif

            meters.declare_value_ids(slot, table->ids, table->ids_length);
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

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
            c32.r[0] = register_buffer[register_buffer_index + 0];
            c32.r[1] = register_buffer[register_buffer_index + 1];
        }
        else {
            c32.r[0] = register_buffer[register_buffer_index + 1];
            c32.r[1] = register_buffer[register_buffer_index + 0];
        }

        break;

    case 4:
        if (MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(value_type)) {
            c64.r[0] = register_buffer[register_buffer_index + 0];
            c64.r[1] = register_buffer[register_buffer_index + 1];
            c64.r[2] = register_buffer[register_buffer_index + 2];
            c64.r[3] = register_buffer[register_buffer_index + 3];
        }
        else {
            c64.r[0] = register_buffer[register_buffer_index + 3];
            c64.r[1] = register_buffer[register_buffer_index + 2];
            c64.r[2] = register_buffer[register_buffer_index + 1];
            c64.r[3] = register_buffer[register_buffer_index + 0];
        }

        break;

    default:
        logger.printfln("%s / %s has unsupported register count: %u", get_meter_modbus_tcp_table_id_name(table_id), table->specs[read_index].name, register_count);
        return;
    }

    switch (value_type) {
    case ModbusValueType::None:
        break;

    case ModbusValueType::U16:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %u",
                        get_meter_modbus_tcp_table_id_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        register_buffer[register_buffer_index]);
#endif

        value = static_cast<float>(register_buffer[register_buffer_index]);
        break;

    case ModbusValueType::S16:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %d",
                        get_meter_modbus_tcp_table_id_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        static_cast<int16_t>(register_buffer[register_buffer_index]));
#endif

        value = static_cast<float>(static_cast<int16_t>(register_buffer[register_buffer_index]));
        break;

    case ModbusValueType::U32BE:
    case ModbusValueType::U32LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %u (%u %u)",
                        get_meter_modbus_tcp_table_id_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        c32.u,
                        register_buffer[register_buffer_index + 0],
                        register_buffer[register_buffer_index + 1]);
#endif

        value = static_cast<float>(c32.u);
        break;

    case ModbusValueType::S32BE:
    case ModbusValueType::S32LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %d (%u %u)",
                        get_meter_modbus_tcp_table_id_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        static_cast<int32_t>(c32.u),
                        register_buffer[register_buffer_index + 0],
                        register_buffer[register_buffer_index + 1]);
#endif

        value = static_cast<float>(static_cast<int32_t>(c32.u));
        break;

    case ModbusValueType::F32BE:
    case ModbusValueType::F32LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %f (%u %u)",
                        get_meter_modbus_tcp_table_id_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        static_cast<double>(c32.f),
                        register_buffer[register_buffer_index + 0],
                        register_buffer[register_buffer_index + 1]);
#endif

        value = c32.f;
        break;

    case ModbusValueType::U64BE:
    case ModbusValueType::U64LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %llu (%u %u %u %u)",
                        get_meter_modbus_tcp_table_id_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        c64.u,
                        register_buffer[register_buffer_index + 0],
                        register_buffer[register_buffer_index + 1],
                        register_buffer[register_buffer_index + 2],
                        register_buffer[register_buffer_index + 3]);
#endif

        value = static_cast<float>(c64.u);
        break;

    case ModbusValueType::S64BE:
    case ModbusValueType::S64LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %lld (%u %u %u %u)",
                        get_meter_modbus_tcp_table_id_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        static_cast<int64_t>(c64.u),
                        register_buffer[register_buffer_index + 0],
                        register_buffer[register_buffer_index + 1],
                        register_buffer[register_buffer_index + 2],
                        register_buffer[register_buffer_index + 3]);
#endif

        value = static_cast<float>(static_cast<int64_t>(c64.u));
        break;

    case ModbusValueType::F64BE:
    case ModbusValueType::F64LE:
#ifdef DEBUG_LOG_ALL_VALUES
        logger.printfln("%s / %s (%zu): %f (%u %u %u %u)",
                        get_meter_modbus_tcp_table_id_name(table_id),
                        table->specs[read_index].name,
                        table->specs[read_index].start_address,
                        c64.f,
                        register_buffer[register_buffer_index + 0],
                        register_buffer[register_buffer_index + 1],
                        register_buffer[register_buffer_index + 2],
                        register_buffer[register_buffer_index + 3]);
#endif

        value = static_cast<float>(c64.f);
        break;

    default:
        break;
    }

    value = (value + table->specs[read_index].offset) * table->specs[read_index].scale_factor;

    if (is_sungrow_grid_meter()) {
        if (register_start_address == SUNGROW_INVERTER_GRID_FREQUENCY_ADDRESS) {
            if (value > 100) {
                // according to the spec the grid frequency is given
                // as 0.1 Hz, but some inverters report it as 0.01 Hz
                value /= 10;
            }
        }
    }
    else if (is_sungrow_inverter_meter()) {
        if (register_start_address == SUNGROW_STRING_INVERTER_TOTAL_ACTVE_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], -value);
        }
    }
    else if (is_sungrow_battery_meter()) {
        if (register_start_address == SUNGROW_HYBRID_INVERTER_RUNNING_STATE_ADDRESS) {
            sungrow_hybrid_inverter_running_state = register_buffer[0];
        }
        else if (register_start_address == SUNGROW_HYBRID_INVERTER_BATTERY_CURRENT_ADDRESS) {
            if ((sungrow_hybrid_inverter_running_state & (1 << 2)) != 0) {
                value = -value;
            }
        }
        else if (register_start_address == SUNGROW_HYBRID_INVERTER_BATTERY_POWER_ADDRESS) {
            if ((sungrow_hybrid_inverter_running_state & (1 << 2)) != 0) {
                value = -value;
            }
        }
    }
    else if (is_victron_energy_gx_inverter_meter()) {
        if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L1_ADDRESS) {
            victron_energy_gx_ac_coupled_pv_on_output_l1_power = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L2_ADDRESS) {
            victron_energy_gx_ac_coupled_pv_on_output_l2_power = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L3_ADDRESS) {
            victron_energy_gx_ac_coupled_pv_on_output_l3_power = value;

            float power = victron_energy_gx_ac_coupled_pv_on_output_l1_power
                        + victron_energy_gx_ac_coupled_pv_on_output_l2_power
                        + victron_energy_gx_ac_coupled_pv_on_output_l3_power;

            meters.update_value(slot, table->index[read_index + 1], -power);
        }
    }
    else if (is_victron_energy_gx_load_meter()) {
        if (register_start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L1_ADDRESS) {
            victron_energy_gx_ac_consumption_l1_power = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L2_ADDRESS) {
            victron_energy_gx_ac_consumption_l2_power = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L3_ADDRESS) {
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
