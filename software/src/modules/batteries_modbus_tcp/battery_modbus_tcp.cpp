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

#include "gcc_warnings.h"

struct Execution {
    TFModbusTCPSharedClient *client = nullptr;
    const BatteryModbusTCP::TableSpec *table = nullptr;
    size_t index = 0;
    BatteryModbusTCP::ExecuteCallback callback = nullptr;
};

BatteryModbusTCP::TableSpec *BatteryModbusTCP::init_table(const Config *config)
{
    const Config *register_blocks_config = static_cast<const Config *>(config->get("register_blocks"));
    size_t register_blocks_count         = register_blocks_config->count();

    if (register_blocks_count == 0) {
        return nullptr;
    }

    TableSpec *table = static_cast<TableSpec *>(malloc(sizeof(TableSpec)));

    table->device_address        = static_cast<uint8_t>(config->get("device_address")->asUint());
    table->register_blocks       = static_cast<RegisterBlockSpec *>(malloc(sizeof(RegisterBlockSpec) * register_blocks_count));
    table->register_blocks_count = register_blocks_count;

    for (size_t i = 0; i < register_blocks_count; ++i) {
        auto register_block_config = register_blocks_config->get(i);

        table->register_blocks[i].function_code = register_block_config->get("func")->asEnum<ModbusFunctionCode>();
        table->register_blocks[i].start_address = static_cast<uint16_t>(register_block_config->get("addr")->asUint());

        auto values_config    = register_block_config->get("vals");
        uint16_t values_count = static_cast<uint16_t>(values_config->count());
        size_t values_byte_count;

        if (table->register_blocks[i].function_code == ModbusFunctionCode::WriteSingleCoil
         || table->register_blocks[i].function_code == ModbusFunctionCode::WriteMultipleCoils) {
            values_byte_count = (values_count + 7u) / 8u;
        }
        else {
            values_byte_count = values_count * 2u;
        }

        table->register_blocks[i].values_buffer = malloc(values_byte_count);
        table->register_blocks[i].values_count  = values_count;

        if (table->register_blocks[i].function_code == ModbusFunctionCode::WriteSingleCoil
         || table->register_blocks[i].function_code == ModbusFunctionCode::WriteMultipleCoils) {
            uint8_t *values_buffer = static_cast<uint8_t *>(table->register_blocks[i].values_buffer);

            values_buffer[values_byte_count - 1] = 0;

            for (uint16_t k = 0; k < values_count; ++k) {
                uint8_t mask = static_cast<uint8_t>(1u << (k % 8));

                if (values_config->get(k)->asUint() != 0) {
                    values_buffer[k / 8] |= mask;
                }
                else {
                    values_buffer[k / 8] &= ~mask;
                }
            }
        }
        else {
            uint16_t *values_buffer = static_cast<uint16_t *>(table->register_blocks[i].values_buffer);

            for (uint16_t k = 0; k < values_count; ++k) {
                values_buffer[k] = static_cast<uint16_t>(values_config->get(k)->asUint());
            }
        }
    }

    return table;
}

void BatteryModbusTCP::free_table(BatteryModbusTCP::TableSpec *table)
{
    if (table == nullptr) {
        return;
    }

    for (size_t i = 0; i < table->register_blocks_count; ++i) {
        free(table->register_blocks[i].values_buffer);
    }

    free(table->register_blocks);
    free(table);
}

static void execute_finish(Execution *execution, const char *error)
{
    execution->callback(error);
    delete execution;
}

