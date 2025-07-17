/* esp32-firmware
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

#include "meter_rct_power.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "rct_power_client.h"
#include "modules/meters/meter_location.enum.h"
#include "tools/float.h"

static const MeterValueID inverter_value_ids[] = {
    MeterValueID::VoltageL1N,
    MeterValueID::VoltageL2N,
    MeterValueID::VoltageL3N,
    MeterValueID::VoltageL1L2,
    MeterValueID::VoltageL2L3,
    MeterValueID::VoltageL3L1,
    MeterValueID::CurrentL1ImExSum,
    MeterValueID::CurrentL2ImExSum,
    MeterValueID::CurrentL3ImExSum,
    MeterValueID::PowerActiveL1ImExDiff,
    MeterValueID::PowerActiveL2ImExDiff,
    MeterValueID::PowerActiveL3ImExDiff,
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::PowerApparentL1ImExDiff,
    MeterValueID::PowerApparentL2ImExDiff,
    MeterValueID::PowerApparentL3ImExDiff,
    MeterValueID::PowerApparentLSumImExDiff,
    MeterValueID::PowerReactiveL1IndCapDiff,
    MeterValueID::PowerReactiveL2IndCapDiff,
    MeterValueID::PowerReactiveL3IndCapDiff,
    MeterValueID::PowerReactiveLSumIndCapDiff,
};

static const RCTValueSpec inverter_rct_value_specs[] = {
    {0xCF053085,  1.0f}, // AC voltage phase 1 [V]       / g_sync.u_l_rms[0]
    {0x54B4684E,  1.0f}, // AC voltage phase 2 [V]       / g_sync.u_l_rms[1]
    {0x2545E22D,  1.0f}, // AC voltage phase 3 [V]       / g_sync.u_l_rms[2]
    {0x63476DBE,  1.0f}, // Phase to phase voltage 1 [V] / g_sync.u_ptp_rms[0]
    {0x485AD749,  1.0f}, // Phase to phase voltage 2 [V] / g_sync.u_ptp_rms[1]
    {0xF25C339B,  1.0f}, // Phase to phase voltage 3 [V] / g_sync.u_ptp_rms[2]
    {0x89EE3EB5,  1.0f}, // Current phase 1 [A]          / g_sync.i_dr_eff[0]
    {0x650C1ED7,  1.0f}, // Current phase 2 [A]          / g_sync.i_dr_eff[1]
    {0x92BC682B,  1.0f}, // Current phase 3 [A]          / g_sync.i_dr_eff[2]
    {0x71E10B51, -1.0f}, // AC power phase 1 [W]         / g_sync.p_ac_lp[0]
    {0x6E1C5B78, -1.0f}, // AC power phase 2 [W]         / g_sync.p_ac_lp[1]
    {0xB9928C51, -1.0f}, // AC power phase 3 [W]         / g_sync.p_ac_lp[2]
    {0xDB2D69AE, -1.0f}, // AC power [W]                 / g_sync.p_ac_sum_lp
    {0x3A444FC6, -1.0f}, // Apparent power phase 1 [VA]  / g_sync.s_ac_lp[0]
    {0x4077335D, -1.0f}, // Apparent power phase 2 [VA]  / g_sync.s_ac_lp[1]
    {0x883DE9AB, -1.0f}, // Apparent power phase 3 [VA]  / g_sync.s_ac_lp[2]
    {0xDCA1CF26, -1.0f}, // Apparent power [VA]          / g_sync.s_ac_sum_lp
    {0xE94C2EFC,  1.0f}, // Reactive power phase 1 [var] / g_sync.q_ac[0]
    {0x82E3C121,  1.0f}, // Reactive power phase 2 [var] / g_sync.q_ac[1]
    {0xBCA77559,  1.0f}, // Reactive power phase 3 [var] / g_sync.q_ac[2]
    {0x7C78CBAC,  1.0f}, // Reactive power [var]         / g_sync.q_ac_sum_lp
};

static_assert(ARRAY_SIZE(inverter_value_ids) == ARRAY_SIZE(inverter_rct_value_specs));

static const MeterValueID grid_value_ids[] = {
    MeterValueID::PowerActiveL1ImExDiff,
    MeterValueID::PowerActiveL2ImExDiff,
    MeterValueID::PowerActiveL3ImExDiff,
    MeterValueID::PowerActiveLSumImExDiff,
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::FrequencyLAvg,
};

static const RCTValueSpec grid_rct_value_specs[] = {
    {0x27BE51D9,  1.0f},   // Grid power phase 1 [W]         / g_sync.p_ac_sc[0]
    {0xF5584F90,  1.0f},   // Grid power phase 2 [W]         / g_sync.p_ac_sc[1]
    {0xB221BCFA,  1.0f},   // Grid power phase 3 [W]         / g_sync.p_ac_sc[2]
    {0x91617C58,  1.0f},   // Total grid power [W]           / g_sync.p_ac_grid_sum_lp
    {0x44D4C533, -0.001f}, // Total energy grid feed-in [Wh] / energy.e_grid_feed_total
    {0x62FBE7DC,  0.001f}, // Total energy grid load [Wh]    / energy.e_grid_load_total
    {0x1C4A665F,  1.0f},   // Grid frequency [Hz]            / grid_pll[0].f
};

static_assert(ARRAY_SIZE(grid_value_ids) == ARRAY_SIZE(grid_rct_value_specs));

static const MeterValueID battery_value_ids[] = {
    MeterValueID::VoltageDC,
    MeterValueID::CurrentDCChaDisDiff,
    MeterValueID::PowerDCChaDisDiff,
    MeterValueID::EnergyDCCharge,
    MeterValueID::EnergyDCDischarge,
    MeterValueID::StateOfCharge,
    MeterValueID::Temperature,
};

static const RCTValueSpec battery_rct_value_specs[] = {
    {0x65EED11B,   1.0f},   // Battery voltage [V]                 / battery.voltage
    {0x21961B58,  -1.0f},   // Battery current [A]                 / battery.current
    {0x400F015B,  -1.0f},   // Battery power [W]                   / g_sync.p_acc_lp
    {0x5570401B,   0.001f}, // Total energy flow into battery [Wh] / battery.stored_energy
    {0xA9033880,   0.001f}, // Total energy flow from battery [Wh] / battery.used_energy
    {0x959930BF, 100.0f},   // Battery SOC [0.01 %]                / battery.soc
    {0x902AFAFB,   1.0f},   // Battery temperature [°C]            / battery.temperature
};

static_assert(ARRAY_SIZE(battery_value_ids) == ARRAY_SIZE(battery_rct_value_specs));

static const MeterValueID load_value_ids[] = {
    MeterValueID::PowerActiveL1ImExDiff,
    MeterValueID::PowerActiveL2ImExDiff,
    MeterValueID::PowerActiveL3ImExDiff,
    MeterValueID::EnergyActiveLSumImExDiff,
};

static const RCTValueSpec load_rct_value_specs[] = {
    {0x3A39CA2,  1.0f},   // Load household phase 1 [W]  / g_sync.p_ac_load[0]
    {0x2788928C, 1.0f},   // Load household phase 2 [W]  / g_sync.p_ac_load[1]
    {0xF0B436DD, 1.0f},   // Load household phase 3 [W]  / g_sync.p_ac_load[2]
    {0xEFF4B537, 0.001f}, // Household total energy [Wh] / energy.e_load_total
};

static_assert(ARRAY_SIZE(load_value_ids) == ARRAY_SIZE(load_rct_value_specs));

static const MeterValueID pv_value_ids[] = {
    MeterValueID::VoltagePV1,
    MeterValueID::PowerPV1Export,
    MeterValueID::EnergyPV1Export,
    MeterValueID::VoltagePV2,
    MeterValueID::PowerPV2Export,
    MeterValueID::EnergyPV2Export,

    MeterValueID::VoltagePVAvg,
    MeterValueID::PowerPVSumExport,
    MeterValueID::PowerPVSumImExDiff,
    MeterValueID::EnergyPVSumExport
};

static const RCTValueSpec pv_rct_value_specs[] = {
    {0xB298395D, 1.0f},   // Solar generator A voltage [V]       / dc_conv.dc_conv_struct[0].u_sg_lp
    {0xB5317B78, 1.0f},   // Solar generator A power [W]         / dc_conv.dc_conv_struct[0].p_dc
    {0xFC724A9E, 0.001f}, // Solar generator A total energy [Wh] / energy.e_dc_total[0]
    {0x5BB8075A, 1.0f},   // Solar generator B voltage [V]       / dc_conv.dc_conv_struct[1].u_sg_lp
    {0xAA9AA253, 1.0f},   // Solar generator B power [W]         / dc_conv.dc_conv_struct[1].p_dc
    {0x68EEFD3D, 0.001f}, // Solar generator B total energy [Wh] / energy.e_dc_total[1]
};

static_assert(ARRAY_SIZE(pv_value_ids) - 4 == ARRAY_SIZE(pv_rct_value_specs));

MeterClassID MeterRCTPower::get_class() const
{
    return MeterClassID::RCTPower;
}

void MeterRCTPower::setup(Config *ephemeral_config)
{
    host          = ephemeral_config->get("host")->asString();
    port          = static_cast<uint16_t>(ephemeral_config->get("port")->asUint());
    virtual_meter = ephemeral_config->get("virtual_meter")->asEnum<VirtualMeter>();

    MeterLocation default_location = MeterLocation::Unknown;

    switch (virtual_meter) {
    case VirtualMeter::None:
        logger.printfln_meter("No Virtual Meter selected");
        return;

    case VirtualMeter::Inverter:
        value_specs = inverter_rct_value_specs;
        value_specs_length = ARRAY_SIZE(inverter_rct_value_specs);
        value_ids = inverter_value_ids;
        value_ids_length = ARRAY_SIZE(inverter_value_ids);
        default_location = MeterLocation::Inverter;
        break;

    case VirtualMeter::Grid:
        value_specs = grid_rct_value_specs;
        value_specs_length = ARRAY_SIZE(grid_rct_value_specs);
        value_ids = grid_value_ids;
        value_ids_length = ARRAY_SIZE(grid_value_ids);
        default_location = MeterLocation::Grid;
        break;

    case VirtualMeter::Battery:
        value_specs = battery_rct_value_specs;
        value_specs_length = ARRAY_SIZE(battery_rct_value_specs);
        value_ids = battery_value_ids;
        value_ids_length = ARRAY_SIZE(battery_value_ids);
        default_location = MeterLocation::Battery;
        break;

    case VirtualMeter::Load:
        value_specs = load_rct_value_specs;
        value_specs_length = ARRAY_SIZE(load_rct_value_specs);
        value_ids = load_value_ids;
        value_ids_length = ARRAY_SIZE(load_value_ids);
        default_location = MeterLocation::Load;
        break;

    case VirtualMeter::PV:
        value_specs = pv_rct_value_specs;
        value_specs_length = ARRAY_SIZE(pv_rct_value_specs);
        value_ids = pv_value_ids;
        value_ids_length = ARRAY_SIZE(pv_value_ids);
        default_location = MeterLocation::PV;
        break;

    default:
        logger.printfln_meter("Unknown Virtual Meter: %u", static_cast<uint8_t>(virtual_meter));
        return;
    }

    meters.declare_value_ids(slot, value_ids, value_ids_length);

    if (ephemeral_config->get("location")->asEnum<MeterLocation>() == MeterLocation::Unknown && default_location != MeterLocation::Unknown) {
        ephemeral_config->get("location")->updateEnum(default_location);
    }

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (read_allowed) {
            read_next();
        }
    }, 2_s, 250_ms);
}

void MeterRCTPower::register_events()
{
    if (value_specs == nullptr) {
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

void MeterRCTPower::pre_reboot()
{
    stop_connection();
}

void MeterRCTPower::connect_callback()
{
    value_index = 0;

    read_next();
}

void MeterRCTPower::disconnect_callback()
{
    read_allowed = false;
}

void MeterRCTPower::read_next()
{
    if (value_specs_length == 0) {
        return;
    }

    read_allowed = false;

    if (value_index >= value_specs_length) {
        value_index = 0;
    }

    static_cast<RCTPowerSharedClient *>(connected_client)->read(&value_specs[value_index], 2_s,
    [this](RCTPowerClientTransactionResult result, float value) {
        if (result != RCTPowerClientTransactionResult::Success) {
            if (result == RCTPowerClientTransactionResult::Timeout) {
                auto timeout = errors->get("timeout");
                timeout->updateUint(timeout->asUint() + 1);
            }
            else if (result == RCTPowerClientTransactionResult::ChecksumMismatch) {
                auto checksum_mismatch = errors->get("checksum_mismatch");
                checksum_mismatch->updateUint(checksum_mismatch->asUint() + 1);
            }
            else {
                logger.printfln_meter("Error reading ID 0x%08lx: %s [%i]",
                                      value_specs[value_index].id,
                                      get_rct_power_client_transaction_result_name(result),
                                      static_cast<int>(result));
            }
        }
        else {
            meters.update_value(slot, value_index, value);

            if (virtual_meter == VirtualMeter::PV) {
                if (value_ids[value_index] == MeterValueID::VoltagePV1) {
                    pv1_voltage = value;
                }
                else if (value_ids[value_index] == MeterValueID::PowerPV1Export) {
                    pv1_power = value;
                }
                else if (value_ids[value_index] == MeterValueID::EnergyPV1Export) {
                    pv1_energy = value;
                }
                else if (value_ids[value_index] == MeterValueID::VoltagePV2) {
                    pv2_voltage = value;
                }
                else if (value_ids[value_index] == MeterValueID::PowerPV2Export) {
                    pv2_power = value;
                }
                else if (value_ids[value_index] == MeterValueID::EnergyPV2Export) {
                    pv2_energy = value;

                    float voltage_sum = 0.0f;
                    float voltage_count = 0.0f;

                    if (!is_exactly_zero(pv1_voltage)) {
                        voltage_sum += pv1_voltage;
                        ++voltage_count;
                    }

                    if (!is_exactly_zero(pv2_voltage)) {
                        voltage_sum += pv2_voltage;
                        ++voltage_count;
                    }

                    float voltage_avg = voltage_sum / voltage_count;
                    float power_sum = pv1_power + pv2_power;
                    float energy_sum = pv1_energy + pv2_energy;

                    meters.update_value(slot, value_index + 1, voltage_avg);
                    meters.update_value(slot, value_index + 2, power_sum);
                    meters.update_value(slot, value_index + 3, zero_safe_negation(power_sum));
                    meters.update_value(slot, value_index + 4, energy_sum);
                }
            }

            meters.finish_update(slot);

            ++value_index;
        }

        read_allowed = true;
    });
}
