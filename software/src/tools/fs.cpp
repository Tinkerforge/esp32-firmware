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
#include <sys/stat.h>
#include <esp_partition.h>

#include "fs.h"

#include "event_log_prefix.h"
#include "main_dependencies.h" // for logger
#include "options.h"
#include "tools.h" // for LogSilencer

#include "gcc_warnings.h"

static constexpr const char *DATA_MOUNTPOINT = "/data";

#if OPTIONS_MIN_DATA_PARTITION_FORMAT() == 0
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

    return err == ESP_OK;
}
#endif

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
static bool mirror_filesystem(fs::FS &fromFS, fs::FS &toFS, const String &root_name, int levels, uint8_t *buf, size_t buf_size)
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

            if (!mirror_filesystem(fromFS, toFS, StringSumHelper(source.name()) + "/", levels - 1, buf, buf_size)) {
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

#if OPTIONS_MIN_DATA_PARTITION_FORMAT() == 0
static void try_convert_spiffs(const char *partition_label)
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
    if (is_spiffs_available(partition_label, DATA_MOUNTPOINT)) {
        logger.printfln("Data partition is mountable as SPIFFS. Migrating to LittleFS.");

        logger.printfln("Formatting core dump partition as LittleFS.");
        {
            LogSilencer ls;
            LittleFS.begin(false, "/conf_backup", 10, "coredump");
        }
        LittleFS.format();
        LittleFS.begin(false, "/conf_backup", 10, "coredump");

        logger.printfln("Mirroring data to core dump partition.");
        SPIFFS.begin(false, "/spiffs", 10, partition_label);
        const size_t buffer_size = 4096;
        uint8_t *buf = static_cast<decltype(buf)>(malloc(buffer_size));
        mirror_filesystem(SPIFFS, LittleFS, "/", 4, buf, buffer_size);
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
            LittleFS.begin(false, DATA_MOUNTPOINT, 10, partition_label);
        }
        LittleFS.format();
        should_factory_reset_bricklets = true;
        LittleFS.begin(false, DATA_MOUNTPOINT, 10, partition_label);

        logger.printfln("Mirroring data backup to data partition.");
        fs::LittleFSFS configFS;
        configFS.begin(false, "/conf_backup", 10, "coredump");
        const size_t buffer_size = 4096;
        uint8_t *buf = static_cast<decltype(buf)>(malloc(buffer_size));
        mirror_filesystem(configFS, LittleFS, "/", 4, buf, buffer_size);
        free(buf);
        configFS.end();
        LittleFS.end();

        logger.printfln("Erasing core dump partition.");
        auto _partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, "coredump");
        esp_partition_erase_range(_partition, 0, 0x10000);
        logger.printfln("Migration done!");
    }
}
#endif

static bool mount_or_format_littlefs(const char *partition_label)
{
    size_t format_attempts = 2;

    do {
        if (LittleFS.begin(false, DATA_MOUNTPOINT, 10, partition_label)) {
            break;
        }

        if (format_attempts == 0) {
            return false;
        }

        format_attempts--;

        // The is_littlefs_available is probably not necessary anymore.
        if (!is_littlefs_available(partition_label, DATA_MOUNTPOINT)) {
            logger.printfln("Data partition is not mountable as LittleFS. Formatting now.");
            {
                LogSilencer ls;
                LittleFS.begin(false, DATA_MOUNTPOINT, 10, partition_label);
            }
            if (LittleFS.format()) {
                logger.printfln("Data partition is now formatted as LittleFS");
            } else {
                logger.printfln("Data partition could not be formatted as LittleFS");
            }
            should_factory_reset_bricklets = true;
        }
    } while (true);

    size_t part_size = LittleFS.totalBytes();
    size_t part_used = LittleFS.usedBytes();
    logger.printfln("Mounted data partition: %u of %u bytes (%3.1f %%) used", part_used, part_size, static_cast<double>(static_cast<float>(100 * part_used) / static_cast<float>(part_size)));

    return true;
}

const char *data_partition_candidates[] = {
#if OPTIONS_MIN_DATA_PARTITION_FORMAT() <= 1
    "spiffs",
#endif
    "data",
};

