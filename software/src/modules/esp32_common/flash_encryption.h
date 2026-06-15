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

#include <WString.h>

#include <esp_partition.h>

class ESP32CommonEncryption final
{
    friend class ESP32Common;

private:
    static bool check_and_set_encryption_key(bool deterministic);
    static bool erase_core_dump(String *errmsg);
    static bool encrypt_app(String *errmsg, const esp_partition_t *running_partition, uint32_t running_app_number);
    static bool encrypt_bootloader_partition_table(const esp_partition_t *running_partition);
    static bool burn_secure_encryption_fuses_restart();
    static void encrypt_app_bootloader_partition_table(String *errmsg, bool deterministic);
    static bool mark_and_erase_data_partition();

    static bool is_data_partition_encrypted();
};
