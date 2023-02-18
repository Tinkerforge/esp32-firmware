/* en+ hardware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C)      2021 Birger Schmidt <bs-warp@netgaroo.com>
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

#define GREEN_LED 25
#define RED_LED 33
#define BUTTON 32
#define LED_OFF HIGH
#define LED_ON LOW
#define BUTTON_NOT_PRESSED HIGH
#define BUTTON_PRESSED LOW

class AC011KHardware {
public:
    AC011KHardware();
    void pre_setup();
    void persist_config();
    void setup();
    void register_urls();
    void loop();

    ConfigRoot ac011k_hardware;
    ConfigRoot meter;
    ConfigRoot config;

    bool initialized = false;
private:

};
