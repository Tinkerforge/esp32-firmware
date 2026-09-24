/* esp32-firmware
 * Copyright (C) 2026 Matthias Bolte <matthias@tinkerforge.com>
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

#define EVENT_LOG_PREFIX "batteries_sunspc"

#include "battery_sun_spec.h"

#include <stdio.h>
#include <TFModbusTCPClient.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tools.h"
#include "tools/memory.h"
#include "tools/hexdump.h"
#include "tools/sun_spec.h"
#include "modules/modbus_tcp_client/modbus_tcp_tools.h"

#include "gcc_warnings.h"

#define RESOLVE_TIMEOUT 60_s
#define REPEAT_INTERVAL 60_s
#define WATCHDOG_TIMEOUT 90_s

#define ChaGriSet_PV   0u
#define ChaGriSet_GRID 1u

#define StorCtlMod_CHARGE    (1u << 0)
#define StorCtlMod_DISCHARGE (1u << 1)

#define writer_logfln(event_log, fmt_en, fmt_de, ...) writer_logfln_(ctx, event_log, ctx->language == Language::English ? "%s" fmt_en : "%s" fmt_de, ctx->log_prefix __VA_OPT__(,) __VA_ARGS__)

#define trace(fmt, ...) \
    do { \
        batteries_sun_spec.trace_timestamp(); \
        logger.tracefln_plain(batteries_sun_spec.trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

static char get_battery_mode_as_char(BatteryMode mode)
{
    if (mode == BatteryMode::None) {
        return 'n';
    }

    return '0' + static_cast<char>(mode);
}

[[gnu::format(__printf__, 3, 4)]]
static void writer_logfln_(BatterySunSpec::WriterContext *ctx, bool event_log, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    ctx->vlogfln(event_log, fmt, args);
    va_end(args);
}

static void writer_next(BatterySunSpec::WriterContext *ctx);

static void writer_finish(BatterySunSpec::WriterContext *ctx, bool success)
{
    if (ctx->destroy_requested) {
        delete ctx;
        return;
    }

    if (success && ctx->battery != nullptr) {
        ctx->battery->set_state_mode(ctx->mode);
    }

    millis_t delay = success ? seconds_t{REPEAT_INTERVAL} : 5_s;

    ctx->task_id = task_scheduler.scheduleOnce([ctx]() {
        if (ctx->destroy_requested) {
            delete ctx;
            return;
        }

        ctx->task_id = 0;

        ++ctx->repeat_count;
        ctx->state = BatterySunSpec::WriterState::Idle;
        ctx->state_next = BatterySunSpec::WriterState::ReadInOutWRteSF;

        trace("b%lu t%d ww m%c r%zu",
              ctx->slot,
              ctx->test ? 1 : 0,
              get_battery_mode_as_char(ctx->mode),
              ctx->repeat_count);

        writer_logfln(false,
                      "Setting mode \"%s\" (repeat %zu)",
                      "Setze Modus \"%s\" (Wiederholung %zu)",
                      Batteries::get_battery_mode_display_name(ctx->mode, ctx->language),
                      ctx->repeat_count);

        writer_next(ctx);
    }, delay);
}

static void writer_next(BatterySunSpec::WriterContext *ctx)
{
    if (ctx->destroy_requested) {
        delete ctx;
        return;
    }

    uint16_t stor_ctl_mod;
    float in_w_rte;
    float out_w_rte;
    TFModbusTCPFunctionCode function_code;
    uint16_t start_address;
    uint16_t data_count;

    ctx->state = ctx->state_next;

    switch (ctx->state) {
    case BatterySunSpec::WriterState::Idle:
        esp_system_abort("Idle state during writing");

    case BatterySunSpec::WriterState::ReadInOutWRteSF:
        function_code = TFModbusTCPFunctionCode::ReadHoldingRegisters;
        start_address = ctx->start_address + 25; // InOutWRte_SF
        data_count = 1;
        ctx->buffer[0] = 0; // unsed
        ctx->buffer[1] = 0; // unsed

        ctx->state_next = BatterySunSpec::WriterState::WriteChaGriSet;
        break;

    case BatterySunSpec::WriterState::WriteChaGriSet:
        function_code = TFModbusTCPFunctionCode::WriteSingleRegister;
        start_address = ctx->start_address + 17; // ChaGriSet
        data_count = 1;
        ctx->buffer[0] = ctx->mode == BatteryMode::ForceCharge ? ChaGriSet_GRID : ChaGriSet_PV;
        ctx->buffer[1] = 0; // unsed

        ctx->state_next = BatterySunSpec::WriterState::WriteStorCtlMod;
        break;

    case BatterySunSpec::WriterState::WriteStorCtlMod:
        switch (ctx->mode) {
        case BatteryMode::None:
            esp_system_abort("None mode during writing");

        case BatteryMode::Block:
            stor_ctl_mod = StorCtlMod_CHARGE | StorCtlMod_DISCHARGE;
            break;

        case BatteryMode::Normal:
            stor_ctl_mod = 0;
            break;

        case BatteryMode::BlockDischarge:
            stor_ctl_mod = StorCtlMod_DISCHARGE;
            break;

        case BatteryMode::ForceCharge:
            stor_ctl_mod = StorCtlMod_CHARGE | StorCtlMod_DISCHARGE;
            break;

        case BatteryMode::BlockCharge:
            stor_ctl_mod = StorCtlMod_CHARGE;
            break;

        case BatteryMode::ForceDischarge:
            stor_ctl_mod = StorCtlMod_CHARGE | StorCtlMod_DISCHARGE;
            break;

        default:
            esp_system_abortf<48>("Invalid mode during writing: %d", static_cast<int>(ctx->mode));
        }

        function_code = TFModbusTCPFunctionCode::WriteSingleRegister;
        start_address = ctx->start_address + 5; // StorCtlMod
        data_count = 1;
        ctx->buffer[0] = stor_ctl_mod;
        ctx->buffer[1] = 0; // unsed

        ctx->state_next = BatterySunSpec::WriterState::WriteInOutWRte;
        break;

    case BatterySunSpec::WriterState::WriteInOutWRte:
        switch (ctx->mode) {
        case BatteryMode::None:
            esp_system_abort("None mode during writing");

        case BatteryMode::Block:
            in_w_rte = 0;
            out_w_rte = 0;
            break;

        case BatteryMode::Normal:
            in_w_rte = 100;
            out_w_rte = 100;
            break;

        case BatteryMode::BlockDischarge:
            in_w_rte = 100;
            out_w_rte = 0;
            break;

        case BatteryMode::ForceCharge:
            in_w_rte = ctx->force_charge_rate;
            out_w_rte = -ctx->force_charge_rate;
            break;

        case BatteryMode::BlockCharge:
            in_w_rte = 0;
            out_w_rte = 100;
            break;

        case BatteryMode::ForceDischarge:
            in_w_rte = -ctx->force_discharge_rate;
            out_w_rte = ctx->force_discharge_rate;
            break;

        default:
            esp_system_abortf<48>("Invalid mode during writing: %d", static_cast<int>(ctx->mode));
        }

        function_code = TFModbusTCPFunctionCode::WriteMultipleRegisters;
        start_address = ctx->start_address + 12; // OutWRte + InWRte
        data_count = 2;
        ctx->buffer[0] = static_cast<uint16_t>(out_w_rte / ctx->in_out_w_rte_scale_factor);
        ctx->buffer[1] = static_cast<uint16_t>(in_w_rte / ctx->in_out_w_rte_scale_factor);

        ctx->state_next = BatterySunSpec::WriterState::WriteInOutWRteRvrtTms;
        break;

    case BatterySunSpec::WriterState::WriteInOutWRteRvrtTms:
        function_code = TFModbusTCPFunctionCode::WriteSingleRegister;
        start_address = ctx->start_address + 15; // InOutWRte_RvrtTms
        data_count = 1;
        ctx->buffer[0] = seconds_t{WATCHDOG_TIMEOUT}.as<uint16_t>();
        ctx->buffer[1] = 0; // unsed

        ctx->state_next = BatterySunSpec::WriterState::Done;
        break;

    case BatterySunSpec::WriterState::Done:
        writer_finish(ctx, true);
        return;

    default:
        esp_system_abortf<48>("Invalid state during writing: %d", static_cast<int>(ctx->state));
    }

    ctx->transact_pending = true;

    ctx->shared_client->transact(ctx->device_address,
                                 function_code,
                                 start_address,
                                 data_count,
                                 static_cast<void *>(ctx->buffer),
                                 2_s,
    [ctx, start_address, data_count](TFModbusTCPClientTransactionResult result, const char *error_message) {
        if (ctx->destroy_requested) {
            delete ctx;
            return;
        }

        ctx->transact_pending = false;

        if (result != TFModbusTCPClientTransactionResult::Success) {
            trace("b%lu t%d ww m%c s%d a%u c%u r%u,%u e%d%s%s",
                  ctx->slot,
                  ctx->test ? 1 : 0,
                  get_battery_mode_as_char(ctx->mode),
                  static_cast<int>(ctx->state),
                  start_address,
                  data_count,
                  ctx->buffer[0], ctx->buffer[1],
                  static_cast<int>(result),
                  error_message != nullptr ? " / " : "",
                  error_message != nullptr ? error_message : "");

            writer_logfln(true,
                          "Setting mode \"%s\" failed at step %d: %s (%d)%s%s",
                          "Setzen des Modus \"%s\" schlug fehl bei Schritt %d: %s (%d)%s%s",
                          Batteries::get_battery_mode_display_name(ctx->mode, ctx->language),
                          static_cast<int>(ctx->state),
                          get_tf_modbus_tcp_client_transaction_result_name(result),
                          static_cast<int>(result),
                          error_message != nullptr ? " / " : "",
                          error_message != nullptr ? error_message : "");

            writer_finish(ctx, false);
            return;
        }

        trace("b%lu t%d ww m%c s%d a%u c%u r%u,%u",
              ctx->slot,
              ctx->test ? 1 : 0,
              get_battery_mode_as_char(ctx->mode),
              static_cast<int>(ctx->state),
              start_address,
              data_count,
              ctx->buffer[0], ctx->buffer[1]);

        if (ctx->state == BatterySunSpec::WriterState::ReadInOutWRteSF) {
            ctx->in_out_w_rte_scale_factor = get_sun_spec_scale_factor(static_cast<int16_t>(ctx->buffer[0]));
        }

        writer_next(ctx); // FIXME: maybe add a little delay between writes to avoid bursts?
    });
}

BatterySunSpec::WriterContext *BatterySunSpec::create_writer(BatterySunSpec *battery,
                                                             uint32_t slot,
                                                             bool test,
                                                             TFModbusTCPSharedClient *shared_client,
                                                             uint8_t device_address,
                                                             uint16_t start_address,
                                                             int8_t force_charge_rate,
                                                             int8_t force_discharge_rate,
                                                             BatteryMode mode,
                                                             const char *log_prefix,
                                                             VLogFLnFunction &&vlogfln,
                                                             WriterFailureFunction &&failure,
                                                             Language language /*= Language::English*/)
{
    trace("b%lu t%d wc m%c",
          slot,
          test ? 1 : 0,
          get_battery_mode_as_char(mode));

    WriterContext *ctx = new WriterContext;

    ctx->language = language;
    ctx->battery = battery;
    ctx->slot = slot;
    ctx->shared_client = shared_client;
    ctx->device_address = device_address;
    ctx->start_address = start_address;
    ctx->force_charge_rate = force_charge_rate;
    ctx->force_discharge_rate = force_discharge_rate;
    ctx->mode = mode;
    ctx->log_prefix = log_prefix;
    ctx->vlogfln = std::move(vlogfln);
    ctx->failure = std::move(failure);
    ctx->test = test;
    ctx->task_id = task_scheduler.scheduleOnce([ctx]() {
        if (ctx->destroy_requested) {
            delete ctx;
            return;
        }

        ctx->task_id = 0;

        trace("b%lu t%d ww m%c",
              ctx->slot,
              ctx->test ? 1 : 0,
              get_battery_mode_as_char(ctx->mode));

        uint32_t repeat_interval_s = seconds_t{REPEAT_INTERVAL}.as<uint32_t>();

        writer_logfln(false,
                      "Setting mode \"%s\" (will repeat in %lu second%s)",
                      "Setze Modus \"%s\" (Wiederholung in %lu Sekunde%s)",
                      Batteries::get_battery_mode_display_name(ctx->mode, ctx->language),
                      repeat_interval_s,
                      repeat_interval_s > 1 ? (ctx->language == Language::English ? "s" : "n") : "");

        writer_next(ctx);
    });

    return ctx;
}

