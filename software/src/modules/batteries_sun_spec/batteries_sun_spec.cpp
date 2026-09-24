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

#include "batteries_sun_spec.h"

#include <TFJson.h>

#include "battery_sun_spec.h"
#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tools/printf.h"

#include "gcc_warnings.h"

#define trace(fmt, ...) \
    do { \
        trace_timestamp(); \
        logger.tracefln_plain(trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

void BatteriesSunSpec::pre_setup()
{
    this->trace_buffer_index = logger.alloc_trace_buffer("batteries_sunspc");

    for (size_t i = 0; i < OPTIONS_BATTERIES_MAX_SLOTS(); ++i) {
        instances[i] = nullptr;
    }

    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 32)},
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"device_address", Config::Uint8(1)},
        {"manufacturer_name", Config::Str("", 0, 32)},
        {"model_name", Config::Str("", 0, 32)},
        {"serial_number", Config::Str("", 0, 32)},
        {"model_instance", Config::Uint16(0)},
        {"force_charge_rate", Config::Uint8(100, 100)},
        {"force_discharge_rate", Config::Uint8(100, 100)},
    });

    state_prototype = Config::Object({
        {"mode", Config::Enum(BatteryMode::None)},
        {"resolving", Config::Bool(false)},
        {"testing", Config::Bool(false)},
    });

    /*errors_prototype = Config::Object({
        {"timeout", Config::Uint32(0)},
    });*/

    batteries.register_battery_generator(get_class(), this);

    test_config = ConfigRoot{Config::Object({
        {"slot", Config::Uint(0, 0, OPTIONS_BATTERIES_MAX_SLOTS() - 1)},
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"device_address", Config::Uint8(1)},
        {"manufacturer_name", Config::Str("", 0, 32)},
        {"model_name", Config::Str("", 0, 32)},
        {"serial_number", Config::Str("", 0, 32)},
        {"model_instance", Config::Uint16(0)},
        {"force_charge_rate", Config::Uint8(100, 100)},
        {"force_discharge_rate", Config::Uint8(100, 100)},
        {"mode", Config::Enum(BatteryMode::Block)},
        {"cookie", Config::Uint32(0)},
    })};

    test_continue_config = ConfigRoot{Config::Object({
        {"cookie", Config::Uint32(0)},
    })};

    test_stop_config = test_continue_config;
}

void BatteriesSunSpec::register_urls()
{
    api.addCommand("batteries_sun_spec/test", &test_config, {}, [this](Language language, String &errmsg) {
        defer {
            // When done parsing the test command, drop Strings and Array items from config to free memory
            test_config.get("host")->clearString();
            test_config.get("manufacturer_name")->clearString();
            test_config.get("model_name")->clearString();
            test_config.get("serial_number")->clearString();
        };

        if (test != nullptr) {
            errmsg = language == Language::English
                     ? "Another test is already in progress, please try again later!"
                     : "Ein anderer Test läuft bereits, bitte später noch einmal versuchen!";
            return;
        }

        test = new Test;
        test->language = language;
        test->last_keep_alive = now_us();
        test->slot = test_config.get("slot")->asUint();
        test->host = test_config.get("host")->asString();
        test->port = static_cast<uint16_t>(test_config.get("port")->asUint());
        test->device_address = test_config.get("device_address")->asUint8();
        test->manufacturer_name = test_config.get("manufacturer_name")->asString();
        test->model_name = test_config.get("model_name")->asString();
        test->serial_number = test_config.get("serial_number")->asString();
        test->model_instance = test_config.get("model_instance")->asUint16();
        test->force_charge_rate = static_cast<int8_t>(test_config.get("force_charge_rate")->asUint8());
        test->force_discharge_rate = static_cast<int8_t>(test_config.get("force_discharge_rate")->asUint8());
        test->mode = test_config.get("mode")->asEnum<BatteryMode>();
        test->cookie = test_config.get("cookie")->asUint();
        test->state = TestState::Start;

        snprintf(test->trace_prefix, sizeof(test->trace_prefix), "b%lur t1 ", test->slot);

        test_printfln(language == Language::English
                      ? "Starting test for mode \"%s\""
                      : "Starte Test für Modus \"%s\"",
                      Batteries::get_battery_mode_display_name(test->mode, language));
    }, true);

    api.addCommand("batteries_sun_spec/test_continue", &test_continue_config, {}, [this](Language language, String &errmsg) {
        if (test == nullptr) {
            return;
        }

        uint32_t cookie = test_continue_config.get("cookie")->asUint();

        if (cookie != test->cookie) {
            errmsg = language == Language::English ? "Cannot continue another test" : "Kann keinen anderen Test fortsetzen";
            return;
        }

        test->language        = language;
        test->last_keep_alive = now_us();
    }, true);

    api.addCommand("batteries_sun_spec/test_stop", &test_stop_config, {}, [this](Language language, String &errmsg) {
        if (test == nullptr) {
            return;
        }

        uint32_t cookie = test_stop_config.get("cookie")->asUint();

        if (cookie != test->cookie) {
            errmsg = language == Language::English ? "Cannot stop another test" : "Kann keinen anderen Test stoppen";
            return;
        }

        test->language = language;
        test->stop     = true;
    }, true);
}

