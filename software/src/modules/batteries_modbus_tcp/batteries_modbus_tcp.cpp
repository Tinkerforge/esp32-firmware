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

BatteriesModbusTCP::TableSpec *BatteriesModbusTCP::init_table(const Config *config)
{
    const Config *registers_config = static_cast<const Config *>(config->get("registers"));
    size_t registers_count         = registers_config->count();

    if (registers_count == 0) {
        return nullptr;
    }

    TableSpec *table = static_cast<TableSpec *>(malloc(sizeof(TableSpec)));

    table->device_address  = static_cast<uint8_t>(config->get("device_address")->asUint());
    table->registers       = static_cast<RegisterSpec *>(malloc(sizeof(RegisterSpec) * registers_count));
    table->registers_count = registers_count;

    for (size_t i = 0; i < registers_count; ++i) {
        auto register_config = registers_config->get(i);

        table->registers[i].register_type = register_config->get("rtyp")->asEnum<ModbusRegisterType>();
        table->registers[i].start_address = static_cast<uint16_t>(register_config->get("addr")->asUint());

        auto values_config    = register_config->get("vals");
        uint16_t values_count = static_cast<uint16_t>(values_config->count());
        size_t values_byte_count;

        if (table->registers[i].register_type == ModbusRegisterType::Coil) {
            values_byte_count = (values_count + 7u) / 8u;
        }
        else {
            values_byte_count = values_count * 2u;
        }

        table->registers[i].values_buffer = malloc(values_byte_count);
        table->registers[i].values_count  = values_count;

        if (table->registers[i].register_type == ModbusRegisterType::Coil) {
            uint8_t *coils_buffer = reinterpret_cast<uint8_t *>(table->registers[i].values_buffer);

            coils_buffer[values_byte_count - 1] = 0;

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
            uint16_t *registers_buffer = reinterpret_cast<uint16_t *>(table->registers[i].values_buffer);

            for (uint16_t k = 0; k < values_count; ++k) {
                registers_buffer[k] = static_cast<uint16_t>(values_config->get(k)->asUint());
            }
        }
    }

    return table;
}

void BatteriesModbusTCP::free_table(BatteriesModbusTCP::TableSpec *table)
{
    if (table == nullptr) {
        return;
    }

    for (size_t i = 0; i < table->registers_count; ++i) {
        free(table->registers[i].values_buffer);
    }

    free(table->registers);
    free(table);
}

void BatteriesModbusTCP::pre_setup()
{
    table_prototypes.push_back({BatteryModbusTCPTableID::None, *Config::Null()});

    table_custom_registers_prototype = Config::Object({
        {"desc", Config::Str("", 0, 32)},
        {"rtyp", Config::Enum(ModbusRegisterType::HoldingRegister)}, // FIXME: replace with function code?
        {"addr", Config::Uint16(0)},
        {"vals", Config::Array({},
            Config::get_prototype_uint16_0(),
            1,
            BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_VALUES,
            Config::type_id<Config::ConfUint>()
        )},
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
        {"forbid_charge", Config::Object({
            {"device_address", Config::Uint8(1)},
            {"registers", Config::Array({},
                &table_custom_registers_prototype,
                0,
                BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTERS,
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"revoke_charge_override", Config::Object({
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
    api.addCommand("batteries_modbus_tcp/execute", &execute_config, {}, [this](String &/*errmsg*/) {
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

        const String &host = execute_config.get("host")->asString();
        uint16_t port = static_cast<uint16_t>(execute_config.get("port")->asUint());
        const Config *table = static_cast<const Config *>(execute_config.get("table")->get());

        defer {
            // When done parsing the execute command, drop Strings and Array items from config
            // to free memory. This invalidates the "host" references above, which will be copied
            // by the lambda before being cleared.
            execute_config.get("host")->clearString();
            execute_config.get("table")->get()->get("registers")->removeAll();
        };

        execute_cookie = cookie;
        execute_table = init_table(table);

        if (execute_table == nullptr) {
            report_errorf(cookie, "Table has no registers");
            return;
        }

        modbus_tcp_client.get_pool()->acquire(host.c_str(), port,
        [this, cookie, host, port](TFGenericTCPClientConnectResult connect_result, int error_number, TFGenericTCPSharedClient *shared_client) {
            if (connect_result != TFGenericTCPClientConnectResult::Connected) {
                char connect_error[256] = "";

                GenericTCPClientConnectorBase::format_connect_error(connect_result, error_number, host.c_str(), port, connect_error, sizeof(connect_error));
                report_errorf(cookie, "%s", connect_error);

                free_table(execute_table);
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

            free_table(execute_table);
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
        return;
    }

    if (current_execute_index >= execute_table->registers_count) {
        report_success(execute_cookie);
        release_client(); // execution is done
        return;
    }

    RegisterSpec *register_ = &execute_table->registers[current_execute_index];
    TFModbusTCPFunctionCode function_code;

    switch (register_->register_type) {
    case ModbusRegisterType::HoldingRegister:
        function_code = TFModbusTCPFunctionCode::WriteMultipleRegisters;
        execute_data_name = "register";
        break;

    case ModbusRegisterType::Coil:
        function_code = TFModbusTCPFunctionCode::WriteMultipleCoils;
        execute_data_name = "coil";
        break;

    case ModbusRegisterType::InputRegister:
    case ModbusRegisterType::DiscreteInput:
    default:
        report_errorf(execute_cookie, "Unsupported register type to write: %u", static_cast<uint8_t>(register_->register_type));
        release_client();
        return;
    }

    static_cast<TFModbusTCPSharedClient *>(execute_client)->transact(execute_table->device_address,
                                                                     function_code,
                                                                     register_->start_address,
                                                                     register_->values_count,
                                                                     register_->values_buffer,
                                                                     2_s,
    [this](TFModbusTCPClientTransactionResult result) {
        if (result != TFModbusTCPClientTransactionResult::Success) {
            report_errorf(execute_cookie, "Execution failed at %s %zu of %zu: %s (%d)",
                          execute_data_name, current_execute_index + 1, execute_table->registers_count,
                          get_tf_modbus_tcp_client_transaction_result_name(result),
                          static_cast<int>(result));

            release_client();
            return;
        }

        ++current_execute_index;

        if (current_execute_index >= execute_table->registers_count) {
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
