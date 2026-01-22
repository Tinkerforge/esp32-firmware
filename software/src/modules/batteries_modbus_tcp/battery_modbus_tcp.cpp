/* esp32-firmware
 * Copyright (C) 2024-2025 Matthias Bolte <matthias@tinkerforge.com>
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

#define trace(fmt, ...) \
    do { \
        batteries_modbus_tcp.trace_timestamp(); \
        logger.tracefln_plain(batteries_modbus_tcp.trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

static const char *get_battery_mode_display_name(BatteryMode value, Language language = Language::English)
{
    switch (value) {
    case BatteryMode::None:             return language == Language::English ? "none"                                : "nichts";
    case BatteryMode::Block:            return language == Language::English ? "block charge, block discharge"       : "Laden blockieren, Entladen blockieren";
    case BatteryMode::Normal:           return language == Language::English ? "charge normally, discharge normally" : "normal Laden, normal Entladen";
    case BatteryMode::ChargeFromExcess: return language == Language::English ? "charge normally, block discharge"    : "normal Laden, Entladen blockieren";
    case BatteryMode::ChargeFromGrid:   return language == Language::English ? "force charge, block discharge"       : "Laden erzwingen, Entladen blockieren";
    case BatteryMode::DischargeToLoad:  return language == Language::English ? "block charge, discharge normally"    : "Laden blockieren, normal Entladen";
    case BatteryMode::DischargeToGrid:  return language == Language::English ? "block charge, force discharge"       : "Laden blockieren, Entladen erzwingen";
    default:                            return language == Language::English ? "unknown"                             : "unbekannt";
    }
}

static char get_battery_mode_as_char(BatteryMode mode)
{
    if (mode == BatteryMode::None) {
        return 'n';
    }

    return '0' + static_cast<char>(mode);
}

void BatteryModbusTCP::load_custom_table(BatteryModbusTCP::TableSpec **table_ptr, const Config *config)
{
    const Config *register_blocks_config = static_cast<const Config *>(config->get("register_blocks"));
    size_t register_blocks_count         = register_blocks_config->count();
    BatteryModbusTCP::TableSpec *table   = static_cast<BatteryModbusTCP::TableSpec *>(malloc_psram_or_dram(sizeof(BatteryModbusTCP::TableSpec)));

    table->register_blocks       = static_cast<BatteryModbusTCP::RegisterBlockSpec *>(malloc_psram_or_dram(sizeof(BatteryModbusTCP::RegisterBlockSpec) * register_blocks_count));
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
        BatteryModbusTCP::RegisterBlockSpec *register_block = &table->register_blocks[i];

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

    *table_ptr = table;
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

[[gnu::format(__printf__, 3, 4)]]
static void table_writer_logfln(BatteryModbusTCP::TableWriter *writer, bool error, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    writer->vlogfln(error, fmt, args);
    va_end(args);
}

static void next_table_writer_step(BatteryModbusTCP::TableWriter *writer);

static void last_table_writer_step(BatteryModbusTCP::TableWriter *writer, bool success)
{
    task_scheduler.cancel(writer->task_id);

    if (writer->delete_requested) {
        delete writer;
        return;
    }

    millis_t delay = 5_s;

    if (success) {
        delay = seconds_t{writer->repeat_interval};
    }

    ++writer->repeat_count;
    writer->task_id = 0;
    writer->index = 0;

    if (delay != 0_s) {
        writer->task_id = task_scheduler.scheduleOnce([writer]() {
            trace("b%lu t%d ww m%c r%zu",
                  writer->slot,
                  writer->test ? 1 : 0,
                  get_battery_mode_as_char(writer->mode),
                  writer->repeat_count);

            table_writer_logfln(writer, false,
                                writer->language == Language::English
                                ? "Setting mode \"%s\" (repeat %zu)"
                                : "Setze Modus \"%s\" (Wiederholdung %zu)",
                                get_battery_mode_display_name(writer->mode, writer->language), writer->repeat_count);
            next_table_writer_step(writer);
        }, delay);
    }
    else {
        writer->finished();
    }
}

static void next_table_writer_step(BatteryModbusTCP::TableWriter *writer)
{
    if (writer->index >= writer->table->register_blocks_count) {
        last_table_writer_step(writer, true);
        return;
    }

    if (writer->delete_requested) {
        last_table_writer_step(writer, false);
        return;
    }

    const BatteryModbusTCP::RegisterBlockSpec *register_block = &writer->table->register_blocks[writer->index];
    TFModbusTCPFunctionCode function_code;
    uint16_t data_count;
    const void *buffer;
    void *buffer_to_free = nullptr;

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
            table_writer_logfln(writer, true,
                                writer->language == Language::English
                                ? "Could not allocate read buffer"
                                : "Konnte Lesepuffer nicht allokieren");
            last_table_writer_step(writer, false);
            return;
        }

        break;

    case ModbusFunctionCode::ReadCoils:
    case ModbusFunctionCode::ReadDiscreteInputs:
    case ModbusFunctionCode::ReadHoldingRegisters:
    case ModbusFunctionCode::ReadInputRegisters:
    default:
        table_writer_logfln(writer, true,
                            writer->language == Language::English
                            ? "Unsupported function code: %u"
                            : "Funktionscode nicht unterstüzt: %u",
                            static_cast<uint8_t>(register_block->function_code));
        last_table_writer_step(writer, false);
        return;
    }

    writer->transact_pending = true;

    static_cast<TFModbusTCPSharedClient *>(writer->client)->transact(writer->device_address,
                                                                     function_code,
                                                                     register_block->start_address,
                                                                     data_count,
                                                                     const_cast<void *>(buffer),
                                                                     2_s,
    [writer, register_block, data_count, buffer, buffer_to_free](TFModbusTCPClientTransactionResult result, const char *error_message) {
        bool has_step2 = register_block->function_code == ModbusFunctionCode::ReadMaskWriteSingleRegister
                      || register_block->function_code == ModbusFunctionCode::ReadMaskWriteMultipleRegisters;

        if (result != TFModbusTCPClientTransactionResult::Success) {
            trace("b%lu t%d ww m%c i%zu/%zu%s e%d%s%s",
                  writer->slot,
                  writer->test ? 1 : 0,
                  get_battery_mode_as_char(writer->mode),
                  writer->index + 1,
                  writer->table->register_blocks_count,
                  has_step2 ? " s1/2" : "",
                  static_cast<int>(result),
                  error_message != nullptr ? " / " : "",
                  error_message != nullptr ? error_message : "");

            table_writer_logfln(writer,
                                true,
                                writer->language == Language::English
                                ? "Setting mode \"%s\" failed at %zu of %zu: %s (%d)%s%s"
                                : "Setzen des Modus \"%s\" schlug fehl bei %zu von %zu: %s (%d)%s%s",
                                get_battery_mode_display_name(writer->mode, writer->language),
                                writer->index + 1, writer->table->register_blocks_count,
                                get_tf_modbus_tcp_client_transaction_result_name(result),
                                static_cast<int>(result),
                                error_message != nullptr ? " / " : "",
                                error_message != nullptr ? error_message : "");

            writer->transact_pending = false;

            free(buffer_to_free);
            last_table_writer_step(writer, false);
            return;
        }

        if (has_step2) {
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

            static_cast<TFModbusTCPSharedClient *>(writer->client)->transact(writer->device_address,
                                                                             step2_function_code,
                                                                             register_block->start_address,
                                                                             data_count,
                                                                             const_cast<void *>(buffer),
                                                                             2_s,
            [writer, buffer_to_free](TFModbusTCPClientTransactionResult step2_result, const char *step2_error_message) {
                if (step2_result != TFModbusTCPClientTransactionResult::Success) {
                    trace("b%lu t%d ww m%c i%zu/%zu s2/2 e%d%s%s",
                          writer->slot,
                          writer->test ? 1 : 0,
                          get_battery_mode_as_char(writer->mode),
                          writer->index + 1,
                          writer->table->register_blocks_count,
                          static_cast<int>(step2_result),
                          step2_error_message != nullptr ? " / " : "",
                          step2_error_message != nullptr ? step2_error_message : "");

                    table_writer_logfln(writer,
                                        true,
                                        writer->language == Language::English
                                        ? "Setting mode \"%s\" failed at %zu of %zu: %s (%d)%s%s"
                                        : "Setzen des Modus \"%s\" (Schritt 2) schlug fehl bei %zu von %zu: %s (%d)%s%s",
                                        get_battery_mode_display_name(writer->mode, writer->language),
                                        writer->index + 1, writer->table->register_blocks_count,
                                        get_tf_modbus_tcp_client_transaction_result_name(step2_result),
                                        static_cast<int>(step2_result),
                                        step2_error_message != nullptr ? " / " : "",
                                        step2_error_message != nullptr ? step2_error_message : "");

                    writer->transact_pending = false;

                    free(buffer_to_free);
                    last_table_writer_step(writer, false);
                    return;
                }

                ++writer->index;
                writer->transact_pending = false;

                free(buffer_to_free);
                next_table_writer_step(writer); // FIXME: maybe add a little delay between writes to avoid bursts?
            });
        }
        else {
            ++writer->index;
            writer->transact_pending = false;

            free(buffer_to_free);
            next_table_writer_step(writer); // FIXME: maybe add a little delay between writes to avoid bursts?
        }
    });
}

BatteryModbusTCP::TableWriter *BatteryModbusTCP::create_table_writer(uint32_t slot,
                                                                     bool test,
                                                                     TFModbusTCPSharedClient *client,
                                                                     uint8_t device_address,
                                                                     uint16_t repeat_interval, // seconds
                                                                     BatteryMode mode,
                                                                     TableSpec *table,
                                                                     TableWriterVLogFLnFunction &&vlogfln,
                                                                     TableWriterFinishedFunction &&finished,
                                                                     Language language /*= Language::English*/)
{
    trace("b%lu t%d wc m%c",
          slot,
          test ? 1 : 0,
          get_battery_mode_as_char(mode));

    TableWriter *writer = new TableWriter;

    writer->language = language;
    writer->slot = slot;
    writer->client = client;
    writer->device_address = device_address;
    writer->repeat_interval = repeat_interval;
    writer->mode = mode;
    writer->table = table;
    writer->vlogfln = std::move(vlogfln);
    writer->finished = std::move(finished);
    writer->test = test;

    if (table->register_blocks_count > 0) {
        writer->task_id = task_scheduler.scheduleOnce([writer]() {
            trace("b%lu t%d ww m%c %c",
                  writer->slot,
                  writer->test ? 1 : 0,
                  get_battery_mode_as_char(writer->mode),
                  writer->repeat_interval > 0 ? 'f' : 'o');

            if (writer->repeat_interval > 0) {
                table_writer_logfln(writer, false,
                                    writer->language == Language::English
                                    ? "Setting mode \"%s\" (will repeat in %u second%s)"
                                    : "Setze Modus \"%s\" (Wiederholung in %u Sekunde%s)",
                                    get_battery_mode_display_name(writer->mode, writer->language),
                                    writer->repeat_interval,
                                    writer->repeat_interval > 1 ? (writer->language == Language::English ? "s" : "n") : "");
            }
            else {
                table_writer_logfln(writer, false,
                                    writer->language == Language::English
                                    ? "Setting mode \"%s\" (once)"
                                    : "Setze Modus \"%s\" (einmalig)",
                                    get_battery_mode_display_name(writer->mode, writer->language));
            }

            next_table_writer_step(writer);
        });
    }

    return writer;
}

