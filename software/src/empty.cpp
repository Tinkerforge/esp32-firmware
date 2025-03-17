/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include <Arduino.h>
#include <stdint.h>

typedef struct {
    uint32_t magic; // CUSTOM_APP_DESC_MAGIC
    uint8_t version; // BUILD_CUSTOM_APP_DESC_VERSION
    uint8_t padding[3];
    uint8_t fw_version[4]; // major, minor, patch, beta
    uint32_t fw_build_time;
} build_custom_app_desc_t;

static_assert(sizeof(build_custom_app_desc_t) == 16, "build_custom_app_desc_t has wrong size");

extern const __attribute__((section(".rodata_custom_desc"))) build_custom_app_desc_t build_custom_app_desc = {0, 0, {0, 0, 0}, {0, 0, 0, 0}, 0};

void setup()
{
}

void loop()
{
}
