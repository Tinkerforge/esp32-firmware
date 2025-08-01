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
#include "tools/float.h"
#include "tools/sun_spec.h"

#include "gcc_warnings.h"

//#define DEBUG_VALUES_TO_TRACE_LOG

#define NUMBER_TO_ADDRESS(number) ((number) - 1u)

#define SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS                               static_cast<size_t>(SungrowInverterOutputTypeAddress::OutputType)

#define SUNGROW_HYBRID_INVERTER_GRID_FREQUENCY_ADDRESS                     static_cast<size_t>(SungrowHybridInverterGridAddress::GridFrequency)

#define SUNGROW_STRING_INVERTER_GRID_FREQUENCY_ADDRESS                     static_cast<size_t>(SungrowStringInverterGridAddress::GridFrequency)

#define SUNGROW_HYBRID_INVERTER_TOTAL_ACTIVE_POWER_ADDRESS                 static_cast<size_t>(SungrowHybridInverter1P2LAddress::TotalActivePower)

#define SUNGROW_STRING_INVERTER_TOTAL_ACTIVE_POWER_ADDRESS                 static_cast<size_t>(SungrowStringInverter1P2LAddress::TotalActivePower)

#define SUNGROW_HYBRID_INVERTER_RUNNING_STATE_ADDRESS                      static_cast<size_t>(SungrowHybridInverterBatteryAddress::RunningState)
#define SUNGROW_HYBRID_INVERTER_BATTERY_CURRENT_ADDRESS                    static_cast<size_t>(SungrowHybridInverterBatteryAddress::BatteryCurrent)
#define SUNGROW_HYBRID_INVERTER_BATTERY_POWER_ADDRESS                      static_cast<size_t>(SungrowHybridInverterBatteryAddress::BatteryPower)

#define SUNGROW_HYBRID_INVERTER_MPPT_1_VOLTAGE_ADDRESS                     static_cast<size_t>(SungrowHybridInverterPVAddress::MPPT1Voltage)
#define SUNGROW_HYBRID_INVERTER_MPPT_1_CURRENT_ADDRESS                     static_cast<size_t>(SungrowHybridInverterPVAddress::MPPT1Current)
#define SUNGROW_HYBRID_INVERTER_MPPT_2_VOLTAGE_ADDRESS                     static_cast<size_t>(SungrowHybridInverterPVAddress::MPPT2Voltage)
#define SUNGROW_HYBRID_INVERTER_MPPT_2_CURRENT_ADDRESS                     static_cast<size_t>(SungrowHybridInverterPVAddress::MPPT2Current)
#define SUNGROW_HYBRID_INVERTER_TOTAL_DC_POWER_ADDRESS                     static_cast<size_t>(SungrowHybridInverterPVAddress::TotalDCPower)

#define SUNGROW_STRING_INVERTER_MPPT_1_VOLTAGE_ADDRESS                     static_cast<size_t>(SungrowStringInverterPVAddress::MPPT1Voltage)
#define SUNGROW_STRING_INVERTER_MPPT_1_CURRENT_ADDRESS                     static_cast<size_t>(SungrowStringInverterPVAddress::MPPT1Current)
#define SUNGROW_STRING_INVERTER_MPPT_2_VOLTAGE_ADDRESS                     static_cast<size_t>(SungrowStringInverterPVAddress::MPPT2Voltage)
#define SUNGROW_STRING_INVERTER_MPPT_2_CURRENT_ADDRESS                     static_cast<size_t>(SungrowStringInverterPVAddress::MPPT2Current)
#define SUNGROW_STRING_INVERTER_TOTAL_DC_POWER_ADDRESS                     static_cast<size_t>(SungrowStringInverterPVAddress::TotalDCPower)

#define VICTRON_ENERGY_GX_AC_CONSUMPTION_L1_ADDRESS                        static_cast<size_t>(VictronEnergyGXLoadAddress::ACConsumptionL1)
#define VICTRON_ENERGY_GX_AC_CONSUMPTION_L2_ADDRESS                        static_cast<size_t>(VictronEnergyGXLoadAddress::ACConsumptionL2)
#define VICTRON_ENERGY_GX_AC_CONSUMPTION_L3_ADDRESS                        static_cast<size_t>(VictronEnergyGXLoadAddress::ACConsumptionL3)

#define VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_OUTPUT_L1_ADDRESS         static_cast<size_t>(VictronEnergyGXPVAddress::ACCoupledPVPowerOnOutputL1)
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_OUTPUT_L2_ADDRESS         static_cast<size_t>(VictronEnergyGXPVAddress::ACCoupledPVPowerOnOutputL2)
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_OUTPUT_L3_ADDRESS         static_cast<size_t>(VictronEnergyGXPVAddress::ACCoupledPVPowerOnOutputL3)
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_INPUT_L1_ADDRESS          static_cast<size_t>(VictronEnergyGXPVAddress::ACCoupledPVPowerOnInputL1)
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_INPUT_L2_ADDRESS          static_cast<size_t>(VictronEnergyGXPVAddress::ACCoupledPVPowerOnInputL2)
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_INPUT_L3_ADDRESS          static_cast<size_t>(VictronEnergyGXPVAddress::ACCoupledPVPowerOnInputL3)
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_GENERATOR_L1_ADDRESS      static_cast<size_t>(VictronEnergyGXPVAddress::ACCoupledPVPowerOnGeneratorL1)
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_GENERATOR_L2_ADDRESS      static_cast<size_t>(VictronEnergyGXPVAddress::ACCoupledPVPowerOnGeneratorL2)
#define VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_GENERATOR_L3_ADDRESS      static_cast<size_t>(VictronEnergyGXPVAddress::ACCoupledPVPowerOnGeneratorL3)
#define VICTRON_ENERGY_GX_DC_COUPLED_PV_POWER_ADDRESS                      static_cast<size_t>(VictronEnergyGXPVAddress::DCCoupledPVPower)

#define DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS                           static_cast<size_t>(DeyeHybridInverterDeviceTypeAddress::DeviceType)

#define DEYE_HYBRID_INVERTER_PV1_POWER_ADDRESS                             static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV1Power)
#define DEYE_HYBRID_INVERTER_PV2_POWER_ADDRESS                             static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV2Power)
#define DEYE_HYBRID_INVERTER_PV3_POWER_ADDRESS                             static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV3Power)
#define DEYE_HYBRID_INVERTER_PV4_POWER_ADDRESS                             static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV4Power)
#define DEYE_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS                           static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV1Voltage)
#define DEYE_HYBRID_INVERTER_PV1_CURRENT_ADDRESS                           static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV1Current)
#define DEYE_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS                           static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV2Voltage)
#define DEYE_HYBRID_INVERTER_PV2_CURRENT_ADDRESS                           static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV2Current)
#define DEYE_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS                           static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV3Voltage)
#define DEYE_HYBRID_INVERTER_PV3_CURRENT_ADDRESS                           static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV3Current)
#define DEYE_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS                           static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV4Voltage)
#define DEYE_HYBRID_INVERTER_PV4_CURRENT_ADDRESS                           static_cast<size_t>(DeyeHybridInverterLowVoltagePVAddress::PV4Current)

#define ALPHA_ESS_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV1Voltage)
#define ALPHA_ESS_HYBRID_INVERTER_PV1_CURRENT_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV1Current)
#define ALPHA_ESS_HYBRID_INVERTER_PV1_POWER_ADDRESS                        static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV1Power)
#define ALPHA_ESS_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV2Voltage)
#define ALPHA_ESS_HYBRID_INVERTER_PV2_CURRENT_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV2Current)
#define ALPHA_ESS_HYBRID_INVERTER_PV2_POWER_ADDRESS                        static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV2Power)
#define ALPHA_ESS_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV3Voltage)
#define ALPHA_ESS_HYBRID_INVERTER_PV3_CURRENT_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV3Current)
#define ALPHA_ESS_HYBRID_INVERTER_PV3_POWER_ADDRESS                        static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV3Power)
#define ALPHA_ESS_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV4Voltage)
#define ALPHA_ESS_HYBRID_INVERTER_PV4_CURRENT_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV4Current)
#define ALPHA_ESS_HYBRID_INVERTER_PV4_POWER_ADDRESS                        static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV4Power)
#define ALPHA_ESS_HYBRID_INVERTER_PV5_VOLTAGE_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV5Voltage)
#define ALPHA_ESS_HYBRID_INVERTER_PV5_CURRENT_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV5Current)
#define ALPHA_ESS_HYBRID_INVERTER_PV5_POWER_ADDRESS                        static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV5Power)
#define ALPHA_ESS_HYBRID_INVERTER_PV6_VOLTAGE_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV6Voltage)
#define ALPHA_ESS_HYBRID_INVERTER_PV6_CURRENT_ADDRESS                      static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV6Current)
#define ALPHA_ESS_HYBRID_INVERTER_PV6_POWER_ADDRESS                        static_cast<size_t>(AlphaESSHybridInverterPVAddress::PV6Power)

#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_ACTIVE_POWER_ADDRESS                    static_cast<size_t>(ShellyEMMonophaseChannel1AsL1Address::Channel1ActivePower)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_TOTAL_ACTIVE_ENERGY_ADDRESS             static_cast<size_t>(ShellyEMMonophaseChannel1AsL1Address::Channel1TotalActiveEnergyPerpetualCount)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_TOTAL_ACTIVE_RETURNED_ENERGY_ADDRESS    static_cast<size_t>(ShellyEMMonophaseChannel1AsL1Address::Channel1TotalActiveReturnedEnergyPerpetualCount)

#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_ACTIVE_POWER_ADDRESS                    static_cast<size_t>(ShellyEMMonophaseChannel2AsL1Address::Channel2ActivePower)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_TOTAL_ACTIVE_ENERGY_ADDRESS             static_cast<size_t>(ShellyEMMonophaseChannel2AsL1Address::Channel2TotalActiveEnergyPerpetualCount)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_TOTAL_ACTIVE_RETURNED_ENERGY_ADDRESS    static_cast<size_t>(ShellyEMMonophaseChannel2AsL1Address::Channel2TotalActiveReturnedEnergyPerpetualCount)

#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_ACTIVE_POWER_ADDRESS                    static_cast<size_t>(ShellyEMMonophaseChannel3AsL1Address::Channel3ActivePower)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_TOTAL_ACTIVE_ENERGY_ADDRESS             static_cast<size_t>(ShellyEMMonophaseChannel3AsL1Address::Channel3TotalActiveEnergyPerpetualCount)
#define SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_TOTAL_ACTIVE_RETURNED_ENERGY_ADDRESS    static_cast<size_t>(ShellyEMMonophaseChannel3AsL1Address::Channel3TotalActiveReturnedEnergyPerpetualCount)