void BatteryModbusTCP::destroy_table_writer(BatteryModbusTCP::TableWriter *writer)
{
    if (writer == nullptr) {
        return;
    }

    trace("b%lu t%d wd m%c",
          writer->slot,
          writer->test ? 1 : 0,
          get_battery_mode_as_char(writer->mode));

    if (writer->transact_pending) {
        writer->delete_requested = true;
        return;
    }

    task_scheduler.cancel(writer->task_id);
    delete writer;
}

BatteryClassID BatteryModbusTCP::get_class() const
{
    return BatteryClassID::ModbusTCP;
}

void BatteryModbusTCP::setup(const Config &ephemeral_config)
{
    host     = ephemeral_config.get("host")->asString();
    port     = static_cast<uint16_t>(ephemeral_config.get("port")->asUint());
    table_id = ephemeral_config.get("table")->getTag<BatteryModbusTCPTableID>();

    const Config *table_config = static_cast<const Config *>(ephemeral_config.get("table")->get());
    const Config *battery_modes_config;

    switch (table_id) {
    case BatteryModbusTCPTableID::None:
        logger.printfln_battery("No table selected");
        return;

    case BatteryModbusTCPTableID::Custom:
        device_address = table_config->get("device_address")->asUint8();
        repeat_interval = table_config->get("repeat_interval")->asUint16();
        battery_modes_config = static_cast<const Config *>(table_config->get("battery_modes"));

        for (size_t i = static_cast<size_t>(BatteryMode::Block); i <= static_cast<size_t>(BatteryMode::_max); ++i) {
            load_custom_table(&tables[i], static_cast<const Config *>(battery_modes_config->get(i)));
        }

        break;

    case BatteryModbusTCPTableID::VictronEnergyGX:
        device_address = table_config->get("device_address")->asUint8();
        load_victron_energy_gx_tables(tables, &repeat_interval, table_config);
        break;

    case BatteryModbusTCPTableID::DeyeHybridInverter:
        device_address = table_config->get("device_address")->asUint8();
        load_deye_hybrid_inverter_tables(tables, &repeat_interval, table_config);
        break;

    case BatteryModbusTCPTableID::AlphaESSHybridInverter:
        device_address = table_config->get("device_address")->asUint8();
        load_alpha_ess_hybrid_inverter_tables(tables, &repeat_interval, table_config);
        break;

    case BatteryModbusTCPTableID::HaileiHybridInverter:
        device_address = table_config->get("device_address")->asUint8();
        load_hailei_hybrid_inverter_tables(tables, &repeat_interval, table_config);
        break;

    case BatteryModbusTCPTableID::SungrowHybridInverter:
        device_address = table_config->get("device_address")->asUint8();
        load_sungrow_hybrid_inverter_tables(tables, &repeat_interval, table_config);
        break;

    case BatteryModbusTCPTableID::SMAHybridInverter:
        device_address = table_config->get("device_address")->asUint8();
        load_sma_hybrid_inverter_tables(tables, &repeat_interval, table_config);
        break;

    default:
        logger.printfln_battery("Unknown table: %u", static_cast<uint8_t>(table_id));
        return;
    }
}

