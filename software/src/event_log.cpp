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

#include "web_server.h"

#include "time.h"

#include "tools.h"

// Global definition here to match the declaration in event_log.h.
EventLog logger;

void EventLog::pre_init()
{
    event_buf.setup();
}

void EventLog::get_timestamp(char buf[TIMESTAMP_LEN + 1])
{
    struct timeval tv_now;
    struct tm timeinfo;

    if (clock_synced(&tv_now)) {
        localtime_r(&tv_now.tv_sec, &timeinfo);

        // ISO 8601 allows omitting the T between date and time. Also  ',' is the preferred decimal sign.
        int written = strftime(buf, TIMESTAMP_LEN + 1, "%F %T", &timeinfo);
        snprintf(buf + written, TIMESTAMP_LEN + 1 - written, ",%03ld  ", tv_now.tv_usec / 1000);
    } else {
        auto now = millis();
        auto secs = now / 1000;
        auto ms = now % 1000;
        auto to_write = snprintf(nullptr, 0, "%lu", secs) + 6; // + 6 for the decimal sign, fractional part and two spaces
        auto start = TIMESTAMP_LEN - to_write;

        for (int i = 0; i < TIMESTAMP_LEN; ++i)
            buf[i] = ' ';
        snprintf(buf + start, to_write + 1, "%lu,%03lu  ", secs, ms); // + 1 for the null terminator
    }

    buf[TIMESTAMP_LEN] = '\0';
}

void EventLog::write(const char *buf, size_t len)
{
    std::lock_guard<std::mutex> lock{event_buf_mutex};

    size_t to_write = TIMESTAMP_LEN + len + 1; // 12 for the longest timestamp (-2^31) and a space; 1 for the \n

    char timestamp_buf[TIMESTAMP_LEN + 1] = {0};
    this->get_timestamp(timestamp_buf);

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
}

void EventLog::printfln(const char *fmt, va_list args) {
    char buf[256];
    auto buf_size = sizeof(buf) / sizeof(buf[0]);
    memset(buf, 0, buf_size);

    auto written = vsnprintf(buf, buf_size, fmt, args);
    if (written >= buf_size) {
        write("Next log message was truncated. Bump EventLog::printfln buffer size!", 68); // Don't include termination in write request.
        written = buf_size - 1; // Don't include termination, which vsnprintf always leaves in.
    }

    write(buf, written);
}

void EventLog::printfln(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    this->printfln(fmt, args);
    va_end(args);
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

        request.beginChunkedResponse(200, "text/plain");

        for (int index = 0; index < used; index += CHUNK_SIZE) {
            size_t to_write = MIN(CHUNK_SIZE, used - index);

            for (int i = 0; i < to_write; ++i) {
                event_buf.peek_offset((char *)(chunk_buf.get() + i), index + i);
            }

            request.sendChunk(chunk_buf.get(), to_write);
        }

        return request.endChunkedResponse();
    });
}
