/* esp32-firmware
 * Copyright (C) 2024 Erik Fleckstein <erik@tinkerforge.com>
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

#include "header_logger.h"

#include "event_log_prefix.h"
#include "main_dependencies.h"

int header_printfln(const char *prefix, const char *fmt, va_list args)
{
    return logger.printfln_prefixed(prefix, strlen(prefix), fmt, args);
}

int header_printfln(const char *prefix, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int result = header_printfln(prefix, fmt, args);
    va_end(args);

    return result;
}
