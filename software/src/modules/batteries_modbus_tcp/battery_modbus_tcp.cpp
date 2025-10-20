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
#include "battery_modbus_tcp_specs.h"
#include "tools/memory.h"

#include "gcc_warnings.h"

struct Execution {
    uint8_t device_address = 0;
    TFModbusTCPSharedClient *client = nullptr;
    BatteryModbusTCP::TableSpec *table = nullptr;
    size_t index = 0;
    BatteryModbusTCP::ExecuteCallback callback = nullptr;
};

BatteryModbusTCP::TableSpec *BatteryModbusTCP::load_custom_table(const Config *config, bool load_repeat_interval)
{
    const Config *register_blocks_config = static_cast<const Config *>(config->get("register_blocks"));
    size_t register_blocks_count         = register_blocks_config->count();
    TableSpec *table                     = static_cast<TableSpec *>(malloc_psram_or_dram(sizeof(TableSpec)));

    if (load_repeat_interval) {
        table->repeat_interval = config->get("repeat_interval")->asUint16();
    }
    else {
        table->repeat_interval = 0;
    }

    table->register_blocks       = static_cast<RegisterBlockSpec *>(malloc_psram_or_dram(sizeof(RegisterBlockSpec) * register_blocks_count));
    table->register_blocks_count = register_blocks_count;

    size_t total_buffer_length = 0; // bytes

    for (size_t i = 0; i < register_blocks_count; ++i) {
        auto register_block_config = register_blocks_config->get(i);
        ModbusFunctionCode function_code = register_block_config->get("func")->asEnum<ModbusFunctionCode>();
        uint16_t values_count = static_cast<uint16_t>(register_block_config->get("vals")->count());

        if (function_code == ModbusFunctionCode::WriteSingleCoil
         || function_code == ModbusFunctionCode::WriteMultipleCoils) {
            total_buffer_length += (values_count + 7u) / 8u;
        }
        else {
            total_buffer_length += values_count * 2u;
        }
    }

    uint8_t *total_buffer = nullptr;
    size_t total_buffer_offset = 0; // bytes

    if (register_blocks_count > 0) {
        total_buffer = static_cast<uint8_t *>(malloc_psram_or_dram(total_buffer_length));
    }

    for (size_t i = 0; i < register_blocks_count; ++i) {
        auto register_block_config = register_blocks_config->get(i);
        RegisterBlockSpec *register_block = &table->register_blocks[i];

        register_block->function_code = register_block_config->get("func")->asEnum<ModbusFunctionCode>();
        register_block->start_address = static_cast<uint16_t>(register_block_config->get("addr")->asUint());

        auto values_config    = register_block_config->get("vals");
        uint16_t values_count = static_cast<uint16_t>(values_config->count());
        size_t buffer_length; // bytes

        register_block->buffer       = total_buffer + total_buffer_offset;
        register_block->values_count = values_count;

        if (register_block->function_code == ModbusFunctionCode::WriteSingleCoil
         || register_block->function_code == ModbusFunctionCode::WriteMultipleCoils) {
            buffer_length = (values_count + 7u) / 8u;
        }
        else {
            buffer_length = values_count * 2u;
        }

        if (register_block->function_code == ModbusFunctionCode::WriteSingleCoil
         || register_block->function_code == ModbusFunctionCode::WriteMultipleCoils) {
            uint8_t *coils_buffer = const_cast<uint8_t *>(static_cast<const uint8_t *>(register_block->buffer));

            coils_buffer[buffer_length - 1] = 0;

            for (uint16_t k = 0; k < values_count; ++k) {
                uint8_t mask = static_cast<uint8_t>(1u << (k % 8));

                if (values_config->get(k)->asUint() != 0) {
                    coils_buffer[k / 8] |= mask;
                }
                else {
                    coils_buffer[k / 8] &= ~mask;
                }
            }
        }
        else {
            uint16_t *registers_buffer = const_cast<uint16_t *>(static_cast<const uint16_t *>(register_block->buffer));

            for (uint16_t k = 0; k < values_count; ++k) {
                registers_buffer[k] = values_config->get(k)->asUint16();
            }
        }

        total_buffer_offset += buffer_length;
    }

    return table;
}

