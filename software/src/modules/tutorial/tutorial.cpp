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

#include "tutorial.h"

#include <Arduino.h>

#include "api.h"
#include "tools.h"

extern API api;

Tutorial::Tutorial()
{

}

void Tutorial::setup()
{
    tutorial_state = Config::Object({
        {"color", Config::Str("#FF0000", 7, 7)}
    });

    initialized = true;
}

void Tutorial::register_urls()
{
    api.addState("tutorial/state", &tutorial_state, {}, 1000);
}

void Tutorial::loop()
{

}
