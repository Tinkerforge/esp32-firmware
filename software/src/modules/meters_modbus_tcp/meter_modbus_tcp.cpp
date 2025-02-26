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
#include "modules/meters/meter_location.enum.h"
#include "modules/modbus_tcp_client/modbus_tcp_tools.h"
#include "modules/modbus_tcp_client/modbus_register_address_mode.enum.h"
#include "meters_modbus_tcp_defs.inc"

#include "gcc_warnings.h"

//#define DEBUG_VALUES_TO_TRACE_LOG

#define NUMBER_TO_ADDRESS(number) ((number) - 1u)

#define SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS                               static_cast<size_t>(SungrowInverterOutputTypeAddress::OutputType)
#define SUNGROW_INVERTER_GRID_FREQUENCY_ADDRESS                            static_cast<size_t>(SungrowHybridInverterGridAddress::GridFrequency)
#define SUNGROW_HYBRID_INVERTER_RUNNING_STATE_ADDRESS                      static_cast<size_t>(SungrowHybridInverterBatteryAddress::RunningState)
#define SUNGROW_HYBRID_INVERTER_BATTERY_CURRENT_ADDRESS                    static_cast<size_t>(SungrowHybridInverterBatteryAddress::BatteryCurrent)
#define SUNGROW_HYBRID_INVERTER_BATTERY_POWER_ADDRESS                      static_cast<size_t>(SungrowHybridInverterBatteryAddress::BatteryPower)
#define SUNGROW_STRING_INVERTER_TOTAL_ACTIVE_POWER_ADDRESS                 static_cast<size_t>(SungrowStringInverter1P2LAddress::TotalActivePowerExport)

#define VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L1_ADDRESS               static_cast<size_t>(VictronEnergyGXInverterAddress::ACCoupledPVOnOutputL1)
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L2_ADDRESS               static_cast<size_t>(VictronEnergyGXInverterAddress::ACCoupledPVOnOutputL2)
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L3_ADDRESS               static_cast<size_t>(VictronEnergyGXInverterAddress::ACCoupledPVOnOutputL3)
#define VICTRON_ENERGY_GX_AC_CONSUMPTION_L1_ADDRESS                        static_cast<size_t>(VictronEnergyGXLoadAddress::ACConsumptionL1)
#define VICTRON_ENERGY_GX_AC_CONSUMPTION_L2_ADDRESS                        static_cast<size_t>(VictronEnergyGXLoadAddress::ACConsumptionL2)
#define VICTRON_ENERGY_GX_AC_CONSUMPTION_L3_ADDRESS                        static_cast<size_t>(VictronEnergyGXLoadAddress::ACConsumptionL3)

#define DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS                           static_cast<size_t>(DeyeHybridInverterDeviceTypeAddress::DeviceType)
#define DEYE_HYBRID_INVERTER_PV1_POWER_ADDRESS                             static_cast<size_t>(DeyeHybridInverterPVAddress::PV1Power)
#define DEYE_HYBRID_INVERTER_PV2_POWER_ADDRESS                             static_cast<size_t>(DeyeHybridInverterPVAddress::PV2Power)
#define DEYE_HYBRID_INVERTER_PV3_POWER_ADDRESS                             static_cast<size_t>(DeyeHybridInverterPVAddress::PV3Power)
#define DEYE_HYBRID_INVERTER_PV4_POWER_ADDRESS                             static_cast<size_t>(DeyeHybridInverterPVAddress::PV4Power)
#define DEYE_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS                           static_cast<size_t>(DeyeHybridInverterPVAddress::PV1Voltage)
#define DEYE_HYBRID_INVERTER_PV1_CURRENT_ADDRESS                           static_cast<size_t>(DeyeHybridInverterPVAddress::PV1Current)
#define DEYE_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS                           static_cast<size_t>(DeyeHybridInverterPVAddress::PV2Voltage)
#define DEYE_HYBRID_INVERTER_PV2_CURRENT_ADDRESS                           static_cast<size_t>(DeyeHybridInverterPVAddress::PV2Current)
#define DEYE_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS                           static_cast<size_t>(DeyeHybridInverterPVAddress::PV3Voltage)
#define DEYE_HYBRID_INVERTER_PV3_CURRENT_ADDRESS                           static_cast<size_t>(DeyeHybridInverterPVAddress::PV3Current)
#define DEYE_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS                           static_cast<size_t>(DeyeHybridInverterPVAddress::PV4Voltage)
#define DEYE_HYBRID_INVERTER_PV4_CURRENT_ADDRESS                           static_cast<size_t>(DeyeHybridInverterPVAddress::PV4Current)

#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_ACTIVE_POWER                    static_cast<size_t>(ShellyEMMonophaseChannel1AsL1Address::Channel1ActivePower)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_TOTAL_ACTIVE_ENERGY             static_cast<size_t>(ShellyEMMonophaseChannel1AsL1Address::Channel1TotalActiveEnergyPerpetualCount)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_TOTAL_ACTIVE_RETURNED_ENERGY    static_cast<size_t>(ShellyEMMonophaseChannel1AsL1Address::Channel1TotalActiveReturnedEnergyPerpetualCount)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_ACTIVE_POWER                    static_cast<size_t>(ShellyEMMonophaseChannel2AsL1Address::Channel2ActivePower)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_TOTAL_ACTIVE_ENERGY             static_cast<size_t>(ShellyEMMonophaseChannel2AsL1Address::Channel2TotalActiveEnergyPerpetualCount)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_TOTAL_ACTIVE_RETURNED_ENERGY    static_cast<size_t>(ShellyEMMonophaseChannel2AsL1Address::Channel2TotalActiveReturnedEnergyPerpetualCount)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_ACTIVE_POWER                    static_cast<size_t>(ShellyEMMonophaseChannel3AsL1Address::Channel3ActivePower)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_TOTAL_ACTIVE_ENERGY             static_cast<size_t>(ShellyEMMonophaseChannel3AsL1Address::Channel3TotalActiveEnergyPerpetualCount)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_TOTAL_ACTIVE_RETURNED_ENERGY    static_cast<size_t>(ShellyEMMonophaseChannel3AsL1Address::Channel3TotalActiveReturnedEnergyPerpetualCount)

