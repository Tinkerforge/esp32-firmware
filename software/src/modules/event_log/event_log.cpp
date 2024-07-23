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

    printfln_plain("    **** TINKERFORGE " BUILD_DISPLAY_NAME_UPPER " V%s ****", build_version_full_str_upper());
    printfln_plain("         %uK RAM SYSTEM   %u HEAP BYTES FREE", ESP.getHeapSize() / 1024, ESP.getFreeHeap());
    printfln_plain("READY.");
    printfln("Last reset reason was: %s", tf_reset_reason());
}

void EventLog::pre_setup()
{
    boot_id = Config::Object({
        {"boot_id", Config::Uint32(0)}
    });
}

void EventLog::post_setup()
{
    // Entropy is created by the wifi modem.
    auto id = esp_random();
    boot_id.get("boot_id")->updateUint(id);
}

void EventLog::get_timestamp(char buf[TIMESTAMP_LEN + 1])
{
    struct timeval tv_now;
    struct tm timeinfo;

    if (clock_synced(&tv_now)) {
        localtime_r(&tv_now.tv_sec, &timeinfo);

        // ISO 8601 allows omitting the T between date and time. Also  ',' is the preferred decimal sign.
        int written = strftime(buf, TIMESTAMP_LEN + 1, "%F %T", &timeinfo);
        snprintf(buf + written, TIMESTAMP_LEN + 1 - written, ",%03ld ", tv_now.tv_usec / 1000);
    } else {
        auto now = millis();
        auto secs = now / 1000;
        auto ms = now % 1000;
        auto to_write = snprintf_u(nullptr, 0, "%lu", secs) + 5; // + 5 for the decimal sign, fractional part and one space
        auto start = TIMESTAMP_LEN - to_write;

        for (int i = 0; i < start; ++i)
            buf[i] = ' ';
        snprintf(buf + start, to_write + 1, "%lu,%03lu ", secs, ms); // + 1 for the null terminator
    }

    buf[TIMESTAMP_LEN] = '\0';
}

void EventLog::trace_timestamp() {
#if defined(BOARD_HAS_PSRAM)
    // completely written in get_timestamp
    char timestamp_buf[TIMESTAMP_LEN + 1];
    this->get_timestamp(timestamp_buf);

    timestamp_buf[TIMESTAMP_LEN] = '\n';
    this->trace_write(timestamp_buf, TIMESTAMP_LEN + 1);
#endif
}

void EventLog::trace_write(const char *buf) {
#if defined(BOARD_HAS_PSRAM)
    this->trace_write(buf, strlen(buf));
#endif
}

void EventLog::trace_write(const char *buf, size_t len)
{
#if defined(BOARD_HAS_PSRAM)
    if (len >= 2 && buf[len - 2] == '\r' && buf[len - 1] == '\n') {
        len -= 2;
    }

    bool drop_line = trace_buf.free() < (len + 1);

    trace_buf.push_n(buf, len);
    if (buf[len - 1] != '\n') {
        trace_buf.push('\n');
    }

    if (drop_line)
        trace_buf.pop_until('\n');
#endif
}

int EventLog::tracefln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, va_list args)
{
#if defined(BOARD_HAS_PSRAM)
    char buf[768];
    auto buf_size = ARRAY_SIZE(buf);
    auto written = 0;
/*
    *(buf + written) = '|';
    ++written;

    *(buf + written) = ' ';
    ++written;

    if (prefix != nullptr && prefix_len < buf_size) {
        memcpy(buf + written, prefix, prefix_len);
        written += prefix_len;
    }

    while (written < event_log_alignment + 2) {
        *(buf + written) = ' ';
        ++written;
    }

    *(buf + written) = ' ';
    ++written;

    *(buf + written) = '|';
    ++written;

    *(buf + written) = ' ';
    ++written;
*/
    written += vsnprintf_u(buf + written, buf_size - written, fmt, args);
    if (written >= buf_size) {
        trace_write("Next log message was truncated. Bump EventLog::printfln buffer size!", 68); // Don't include termination in write request.
        written = buf_size - 1; // Don't include termination, which vsnprintf always leaves in.
    }

    trace_write(buf, written);

    return written;
#else
    return 0;
#endif
}

