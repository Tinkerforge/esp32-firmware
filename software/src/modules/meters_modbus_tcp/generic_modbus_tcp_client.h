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

#pragma once

#include <TFModbusTCPClient.h>
#include <TFModbusTCPClientPool.h>

#include "modules/modbus_tcp_client/generic_tcp_client_pool_connector.h"
#include "modbus_register_type.enum.h"
#include "tools.h"

class GenericModbusTCPClient : protected GenericTCPClientPoolConnector
{
protected:
    struct ReadRequest {
        ModbusRegisterType register_type;
        size_t start_address;
        size_t register_count;
        uint16_t *data[2] = { nullptr, nullptr };
        bool read_twice;
        TFModbusTCPClientTransactionResult result;
        std::function<void(void)> done_callback;
    };

    GenericModbusTCPClient(const char *event_log_prefix_override_, TFModbusTCPClientPool *pool) :
        GenericTCPClientPoolConnector(event_log_prefix_override_, pool) {}

    void connect_callback() override;
    void start_generic_read();

    uint8_t device_address = 0;
    ReadRequest generic_read_request;

private:
    void read_next();

    micros_t last_successful_read = 0_us;
    micros_t successful_read_timeout = 1_m;

    uint8_t read_buffer_num;
    uint16_t read_block_size;
    uint16_t registers_done_count;

    TFModbusTCPClientTransactionResult last_read_result = TFModbusTCPClientTransactionResult::Success;
    size_t last_read_result_burst_length = 0;
};