#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_INPUT_OR_MODEL_ID_ADDRESS       static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryTypeAddress::InputOrModelID)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DCA_SF_ADDRESS                  static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::DCA_SF)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DCV_SF_ADDRESS                  static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::DCV_SF)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DCW_SF_ADDRESS                  static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::DCW_SF)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DCWH_SF_ADDRESS                 static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::DCWH_SF)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHARGE_DCA_ADDRESS              static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::ChargeDCA)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHARGE_DCV_ADDRESS              static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::ChargeDCV)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHARGE_DCW_ADDRESS              static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::ChargeDCW)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHARGE_DCWH_ADDRESS             static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::ChargeDCWH)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DISCHARGE_DCA_ADDRESS           static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::DischargeDCA)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DISCHARGE_DCV_ADDRESS           static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::DischargeDCV)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DISCHARGE_DCW_ADDRESS           static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::DischargeDCW)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DISCHARGE_DCWH_ADDRESS          static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::DischargeDCWH)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHASTATE_ADDRESS                static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::ChaState)
#define FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHASTATE_SF_ADDRESS             static_cast<size_t>(FroniusGEN24PlusHybridInverterBatteryIntegerAddress::ChaState_SF)

#define CARLO_GAVAZZI_EM100_OR_ET100_W                                     static_cast<size_t>(CarloGavazziEM100andET100AtL1Address::W)
#define CARLO_GAVAZZI_EM100_OR_ET100_KWH_POSITIVE_TOTAL                    static_cast<size_t>(CarloGavazziEM100andET100AtL1Address::KWhPositiveTotal)
#define CARLO_GAVAZZI_EM100_OR_ET100_KWH_NEGATIVE_TOTAL                    static_cast<size_t>(CarloGavazziEM100andET100AtL1Address::KWhNegativeTotal)
#define CARLO_GAVAZZI_EM510_W                                              static_cast<size_t>(CarloGavazziEM510atL1Address::W)
#define CARLO_GAVAZZI_EM510_KWH_POSITIVE_TOTAL                             static_cast<size_t>(CarloGavazziEM510atL1Address::KWhPositiveTotal)
#define CARLO_GAVAZZI_EM510_KWH_NEGATIVE_TOTAL                             static_cast<size_t>(CarloGavazziEM510atL1Address::KWhNegativeTotal)

#define SOLAREDGE_INVERTER_BATTERY_1_TEMPERATURE                           static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery1AverageTemperature)
#define SOLAREDGE_INVERTER_BATTERY_1_VOLTAGE                               static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery1InstantaneousVoltage)
#define SOLAREDGE_INVERTER_BATTERY_1_CURRENT                               static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery1InstantaneousCurrent)
#define SOLAREDGE_INVERTER_BATTERY_1_POWER                                 static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery1InstantaneousPower)
#define SOLAREDGE_INVERTER_BATTERY_1_EXPORT_ENERGY                         static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery1LifetimeExportEnergyCounter)
#define SOLAREDGE_INVERTER_BATTERY_1_IMPORT_ENERGY                         static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery1LifetimeImportEnergyCounter)
#define SOLAREDGE_INVERTER_BATTERY_1_STATE_OF_CHARGE                       static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery1StateOfEnergy)
#define SOLAREDGE_INVERTER_BATTERY_2_TEMPERATURE                           static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery2AverageTemperature)
#define SOLAREDGE_INVERTER_BATTERY_2_VOLTAGE                               static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery2InstantaneousVoltage)
#define SOLAREDGE_INVERTER_BATTERY_2_CURRENT                               static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery2InstantaneousCurrent)
#define SOLAREDGE_INVERTER_BATTERY_2_POWER                                 static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery2InstantaneousPower)
#define SOLAREDGE_INVERTER_BATTERY_2_EXPORT_ENERGY                         static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery2LifetimeExportEnergyCounter)
#define SOLAREDGE_INVERTER_BATTERY_2_IMPORT_ENERGY                         static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery2LifetimeImportEnergyCounter)
#define SOLAREDGE_INVERTER_BATTERY_2_STATE_OF_CHARGE                       static_cast<size_t>(SolaredgeInverterBatteryAddress::Battery2StateOfEnergy)

#define MODBUS_VALUE_TYPE_TO_REGISTER_COUNT(x) (static_cast<uint8_t>(x) & 0x07)
#define MODBUS_VALUE_TYPE_TO_REGISTER_ORDER_LE(x) ((static_cast<uint8_t>(x) >> 5) & 1)

