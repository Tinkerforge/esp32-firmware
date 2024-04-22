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

#if __INCLUDE_LEVEL__ > 1
#error "Don't include event_log.h in headers, only in sources!"
#endif

#include <stdarg.h>
#include <mutex>

#include <Arduino.h>

#include "ringbuffer.h"
#include "malloc_tools.h"

// Length of a timestamp with two spaces at the end. For example "2022-02-11 12:34:56,789"
// Also change in frontend when changing here!
#define TIMESTAMP_LEN 24

const char *get_module_offset_and_length(const char *path, size_t *out_length);
size_t strlen_with_log_alignment(const char *c);

#ifdef EVENT_LOG_PREFIX
static const char *logger_prefix = EVENT_LOG_PREFIX;
static size_t logger_prefix_len = strlen_with_log_alignment(logger_prefix);
#else
static size_t logger_prefix_len = 0;
static const char *logger_prefix = get_module_offset_and_length(__BASE_FILE__, &logger_prefix_len);
#endif

#define printfln(...) printfln_prefixed(logger_prefix, logger_prefix_len, __VA_ARGS__)

#define printfln_plain(...) printfln_prefixed(nullptr, 0, __VA_ARGS__)

class EventLog
{
public:
    std::mutex event_buf_mutex;
    TF_Ringbuffer<char,
                  10000,
                  uint32_t,
#if defined(BOARD_HAS_PSRAM)
                  malloc_psram,
#else
                  malloc_32bit_addressed,
#endif
                  heap_caps_free> event_buf;

    void pre_init();
    void pre_setup();
    void post_setup();

    void write(const char *buf, size_t len);

    int printfln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, va_list args);
    [[gnu::format(__printf__, 4, 5)]] int printfln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, ...);

    #define tf_dbg(fmt, ...) printfln("[%s:%d] " fmt, __FILE__, __LINE__ __VA_OPT__(,) __VA_ARGS__)

    void drop(size_t count);

    void register_urls();

    void get_timestamp(char buf[TIMESTAMP_LEN + 1]);

    bool sending_response = false;
};

// Make global variable available everywhere because it is not declared in modules.h.
// Definition is in event_log.cpp.
extern EventLog logger;

// To capture ESP-IDF log messages, use
// esp_log_set_vprintf(tf_event_log_vprintfln);
// If this is in a c (not cpp) file, also add
// extern int tf_event_log_vprintfln(const char *fmt, va_list args);
// instead of including event_log.h (a C++ header)
extern "C" int tf_event_log_vprintfln(const char *fmt, va_list args);

// To directly print from external C code, add
// extern int tf_event_log_printfln(const char *fmt, ...);
extern "C" int tf_event_log_printfln(const char *fmt, ...);
