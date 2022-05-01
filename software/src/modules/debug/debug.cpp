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

#include "debug.h"

#include <Arduino.h>

#include "api.h"
#include "tools.h"
#include "task_scheduler.h"

extern TaskScheduler task_scheduler;
extern API api;

Debug::Debug()
{
}

void Debug::setup()
{
    debug_state = Config::Object({
        {"uptime", Config::Uint32(0)},
        {"free_heap", Config::Uint32(0)},
        {"largest_free_heap_block", Config::Uint32(0)},
        {"free_psram", Config::Uint32(0)},
        {"largest_free_psram_block", Config::Uint32(0)}
    });

    task_scheduler.scheduleWithFixedDelay([this](){
        debug_state.get("uptime")->updateUint(millis());
        debug_state.get("free_heap")->updateUint(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        debug_state.get("largest_free_heap_block")->updateUint(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        debug_state.get("free_psram")->updateUint(heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        debug_state.get("largest_free_psram_block")->updateUint(heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    }, 1000, 1000);

    initialized = true;
}

void Debug::register_urls()
{
    api.addState("debug/state", &debug_state, {}, 1000);
}

void Debug::loop()
{
}