void BatterySunSpec::destroy_writer(BatterySunSpec::WriterContext *ctx)
{
    if (ctx == nullptr) {
        return;
    }

    trace("b%lu t%d wd m%c",
          ctx->slot,
          ctx->test ? 1 : 0,
          get_battery_mode_as_char(ctx->mode));


    if (ctx->transact_pending) {
        ctx->destroy_requested = true;
        return;
    }

    task_scheduler.cancel(ctx->task_id);
    delete ctx;
}

BatteryClassID BatterySunSpec::get_class() const
{
    return BatteryClassID::SunSpec;
}

void BatterySunSpec::setup(const Config &ephemeral_config)
{
    host = ephemeral_config.get("host")->asString();
    port = static_cast<uint16_t>(ephemeral_config.get("port")->asUint());
    device_address = ephemeral_config.get("device_address")->asUint8();
    manufacturer_name = ephemeral_config.get("manufacturer_name")->asString();
    model_name = ephemeral_config.get("model_name")->asString();
    serial_number = ephemeral_config.get("serial_number")->asString();
    model_instance = ephemeral_config.get("model_instance")->asUint16();
    force_charge_rate = static_cast<int8_t>(ephemeral_config.get("force_charge_rate")->asUint8());
    force_discharge_rate = static_cast<int8_t>(ephemeral_config.get("force_discharge_rate")->asUint8());

    snprintf(trace_prefix, sizeof(trace_prefix), "b%lur t0 ", slot);
}