void BatteryModbusTCP::register_events()
{
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

void BatteryModbusTCP::pre_reboot()
{
    stop_connection();
}

[[gnu::const]]
bool BatteryModbusTCP::supports_mode(BatteryMode mode) const
{
    if (mode == BatteryMode::None) {
        return false;
    }

    return tables[static_cast<size_t>(mode)] != nullptr;
}

void BatteryModbusTCP::set_mode(BatteryMode mode)
{
    if (requested_mode == mode) {
        return;
    }

    requested_mode = mode;
    finished = false;

    update_active_mode();
}

void BatteryModbusTCP::set_paused(bool paused_)
{
    if (this->paused == paused_) {
        return;
    }

    this->paused = paused_;

    update_active_mode();
}

void BatteryModbusTCP::connect_callback(TFGenericTCPClientConnectResult result)
{
    trace("b%lu t0 cc%d", slot, static_cast<int>(result));

    if (result != TFGenericTCPClientConnectResult::Connected) {
        return;
    }

    update_active_mode();
}

void BatteryModbusTCP::disconnect_callback(TFGenericTCPClientDisconnectReason reason)
{
    trace("b%lu t0 cd%d", slot, static_cast<int>(reason));

    update_active_mode();
}

void BatteryModbusTCP::update_active_mode()
{
    BatteryMode next_mode = requested_mode;

    if (connected_client == nullptr || requested_mode == BatteryMode::None || paused || finished) {
        next_mode = BatteryMode::None;
    }

    if (active_mode == next_mode) {
        return;
    }

    TableSpec *table = nullptr;

    if (next_mode != BatteryMode::None) {
        table = tables[static_cast<size_t>(next_mode)];
    }

    if (table == nullptr) {
        next_mode = BatteryMode::None;
    }

    destroy_table_writer(active_writer);
    active_writer = nullptr;

    trace("b%lu t0 %s r%c%s%s m%c->%c",
          slot,
          connected_client != nullptr ? "ce" : "nc",
          get_battery_mode_as_char(requested_mode),
          paused ? " p" : "",
          finished ? " f" : "",
          get_battery_mode_as_char(active_mode),
          get_battery_mode_as_char(next_mode));

    active_mode = next_mode;

    if (table != nullptr) {
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=format"
#endif
        active_writer = create_table_writer(slot, false, static_cast<TFModbusTCPSharedClient *>(connected_client), device_address, repeat_interval, active_mode, table,
        [this](bool error, const char *fmt, va_list args) {
            if (!error) {
                return;
            }

            char message[256];

            vsnprintf(message, sizeof(message), fmt, args);
            logger.printfln_battery("%s", message);
        },
        [this]() {
            finished = true;

            update_active_mode();
        });
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    }
}
