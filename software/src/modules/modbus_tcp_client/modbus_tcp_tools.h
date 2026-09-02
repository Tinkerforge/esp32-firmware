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

#include "generated/modbus_function_code.enum.h"

enum class ModbusDataType
{
    Coil,
    Register
};

void modbus_bswap_registers(uint16_t *register_start, size_t register_count);

ModbusDataType modbus_get_data_type(ModbusFunctionCode function_code);

size_t modbus_get_buffer_length(ModbusFunctionCode function_code, size_t data_count);

struct ModbusDeserializer
{
    uint16_t *buf;
    size_t idx;

    uint16_t read_uint16();
    uint32_t read_uint32();
    float read_float32();
    void read_string(char *string, size_t length);
};
