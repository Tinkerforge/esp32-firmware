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

#include <stdint.h>

// full green
#define HUE_OK          120
// full yellow
#define HUE_WARNING      60
// full red
#define HUE_ERROR         0
// full yellow
#define HUE_IMPORT       60
// full blue
#define HUE_BALANCED    240
// full green
#define HUE_EXPORT      120
// full magenta
#define HUE_BADCONFIG   300
// full magenta
#define HUE_UNKNOWN     301

class EmRgbLed
{
public:
    enum class Status {
        OK = 0,
        Warning = 1,
        Error = 2,
        BadConfig = 3,
    };
    enum class GridBalance {
        Export = 0,
        Balanced = 1,
        Import = 2,
    };

    EmRgbLed() {}
    void set_status(Status status);
    void update_grid_balance(GridBalance balance);

private:
    void update_led();

    Status   status            = Status::OK;
    bool     have_grid_balance = false;
    uint32_t hue_balance       = 0;
};
