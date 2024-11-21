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

#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <memory>

class StringWriter
{
public:
    StringWriter(char *buffer, size_t buffer_len);
    StringWriter(const StringWriter &other) = delete;
    const StringWriter &operator=(const StringWriter &other) = delete;

    virtual ~StringWriter() {}

    size_t getCapacity() const { return capacity; }
    void setLength(size_t new_length);
    size_t getLength() const { return length; }
    void clear() { setLength(0); }
    size_t getRemainingLength() const { return capacity - length; }
    char *getPtr() const { return buffer; }
    char *getRemainingPtr() const { return buffer + length; }
    ssize_t puts(const char *string, ssize_t string_len = -1);
    ssize_t putc(char c);
    ssize_t vprintf(const char *fmt, va_list args);
    [[gnu::format(__printf__, 2, 3)]] ssize_t printf(const char *fmt, ...);

protected:
    size_t capacity; // excluding NUL-terminator
    size_t length = 0; // excluding NUL-terminator
    char *buffer;

    static char *empty;
};

class StringBuilder : public StringWriter
{
public:
    StringBuilder();
    virtual ~StringBuilder();

    bool setCapacity(size_t capacity);
    bool shrink() { return setCapacity(getLength()); }
    std::unique_ptr<char> take();
};
