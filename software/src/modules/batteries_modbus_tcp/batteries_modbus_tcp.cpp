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
#include "battery_modbus_tcp_specs.h"
#include "modules/modbus_tcp_client/modbus_register_address_mode.enum.h"
#include "tools.h"

#include "gcc_warnings.h"

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

void BatteriesModbusTCP::pre_setup()
{
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
        {"register_address_mode", Config::Enum(ModbusRegisterAddressMode::Address)},
        {"permit_grid_charge", Config::Object({
            {"repeat_interval", Config::Uint16(60)},
            {"register_blocks", Config::Array({},
                &table_custom_register_block_prototype,
                0,
                OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS(),
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"revoke_grid_charge_override", Config::Object({
            {"repeat_interval", Config::Uint16(60)},
            {"register_blocks", Config::Array({},
                &table_custom_register_block_prototype,
                0,
                OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS(),
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"forbid_discharge", Config::Object({
            {"repeat_interval", Config::Uint16(60)},
            {"register_blocks", Config::Array({},
                &table_custom_register_block_prototype,
                0,
                OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS(),
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"revoke_discharge_override", Config::Object({
            {"repeat_interval", Config::Uint16(60)},
            {"register_blocks", Config::Array({},
                &table_custom_register_block_prototype,
                0,
                OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS(),
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"forbid_charge", Config::Object({
            {"repeat_interval", Config::Uint16(60)},
            {"register_blocks", Config::Array({},
                &table_custom_register_block_prototype,
                0,
                OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS(),
                Config::type_id<Config::ConfObject>()
            )},
        })},
        {"revoke_charge_override", Config::Object({
            {"repeat_interval", Config::Uint16(60)},
            {"register_blocks", Config::Array({},
                &table_custom_register_block_prototype,
                0,
                OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS(),
                Config::type_id<Config::ConfObject>()
            )},
        })},
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

    execute_table_prototypes.push_back({BatteryModbusTCPTableID::Custom, Config::Object({
        {"device_address", Config::Uint8(1)},
        {"register_blocks", Config::Array({},
            &table_custom_register_block_prototype,
            0,
            OPTIONS_BATTERIES_MODBUS_TCP_MAX_CUSTOM_REGISTER_BLOCKS(),
            Config::type_id<Config::ConfObject>()
        )},
    })});

    execute_table_prototypes.push_back({BatteryModbusTCPTableID::VictronEnergyGX, Config::Object({
        {"device_address", Config::Uint8(DefaultDeviceAddress::VictronEnergyGX)},
        {"action", Config::Enum(BatteryAction::PermitGridCharge)},
    })});

    execute_table_prototypes.push_back({BatteryModbusTCPTableID::DeyeHybridInverter, Config::Object({
        {"device_address", Config::Uint8(DefaultDeviceAddress::DeyeHybridInverter)},
        {"action", Config::Enum(BatteryAction::PermitGridCharge)},
    })});

    execute_table_prototypes.push_back({BatteryModbusTCPTableID::AlphaESSHybridInverter, Config::Object({
        {"device_address", Config::Uint8(DefaultDeviceAddress::AlphaESSHybridInverter)},
        {"action", Config::Enum(BatteryAction::PermitGridCharge)},
    })});

    execute_table_prototypes.push_back({BatteryModbusTCPTableID::HaileiHybridInverter, Config::Object({
        {"device_address", Config::Uint8(DefaultDeviceAddress::HaileiHybridInverter)},
        {"action", Config::Enum(BatteryAction::PermitGridCharge)},
    })});

    execute_table_prototypes.push_back({BatteryModbusTCPTableID::SungrowHybridInverter, Config::Object({
        {"device_address", Config::Uint8(DefaultDeviceAddress::SungrowHybridInverter)},
        {"action", Config::Enum(BatteryAction::PermitGridCharge)},
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

void BatteriesModbusTCP::register_urls()
{
    api.addCommand("batteries_modbus_tcp/execute", &execute_config, {}, [this](String &/*errmsg*/) {
        uint32_t cookie = execute_config.get("cookie")->asUint();
        BatteryModbusTCPTableID table_id = execute_config.get("table")->getTag<BatteryModbusTCPTableID>();
        Config *table_config = static_cast<Config *>(execute_config.get("table")->get());
        uint8_t device_address;
        BatteryAction action;
        const BatteryModbusTCP::TableSpec *table;
        BatteryModbusTCP::TableSpec *table_to_free = nullptr;

        switch (table_id) {
        case BatteryModbusTCPTableID::None:
            report_errorf(cookie, "No table");
            return;

        case BatteryModbusTCPTableID::Custom:
            device_address = table_config->get("device_address")->asUint8();
            table_to_free = BatteryModbusTCP::load_table(table_config, false);
            table = table_to_free;
            break;

        case BatteryModbusTCPTableID::VictronEnergyGX:
            device_address = table_config->get("device_address")->asUint8();
            action = table_config->get("action")->asEnum<BatteryAction>();
            table = get_victron_energy_gx_table(action);

            if (table == nullptr) {
                report_errorf(cookie, "Unknown Victron Energy GX action: %u", static_cast<uint8_t>(action));
                return;
            }

            break;

        case BatteryModbusTCPTableID::DeyeHybridInverter:
            device_address = table_config->get("device_address")->asUint8();
            action = table_config->get("action")->asEnum<BatteryAction>();
            table = get_deye_hybrid_inverter_table(action);

            if (table == nullptr) {
                report_errorf(cookie, "Unknown Deye Hybrid Inverter action: %u", static_cast<uint8_t>(action));
                return;
            }

            break;

        case BatteryModbusTCPTableID::AlphaESSHybridInverter:
            device_address = table_config->get("device_address")->asUint8();
            action = table_config->get("action")->asEnum<BatteryAction>();
            table = get_alpha_ess_hybrid_inverter_table(action);

            if (table == nullptr) {
                report_errorf(cookie, "Unknown Alpha ESS Hybrid Inverter action: %u", static_cast<uint8_t>(action));
                return;
            }

            break;

        case BatteryModbusTCPTableID::HaileiHybridInverter:
            device_address = table_config->get("device_address")->asUint8();
            action = table_config->get("action")->asEnum<BatteryAction>();
            table = get_hailei_hybrid_inverter_table(action);

            if (table == nullptr) {
                report_errorf(cookie, "Unknown Hailei Hybrid Inverter action: %u", static_cast<uint8_t>(action));
                return;
            }

            break;

        case BatteryModbusTCPTableID::SungrowHybridInverter:
            device_address = table_config->get("device_address")->asUint8();
            action = table_config->get("action")->asEnum<BatteryAction>();
            table = get_sungrow_hybrid_inverter_table(action);

            if (table == nullptr) {
                report_errorf(cookie, "Unknown Sungrow Hybrid Inverter action: %u", static_cast<uint8_t>(action));
                return;
            }

            break;

        default:
            report_errorf(cookie, "Unknown table: %u", static_cast<uint8_t>(table_id));
            return;
        }

        const String &host = execute_config.get("host")->asString();
        uint16_t port = static_cast<uint16_t>(execute_config.get("port")->asUint());

        defer {
            // When done parsing the execute command, drop Strings and Array items from config
            // to free memory. This invalidates the "host" references above, which will be copied
            // by the execute call before being cleared.
            execute_config.get("host")->clearString();

            if (table_id == BatteryModbusTCPTableID::Custom) {
                table_config->get("register_blocks")->removeAll();
            }
        };

        modbus_tcp_client.get_pool()->acquire(host.c_str(), port,
        [cookie, host, port, device_address, table, table_to_free](TFGenericTCPClientConnectResult result, int error_number, TFGenericTCPSharedClient *shared_client, TFGenericTCPClientPoolShareLevel share_level) {
            if (result != TFGenericTCPClientConnectResult::Connected) {
                char error[256] = "";

                GenericTCPClientConnectorBase::format_connect_error(result, error_number, share_level, host.c_str(), port, error, sizeof(error));
                report_errorf(cookie, "%s", error);
                BatteryModbusTCP::free_table(table_to_free);
                return;
            }

            TFModbusTCPSharedClient *client = static_cast<TFModbusTCPSharedClient *>(shared_client);

            BatteryModbusTCP::execute(client, device_address, table, [cookie, client, table_to_free](const char *error) {
                if (error == nullptr) {
                    report_success(cookie);
                }
                else {
                    report_errorf(cookie, "%s", error);
                }

                task_scheduler.scheduleOnce([client]() {
                    modbus_tcp_client.get_pool()->release(client);
                });

                BatteryModbusTCP::free_table(table_to_free);
            });
        },
        [](TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPSharedClient *shared_client, TFGenericTCPClientPoolShareLevel share_level) {
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

String BatteriesModbusTCP::validate_config(Config &update, ConfigSource source)
{
    if (update.get("table")->getTag<BatteryModbusTCPTableID>() != BatteryModbusTCPTableID::Custom) {
        return String();
    }

    const char * const keys[] = {
        "permit_grid_charge",
        "revoke_grid_charge_override",
        "forbid_discharge",
        "revoke_discharge_override",
        "forbid_charge",
        "revoke_charge_override",
    };

    size_t total_values_count = 0;

    for (const char *key : keys) {
        auto register_blocks = update.get("table")->get()->get(key)->get("register_blocks");

        for (size_t i = 0; i < register_blocks->count(); ++i) {
            // FIXME: validate func is valid and vals length match func restrictions

            auto start_address = register_blocks->get(i)->get("addr")->asUint();
            auto values_count  = register_blocks->get(i)->get("vals")->count();

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
