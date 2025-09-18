/* esp32-firmware
 * Copyright (C) 2025 Erik Fleckstein <erik@tinkerforge.com>
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

#include <TFTools/Micros.h>

// If a firmware update results in the web server not starting anymore,
// we should roll back to a working firmware.
// The previous firmware worked well enough that it was possible to flash a firmware.
// Don't update info/last_boots before we know the web server works,
// but update it before setting the firmware to good,
// so that those processes don't race against each other.

constexpr auto WEB_SERVER_DEAD_TIMEOUT = 5_min;
constexpr auto UPDATE_LAST_BOOTS_DELAY = 6_min;
constexpr auto VALIDATE_FIRMWARE_DELAY = 7_min;
