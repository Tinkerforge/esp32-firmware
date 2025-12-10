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

#include "batteries_modbus_tcp.h"

#include <TFJson.h>

#include "battery_modbus_tcp.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "battery_modbus_tcp_specs.h"
#include "modules/modbus_tcp_client/modbus_register_address_mode.enum.h"
#include "tools.h"

#include "gcc_warnings.h"

#define trace(fmt, ...) \
    do { \
        trace_timestamp(); \
        logger.tracefln_plain(trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

void BatteriesModbusTCP::pre_setup()
{
    this->trace_buffer_index = logger.alloc_trace_buffer("batteries_mbtcp", 8192);

    for (size_t i = 0; i < OPTIONS_BATTERIES_MAX_SLOTS(); ++i) {
        instances[i] = nullptr;
    }

    table_prototypes.push_back({BatteryModbusTCPTableID::None, *Config::Null()});

    table_custom_register_block_prototype = Config::Object({
        {"desc", Config::Str("", 0, 32)},
        {"func", Config::Enum(ModbusFunctionCode::WriteMultipleRegisters)},
        {"addr", Config::Uint16(0)},
        {"vals", Config::Array({},
            Config::get_prototype_uint16_0(),
            1,
            OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_VALUES_PER_REGISTER_BLOCK(),
            Config::type_id<Config::ConfUint>()
        )},
    });

    table_prototypes.push_back({BatteryModbusTCPTableID::Custom, Config::Object({
        {"device_address", Config::Uint8(1)},
        {"repeat_interval", Config::Uint16(60)},
        {"register_address_mode", Config::Enum(ModbusRegisterAddressMode::Address)},
        {"battery_modes", Config::Tuple(6, Config::Object({
            {"register_blocks", Config::Array({},
                &table_custom_register_block_prototype,
                0,
                OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS(),
                Config::type_id<Config::ConfObject>()
            )},
        }))},
    })});

    get_battery_modbus_tcp_table_prototypes(&table_prototypes);

    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 32)},
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"table", Config::Union<BatteryModbusTCPTableID>(*Config::Null(),
            BatteryModbusTCPTableID::None,
            table_prototypes.data(),
            static_cast<uint8_t>(table_prototypes.size())
        )},
    });

    /*errors_prototype = Config::Object({
        {"timeout", Config::Uint32(0)},
    });*/

    batteries.register_battery_generator(get_class(), this);

    test_table_prototypes.push_back({BatteryModbusTCPTableID::Custom, Config::Object({
        {"device_address", Config::Uint8(1)},
        {"repeat_interval", Config::Uint16(60)},
        {"register_blocks", Config::Array({},
            &table_custom_register_block_prototype,
            0,
            OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS(),
            Config::type_id<Config::ConfObject>()
        )},
    })});

    get_battery_modbus_tcp_test_table_prototypes(&test_table_prototypes);

    test_config = ConfigRoot{Config::Object({
        {"slot", Config::Uint(0, 0, OPTIONS_BATTERIES_MAX_SLOTS())},
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"table", Config::Union<BatteryModbusTCPTableID>(*Config::Null(),
            BatteryModbusTCPTableID::None,
            test_table_prototypes.data(),
            static_cast<uint8_t>(test_table_prototypes.size())
        )},
        {"cookie", Config::Uint32(0)},
    })};

    test_continue_config = ConfigRoot{Config::Object({
        {"cookie", Config::Uint32(0)},
    })};

    test_stop_config = test_continue_config;
}

