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

#include "batteries_modbus_tcp.h"

#include <TFJson.h>

#include "battery_modbus_tcp.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modules/modbus_tcp_client/modbus_register_address_mode.enum.h"
#include "tools.h"

#include "gcc_warnings.h"

BatteriesModbusTCP::ValueTable *BatteriesModbusTCP::read_table_config(const Config *config)
{
    ValueTable *table = new ValueTable;

    table->device_address = static_cast<uint8_t>(config->get("device_address")->asUint());

    const Config *registers_config = static_cast<const Config *>(config->get("registers"));
    size_t registers_count         = registers_config->count();

    ValueSpec *values = new ValueSpec[registers_count];

    for (size_t i = 0; i < registers_count; ++i) {
        values[i].register_type = registers_config->get(i)->get("rtype")->asEnum<ModbusRegisterType>();
        values[i].start_address = static_cast<uint16_t>(registers_config->get(i)->get("addr")->asUint());
        values[i].value         = static_cast<uint16_t>(registers_config->get(i)->get("value")->asUint());
    }

    table->values = values;
    table->values_length = registers_count;

    return table;
}

void BatteriesModbusTCP::pre_setup()
{
    table_prototypes.push_back({BatteryModbusTCPTableID::None, *Config::Null()});

    table_custom_registers_prototype = Config::Object({
        {"rtype", Config::Uint8(static_cast<uint8_t>(ModbusRegisterType::HoldingRegister))},
        {"addr", Config::Uint16(0)},
        {"value", Config::Uint16(0)},
    });

    table_prototypes.push_back({BatteryModbusTCPTableID::Custom, Config::Object({
        {"register_address_mode", Config::Enum(ModbusRegisterAddressMode::Address)},
        {"permit_grid_charge", Config::Object({
            {"device_address", Config::Uint8(1)},
            {"registers", Config::Array({},
                &table_custom_registers_prototype,
                0,
                BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTERS,
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"revoke_grid_charge_override", Config::Object({
            {"device_address", Config::Uint8(1)},
            {"registers", Config::Array({},
                &table_custom_registers_prototype,
                0,
                BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTERS,
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"forbid_discharge", Config::Object({
            {"device_address", Config::Uint8(1)},
            {"registers", Config::Array({},
                &table_custom_registers_prototype,
                0,
                BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTERS,
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"revoke_discharge_override", Config::Object({
            {"device_address", Config::Uint8(1)},
            {"registers", Config::Array({},
                &table_custom_registers_prototype,
                0,
                BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTERS,
                Config::type_id<Config::ConfObject>()
            )},
        })},
    })});

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

    execute_table_prototypes.push_back({BatteryModbusTCPTableID::Custom, Config::Object({
        {"device_address", Config::Uint8(1)},
        {"registers", Config::Array({},
            &table_custom_registers_prototype,
            0,
            BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTERS,
            Config::type_id<Config::ConfObject>()
        )},
    })});

    execute_config = ConfigRoot{Config::Object({
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"table", Config::Union<BatteryModbusTCPTableID>(*Config::Null(),
            BatteryModbusTCPTableID::None,
            execute_table_prototypes.data(),
            static_cast<uint8_t>(execute_table_prototypes.size())
        )},
        {"cookie", Config::Uint32(0)},
    })};

    batteries.register_battery_generator(get_class(), this);
}

[[gnu::format(__printf__, 2, 3)]] static void report_errorf(uint32_t cookie, const char *fmt, ...);
static void report_errorf(uint32_t cookie, const char *fmt, ...)
{
    va_list args;
    char buf[256];
    TFJsonSerializer json{buf, sizeof(buf)};

    json.addObject();
    json.addMemberNumber("cookie", cookie);
    va_start(args, fmt);
    json.addMemberStringVF("error", fmt, args);
    va_end(args);
    json.endObject();
    json.end();

    ws.pushRawStateUpdate(buf, "batteries_modbus_tcp/execute_result");
}

static void report_success(uint32_t cookie)
{
    char buf[256];
    TFJsonSerializer json{buf, sizeof(buf)};

    json.addObject();
    json.addMemberNumber("cookie", cookie);
    json.addMemberNull("error");
    json.endObject();
    json.end();

    ws.pushRawStateUpdate(buf, "batteries_modbus_tcp/execute_result");
}

void BatteriesModbusTCP::register_urls()
{
    api.addCommand("batteries_modbus_tcp/execute", &execute_config, {}, [this](String &errmsg) {
        uint32_t cookie = execute_config.get("cookie")->asUint();

        if (execute_client != nullptr) {
            report_errorf(cookie, "Another action is already in progress");
            return;
        }

        BatteryModbusTCPTableID table_id = execute_config.get("table")->getTag<BatteryModbusTCPTableID>();

        switch (table_id) {
        case BatteryModbusTCPTableID::None:
            report_errorf(cookie, "No table");
            return;

        case BatteryModbusTCPTableID::Custom:
            break;

        default:
            report_errorf(cookie, "Unknown table: %u", static_cast<uint8_t>(table_id));
            return;
        }

        String host = execute_config.get("host")->asString();
        uint16_t port = static_cast<uint16_t>(execute_config.get("port")->asUint());
        const Config *table = static_cast<const Config *>(execute_config.get("table")->get());

        execute_cookie = cookie;
        execute_table = BatteriesModbusTCP::read_table_config(table);

        modbus_tcp_client.get_pool()->acquire(host.c_str(), port,
        [this, cookie, host, port](TFGenericTCPClientConnectResult connect_result, int error_number, TFGenericTCPSharedClient *shared_client) {
            if (connect_result != TFGenericTCPClientConnectResult::Connected) {
                char connect_error[256] = "";

                GenericTCPClientConnectorBase::format_connect_error(connect_result, error_number, host.c_str(), port, connect_error, sizeof(connect_error));
                report_errorf(cookie, "%s", connect_error);

                delete execute_table;
                execute_table = nullptr;

                return;
            }

            execute_client = shared_client;
            current_execute_index = 0;

            write_next();
        },
        [this](TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPSharedClient *shared_client) {
            if (execute_client != shared_client) {
                return;
            }

            execute_client = nullptr;

            delete execute_table;
            execute_table = nullptr;
        });
    }, true);
}

[[gnu::const]] BatteryClassID BatteriesModbusTCP::get_class() const
{
    return BatteryClassID::ModbusTCP;
}

IBattery *BatteriesModbusTCP::new_battery(uint32_t slot, Config *state, Config *errors)
{
    return new BatteryModbusTCP(slot, state, errors, modbus_tcp_client.get_pool());
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

void BatteriesModbusTCP::write_next()
{
    if (execute_table == nullptr) {
        return;
    }

    if (execute_client == nullptr) {
        // FIXME: maybe retry if connection is lost in the middle of an action, instead of aborting
        return;
    }

    if (current_execute_index >= execute_table->values_length) {
        report_success(execute_cookie);
        release_client(); // execution is done
        return;
    }

    const ValueSpec *spec = &execute_table->values[current_execute_index];
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
        esp_system_abort("batteries_modbus_tcp: Unsupported register type to write.");
    }

    static_cast<TFModbusTCPSharedClient *>(execute_client)->transact(execute_table->device_address,
                                                                     function_code,
                                                                     spec->start_address,
                                                                     1,
                                                                     const_cast<uint16_t *>(&spec->value),
                                                                     2_s,
    [this, spec, function_code](TFModbusTCPClientTransactionResult result) {
        if (result != TFModbusTCPClientTransactionResult::Success) {
            report_errorf(execute_cookie, "Execution failed at value %zu of %zu: %s (%d)",
                          current_execute_index, execute_table->values_length,
                          get_tf_modbus_tcp_client_transaction_result_name(result),
                          static_cast<int>(result));

            // FIXME: maybe retry on error in the middle of an action, instead of aborting
            release_client();
            return;
        }

        ++current_execute_index;

        if (current_execute_index >= execute_table->values_length) {
            report_success(execute_cookie);
            release_client(); // execution is done
            return;
        }

        write_next(); // FIXME: maybe add a little delay between writes to avoid bursts?
    });
}

void BatteriesModbusTCP::release_client()
{
    task_scheduler.scheduleOnce([this]() {
        if (execute_client != nullptr) {
            modbus_tcp_client.get_pool()->release(execute_client);
        }
    });
}
