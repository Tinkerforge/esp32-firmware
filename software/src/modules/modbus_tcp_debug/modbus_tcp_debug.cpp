/* esp32-firmware
 * Copyright (C) 2025 Matthias Bolte <matthias@tinkerforge.com>
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

#include "modbus_tcp_debug.h"

#include <stdio.h>
#include <stdarg.h>
#include <TFJson.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools/hexdump.h"
#include "modules/modbus_tcp_client/generic_tcp_client_connector_base.h"

void ModbusTCPDebug::pre_setup()
{
    transact = Config::Object({
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"device_address", Config::Uint8(0)},
        {"function_code", Config::Uint8(0)},
        {"start_address", Config::Uint16(0)},
        {"data_count", Config::Uint16(0)},
        {"write_data", Config::Str("", 0, TF_MODBUS_TCP_MAX_WRITE_REGISTER_COUNT * 4)},
        {"timeout", Config::Uint(2000)},
        {"cookie", Config::Uint32(0)},
    });
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
    json.addMemberNull("read_data");
    va_end(args);
    json.endObject();
    json.end();

    ws.pushRawStateUpdate(buf, "modbus_tcp_debug/transact_result");
}

void ModbusTCPDebug::register_urls()
{
    api.addCommand("modbus_tcp_debug/transact", &transact, {}, [this](String &errmsg) {
        uint32_t cookie = transact.get("cookie")->asUint();

        if (connected_client != nullptr) {
            report_errorf(cookie, "Another transaction is already in progress");
            return;
        }

        String host = transact.get("host")->asString();
        uint16_t port = transact.get("port")->asUint();
        uint8_t device_address = transact.get("device_address")->asUint();
        TFModbusTCPFunctionCode function_code = transact.get("function_code")->asEnum<TFModbusTCPFunctionCode>();
        uint16_t start_address = transact.get("start_address")->asUint();
        uint16_t data_count = transact.get("data_count")->asUint();
        String write_data = transact.get("write_data")->asString();
        millis_t timeout = millis_t{transact.get("timeout")->asUint()};
        bool hexload_registers = false;
        bool hexdump_registers = false;

        switch (function_code) {
        case TFModbusTCPFunctionCode::ReadCoils:
        case TFModbusTCPFunctionCode::ReadDiscreteInputs:
            report_errorf(cookie, "Function code %u is not supported yet", static_cast<uint8_t>(function_code));
            return;

        case TFModbusTCPFunctionCode::ReadHoldingRegisters:
        case TFModbusTCPFunctionCode::ReadInputRegisters:
            hexdump_registers = true;
            break;

        case TFModbusTCPFunctionCode::WriteSingleCoil:
            report_errorf(cookie, "Function code %u is not supported yet", static_cast<uint8_t>(function_code));
            return;

        case TFModbusTCPFunctionCode::WriteSingleRegister:
            hexload_registers = true;
            break;

        case TFModbusTCPFunctionCode::WriteMultipleCoils:
            report_errorf(cookie, "Function code %u is not supported yet", static_cast<uint8_t>(function_code));
            return;

        case TFModbusTCPFunctionCode::WriteMultipleRegisters:
            hexload_registers = true;
            break;
        }

        modbus_tcp_client.get_pool()->acquire(host.c_str(), port,
        [this, cookie, host, port, device_address, function_code, start_address, data_count, write_data, timeout, hexload_registers, hexdump_registers](TFGenericTCPClientConnectResult connect_result, int error_number, TFGenericTCPSharedClient *shared_client) {
            if (connect_result != TFGenericTCPClientConnectResult::Connected) {
                char connect_error[256] = "";

                GenericTCPClientConnectorBase::format_connect_error(connect_result, error_number, host.c_str(), port, connect_error, sizeof(connect_error));
                report_errorf(cookie, "%s", connect_error);
                return;
            }

            connected_client = shared_client;
            transact_buffer = static_cast<uint16_t *>(malloc(sizeof(uint16_t) * TF_MODBUS_TCP_MAX_READ_REGISTER_COUNT));

            if (transact_buffer == nullptr) {
                report_errorf(cookie, "Cannot allocate transaction buffer");

                release_client = true;
                return;
            }

            // FIXME: hexload coils for coil function codes

            if (hexload_registers) {
                size_t nibble_count = write_data.length();

                if (nibble_count > TF_MODBUS_TCP_MAX_WRITE_REGISTER_COUNT * 4) {
                    report_errorf(cookie, "Write data is too long");

                    release_client = true;
                    return;
                }

                if ((nibble_count % 4) != 0) {
                    report_errorf(cookie, "Write data length must be multiple of 4");

                    release_client = true;
                    return;
                }

                if (nibble_count != data_count * 4) {
                    report_errorf(cookie, "Write data nibble count mismatch");

                    release_client = true;
                    return;
                }

                ssize_t data_hexload_len = hexload<uint16_t>(write_data.c_str(), nibble_count, transact_buffer, TF_MODBUS_TCP_MAX_WRITE_REGISTER_COUNT);

                if (data_hexload_len < 0) {
                    report_errorf(cookie, "Write data is malformed");

                    release_client = true;
                    return;
                }

                if (data_hexload_len != data_count) {
                    report_errorf(cookie, "Write data register count mismatch");

                    release_client = true;
                    return;
                }
            }

            static_cast<TFModbusTCPSharedClient *>(connected_client)->transact(device_address, function_code, start_address, data_count, transact_buffer, timeout,
            [this, cookie, data_count, hexdump_registers](TFModbusTCPClientTransactionResult transact_result) {
                if (transact_result != TFModbusTCPClientTransactionResult::Success) {
                    report_errorf(cookie, "Transaction failed: %s (%d)",
                                  get_tf_modbus_tcp_client_transaction_result_name(transact_result),
                                  static_cast<int>(transact_result));

                    release_client = true;
                    return;
                }

                char data_hexdump[TF_MODBUS_TCP_MAX_READ_REGISTER_COUNT * 4 + 1];
                char buf[64 + sizeof(data_hexdump)];
                TFJsonSerializer json{buf, sizeof(buf)};

                json.addObject();
                json.addMemberNumber("cookie", cookie);
                json.addMemberNull("error");

                // FIXME: hexdump coils for coil function codes

                if (hexdump_registers) {
                    hexdump<uint16_t>(transact_buffer, data_count, data_hexdump, ARRAY_SIZE(data_hexdump), HexdumpCase::Lower);
                    json.addMemberString("read_data", data_hexdump);
                }
                else {
                    json.addMemberNull("read_data");
                }

                json.endObject();
                json.end();

                ws.pushRawStateUpdate(buf, "modbus_tcp_debug/transact_result");

                release_client = true;
            });
        },
        [this](TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPSharedClient *shared_client) {
            if (connected_client != shared_client) {
                return;
            }

            connected_client = nullptr;
            release_client = false;

            free(transact_buffer);
            transact_buffer = nullptr;
        });
    }, true);
}

void ModbusTCPDebug::loop()
{
    if (release_client) {
        modbus_tcp_client.get_pool()->release(connected_client);
    }
}
