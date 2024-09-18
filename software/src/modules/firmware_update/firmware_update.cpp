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
        {"publisher", Config::Str(signature_publisher, 0, strlen(signature_publisher))},
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
    if (strlen(BUILD_FIRMWARE_UPDATE_URL) > 0) {
        api.restorePersistentConfig("firmware_update/config", &config);
    }

    update_url = config.get("update_url")->asString();

    if (update_url.length() > 0 && !update_url.endsWith("/")) {
        update_url += "/";
    }

    cert_id = config.get("cert_id")->asInt();

#if signature_sodium_public_key_length != 0
    logger.printfln("Firmware is signed by: %s", signature_publisher);

    if (update_url.length() > 0) {
        api.addFeature("firmware_update");
    }
#else
    logger.printfln("Firmware is not signed");
#endif

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

#if signature_sodium_public_key_length != 0
static const char *firmware_url_infix = "_firmware_";
static size_t firmware_url_infix_len = strlen(firmware_url_infix);
static size_t firmware_url_version_len = strlen("MAJ_MIN_PAT_beta_BET_TIMESTAM");
static const char *firmware_url_suffix = "_merged.bin";
static size_t firmware_url_suffix_len = strlen(firmware_url_suffix);
#endif

void FirmwareUpdate::register_urls()
{
    if (strlen(BUILD_FIRMWARE_UPDATE_URL) > 0) {
        api.addPersistentConfig("firmware_update/config", &config);
    }
    else {
        api.addState("firmware_update/config", &config);
    }

    api.addState("firmware_update/state", &state);

    api.addCommand("firmware_update/check_for_update", Config::Null(), {}, [this]() {
        check_for_update();
    }, true);

    api.addCommand("firmware_update/install_firmware", &install_firmware_config, {}, [this]() {
#if signature_sodium_public_key_length == 0
        logger.printfln("Installing firmware from URL is not supported (installed firmware is unsigned)");

        state.get("install_state")->updateEnum(InstallState::NotSupported);
        state.get("install_progress")->updateUint(0);
#else
        String version_str = install_firmware_config.get("version")->asString();
        SemanticVersion version;

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
#endif
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

                    if (check_for_update_in_progress || install_firmware_in_progress) {
                        https_client.abort_async();
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

                    if (check_for_update_in_progress || install_firmware_in_progress) {
                        https_client.abort_async();
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
#if MODULE_EM_V1_AVAILABLE() && !MODULE_EVSE_COMMON_AVAILABLE()
    bool block_firmware_update_with_vehicle_connected = em_v1.block_firmware_update_with_vehicle_connected();
#else
    bool block_firmware_update_with_vehicle_connected = true;
#endif

    return block_firmware_update_with_vehicle_connected && vehicle_connected;
}

static const char *index_url_suffix = "_firmware_v1.txt";
static size_t index_url_suffix_len = strlen(index_url_suffix);

// index files are not signed to allow customer fleet update managment, firmwares are signed
void FirmwareUpdate::check_for_update()
{
#if signature_sodium_public_key_length == 0
    logger.printfln("Checking for firmware update is not supported (installed firmware is unsigned)");

    state.get("check_timestamp")->updateUint(time(nullptr));
    state.get("check_state")->updateEnum(CheckState::NotSupported);
    state.get("update_version")->updateString("");
#else
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

    if (https_client.is_busy()) {
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
    //last_version_timestamp = time(nullptr);
    check_for_update_in_progress = true;

    https_client.download_async(index_url_ptr.get(), cert_id, [this](AsyncHTTPSClientEvent *event) {
        switch (event->type) {
        case AsyncHTTPSClientEventType::Error:
            switch (event->error) {
            case AsyncHTTPSClientError::NoHTTPSURL:
                logger.printfln("No HTTPS update URL");
                state.get("check_state")->updateEnum(CheckState::InternalError);
                break;

            case AsyncHTTPSClientError::Busy:
                logger.printfln("HTTP client is busy");
                state.get("check_state")->updateEnum(CheckState::Busy);
                break;

            case AsyncHTTPSClientError::NoCert:
                logger.printfln("Certificate with ID %d is not available", cert_id);
                state.get("check_state")->updateEnum(CheckState::NoCert);
                break;

            case AsyncHTTPSClientError::NoResponse:
                logger.printfln("Update server %s did not respond", update_url.c_str());
                state.get("check_state")->updateEnum(CheckState::NoResponse);
                break;

            case AsyncHTTPSClientError::ShortRead:
                logger.printfln("Firmware index download ended prematurely");
                state.get("check_state")->updateEnum(CheckState::DownloadShortRead);
                break;

            case AsyncHTTPSClientError::HTTPError:
                logger.printfln("HTTP error while downloading firmware index");
                state.get("check_state")->updateEnum(CheckState::DownloadError);
                break;

            case AsyncHTTPSClientError::HTTPClientInitFailed:
                logger.printfln("Error while creating HTTP client");
                state.get("check_state")->updateEnum(CheckState::HTTPClientInitFailed);
                break;

            case AsyncHTTPSClientError::HTTPClientError:
                logger.printfln("Error while downloading firmware index: %s", esp_err_to_name(event->error_http_client));
                state.get("check_state")->updateEnum(CheckState::DownloadError);
                break;

            case AsyncHTTPSClientError::HTTPStatusError:
                logger.printfln("HTTP error while downloading firmware index: %d", event->error_http_status);
                state.get("check_state")->updateEnum(CheckState::DownloadError);
                break;

            // use default to prevent warnings since we dont use a body, cookies or headers here
            default:
                logger.printfln("Uncovered error, this should never happen!");
                state.get("check_state")->updateEnum(CheckState::InternalError);
                break;
            }

            check_for_update_in_progress = false;
            break;

        case AsyncHTTPSClientEventType::Data:
            handle_index_data(event->data_chunk, event->data_chunk_len);
            break;

        case AsyncHTTPSClientEventType::Aborted:
            if (state.get("check_state")->asEnum<CheckState>() == CheckState::InProgress) {
                logger.printfln("Update check aborted");
                state.get("check_state")->updateEnum(CheckState::Aborted);
            }

            check_for_update_in_progress = false;
            break;

        case AsyncHTTPSClientEventType::Finished:
            handle_index_data("\n", 1);

            if (state.get("check_state")->asEnum<CheckState>() == CheckState::InProgress) {
                logger.printfln("No firmware update available");
                state.get("check_state")->updateEnum(CheckState::Idle);
                state.get("update_version")->updateString("");
            }

            check_for_update_in_progress = false;
            break;
        }
    });
#endif
}

void FirmwareUpdate::handle_index_data(const void *data, size_t data_len)
{
    const char *data_start = static_cast<const char *>(data);
    const char *data_end = data_start + data_len;

    while (data_start < data_end) {
        // fill buffer with new data
        char *index_buf_free_start = index_buf + index_buf_used;
        char *index_buf_free_end = index_buf + sizeof(index_buf) - 1;

        if (index_buf_free_start == index_buf_free_end) {
            logger.printfln("Firmware index is malformed");
            state.get("check_state")->updateEnum(CheckState::IndexMalformed);
            https_client.abort_async();
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

            if (index_buf[0] != '\0') {
                SemanticVersion version;
                bool found_update = false;

                if (!version.from_string(index_buf)) {
                    logger.printfln("Firmware index entry is malformed: %s", index_buf);
                    state.get("check_state")->updateEnum(CheckState::VersionMalformed);
                    https_client.abort_async();
                    return;
                }

                // ignore all versions that are older than the current version
                if (compare_version(version.major, version.minor, version.patch, version.beta, version.timestamp,
                                    BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, BUILD_VERSION_BETA, build_timestamp()) <= 0) {
                    logger.printfln("No firmware update available");
                    state.get("check_state")->updateEnum(CheckState::Idle);
                    state.get("update_version")->updateString("");
                    https_client.abort_async();
                    return;
                }

//              if (/* all updates */) {
                    found_update = true;
/*              }
                else { // only stable updates
                    // the stable update is the newest version that was
                    // released more than 7 days before the next version
                    if (version.timestamp + (7 * 24 * 60 * 60) < last_version_timestamp) {
                        found_update = true;
                    }

                    last_version_timestamp = version.timestamp;
                }*/

                if (found_update) {
                    logger.printfln("Firmware update available: %s", index_buf);
                    state.get("check_state")->updateEnum(CheckState::Idle);
                    state.get("update_version")->updateString(index_buf);
                    https_client.abort_async();
                    return;
                }
            }

            size_t index_buf_consumed = p + 1 - index_buf;

            memmove(index_buf, p + 1, index_buf_used - index_buf_consumed);

            index_buf_used -= index_buf_consumed;
            index_buf[index_buf_used] = '\0';

            p = strchr(index_buf, '\n');
        }
    }
}

void FirmwareUpdate::install_firmware(const char *url)
{
#if signature_sodium_public_key_length == 0
    logger.printfln("Installing firmware from URL is not supported (installed firmware is unsigned)");

    state.get("install_state")->updateEnum(InstallState::NotSupported);
    state.get("install_progress")->updateUint(0);
#else
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

    if (https_client.is_busy()) {
        logger.printfln("HTTP client is busy");
        state.get("install_state")->updateEnum(InstallState::Busy);
        return;
    }

    install_firmware_in_progress = true;

    https_client.download_async(url, cert_id, [this](AsyncHTTPSClientEvent *event) {
        InstallState result;

        switch (event->type) {
        case AsyncHTTPSClientEventType::Error:
            switch (event->error) {
            case AsyncHTTPSClientError::NoHTTPSURL:
                logger.printfln("No HTTPS update URL");
                state.get("install_state")->updateEnum(InstallState::InternalError);
                break;

            case AsyncHTTPSClientError::Busy:
                logger.printfln("HTTP client is busy");
                state.get("install_state")->updateEnum(InstallState::Busy);
                break;

            case AsyncHTTPSClientError::NoCert:
                logger.printfln("Certificate with ID %d is not available", cert_id);
                state.get("install_state")->updateEnum(InstallState::NoCert);
                break;

            case AsyncHTTPSClientError::NoResponse:
                logger.printfln("Update server %s did not respond", update_url.c_str());
                state.get("install_state")->updateEnum(InstallState::NoResponse);
                break;

            case AsyncHTTPSClientError::ShortRead:
                logger.printfln("Firmware download ended prematurely");
                state.get("install_state")->updateEnum(InstallState::DownloadShortRead);
                break;

            case AsyncHTTPSClientError::HTTPError:
                logger.printfln("HTTP error while downloading firmware");
                state.get("install_state")->updateEnum(InstallState::DownloadError);
                break;

            case AsyncHTTPSClientError::HTTPClientInitFailed:
                logger.printfln("Error while creating HTTP client");
                state.get("install_state")->updateEnum(InstallState::HTTPClientInitFailed);
                break;

            case AsyncHTTPSClientError::HTTPClientError:
                logger.printfln("Error while downloading firmware: %s", esp_err_to_name(event->error_http_client));
                state.get("install_state")->updateEnum(InstallState::DownloadError);
                break;

            case AsyncHTTPSClientError::HTTPStatusError:
                logger.printfln("HTTP error while downloading firmware: %d", event->error_http_status);
                state.get("install_state")->updateEnum(InstallState::DownloadError);
                break;

            // use default to prevent warnings since we dont use a body, cookies or headers here
            default:
                logger.printfln("Uncovered error, this should never happen!");
                state.get("check_state")->updateEnum(CheckState::InternalError);
                break;
            }

            Update.abort();
            install_firmware_in_progress = false;
            break;

        case AsyncHTTPSClientEventType::Data:
            if (event->data_complete_len <= FIRMWARE_OFFSET) {
                logger.printfln("Firmware file is too small: %u", event->data_complete_len);
                state.get("install_state")->updateEnum(InstallState::FirmwareTooSmall);
                https_client.abort_async();
                return;
            }

            result = handle_firmware_chunk(event->data_chunk_offset, (uint8_t *)event->data_chunk, event->data_chunk_len, event->data_remaining_len, event->data_complete_len, nullptr);

            if (result != InstallState::InProgress) {
                https_client.abort_async();
            }

            state.get("install_state")->updateEnum(result);
            state.get("install_progress")->updateUint(event->data_chunk_offset * 100 / event->data_complete_len);
            break;

        case AsyncHTTPSClientEventType::Aborted:
            if (state.get("install_state")->asEnum<InstallState>() == InstallState::InProgress) {
                logger.printfln("Firmware install aborted");
                state.get("install_state")->updateEnum(InstallState::Aborted);
                state.get("install_progress")->updateUint(0);
                Update.abort();
            }

            install_firmware_in_progress = false;
            break;

        case AsyncHTTPSClientEventType::Finished:
            logger.printfln("Firmware successfully installed");
            state.get("install_state")->updateEnum(InstallState::Rebooting);
            state.get("install_progress")->updateUint(0);
            trigger_reboot("Firmware update", 1000);
            install_firmware_in_progress = false;
            break;
        }
    });
#endif
}
