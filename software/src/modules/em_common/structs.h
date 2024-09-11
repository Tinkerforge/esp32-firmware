
/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <stdint.h>

typedef struct {
    uint32_t last_update;
    bool is_valid;

    float power;
    float current[3];

    uint32_t uptime;
    uint16_t voltage;

    uint8_t energy_meter_type;
    uint32_t error_count[6];
} EMAllDataCommon;

struct sdcard_info {
    uint32_t sd_status;
    uint32_t lfs_status;
    uint32_t card_type;
    uint32_t sector_count;
    uint16_t sector_size;
    uint8_t  manufacturer_id;
    uint8_t  product_rev;
    char     product_name[6];
};
