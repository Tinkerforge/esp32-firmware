/* esp32-firmware
 * Copyright (C) 2025 Mattias Schäffersmann <mattias@tinkerforge.com>
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
#include <stddef.h>
#include <stdint.h>

// ESP IDF
#include <esp_efuse.h>
#include <esp_flash.h>
#include <esp_mac.h>
#include <esp_ota_ops.h>
#include <esp_random.h>
#include <esp_secure_boot.h>
#include <mbedtls/sha256.h>
#include <soc/efuse_struct.h>
#include <sodium.h>

// Other libraries
#include <WString.h>

// This module
#include "./esp32_common.h"
//#include "./flash_decryption.h"
#include "./flash_encryption.h"
#include "./generated/module_dependencies.h"
#include "./secure_boot.h"
#include "./esp32_tools.h"

// Other project things
#include "bindings/base58.h"
#include "build.h"
#include "event_log_prefix.h"
#include "options.h"
#include "tools.h" // for defer{}
#include "tools/fs.h"
#include "tools/malloc.h"
#include "tools/string_builder.h"

#include "gcc_warnings.h"

static constexpr uint32_t LAST_BASE58_ID = 1000000; // Comment UID as base58 and zbase32 here
static constexpr uint32_t LAST_DIRECT_WIFI_PASSPHRASE_ID = LAST_BASE58_ID;

// TF_HAL hal;
// extern int8_t blue_led_pin;
// extern int8_t green_led_pin;
// extern int8_t button_pin;

// TODO steal from brick modules
// #if TF_LOCAL_ENABLE != 0
// static TF_Local local;
// #endif

[[gnu::noinline]] static uint16_t parse_secure_request(WebServerRequest &req, String *errmsg, bool *deterministic_out);

// Use z-base-32 from ZRTP (RFC 6189)
// - Case-insensitive, no hostname collisions
// - Characters chosen to avoid ambiguities for human readers
// - All existing ESP32 Bricks have either three-digit UIDs or four-digit UIDs with a leading 2.
//   z-base-32 doesn't contain '2' and all future IDs have four digits or more, so no collisions are possible.

static constexpr const char *ZBASE32_ALPHABET = "ybndrfg8ejkmcpqxot1uwisza345h769";
static constexpr size_t ZBASE32_MAX_STRLEN = 7; // log32(2^32-1) = 6.4

static void zbase32_encode(uint32_t value, String *out_str)
{
    char buf[ZBASE32_MAX_STRLEN + 1];
    char *str = buf + ZBASE32_MAX_STRLEN;
    *str = 0;

    do {
        const uint32_t rem = value % 32;
        value = value / 32;
        *(--str) = ZBASE32_ALPHABET[rem];
    } while (value > 0);

    *out_str = str;
}

static constexpr const char *BASE58_ALPHABET = "123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ";
static constexpr size_t BASE58_MAX_STRLEN = 6; // log58(2^32-1) = 5.463

static void base58_encode(uint32_t value, String *out_str)
{
    char buf[BASE58_MAX_STRLEN + 1];
    char *str = buf + BASE58_MAX_STRLEN;
    *str = 0;

    do {
        const uint32_t rem = value % 58;
        value = value / 58;
        *(--str) = BASE58_ALPHABET[rem];
    } while (value > 0);

    *out_str = str;
}

static void base58_encode_fixed_width(uint32_t value, size_t digits, char *out_str)
{
    char *str = out_str + digits;
    *str = 0;

    while (out_str < str) {
        const uint32_t rem = value % 58;
        value = value / 58;
        *(--str) = BASE58_ALPHABET[rem];
    }
}


void ESP32Common::pre_init()
{
#if MODULE_DEBUG_AVAILABLE() && defined(DEBUG_FS_ENABLE)
    if (!esp_secure_boot_enabled()) {
        if (esp_ota_get_running_partition() == nullptr) {
            String errmsg;
            if (!debug.repartition(errmsg, true)) {
                logger.printfln("Can't find running partition and repartitioning failed: %s", errmsg.c_str());
            }
        }
    }
#endif
}

void ESP32Common::pre_setup()
{
    uid_num = EFUSE.blk3_rdata7.val;

    uid_str = perm_new_prefer<String>(RAM::PSRAM, RAM::DRAM, RAM::_NONE);

    if (uid_num <= LAST_BASE58_ID) {
        base58_encode(uid_num, uid_str);
    } else {
        zbase32_encode(uid_num, uid_str);
    }
}

void ESP32Common::setup()
{
    // TODO move watchdog here from brick modules

    initialized = true;

    // esp32_common_dump_mem(&EFUSE.blk0_rdata0.val, 7);
    // esp32_common_dump_mem(&EFUSE.blk1_rdata0.val, 8);
    // esp32_common_dump_mem(&EFUSE.blk2_rdata0.val, 8);
    // esp32_common_dump_mem(&EFUSE.blk3_rdata0.val, 8);
}

void ESP32Common::register_urls()
{
#ifdef DEBUG_FS_ENABLE
    server.on_HTTPThread("/esp32/dump_flash", HTTP_GET, [this](WebServerRequest request) {
        constexpr uint32_t BUFFER_SIZE = 4000;
        char buffer[BUFFER_SIZE];

        uint32_t flash_size = 0;
        esp_err_t err = esp_flash_get_physical_size(esp_flash_default_chip, &flash_size);

        if (err != ESP_OK) {
            logger.printfln("Failed to query physical Flash size: %s (0x%x). Using data from binary image header.", esp_err_to_name(err), static_cast<unsigned>(err));
            flash_size = esp_flash_default_chip->size;
        }

        request.beginChunkedResponse_bytes(200);

        for (uint32_t offset = 0; offset < flash_size; offset += BUFFER_SIZE) {
            const uint32_t to_send = std::min(BUFFER_SIZE, flash_size - offset);

            err = esp_flash_read(esp_flash_default_chip, buffer, offset, to_send);

            if (err != ESP_OK) {
                logger.printfln("esp_flash_read failed: %s (0x%x)", esp_err_to_name(err), static_cast<unsigned>(err));
                return WebServerRequestReturnProtect{.error = ESP_FAIL};
            }

            SEND_CHUNK_OR_FAIL_LEN(request, buffer, to_send);
        }

        return request.endChunkedResponse();
    });
#endif

#if OPTIONS_EXPORT_PRIVATE_KEYS()
    server.on_HTTPThread("/esp32/dump_wifi_passphrase", HTTP_GET, [this](WebServerRequest req) {
        // Don't attempt to clear the passphrase from memory.
        // If you use this endpoint, you have other issues.
        return req.send_plain(200, get_default_wifi_passphrase());
    });
#endif

#if OPTIONS_EXPORT_PRIVATE_KEYS() && defined(DEBUG_FS_ENABLE)
    server.on_HTTPThread("/esp32/create_complete_master_key", HTTP_GET, [this](WebServerRequest req) {
        const bool success = this->create_complete_master_key();
        return req.send_plain(200, success ? "Success" : "Fail");
    });

    // server.on_HTTPThread("/esp32/create_basic_master_key", HTTP_GET, [this](WebServerRequest req) {
    //     const bool success = this->create_basic_master_key();
    //     return req.send_plain(200, success ? "Success" : "Fail");
    // });
#endif

    // Only allow encryption and Secure Boot if there is enough space for a suitable bootloader.
#if OPTIONS_ENABLE_SECURE_BOOT_API() && defined(CONFIG_SECURE_BOOT) && ESP_PRIMARY_PARTITION_TABLE_OFFSET == 0xe000
    if (!ESP32CommonSecureBoot::is_secure_boot_enabled()) {
        logger.printfln("Secure Boot not enabled");

        // Secure Boot and Flash encryption not enabled: Allow securing the device.
        server.on_HTTPThread("/esp32/secure_device", HTTP_PUT, [](WebServerRequest req) {
            String errmsg;
            bool deterministic;

            const uint16_t parse_status = parse_secure_request(req, &errmsg, &deterministic);

            if (parse_status != 200) {
                return req.send_plain(parse_status, errmsg);
            }

            ESP32CommonEncryption::encrypt_app_bootloader_partition_table(&errmsg, deterministic);

            if (errmsg.isEmpty()) {
                return req.send_plain(200, "Success");
            } else {
                return req.send_plain(500, errmsg);
            }
        });
    } else {
        logger.printfln("Secure Boot enabled");

        // Secure Boot enabled: SBv2 key can be checked/generated.
        server.on_HTTPThread("/esp32/check_sbv2", HTTP_GET, [this](WebServerRequest req) {
            const bool success = ESP32CommonSecureBoot::check_secure_boot_v2_key();
            return req.send_plain(200, success ? "Success" : "Fail");
        });

        // Flash encryption enabled (because Secure Boot is enabled) and data partition not encrypted: Data partition can be encrypted.
        if (!ESP32CommonEncryption::is_data_partition_encrypted()) {
            logger.printfln("Data partition not encrypted");

            server.on_HTTPThread("/esp32/encrypt_data", HTTP_GET, [this](WebServerRequest req) {
                bool success = false;

                // Ignore return value, 'success' will still be false on error.
                (void)task_scheduler.await([&success]() {
                    success = rewrite_data_partition([]() {
                        return ESP32CommonEncryption::mark_and_erase_data_partition();
                    });

                }, 1_min); // Probably only 30_s due to the main thread watchdog.

                if (success) {
                    return req.send_plain(200, "Success");
                } else {
                    return req.send_plain(500, "Failure");
                }
            });
        }

        // Secure Boot enabled and not yet locked down: Allow lockdown.
        if (!ESP32CommonSecureBoot::is_locked_down()) {
            logger.printfln("Not locked down");

            server.on("/esp32/lockdown", HTTP_GET, [this](WebServerRequest req) {
                micros_t t_start = now_us();
                const bool success = ESP32CommonSecureBoot::lockdown();
                uint32_t runtime = (now_us() - t_start).as<uint32_t>();
                logger.printfln("Lockdown took %luus", runtime);

                if (success) {
                    logger.printfln("Lockdown successful");
                    return req.send_plain(200, "Success");
                } else {
                    logger.printfln("Lockdown failed");
                    return req.send_plain(500, "Failure");
                }
            });
        }
    }
#endif
}

[[gnu::noinline]]
[[gnu::unused]] // May be unused, depending on options and sdkconfig
static uint16_t parse_secure_request(WebServerRequest &req, String *errmsg, bool *deterministic_out)
{
    if (req.contentLength() > 256) {
        *errmsg = "Content Too Large";
        return 413;
    }

    char buf[256];
    int received = req.receive(buf, std::size(buf));

    if (received < 0) {
        *errmsg = "Failed to receive request payload";
        return 500;
    }

    size_t input_length = static_cast<size_t>(received);
    StaticJsonDocument<256> doc;
    const DeserializationError error = deserializeJson(doc, buf, input_length);

    if (error) {
        char error_string[64];
        StringWriter sw(error_string, ARRAY_SIZE(error_string));
        sw.puts("Failed to deserialize string: ");
        sw.puts(error.c_str());
        *errmsg = sw.getPtr();
        return 400;
    }

    if (!doc["destroy_my_data"].as<bool>()) {
        *errmsg = "Request did not accept destroying all data";
        return 400;
    }

    if (!doc.containsKey("deterministic")) {
        *errmsg = "Missing required key 'deterministic'";
        return 400;
    }

    *deterministic_out = doc["deterministic"].as<bool>();

    return 200;
}

/*
Efuse data from IDF's esp_efuse_table.csv

EFUSE_BLK_3 is 256 bit (32 byte, 8 registers) long and organized as follows:
register 0:                                                     use me
    byte 0 - Custom MAC CRC, zero by default                    leave free
    byte 1 - MAC bytes 0, zero by default                       leave free
    byte 2 - MAC bytes 1, zero by default                       use me
    byte 3 - MAC bytes 2, zero by default                       use me
register 1:                                                     use me
    byte 0 - MAC bytes 3, zero by default                       use me
    byte 1 - MAC bytes 4, zero by default                       use me
    byte 2 - MAC bytes 5, zero by default                       use me
    byte 3 - Wifi passphrase chunk 0 byte 0                     already used
register 2:                                                     already used
    byte 0 - Wifi passphrase chunk 0 byte 1                     already used
    byte 1 - Wifi passphrase chunk 0 byte 2                     already used
    byte 2 - Wifi passphrase chunk 1 byte 0                     already used
    byte 3 - Wifi passphrase chunk 1 byte 1                     already used
register 3:                                                     use me
    byte 0 - ADC 1 calibration data, zeros by default           use me
    byte 1 - ADC 1 calibration data, zeros by default           use me
    byte 2 - ADC 2 calibration data, zeros by default           use me
    byte 3 - ADC 2 calibration data, zeros by default           use me
register 4:                                                     use me
    Secure version for anti-rollback, zeros by default          use me
register 5:                                                     already used
    byte 0 - Wifi passphrase chunk 1 byte 2                     already used
    byte 1 - Wifi passphrase chunk 2 byte 0                     already used
    byte 2 - Wifi passphrase chunk 2 byte 1                     already used
    byte 3 - Custom MAC Version, must be zero so that the data in block 0 and 1 is ignored
register 6:                                                     already used
    byte 0 - Wifi passphrase chunk 2 byte 2                     already used
    byte 1 - Wifi passphrase chunk 3 byte 0                     already used
    byte 2 - Wifi passphrase chunk 3 byte 1                     already used
    byte 3 - Wifi passphrase chunk 3 byte 2                     already used
register 7:                                                     already used
    UID                                                         already used
*/

