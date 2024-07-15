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

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>
#include <esp_heap_caps.h>
#include <limits>

#include "ringbuffer.h"
#include "malloc_tools.h"

// How many hours to keep the coarse history for
#define HISTORY_HOURS 48

// How many minutes to keep the fine history for.
// This also controls the coarseness of the coarse history.
// For example 4 means that we accumulate 4 minutes of samples
// with the maximum rate i.e. ~ 3 samples per second (Querying the state
// takes about 380 ms).
// When we have 4 minutes worth of samples, we take the average
// and add it to the coarse history.
#define HISTORY_MINUTE_INTERVAL 4

#define HISTORY_RING_BUF_SIZE (HISTORY_HOURS * 60 / HISTORY_MINUTE_INTERVAL)

#ifndef METER_VALUE_HISTORY_VALUE_TYPE
#define METER_VALUE_HISTORY_VALUE_TYPE int16_t
#endif

#ifndef METER_VALUE_HISTORY_VALUE_MAX
#define METER_VALUE_HISTORY_VALUE_MAX 32767
#endif

#ifndef METER_VALUE_HISTORY_VALUE_MIN
#define METER_VALUE_HISTORY_VALUE_MIN -32767
#endif

// Check for < because ::lowest() is a reserved value.
static_assert(std::numeric_limits<METER_VALUE_HISTORY_VALUE_TYPE>::lowest() < METER_VALUE_HISTORY_VALUE_MIN);
static_assert(std::numeric_limits<METER_VALUE_HISTORY_VALUE_TYPE>::max() >= METER_VALUE_HISTORY_VALUE_MAX);

// We use int to format the buffer, so at most int is allowed.
static_assert(std::numeric_limits<int>::lowest() <= METER_VALUE_HISTORY_VALUE_MIN);
static_assert(std::numeric_limits<int>::max() >= METER_VALUE_HISTORY_VALUE_MAX);

class StringBuilder;

class ValueHistory
{
public:
    ValueHistory()
    {
    }

    void setup();
    void register_urls(String base_url);
    void register_urls_empty(String base_url);
    void add_sample(float sample);
    void tick(uint32_t now, bool update_history, METER_VALUE_HISTORY_VALUE_TYPE *live_sample, METER_VALUE_HISTORY_VALUE_TYPE *history_sample);
    void format_live(uint32_t now, StringBuilder *sb);
    void format_live_samples(StringBuilder *sb);
    void format_history(uint32_t now, StringBuilder *sb);
    void format_history_samples(StringBuilder *sb);
    float samples_per_second();

    int64_t sum_this_interval = 0;
    int all_samples_this_interval = 0;
    int valid_samples_this_interval = 0;
    uint32_t begin_this_interval = 0;
    uint32_t end_this_interval = 0;

    int samples_last_interval = 0;
    uint32_t begin_last_interval = 0;
    uint32_t end_last_interval = 0;

    uint32_t sample_count = 0;
    float sample_sum = 0;

    METER_VALUE_HISTORY_VALUE_TYPE last_live_val;
    int last_live_val_valid = 0;

    TF_PackedRingbuffer<METER_VALUE_HISTORY_VALUE_TYPE,
                  3 * 60 * HISTORY_MINUTE_INTERVAL,
                  uint32_t,
#if defined(BOARD_HAS_PSRAM)
                  malloc_psram,
#else
                  malloc_32bit_addressed,
#endif
                  heap_caps_free> live;
    uint32_t live_last_update = 0;

    TF_PackedRingbuffer<METER_VALUE_HISTORY_VALUE_TYPE,
                  HISTORY_RING_BUF_SIZE,
                  uint32_t,
#if defined(BOARD_HAS_PSRAM)
                  malloc_psram,
#else
                  malloc_32bit_addressed,
#endif
                  heap_caps_free> history;
    uint32_t history_last_update = 0;

    size_t chars_per_value = -1;
};