void BatterySunSpec::register_events()
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

void BatterySunSpec::pre_reboot()
{
    stop_connection();
}

void BatterySunSpec::set_mode(BatteryMode mode)
{
    // FIXME: why is setting mode none accepted here?

    if (requested_mode == mode) {
        return;
    }

    requested_mode = mode;

    update_pending_mode();
}

void BatterySunSpec::set_testing(bool testing_)
{
    state->get("testing")->updateBool(testing_);

    if (this->testing == testing_) {
        return;
    }

    if (!testing_) {
        // clear mode in case the test is ending. if the global battery
        // control is active it will control the battery immediately after
        // the test has ended and set the mode again
        set_state_mode(BatteryMode::None);
    }

    this->testing = testing_;

    update_pending_mode();
}

void BatterySunSpec::set_state_mode(BatteryMode mode)
{
    state->get("mode")->updateEnum(mode);
}

void BatterySunSpec::set_state_resolving(bool resolving)
{
    state->get("resolving")->updateBool(resolving);
}

void BatterySunSpec::connect_callback(TFGenericTCPClientConnectResult result, TFGenericTCPClientPoolShareLevel share_level)
{
    trace("b%lu t0 cc%d sl%d", slot, static_cast<int>(result), static_cast<int>(share_level));

    if (result != TFGenericTCPClientConnectResult::Connected) {
        return;
    }

    last_connect = now_us();
    resolve_pending = true; // (re-)resolve after (re-)connect

    update_pending_mode();
}

