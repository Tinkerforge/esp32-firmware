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

#include "value_history.h"

#include "modules.h"

void ValueHistory::setup()
{
    history.setup();
    live.setup();
    history.clear();
    live.clear();

    history_last_update = millis();

    for (int i = 0; i < history.size(); ++i) {
        //float f = 5000.0 * sin(PI/120.0 * i) + 5000.0;
        // Use negative state to mark that these are pre-filled.
        history.push(-1);
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        int16_t history_val;

        if (samples_this_interval == 0) {
            history_val = -1;
            history.push(history_val); //TODO push 0 or -1 here? -1 will be translated into null when sending as json. However we document that there is only at most one block of null values at the start of the array indicating a reboot
            samples_last_interval = 0;
            samples_this_interval = 0;
        } else {
            float live_sum = 0;
            int16_t live_val;
            for(int i = 0; i < samples_this_interval; ++i) {
                live.peek_offset(&live_val, live.used() - 1 - i);
                live_sum += live_val;
            }

            history_val = live_sum / samples_this_interval;
            history.push(history_val);
            samples_last_interval = samples_this_interval;
            samples_this_interval = 0;
        }

        history_last_update = millis();

#if MODULE_WS_AVAILABLE()
        char *buf;
        int buf_written;
        const char *prefix = "{\"topic\":\"meter/history_samples\",\"payload\":{\"samples\":";

        if (history_val < 0) {
            buf_written = asprintf(&buf, "%s[null]}}\n", prefix);
        } else {
            buf_written = asprintf(&buf, "%s[%d]}}\n", prefix, (int)history_val);
        }

        if (buf_written > 0) {
            ws.web_sockets.sendToAllOwned(buf, buf_written);
        }
#endif
    }, 1000 * 60 * HISTORY_MINUTE_INTERVAL, 1000 * 60 * HISTORY_MINUTE_INTERVAL);

#if MODULE_WS_AVAILABLE()
    ws.addOnConnectCallback([this](WebSocketsClient client) {
        const size_t buf_size = RING_BUF_SIZE * 6 + 200;

        // live
        size_t buf_written = 0;
        char *buf = (char *)malloc(buf_size);

        if (buf == nullptr)
            return;

        buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%s", "{\"topic\":\"meter/live\",\"payload\":");
        buf_written += format_live(buf + buf_written, buf_size - buf_written);
        buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%s", "}\n");

        ws.web_sockets.sendToAllOwned(buf, buf_written);

        // history
        buf_written = 0;
        buf = (char *)malloc(buf_size);

        if (buf == nullptr)
            return;

        buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%s", "{\"topic\":\"meter/history\",\"payload\":");
        buf_written += format_history(buf + buf_written, buf_size - buf_written);
        buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%s", "}\n");

        ws.web_sockets.sendToAllOwned(buf, buf_written);
    });
#endif
}

void ValueHistory::register_urls(String base_url)
{
    server.on(("/" + base_url + "history").c_str(), HTTP_GET, [this](WebServerRequest request) {
        /*if (!initialized) {
            request.send(400, "text/html", "not initialized");
            return;
        }*/

        const size_t buf_size = RING_BUF_SIZE * 6 + 100;
        std::unique_ptr<char[]> buf{new char[buf_size]};
        size_t buf_written = format_history(buf.get(), buf_size);

        return request.send(200, "application/json; charset=utf-8", buf.get(), buf_written);
    });

    server.on(("/" + base_url + "live").c_str(), HTTP_GET, [this](WebServerRequest request) {
        /*if (!initialized) {
            request.send(400, "text/html", "not initialized");
            return;
        }*/

        const size_t buf_size = RING_BUF_SIZE * 6 + 100;
        std::unique_ptr<char[]> buf{new char[buf_size]};
        size_t buf_written = format_live(buf.get(), buf_size);

        return request.send(200, "application/json; charset=utf-8", buf.get(), buf_written);
    });
}

void ValueHistory::add_sample(float sample)
{
    int16_t val = std::min(INT16_MAX, (int)roundf(sample));
    live.push(val);
    ++samples_this_interval;

#if MODULE_WS_AVAILABLE()
    char *buf;
    int buf_written = asprintf(&buf, "{\"topic\":\"meter/live_samples\",\"payload\":{\"samples_per_second\":%f,\"samples\":[%d]}}\n", samples_per_second(), (int)val);

    if (buf_written > 0) {
        ws.web_sockets.sendToAllOwned(buf, buf_written);
    }
#endif
}

size_t ValueHistory::format_live(char *buf, size_t buf_size)
{
    size_t buf_written = 0;
    int16_t val;
    live.peek(&val);

    buf_written += snprintf(buf + buf_written, buf_size - buf_written, "{\"samples_per_second\":%f,\"samples\":[%d", samples_per_second(), val);

    for (int i = 1; (i < live.used() - 1) && live.peek_offset(&val, i) && buf_written < buf_size; ++i) {
        buf_written += snprintf(buf + buf_written, buf_size - buf_written, ",%d", val);
    }

    if (buf_written < buf_size) {
        buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%s", "]}");
    }

    return buf_written;
}

size_t ValueHistory::format_history(char *buf, size_t buf_size)
{
    size_t buf_written = 0;
    uint32_t offset = millis() - history_last_update;
    int16_t val;
    history.peek(&val);

    // Negative values are prefilled, because the ESP was booted less than 48 hours ago.
    if (val < 0) {
        buf_written += snprintf(buf + buf_written, buf_size - buf_written, "{\"offset\":%u,\"samples\":[%s", offset, "null");
    } else {
        buf_written += snprintf(buf + buf_written, buf_size - buf_written, "{\"offset\":%u,\"samples\":[%d", offset, (int)val);
    }

    for (int i = 1; i < history.used() && history.peek_offset(&val, i) && buf_written < buf_size; ++i) {
        // Negative values are prefilled, because the ESP was booted less than 48 hours ago.
        if (val < 0) {
            buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%s", ",null");
        } else {
            buf_written += snprintf(buf + buf_written, buf_size - buf_written, ",%d", (int)val);
        }
    }

    if (buf_written < buf_size) {
        buf_written += snprintf(buf + buf_written, buf_size - buf_written, "%s", "]}");
    }

    return buf_written;
}

float ValueHistory::samples_per_second()
{
    float samples_per_second = 0;

    if (this->samples_last_interval > 0) {
        samples_per_second = (float)this->samples_last_interval / (60 * HISTORY_MINUTE_INTERVAL);
    } else {
        samples_per_second = (float)this->samples_this_interval / millis() * 1000;
    }

    return samples_per_second;
}