void BatteriesModbusTCP::register_urls()
{
    api.addCommand("batteries_modbus_tcp/test", &test_config, {}, [this](String &errmsg) {
        BatteryModbusTCPTableID table_id = test_config.get("table")->getTag<BatteryModbusTCPTableID>();
        Config *table_config = static_cast<Config *>(test_config.get("table")->get());

        defer {
            // When done parsing the test command, drop Strings and Array items from config to free memory
            test_config.get("host")->clearString();

            if (table_id == BatteryModbusTCPTableID::Custom) {
                table_config->get("register_blocks")->removeAll();
            }
        };

        if (test != nullptr) {
            errmsg = "Another test is already in progress, please try again later!";
            return;
        }

        test = new Test;

        test_printfln("Starting test");

        test->slot = test_config.get("slot")->asUint();
        test->host = test_config.get("host")->asString();
        test->port = static_cast<uint16_t>(test_config.get("port")->asUint());
        test->cookie = test_config.get("cookie")->asUint();
        test->last_keep_alive = now_us();
        test->state = TestState::Done;

        switch (table_id) {
        case BatteryModbusTCPTableID::None:
            test_printfln("No table");
            return;

        case BatteryModbusTCPTableID::Custom:
            test->device_address = table_config->get("device_address")->asUint8();
            test->repeat_interval = table_config->get("repeat_interval")->asUint16();
            test->mode = BatteryMode::None;

            BatteryModbusTCP::load_custom_table(&test->table, table_config);

            if (test->table == nullptr) {
                test_printfln("Invalid custom table");
                return;
            }

            break;

        case BatteryModbusTCPTableID::VictronEnergyGX:
            test->device_address = table_config->get("device_address")->asUint8();
            test->mode = table_config->get("mode")->asEnum<BatteryMode>();

            load_victron_energy_gx_table(&test->table, &test->repeat_interval, test->mode, table_config);

            if (test->table == nullptr) {
                test_printfln("Unknown Victron Energy GX mode: %u", static_cast<uint8_t>(test->mode));
                return;
            }

            break;

        case BatteryModbusTCPTableID::DeyeHybridInverter:
            test->device_address = table_config->get("device_address")->asUint8();
            test->mode = table_config->get("mode")->asEnum<BatteryMode>();

            load_deye_hybrid_inverter_table(&test->table, &test->repeat_interval, test->mode, table_config);

            if (test->table == nullptr) {
                test_printfln("Unknown Deye Hybrid Inverter mode: %u", static_cast<uint8_t>(test->mode));
                return;
            }

            break;

        case BatteryModbusTCPTableID::AlphaESSHybridInverter:
            test->device_address = table_config->get("device_address")->asUint8();
            test->mode = table_config->get("mode")->asEnum<BatteryMode>();

            load_alpha_ess_hybrid_inverter_table(&test->table, &test->repeat_interval, test->mode, table_config);

            if (test->table == nullptr) {
                test_printfln("Unknown Alpha ESS Hybrid Inverter mode: %u", static_cast<uint8_t>(test->mode));
                return;
            }

            break;

        case BatteryModbusTCPTableID::HaileiHybridInverter:
            test->device_address = table_config->get("device_address")->asUint8();
            test->mode = table_config->get("mode")->asEnum<BatteryMode>();

            load_hailei_hybrid_inverter_table(&test->table, &test->repeat_interval, test->mode, table_config);

            if (test->table == nullptr) {
                test_printfln("Unknown Hailei Hybrid Inverter mode: %u", static_cast<uint8_t>(test->mode));
                return;
            }

            break;

        case BatteryModbusTCPTableID::SungrowHybridInverter:
            test->device_address = table_config->get("device_address")->asUint8();
            test->mode = table_config->get("mode")->asEnum<BatteryMode>();

            load_sungrow_hybrid_inverter_table(&test->table, &test->repeat_interval, test->mode, table_config);

            if (test->table == nullptr) {
                test_printfln("Unknown Sungrow Hybrid Inverter mode: %u", static_cast<uint8_t>(test->mode));
                return;
            }

            break;

        default:
            test_printfln("Unknown table: %u", static_cast<uint8_t>(table_id));
            return;
        }

        test->state = TestState::Connect;
    }, true);

    api.addCommand("batteries_modbus_tcp/test_continue", &test_continue_config, {}, [this](String &errmsg) {
        if (test == nullptr) {
            return;
        }

        uint32_t cookie = test_continue_config.get("cookie")->asUint();

        if (cookie != test->cookie) {
            errmsg = "Cannot continue another test";
            return;
        }

        test->last_keep_alive = now_us();
    }, true);

    api.addCommand("batteries_modbus_tcp/test_stop", &test_stop_config, {}, [this](String &errmsg) {
        if (test == nullptr) {
            return;
        }

        uint32_t cookie = test_stop_config.get("cookie")->asUint();

        if (cookie != test->cookie) {
            errmsg = "Cannot stop another test";
            return;
        }

        test->stop = true;
    }, true);
}

