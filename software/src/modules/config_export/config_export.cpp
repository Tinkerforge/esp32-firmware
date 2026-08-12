/* esp32-firmware
 * Copyright (C) 2026 Erik Fleckstein <erik@tinkerforge.com>
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

#include "config_export.h"

#include <limits.h>
#include <stdlib.h>

#include <mbedtls/base64.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "options.h"
#include "tools.h"
#include "tools/fs.h"
#include "tools/memory.h"

#include "gcc_warnings.h"

constexpr const char *non_persistent_configs_to_export[] {
    "users/config"
};

void ConfigExport::pre_setup()
{
    this->allow_path(API::CONFIG_DIRECTORY, ExportType::JSON);
}

constexpr const char *export_type_strings[] {
    "json",
    "text",
    "binary"
};

#define OR_RETURN(x) do { \
    if (const esp_err_t err = x; err != ESP_OK) \
        return err; \
} while (false)

#define OR_FAIL(x) do { \
    if (const esp_err_t err = x; err != ESP_OK) \
        return WebServerRequestReturnProtect{.error = err}; \
} while (false)

static esp_err_t export_json(const char *path, WebServerChunkedResponse &request)
{
    File f = LittleFS.open(path);
    if (!f)
        return ESP_FAIL;

    while (f.available()) {
        if (request.free() == 0)
            OR_RETURN(request.flush());

        size_t read = f.read(reinterpret_cast<uint8_t *>(request.write_ptr()), request.free());
        request.written(read);
    }

    return ESP_OK;
}

static esp_err_t export_text(const char *path, WebServerChunkedResponse &request)
{
    File f = LittleFS.open(path);
    if (!f)
        return ESP_FAIL;

    uint8_t read_buf[512];

    while (f.available()) {
        size_t read_from_file = f.read(read_buf, std::size(read_buf));

        // Will run one time if no flush is necessary,
        // two times if we need one flush and the assertions hold.
        while (true) {
            size_t write_buf_size = request.free();
            TFJsonSerializer json{request.write_ptr(), write_buf_size};

            json._writeEscaped(reinterpret_cast<char *>(read_buf), read_from_file);

            if (json.buf_required > write_buf_size) {
                // The free size of the request's buffer was not big enough to hold
                // the escaped read_buf.
                // Flushing before calling request.written(...),
                // is a bit inefficient, because we will escape the portion that did
                // fit into the request's buffer again, but there is no easy way
                // to figure out how many bytes of input were read by _writeEscaped
                // before the output buffer was full.
                OR_RETURN(request.flush());

                // After a flush, the buffer should be empty
                // and an empty buffer should fit the JSON escaped read-buffer
                // even if it expanded to six times it's unescaped size.
                // This is the worst case,
                // because each char of input can be a control character that expands to \u00xy.
                static_assert(std::size(read_buf) * 6 < WebServerChunkedResponse::BUF_SIZE);
                assert(request.buf_filled == 0);
                continue;
            }

            request.written(json.buf_required);
            break;
        }
    }

    return ESP_OK;
}

static esp_err_t export_binary(const char *path, WebServerChunkedResponse &request)
{
    File f = LittleFS.open(path);
    if (!f)
        return ESP_FAIL;

    // Using a buffer that is divisable by 3 makes sure only the last mbedtls_base64_encode call inserts padding.
    uint8_t read_buf[510];

    while (f.available()) {
        size_t read_from_file = f.read(read_buf, std::size(read_buf));
        size_t written;

        mbedtls_base64_encode(nullptr, 0, &written, read_buf, read_from_file);

        if (request.free() < written)
            OR_RETURN(request.flush());

        int err = mbedtls_base64_encode(reinterpret_cast<unsigned char *>(request.write_ptr()), request.free(), &written, read_buf, read_from_file);
        assert(err == 0);
        request.written(written);
    }

    return ESP_OK;
}

static esp_err_t export_file(ConfigExport::ExportType type, const char *path, WebServerChunkedResponse &request, bool leading_comma)
{
    {
        char buf[constexpr_strlen(API::CONFIG_DIRECTORY) + 1 + API::MAX_PATH_LEN + 128];
        StringWriter sw{buf, std::size(buf)};
        // JSON does not need additional "" around the payload,
        // other types do, because they are sent as JSON strings.
        sw.printf("%s[\"%s\",\"%s\",%s", leading_comma ? ",\n" : "", export_type_strings[type], path, type == ConfigExport::ExportType::JSON ? "" : "\"");

        OR_RETURN(request.sendChunk(buf, sw.getLength()));
    }

    switch (type) {
        case ConfigExport::ExportType::JSON:
            OR_RETURN(export_json(path, request));
            break;
        case ConfigExport::ExportType::Text:
            OR_RETURN(export_text(path, request));
            break;
        case ConfigExport::ExportType::Binary:
            OR_RETURN(export_binary(path, request));
            break;

        default:
            esp_system_abortf<96>("Unknown config export type %d", type);
    }

    {
        char buf[3];
        StringWriter sw{buf, std::size(buf)};
        // JSON does not need additional "" around the payload,
        // other types do, because they are sent as JSON strings.
        sw.printf("%s]",
                  type == ConfigExport::ExportType::JSON ? "" : "\"");

        OR_RETURN(request.sendChunk(buf));
    }

    return ESP_OK;
}

void ConfigExport::register_urls()
{
    server.on("/config_export/export", HTTP_GET, [this](WebServerRequest request) {
        request.beginChunkedResponse(200, "application/json");
        WebServerChunkedResponse resp{request};

        OR_FAIL(resp.sendChunk("[\n"));

        OR_FAIL(export_file(ExportType::JSON, "/config/version", resp, false));

        // Export configs that are persisted
        for (const auto &reg : api.states) {
            if (!reg.get_is_persistent_config())
                continue;

            auto path = API::getLittleFSConfigPath(reg.path);
            if (!LittleFS.exists(path))
                continue;

            char buf[constexpr_strlen(API::CONFIG_DIRECTORY) + 1 + API::MAX_PATH_LEN + 128];
            StringWriter sw{buf, std::size(buf)};
            sw.printf(",\n[\"json\",\"%s\",", path.c_str());
            OR_FAIL(resp.sendChunk(buf, sw.getLength()));

            auto payload = reg.config->to_string_except(reg.keys_to_censor, reg.get_keys_to_censor_len());
            OR_FAIL(resp.sendChunk(payload));

            OR_FAIL(resp.sendChunk("]"));
        }

        // Export special cases
        for (const char *reg_path : non_persistent_configs_to_export) {
            auto path = API::getLittleFSConfigPath(reg_path);
            if (!LittleFS.exists(path))
                continue;

            const auto *reg = api.getStateRegistration(reg_path);
            if (reg == nullptr)
                continue;

            char buf[constexpr_strlen(API::CONFIG_DIRECTORY) + 1 + API::MAX_PATH_LEN + 128];
            StringWriter sw{buf, std::size(buf)};
            sw.printf(",\n[\"json\",\"%s\",", path.c_str());
            OR_FAIL(resp.sendChunk(buf, sw.getLength()));

            auto payload = reg->config->to_string_except(reg->keys_to_censor, reg->get_keys_to_censor_len());
            OR_FAIL(resp.sendChunk(payload));

            OR_FAIL(resp.sendChunk("]"));
        }

        // Export not persisted configs as "default"
        for (const auto &reg : api.states) {
            if (!reg.get_is_persistent_config())
                continue;

            auto path = API::getLittleFSConfigPath(reg.path);
            if (LittleFS.exists(path))
                continue;

            char buf[constexpr_strlen(API::CONFIG_DIRECTORY) + 1 + API::MAX_PATH_LEN + 128];
            StringWriter sw{buf, std::size(buf)};
            sw.printf(",\n[\"default\",\"%s\"]", path.c_str());
            OR_FAIL(resp.sendChunk(buf, sw.getLength()));
        }

        // Export other registered paths
        // Skip first registration, /config is already handled above.
        for (size_t i = 1; i < this->allowed_paths.size(); ++i) {
            const auto &reg = this->allowed_paths[i];

            for_filepath_in(reg.path, [reg, &resp](const String &path, bool is_dir) {
                if (is_dir)
                    esp_system_abort("sub-dirs not implemented yet!");

                return export_file(reg.type, path.c_str(), resp, true) == ESP_OK;
            });
        }

        OR_FAIL(resp.sendChunk("\n]"));

        OR_FAIL(resp.flush());

        return request.endChunkedResponse();
    });
}

void ConfigExport::allow_path(const char *path, ExportType type)
{
    assert(address_is_in_rodata(path));

    allowed_paths.add({path, type});
}

