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
#include "module_dependencies.h"

#include "gcc_warnings.h"
#ifdef __GNUC__
// The code is this file contains several casts to a type defined by a macro, which may result in useless casts.
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

// Unchecked snprintf that returns size_t
_ATTRIBUTE ((__format__ (__printf__, 3, 4)))
static size_t snprintf_u(char *buf, size_t len, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int res = vsnprintf(buf, len, format, args);
    va_end(args);

    return res < 0 ? 0 : static_cast<size_t>(res);
}

void ValueHistory::setup()
{
    history.setup();
    live.setup();
    history.clear();
    live.clear();

    METER_VALUE_HISTORY_VALUE_TYPE val_min = std::numeric_limits<METER_VALUE_HISTORY_VALUE_TYPE>::lowest();

    for (size_t i = 0; i < history.size(); ++i) {
        //float f = 5000.0 * sin(PI/120.0 * i) + 5000.0;
        // Use negative state to mark that these are pre-filled.
        history.push(val_min);
    }

    chars_per_value = max(String(METER_VALUE_HISTORY_VALUE_MIN).length(), String(METER_VALUE_HISTORY_VALUE_MAX).length());
    // val_min values are replaced with null -> require at least 4 chars per value.
    chars_per_value = max(4U, chars_per_value);
    // For ',' between the values.
    ++chars_per_value;

#if MODULE_WS_AVAILABLE()
    ws.addOnConnectCallback([this](WebSocketsClient client) {
        const size_t buf_size = RING_BUF_SIZE * chars_per_value + 200;

        // live
        size_t buf_written = 0;
        char *buf = static_cast<char *>(malloc(buf_size));

        if (buf == nullptr)
            return;

        buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "{\"topic\":\"%s/live\",\"payload\":", base_url.c_str());
        buf_written += format_live(buf + buf_written, buf_size - buf_written);
        buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "}\n");

        client.sendOwned(buf, buf_written);

        // history
        buf_written = 0;
        buf = static_cast<char *>(malloc(buf_size));

        if (buf == nullptr)
            return;

        buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "{\"topic\":\"%s/history\",\"payload\":", base_url.c_str());
        buf_written += format_history(buf + buf_written, buf_size - buf_written);
        buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "}\n");

        client.sendOwned(buf, buf_written);
    });
#endif
}

void ValueHistory::register_urls(String base_url_)
{
    base_url = base_url_;

    server.on(("/" + base_url + "/history").c_str(), HTTP_GET, [this](WebServerRequest request) {
        /*if (!initialized) {
            request.send(400, "text/html", "not initialized");
            return;
        }*/

        const size_t buf_size = RING_BUF_SIZE * chars_per_value + 100;
        std::unique_ptr<char[]> buf{new char[buf_size]};
        size_t buf_written = format_history(buf.get(), buf_size);

        return request.send(200, "application/json; charset=utf-8", buf.get(), static_cast<ssize_t>(buf_written));
    });

    server.on(("/" + base_url + "/live").c_str(), HTTP_GET, [this](WebServerRequest request) {
        /*if (!initialized) {
            request.send(400, "text/html", "not initialized");
            return;
        }*/

        const size_t buf_size = RING_BUF_SIZE * chars_per_value + 100;
        std::unique_ptr<char[]> buf{new char[buf_size]};
        size_t buf_written = format_live(buf.get(), buf_size);

        return request.send(200, "application/json; charset=utf-8", buf.get(), static_cast<ssize_t>(buf_written));
    });
}

void ValueHistory::add_sample(float sample)
{
    METER_VALUE_HISTORY_VALUE_TYPE val_min = std::numeric_limits<METER_VALUE_HISTORY_VALUE_TYPE>::lowest();
    METER_VALUE_HISTORY_VALUE_TYPE val = clamp(static_cast<METER_VALUE_HISTORY_VALUE_TYPE>(METER_VALUE_HISTORY_VALUE_MIN),
                                               static_cast<METER_VALUE_HISTORY_VALUE_TYPE>(roundf(sample)),
                                               static_cast<METER_VALUE_HISTORY_VALUE_TYPE>(METER_VALUE_HISTORY_VALUE_MAX));
    live.push(val);
    live_last_update = millis();
    end_this_interval = live_last_update;

    if (samples_this_interval == 0) {
        begin_this_interval = live_last_update;
    }

    ++samples_this_interval;
    sum_this_interval += val;

#if MODULE_WS_AVAILABLE()
    {
        char *buf;
        int buf_written = asprintf(&buf, "{\"topic\":\"%s/live_samples\",\"payload\":{\"samples_per_second\":%f,\"samples\":[%d]}}\n", base_url.c_str(), static_cast<double>(samples_per_second()), static_cast<int>(val));

        if (buf_written > 0) {
            ws.web_sockets.sendToAllOwned(buf, static_cast<size_t>(buf_written));
        }
    }
#endif

    // start history task when first sample arrives. this adds the first sample to the
    // history immediately to avoid and empty history for the first history period
    if (live.used() == 1) {
        task_scheduler.scheduleWithFixedDelay([this, val_min](){
            METER_VALUE_HISTORY_VALUE_TYPE history_val;

            if (samples_this_interval == 0) {
                history_val = val_min; // TODO push 0 or intxy_t min here? intxy_t min will be translated into null when sending as json. However we document that there is only at most one block of null values at the start of the array indicating a reboot
            } else {
                history_val = static_cast<METER_VALUE_HISTORY_VALUE_TYPE>(sum_this_interval / samples_this_interval);
            }

            history.push(history_val);
            history_last_update = millis();

            samples_last_interval = samples_this_interval;
            begin_last_interval = begin_this_interval;
            end_last_interval = end_this_interval;

            sum_this_interval = 0;
            samples_this_interval = 0;
            begin_this_interval = 0;
            end_this_interval = 0;

#if MODULE_WS_AVAILABLE()
            char *buf;
            int buf_written;

            if (history_val == val_min) {
                buf_written = asprintf(&buf, "{\"topic\":\"%s/history_samples\",\"payload\":{\"samples\":[null]}}\n", base_url.c_str());
            } else {
                buf_written = asprintf(&buf, "{\"topic\":\"%s/history_samples\",\"payload\":{\"samples\":[%d]}}\n", base_url.c_str(), static_cast<int>(history_val));
            }

            if (buf_written > 0) {
                ws.web_sockets.sendToAllOwned(buf, static_cast<size_t>(buf_written));
            }
#endif
        }, 0, 1000 * 60 * HISTORY_MINUTE_INTERVAL);
    }
}

