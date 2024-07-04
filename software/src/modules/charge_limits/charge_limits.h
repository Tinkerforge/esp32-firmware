/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

class ChargeLimits final : public IModule
#if MODULE_AUTOMATION_AVAILABLE()
                         , public IAutomationBackend
#endif
{
public:
    ChargeLimits(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

    ConfigRoot config;
    ConfigRoot config_in_use;
    ConfigRoot state;

    ConfigRoot override_duration;
    ConfigRoot override_energy;

    uint32_t duration_left;
    uint32_t energy_left;

private:
    bool was_triggered = false;
};
