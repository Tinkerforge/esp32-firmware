/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "tools.h"

#include "bindings/errors.h"

#include <Arduino.h>

#include "SPIFFS.h"
#include "esp_spiffs.h"
#include "LittleFS.h"
#include "esp_littlefs.h"

#include <soc/efuse_reg.h>
#include "bindings/base58.h"
#include "bindings/bricklet_unknown.h"
#include "event_log.h"
#include "esp_log.h"

extern EventLog logger;

bool deadline_elapsed(uint32_t deadline_ms)
{
    uint32_t now = millis();

    return ((uint32_t)(now - deadline_ms)) < (UINT32_MAX / 2);
}

void read_efuses(uint32_t *ret_uid_num, char *ret_uid_str, char *ret_passphrase)
{
    uint32_t blocks[8] = {0};

    for (int32_t block3Address = EFUSE_BLK3_RDATA0_REG, i = 0; block3Address <= EFUSE_BLK3_RDATA7_REG; block3Address += 4, ++i) {
        blocks[i] = REG_GET_FIELD(block3Address, EFUSE_BLK3_DOUT0);
    }

    uint32_t passphrase[4] = {0};

    /*
    EFUSE_BLK_3 is 256 bit (32 byte, 8 blocks) long and organized as follows:
    block 0:
        Custom MAC CRC + MAC bytes 0 to 2
    block 1:
        Custom MAC bytes 3 to 5
        byte 3 - Wifi passphrase chunk 0 byte 0
    block 2:
        byte 0 - Wifi passphrase chunk 0 byte 1
        byte 1 - Wifi passphrase chunk 0 byte 2
        byte 2 - Wifi passphrase chunk 1 byte 0
        byte 3 - Wifi passphrase chunk 1 byte 1
    block 3:
        ADC 1 calibration data
    block 4:
        ADC 2 calibration data
    block 5:
        byte 0 - Wifi passphrase chunk 1 byte 2
        byte 1 - Wifi passphrase chunk 2 byte 0
        byte 2 - Wifi passphrase chunk 2 byte 1
        byte 3 - Custom MAC Version
    block 6:
        byte 0 - Wifi passphrase chunk 2 byte 2
        byte 1 - Wifi passphrase chunk 3 byte 0
        byte 2 - Wifi passphrase chunk 3 byte 1
        byte 3 - Wifi passphrase chunk 3 byte 2
    block 7:
        UID
    */

    passphrase[0] = ((blocks[1] & 0xFF000000) >> 24) | ((blocks[2] & 0x0000FFFF) << 8);
    passphrase[1] = ((blocks[2] & 0xFFFF0000) >> 16) | ((blocks[5] & 0x000000FF) << 16);
    passphrase[2] = ((blocks[5] & 0x00FFFF00) >> 8)  | ((blocks[6] & 0x000000FF) << 16);
    passphrase[3] =  (blocks[6] & 0xFFFFFF00) >> 8;
    *ret_uid_num = blocks[7];

    char buf[7] = {0};

    for (int i = 0; i < 4; ++i) {
        if (i != 0) {
            ret_passphrase[i * 5 - 1] = '-';
        }

        tf_base58_encode(passphrase[i], buf);

        if (strnlen(buf, sizeof(buf) / sizeof(buf[0])) != 4) {
            logger.printfln("efuse error: malformed passphrase!");
        } else {
            memcpy(ret_passphrase + i * 5, buf, 4);
        }
    }

    tf_base58_encode(*ret_uid_num, ret_uid_str);
}

int check(int rc, const char *msg)
{
    if (rc >= 0) {
        return rc;
    }

    logger.printfln("%lu Failed to %s rc: %s", millis(), msg, tf_hal_strerror(rc));
    delay(10);

    return rc;
}

class LogSilencer {
public:
    LogSilencer(const char *tag) : tag(tag), level_to_restore(ESP_LOG_NONE) {
        level_to_restore = esp_log_level_get(tag);
        esp_log_level_set(tag, ESP_LOG_NONE);
    }

