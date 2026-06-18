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

#include <sdkconfig.h>

#ifdef CONFIG_SECURE_BOOT

// Base libraries
#include <algorithm>
#include <memory>
#include <stdint.h>

// ESP IDF
#include <esp_efuse.h>
#include <esp_efuse_table.h>
#include <esp_flash.h>
#include <esp_flash_internal.h>
#include <esp_flash_partitions.h>
#include <esp_image_format.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_private/system_internal.h>
#include <esp_secure_boot.h>
#include <hal/efuse_hal.h>
#include <soc/efuse_reg.h>
#include <soc/efuse_struct.h>
#include <spi_flash_chip_driver.h>
#include <spi_flash_mmap.h>

// mbedTLS
#include <mbedtls/error.h>
#include <mbedtls/sha256.h>

// Other libs
#include <sodium.h>
#include <TFTools/Micros.h>
#include <WString.h>

// This module
#include "./flash_encryption.h"
#include "./generated/module_dependencies.h"
#include "./partitions.h"
#include "./secure_boot.h"
#include "./esp32_tools.h"

// Other project things
#include "event_log_prefix.h"
#include "tools.h"
#include "tools/fs.h"
#include "tools/mbedtls.h"

#include "gcc_warnings.h"

[[gnu::noinline]]
static void log_mbedtls_error(int error, const char *msg)
{
    char error_buf[128];
    mbedtls_strerror(error, error_buf, sizeof(error_buf));
    logger.printfln("SBv2: %s: -0x%04x (%s)", msg, static_cast<unsigned>(-error), error_buf);
}

static uint32_t align_up(uint32_t value, uint32_t multiple)
{
    const uint32_t lower_bits = multiple - 1;
    assert((multiple & lower_bits) == 0);
    return (value + lower_bits) & ~lower_bits;
}