void BatteriesSunSpec::loop()
{
    if (test == nullptr) {
        return;
    }

    if (test->printfln_buffer_used > 0 && deadline_elapsed(test->printfln_last_flush + 500_ms)) {
        test_flush_log();
    }

    if (!test->stop && deadline_elapsed(test->last_keep_alive + 10_s)) {
        const char *message = test->language == Language::English
                              ? "Stopping test because no continue call was received for more than 10 seconds"
                              : "Stoppe Test, da für mehr als 10 Sekunden kein Fortsetzen-Aufruf empfangen wurde";

        logger.printfln("%s", message);
        test_printfln("%s", message);

        test->stop = true;
    }

    switch (test->state) {
    case TestState::Start:
        if (instances[test->slot] != nullptr) {
            instances[test->slot]->set_testing(true);
        }

        test->state = TestState::Connect;
        break;

    case TestState::Connect:
        if (test->stop) {
            test->state = TestState::Done;
            break;
        }

        test_printfln(test->language == Language::English ? "Connecting to %s:%u" : "Verbinde zu %s:%u", test->host.c_str(), test->port);

        test->reconnect = false;
        test->state = TestState::Connecting;

        modbus_tcp_client.get_pool()->acquire(test->host.c_str(), test->port, &test->shared_client,
        [this](TFGenericTCPClientConnectResult result, int error_number, TFGenericTCPSharedClient *shared_client, TFGenericTCPClientPoolShareLevel share_level) {
            trace("b%lu t1 cc%d sl%d", test->slot, static_cast<int>(result), static_cast<int>(share_level));

            test->shared_client = shared_client;

            if (result != TFGenericTCPClientConnectResult::Connected) {
                char buf[256] = "";

                GenericTCPClientPoolConnector::format_connect_error(result, error_number, share_level, test->host.c_str(), test->port, buf, sizeof(buf), test->language);
                test_printfln("%s", buf);

                test->state = TestState::Done;
                return;
            }

            test->state = TestState::CreateResolver;
        },
        [this](TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPSharedClient *shared_client, TFGenericTCPClientPoolShareLevel share_level) {
            trace("b%lu t1 cd%d sl%d", test->slot, static_cast<int>(reason), static_cast<int>(share_level));

            char buf[256] = "";

            GenericTCPClientPoolConnector::format_disconnect_reason(reason, error_number, share_level, test->host.c_str(), test->port, buf, sizeof(buf), test->language);
            test_printfln("%s", buf);

            test->shared_client = nullptr;
            test->reconnect = reason == TFGenericTCPClientDisconnectReason::Forced;

            if (test->state == TestState::Resolving) {
                test->state = TestState::DestroyResolver;
                test->state_after_destroy_resolver = TestState::Disconnect;

                // immediately destroy the resolver to stop the separate
                // resolver task from accessing the disconnected client
                test->resolver->destroy();
                test->resolver = nullptr;
            }
            else {
                test->state = TestState::DestroyWriter;

                // immediately destroy the writer to stop the separate
                // writer task from accessing the disconnected client
                BatterySunSpec::destroy_writer(test->writer_ctx);
                test->writer_ctx = nullptr;
            }
        });

        break;

    case TestState::Connecting:
        break;

    case TestState::Disconnect:
        if (test->shared_client != nullptr) {
            modbus_tcp_client.get_pool()->release(test->shared_client);
        }
        else {
            test->state = TestState::Done;
        }

        break;

    case TestState::Done: {
            test_printfln(test->language == Language::English
                          ? (test->stop ? "Test stopped" : "Test finished")
                          : (test->stop ? "Test gestoppt" : "Test abgeschlossen"));
            test_flush_log();

#if MODULE_WS_AVAILABLE()
            char buf[128];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", test->cookie);
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "batteries_sun_spec/test_done")) {
                break; // need to report the test as done before doing something else
            }
#endif

            if (instances[test->slot] != nullptr) {
                instances[test->slot]->set_testing(false);
            }

            delete test;
            test = nullptr;
        }

        break;

    case TestState::CreateResolver:
        if (test->stop) {
            test->state = TestState::Disconnect;
            break;
        }

        test->state = TestState::Resolving;

        if (instances[test->slot] != nullptr) {
            instances[test->slot]->set_state_resolving(true);
        }

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsuggest-attribute=format"
#endif
        test->resolver = SunSpecResolver::create("",
                                                 [this](const char *fmt, va_list args) { test_vprintfln(fmt, args); },
                                                 test->trace_prefix,
                                                 [](const char *fmt, va_list args) { batteries_sun_spec.trace_timestamp(); logger.vtracefln_plain(batteries_sun_spec.trace_buffer_index, fmt, args); },
                                                 [](const char *buf, size_t len) { batteries_sun_spec.trace_timestamp(); logger.trace_plain(batteries_sun_spec.trace_buffer_index, buf, len); },
                                                 test->shared_client,
                                                 test->device_address,
                                                 test->manufacturer_name.c_str(),
                                                 test->model_name.c_str(),
                                                 test->serial_number.c_str(),
                                                 124,
                                                 test->model_instance,
                                                 [this](SunSpecResolverCommonModel *common_model, uint16_t start_address, uint16_t block_length) { test_resolve_result(common_model, start_address, block_length); },
                                                 []() {},
                                                 test->language);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
        break;

    case TestState::DestroyResolver:
        if (test->resolver != nullptr) {
            test->resolver->destroy();
            test->resolver = nullptr;
        }

        if (instances[test->slot] != nullptr) {
            instances[test->slot]->set_state_resolving(false);
        }

        test->state = test->state_after_destroy_resolver;
        break;

    case TestState::Resolving:
        if (test->stop) {
            test->state = TestState::DestroyResolver;
            test->state_after_destroy_resolver = TestState::Disconnect;
            break;
        }

        break;

    case TestState::CreateWriter:
        if (test->stop) {
            test->state = TestState::Disconnect;
            break;
        }

        test->state = TestState::Writing;

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wsuggest-attribute=format"
#endif
        test->writer_ctx = BatterySunSpec::create_writer(instances[test->slot], test->slot, true, static_cast<TFModbusTCPSharedClient *>(test->shared_client),
                                                         test->device_address, test->resolved_address, test->force_charge_rate, test->force_discharge_rate, test->mode, "",
        [this](bool event_log, const char *fmt, va_list args) {
            test_vprintfln(fmt, args);
        },
        [this]() {
            test->state = TestState::DestroyWriter;
        },
        test->language);
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

        break;

    case TestState::DestroyWriter:
        BatterySunSpec::destroy_writer(test->writer_ctx);
        test->writer_ctx = nullptr;

        if (test->shared_client != nullptr) {
            test->state = TestState::Disconnect;
        }
        else if (test->reconnect) {
            test->state = TestState::Connect;
        }
        else {
            test->state = TestState::Done;
        }

        break;

    case TestState::Writing:
        if (test->stop) {
            test->state = TestState::DestroyWriter;
            break;
        }

        break;

    default:
        esp_system_abortf<48>("Invalid state during test: %d", static_cast<int>(test->state));
    }
}