#define GOODWE_HYBRID_INVERTER_BATTERY_1_MODE_ADDRESS                      static_cast<size_t>(GoodweHybridInverterBatteryModesAddress::Battery1Mode)
#define GOODWE_HYBRID_INVERTER_BATTERY_2_MODE_ADDRESS                      static_cast<size_t>(GoodweHybridInverterBatteryModesAddress::Battery2Mode)

#define GOODWE_HYBRID_INVERTER_BATTERY_1_VOLTAGE_ADDRESS                   static_cast<size_t>(GoodweHybridInverterBattery1And2Address::Battery1Voltage)
#define GOODWE_HYBRID_INVERTER_BATTERY_1_CURRENT_ADDRESS                   static_cast<size_t>(GoodweHybridInverterBattery1And2Address::Battery1Current)
#define GOODWE_HYBRID_INVERTER_BATTERY_1_POWER_ADDRESS                     static_cast<size_t>(GoodweHybridInverterBattery1And2Address::Battery1Power)
#define GOODWE_HYBRID_INVERTER_BMS_1_PACK_TEMPERATURE_ADDRESS              static_cast<size_t>(GoodweHybridInverterBattery1And2Address::BMS1PackTemperature)
#define GOODWE_HYBRID_INVERTER_BATTERY_1_CAPACITY_ADDRESS                  static_cast<size_t>(GoodweHybridInverterBattery1And2Address::Battery1Capacity)
#define GOODWE_HYBRID_INVERTER_BATTERY_2_VOLTAGE_ADDRESS                   static_cast<size_t>(GoodweHybridInverterBattery1And2Address::Battery2Voltage)
#define GOODWE_HYBRID_INVERTER_BATTERY_2_CURRENT_ADDRESS                   static_cast<size_t>(GoodweHybridInverterBattery1And2Address::Battery2Current)
#define GOODWE_HYBRID_INVERTER_BATTERY_2_POWER_ADDRESS                     static_cast<size_t>(GoodweHybridInverterBattery1And2Address::Battery2Power)
#define GOODWE_HYBRID_INVERTER_BMS_2_PACK_TEMPERATURE_ADDRESS              static_cast<size_t>(GoodweHybridInverterBattery1And2Address::BMS2PackTemperature)
#define GOODWE_HYBRID_INVERTER_BATTERY_2_CAPACITY_ADDRESS                  static_cast<size_t>(GoodweHybridInverterBattery1And2Address::Battery2Capacity)

#define GOODWE_HYBRID_INVERTER_PV_MODE_ADDRESS                             static_cast<size_t>(GoodweHybridInverterPVAddress::PVMode)
#define GOODWE_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS                         static_cast<size_t>(GoodweHybridInverterPVAddress::PV1Voltage)
#define GOODWE_HYBRID_INVERTER_PV1_CURRENT_ADDRESS                         static_cast<size_t>(GoodweHybridInverterPVAddress::PV1Current)
#define GOODWE_HYBRID_INVERTER_PV1_POWER_ADDRESS                           static_cast<size_t>(GoodweHybridInverterPVAddress::PV1Power)
#define GOODWE_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS                         static_cast<size_t>(GoodweHybridInverterPVAddress::PV2Voltage)
#define GOODWE_HYBRID_INVERTER_PV2_CURRENT_ADDRESS                         static_cast<size_t>(GoodweHybridInverterPVAddress::PV2Current)
#define GOODWE_HYBRID_INVERTER_PV2_POWER_ADDRESS                           static_cast<size_t>(GoodweHybridInverterPVAddress::PV2Power)
#define GOODWE_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS                         static_cast<size_t>(GoodweHybridInverterPVAddress::PV3Voltage)
#define GOODWE_HYBRID_INVERTER_PV3_CURRENT_ADDRESS                         static_cast<size_t>(GoodweHybridInverterPVAddress::PV3Current)
#define GOODWE_HYBRID_INVERTER_PV3_POWER_ADDRESS                           static_cast<size_t>(GoodweHybridInverterPVAddress::PV3Power)
#define GOODWE_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS                         static_cast<size_t>(GoodweHybridInverterPVAddress::PV4Voltage)
#define GOODWE_HYBRID_INVERTER_PV4_CURRENT_ADDRESS                         static_cast<size_t>(GoodweHybridInverterPVAddress::PV4Current)
#define GOODWE_HYBRID_INVERTER_PV4_POWER_ADDRESS                           static_cast<size_t>(GoodweHybridInverterPVAddress::PV4Power)

#define SOLAX_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS                          static_cast<size_t>(SolaxHybridInverterPVAddress::PV1Voltage)
#define SOLAX_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS                          static_cast<size_t>(SolaxHybridInverterPVAddress::PV2Voltage)
#define SOLAX_HYBRID_INVERTER_PV1_CURRENT_ADDRESS                          static_cast<size_t>(SolaxHybridInverterPVAddress::PV1Current)
#define SOLAX_HYBRID_INVERTER_PV2_CURRENT_ADDRESS                          static_cast<size_t>(SolaxHybridInverterPVAddress::PV2Current)
#define SOLAX_HYBRID_INVERTER_PV1_POWER_ADDRESS                            static_cast<size_t>(SolaxHybridInverterPVAddress::PV1Power)
#define SOLAX_HYBRID_INVERTER_PV2_POWER_ADDRESS                            static_cast<size_t>(SolaxHybridInverterPVAddress::PV2Power)

#define FRONIUS_GEN24_PLUS_INPUT_ID_OR_MODEL_ID_ADDRESS                    static_cast<size_t>(FroniusGEN24PlusBatteryTypeAddress::InputIDOrModelID)
#define FRONIUS_GEN24_PLUS_DCA_SF_ADDRESS                                  static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::DCA_SF)
#define FRONIUS_GEN24_PLUS_DCV_SF_ADDRESS                                  static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::DCV_SF)
#define FRONIUS_GEN24_PLUS_DCW_SF_ADDRESS                                  static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::DCW_SF)
#define FRONIUS_GEN24_PLUS_DCWH_SF_ADDRESS                                 static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::DCWH_SF)
#define FRONIUS_GEN24_PLUS_CHARGE_DCA_ADDRESS                              static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::ChargeDCA)
#define FRONIUS_GEN24_PLUS_CHARGE_DCV_ADDRESS                              static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::ChargeDCV)
#define FRONIUS_GEN24_PLUS_CHARGE_DCW_ADDRESS                              static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::ChargeDCW)
#define FRONIUS_GEN24_PLUS_CHARGE_DCWH_ADDRESS                             static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::ChargeDCWH)
#define FRONIUS_GEN24_PLUS_DISCHARGE_DCA_ADDRESS                           static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::DischargeDCA)
#define FRONIUS_GEN24_PLUS_DISCHARGE_DCV_ADDRESS                           static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::DischargeDCV)
#define FRONIUS_GEN24_PLUS_DISCHARGE_DCW_ADDRESS                           static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::DischargeDCW)
#define FRONIUS_GEN24_PLUS_DISCHARGE_DCWH_ADDRESS                          static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::DischargeDCWH)
#define FRONIUS_GEN24_PLUS_CHASTATE_ADDRESS                                static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::ChaState)
#define FRONIUS_GEN24_PLUS_CHASTATE_SF_ADDRESS                             static_cast<size_t>(FroniusGEN24PlusBatteryIntegerAddress::ChaState_SF)

#define HAILEI_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV1Voltage)
#define HAILEI_HYBRID_INVERTER_PV1_CURRENT_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV1Current)
#define HAILEI_HYBRID_INVERTER_PV1_POWER_ADDRESS                           static_cast<size_t>(HaileiHybridInverterPVAddress::PV1Power)
#define HAILEI_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV2Voltage)
#define HAILEI_HYBRID_INVERTER_PV2_CURRENT_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV2Current)
#define HAILEI_HYBRID_INVERTER_PV2_POWER_ADDRESS                           static_cast<size_t>(HaileiHybridInverterPVAddress::PV2Power)
#define HAILEI_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV3Voltage)
#define HAILEI_HYBRID_INVERTER_PV3_CURRENT_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV3Current)
#define HAILEI_HYBRID_INVERTER_PV3_POWER_ADDRESS                           static_cast<size_t>(HaileiHybridInverterPVAddress::PV3Power)
#define HAILEI_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV4Voltage)
#define HAILEI_HYBRID_INVERTER_PV4_CURRENT_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV4Current)
#define HAILEI_HYBRID_INVERTER_PV4_POWER_ADDRESS                           static_cast<size_t>(HaileiHybridInverterPVAddress::PV4Power)
#define HAILEI_HYBRID_INVERTER_PV5_VOLTAGE_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV5Voltage)
#define HAILEI_HYBRID_INVERTER_PV5_CURRENT_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV5Current)
#define HAILEI_HYBRID_INVERTER_PV5_POWER_ADDRESS                           static_cast<size_t>(HaileiHybridInverterPVAddress::PV5Power)
#define HAILEI_HYBRID_INVERTER_PV6_VOLTAGE_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV6Voltage)
#define HAILEI_HYBRID_INVERTER_PV6_CURRENT_ADDRESS                         static_cast<size_t>(HaileiHybridInverterPVAddress::PV6Current)
#define HAILEI_HYBRID_INVERTER_PV6_POWER_ADDRESS                           static_cast<size_t>(HaileiHybridInverterPVAddress::PV6Power)

#define FOX_ESS_H3_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS                     static_cast<size_t>(FoxESSH3HybridInverterPVAddress::PV1Voltage)
#define FOX_ESS_H3_HYBRID_INVERTER_PV1_CURRENT_ADDRESS                     static_cast<size_t>(FoxESSH3HybridInverterPVAddress::PV1Current)
#define FOX_ESS_H3_HYBRID_INVERTER_PV1_POWER_ADDRESS                       static_cast<size_t>(FoxESSH3HybridInverterPVAddress::PV1Power)
#define FOX_ESS_H3_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS                     static_cast<size_t>(FoxESSH3HybridInverterPVAddress::PV2Voltage)
#define FOX_ESS_H3_HYBRID_INVERTER_PV2_CURRENT_ADDRESS                     static_cast<size_t>(FoxESSH3HybridInverterPVAddress::PV2Current)
#define FOX_ESS_H3_HYBRID_INVERTER_PV2_POWER_ADDRESS                       static_cast<size_t>(FoxESSH3HybridInverterPVAddress::PV2Power)