bool ESP32CommonEncryption::check_and_set_encryption_key(bool deterministic)
{
    // FLASH_CRYPT_CONFIG
    const uint32_t flash_crypt_config = EFUSE.blk0_rdata5.rd_flash_crypt_config;

    if (flash_crypt_config == 0) {
        const int flash_crypt_config_size = esp_efuse_get_field_size(ESP_EFUSE_FLASH_CRYPT_CONFIG);
        const uint32_t crypt_config = EFUSE_FLASH_CRYPT_CONFIG;

        const int err = esp_efuse_write_field_blob(ESP_EFUSE_FLASH_CRYPT_CONFIG, &crypt_config, static_cast<size_t>(flash_crypt_config_size));

        if (err != ESP_OK) {
            logger.printfln("Setting FLASH_CRYPT_CONFIG failed: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
            return false;
        }
    } else if (flash_crypt_config == EFUSE_FLASH_CRYPT_CONFIG) {
        logger.printfln("flash_crypt_config already set to 0xF");
    } else {
        logger.printfln("flash_crypt_config set to unexpected value 0x%lx", flash_crypt_config);
        return false;
    }

    // Generate encryption key
    uint8_t encryption_subkey[32];

    static constexpr const char subkey_context[crypto_kdf_blake2b_CONTEXTBYTES] = {'f','l','a','s','h','e','n','c'};

    if (deterministic) {
        logger.printfln("Generating deterministic Flash encryption key, derived from device's master key");

        if (!esp32_common.get_subkey(encryption_subkey, sizeof(encryption_subkey), 0, subkey_context)) {
            logger.printfln("Subkey failed, cannot set Flash encryption key");
            return false;
        }
    } else {
        logger.printfln("Generating random Flash encryption key");

        mbedtls_entropy_raii entropy;
        mbedtls_ctr_drbg_raii ctr_drbg;

        int err = mbedtls_ctr_drbg_seed(&ctr_drbg.ctx, mbedtls_entropy_func, &entropy.ctx, reinterpret_cast<const unsigned char *>(subkey_context), std::size(subkey_context));

        if (err != 0) {
            log_mbedtls_error(err, "mbedtls_ctr_drbg_seed failed");
            return false;
        }

        err = mbedtls_ctr_drbg_random(&ctr_drbg.ctx, encryption_subkey, sizeof(encryption_subkey));

        if (err != 0) {
            log_mbedtls_error(err, "mbedtls_ctr_drbg_random failed");
            return false;
        }
    }

    //esp32_common_dump_mem(encryption_subkey, sizeof(encryption_subkey));

    // Check or write encryption key
    const int key_read_protected_size  = esp_efuse_get_field_size(ESP_EFUSE_RD_DIS_ENCRYPT_FLASH_KEY);
    const int key_write_protected_size = esp_efuse_get_field_size(ESP_EFUSE_WR_DIS_ENCRYPT_FLASH_KEY);

    uint32_t key_read_protected = 0;
    int err = esp_efuse_read_field_blob(ESP_EFUSE_RD_DIS_ENCRYPT_FLASH_KEY, &key_read_protected, std::min(static_cast<size_t>(key_read_protected_size), 32u));

    logger.printfln("key read  protected=%lu size=%i err=%i", key_read_protected, key_read_protected_size, err);

    uint8_t encryption_key_fuses[32];

    if (key_read_protected) {
        memset(encryption_key_fuses, 0, sizeof(encryption_key_fuses));
    } else {
        memcpy(encryption_key_fuses, const_cast<uint32_t *>(&EFUSE.blk1_rdata0.val), sizeof(encryption_key_fuses)); // Cast removes volatile, not const
    }

    uint32_t key_write_protected = 0;
    err = esp_efuse_read_field_blob(ESP_EFUSE_WR_DIS_ENCRYPT_FLASH_KEY, &key_write_protected, std::min(static_cast<size_t>(key_write_protected_size), 32u));

    logger.printfln("key write protected=%lu size=%i err=%i", key_write_protected, key_write_protected_size, err);

    bool ret;

    if (key_write_protected) {
        if (key_read_protected) {
            logger.printfln("Encryption key fully protected");
            ret = true;
            goto out;
        }

        if (memcmp(encryption_key_fuses, encryption_subkey, sizeof(encryption_key_fuses)) == 0) {
            logger.printfln("Encryption key already set and matches");
        } else {
            logger.printfln("Encryption key already set and doesn't match");
        }

    } else {
        for (size_t i = 0; i < std::size(encryption_subkey); i++) {
            uint8_t byte_subkey = encryption_subkey[i];
            uint8_t byte_fuses  = encryption_key_fuses[i];

            if ((~byte_subkey & byte_fuses) != 0) {
                logger.printfln("Encryption key byte %zu has incorrect bits set: expected 0x%hhx, found 0x%hhx", i, byte_subkey, byte_fuses);
                ret = false;
                goto out;
            }

            // Mask out already set bits
            encryption_subkey[i] = byte_subkey & ~byte_fuses;
        }

        //esp32_common_dump_mem(encryption_subkey, sizeof(encryption_subkey));

        err = esp_efuse_write_block(EFUSE_BLK_ENCRYPT_FLASH, encryption_subkey, 0, sizeof(encryption_subkey) * 8);

        if (err != ESP_OK) {
            logger.printfln("Writing encryption key failed: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
            ret = false;
            goto out;
        }

        key_write_protected = ~0lu;
        err = esp_efuse_write_field_blob(ESP_EFUSE_WR_DIS_ENCRYPT_FLASH_KEY, &key_write_protected, std::min(static_cast<size_t>(key_write_protected_size), 32u));

        if (err != ESP_OK) {
            logger.printfln("Write-protecting encryption key failed: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
            ret = false;
            goto out;
        }
    }

    ret = true;

out:
    sodium_memzero(encryption_subkey,    sizeof(encryption_subkey));
    sodium_memzero(encryption_key_fuses, sizeof(encryption_key_fuses));

    return ret;
}

static bool encrypt_ota_state(uint32_t ota_index, esp_ota_img_states_t new_state)
{
    assert(ota_index < 2);

    const esp_partition_t *ota_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, nullptr);

    if (ota_partition == nullptr) {
        logger.printfln("Could not find OTA partition");
        return false;
    }

    const uint32_t ota_partition_start = ota_partition->address;
    esp_ota_select_entry_t ota_data;

    esp_err_t err = esp_flash_read(ota_partition->flash_chip, &ota_data, ota_partition_start + ota_index * SPI_FLASH_SEC_SIZE, sizeof(ota_data));

    if (err != ESP_OK) {
        logger.printfln("Could not read OTA partition page %lu: %s (0x%04x)", ota_index, esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    ota_data.ota_state = new_state;

    err = esp_partition_erase_range(ota_partition, SPI_FLASH_SEC_SIZE * ota_index, SPI_FLASH_SEC_SIZE);

    if (err != ESP_OK) {
        logger.printfln("Could not erase OTA partition page %lu: %s (0x%04x)", ota_index, esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    err = esp_flash_write_encrypted(ota_partition->flash_chip, ota_partition_start + ota_index * SPI_FLASH_SEC_SIZE, &ota_data, sizeof(ota_data));

    if (err != ESP_OK) {
        logger.printfln("Could not write OTA partition page %lu: %s (0x%04x)", ota_index, esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    return true;
}

[[gnu::noinline]]
bool ESP32CommonEncryption::erase_core_dump(String *errmsg)
{
    logger.printfln("Erasing core dump partition");

    const esp_partition_t *core_dump_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "coredump");

    if (core_dump_partition == nullptr) {
        logger.printfln("Couldn't find core dump partition");
        *errmsg = "Couldn't find core dump partition";
        return false;
    }

    esp_err_t cde_err = esp_flash_erase_region(core_dump_partition->flash_chip, core_dump_partition->address, core_dump_partition->size);

    if (cde_err != ESP_OK) {
        logger.printfln("Couldn't erase core dump partition: %s (0x%04x)", esp_err_to_name(cde_err), static_cast<unsigned>(cde_err));
        *errmsg = "Core dump partition erase error";
        return false;
    }

    return true;
}

[[gnu::noinline]]
bool ESP32CommonEncryption::encrypt_app(String *errmsg, const esp_partition_t *running_partition, uint32_t running_app_number)
{
    const char *other_label;

    if (running_app_number == 0) {
        other_label = "app1";
    } else {
        other_label = "app0";
    }

    const esp_partition_t *other_partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, other_label);

    if (other_partition == nullptr) {
        logger.printfln("Couldn't find other partition '%s'", other_label);
        *errmsg = "Couldn't find other partition";
        return false;
    }

    const esp_partition_pos_t running_pos = {
        .offset = running_partition->address,
        .size   = running_partition->size,
    };

    esp_image_metadata_t image_data = {};
    const esp_err_t verify_err = esp_image_verify(ESP_IMAGE_VERIFY, &running_pos, &image_data);

    if (verify_err != ESP_OK) {
        logger.printfln("App image verification failed: start=0x%lx len=0x%lx %s (0x%04x)", image_data.start_addr, image_data.image_len, esp_err_to_name(verify_err), static_cast<unsigned>(verify_err));
        return false;
    }

    if (running_partition->address != image_data.start_addr) {
        logger.printfln("Partition address doesn't match image start address: %lu != %lu", running_partition->address, image_data.start_addr);
        return false;
    }

    const uint32_t block_erase_size  = other_partition->flash_chip->chip_drv->block_erase_size;
    const uint32_t sector_size = other_partition->flash_chip->chip_drv->sector_size;

    const uint32_t other_partition_start = other_partition->address;
    const uint32_t other_partition_size  = other_partition->size;
    uint32_t offset = 0;

    // Address and size must be multiples of the block erase size.
    if (other_partition->address % block_erase_size != 0 && other_partition->size % block_erase_size != 0) {
        logger.printfln("Invalid partition address or size: 0x%lx@0x%lx", other_partition->size, other_partition->address);
        *errmsg = "Invalid partition address or size";
        return false;
    }

    logger.printfln("Block-erasing %s", other_partition->label);

    while (offset < other_partition_size) {
        const uint32_t block_num = offset / block_erase_size;

        Serial.printf("%3lu\r", block_num);

        const esp_err_t erase_err = esp_flash_erase_region(other_partition->flash_chip, other_partition_start + offset, block_erase_size);

        if (erase_err != ESP_OK) {
            logger.printfln("Block erase error: %s (0x%04x)", esp_err_to_name(erase_err), static_cast<unsigned>(erase_err));
            *errmsg = "Block erase error";
            return false;
        }

        offset += block_erase_size;

        if (block_num % 16 == 0) {
            if (!esp32_poke_main_thread()) {
                return false;
            }
        }
    }

    // Encrypt running app into other partition.

    const uint32_t copy_block_size = other_partition->erase_size;

    if (copy_block_size != sector_size) {
        logger.printfln("Copy block size doesn't match sector size: %lu != %lu", copy_block_size, sector_size);
        return false;
    }

    void *ptr = heap_caps_malloc(copy_block_size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

    if (ptr == nullptr) {
        logger.printfln("Failed to allocate DRAM");
        *errmsg = "Failed to allocate DRAM";
        return false;
    }

    std::unique_ptr<uint8_t[]> uptr{static_cast<uint8_t *>(ptr)};

    int mbedtls_err;
    mbedtls_sha256_raii sha256;

    if ((mbedtls_err = mbedtls_sha256_starts(&sha256.ctx, false)) != 0) {
        *errmsg = "mbedtls_sha256_starts failed";
        log_mbedtls_error(mbedtls_err, errmsg->c_str());
        return false;
    }

    const uint32_t running_partition_start = running_partition->address;
    const uint32_t image_len = align_up(image_data.image_len, copy_block_size); // copy_block_size == sector_size
    offset = 0;

    const uint32_t block_write_count = image_len / copy_block_size;

    logger.printfln("Encrypting from %s to %s, 0x%06lx to 0x%06lx, len=%lu", running_partition->label, other_partition->label, running_partition_start, other_partition_start, image_len);

    while (offset < image_len) {
        const uint32_t block_num = offset / copy_block_size;
        const uint32_t pct = 100 * block_num / block_write_count;

        Serial.printf("%3lu %4lu/%4lu\r", pct, block_num, block_write_count);

        esp_err_t err = esp_flash_read(running_partition->flash_chip, ptr, running_partition_start + offset, copy_block_size);

        if (err != ESP_OK) {
            logger.printfln("Sector read error: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
            *errmsg = "Sector read error";
            return false;
        }

        if ((mbedtls_err = mbedtls_sha256_update(&sha256.ctx, static_cast<const uint8_t *>(ptr), copy_block_size)) != 0) {
            *errmsg = "mbedtls_sha256_update failed";
            log_mbedtls_error(mbedtls_err, errmsg->c_str());
            return false;
        }

        err = esp_flash_write_encrypted(other_partition->flash_chip, other_partition_start + offset, ptr, copy_block_size);

        if (err != ESP_OK) {
            logger.printfln("Sector write encrypted error: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
            *errmsg = "Sector write encrypted error";
            return false;
        }

        offset += copy_block_size;

        if (block_num % 64 == 0) {
            if (!esp32_poke_main_thread()) {
                return false;
            }
        }
    }

    uint8_t app_image_sha256[ESP_SECURE_BOOT_KEY_DIGEST_SHA_256_LEN];

    if ((mbedtls_err = mbedtls_sha256_finish(&sha256.ctx, app_image_sha256)) != 0) {
        *errmsg = "mbedtls_sha256_finish failed";
        log_mbedtls_error(mbedtls_err, errmsg->c_str());
        return false;
    }

    memset(ptr, 0xFF, copy_block_size);
    ets_secure_boot_sig_block_t *sig_block = static_cast<decltype(sig_block)>(ptr);
    if (!ESP32CommonSecureBoot::sb_sign_image(sig_block, app_image_sha256)) {
        logger.printfln("App image signing failed");
        *errmsg = "App image signing failed";
        return false;
    }

    esp_err_t err = esp_flash_write_encrypted(other_partition->flash_chip, other_partition_start + image_len, ptr, copy_block_size);

    if (err != ESP_OK) {
        logger.printfln("Sector write encrypted error: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        *errmsg = "Sector write encrypted error";
        return false;
    }

    // const size_t total_data_len = image_len + copy_block_size;
    // TO DO: Pad with 0xff to block size. <- Apparently not necessary?

    return true;
}

[[gnu::noinline]]
bool ESP32CommonEncryption::encrypt_bootloader_partition_table(const esp_partition_t *running_partition)
{
    //// Bootloader ////

    uint32_t bootloader_len;

    esp_err_t err = esp_image_verify_bootloader(&bootloader_len);

    if (err != ESP_OK) {
        logger.printfln("Failed to verify bootloader: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    const bool flash_encryption_enabled = efuse_hal_flash_encryption_enabled();
    const uint32_t sector_size = running_partition->flash_chip->chip_drv->sector_size;

    const uint32_t bootloader_len_padded = align_up(bootloader_len, sector_size);
    const uint32_t bootloader_len_signed = bootloader_len_padded + sizeof(ets_secure_boot_sig_block_t);
    const uint32_t bootloader_len_erase  = align_up(bootloader_len_signed, sector_size);

    void *bootloader_buf = heap_caps_malloc(bootloader_len_signed, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL); //malloc_psram_or_dram(bootloader_len);

    if (bootloader_buf == nullptr) {
        logger.printfln("Failed to allocate DRAM");
        return false;
    }

    std::unique_ptr<uint8_t[]> ubootloader_buf{static_cast<uint8_t *>(bootloader_buf)};

    void *pt_buf = heap_caps_malloc(ESP_PARTITION_TABLE_SIZE, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL); //malloc_psram_or_dram(ESP_PARTITION_TABLE_SIZE);

    if (pt_buf == nullptr) {
        logger.printfln("Failed to allocate DRAM");
        return false;
    }

    std::unique_ptr<uint8_t[]> upt_buf{static_cast<uint8_t *>(pt_buf)};

    if (flash_encryption_enabled) {
        err = esp_flash_read_encrypted(running_partition->flash_chip, ESP_PRIMARY_BOOTLOADER_OFFSET, bootloader_buf, bootloader_len);
    } else {
        err = esp_flash_read(running_partition->flash_chip, bootloader_buf, ESP_PRIMARY_BOOTLOADER_OFFSET, bootloader_len);
    }

    if (err != ESP_OK) {
        logger.printfln("Failed to read bootloader: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    // Pad to sector size
    memset(ubootloader_buf.get() + bootloader_len, 0xFF, bootloader_len_padded - bootloader_len);

    // Hash bootloader
    ets_secure_boot_sig_block_t *sig_block = static_cast<decltype(sig_block)>(static_cast<void *>(ubootloader_buf.get() + bootloader_len_padded));

    int mbedtls_err = mbedtls_sha256(ubootloader_buf.get(), bootloader_len_padded, sig_block->image_digest, 0);

    if (mbedtls_err != 0) {
        log_mbedtls_error(mbedtls_err, "Bootloader hashing failed");
        return false;
    }

    // Sign bootloader
    if (!ESP32CommonSecureBoot::sb_sign_image(sig_block, nullptr)) {
        logger.printfln("Bootloader signing failed");
        return false;
    }

    //// Partition table ////

    if (flash_encryption_enabled) {
        err = esp_flash_read_encrypted(running_partition->flash_chip, ESP_PRIMARY_PARTITION_TABLE_OFFSET, pt_buf, ESP_PARTITION_TABLE_SIZE);
    } else {
        err = esp_flash_read(running_partition->flash_chip, pt_buf, ESP_PRIMARY_PARTITION_TABLE_OFFSET, ESP_PARTITION_TABLE_SIZE);
    }

    if (err != ESP_OK) {
        logger.printfln("Failed to read partition table: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    const uint32_t pt_len_minimum = ESP32CommonPartitions::get_pt_size(upt_buf.get());

    if (pt_len_minimum == 0) {
        logger.printfln("Failed to get partition table size");
        return false;
    }

    static const constexpr char *encrypted_partition_labels[] = {
        "otadata",
        "app0",
        "app1",
        "coredump",
        //"data", // Don't mark as encrypted here. Encrypt after reboot.
        "factorydata",
    };

    for (size_t i = 0; i < std::size(encrypted_partition_labels); i++) {
        if (!ESP32CommonPartitions::mark_encrypted(upt_buf.get(), encrypted_partition_labels[i])) {
            logger.printfln("Failed to mark partition '%s' as encrypted", encrypted_partition_labels[i]);
            return false;
        }
    }

    if (!ESP32CommonPartitions::update_md5(upt_buf.get())) {
        logger.printfln("Failed to update partition table MD5");
        return false;
    }

    const uint32_t pt_erase_len = align_up(ESP_PARTITION_TABLE_SIZE, sector_size);
    const uint32_t pt_write_len = align_up(pt_len_minimum, 16);

    esp_flash_set_dangerous_write_protection(esp_flash_default_chip, false);

    // === CRITICAL SECTION START ===
    err = esp_flash_erase_region(running_partition->flash_chip, ESP_PRIMARY_PARTITION_TABLE_OFFSET, pt_erase_len);

    if (err != ESP_OK) {
        logger.printfln("Failed to erase partition table: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    err = esp_flash_write_encrypted(running_partition->flash_chip, ESP_PRIMARY_PARTITION_TABLE_OFFSET, pt_buf, pt_write_len);

    if (err != ESP_OK) {
        logger.printfln("Failed to write partition table: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    pt_buf = nullptr;
    upt_buf.reset();

    if (!flash_encryption_enabled) {
        err = esp_flash_erase_region(running_partition->flash_chip, ESP_PRIMARY_BOOTLOADER_OFFSET, bootloader_len_erase);

        if (err != ESP_OK) {
            logger.printfln("Failed to erase bootloader: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
            return false;
        }

        err = esp_flash_write_encrypted(running_partition->flash_chip, ESP_PRIMARY_BOOTLOADER_OFFSET, bootloader_buf, bootloader_len_signed);

        if (err != ESP_OK) {
            logger.printfln("Failed to write bootloader: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
            return false;
        }
    } else {
        err = esp_flash_erase_region(running_partition->flash_chip, ESP_PRIMARY_BOOTLOADER_OFFSET + bootloader_len_padded, align_up(sizeof(ets_secure_boot_sig_block_t), sector_size));

        if (err != ESP_OK) {
            logger.printfln("Failed to erase bootloader signature: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
            return false;
        }

        err = esp_flash_write_encrypted(running_partition->flash_chip, ESP_PRIMARY_BOOTLOADER_OFFSET + bootloader_len_padded, bootloader_buf, sizeof(ets_secure_boot_sig_block_t));

        if (err != ESP_OK) {
            logger.printfln("Failed to write bootloader signature: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
            return false;
        }
    }

    esp_flash_set_dangerous_write_protection(esp_flash_default_chip, true);

    return true;
}

[[gnu::noinline]]
static void print_fuse_commit_error(esp_err_t err)
{
    logger.printfln("Fuses commit failed: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
}

[[gnu::noinline]]
IRAM_ATTR
static bool commit_fuses_and_restart()
{
    esp_err_t err = esp_efuse_batch_write_commit();

    if (err != ESP_OK) {
        print_fuse_commit_error(err);
        return false;
    }

    // Force unfriendly restart, never reached anyway
    esp_restart_noos_dig();
}

bool ESP32CommonEncryption::burn_secure_encryption_fuses_restart()
{
    esp_efuse_batch_write_begin();

    esp_err_t err = esp_efuse_write_field_cnt(ESP_EFUSE_FLASH_CRYPT_CNT, (*ESP_EFUSE_FLASH_CRYPT_CNT)->bit_count);

    if (err != ESP_OK) {
        logger.printfln("Failed to write fuses to enable encryption: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        esp_efuse_batch_write_cancel();
        return false;
    }

    if (!ESP32CommonSecureBoot::set_secure_boot_fuse()) {
        esp_efuse_batch_write_cancel();
        return false;
    }

    return commit_fuses_and_restart();
}

void ESP32CommonEncryption::encrypt_app_bootloader_partition_table(String *errmsg, bool deterministic)
{
    const bool flash_encryption_enabled = efuse_hal_flash_encryption_enabled();
    const bool secure_boot_enabled = esp_secure_boot_enabled();

    if (flash_encryption_enabled || secure_boot_enabled) {
        if (flash_encryption_enabled && secure_boot_enabled) {
            logger.printfln("Encryption and Secure Boot already enabled?");
            return;
        } else if (flash_encryption_enabled && !secure_boot_enabled) {
            logger.printfln("Encryption enabled but Secure Boot is not. Attempting resume.");
        } else {
            logger.printfln("Encryption and Secure Boot inconsistent: flash_encryption_enabled=%i secure_boot_enabled=%i", flash_encryption_enabled, secure_boot_enabled);
            *errmsg = "Encryption and Secure Boot inconsistent";
            return;
        }
    }

    const esp_partition_t *const running_partition = esp_ota_get_running_partition();

    uint32_t running_app_number;

    if (strncmp(running_partition->label, "app0", std::size(running_partition->label)) == 0) {
        running_app_number = 0;
    } else if (strncmp(running_partition->label, "app1", std::size(running_partition->label)) == 0) {
        running_app_number = 1;
    } else {
        *errmsg = "Partition label error";
        return;
    }

    if (!check_and_set_encryption_key(deterministic)) {
        *errmsg = "Failed to set encryption key";
        return;
    }

    if (!ESP32CommonSecureBoot::set_secure_boot_key()) {
        *errmsg = "Failed to set Secure Boot v2 key";
        return;
    }

    if (!erase_core_dump(errmsg)) {
        *errmsg = "Failed to erase core dump partition";
        return;
    }

    if (!flash_encryption_enabled) {
        if (!encrypt_app(errmsg, running_partition, running_app_number)) {
            *errmsg = "Failed to encrypt app";
            return;
        }
    }

    if (!esp32_poke_main_thread()) {
        *errmsg = "Failed to poke main thread";
        return;
    }

    const uint32_t other_app_number = 1 - running_app_number;
    encrypt_ota_state(other_app_number,   ESP_OTA_IMG_VALID);
    encrypt_ota_state(running_app_number, ESP_OTA_IMG_INVALID);

    task_scheduler.scheduleOnce([running_partition]() {
        logger.printfln("Encrypting bootloader and partition table");

        if (!encrypt_bootloader_partition_table(running_partition)) {
            return;
        }

        burn_secure_encryption_fuses_restart();
    }, 1_s);
}

bool ESP32CommonEncryption::mark_and_erase_data_partition()
{
    const bool flash_encryption_enabled = efuse_hal_flash_encryption_enabled();

    void *pt_buf = heap_caps_malloc(ESP_PARTITION_TABLE_SIZE, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

    if (pt_buf == nullptr) {
        logger.printfln("Failed to allocate DRAM");
        return false;
    }

    std::unique_ptr<uint8_t[]> upt_buf{static_cast<uint8_t *>(pt_buf)};


    const esp_partition_t *data_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "data");

    if (data_partition == nullptr) {
        logger.printfln("Couldn't find data partition");
        return false;
    }

    esp_err_t err;

    if (flash_encryption_enabled) {
        err = esp_flash_read_encrypted(data_partition->flash_chip, ESP_PRIMARY_PARTITION_TABLE_OFFSET, pt_buf, ESP_PARTITION_TABLE_SIZE);
    } else {
        err = esp_flash_read(data_partition->flash_chip, pt_buf, ESP_PRIMARY_PARTITION_TABLE_OFFSET, ESP_PARTITION_TABLE_SIZE);
    }

    if (err != ESP_OK) {
        logger.printfln("Failed to read partition table: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    const uint32_t pt_len_minimum = ESP32CommonPartitions::get_pt_size(upt_buf.get());

    if (pt_len_minimum == 0) {
        logger.printfln("Failed to get partition table size");
        return false;
    }

    if (!ESP32CommonPartitions::mark_encrypted(upt_buf.get(), "data")) {
        logger.printfln("Failed to mark data partition as encrypted");
        return false;
    }

    if (!ESP32CommonPartitions::update_md5(upt_buf.get())) {
        logger.printfln("Failed to update partition table MD5");
        return false;
    }

    const uint32_t pt_erase_len = align_up(ESP_PARTITION_TABLE_SIZE, data_partition->erase_size);
    const uint32_t pt_write_len = align_up(pt_len_minimum, 16);

    logger.printfln("Erasing data partition and rewriting partition table");

    // Erase data partition
    err = esp_flash_erase_region(data_partition->flash_chip, data_partition->address, data_partition->size);

    if (err != ESP_OK) {
        logger.printfln("Couldn't erase data partition: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
    }

    // Erase partition table
    esp_flash_set_dangerous_write_protection(esp_flash_default_chip, false);

    err = esp_flash_erase_region(data_partition->flash_chip, ESP_PRIMARY_PARTITION_TABLE_OFFSET, pt_erase_len);

    if (err != ESP_OK) {
        logger.printfln("Failed to erase partition table: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    // Write partition table
    if (flash_encryption_enabled) {
        err = esp_flash_write_encrypted(data_partition->flash_chip, ESP_PRIMARY_PARTITION_TABLE_OFFSET, pt_buf, pt_write_len);
    } else {
        err = esp_flash_write(data_partition->flash_chip, pt_buf, ESP_PRIMARY_PARTITION_TABLE_OFFSET, pt_write_len);
    }

    if (err != ESP_OK) {
        logger.printfln("Failed to write partition table: %s (0x%04x)", esp_err_to_name(err), static_cast<unsigned>(err));
        return false;
    }

    esp_flash_set_dangerous_write_protection(esp_flash_default_chip, true);

    // Partition metadata is loaded at boot time and remains static afterwards.
    // Now that encryption has been enabled, the metadata must be updated so that the data partition can be encrypted after remounting.
    // Casting away const is safe here because the metadata is known to reside in RAM.

    logger.printfln("Marking runtime data partition as encrypted");
    esp_partition_t *data_partition_hack = const_cast<esp_partition_t *>(data_partition);
    data_partition_hack->encrypted = true;

    // Attempt to force write-back.
    asm volatile ("" ::: "memory");

    return true;
}

bool ESP32CommonEncryption::is_data_partition_encrypted()
{
    const esp_partition_t *data_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "data");

    if (data_partition == nullptr) {
        logger.printfln("Couldn't find data partition");
        return false;
    }

    return data_partition->encrypted;
}

#endif
