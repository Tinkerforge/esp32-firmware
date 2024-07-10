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

#include <Update.h>
#include <TFJson.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "build.h"
#include "./crc32.h"

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);

// Newer firmwares contain a firmware info page.
#define FIRMWARE_INFO_OFFSET (0xd000 - 0x1000)
#define FIRMWARE_INFO_LENGTH 0x1000
#define FIRMWARE_INFO_MAGIC_0 0x12CE2171
#define FIRMWARE_INFO_MAGIC_1 0x6E12F0

// Signed firmwares contain a signature info page.
#define SIGNATURE_INFO_OFFSET (0xc000 - 0x1000)
#define SIGNATURE_INFO_LENGTH 0x1000
#define SIGNATURE_INFO_MAGIC_0 0xE6210F21
#define SIGNATURE_INFO_MAGIC_1 0xEC1217

#define SIGNATURE_INFO_SIGNATURE_OFFSET (SIGNATURE_INFO_OFFSET + offsetof(signature_info_t, signature))
#define SIGNATURE_INFO_SIGNATURE_LENGTH crypto_sign_BYTES

// The firmware files are merged with the bootloader, partition table, firmware_info and slot configuration bins.
// The bootloader starts at offset 0x1000, which is the first byte in the firmware file.
// The first firmware slot (i.e. the one that is flashed over USB) starts at 0x10000.
// So we have to skip the first 0x10000 - 0x1000 bytes, after them the actual firmware starts.
#define FIRMWARE_OFFSET (0x10000 - 0x1000)

#define CHECK_FOR_UPDATE_TIMEOUT 15000

#if !MODULE_CERTS_AVAILABLE()
#define MAX_CERT_ID -1
#endif

#if signature_public_key_length != 0
static_assert(signature_public_key_length == crypto_sign_PUBLICKEYBYTES);
#endif

template <typename T>
void BlockReader<T>::reset()
{
    block = T{};
    read_block_len = 0;
    block_found = false;
    actual_checksum = 0;
    expected_checksum = 0;
    read_expected_checksum_len = 0;
}

template <typename T>
bool BlockReader<T>::handle_chunk(size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len)
{
    if (chunk_offset + chunk_len >= block_offset && chunk_offset < block_offset + block_len) {
        uint8_t *start = chunk_data;
        size_t len = chunk_len;

        if (chunk_offset < block_offset) {
            size_t to_skip = block_offset - chunk_offset;
            start += to_skip;
            len -= to_skip;
        }

        if (chunk_offset + chunk_len > block_offset + block_len - 4) {
            len -= (chunk_offset + chunk_len) - (block_offset + block_len - 4); // -4 to not calculate the CRC of itself
        }

        if (read_block_len < sizeof(block)) {
            size_t to_read = std::min(len, sizeof(block) - read_block_len);
            memcpy(((uint8_t *)&block) + read_block_len, start, to_read);
            read_block_len += to_read;
        }

        crc32_ieee_802_3_recalculate(start, len, &actual_checksum);

        const size_t expected_checksum_offset = block_offset + block_len - 4;

        if (chunk_offset + chunk_len < expected_checksum_offset) {
            return false;
        }

        if (chunk_offset < expected_checksum_offset) {
            size_t to_skip = expected_checksum_offset - chunk_offset;
            start = chunk_data + to_skip;
            len = chunk_len - to_skip;
        }

        if (chunk_offset + chunk_len > expected_checksum_offset + 4) {
            len -= (chunk_offset + chunk_len) - (expected_checksum_offset + 4);
        }

        if (read_expected_checksum_len < sizeof(expected_checksum)) {
            size_t to_read = std::min(len, sizeof(expected_checksum) - read_expected_checksum_len);
            memcpy((uint8_t *)&expected_checksum + read_expected_checksum_len, start, to_read);
            read_expected_checksum_len += to_read;
        }

        block_found = read_expected_checksum_len == sizeof(expected_checksum) && actual_magic_0 == expected_magic_0 && (actual_magic_1 & 0x00FFFFFF) == expected_magic_1;
    }

    return chunk_offset + chunk_len >= block_offset + block_len;
}

FirmwareUpdate::FirmwareUpdate() :
    firmware_info(FIRMWARE_INFO_OFFSET, FIRMWARE_INFO_LENGTH, FIRMWARE_INFO_MAGIC_0, FIRMWARE_INFO_MAGIC_1)
#if signature_public_key_length != 0
    , signature_info(SIGNATURE_INFO_OFFSET, SIGNATURE_INFO_LENGTH, SIGNATURE_INFO_MAGIC_0, SIGNATURE_INFO_MAGIC_1)