    ~LogSilencer() {
        esp_log_level_set(tag, level_to_restore);
    }
    const char *tag;
    esp_log_level_t level_to_restore;
};

bool is_spiffs_available(const char *part_label, const char *base_path)
{
    LogSilencer ls{"SPIFFS"};

    esp_vfs_spiffs_conf_t conf = {
        .base_path = base_path,
        .partition_label = part_label,
        .max_files = 10,
        .format_if_mount_failed = false
    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    esp_vfs_spiffs_unregister(part_label);

    if (err == ESP_FAIL || err == ESP_ERR_NOT_FOUND) {
        return false;
    } else if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
        return true;
    }

    // Should only be ESP_NO_MEM
    return false;
}

bool is_littlefs_available(const char *part_label, const char *base_path)
{
    LogSilencer ls{"esp_littlefs"};

    esp_vfs_littlefs_conf_t conf = {
        .base_path = base_path,
        .partition_label = part_label,
        .format_if_mount_failed = false,
        .dont_mount = false
    };

    esp_err_t err = esp_vfs_littlefs_register(&conf);
    esp_vfs_littlefs_unregister(part_label);

    if (err == ESP_FAIL || err == ESP_ERR_NOT_FOUND) {
        return false;
    } else if (err == ESP_OK || err == ESP_ERR_INVALID_STATE) {
        return true;
    }

    // Should only be ESP_NO_MEM
    return false;
}

// Adapted from https://github.com/espressif/arduino-esp32/blob/master/libraries/LittleFS/examples/LITTLEFS_PlatformIO/src/main.cpp
bool mirror_filesystem(fs::FS &fromFS, fs::FS &toFS, String root_name, int levels, uint8_t *buf, size_t buf_size)
{
    // File works RAII style, no need to close.
    File root = fromFS.open(root_name);

    if (!root) {
        logger.printfln("Failed to open source directory %s!", root_name.c_str());
        return false;
    }

    if (!root.isDirectory()) {
        logger.printfln("Source file %s is not a directory!", root_name.c_str());
        return false;
    }

    File source;

    while (source = root.openNextFile()) {
        if (source.isDirectory()) {
            if (levels <= 0) {
                logger.printfln("Skipping over directory %s. Depth limit exceeded.", root_name.c_str());
                continue;
            }

            logger.printfln("Recursing in directory %s. Depth left %d.", root_name.c_str(), levels - 1);
            toFS.mkdir(source.name());

            if (!mirror_filesystem(fromFS, toFS, String(source.name()) + "/", levels - 1, buf, buf_size)) {
                return false;
            }

            continue;
        }

        File target = toFS.open(root_name + source.name(), FILE_WRITE);

        while (source.available()) {
            size_t read = source.read(buf, buf_size);
            size_t written = target.write(buf, read);

            if (written != read) {
                logger.printfln("Failed to write file %s: written %u read %u", target.name(), written, read);
                return false;
            }
        }
    }

    return true;
}

