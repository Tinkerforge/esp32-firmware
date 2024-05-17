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

#include "firmware_update.h"
#include "module_dependencies.h"

#include <Update.h>
#include <esp_http_client.h>

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "build.h"
#include "web_server.h"

#include "./crc32.h"

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);

// Newer firmwares contain a firmware info page.
#define FIRMWARE_INFO_OFFSET (0xd000 - 0x1000)
#define FIRMWARE_INFO_LENGTH 0x1000

void FirmwareUpdate::pre_setup()
{
    config = Config::Object({
        {"update_url", Config::Str("", 0, 128)},
    });

    available_updates = Config::Object({
        {"timestamp", Config::Uint(0)},
        {"cookie", Config::Uint(0)},
        {"beta", Config::Str("", 0, 32)},
        {"release", Config::Str("", 0, 32)},
        {"stable", Config::Str("", 0, 32)},
    });
}

void FirmwareUpdate::setup()
{
    if (!api.restorePersistentConfig("firmware_update/config", &config)) {
        config.get("update_url")->updateString(BUILD_FIRMWARE_UPDATE_URL);
    }

    update_url = config.get("update_url")->asString();

    if (update_url.length() > 0) {
        if (!update_url.endsWith("/")) {
            update_url += "/";
        }

        update_url += BUILD_NAME;
        update_url += "_firmware.txt";
    }

    initialized = true;
}

void FirmwareUpdate::reset_firmware_info()
{
    calculated_checksum = 0;
    info = firmware_info_t{};
    info_offset = 0;
    checksum_offset = 0;
    update_aborted = false;
    info_found = false;
}

bool FirmwareUpdate::handle_firmware_info_chunk(size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len)
{
    uint8_t *start = chunk_data;
    size_t len = chunk_len;

    if (chunk_offset < FIRMWARE_INFO_OFFSET) {
        size_t to_skip = FIRMWARE_INFO_OFFSET - chunk_offset;
        start += to_skip;
        len -= to_skip;
    }

    len = MIN(len, (FIRMWARE_INFO_OFFSET + FIRMWARE_INFO_LENGTH) - chunk_offset - 4); // -4 to not calculate the CRC of itself

    if (info_offset < sizeof(info)) {
        size_t to_write = MIN(len, sizeof(info) - info_offset);
        memcpy(((uint8_t *)&info) + info_offset, start, to_write);
        info_offset += to_write;
    }

    crc32_ieee_802_3_recalculate(start, len, &calculated_checksum);

    const size_t checksum_start = FIRMWARE_INFO_OFFSET + FIRMWARE_INFO_LENGTH - 4;

    if (chunk_offset + chunk_len < checksum_start)
        return false;

    if (chunk_offset < checksum_start) {
        size_t to_skip = checksum_start - chunk_offset;
        start = chunk_data + to_skip;
        len = chunk_len - to_skip;
    }

    len = MIN(len, 4);

    if (checksum_offset < sizeof(checksum)) {
        size_t to_write = MIN(len, sizeof(checksum) - checksum_offset);
        memcpy((uint8_t *)&checksum + checksum_offset, start, to_write);
        checksum_offset += to_write;
    }

    info_found = checksum_offset == sizeof(checksum) && info.magic[0] == 0x12CE2171 && (info.magic[1] & 0x00FFFFFF) == 0x6E12F0;
    return info_found;
}