static void execute_next(Execution *execution)
{
    if (execution->index >= execution->table->register_blocks_count) {
        execute_finish(execution, nullptr);
        return;
    }

    BatteryModbusTCP::RegisterBlockSpec *register_block = &execution->table->register_blocks[execution->index];
    TFModbusTCPFunctionCode function_code;
    uint16_t data_count;
    void *buffer;
    void *buffer_to_free = nullptr;
    char error[128];

    switch (register_block->function_code) {
    case ModbusFunctionCode::WriteSingleCoil:
        function_code = TFModbusTCPFunctionCode::WriteSingleCoil;
        data_count = register_block->values_count;
        buffer = register_block->values_buffer;
        break;

    case ModbusFunctionCode::WriteSingleRegister:
        function_code = TFModbusTCPFunctionCode::WriteSingleRegister;
        data_count = register_block->values_count;
        buffer = register_block->values_buffer;
        break;

    case ModbusFunctionCode::WriteMultipleCoils:
        function_code = TFModbusTCPFunctionCode::WriteMultipleCoils;
        data_count = register_block->values_count;
        buffer = register_block->values_buffer;
        break;

    case ModbusFunctionCode::WriteMultipleRegisters:
        function_code = TFModbusTCPFunctionCode::WriteMultipleRegisters;
        data_count = register_block->values_count;
        buffer = register_block->values_buffer;
        break;

    case ModbusFunctionCode::MaskWriteRegister:
        function_code = TFModbusTCPFunctionCode::MaskWriteRegister;
        data_count = register_block->values_count;
        buffer = register_block->values_buffer;
        break;

    case ModbusFunctionCode::ReadMaskWriteSingleRegister:
    case ModbusFunctionCode::ReadMaskWriteMultipleRegisters:
        function_code = TFModbusTCPFunctionCode::ReadHoldingRegisters;
        data_count = register_block->values_count / 2;
        buffer = malloc(sizeof(uint16_t) * data_count);
        buffer_to_free = buffer;

        if (buffer == nullptr) {
            execute_finish(execution, "Could not allocate read buffer");
            return;
        }

        break;

    case ModbusFunctionCode::ReadCoils:
    case ModbusFunctionCode::ReadDiscreteInputs:
    case ModbusFunctionCode::ReadHoldingRegisters:
    case ModbusFunctionCode::ReadInputRegisters:
    default:
        snprintf(error, sizeof(error), "Unsupported function code: %u", static_cast<uint8_t>(register_block->function_code));
        execute_finish(execution, error);
        return;
    }

    static_cast<TFModbusTCPSharedClient *>(execution->client)->transact(execution->table->device_address,
                                                                        function_code,
                                                                        register_block->start_address,
                                                                        data_count,
                                                                        buffer,
                                                                        2_s,
    [execution, register_block, data_count, buffer, buffer_to_free](TFModbusTCPClientTransactionResult result, const char *error_message) {
        if (result != TFModbusTCPClientTransactionResult::Success) {
            char transact_error[128];

            snprintf(transact_error, sizeof(transact_error),
                     "Action execution failed at %zu of %zu: %s (%d)%s%s",
                     execution->index + 1, execution->table->register_blocks_count,
                     get_tf_modbus_tcp_client_transaction_result_name(result),
                     static_cast<int>(result),
                     error_message != nullptr ? " / " : "",
                     error_message != nullptr ? error_message : "");

            free(buffer_to_free);
            execute_finish(execution, transact_error);
            return;
        }

        if (register_block->function_code == ModbusFunctionCode::ReadMaskWriteSingleRegister
         || register_block->function_code == ModbusFunctionCode::ReadMaskWriteMultipleRegisters) {
            uint16_t *masks = static_cast<uint16_t *>(register_block->values_buffer);
            uint16_t *values = static_cast<uint16_t *>(buffer);

            for (uint16_t i = 0; i < data_count; ++i) {
                values[i] = (values[i] & masks[i * 2]) | (masks[i * 2 + 1] & ~masks[i * 2]);
            }

            TFModbusTCPFunctionCode step2_function_code;

            if (register_block->function_code == ModbusFunctionCode::ReadMaskWriteSingleRegister) {
                step2_function_code = TFModbusTCPFunctionCode::WriteSingleRegister;
            }
            else {
                step2_function_code = TFModbusTCPFunctionCode::WriteMultipleRegisters;
            }

            static_cast<TFModbusTCPSharedClient *>(execution->client)->transact(execution->table->device_address,
                                                                                step2_function_code,
                                                                                register_block->start_address,
                                                                                data_count,
                                                                                buffer,
                                                                                2_s,
            [execution, buffer_to_free](TFModbusTCPClientTransactionResult step2_result, const char *step2_error_message) {
                if (step2_result != TFModbusTCPClientTransactionResult::Success) {
                    char step2_transact_error[128];

                    snprintf(step2_transact_error, sizeof(step2_transact_error),
                             "Action execution (step 2) failed at %zu of %zu: %s (%d)%s%s",
                             execution->index + 1, execution->table->register_blocks_count,
                             get_tf_modbus_tcp_client_transaction_result_name(step2_result),
                             static_cast<int>(step2_result),
                             step2_error_message != nullptr ? " / " : "",
                             step2_error_message != nullptr ? step2_error_message : "");

                    free(buffer_to_free);
                    execute_finish(execution, step2_transact_error);
                    return;
                }

                ++execution->index;

                free(buffer_to_free);
                execute_next(execution); // FIXME: maybe add a little delay between writes to avoid bursts?
            });
        }
        else {
            ++execution->index;

            free(buffer_to_free);
            execute_next(execution); // FIXME: maybe add a little delay between writes to avoid bursts?
        }
    });
}

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
            tables[static_cast<uint32_t>(IBattery::Action::PermitGridCharge)]         = init_table(static_cast<const Config *>(table_config->get("permit_grid_charge")));
            tables[static_cast<uint32_t>(IBattery::Action::RevokeGridChargeOverride)] = init_table(static_cast<const Config *>(table_config->get("revoke_grid_charge_override")));
            tables[static_cast<uint32_t>(IBattery::Action::ForbidDischarge)]          = init_table(static_cast<const Config *>(table_config->get("forbid_discharge")));
            tables[static_cast<uint32_t>(IBattery::Action::RevokeDischargeOverride)]  = init_table(static_cast<const Config *>(table_config->get("revoke_discharge_override")));
            tables[static_cast<uint32_t>(IBattery::Action::ForbidCharge)]             = init_table(static_cast<const Config *>(table_config->get("forbid_charge")));
            tables[static_cast<uint32_t>(IBattery::Action::RevokeChargeOverride)]     = init_table(static_cast<const Config *>(table_config->get("revoke_charge_override")));
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

void BatteryModbusTCP::get_repeat_intervals(uint16_t intervals_s[6]) const
{
    for (size_t i = 0; i < 6; i++) {
        intervals_s[i] = 60; // TODO Use values from config. In seconds, 0 = no repeat.
    }
}

[[gnu::const]]
bool BatteryModbusTCP::supports_action(Action /*action*/) const
{
    return true;
}

void BatteryModbusTCP::start_action(Action action, std::function<void(bool)> &&callback)
{
    const TableSpec *table = tables[static_cast<uint32_t>(action)];

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

    execute(static_cast<TFModbusTCPSharedClient *>(connected_client), table, [this, callback](const char *error) {
        if (error != nullptr) {
            logger.printfln_battery("%s", error);
        }

        if (callback) {
            callback(error == nullptr);
        }
    });
}

void BatteryModbusTCP::connect_callback()
{
}

void BatteryModbusTCP::disconnect_callback()
{
}

void BatteryModbusTCP::execute(TFModbusTCPSharedClient *client, const TableSpec *table, ExecuteCallback &&callback)
{
    Execution *execution = new Execution;

    execution->client = client;
    execution->table = table;
    execution->index = 0;
    execution->callback = std::move(callback);

    execute_next(execution);
}
