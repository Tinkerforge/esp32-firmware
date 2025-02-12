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

#pragma once

#include <stddef.h>
#include <stdint.h>

class IMetersSunSpecParser
{
public:
    virtual ~IMetersSunSpecParser() = default;

    virtual bool detect_values(const uint16_t *const register_data[2], size_t register_count, uint32_t quirks, size_t *registers_to_read) = 0;
    virtual bool parse_values(const uint16_t *const register_data[2], size_t register_count, uint32_t quirks) = 0;

    virtual bool must_read_twice() = 0;
    virtual bool is_model_length_supported(uint32_t model_length) = 0;
    virtual uint32_t get_interesting_registers_count() = 0;
};
