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
#include "lwip/ip_addr.h"

// Connect attempts are blocking. Use a low timeout that should usually work and just try again if it doesn't.
#define MODBUSIP_CONNECT_TIMEOUT 500

#include "ModbusTCP.h"

#include "tools.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

#if MODBUSIP_MAXFRAME < MODBUS_MAX_WORDS * 2 + 2
//#warning MODBUSIP_MAXFRAME should be increased to MODBUS_MAX_WORDS * 2 + 2
#define METERS_MODBUSTCP_MAX_HREG_WORDS ((MODBUSIP_MAXFRAME - 2) / 2)
#else
#define METERS_MODBUSTCP_MAX_HREG_WORDS (MODBUS_MAX_WORDS)
#endif

class GenericModbusTCPClient
{
protected:
    GenericModbusTCPClient(ModbusTCP *mb_) : mb(mb_) {}
    virtual ~GenericModbusTCPClient() = default;

    void start_connection();

    ModbusTCP * const mb;

    String host_name;
    IPAddress host_ip;
    uint16_t port = 0;
    uint8_t modbus_address = 0;

private:
    void check_ip(const ip_addr_t *ip, int err);
    virtual void connect_callback() = 0;

    dns_gethostbyname_addrtype_lwip_ctx_async_data host_data;
    uint32_t connect_backoff_ms = 1000;
    bool resolve_error_printed = false;
    bool connect_error_printed = false;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