String FirmwareUpdate::check_firmware_info(bool firmware_info_found, bool detect_downgrade, bool log)
{
    if (!firmware_info_found && BUILD_REQUIRE_FIRMWARE_INFO) {
        if (log) {
            logger.printfln("Failed to update: Firmware update has no info page!");
        }
        return "{\"error\":\"firmware_update.script.no_info_page\"}";
    }
    if (firmware_info_found) {
        if (checksum != calculated_checksum) {
            if (log) {
                logger.printfln("Failed to update: Firmware info page corrupted! Embedded checksum %x calculated checksum %x", checksum, calculated_checksum);
            }
            return "{\"error\":\"firmware_update.script.info_page_corrupted\"}";
        }

        if (strncmp(BUILD_DISPLAY_NAME, info.firmware_name, ARRAY_SIZE(info.firmware_name)) != 0) {
            if (log) {
                logger.printfln("Failed to update: Firmware is for a %.*s but this is a %s!", static_cast<int>(ARRAY_SIZE(info.firmware_name)), info.firmware_name, BUILD_DISPLAY_NAME);
            }
            return "{\"error\":\"firmware_update.script.wrong_firmware_type\"}";
        }

        if (detect_downgrade && compare_version(info.fw_version[0], info.fw_version[1], info.fw_version[2], info.fw_version_beta, info.fw_build_time,
                                                BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, BUILD_VERSION_BETA, build_timestamp()) < 0) {
            if (log) {
                logger.printfln("Failed to update: Firmware is a downgrade!");
            }

            char buf[256];
            char info_beta[12] = "";
            char build_beta[12] = "";

            if (info.fw_version_beta != 255) {
                snprintf(info_beta, ARRAY_SIZE(info_beta), "-beta.%u", info.fw_version_beta);
            }

            if (BUILD_VERSION_BETA != 255) {
                snprintf(build_beta, ARRAY_SIZE(build_beta), "-beta.%i", BUILD_VERSION_BETA);
            }

            snprintf(buf, ARRAY_SIZE(buf), "{\"error\":\"firmware_update.script.downgrade\",\"fw\":\"%u.%u.%u%s+%x\",\"installed\":\"%i.%i.%i%s+%x\"}",
                     info.fw_version[0], info.fw_version[1], info.fw_version[2], info_beta, info.fw_build_time,
                     BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, build_beta, build_timestamp());

            return String(buf);
        }
    }
    return "";
}

bool FirmwareUpdate::handle_update_chunk(int command, std::function<void(const char *, const char *)> result_cb, size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len, size_t remaining, size_t complete_len) {
    // The firmware files are merged with the bootloader, partition table, firmware_info and slot configuration bins.
    // The bootloader starts at offset 0x1000, which is the first byte in the firmware file.
    // The first firmware slot (i.e. the one that is flashed over USB) starts at 0x10000.
    // So we have to skip the first 0x10000 - 0x1000 bytes, after them the actual firmware starts.
    // Don't skip anything if we flash the LittleFS.
    const size_t firmware_offset = command == U_FLASH ? 0x10000 - 0x1000 : 0;
    static bool firmware_info_found = false;

    if (chunk_offset == 0 && !Update.begin(complete_len - firmware_offset, command)) {
        logger.printfln("Failed to start update: %s", Update.errorString());
        result_cb("text/plain", Update.errorString());
        Update.abort();
        update_aborted = true;
        return true;
    }

    if (chunk_offset == 0) {
        reset_firmware_info();
        firmware_info_found = false;
    }

    if (update_aborted) {
        return true;
    }

    if (chunk_offset + chunk_len >= FIRMWARE_INFO_OFFSET && chunk_offset < FIRMWARE_INFO_OFFSET + FIRMWARE_INFO_LENGTH) {
        firmware_info_found = handle_firmware_info_chunk(chunk_offset, chunk_data, chunk_len);
    }

    if (chunk_offset + chunk_len >= FIRMWARE_INFO_OFFSET + FIRMWARE_INFO_LENGTH) {
        String error = this->check_firmware_info(firmware_info_found, false, true);
        if (!error.isEmpty()) {
            result_cb("application/json", error.c_str());
            Update.abort();
            update_aborted = true;
            return true;
        }
    }

    if (chunk_offset + chunk_len < firmware_offset) {
        return true;
    }

    uint8_t *start = chunk_data;
    size_t len = chunk_len;

    if (chunk_offset < firmware_offset) {
        size_t to_skip = firmware_offset - chunk_offset;
        start += to_skip;
        len -= to_skip;
    }

    auto written = Update.write(start, len);
    if (written != len) {
        logger.printfln("Failed to write update chunk with length %u; written %u, error: %s", len, written, Update.errorString());
        result_cb("text/plain", (String("Failed to write update: ") + Update.errorString()).c_str());
        this->firmware_update_running = false;
        Update.abort();
        return false;
    }

    if (remaining == 0 && !Update.end(true)) {
        logger.printfln("Failed to apply update: %s", Update.errorString());
        result_cb("text/plain", (String("Failed to apply update: ") + Update.errorString()).c_str());
        this->firmware_update_running = false;
        Update.abort();
        return false;
    }

    return true;
}

