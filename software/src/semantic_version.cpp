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

// <major:int>.<minor:int>.<patch:int>[-beta.<beta:int>][+<timestamp:hex>]
bool SemanticVersion::from_string(const char *buf, Format format)
{
    const char *p = buf;
    char *end;
    char timestamp_marker = format == Format::WithTimestamp ? '+' : '\0';

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

    if (p == end || (strncmp(end, "-beta.", 6) != 0 && *end != timestamp_marker) || patch_candidate > 254) {
        return false;
    }

    // beta
    uint32_t beta_candidate = 255;

    if (*end != timestamp_marker) {
        p = end + 6; // skip "-beta."
        beta_candidate = strtoul(p, &end, 10);

        if (p == end || *end != timestamp_marker || beta_candidate > 254) {
            return false;
        }
    }

    // timestamp
    uint32_t timestamp_candidate = UINT32_MAX;

    if (format == Format::WithTimestamp) {
        p = end + 1; // skip plus
        timestamp_candidate = strtoul(p, &end, 16);

        if (p == end || *end != '\0') {
            return false;
        }
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
    if (timestamp != UINT32_MAX) {
        if (beta != 255) {
            return snprintf(buf, len, "%u.%u.%u-beta.%u+%lx", major, minor, patch, beta, timestamp);
        }
        else {
            return snprintf(buf, len, "%u.%u.%u+%lx", major, minor, patch, timestamp);
        }
    }
    else if (beta != 255) {
        return snprintf(buf, len, "%u.%u.%u-beta.%u", major, minor, patch, beta);
    }
    else {
        return snprintf(buf, len, "%u.%u.%u", major, minor, patch);
    }
}

int SemanticVersion::compare(const SemanticVersion &other) const
{
    if (major > other.major) {
        return 1;
    }

    if (major < other.major) {
        return -1;
    }

    if (minor > other.minor) {
        return 1;
    }

    if (minor < other.minor) {
        return -1;
    }

    if (patch > other.patch) {
        return 1;
    }

    if (patch < other.patch) {
        return -1;
    }

    if (beta == 255 && other.beta != 255) {
        return 1;
    }

    if (beta != 255 && other.beta == 255) {
        return -1;
    }

    if (beta > other.beta) {
        return 1;
    }

    if (beta < other.beta) {
        return -1;
    }

    // FIXME: what should be the ordering of a version with a
    //        timestamp to a version without a timestamp?
    if (timestamp == UINT32_MAX && other.timestamp != UINT32_MAX) {
        return 1;
    }

    if (timestamp != UINT32_MAX && other.timestamp == UINT32_MAX) {
        return -1;
    }

    if (timestamp > other.timestamp) {
        return 1;
    }

    if (timestamp < other.timestamp) {
        return -1;
    }

    return 0;
}
