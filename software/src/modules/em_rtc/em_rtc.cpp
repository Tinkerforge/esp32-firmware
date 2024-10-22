/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "em_rtc.h"

#include "module_dependencies.h"

void EmRtc::setup()
{
    if (!em_common.initialized)
        return;

    rtc.register_backend(this);

    initialized = true;
}

void EmRtc::set_time(const tm &tm, int32_t microseconds)
{
    // TODO: can we do something with the microseconds here?
    em_common.set_time(tm);
}

struct timeval EmRtc::get_time()
{
    return em_common.get_time();
}

void EmRtc::reset()
{
}