#endif
{
}

void FirmwareUpdate::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"update_url", Config::Str(BUILD_FIRMWARE_UPDATE_URL, 0, 128)},
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
        {"update_version", Config::Str("", 0, 32)},
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

    override_signature = ConfigRoot{Config::Object({
        {"cookie", Config::Uint32(0)},
    })};
}

void FirmwareUpdate::setup()
{
    api.restorePersistentConfig("firmware_update/config", &config);

    update_url = config.get("update_url")->asString();

    if (update_url.length() > 0) {
        if (!update_url.endsWith("/")) {
            update_url += "/";
        }

        update_url += BUILD_NAME;
        update_url += "_firmware_v1.txt";
    }

    cert_id = config.get("cert_id")->asInt();
    initialized = true;
}

String FirmwareUpdate::check_firmware_info(bool detect_downgrade, bool log)
{
    if (!firmware_info.block_found && BUILD_REQUIRE_FIRMWARE_INFO) {
        if (log) {
            logger.printfln("Failed to update: Firmware has no info page!");
        }

        return "{\"error\":\"no_info_page\"}";
    }

    if (firmware_info.block_found) {
        if (firmware_info.expected_checksum != firmware_info.actual_checksum) {
            if (log) {
                logger.printfln("Failed to update: Firmware info page corrupted! Expected checksum %x, actual checksum %x",
                                firmware_info.expected_checksum, firmware_info.actual_checksum);
            }

            return "{\"error\":\"info_page_corrupted\"}";
        }

        firmware_info.block.firmware_name[ARRAY_SIZE(firmware_info.block.firmware_name) - 1] = '\0';

        if (strcmp(BUILD_DISPLAY_NAME, firmware_info.block.firmware_name) != 0) {
            if (log) {
                logger.printfln("Failed to update: Firmware is for a %s but this is a %s!",
                                firmware_info.block.firmware_name, BUILD_DISPLAY_NAME);
            }

            return "{\"error\":\"wrong_firmware_type\"}";
        }

        if (detect_downgrade && compare_version(firmware_info.block.fw_version[0], firmware_info.block.fw_version[1], firmware_info.block.fw_version[2],
                                                firmware_info.block.fw_version_beta, firmware_info.block.fw_build_time,
                                                BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH,
                                                BUILD_VERSION_BETA, build_timestamp()) < 0) {
            if (log) {
                logger.printfln("Failed to update: Firmware is a downgrade!");
            }

            char buf[256];
            char info_beta[12] = "";
            char build_beta[12] = "";

            if (firmware_info.block.fw_version_beta != 255) {
                snprintf(info_beta, ARRAY_SIZE(info_beta), "-beta.%u", firmware_info.block.fw_version_beta);
            }

            if (BUILD_VERSION_BETA != 255) {
                snprintf(build_beta, ARRAY_SIZE(build_beta), "-beta.%i", BUILD_VERSION_BETA);
            }

            snprintf(buf, ARRAY_SIZE(buf), "{\"error\":\"downgrade\",\"firmware\":\"%u.%u.%u%s+%x\",\"installed\":\"%i.%i.%i%s+%x\"}",
                     firmware_info.block.fw_version[0], firmware_info.block.fw_version[1], firmware_info.block.fw_version[2], info_beta, firmware_info.block.fw_build_time,
                     BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, build_beta, build_timestamp());

            return String(buf);
        }
    }

    return "";
}

