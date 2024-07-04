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

#include <stdint.h>
#include <stdlib.h>
#include <ModbusTCP.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    //#pragma GCC diagnostic ignored "-Weffc++"
#endif

[[gnu::const]] const char* get_modbus_result_code_name(Modbus::ResultCode event);

void modbus_bswap_registers(uint16_t *register_start, size_t register_count);

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif

struct ModbusDeserializer
{
    uint16_t *buf;
    size_t idx;

    uint16_t read_uint16();
    uint32_t read_uint32();
    float read_float32();
    void read_string(char *string, size_t length);
};