[[gnu::noinline]]
bool ESP32Common::get_subkey(uint8_t *subkey, size_t subkey_len, uint64_t subkey_id, const char subkey_context[crypto_kdf_blake2b_CONTEXTBYTES])
{
    if (subkey_len < crypto_kdf_blake2b_BYTES_MIN || crypto_kdf_blake2b_BYTES_MAX < subkey_len) {
        logger.printfln("Invalid subkey length %zu", subkey_len);
        return false;
    }

    if (sodium_init() < 0) {
        logger.printfln("Failed to initialize libsodium, subkey not available");
        return false;
    }

    if (uid_num <= LAST_DIRECT_WIFI_PASSPHRASE_ID) {
        if (!fill_missing_master_key()) {
            return false;
        }
    }

    // Read bank 3
    union {
        uint8_t  u8[32];
        uint32_t u32[8];
    } bank3;

    static_assert(sizeof(bank3.u8) == crypto_kdf_blake2b_KEYBYTES);

    memcpy(&bank3, const_cast<uint32_t *>(&EFUSE.blk3_rdata0.val), sizeof(bank3)); // Cast removes volatile, not const

    if (bank3.u32[0] == 0 && bank3.u32[1] == 0 && bank3.u32[2] == 0 && bank3.u32[3] == 0 && bank3.u32[4] == 0 && bank3.u32[5] == 0 && bank3.u32[6] == 0) {
        logger.printfln("Master key not set, subkey unavailable");
        return false;
    }

#ifdef CONFIG_SOC_IEEE802154_SUPPORTED
#error "8 byte MACs not supported"
#endif
    uint8_t factory_mac[6];
    esp_efuse_mac_get_default(factory_mac);
    //logger.printfln("%02x:%02x:%02x:%02x:%02x:%02x factory MAC", factory_mac[0], factory_mac[1], factory_mac[2], factory_mac[3], factory_mac[4], factory_mac[5]);

    // Fill reserved two bytes with two bytes from the factory MAC address.
    bank3.u8[0] = factory_mac[3];
    bank3.u8[1] = factory_mac[4];

    // Fill unusable MAC version byte with the last byte from the factory MAC address.
    bank3.u8[23] = factory_mac[5];

    sodium_memzero(factory_mac, sizeof(factory_mac));

    const int derive_err = crypto_kdf_blake2b_derive_from_key(subkey, subkey_len, 0, subkey_context, bank3.u8);

    sodium_memzero(&bank3, sizeof(bank3));

    if (derive_err != 0) {
        logger.printfln("crypto_kdf_blake2b_derive_from_key failed: %i", derive_err);
        return false;
    }

    return true;
}

