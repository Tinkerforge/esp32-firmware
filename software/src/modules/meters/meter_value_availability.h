/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

enum class MeterValueAvailability {
    Fresh,              // Meter declared requested value ID and value is fresh.
    Stale,              // Meter declared requested value ID and value is stale or not yet set.
    Unavailable,        // (a) Meter declared its value IDs but requisted value ID was not among them. (b) Meter hasn't declared its values and meter config can't provide this value ID.
    CurrentlyUnknown,   // Meter hasn't declared its value IDs yet and the meter config doesn't know.
};
