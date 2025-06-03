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

#define EVENT_LOG_PREFIX "batteries_mbtcp"

#include "battery_modbus_tcp.h"

#include <TFModbusTCPClient.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modules/modbus_tcp_client/modbus_tcp_tools.h"

BatteryClassID BatteryModbusTCP::get_class() const
{
    return BatteryClassID::ModbusTCP;
}

void BatteryModbusTCP::setup(const Config &ephemeral_config)
{
    host      = ephemeral_config.get("host")->asString();
    port      = static_cast<uint16_t>(ephemeral_config.get("port")->asUint());
    table_id  = ephemeral_config.get("table")->getTag<BatteryModbusTCPTableID>();

    switch (table_id) {
    case BatteryModbusTCPTableID::None:
        logger.printfln_battery("No table selected");
        return;

    case BatteryModbusTCPTableID::Custom: {
            const Config *table_config = static_cast<const Config *>(ephemeral_config.get("table")->get());

            // FIXME: leaking this, because as of right now battery instances don't get destroyed
            custom_table_permit_grid_charge          = BatteriesModbusTCP::read_table_config(static_cast<const Config *>(table_config->get("permit_grid_charge")));
            custom_table_revoke_grid_charge_override = BatteriesModbusTCP::read_table_config(static_cast<const Config *>(table_config->get("revoke_grid_charge_override")));
            custom_table_forbid_discharge            = BatteriesModbusTCP::read_table_config(static_cast<const Config *>(table_config->get("forbid_discharge")));
            custom_table_revoke_discharge_override   = BatteriesModbusTCP::read_table_config(static_cast<const Config *>(table_config->get("revoke_discharge_override")));

            tables[static_cast<uint32_t>(IBattery::Action::PermitGridCharge)]         = custom_table_permit_grid_charge;
            tables[static_cast<uint32_t>(IBattery::Action::RevokeGridChargeOverride)] = custom_table_revoke_grid_charge_override;
            tables[static_cast<uint32_t>(IBattery::Action::ForbidDischarge)]          = custom_table_forbid_discharge;
            tables[static_cast<uint32_t>(IBattery::Action::RevokeDischargeOverride)]  = custom_table_revoke_discharge_override;
        }

        break;

    default:
        logger.printfln_battery("Unknown table: %u", static_cast<uint8_t>(table_id));
        return;
    }
}

void BatteryModbusTCP::register_events()
{
    // FIXME: maybe don't keep the connection open, but instead only connect for the duration of action's execution
    event.registerEvent("network/state", {"connected"}, [this](const Config *connected) {
        if (connected->asBool()) {
            start_connection();
        } else {
            stop_connection();
        }

        return EventResult::OK;
    });
}

void BatteryModbusTCP::pre_reboot()
{
    stop_connection();
}

bool BatteryModbusTCP::supports_action(Action /*action*/)
{
    return true;
}

bool BatteryModbusTCP::start_action(Action action)
{
    if (connected_client == nullptr) {
        logger.printfln_battery("Not connected, cannot start action");
        return false;
    }

    if (has_current_action) {
        logger.printfln_battery("Another action is already in progress");
        return false;
    }

    has_current_action   = true;
    current_action       = action;
    current_action_index = 0;

    write_next();

    return true;
}

bool BatteryModbusTCP::get_current_action(Action *action)
{
    *action = current_action;

    return has_current_action;
}

void BatteryModbusTCP::connect_callback()
{
}

void BatteryModbusTCP::disconnect_callback()
{
}

void BatteryModbusTCP::write_next()
{
    if (!has_current_action) {
        return;
    }

    if (connected_client == nullptr) {
        // FIXME: maybe retry if connection is lost in the middle of an action, instead of aborting
        has_current_action = false;
        return;
    }

    const BatteriesModbusTCP::ValueTable *table = tables[static_cast<uint32_t>(current_action)];

    if (current_action_index >= table->values_length) {
        has_current_action = false; // action is done
        return;
    }

    const BatteriesModbusTCP::ValueSpec *spec = &table->values[current_action_index];
    TFModbusTCPFunctionCode function_code;

    switch (spec->register_type) {
    case ModbusRegisterType::HoldingRegister:
        function_code = TFModbusTCPFunctionCode::WriteMultipleRegisters;
        break;

    case ModbusRegisterType::Coil:
        function_code = TFModbusTCPFunctionCode::WriteMultipleCoils;
        break;

    case ModbusRegisterType::InputRegister:
    case ModbusRegisterType::DiscreteInput:
    default:
        esp_system_abort("battery_modbus_tcp: Unsupported register type to write.");
    }

    static_cast<TFModbusTCPSharedClient *>(connected_client)->transact(table->device_address,
                                                                       function_code,
                                                                       spec->start_address,
                                                                       1,
                                                                       const_cast<uint16_t *>(&spec->value),
                                                                       2_s,
    [this, table, spec, function_code](TFModbusTCPClientTransactionResult result) {
        if (result != TFModbusTCPClientTransactionResult::Success) {
            logger.printfln_battery("Modbus write error (host='%s' port=%u devaddr=%u fcode=%d regaddr=%u value=%u): %s (%d)",
                                    host.c_str(),
                                    port,
                                    table->device_address,
                                    static_cast<int>(function_code),
                                    spec->start_address,
                                    spec->value,
                                    get_tf_modbus_tcp_client_transaction_result_name(result),
                                    static_cast<int>(result));

            // FIXME: maybe retry on error in the middle of an action, instead of aborting
            has_current_action = false;
            return;
        }

        ++current_action_index;

        if (current_action_index >= table->values_length) {
            has_current_action = false; // action is done
            return;
        }

        write_next(); // FIXME: maybe add a little delay between writes to avoid bursts?
    });
}
