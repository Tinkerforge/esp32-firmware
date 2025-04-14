/* esp32-firmware
 * Copyright (C) 2025 Matthias Bolte <matthias@tinkerforge.com>
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

#include <stdlib.h>
#include <type_traits>

enum class HexdumpCase
{
    Lower,
    Upper,
};

template <typename T>
size_t hexdump(const T *data, size_t data_len, char *buf, size_t buf_len, HexdumpCase case_, char separator = '\0')
{
    static_assert(std::is_arithmetic<T>::value, "data has to be an arithmetic type");

    static const char *alphabet_lower = "0123456789abcdef";
    static const char *alphabet_upper = "0123456789ABCDEF";

    const char *alphabet = case_ == HexdumpCase::Lower ? alphabet_lower : alphabet_upper;

    if (buf_len < 1) {
        return 0;
    }

    size_t nibble_count = sizeof(T) * 2;
    size_t buf_i = 0;

    for (size_t data_i = 0; data_i < data_len && buf_i + 1 < buf_len; ++data_i) {
        for (size_t nibble_i = 0; nibble_i < nibble_count && buf_i + 1 < buf_len; ++nibble_i, ++buf_i) {
            buf[buf_i] = alphabet[(data[data_i] >> ((nibble_count * 4 - 4) - 4 * nibble_i)) & 0x0f];
        }

        if (separator != '\0' && data_i + 1 < data_len && buf_i + 1 < buf_len) {
            buf[buf_i] = separator;
            ++buf_i;
        }
    }

    buf[buf_i] = '\0';

    return buf_i;
}

template <typename T>
ssize_t hexload(const char *buf, size_t buf_len, T *data, size_t data_len, char separator = '\0')
{
    static_assert(std::is_arithmetic<T>::value, "data has to be an arithmetic type");

    if (data_len < 1) {
        return 0;
    }

    size_t nibble_count = sizeof(T) * 2;

    if (((separator != '\0' ? buf_len + 1 : buf_len) % (separator != '\0' ? nibble_count + 1 : nibble_count)) != 0) {
        return -1;
    }

    size_t data_i = 0;

    for (size_t buf_i = 0; buf_i + nibble_count <= buf_len && data_i < data_len; ++data_i) {
        data[data_i] = 0;

        for (size_t nibble_i = 0; nibble_i < nibble_count; ++nibble_i, ++buf_i) {
            char nibble = buf[buf_i];
            char base;
            T offset;

            if (nibble >= '0' && nibble <= '9') {
                base = '0';
                offset = 0;
            }
            else if (nibble >= 'a' && nibble <= 'f') {
                base = 'a';
                offset = 10;
            }
            else if (nibble >= 'A' && nibble <= 'F') {
                base = 'A';
                offset = 10;
            }
            else {
                return -1;
            }

            data[data_i] |= (static_cast<T>(nibble - base) + offset) << ((nibble_count * 4 - 4) - 4 * nibble_i);
        }

        if (separator != '\0' && buf_i < buf_len) {
            if (buf[buf_i] != separator) {
                return -1;
            }

            ++buf_i;
        }
    }

    return data_i;
}
