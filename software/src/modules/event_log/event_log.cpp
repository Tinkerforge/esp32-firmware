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

#undef vprintfln
#undef vprintfln_continue
#undef  printfln
#undef  printfln_continue
#undef  printfln_debug
#undef vtracefln
#undef vtracefln_continue
#undef  tracefln
#undef  tracefln_continue
#undef  tracefln_debug

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

    printfln_prefixed("", 0, "    **** TINKERFORGE " BUILD_DISPLAY_NAME_UPPER " V%s ****", build_version_full_str_upper());
    printfln_prefixed("", 0, "         %uK RAM SYSTEM   %u HEAP BYTES FREE", ESP.getHeapSize() / 1024, ESP.getFreeHeap());
    printfln_prefixed("", 0, "READY.");
    printfln_prefixed("", 0, "Last reset reason was: %s", tf_reset_reason());
}

size_t EventLog::alloc_trace_buffer(const char *name, size_t size) {
#if defined(BOARD_HAS_PSRAM)
    if (boot_stage > BootStage::PRE_SETUP){
        esp_system_abort("Using alloc_trace_buffer after the pre_setup is not allowed!");
    }

    if (this->trace_buffer_size_allocd + trace_buffer_size_allocd > MAX_TRACE_BUFFERS_SIZE){
        esp_system_abort("Maximum size of trace buffers exceeded!");
    }

    if (trace_buffers_in_use == trace_buffers.size()){
        esp_system_abort("Maximum number of trace buffers exceeded!");
    }

    trace_buffers[trace_buffers_in_use].name = name;
    trace_buffers[trace_buffers_in_use].buf.setup(size);
    ++trace_buffers_in_use;
    return trace_buffers_in_use - 1;
#else
    return -1;
#endif
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
        char chunk_buf[CHUNK_SIZE]; // The HTTP task's stack is large enough.
        auto used = event_buf.used();

        request.beginChunkedResponse(200);

        for (int index = 0; index < used; index += CHUNK_SIZE) {
            size_t to_write = MIN(CHUNK_SIZE, used - index);

            for (int i = 0; i < to_write; ++i) {
                event_buf.peek_offset(chunk_buf + i, index + i);
            }

            int result = request.sendChunk(chunk_buf, to_write);
            if (result != ESP_OK) {
                if (result != ESP_ERR_HTTPD_RESP_SEND) { // Don't log connection closed during transfer. This happens when the front-end is reloaded after a websocket reconnect.
                    Serial.printf("/event_log sendChunk failed: %s (0x%X)\n", esp_err_to_name(result), static_cast<unsigned int>(result)); // Can't write to the event log while holding the event_buf_mutex.
                }
                break;
            }
        }

        return request.endChunkedResponse();
    });


    server.on_HTTPThread("/trace_log", HTTP_GET, [this](WebServerRequest request) {
#if defined(BOARD_HAS_PSRAM)
        request.beginChunkedResponse(200);

        for (size_t i = 0; i < trace_buffers_in_use; ++i) {
            auto &trace_buffer = trace_buffers[i];
            std::lock_guard<std::mutex> lock{trace_buffer.mutex};

            char *first_chunk, *second_chunk;
            size_t first_len, second_len;
            trace_buffer.buf.get_chunks(&first_chunk, &first_len, &second_chunk, &second_len);

            char buf[128];
            size_t written = snprintf(buf, ARRAY_SIZE(buf), "__begin_%.100s__\n", trace_buffer.name);
            request.sendChunk(buf, written);

            if (first_len > 0)
                request.sendChunk(first_chunk, first_len);
            if (second_len > 0)
                request.sendChunk(second_chunk, second_len);

            written = snprintf(buf, ARRAY_SIZE(buf), "__end_%.100s__\n", trace_buffer.name);
            request.sendChunk(buf, written);
        }

        return request.endChunkedResponse();
#else
        return request.send(200);
#endif
    });


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

    bool skip_prefix = prefix == nullptr && prefix_len == 0;
    if (!skip_prefix) {
        if (prefix != nullptr && prefix_len > 0 && written + prefix_len <= buf_len) {
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

    while (event_buf.used() > 0 && c != '\n') {
        event_buf.pop(&c);
    }
}

void EventLog::print_timestamp()
{
    char buf[EVENT_LOG_TIMESTAMP_LENGTH + 1 /* \n | \0 */];

    format_timestamp(buf);
    buf[EVENT_LOG_TIMESTAMP_LENGTH] = '\n';

    print_plain(buf, EVENT_LOG_TIMESTAMP_LENGTH + 1);
}

size_t EventLog::print_plain(const char *buf, size_t len)
{
    Serial.write(buf, len);

    {
        std::lock_guard<std::mutex> lock{event_buf_mutex};

        if (event_buf.free() < len) {
            print_drop(len - event_buf.free());
        }

        for (size_t i = 0; i < len; ++i) {
            event_buf.push(buf[i]);
        }
    }

#if MODULE_WS_AVAILABLE()
    size_t stripped_len = len;

    if (len >= 1 && buf[len - 1] == '\n') {
        stripped_len = len - 1;
    }

    size_t json_len;

    {
        TFJsonSerializer json{nullptr, 0};
        json.addString(buf, stripped_len, false);
        json_len = json.end();
    }

    CoolString payload;

    if (payload.reserve(1 /* " */ + json_len + 1 /* " */ + 1 /* \0 */)) {
        payload += '"';

        {
            TFJsonSerializer json{payload.begin() + payload.length(), json_len + 1 /* \0 */};
            json.addString(buf, stripped_len, false);
            payload.setLength(payload.length() + json.end());
        }

        payload += '"';

        ws.pushRawStateUpdate(payload, "event_log/message");
    }
#endif

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

    // The IDF might log messages ending with "\r\n" via tf_event_log_[v]printfln
    if (written >= 2 && buf[written - 2] == '\r' && buf[written - 1] == '\n') {
        written -= 2;
    }

    buf[written++] = '\n'; // At this point written < buf_len is guaranteed
    print_plain(buf, written);

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

    // The IDF might log messages ending with "\r\n" via tf_event_log_[v]printfln
    if (written >= 2 && buf[written - 2] == '\r' && buf[written - 1] == '\n') {
        written -= 2;
    }

    buf[written++] = '\n'; // At this point written < buf_len is guaranteed
    print_plain(buf, written);

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

size_t EventLog::get_trace_buffer_idx(const char *name) {
#if defined(BOARD_HAS_PSRAM)
    for (size_t i = 0; i < trace_buffers_in_use; ++i) {
        if (trace_buffers[i].name != name) // TODO rodata check
            continue;
        return i;
    }
#else
    (void)name;
#endif
    return -1;
}

void EventLog::trace_drop(size_t trace_buf_idx, size_t count)
{
#if defined(BOARD_HAS_PSRAM)
    char c = '\n';

    if (trace_buf_idx == -1)
        return;

    auto *trace_buffer = &this->trace_buffers[trace_buf_idx];

    for (int i = 0; i < count; ++i) {
        trace_buffer->buf.pop(&c);
    }

    while (trace_buffer->buf.used() > 0 && c != '\n') {
        trace_buffer->buf.pop(&c);
    }
#else
    (void)trace_buf_idx;
    (void)count;
#endif
}

void EventLog::trace_timestamp(size_t trace_buf_idx)
{
#if defined(BOARD_HAS_PSRAM)
    char buf[EVENT_LOG_TIMESTAMP_LENGTH + 1 /* \n | \0 */];

    format_timestamp(buf);
    buf[EVENT_LOG_TIMESTAMP_LENGTH] = '\n';

    trace_plain(trace_buf_idx, buf, EVENT_LOG_TIMESTAMP_LENGTH + 1);
#else
    (void)trace_buf_idx;
#endif
}

size_t EventLog::trace_plain(size_t trace_buf_idx, const char *buf, size_t len)
{
#if defined(BOARD_HAS_PSRAM)
    if (trace_buf_idx == -1)
        return 0;

    auto *trace_buffer = &this->trace_buffers[trace_buf_idx];

    std::lock_guard<std::mutex> lock{trace_buffer->mutex};
    bool drop_line = trace_buffer->buf.free() < len;

    trace_buffer->buf.push_n(buf, len);

    if (drop_line) {
        trace_buffer->buf.pop_until('\n');
    }

    return len;
#else
    (void)trace_buf_idx;
    (void)buf;
    (void)len;

    return 0;
#endif
}

size_t EventLog::vtracefln_plain(size_t trace_buf_idx, const char *fmt, va_list args)
{
    size_t written = 0;
#if defined(BOARD_HAS_PSRAM)
    char buf[256];
    size_t buf_len = ARRAY_SIZE(buf);

    written += vsnprintf_u(buf, buf_len, fmt, args);

    if (written >= buf_len) {
        tracefln_plain(trace_buf_idx, "Next log message was truncated. Bump EventLog::vtracefln_plain buffer size!");
        written = buf_len - 1; // Don't include termination, which vsnprintf always leaves in
    }

    // The IDF might log messages ending with "\r\n" via tf_event_log_[v]printfln
    if (written >= 2 && buf[written - 2] == '\r' && buf[written - 1] == '\n') {
        written -= 2;
    }

    buf[written++] = '\n'; // At this point written < buf_len is guaranteed
    trace_plain(trace_buf_idx, buf, written);
#endif

    return written;
}

size_t EventLog::tracefln_plain(size_t trace_buf_idx, const char *fmt, ...)
{
    size_t written = 0;
#if defined(BOARD_HAS_PSRAM)
    va_list args;

    va_start(args, fmt);
    written += vtracefln_plain(trace_buf_idx, fmt, args);
    va_end(args);
#endif

    return written;
}

size_t EventLog::vtracefln_prefixed(size_t trace_buf_idx, const char *prefix, size_t prefix_len, const char *fmt, va_list args)
{
    size_t written = 0;
#if defined(BOARD_HAS_PSRAM)
    char buf[EVENT_LOG_TIMESTAMP_LENGTH + 256];
    size_t buf_len = ARRAY_SIZE(buf);

    written += vsnprintf_prefixed(buf, buf_len, prefix, prefix_len, fmt, args);

    if (written >= buf_len) {
        tracefln_prefixed(trace_buf_idx, prefix, prefix_len, "Next log message was truncated. Bump EventLog::vtracefln_prefixed buffer size!");
        written = buf_len - 1; // Don't include termination, which vsnprintf always leaves in.
    }

    // The IDF might log messages ending with "\r\n" via tf_event_log_[v]printfln
    if (written >= 2 && buf[written - 2] == '\r' && buf[written - 1] == '\n') {
        written -= 2;
    }

    buf[written++] = '\n'; // At this point written < buf_len is guaranteed
    trace_plain(trace_buf_idx, buf, written);
#endif

    return written;
}

size_t EventLog::tracefln_prefixed(size_t trace_buf_idx, const char *prefix, size_t prefix_len, const char *fmt, ...)
{
    size_t written = 0;
#if defined(BOARD_HAS_PSRAM)
    va_list args;

    va_start(args, fmt);
    written += vtracefln_prefixed(trace_buf_idx, prefix, prefix_len, fmt, args);
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
