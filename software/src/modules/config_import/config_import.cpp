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

#include "config_import.h"

#include <limits.h>
#include <stdlib.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "options.h"
#include "tools.h"
#include "tools/fs.h"
#include "tools/memory.h"

#include "gcc_warnings.h"

void ConfigImport::pre_setup()
{
    this->allow_path(API::CONFIG_DIRECTORY, 128, OPTIONS_API_JSON_MAX_LENGTH());
}

static void clean_up() {
    remove_directory("/config_import");
    remove_directory("/config_import_finished");
}

void ConfigImport::register_urls()
{
    server.on("/config_import/abort", HTTP_PUT, [this](WebServerRequest request) {
        clean_up();
        return request.send_plain(200);
    });

    server.on("/config_import/begin", HTTP_PUT, [this](WebServerRequest request) {
        String import_dir{"/config_import"};

        remove_directory(import_dir.c_str());
        LittleFS.mkdir(import_dir);
        LittleFS.mkdir(import_dir + API::CONFIG_DIRECTORY);

        for (const auto &reg : api.states) {
            if (!reg.get_is_persistent_config())
                continue;

            if (!LittleFS.exists(API::getLittleFSConfigPath(reg.path)))
                continue;

            File f = LittleFS.open(import_dir + API::getLittleFSConfigPath(reg.path, false), "w", true);
            reg.config->save_to_file(f, reg.keys_to_censor, reg.get_keys_to_censor_len());
        }

        constexpr size_t buf_size = 4096;
        auto buf = heap_alloc_array<uint8_t>(buf_size);

        {
            File src = LittleFS.open(StringSumHelper(API::CONFIG_DIRECTORY) + "/version", "r");
            File dst = LittleFS.open(import_dir + API::CONFIG_DIRECTORY + "/version", "w", true);
            copy_file(src, dst, buf.get(), buf_size);
        }

        // Skip first registration, /config is already handled above.
        for (size_t i = 1; i < this->allowed_paths.size(); ++i) {
            const auto &reg = this->allowed_paths[i];

            for_file_in(reg.path, [buf_ptr = buf.get(), &import_dir](File *src) {

                String dest_path = import_dir + src->path();
                File dest = LittleFS.open(dest_path, "w", true);

                return copy_file(*src, dest, buf_ptr, buf_size);
            }, true);
        }

        return request.send_plain(200);
    });

    server.on("/config_import/finish", HTTP_PUT, [](WebServerRequest request) {
        LittleFS.rename("/config_import", "/config_import_finished");

        trigger_reboot("config import", 1_s);

        return request.send_plain(200);
    });

    server.on("/config_import/file/*", HTTP_DELETE, [this](WebServerRequest request) {
        constexpr size_t PATH_BUFSIZE = PATH_MAX;
        auto import_path = heap_alloc_array<char>(PATH_BUFSIZE);
        char *path_suffix;
        {
            StringWriter sw{import_path.get(), PATH_BUFSIZE};
            sw.puts("/config_import");
            path_suffix = sw.getRemainingPtr();
        }

        bool is_dir = false;

        const Reg *reg;
        switch (this->sanitize_path(request.uriCStr(), constexpr_strlen("/config_import/file"), &reg, path_suffix)) {
            case SanitizePathError::RealpathFailed:
                return request.send_plain(500, "Failed to import file: realpath failed.");

            case SanitizePathError::PathNotFound:
                return request.send_plain(400, "Failed to import file: Path does not point into allowed directory");

            case SanitizePathError::FilenameTooLong:
                return request.send_plain(400, "Failed to import file: Filename too large");

            case SanitizePathError::IsDirectory:
                is_dir = true;
                break;

            case SanitizePathError::OK:
            default:
                break;
        }

        if (is_dir)
            remove_directory(import_path.get());
        else
            LittleFS.remove(import_path.get());

        return request.send_plain(200);
    });

    // max URI len is 512 bytes, should be sufficient for filenames.
    server.on("/config_import/file/*", HTTP_PUT, [this](WebServerRequest request) {
        constexpr size_t PATH_BUFSIZE = PATH_MAX;
        auto import_path = heap_alloc_array<char>(PATH_BUFSIZE);
        char *path_suffix;
        {
            StringWriter sw{import_path.get(), PATH_BUFSIZE};
            sw.puts("/config_import");
            path_suffix = sw.getRemainingPtr();
        }

        const Reg *reg;
        switch (this->sanitize_path(request.uriCStr(), constexpr_strlen("/config_import/file"), &reg, path_suffix)) {
            case SanitizePathError::RealpathFailed:
                return request.send_plain(500, "Failed to import file: realpath failed.");

            case SanitizePathError::PathNotFound:
                return request.send_plain(400, "Failed to import file: Path does not point into allowed directory");

            case SanitizePathError::FilenameTooLong:
                return request.send_plain(400, "Failed to import file: Filename too large");

            case SanitizePathError::IsDirectory:
                return request.send_plain(400, "Failed to import file: Is a directory");

            case SanitizePathError::OK:
            default:
                break;
        }

        if (request.contentLength() == 0 || request.contentLength() > reg->max_file_size)
            return request.send_plain(413, "Failed to import file: Content Too Large");

        return this->receive_file(request, import_path.get());
    });

    // max URI len is 512 bytes, should be sufficient for filenames.
    server.on("/config_import/json_update/*", HTTP_PUT, [this](WebServerRequest request) {
        constexpr size_t PATH_BUFSIZE = PATH_MAX;
        auto import_path = heap_alloc_array<char>(PATH_BUFSIZE);
        char *path_suffix;
        {
            StringWriter sw{import_path.get(), PATH_BUFSIZE};
            sw.puts("/config_import/json_update");
            path_suffix = sw.getRemainingPtr();
        }

        const Reg *reg;
        switch (this->sanitize_path(request.uriCStr(), constexpr_strlen("/config_import/json_update"), &reg, path_suffix)) {
            case SanitizePathError::RealpathFailed:
                return request.send_plain(500, "Failed to import file: realpath failed.");

            case SanitizePathError::PathNotFound:
                return request.send_plain(400, "Failed to import file: Path does not point into allowed directory");

            case SanitizePathError::FilenameTooLong:
                return request.send_plain(400, "Failed to import file: Filename too large");

            case SanitizePathError::IsDirectory:
                return request.send_plain(400, "Failed to import file: Is a directory");

            case SanitizePathError::OK:
            default:
                break;
        }

        if (request.contentLength() == 0 || request.contentLength() > reg->max_file_size)
            return request.send_plain(413, "Failed to import file: Content Too Large");

        return this->receive_file(request, import_path.get());
    });
}

