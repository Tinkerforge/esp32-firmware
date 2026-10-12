/* esp32-firmware
 * Copyright (C) 2026 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <esp_err.h>
#include <esp_partition.h>

enum class PartitionEraseType {
    Beginning,
    All,
    InUse,
};

esp_err_t erase_partition_with_delay(const esp_partition_t *partition, PartitionEraseType erase_type, size_t *erased_size_out = nullptr);
esp_err_t erase_partition_with_delay(const char *partition_label, PartitionEraseType erase_type, size_t *erased_size_out = nullptr);
