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

#include <Arduino.h>
#include <SPIFFS.h>
#include <esp_spiffs.h>
#include <LittleFS.h>
#include <esp_littlefs.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <freertos/task.h>
#include <lwip/udp.h>
#include <esp_log.h>
#include <soc/efuse_reg.h>
#include <arpa/inet.h>

#include "event_log_prefix.h"
#include "main_dependencies.h"
#include "bindings/errors.h"
#include "bindings/base58.h"
#include "bindings/bricklet_unknown.h"
#include "build.h"

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

// implement TFTools/Micros.h now_us
micros_t now_us()
{
    return micros_t{esp_timer_get_time()};
}

// implement TFTools/Tools.h now_us
[[gnu::noreturn]] void system_abort(const char *message)
{
    esp_system_abort(message);
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

        if (strnlen(buf, ARRAY_SIZE(buf)) != 4) {
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

int vprintf_dev_null(const char *fmt, va_list args)
{
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

    esp_vfs_littlefs_conf_t conf;
    memset(&conf, 0, sizeof(conf));
    conf.base_path = base_path;
    conf.partition_label = part_label;
    conf.format_if_mount_failed = false;

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

bool should_factory_reset_bricklets = false;

bool mount_or_format_spiffs()
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
        should_factory_reset_bricklets = true;
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
        should_factory_reset_bricklets = true;
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

static bool flash_plugin(TF_Unknown *bricklet, const uint8_t *firmware, size_t firmware_len, int regular_plugin_upto)
{
    logger.printfln_continue("Setting bootloader mode to bootloader.");
    tf_unknown_set_bootloader_mode(bricklet, 0, nullptr);
    logger.printfln_continue("Waiting for bootloader...");

    if (!wait_for_bootloader_mode(bricklet, 0)) {
        logger.printfln_continue("Timed out, flashing failed");
        return false;
    }

    logger.printfln_continue("Device is in bootloader, flashing...");

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
                logger.printfln_continue("Failed to set firmware pointer to %d", start);
                return false;
            }
        }

        if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
            if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                logger.printfln_continue("Failed to write firmware at %d", start);
                return false;
            }
        }
    }

    if (write_footer) {
        for (int position = num_packets - 4; position < num_packets; ++position) {
            int start = position * 64;

            if (tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                if (tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                    logger.printfln_continue("(Footer) Failed to set firmware pointer to %d", start);
                    return false;
                }
            }

            if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                    logger.printfln_continue("(Footer) Failed to write firmware at %d", start);
                    return false;
                }
            }
        }
    }

    logger.printfln_continue("Device flashed successfully.");

    return true;
}

static bool flash_firmware(TF_Unknown *bricklet, const uint8_t *firmware, size_t firmware_len)
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
        logger.printfln_continue("Firmware end marker not found. Is this a valid firmware?");
        return false;
    }

    if (!flash_plugin(bricklet, firmware, firmware_len, regular_plugin_upto)) {
        return false;
    }

    logger.printfln_continue("Setting bootloader mode to firmware.");

    uint8_t ret_status = 0;

    tf_unknown_set_bootloader_mode(bricklet, 1, &ret_status);

    if (ret_status != 0 && ret_status != 2) {
        logger.printfln_continue("Failed to set bootloader mode to firmware. status %d.", ret_status);

        if (ret_status != 5) {
            return false;
        }

        logger.printfln_continue("Status is 5, retrying.");

        if (!flash_plugin(bricklet, firmware, firmware_len, firmware_len)) {
            return false;
        }

        ret_status = 0;

        logger.printfln_continue("Setting bootloader mode to firmware.");
        tf_unknown_set_bootloader_mode(bricklet, 1, &ret_status);

        if (ret_status != 0 && ret_status != 2) {
            logger.printfln_continue("(Second attempt) Failed to set bootloader mode to firmware. status %d.", ret_status);
            return false;
        }
    }

    logger.printfln_continue("Waiting for firmware...");

    if (!wait_for_bootloader_mode(bricklet, 1)) {
        logger.printfln_continue("Timed out, flashing failed");
        return false;
    }

    logger.printfln_continue("Firmware flashed successfully");

    return true;
}

#define FIRMWARE_MAJOR_OFFSET 10
#define FIRMWARE_MINOR_OFFSET 11
#define FIRMWARE_PATCH_OFFSET 12

