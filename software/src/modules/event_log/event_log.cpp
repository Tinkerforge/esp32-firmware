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

#include "event_log.h"

#include <Arduino.h>
#include <time.h>
#include <inttypes.h>
#include <TFJson.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools.h"

void EventLog::pre_init()
{
    event_buf.setup();
#if defined(BOARD_HAS_PSRAM)
    trace_buf.setup();
#endif

    printfln_prefixed(nullptr, 0, "    **** TINKERFORGE " BUILD_DISPLAY_NAME_UPPER " V%s ****", build_version_full_str_upper());
    printfln_prefixed(nullptr, 0, "         %uK RAM SYSTEM   %u HEAP BYTES FREE", ESP.getHeapSize() / 1024, ESP.getFreeHeap());
    printfln_prefixed(nullptr, 0, "READY.");
    printfln("Last reset reason was: %s", tf_reset_reason());
}

void EventLog::pre_setup()
{
    boot_id = Config::Object({
        {"boot_id", Config::Uint32(0)}
    });
}

#define CHUNK_SIZE 1024

void EventLog::register_urls()
{
    server.on_HTTPThread("/event_log", HTTP_GET, [this](WebServerRequest request) {
        std::lock_guard<std::mutex> lock{event_buf_mutex};
        auto chunk_buf = heap_alloc_array<char>(CHUNK_SIZE);
        auto used = event_buf.used();

        request.beginChunkedResponse(200);

        for (int index = 0; index < used; index += CHUNK_SIZE) {
            size_t to_write = MIN(CHUNK_SIZE, used - index);

            for (int i = 0; i < to_write; ++i) {
                event_buf.peek_offset((char *)(chunk_buf.get() + i), index + i);
            }

            request.sendChunk(chunk_buf.get(), to_write);
        }

        return request.endChunkedResponse();
    });

#if defined(BOARD_HAS_PSRAM)
    server.on_HTTPThread("/trace_log", HTTP_GET, [this](WebServerRequest request) {
        //std::lock_guard<std::mutex> lock{event_buf_mutex};

        request.beginChunkedResponse(200);

        char *first_chunk, *second_chunk;
        size_t first_len, second_len;
        trace_buf.get_chunks(&first_chunk, &first_len, &second_chunk, &second_len);

        request.sendChunk(first_chunk, first_len);
        if (second_len > 0)
            request.sendChunk(second_chunk, second_len);

        return request.endChunkedResponse();
    });
#endif

    api.addState("event_log/boot_id", &boot_id);
}

void EventLog::post_setup()
{
    // Entropy is created by the wifi modem.
    auto id = esp_random();
    boot_id.get("boot_id")->updateUint(id);
}

void EventLog::format_timestamp(char buf[EVENT_LOG_TIMESTAMP_LENGTH + 1 /* \0 */])
{
    struct timeval tv_now;
    struct tm timeinfo;

    if (rtc.clock_synced(&tv_now)) {
        localtime_r(&tv_now.tv_sec, &timeinfo);

        // ISO 8601 allows omitting the T between date and time. Also  ',' is the preferred decimal sign.
        size_t written = strftime(buf, EVENT_LOG_TIMESTAMP_LENGTH + 1, "%F %T", &timeinfo);
        snprintf(buf + written, EVENT_LOG_TIMESTAMP_LENGTH + 1 - written, ",%03ld", tv_now.tv_usec / 1000);
    } else {
        uint32_t now = millis();
        uint32_t secs = now / 1000;
        uint32_t ms = now % 1000;
        size_t to_write = snprintf_u(nullptr, 0, "%" PRIu32, secs) + 4; // +4 for the decimal sign and fractional part
        size_t start = EVENT_LOG_TIMESTAMP_LENGTH - to_write;

        for (size_t i = 0; i < start; ++i) {
            buf[i] = ' ';
        }

        snprintf(buf + start, to_write + 1, "%" PRIu32 ",%03" PRIu32, secs, ms); // +1 for the NUL-terminator
    }

    buf[EVENT_LOG_TIMESTAMP_LENGTH] = '\0';
}

