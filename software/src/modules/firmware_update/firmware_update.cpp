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
#include "string_builder.h"
#include "check_state.enum.h"
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

// The firmware files are merged with the bootloader, partition table, signature_info,
// firmware_info and slot configuration bins.
// The bootloader starts at offset 0x1000, which is the first byte in the firmware file.
// The first firmware slot (i.e. the one that is flashed over USB) starts at 0x10000.
// So we have to skip the first 0x10000 - 0x1000 bytes, after them the actual firmware starts.
#define FIRMWARE_OFFSET (0x10000 - 0x1000)

#define CHECK_FOR_UPDATE_TIMEOUT 15000

#define INSTALL_FIRMWARE_TIMEOUT 15000

#if !MODULE_CERTS_AVAILABLE()
#define MAX_CERT_ID -1
#endif

#if signature_sodium_public_key_length != 0
static_assert(signature_sodium_public_key_length == crypto_sign_PUBLICKEYBYTES);
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
#if signature_sodium_public_key_length != 0
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
        {"check_state", Config::Uint8(static_cast<uint8_t>(CheckState::Idle))},
        {"update_version", Config::Str("", 0, 32)},
        {"install_progress", Config::Uint(0, 0, 100)},
        {"install_state", Config::Uint8(static_cast<uint8_t>(InstallState::Idle))},
    });

    install_firmware_config = ConfigRoot{Config::Object({
        {"version", Config::Str("", 0, 32)},
    }), [this](Config &update, ConfigSource source) -> String {
        SemanticVersion version;

        if (!version.from_string(update.get("version")->asString().c_str())) {
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

    if (!update_url.endsWith("/")) {
        update_url += "/";
    }

    cert_id = config.get("cert_id")->asInt();
    initialized = true;
}

static void install_state_to_json_error(InstallState state, TFJsonSerializer *json_ptr)
{
    json_ptr->addObject();
    json_ptr->addMemberNumber("error", static_cast<uint8_t>(state));
    json_ptr->endObject();
    json_ptr->end();
}

InstallState FirmwareUpdate::check_firmware_info(bool detect_downgrade, bool log, TFJsonSerializer *json_ptr)
{
    if (!firmware_info.block_found && BUILD_REQUIRE_FIRMWARE_INFO) {
        if (log) {
            logger.printfln("Failed to update: Firmware has no info page!");
        }

        return InstallState::NoInfoPage;
    }

    if (firmware_info.block_found) {
        if (firmware_info.expected_checksum != firmware_info.actual_checksum) {
            if (log) {
                logger.printfln("Failed to update: Firmware info page corrupted! Expected checksum %x, actual checksum %x",
                                firmware_info.expected_checksum, firmware_info.actual_checksum);
            }

            return InstallState::InfoPageCorrupted;
        }

        firmware_info.block.firmware_name[ARRAY_SIZE(firmware_info.block.firmware_name) - 1] = '\0';

        if (strcmp(BUILD_DISPLAY_NAME, firmware_info.block.firmware_name) != 0) {
            if (log) {
                logger.printfln("Failed to update: Firmware is for a %s but this is a %s!",
                                firmware_info.block.firmware_name, BUILD_DISPLAY_NAME);
            }

            return InstallState::WrongFirmwareType;
        }

        if (detect_downgrade && compare_version(firmware_info.block.fw_version[0], firmware_info.block.fw_version[1], firmware_info.block.fw_version[2],
                                                firmware_info.block.fw_version_beta, firmware_info.block.fw_build_time,
                                                BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH,
                                                BUILD_VERSION_BETA, build_timestamp()) < 0) {
            if (log) {
                logger.printfln("Firmware is a downgrade!");
            }

            if (json_ptr != nullptr) {
                char info_beta[12] = "";
                char build_beta[12] = "";

                if (firmware_info.block.fw_version_beta != 255) {
                    snprintf(info_beta, ARRAY_SIZE(info_beta), "-beta.%u", firmware_info.block.fw_version_beta);
                }

                if (BUILD_VERSION_BETA != 255) {
                    snprintf(build_beta, ARRAY_SIZE(build_beta), "-beta.%u", BUILD_VERSION_BETA);
                }

                json_ptr->addObject();
                json_ptr->addMemberNumber("error", static_cast<uint8_t>(InstallState::Downgrade));
                json_ptr->addMemberStringF("firmware_version", "%u.%u.%u%s+%x", firmware_info.block.fw_version[0], firmware_info.block.fw_version[1], firmware_info.block.fw_version[2], info_beta, firmware_info.block.fw_build_time);
                json_ptr->addMemberStringF("installed_version", "%u.%u.%u%s+%x", BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, build_beta, build_timestamp());
                json_ptr->endObject();
                json_ptr->end();
            }

            return InstallState::Downgrade;
        }
    }

    return InstallState::InProgress;
}

InstallState FirmwareUpdate::handle_firmware_chunk(size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len, size_t remaining_len, size_t complete_len, TFJsonSerializer *json_ptr)
{
    if (chunk_offset == 0) {
#if signature_sodium_public_key_length != 0
        if (signature_override_cookie != 0) {
            signature_override_cookie = 0;
            Update.abort();
        }
#endif

        if (!Update.begin(complete_len - FIRMWARE_OFFSET, U_FLASH)) {
            logger.printfln("Failed to begin update: %s", Update.errorString());
            Update.abort();
            return InstallState::FlashBeginFailed;
        }

        firmware_info.reset();

#if signature_sodium_public_key_length != 0
        if (sodium_init() < 0 || crypto_sign_init(&signature_state) < 0) {
            logger.printfln("Failed to begin signature verification");
            Update.abort();
            return InstallState::SignatureBeginFailed;
        }

        signature_info.reset();
#endif
    }

#if signature_sodium_public_key_length != 0
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
        logger.printfln("Failed to update signature verification");
        Update.abort();
        return InstallState::SignatureUpdateFailed;
    }
#endif

    if (firmware_info.handle_chunk(chunk_offset, chunk_data, chunk_len)) {
        InstallState result = check_firmware_info(false, true, json_ptr);

        if (result != InstallState::InProgress) {
            Update.abort();
            return result;
        }
    }

    if (chunk_offset + chunk_len < FIRMWARE_OFFSET) {
        return InstallState::InProgress;
    }

    uint8_t *start = chunk_data;
    size_t len = chunk_len;

    if (chunk_offset < FIRMWARE_OFFSET) {
        size_t to_skip = FIRMWARE_OFFSET - chunk_offset;
        start += to_skip;
        len -= to_skip;
    }

    size_t written = Update.write(start, len);

    if (written != len) {
        logger.printfln("Failed to write update chunk with length %u; written %u, error: %s", len, written, Update.errorString());
        Update.abort();
        return InstallState::FlashShortWrite;
    }

    if (remaining_len == 0) {
#if signature_sodium_public_key_length != 0
        signature_info.block.publisher[ARRAY_SIZE(signature_info.block.publisher) - 1] = '\0';

        if (crypto_sign_final_verify(&signature_state, signature_info.block.signature, signature_sodium_public_key_data) < 0) {
            signature_override_cookie = esp_random();

            if (signature_override_cookie == 0) {
                signature_override_cookie = 1;
            }

            if (json_ptr != nullptr) {
                json_ptr->addObject();
                json_ptr->addMemberNumber("error", static_cast<uint8_t>(InstallState::SignatureVerifyFailed));

                if (signature_info.block.publisher[0] == 0xff) {
                    json_ptr->addMemberNull("actual_publisher");
                }
                else {
                    json_ptr->addMemberString("actual_publisher", signature_info.block.publisher);
                }

                json_ptr->addMemberString("expected_publisher", signature_publisher);
                json_ptr->addMemberNumber("cookie", signature_override_cookie);
                json_ptr->endObject();
                json_ptr->end();
            }

            logger.printfln("Failed to verify signature");
            return InstallState::SignatureVerifyFailed;
        }

        logger.printfln("Update signature is valid, published by %s", signature_info.block.publisher);
#endif

        if (!Update.end(true) || Update.hasError()) {
            logger.printfln("Failed to apply update: %s", Update.errorString());
            return InstallState::FlashApplyFailed;
        }
    }

    return InstallState::InProgress;
}

static const char *firmware_url_infix = "_firmware_";
static size_t firmware_url_infix_len = strlen(firmware_url_infix);
static size_t firmware_url_version_len = strlen("MAJ_MIN_PAT_beta_BET_TIMESTAM");
static const char *firmware_url_suffix = "_merged.bin";
static size_t firmware_url_suffix_len = strlen(firmware_url_suffix);

void FirmwareUpdate::register_urls()
{
    api.addPersistentConfig("firmware_update/config", &config);
    api.addState("firmware_update/state", &state);

    api.addCommand("firmware_update/check_for_update", Config::Null(), {}, [this]() {
        check_for_update();
    }, true);

    api.addCommand("firmware_update/install_firmware", &install_firmware_config, {}, [this]() {
        String version_str = install_firmware_config.get("version")->asString();
        SemanticVersion version;

        // FIXME: check if manual firmware update is in progress

        if (!version.from_string(version_str.c_str())) {
            logger.printfln("Version is malformed: %s", version_str.c_str());
            state.get("install_state")->updateEnum(InstallState::VersionMalformed);
            state.get("install_progress")->updateUint(0);
            return;
        }

        if (update_url.length() == 0) {
            logger.printfln("No update URL configured");
            state.get("install_state")->updateEnum(InstallState::NoUpdateURL);
            state.get("install_progress")->updateUint(0);
            return;
        }

        StringBuilder firmware_url;

        if (firmware_url.setCapacity(update_url.length() + BUILD_NAME_LENGTH + firmware_url_infix_len + firmware_url_version_len + firmware_url_suffix_len) == 0) {
            logger.printfln("Could not build firmware URL");
            state.get("install_state")->updateEnum(InstallState::InternalError);
            state.get("install_progress")->updateUint(0);
            return;
        }

        firmware_url.puts(update_url.c_str(), update_url.length());
        firmware_url.puts(BUILD_NAME, BUILD_NAME_LENGTH);
        firmware_url.puts(firmware_url_infix, firmware_url_infix_len);
        firmware_url.printf("%u_%u_%u", version.major, version.minor, version.patch);

        if (version.beta != 255) {
            firmware_url.printf("_beta_%u", version.beta);
        }

        firmware_url.printf("_%x", version.timestamp);
        firmware_url.puts(firmware_url_suffix, firmware_url_suffix_len);

        std::unique_ptr<char> firmware_url_ptr = firmware_url.take();

        install_firmware(firmware_url_ptr.get());
    }, true);

    api.addCommand("firmware_update/override_signature", &override_signature, {}, [this](String &result) {
#if signature_sodium_public_key_length != 0
        char json_buf[64] = "";
        TFJsonSerializer json{json_buf, sizeof(json_buf)};

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

        if (!Update.end(true) || Update.hasError()) {
            result = "Failed to apply update";
            logger.printfln("%s: %s", result.c_str(), Update.errorString());
            return;
        }

        trigger_reboot("Firmware update", 1000);
#else
        result = "Signature verification is disabled";
#endif
    }, true);

    server.on_HTTPThread("/check_firmware", HTTP_POST, [this](WebServerRequest request) {
        char json_buf[256] = "";
        TFJsonSerializer json{json_buf, sizeof(json_buf)};
        InstallState result = check_firmware_info(true, false, &json);

        task_scheduler.await([this](){check_firmware_in_progress = false;});

        if (result != InstallState::InProgress) {
            if (json_buf[0] == '\0') {
                install_state_to_json_error(result, &json);
            }

            return request.send(400, "application/json", json_buf);
        }

        return request.send(200, "text/plain", "Check OK");
    },
    [this](WebServerRequest request, String filename, size_t offset, uint8_t *data, size_t len, size_t remaining) {
        if (is_vehicle_blocking_update()) {
            char json_buf[64] = "";
            TFJsonSerializer json{json_buf, sizeof(json_buf)};

            install_state_to_json_error(InstallState::VehicleConnected, &json);
            request.send(400, "application/json", json_buf);
            return false;
        }

        if (offset == 0) {
            bool ready = false;

            while (!ready) {
                task_scheduler.await([this, &ready](){
                    check_firmware_in_progress = true;
                    ready = true;

                    if (check_for_update_in_progress) {
                        check_for_update_aborted = true;
                        ready = false;
                    }

                    if (install_firmware_in_progress) {
                        install_firmware_aborted = true;
                        ready = false;
                    }
                });

                delay(100); // wait for other operations to react
            }

            firmware_info.reset();
        }

        if (offset + len > FIRMWARE_INFO_LENGTH) {
            char json_buf[64] = "";
            TFJsonSerializer json{json_buf, sizeof(json_buf)};

            install_state_to_json_error(InstallState::InfoPageTooBig, &json);
            request.send(400, "application/json", json_buf);
            task_scheduler.await([this](){check_firmware_in_progress = false;});
            return false;
        }

        firmware_info.handle_chunk(FIRMWARE_INFO_OFFSET + offset, data, len);
        return true;
    },
    [this](WebServerRequest request, int error_code) {
        logger.printfln("File reception failed: %s (%d)", strerror(error_code), error_code);
        task_scheduler.await([this](){check_firmware_in_progress = false;});
        return request.send(500, "Failed to receive file");
    });

    server.on_HTTPThread("/flash_firmware", HTTP_POST, [this](WebServerRequest request) {
        trigger_reboot("Firmware update", 1000);
        task_scheduler.await([this](){flash_firmware_in_progress = false;});
        return request.send(200, "text/plain", "Update OK");
    },
    [this](WebServerRequest request, String filename, size_t offset, uint8_t *data, size_t len, size_t remaining) {
        if (offset == 0) {
            bool ready = false;

            while (!ready) {
                task_scheduler.await([this, &ready](){
                    flash_firmware_in_progress = true;
                    ready = true;

                    if (check_for_update_in_progress) {
                        check_for_update_aborted = true;
                        ready = false;
                    }

                    if (install_firmware_in_progress) {
                        install_firmware_aborted = true;
                        ready = false;
                    }
                });

                delay(100); // wait for other operations to react
            }
        }

        char json_buf[256] = "";
        TFJsonSerializer json{json_buf, sizeof(json_buf)};

        InstallState result = handle_firmware_chunk(offset, data, len, remaining, request.contentLength(), &json);

        if (result != InstallState::InProgress) {
            if (json_buf[0] == '\0') {
                install_state_to_json_error(result, &json);
            }

            request.send(400, "application/json", json_buf);
            task_scheduler.await([this](){flash_firmware_in_progress = false;});
            return false;
        }

        return true;
    },
    [this](WebServerRequest request, int error_code) {
        logger.printfln("File reception failed: %s (%d)", strerror(error_code), error_code);
        Update.abort();
        task_scheduler.await([this](){flash_firmware_in_progress = false;});
        return request.send(500, "Failed to receive file");
    });
}

bool FirmwareUpdate::is_vehicle_blocking_update() const
{
#if MODULE_ENERGY_MANAGER_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
    bool block_firmware_update_with_vehicle_connected = energy_manager.block_firmware_update_with_vehicle_connected();
#else
    bool block_firmware_update_with_vehicle_connected = true;
#endif

    return block_firmware_update_with_vehicle_connected && vehicle_connected;
}

static esp_err_t index_event_handler(esp_http_client_event_t *event)
{
    FirmwareUpdate *that = static_cast<FirmwareUpdate *>(event->user_data);

    switch (event->event_id) {
    case HTTP_EVENT_ERROR:
        that->handle_index_data(nullptr, 0);
        break;

    case HTTP_EVENT_ON_DATA:
        that->handle_index_data(event->data, event->data_len);
        break;

    case HTTP_EVENT_ON_FINISH:
        that->handle_index_data("\n", 1);
        break;

    default:
        break;
    }

    return ESP_OK;
}

static const char *index_url_suffix = "_firmware_v1.txt";
static size_t index_url_suffix_len = strlen(index_url_suffix);

// index files are not signed to allow customer fleet update managment, firmwares are signed
void FirmwareUpdate::check_for_update()
{
    logger.printfln("Checking for firmware update");

    state.get("check_timestamp")->updateUint(time(nullptr));
    state.get("check_state")->updateEnum(CheckState::InProgress);
    state.get("update_version")->updateString("");

    if (check_firmware_in_progress || flash_firmware_in_progress || install_firmware_in_progress) {
        logger.printfln("Firmware install in progress");
        state.get("check_state")->updateEnum(CheckState::Busy);
        return;
    }

    state.get("install_state")->updateEnum(InstallState::Idle);
    state.get("install_progress")->updateUint(0);

    if (http_client != nullptr) {
        logger.printfln("HTTP client is already in use");
        state.get("check_state")->updateEnum(CheckState::Busy);
        return;
    }

    if (update_url.length() == 0) {
        logger.printfln("No update URL configured");
        state.get("check_state")->updateEnum(CheckState::NoUpdateURL);
        return;
    }

    StringBuilder index_url;

    if (index_url.setCapacity(update_url.length() + BUILD_NAME_LENGTH + index_url_suffix_len) == 0) {
        logger.printfln("Could not build firmware index URL");
        state.get("check_state")->updateEnum(CheckState::InternalError);
        return;
    }

    index_url.puts(update_url.c_str(), update_url.length());
    index_url.puts(BUILD_NAME, BUILD_NAME_LENGTH);
    index_url.puts(index_url_suffix, index_url_suffix_len);

    std::unique_ptr<char> index_url_ptr = index_url.take();

    index_buf_used = 0;
    update_version.major = 255;
    //last_version_timestamp = time(nullptr);
    check_for_update_in_progress = true;
    check_for_update_aborted = false;

    esp_http_client_config_t http_config = {};

    http_config.url = index_url_ptr.get();
    http_config.event_handler = index_event_handler;
    http_config.user_data = this;
    http_config.is_async = true;
    http_config.timeout_ms = 50;
    http_config.buffer_size = 1024;
    http_config.buffer_size_tx = 256;

    if (cert_id < 0) {
        http_config.crt_bundle_attach = esp_crt_bundle_attach;
    }
    else {
#if MODULE_CERTS_AVAILABLE()
        size_t cert_len = 0;

        cert = certs.get_cert(static_cast<uint8_t>(cert_id), &cert_len);

        if (cert == nullptr) {
            logger.printfln("Certificate with ID %d is not available", cert_id);
            state.get("check_state")->updateEnum(CheckState::NoCert);
            check_for_update_in_progress = false;
            return;
        }

        http_config.cert_pem = (const char *)cert.get();
#else
        // defense in depth: it should not be possible to arrive here because in case
        // that the certs module is not available the cert_id should always be -1
        logger.printfln("Can't use custom certificate: certs module is not built into this firmware!");
        state.get("check_state")->updateEnum(CheckState::NoCert);
        check_for_update_in_progress = false;
        return;
#endif
    }

    http_client = esp_http_client_init(&http_config);

    if (http_client == nullptr) {
        logger.printfln("Error while creating HTTP client");
        state.get("check_state")->updateEnum(CheckState::HTTPClientInitFailed);
        cert.reset();
        check_for_update_in_progress = false;
        return;
    }

    check_begin = millis();

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (deadline_elapsed(check_begin + CHECK_FOR_UPDATE_TIMEOUT)) {
            logger.printfln("Update server %s did not respond", update_url.c_str());
            state.get("check_state")->updateEnum(CheckState::NoResponse);
            check_for_update_aborted = true;
        }

        if (check_for_update_aborted) {
            if (state.get("check_state")->asEnum<CheckState>() == CheckState::InProgress) {
                logger.printfln("Update check aborted");
                state.get("check_state")->updateEnum(CheckState::Aborted);
            }

            esp_http_client_close(http_client);
        }
        else {
            esp_err_t err = esp_http_client_perform(http_client);

            if (err == ESP_ERR_HTTP_EAGAIN) {
                return;
            }
            else if (err != ESP_OK) {
                logger.printfln("Error while downloading firmware index: %s", esp_err_to_name(err));
                state.get("check_state")->updateEnum(CheckState::DownloadError);
            }
            else if (state.get("check_state")->asEnum<CheckState>() == CheckState::InProgress) {
                char update_version_buf[SEMANTIC_VERSION_MAX_STRING_LENGTH] = "";

                if (update_version.major != 255) {
                    update_version.to_string(update_version_buf, ARRAY_SIZE(update_version_buf));
                    logger.printfln("Firmware update available: %s", update_version_buf);
                }
                else {
                    logger.printfln("No firmware update available");
                }

                state.get("check_state")->updateEnum(CheckState::Idle);
                state.get("update_version")->updateString(update_version_buf);
            }
        }

        esp_http_client_cleanup(http_client);
        http_client = nullptr;
        cert.reset();
        check_for_update_in_progress = false;

        task_scheduler.cancel(task_scheduler.currentTaskId());
    }, 100, 100);
}

