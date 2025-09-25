/* esp32-firmware
 * Copyright (C) 2025 Frederic Henrichs <frederic@tinkerforge.com>
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
#include <cstdint>
#include <Arduino.h>

// Needed for DISPLAY_NAME_LENGTH definition
#include "../users/users.h"

#define USER_FILTER_ALL_USERS -2
#define USER_FILTER_DELETED_USERS -1

#define CHARGE_RECORD_FOLDER "/charge-records"

struct [[gnu::packed]] ChargeStart {
    uint32_t timestamp_minutes = 0;
    float meter_start = 0.0f;
    uint8_t user_id = 0;
};

static_assert(sizeof(ChargeStart) == 9, "Unexpected size of ChargeStart");

struct [[gnu::packed]] ChargeEnd {
    uint32_t charge_duration : 24;
    float meter_end = 0.0f;
};

static_assert(sizeof(ChargeEnd) == 7, "Unexpected size of ChargeEnd");

struct [[gnu::packed]] Charge {
    ChargeStart cs;
    ChargeEnd ce;
};

struct display_name_entry {
    uint32_t length;
    uint32_t name[DISPLAY_NAME_LENGTH / sizeof(uint32_t)];
};

size_t get_display_name(uint8_t user_id, char *ret_buf, display_name_entry *display_name_cache);
String chargeRecordFilename(uint32_t i);
