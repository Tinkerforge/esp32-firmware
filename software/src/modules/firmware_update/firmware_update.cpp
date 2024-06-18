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

#define SIGNATURE_LENGTH crypto_sign_BYTES
#define SIGNATURE_OFFSET (FIRMWARE_INFO_OFFSET - SIGNATURE_LENGTH)

#define CHECK_FOR_UPDATE_TIMEOUT 15000

#if !MODULE_CERTS_AVAILABLE()
#define MAX_CERT_ID -1
#endif

#if signature_public_key_length != 0
static_assert(signature_public_key_length == crypto_sign_PUBLICKEYBYTES);
#endif

void FirmwareUpdate::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"update_url", Config::Str("", 0, 128)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
    }), [this](Config &update, ConfigSource source) -> String {
        String update_url = update.get("update_url")->asString();

        if (update_url.length() > 0 && !update_url.startsWith("https://"))
            return "HTTPS required for update URL";

        return "";
    }};

    state = Config::Object({
        {"check_timestamp", Config::Uint(0)},
        {"check_error", Config::Str("", 0, 16)},
        {"beta_update", Config::Str("", 0, 32)},
        {"release_update", Config::Str("", 0, 32)},
        {"stable_update", Config::Str("", 0, 32)},
    });

    install_firmware = ConfigRoot{Config::Object({
        {"version", Config::Str("", 0, 32)},
    }), [this](Config &update, ConfigSource source) -> String {
        SemanticVersion p;

        if (!parse_version(update.get("version")->asString().c_str(), &p)) {
            return "Version is malformed";
        }

        return "";
    }};
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

    cert_id = config.get("cert_id")->asInt();
    initialized = true;
}

#if signature_public_key_length != 0
void FirmwareUpdate::handle_signature_chunk(size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len)
{
    for (size_t i = 0; i < chunk_len; ++i) {
        size_t k = chunk_offset + i;

        if (k >= SIGNATURE_OFFSET && k < SIGNATURE_OFFSET + SIGNATURE_LENGTH) {
            signature_data[k - SIGNATURE_OFFSET] = chunk_data[i];
            chunk_data[i] = 0xFF;
        }
    }
}
#endif

void FirmwareUpdate::reset_firmware_info()
{
    calculated_checksum = 0;
    info = firmware_info_t{};
    info_offset = 0;
    checksum_offset = 0;
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
        return "{\"error\":\"no_info_page\"}";
    }
    if (firmware_info_found) {
        if (checksum != calculated_checksum) {
            if (log) {
                logger.printfln("Failed to update: Firmware info page corrupted! Embedded checksum %x calculated checksum %x", checksum, calculated_checksum);
            }
            return "{\"error\":\"info_page_corrupted\"}";
        }

        if (strncmp(BUILD_DISPLAY_NAME, info.firmware_name, ARRAY_SIZE(info.firmware_name)) != 0) {
            if (log) {
                logger.printfln("Failed to update: Firmware is for a %.*s but this is a %s!", static_cast<int>(ARRAY_SIZE(info.firmware_name)), info.firmware_name, BUILD_DISPLAY_NAME);
            }
            return "{\"error\":\"wrong_firmware_type\"}";
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

            snprintf(buf, ARRAY_SIZE(buf), "{\"error\":\"downgrade\",\"firmware\":\"%u.%u.%u%s+%x\",\"installed\":\"%i.%i.%i%s+%x\"}",
                     info.fw_version[0], info.fw_version[1], info.fw_version[2], info_beta, info.fw_build_time,
                     BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, build_beta, build_timestamp());

            return String(buf);
        }
    }
    return "";
}

