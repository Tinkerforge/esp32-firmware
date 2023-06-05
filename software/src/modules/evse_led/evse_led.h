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

#pragma once

#include "config.h"

#include "module.h"
#include "tools.h"

class EvseLed final : public IModule
{
public:
    EvseLed(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    enum Blink {
        None = -1,
        Off = 0,
        /* 1 - 254 = PWM dimmed */
        On = 255,
        Ack = 1001,
        Nack = 1002,
        Nag = 1003,
        ErrorStart = 2001,
        ErrorEnd = 2010,
    };

    bool set_module(Blink state, uint16_t duration_ms);
    bool set_api(Blink state, uint16_t duration_ms);

    ConfigRoot config, config_in_use;
    ConfigRoot led;

private:
    bool accepts_new_state(Blink state);
    bool set(Blink state, uint16_t duration_ms, bool via_api);

    Blink current_state = Blink::None;
    micros_t current_duration_end_us = 0_usec;
    bool current_state_via_api = false;
};