int EventLog::tracefln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, ...)
{
#if defined(BOARD_HAS_PSRAM)
    va_list args;
    va_start(args, fmt);
    int result = tracefln_prefixed(prefix, prefix_len, fmt, args);
    va_end(args);

    return result;
#else
    return 0;
#endif
}

void EventLog::trace_drop(size_t count)
{
#if defined(BOARD_HAS_PSRAM)
    char c = '\n';
    for (int i = 0; i < count; ++i)
        trace_buf.pop(&c);

    while (trace_buf.used() > 0 && c != '\n')
        trace_buf.pop(&c);
#endif
}

void EventLog::write(const char *buf, size_t len)
{
    std::lock_guard<std::mutex> lock{event_buf_mutex};

    size_t to_write = TIMESTAMP_LEN + len + 1; // 24 for an ISO-8601 timestamp and a space; 1 for the \n

    char timestamp_buf[TIMESTAMP_LEN + 1] = {0};
    this->get_timestamp(timestamp_buf);

    // Strip away \r\n.
    // We only use \n as line endings for the serial output as well as the event log.
    // Removing \r\n by reducing the length works,
    // because if a message does not end in \n we add the \n below.
    if (len >= 2 && buf[len - 2] == '\r' && buf[len - 1] == '\n') {
        len -= 2;
    }

    Serial.print(timestamp_buf);
    Serial.write(buf, len);

    if (buf[len - 1] != '\n') {
        Serial.println("");
    }

    if (event_buf.free() < to_write) {
        drop(to_write - event_buf.free());
    }

    for (int i = 0; i < TIMESTAMP_LEN; ++i) {
        event_buf.push(timestamp_buf[i]);
    }

    for (int i = 0; i < len; ++i) {
        event_buf.push(buf[i]);
    }
    if (buf[len - 1] != '\n') {
        event_buf.push('\n');
    }

#if MODULE_WS_AVAILABLE()
    size_t req_len = 0;
    {
        TFJsonSerializer json{nullptr, 0};
        json.addString(buf, len, false);
        req_len = json.end();
    }

    CoolString payload;
    if (!payload.reserve(2 + TIMESTAMP_LEN + req_len + 1)) // 2 - \"\"; 1 - \0
        return;

    payload += '"';

    payload.concat(timestamp_buf);

    {
        TFJsonSerializer json{payload.begin() + payload.length(), req_len + 1};
        json.addString(buf, len, false);
        payload.setLength(payload.length() + json.end());
    }

    payload += '"';
    ws.pushRawStateUpdate(payload, "event_log/message");
#endif
}

int EventLog::printfln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, va_list args)
{
    char buf[256];
    auto buf_size = ARRAY_SIZE(buf);
    auto written = 0;

    *(buf + written) = '|';
    ++written;

    *(buf + written) = ' ';
    ++written;

    if (prefix != nullptr && prefix_len < buf_size) {
        memcpy(buf + written, prefix, prefix_len);
        written += prefix_len;
    }

    while (written < event_log_alignment + 2) {
        *(buf + written) = ' ';
        ++written;
    }

    *(buf + written) = ' ';
    ++written;

    *(buf + written) = '|';
    ++written;

    *(buf + written) = ' ';
    ++written;

    written += vsnprintf_u(buf + written, buf_size - written, fmt, args);
    if (written >= buf_size) {
        write("Next log message was truncated. Bump EventLog::printfln buffer size!", 68); // Don't include termination in write request.
        written = buf_size - 1; // Don't include termination, which vsnprintf always leaves in.
    }

    write(buf, written);

    return written;
}

int EventLog::printfln_prefixed(const char *prefix, size_t prefix_len, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = printfln_prefixed(prefix, prefix_len, fmt, args);
    va_end(args);

    return result;
}

void EventLog::drop(size_t count)
{
    char c = '\n';
    for (int i = 0; i < count; ++i)
        event_buf.pop(&c);

    while (event_buf.used() > 0 && c != '\n')
        event_buf.pop(&c);
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

extern EventLog logger;

int tf_event_log_vprintfln(const char *fmt, va_list args)
{
    return logger.printfln_prefixed("external code", 13, fmt, args);
}

int tf_event_log_printfln(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = logger.printfln_prefixed("external code", 13, fmt, args);
    va_end(args);

    return result;
}
