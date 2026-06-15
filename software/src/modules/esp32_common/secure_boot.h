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

#include <esp_secure_boot.h>
#include <mbedtls/pk.h>

class ESP32CommonSecureBoot final
{
    friend class ESP32Common;
    friend class ESP32CommonEncryption;
    friend class FirmwareUpdate;

private:
    static bool load_cached_secure_boot_v2_key(mbedtls_pk_context *rsa_key, mbedtls_f_rng_t *f_rng, void *p_rng, uint8_t *sb_key);
    static bool cache_secure_boot_v2_key(const mbedtls_pk_context *rsa_key, uint8_t *sb_key);
    static bool generate_secure_boot_v2_key(mbedtls_pk_context *rsa_key);
    static bool generate_and_cache_secure_boot_v2_key(mbedtls_pk_context *rsa_key, uint8_t *sb_key);
    static bool load_or_generate_secure_boot_v2_key(mbedtls_pk_context *rsa_key, mbedtls_f_rng_t *f_rng, void *p_rng, uint8_t *sb_key);
    static bool check_secure_boot_v2_key();
    static bool get_secure_boot_key(uint8_t key_digest[ESP_SECURE_BOOT_DIGEST_LEN]);

    static bool sb_sign_image(ets_secure_boot_sig_block_t *sig_block, const uint8_t image_digest[ESP_SECURE_BOOT_DIGEST_LEN]);
    static bool sb_sign_image_or_skip(ets_secure_boot_sig_block_t *sig_block, const uint8_t image_digest[ESP_SECURE_BOOT_DIGEST_LEN]);
    static bool set_secure_boot_key();
    static bool set_secure_boot_fuse();

    static bool lockdown();

    static bool is_secure_boot_enabled();
    static bool is_locked_down();
};