bool mount_or_format_spiffs(void)
{
    /*
    tl;dr:
    - Config partition mountable as SPIFFS?
    - Format core dump partition
    - Copy configuration from config to core dump partition
    - Core dump partition mountable as LittleFS?
    - Format config partition
    - Copy configuration from core dump to config partition
    - Erase core dump partition
    - Mount config partition, format on fail -> Done
    */

    // If we still have an SPIFFS, copy the configuration into the "backup" slot, i.e. the core dump partition
    // Format the core dump partition in any case, maybe we where interrupted while copying the last time.
    if (is_spiffs_available("spiffs", "/spiffs")) {
        logger.printfln("Configuration partition is mountable as SPIFFS. Migrating to LittleFS.");

        logger.printfln("Formatting core dump partition as LittleFS.");
        {
            LogSilencer ls{"esp_littlefs"};
            LogSilencer ls2{"ARDUINO"};
            LittleFS.begin(false, "/conf_backup", 10, "coredump");
        }
        LittleFS.format();
        LittleFS.begin(false, "/conf_backup", 10, "coredump");

        logger.printfln("Mirroring configuration to core dump partition.");
        SPIFFS.begin(false);
        uint8_t *buf = (uint8_t *)malloc(4096);
        mirror_filesystem(SPIFFS, LittleFS, "/", 4, buf, 4096);
        free(buf);
        SPIFFS.end();
        LittleFS.end();
    }

    // If we have a configuration backup in the core dump partition, we either are currently migrating, or
    // where interrupted while doing so. Format the configuration partition in any case
    // (maybe we copied only half the config last time). Then copy over the configuration backup files
    // and erase the backup completely.
    if (is_littlefs_available("coredump", "/conf_backup")) {
        logger.printfln("Core dump partition is mountable as LittleFS, configuration backup found. Continuing migration.");

        logger.printfln("Formatting configuration partition as LittleFS.");
        {
            LogSilencer ls{"esp_littlefs"};
            LogSilencer ls2{"ARDUINO"};
            LittleFS.begin(false, "/spiffs", 10, "spiffs");
        }
        LittleFS.format();
        LittleFS.begin(false, "/spiffs", 10, "spiffs");

        logger.printfln("Mirroring configuration backup to configuration partition.");
        fs::LittleFSFS configFS;
        configFS.begin(false, "/conf_backup", 10, "coredump");
        uint8_t *buf = (uint8_t *)malloc(4096);
        mirror_filesystem(configFS, LittleFS, "/", 4, buf, 4096);
        free(buf);
        configFS.end();
        LittleFS.end();

        logger.printfln("Erasing core dump partition.");
        auto _partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, "coredump");
        ESP.partitionEraseRange(_partition, 0, 0x10000);
        logger.printfln("Migration done!");
    }

    if (!is_littlefs_available("spiffs", "/spiffs")) {
        logger.printfln("Configuration partition is not mountable as LittleFS. Formatting now.");
        {
            LogSilencer ls{"esp_littlefs"};
            LittleFS.begin(false, "/spiffs", 10, "spiffs");
        }
        LittleFS.format();
        logger.printfln("Configuration partition is now formatted as LittleFS.");
    }

    if (!LittleFS.begin(false, "/spiffs", 10, "spiffs")) {
        return false;
    }

    size_t part_size = LittleFS.totalBytes();
    size_t part_used = LittleFS.usedBytes();
    logger.printfln("Mounted configuration partition. %u of %u bytes (%3.1f %%) used", part_used, part_size, ((float)part_used / (float)part_size) * 100.0f);

    return true;
}

String read_or_write_config_version(const char *firmware_version)
{
    if (LittleFS.exists("/config/version")) {
        const size_t capacity = JSON_OBJECT_SIZE(1) + 60;
        StaticJsonDocument<capacity> doc;
        File file = LittleFS.open("/config/version", "r");

        deserializeJson(doc, file);
        file.close();

        return doc["spiffs"].as<const char *>();
    } else {
        File file = LittleFS.open("/config/version", "w");

        file.printf("{\"spiffs\": \"%s\"}", firmware_version);
        file.close();

        return firmware_version;
    }
}

static bool wait_for_bootloader_mode(TF_Unknown *bricklet, int target_mode)
{
    uint8_t mode = 255;

    for (int i = 0; i < 10; ++i) {
        if (tf_unknown_get_bootloader_mode(bricklet, &mode) != TF_E_OK) {
            continue;
        }

        if (mode == target_mode) {
            break;
        }

        delay(250);
    }

    return mode == target_mode;
}

