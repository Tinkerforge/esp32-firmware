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

#include <stddef.h>

class StringBuilder;

class IDebugProtocolBackend
{
public:
    virtual ~IDebugProtocolBackend() = default;

    [[gnu::const]] virtual size_t get_debug_header_length() const = 0;
    virtual void get_debug_header(StringBuilder *sb) = 0;
    [[gnu::const]] virtual size_t get_debug_line_length() const = 0;
    virtual void get_debug_line(StringBuilder *sb) = 0;
};