[[gnu::const]] BatteryClassID BatteriesSunSpec::get_class() const
{
    return BatteryClassID::SunSpec;
}

IBattery *BatteriesSunSpec::new_battery(uint32_t slot, Config *state, Config *errors)
{
    BatterySunSpec *battery = new BatterySunSpec(slot, state, errors, modbus_tcp_client.get_pool());

    instances[slot] = battery;

    return battery;
}

const Config *BatteriesSunSpec::get_config_prototype()
{
    return &config_prototype;
}

const Config *BatteriesSunSpec::get_state_prototype()
{
    return &state_prototype;
}

const Config *BatteriesSunSpec::get_errors_prototype()
{
    return Config::Null();//&errors_prototype;
}

void BatteriesSunSpec::test_resolve_result(SunSpecResolverCommonModel *common_model, uint16_t start_address, uint16_t block_length)
{
    if (test == nullptr) {
        return;
    }

    test->resolver = nullptr;

    if (common_model == nullptr) {
        test->state = TestState::Disconnect;
        return;
    }

    if (block_length != 24) {
        test_printfln(test->language == Language::English
                      ? "SunSpec model 124/%u at %s:%u:%u:%u has unsupported length: %u"
                      : "SunSpec-Modell 124/%u unter %s:%u:%u:%u hat nicht unterstüzte Länge: %u",
                      test->model_instance, test->host.c_str(), test->port, test->device_address, start_address, block_length);
        test->state = TestState::Disconnect;
        return;
    }

    test->resolved_address = start_address;
    test->state = TestState::DestroyResolver;
    test->state_after_destroy_resolver = TestState::CreateWriter;
}

