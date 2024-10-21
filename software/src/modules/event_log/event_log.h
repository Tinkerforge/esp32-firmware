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

#include <stdarg.h>
#include <mutex>
#include <Arduino.h>

#include "module.h"
#include "config.h"
#include "ringbuffer.h"
#include "malloc_tools.h"

// Length of an ISO 8601 timestamp. For example "2022-02-11 12:34:56,789"
// Also change in frontend when changing here!
#define EVENT_LOG_TIMESTAMP_LENGTH 23

class EventLog final : public IModule
{
public:
    EventLog() {}

    void pre_init() override;
    void pre_setup() override;
    void register_urls() override;

    void post_setup();

    void format_timestamp(char buf[EVENT_LOG_TIMESTAMP_LENGTH + 1 /* \0 */]);
    size_t vsnprintf_prefixed(char *buf, size_t buf_len, const char *prefix, size_t prefix_len, const char *fmt, va_list args);

    void print_drop(size_t count);
    void print_timestamp();

    size_t println_plain(const char *buf, size_t len);
    size_t vprintfln_plain(const char *fmt, va_list args);
    [[gnu::format(__printf__, 2, 3)]] size_t printfln_plain(const char *fmt, ...);

    size_t vprintfln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, va_list args);
    [[gnu::format(__printf__, 4, 5)]] size_t printfln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, ...);

    void trace_drop(size_t count);
    void trace_timestamp();

    size_t traceln_plain(const char *buf, size_t len);
    size_t vtracefln_plain(const char *fmt, va_list args);
    [[gnu::format(__printf__, 2, 3)]] size_t tracefln_plain(const char *fmt, ...);

    size_t vtracefln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, va_list args);
    [[gnu::format(__printf__, 4, 5)]] size_t tracefln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, ...);

private:
    std::mutex event_buf_mutex;
    TF_PackedRingbuffer<char,
                        10000,
                        uint32_t,
#if defined(BOARD_HAS_PSRAM)
                        malloc_psram,
#else
                        malloc_32bit_addressed,
#endif
                        heap_caps_free> event_buf;

#if defined(BOARD_HAS_PSRAM)
    std::mutex trace_buf_mutex;
    TF_Ringbuffer<char,
                  2 << 20,
                  malloc_psram,
                  heap_caps_free> trace_buf;
#endif

    ConfigRoot boot_id;
};

// To capture ESP-IDF log messages, use
// esp_log_set_vprintf(tf_event_log_vprintfln);
// If this is in a c (not cpp) file, also add
// extern int tf_event_log_vprintfln(const char *fmt, va_list args);
// instead of including event_log.h (a C++ header)
extern "C" int tf_event_log_vprintfln(const char *fmt, va_list args);

// To directly print from external C code, add
// extern int tf_event_log_printfln(const char *fmt, ...);
extern "C" [[gnu::format(__printf__, 1, 2)]] int tf_event_log_printfln(const char *fmt, ...);
