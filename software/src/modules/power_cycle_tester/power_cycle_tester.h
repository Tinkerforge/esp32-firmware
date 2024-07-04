/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "module.h"

enum class POTI_POS {
    MINUS_150,
    MINUS_90,
    MINUS_45,
    ZERO,
    PLUS_45,
    PLUS_90,
    PLUS_150
};

class PowerCycleTester final : public IModule
{
public:
    PowerCycleTester(){}
    void setup() override;

    void led_green();
    void led_yellow();
    void led_red();

    void trigger_on_poti_pos(POTI_POS poti_pos, uint32_t delay);
};
