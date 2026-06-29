/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

class OveR37 final : public IModule
{
public:
    OveR37(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    void update_state_from_all_data(uint8_t ove_r37_state, uint8_t trip_reason, uint8_t flags);

private:
    bool update_config_from_bricklet();

    ConfigRoot config;
    ConfigRoot config_update;
    ConfigRoot state;

    bool config_read = false;
};
