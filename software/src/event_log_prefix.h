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

extern size_t event_log_alignment;

const char *get_module_offset_and_length(const char *path, size_t *out_length, bool check_len);
size_t strlen_with_event_log_alignment(const char *c, bool check_len);

#ifdef EVENT_LOG_PREFIX
static const char *event_log_prefix = EVENT_LOG_PREFIX;
static size_t event_log_prefix_len  = strlen_with_event_log_alignment(event_log_prefix, true);
#else
static size_t event_log_prefix_len  = 0;
static const char *event_log_prefix = get_module_offset_and_length(__BASE_FILE__, &event_log_prefix_len, true);
#endif

#ifdef TRACE_LOG_PREFIX
static const char *trace_log_prefix = TRACE_LOG_PREFIX;
static size_t trace_log_prefix_len  = strlen_with_event_log_alignment(trace_log_prefix, false);
#else
static size_t trace_log_prefix_len  = 0;
static const char *trace_log_prefix = get_module_offset_and_length(__BASE_FILE__, &trace_log_prefix_len, false);
#endif
