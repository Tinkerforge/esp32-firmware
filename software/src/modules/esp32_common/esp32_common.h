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

#include <stdint.h>
#include <WString.h>

#include <sodium.h>

#include "config.h"
#include "module.h"

class ESP32Common final : public IModule
{
    friend class ESP32CommonEncryption;
    friend class ESP32CommonSecureBoot;

public:
    ESP32Common(){}

    void pre_init() override;
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    [[gnu::const]] inline uint32_t get_uid_num() {return uid_num;};
    [[gnu::const]] const char *get_uid_cstr() {return uid_str->c_str();};
    [[gnu::const]] inline const String &get_uid_str() {return *uid_str;};
    [[gnu::const]] String get_base58_uid_str();
    [[gnu::const]] String get_default_name(char separator = '-');
    [[gnu::const]] String get_default_wifi_passphrase();

    bool initHAL();
    bool destroyHAL();

private:
    bool create_complete_master_key();
    bool create_basic_master_key();
    bool fill_missing_master_key();
    bool get_subkey(uint8_t *subkey, size_t subkey_len, uint64_t subkey_id, const char subkey_context[crypto_kdf_blake2b_CONTEXTBYTES]);
    bool get_wifi_passphrase_blocks_derived(uint32_t passphrase_blocks[4]);

    uint32_t uid_num = 0;
    String *uid_str = nullptr;
};
