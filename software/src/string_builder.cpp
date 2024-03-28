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

#include "string_builder.h"

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

char *StringWriter::empty = const_cast<char *>("");

StringWriter::StringWriter(char *buffer, size_t buffer_len) : capacity(buffer_len - 1), buffer(buffer)
{
    if (buffer_len < 1) {
        esp_system_abort("StringWriter: Buffer too short");
    }

    if (buffer != empty) {
        buffer[capacity] = '\0';
        buffer[length] = '\0';
    }
}

void StringWriter::setLength(size_t new_length)
{
    if (new_length > capacity) {
        new_length = capacity;
    }

    length = new_length;
    buffer[length] = '\0';
}

ssize_t StringWriter::puts(const char *string, ssize_t string_len)
{
    ssize_t remaining = getRemainingLength();

    if (remaining <= 0) {
        return 0;
    }

    if (string_len < 0) {
        string_len = strlen(string);
    }

    if (string_len > remaining) {
        string_len = remaining;
    }

    memcpy(buffer + length, string, string_len);

    length += string_len;

    buffer[length] = '\0';

    return string_len;
}

ssize_t StringWriter::putc(char c)
{
    if (getRemainingLength() <= 0) {
        return 0;
    }

    buffer[length] = c;
    length += 1;
    buffer[length] = '\0';

    return 1;
}

ssize_t StringWriter::vprintf(const char *fmt, va_list args)
{
    ssize_t remaining = getRemainingLength();

    if (remaining <= 0) {
        return 0;
    }

    ssize_t written = vsnprintf(buffer + length, remaining + 1 /* +1 for NUL-terminator */, fmt, args);

    if (written < 0) {
        return -1;
    }

    if (written > remaining) {
        written = remaining;
    }

    length += written;

    return written;
}

ssize_t StringWriter::printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    ssize_t written = vprintf(fmt, args);

    va_end(args);

    return written;
}


StringBuilder::StringBuilder() : StringWriter(empty, 1) {}

StringBuilder::~StringBuilder()
{
    setCapacity(0);
}

bool StringBuilder::setCapacity(size_t new_capacity)
{
    if (capacity == 0 && new_capacity > 0) {
        char *new_buffer = static_cast<char *>(malloc(new_capacity + 1)); // +1 for NUL-terminator

        if (new_buffer == nullptr) {
            return false;
        }

        new_buffer[0] = '\0';
        new_buffer[new_capacity] = '\0';

        capacity = new_capacity;
        length = 0;
        buffer = new_buffer;
    }
    else if (capacity > 0 && new_capacity == 0) {
        free(buffer);

        capacity = 0;
        length = 0;
        buffer = empty;
    }
    else if (capacity != new_capacity) {
        char *new_buffer = static_cast<char *>(realloc(buffer, new_capacity + 1)); // +1 for NUL-terminator

        if (new_buffer == nullptr) {
            return false;
        }

        new_buffer[new_capacity] = '\0';

        if (length > new_capacity) {
            length = new_capacity;
        }

        capacity = new_capacity;
        buffer = new_buffer;
    }

    return true;
}

std::unique_ptr<char> StringBuilder::take()
{
    char *tmp = buffer;

    if (tmp == empty) {
        tmp = strdup("");

        if (tmp == nullptr) {
            esp_system_abort("StringBuilder: Cannot allocate 1 byte");
        }
    }

    capacity = 0;
    length = 0;
    buffer = empty;

    return std::unique_ptr<char>{tmp};
}
