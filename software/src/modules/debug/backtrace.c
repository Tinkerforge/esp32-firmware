/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "backtrace.h"

#include <stdint.h>
#include <string.h>
#include "esp_debug_helpers.h"
#include "soc/soc_memory_types.h"
#include "freertos/projdefs.h"
#include "freertos/portable.h"
#include "freertos/xtensa_context.h"

size_t strn_backtrace(char *strn, size_t remaining, size_t skip_frames)
{
    size_t total_written = 0;

    esp_backtrace_frame_t frame = {0};
    esp_backtrace_get_start(&(frame.pc), &(frame.sp), &(frame.next_pc));

    bool corrupt = !esp_stack_ptr_is_sane(frame.sp) || !esp_ptr_executable((void *)esp_cpu_process_stack_pc(frame.pc));

    if (remaining >= 23) {
        for (;;) {
            if (skip_frames > 0) {
                skip_frames--;
            } else {
                size_t written = snprintf(strn + total_written, remaining, "0x%08x:0x%08x ", esp_cpu_process_stack_pc(frame.pc), frame.sp);
                if (written <= 0) {
                    return total_written;
                }
                total_written += written;
                remaining -= written;
            }

            if (frame.next_pc == 0 || corrupt || remaining < 23) {
                break;
            }

            if (!esp_backtrace_get_next_frame(&frame)) {
                corrupt = true;
            }
        }
    }

    const char *fin;
    if (corrupt) {
        fin = "|<-CRPT";
    } else if (frame.next_pc != 0) {
        fin = "|<-CONT";
    } else {
        fin = NULL;
    }

    if (fin) {
        size_t required = strlen(fin) + 1;
        if (remaining < required) {
            size_t missing = required - remaining;
            if (total_written >= missing) {
                total_written -= missing;
                remaining = required;
            } else {
                remaining += total_written;
                total_written = 0;
            }
        }
        size_t written = snprintf(strn + total_written, remaining, fin);
        if (written > 0) {
            total_written += written;
        }
    }

    return total_written;
}
