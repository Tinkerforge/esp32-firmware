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

#include <Arduino.h>

#include "modbus_tcp_tools.h"

void modbus_bswap_registers(uint16_t *register_start, size_t register_count)
{
    for (size_t i = 0; i < register_count; i++) {
        *register_start = __bswap16(*register_start);
        register_start++;
    }
}

uint16_t ModbusDeserializer::read_uint16()
{
    uint16_t result = buf[idx];

    idx += 1;

    return result;
}

uint32_t ModbusDeserializer::read_uint32()
{
    uint32_t result = (static_cast<uint32_t>(buf[idx]) << 16) | buf[idx + 1];

    idx += 2;

    return result;
}

float ModbusDeserializer::read_float32()
{
    union {
        float result;
        uint32_t u32;
    } uni;

    uni.u32 = read_uint32();

    // idx advanced in read_uint32()

    return uni.result;
}

// length must be one longer than the expected string length for NUL termination
void ModbusDeserializer::read_string(char *string, size_t length)
{
    for (size_t i = 0; i < length - 1; i += 2, ++idx) {
        uint16_t reg = buf[idx];

        string[i] = static_cast<char>((reg >> 8) & 0xFF);

        if (i + 1 < length) {
            string[i + 1] = static_cast<char>(reg & 0xFF);
        }
    }

    string[length - 1] = '\0';
}
