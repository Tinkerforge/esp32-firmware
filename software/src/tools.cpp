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
#include "esp_system.h"
#include "esp_timer.h"

#include <soc/efuse_reg.h>
#include "bindings/base58.h"
#include "bindings/bricklet_unknown.h"
#include "event_log.h"
#include "esp_log.h"
#include "build.h"
#include "task_scheduler.h"

#include <arpa/inet.h>

extern TF_HAL hal;

const char *tf_reset_reason()
{
    esp_reset_reason_t reason = esp_reset_reason();

    switch (reason) {
        case ESP_RST_POWERON:
            return "Reset due to power-on.";

        case ESP_RST_EXT:
            return "Reset by external pin.";

        case ESP_RST_SW:
            return "Software reset via esp_restart.";

        case ESP_RST_PANIC:
            return "Software reset due to exception/panic.";

        case ESP_RST_INT_WDT:
            return "Reset due to interrupt watchdog.";

        case ESP_RST_TASK_WDT:
            return "Reset due to task watchdog.";

        case ESP_RST_WDT:
            return "Reset due to some watchdog.";

        case ESP_RST_DEEPSLEEP:
            return "Reset after exiting deep sleep mode.";

        case ESP_RST_BROWNOUT:
            return "Brownout reset.";

        case ESP_RST_SDIO:
            return "Reset over SDIO.";

        default:
            return "Reset reason unknown.";
    }
}

bool a_after_b(uint32_t a, uint32_t b)
{
    return ((uint32_t)(a - b)) < (UINT32_MAX / 2);
}

bool deadline_elapsed(uint32_t deadline_ms)
{
    return a_after_b(millis(), deadline_ms);
}

micros_t operator""_usec(unsigned long long int i) {
    return micros_t{(int64_t)i};
}

micros_t now_us() {
    return micros_t{esp_timer_get_time()};
}

bool deadline_elapsed(micros_t deadline_us) {
    return deadline_us == 0_usec || deadline_us < now_us();
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

int vprintf_dev_null(const char *format, va_list ap) {
    return 0;
}

LogSilencer::LogSilencer() : old_fn(nullptr)
{
    old_fn = esp_log_set_vprintf(vprintf_dev_null);
}

LogSilencer::~LogSilencer()
{
    esp_log_set_vprintf(old_fn);
}

bool is_spiffs_available(const char *part_label, const char *base_path)
{
    LogSilencer ls;

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
    LogSilencer ls;

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
        logger.printfln("Data partition is mountable as SPIFFS. Migrating to LittleFS.");

        logger.printfln("Formatting core dump partition as LittleFS.");
        {
            LogSilencer ls;
            LittleFS.begin(false, "/conf_backup", 10, "coredump");
        }
        LittleFS.format();
        LittleFS.begin(false, "/conf_backup", 10, "coredump");

        logger.printfln("Mirroring data to core dump partition.");
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
        logger.printfln("Core dump partition is mountable as LittleFS, data backup found. Continuing migration.");

        logger.printfln("Formatting data partition as LittleFS.");
        {
            LogSilencer ls;
            LittleFS.begin(false, "/spiffs", 10, "spiffs");
        }
        LittleFS.format();
        LittleFS.begin(false, "/spiffs", 10, "spiffs");

        logger.printfln("Mirroring data backup to data partition.");
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
        logger.printfln("Data partition is not mountable as LittleFS. Formatting now.");
        {
            LogSilencer ls;
            LittleFS.begin(false, "/spiffs", 10, "spiffs");
        }
        LittleFS.format();
        logger.printfln("Data partition is now formatted as LittleFS.");
    }

    if (!LittleFS.begin(false, "/spiffs", 10, "spiffs")) {
        return false;
    }

    size_t part_size = LittleFS.totalBytes();
    size_t part_used = LittleFS.usedBytes();
    logger.printfln("Mounted data partition. %u of %u bytes (%3.1f %%) used", part_used, part_size, ((float)part_used / (float)part_size) * 100.0f);

    return true;
}

