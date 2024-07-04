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
#include "tools.h"
#include "string_builder.h"

#include "gcc_warnings.h"
#ifdef __GNUC__
// The code is this file contains several casts to a type defined by a macro, which may result in useless casts.
#pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

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

    for (size_t i = 0; i < live.size(); ++i) {
        //float f = 5000.0 * sin(PI/120.0 * i) + 5000.0;
        // Use negative state to mark that these are pre-filled.
        live.push(val_min);
    }

    chars_per_value = max(String(METER_VALUE_HISTORY_VALUE_MIN).length(), String(METER_VALUE_HISTORY_VALUE_MAX).length());
    // val_min values are replaced with null -> require at least 4 chars per value.
    chars_per_value = max(4U, chars_per_value);
    // For ',' between the values.
    ++chars_per_value;
}

void ValueHistory::register_urls(String base_url)
{
    server.on(("/" + base_url + "history").c_str(), HTTP_GET, [this](WebServerRequest request) {
        StringBuilder sb;

        if (!sb.setCapacity(HISTORY_RING_BUF_SIZE * chars_per_value + 100)) {
            return request.send(500, "text/plain", "Failed to allocate buffer");
        }

        format_history(millis(), &sb);

        return request.send(200, "application/json; charset=utf-8", sb.getPtr(), static_cast<ssize_t>(sb.getLength()));
    });

    server.on(("/" + base_url + "live").c_str(), HTTP_GET, [this](WebServerRequest request) {
        StringBuilder sb;

        if (!sb.setCapacity(HISTORY_RING_BUF_SIZE * chars_per_value + 100)) {
            return request.send(500, "text/plain", "Failed to allocate buffer");
        }

        format_live(millis(), &sb);

        return request.send(200, "application/json; charset=utf-8", sb.getPtr(), static_cast<ssize_t>(sb.getLength()));
    });
}

void ValueHistory::register_urls_empty(String base_url)
{
    const char *empty_history = "{\"offset\":0,\"samples\":[]}";
    ssize_t empty_history_len = static_cast<ssize_t>(strlen(empty_history));
    server.on(("/" + base_url + "history").c_str(), HTTP_GET, [this, empty_history, empty_history_len](WebServerRequest request) {
        return request.send(200, "application/json; charset=utf-8", empty_history, empty_history_len);
    });

    const char *empty_live = "{\"offset\":0,\"samples_per_second\":0.0,\"samples\":[]}";
    ssize_t empty_live_len = static_cast<ssize_t>(strlen(empty_live));
    server.on(("/" + base_url + "live").c_str(), HTTP_GET, [this, empty_live, empty_live_len](WebServerRequest request) {
        return request.send(200, "application/json; charset=utf-8", empty_live, empty_live_len);
    });
}

void ValueHistory::add_sample(float sample)
{
    ++sample_count;
    sample_sum += sample;
}

void ValueHistory::tick(uint32_t now, bool update_history, METER_VALUE_HISTORY_VALUE_TYPE *live_sample, METER_VALUE_HISTORY_VALUE_TYPE *history_sample)
{
    METER_VALUE_HISTORY_VALUE_TYPE val_min = std::numeric_limits<METER_VALUE_HISTORY_VALUE_TYPE>::lowest();
    METER_VALUE_HISTORY_VALUE_TYPE live_val = val_min;

    if (sample_count > 0) {
        live_val = clamp(static_cast<METER_VALUE_HISTORY_VALUE_TYPE>(METER_VALUE_HISTORY_VALUE_MIN),
                         static_cast<METER_VALUE_HISTORY_VALUE_TYPE>(roundf(sample_sum / static_cast<float>(sample_count))),
                         static_cast<METER_VALUE_HISTORY_VALUE_TYPE>(METER_VALUE_HISTORY_VALUE_MAX));

        sample_count = 0;
        sample_sum = 0;

        last_live_val = live_val;
        last_live_val_valid = 4;
    }
    else if (last_live_val_valid > 0) {
        // reuse last value up to 4 times (2.499 seconds) to avoid gaps due
        // to jitter and loss of samples to due our web server being busy
        // for a moment and not being able to accept new samples over HTTP
        live_val = last_live_val;
        --last_live_val_valid;
    }

    live.push(live_val);
    *live_sample = live_val;
    live_last_update = now;
    end_this_interval = live_last_update;

    if (all_samples_this_interval == 0) {
        begin_this_interval = live_last_update;
    }

    ++all_samples_this_interval;

    if (live_val != val_min) {
        ++valid_samples_this_interval;
        sum_this_interval += live_val;
    }

    *history_sample = val_min;

    if (update_history) {
        METER_VALUE_HISTORY_VALUE_TYPE history_val;

        if (valid_samples_this_interval == 0) {
            history_val = val_min; // TODO push 0 or intxy_t min here? intxy_t min will be translated into null when sending as json. However we document that there is only at most one block of null values at the start of the array indicating a reboot
        } else {
            history_val = static_cast<METER_VALUE_HISTORY_VALUE_TYPE>(sum_this_interval / valid_samples_this_interval);
        }

        history.push(history_val);
        *history_sample = history_val;
        history_last_update = now;

        samples_last_interval = all_samples_this_interval;
        begin_last_interval = begin_this_interval;
        end_last_interval = end_this_interval;

        sum_this_interval = 0;
        all_samples_this_interval = 0;
        valid_samples_this_interval = 0;
        begin_this_interval = 0;
        end_this_interval = 0;
    }
}

