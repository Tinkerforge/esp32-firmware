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

#include "imodel_parser.h"
#include "dc_port_type.enum.h"

class MetersSunSpecParser714 : public IMetersSunSpecParser
{
public:
    MetersSunSpecParser714(uint32_t slot_, DCPortType port_type_) : slot(slot_), port_type(port_type_) {}

    bool detect_values(const uint16_t *const register_data[2], uint32_t quirks, size_t *registers_to_read) override;
    bool parse_values(const uint16_t *const register_data[2], uint32_t quirks) override;

    bool must_read_twice() override;
    bool is_model_length_supported(uint32_t model_length) override;
    uint32_t get_interesting_registers_count() override;

private:
    bool is_valid(const uint16_t *const register_data[2]);

    const uint32_t slot;
    const DCPortType port_type;
    size_t cached_port_count = 0;
};