void BatteriesModbusTCP::loop()
{
    if (test == nullptr) {
        return;
    }

    if (test->printfln_buffer_used > 0 && deadline_elapsed(test->printfln_last_flush + 500_ms)) {
        test_flush_log();
    }

    if (!test->stop && deadline_elapsed(test->last_keep_alive + 10_s)) {
        const char *message = "Stopping test because no continue call was received for more than 10 seconds";

        logger.printfln("%s", message);
        test_printfln("%s", message);

        test->stop = true;
    }

    switch (test->state) {
    case TestState::Connect:
        if (test->stop) {
            test->state = TestState::Done;
            break;
        }

        test_printfln("Connecting to %s:%u", test->host.c_str(), test->port);

        test->reconnect = false;
        test->state = TestState::Connecting;

        modbus_tcp_client.get_pool()->acquire(test->host.c_str(), test->port,
        [this](TFGenericTCPClientConnectResult result, int error_number, TFGenericTCPSharedClient *shared_client, TFGenericTCPClientPoolShareLevel share_level) {
            trace("b%lu t1 cc%d", test->slot, static_cast<int>(result));

            if (result != TFGenericTCPClientConnectResult::Connected) {
                char buf[256] = "";

                GenericTCPClientConnectorBase::format_connect_error(result, error_number, share_level, test->host.c_str(), test->port, buf, sizeof(buf));
                test_printfln("%s", buf);

                test->state = TestState::Done;
                return;
            }

            test->client = shared_client;
            test->state = TestState::CreateTableWriter;
        },
        [this](TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPSharedClient *shared_client, TFGenericTCPClientPoolShareLevel share_level) {
            trace("b%lu t1 cd%d", test->slot, static_cast<int>(reason));

            char buf[256] = "";

            GenericTCPClientConnectorBase::format_disconnect_reason(reason, error_number, share_level, test->host.c_str(), test->port, buf, sizeof(buf));
            test_printfln("%s", buf);

            test->client = nullptr;
            test->reconnect = reason == TFGenericTCPClientDisconnectReason::Forced;
            test->state = TestState::DestroyTableWriter;

            // immediately destroy the writer to stop the separate
            // writer task from accessing the disconnected client
            BatteryModbusTCP::destroy_table_writer(test->writer);
            test->writer = nullptr;
        });

        break;

    case TestState::Connecting:
        break;

    case TestState::Disconnect:
        if (test->client != nullptr) {
            modbus_tcp_client.get_pool()->release(test->client);
        }
        else {
            test->state = TestState::Done;
        }

        break;

    case TestState::Done: {
            test_printfln(test->stop ? "Test stopped" : "Test finished");
            test_flush_log();

#if MODULE_WS_AVAILABLE()
            char buf[128];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", test->cookie);
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "batteries_modbus_tcp/test_done")) {
                break; // need to report the test as done before doing something else
            }
#endif

            delete test;
            test = nullptr;
        }

        break;

    case TestState::CreateTableWriter:
        if (test->stop) {
            test->state = TestState::Disconnect;
            break;
        }

        if (instances[test->slot] != nullptr) {
            instances[test->slot]->set_paused(true);
        }

        test->state = TestState::TableWriting;

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wsuggest-attribute=format"
#endif
        test->writer = BatteryModbusTCP::create_table_writer(test->slot, true, static_cast<TFModbusTCPSharedClient *>(test->client), test->device_address, test->repeat_interval, test->mode, test->table,
        [this](bool error, const char *fmt, va_list args) {
            test_vprintfln(fmt, args);
        },
        [this]() {
            test->state = TestState::DestroyTableWriter;
        });
#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

        break;

    case TestState::DestroyTableWriter:
        BatteryModbusTCP::destroy_table_writer(test->writer);
        test->writer = nullptr;

        if (!test->reconnect) {
            BatteryModbusTCP::free_table(test->table);
            test->table = nullptr;
        }

        if (instances[test->slot] != nullptr) {
            instances[test->slot]->set_paused(false);
        }

        if (test->client != nullptr) {
            test->state = TestState::Disconnect;
        }
        else if (test->reconnect) {
            test->state = TestState::Connect;
        }
        else {
            test->state = TestState::Done;
        }

        break;

    case TestState::TableWriting:
        if (test->stop) {
            test->state = TestState::DestroyTableWriter;
        }

        break;

    default:
        esp_system_abort("batteries_modbus_tcp: Invalid state.");
    }
}

[[gnu::const]] BatteryClassID BatteriesModbusTCP::get_class() const
{
    return BatteryClassID::ModbusTCP;
}

IBattery *BatteriesModbusTCP::new_battery(uint32_t slot, Config *state, Config *errors)
{
    BatteryModbusTCP *battery = new BatteryModbusTCP(slot, state, errors, modbus_tcp_client.get_pool());

    instances[slot] = battery;

    return battery;
}

const Config *BatteriesModbusTCP::get_config_prototype()
{
    return &config_prototype;
}

const Config *BatteriesModbusTCP::get_state_prototype()
{
    return Config::Null();
}

const Config *BatteriesModbusTCP::get_errors_prototype()
{
    return Config::Null();//&errors_prototype;
}

String BatteriesModbusTCP::validate_config(Config &update, ConfigSource source)
{
    if (update.get("table")->getTag<BatteryModbusTCPTableID>() != BatteryModbusTCPTableID::Custom) {
        return String();
    }

    size_t total_values_count = 0;

    for (size_t i = 0; i < 6; ++i) {
        auto register_blocks = update.get("table")->get()->get("battery_modes")->get(i)->get("register_blocks");

        for (size_t k = 0; k < register_blocks->count(); ++k) {
            // FIXME: validate func is valid and vals length match func restrictions

            auto start_address = register_blocks->get(k)->get("addr")->asUint();
            auto values_count  = register_blocks->get(k)->get("vals")->count();

            total_values_count += values_count;

            if (start_address + values_count > 65536) {
                return "Register address + number of values must be less than 65536!";
            }
        }
    }

    if (total_values_count > OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_TOTAL_VALUES()) {
        return "At most " MACRO_VALUE_TO_STRING(OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_TOTAL_VALUES()) " total values are allowed!";
    }

    return String();
}

void BatteriesModbusTCP::test_flush_log()
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
    ws.pushRawStateUpdate(buf, "batteries_modbus_tcp/test_log"); // FIXME: error handling
#endif
}

void BatteriesModbusTCP::test_vprintfln(const char *fmt, va_list args)
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

void BatteriesModbusTCP::test_printfln(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    test_vprintfln(fmt, args);
    va_end(args);
}

void BatteriesModbusTCP::trace_timestamp()
{
    if (last_trace_timestamp < 0_us || deadline_elapsed(last_trace_timestamp + 5_min)) {
        last_trace_timestamp = now_us();
        logger.trace_timestamp(trace_buffer_index);
    }
}