void ValueHistory::format_live(uint32_t now, StringBuilder *sb)
{
    sb->printf("{\"offset\":%u,\"samples_per_second\":%f,\"samples\":[", now - live_last_update, static_cast<double>(samples_per_second()));
    format_live_samples(sb);
    sb->puts("]}");
}

void ValueHistory::format_live_samples(StringBuilder *sb)
{
    METER_VALUE_HISTORY_VALUE_TYPE val_min = std::numeric_limits<METER_VALUE_HISTORY_VALUE_TYPE>::lowest();
    METER_VALUE_HISTORY_VALUE_TYPE val;

    if (live.peek(&val)) {
        if (val == val_min) {
            sb->puts("null");
        } else {
            sb->printf("%d", static_cast<int>(val));
        }

        size_t used = live.used();

        for (size_t i = 1; i < used && live.peek_offset(&val, i) && sb->getRemainingLength() > 0; ++i) {
            if (val == val_min) {
                sb->puts(",null");
            } else {
                sb->printf(",%d", static_cast<int>(val));
            }
        }
    }
}

void ValueHistory::format_history(uint32_t now, StringBuilder *sb)
{
    sb->printf("{\"offset\":%u,\"samples\":[", now - history_last_update);
    format_history_samples(sb);
    sb->puts("]}");
}

void ValueHistory::format_history_samples(StringBuilder *sb)
{
    METER_VALUE_HISTORY_VALUE_TYPE val_min = std::numeric_limits<METER_VALUE_HISTORY_VALUE_TYPE>::lowest();
    METER_VALUE_HISTORY_VALUE_TYPE val;

    if (history.peek(&val)) {
        if (val == val_min) {
            sb->puts("null");
        } else {
            sb->printf("%d", static_cast<int>(val));
        }

        size_t used = history.used();

        for (size_t i = 1; i < used && history.peek_offset(&val, i) && sb->getRemainingLength() > 0; ++i) {
            if (val == val_min) {
                sb->puts(",null");
            } else {
                sb->printf(",%d", static_cast<int>(val));
            }
        }
    }
}

float ValueHistory::samples_per_second()
{
    float samples_per_second = 0;

    // Only calculate samples_per_second based on the last interval
    // if we have seen at least 2 values. With the API meter module,
    // it can happen that we see exactly one value in the first interval.
    // In this case 0 samples_per_second is reported for the next
    // interval (i.e. four minutes).
    if (samples_last_interval > 1) {
        uint32_t duration = end_last_interval - begin_last_interval;

        if (duration > 0) {
            // (samples_last_interval - 1) because there are N samples but only (N - 1) gaps
            // between them covering (end_last_interval - begin_last_interval) milliseconds
            samples_per_second = static_cast<float>((samples_last_interval - 1) * 1000) / static_cast<float>(duration);
        }
    }
    // Checking only for > 0 in this branch is fine: If we have seen
    // 0 or 1 samples in the last interval and exactly 1 in this interval,
    // we can only report that samples_per_second is 0.
    // This fixes itself when the next sample arrives.
    else if (all_samples_this_interval > 0) {
        uint32_t duration = end_this_interval - begin_this_interval;

        if (duration > 0) {
            // (all_samples_this_interval - 1) because there are N samples but only (N - 1) gaps
            // between them covering (end_this_interval - begin_this_interval) milliseconds
            samples_per_second = static_cast<float>((all_samples_this_interval - 1) * 1000) / static_cast<float>(duration);
        }
    }

    return samples_per_second;
}