#define CARLO_GAVAZZI_EM100_OR_ET100_W_ADDRESS                             static_cast<size_t>(CarloGavazziEM100andET100AtL1Address::W)
#define CARLO_GAVAZZI_EM100_OR_ET100_KWH_POSITIVE_TOTAL_ADDRESS            static_cast<size_t>(CarloGavazziEM100andET100AtL1Address::KWhPositiveTotal)
#define CARLO_GAVAZZI_EM100_OR_ET100_KWH_NEGATIVE_TOTAL_ADDRESS            static_cast<size_t>(CarloGavazziEM100andET100AtL1Address::KWhNegativeTotal)

#define CARLO_GAVAZZI_EM510_W_ADDRESS                                      static_cast<size_t>(CarloGavazziEM510atL1Address::W)
#define CARLO_GAVAZZI_EM510_KWH_POSITIVE_TOTAL_ADDRESS                     static_cast<size_t>(CarloGavazziEM510atL1Address::KWhPositiveTotal)
#define CARLO_GAVAZZI_EM510_KWH_NEGATIVE_TOTAL_ADDRESS                     static_cast<size_t>(CarloGavazziEM510atL1Address::KWhNegativeTotal)

#define SOLAREDGE_BATTERY_1_TEMPERATURE_ADDRESS                            static_cast<size_t>(SolaredgeBatteryAddress::Battery1AverageTemperature)
#define SOLAREDGE_BATTERY_1_VOLTAGE_ADDRESS                                static_cast<size_t>(SolaredgeBatteryAddress::Battery1InstantaneousVoltage)
#define SOLAREDGE_BATTERY_1_CURRENT_ADDRESS                                static_cast<size_t>(SolaredgeBatteryAddress::Battery1InstantaneousCurrent)
#define SOLAREDGE_BATTERY_1_POWER_ADDRESS                                  static_cast<size_t>(SolaredgeBatteryAddress::Battery1InstantaneousPower)
#define SOLAREDGE_BATTERY_1_EXPORT_ENERGY_ADDRESS                          static_cast<size_t>(SolaredgeBatteryAddress::Battery1LifetimeExportEnergyCounter)
#define SOLAREDGE_BATTERY_1_IMPORT_ENERGY_ADDRESS                          static_cast<size_t>(SolaredgeBatteryAddress::Battery1LifetimeImportEnergyCounter)
#define SOLAREDGE_BATTERY_1_STATE_OF_CHARGE_ADDRESS                        static_cast<size_t>(SolaredgeBatteryAddress::Battery1StateOfEnergy)
#define SOLAREDGE_BATTERY_2_TEMPERATURE_ADDRESS                            static_cast<size_t>(SolaredgeBatteryAddress::Battery2AverageTemperature)
#define SOLAREDGE_BATTERY_2_VOLTAGE_ADDRESS                                static_cast<size_t>(SolaredgeBatteryAddress::Battery2InstantaneousVoltage)
#define SOLAREDGE_BATTERY_2_CURRENT_ADDRESS                                static_cast<size_t>(SolaredgeBatteryAddress::Battery2InstantaneousCurrent)
#define SOLAREDGE_BATTERY_2_POWER_ADDRESS                                  static_cast<size_t>(SolaredgeBatteryAddress::Battery2InstantaneousPower)
#define SOLAREDGE_BATTERY_2_EXPORT_ENERGY_ADDRESS                          static_cast<size_t>(SolaredgeBatteryAddress::Battery2LifetimeExportEnergyCounter)
#define SOLAREDGE_BATTERY_2_IMPORT_ENERGY_ADDRESS                          static_cast<size_t>(SolaredgeBatteryAddress::Battery2LifetimeImportEnergyCounter)
#define SOLAREDGE_BATTERY_2_STATE_OF_CHARGE_ADDRESS                        static_cast<size_t>(SolaredgeBatteryAddress::Battery2StateOfEnergy)

#define E3DC_PV_POWER_ADDRESS                                              static_cast<size_t>(E3DCPVAddress::PVPower)

#define HUAWEI_SUN2000_ENERGY_STORAGE_PRODUCT_MODEL_ADDRESS                static_cast<size_t>(HuaweiSUN2000BatteryProductModelAddress::EnergyStorageProductModel)

#define HUAWEI_SUN2000_NUMBER_OF_PV_STRINGS_ADDRESS                        static_cast<size_t>(HuaweiSUN2000PVStringCountAddress::NumberOfPVStrings)

#define HUAWEI_SUN2000_INPUT_POWER_ADDRESS                                 static_cast<size_t>(HuaweiSUN2000PVNoStringsAddress::InputPower)

#define HUAWEI_SUN2000_SMART_DONGLE_INPUT_POWER_ADDRESS                    static_cast<size_t>(HuaweiSUN2000SmartDonglePVAddress::InputPower)

#define HUAWEI_EMMA_LOAD_POWER_ADDRESS                                     static_cast<size_t>(HuaweiEMMALoadAddress::LoadPower)

#define HUAWEI_EMMA_PV_OUTPUT_POWER_ADDRESS                                static_cast<size_t>(HuaweiEMMAPVAddress::PVOutputPower)

#define SOLAX_STRING_INVERTER_OUTPUT_POWER_ADDRESS                         static_cast<size_t>(SolaxStringInverterAddress::OutputPower)

#define SOLAX_STRING_INVERTER_PV1_VOLTAGE_ADDRESS                          static_cast<size_t>(SolaxStringInverterPVAddress::PV1Voltage)
#define SOLAX_STRING_INVERTER_PV2_VOLTAGE_ADDRESS                          static_cast<size_t>(SolaxStringInverterPVAddress::PV2Voltage)
#define SOLAX_STRING_INVERTER_PV1_CURRENT_ADDRESS                          static_cast<size_t>(SolaxStringInverterPVAddress::PV1Current)
#define SOLAX_STRING_INVERTER_PV2_CURRENT_ADDRESS                          static_cast<size_t>(SolaxStringInverterPVAddress::PV2Current)
#define SOLAX_STRING_INVERTER_PV1_POWER_ADDRESS                            static_cast<size_t>(SolaxStringInverterPVAddress::PV1Power)
#define SOLAX_STRING_INVERTER_PV2_POWER_ADDRESS                            static_cast<size_t>(SolaxStringInverterPVAddress::PV2Power)
#define SOLAX_STRING_INVERTER_PV3_VOLTAGE_ADDRESS                          static_cast<size_t>(SolaxStringInverterPVAddress::PV3Voltage)
#define SOLAX_STRING_INVERTER_PV3_CURRENT_ADDRESS                          static_cast<size_t>(SolaxStringInverterPVAddress::PV3Current)
#define SOLAX_STRING_INVERTER_PV3_POWER_ADDRESS                            static_cast<size_t>(SolaxStringInverterPVAddress::PV3Power)

#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS               static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV1Voltage)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV1_CURRENT_ADDRESS               static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV1Current)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV1_POWER_ADDRESS                 static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV1Power)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS               static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV2Voltage)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV2_CURRENT_ADDRESS               static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV2Current)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV2_POWER_ADDRESS                 static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV2Power)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS               static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV3Voltage)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV3_CURRENT_ADDRESS               static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV3Current)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV3_POWER_ADDRESS                 static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV3Power)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS               static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV4Voltage)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV4_CURRENT_ADDRESS               static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV4Current)
#define FOX_ESS_H3_SMART_HYBRID_INVERTER_PV4_POWER_ADDRESS                 static_cast<size_t>(FoxESSH3SmartHybridInverterPVAddress::PV4Power)

#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV1Voltage)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV1_CURRENT_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV1Current)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV1_POWER_ADDRESS                   static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV1Power)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV2Voltage)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV2_CURRENT_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV2Current)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV2_POWER_ADDRESS                   static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV2Power)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV3Voltage)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV3_CURRENT_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV3Current)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV3_POWER_ADDRESS                   static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV3Power)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV4Voltage)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV4_CURRENT_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV4Current)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV4_POWER_ADDRESS                   static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV4Power)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV5_VOLTAGE_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV5Voltage)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV5_CURRENT_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV5Current)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV5_POWER_ADDRESS                   static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV5Power)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV6_VOLTAGE_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV6Voltage)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV6_CURRENT_ADDRESS                 static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV6Current)
#define FOX_ESS_H3_PRO_HYBRID_INVERTER_PV6_POWER_ADDRESS                   static_cast<size_t>(FoxESSH3ProHybridInverterPVAddress::PV6Power)