size_t EventLog::vsnprintf_prefixed(char *buf, size_t buf_len, const char *prefix, size_t prefix_len, const char *fmt, va_list args)
{
    if (buf_len < EVENT_LOG_TIMESTAMP_LENGTH + 1 /* \0 */) {
        return 0;
    }

    size_t written = 0;

    format_timestamp(buf);
    written += EVENT_LOG_TIMESTAMP_LENGTH;

    if (written + 3 <= buf_len) {
        buf[written++] = ' ';
        buf[written++] = '|';
        buf[written++] = ' ';
    }

    if (prefix != nullptr && written + prefix_len <= buf_len) {
        memcpy(buf + written, prefix, prefix_len);
        written += prefix_len;
    }

    while (written < buf_len && written < EVENT_LOG_TIMESTAMP_LENGTH + 3 + event_log_alignment) {
        buf[written++] = ' ';
    }

    if (written + 3 <= buf_len) {
        buf[written++] = ' ';
        buf[written++] = '|';
        buf[written++] = ' ';
    }

    if (written < buf_len) {
        written += vsnprintf_u(buf + written, buf_len - written, fmt, args);
    }

    return written;
}

void EventLog::print_drop(size_t count)
{
    char c = '\n';

    for (int i = 0; i < count; ++i) {
        event_buf.pop(&c);
    }

    while (event_buf.used() > 0 && c != '\n'){
        event_buf.pop(&c);
    }
}

void EventLog::print_timestamp()
{
    char buf[EVENT_LOG_TIMESTAMP_LENGTH + 1 /* \0 */];

    format_timestamp(buf);
    println_plain(buf, EVENT_LOG_TIMESTAMP_LENGTH);
}

size_t EventLog::println_plain(const char *buf, size_t len)
{
    // Strip away \r\n. We only use \n as line endings for the serial
    // output as well as the event log. Removing \r\n by reducing the length
    // works, because if a message does not end in \n we add the \n below.
    if (len >= 2 && buf[len - 2] == '\r' && buf[len - 1] == '\n') {
        len -= 2;
    }

    Serial.write(buf, len);

    if (buf[len - 1] != '\n') {
        Serial.println("");
    }

    {
        std::lock_guard<std::mutex> lock{event_buf_mutex};

        if (event_buf.free() < len + 1 /* \n */) {
            print_drop(len + 1 /* \n */ - event_buf.free());
        }

        for (size_t i = 0; i < len; ++i) {
            event_buf.push(buf[i]);
        }

        if (buf[len - 1] != '\n') {
            event_buf.push('\n');
        }
    }

#if MODULE_WS_AVAILABLE()
    size_t json_len;

    {
        TFJsonSerializer json{nullptr, 0};
        json.addString(buf, len, false);
        json_len = json.end();
    }

    CoolString payload;

    if (payload.reserve(1 /* " */ + json_len + 1 /* " */ + 1 /* \0 */)) {
        payload += '"';

        {
            TFJsonSerializer json{payload.begin() + payload.length(), json_len + 1 /* \0 */};
            json.addString(buf, len, false);
            payload.setLength(payload.length() + json.end());
        }

        payload += '"';

        ws.pushRawStateUpdate(payload, "event_log/message");
    }
#endif

    if (buf[len - 1] != '\n') {
        return len + 1;
    }

    return len;
}

size_t EventLog::vprintfln_plain(const char *fmt, va_list args)
{
    size_t written = 0;
    char buf[256];
    size_t buf_len = ARRAY_SIZE(buf);

    written += vsnprintf_u(buf, buf_len, fmt, args);

    if (written >= buf_len) {
        printfln_plain("Next log message was truncated. Bump EventLog::vprintfln_plain buffer size!");
        written = buf_len - 1; // Don't include termination, which vsnprintf always leaves in.
    }

    println_plain(buf, written);

    return written;
}

size_t EventLog::printfln_plain(const char *fmt, ...)
{
    size_t written = 0;
    va_list args;

    va_start(args, fmt);
    written += vprintfln_plain(fmt, args);
    va_end(args);

    return written;
}

size_t EventLog::vprintfln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, va_list args)
{
    size_t written = 0;
    char buf[EVENT_LOG_TIMESTAMP_LENGTH + 256];
    size_t buf_len = ARRAY_SIZE(buf);

    written += vsnprintf_prefixed(buf, buf_len, prefix, prefix_len, fmt, args);

    if (written >= buf_len) {
        printfln_prefixed(prefix, prefix_len, "Next log message was truncated. Bump EventLog::vprintfln_prefixed buffer size!");
        written = buf_len - 1; // Don't include termination, which vsnprintf always leaves in
    }

    println_plain(buf, written);

    return written;
}

