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

#include "config.h"

class InputPin {
public:
    InputPin(uint32_t num_name, uint32_t num_logic, const ConfigRoot &conf);

    void update(bool level);

private:
    uint8_t num_name;
    uint8_t num_logic;
    bool invert_pin;
    void (InputPin::*update_func)(bool level);
    int32_t limit;
    void *out_dst;

    void nop(bool level);
    void block_charging(bool level);
    void switch_excess_charging(bool level);
    void limit_max_current(bool level);
    void override_grid_draw(bool level);
};
