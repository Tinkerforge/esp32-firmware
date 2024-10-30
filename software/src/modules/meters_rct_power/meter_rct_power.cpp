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

MeterClassID MeterRCTPower::get_class() const
{
    return MeterClassID::RCTPower;
}

void MeterRCTPower::setup(const Config &ephemeral_config)
{
    host_name     = ephemeral_config.get("host")->asString();
    port          = static_cast<uint16_t>(ephemeral_config.get("port")->asUint());
    virtual_meter = ephemeral_config.get("virtual_meter")->asEnum<VirtualMeter>();

    switch (virtual_meter) {
    case VirtualMeter::None:
        logger.printfln("No Virtual Meter selected");
        return;

    case VirtualMeter::InverterUnused:
        logger.printfln("Invalid Virtual Meter: %u", static_cast<uint8_t>(virtual_meter));
        return;

    case VirtualMeter::Grid:
        value_specs = grid_rct_value_specs;
        value_specs_length = ARRAY_SIZE(grid_rct_value_specs);
        meters.declare_value_ids(slot, grid_value_ids, ARRAY_SIZE(grid_value_ids));
        break;

    case VirtualMeter::Battery:
        value_specs = battery_rct_value_specs;
        value_specs_length = ARRAY_SIZE(battery_rct_value_specs);
        meters.declare_value_ids(slot, battery_value_ids, ARRAY_SIZE(battery_value_ids));
        break;

    default:
        logger.printfln("Unknown Virtual Meter: %u", static_cast<uint8_t>(virtual_meter));
        return;
    }

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (read_allowed) {
            read_allowed = false;
            read_next();
        }
    }, 2_s, 250_ms);
}

void MeterRCTPower::register_events()
{
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

void MeterRCTPower::pre_reboot()
{
    stop_connection();
}

void MeterRCTPower::connect_callback()
{
    value_specs_index = 0;

    read_next();
}

void MeterRCTPower::disconnect_callback()
{
    read_allowed = false;
}

void MeterRCTPower::read_next()
{
    if (connected_client == nullptr) {
        esp_system_abort("meter_rct_power: Not connected while trying to read");
    }

    if (value_specs_length == 0) {
        return;
    }

    if (value_specs_index >= value_specs_length) {
        value_specs_index = 0;
    }

    static_cast<RCTPowerSharedClient *>(connected_client)->read(&value_specs[value_specs_index], 2_s,
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
                logger.printfln("Error reading ID 0x%08u: %s [%d]",
                                value_specs[value_specs_index].id,
                                get_rct_power_client_transaction_result_name(result),
                                static_cast<int>(result));
            }
        }
        else {
            meters.update_value(slot, value_specs_index, value);
            meters.finish_update(slot);

            ++value_specs_index;
        }

        read_allowed = true;
    });
}