int ensure_matching_firmware(TF_TFP *tfp, const char *name, const char *purpose, const uint8_t *firmware, size_t firmware_len, bool force)
{
    TFPSwap tfp_swap(tfp);
    TF_Unknown bricklet;
    auto old_timeout = tf_hal_get_timeout(&hal);
    defer {tf_hal_set_timeout(&hal, old_timeout);};
    tf_hal_set_timeout(&hal, 2500 * 1000);


    int rc = tf_unknown_create(&bricklet, tfp);
    defer {tf_unknown_destroy(&bricklet);};

    if (rc != TF_E_OK) {
        logger.printfln("%s init failed (rc %d).", name, rc);
        return -1;
    }

    uint8_t firmware_version[3] = {0};

    rc = tf_unknown_get_identity(&bricklet, nullptr, nullptr, nullptr, nullptr, firmware_version, nullptr);

    if (rc != TF_E_OK) {
        logger.printfln("%s get identity failed (rc %d).", name, rc);
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
            logger.printfln("Forcing %s firmware update to %d.%d.%d. Flashing firmware...",
                             name,
                             embedded_firmware_version[0], embedded_firmware_version[1], embedded_firmware_version[2]);
        } else {
            logger.printfln("%s firmware is %d.%d.%d not the expected %d.%d.%d. Flashing firmware...",
                             name,
                             firmware_version[0], firmware_version[1], firmware_version[2],
                             embedded_firmware_version[0], embedded_firmware_version[1], embedded_firmware_version[2]);
        }

        if (!flash_firmware(&bricklet, firmware, firmware_len)) {
            logger.printfln("%s flashing failed.", name);
            return -1;
        }
    }

    return 0;
}

int compare_version(uint8_t left_major, uint8_t left_minor, uint8_t left_patch, uint8_t left_beta /* 255 == no beta */, uint32_t left_timestamp,
                    uint8_t right_major, uint8_t right_minor, uint8_t right_patch, uint8_t right_beta /* 255 == no beta */, uint32_t right_timestamp) {
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

    if (left_beta == 255 && right_beta != 255)
        return 1;

    if (left_beta != 255 && right_beta == 255)
        return -1;

    if (left_beta > right_beta)
        return 1;

    if (left_beta < right_beta)
        return -1;

    if (left_timestamp > right_timestamp)
        return 1;

    if (left_timestamp < right_timestamp)
        return -1;

    return 0;
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

    ::rmdir(("/spiffs" + path_string).c_str());
}

bool is_in_subnet(IPAddress ip, IPAddress subnet, IPAddress to_check)
{
    return (((uint32_t)ip) & ((uint32_t)subnet)) == (((uint32_t)to_check) & ((uint32_t)subnet));
}