String ESP32Common::get_base58_uid_str()
{
    if (uid_num <= LAST_BASE58_ID) {
        return *uid_str;
    } else {
        char str[7]; // TF_BASE58_MAX_STR_SIZE is 7 but not exported
        tf_base58_encode(uid_num, str);

        return String{str};
    }
}

String ESP32Common::get_default_name(char separator)
{
    char buffer[128];
    StringWriter sw(buffer, std::size(buffer));

    sw.puts(OPTIONS_HOSTNAME_PREFIX());
    sw.putc(separator);
    sw.puts(*uid_str);

    return sw.toString();
}

[[gnu::noinline]]
static bool get_wifi_passphrase_blocks_direct(uint32_t passphrase_blocks[4])
{
    // Copy unused register 0 so that the numbering is easier.
    // Doesn't need register 7.
    uint32_t bank3_r06[7];

    memcpy(bank3_r06, const_cast<uint32_t *>(&EFUSE.blk3_rdata0.val), sizeof(bank3_r06)); // Cast removes volatile, not const

    // Old WiFi passphrase schema
    passphrase_blocks[0] = ((bank3_r06[1] & 0xFF000000) >> 24) | ((bank3_r06[2] & 0x0000FFFF) << 8);
    passphrase_blocks[1] = ((bank3_r06[2] & 0xFFFF0000) >> 16) | ((bank3_r06[5] & 0x000000FF) << 16);
    passphrase_blocks[2] = ((bank3_r06[5] & 0x00FFFF00) >> 8)  | ((bank3_r06[6] & 0x000000FF) << 16);
    passphrase_blocks[3] =  (bank3_r06[6] & 0xFFFFFF00) >> 8;

    sodium_memzero(bank3_r06, sizeof(bank3_r06));

    if (passphrase_blocks[0] == 0 && passphrase_blocks[1] == 0 && passphrase_blocks[2] == 0 && passphrase_blocks[3] == 0) {
        logger.printfln("Old-style WiFi default passphrase unset");
        return false;
    }

    return true;
}