#define SMA_HYBRID_INVERTER_BATTERY_CHARGE_POWER_ADDRESS                   static_cast<size_t>(SMAHybridInverterBatteryAddress::BatChrgCurBatCha)
#define SMA_HYBRID_INVERTER_BATTERY_DISCHARGE_POWER_ADDRESS                static_cast<size_t>(SMAHybridInverterBatteryAddress::BatDschCurBatDsch)

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

    MeterLocation default_location = MeterLocation::Unknown;

    switch (table_id) {
    case MeterModbusTCPTableID::None:
        logger.printfln_meter("No table selected");
        break;

    case MeterModbusTCPTableID::Custom: {
            device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

            const Config *registers = static_cast<const Config *>(ephemeral_config->get("table")->get()->get("registers"));
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

            // FIXME: leaking this, because as of right now meter instances don't get destroyed
            custom_table->specs = custom_specs;
            custom_table->specs_length = registers_count;
            custom_table->ids = custom_ids;
            custom_table->ids_length = registers_count;
            custom_table->index = custom_index;

            table = custom_table;
        }

        break;

    case MeterModbusTCPTableID::SungrowHybridInverter:
        sungrow_hybrid_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SungrowHybridInverterVirtualMeter>();
        sungrow_inverter_output_type = -1;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (sungrow_hybrid_inverter.virtual_meter) {
        case SungrowHybridInverterVirtualMeter::None:
            logger.printfln_meter("No Sungrow Hybrid Inverter Virtual Meter selected");
            break;

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

        case SungrowHybridInverterVirtualMeter::PV:
            table = &sungrow_hybrid_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Sungrow Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(sungrow_hybrid_inverter.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::SungrowStringInverter:
        sungrow_string_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SungrowStringInverterVirtualMeter>();
        sungrow_inverter_output_type = -1;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (sungrow_string_inverter.virtual_meter) {
        case SungrowStringInverterVirtualMeter::None:
            logger.printfln_meter("No Sungrow String Inverter Virtual Meter selected");
            break;

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

        case SungrowStringInverterVirtualMeter::PV:
            table = &sungrow_string_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Sungrow String Inverter Virtual Meter: %u", static_cast<uint8_t>(sungrow_string_inverter.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::SolarmaxMaxStorage:
        solarmax_max_storage.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SolarmaxMaxStorageVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (solarmax_max_storage.virtual_meter) {
        case SolarmaxMaxStorageVirtualMeter::None:
            logger.printfln_meter("No Solarmax Max Storage Virtual Meter selected");
            break;

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
            logger.printfln_meter("Unknown Solarmax Max Storage Virtual Meter: %u", static_cast<uint8_t>(solarmax_max_storage.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::VictronEnergyGX:
        victron_energy_gx.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<VictronEnergyGXVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (victron_energy_gx.virtual_meter) {
        case VictronEnergyGXVirtualMeter::None:
            logger.printfln_meter("No Victron Energy GX Virtual Meter selected");
            break;

        case VictronEnergyGXVirtualMeter::InverterUnused:
            logger.printfln_meter("Invalid Victron Energy GX Virtual Meter: %u", static_cast<uint8_t>(victron_energy_gx.virtual_meter));
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

        case VictronEnergyGXVirtualMeter::PV:
            table = &victron_energy_gx_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Victron Energy GX Virtual Meter: %u", static_cast<uint8_t>(victron_energy_gx.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::DeyeHybridInverter:
        deye_hybrid_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<DeyeHybridInverterVirtualMeter>();
        deye_hybrid_inverter.device_type = -1;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (deye_hybrid_inverter.virtual_meter) {
        case DeyeHybridInverterVirtualMeter::None:
            logger.printfln_meter("No Deye Hybrid Inverter Virtual Meter selected");
            break;

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
            table = &deye_hybrid_inverter_device_type_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Deye Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(deye_hybrid_inverter.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::AlphaESSHybridInverter:
        alpha_ess_hybrid_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<AlphaESSHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (alpha_ess_hybrid_inverter.virtual_meter) {
        case AlphaESSHybridInverterVirtualMeter::None:
            logger.printfln_meter("No Alpha ESS Hybrid Inverter Virtual Meter selected");
            break;

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

        case AlphaESSHybridInverterVirtualMeter::PV:
            table = &alpha_ess_hybrid_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Alpha ESS Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(alpha_ess_hybrid_inverter.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::ShellyProEM:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        shelly_pro_em.monophase_channel = ephemeral_config->get("table")->get()->get("monophase_channel")->asEnum<ShellyEMMonophaseChannel>();
        shelly_pro_em.monophase_mapping = ephemeral_config->get("table")->get()->get("monophase_mapping")->asEnum<ShellyEMMonophaseMapping>();

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
        goodwe_hybrid_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<GoodweHybridInverterVirtualMeter>();
        goodwe_hybrid_inverter.battery_1_mode = -1;
        goodwe_hybrid_inverter.battery_2_mode = -1;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (goodwe_hybrid_inverter.virtual_meter) {
        case GoodweHybridInverterVirtualMeter::None:
            logger.printfln_meter("No Goodwe Hybrid Inverter Virtual Meter selected");
            break;

        case GoodweHybridInverterVirtualMeter::Inverter:
            table = &goodwe_hybrid_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case GoodweHybridInverterVirtualMeter::Grid:
            table = &goodwe_hybrid_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case GoodweHybridInverterVirtualMeter::Battery:
            table = &goodwe_hybrid_inverter_battery_modes_table;
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

        case GoodweHybridInverterVirtualMeter::PV:
            table = &goodwe_hybrid_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Goodwe Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(goodwe_hybrid_inverter.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::SolaxHybridInverter:
        solax_hybrid_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SolaxHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (solax_hybrid_inverter.virtual_meter) {
        case SolaxHybridInverterVirtualMeter::None:
            logger.printfln_meter("No Solax Hybrid Inverter Virtual Meter selected");
            break;

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

        case SolaxHybridInverterVirtualMeter::PV:
            table = &solax_hybrid_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Solax Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(solax_hybrid_inverter.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::FroniusGEN24Plus:
        fronius_gen24_plus.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<FroniusGEN24PlusVirtualMeter>();
        fronius_gen24_plus.input_id_or_model_id = -1;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (fronius_gen24_plus.virtual_meter) {
        case FroniusGEN24PlusVirtualMeter::None:
            logger.printfln_meter("No Fronius GEN24 Plus Virtual Meter selected");
            break;

        case FroniusGEN24PlusVirtualMeter::InverterUnused:
            logger.printfln_meter("Invalid Fronius GEN24 Plus Virtual Meter: %u", static_cast<uint8_t>(fronius_gen24_plus.virtual_meter));
            default_location = MeterLocation::Inverter;
            break;

        case FroniusGEN24PlusVirtualMeter::GridUnused:
            logger.printfln_meter("Invalid Fronius GEN24 Plus Virtual Meter: %u", static_cast<uint8_t>(fronius_gen24_plus.virtual_meter));
            default_location = MeterLocation::Grid;
            break;

        case FroniusGEN24PlusVirtualMeter::Battery:
            table = &fronius_gen24_plus_battery_type_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln_meter("Unknown Fronius GEN24 Plus Virtual Meter: %u", static_cast<uint8_t>(fronius_gen24_plus.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::HaileiHybridInverter:
        hailei_hybrid_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<HaileiHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (hailei_hybrid_inverter.virtual_meter) {
        case HaileiHybridInverterVirtualMeter::None:
            logger.printfln_meter("No Hailei Hybrid Inverter Virtual Meter selected");
            break;

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

        case HaileiHybridInverterVirtualMeter::PV:
            table = &hailei_hybrid_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Hailei Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(hailei_hybrid_inverter.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::FoxESSH3HybridInverter:
        fox_ess_h3_hybrid_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<FoxESSH3HybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (fox_ess_h3_hybrid_inverter.virtual_meter) {
        case FoxESSH3HybridInverterVirtualMeter::None:
            logger.printfln_meter("No Fox ESS H3 Hybrid Inverter Virtual Meter selected");
            break;

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

        case FoxESSH3HybridInverterVirtualMeter::Load:
            table = &fox_ess_h3_hybrid_inverter_load_table;
            default_location = MeterLocation::Load;
            break;

        case FoxESSH3HybridInverterVirtualMeter::PV:
            table = &fox_ess_h3_hybrid_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Fox ESS H3 Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(fox_ess_h3_hybrid_inverter.virtual_meter));
            break;
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
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        carlo_gavazzi_et100.phase = ephemeral_config->get("table")->get()->get("phase")->asEnum<CarloGavazziPhase>();
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
        carlo_gavazzi_em280.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<CarloGavazziEM280VirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
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
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &carlo_gavazzi_em530_and_em540_table;
        break;

    case MeterModbusTCPTableID::CarloGavazziEM540:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &carlo_gavazzi_em530_and_em540_table;
        break;

    case MeterModbusTCPTableID::Solaredge:
        solaredge.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SolaredgeVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (solaredge.virtual_meter) {
        case SolaredgeVirtualMeter::None:
            logger.printfln_meter("No Solaredge Virtual Meter selected");
            break;

        case SolaredgeVirtualMeter::InverterUnused:
            logger.printfln_meter("Invalid Solaredge Virtual Meter: %u", static_cast<uint8_t>(solaredge.virtual_meter));
            default_location = MeterLocation::Inverter;
            break;

        case SolaredgeVirtualMeter::GridUnused:
            logger.printfln_meter("Invalid Solaredge Virtual Meter: %u", static_cast<uint8_t>(solaredge.virtual_meter));
            default_location = MeterLocation::Grid;
            break;

        case SolaredgeVirtualMeter::Battery:
            table = &solaredge_battery_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln_meter("Unknown Solaredge Hybrid Virtual Meter: %u", static_cast<uint8_t>(solaredge.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::EastronSDM630TCP:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        max_register_count = static_cast<size_t>(std::min(METER_MODBUS_TCP_REGISTER_BUFFER_SIZE, 50));
        table = &eastron_sdm630_tcp_table;
        break;

    case MeterModbusTCPTableID::TinkerforgeWARPCharger:
        device_address = 1;
        table = &tinkerforge_warp_charger_table;
        default_location = MeterLocation::Load;
        break;

    case MeterModbusTCPTableID::SAXPowerHomeBasicMode:
        sax_power.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SAXPowerVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (sax_power.virtual_meter) {
        case SAXPowerVirtualMeter::None:
            logger.printfln_meter("No SAX Power Home Basic Mode Virtual Meter selected");
            break;

        case SAXPowerVirtualMeter::InverterUnused:
            logger.printfln_meter("Invalid SAX Power Home Basic Mode Virtual Meter: %u", static_cast<uint8_t>(sax_power.virtual_meter));
            default_location = MeterLocation::Inverter;
            break;

        case SAXPowerVirtualMeter::Grid:
            table = &sax_power_home_basic_mode_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case SAXPowerVirtualMeter::Battery:
            table = &sax_power_home_basic_mode_battery_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln_meter("Unknown SAX Power Home Basic Mode Virtual Meter: %u", static_cast<uint8_t>(sax_power.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::SAXPowerHomeExtendedMode:
        sax_power.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SAXPowerVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (sax_power.virtual_meter) {
        case SAXPowerVirtualMeter::None:
            logger.printfln_meter("No SAX Power Home Extended Mode Virtual Meter selected");
            break;

        case SAXPowerVirtualMeter::InverterUnused:
            logger.printfln_meter("Invalid SAX Power Home Extended Mode Virtual Meter: %u", static_cast<uint8_t>(sax_power.virtual_meter));
            default_location = MeterLocation::Inverter;
            break;

        case SAXPowerVirtualMeter::Grid:
            table = &sax_power_home_extended_mode_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case SAXPowerVirtualMeter::Battery:
            table = &sax_power_home_extended_mode_battery_table;
            default_location = MeterLocation::Battery;
            break;

        default:
            logger.printfln_meter("Unknown SAX Power Home Extended Mode Virtual Meter: %u", static_cast<uint8_t>(sax_power.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::E3DC:
        e3dc.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<E3DCVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (e3dc.virtual_meter) {
        case E3DCVirtualMeter::None:
            logger.printfln_meter("No E3/DC Virtual Meter selected");
            break;

        case E3DCVirtualMeter::InverterUnused:
            logger.printfln_meter("Invalid E3/DC Virtual Meter: %u", static_cast<uint8_t>(e3dc.virtual_meter));
            default_location = MeterLocation::Inverter;
            break;

        case E3DCVirtualMeter::Grid:
            table = &e3dc_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case E3DCVirtualMeter::Battery:
            table = &e3dc_battery_table;
            default_location = MeterLocation::Battery;
            break;

        case E3DCVirtualMeter::Load:
            table = &e3dc_load_table;
            default_location = MeterLocation::Load;
            break;

        case E3DCVirtualMeter::PV:
            table = &e3dc_pv_table;
            default_location = MeterLocation::PV;
            break;

        case E3DCVirtualMeter::AdditionalGeneration:
            table = &e3dc_additional_generation_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown E3/DC Virtual Meter: %u", static_cast<uint8_t>(e3dc.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::HuaweiSUN2000:
        huawei_sun2000.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<HuaweiSUN2000VirtualMeter>();
        huawei_sun2000.energy_storage_product_model = -1;
        huawei_sun2000.number_of_pv_strings = -1;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (huawei_sun2000.virtual_meter) {
        case HuaweiSUN2000VirtualMeter::None:
            logger.printfln_meter("No Huawei SUN2000 Virtual Meter selected");
            break;

        case HuaweiSUN2000VirtualMeter::Inverter:
            table = &huawei_sun2000_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case HuaweiSUN2000VirtualMeter::Grid:
            table = &huawei_sun2000_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case HuaweiSUN2000VirtualMeter::Battery:
            table = &huawei_sun2000_battery_product_model_table;
            default_location = MeterLocation::Battery;
            break;

        case HuaweiSUN2000VirtualMeter::PV:
            table = &huawei_sun2000_pv_string_count_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Huawei SUN2000 Virtual Meter: %u", static_cast<uint8_t>(huawei_sun2000.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::HuaweiSUN2000SmartDongle:
        huawei_sun2000_smart_dongle.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<HuaweiSUN2000SmartDongleVirtualMeter>();
        huawei_sun2000_smart_dongle.energy_storage_product_model = -1;
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (huawei_sun2000_smart_dongle.virtual_meter) {
        case HuaweiSUN2000SmartDongleVirtualMeter::None:
            logger.printfln_meter("No Huawei SUN2000 Smart Dongle Virtual Meter selected");
            break;

        case HuaweiSUN2000SmartDongleVirtualMeter::InverterUnused:
            logger.printfln_meter("Invalid Huawei SUN2000 Smart Dongle Virtual Meter: %u", static_cast<uint8_t>(huawei_sun2000_smart_dongle.virtual_meter));
            default_location = MeterLocation::Inverter;
            break;

        case HuaweiSUN2000SmartDongleVirtualMeter::Grid:
            table = &huawei_sun2000_smart_dongle_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case HuaweiSUN2000SmartDongleVirtualMeter::Battery:
            table = &huawei_sun2000_battery_product_model_table;
            default_location = MeterLocation::Battery;
            break;

        case HuaweiSUN2000SmartDongleVirtualMeter::PV:
            table = &huawei_sun2000_smart_dongle_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Huawei SUN2000 Smart Dongle Virtual Meter: %u", static_cast<uint8_t>(huawei_sun2000_smart_dongle.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::HuaweiEMMA:
        huawei_emma.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<HuaweiEMMAVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (huawei_emma.virtual_meter) {
        case HuaweiEMMAVirtualMeter::None:
            logger.printfln_meter("No Huawei EMMA Virtual Meter selected");
            break;

        case HuaweiEMMAVirtualMeter::Inverter:
            table = &huawei_emma_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case HuaweiEMMAVirtualMeter::GridInternalSensor:
            table = &huawei_emma_grid_internal_sensor_table;
            default_location = MeterLocation::Grid;
            break;

        case HuaweiEMMAVirtualMeter::Battery:
            table = &huawei_emma_battery_table;
            default_location = MeterLocation::Battery;
            break;

        case HuaweiEMMAVirtualMeter::Load:
            table = &huawei_emma_load_table;
            default_location = MeterLocation::Load;
            break;

        case HuaweiEMMAVirtualMeter::PV:
            table = &huawei_emma_pv_table;
            default_location = MeterLocation::PV;
            break;

        case HuaweiEMMAVirtualMeter::GridExternalSensor:
            table = &huawei_emma_grid_external_sensor_table;
            default_location = MeterLocation::Grid;
            break;

        default:
            logger.printfln_meter("Unknown Huawei EMMA Virtual Meter: %u", static_cast<uint8_t>(huawei_emma.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::CarloGavazziEM580:
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());
        table = &carlo_gavazzi_em580_table;
        break;

    case MeterModbusTCPTableID::SolaxStringInverter:
        solax_string_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SolaxStringInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (solax_string_inverter.virtual_meter) {
        case SolaxStringInverterVirtualMeter::None:
            logger.printfln_meter("No Solax String Inverter Virtual Meter selected");
            break;

        case SolaxStringInverterVirtualMeter::Inverter:
            table = &solax_string_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case SolaxStringInverterVirtualMeter::Grid:
            table = &solax_string_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case SolaxStringInverterVirtualMeter::PV:
            table = &solax_string_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Solax String Inverter Virtual Meter: %u", static_cast<uint8_t>(solax_string_inverter.virtual_meter));
            break;
        }

        break;

    case MeterModbusTCPTableID::FoxESSH3SmartHybridInverter:
        fox_ess_h3_smart_hybrid_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<FoxESSH3SmartHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (fox_ess_h3_smart_hybrid_inverter.virtual_meter) {
        case FoxESSH3SmartHybridInverterVirtualMeter::None:
            logger.printfln_meter("No Fox ESS H3 Smart Hybrid Inverter Virtual Meter selected");
            return;

        case FoxESSH3SmartHybridInverterVirtualMeter::Inverter:
            table = &fox_ess_h3_smart_hybrid_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case FoxESSH3SmartHybridInverterVirtualMeter::Grid:
            table = &fox_ess_h3_smart_hybrid_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case FoxESSH3SmartHybridInverterVirtualMeter::Battery:
            table = &fox_ess_h3_smart_hybrid_inverter_battery_table;
            default_location = MeterLocation::Battery;
            break;

        case FoxESSH3SmartHybridInverterVirtualMeter::Load:
            table = &fox_ess_h3_smart_hybrid_inverter_load_table;
            default_location = MeterLocation::Load;
            break;

        case FoxESSH3SmartHybridInverterVirtualMeter::PV:
            table = &fox_ess_h3_smart_hybrid_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Fox ESS H3 Smart Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(fox_ess_h3_smart_hybrid_inverter.virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::FoxESSH3ProHybridInverter:
        fox_ess_h3_pro_hybrid_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<FoxESSH3ProHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (fox_ess_h3_pro_hybrid_inverter.virtual_meter) {
        case FoxESSH3ProHybridInverterVirtualMeter::None:
            logger.printfln_meter("No Fox ESS H3 Pro Hybrid Inverter Virtual Meter selected");
            return;

        case FoxESSH3ProHybridInverterVirtualMeter::Inverter:
            table = &fox_ess_h3_pro_hybrid_inverter_table;
            default_location = MeterLocation::Inverter;
            break;

        case FoxESSH3ProHybridInverterVirtualMeter::Grid:
            table = &fox_ess_h3_pro_hybrid_inverter_grid_table;
            default_location = MeterLocation::Grid;
            break;

        case FoxESSH3ProHybridInverterVirtualMeter::Battery:
            table = &fox_ess_h3_pro_hybrid_inverter_battery_table;
            default_location = MeterLocation::Battery;
            break;

        case FoxESSH3ProHybridInverterVirtualMeter::Load:
            table = &fox_ess_h3_pro_hybrid_inverter_load_table;
            default_location = MeterLocation::Load;
            break;

        case FoxESSH3ProHybridInverterVirtualMeter::PV:
            table = &fox_ess_h3_pro_hybrid_inverter_pv_table;
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown Fox ESS H3 Pro Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(fox_ess_h3_pro_hybrid_inverter.virtual_meter));
            return;
        }

        break;

    case MeterModbusTCPTableID::SMAHybridInverter:
        sma_hybrid_inverter.virtual_meter = ephemeral_config->get("table")->get()->get("virtual_meter")->asEnum<SMAHybridInverterVirtualMeter>();
        device_address = static_cast<uint8_t>(ephemeral_config->get("table")->get()->get("device_address")->asUint());

        switch (sma_hybrid_inverter.virtual_meter) {
        case SMAHybridInverterVirtualMeter::None:
            logger.printfln_meter("No SMA Hybrid Inverter Virtual Meter selected");
            return;

        case SMAHybridInverterVirtualMeter::InverterUnused:
            logger.printfln_meter("Invalid SMA Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(sma_hybrid_inverter.virtual_meter));
            default_location = MeterLocation::Inverter;
            break;

        case SMAHybridInverterVirtualMeter::GridUnused:
            logger.printfln_meter("Invalid SMA Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(sma_hybrid_inverter.virtual_meter));
            default_location = MeterLocation::Grid;
            break;

        case SMAHybridInverterVirtualMeter::Battery:
            table = &sma_hybrid_inverter_battery_table;
            default_location = MeterLocation::Battery;
            break;

        case SMAHybridInverterVirtualMeter::LoadUnused:
            logger.printfln_meter("Invalid SMA Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(sma_hybrid_inverter.virtual_meter));
            default_location = MeterLocation::Load;
            break;

        case SMAHybridInverterVirtualMeter::PVUnused:
            logger.printfln_meter("Invalid SMA Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(sma_hybrid_inverter.virtual_meter));
            default_location = MeterLocation::PV;
            break;

        default:
            logger.printfln_meter("Unknown SMA Hybrid Inverter Virtual Meter: %u", static_cast<uint8_t>(sma_hybrid_inverter.virtual_meter));
            return;
        }

        break;

    default:
        logger.printfln_meter("Unknown table: %u", static_cast<uint8_t>(table_id));
        break;
    }

    if (table == nullptr) {
        return;
    }

    if (ephemeral_config->get("location")->asEnum<MeterLocation>() == MeterLocation::Unknown && default_location != MeterLocation::Unknown) {
        ephemeral_config->get("location")->updateEnum(default_location);
    }

    if (table->ids_length > 0) {
        meters.declare_value_ids(slot, table->ids, table->ids_length);
    }

    if (table->specs_length > 0) {
        task_scheduler.scheduleWithFixedDelay([this]() {
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

bool MeterModbusTCP::is_fox_ess_h3_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3HybridInverter
        && fox_ess_h3_hybrid_inverter.virtual_meter == FoxESSH3HybridInverterVirtualMeter::PV;
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

bool MeterModbusTCP::is_solaredge_battery_meter() const
{
    return table_id == MeterModbusTCPTableID::Solaredge
        && solaredge.virtual_meter == SolaredgeVirtualMeter::Battery;
}

bool MeterModbusTCP::is_e3dc_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::E3DC
        && e3dc.virtual_meter == E3DCVirtualMeter::PV;
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

bool MeterModbusTCP::is_huawei_sun2000_smart_dongle_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::HuaweiSUN2000SmartDongle
        && huawei_sun2000_smart_dongle.virtual_meter == HuaweiSUN2000SmartDongleVirtualMeter::PV;
}

bool MeterModbusTCP::is_huawei_emma_load_meter() const
{
    return table_id == MeterModbusTCPTableID::HuaweiEMMA
        && huawei_emma.virtual_meter == HuaweiEMMAVirtualMeter::Load;
}

bool MeterModbusTCP::is_huawei_emma_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::HuaweiEMMA
        && huawei_emma.virtual_meter == HuaweiEMMAVirtualMeter::PV;
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

bool MeterModbusTCP::is_fox_ess_h3_smart_hybrid_inverter_pv_meter() const
{
    return table_id == MeterModbusTCPTableID::FoxESSH3SmartHybridInverter
        && fox_ess_h3_smart_hybrid_inverter.virtual_meter == FoxESSH3SmartHybridInverterVirtualMeter::PV;
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

    if ((is_sungrow_hybrid_inverter_meter() || is_sungrow_string_inverter_meter())
     && generic_read_request.start_address == SUNGROW_INVERTER_OUTPUT_TYPE_ADDRESS) {
        if (sungrow_inverter_output_type < 0) {
            bool success = true;

            switch (c16.u) {
            case 0:
                if (table_id == MeterModbusTCPTableID::SungrowHybridInverter) {
                    table = &sungrow_hybrid_inverter_1p2l_table;
                }
                else { // MeterModbusTCPTableID::SungrowStringInverter
                    table = &sungrow_string_inverter_1p2l_table;
                }

                logger.printfln_meter("Sungrow 1P2L inverter detected");
                break;

            case 1:
                if (table_id == MeterModbusTCPTableID::SungrowHybridInverter) {
                    table = &sungrow_hybrid_inverter_3p4l_table;
                }
                else { // MeterModbusTCPTableID::SungrowStringInverter
                    table = &sungrow_string_inverter_3p4l_table;
                }

                logger.printfln_meter("Sungrow 3P4L inverter detected");
                break;

            case 2:
                if (table_id == MeterModbusTCPTableID::SungrowHybridInverter) {
                    table = &sungrow_hybrid_inverter_3p3l_table;
                }
                else { // MeterModbusTCPTableID::SungrowStringInverter
                    table = &sungrow_string_inverter_3p3l_table;
                }

                logger.printfln_meter("Sungrow 3P3L inverter detected");
                break;

            default:
                success = false;
                logger.printfln_meter("Sungrow inverter has unknown output type: %u", c16.u);
                break;
            }

            if (success) {
                sungrow_inverter_output_type = c16.u;

                meters.declare_value_ids(slot, table->ids, table->ids_length);
            }
        }

        read_allowed = true;
        read_index = 0;
        register_buffer_index = METER_MODBUS_TCP_REGISTER_BUFFER_SIZE;

        prepare_read();
        return;
    }

    if ((is_deye_hybrid_inverter_battery_meter() || is_deye_hybrid_inverter_pv_meter())
     && generic_read_request.start_address == DEYE_HYBRID_INVERTER_DEVICE_TYPE_ADDRESS) {
        if (deye_hybrid_inverter.device_type < 0) {
            bool success = true;

            switch (c16.u) {
            case 0x0002:
            case 0x0003:
            case 0x0004:
                logger.printfln_meter("Deye hybrid inverter has unsupported device type: 0x%04x", c16.u);
                return;

            case 0x0005:
                if (is_deye_hybrid_inverter_battery_meter()) {
                    table = &deye_hybrid_inverter_low_voltage_battery_table;
                }
                else {
                    table = &deye_hybrid_inverter_low_voltage_pv_table;
                }

                logger.printfln_meter("Deye hybrid inverter with low-voltage battery detected");
                break;

            case 0x0006:
            case 0x0106:
                if (is_deye_hybrid_inverter_battery_meter()) {
                    table = &deye_hybrid_inverter_high_voltage_battery_table;
                }
                else {
                    table = &deye_hybrid_inverter_high_voltage_pv_table;
                }

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
        if (generic_read_request.start_address == GOODWE_HYBRID_INVERTER_BATTERY_1_MODE_ADDRESS) {
            if (goodwe_hybrid_inverter.battery_1_mode < 0) {
                goodwe_hybrid_inverter.battery_1_mode = c16.u;
            }
        }
        else if (generic_read_request.start_address == GOODWE_HYBRID_INVERTER_BATTERY_2_MODE_ADDRESS) {
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
     && generic_read_request.start_address == FRONIUS_GEN24_PLUS_INPUT_ID_OR_MODEL_ID_ADDRESS) {
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
     && generic_read_request.start_address == HUAWEI_SUN2000_ENERGY_STORAGE_PRODUCT_MODEL_ADDRESS) {
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
     && generic_read_request.start_address == HUAWEI_SUN2000_NUMBER_OF_PV_STRINGS_ADDRESS) {
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
     && generic_read_request.start_address == HUAWEI_SUN2000_ENERGY_STORAGE_PRODUCT_MODEL_ADDRESS) {
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
        if (register_start_address == SUNGROW_HYBRID_INVERTER_TOTAL_ACTIVE_POWER_ADDRESS
         || register_start_address == SUNGROW_STRING_INVERTER_TOTAL_ACTIVE_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], zero_safe_negation(value));
        }
    }
    else if (is_sungrow_hybrid_inverter_grid_meter() || is_sungrow_string_inverter_grid_meter()) {
        if (register_start_address == SUNGROW_HYBRID_INVERTER_GRID_FREQUENCY_ADDRESS
         || register_start_address == SUNGROW_STRING_INVERTER_GRID_FREQUENCY_ADDRESS) {
            if (value > 100) {
                // according to the spec the grid frequency is given
                // as 0.1 Hz, but some inverters report it as 0.01 Hz
                value /= 10;
            }
        }
    }
    else if (is_sungrow_hybrid_inverter_battery_meter()) {
        if (register_start_address == SUNGROW_HYBRID_INVERTER_RUNNING_STATE_ADDRESS) {
            sungrow_hybrid_inverter.running_state = c16.u;
        }
        else if (register_start_address == SUNGROW_HYBRID_INVERTER_BATTERY_CURRENT_ADDRESS
              || register_start_address == SUNGROW_HYBRID_INVERTER_BATTERY_POWER_ADDRESS) {
            // it seems that from october 2024 the running-state register stays at 0 and
            // the current and power values are reported signed already
            //
            // https://github.com/evcc-io/evcc/issues/18270
            // https://github.com/mkaiser/Sungrow-SHx-Inverter-Modbus-Home-Assistant/blob/a3b82db9a3b33bdda5c35108578d4c7685f82f7d/modbus_sungrow.yaml#L1281
            if ((sungrow_hybrid_inverter.running_state & (1 << 2)) != 0) {
                // discharging flag is set, assume unsigned value, but fabs anyway to be sure
                value = zero_safe_negation(fabs(value));
            }
        }
    }
    else if (is_sungrow_hybrid_inverter_pv_meter()) {
        if (register_start_address == SUNGROW_HYBRID_INVERTER_MPPT_1_VOLTAGE_ADDRESS) {
            sungrow_hybrid_inverter.mppt1_voltage = value;
        }
        else if (register_start_address == SUNGROW_HYBRID_INVERTER_MPPT_1_CURRENT_ADDRESS) {
            sungrow_hybrid_inverter.mppt1_current = value;
        }
        else if (register_start_address == SUNGROW_HYBRID_INVERTER_MPPT_2_VOLTAGE_ADDRESS) {
            sungrow_hybrid_inverter.mppt2_voltage = value;
        }
        else if (register_start_address == SUNGROW_HYBRID_INVERTER_MPPT_2_CURRENT_ADDRESS) {
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
        else if (register_start_address == SUNGROW_HYBRID_INVERTER_TOTAL_DC_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], zero_safe_negation(value));
        }
    }
    else if (is_sungrow_string_inverter_pv_meter()) {
        if (register_start_address == SUNGROW_STRING_INVERTER_MPPT_1_VOLTAGE_ADDRESS) {
            sungrow_string_inverter.mppt1_voltage = value;
        }
        else if (register_start_address == SUNGROW_STRING_INVERTER_MPPT_1_CURRENT_ADDRESS) {
            sungrow_string_inverter.mppt1_current = value;
        }
        else if (register_start_address == SUNGROW_STRING_INVERTER_MPPT_2_VOLTAGE_ADDRESS) {
            sungrow_string_inverter.mppt2_voltage = value;
        }
        else if (register_start_address == SUNGROW_STRING_INVERTER_MPPT_2_CURRENT_ADDRESS) {
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
        else if (register_start_address == SUNGROW_STRING_INVERTER_TOTAL_DC_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], zero_safe_negation(value));
        }
    }
    else if (is_victron_energy_gx_load_meter()) {
        if (register_start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L1_ADDRESS) {
            victron_energy_gx.ac_consumption_l1 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L2_ADDRESS) {
            victron_energy_gx.ac_consumption_l2 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_CONSUMPTION_L3_ADDRESS) {
            victron_energy_gx.ac_consumption_l3 = value;

            float power_sum = victron_energy_gx.ac_consumption_l1
                            + victron_energy_gx.ac_consumption_l2
                            + victron_energy_gx.ac_consumption_l3;

            meters.update_value(slot, table->index[read_index + 1], power_sum);
            meters.update_value(slot, table->index[read_index + 2], power_sum);
        }
    }
    else if (is_victron_energy_gx_pv_meter()) {
        if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_OUTPUT_L1_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_power_on_output_l1 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_OUTPUT_L2_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_power_on_output_l2 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_OUTPUT_L3_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_power_on_output_l3 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_INPUT_L1_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_power_on_input_l1 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_INPUT_L2_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_power_on_input_l2 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_INPUT_L3_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_power_on_input_l3 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_GENERATOR_L1_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_power_on_generator_l1 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_GENERATOR_L2_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_power_on_generator_l2 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_AC_COUPLED_PV_POWER_ON_GENERATOR_L3_ADDRESS) {
            victron_energy_gx.ac_coupled_pv_power_on_generator_l3 = value;
        }
        else if (register_start_address == VICTRON_ENERGY_GX_DC_COUPLED_PV_POWER_ADDRESS) {
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
            meters.update_value(slot, table->index[read_index + 2], zero_safe_negation(power_sum));
        }
    }
    else if (is_deye_hybrid_inverter_pv_meter()) {
        if (register_start_address == DEYE_HYBRID_INVERTER_PV1_POWER_ADDRESS) {
            deye_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV2_POWER_ADDRESS) {
            deye_hybrid_inverter.pv2_power = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV3_POWER_ADDRESS) {
            deye_hybrid_inverter.pv3_power = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV4_POWER_ADDRESS) {
            deye_hybrid_inverter.pv4_power = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS) {
            deye_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV1_CURRENT_ADDRESS) {
            deye_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS) {
            deye_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV2_CURRENT_ADDRESS) {
            deye_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS) {
            deye_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV3_CURRENT_ADDRESS) {
            deye_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS) {
            deye_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == DEYE_HYBRID_INVERTER_PV4_CURRENT_ADDRESS) {
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
            meters.update_value(slot, table->index[read_index + 2], zero_safe_negation(power_sum));
            meters.update_value(slot, table->index[read_index + 3], voltage_avg);
            meters.update_value(slot, table->index[read_index + 4], current_sum);
        }
    }
    else if (is_alpha_ess_hybrid_inverter_pv_meter()) {
        if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS) {
            alpha_ess_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV1_CURRENT_ADDRESS) {
            alpha_ess_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV1_POWER_ADDRESS) {
            alpha_ess_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS) {
            alpha_ess_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV2_CURRENT_ADDRESS) {
            alpha_ess_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV2_POWER_ADDRESS) {
            alpha_ess_hybrid_inverter.pv2_power = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS) {
            alpha_ess_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV3_CURRENT_ADDRESS) {
            alpha_ess_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV3_POWER_ADDRESS) {
            alpha_ess_hybrid_inverter.pv3_power = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS) {
            alpha_ess_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV4_CURRENT_ADDRESS) {
            alpha_ess_hybrid_inverter.pv4_current = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV4_POWER_ADDRESS) {
            alpha_ess_hybrid_inverter.pv4_power = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV5_VOLTAGE_ADDRESS) {
            alpha_ess_hybrid_inverter.pv5_voltage = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV5_CURRENT_ADDRESS) {
            alpha_ess_hybrid_inverter.pv5_current = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV5_POWER_ADDRESS) {
            alpha_ess_hybrid_inverter.pv5_power = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV6_VOLTAGE_ADDRESS) {
            alpha_ess_hybrid_inverter.pv6_voltage = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV6_CURRENT_ADDRESS) {
            alpha_ess_hybrid_inverter.pv6_current = value;
        }
        else if (register_start_address == ALPHA_ESS_HYBRID_INVERTER_PV6_POWER_ADDRESS) {
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
            meters.update_value(slot, table->index[read_index + 4], zero_safe_negation(power_sum));
        }
    }
    else if (is_shelly_pro_xem_monophase()) {
        if (register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_ACTIVE_POWER_ADDRESS
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_TOTAL_ACTIVE_ENERGY_ADDRESS
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_1_TOTAL_ACTIVE_RETURNED_ENERGY_ADDRESS
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_ACTIVE_POWER_ADDRESS
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_TOTAL_ACTIVE_ENERGY_ADDRESS
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_2_TOTAL_ACTIVE_RETURNED_ENERGY_ADDRESS
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_ACTIVE_POWER_ADDRESS
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_TOTAL_ACTIVE_ENERGY_ADDRESS
         || register_start_address == SHELLY_PRO_XEM_MONOPHASE_CHANNEL_3_TOTAL_ACTIVE_RETURNED_ENERGY_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_goodwe_hybrid_inverter_battery_meter() && goodwe_hybrid_inverter.battery_1_mode != 0 && goodwe_hybrid_inverter.battery_2_mode != 0) {
        if (register_start_address == GOODWE_HYBRID_INVERTER_BATTERY_1_VOLTAGE_ADDRESS) {
            goodwe_hybrid_inverter.battery_1_voltage = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_BATTERY_1_CURRENT_ADDRESS) {
            goodwe_hybrid_inverter.battery_1_current = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_BATTERY_1_POWER_ADDRESS) {
            goodwe_hybrid_inverter.battery_1_power = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_BMS_1_PACK_TEMPERATURE_ADDRESS) {
            goodwe_hybrid_inverter.bms_1_pack_temperature = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_BATTERY_1_CAPACITY_ADDRESS) {
            goodwe_hybrid_inverter.battery_1_capacity = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_BATTERY_2_VOLTAGE_ADDRESS) {
            goodwe_hybrid_inverter.battery_2_voltage = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_BATTERY_2_CURRENT_ADDRESS) {
            goodwe_hybrid_inverter.battery_2_current = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_BATTERY_2_POWER_ADDRESS) {
            goodwe_hybrid_inverter.battery_2_power = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_BMS_2_PACK_TEMPERATURE_ADDRESS) {
            goodwe_hybrid_inverter.bms_2_pack_temperature = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_BATTERY_2_CAPACITY_ADDRESS) {
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
        if (register_start_address == SOLAX_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS) {
            solax_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == SOLAX_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS) {
            solax_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == SOLAX_HYBRID_INVERTER_PV1_CURRENT_ADDRESS) {
            solax_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == SOLAX_HYBRID_INVERTER_PV2_CURRENT_ADDRESS) {
            solax_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == SOLAX_HYBRID_INVERTER_PV1_POWER_ADDRESS) {
            solax_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == SOLAX_HYBRID_INVERTER_PV2_POWER_ADDRESS) {
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
            meters.update_value(slot, table->index[read_index + 4], zero_safe_negation(power_sum));
        }
    }
    else if (is_goodwe_hybrid_inverter_pv_meter()) {
        if (register_start_address == GOODWE_HYBRID_INVERTER_PV_MODE_ADDRESS) {
            goodwe_hybrid_inverter.pv_mode = c32.u;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 0) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV1_CURRENT_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 0) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV1_POWER_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 0) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 8) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV2_CURRENT_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 8) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV2_POWER_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 8) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv2_power = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 16) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV3_CURRENT_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 16) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV3_POWER_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 16) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv3_power = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 24) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV4_CURRENT_ADDRESS) {
            if (((goodwe_hybrid_inverter.pv_mode >> 24) & 0xFF) == 0) {
                value = 0.0f; // ignore non-null values for disconnected PV inputs
            }

            goodwe_hybrid_inverter.pv4_current = value;
        }
        else if (register_start_address == GOODWE_HYBRID_INVERTER_PV4_POWER_ADDRESS) {
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
            meters.update_value(slot, table->index[read_index + 4], zero_safe_negation(power_sum));
        }
    }
    else if (is_fronius_gen24_plus_battery_meter()) {
        size_t start_address = register_start_address - fronius_gen24_plus.start_address_shift;

        if (start_address == FRONIUS_GEN24_PLUS_DCA_SF_ADDRESS) {
            fronius_gen24_plus.dca_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FRONIUS_GEN24_PLUS_DCV_SF_ADDRESS) {
            fronius_gen24_plus.dcv_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FRONIUS_GEN24_PLUS_DCW_SF_ADDRESS) {
            fronius_gen24_plus.dcw_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FRONIUS_GEN24_PLUS_DCWH_SF_ADDRESS) {
            fronius_gen24_plus.dcwh_sf = static_cast<int16_t>(c16.u); // SunSpec: sunssf
        }
        else if (start_address == FRONIUS_GEN24_PLUS_CHARGE_DCA_ADDRESS) {
            fronius_gen24_plus.charge_dca = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_CHARGE_DCV_ADDRESS) {
            fronius_gen24_plus.charge_dcv = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_CHARGE_DCW_ADDRESS) {
            fronius_gen24_plus.charge_dcw = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_CHARGE_DCWH_ADDRESS) {
            // Is 0 in firmware versions <= 1.30 while discharging. As this is an acc32 map 0 to NaN. This will make the
            // meters framework ignore this value during discharging and keep the pervious value
            fronius_gen24_plus.charge_dcwh = c32.u == 0 ? NAN : value; // SunSpec: acc32
        }
        else if (start_address == FRONIUS_GEN24_PLUS_DISCHARGE_DCA_ADDRESS) {
            fronius_gen24_plus.discharge_dca = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_DISCHARGE_DCV_ADDRESS) {
            fronius_gen24_plus.discharge_dcv = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_DISCHARGE_DCW_ADDRESS) {
            fronius_gen24_plus.discharge_dcw = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_DISCHARGE_DCWH_ADDRESS) {
            // Is 0 in firmware versions <= 1.30 while charging. As this is an acc32 map 0 to NaN. This will make the
            // meters framework ignore this value during charging and keep the pervious value
            fronius_gen24_plus.discharge_dcwh = c32.u == 0 ? NAN : value; // SunSpec: acc32
        }
        else if (start_address == FRONIUS_GEN24_PLUS_CHASTATE_ADDRESS) {
            fronius_gen24_plus.chastate = value; // SunSpec: uint16
        }
        else if (start_address == FRONIUS_GEN24_PLUS_CHASTATE_SF_ADDRESS) {
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
        if (register_start_address == HAILEI_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS) {
            hailei_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV1_CURRENT_ADDRESS) {
            hailei_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV1_POWER_ADDRESS) {
            hailei_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS) {
            hailei_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV2_CURRENT_ADDRESS) {
            hailei_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV2_POWER_ADDRESS) {
            hailei_hybrid_inverter.pv2_power = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS) {
            hailei_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV3_CURRENT_ADDRESS) {
            hailei_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV3_POWER_ADDRESS) {
            hailei_hybrid_inverter.pv3_power = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS) {
            hailei_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV4_CURRENT_ADDRESS) {
            hailei_hybrid_inverter.pv4_current = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV4_POWER_ADDRESS) {
            hailei_hybrid_inverter.pv4_power = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV5_VOLTAGE_ADDRESS) {
            hailei_hybrid_inverter.pv5_voltage = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV5_CURRENT_ADDRESS) {
            hailei_hybrid_inverter.pv5_current = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV5_POWER_ADDRESS) {
            hailei_hybrid_inverter.pv5_power = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV6_VOLTAGE_ADDRESS) {
            hailei_hybrid_inverter.pv6_voltage = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV6_CURRENT_ADDRESS) {
            hailei_hybrid_inverter.pv6_current = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV6_POWER_ADDRESS) {
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
            meters.update_value(slot, table->index[read_index + 4], zero_safe_negation(power_sum));
        }
    }
    else if (is_fox_ess_h3_hybrid_inverter_pv_meter()) {
        if (register_start_address == FOX_ESS_H3_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS) {
            fox_ess_h3_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV1_CURRENT_ADDRESS) {
            fox_ess_h3_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV1_POWER_ADDRESS) {
            fox_ess_h3_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS) {
            fox_ess_h3_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV2_CURRENT_ADDRESS) {
            fox_ess_h3_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == HAILEI_HYBRID_INVERTER_PV2_POWER_ADDRESS) {
            fox_ess_h3_hybrid_inverter.pv2_power = value;

            float voltage_sum = 0.0f;
            float voltage_count = 0.0f;

            if (!is_exactly_zero(fox_ess_h3_hybrid_inverter.pv1_voltage)) {
                voltage_sum += fox_ess_h3_hybrid_inverter.pv1_voltage;
                ++voltage_count;
            }

            if (!is_exactly_zero(fox_ess_h3_hybrid_inverter.pv2_voltage)) {
                voltage_sum += fox_ess_h3_hybrid_inverter.pv2_voltage;
                ++voltage_count;
            }

            float voltage_avg = voltage_sum / voltage_count;

            float current_sum = fox_ess_h3_hybrid_inverter.pv1_current
                              + fox_ess_h3_hybrid_inverter.pv2_current;

            float power_sum = fox_ess_h3_hybrid_inverter.pv1_power
                            + fox_ess_h3_hybrid_inverter.pv2_power;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
            meters.update_value(slot, table->index[read_index + 4], zero_safe_negation(power_sum));
        }
    }
    else if (is_carlo_gavazzi_em100_or_et100()) {
        if (register_start_address == CARLO_GAVAZZI_EM100_OR_ET100_W_ADDRESS
         || register_start_address == CARLO_GAVAZZI_EM100_OR_ET100_KWH_POSITIVE_TOTAL_ADDRESS
         || register_start_address == CARLO_GAVAZZI_EM100_OR_ET100_KWH_NEGATIVE_TOTAL_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_carlo_gavazzi_em510()) {
        if (register_start_address == CARLO_GAVAZZI_EM510_W_ADDRESS
         || register_start_address == CARLO_GAVAZZI_EM510_KWH_POSITIVE_TOTAL_ADDRESS
         || register_start_address == CARLO_GAVAZZI_EM510_KWH_NEGATIVE_TOTAL_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_solaredge_battery_meter()) {
        if (register_start_address == SOLAREDGE_BATTERY_1_TEMPERATURE_ADDRESS) {
            solaredge.battery_1_temperature = value;
        }
        else if (register_start_address == SOLAREDGE_BATTERY_1_VOLTAGE_ADDRESS) {
            solaredge.battery_1_voltage = value;
        }
        else if (register_start_address == SOLAREDGE_BATTERY_1_CURRENT_ADDRESS) {
            solaredge.battery_1_current = value;
        }
        else if (register_start_address == SOLAREDGE_BATTERY_1_POWER_ADDRESS) {
            solaredge.battery_1_power = value;
        }
        else if (register_start_address == SOLAREDGE_BATTERY_1_EXPORT_ENERGY_ADDRESS) {
            solaredge.battery_1_export_energy = value;
        }
        else if (register_start_address == SOLAREDGE_BATTERY_1_IMPORT_ENERGY_ADDRESS) {
            solaredge.battery_1_import_energy = value;
        }
        else if (register_start_address == SOLAREDGE_BATTERY_1_STATE_OF_CHARGE_ADDRESS) {
            solaredge.battery_1_state_of_charge = value;
        }
        else if (register_start_address == SOLAREDGE_BATTERY_2_TEMPERATURE_ADDRESS) {
            value = nan_safe_sum(solaredge.battery_1_temperature, value);
        }
        else if (register_start_address == SOLAREDGE_BATTERY_2_VOLTAGE_ADDRESS) {
            value = nan_safe_avg(solaredge.battery_1_voltage, value);
        }
        else if (register_start_address == SOLAREDGE_BATTERY_2_CURRENT_ADDRESS) {
            value = nan_safe_sum(solaredge.battery_1_current, value);
        }
        else if (register_start_address == SOLAREDGE_BATTERY_2_POWER_ADDRESS) {
            value = nan_safe_sum(solaredge.battery_1_power, value);
        }
        else if (register_start_address == SOLAREDGE_BATTERY_2_EXPORT_ENERGY_ADDRESS) {
            value = nan_safe_sum(solaredge.battery_1_export_energy, value);
        }
        else if (register_start_address == SOLAREDGE_BATTERY_2_IMPORT_ENERGY_ADDRESS) {
            value = nan_safe_sum(solaredge.battery_1_import_energy, value);
        }
        else if (register_start_address == SOLAREDGE_BATTERY_2_STATE_OF_CHARGE_ADDRESS) {
            value = nan_safe_avg(solaredge.battery_1_state_of_charge, value);
        }
    }
    else if (is_e3dc_pv_meter()) {
        if (register_start_address == E3DC_PV_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], zero_safe_negation(value));
        }
    }
    else if (is_huawei_sun2000_pv_meter()) {
        if (register_start_address == HUAWEI_SUN2000_INPUT_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], zero_safe_negation(value));
        }
    }
    else if (is_huawei_sun2000_smart_dongle_pv_meter()) {
        if (register_start_address == HUAWEI_SUN2000_SMART_DONGLE_INPUT_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], zero_safe_negation(value));
        }
    }
    else if (is_huawei_emma_load_meter()) {
        if (register_start_address == HUAWEI_EMMA_LOAD_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], value);
        }
    }
    else if (is_huawei_emma_pv_meter()) {
        if (register_start_address == HUAWEI_EMMA_PV_OUTPUT_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], zero_safe_negation(value));
        }
    }
    else if (is_solax_string_inverter_meter()) {
        if (register_start_address == SOLAX_STRING_INVERTER_OUTPUT_POWER_ADDRESS) {
            meters.update_value(slot, table->index[read_index + 1], zero_safe_negation(value));
        }
    }
    else if (is_solax_string_inverter_pv_meter()) {
        if (register_start_address == SOLAX_STRING_INVERTER_PV1_VOLTAGE_ADDRESS) {
            solax_string_inverter.pv1_voltage = value;
        }
        else if (register_start_address == SOLAX_STRING_INVERTER_PV2_VOLTAGE_ADDRESS) {
            solax_string_inverter.pv2_voltage = value;
        }
        else if (register_start_address == SOLAX_STRING_INVERTER_PV1_CURRENT_ADDRESS) {
            solax_string_inverter.pv1_current = value;
        }
        else if (register_start_address == SOLAX_STRING_INVERTER_PV2_CURRENT_ADDRESS) {
            solax_string_inverter.pv2_current = value;
        }
        else if (register_start_address == SOLAX_STRING_INVERTER_PV1_POWER_ADDRESS) {
            solax_string_inverter.pv1_power = value;
        }
        else if (register_start_address == SOLAX_STRING_INVERTER_PV2_POWER_ADDRESS) {
            solax_string_inverter.pv2_power = value;
        }
        else if (register_start_address == SOLAX_STRING_INVERTER_PV3_VOLTAGE_ADDRESS) {
            solax_string_inverter.pv3_voltage = value;
        }
        else if (register_start_address == SOLAX_STRING_INVERTER_PV3_CURRENT_ADDRESS) {
            solax_string_inverter.pv3_current = value;
        }
        else if (register_start_address == SOLAX_STRING_INVERTER_PV3_POWER_ADDRESS) {
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
            meters.update_value(slot, table->index[read_index + 4], zero_safe_negation(power_sum));
        }
    }
    else if (is_fox_ess_h3_smart_hybrid_inverter_pv_meter()) {
        if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV1_CURRENT_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV1_POWER_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV2_CURRENT_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV2_POWER_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv2_power = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV3_CURRENT_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV3_POWER_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv3_power = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV4_CURRENT_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv4_current = value;
        }
        else if (register_start_address == FOX_ESS_H3_SMART_HYBRID_INVERTER_PV4_POWER_ADDRESS) {
            fox_ess_h3_smart_hybrid_inverter.pv4_power = value;

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

            float power_sum = fox_ess_h3_smart_hybrid_inverter.pv1_power
                            + fox_ess_h3_smart_hybrid_inverter.pv2_power
                            + fox_ess_h3_smart_hybrid_inverter.pv3_power
                            + fox_ess_h3_smart_hybrid_inverter.pv4_power;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
            meters.update_value(slot, table->index[read_index + 4], zero_safe_negation(power_sum));
        }
    }
    else if (is_fox_ess_h3_pro_hybrid_inverter_pv_meter()) {
        if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV1_VOLTAGE_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv1_voltage = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV1_CURRENT_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv1_current = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV1_POWER_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv1_power = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV2_VOLTAGE_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv2_voltage = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV2_CURRENT_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv2_current = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV2_POWER_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv2_power = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV3_VOLTAGE_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv3_voltage = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV3_CURRENT_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv3_current = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV3_POWER_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv3_power = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV4_VOLTAGE_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv4_voltage = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV4_CURRENT_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv4_current = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV4_POWER_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv4_power = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV5_VOLTAGE_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv5_voltage = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV5_CURRENT_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv5_current = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV5_POWER_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv5_power = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV6_VOLTAGE_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv6_voltage = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV6_CURRENT_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv6_current = value;
        }
        else if (register_start_address == FOX_ESS_H3_PRO_HYBRID_INVERTER_PV6_POWER_ADDRESS) {
            fox_ess_h3_pro_hybrid_inverter.pv6_power = value;

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

            float power_sum = fox_ess_h3_pro_hybrid_inverter.pv1_power
                            + fox_ess_h3_pro_hybrid_inverter.pv2_power
                            + fox_ess_h3_pro_hybrid_inverter.pv3_power
                            + fox_ess_h3_pro_hybrid_inverter.pv4_power
                            + fox_ess_h3_pro_hybrid_inverter.pv5_power
                            + fox_ess_h3_pro_hybrid_inverter.pv6_power;

            meters.update_value(slot, table->index[read_index + 1], voltage_avg);
            meters.update_value(slot, table->index[read_index + 2], current_sum);
            meters.update_value(slot, table->index[read_index + 3], power_sum);
            meters.update_value(slot, table->index[read_index + 4], zero_safe_negation(power_sum));
        }
    }
    else if (is_sma_hybrid_inverter_battery_meter()) {
        if (register_start_address == SMA_HYBRID_INVERTER_BATTERY_CHARGE_POWER_ADDRESS) {
            sma_hybrid_inverter.battery_charge_power = value;
        }
        else if (register_start_address == SMA_HYBRID_INVERTER_BATTERY_DISCHARGE_POWER_ADDRESS) {
            sma_hybrid_inverter.battery_discharge_power = value;

            float power = sma_hybrid_inverter.battery_charge_power - sma_hybrid_inverter.battery_discharge_power;

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