void BatteriesSunSpec::test_flush_log()
{
    if (test == nullptr) {
        return;
    }

#if MODULE_WS_AVAILABLE()
    char buf[1024];
    TFJsonSerializer json{buf, sizeof(buf)};

    json.addObject();
    json.addMemberNumber("cookie", test->cookie);
    json.addMemberString("message", test->printfln_buffer);
    json.endObject();
    json.end();
#endif

    test->printfln_buffer_used = 0;
    test->printfln_last_flush = now_us();

#if MODULE_WS_AVAILABLE()
    ws.pushRawStateUpdate(buf, "batteries_sun_spec/test_log"); // FIXME: error handling
#endif
}

void BatteriesSunSpec::test_vprintfln(const char *fmt, va_list args)
{
    if (test == nullptr) {
        return;
    }

    va_list args_copy;
    va_copy(args_copy, args);
    size_t used = vsnprintf_u(nullptr, 0, fmt, args_copy);
    va_end(args);

    if (test->printfln_buffer_used + used + 1 /* for \n */ >= sizeof(test->printfln_buffer)) {
        test_flush_log();
    }

    test->printfln_buffer_used += vsnprintf_u(test->printfln_buffer + test->printfln_buffer_used, sizeof(test->printfln_buffer) - test->printfln_buffer_used, fmt, args);

    test->printfln_buffer[test->printfln_buffer_used++] = '\n';
    test->printfln_buffer[test->printfln_buffer_used] = '\0';
}

void BatteriesSunSpec::test_printfln(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    test_vprintfln(fmt, args);
    va_end(args);
}

void BatteriesSunSpec::trace_timestamp()
{
    if (last_trace_timestamp < 0_us || deadline_elapsed(last_trace_timestamp + 1_min)) {
        last_trace_timestamp = now_us();
        logger.trace_timestamp(trace_buffer_index);
    }
}