[[gnu::noinline]]
bool ESP32Common::get_wifi_passphrase_blocks_derived(uint32_t passphrase_blocks[4])
{
    union {
        uint8_t  u8[32];
        uint64_t u64[4];
    } passphrase_data;

    static_assert(sizeof(passphrase_data.u8) == sizeof(passphrase_data.u64));

    constexpr const char subkey_context[crypto_kdf_blake2b_CONTEXTBYTES] = {'w','i','f','i','p','a','s','s'};

    if (!get_subkey(passphrase_data.u8, sizeof(passphrase_data.u8), 0, subkey_context)) {
        logger.printfln("Subkey failed, WiFi default passphrase unavailable");
        return false;
    }

    for (size_t i = 0; i < std::size(passphrase_data.u64); i++) {
        // Limit each passphrase block to four base58 digits, which contain ~23.43 bits of entropy (log2(58^4)). Must use 64 bits as input.
        // With 24 bits of input, half the passwords appear twice as often as the other half, which makes them prime brute force targets.
        // With 32 bits of input, one in 2^8.57 passwords appear more often as others, which is still useful for an attack.
        // With 64 bits of input, one in 2^40.57 passwords appear more often as others, which should provide no practical advantage for an attack.
        passphrase_blocks[i] = static_cast<uint32_t>(passphrase_data.u64[i] % (58*58*58*58));
    }

    sodium_memzero(&passphrase_data, sizeof(passphrase_data));

    return true;
}

