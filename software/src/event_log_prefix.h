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

#include <stdlib.h>

#if __INCLUDE_LEVEL__ > 1
#error "Don't include event_log_prefix.h in headers, only in sources!"
#endif

const char *get_module_offset_and_length(const char *path, size_t *out_length);
size_t strlen_with_event_log_alignment(const char *c);

#ifdef EVENT_LOG_PREFIX
static const char *event_log_prefix = EVENT_LOG_PREFIX;
static size_t event_log_prefix_len = strlen_with_event_log_alignment(event_log_prefix);
#else
static size_t event_log_prefix_len = 0;
static const char *event_log_prefix = get_module_offset_and_length(__BASE_FILE__, &event_log_prefix_len);
#endif

#define printfln(...) printfln_prefixed(event_log_prefix, event_log_prefix_len, __VA_ARGS__)

#define printfln_plain(...) printfln_prefixed(nullptr, 0, __VA_ARGS__)

#define tracefln(...) tracefln_prefixed(event_log_prefix, event_log_prefix_len, __VA_ARGS__)

#define tracefln_plain(...) tracefln_prefixed(nullptr, 0, __VA_ARGS__)

extern size_t event_log_alignment;
