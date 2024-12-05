/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#define EVENT_LOG_PREFIX "gen_mbtcp_client"

#include "generic_modbus_tcp_client.h"

#include <errno.h>
#include <string.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modbus_tcp_tools.h"

#include "gcc_warnings.h"

void GenericModbusTCPClient::connect_callback()
{
    last_successful_read = now_us();

    last_read_result = TFModbusTCPClientTransactionResult::Success;
    last_read_result_burst_length = 0;
}

void GenericModbusTCPClient::start_generic_read()
{
    if (connected_client == nullptr) {
        return;
    }

    if (deadline_elapsed(last_successful_read + successful_read_timeout)) {
        logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                 "Last successful read occurred too long ago, reconnecting to %s:%u", host_name.c_str(), port);
        force_reconnect();
        return;
    }

    read_buffer_num = 0;
    registers_done_count = 0;

    if (generic_read_request.register_count == 0) {
        generic_read_request.result = TFModbusTCPClientTransactionResult::InvalidArgument;
        generic_read_request.done_callback();
        return;
    }

    size_t read_blocks = (generic_read_request.register_count + TF_MODBUS_TCP_MAX_READ_REGISTER_COUNT - 1) / TF_MODBUS_TCP_MAX_READ_REGISTER_COUNT;
    read_block_size = static_cast<uint16_t>((generic_read_request.register_count + read_blocks - 1) / read_blocks);

    read_next();
}

void GenericModbusTCPClient::read_next()
{
    if (connected_client == nullptr) {
        esp_system_abort("generic_modbus_tcp_client: Not connected while trying to read");
    }

    uint16_t *target_buffer = generic_read_request.data[read_buffer_num] + registers_done_count;
    uint16_t read_start_address = static_cast<uint16_t>(generic_read_request.start_address + registers_done_count);
    uint16_t registers_remaining = static_cast<uint16_t>(generic_read_request.register_count - registers_done_count);
    uint16_t read_count = registers_remaining < read_block_size ? registers_remaining : read_block_size;
    TFModbusTCPDataType data_type;

    switch (generic_read_request.register_type) {
    case ModbusRegisterType::HoldingRegister: data_type = TFModbusTCPDataType::HoldingRegister; break;
    case ModbusRegisterType::InputRegister:   data_type = TFModbusTCPDataType::InputRegister;   break;
    default:
        esp_system_abort("generic_modbus_tcp_client: Unsupported register type to read.");
    }

    static_cast<TFModbusTCPSharedClient *>(connected_client)->read(data_type, device_address, read_start_address, read_count, target_buffer, 2_s,
    [this, data_type, read_start_address, read_count](TFModbusTCPClientTransactionResult result) {
        if (last_read_result == result) {
            ++last_read_result_burst_length;
        }
        else {
            last_read_result = result;
            last_read_result_burst_length = 1;
        }

        if (result != TFModbusTCPClientTransactionResult::Success) {
            if (result != TFModbusTCPClientTransactionResult::Timeout || (last_read_result_burst_length % 10) == 0) {
                logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                         "Modbus read error (host='%s' port=%u dtype=%d devaddr=%u regaddr=%u regcnt=%u burstlen=%zu): %s (%d)",
                                         host_name.c_str(),
                                         port,
                                         static_cast<int>(data_type),
                                         device_address,
                                         read_start_address,
                                         read_count,
                                         last_read_result_burst_length,
                                         get_tf_modbus_tcp_client_transaction_result_name(result),
                                         static_cast<int>(result));
            }

            generic_read_request.result = result;
            generic_read_request.done_callback();
            return;
        }

        registers_done_count = static_cast<uint16_t>(registers_done_count + read_block_size);

        if (registers_done_count >= generic_read_request.register_count) {
            // buffer done
            if (generic_read_request.read_twice && read_buffer_num == 0) {
                // Two reads requested and first read is done. -> Next buffer.
                read_buffer_num = 1;
                registers_done_count = 0;
            } else {
                // Only one read requested or second buffer done. -> All done.
                last_successful_read = now_us();
                generic_read_request.result = TFModbusTCPClientTransactionResult::Success;
                generic_read_request.done_callback();
                return;
            }
        }

        read_next();
    });

    // Read successfully dispatched, will continue in callback.
}