size_t EventLog::printfln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, ...)
{
    size_t written = 0;
    va_list args;

    va_start(args, fmt);
    written += vprintfln_prefixed(prefix, prefix_len, fmt, args);
    va_end(args);

    return written;
}

void EventLog::trace_drop(size_t count)
{
#if defined(BOARD_HAS_PSRAM)
    char c = '\n';

    for (int i = 0; i < count; ++i) {
        trace_buf.pop(&c);
    }

    while (trace_buf.used() > 0 && c != '\n'){
        trace_buf.pop(&c);
    }
#else
    (void)count;
#endif
}

void EventLog::trace_timestamp()
{
#if defined(BOARD_HAS_PSRAM)
    char buf[EVENT_LOG_TIMESTAMP_LENGTH + 1 /* \0 */];

    format_timestamp(buf);
    traceln_plain(buf, EVENT_LOG_TIMESTAMP_LENGTH);
#endif
}

size_t EventLog::traceln_plain(const char *buf, size_t len)
{
#if defined(BOARD_HAS_PSRAM)
    // Strip away \r\n. We only use \n as line endings for the serial
    // output as well as the event log. Removing \r\n by reducing the length
    // works, because if a message does not end in \n we add the \n below.
    if (len >= 2 && buf[len - 2] == '\r' && buf[len - 1] == '\n') {
        len -= 2;
    }

    {
        std::lock_guard<std::mutex> lock{trace_buf_mutex};
        bool drop_line = trace_buf.free() < (len + 1 /* \n */);

        trace_buf.push_n(buf, len);

        if (buf[len - 1] != '\n') {
            trace_buf.push('\n');
        }

        if (drop_line) {
            trace_buf.pop_until('\n');
        }
    }

    return len;
#else
    (void)buf;
    (void)len;

    return 0;
#endif
}

size_t EventLog::vtracefln_plain(const char *fmt, va_list args)
{
    size_t written = 0;
#if defined(BOARD_HAS_PSRAM)
    char buf[256];
    size_t buf_len = ARRAY_SIZE(buf);

    written += vsnprintf_u(buf, buf_len, fmt, args);

    if (written >= buf_len) {
        tracefln_plain("Next log message was truncated. Bump EventLog::vtracefln_plain buffer size!");
        written = buf_len - 1; // Don't include termination, which vsnprintf always leaves in
    }

    traceln_plain(buf, written);
#endif

    return written;
}

size_t EventLog::tracefln_plain(const char *fmt, ...)
{
    size_t written = 0;
#if defined(BOARD_HAS_PSRAM)
    va_list args;

    va_start(args, fmt);
    written += vtracefln_plain(fmt, args);
    va_end(args);
#endif

    return written;
}

size_t EventLog::vtracefln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, va_list args)
{
    size_t written = 0;
#if defined(BOARD_HAS_PSRAM)
    char buf[EVENT_LOG_TIMESTAMP_LENGTH + 256];
    size_t buf_len = ARRAY_SIZE(buf);

    written += vsnprintf_prefixed(buf, buf_len, prefix, prefix_len, fmt, args);

    if (written >= buf_len) {
        tracefln_prefixed(prefix, prefix_len, "Next log message was truncated. Bump EventLog::vtracefln_prefixed buffer size!");
        written = buf_len - 1; // Don't include termination, which vsnprintf always leaves in.
    }

    traceln_plain(buf, written);
#endif

    return written;
}

size_t EventLog::tracefln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, ...)
{
    size_t written = 0;
#if defined(BOARD_HAS_PSRAM)
    va_list args;

    va_start(args, fmt);
    written += vtracefln_prefixed(prefix, prefix_len, fmt, args);
    va_end(args);
#endif

    return written;
}

int tf_event_log_vprintfln(const char *fmt, va_list args)
{
    return logger.vprintfln_prefixed("external code", 13, fmt, args);
}

int tf_event_log_printfln(const char *fmt, ...)
{
    size_t written = 0;
    va_list args;

    va_start(args, fmt);
    written += logger.vprintfln_prefixed("external code", 13, fmt, args);
    va_end(args);

    return written;
}
