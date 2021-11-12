/* esp32-lib
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

extern WebServer server;

void EventLog::write(const char *buf, size_t len)
{
    std::lock_guard<std::mutex> lock{event_buf_mutex};
    String t = String(millis());
    size_t to_write = 12 + len + 1; // 12 for the longest timestamp (-2^31) and a space; 1 for the \n

    Serial.print(t);
    for(int i = 0; i < 12 - t.length(); ++i)
        Serial.print(' ');
    Serial.print(buf);

    if (buf[len - 1] != '\n') {
        Serial.println("");
    }

    if (event_buf.free() < to_write) {
        drop(to_write - event_buf.free());
    }

    for(int i = 0; i < t.length(); ++i) {
        event_buf.push(t[i]);
    }
    for(int i = 0; i < 12 - t.length(); ++i)
        event_buf.push(' ');

    for(int i = 0; i < len; ++i) {
        event_buf.push(buf[i]);
    }
    if (buf[len - 1] != '\n') {
        event_buf.push('\n');
    }
}

void EventLog::printfln(const char *fmt, ...)
{
    char buf[128];
    memset(buf, 0, sizeof(buf)/sizeof(buf[0]));

    va_list args;
    va_start (args, fmt);
    auto written = vsnprintf(buf, sizeof(buf)/sizeof(buf[0]), fmt, args);
    va_end (args);

    if (written >= sizeof(buf) / sizeof(buf[0])) {
        write("Next log message was truncated. Bump EventLog::printfln buffer size!", 69);
    }

    write(buf, written);
}

void EventLog::drop(size_t count)
{
    char c;
    for(int i = 0; i < count; ++i)
        event_buf.pop(&c);

    while(event_buf.used() > 0 && c != '\n')
        event_buf.pop(&c);
}

#define CHUNK_SIZE 1024
char chunk_buf[CHUNK_SIZE] = {0};

void EventLog::register_urls()
{
    server.on("/event_log", HTTP_GET, [this](WebServerRequest request) {
        std::lock_guard<std::mutex> lock{event_buf_mutex};
        auto used = event_buf.used();

        request.beginChunkedResponse(200, "text/plain");

        for(int index = 0; index < used; index += CHUNK_SIZE) {
            size_t to_write = MIN(CHUNK_SIZE, used - index);

            for(int i = 0; i < to_write; ++i) {
                event_buf.peek_offset((char *)(chunk_buf + i), index + i);
            }
            request.sendChunk(chunk_buf, to_write);
        }

        request.endChunkedResponse();
    });
}
