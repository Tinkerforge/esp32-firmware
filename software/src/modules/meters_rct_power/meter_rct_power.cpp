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

static const MeterValueID grid_value_ids[] = {
    MeterValueID::EnergyActiveLSumExport,
    MeterValueID::EnergyActiveLSumImport,
    MeterValueID::PowerActiveLSumImExDiff,
};

static const RCTValueSpec grid_rct_value_specs[] = {
    {0x44D4C533, -0.001f}, // Total energy grid feed-in [Wh]
    {0x62FBE7DC,  0.001f}, // Total energy grid load [Wh]
    {0x91617C58,  1.0f},   // Total grid power [W]
};

static const MeterValueID battery_value_ids[] = {
    MeterValueID::CurrentDC,
    MeterValueID::PowerDCChaDisDiff,
    MeterValueID::EnergyDCCharge,
    MeterValueID::VoltageDC,
    MeterValueID::Temperature,
    MeterValueID::StateOfCharge,
    MeterValueID::EnergyDCDischarge,
};

static const RCTValueSpec battery_rct_value_specs[] = {
    {0x21961B58,  -1.0f},   // Battery current [A]
    {0x400F015B,  -1.0f},   // Battery power [W]
    {0x5570401B,   0.001f}, // Total energy flow into battery [Wh]
    {0x65EED11B,   1.0f},   // Battery voltage [V]
    {0x902AFAFB,   1.0f},   // Battery temperature [°C]
    {0x959930BF, 100.0f},   // Battery SOC [0.01 %]
    {0xA9033880,   0.001f}, // Total energy flow from battery [Wh]
};

static const MeterValueID load_value_ids[] = {
    MeterValueID::PowerActiveL1ImExDiff,
    MeterValueID::PowerActiveL2ImExDiff,
    MeterValueID::EnergyActiveLSumImExDiff,
    MeterValueID::PowerActiveL3ImExDiff,
};

static const RCTValueSpec load_rct_value_specs[] = {
    {0x03A39CA2, 1.0f},   // Load household phase 1 [W]
    {0x2788928C, 1.0f},   // Load household phase 2 [W]
    {0xEFF4B537, 0.001f}, // Household total energy [Wh] // FIXME: direction?
    {0xF0B436DD, 1.0f},   // Load household phase 3 [W]
};

static const MeterValueID pv_value_ids[] = {
    MeterValueID::VoltagePV2,
    MeterValueID::PowerPV2Export,
    MeterValueID::VoltagePV1,
    MeterValueID::PowerPV1Export,
    MeterValueID::VoltagePVAvg,
    MeterValueID::PowerPVSumExport,
    MeterValueID::PowerPVSumImExDiff,
};

static const RCTValueSpec pv_rct_value_specs[] = {
    {0x5BB8075A, 1.0f}, // Solar generator B voltage [V]
    {0xAA9AA253, 1.0f}, // Solar generator B power [W]
    {0xB298395D, 1.0f}, // Solar generator A voltage [V]
    {0xB5317B78, 1.0f}, // Solar generator A power [W]
};

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

    case VirtualMeter::InverterUnused:
        logger.printfln_meter("Invalid Virtual Meter: %u", static_cast<uint8_t>(virtual_meter));
        default_location = MeterLocation::Inverter;
        return;

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
                if (value_ids[value_index] == MeterValueID::VoltagePV2) {
                    pv2_voltage = value;
                }
                else if (value_ids[value_index] == MeterValueID::PowerPV2Export) {
                    pv2_power = value;
                }
                if (value_ids[value_index] == MeterValueID::VoltagePV1) {
                    pv1_voltage = value;
                }
                else if (value_ids[value_index] == MeterValueID::PowerPV1Export) {
                    pv1_power = value;

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

                    meters.update_value(slot, value_index + 1, voltage_avg);
                    meters.update_value(slot, value_index + 2, power_sum);
                    meters.update_value(slot, value_index + 3, zero_safe_negation(power_sum));
                }
            }

            meters.finish_update(slot);

            ++value_index;
        }

        read_allowed = true;
    });
}