void BatteryModbusTCP::free_table(BatteryModbusTCP::TableSpec *table)
{
    if (table == nullptr || address_is_in_rodata(table)) {
        return;
    }

    if (table->register_blocks_count > 0) {
        free_any(table->register_blocks[0].buffer);
    }

    free_any(table->register_blocks);
    free_any(table);
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

    const BatteryModbusTCP::RegisterBlockSpec *register_block = &execution->table->register_blocks[execution->index];
    TFModbusTCPFunctionCode function_code;
    uint16_t data_count;
    const void *buffer;
    void *buffer_to_free = nullptr;
    char error[128];

    switch (register_block->function_code) {
    case ModbusFunctionCode::WriteSingleCoil:
        function_code = TFModbusTCPFunctionCode::WriteSingleCoil;
        data_count = register_block->values_count;
        buffer = register_block->buffer;
        break;

    case ModbusFunctionCode::WriteSingleRegister:
        function_code = TFModbusTCPFunctionCode::WriteSingleRegister;
        data_count = register_block->values_count;
        buffer = register_block->buffer;
        break;

    case ModbusFunctionCode::WriteMultipleCoils:
        function_code = TFModbusTCPFunctionCode::WriteMultipleCoils;
        data_count = register_block->values_count;
        buffer = register_block->buffer;
        break;

    case ModbusFunctionCode::WriteMultipleRegisters:
        function_code = TFModbusTCPFunctionCode::WriteMultipleRegisters;
        data_count = register_block->values_count;
        buffer = register_block->buffer;
        break;

    case ModbusFunctionCode::MaskWriteRegister:
        function_code = TFModbusTCPFunctionCode::MaskWriteRegister;
        data_count = register_block->values_count;
        buffer = register_block->buffer;
        break;

    case ModbusFunctionCode::ReadMaskWriteSingleRegister:
    case ModbusFunctionCode::ReadMaskWriteMultipleRegisters:
        function_code = TFModbusTCPFunctionCode::ReadHoldingRegisters;
        data_count = register_block->values_count / 2;
        buffer_to_free = malloc(sizeof(uint16_t) * data_count);
        buffer = buffer_to_free;

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

    static_cast<TFModbusTCPSharedClient *>(execution->client)->transact(execution->device_address,
                                                                        function_code,
                                                                        register_block->start_address,
                                                                        data_count,
                                                                        const_cast<void *>(buffer),
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
            const uint16_t *masks = static_cast<const uint16_t *>(register_block->buffer);
            uint16_t *values = static_cast<uint16_t *>(buffer_to_free);

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

            static_cast<TFModbusTCPSharedClient *>(execution->client)->transact(execution->device_address,
                                                                                step2_function_code,
                                                                                register_block->start_address,
                                                                                data_count,
                                                                                const_cast<void *>(buffer),
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

    const Config *table_config = static_cast<const Config *>(ephemeral_config.get("table")->get());

    switch (table_id) {
    case BatteryModbusTCPTableID::None:
        logger.printfln_battery("No table selected");
        return;

    case BatteryModbusTCPTableID::Custom:
        device_address = table_config->get("device_address")->asUint8();

        // FIXME: leaking this, because as of right now battery instances don't get destroyed
        tables[static_cast<size_t>(BatteryAction::PermitGridCharge)]         = load_custom_table(static_cast<const Config *>(table_config->get("permit_grid_charge")));
        tables[static_cast<size_t>(BatteryAction::RevokeGridChargeOverride)] = load_custom_table(static_cast<const Config *>(table_config->get("revoke_grid_charge_override")));
        tables[static_cast<size_t>(BatteryAction::ForbidDischarge)]          = load_custom_table(static_cast<const Config *>(table_config->get("forbid_discharge")));
        tables[static_cast<size_t>(BatteryAction::RevokeDischargeOverride)]  = load_custom_table(static_cast<const Config *>(table_config->get("revoke_discharge_override")));
        tables[static_cast<size_t>(BatteryAction::ForbidCharge)]             = load_custom_table(static_cast<const Config *>(table_config->get("forbid_charge")));
        tables[static_cast<size_t>(BatteryAction::RevokeChargeOverride)]     = load_custom_table(static_cast<const Config *>(table_config->get("revoke_charge_override")));
        break;

    case BatteryModbusTCPTableID::VictronEnergyGX:
        device_address = table_config->get("device_address")->asUint8();
        load_victron_energy_gx_tables(tables, table_config);
        break;

    case BatteryModbusTCPTableID::DeyeHybridInverter:
        device_address = table_config->get("device_address")->asUint8();
        load_deye_hybrid_inverter_tables(tables, table_config);
        break;

    case BatteryModbusTCPTableID::AlphaESSHybridInverter:
        device_address = table_config->get("device_address")->asUint8();
        load_alpha_ess_hybrid_inverter_tables(tables, table_config);
        break;

    case BatteryModbusTCPTableID::HaileiHybridInverter:
        device_address = table_config->get("device_address")->asUint8();
        load_hailei_hybrid_inverter_tables(tables, table_config);
        break;

    case BatteryModbusTCPTableID::SungrowHybridInverter:
        device_address = table_config->get("device_address")->asUint8();
        load_sungrow_hybrid_inverter_tables(tables, table_config);
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
        intervals_s[i] = tables[i]->repeat_interval; // In seconds, 0 = no repeat.
    }
}

[[gnu::const]]
bool BatteryModbusTCP::supports_action(BatteryAction /*action*/) const
{
    return true;
}

void BatteryModbusTCP::start_action(BatteryAction action, std::function<void(bool)> &&callback)
{
    TableSpec *table = tables[static_cast<size_t>(action)];

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

    execute(static_cast<TFModbusTCPSharedClient *>(connected_client), device_address, table, [this, callback](const char *error) {
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

void BatteryModbusTCP::execute(TFModbusTCPSharedClient *client, uint8_t device_address, TableSpec *table, ExecuteCallback &&callback)
{
    Execution *execution = new Execution;

    execution->device_address = device_address;
    execution->client = client;
    execution->table = table;
    execution->index = 0;
    execution->callback = std::move(callback);

    execute_next(execution);
}
