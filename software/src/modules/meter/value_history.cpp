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

void ValueHistory::setup()
{
    history.setup();
    live.setup();
    history.clear();
    live.clear();

    for (int i = 0; i < history.size(); ++i) {
        //float f = 5000.0 * sin(PI/120.0 * i) + 5000.0;
        // Use negative state to mark that these are pre-filled.
        history.push(-1);
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        if (samples_last_interval == 0) {
            history.push(-1); //TODO push 0 or -1 here? -1 will be translated into null when sending as json. However we document that there is only at most one block of null values at the start of the array indicating a reboot
            samples_per_interval = 0;
            samples_last_interval = 0;
            return;
        }

        float live_sum = 0;
        int16_t val;
        for(int i = 0; i < samples_last_interval; ++i) {
            live.peek_offset(&val, live.used() - 1 - i);
            live_sum += val;
        }

        history.push((int16_t)(live_sum / samples_last_interval));
        samples_per_interval = samples_last_interval;
        samples_last_interval = 0;
    }, 1000 * 60 * HISTORY_MINUTE_INTERVAL, 1000 * 60 * HISTORY_MINUTE_INTERVAL);
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
        size_t buf_written = 0;

        int16_t val;
        history.peek(&val);
        // Negative values are prefilled, because the ESP was booted less than 48 hours ago.
        if (val < 0)
            buf_written += snprintf(buf.get() + buf_written, buf_size - buf_written, "%s", "[null");
        else
            buf_written += snprintf(buf.get() + buf_written, buf_size - buf_written, "[%d", (int)val);

        for (int i = 1; i < history.used() && history.peek_offset(&val, i) && buf_written < buf_size; ++i) {
            // Negative values are prefilled, because the ESP was booted less than 48 hours ago.
            if (val < 0)
                buf_written += snprintf(buf.get() + buf_written, buf_size - buf_written, "%s", ",null");
            else
                buf_written += snprintf(buf.get() + buf_written, buf_size - buf_written, ",%d", (int)val);
        }

        if (buf_written < buf_size)
            buf_written += snprintf(buf.get() + buf_written, buf_size - buf_written, "%c", ']');

        return request.send(200, "application/json; charset=utf-8", buf.get(), buf_written);
    });

    server.on(("/" + base_url + "live").c_str(), HTTP_GET, [this](WebServerRequest request) {
        /*if (!initialized) {
            request.send(400, "text/html", "not initialized");
            return;
        }*/

        const size_t buf_size = RING_BUF_SIZE * 6 + 100;
        std::unique_ptr<char[]> buf{new char[buf_size]};
        size_t buf_written = 0;

        int16_t val;
        live.peek(&val);
        float samples_per_second = 0;
        if (this->samples_per_interval > 0) {
            samples_per_second = ((float)this->samples_per_interval) / (60 * HISTORY_MINUTE_INTERVAL);
        } else {
            samples_per_second = (float)this->samples_last_interval / millis() * 1000;
        }
        buf_written += snprintf(buf.get() + buf_written, buf_size - buf_written, "{\"samples_per_second\":%f,\"samples\":[%d", samples_per_second, val);

        for (int i = 1; (i < live.used() - 1) && live.peek_offset(&val, i) && buf_written < buf_size; ++i) {
            buf_written += snprintf(buf.get() + buf_written, buf_size - buf_written, ",%d", val);
        }
        if (buf_written < buf_size)
            buf_written += snprintf(buf.get() + buf_written, buf_size - buf_written, "%s", "]}");
        return request.send(200, "application/json; charset=utf-8", buf.get(), buf_written);
    });
}

void ValueHistory::add_sample(float sample)
{
    int16_t val = (int16_t)min((float)INT16_MAX, sample);
    live.push(val);
    ++samples_last_interval;
}