size_t ValueHistory::format_live(char *buf, size_t buf_size)
{
    size_t buf_written = 0;
    uint32_t offset = millis() - live_last_update;
    METER_VALUE_HISTORY_VALUE_TYPE val;

    if (!live.peek(&val)) {
        buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "{\"offset\":%u,\"samples_per_second\":%f,\"samples\":[]}", offset, static_cast<double>(samples_per_second()));
    }
    else {
        buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "{\"offset\":%u,\"samples_per_second\":%f,\"samples\":[%d", offset, static_cast<double>(samples_per_second()), static_cast<int>(val));

        // This would underflow if live is empty, but it's guaranteed to have at least one entry by the peek() check above.
        size_t last_sample = live.used() - 1;
        for (size_t i = 1; i < last_sample && live.peek_offset(&val, i) && buf_written < buf_size; ++i) {
            buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, ",%d", static_cast<int>(val));
        }

        if (buf_written < buf_size) {
            buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "%s", "]}");
        }
    }

    return buf_written;
}

size_t ValueHistory::format_history(char *buf, size_t buf_size)
{
    METER_VALUE_HISTORY_VALUE_TYPE val_min = std::numeric_limits<METER_VALUE_HISTORY_VALUE_TYPE>::lowest();
    size_t buf_written = 0;
    uint32_t offset = millis() - history_last_update;
    METER_VALUE_HISTORY_VALUE_TYPE val;

    if (!history.peek(&val)) {
        buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "{\"offset\":%u,\"samples\":[]}", offset);
    }
    else {
        // intxy_t min values are prefilled, because the ESP was booted less than 48 hours ago.
        if (val == val_min) {
            buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "{\"offset\":%u,\"samples\":[%s", offset, "null");
        } else {
            buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "{\"offset\":%u,\"samples\":[%d", offset, static_cast<int>(val));
        }

        for (size_t i = 1; i < history.used() && history.peek_offset(&val, i) && buf_written < buf_size; ++i) {
            // intxy_t min values are prefilled, because the ESP was booted less than 48 hours ago.
            if (val == val_min) {
                buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "%s", ",null");
            } else {
                buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, ",%d", static_cast<int>(val));
            }
        }

        if (buf_written < buf_size) {
            buf_written += snprintf_u(buf + buf_written, buf_size - buf_written, "%s", "]}");
        }
    }

    return buf_written;
}

float ValueHistory::samples_per_second()
{
    float samples_per_second = 0;

    // Only calculate samples_per_second based on the last interval
    // if we have seen at least 2 values. With the API meter module,
    // it can happen that we see exactly one value in the first interval.
    // In this case 0 samples_per_second is reported for the next
    // interval (i.e. four minutes).
    if (this->samples_last_interval > 1) {
        uint32_t duration = end_last_interval - begin_last_interval;

        if (duration > 0) {
            // (samples_last_interval - 1) because there are N samples but only (N - 1) gaps
            // between them covering (end_last_interval - begin_last_interval) milliseconds
            samples_per_second = static_cast<float>((this->samples_last_interval - 1) * 1000) / static_cast<float>(duration);
        }
    }
    // Checking only for > 0 in this branch is fine: If we have seen
    // 0 or 1 samples in the last interval and exactly 1 in this interval,
    // we can only report that samples_per_second is 0.
    // This fixes itself when the next sample arrives.
    else if (this->samples_this_interval > 0) {
        uint32_t duration = end_this_interval - begin_this_interval;

        if (duration > 0) {
            // (samples_this_interval - 1) because there are N samples but only (N - 1) gaps
            // between them covering (end_this_interval - begin_this_interval) milliseconds
            samples_per_second = static_cast<float>((this->samples_this_interval - 1) * 1000) / static_cast<float>(duration);
        }
    }

    return samples_per_second;
}
