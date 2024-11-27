/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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


class Eco final : public IModule
{
private:
    void update();

    ConfigRoot config;
    ConfigRoot charge_plan;
    ConfigRoot charge_plan_update;
    ConfigRoot state;
    ConfigRoot state_chargers_prototype;

    size_t trace_buffer_index;

public:
    enum class Decision : uint8_t {
        Normal = 0,
        Fast = 1,
        Block = 2
    };

    Eco(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    Decision get_decision();

    Decision current_decision = Decision::Normal;
};