void FirmwareUpdate::handle_index_data(const void *data, size_t data_len)
{
    if (check_for_update_aborted) {
        return;
    }

    if (data == nullptr) {
        logger.printfln("HTTP error while downloading firmware index");
        state.get("check_state")->updateEnum(CheckState::DownloadError);
        check_for_update_aborted = true;
        return;
    }

    int code = esp_http_client_get_status_code(http_client);

    if (code != 200) {
        logger.printfln("HTTP error while downloading firmware index: %d", code);
        state.get("check_state")->updateEnum(CheckState::DownloadError);
        check_for_update_aborted = true;
        return;
    }

    const char *data_start = static_cast<const char *>(data);
    const char *data_end = data_start + data_len;

    while (data_start < data_end && !check_for_update_aborted) {
        // fill buffer with new data
        char *index_buf_free_start = index_buf + index_buf_used;
        char *index_buf_free_end = index_buf + sizeof(index_buf) - 1;

        if (index_buf_free_start == index_buf_free_end) {
            logger.printfln("Firmware index is malformed");
            state.get("check_state")->updateEnum(CheckState::IndexMalformed);
            check_for_update_aborted = true;
            return;
        }

        while (index_buf_free_start < index_buf_free_end && data_start < data_end) {
            if (*data_start != ' ' && *data_start != '\t' && *data_start != '\r') {
                *index_buf_free_start++ = *data_start;
            }

            ++data_start;
        }

        *index_buf_free_start = '\0';
        index_buf_used = index_buf_free_start - index_buf;

        // parse version
        char *p = strchr(index_buf, '\n');

        while (p != nullptr) {
            *p = '\0';

            SemanticVersion version;

            if (!version.from_string(index_buf)) {
                logger.printfln("Firmware index entry is malformed: %s", index_buf);
                state.get("check_state")->updateEnum(CheckState::VersionMalformed);
                check_for_update_aborted = true;
                return;
            }

            // ignore all versions that are older than the current version
            if (compare_version(version.major, version.minor, version.patch, version.beta, version.timestamp,
                                BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, BUILD_VERSION_BETA, build_timestamp()) <= 0) {
                check_for_update_aborted = true;
                return;
            }

//          if (/* all updates */) {
                update_version = version;
                check_for_update_aborted = true;
                return;
/*          }
            else { // only stable updates
                // the stable update is the newest version that was
                // released more than 7 days before the next version
                if (version.timestamp + (7 * 24 * 60 * 60) < last_version_timestamp) {
                    update_version = version;
                    check_for_update_aborted = true;
                    return;
                }

                last_version_timestamp = version.timestamp;
            }

            size_t index_buf_consumed = p + 1 - index_buf;

            memmove(index_buf, p + 1, index_buf_used - index_buf_consumed);

            index_buf_used -= index_buf_consumed;
            index_buf[index_buf_used] = '\0';

            p = strchr(index_buf, '\n');*/
        }
    }
}