String ESP32Common::get_default_wifi_passphrase()
{
    uint32_t passphrase_blocks[4];
    bool passphrase_success;

    if (uid_num <= LAST_DIRECT_WIFI_PASSPHRASE_ID) {
        passphrase_success = get_wifi_passphrase_blocks_direct(passphrase_blocks);
    } else {
        passphrase_success = get_wifi_passphrase_blocks_derived(passphrase_blocks);
    }

    if (!passphrase_success) {
        return String();
    }

    const size_t BLOCK_COUNT = 4;
    const size_t BLOCK_LENGTH = 4;

    char passphrase_buf[BLOCK_COUNT * BLOCK_LENGTH + (BLOCK_COUNT - 1) + 1]; // 4*4 characters + 3 separators + termination
    StringWriter pass(passphrase_buf, std::size(passphrase_buf));

    for (size_t i = 0; i < BLOCK_COUNT; ++i) {
        base58_encode_fixed_width(passphrase_blocks[i], BLOCK_LENGTH, pass.getRemainingPtr());
        pass.setLength(pass.getLength() + BLOCK_LENGTH);

        pass.putc('-'); // Trailing dash gets truncated by termination
    }

    sodium_memzero(passphrase_blocks, sizeof(passphrase_blocks));

#if OPTIONS_DUMP_DEFAULT_WIFI_PASSPHRASE()
    Serial.write("WiFi passphrase: ");
    Serial.write(pass.getPtr(), pass.getLength());
    Serial.write(static_cast<uint8_t>('\n'));
#endif

    // Zero passphrase buffer after the toString() call
    defer {
        pass.clear();
        sodium_memzero(passphrase_buf, sizeof(passphrase_buf));
    };

    return pass.toString();
}