static const char *find_data_partition_label()
{
    for (size_t i = 0; i < std::size(data_partition_candidates); i++) {
        const char *label = data_partition_candidates[i];
        const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, label);

        if (partition != nullptr) {
            return label;
        }
    }

    return nullptr;
}

bool mount_or_format_data_partition()
{
    const char *partition_label = find_data_partition_label();

    if (partition_label == nullptr) {
        logger.printfln("Couldn't find data partition by label");

        return false;
    }

#if OPTIONS_MIN_DATA_PARTITION_FORMAT() == 0
    const char *spiffs_label = "spiffs";
    if (partition_label == spiffs_label) { // Rely on string de-duplication
        try_convert_spiffs(partition_label);
    }
#endif

    return mount_or_format_littlefs(partition_label);
}

struct directory_entry {
    bool is_dir;
    char *path;
    void *data;
    size_t data_len;
    directory_entry *next;
};

static bool rewrite_recursor_reader(const char *mountpoint, const char *dir, directory_entry ***last_entry) {
    const size_t mountpoint_strlen = strlen(mountpoint);

    File root = LittleFS.open(dir);

    while (true) {
        bool is_dir;
        String path = root.getNextFileName(&is_dir);
        if (path.isEmpty()) {
            return true;
        }

        directory_entry *new_entry = static_cast<decltype(new_entry)>(malloc_psram_or_dram(sizeof(*new_entry)));
        new_entry->is_dir = is_dir;
        new_entry->next = nullptr;

        size_t pathlen = path.length();
        new_entry->path = static_cast<char *>(malloc_psram_or_dram(mountpoint_strlen + pathlen + 1));
        memcpy(new_entry->path, mountpoint, mountpoint_strlen);
        memcpy(new_entry->path + mountpoint_strlen, path.c_str(), pathlen);
        new_entry->path[mountpoint_strlen + pathlen] = 0;

        **last_entry = new_entry;
        *last_entry = &new_entry->next;

        if (is_dir) {
            new_entry->data = nullptr;
            new_entry->data_len = 0;

            logger.printfln("Entering directory %s", new_entry->path);

            if (!rewrite_recursor_reader(mountpoint, path.c_str(), last_entry)) {
                return false;
            }
        } else {
            int fd = open(new_entry->path, O_RDONLY);

            if (fd < 0) {
                logger.printfln("Error opening file '%s' for reading: %s (%i)", path.c_str(), strerror(errno), errno);
                new_entry->data = nullptr;
                new_entry->data_len = 0;
            } else {
                struct stat statbuf;
                if (fstat(fd, &statbuf) != 0) {
                    new_entry->data = nullptr;
                    new_entry->data_len = 0;
                } else {
                    new_entry->data_len = static_cast<size_t>(statbuf.st_size);
                    new_entry->data = malloc_psram_or_dram(new_entry->data_len);

                    int readbytes = read(fd, new_entry->data, new_entry->data_len);
                    if (static_cast<size_t>(readbytes) != new_entry->data_len) {
                        logger.printfln("Error reading file '%s': %s (%i)", path.c_str(), strerror(errno), errno);
                        free(new_entry->data);
                        new_entry->data = nullptr;
                        new_entry->data_len = 0;
                    } else {
                        logger.printfln("Read %zu bytes from file %s", new_entry->data_len, path.c_str());
                    }
                }

                close(fd);
            }
        }
    }
};