static esp_err_t firmware_event_handler(esp_http_client_event_t *event)
{
    FirmwareUpdate *that = static_cast<FirmwareUpdate *>(event->user_data);

    switch (event->event_id) {
    case HTTP_EVENT_ERROR:
        that->handle_firmware_data(nullptr, 0);
        break;

    case HTTP_EVENT_ON_DATA:
        that->handle_firmware_data(event->data, event->data_len);
        break;

    default:
        break;
    }

    return ESP_OK;
}

void FirmwareUpdate::install_firmware(const char *url)
{
    logger.printfln("Installing firmware: %s", url);

    state.get("install_state")->updateEnum(InstallState::InProgress);
    state.get("install_progress")->updateUint(0);

    if (check_firmware_in_progress || flash_firmware_in_progress || check_for_update_in_progress) {
        logger.printfln("Firmware install or check for update in progress");
        state.get("install_state")->updateEnum(InstallState::Busy);
        return;
    }

    if (is_vehicle_blocking_update()) {
        logger.printfln("Cannot install firmware while a vehicle is connected");
        state.get("install_state")->updateEnum(InstallState::VehicleConnected);
        return;
    }

    if (http_client != nullptr) {
        logger.printfln("HTTP client is busy");
        state.get("install_state")->updateEnum(InstallState::Busy);
        return;
    }

    firmware_data_offset = 0;
    firmware_len = 0;
    install_firmware_in_progress = true;
    install_firmware_aborted = false;

    esp_http_client_config_t http_config = {};

    http_config.url = url;
    http_config.event_handler = firmware_event_handler;
    http_config.user_data = this;
    http_config.is_async = true;
    http_config.timeout_ms = 50;
    http_config.buffer_size = 1024;
    http_config.buffer_size_tx = 256;

    if (cert_id < 0) {
        http_config.crt_bundle_attach = esp_crt_bundle_attach;
    }
    else {
#if MODULE_CERTS_AVAILABLE()
        size_t cert_len = 0;

        cert = certs.get_cert(static_cast<uint8_t>(cert_id), &cert_len);

        if (cert == nullptr) {
            logger.printfln("Certificate with ID %d is not available", cert_id);
            state.get("install_state")->updateEnum(InstallState::NoCert);
            install_firmware_in_progress = false;
            return;
        }

        http_config.cert_pem = (const char *)cert.get();
#else
        // defense in depth: it should not be possible to arrive here because in case
        // that the certs module is not available the cert_id should always be -1
        logger.printfln("Can't use custom certificate: certs module is not built into this firmware!");
        state.get("install_state")->updateEnum(InstallState::NoCert);
        install_firmware_in_progress = false;
        return;
#endif
    }

    http_client = esp_http_client_init(&http_config);

    if (http_client == nullptr) {
        logger.printfln("Error while creating HTTP client");
        state.get("install_state")->updateEnum(InstallState::HTTPClientInitFailed);
        cert.reset();
        install_firmware_in_progress = false;
        return;
    }

    last_install_alive = millis();

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (deadline_elapsed(last_install_alive + INSTALL_FIRMWARE_TIMEOUT)) {
            logger.printfln("Update server %s did not respond", update_url.c_str());
            state.get("install_state")->updateEnum(InstallState::NoResponse);
            Update.abort();
            install_firmware_aborted = true;
        }

        if (install_firmware_aborted) {
            if (state.get("install_state")->asEnum<InstallState>() == InstallState::InProgress) {
                logger.printfln("Firmware install aborted");
                state.get("install_state")->updateEnum(InstallState::Aborted);
                state.get("install_progress")->updateUint(0);
                Update.abort();
            }

            esp_http_client_close(http_client);
        }
        else {
            esp_err_t err = esp_http_client_perform(http_client);

            if (err == ESP_ERR_HTTP_EAGAIN) {
                return;
            }
            else if (err != ESP_OK) {
                logger.printfln("Error while downloading firmware file: %s", esp_err_to_name(err));
                state.get("install_state")->updateEnum(InstallState::DownloadError);
            }
            else if (state.get("install_state")->asEnum<InstallState>() == InstallState::InProgress) {
                if (firmware_len > 0 && firmware_data_offset == firmware_len) {
                    logger.printfln("Firmware successfully installed");
                    state.get("install_state")->updateEnum(InstallState::Rebooting);
                    state.get("install_progress")->updateUint(0);
                    trigger_reboot("Firmware update", 1000);
                }
                else {
                    logger.printfln("Firmware download ended prematurely");
                    state.get("install_state")->updateEnum(InstallState::DownloadShortRead);
                    Update.abort();
                }
            }
        }

        esp_http_client_cleanup(http_client);
        http_client = nullptr;
        cert.reset();
        install_firmware_in_progress = false;

        task_scheduler.cancel(task_scheduler.currentTaskId());
    }, 100, 100);
}