static bool flash_plugin(TF_Unknown *bricklet, const uint8_t *firmware, size_t firmware_len, int regular_plugin_upto, EventLog *logger)
{
    logger->printfln("    Setting bootloader mode to bootloader.");
    tf_unknown_set_bootloader_mode(bricklet, 0, nullptr);
    logger->printfln("    Waiting for bootloader...");

    if (!wait_for_bootloader_mode(bricklet, 0)) {
        logger->printfln("    Timed out, flashing failed");
        return false;
    }

    logger->printfln("    Device is in bootloader, flashing...");

    int num_packets = firmware_len / 64;
    int last_packet = 0;
    bool write_footer = false;

    if (regular_plugin_upto >= firmware_len - 64 * 4) {
        last_packet = num_packets;
    } else {
        last_packet = ((regular_plugin_upto / 256) + 1) * 4;
        write_footer = true;
    }

    for (int position = 0; position < last_packet; ++position) {
        int start = position * 64;

        if (tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
            if (tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                logger->printfln("    Failed to set firmware pointer to %d", start);
                return false;
            }
        }

        if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
            if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                logger->printfln("    Failed to write firmware at %d", start);
                return false;
            }
        }
    }

    if (write_footer) {
        for (int position = num_packets - 4; position < num_packets; ++position) {
            int start = position * 64;

            if (tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                if (tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                    logger->printfln("    (Footer) Failed to set firmware pointer to %d", start);
                    return false;
                }
            }

            if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                    logger->printfln("    (Footer) Failed to write firmware at %d", start);
                    return false;
                }
            }
        }
    }

    logger->printfln("    Device flashed successfully.");

    return true;
}

static bool flash_firmware(TF_Unknown *bricklet, const uint8_t *firmware, size_t firmware_len, EventLog *logger)
{
    int regular_plugin_upto = -1;

    for (int i = firmware_len - 13; i >= 4; --i) {
        if (firmware[i] == 0x12
         && firmware[i - 1] == 0x34
         && firmware[i - 2] == 0x56
         && firmware[i - 3] == 0x78) {
             regular_plugin_upto = i;
             break;
         }
    }

    if (regular_plugin_upto == -1) {
        logger->printfln("    Firmware end marker not found. Is this a valid firmware?");
        return false;
    }

    if (!flash_plugin(bricklet, firmware, firmware_len, regular_plugin_upto, logger)) {
        return false;
    }

    logger->printfln("    Setting bootloader mode to firmware.");

    uint8_t ret_status = 0;

    tf_unknown_set_bootloader_mode(bricklet, 1, &ret_status);

    if (ret_status != 0 && ret_status != 2) {
        logger->printfln("    Failed to set bootloader mode to firmware. status %d.", ret_status);

        if (ret_status != 5) {
            return false;
        }

        logger->printfln("    Status is 5, retrying.");

        if (!flash_plugin(bricklet, firmware, firmware_len, regular_plugin_upto, logger)) {
            return false;
        }

        ret_status = 0;

        logger->printfln("    Setting bootloader mode to firmware.");
        tf_unknown_set_bootloader_mode(bricklet, 1, &ret_status);

        if (ret_status != 0 && ret_status != 2) {
            logger->printfln("    (Second attempt) Failed to set bootloader mode to firmware. status %d.", ret_status);
            return false;
        }
    }

    logger->printfln("    Waiting for firmware...");

    if (!wait_for_bootloader_mode(bricklet, 1)) {
        logger->printfln("    Timed out, flashing failed");
        return false;
    }

    logger->printfln("    Firmware flashed successfully");

    return true;
}

#define FIRMWARE_MAJOR_OFFSET 10
#define FIRMWARE_MINOR_OFFSET 11
#define FIRMWARE_PATCH_OFFSET 12


class TFPSwap {
public:
    TFPSwap(TF_TFP *tfp) :
        tfp(tfp),
        device(tfp->device),
        cb_handler(tfp->cb_handler)
    {
        tfp->device = nullptr;
        tfp->cb_handler = nullptr;
    }

    ~TFPSwap()
    {
        tfp->device = device;
        tfp->cb_handler = cb_handler;
    }

private:
    TF_TFP *tfp;
    void *device;
    TF_TFP_CallbackHandler cb_handler;
};