bool FirmwareUpdate::handle_firmware_chunk(int command, std::function<void(const char *, const char *)> result_cb, size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len, size_t remaining, size_t complete_len) {
    // The firmware files are merged with the bootloader, partition table, firmware_info and slot configuration bins.
    // The bootloader starts at offset 0x1000, which is the first byte in the firmware file.
    // The first firmware slot (i.e. the one that is flashed over USB) starts at 0x10000.
    // So we have to skip the first 0x10000 - 0x1000 bytes, after them the actual firmware starts.
    // Don't skip anything if we flash the LittleFS.
    const size_t firmware_offset = command == U_FLASH ? 0x10000 - 0x1000 : 0;
    static bool firmware_info_found = false;

    if (chunk_offset == 0) {
        if (!Update.begin(complete_len - firmware_offset, command)) {
            logger.printfln("Failed to start update: %s", Update.errorString());
            result_cb("text/plain", Update.errorString());
            Update.abort();
            return false;
        }

        reset_firmware_info();
        firmware_info_found = false;

#if signature_public_key_length != 0
        if (sodium_init() < 0 || crypto_sign_init(&signature_state) < 0) {
            const char *message = "Failed to initialize signature verification";
            logger.printfln(message);
            result_cb("text/plain", message);
            Update.abort();
            return false;
        }

        memset(signature_data, 0xFF, SIGNATURE_LENGTH);
#endif
    }

#if signature_public_key_length != 0
    if (chunk_offset + chunk_len >= SIGNATURE_OFFSET && chunk_offset < SIGNATURE_OFFSET + SIGNATURE_LENGTH) {
        handle_signature_chunk(chunk_offset, chunk_data, chunk_len);
    }

    if (crypto_sign_update(&signature_state, chunk_data, chunk_len) < 0) {
        const char *message = "Failed to process signature verification";
        logger.printfln(message);
        result_cb("text/plain", message);
        Update.abort();
        return false;
    }
#endif

    if (chunk_offset + chunk_len >= FIRMWARE_INFO_OFFSET && chunk_offset < FIRMWARE_INFO_OFFSET + FIRMWARE_INFO_LENGTH) {
        firmware_info_found = handle_firmware_info_chunk(chunk_offset, chunk_data, chunk_len);
    }

    if (chunk_offset + chunk_len >= FIRMWARE_INFO_OFFSET + FIRMWARE_INFO_LENGTH) {
        String error = this->check_firmware_info(firmware_info_found, false, true);
        if (!error.isEmpty()) {
            result_cb("application/json", error.c_str());
            Update.abort();
            return false;
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
        Update.abort();
        return false;
    }

    if (remaining == 0) {
#if signature_public_key_length != 0
        if (crypto_sign_final_verify(&signature_state, signature_data, signature_public_key_data) < 0) {
            const char *message = "Failed to verify signature";
            logger.printfln(message);
            result_cb("text/plain", message);
            Update.abort();
            return false;
        }
#endif

        if (!Update.end(true)) {
            logger.printfln("Failed to apply update: %s", Update.errorString());
            result_cb("text/plain", (String("Failed to apply update: ") + Update.errorString()).c_str());
            Update.abort();
            return false;
        }
    }

    return true;
}

void FirmwareUpdate::register_urls()
{
    api.addPersistentConfig("firmware_update/config", &config);
    api.addState("firmware_update/state", &state);

    api.addCommand("firmware_update/check_for_update", Config::Null(), {}, [this]() {
        check_for_update();
    }, true);

    api.addCommand("firmware_update/install_firmware", &install_firmware, {}, [this]() {
        String version = install_firmware.get("version")->asString();

        logger.printfln("Install %s", version.c_str());
    }, true);

    server.on("/check_firmware", HTTP_POST, [this](WebServerRequest request) {
        if (!this->info_found && BUILD_REQUIRE_FIRMWARE_INFO) {
            return request.send(400, "application/json", "{\"error\":\"no_info_page\"}");
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
            request.send(400, "application/json", "{\"error\":\"vehicle_connected\"}");
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
        if(!Update.hasError()) {
            logger.printfln("Firmware flashed successfully! Rebooting in one second.");
            task_scheduler.scheduleOnce([](){ESP.restart();}, 1000);
        }

        return request.send(Update.hasError() ? 400: 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
    },
    [this](WebServerRequest request, String filename, size_t offset, uint8_t *data, size_t len, size_t remaining) {
        WebServerRequest *request_ptr = &request;

        return handle_firmware_chunk(U_FLASH, [request_ptr](const char *mimetype, const char *message) {
            request_ptr->send(400, mimetype, message);
        }, offset, data, len, remaining, request.contentLength());
    });
}

// <major:int>.<minor:int>.<patch:int>[-beta.<beta:int>]+<timestamp:hex>
bool FirmwareUpdate::parse_version(const char *p, SemanticVersion *version) const
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

String FirmwareUpdate::format_version(SemanticVersion *version) const
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

static esp_err_t update_event_handler(esp_http_client_event_t *event)
{
    FirmwareUpdate *that = static_cast<FirmwareUpdate *>(event->user_data);

    switch (event->event_id) {
    case HTTP_EVENT_ON_DATA:
        that->handle_update_data(event->data, event->data_len);
        break;

    case HTTP_EVENT_ON_FINISH:
        that->handle_update_data("\n", 1);
        break;

    default:
        break;
    }

    return ESP_OK;
}

// list files are not signed to allow customer fleet update managment, firmwares are signed
void FirmwareUpdate::check_for_update()
{
    if (http_client != nullptr) {
        logger.printfln("Already checking for update");
        return;
    }

    state.get("check_timestamp")->updateUint(time(nullptr));
    state.get("check_error")->updateString("pending");
    state.get("beta_update")->updateString("");
    state.get("release_update")->updateString("");
    state.get("stable_update")->updateString("");

    if (update_url.length() == 0) {
        logger.printfln("No update URL configured");
        state.get("check_error")->updateString("no_update_url");
        return;
    }

    update_buf_used = 0;
    beta_update.major = 255;
    release_update.major = 255;
    stable_update.major = 255;
    update_mask = 0;
    update_complete = false;
    last_non_beta_timestamp = time(nullptr);

    esp_http_client_config_t http_config = {};

    http_config.url = update_url.c_str();
    http_config.event_handler = update_event_handler;
    http_config.user_data = this;
    http_config.is_async = true;
    http_config.timeout_ms = 50;

    size_t cert_len = 0;

    if (cert_id < 0) {
        http_config.crt_bundle_attach = esp_crt_bundle_attach;
    }
    else {
#if MODULE_CERTS_AVAILABLE()
        cert = certs.get_cert(static_cast<uint8_t>(cert_id), &cert_len);

        if (cert == nullptr) {
            logger.printfln("Certificate with ID %d is not available", cert_id);
            state.get("check_error")->updateString("no_cert");
            return;
        }

        http_config.cert_pem = (const char *)cert.get();
#else
        // defense in depth: it should not be possible to arrive here because in case
        // that the certs module is not available the cert_id should always be -1
        logger.printfln("Can't use custom certitifate: certs module is not built into this firmware!");
        return;
#endif
    }

    http_client = esp_http_client_init(&http_config);

    if (http_client == nullptr) {
        logger.printfln("Error while creating HTTP client");
        cert.reset();
        return;
    }

    last_update_begin = millis();

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (deadline_elapsed(last_update_begin + CHECK_FOR_UPDATE_TIMEOUT)) {
            logger.printfln("Update server %s did not respond", update_url.c_str());
            state.get("check_error")->updateString("no_response");
            update_complete = true;
        }

        if (update_complete) {
            esp_http_client_close(http_client);
        }
        else {
            esp_err_t err = esp_http_client_perform(http_client);

            if (err == ESP_ERR_HTTP_EAGAIN) {
                return;
            }
            else if (err != ESP_OK) {
                logger.printfln("Error while downloading firmware list: %s", esp_err_to_name(err));
                state.get("check_error")->updateString("download_error");
            }
            else if (state.get("check_error")->asString() == "pending") {
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

                state.get("check_error")->updateString("");
                state.get("beta_update")->updateString(beta_update_str);
                state.get("release_update")->updateString(release_update_str);
                state.get("stable_update")->updateString(stable_update_str);
            }
        }

        esp_http_client_cleanup(http_client);
        http_client = nullptr;
        cert.reset();

        task_scheduler.cancel(task_scheduler.currentTaskId());
    }, 100, 100);
}

void FirmwareUpdate::handle_update_data(const void *data, size_t data_len)
{
    if (update_complete) {
        return;
    }

    const char *data_start = static_cast<const char *>(data);
    const char *data_end = data_start + data_len;

    while (data_start < data_end) {
        // fill buffer with new data
        char *update_buf_free_start = update_buf + update_buf_used;
        char *update_buf_free_end = update_buf + sizeof(update_buf) - 1;

        if (update_buf_free_start == update_buf_free_end) {
            logger.printfln("Firmware list is malformed");
            state.get("check_error")->updateString("list_malformed");
            update_complete = true;
            return;
        }

        while (update_buf_free_start < update_buf_free_end && data_start < data_end) {
            if (*data_start != ' ' && *data_start != '\t' && *data_start != '\r') {
                *update_buf_free_start++ = *data_start;
            }

            ++data_start;
        }

        *update_buf_free_start = '\0';
        update_buf_used = update_buf_free_start - update_buf;

        // parse version
        char *p = strchr(update_buf, '\n');

        while (p != nullptr && update_mask != 111) {
            *p = '\0';

            SemanticVersion version;

            if (!parse_version(update_buf, &version)) {
                logger.printfln("Firmware list entry is malformed: %s", update_buf);
                state.get("check_error")->updateString("list_malformed");
                update_complete = true;
                return;
            }

            // ignore all versions that are older than the current version
            if (compare_version(version.major, version.minor, version.patch, version.beta, version.timestamp,
                                BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, BUILD_VERSION_BETA, build_timestamp()) <= 0) {
                update_mask = 111;
                break;
            }

            if (version.beta != 255) {
                // the beta update is the newest beta version that is newer than any non-beta version
                if ((update_mask & 100) == 0) {
                    beta_update = version;
                    update_mask |= 100;
                }
            }
            else {
                update_mask |= 100; // ignore beta versions older than the newest non-beta version

                // the release update is the newest non-beta version
                if ((update_mask & 10) == 0) {
                    release_update = version;
                    update_mask |= 10;
                }

                // the stable update is the newest non-beta version that was
                // released more than 7 days before the next non-beta version
                if ((update_mask & 1) == 0 && version.timestamp + (7 * 24 * 60 * 60) < last_non_beta_timestamp) {
                    stable_update = version;
                    update_mask |= 1;
                }

                last_non_beta_timestamp = version.timestamp;
            }

            size_t update_buf_consumed = p + 1 - update_buf;

            memmove(update_buf, p + 1, update_buf_used - update_buf_consumed);

            update_buf_used -= update_buf_consumed;
            update_buf[update_buf_used] = '\0';

            p = strchr(update_buf, '\n');
        }

        if (update_mask == 111) {
            update_complete = true;
            return;
        }
    }
}
