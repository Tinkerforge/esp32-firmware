/* esp32-lib
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

#include <stdarg.h>
#include <mutex>

#include <Arduino.h>

#include "ringbuffer.h"
#include "malloc_tools.h"

#include "bindings/macros.h"

class EventLog {
public:
    std::mutex event_buf_mutex;
    TF_Ringbuffer<char, 10000, uint32_t, malloc_32bit_addressed, heap_caps_free> event_buf;

    void write(const char *buf, size_t len);

    void printfln(const char *fmt, ...) __attribute__((__format__(__printf__, 2, 3)));

    void drop(size_t count);

    void register_urls();

    bool sending_response = false;
};
