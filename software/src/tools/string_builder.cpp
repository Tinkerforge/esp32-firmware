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

#include <esp_system.h>
#include <stdio.h>
#include <string.h>

#include "gcc_warnings.h"

char *StringWriter::empty = const_cast<char *>("");

StringWriter::StringWriter(char *buffer_, size_t buffer_len) : capacity(buffer_len - 1), buffer(buffer_)
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

size_t StringWriter::puts(const char *string, ssize_t string_len_opt)
{
    size_t remaining = getRemainingLength();

    if (remaining == 0) {
        return 0;
    }

    size_t string_len;

    if (string_len_opt < 0) {
        string_len = strlen(string);
    }
    else {
        string_len = static_cast<size_t>(string_len_opt);
    }

    if (string_len > remaining) {
        string_len = remaining;
    }

    memcpy(buffer + length, string, string_len);

    length += string_len;

    buffer[length] = '\0';

    return string_len;
}

size_t StringWriter::putc(char c)
{
    if (getRemainingLength() == 0) {
        return 0;
    }

    buffer[length] = c;
    length += 1;
    buffer[length] = '\0';

    return 1;
}

size_t StringWriter::putcn(char c, size_t n)
{
    size_t remaining = getRemainingLength();

    if (remaining == 0) {
        return 0;
    }

    if (n > remaining) {
        n = remaining;
    }

    memset(buffer + length, c, n);
    length += n;

    buffer[length] = '\0';

    return n;
}

size_t StringWriter::vprintf(const char *fmt, va_list args)
{
    size_t remaining = getRemainingLength();

    if (remaining == 0) {
        return 0;
    }

    ssize_t written_or_error = vsnprintf(buffer + length, remaining + 1 /* +1 for NUL-terminator */, fmt, args);

    if (written_or_error < 0) {
        buffer[length] = '\0'; // undo whatever vsnprintf might have done

        return 0;
    }

    size_t written = static_cast<size_t>(written_or_error);

    if (written > remaining) {
        written = remaining;
    }

    length += written;

    return written;
}

size_t StringWriter::printf(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    size_t written = vprintf(fmt, args);

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

char *StringBuilder::take()
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

    return tmp;
}
