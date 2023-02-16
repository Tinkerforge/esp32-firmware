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

class EmRgbLed
{
public:
    enum class Priority {
        Low = 0,
        Mid = 1,
        High = 2,
    };
    enum class Severity {
        OK = 0,
        Warning = 1,
        Error = 2,
    };
    enum class GridBalance {
        Export = 0,
        Balanced = 1,
        Import = 2,
    };

    EmRgbLed();
    void setup();
    void add_status(const String &id, Severity severity, Priority priority);
    void remove_status(const String &id);
    void update_grid_balance(GridBalance balance);

private:
    void update_led();

    bool have_grid_balance;
    bool show_grid_balance;
    float hue;
    float hue_balance;

    int16_t hue_ok;
    int16_t hue_warning;
    int16_t hue_error;
    int16_t hue_import;
    int16_t hue_balanced;
    int16_t hue_export;

    float value_breathing[100];
    int32_t breathing_pos;
    int32_t breathing_step;
};
