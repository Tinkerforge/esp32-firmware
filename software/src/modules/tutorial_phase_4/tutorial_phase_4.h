/* esp32-firmware
 * Copyright (C) 2022 Matthias Bolte <matthias@tinkerforge.com>
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
#include "bindings/bricklet_rgb_led_button.h"

class TutorialPhase4 final : public IModule
{
public:
    TutorialPhase4(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;
    void set_bricklet_color(String color);

    // ConfigRoot object to represent the color to be send to the frontend module
    ConfigRoot config;

    // Extra ConfigRoot object to represent color updates received from the frontend module
    ConfigRoot config_update;

    // RGB LED Button Bricklet object to represent a connected Bricklet
    TF_RGBLEDButton rgb_led_button;
};
