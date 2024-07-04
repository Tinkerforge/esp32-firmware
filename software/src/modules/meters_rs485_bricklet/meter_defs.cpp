/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

#include "meter_defs.h"

void convert_to_float(const uint16_t *regs, float *target, const uint16_t *indices, size_t count)
{
    union {
        float f;
        uint16_t regs[2];
    } value;

    for (size_t i = 0; i < count; ++i) {
        size_t reg = indices[i] - 1; // -1: convert from register to address
        value.regs[1] = regs[reg + 0];
        value.regs[0] = regs[reg + 1];
        target[i] = value.f;
    }
}