void FirmwareUpdate::register_urls()
{
    api.addPersistentConfig("firmware_update/config", &config);
    api.addState("firmware_update/available_updates", &available_updates);

    api.addCommand("firmware_update/check_for_updates", Config::Null(), {}, [this]() {
        check_for_updates();
    }, true);

    server.on("/check_firmware", HTTP_POST, [this](WebServerRequest request) {
        if (!this->info_found && BUILD_REQUIRE_FIRMWARE_INFO) {
            return request.send(400, "application/json", "{\"error\":\"firmware_update.script.no_info_page\"}");
        }
        return request.send(200);
    },
    [this](WebServerRequest request, String filename, size_t offset, uint8_t *data, size_t len, size_t remaining) {
        if (offset == 0) {
            this->reset_firmware_info();
        }

        bool firmware_update_allowed_check_required = true;
#if MODULE_ENERGY_MANAGER_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
        firmware_update_allowed_check_required = energy_manager.disallow_fw_update_with_vehicle_connected();
#endif
        if (firmware_update_allowed_check_required && !firmware_update_allowed) {
            request.send(400, "application/json", "{\"error\":\"firmware_update.script.vehicle_connected\"}");
            return false;
        }

        if (offset > FIRMWARE_INFO_LENGTH) {
            request.send(400, "text/plain", "Too long!");
            return false;
        }

        bool firmware_info_found = handle_firmware_info_chunk(offset + FIRMWARE_INFO_OFFSET, data, len);

        if (offset + len >= FIRMWARE_INFO_LENGTH) {
            String error = this->check_firmware_info(firmware_info_found, true, false);
            if (!error.isEmpty()) {
                request.send(400, "application/json", error.c_str());
            }
        }

        return true;
    });

    server.on_HTTPThread("/flash_firmware", HTTP_POST, [this](WebServerRequest request) {
        if (update_aborted)
            return request.unsafe_ResponseAlreadySent(); // Already sent in upload callback.

        this->firmware_update_running = false;

        if(!Update.hasError()) {
            logger.printfln("Firmware flashed successfully! Rebooting in one second.");
            task_scheduler.scheduleOnce([](){ESP.restart();}, 1000);
        }

        return request.send(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
    },
    [this](WebServerRequest request, String filename, size_t offset, uint8_t *data, size_t len, size_t remaining) {
        this->firmware_update_running = true;

        WebServerRequest *request_ptr = &request;

        return handle_update_chunk(U_FLASH, [request_ptr](const char *mimetype, const char *message) {
            request_ptr->send(400, mimetype, message);
        }, offset, data, len, remaining, request.contentLength());
    });
}

struct Version {
    uint8_t major = 255;
    uint8_t minor = 255;
    uint8_t patch = 255;
    uint8_t beta = 255;
    uint32_t timestamp = 0;
};

// <major:int>.<minor:int>.<patch:int>[-beta.<beta:int>]+<timestamp:hex>
static bool parse_version(const char *p, Version *version)
{
    char *end;

    // major
    uint32_t major = strtoul(p, &end, 10);

    if (p == end || *end != '.' || major > 254) {
        return false;
    }

    version->major = major;
    p = end + 1;

    // minor
    uint32_t minor = strtoul(p, &end, 10);

    if (p == end || *end != '.' || minor > 254) {
        return false;
    }

    version->minor = minor;
    p = end + 1;

    // patch
    uint32_t patch = strtoul(p, &end, 10);

    if (p == end || (strncmp(end, "-beta.", 6) && *end != '+') || patch > 254) {
        return false;
    }

    version->patch = patch;

    // beta
    if (*end == '+') {
        p = end + 1;
        version->beta = 255;
    }
    else {
        p = end + 6;

        uint32_t beta = strtoul(p, &end, 10);

        if (p == end || *end != '+' || beta > 254) {
            return false;
        }

        version->beta = beta;
        p = end + 1;
    }

    // timestamp
    version->timestamp = strtoul(p, &end, 16);

    if (p == end || *end != '\0') {
        return false;
    }

    return true;
}

static String format_version(Version *version)
{
    char buf[64];

    if (version->beta != 255) {
        snprintf(buf, sizeof(buf), "%u.%u.%u-beta.%u+%x", version->major, version->minor, version->patch, version->beta, version->timestamp);
    }
    else {
        snprintf(buf, sizeof(buf), "%u.%u.%u+%x", version->major, version->minor, version->patch, version->timestamp);
    }

    return String(buf);
}

// list files are not signed to allow customer fleet update managment, firmwares are signed
void FirmwareUpdate::check_for_updates()
{
    ++update_cookie;

    available_updates.get("cookie")->updateUint(update_cookie);

    if (update_url.length() == 0) {
        logger.printfln("No update URL configured");
        return;
    }

    esp_http_client_config_t config;
    memset(&config, 0, sizeof(config));

    config.url = update_url.c_str();
    // FIXME: allow custom certificate
    config.crt_bundle_attach = esp_crt_bundle_attach;
    config.transport_type = HTTP_TRANSPORT_OVER_SSL;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_open(client, 0);

    if (err != ESP_OK) {
        logger.printfln("Error while opening firmware list: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return;
    }

    defer {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
    };

    int content_length = esp_http_client_fetch_headers(client);

    if (content_length < 0) {
        logger.printfln("Error while reading firmware list HTTP headers: %s", esp_err_to_name(content_length));
        return;
    }

    char buf[64 + 1];
    size_t buf_used = 0;
    int total_read_len = 0;
    char *p;
    Version beta_update;
    Version release_update;
    Version stable_update;
    uint8_t updates_mask = 0;
    uint32_t last_non_beta_timestamp = time(nullptr);

    while (total_read_len < content_length && updates_mask != 111) {
        size_t buf_free = sizeof(buf) - buf_used - 1;

        if (buf_free == 0) {
            logger.printfln("Firmware list is malformed");
            return;
        }

        int read_len = esp_http_client_read(client, buf + buf_used, buf_free);

        if (read_len < 0) {
            logger.printfln("Error while reading firmware list content: %s", esp_err_to_name(read_len));
            return;
        }

        total_read_len += read_len;
        buf_used += read_len;
        buf[buf_used] = '\0';
        p = strchr(buf, '\n');

        if (p == nullptr && total_read_len >= content_length) {
            p = buf + buf_used;
        }

        while (p != nullptr && updates_mask != 111) {
            *p = '\0';

            Version version;

            if (!parse_version(buf, &version)) {
                logger.printfln("Firmware list entry is malformed: %s", buf);
                return;
            }

            // ignore all versions that are older than the current version
            if (compare_version(version.major, version.minor, version.patch, version.beta, version.timestamp,
                                BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, BUILD_VERSION_BETA, build_timestamp()) <= 0) {
                updates_mask = 111;
                break;
            }

            if (version.beta != 255) {
                // the beta update is the newest beta version that is newer than any non-beta version
                if ((updates_mask & 100) == 0) {
                    beta_update = version;
                    updates_mask |= 100;
                }
            }
            else {
                updates_mask |= 100; // ignore beta versions older than the newest non-beta version

                // the release update is the newest non-beta version
                if ((updates_mask & 10) == 0) {
                    release_update = version;
                    updates_mask |= 10;
                }

                // the stable update is the newest non-beta version that was
                // released more than 7 days before the next non-beta version
                if ((updates_mask & 1) == 0 && version.timestamp + (7 * 24 * 60 * 60) < last_non_beta_timestamp) {
                    stable_update = version;
                    updates_mask |= 1;
                }

                last_non_beta_timestamp = version.timestamp;
            }

            if (p == buf + buf_used) {
                break;
            }

            size_t buf_consumed = p + 1 - buf;

            memmove(buf, p + 1, buf_used - buf_consumed);

            buf_used -= buf_consumed;
            buf[buf_used] = '\0';

            p = strchr(buf, '\n');
        }
    }

    String beta_update_str = "";
    String release_update_str = "";
    String stable_update_str = "";

    if (beta_update.major != 255) {
        beta_update_str = format_version(&beta_update);
    }

    if (release_update.major != 255) {
        release_update_str = format_version(&release_update);
    }

    if (stable_update.major != 255) {
        stable_update_str = format_version(&stable_update);
    }

    available_updates.get("timestamp")->updateUint(time(nullptr));
    available_updates.get("beta")->updateString(beta_update_str);
    available_updates.get("release")->updateString(release_update_str);
    available_updates.get("stable")->updateString(stable_update_str);
}
