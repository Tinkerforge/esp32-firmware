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

#include "semantic_version.h"

// <major:int>.<minor:int>.<patch:int>[-beta.<beta:int>]+<timestamp:hex>
bool SemanticVersion::from_string(const char *buf)
{
    const char *p = buf;
    char *end;

    // major
    uint32_t major_candidate = strtoul(p, &end, 10);

    if (p == end || *end != '.' || major_candidate > 254) {
        return false;
    }

    p = end + 1; // skip dot

    // minor
    uint32_t minor_candidate = strtoul(p, &end, 10);

    if (p == end || *end != '.' || minor_candidate > 254) {
        return false;
    }

    p = end + 1; // skip dot

    // patch
    uint32_t patch_candidate = strtoul(p, &end, 10);

    if (p == end || (strncmp(end, "-beta.", 6) != 0 && *end != '+') || patch_candidate > 254) {
        return false;
    }

    // beta
    uint32_t beta_candidate = 255;

    if (*end != '+') {
        p = end + 6;
        beta_candidate = strtoul(p, &end, 10);

        if (p == end || *end != '+' || beta_candidate > 254) {
            return false;
        }
    }

    p = end + 1; // skip plus

    // timestamp
    uint32_t timestamp_candidate = strtoul(p, &end, 16);

    if (p == end || *end != '\0') {
        return false;
    }

    major = major_candidate;
    minor = minor_candidate;
    patch = patch_candidate;
    beta = beta_candidate;
    timestamp = timestamp_candidate;

    return true;
}

int SemanticVersion::to_string(char *buf, size_t len) const
{
    if (beta != 255) {
        return snprintf(buf, len, "%u.%u.%u-beta.%u+%x", major, minor, patch, beta, timestamp);
    }
    else {
        return snprintf(buf, len, "%u.%u.%u+%x", major, minor, patch, timestamp);
    }
}
