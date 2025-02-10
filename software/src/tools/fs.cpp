/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <SPIFFS.h>
#include <esp_spiffs.h>
#include <LittleFS.h>
#include <esp_littlefs.h>

#include "fs.h"

#include "event_log_prefix.h"
#include "main_dependencies.h" // for logger
#include "tools.h" // for LogSilencer

static bool is_spiffs_available(const char *part_label, const char *base_path)
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

static bool is_littlefs_available(const char *part_label, const char *base_path)
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
static bool mirror_filesystem(fs::FS &fromFS, fs::FS &toFS, String root_name, int levels, uint8_t *buf, size_t buf_size)
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

    while ((source = root.openNextFile())) {
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

bool for_file_in(const char *dir, bool (*callback)(File *open_file), bool skip_directories)
{
    File root = LittleFS.open(dir);
    File file;
    while ((file = root.openNextFile())) {
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
