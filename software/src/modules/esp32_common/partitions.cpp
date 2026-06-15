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

#include <stddef.h>
#include <stdint.h>

#include <esp_flash_partitions.h>
#include <esp_partition.h>
#include <esp_rom_md5.h>

#include "generated/module_dependencies.h"
#include "partitions.h"

#include "event_log_prefix.h"

#include "gcc_warnings.h"

static_assert(alignof(esp_partition_info_t) <= 4);
#pragma GCC diagnostic ignored "-Wcast-align"

size_t ESP32CommonPartitions::get_pt_size(const uint8_t *pt_data)
{
    int num_partitions = 0;
    esp_err_t err = esp_partition_table_verify(reinterpret_cast<const esp_partition_info_t *>(pt_data), true, &num_partitions);

    if (err != ESP_OK || num_partitions < 0) {
        logger.printfln("Invalid partition table (%i)", num_partitions);
        return 0;
    }

    return static_cast<size_t>((num_partitions + 2)) * sizeof(esp_partition_info_t); // +2 for MD5 entry and all-blank end entry
}

bool ESP32CommonPartitions::mark_encrypted(uint8_t *pt_data, const char *label)
{
    esp_partition_info_t *partitions = reinterpret_cast<esp_partition_info_t *>(pt_data);

    for (size_t part_num = 0; part_num < ESP_PARTITION_TABLE_MAX_ENTRIES; part_num++) {
        esp_partition_info_t *part = partitions + part_num;

        if (part->type == PART_TYPE_END) {
            break;
        }

        if (part->magic == ESP_PARTITION_MAGIC && strncmp(reinterpret_cast<const char *>(part->label), label, std::size(part->label) - 1) == 0) {
            part->flags |= PART_FLAG_ENCRYPTED;
            return true;
        };
    }

    logger.printfln("Partition '%s' not found", label);
    return false;
}

bool ESP32CommonPartitions::update_md5(uint8_t *pt_data)
{
    md5_context_t context;
    esp_rom_md5_init(&context);

    esp_partition_info_t *partitions = reinterpret_cast<esp_partition_info_t *>(pt_data);

    for (size_t part_num = 0; part_num < ESP_PARTITION_TABLE_MAX_ENTRIES; part_num++) {
        esp_partition_info_t *part = partitions + part_num;

        if (part->magic == 0xFFFF && part->type == PART_TYPE_END) {
            break;
        }

        if (part->magic == ESP_PARTITION_MAGIC_MD5) {
            uint8_t *md5_data = reinterpret_cast<uint8_t *>(part) + ESP_PARTITION_MD5_OFFSET;

            esp_rom_md5_final(md5_data, &context);
            return true;
        };

        esp_rom_md5_update(&context, part, sizeof(*part));
    }

    return false;
}