bool is_valid_subnet_mask(IPAddress subnet)
{
    bool zero_seen = false;
    // IPAddress is in network byte order!
    uint32_t addr = ntohl((uint32_t)subnet);
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

TaskHandle_t mainTaskHandle;
void set_main_task_handle()
{
    mainTaskHandle = xTaskGetCurrentTaskHandle();
}

void led_blink(int8_t led_pin, int interval, int blinks_per_interval, int off_time_ms)
{
    if (led_pin < 0)
        return;

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

uint16_t internet_checksum(const uint8_t *data, size_t length)
{
    uint32_t checksum = 0xffff;

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

struct gethostbyname_parameters {
    const char *hostname;
    ip_addr_t *addr;
    dns_found_callback found_callback;
    void *callback_arg;
    u8_t dns_addrtype;
};

static esp_err_t gethostbyname_lwip_ctx(void *ctx)
{
    gethostbyname_parameters *parameters = static_cast<gethostbyname_parameters *>(ctx);
    return dns_gethostbyname(parameters->hostname, parameters->addr, parameters->found_callback, parameters->callback_arg);
}

err_t dns_gethostbyname_lwip_ctx(const char *hostname, ip_addr_t *addr, dns_found_callback found_callback, void *callback_arg)
{
    gethostbyname_parameters parameters;
    parameters.hostname = hostname;
    parameters.addr = addr;
    parameters.found_callback = found_callback;
    parameters.callback_arg = callback_arg;

    return esp_netif_tcpip_exec(gethostbyname_lwip_ctx, &parameters);
}

static esp_err_t gethostbyname_addrtype_lwip_ctx(void *ctx)
{
    gethostbyname_parameters *parameters = static_cast<gethostbyname_parameters *>(ctx);
    return dns_gethostbyname_addrtype(parameters->hostname, parameters->addr, parameters->found_callback, parameters->callback_arg, parameters->dns_addrtype);
}

err_t dns_gethostbyname_addrtype_lwip_ctx(const char *hostname, ip_addr_t *addr, dns_found_callback found_callback, void *callback_arg, u8_t dns_addrtype)
{
    gethostbyname_parameters parameters;
    parameters.hostname = hostname;
    parameters.addr = addr;
    parameters.found_callback = found_callback;
    parameters.callback_arg = callback_arg;
    parameters.dns_addrtype = dns_addrtype;

    return esp_netif_tcpip_exec(gethostbyname_addrtype_lwip_ctx, &parameters);
}

static void gethostbyname_addrtype_lwip_ctx_async(const char */*hostname*/, const ip_addr_t *addr, void *callback_arg)
{
    dns_gethostbyname_addrtype_lwip_ctx_async_data *data = static_cast<dns_gethostbyname_addrtype_lwip_ctx_async_data *>(callback_arg);

    data->err = ERR_OK; // ERR_OK because we got a response. Response might be negative and ipaddr a nullptr, though.

    if (addr != nullptr) {
        data->addr = *addr;
        data->addr_ptr = &data->addr;
    }
    else {
        data->addr_ptr = nullptr;
    }

    task_scheduler.scheduleOnce([data]() {
        data->found_callback(data);
    });
}

void dns_gethostbyname_addrtype_lwip_ctx_async(const char *hostname,
                                               std::function<void(dns_gethostbyname_addrtype_lwip_ctx_async_data *callback_arg)> &&found_callback,
                                               dns_gethostbyname_addrtype_lwip_ctx_async_data *callback_arg,
                                               u8_t dns_addrtype)
{
    callback_arg->found_callback = std::move(found_callback);
    err_t err = dns_gethostbyname_addrtype_lwip_ctx(hostname, &callback_arg->addr, gethostbyname_addrtype_lwip_ctx_async, callback_arg, dns_addrtype);

    // Don't set the callback_arg's err if the result is not available yet.
    // The callback handler might be executed before dns_gethostbyname_addrtype_lwip_ctx returns.
    if (err == ERR_INPROGRESS)
        return;

    callback_arg->err = err;
    callback_arg->addr_ptr = &callback_arg->addr;

    callback_arg->found_callback(callback_arg); // Can't call local found_callback anymore because it has been std::forward'ed.
}

static esp_err_t poke_localhost_fn(void * /*ctx*/)
{
    udp_pcb *l_udp_pcb = udp_new();
    if (l_udp_pcb) {
        //udp_bind(l_udp_pcb, IP_ADDR_ANY, 0); // pcb will be bound in udp_sendto()

        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_ROM); // PBUF_ROM because we have no payload
        if (p) {
            p->payload = nullptr; // payload can be nullptr because length is 0 and pbuf type is PBUF_ROM
            p->len = 0;
            p->tot_len = 0;

            ip_addr_t dst_addr;
            dst_addr.type = IPADDR_TYPE_V4;
            dst_addr.u_addr.ip4.addr = htonl(IPADDR_LOOPBACK);

            errno = 0;
            err_t err = udp_sendto(l_udp_pcb,p, &dst_addr, 9);
            if (err != ERR_OK) {
                logger.printfln("udp_sendto failed: %i | %s (%i)", err, strerror(errno), errno);
            }

            pbuf_free(p);
        }
        udp_remove(l_udp_pcb);
    }
    return ESP_OK; // Don't care about errors.
}

void poke_localhost()
{
    esp_netif_tcpip_exec(poke_localhost_fn, nullptr);
}

void trigger_reboot(const char *initiator, millis_t delay_ms)
{
    logger.printfln("Reboot requested by %s.", initiator);
    task_scheduler.scheduleOnce([]() {
        ESP.restart();
    }, delay_ms);
}

void list_dir(fs::FS &fs, const char *dirname, uint8_t max_depth, uint8_t current_depth)
{
    File root = fs.open(dirname);
    if (!root) {
        logger.printfln("%*c%s/ - failed to open directory", current_depth * 4, ' ', root.name());
        return;
    }
    if (!root.isDirectory()) {
        logger.printfln("%*c%s/ - not a directory", current_depth * 4, ' ', root.name());
        return;
    }
    logger.printfln("%*c%s/", current_depth * 4, ' ', root.name());

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            if (max_depth) {
                list_dir(fs, file.path(), max_depth - 1, current_depth + 1);
            }
        } else {
            size_t prefix_len = (current_depth + 1) * 4 + strlen(file.name());
            time_t t = file.getLastWrite();
            struct tm *tmstruct = localtime(&t);

            logger.printfln("%*c%s%*c%d-%02d-%02d %02d:%02d:%02d    %u",
                            (current_depth + 1) * 4,
                            ' ',
                            file.name(),
                            prefix_len < 40 ? 40 - prefix_len : 4,
                            ' ',
                            tmstruct->tm_year + 1900,
                            tmstruct->tm_mon + 1,
                            tmstruct->tm_mday,
                            tmstruct->tm_hour,
                            tmstruct->tm_min,
                            tmstruct->tm_sec,
                            file.size());
        }
        file = root.openNextFile();
    }
}