bool FirmwareUpdate::handle_firmware_chunk(std::function<void(uint16_t code, const char *, const char *)> result_cb, size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len, size_t remaining, size_t complete_len) {
    if (chunk_offset == 0) {
#if signature_public_key_length != 0
        if (signature_override_cookie != 0) {
            signature_override_cookie = 0;
            Update.abort();
        }
#endif

        if (!Update.begin(complete_len - FIRMWARE_OFFSET, U_FLASH)) {
            logger.printfln("Failed to start update: %s", Update.errorString());
            result_cb(400, "text/plain", Update.errorString());
            Update.abort();
            return false;
        }

        firmware_info.reset();

#if signature_public_key_length != 0
        if (sodium_init() < 0 || crypto_sign_init(&signature_state) < 0) {
            const char *message = "Failed to initialize signature verification";
            logger.printfln(message);
            result_cb(400, "text/plain", message);
            Update.abort();
            return false;
        }

        signature_info.reset();
#endif
    }

#if signature_public_key_length != 0
    signature_info.handle_chunk(chunk_offset, chunk_data, chunk_len);

    if (chunk_offset + chunk_len >= SIGNATURE_INFO_SIGNATURE_OFFSET && chunk_offset < SIGNATURE_INFO_SIGNATURE_OFFSET + SIGNATURE_INFO_SIGNATURE_LENGTH) {
        uint8_t *start = chunk_data;
        size_t len = chunk_len;
        size_t to_skip = 0;

        if (chunk_offset < SIGNATURE_INFO_SIGNATURE_OFFSET) {
            to_skip = SIGNATURE_INFO_SIGNATURE_OFFSET - chunk_offset;
            start += to_skip;
            len -= to_skip;
        }

        if (chunk_offset + chunk_len > SIGNATURE_INFO_SIGNATURE_OFFSET + SIGNATURE_INFO_SIGNATURE_LENGTH) {
            len -= (chunk_offset + chunk_len) - (SIGNATURE_INFO_SIGNATURE_OFFSET + SIGNATURE_INFO_SIGNATURE_LENGTH);
        }

        memset(start, 0x55, len);
    }

    if (crypto_sign_update(&signature_state, chunk_data, chunk_len) < 0) {
        const char *message = "Failed to process signature verification";
        logger.printfln(message);
        result_cb(400, "text/plain", message);
        Update.abort();
        return false;
    }
#endif

    if (firmware_info.handle_chunk(chunk_offset, chunk_data, chunk_len)) {
        String error = this->check_firmware_info(false, true);

        if (!error.isEmpty()) {
            result_cb(400, "application/json", error.c_str());
            Update.abort();
            return false;
        }
    }

    if (chunk_offset + chunk_len < FIRMWARE_OFFSET) {
        return true;
    }

    uint8_t *start = chunk_data;
    size_t len = chunk_len;

    if (chunk_offset < FIRMWARE_OFFSET) {
        size_t to_skip = FIRMWARE_OFFSET - chunk_offset;
        start += to_skip;
        len -= to_skip;
    }

    auto written = Update.write(start, len);
    if (written != len) {
        logger.printfln("Failed to write update chunk with length %u; written %u, error: %s", len, written, Update.errorString());
        result_cb(400, "text/plain", (String("Failed to write update: ") + Update.errorString()).c_str());
        Update.abort();
        return false;
    }

    if (remaining == 0) {
#if signature_public_key_length != 0
        signature_info.block.publisher[ARRAY_SIZE(signature_info.block.publisher) - 1] = '\0';

        if (crypto_sign_final_verify(&signature_state, signature_info.block.signature, signature_public_key_data) < 0) {
            signature_override_cookie = esp_random();

            if (signature_override_cookie == 0) {
                signature_override_cookie = 1;
            }

            char buf[128];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();

            if (signature_info.block.publisher[0] == 0xff) {
                json.addMemberNull("actual_publisher");
            }
            else {
                json.addMemberString("actual_publisher", signature_info.block.publisher);
            }

            json.addMemberString("expected_publisher", signature_publisher);
            json.addMemberNumber("cookie", signature_override_cookie);
            json.endObject();
            json.end();

            logger.printfln("Failed to verify signature");
            result_cb(406, "application/json", buf);
            return false;
        }

        logger.printfln("Update signature is valid, published by %s", signature_info.block.publisher);
#endif

        if (!Update.end(true)) {
            logger.printfln("Failed to apply update: %s", Update.errorString());
            result_cb(400, "text/plain", (String("Failed to apply update: ") + Update.errorString()).c_str());
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

    api.addCommand("firmware_update/override_signature", &override_signature, {}, [this](String &result) {
#if signature_public_key_length != 0
        if (signature_override_cookie == 0) {
            result = "No update pending";
            return;
        }

        uint32_t cookie = override_signature.get("cookie")->asUint();

        if (signature_override_cookie != cookie) {
            result = "Wrong signature override cookie";
            return;
        }

        logger.printfln("Overriding failed signature verification");

        signature_override_cookie = 0;

        if (!Update.end(true)) {
            logger.printfln("Failed to apply update: %s", Update.errorString());
            result = String("Failed to apply update: ") + Update.errorString();
            Update.abort();
            return;
        }

        if(!Update.hasError()) {
            logger.printfln("Firmware flashed successfully! Rebooting in one second.");
            task_scheduler.scheduleOnce([](){ESP.restart();}, 1000);
        }
#else
        result = "Signature verification is disabled";
#endif
    }, true);

    server.on("/check_firmware", HTTP_POST, [this](WebServerRequest request) {
        String error = this->check_firmware_info(true, false);

        if (!error.isEmpty()) {
            return request.send(400, "application/json", error.c_str());
        }

        return request.send(200);
    },
    [this](WebServerRequest request, String filename, size_t offset, uint8_t *data, size_t len, size_t remaining) {
        bool firmware_update_allowed_check_required = true;
#if MODULE_ENERGY_MANAGER_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
        firmware_update_allowed_check_required = energy_manager.disallow_fw_update_with_vehicle_connected();
#endif
        if (firmware_update_allowed_check_required && !firmware_update_allowed) {
            request.send(400, "application/json", "{\"error\":\"vehicle_connected\"}");
            return false;
        }

        if (offset + len > FIRMWARE_INFO_LENGTH) {
            request.send(400, "text/plain", "Too long!");
            return false;
        }

        if (offset == 0) {
            this->firmware_info.reset();
        }

        firmware_info.handle_chunk(FIRMWARE_INFO_OFFSET + offset, data, len);
        return true;
    });

    server.on_HTTPThread("/flash_firmware", HTTP_POST, [this](WebServerRequest request) {
        if(!Update.hasError()) {
            logger.printfln("Firmware flashed successfully! Rebooting in one second.");
            task_scheduler.scheduleOnce([](){ESP.restart();}, 1000);
        }

        return request.send(Update.hasError() ? 400 : 200, "text/plain", Update.hasError() ? Update.errorString() : "Update OK");
    },
    [this](WebServerRequest request, String filename, size_t offset, uint8_t *data, size_t len, size_t remaining) {
        WebServerRequest *request_ptr = &request;

        return handle_firmware_chunk([request_ptr](uint16_t code, const char *mimetype, const char *message) {
            request_ptr->send(code, mimetype, message);
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
    case HTTP_EVENT_ERROR:
        that->handle_update_data(nullptr, 0);
        break;

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
    state.get("update_version")->updateString("");

    if (update_url.length() == 0) {
        logger.printfln("No update URL configured");
        state.get("check_error")->updateString("no_update_url");
        return;
    }

    update_buf_used = 0;
    update_version.major = 255;
    //last_version_timestamp = time(nullptr);
    check_complete = false;

    esp_http_client_config_t http_config = {};

    http_config.url = update_url.c_str();
    http_config.event_handler = update_event_handler;
    http_config.user_data = this;
    http_config.is_async = true;
    http_config.timeout_ms = 50;

    if (cert_id < 0) {
        http_config.crt_bundle_attach = esp_crt_bundle_attach;
    }
    else {
#if MODULE_CERTS_AVAILABLE()
        size_t cert_len = 0;

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
            check_complete = true;
        }

        if (check_complete) {
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
                String update_version_str = "";

                if (update_version.major != 255) {
                    update_version_str = format_version(&update_version);
                }

                state.get("check_error")->updateString("");
                state.get("update_version")->updateString(update_version_str);
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
    if (check_complete) {
        return;
    }

    if (data == nullptr) {
        logger.printfln("HTTP error while downloading firmware list");
        state.get("check_error")->updateString("download_error");
        check_complete = true;
        return;
    }

    int code = esp_http_client_get_status_code(http_client);

    if (code != 200) {
        logger.printfln("HTTP error while downloading firmware list: %d", code);
        state.get("check_error")->updateString("download_error");
        check_complete = true;
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
            check_complete = true;
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

        while (p != nullptr) {
            *p = '\0';

            SemanticVersion version;

            if (!parse_version(update_buf, &version)) {
                logger.printfln("Firmware list entry is malformed: %s", update_buf);
                state.get("check_error")->updateString("list_malformed");
                check_complete = true;
                return;
            }

            // ignore all versions that are older than the current version
            if (compare_version(version.major, version.minor, version.patch, version.beta, version.timestamp,
                                BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, BUILD_VERSION_BETA, build_timestamp()) <= 0) {
                check_complete = true;
                return;
            }

//          if (/* all updates */) {
                update_version = version;
                check_complete = true;
                return;
/*          }
            else { // only stable updates
                // the stable update is the newest version that was
                // released more than 7 days before the next version
                if (version.timestamp + (7 * 24 * 60 * 60) < last_version_timestamp) {
                    update_version = version;
                    check_complete = true;
                    return;
                }

                last_version_timestamp = version.timestamp;
            }

            size_t update_buf_consumed = p + 1 - update_buf;

            memmove(update_buf, p + 1, update_buf_used - update_buf_consumed);

            update_buf_used -= update_buf_consumed;
            update_buf[update_buf_used] = '\0';

            p = strchr(update_buf, '\n');*/
        }
    }
}
