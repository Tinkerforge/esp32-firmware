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

#include "module.h"
#include "config.h"
#include "bindings/bricklet_co2_v2.h"
#include "bindings/bricklet_lcd_128x64.h"
#include "bindings/bricklet_rgb_led_button.h"
#include "bindings/bricklet_rgb_led_v2.h"

class Co2Ampel final : public IModule
{
public:
    Co2Ampel(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void set_color(uint32_t c);

    ConfigRoot state;
    ConfigRoot config;

private:
    TF_CO2V2 co2;
    TF_LCD128x64 lcd;
    TF_RGBLEDButton rlb;
    TF_RGBLEDV2 rgb;
};
