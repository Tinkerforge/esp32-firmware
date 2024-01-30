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

#include <stdint.h>
#include <vector>

#include "module.h"
#include "tools.h"

class Debug final : public IModule
{
public:
    enum TaskAvailability {
        ExpectPresent,
        Optional,
        ExpectMissing,
    };

    Debug(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;
    void loop() override;

    void register_task(const char *task_name, uint32_t stack_size, TaskAvailability availability = ExpectPresent);
    void register_task(TaskHandle_t handle, uint32_t stack_size);

private:
    ConfigRoot state_static;
    ConfigRoot state_fast;
    ConfigRoot state_slow;
    ConfigRoot state_hwm;

    std::vector<TaskHandle_t> task_handles;
    bool show_hwm_changes = false;

    micros_t last_state_update;
    uint32_t integrity_check_runs = 0;
    uint32_t integrity_check_runtime_sum = 0;
    uint32_t integrity_check_runtime_max = 0;
    bool     integrity_check_print_errors = true;
};
