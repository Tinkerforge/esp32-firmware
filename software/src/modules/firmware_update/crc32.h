/* bricklib2
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * crc32.h: Implementation of CRC32 checksum calculation
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

#ifndef CRC32_H
#define CRC32_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void crc32_ieee_802_3_recalculate(const void *data, size_t length, uint32_t *crc);
uint32_t crc32_ieee_802_3(const void *data, size_t length);

#endif