void FirmwareUpdate::handle_firmware_data(void *data, size_t data_len)
{
    if (install_firmware_aborted) {
        return;
    }

    last_install_alive = millis();

    if (data == nullptr) {
        logger.printfln("HTTP error while downloading firmware file");
        state.get("install_state")->updateEnum(InstallState::DownloadError);
        Update.abort();
        install_firmware_aborted = true;
        return;
    }

    int code = esp_http_client_get_status_code(http_client);

    if (code != 200) {
        logger.printfln("HTTP error while downloading firmware file: %d", code);
        state.get("install_state")->updateEnum(InstallState::DownloadError);
        Update.abort();
        install_firmware_aborted = true;
        return;
    }

    if (firmware_data_offset == 0) {
        firmware_len = (size_t)esp_http_client_get_content_length(http_client);

        if (firmware_len <= FIRMWARE_OFFSET) {
            logger.printfln("Firmware file is too small: %u", firmware_len);
            state.get("install_state")->updateEnum(InstallState::FirmwareTooSmall);
            Update.abort();
            install_firmware_aborted = true;
            return;
        }
    }

    InstallState result = handle_firmware_chunk(firmware_data_offset, (uint8_t *)data, data_len, firmware_len - firmware_data_offset - data_len, firmware_len, nullptr);

    if (result == InstallState::InProgress) {
        firmware_data_offset += data_len;
    }
    else {
        install_firmware_aborted = true;
    }

    state.get("install_state")->updateEnum(result);
    state.get("install_progress")->updateUint(firmware_data_offset * 100 / firmware_len);
}