String read_config_version()
{
    if (LittleFS.exists("/config/version")) {
        StaticJsonDocument<JSON_OBJECT_SIZE(1) + 60> doc;
        File file = LittleFS.open("/config/version", "r");

        deserializeJson(doc, file);
        file.close();

        return doc["spiffs"].as<const char *>();
    }
    logger.printfln("Failed to read config version!");
    return BUILD_VERSION_STRING;
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

        if (!flash_plugin(bricklet, firmware, firmware_len, firmware_len, logger)) {
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

class TFPSwap
{
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
    auto old_timeout = tf_hal_get_timeout(&hal);
    defer {tf_hal_set_timeout(&hal, old_timeout);};
    tf_hal_set_timeout(&hal, 2500 * 1000);


    int rc = tf_unknown_create(&bricklet, tfp);
    defer {tf_unknown_destroy(&bricklet);};

    if (rc != TF_E_OK) {
        logger->printfln("%s init failed (rc %d).", name, rc);
        return -1;
    }

    uint8_t firmware_version[3] = {0};

    rc = tf_unknown_get_identity(&bricklet, nullptr, nullptr, nullptr, nullptr, firmware_version, nullptr);

    if (rc != TF_E_OK) {
        logger->printfln("%s get identity failed (rc %d).", name, rc);
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

    uint8_t mode;
    tf_unknown_get_bootloader_mode(&bricklet, &mode);
    flash_required |= mode != TF_UNKNOWN_BOOTLOADER_MODE_FIRMWARE;

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
            logger->printfln("%s flashing failed.", name);
            return -1;
        }
    }

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

#define BUILD_YEAR \
    ( \
        (__DATE__[ 7] - '0') * 1000 + \
        (__DATE__[ 8] - '0') *  100 + \
        (__DATE__[ 9] - '0') *   10 + \
        (__DATE__[10] - '0') \
    )

bool clock_synced(struct timeval *out_tv_now)
{
    gettimeofday(out_tv_now, nullptr);
    return out_tv_now->tv_sec > ((BUILD_YEAR - 1970) * 365 * 24 * 60 * 60);
}

uint32_t timestamp_minutes()
{
    struct timeval tv_now;

    if (!clock_synced(&tv_now))
        return 0;

    return tv_now.tv_sec / 60;
}

bool for_file_in(const char *dir, bool (*callback)(File *open_file), bool skip_directories)
{
    File root = LittleFS.open(dir);
    File file;
    while (file = root.openNextFile()) {
        if (skip_directories && file.isDirectory()) {
            continue;
        }
        if (!callback(&file))
            return false;
    }
    return true;
}

void remove_directory(const char *path)
{
    String path_string;
    if (*path != '/') {
        logger.printfln("Remove directory called with path %s that does not start with a /.", path);
        path_string = String("/") + path;
    } else {
        path_string = path;
    }

    // This is more involved than expected:
    // rmdir only deletes empty directories, so remove all files first
    // Also LittleFS.rmdir will call the vfs_api.cpp implementation that
    // helpfully checks the mountpoint's name. If it is
    // "/spiffs", the directory will not be deleted, but instead
    // "rmdir is unnecessary in SPIFFS" is printed. However our mountpoint
    // is only called /spiffs for historical reasons, we use
    // LittleFS instead. Calling ::rmdir directly bypasses
    // this and other helpful checks.
    for_file_in(path_string.c_str(), [](File *f) {
            bool dir = f->isDirectory();
            String file_path = String(f->path());
            // F will be closed after the callback returns.
            // However the recursive call below can potentially open
            // many files in parallel.
            // As we close the file before using the path, we have to
            // copy the path into a String. close() frees the buffer that
            // f->path() points to.
            f->close();
            if (dir)
                remove_directory(file_path.c_str());
            else
                LittleFS.remove(file_path.c_str());
            return true;
        }, false);

    ::rmdir((String("/spiffs") + path_string).c_str());
}


bool is_in_subnet(IPAddress ip, IPAddress subnet, IPAddress to_check) {
    return (((uint32_t)ip) & ((uint32_t)subnet)) == (((uint32_t)to_check) & ((uint32_t)subnet));
}

bool is_valid_subnet_mask(IPAddress subnet) {
    bool zero_seen = false;
    // IPAddress is in network byte order!
    uint32_t addr = ntohl((uint32_t) subnet);
    for (int i = 31; i >= 0; --i) {
        bool bit_is_one = (addr & (1 << i));
        if (zero_seen && bit_is_one) {
            return false;
        } else if (!zero_seen && !bit_is_one) {
            zero_seen = true;
        }
    }
    return true;
}

void led_blink(int8_t led_pin, int interval, int blinks_per_interval, int off_time_ms) {
    int t_in_second = millis() % interval;
    if (off_time_ms != 0 && (interval - t_in_second <= off_time_ms)) {
        digitalWrite(led_pin, 1);
        return;
    }

    // We want blinks_per_interval blinks and blinks_per_interval pauses between them. The off_time counts as pause.
    int state_count = ((2 * blinks_per_interval) - (off_time_ms != 0 ? 1 : 0));
    int state_interval = (interval - off_time_ms) / state_count;
    bool led = (t_in_second / state_interval) % 2 != 0;

    digitalWrite(led_pin, led);
}

uint16_t internet_checksum(const uint8_t* data, size_t length) {
    uint32_t checksum=0xffff;

    for (size_t i = 0; i < length - 1; i += 2) {
        uint16_t buf;
        memcpy(&buf, data + i, 2);
        checksum += buf;
    }

    uint32_t carry = checksum >> 16;
    checksum = (checksum & 0xFFFF) + carry;
    checksum = ~checksum;
    return checksum;
}

void trigger_reboot(const char *initiator)
{
    task_scheduler.scheduleOnce([initiator]() {
        logger.printfln("Reboot requested by %s.", initiator);
        delay(1500);
        ESP.restart();
    }, 0);
}

void list_dir(fs::FS &fs, const char * dirname, uint8_t max_depth, uint8_t current_depth) {
    File root = fs.open(dirname);
    if(!root) {
        logger.printfln("%*c%s/ - failed to open directory", current_depth * 4, ' ', root.name());
        return;
    }
    if(!root.isDirectory()) {
        logger.printfln("%*c%s/ - not a directory", current_depth * 4, ' ', root.name());
        return;
    }
    logger.printfln("%*c%s/", current_depth * 4, ' ', root.name());

    File file = root.openNextFile();
    while(file) {
        if(file.isDirectory()) {
            if(max_depth) {
                list_dir(fs, file.path(), max_depth - 1, current_depth + 1);
            }
        } else {
            size_t prefix_len = (current_depth + 1) * 4 + strlen(file.name());
            time_t t = file.getLastWrite();
            struct tm * tmstruct = localtime(&t);

            logger.printfln("%*c%s%*c%d-%02d-%02d %02d:%02d:%02d    %u",
                            (current_depth + 1) * 4,
                            ' ',
                            file.name(),
                            prefix_len < 40 ? 40 - prefix_len : 4,
                            ' ',
                            tmstruct->tm_year+1900,
                            tmstruct->tm_mon+1,
                            tmstruct->tm_mday,
                            tmstruct->tm_hour,
                            tmstruct->tm_min,
                            tmstruct->tm_sec,
                            file.size());
        }
        file = root.openNextFile();
    }
}

time_t ms_until_datetime(int *year, int *month, int *day, int *hour, int *minutes, int *seconds) {
	struct timeval tv;
	gettimeofday(&tv, NULL);

	struct tm datetime;
	localtime_r(&tv.tv_sec, &datetime);

	if (year)    datetime.tm_year = *year    < 0 ? datetime.tm_year - *year    : *year  - 1900;
	if (month)   datetime.tm_mon  = *month   < 0 ? datetime.tm_mon  - *month   : *month - 1;
	if (day)     datetime.tm_mday = *day     < 0 ? datetime.tm_mday - *day     : *day;
	if (hour)    datetime.tm_hour = *hour    < 0 ? datetime.tm_hour - *hour    : *hour;
	if (minutes) datetime.tm_min  = *minutes < 0 ? datetime.tm_min  - *minutes : *minutes;
	if (seconds) datetime.tm_sec  = *seconds < 0 ? datetime.tm_sec  - *seconds : *seconds;

	time_t ts = mktime(&datetime);

	return (ts - tv.tv_sec) * 1000 - tv.tv_usec / 1000;
}

time_t ms_until_time(int h, int m) {
	int s = 0;
	time_t delay = ms_until_datetime(NULL, NULL, NULL, &h, &m, &s);
	if (delay <= 0) {
		int d = -1;
		delay = ms_until_datetime(NULL, NULL, &d, &h, &m, &s);
	}
	return delay;
}

bool Ownership::try_acquire(uint32_t owner_id)
{
    mutex.lock();

    if (owner_id == current_owner_id) {
        return true;
    }

    mutex.unlock();

    return false;
}

void Ownership::release()
{
    mutex.unlock();
}

uint32_t Ownership::current()
{
    return current_owner_id;
}

uint32_t Ownership::next()
{
    mutex.lock();

    uint32_t owner_id = ++current_owner_id;

    mutex.unlock();

    return owner_id;
}

OwnershipGuard::OwnershipGuard(Ownership *ownership, uint32_t owner_id): ownership(ownership)
{
    acquired = ownership->try_acquire(owner_id);
}

OwnershipGuard::~OwnershipGuard()
{
    if (acquired) {
        ownership->release();
    }
}

bool OwnershipGuard::have_ownership()
{
    return acquired;
}
