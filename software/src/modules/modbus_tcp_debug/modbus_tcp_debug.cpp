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

#include "event_log_prefix.h"
#include "module_dependencies.h"
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
        {"byte_order", Config::Uint8(0)},
    });
}

[[gnu::format(__printf__, 4, 5)]] static void report_errorf(IChunkedResponse *response, Ownership *ownership, uint32_t owner_id, const char *fmt, ...);
static void report_errorf(IChunkedResponse *response, Ownership *ownership, uint32_t owner_id, const char *fmt, ...)
{
    OwnershipGuard ownership_guard(ownership, owner_id);

    if (!ownership_guard.have_ownership()) {
        return;
    }

    va_list args;

    response->begin(true);
    va_start(args, fmt);
    response->vwritef(fmt, args);
    va_end(args);
    response->flush();
    response->end("");
}

void ModbusTCPDebug::register_urls()
{
    api.addResponse("modbus_tcp_debug/transact", &transact, {}, [this](IChunkedResponse *response, Ownership *ownership, uint32_t owner_id) {
        if (client != nullptr) {
            report_errorf(response, ownership, owner_id, "Another transaction is already in progress");
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
        TFModbusTCPByteOrder byte_order = transact.get("byte_order")->asEnum<TFModbusTCPByteOrder>();

        switch (function_code) {
        case TFModbusTCPFunctionCode::ReadCoils:
        case TFModbusTCPFunctionCode::ReadDiscreteInputs:
            report_errorf(response, ownership, owner_id, "Function code %u is not supported yet", static_cast<uint8_t>(function_code));
            return;

        case TFModbusTCPFunctionCode::ReadHoldingRegisters:
        case TFModbusTCPFunctionCode::ReadInputRegisters:
            break;

        case TFModbusTCPFunctionCode::WriteSingleCoil:
        case TFModbusTCPFunctionCode::WriteSingleRegister:
        case TFModbusTCPFunctionCode::WriteMultipleCoils:
        case TFModbusTCPFunctionCode::WriteMultipleRegisters:
            report_errorf(response, ownership, owner_id, "Function code %u is not supported yet", static_cast<uint8_t>(function_code));
            return;
        }

        client = new TFModbusTCPClient(byte_order);

        client->connect(host.c_str(), port,
        [this, response, ownership, owner_id, host, port, device_address, function_code, start_address, data_count, write_data, timeout](TFGenericTCPClientConnectResult connect_result, int error_number) {
            if (connect_result != TFGenericTCPClientConnectResult::Connected) {
                char connect_error[256] = "";

                GenericTCPClientConnectorBase::format_connect_error(connect_result, error_number, host.c_str(), port, connect_error, sizeof(connect_error));
                report_errorf(response, ownership, owner_id, "%s", connect_error);

                delete client;
                client = nullptr;
                client_disconnect = false;
                return;
            }

            uint16_t buffer[TF_MODBUS_TCP_MAX_READ_REGISTER_COUNT] = {0};

            // FIXME: write data into buffer for write function codes

            client->transact(device_address, function_code, start_address, data_count, buffer, timeout,
            [this, response, ownership, owner_id, data_count, &buffer](TFModbusTCPClientTransactionResult transact_result) {
                if (transact_result != TFModbusTCPClientTransactionResult::Success) {
                    report_errorf(response, ownership, owner_id,
                                  "Transaction failed: %s (%d)",
                                  get_tf_modbus_tcp_client_transaction_result_name(transact_result),
                                  static_cast<int>(transact_result));

                    client_disconnect = true;
                    return;
                }

                OwnershipGuard ownership_guard(ownership, owner_id);

                if (!ownership_guard.have_ownership()) {
                    client_disconnect = true;
                    return;
                }

                // FIXME: format coils for coil function codes

                static const char *lookup = "0123456789abcdef";
                char read_data[TF_MODBUS_TCP_MAX_READ_REGISTER_COUNT * 4 + 1];

                for (size_t i = 0; i < data_count; ++i) {
                    for (size_t n = 0; n < 4; ++n) {
                        read_data[4 * i + n] = lookup[(buffer[i] >> (12 - 4 * n)) & 0x0f];
                    }
                }

                read_data[data_count * 4] = '\0';

                response->begin(true);
                response->writef("READDATA:%s", read_data);
                response->flush();
                response->end("");

                client_disconnect = true;
            });
        },
        [this](TFGenericTCPClientDisconnectReason reason, int error_number) {
            delete client;
            client = nullptr;
            client_disconnect = false;
        });
    });
}

void ModbusTCPDebug::loop()
{
    if (client != nullptr) {
        client->tick();

        if (client_disconnect) {
            client->disconnect();
        }
    }
}
