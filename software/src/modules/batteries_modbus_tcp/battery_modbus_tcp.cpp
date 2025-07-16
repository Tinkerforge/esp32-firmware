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
            tables[static_cast<uint32_t>(IBattery::Action::PermitGridCharge)]         = BatteriesModbusTCP::init_table(static_cast<const Config *>(table_config->get("permit_grid_charge")));
            tables[static_cast<uint32_t>(IBattery::Action::RevokeGridChargeOverride)] = BatteriesModbusTCP::init_table(static_cast<const Config *>(table_config->get("revoke_grid_charge_override")));
            tables[static_cast<uint32_t>(IBattery::Action::ForbidDischarge)]          = BatteriesModbusTCP::init_table(static_cast<const Config *>(table_config->get("forbid_discharge")));
            tables[static_cast<uint32_t>(IBattery::Action::RevokeDischargeOverride)]  = BatteriesModbusTCP::init_table(static_cast<const Config *>(table_config->get("revoke_discharge_override")));
            tables[static_cast<uint32_t>(IBattery::Action::ForbidCharge)]             = BatteriesModbusTCP::init_table(static_cast<const Config *>(table_config->get("forbid_charge")));
            tables[static_cast<uint32_t>(IBattery::Action::RevokeChargeOverride)]     = BatteriesModbusTCP::init_table(static_cast<const Config *>(table_config->get("revoke_charge_override")));
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

void BatteryModbusTCP::start_action(Action action, std::function<void(bool)> &&callback)
{
    const BatteriesModbusTCP::TableSpec *table = tables[static_cast<uint32_t>(action)];

    if (table == nullptr) {
        if (callback) {
            callback(true);
        }

        return; // nothing to do
    }

    if (connected_client == nullptr) {
        logger.printfln_battery("Not connected, cannot start action");

        if (callback) {
            callback(false);
        }

        return;
    }

    Execution *execution = new Execution;

    execution->table = tables[static_cast<uint32_t>(action)];
    execution->callback = std::move(callback);
    execution->index = 0;

    write_next(execution);
}

void BatteryModbusTCP::connect_callback()
{
}

void BatteryModbusTCP::disconnect_callback()
{
}

void BatteryModbusTCP::write_next(Execution *execution)
{
    if (execution->index >= execution->table->register_blocks_count) {
        if (execution->callback) {
            execution->callback(true);
        }

        delete execution;
        return; // action is done
    }

    if (connected_client == nullptr) {
        if (execution->callback) {
            execution->callback(false);
        }

        delete execution;
        return;
    }

    BatteriesModbusTCP::RegisterBlockSpec *register_block = &execution->table->register_blocks[execution->index];
    TFModbusTCPFunctionCode function_code;

    switch (register_block->function_code) {
    case ModbusFunctionCode::WriteSingleCoil:
        function_code = TFModbusTCPFunctionCode::WriteSingleCoil;
        break;

    case ModbusFunctionCode::WriteSingleRegister:
        function_code = TFModbusTCPFunctionCode::WriteSingleRegister;
        break;

    case ModbusFunctionCode::WriteMultipleCoils:
        function_code = TFModbusTCPFunctionCode::WriteMultipleCoils;
        break;

    case ModbusFunctionCode::WriteMultipleRegisters:
        function_code = TFModbusTCPFunctionCode::WriteMultipleRegisters;
        break;

    default:
        logger.printfln_battery("Unsupported function code: %u", static_cast<uint8_t>(register_block->function_code));

        if (execution->callback) {
            execution->callback(false);
        }

        delete execution;
        return;
    }

    static_cast<TFModbusTCPSharedClient *>(connected_client)->transact(execution->table->device_address,
                                                                       function_code,
                                                                       register_block->start_address,
                                                                       register_block->values_count,
                                                                       register_block->values_buffer,
                                                                       2_s,
    [this, execution](TFModbusTCPClientTransactionResult result, const char *error_message) {
        if (result != TFModbusTCPClientTransactionResult::Success) {
            logger.printfln_battery("Action execution failed at %zu of %zu: %s (%d)%s%s",
                                    execution->index + 1, execution->table->register_blocks_count,
                                    get_tf_modbus_tcp_client_transaction_result_name(result),
                                    static_cast<int>(result),
                                    error_message != nullptr ? " / " : "",
                                    error_message != nullptr ? error_message : "");

            if (execution->callback) {
                execution->callback(false);
            }

            delete execution;
            return;
        }

        ++execution->index;

        write_next(execution); // FIXME: maybe add a little delay between writes to avoid bursts?
    });
}