[[gnu::noinline]]
bool ESP32Common::create_complete_master_key()
{
    if ((EFUSE.blk3_rdata0.val & 0xFFFF0000) != 0 ||
        EFUSE.blk3_rdata1.val != 0 ||
        EFUSE.blk3_rdata2.val != 0 ||
        EFUSE.blk3_rdata3.val != 0 ||
        EFUSE.blk3_rdata4.val != 0 ||
        (EFUSE.blk3_rdata5.val & 0x00FFFFFF ) != 0 ||
        EFUSE.blk3_rdata6.val != 0) {
            logger.printfln("Master key already set");
            return true;
    }

    uint32_t rnd_data[7];
    for (size_t i = 0; i < std::size(rnd_data); i++) {
        rnd_data[i] = esp_random();
    }

    esp_efuse_batch_write_begin();

    esp_err_t err1 = esp_efuse_write_block(EFUSE_BLK3, static_cast<void *>(rnd_data + 0),     16, 21 *  8); // 1/2 of register 0, registers 1-4, and 3/4 of register 5
    esp_err_t err2 = esp_efuse_write_block(EFUSE_BLK3, static_cast<void *>(rnd_data + 6), 6 * 32,      32); // register 6

    if (err1 != ESP_OK || err2 != ESP_OK) {
        logger.printfln("EFUSE block write failed: 0x%04x 0x%04x", static_cast<unsigned>(err1), static_cast<unsigned>(err2));
        esp_efuse_batch_write_cancel();
        return false;
    }

    err1 = esp_efuse_batch_write_commit();

    if (err1 != ESP_OK) {
        logger.printfln("EFUSE batch write failed: %s (0x%04x)", esp_err_to_name(err1), static_cast<unsigned>(err1));
        return false;
    }

    logger.printfln("Complete master key set");
    return true;
}