int ensure_matching_firmware(TF_TFP *tfp, const char *name, const char *purpose, const uint8_t *firmware, size_t firmware_len, EventLog *logger, bool force)
{
    TFPSwap tfp_swap(tfp);
    TF_Unknown bricklet;

    int rc = tf_unknown_create(&bricklet, tfp);

    if (rc != TF_E_OK) {
        logger->printfln("%s init failed (rc %d). Disabling %s support.", name, rc, purpose);
        return -1;
    }

    uint8_t firmware_version[3] = {0};

    rc = tf_unknown_get_identity(&bricklet, nullptr, nullptr, nullptr, nullptr, firmware_version, nullptr);

    if (rc != TF_E_OK) {
        logger->printfln("%s get identity failed (rc %d). Disabling %s support.", name, rc, purpose);
        return -1;
    }

    uint8_t embedded_firmware_version[3] = {
        firmware[firmware_len - FIRMWARE_MAJOR_OFFSET],
        firmware[firmware_len - FIRMWARE_MINOR_OFFSET],
        firmware[firmware_len - FIRMWARE_PATCH_OFFSET],
    };

    bool flash_required = force;

    for (int i = 0; i < 3; ++i) {
        // Intentionally use != here: we also want to downgrade the bricklet firmware if the ESP firmware embeds an older one.
        // This makes sure, that the interfaces fit.
        flash_required |= firmware_version[i] != embedded_firmware_version[i];
    }

    if (flash_required) {
        if (force) {
            logger->printfln("Forcing %s firmware update to %d.%d.%d. Flashing firmware...",
                             name,
                             embedded_firmware_version[0], embedded_firmware_version[1], embedded_firmware_version[2]);
        } else {
            logger->printfln("%s firmware is %d.%d.%d not the expected %d.%d.%d. Flashing firmware...",
                             name,
                             firmware_version[0], firmware_version[1], firmware_version[2],
                             embedded_firmware_version[0], embedded_firmware_version[1], embedded_firmware_version[2]);
        }

        if (!flash_firmware(&bricklet, firmware, firmware_len, logger)) {
            logger->printfln("%s flashing failed. Disabling %s support.", name, purpose);
            return -1;
        }
    }

    tf_unknown_destroy(&bricklet);

    return 0;
}


int compare_version(uint8_t left_major, uint8_t left_minor, uint8_t left_patch,
                    uint8_t right_major, uint8_t right_minor, uint8_t right_patch) {
    if (left_major > right_major)
        return 1;

    if (left_major < right_major)
        return -1;

    if (left_minor > right_minor)
        return 1;

    if (left_minor < right_minor)
        return -1;

    if (left_patch > right_patch)
        return 1;

    if (left_patch < right_patch)
        return -1;

    return 0;
}

bool clock_synced(struct timeval *out_tv_now) {
    gettimeofday(out_tv_now, nullptr);
    return out_tv_now->tv_sec > ((2016 - 1970) * 365 * 24  * 60 * 60);
}

bool for_file_in(const char *dir, bool (*callback)(File *open_file), bool skip_directories) {
    File root = LittleFS.open(dir);
    File file;
    while(file = root.openNextFile()){
        if(skip_directories && file.isDirectory()){
            continue;
        }
        if (!callback(&file))
            return false;
    }
    return true;
}

void remove_directory(const char *path) {
    // This is more involved than expected:
    // rmdir only deletes empty directories, so remove all files first
    // Also LittleFS.rmdir will call the vfs_api.cpp implementation that
    // helpfully checks the mountpoint's name. If it is
    // "/spiffs", the directory will not be deleted, but instead
    // "rmdir is unnecessary in SPIFFS" is printed. However our mountpoint
    // is only called /spiffs for historical reasons, we use
    // LittleFS instead. Calling ::rmdir directly bypasses
    // this and other helpful checks.
    for_file_in(path, [](File *f) {
            String file_path = f->path();
            f->close();
            LittleFS.remove(file_path);
            return true;
        });

    ::rmdir((String("/spiffs/") + path).c_str());
}