#define trace(fmt, ...) \
    do { \
        meters_modbus_tcp.trace_timestamp(); \
        logger.tracefln_plain(trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

static const float fronius_scale_factors[21] = {
              0.0000000001f,    // 10^-10
              0.000000001f,     // 10^-9
              0.00000001f,      // 10^-8
              0.0000001f,       // 10^-7
              0.000001f,        // 10^-6
              0.00001f,         // 10^-5
              0.0001f,          // 10^-4
              0.001f,           // 10^-3
              0.01f,            // 10^-2
              0.1f,             // 10^-1
              1.0f,             // 10^0
             10.0f,             // 10^1
            100.0f,             // 10^2
           1000.0f,             // 10^3
          10000.0f,             // 10^4
         100000.0f,             // 10^5
        1000000.0f,             // 10^6
       10000000.0f,             // 10^7
      100000000.0f,             // 10^8
     1000000000.0f,             // 10^9
    10000000000.0f,             // 10^10
};

static float get_fronius_scale_factor(int16_t sf)
{
    if (sf < -10) {
        if (sf == INT16_MIN) { // scale factor not implemented
            return 1;
        } else {
            return NAN;
        }
    } else if (sf > 10) {
        return NAN;
    }

    return fronius_scale_factors[sf + 10];
}

static float nan_safe_sum(float a, float b)
{
    if (isnan(a)) {
        return b;
    }

    if (isnan(b)) {
        return a;
    }

    return a + b;
}

static float nan_safe_avg(float a, float b)
{
    if (isnan(a)) {
        return b;
    }

    if (isnan(b)) {
        return a;
    }

    return (a + b) / 2.0f;
}

MeterClassID MeterModbusTCP::get_class() const
{
    return MeterClassID::ModbusTCP;
}

void MeterModbusTCP::setup(Config *ephemeral_config)
{
    host_name = ephemeral_config->get("host")->asString();
    port      = static_cast<uint16_t>(ephemeral_config->get("port")->asUint());
    table_id  = ephemeral_config->get("table")->getTag<MeterModbusTCPTableID>();

    MeterLocation default_location = MeterLocation::Unknown;

    switch (table_id) {
    case MeterModbusTCPTableID::None:
        logger.printfln("No table selected");
        return;

    case MeterModbusTCPTableID::Custom: {
            device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

            const Config *registers = static_cast<const Config *>(ephemeral_config->get("table")->get()->get("registers"));
            size_t registers_count = registers->count();

            // FIXME: leaking this, because as of right now meter instances don't get destroyed
            ValueSpec *customs_specs = new ValueSpec[registers_count];
            MeterValueID *customs_ids = new MeterValueID[registers_count];
            uint32_t *customs_index = new uint32_t[registers_count];

            for (size_t i = 0; i < registers_count; ++i) {
                MeterValueID value_id = registers->get(i)->get("id")->asEnum<MeterValueID>();

                customs_specs[i].name = getMeterValueName(value_id);
                customs_specs[i].register_type = registers->get(i)->get("rtype")->asEnum<ModbusRegisterType>();
                customs_specs[i].start_address = registers->get(i)->get("addr")->asUint();
                customs_specs[i].value_type = registers->get(i)->get("vtype")->asEnum<ModbusValueType>();
                customs_specs[i].drop_sign = false; // FIXME: expose in API?
                customs_specs[i].offset = registers->get(i)->get("off")->asFloat();
                customs_specs[i].scale_factor = registers->get(i)->get("scale")->asFloat();

                customs_ids[i] = value_id;

                customs_index[i] = i;
            }

            // FIXME: leaking this, because as of right now meter instances don't get destroyed
            custom.table = new ValueTable;
            custom.table->specs = customs_specs;
            custom.table->specs_length = registers_count;
            custom.table->ids = customs_ids;
            custom.table->ids_length = registers_count;
            custom.table->index = customs_index;

            table = custom.table;
        }

        break;

    case MeterModbusTCPTableID::SungrowHybridInverter:
        sungrow.hybrid_inverter_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SungrowHybridInverterVirtualMeter>();
        sungrow.inverter_output_type = -1;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (sungrow.hybrid_inverter_virtual_meter) {
        case SungrowHybridInverterVirtualMeter::None:
            logger.printfln("No Sungrow Hybrid Inverter Virtual Meter selected");
            return;

        case SungrowHybridInverterVirtualMeter::Inverter:
            table = &sungrow_inverter_output_type_table;
            default_location = MeterLocation::Inverter;
            break;

        case SungrowHybridInverterVirtualMeter::Grid:
            table = &sungrow_hybrid_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case SungrowHybridInverterVirtualMeter::Battery:
            table = &sungrow_hybrid_inverter_battery_table;
            default_location = MeterLocation::Battery;
            break;

        case SungrowHybridInverterVirtualMeter::Load:
            table = &sungrow_hybrid_inverter_load_table;
            default_location = MeterLocation::Load;
            break;

        default:
            logger.printfln("Unknown Sungrow Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(sungrow.hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::SungrowStringInverter:
        sungrow.string_inverter_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SungrowStringInverterVirtualMeter>();
        sungrow.inverter_output_type = -1;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (sungrow.string_inverter_virtual_meter) {
        case SungrowStringInverterVirtualMeter::None:
            logger.printfln("No Sungrow String Inverter Virtual Meter selected");
            return;

        case SungrowStringInverterVirtualMeter::Inverter:
            table = &sungrow_inverter_output_type_table;
            default_location = MeterLocation::Inverter;
            break;

        case SungrowStringInverterVirtualMeter::Grid:
            table = &sungrow_string_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case SungrowStringInverterVirtualMeter::Load:
            table = &sungrow_string_inverter_load_table;
            default_location = MeterLocation::Load;
            break;

        default:
            logger.printfln("Unknown Sungrow String Inverter Virtual Meter: %u", static_cast<uint8_t>(sungrow.string_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::SolarmaxMaxStorage:
        solarmax.max_storage_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SolarmaxMaxStorageVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (solarmax.max_storage_virtual_meter) {
        case SolarmaxMaxStorageVirtualMeter::None:
            logger.printfln("No Solarmax Max Storage Virtual Meter selected");
            return;

        case SolarmaxMaxStorageVirtualMeter::Inverter:
            table = &solarmax_max_storage_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case SolarmaxMaxStorageVirtualMeter::Grid:
            table = &solarmax_max_storage_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case SolarmaxMaxStorageVirtualMeter::Battery:
            table = &solarmax_max_storage_battery_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln("Unknown Solarmax Max Storage Virtual Meter: %u", static_cast<uint8_t>(solarmax.max_storage_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::VictronEnergyGX:
        victron_energy_gx.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<VictronEnergyGXVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (victron_energy_gx.virtual_meter) {
        case VictronEnergyGXVirtualMeter::None:
            logger.printfln("No Victron Energy GX Virtual Meter selected");
            return;

        case VictronEnergyGXVirtualMeter::Inverter:
            table = &victron_energy_gx_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case VictronEnergyGXVirtualMeter::Grid:
            table = &victron_energy_gx_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case VictronEnergyGXVirtualMeter::Battery:
            table = &victron_energy_gx_battery_table;
            default_location = MeterLocation::Battery;
            break;

        case VictronEnergyGXVirtualMeter::Load:
            table = &victron_energy_gx_load_table;
            default_location = MeterLocation::Load;
            break;

        default:
            logger.printfln("Unknown Victron Energy GX Virtual Meter: %u", static_cast<uint8_t>(victron_energy_gx.virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::DeyeHybridInverter:
        deye.hybrid_inverter_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<DeyeHybridInverterVirtualMeter>();
        deye.hybrid_inverter_device_type = -1;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (deye.hybrid_inverter_virtual_meter) {
        case DeyeHybridInverterVirtualMeter::None:
            logger.printfln("No Deye Hybrid Inverter Virtual Meter selected");
            return;

        case DeyeHybridInverterVirtualMeter::Inverter:
            table = &deye_hybrid_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case DeyeHybridInverterVirtualMeter::Grid:
            table = &deye_hybrid_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case DeyeHybridInverterVirtualMeter::Battery:
            table = &deye_hybrid_inverter_device_type_table;
            default_location = MeterLocation::Battery;
            break;

        case DeyeHybridInverterVirtualMeter::Load:
            table = &deye_hybrid_inverter_load_table;
            default_location = MeterLocation::Load;
            break;

        case DeyeHybridInverterVirtualMeter::PV:
            table = &deye_hybrid_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln("Unknown Deye Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(deye.hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::AlphaESSHybridInverter:
        alpha_ess.hybrid_inverter_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<AlphaESSHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (alpha_ess.hybrid_inverter_virtual_meter) {
        case AlphaESSHybridInverterVirtualMeter::None:
            logger.printfln("No Alpha ESS Hybrid Inverter Virtual Meter selected");
            return;

        case AlphaESSHybridInverterVirtualMeter::Inverter:
            table = &alpha_ess_hybrid_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case AlphaESSHybridInverterVirtualMeter::Grid:
            table = &alpha_ess_hybrid_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case AlphaESSHybridInverterVirtualMeter::Battery:
            table = &alpha_ess_hybrid_inverter_battery_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln("Unknown Alpha ESS Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(alpha_ess.hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::ShellyProEM:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        shelly_pro_em.monophase_channel = ephemeral_config->get("table")->get()->get("monophase_channel")->asEnum<ShellyEMMonophaseChannel>();
        shelly_pro_em.monophase_mapping = ephemeral_config->get("table")->get()->get("monophase_mapping")->asEnum<ShellyEMMonophaseMapping>();

        switch (shelly_pro_em.monophase_channel) {
        case ShellyEMMonophaseChannel::None:
            logger.printfln("No Shelly Pro EM Monophase Channel selected");
            return;

        case ShellyEMMonophaseChannel::First:
            switch (shelly_pro_em.monophase_mapping) {
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
                logger.printfln("Unknown Shelly Pro EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_em.monophase_mapping));
                return;
            }

            break;

        case ShellyEMMonophaseChannel::Second:
            switch (shelly_pro_em.monophase_mapping) {
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
                logger.printfln("Unknown Shelly Pro EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_em.monophase_mapping));
                return;
            }

            break;

        case ShellyEMMonophaseChannel::Third:
            logger.printfln("Impossible Shelly Pro EM Monophase Channel selected: Third");
            return;

        default:
            logger.printfln("Unknown Shelly Pro EM Monophase Channel: %u", static_cast<uint8_t>(shelly_pro_em.monophase_channel));
            return;
        }

        break;

    case MeterModbusTCPTableID::ShellyPro3EM:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        shelly_pro_3em.device_profile = ephemeral_config->get("table")->get()->get("device_profile")->asEnum<ShellyPro3EMDeviceProfile>();
        shelly_pro_3em.monophase_channel = ephemeral_config->get("table")->get()->get("monophase_channel")->asEnum<ShellyEMMonophaseChannel>();
        shelly_pro_3em.monophase_mapping = ephemeral_config->get("table")->get()->get("monophase_mapping")->asEnum<ShellyEMMonophaseMapping>();

        switch (shelly_pro_3em.device_profile) {
        case ShellyPro3EMDeviceProfile::Triphase:
            table = &shelly_em_triphase_table;
            break;

        case ShellyPro3EMDeviceProfile::Monophase:
            switch (shelly_pro_3em.monophase_channel) {
            case ShellyEMMonophaseChannel::None:
                logger.printfln("No Shelly Pro 3EM Monophase Channel selected");
                return;

            case ShellyEMMonophaseChannel::First:
                switch (shelly_pro_3em.monophase_mapping) {
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
                    logger.printfln("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_mapping));
                    return;
                }

                break;

            case ShellyEMMonophaseChannel::Second:
                switch (shelly_pro_3em.monophase_mapping) {
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
                    logger.printfln("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_mapping));
                    return;
                }

                break;

            case ShellyEMMonophaseChannel::Third:
                switch (shelly_pro_3em.monophase_mapping) {
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
                    logger.printfln("Unknown Shelly Pro 3EM Monophase Mapping: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_mapping));
                    return;
                }

                break;

            default:
                logger.printfln("Unknown Shelly Pro 3EM Monophase Channel: %u", static_cast<uint8_t>(shelly_pro_3em.monophase_channel));
                return;
            }

            break;

        default:
            logger.printfln("Unknown Shelly Pro 3EM Device Profile: %u", static_cast<uint8_t>(shelly_pro_3em.device_profile));
            return;
        }

        break;

    case MeterModbusTCPTableID::GoodweHybridInverter:
        goodwe.hybrid_inverter_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<GoodweHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (goodwe.hybrid_inverter_virtual_meter) {
        case GoodweHybridInverterVirtualMeter::None:
            logger.printfln("No Goodwe Hybrid Inverter Virtual Meter selected");
            return;

        case GoodweHybridInverterVirtualMeter::Inverter:
            table = &goodwe_hybrid_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case GoodweHybridInverterVirtualMeter::Grid:
            table = &goodwe_hybrid_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case GoodweHybridInverterVirtualMeter::Battery:
            table = &goodwe_hybrid_inverter_battery_table;
            default_location = MeterLocation::Battery;
            break;

        case GoodweHybridInverterVirtualMeter::Load:
            table = &goodwe_hybrid_inverter_load_table;
            default_location = MeterLocation::Load;
            break;

        case GoodweHybridInverterVirtualMeter::BackupLoad:
            table = &goodwe_hybrid_inverter_backup_load_table;
            default_location = MeterLocation::Load;
            break;

        case GoodweHybridInverterVirtualMeter::Meter:
            table = &goodwe_hybrid_inverter_meter_table;
            default_location = MeterLocation::Other;
            break;

        default:
            logger.printfln("Unknown Goodwe Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(goodwe.hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::SolaxHybridInverter:
        solax.hybrid_inverter_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SolaxHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (solax.hybrid_inverter_virtual_meter) {
        case SolaxHybridInverterVirtualMeter::None:
            logger.printfln("No Solax Hybrid Inverter Virtual Meter selected");
            return;

        case SolaxHybridInverterVirtualMeter::Inverter:
            table = &solax_hybrid_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case SolaxHybridInverterVirtualMeter::Grid:
            table = &solax_hybrid_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case SolaxHybridInverterVirtualMeter::Battery:
            table = &solax_hybrid_inverter_battery_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln("Unknown Solax Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(solax.hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::FroniusGEN24PlusHybridInverter:
        fronius.gen24_plus_hybrid_inverter_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<FroniusGEN24PlusHybridInverterVirtualMeter>();
        fronius.gen24_plus_hybrid_inverter_input_or_model_id = 0;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (fronius.gen24_plus_hybrid_inverter_virtual_meter) {
        case FroniusGEN24PlusHybridInverterVirtualMeter::None:
            logger.printfln("No Fronius GEN24 Plus Hybrid Inverter Virtual Meter selected");
            return;

        case FroniusGEN24PlusHybridInverterVirtualMeter::InverterUnused:
            logger.printfln("Invalid Fronius GEN24 Plus Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(fronius.gen24_plus_hybrid_inverter_virtual_meter));
            default_location = MeterLocation::Inverter;
            return;

        case FroniusGEN24PlusHybridInverterVirtualMeter::GridUnused:
            logger.printfln("Invalid Fronius GEN24 Plus Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(fronius.gen24_plus_hybrid_inverter_virtual_meter));
            default_location = MeterLocation::Grid;
            return;

        case FroniusGEN24PlusHybridInverterVirtualMeter::Battery:
            table = &fronius_gen24_plus_hybrid_inverter_battery_type_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln("Unknown Fronius GEN24 Plus Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(fronius.gen24_plus_hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::HaileiHybridInverter:
        hailei.hybrid_inverter_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<HaileiHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (hailei.hybrid_inverter_virtual_meter) {
        case HaileiHybridInverterVirtualMeter::None:
            logger.printfln("No Hailei Hybrid Inverter Virtual Meter selected");
            return;

        case HaileiHybridInverterVirtualMeter::Inverter:
            table = &hailei_hybrid_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case HaileiHybridInverterVirtualMeter::Grid:
            table = &hailei_hybrid_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case HaileiHybridInverterVirtualMeter::Battery:
            table = &hailei_hybrid_inverter_battery_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln("Unknown Hailei Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(hailei.hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::FoxESSH3HybridInverter:
        fox_ess_h3.hybrid_inverter_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<FoxESSH3HybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (fox_ess_h3.hybrid_inverter_virtual_meter) {
        case FoxESSH3HybridInverterVirtualMeter::None:
            logger.printfln("No Fox ESS H3 Hybrid Inverter Virtual Meter selected");
            return;

        case FoxESSH3HybridInverterVirtualMeter::Inverter:
            table = &fox_ess_h3_hybrid_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case FoxESSH3HybridInverterVirtualMeter::Grid:
            table = &fox_ess_h3_hybrid_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case FoxESSH3HybridInverterVirtualMeter::Battery:
            table = &fox_ess_h3_hybrid_inverter_battery_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln("Unknown Fox ESS H3 Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(fox_ess_h3.hybrid_inverter_virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::SiemensPAC2200:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &siemens_pac2200_table;
        break;

    case MeterModbusTCPTableID::SiemensPAC3120:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &siemens_pac3120_and_pac3220_table;
        break;

    case MeterModbusTCPTableID::SiemensPAC3200:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &siemens_pac3200_table;
        break;

    case MeterModbusTCPTableID::SiemensPAC3220:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &siemens_pac3120_and_pac3220_table;
        break;

    case MeterModbusTCPTableID::SiemensPAC4200:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &siemens_pac4200_table;
        break;

    case MeterModbusTCPTableID::SiemensPAC4220:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &siemens_pac4220_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM24DIN:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 11));
        table = &carlo_gavazzi_em24_din_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM24E1:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &carlo_gavazzi_em24_e1_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM100:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        carlo_gavazzi_em100.phase = ephemeral_config->get("table")->get()->get("phase")->asEnum<CarloGavazziPhase>();
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));

        switch (carlo_gavazzi_em100.phase) {
        case CarloGavazziPhase::None:
            logger.printfln("No Carlo Gavazzi EM100 Phase selected");
            return;

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
            logger.printfln("Unknown Carlo Gavazzi EM100 Phase: %u", static_cast<uint8_t>(carlo_gavazzi_em100.phase));
            return;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziET100:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        carlo_gavazzi_et100.phase = ephemeral_config->get("table")->get()->get("phase")->asEnum<CarloGavazziPhase>();
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));

        switch (carlo_gavazzi_et100.phase) {
        case CarloGavazziPhase::None:
            logger.printfln("No Carlo Gavazzi ET100 Phase selected");
            return;

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
            logger.printfln("Unknown Carlo Gavazzi ET100 Phase: %u", static_cast<uint8_t>(carlo_gavazzi_et100.phase));
            return;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziEM210:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 61));
        table = &carlo_gavazzi_em210_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM270:
        carlo_gavazzi_em270.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<CarloGavazziEM270VirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 18));

        switch (carlo_gavazzi_em270.virtual_meter) {
        case CarloGavazziEM270VirtualMeter::None:
            logger.printfln("No Carlo Gavazzi EM270 Virtual Meter selected");
            return;

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
            logger.printfln("Unknown Carlo Gavazzi EM270 Virtual Meter: %u", static_cast<uint8_t>(carlo_gavazzi_em270.virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziEM280:
        carlo_gavazzi_em280.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<CarloGavazziEM280VirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 18));

        switch (carlo_gavazzi_em280.virtual_meter) {
        case CarloGavazziEM280VirtualMeter::None:
            logger.printfln("No Carlo Gavazzi EM280 Virtual Meter selected");
            return;

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
            logger.printfln("Unknown Carlo Gavazzi EM280 Virtual Meter: %u", static_cast<uint8_t>(carlo_gavazzi_em280.virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziEM300:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));
        table = &carlo_gavazzi_em300_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziET300:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));
        table = &carlo_gavazzi_et300_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM510:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        carlo_gavazzi_em510.phase = ephemeral_config->get("table")->get()->get("phase")->asEnum<CarloGavazziPhase>();
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));

        switch (carlo_gavazzi_em510.phase) {
        case CarloGavazziPhase::None:
            logger.printfln("No Carlo Gavazzi EM510 Phase selected");
            return;

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
            logger.printfln("Unknown Carlo Gavazzi EM510 Phase: %u", static_cast<uint8_t>(carlo_gavazzi_em510.phase));
            return;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziEM530:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &carlo_gavazzi_em530_and_em540_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM540:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &carlo_gavazzi_em530_and_em540_table;
        break;

    case MeterModbusTCPTableID::SolaredgeInverter:
        solaredge.inverter_virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SolaredgeInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (solaredge.inverter_virtual_meter) {
        case SolaredgeInverterVirtualMeter::None:
            logger.printfln("No Solaredge Inverter Virtual Meter selected");
            return;

        case SolaredgeInverterVirtualMeter::InverterUnused:
            logger.printfln("Invalid Solaredge Inverter Virtual Meter: %u", static_cast<uint8_t>(solaredge.inverter_virtual_meter));
            default_location = MeterLocation::Inverter;
            return;

        case SolaredgeInverterVirtualMeter::GridUnused:
            logger.printfln("Invalid Solaredge Inverter Virtual Meter: %u", static_cast<uint8_t>(solaredge.inverter_virtual_meter));
            default_location = MeterLocation::Grid;
            return;

        case SolaredgeInverterVirtualMeter::Battery:
            table = &solaredge_inverter_battery_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln("Unknown Solaredge Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(solaredge.inverter_virtual_meter));
            return;
        }

        break;

    default:
        logger.printfln("Unknown table: %u", static_cast<uint8_t>(table_id));
        return;
    }

    if (ephemeral_config->get("location")->asEnum<MeterLocation>() == MeterLocation::Unknown && default_location != MeterLocation::Unknown) {
        ephemeral_config->get("location")->updateEnum(default_location);
    }

    if (table->ids_length > 0) {
        meters.declare_value_ids(slot, table->ids, table->ids_length);
    }

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (read_allowed) {
            read_next();
        }
    }, 2_s, 1_s);
}

void MeterModbusTCP::register_events()
{
    if (table == nullptr) {
        return;
    }

    event.registerEvent("network/state", {"connected"}, [this](const Config *connected) {
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

    if (register_buffer_index < generic_read_request.register_count
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

bool MeterModbusTCP::is_sungrow_inverter_meter() const
{
    return (table_id == MeterModbusTCPTableID::SungrowHybridInverter
         && sungrow.hybrid_inverter_virtual_meter == SungrowHybridInverterVirtualMeter::Inverter)
        || (table_id == MeterModbusTCPTableID::SungrowStringInverter
         && sungrow.string_inverter_virtual_meter == SungrowStringInverterVirtualMeter::Inverter);
}

bool MeterModbusTCP::is_sungrow_grid_meter() const
{
    return (table_id == MeterModbusTCPTableID::SungrowHybridInverter
         && sungrow.hybrid_inverter_virtual_meter == SungrowHybridInverterVirtualMeter::Grid)
        || (table_id == MeterModbusTCPTableID::SungrowStringInverter
         && sungrow.string_inverter_virtual_meter == SungrowStringInverterVirtualMeter::Grid);
}

bool MeterModbusTCP::is_sungrow_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::SungrowHybridInverter
        && sungrow.hybrid_inverter_virtual_meter == SungrowHybridInverterVirtualMeter::Battery;
}

bool MeterModbusTCP::is_victron_energy_gx_inverter_meter() const
{
    return table_id == MeterModbusTCPTableID::VictronEnergyGX
        && victron_energy_gx.virtual_meter == VictronEnergyGXVirtualMeter::Inverter;
}

bool MeterModbusTCP::is_victron_energy_gx_load_meter() const
{
    return table_id == MeterModbusTCPTableID::VictronEnergyGX
        && victron_energy_gx.virtual_meter == VictronEnergyGXVirtualMeter::Load;
}

bool MeterModbusTCP::is_deye_hybrid_inverter_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::DeyeHybridInverter
        && deye.hybrid_inverter_virtual_meter == DeyeHybridInverterVirtualMeter::Battery;
}

bool MeterModbusTCP::is_deye_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::DeyeHybridInverter
        && deye.hybrid_inverter_virtual_meter == DeyeHybridInverterVirtualMeter::PV;
}

bool MeterModbusTCP::is_shelly_pro_xem_monophase() const
{
    return table_id == MeterModbusTCPTableID::ShellyProEM
        || (table_id == MeterModbusTCPTableID::ShellyPro3EM
            && shelly_pro_3em.device_profile == ShellyPro3EMDeviceProfile::Monophase);
}

bool MeterModbusTCP::is_fronius_gen24_plus_hybrid_inverter_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::FroniusGEN24PlusHybridInverter
        && fronius.gen24_plus_hybrid_inverter_virtual_meter == FroniusGEN24PlusHybridInverterVirtualMeter::Battery;
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
        && solaredge.inverter_virtual_meter == SolaredgeInverterVirtualMeter::Battery;
}

void MeterModbusTCP::read_done_callback()
{
    if (generic_read_request.result != TFModbusTCPClientTransactionResult::Success) {
        trace("m%u t%u a%zu:%x c%zu e%u",
              slot,
              static_cast<uint8_t>(table_id),
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
        logger.printfln("%s / %s has unsupported register count: %zu", get_meter_modbus_tcp_table_id_name(table_id), table->specs[read_index].name, register_count);
        return;
    }

    switch (value_type) {
    case ModbusValueType::None:
        trace("m%u t%u i%zu n a%zu:%x",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address);
        break;

    case ModbusValueType::U16:
        trace("m%u t%u i%zu u16 a%zu:%x r%u v%u",
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
        trace("m%u t%u i%zu s16 a%zu:%x r%u v%d",
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
        trace("m%u t%u i%zu u32%s a%zu:%x r%u,%u v%u",
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
        trace("m%u t%u i%zu s32%s a%zu:%x r%u,%u v%d",
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
        trace("m%u t%u i%zu f32%s a%zu:%x r%u,%u v%f",
              slot,
              static_cast<uint8_t>(table_id),
              read_index,
              endian,
              table->specs[read_index].start_address,
              table->specs[read_index].start_address,
              register_buffer[register_buffer_index + 0],
              register_buffer[register_buffer_index + 1],
              static_cast<double>(c32.f));

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
        if (!table->f32_negative_max_as_nan || c32.f != -FLT_MAX) { // Really compare exactly with -FLT_MAX
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
            value = c32.f;
        }

        break;

    case ModbusValueType::U64BE:
    case ModbusValueType::U64LE:
        trace("m%u t%u i%zu u64%s a%zu:%x r%u,%u,%u,%u v%llu",
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
        trace("m%u t%u i%zu s64%s a%zu:%x r%u,%u,%u,%u v%lld",
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
        trace("m%u t%u i%zu f64%s a%zu:%x r%u,%u,%u,%u v%f",
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

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfloat-equal"
#endif
    if (value != 0.0f) { // Really compare exactly with 0.0f
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
        // Don't convert 0.0f into -0.0f if the scale factor is negative
        value *= table->specs[read_index].scale_factor;
    }

    if (is_sungrow_inverter_meter()
     && generic_read_request.start_address == SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS) {
        if (sungrow.inverter_output_type < 0) {
            switch (c16.u) {
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
                table = nullptr;
                logger.printfln("%s has unknown Output Type: %u", get_meter_modbus_tcp_table_id_name(table_id), c16.u);
                return;
            }

            sungrow.inverter_output_type = c16.u;

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
        if (deye.hybrid_inverter_device_type < 0) {
            switch (c16.u) {
            case 0x0002:
            case 0x0003:
            case 0x0004:
                logger.printfln("%s has unsupported Device Type: 0x%04x", get_meter_modbus_tcp_table_id_name(table_id), c16.u);
                return;

            case 0x0005:
                table = &deye_hybrid_inverter_low_voltage_battery_table;
                break;

            case 0x0006:
            case 0x0106:
                table = &deye_hybrid_inverter_high_voltage_battery_table;
                break;

            default:
                table = nullptr;
                logger.printfln("%s has unknown Device Type: 0x%04x", get_meter_modbus_tcp_table_id_name(table_id), c16.u);
                return;
            }

            deye.hybrid_inverter_device_type = c16.u;

            meters.declare_value_ids(slot, table->ids, table->ids_length);
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

    if (is_fronius_gen24_plus_hybrid_inverter_battery_meter()
     && generic_read_request.start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_INPUT_OR_MODEL_ID_ADDRESS) {
        if (fronius.gen24_plus_hybrid_inverter_input_or_model_id == 0) {
            switch (c16.u) {
            case 1: // module/1/ID: Input ID
                table = &fronius_gen24_plus_hybrid_inverter_battery_integer_table;
                fronius.gen24_plus_hybrid_inverter_start_address_shift = 0;
                break;

            case 160: // ID: SunSpec Model ID
                table = &fronius_gen24_plus_hybrid_inverter_battery_float_table;
                fronius.gen24_plus_hybrid_inverter_start_address_shift = 10;
                break;

            default:
                table = nullptr;
                logger.printfln("%s has unknown input or model ID: %u", get_meter_modbus_tcp_table_id_name(table_id), c16.u);
                return;
            }

            fronius.gen24_plus_hybrid_inverter_input_or_model_id = c16.u;

            meters.declare_value_ids(slot, table->ids, table->ids_length);
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

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
        if (register_start_address == SUNGROW_STRING_INVERTER_TOTAL_ACTIVE_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], -value);
        }
    }
    else if (is_sungrow_battery_meter()) {
        if (register_start_address == SUNGROW_HYBRID_INVERTER_RUNNING_STATE_ADDRESS) {
            sungrow.hybrid_inverter_running_state = c16.u;
        }
        else if (register_start_address == SUNGROW_HYBRID_INVERTER_BATTERY_CURRENT_ADDRESS) {
            if ((sungrow.hybrid_inverter_running_state & (1 << 2)) != 0) {
                value = -value;
            }
        }
        else if (register_start_address == SUNGROW_HYBRID_INVERTER_BATTERY_POWER_ADDRESS) {
            if ((sungrow.hybrid_inverter_running_state & (1 << 2)) != 0) {
                value = -value;
            }
        }
    }
    else if (is_victron_energy_gx_inverter_meter()) {
        if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L1_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_on_output_l1_power = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L2_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_on_output_l2_power = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_ON_OUTPUT_L3_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_on_output_l3_power = value;

            float power = victron_energy_gx.ac_coupled_pv_on_output_l1_power
                        + victron_energy_gx.ac_coupled_pv_on_output_l2_power
                        + victron_energy_gx.ac_coupled_pv_on_output_l3_power;

            meters.update_value(slot, table->index[read_index + 1], -power);
        }
    }
    else if (is_victron_energy_gx_load_meter()) {
        if (register_start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L1_ADDRESS) {
            victron_energy_gx.ac_consumption_l1_power = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L2_ADDRESS) {
            victron_energy_gx.ac_consumption_l2_power = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L3_ADDRESS) {
            victron_energy_gx.ac_consumption_l3_power = value;

            float power = victron_energy_gx.ac_consumption_l1_power
                        + victron_energy_gx.ac_consumption_l2_power
                        + victron_energy_gx.ac_consumption_l3_power;

            meters.update_value(slot, table->index[read_index + 1], power);
        }
    }
    else if (is_deye_hybrid_inverter_pv_meter()) {
        if (register_start_address == DEYE_HYBRID_INVERTER_PV1_POWER_ADDRESS) {
            deye.hybrid_inverter_pv1_power = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV2_POWER_ADDRESS) {
            deye.hybrid_inverter_pv2_power = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV3_POWER_ADDRESS) {
            deye.hybrid_inverter_pv3_power = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV4_POWER_ADDRESS) {
            deye.hybrid_inverter_pv4_power = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS) {
            deye.hybrid_inverter_pv1_voltage = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV1_CURRENT_ADDRESS) {
            deye.hybrid_inverter_pv1_current = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS) {
            deye.hybrid_inverter_pv2_voltage = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV2_CURRENT_ADDRESS) {
            deye.hybrid_inverter_pv2_current = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS) {
            deye.hybrid_inverter_pv3_voltage = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV3_CURRENT_ADDRESS) {
            deye.hybrid_inverter_pv3_current = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS) {
            deye.hybrid_inverter_pv4_voltage = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV4_CURRENT_ADDRESS) {
            deye.hybrid_inverter_pv4_current = value;

            float power = deye.hybrid_inverter_pv1_power
                        + deye.hybrid_inverter_pv2_power
                        + deye.hybrid_inverter_pv3_power
                        + deye.hybrid_inverter_pv4_power;

            float voltage = (deye.hybrid_inverter_pv1_voltage
                          +  deye.hybrid_inverter_pv2_voltage
                          +  deye.hybrid_inverter_pv3_voltage
                          +  deye.hybrid_inverter_pv4_voltage) / 4; // FIXME: how to handle unused strings?

            float current = deye.hybrid_inverter_pv1_current
                          + deye.hybrid_inverter_pv2_current
                          + deye.hybrid_inverter_pv3_current
                          + deye.hybrid_inverter_pv4_current;

            meters.update_value(slot, table->index[read_index + 1], power);
            meters.update_value(slot, table->index[read_index + 2], -power);
            meters.update_value(slot, table->index[read_index + 3], voltage);
            meters.update_value(slot, table->index[read_index + 4], current);
        }
    }
    else if (is_shelly_pro_xem_monophase()) {
        if (register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_ACTIVE_POWER
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_TOTAL_ACTIVE_ENERGY
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_TOTAL_ACTIVE_RETURNED_ENERGY
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_ACTIVE_POWER
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_TOTAL_ACTIVE_ENERGY
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_TOTAL_ACTIVE_RETURNED_ENERGY
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_ACTIVE_POWER
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_TOTAL_ACTIVE_ENERGY
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_TOTAL_ACTIVE_RETURNED_ENERGY) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_fronius_gen24_plus_hybrid_inverter_battery_meter()) {
        size_t start_address = register_start_address - fronius.gen24_plus_hybrid_inverter_start_address_shift;

        if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DCA_SF_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_dca_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DCV_SF_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_dcv_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DCW_SF_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_dcw_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DCWH_SF_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_dcwh_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHARGE_DCA_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_charge_dca = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHARGE_DCV_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_charge_dcv = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHARGE_DCW_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_charge_dcw = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHARGE_DCWH_ADDRESS) {
            // Is 0 in firmware versions <= 1.30 while discharging. As this is an acc32 map 0 to NaN. This will make the
            // meters framework ignore this value during discharging and keep the pervious value
            fronius.gen24_plus_hybrid_inverter_charge_dcwh = c32.u == 0 ? NAN : value; // SunSpec: acc32
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DISCHARGE_DCA_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_discharge_dca = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DISCHARGE_DCV_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_discharge_dcv = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DISCHARGE_DCW_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_discharge_dcw = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_DISCHARGE_DCWH_ADDRESS) {
            // Is 0 in firmware versions <= 1.30 while charging. As this is an acc32 map 0 to NaN. This will make the
            // meters framework ignore this value during charging and keep the pervious value
            fronius.gen24_plus_hybrid_inverter_discharge_dcwh = c32.u == 0 ? NAN : value; // SunSpec: acc32
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHASTATE_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_chastate = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_HYBRID_INVERTER_CHASTATE_SF_ADDRESS) {
            fronius.gen24_plus_hybrid_inverter_chastate_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf

            float dca_scale_factor = get_fronius_scale_factor(fronius.gen24_plus_hybrid_inverter_dca_sf);
            float dcv_scale_factor = get_fronius_scale_factor(fronius.gen24_plus_hybrid_inverter_dcv_sf);
            float dcw_scale_factor = get_fronius_scale_factor(fronius.gen24_plus_hybrid_inverter_dcw_sf);
            float dcwh_scale_factor = get_fronius_scale_factor(fronius.gen24_plus_hybrid_inverter_dcwh_sf);
            float chastate_scale_factor = get_fronius_scale_factor(fronius.gen24_plus_hybrid_inverter_chastate_sf);

            float charge_dca = fronius.gen24_plus_hybrid_inverter_charge_dca * dca_scale_factor;
            float charge_dcv = fronius.gen24_plus_hybrid_inverter_charge_dcv * dcv_scale_factor;
            float charge_dcw = fronius.gen24_plus_hybrid_inverter_charge_dcw * dcw_scale_factor;

            float discharge_dca = fronius.gen24_plus_hybrid_inverter_discharge_dca * dca_scale_factor;
            float discharge_dcv = fronius.gen24_plus_hybrid_inverter_discharge_dcv * dcv_scale_factor;
            float discharge_dcw = fronius.gen24_plus_hybrid_inverter_discharge_dcw * dcw_scale_factor;

            float current_charge_discharge_diff = charge_dca - discharge_dca; // One of the two value is always 0A
            float voltage = std::max(charge_dcv, discharge_dcv); // In firmware versions <= 1.30 one of the two values is always 0V. In firmware versions >= 1.31 they are the same
            float power_charge_discharge_diff = charge_dcw - discharge_dcw; // One of the two value is always 0W
            float state_of_charge = fronius.gen24_plus_hybrid_inverter_chastate * chastate_scale_factor;
            float energy_charge = fronius.gen24_plus_hybrid_inverter_charge_dcwh * dcwh_scale_factor * 0.001f;
            float energy_discharge = fronius.gen24_plus_hybrid_inverter_discharge_dcwh * dcwh_scale_factor * 0.001f;

            meters.update_value(slot, table->index[read_index + 1], current_charge_discharge_diff);
            meters.update_value(slot, table->index[read_index + 2], voltage);
            meters.update_value(slot, table->index[read_index + 3], power_charge_discharge_diff);
            meters.update_value(slot, table->index[read_index + 4], state_of_charge);
            meters.update_value(slot, table->index[read_index + 5], energy_charge);
            meters.update_value(slot, table->index[read_index + 6], energy_discharge);
        }
    }
    else if (is_carlo_gavazzi_em100_or_et100()) {
        if (register_start_address == CARLO_GAVAZZI_EM100_OR_ET100_W
         || register_start_address == CARLO_GAVAZZI_EM100_OR_ET100_KWH_POSITIVE_TOTAL
         || register_start_address == CARLO_GAVAZZI_EM100_OR_ET100_KWH_NEGATIVE_TOTAL) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_carlo_gavazzi_em510()) {
        if (register_start_address == CARLO_GAVAZZI_EM510_W
         || register_start_address == CARLO_GAVAZZI_EM510_KWH_POSITIVE_TOTAL
         || register_start_address == CARLO_GAVAZZI_EM510_KWH_NEGATIVE_TOTAL) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_solaredge_inverter_battery_meter()) {
        if (register_start_address == SOLAREDGE_INVERTER_BATTERY_1_TEMPERATURE) {
            solaredge.battery_1_temperature = value;
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_1_VOLTAGE) {
            solaredge.battery_1_voltage = value;
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_1_CURRENT) {
            solaredge.battery_1_current = -value; // current is negative while charging
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_1_POWER) {
            solaredge.battery_1_power = value;
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_1_EXPORT_ENERGY) {
            solaredge.battery_1_export_energy = value;
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_1_IMPORT_ENERGY) {
            solaredge.battery_1_import_energy = value;
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_1_STATE_OF_CHARGE) {
            solaredge.battery_1_state_of_charge = value;
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_2_TEMPERATURE) {
            value = nan_safe_sum(solaredge.battery_1_temperature, value);
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_2_VOLTAGE) {
            value = nan_safe_avg(solaredge.battery_1_voltage, value);
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_2_CURRENT) {
            value = nan_safe_sum(solaredge.battery_1_current, -value); // current is negative while charging
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_2_POWER) {
            value = nan_safe_sum(solaredge.battery_1_power, value);
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_2_EXPORT_ENERGY) {
            value = nan_safe_sum(solaredge.battery_1_export_energy, value);
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_2_IMPORT_ENERGY) {
            value = nan_safe_sum(solaredge.battery_1_import_energy, value);
        }
        else if (register_start_address == SOLAREDGE_INVERTER_BATTERY_2_STATE_OF_CHARGE) {
            value = nan_safe_avg(solaredge.battery_1_state_of_charge, value);
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
