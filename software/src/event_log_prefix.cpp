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

#include "event_log_prefix.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

size_t event_log_alignment = 0;

#define ALIGNMENT_WARN_THRESHOLD 16

size_t strlen_with_event_log_alignment(const char *c)
{
    auto result = strlen(c);

    if (result > ALIGNMENT_WARN_THRESHOLD) {
        printf("(1) Event log prefix %.*s is longer than threshold (%u > %u)\n", result, c, result, ALIGNMENT_WARN_THRESHOLD);
    }

    event_log_alignment = std::max(event_log_alignment, result);

    return result;
}

const char *get_module_offset_and_length(const char *path, size_t *out_length)
{
    auto len = strlen(path);
    auto needle = "src/modules/";
    auto needle_len = strlen(needle);

    // Path is shorter or does not start with src/modules/
    if (len < needle_len || memcmp(path, needle, needle_len) != 0) {
        auto last_slash = strrchr(path, '/');

        if (last_slash == nullptr) {
            *out_length = 0;
            return nullptr;
        }

        auto last_dot = strrchr(last_slash, '.');
        if (last_dot == nullptr) {
            *out_length = 0;
            return nullptr;
        }

        *out_length = last_dot - last_slash - 1;

        if (*out_length > ALIGNMENT_WARN_THRESHOLD) {
            printf("(2) Event log prefix %.*s is longer than threshold (%u > %u) in %s\n", *out_length,  last_slash + 1, *out_length, ALIGNMENT_WARN_THRESHOLD, path);
        }

        event_log_alignment = std::max(event_log_alignment, *out_length);

        return last_slash + 1;
    }

    const auto *result = path + needle_len;

    auto *ptr = strchr(result, '/');
    if (ptr == nullptr) {
        *out_length = 0;
        return nullptr;
    }

    *out_length = (size_t)(ptr - result);

    if (*out_length > ALIGNMENT_WARN_THRESHOLD) {
        printf("(3) Event log prefix %.*s is longer than threshold (%u > %u) in %s\n", *out_length, result, *out_length, ALIGNMENT_WARN_THRESHOLD, path);
    }

    event_log_alignment = std::max(event_log_alignment, *out_length);

    return result;
}
