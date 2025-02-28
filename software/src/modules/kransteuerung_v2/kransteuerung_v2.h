/* warp-charger
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

#include "bindings/bricklet_dual_button_v2.h"
#include "bindings/bricklet_industrial_quad_relay_v2.h"

#include "config.h"

#include "module.h"

class KransteuerungV2 final : public IModule
{
public:
    KransteuerungV2(){}
    void setup() override;

    void button_pressed_handler(uint8_t button_l, uint8_t button_r, uint8_t led_l, uint8_t led_r);

private:
    TF_DualButtonV2 db;
    //uint8_t last_button_states[1] = {TF_DUAL_BUTTON_V2_BUTTON_STATE_RELEASED,TF_DUAL_BUTTON_V2_BUTTON_STATE_RELEASED};
    TF_IndustrialQuadRelayV2 relay;

    void set_state(int iec_state);
    int get_state();

    bool connected = false;
    bool car_full = false;
};
