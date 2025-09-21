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

#include "meter_value_id.h"

#include <cstddef>
#include <cstdint>

#include "meter_defs.h"

// If you change the METER_ALL_VALUES_RESETTABLE_MAX_COUNT,
// make sure that it is at least set to the max of all meter arrays in rs458_helpers.cpp
static_assert(METER_ALL_VALUES_RESETTABLE_MAX_COUNT == 88);
extern void rs485_helper_get_value_ids(uint32_t meter_type, const MeterValueID **value_ids, size_t *value_ids_len);
