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

#include "config.h"

#include "module.h"
#include "tools.h"

class Debug final : public IModule
{
public:
    Debug(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;

private:
    ConfigRoot state_fast;
    ConfigRoot state_slow;

    micros_t last_state_update;
    uint32_t integrity_check_runs = 0;
    uint32_t integrity_check_runtime_sum = 0;
    uint32_t integrity_check_runtime_max = 0;
    bool     integrity_check_print_errors = true;
};
