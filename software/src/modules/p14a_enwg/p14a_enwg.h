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
#include "p14a_enwg_source.enum.h"

class P14aEnwg final : public IModule
{
private:
    ConfUnionPrototype<P14aEnwgSource> source_prototypes[3];

    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot control;

    bool eebus_active = false;
    uint32_t eebus_limit_w = 0;

    uint64_t input_check_task_id = 0;
    bool last_input_value = false;
    uint32_t last_phases = 0;
    uint16_t last_current_mA = 32000;

    void update();
    void check_inputs();
    void start_input_check();
    void stop_input_check();
    void check_evse_shutdown_input();

public:
    P14aEnwg() {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    inline bool is_enabled() { return config.get("enable")->asBool(); }
    bool is_heating_active();

    // Returns the limit for managed chargers in W, or 0 if not active.
    uint32_t get_managed_chargers_limit();

    void set_eebus_limit(bool active, uint32_t limit_w);
};
