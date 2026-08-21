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

#include <stdio.h>
#include <TFModbusTCPClient.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "generated/battery_modbus_tcp_specs.h"
#include "tools/memory.h"

#include "gcc_warnings.h"

#define trace(fmt, ...) \
    do { \
        batteries_modbus_tcp.trace_timestamp(); \
        logger.tracefln_plain(batteries_modbus_tcp.trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

static char get_battery_mode_as_char(BatteryMode mode)
{
    if (mode == BatteryMode::None) {
        return 'n';
    }

    return '0' + static_cast<char>(mode);
}

static const char *get_battery_mode_description(BatteryMode mode, BatteryMode effective_mode, Language language, char *buf, size_t buf_len)
{
    if (mode == effective_mode) {
        snprintf(buf, buf_len, "\"%s\"", BatteryModbusTCP::get_battery_mode_display_name(mode, language));
    }
    else {
        snprintf(buf, buf_len,
                 language == Language::English
                 ? "\"%s\", effective \"%s\""
                 : "\"%s\", effektiv \"%s\"",
                 BatteryModbusTCP::get_battery_mode_display_name(mode, language),
                 BatteryModbusTCP::get_battery_mode_display_name(effective_mode, language));
    }

    return buf;
}

[[gnu::const]]
const char *BatteryModbusTCP::get_battery_mode_display_name(BatteryMode mode, Language language)
{
    switch (mode) {
    case BatteryMode::None:           esp_system_abortf<64>("Invalid battery mode for display name lookup: %d", static_cast<int>(mode));
    case BatteryMode::Block:          return language == Language::English ? "block charge, block discharge"       : "Laden blockieren, Entladen blockieren";
    case BatteryMode::Normal:         return language == Language::English ? "charge normally, discharge normally" : "normal Laden, normal Entladen";
    case BatteryMode::BlockDischarge: return language == Language::English ? "charge normally, block discharge"    : "normal Laden, Entladen blockieren";
    case BatteryMode::ForceCharge:    return language == Language::English ? "force charge, block discharge"       : "Laden erzwingen, Entladen blockieren";
    case BatteryMode::BlockCharge:    return language == Language::English ? "block charge, discharge normally"    : "Laden blockieren, normal Entladen";
    case BatteryMode::ForceDischarge: return language == Language::English ? "block charge, force discharge"       : "Laden blockieren, Entladen erzwingen";
    default:                          esp_system_abortf<64>("Unknown battery mode for display name lookup: %d", static_cast<int>(mode));
    }
}

void BatteryModbusTCP::load_custom_table(BatteryModbusTCP::TableSpec **table_ptr, const Config *config)
{
    const Config *register_blocks_config = static_cast<const Config *>(config->get("register_blocks"));
    size_t register_blocks_count         = register_blocks_config->count();
    BatteryModbusTCP::TableSpec *table   = static_cast<BatteryModbusTCP::TableSpec *>(malloc_psram_or_dram(sizeof(BatteryModbusTCP::TableSpec)));

    table->effective_mode        = config->get("effective_mode")->asEnum<BatteryMode>();
    table->register_blocks       = static_cast<BatteryModbusTCP::RegisterBlockSpec *>(malloc_psram_or_dram(sizeof(BatteryModbusTCP::RegisterBlockSpec) * register_blocks_count));
    table->register_blocks_count = register_blocks_count;

    size_t total_buffer_length = 0; // bytes

    for (size_t i = 0; i < register_blocks_count; ++i) {
        auto register_block_config = register_blocks_config->get(i);
        ModbusFunctionCode function_code = register_block_config->get("func")->asEnum<ModbusFunctionCode>();
        uint16_t values_count = static_cast<uint16_t>(register_block_config->get("vals")->count());

        if (function_code == ModbusFunctionCode::ReadCoils
         || function_code == ModbusFunctionCode::ReadDiscreteInputs
         || function_code == ModbusFunctionCode::WriteSingleCoil
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

        if (register_block->function_code == ModbusFunctionCode::ReadCoils
         || register_block->function_code == ModbusFunctionCode::ReadDiscreteInputs
         || register_block->function_code == ModbusFunctionCode::WriteSingleCoil
         || register_block->function_code == ModbusFunctionCode::WriteMultipleCoils) {
            buffer_length = (values_count + 7u) / 8u;
        }
        else {
            buffer_length = values_count * 2u;
        }

        if (register_block->function_code == ModbusFunctionCode::ReadCoils
         || register_block->function_code == ModbusFunctionCode::ReadDiscreteInputs
         || register_block->function_code == ModbusFunctionCode::WriteSingleCoil
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
static void writer_logfln(BatteryModbusTCP::WriterContext *ctx, bool error, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    ctx->vlogfln(error, fmt, args);
    va_end(args);
}

static void next_writer_step(BatteryModbusTCP::WriterContext *ctx);

static void last_writer_step(BatteryModbusTCP::WriterContext *ctx, bool success)
{
    if (ctx->destroy_requested) {
        delete ctx;
        return;
    }

    if (success && ctx->battery != nullptr) {
        ctx->battery->set_state_mode(ctx->mode, ctx->table->effective_mode);
    }

    millis_t delay = success ? seconds_t{ctx->repeat_interval} : 5_s;

    if (delay == 0_s) {
        ctx->finished();
        return;
    }

    ctx->task_id = task_scheduler.scheduleOnce([ctx]() {
        if (ctx->destroy_requested) {
            delete ctx;
            return;
        }

        ctx->task_id = 0;

        char description[128];

        if (ctx->first_non_precondition_index != 0 and !ctx->precondition_met) {
            ctx->index = 0;

            trace("b%lu t%d ww m%c em%c pcc",
                  ctx->slot,
                  ctx->test ? 1 : 0,
                  get_battery_mode_as_char(ctx->mode),
                  get_battery_mode_as_char(ctx->table->effective_mode));
        }
        else {
            ++ctx->repeat_count;
            ctx->index = ctx->first_non_precondition_index;

            trace("b%lu t%d ww m%c em%c r%zu",
                  ctx->slot,
                  ctx->test ? 1 : 0,
                  get_battery_mode_as_char(ctx->mode),
                  get_battery_mode_as_char(ctx->table->effective_mode),
                  ctx->repeat_count);

            writer_logfln(ctx, false,
                          ctx->language == Language::English
                          ? "Setting mode %s (repeat %zu)"
                          : "Setze Modus %s (Wiederholung %zu)",
                          get_battery_mode_description(ctx->mode, ctx->table->effective_mode, ctx->language, description, std::size(description)),
                          ctx->repeat_count);
        }

        next_writer_step(ctx);
    }, delay);
}

static void next_writer_step(BatteryModbusTCP::WriterContext *ctx)
{
    if (ctx->destroy_requested) {
        delete ctx;
        return;
    }

    if (ctx->index >= ctx->table->register_blocks_count) {
        last_writer_step(ctx, true);
        return;
    }

    const BatteryModbusTCP::RegisterBlockSpec *register_block = &ctx->table->register_blocks[ctx->index];
    TFModbusTCPFunctionCode function_code;
    uint16_t data_count;
    const void *buffer;
    void *buffer_to_compare = nullptr;
    size_t buffer_to_compare_len = 0;
    void *buffer_to_free = nullptr;

    switch (register_block->function_code) {
    case ModbusFunctionCode::ReadCoils:
        function_code = TFModbusTCPFunctionCode::ReadCoils;
        data_count = register_block->values_count;
        buffer_to_compare = register_block->buffer;
        buffer_to_compare_len = (data_count + 7u) / 8u;
        buffer_to_free = malloc(buffer_to_compare_len);
        buffer = buffer_to_free;

        if (buffer == nullptr) {
            writer_logfln(ctx, true,
                          ctx->language == Language::English
                          ? "Could not allocate read buffer"
                          : "Konnte Lesepuffer nicht allokieren");
            last_writer_step(ctx, false);
            return;
        }

        break;

    case ModbusFunctionCode::ReadDiscreteInputs:
        function_code = TFModbusTCPFunctionCode::ReadDiscreteInputs;
        data_count = register_block->values_count;
        buffer_to_compare = register_block->buffer;
        buffer_to_compare_len = (data_count + 7u) / 8u;
        buffer_to_free = malloc(buffer_to_compare_len);
        buffer = buffer_to_free;

        if (buffer == nullptr) {
            writer_logfln(ctx, true,
                          ctx->language == Language::English
                          ? "Could not allocate read buffer"
                          : "Konnte Lesepuffer nicht allokieren");
            last_writer_step(ctx, false);
            return;
        }

        break;

    case ModbusFunctionCode::ReadHoldingRegisters:
        function_code = TFModbusTCPFunctionCode::ReadHoldingRegisters;
        data_count = register_block->values_count;
        buffer_to_compare = register_block->buffer;
        buffer_to_compare_len = sizeof(uint16_t) * data_count;
        buffer_to_free = malloc(buffer_to_compare_len);
        buffer = buffer_to_free;

        if (buffer == nullptr) {
            writer_logfln(ctx, true,
                          ctx->language == Language::English
                          ? "Could not allocate read buffer"
                          : "Konnte Lesepuffer nicht allokieren");
            last_writer_step(ctx, false);
            return;
        }

        break;

    case ModbusFunctionCode::ReadInputRegisters:
        function_code = TFModbusTCPFunctionCode::ReadInputRegisters;
        data_count = register_block->values_count;
        buffer_to_compare = register_block->buffer;
        buffer_to_compare_len = sizeof(uint16_t) * data_count;
        buffer_to_free = malloc(buffer_to_compare_len);
        buffer = buffer_to_free;

        if (buffer == nullptr) {
            writer_logfln(ctx, true,
                          ctx->language == Language::English
                          ? "Could not allocate read buffer"
                          : "Konnte Lesepuffer nicht allokieren");
            last_writer_step(ctx, false);
            return;
        }

        break;

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
            writer_logfln(ctx, true,
                          ctx->language == Language::English
                          ? "Could not allocate read buffer"
                          : "Konnte Lesepuffer nicht allokieren");
            last_writer_step(ctx, false);
            return;
        }

        break;

    default:
        writer_logfln(ctx, true,
                      ctx->language == Language::English
                      ? "Unknown function code: %u"
                      : "Funktionscode unbekannt: %u",
                      static_cast<uint8_t>(register_block->function_code));
        last_writer_step(ctx, false);
        return;
    }

    ctx->transact_pending = true;

    static_cast<TFModbusTCPSharedClient *>(ctx->client)->transact(ctx->device_address,
                                                                  function_code,
                                                                  register_block->start_address,
                                                                  data_count,
                                                                  const_cast<void *>(buffer),
                                                                  2_s,
    [ctx, register_block, data_count, buffer, buffer_to_compare, buffer_to_compare_len, buffer_to_free](TFModbusTCPClientTransactionResult result, const char *error_message) {
        if (ctx->destroy_requested) {
            ctx->transact_pending = false;

            free(buffer_to_free);
            delete ctx;
            return;
        }

        char description[128];
        bool has_step2 = register_block->function_code == ModbusFunctionCode::ReadMaskWriteSingleRegister
                      || register_block->function_code == ModbusFunctionCode::ReadMaskWriteMultipleRegisters;

        if (result != TFModbusTCPClientTransactionResult::Success) {
            trace("b%lu t%d ww m%c em%c i%zu/%zu%s e%d%s%s",
                  ctx->slot,
                  ctx->test ? 1 : 0,
                  get_battery_mode_as_char(ctx->mode),
                  get_battery_mode_as_char(ctx->table->effective_mode),
                  ctx->index + 1,
                  ctx->table->register_blocks_count,
                  has_step2 ? " s1/2" : "",
                  static_cast<int>(result),
                  error_message != nullptr ? " / " : "",
                  error_message != nullptr ? error_message : "");

            writer_logfln(ctx,
                          true,
                          ctx->language == Language::English
                          ? (buffer_to_compare != nullptr ? "Check of precondition for mode %s failed at register block %zu of %zu: %s (%d)%s%s" : "Setting mode %s failed at register block %zu of %zu: %s (%d)%s%s")
                          : (buffer_to_compare != nullptr ? "Prüfen der Vorbedingung des Modus %s schlug fehl bei Registerblock %zu von %zu: %s (%d)%s%s" : "Setzen des Modus %s schlug fehl bei Registerblock %zu von %zu: %s (%d)%s%s"),
                          get_battery_mode_description(ctx->mode, ctx->table->effective_mode, ctx->language, description, std::size(description)),
                          ctx->index + 1, ctx->table->register_blocks_count,
                          get_tf_modbus_tcp_client_transaction_result_name(result),
                          static_cast<int>(result),
                          error_message != nullptr ? " / " : "",
                          error_message != nullptr ? error_message : "");

            ctx->transact_pending = false;

            free(buffer_to_free);
            last_writer_step(ctx, false);
            return;
        }

        if (buffer_to_compare != nullptr) {
            ctx->precondition_met = memcmp(buffer, buffer_to_compare, buffer_to_compare_len) == 0;

            if (ctx->precondition_met) {
                ctx->last_precondition_not_met_index_plus_one = 0;

                if (ctx->index + 1 >= ctx->first_non_precondition_index) {
                    trace("b%lu t%d ww m%c em%c i%zu/%zu pcm",
                          ctx->slot,
                          ctx->test ? 1 : 0,
                          get_battery_mode_as_char(ctx->mode),
                          get_battery_mode_as_char(ctx->table->effective_mode),
                          ctx->index + 1,
                          ctx->table->register_blocks_count);

                    if (ctx->battery != nullptr) {
                        ctx->battery->set_state_checking(false);
                    }

                    if (ctx->repeat_interval > 0) {
                        writer_logfln(ctx, false,
                                      ctx->language == Language::English
                                      ? "Setting mode %s (will repeat in %u second%s)"
                                      : "Setze Modus %s (Wiederholung in %u Sekunde%s)",
                                      get_battery_mode_description(ctx->mode, ctx->table->effective_mode, ctx->language, description, std::size(description)),
                                      ctx->repeat_interval,
                                      ctx->repeat_interval > 1 ? (ctx->language == Language::English ? "s" : "n") : "");
                    }
                    else {
                        writer_logfln(ctx, false,
                                      ctx->language == Language::English
                                      ? "Setting mode %s (once)"
                                      : "Setze Modus %s (einmalig)",
                                      get_battery_mode_description(ctx->mode, ctx->table->effective_mode, ctx->language, description, std::size(description)));
                    }
                }
            }
            else {
                trace("b%lu t%d ww m%c em%c i%zu/%zu pcnm",
                      ctx->slot,
                      ctx->test ? 1 : 0,
                      get_battery_mode_as_char(ctx->mode),
                      get_battery_mode_as_char(ctx->table->effective_mode),
                      ctx->index + 1,
                      ctx->table->register_blocks_count);

                if (ctx->last_precondition_not_met_index_plus_one != ctx->index + 1) {
                    ctx->last_precondition_not_met_index_plus_one = ctx->index + 1;

                    writer_logfln(ctx, false,
                                  ctx->language == Language::English
                                  ? "Precondition for mode %s not met at register block %zu of %zu"
                                  : "Vorbedingung des Modus %s nicht erfüllt bei Registerblock %zu von %zu",
                                  get_battery_mode_description(ctx->mode, ctx->table->effective_mode, ctx->language, description, std::size(description)),
                                  ctx->index + 1, ctx->table->register_blocks_count);
                }

                ctx->transact_pending = false;

                free(buffer_to_free);
                last_writer_step(ctx, false);
                return;
            }
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

            static_cast<TFModbusTCPSharedClient *>(ctx->client)->transact(ctx->device_address,
                                                                          step2_function_code,
                                                                          register_block->start_address,
                                                                          data_count,
                                                                          const_cast<void *>(buffer),
                                                                          2_s,
            [ctx, buffer_to_free](TFModbusTCPClientTransactionResult step2_result, const char *step2_error_message) {
                if (ctx->destroy_requested) {
                    ctx->transact_pending = false;

                    free(buffer_to_free);
                    delete ctx;
                    return;
                }

                if (step2_result != TFModbusTCPClientTransactionResult::Success) {
                    trace("b%lu t%d ww m%c em%c i%zu/%zu s2/2 e%d%s%s",
                          ctx->slot,
                          ctx->test ? 1 : 0,
                          get_battery_mode_as_char(ctx->mode),
                          get_battery_mode_as_char(ctx->table->effective_mode),
                          ctx->index + 1,
                          ctx->table->register_blocks_count,
                          static_cast<int>(step2_result),
                          step2_error_message != nullptr ? " / " : "",
                          step2_error_message != nullptr ? step2_error_message : "");

                    char description_[128];

                    writer_logfln(ctx,
                                  true,
                                  ctx->language == Language::English
                                  ? "Setting mode %s failed at register block %zu of %zu: %s (%d)%s%s"
                                  : "Setzen des Modus %s (Schritt 2) schlug fehl bei Registerblock %zu von %zu: %s (%d)%s%s",
                                  get_battery_mode_description(ctx->mode, ctx->table->effective_mode, ctx->language, description_, std::size(description_)),
                                  ctx->index + 1, ctx->table->register_blocks_count,
                                  get_tf_modbus_tcp_client_transaction_result_name(step2_result),
                                  static_cast<int>(step2_result),
                                  step2_error_message != nullptr ? " / " : "",
                                  step2_error_message != nullptr ? step2_error_message : "");

                    ctx->transact_pending = false;

                    free(buffer_to_free);
                    last_writer_step(ctx, false);
                    return;
                }

                ++ctx->index;
                ctx->transact_pending = false;

                free(buffer_to_free);
                next_writer_step(ctx); // FIXME: maybe add a little delay between writes to avoid bursts?
            },
            ctx->transaction_id_mask);
        }
        else {
            ++ctx->index;
            ctx->transact_pending = false;

            free(buffer_to_free);
            next_writer_step(ctx); // FIXME: maybe add a little delay between writes to avoid bursts?
        }
    },
    ctx->transaction_id_mask);
}

BatteryModbusTCP::WriterContext *BatteryModbusTCP::create_writer(BatteryModbusTCP *battery,
                                                                 uint32_t slot,
                                                                 bool test,
                                                                 TFModbusTCPSharedClient *client,
                                                                 uint8_t device_address,
                                                                 uint16_t transaction_id_mask,
                                                                 uint16_t repeat_interval, // seconds
                                                                 BatteryMode mode,
                                                                 TableSpec *table,
                                                                 VLogFLnFunction &&vlogfln,
                                                                 WriterFinishedFunction &&finished,
                                                                 Language language /*= Language::English*/)
{
    trace("b%lu t%d wc m%c em%c",
          slot,
          test ? 1 : 0,
          get_battery_mode_as_char(mode),
          table != nullptr ? get_battery_mode_as_char(table->effective_mode) : '?');

    WriterContext *ctx = new WriterContext;

    ctx->language = language;
    ctx->battery = battery;
    ctx->slot = slot;
    ctx->client = client;
    ctx->device_address = device_address;
    ctx->transaction_id_mask = transaction_id_mask;
    ctx->repeat_interval = repeat_interval;
    ctx->mode = mode;
    ctx->table = table;
    ctx->vlogfln = std::move(vlogfln);
    ctx->finished = std::move(finished);
    ctx->test = test;
    ctx->task_id = task_scheduler.scheduleOnce([ctx]() {
        if (ctx->destroy_requested) {
            delete ctx;
            return;
        }

        ctx->task_id = 0;

        // FIXME: don't allow missing tables and also use empty tables to be able to test them,
        //        otherwise the test immidiatly ends and global battery control resumes
        if (ctx->table == nullptr || ctx->table->register_blocks_count == 0) {
            writer_logfln(ctx, false,
                          ctx->language == Language::English
                          ? "Table is empty, nothing to do"
                          : "Tabelle ist leer, nichts zu tun");

            ctx->finished();
            return;
        }

        trace("b%lu t%d ww m%c em%c %c",
              ctx->slot,
              ctx->test ? 1 : 0,
              get_battery_mode_as_char(ctx->mode),
              get_battery_mode_as_char(ctx->table->effective_mode),
              ctx->repeat_interval > 0 ? 'f' : 'o');

        for (size_t i = 0; i < ctx->table->register_blocks_count; ++i) {
            const BatteryModbusTCP::RegisterBlockSpec *register_block = &ctx->table->register_blocks[i];

            switch (register_block->function_code) {
            case ModbusFunctionCode::ReadCoils:
            case ModbusFunctionCode::ReadDiscreteInputs:
            case ModbusFunctionCode::ReadHoldingRegisters:
            case ModbusFunctionCode::ReadInputRegisters:
                ctx->first_non_precondition_index = i + 1;
                break;

            case ModbusFunctionCode::WriteSingleCoil:
            case ModbusFunctionCode::WriteSingleRegister:
            case ModbusFunctionCode::WriteMultipleCoils:
            case ModbusFunctionCode::WriteMultipleRegisters:
            case ModbusFunctionCode::MaskWriteRegister:
            case ModbusFunctionCode::ReadMaskWriteSingleRegister:
            case ModbusFunctionCode::ReadMaskWriteMultipleRegisters:
                break;

            default:
                writer_logfln(ctx, true,
                              ctx->language == Language::English
                              ? "Unknown function code: %u"
                              : "Funktionscode unbekannt: %u",
                              static_cast<uint8_t>(register_block->function_code));

                ctx->finished();
                return;
            }
        }

        if (ctx->battery != nullptr && ctx->first_non_precondition_index != 0) {
            ctx->battery->set_state_checking(true);
        }

        char description[128];

        if (ctx->first_non_precondition_index != 0) {
            writer_logfln(ctx, false,
                          ctx->language == Language::English
                          ? "Checking precondition for mode %s"
                          : "Prüfe Vorbedingung für Modus %s",
                          get_battery_mode_description(ctx->mode, ctx->table->effective_mode, ctx->language, description, std::size(description)));
        }
        else if (ctx->repeat_interval > 0) {
            writer_logfln(ctx, false,
                          ctx->language == Language::English
                          ? "Setting mode %s (will repeat in %u second%s)"
                          : "Setze Modus %s (Wiederholung in %u Sekunde%s)",
                          get_battery_mode_description(ctx->mode, ctx->table->effective_mode, ctx->language, description, std::size(description)),
                          ctx->repeat_interval,
                          ctx->repeat_interval > 1 ? (ctx->language == Language::English ? "s" : "n") : "");
        }
        else {
            writer_logfln(ctx, false,
                          ctx->language == Language::English
                          ? "Setting mode %s (once)"
                          : "Setze Modus %s (einmalig)",
                          get_battery_mode_description(ctx->mode, ctx->table->effective_mode, ctx->language, description, std::size(description)));
        }

        next_writer_step(ctx);
    });

    return ctx;
}

void BatteryModbusTCP::destroy_writer(BatteryModbusTCP::WriterContext *ctx)
{
    if (ctx == nullptr) {
        return;
    }

    trace("b%lu t%d wd m%c em%c",
          ctx->slot,
          ctx->test ? 1 : 0,
          get_battery_mode_as_char(ctx->mode),
          ctx->table != nullptr ? get_battery_mode_as_char(ctx->table->effective_mode) : '?');

    if (ctx->battery != nullptr && ctx->first_non_precondition_index != 0) {
        ctx->battery->set_state_checking(false);
    }

    if (ctx->transact_pending) {
        ctx->destroy_requested = true;
        return;
    }

    task_scheduler.cancel(ctx->task_id);
    delete ctx;
}

[[gnu::format(__printf__, 3, 4)]]
static void discover_logfln(BatteryModbusTCP::DiscoverContext *ctx, bool error, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    ctx->vlogfln(error, fmt, args);
    va_end(args);
}

static void free_discover(BatteryModbusTCP::DiscoverContext *ctx)
{
    free(ctx->buffer);
    delete ctx;
}

static void read_kostal_plenticore_byte_order(BatteryModbusTCP::DiscoverContext *ctx)
{
    if (ctx->destroy_requested) {
        free_discover(ctx);
        return;
    }

    trace("b%lu t%d dr k",
          ctx->slot,
          ctx->test ? 1 : 0);

    ctx->transact_pending = true;

    static_cast<TFModbusTCPSharedClient *>(ctx->client)->transact(ctx->device_address,
                                                                  TFModbusTCPFunctionCode::ReadHoldingRegisters,
                                                                  5,
                                                                  1,
                                                                  ctx->buffer,
                                                                  2_s,
    [ctx](TFModbusTCPClientTransactionResult result, const char *error_message) {
        if (ctx->destroy_requested) {
            free_discover(ctx);
            return;
        }

        ctx->transact_pending = false;

        bool error;

        if (result == TFModbusTCPClientTransactionResult::Success) {
            error = ctx->complete();
        }
        else {
            trace("b%lu t%d dr k e%d%s%s",
                  ctx->slot,
                  ctx->test ? 1 : 0,
                  static_cast<int>(result),
                  error_message != nullptr ? " / " : "",
                  error_message != nullptr ? error_message : "");

            discover_logfln(ctx, true,
                            ctx->language == Language::English
                            ? "Could not discover Modbus/TCP byte order: %s (%d)%s%s"
                            : "Konnte Modbus/TCP-Byte-Reihenfolge nicht ermitteln: %s (%d)%s%s",
                            get_tf_modbus_tcp_client_transaction_result_name(result),
                            static_cast<int>(result),
                            error_message != nullptr ? " / " : "",
                            error_message != nullptr ? error_message : "");

            error = true;
        }

        if (error) {
            ctx->task_id = task_scheduler.scheduleOnce([ctx]() {
                read_kostal_plenticore_byte_order(ctx);
            }, 5_s);
        }
    },
    ctx->transaction_id_mask);
}

BatteryModbusTCP::DiscoverContext *BatteryModbusTCP::create_discover(BatteryModbusTCP *battery,
                                                                     uint32_t slot,
                                                                     bool test,
                                                                     TFModbusTCPSharedClient *client,
                                                                     uint8_t device_address,
                                                                     uint16_t transaction_id_mask,
                                                                     VLogFLnFunction &&vlogfln,
                                                                     Language language /*= Language::English*/)
{
    trace("b%lu t%d dc", slot, test ? 1 : 0);

    DiscoverContext *discover = new DiscoverContext;

    discover->language = language;
    discover->battery = battery;
    discover->slot = slot;
    discover->client = client;
    discover->device_address = device_address;
    discover->transaction_id_mask = transaction_id_mask;
    discover->vlogfln = std::move(vlogfln);
    discover->test = test;

    if (discover->battery != nullptr) {
        discover->battery->set_state_discovering(true);
    }

    return discover;
}

void BatteryModbusTCP::destroy_discover(DiscoverContext *discover)
{
    if (discover == nullptr) {
        return;
    }

    trace("b%lu t%d dd", discover->slot, discover->test ? 1 : 0);

    if (discover->battery != nullptr) {
        discover->battery->set_state_discovering(false);
    }

    if (discover->transact_pending) {
        discover->destroy_requested = true;
        return;
    }

    task_scheduler.cancel(discover->task_id);
    free_discover(discover);
}

void BatteryModbusTCP::discover_kostal_plenticore_plus_g2_variant(DiscoverContext *ctx, std::function<void(KostalPlenticorePlusG2Variant variant)> &&callback)
{
    ctx->buffer = malloc(sizeof(uint16_t));
    ctx->complete = [ctx, callback = std::move(callback)]() {
        uint16_t byte_order = *static_cast<uint16_t *>(ctx->buffer);

        if (byte_order == 0) {
            trace("b%lu t%d dr k le", ctx->slot, ctx->test ? 1 : 0);

            discover_logfln(ctx, false,
                            ctx->language == Language::English
                            ? "KOSTAL PLENTICORE plus G2 reports little endian byte order"
                            : "KOSTAL PLENTICORE plus G2 meldet Little-Endian Byte-Reihenfolge");

            callback(KostalPlenticorePlusG2Variant::LittleEndian);
            return false;
        }
        else if (byte_order == 1) {
            trace("b%lu t%d dr k be", ctx->slot, ctx->test ? 1 : 0);

            discover_logfln(ctx, false,
                            ctx->language == Language::English
                            ? "KOSTAL PLENTICORE plus G2 reports big endian byte order"
                            : "KOSTAL PLENTICORE plus G2 meldet Big-Endian Byte-Reihenfolge");

            callback(KostalPlenticorePlusG2Variant::BigEndian);
            return false;
        }
        else {
            trace("b%lu t%d dr k u%u", ctx->slot, ctx->test ? 1 : 0, byte_order);

            discover_logfln(ctx, true,
                            ctx->language == Language::English
                            ? "KOSTAL PLENTICORE plus G2 reports unknown byte order: %u"
                            : "KOSTAL PLENTICORE plus G2 meldet unbekannte Byte-Reihenfolge: %u",
                            byte_order);

            return true;
        }
    };

    read_kostal_plenticore_byte_order(ctx);
}

void BatteryModbusTCP::discover_kostal_plenticore_g3_variant(DiscoverContext *ctx, std::function<void(KostalPlenticoreG3Variant variant)> &&callback)
{
    ctx->buffer = malloc(sizeof(uint16_t));
    ctx->complete = [ctx, callback = std::move(callback)]() {
        uint16_t byte_order = *static_cast<uint16_t *>(ctx->buffer);

        if (byte_order == 0) {
            trace("b%lu t%d dr k le", ctx->slot, ctx->test ? 1 : 0);

            discover_logfln(ctx, false,
                            ctx->language == Language::English
                            ? "KOSTAL PLENTICORE G3 reports little endian byte order"
                            : "KOSTAL PLENTICORE G3 meldet Little-Endian Byte-Reihenfolge");

            callback(KostalPlenticoreG3Variant::LittleEndian);
            return false;
        }
        else if (byte_order == 1) {
            trace("b%lu t%d dr k be", ctx->slot, ctx->test ? 1 : 0);

            discover_logfln(ctx, false,
                            ctx->language == Language::English
                            ? "KOSTAL PLENTICORE G3 reports big endian byte order"
                            : "KOSTAL PLENTICORE G3 meldet Big-Endian Byte-Reihenfolge");

            callback(KostalPlenticoreG3Variant::BigEndian);
            return false;
        }
        else {
            trace("b%lu t%d dr k u%u", ctx->slot, ctx->test ? 1 : 0, byte_order);

            discover_logfln(ctx, true,
                            ctx->language == Language::English
                            ? "KOSTAL PLENTICORE G3 reports unknown byte order: %u"
                            : "KOSTAL PLENTICORE G3 meldet unbekannte Byte-Reihenfolge: %u",
                            byte_order);

            return true;
        }
    };

    read_kostal_plenticore_byte_order(ctx);
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

    if (table_id == BatteryModbusTCPTableID::SAXPowerHomeBasicMode) {
        transaction_id_mask = UINT8_MAX; // SAX Power has a bug that the first byte of the Modbus/TCP transaction ID is always 0 in a write response
    }

    switch (table_id) {
    case BatteryModbusTCPTableID::None:
        logger.printfln_battery("No table selected");
        break;

    case BatteryModbusTCPTableID::Custom:
        device_address = table_config->get("device_address")->asUint8();
        transaction_id_mask = UINT16_MAX;
        repeat_interval = table_config->get("repeat_interval")->asUint16();
        break;

#include "generated/battery_modbus_tcp_setup.inc"

    default:
        logger.printfln_battery("Unknown table: %u", static_cast<uint8_t>(table_id));
        break;
    }

    if (table_id == BatteryModbusTCPTableID::KostalPlenticorePlusG2
     || table_id == BatteryModbusTCPTableID::KostalPlenticoreG3) {
        discover_table_config = new Config(*table_config);
    }
    else {
        load_tables(table_config);
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

void BatteryModbusTCP::set_mode(BatteryMode mode)
{
    // FIXME: why is setting mode none accepted here?

    if (requested_mode == mode) {
        return;
    }

    requested_mode = mode;

    update_pending_mode();
}

void BatteryModbusTCP::set_testing(bool testing_)
{
    state->get("testing")->updateBool(testing_);

    if (this->testing == testing_) {
        return;
    }

    if (!testing_) {
        // clear active/effective mode in case the test is ending. if the
        // global battery control is active it will control the battery
        // immediatly after the test has ended and set the mode again
        set_state_mode(BatteryMode::None, BatteryMode::None);
    }

    this->testing = testing_;

    update_pending_mode();
}

void  BatteryModbusTCP::set_state_mode(BatteryMode active_mode, BatteryMode effective_mode)
{
    state->get("active_mode")->updateEnum(active_mode);
    state->get("effective_mode")->updateEnum(effective_mode);
}

void  BatteryModbusTCP::set_state_discovering(bool discovering)
{
    state->get("discovering")->updateBool(discovering);
}

void  BatteryModbusTCP::set_state_checking(bool checking)
{
    state->get("checking")->updateBool(checking);
}

void BatteryModbusTCP::connect_callback(TFGenericTCPClientConnectResult result, TFGenericTCPClientPoolShareLevel share_level)
{
    trace("b%lu t0 cc%d sl%d", slot, static_cast<int>(result), static_cast<int>(share_level));

    if (result != TFGenericTCPClientConnectResult::Connected) {
        return;
    }

    if (table_id == BatteryModbusTCPTableID::KostalPlenticorePlusG2
     || table_id == BatteryModbusTCPTableID::KostalPlenticoreG3) {
        discover_pending = true; // (re-)discover after (re-)connect
    }

    update_pending_mode();
}

void BatteryModbusTCP::disconnect_callback(TFGenericTCPClientDisconnectReason reason, TFGenericTCPClientPoolShareLevel share_level)
{
    trace("b%lu t0 cd%d sl%d", slot, static_cast<int>(reason), static_cast<int>(share_level));

    update_pending_mode();
}

void BatteryModbusTCP::load_tables(const Config *table_config)
{
    const Config *battery_modes_config;

    // free potential old tables first. tables might be reloaded if rediscover happens
    for (size_t i = 0; i < std::size(tables); ++i) {
        free_table(tables[i]);
        tables[i] = nullptr;
    }

    switch (table_id) {
    case BatteryModbusTCPTableID::None:
        logger.printfln_battery("No table selected");
        break;

    case BatteryModbusTCPTableID::Custom:
        battery_modes_config = static_cast<const Config *>(table_config->get("battery_modes"));

        for (size_t i = static_cast<size_t>(BatteryMode::Block); i <= static_cast<size_t>(BatteryMode::_max); ++i) {
            load_custom_table(&tables[i], static_cast<const Config *>(battery_modes_config->get(i)));
        }

        break;

#include "generated/battery_modbus_tcp_load_tables.inc"

    default:
        logger.printfln_battery("Unknown table: %u", static_cast<uint8_t>(table_id));
        break;
    }
}

void BatteryModbusTCP::update_pending_mode()
{
    TableSpec *table;
    bool start_discover;
    BatteryMode next_mode;

    if (requested_mode == BatteryMode::None || connected_client == nullptr || testing) {
        table = nullptr;
        start_discover = false;
        next_mode = BatteryMode::None;
    }
    else if (discover_pending) {
        table = nullptr;
        start_discover = true;
        next_mode = BatteryMode::None;
    }
    else {
        table = tables[static_cast<size_t>(requested_mode)];
        start_discover = false;

        if (table == nullptr) {
            next_mode = BatteryMode::None;
        }
        else {
            next_mode = requested_mode;
        }
    }

    if (pending_mode == next_mode && (discover_ctx != nullptr) == start_discover) {
        return;
    }

    destroy_discover(discover_ctx);
    discover_ctx = nullptr;

    destroy_writer(writer_ctx);
    writer_ctx = nullptr;

    trace("b%lu t0 %s r%c%s m%c->%c%s",
          slot,
          connected_client != nullptr ? "ce" : "nc",
          get_battery_mode_as_char(requested_mode),
          testing ? " tg" : "",
          get_battery_mode_as_char(pending_mode),
          get_battery_mode_as_char(next_mode),
          start_discover ? " sd" : "");

    pending_mode = next_mode;

    if (start_discover) {
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=format"
#endif
        discover_ctx = create_discover(this, slot, false, static_cast<TFModbusTCPSharedClient *>(connected_client),
                                       device_address, transaction_id_mask,
        [this](bool error, const char *fmt, va_list args) {
            char message[256];

            vsnprintf(message, sizeof(message), fmt, args);
            logger.printfln_battery("%s", message);
        });
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

        if (table_id == BatteryModbusTCPTableID::KostalPlenticorePlusG2) {
            discover_kostal_plenticore_plus_g2_variant(discover_ctx,
            [this](KostalPlenticorePlusG2Variant variant) {
                kostal_plenticore_plus_g2_variant = variant;
                load_tables(discover_table_config);

                task_scheduler.scheduleOnce([this]() {
                    discover_pending = false;

                    // destroy the discover context on the main task.
                    // calling destroy_discover in the outer lambda
                    // would delete the std::function holding that lambda
                    destroy_discover(discover_ctx);
                    discover_ctx = nullptr;

                    update_pending_mode();
                });
            });
        }
        else if (table_id == BatteryModbusTCPTableID::KostalPlenticoreG3) {
            discover_kostal_plenticore_g3_variant(discover_ctx,
            [this](KostalPlenticoreG3Variant variant) {
                kostal_plenticore_g3_variant = variant;
                load_tables(discover_table_config);

                task_scheduler.scheduleOnce([this]() {
                    discover_pending = false;

                    // destroy the discover context on the main task.
                    // calling destroy_discover in the outer lambda
                    // would delete the std::function holding that lambda
                    destroy_discover(discover_ctx);
                    discover_ctx = nullptr;

                    update_pending_mode();
                });
            });
        }
        else {
            esp_system_abort("Entered battery discover mode without anything to discover");
        }
    }
    else if (table != nullptr) {
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=format"
#endif
        writer_ctx = create_writer(this, slot, false, static_cast<TFModbusTCPSharedClient *>(connected_client),
                                   device_address, transaction_id_mask, repeat_interval, pending_mode, table,
        [this](bool error, const char *fmt, va_list args) {
            if (!error) {
                return;
            }

            char message[256];

            vsnprintf(message, sizeof(message), fmt, args);
            logger.printfln_battery("%s", message);
        },
        [this]() {
            update_pending_mode();
        });
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    }
}
