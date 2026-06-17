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

// Base libraries
#include <stdint.h>

// ESP IDF
#include <esp_flash.h>
#include <soc/efuse_struct.h>

// mbedTLS
#include <mbedtls/aes.h>
#include <mbedtls/bignum.h>

// This module
#include "./flash_decryption.h"
#include "./generated/module_dependencies.h"

// Other project things
#include "event_log_prefix.h"

#include "gcc_warnings.h"

union limbs_bytes {
    mbedtls_mpi_uint limbs[8];
    uint8_t bytes[32];
};

static const limbs_bytes mul1_data = {
    .bytes = {
        // BE
        // 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00, 0x00, 0x80, 0x00, 0x10,
        // 0x00, 0x00, 0x02, 0x00, 0x00, 0x40, 0x00, 0x08, 0x00, 0x00, 0x04, 0x00, 0x00, 0x80, 0x00, 0x10,
        // LE
        0x10, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00, 0x00, 0x08, 0x00, 0x40, 0x00, 0x00, 0x02, 0x00, 0x00,
        0x10, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00, 0x00, 0x80, 0x00, 0x00, 0x04, 0x00, 0x20, 0x00, 0x00,
    },
};

static const constexpr mbedtls_mpi mul1_mpi = {
    .private_p = const_cast<mbedtls_mpi_uint *>(mul1_data.limbs),
    .private_s = 1,
    .private_n = std::size(mul1_data.limbs),
};

static const limbs_bytes mul2_data = {
    .bytes = {
        // BE
        // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
        // LE
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
};

static const constexpr mbedtls_mpi mul2_mpi = {
    .private_p = const_cast<mbedtls_mpi_uint *>(mul2_data.limbs),
    .private_s = 1,
    .private_n = std::size(mul2_data.limbs),
};

static const limbs_bytes mul2_mask = {
    .bytes = {
        // BE
        // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F,
        // 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F,
        // LE
        0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0,
        0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
};

[[gnu::noinline]]
static bool tweak_key(uint8_t key_out_be[32], uint32_t address)
{
    address >>= 5;

    mbedtls_mpi_uint mul1_limbs[9];
    mbedtls_mpi_uint mul2_limbs[9];

    mbedtls_mpi mul1 = {
        .private_p = mul1_limbs,
        .private_s = 1,
        .private_n = 9,
    };

    mbedtls_mpi mul2 = {
        .private_p = mul2_limbs,
        .private_s = 1,
        .private_n = 9,
    };

    if (mbedtls_mpi_mul_int(&mul1, &mul1_mpi, address) != 0) {
        return false;
    }

    if (mbedtls_mpi_mul_int(&mul2, &mul2_mpi, address) != 0) {
        return false;
    }

    const uint32_t *key_in_le_u32 = const_cast<uint32_t *>(&EFUSE.blk1_rdata0.val);

    for (size_t i = 0; i < 8; i++) {
        uint32_t le32 = key_in_le_u32[i] ^ (mul1_limbs[i] | (mul2_limbs[i] & mul2_mask.limbs[i]));

        uint8_t *out = key_out_be + (7 - i) * 4;
        out[0] = static_cast<uint8_t>(le32 >> 24);
        out[1] = static_cast<uint8_t>(le32 >> 16);
        out[2] = static_cast<uint8_t>(le32 >>  8);
        out[3] = static_cast<uint8_t>(le32 >>  0);
    }

    return true;
}

// Override weakly-linked IDF function
extern "C" esp_err_t tf_read_encrypted(esp_flash_t *chip, uint32_t address, uint8_t *data, uint32_t length);

extern "C" esp_err_t tf_read_encrypted(esp_flash_t *chip, uint32_t address, uint8_t *data, uint32_t length)
{
    if (address % 32 != 0 || length % 16 != 0) {
        return ESP_ERR_INVALID_SIZE;
    }

    esp_err_t err = esp_flash_read(chip, data, address, length);
    if (err != ESP_OK) {
        logger.printfln("Flash read failed: 0x%x", static_cast<unsigned>(err));
        return err;
    }

    esp_aes_context aes;
    aes.key_bytes = 32;
    // Don't have to set aes.key_in_hardware

    for (size_t offset = 0; offset < length; offset += 16) {
        uint32_t flash_address = address + offset;

        if (flash_address % 32 == 0) {
            if (!tweak_key(aes.key, flash_address)) {
                logger.printfln("Key tweaking failed");
                return -1;
            }
        }

        uint8_t data_buf[16];

        // Reverse encrypted data into buffer
        uint32_t *in32ptr = static_cast<uint32_t *>(static_cast<void *>(data + offset));
        for (size_t i = 0; i < sizeof(data_buf) / 4; i++) {
            const uint32_t in32 = in32ptr[i];

            uint8_t *out = data_buf + (3 - i) * 4;
            out[0] = static_cast<uint8_t>(in32 >> 24);
            out[1] = static_cast<uint8_t>(in32 >> 16);
            out[2] = static_cast<uint8_t>(in32 >>  8);
            out[3] = static_cast<uint8_t>(in32 >>  0);
        }

        if (esp_aes_crypt_ecb(&aes, ESP_AES_ENCRYPT, data_buf, data_buf) != 0) {
            logger.printfln("AES crypt ECB failed");
            return -1;
        }

        // Reverse decrypted data into output
        uint8_t *out8ptr = data + offset;
        for (size_t i = 0; i < sizeof(data_buf) / 4; i++) {
            const uint32_t in32 = static_cast<uint32_t *>(static_cast<void *>(data_buf))[i];

            uint8_t *out = out8ptr + (3 - i) * 4;
            out[0] = static_cast<uint8_t>(in32 >> 24);
            out[1] = static_cast<uint8_t>(in32 >> 16);
            out[2] = static_cast<uint8_t>(in32 >>  8);
            out[3] = static_cast<uint8_t>(in32 >>  0);
        }
    }

    return ESP_OK;
}

esp_err_t ESP32CommonDecryption::read_encrypted(esp_flash_t *chip, uint32_t address, uint8_t *data, uint32_t length)
{
    return tf_read_encrypted(chip, address, data, length);
}