[[gnu::noinline]]
bool ESP32Common::create_basic_master_key()
{
    if ((EFUSE.blk3_rdata1.val & 0xFF000000) != 0 ||
        EFUSE.blk3_rdata2.val != 0 ||
        (EFUSE.blk3_rdata5.val & 0x00FFFFFF) != 0 ||
        EFUSE.blk3_rdata6.val != 0) {
            logger.printfln("Basic master key already set");
            return true;
    };

    uint32_t rnd_data[4];
    for (size_t i = 0; i < std::size(rnd_data); i++) {
        rnd_data[i] = esp_random();
    }

    esp_efuse_batch_write_begin();

    esp_err_t err1 = esp_efuse_write_block(EFUSE_BLK3, static_cast<void *>(rnd_data + 0), 32 + 24, 5 * 8); // 5 consecutive bytes in registers 1 and 2
    esp_err_t err2 = esp_efuse_write_block(EFUSE_BLK3, static_cast<void *>(rnd_data + 2),  5 * 32, 3 * 8); // 3 bytes in register 5
    esp_err_t err3 = esp_efuse_write_block(EFUSE_BLK3, static_cast<void *>(rnd_data + 3),  6 * 32,    32); // register 6

    if (err1 != ESP_OK || err2 != ESP_OK || err3 != ESP_OK) {
        logger.printfln("EFUSE block write failed: 0x%04x 0x%04x 0x%04x", static_cast<unsigned>(err1), static_cast<unsigned>(err2), static_cast<unsigned>(err3));
        esp_efuse_batch_write_cancel();
        return false;
    }

    err1 = esp_efuse_batch_write_commit();

    if (err1 != ESP_OK) {
        logger.printfln("EFUSE batch write failed: %s (0x%04x)", esp_err_to_name(err1), static_cast<unsigned>(err1));
        return false;
    }

    logger.printfln("Basic master key set");
    return true;
}

[[gnu::noinline]]
bool ESP32Common::fill_missing_master_key()
{
    if ((EFUSE.blk3_rdata0.val & 0xFFFF0000) != 0 ||
        (EFUSE.blk3_rdata1.val & 0x00FFFFFF) != 0 ||
        EFUSE.blk3_rdata3.val != 0 ||
        EFUSE.blk3_rdata4.val != 0) {
            // No missing master key
            return true;
    }

    if ((EFUSE.blk3_rdata1.val & 0xFF000000) == 0 &&
        EFUSE.blk3_rdata2.val == 0 &&
        (EFUSE.blk3_rdata5.val & 0x00FFFFFF) == 0 &&
        EFUSE.blk3_rdata6.val == 0) {
            logger.printfln("Basic master key unset");
            return false;
    };

    uint32_t rnd_data[4];
    for (size_t i = 0; i < std::size(rnd_data); i++) {
        rnd_data[i] = esp_random();
    }

    esp_efuse_batch_write_begin();

    esp_err_t err1 = esp_efuse_write_block(EFUSE_BLK3, static_cast<void *>(rnd_data + 0),     16, 16 + 24); // 1/2 of register 0 and 3/4 of register 1
    esp_err_t err2 = esp_efuse_write_block(EFUSE_BLK3, static_cast<void *>(rnd_data + 2), 3 * 32, 32 + 32); // register 3 and register 4

    if (err1 != ESP_OK || err2 != ESP_OK) {
        logger.printfln("EFUSE block write failed: 0x%04x 0x%04x", static_cast<unsigned>(err1), static_cast<unsigned>(err2));
        esp_efuse_batch_write_cancel();
        return false;
    }

    err1 = esp_efuse_batch_write_commit();

    if (err1 != ESP_OK) {
        logger.printfln("EFUSE batch write failed: %s (0x%04x)", esp_err_to_name(err1), static_cast<unsigned>(err1));
        return false;
    }

    logger.printfln("Master key completed");
    return true;
}

[[gnu::const]]
bool ESP32Common::initHAL()
{
    return false;
}

[[gnu::const]]
bool ESP32Common::destroyHAL() {
    return false;
}