void BatterySunSpec::disconnect_callback(TFGenericTCPClientDisconnectReason reason, TFGenericTCPClientPoolShareLevel share_level)
{
    trace("b%lu t0 cd%d sl%d", slot, static_cast<int>(reason), static_cast<int>(share_level));

    update_pending_mode();
}

void BatterySunSpec::update_pending_mode()
{
    bool start_resolve;
    BatteryMode next_mode;

    if (requested_mode == BatteryMode::None || shared_client == nullptr || shared_client->get_connection_status() != TFGenericTCPClientConnectionStatus::Connected || testing) {
        start_resolve = false;
        next_mode = BatteryMode::None;
    }
    else if (resolve_pending) {
        start_resolve = true;
        next_mode = BatteryMode::None;
    }
    else {
        start_resolve = false;
        next_mode = requested_mode;
    }

    if (pending_mode == next_mode && (resolver != nullptr) == start_resolve) {
        return;
    }

    task_scheduler.cancel(resolve_start_delayed_task_id);
    resolve_start_delayed_task_id = 0;

    if (resolver != nullptr) {
        resolver->destroy();
        resolver = nullptr;

        set_state_resolving(false);
    }

    destroy_writer(writer_ctx);
    writer_ctx = nullptr;

    trace("b%lu t0 %s r%c%s m%c->%c%s",
          slot,
          shared_client != nullptr && shared_client->get_connection_status() == TFGenericTCPClientConnectionStatus::Connected ? "ce" : "nc",
          get_battery_mode_as_char(requested_mode),
          testing ? " tg" : "",
          get_battery_mode_as_char(pending_mode),
          get_battery_mode_as_char(next_mode),
          start_resolve ? " sr" : "");

    pending_mode = next_mode;

    if (start_resolve) {
        set_state_resolving(true);

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=format"
#endif
        resolver = SunSpecResolver::create(event_log_message_prefix,
                                           [this](const char *fmt, va_list args) { logger.vprintfln(fmt, args); },
                                           trace_prefix,
                                           [](const char *fmt, va_list args) { batteries_sun_spec.trace_timestamp(); logger.vtracefln_plain(batteries_sun_spec.trace_buffer_index, fmt, args); },
                                           [](const char *buf, size_t len) { batteries_sun_spec.trace_timestamp(); logger.trace_plain(batteries_sun_spec.trace_buffer_index, buf, len); },
                                           shared_client,
                                           device_address,
                                           manufacturer_name.c_str(),
                                           model_name.c_str(),
                                           serial_number.c_str(),
                                           124,
                                           model_instance,
                                           [this](SunSpecResolverCommonModel *common_model, uint16_t start_address, uint16_t block_length) { resolve_result(common_model, start_address, block_length); },
                                           []() {});
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    }
    else if (next_mode != BatteryMode::None) {
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=format"
#endif
        writer_ctx = create_writer(this, slot, false, static_cast<TFModbusTCPSharedClient *>(shared_client), device_address,
                                   resolved_address, force_charge_rate, force_discharge_rate, pending_mode, event_log_message_prefix,
        [this](bool event_log, const char *fmt, va_list args) {
            if (!event_log) {
                return;
            }

            logger.vprintfln(fmt, args);
        },
        [this]() {
            update_pending_mode();
        });
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    }
}

void BatterySunSpec::resolve_result(SunSpecResolverCommonModel *common_model, uint16_t start_address, uint16_t block_length)
{
    resolver = nullptr;

    set_state_resolving(false);

    bool error = false;

    if (common_model == nullptr) {
        error = true;
    }
    else if (block_length != 24) {
        logger.printfln_battery("SunSpec model 124/%u at %s:%u:%u:%u has unsupported length: %u",
                                model_instance, host.c_str(), port, device_address, start_address, block_length);
        error = true;
    }

    if (error) {
        resolve_start_delayed_task_id = task_scheduler.scheduleOnce([this]() {
            resolve_start_delayed_task_id = 0;

            if (deadline_elapsed(last_connect + RESOLVE_TIMEOUT)) {
                logger.printfln_battery("Looking for SunSpec model 124/%u takes too long, reconnecting to %s:%u",
                                        model_instance, host.c_str(), port);
                force_reconnect();
                return;
            }

            update_pending_mode();
        }, 5_s);

        return;
    }

    resolved_address = start_address;
    resolve_pending = false;

    update_pending_mode();
}