void ConfigImport::allow_path(const char *path, size_t max_filename_len, size_t max_file_size)
{
    assert(address_is_in_rodata(path));

    allowed_paths.add({path, strlen(path), max_filename_len, max_file_size});
}

ConfigImport::SanitizePathError ConfigImport::sanitize_path(const char *uri, size_t prefix_len, const Reg **out_reg, char *out_path)
{
    static_assert(sizeof(httpd_req_t::uri) + constexpr_strlen(DATA_MOUNTPOINT) + constexpr_strlen("/config_import/") < PATH_MAX);

    constexpr size_t PATH_BUFSIZE = PATH_MAX;
    auto real_path = heap_alloc_array<char>(PATH_BUFSIZE);
    size_t real_path_len = 0;

    {
        char path[PATH_MAX];
        StringWriter sw{path, ARRAY_SIZE(path)};
        sw.puts(DATA_MOUNTPOINT "/");
        sw.puts(uri + prefix_len);

        if (realpath(path, real_path.get()) == nullptr) {
            logger.printfln("Failed to import file: %s (%d)", strerror(errno), errno);
            return SanitizePathError::RealpathFailed;
        }
        real_path_len = strnlen(real_path.get(), PATH_BUFSIZE);
    }

    constexpr const char *expected_prefix = DATA_MOUNTPOINT "/";
    size_t expected_prefix_len = constexpr_strlen(expected_prefix);

    if (real_path_len <= expected_prefix_len || memcmp(real_path.get(), expected_prefix, expected_prefix_len) != 0)
        return SanitizePathError::PathNotFound;

    // We now know that real_path points into the data partition.
    // Advance path, but keep / between mount point and directory:
    // allow_path is to be called with paths like "/certs"
    char *path = real_path.get() + expected_prefix_len - 1;
    size_t path_len = real_path_len - (expected_prefix_len - 1);

    for (const auto &reg : this->allowed_paths) {
        if (path_len <= reg.path_len)
            continue;

        if (memcmp(path, reg.path, reg.path_len) != 0)
            continue;

        if (path_len > reg.path_len + reg.max_filename_len)
            return SanitizePathError::FilenameTooLong;

        if (out_reg != nullptr)
            *out_reg = &reg;

        memcpy(out_path, path, path_len);

        if (LittleFS.exists(path) && LittleFS.open(path).isDirectory())
            return SanitizePathError::IsDirectory;

        return SanitizePathError::OK;
    }

    return SanitizePathError::PathNotFound;
}

WebServerRequestReturnProtect ConfigImport::receive_file(WebServerRequest &request, const char *import_path)
{
    File f = LittleFS.open(import_path, "w", true);
    // This primarily handles the case that import_path contains slashes,
    // but one of the parent directories already exist as a file.
    // We should probably handle this as a separate case in the future and return a 400 instead.
    if (!f) {
        LittleFS.remove(import_path);
        return request.send_plain(500, "Failed to import file: Failed to create file");
    }

    size_t to_write = request.contentLength();
    while (to_write > 0) {
        constexpr size_t BUF_LEN = 2048;
        char buf[BUF_LEN];

        int recvd = request.receive(buf, std::min(BUF_LEN, to_write));

        if (recvd <= 0) {
            f.close();
            LittleFS.remove(import_path);

            if (recvd == 0)
                return request.send_plain(500, "Failed to import file: Failed to receive payload");

            return request.send_plain(500, "Failed to import file: Connection closed before payload was received completely");
        }

        to_write -= f.write(reinterpret_cast<uint8_t *>(buf), static_cast<size_t>(recvd));
    }

    return request.send_plain(200);
}

static bool import_file(const String &path)
{
    return LittleFS.rename(path, path.substring(constexpr_strlen("/config_import_finished")));
}

static bool import_directory(const String &root)
{
    if (!LittleFS.exists(root))
        LittleFS.mkdir(root);

    return for_filepath_in(root.c_str(), [](const String &path, bool is_dir) {
        if (is_dir)
            return import_directory(path);

        return import_file(path);
    });
}

void ConfigImport::import()
{
    if (!LittleFS.exists("/config_import_finished"))
        return;

    for_filepath_in("/config_import_finished", [](const String &path, bool is_dir) {
        if (is_dir) {
            if (path == "/config_import_finished/json_update")
                return true; // Skip json_updates when importing files. Will be handled in restorePersistentConfig.

            return import_directory(path);
        }

        return import_file(path);
    });
}

String ConfigImport::import_json_update(const String &file, ConfigRoot *config) {
    auto path = String{"/config_import_finished/json_update"} + file;
    if (!LittleFS.exists(path))
        return "";

    return config->update_from_file(LittleFS.open(path));
}