bool rewrite_data_partition(const std::function<bool(void)> &unmounted_task_fn)
{

    directory_entry *entries = nullptr;
    directory_entry **last_entry = &entries;

    if (!rewrite_recursor_reader(LittleFS.mountpoint(), "/", &last_entry)) {
        return false;
    }

    LittleFS.end();

    bool task_successful = unmounted_task_fn();

    if (!task_successful) {
        logger.printfln("Unmounted task function failed");
    }

    logger.printfln("Remounting data partition");
    if (!mount_or_format_data_partition()) {
        logger.printfln("Failed to remount data partition");
        task_successful = false;
    }

    while (entries != nullptr) {
        directory_entry *current = entries;
        entries = entries->next;

        current->next = nullptr;

        if (task_successful) {
            if (current->is_dir) {
                if (mkdir(current->path, 0777) != 0) {
                    logger.printfln("Error creating directory '%s': %s (%i)", current->path, strerror(errno), errno);
                } else {
                    logger.printfln("Created directory %s", current->path);
                }
            } else if (current->data_len > 0) {
                int fd = open(current->path, O_WRONLY | O_CREAT);

                if (fd < 0) {
                    logger.printfln("Error opening file '%s' for writing: %s (%i)", current->path, strerror(errno), errno);
                } else {
                    int written = write(fd, current->data, current->data_len);

                    if (static_cast<size_t>(written) != current->data_len) {
                        logger.printfln("Error writing file '%s': %s (%i)", current->path, strerror(errno), errno);
                    } else {
                        logger.printfln("Wrote %zu bytes to file %s", current->data_len, current->path);
                    }

                    close(fd);
                }
            } else {
                logger.printfln("Not restoring '%s'", current->path);
            }
        }

        free(current->path);
        current->path = nullptr;

        if (current->data != nullptr) {
            free(current->data);
            current->data = nullptr;
        }

        free(current);
    }

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

bool for_filename_in(const char *dir, std::function<bool(const char *, size_t, bool)> callback)
{
    File root = LittleFS.open(dir);

    // getNextFile**Name** obviously returns the file's full path m(
    // This is still faster than opening each file.
    size_t skip = strlen(dir);
    if (skip > 1)
        ++skip; // if this is not the root directory, skip the trailing /

    String filename;
    bool is_dir;
    while (!(filename = root.getNextFileName(&is_dir)).isEmpty()) {
        if (!callback(filename.c_str() + skip, filename.length() - skip, is_dir))
            return false;
    }
    return true;
}

bool for_filename_in(const char *dir, std::function<bool(const String &, bool)> callback)
{
    File root = LittleFS.open(dir);

    // getNextFile**Name** obviously returns the file's full path m(
    // This is still faster than opening each file.
    size_t skip = strlen(dir) + 1; // also skip trailing /
    String filename;
    bool is_dir;
    while (!(filename = root.getNextFileName(&is_dir)).isEmpty()) {
        if (!callback(filename.substring(skip), is_dir))
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
    // The mountpoint has since been renamed to /data, but the above note
    // has been left here to avoid SPIFFS-related pitfalls.
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

    ::rmdir((DATA_MOUNTPOINT + path_string).c_str());
}

static int local_stat(struct stat *st, fs::LittleFSFS *file_system, const char *path)
{
    const char *mountpoint = file_system->mountpoint();
    const size_t mp_len   = strlen(mountpoint);
    const size_t path_len = strlen(path);
    const size_t full_len = mp_len + path_len;

    char local_path[256];
    char *full_path;

    if (full_len < sizeof(local_path)) { // less-than because of termination
        full_path = local_path;
    } else {
        full_path = static_cast<decltype(full_path)>(malloc(full_len + 1));
    }

    memcpy(full_path, mountpoint, mp_len);
    memcpy(full_path + mp_len, path, path_len + 1); // Include termination

    const int ret = stat(full_path, st);

    if (full_path != local_path) {
        const int errno_save = errno;

        free(full_path);

        errno = errno_save;
    }

    return ret;
}

ssize_t file_size(fs::LittleFSFS &file_system, const char *path)
{
    struct stat st;

    if (local_stat(&st, &file_system, path) != 0) {
        return -1;
    }

    return static_cast<ssize_t>(st.st_size);
}

ssize_t file_size(fs::LittleFSFS &file_system, const String &path) {
    return file_size(file_system, path.c_str());
}

bool file_exists(fs::LittleFSFS &file_system, const char *path)
{
    struct stat st;

    if (local_stat(&st, &file_system, path) != 0) {
        return false;
    }

    return (st.st_mode & S_IFMT) == S_IFREG; // Check if path points to a regular file.
}

bool file_exists(fs::LittleFSFS &file_system, const String &path) {
    return file_exists(file_system, path.c_str());
}
