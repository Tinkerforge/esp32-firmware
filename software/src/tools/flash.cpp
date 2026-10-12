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

#include <algorithm>
#include <bits/range_access.h>
#include <stdint.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <xtensa_context.h>

#include "event_log_prefix.h"
#include "flash.h"
#include "main_dependencies.h"

#include "gcc_warnings.h"

[[gnu::noinline]]
static uint32_t find_used_size(const esp_partition_t *partition)
{
    uint32_t buf[2048 / sizeof(uint32_t)];
    const uint32_t partition_size = partition->size;

    if (partition_size % sizeof(buf) != 0) {
        logger.printfln("Partition size not a multiple of the buffer size: %lu", partition_size);
        return partition_size;
    }

    uint32_t offset = partition_size;

    while (offset > 0) {
        offset -= sizeof(buf);

        esp_err_t err = esp_partition_read_raw(partition, offset, buf, sizeof(buf));

        if (err != ESP_OK) {
            logger.printfln("Partition read failed @ %lu: 0x%x", offset, static_cast<unsigned>(err));
            return partition_size;
        }

        for (size_t i = 0; i < std::size(buf); i++) {
            if (buf[i] == 0xFFFFFFFFUL) {
                continue;
            }

            // Block not empty
            return ALIGNUP(64UL * 1024UL, offset + sizeof(buf) - 1);
        }
    }

    return 0;
}

esp_err_t erase_partition_with_delay(const esp_partition_t *partition, PartitionEraseType erase_type, size_t *erased_size_out)
{
    uint32_t to_erase;
    esp_err_t err;

    if (erase_type == PartitionEraseType::Beginning) {
        // Erase only minimum erase size
        to_erase = partition->erase_size;
    } else if (erase_type == PartitionEraseType::InUse) {
        to_erase = find_used_size(partition);

        if (to_erase == 0) {
            if (erased_size_out != nullptr) {
                *erased_size_out = 0;
            }
            return ESP_ERR_INVALID_STATE;
        }
    } else {
        // Erase everything
        to_erase = partition->size;
    }

    uint32_t offset = 0;

    while (offset < to_erase) {
        constexpr uint32_t max_block_size = 256UL * 1024UL;
        const uint32_t block_size = std::min(to_erase - offset, max_block_size);

        err = esp_partition_erase_range(partition, offset, block_size);

        if (err != ESP_OK) {
            break;
        }

        if (block_size == max_block_size) {
            vTaskDelay(4);
        }

        offset += block_size;
    }

    if (erased_size_out != nullptr) {
        *erased_size_out = to_erase;
    }

    return ESP_OK;
}

esp_err_t erase_partition_with_delay(const char *partition_label, PartitionEraseType erase_type, size_t *erased_size_out)
{
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, partition_label);

    if (partition == nullptr) {
        return ESP_ERR_NOT_FOUND;
    }

    return erase_partition_with_delay(partition, erase_type, erased_size_out);
}
