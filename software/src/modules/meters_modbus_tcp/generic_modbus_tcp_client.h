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

#include <IPAddress.h>
#include <stdint.h>
#include <lwip/ip_addr.h>

// Connect attempts are blocking. Use a low timeout that should usually work and just try again if it doesn't.
#define MODBUSIP_CONNECT_TIMEOUT 500
#include <ModbusTCP.h>

#include "tools.h"
#include "modbus_register_type.enum.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

#if MODBUSIP_MAXFRAME < MODBUS_MAX_WORDS * 2 + 2
//#warning MODBUSIP_MAXFRAME should be increased to MODBUS_MAX_WORDS * 2 + 2
#define METERS_MODBUS_TCP_MAX_HREG_WORDS ((MODBUSIP_MAXFRAME - 2) / 2)
#else
#define METERS_MODBUS_TCP_MAX_HREG_WORDS (MODBUS_MAX_WORDS)
#endif

class GenericModbusTCPClient
{
protected:
    struct read_request {
        ModbusRegisterType register_type;
        size_t start_address;
        size_t register_count;
        uint16_t *data[2] = { nullptr, nullptr };
        bool read_twice;
        Modbus::ResultCode result_code;
        std::function<void(void)> done_callback;
    };

    GenericModbusTCPClient(ModbusTCP *modbus_) : modbus(modbus_) {}
    virtual ~GenericModbusTCPClient() = default;

    void start_connection();
    void stop_connection();
    void start_generic_read();

    ModbusTCP *const modbus;

    String host_name;
    IPAddress host_ip = IPAddress(0u);
    uint16_t port = 0;
    uint8_t device_address = 0;

    read_request generic_read_request;

private:
    void check_ip(const ip_addr_t *ip, int err);
    virtual void connect_callback() = 0;
    virtual void disconnect_callback() = 0;
    void read_next();

    dns_gethostbyname_addrtype_lwip_ctx_async_data host_data;
    micros_t last_successful_read = 0_us;
    micros_t successful_read_timeout = 1_m;
    uint32_t connect_backoff_ms = 1000;
    int last_connect_errno = 0;
    bool resolve_error_printed = false;

    uint8_t read_buffer_num;
    uint16_t read_block_size;
    uint16_t registers_done_count;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
