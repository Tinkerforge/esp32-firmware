/* esp32-firmware
 * Copyright (C) 2026 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <string.h>

#include "./generated/module_dependencies.h"
#include "./esp32_tools.h"

#include "event_log_prefix.h"
#include "tools.h"
#include "tools/hexdump.h"

#include "gcc_warnings.h"

bool esp32_poke_main_thread()
{
    if (running_in_main_task()) {
        // Main task can't poke itself.
        return true;
    }

    const bool success = task_scheduler.await([]() {
        // Do nothing, just poke main thread.
    });

    if (!success) {
        logger.printfln("Poking main thread failed");
        return false;
    }

    return true;
}

void esp32_common_dump_mem(const void *data, size_t len)
{
    uint8_t copy[32];

    const size_t bytecount = std::min(len, sizeof(copy));
    memcpy(copy, data, bytecount);

    char buf[68];
    hexdump(copy, bytecount, buf, sizeof(buf), HexdumpCase::Lower);
    logger.printfln("%p %s", data, buf);
}

void esp32_common_dump_mem(volatile uint32_t *first_register, size_t regcount)
{
    const void *data = static_cast<const void *>(const_cast<const uint32_t *>(first_register));
    uint8_t copy[32];

    const size_t bytecount = std::min(regcount * sizeof(uint32_t), sizeof(copy));
    memcpy(copy, data, bytecount);

    char buf[68];
    hexdump(copy, bytecount, buf, sizeof(buf), HexdumpCase::Lower);
    logger.printfln("%p %s", data, buf);
}
