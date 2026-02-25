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

#include "config.h"
#include "module.h"

class P14aEnwg final : public IModule
{
private:
    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot control;

    bool slot_enabled = false;

    bool eebus_active = false;
    uint32_t eebus_limit_w = 0;

    void update();

public:
    P14aEnwg() {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    inline bool is_enabled() { return config.get("enable")->asBool(); }

    void set_eebus_limit(bool active, uint32_t limit_w);
};
