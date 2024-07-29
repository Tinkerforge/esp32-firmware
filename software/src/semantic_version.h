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

#pragma once

#include <Arduino.h>

#define SEMANTIC_VERSION_MAX_STRING_LENGTH (29 + 1) // strlen("MAJ.MIN.PAT-beta.BET+TIMESTAM") == 29 + 1 for NUL-terminator

struct SemanticVersion {
    uint8_t major = 255;
    uint8_t minor = 255;
    uint8_t patch = 255;
    uint8_t beta = 255;
    uint32_t timestamp = 0;

    bool from_string(const char *buf);
    int to_string(char *buf, size_t len) const;
};