time_t ms_until_datetime(int *year, int *month, int *day, int *hour, int *minutes, int *seconds)
{
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

time_t ms_until_time(int h, int m)
{
	int s = 0;
	time_t delay = ms_until_datetime(NULL, NULL, NULL, &h, &m, &s);
	if (delay <= 0) {
		int d = -1;
		delay = ms_until_datetime(NULL, NULL, &d, &h, &m, &s);
	}
	return delay;
}

size_t sprintf_u(char *buf, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = vsprintf(buf, fmt, args);
    va_end(args);

    return res < 0 ? 0 : static_cast<size_t>(res);
}

size_t snprintf_u(char *buf, size_t len, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int res = vsnprintf(buf, len, fmt, args);
    va_end(args);

    return res < 0 ? 0 : static_cast<size_t>(res);
}

size_t vsnprintf_u(char *buf, size_t len, const char *fmt, va_list args)
{
    int res = vsnprintf(buf, len, fmt, args);

    return res < 0 ? 0 : static_cast<size_t>(res);
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

OwnershipGuard::OwnershipGuard(Ownership *ownership, uint32_t owner_id) : ownership(ownership)
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

int remove_separator(const char *const in, char *out)
{
    int written = 0;
    size_t s = strlen(in);
    for (int i = 0; i < s; ++i) {
        if (in[i] == ':')
            continue;
        out[written] = in[i];
        ++written;
    }
    out[written] = '\0';
    return written;
}

int strncmp_with_same_len(const char *left, const char *right, size_t right_len)
{
    size_t left_len = strlen(left);
    if (left_len != right_len)
        return -1;
    return strncmp(left, right, right_len);
}

i2c_cmd_handle_t i2c_master_prepare_write_read_device(uint8_t device_address,
                                                      uint8_t *command_buffer, size_t command_buffer_size,
                                                      const uint8_t* write_buffer, size_t write_size,
                                                      uint8_t* read_buffer, size_t read_size) {
    bool write = write_buffer != nullptr && write_size > 0;
    bool read = read_buffer != nullptr && read_size > 0;
    if (!write && !read)
        return nullptr;

    esp_err_t err = ESP_OK;

    i2c_cmd_handle_t handle = i2c_cmd_link_create_static(command_buffer, command_buffer_size);
    assert(handle != NULL);

    if (write) {
        err = i2c_master_start(handle);
        if (err != ESP_OK) {
            goto error;
        }

        err = i2c_master_write_byte(handle, device_address << 1 | I2C_MASTER_WRITE, true);
        if (err != ESP_OK) {
            goto error;
        }

        err = i2c_master_write(handle, write_buffer, write_size, true);
        if (err != ESP_OK) {
            goto error;
        }
    }

    if (read) {
        err = i2c_master_start(handle);
        if (err != ESP_OK) {
            goto error;
        }

        err = i2c_master_write_byte(handle, device_address << 1 | I2C_MASTER_READ, true);
        if (err != ESP_OK) {
            goto error;
        }

        err = i2c_master_read(handle, read_buffer, read_size, I2C_MASTER_LAST_NACK);
        if (err != ESP_OK) {
            goto error;
        }
    }

    err = i2c_master_stop(handle);
    if (err != ESP_OK) {
        goto error;
    }

    return handle;

error:
    i2c_cmd_link_delete_static(handle);
    return nullptr;
}

time_t get_localtime_midnight_in_utc(time_t timestamp)
{
    // Local time for timestamp
    struct tm *tm  = localtime(&timestamp);

    // Local time to today midnight
    tm->tm_hour  =  0;
    tm->tm_min   =  0;
    tm->tm_sec   =  0;
    tm->tm_isdst = -1; // isdst = -1 => let mktime figure out if DST is in effect

    // Return midnight in UTC
    return mktime(tm);
}

Option<time_t> get_localtime_today_midnight_in_utc()
{
    struct timeval tv;
    if (!rtc.clock_synced(&tv))
        return {};

    return get_localtime_midnight_in_utc(tv.tv_sec);
}

