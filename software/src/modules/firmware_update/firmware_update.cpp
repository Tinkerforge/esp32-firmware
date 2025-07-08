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

#include <esp_app_format.h>
#include <esp_rom_crc.h>
#include <spi_flash_mmap.h>
#include <Update.h>
#include <TFJson.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "options.h"
#include "build.h"
#include "tools/string_builder.h"
#include "tools/semantic_version.h"
#include "check_state.enum.h"

static const size_t options_product_id_length = constexpr_strlen(OPTIONS_PRODUCT_ID());

static const SemanticVersion build_version{BUILD_VERSION_MAJOR, BUILD_VERSION_MINOR, BUILD_VERSION_PATCH, BUILD_VERSION_BETA, build_timestamp()};

// Newer firmwares contain a firmware info page.
#define FIRMWARE_INFO_OFFSET (0xd000 - 0x1000)
#define FIRMWARE_INFO_LENGTH 0x1000

static const uint8_t firmware_info_magic[BLOCK_READER_MAGIC_LENGTH] = {0x71, 0x21, 0xCE, 0x12, 0xF0, 0x12, 0x6E};

// Signed firmwares contain a signature info page.
#define SIGNATURE_INFO_OFFSET (0xc000 - 0x1000)
#define SIGNATURE_INFO_LENGTH 0x1000

static const uint8_t signature_info_magic[BLOCK_READER_MAGIC_LENGTH] = {0xE6, 0x21, 0x0F, 0x21, 0xEC, 0x12, 0x17};

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

#if OPTIONS_FIRMWARE_UPDATE_ENABLE_ROLLBACK()
static const bool enable_rollback = true;
#else
static const bool enable_rollback = false;
#endif

// override weakly linked arduino-esp32 function to stop arduino-esp32
// initArduino() from calling esp_ota_mark_app_valid_cancel_rollback()
extern "C" bool verifyRollbackLater()
{
    return enable_rollback;
}

static const char *get_esp_ota_img_state_name(esp_ota_img_states_t ota_state)
{
    switch (ota_state) {
    case ESP_OTA_IMG_NEW:            return "new";            // Monitor the first boot. In bootloader this state is changed to ESP_OTA_IMG_PENDING_VERIFY.
    case ESP_OTA_IMG_PENDING_VERIFY: return "pending-verify"; // First boot for this app was. If while the second boot this state is then it will be changed to ABORTED.
    case ESP_OTA_IMG_VALID:          return "valid";          // App was confirmed as workable. App can boot and work without limits.
    case ESP_OTA_IMG_INVALID:        return "invalid";        // App was confirmed as non-workable. This app will not selected to boot at all.
    case ESP_OTA_IMG_ABORTED:        return "aborted";        // App could not confirm the workable or non-workable. In bootloader IMG_PENDING_VERIFY state will be changed to IMG_ABORTED. This app will not selected to boot at all.
    case ESP_OTA_IMG_UNDEFINED:      return "undefined";      // Undefined. App can boot and work without limits.
    default:                         return "<unknown>";
    }
}

static const char *get_partition_ota_state_name(const esp_partition_t *partition, esp_ota_img_states_t *ota_state)
{
    esp_err_t err = esp_ota_get_state_partition(partition, ota_state);

    if (err != ESP_OK) {
        logger.printfln("Could not get %s partition state: %d", partition->label, err);
        return "<error>";
    }
    else {
        return get_esp_ota_img_state_name(*ota_state);
    }
}

static bool read_custom_app_desc(const esp_partition_t *partition, build_custom_app_desc_t *custom_app_desc, char *fw_version, size_t fw_version_len)
{
    esp_err_t err = esp_partition_read(partition,
                                       sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t),
                                       custom_app_desc,
                                       sizeof(build_custom_app_desc_t));

    if (err != ESP_OK) {
        logger.printfln("Could not read %s partition custom app description: %d", partition->label, err);
        return false;
    }

    if (custom_app_desc->magic != BUILD_CUSTOM_APP_DESC_MAGIC) {
        logger.printfln("Custom app description of %s partition has wrong magic: 0x%08lx", partition->label, custom_app_desc->magic);
        return false;
    }

    if (custom_app_desc->version < 1) {
        logger.printfln("Custom app description of %s partition has unknown version: %u", partition->label, custom_app_desc->version);
        return false;
    }

    SemanticVersion{custom_app_desc->fw_version_major, custom_app_desc->fw_version_minor, custom_app_desc->fw_version_patch,
                    custom_app_desc->fw_version_beta, custom_app_desc->fw_build_timestamp}.to_string(fw_version, fw_version_len);

    return true;
}

template <typename T>
BlockReader<T>::BlockReader(size_t block_offset, size_t block_len, const uint8_t expected_magic[BLOCK_READER_MAGIC_LENGTH]) : block_offset(block_offset), block_len(block_len)
{
    memcpy(this->expected_magic, expected_magic, BLOCK_READER_MAGIC_LENGTH);

    reset();
}

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

        actual_checksum = esp_rom_crc32_le(actual_checksum, start, len);

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

        block_found = read_expected_checksum_len == sizeof(expected_checksum) && memcmp(block.magic, expected_magic, BLOCK_READER_MAGIC_LENGTH) == 0;
    }

    return chunk_offset + chunk_len >= block_offset + block_len;
}

FirmwareUpdate::FirmwareUpdate() :
    firmware_info(FIRMWARE_INFO_OFFSET, FIRMWARE_INFO_LENGTH, firmware_info_magic)
#if signature_sodium_public_key_length != 0
    , signature_info(SIGNATURE_INFO_OFFSET, SIGNATURE_INFO_LENGTH, signature_info_magic)
#endif
{
}

void FirmwareUpdate::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"update_url", Config::Str(OPTIONS_FIRMWARE_UPDATE_UPDATE_URL(), 0, 128)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
    }), [this](Config &update, ConfigSource source) -> String {
        const String &update_url = update.get("update_url")->asString();

        if (update_url.length() > 0 && !update_url.startsWith("https://"))
            return "HTTPS required for update URL";

        return "";
    }};

    state = Config::Object({
        {"publisher", Config::Str(signature_publisher, 0, strlen(signature_publisher))},
        {"check_timestamp", Config::Uint(0)},
        {"check_state", Config::Enum<CheckState>(CheckState::Idle)},
        {"update_version", Config::Str("", 0, SEMANTIC_VERSION_MAX_STRING_LENGTH)},
        {"install_progress", Config::Uint(0, 0, 100)},
        {"install_state", Config::Enum<InstallState>(InstallState::Idle)},
        {"running_partition", Config::Str("", 0, 16)},
        {"app0_state", Config::Uint(ESP_OTA_IMG_INVALID)},
        {"app0_version", Config::Str("", 0, SEMANTIC_VERSION_MAX_STRING_LENGTH)},
        {"app1_state", Config::Uint(ESP_OTA_IMG_INVALID)},
        {"app1_version", Config::Str("", 0, SEMANTIC_VERSION_MAX_STRING_LENGTH)},
        {"rolled_back_version", Config::Str("", 0, SEMANTIC_VERSION_MAX_STRING_LENGTH)},
    });

    install_firmware_config = ConfigRoot{Config::Object({
        {"version", Config::Str("", 0, SEMANTIC_VERSION_MAX_STRING_LENGTH)},
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
    if (strlen(OPTIONS_FIRMWARE_UPDATE_UPDATE_URL()) > 0) {
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

    read_app_partition_state();

    if (enable_rollback) {
        task_scheduler.scheduleOnce([this]() {
            change_running_partition_from_pending_verify_to_valid();
        }, 5_min);
    }

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
    if (!firmware_info.block_found && OPTIONS_FIRMWARE_UPDATE_REQUIRE_FIRMWARE_INFO()) {
        if (log) {
            logger.printfln("Failed to update: Firmware has no info page!");
        }

        return InstallState::NoInfoPage;
    }

    if (firmware_info.block_found) {
        if (firmware_info.expected_checksum != firmware_info.actual_checksum) {
            if (log) {
                logger.printfln("Failed to update: Firmware info page corrupted! Expected checksum 0x%08lx, actual checksum 0x%08lx",
                                firmware_info.expected_checksum, firmware_info.actual_checksum);
            }

            return InstallState::InfoPageCorrupted;
        }

        firmware_info.block.product_name[ARRAY_SIZE(firmware_info.block.product_name) - 1] = '\0';
        firmware_info.block.product_id[ARRAY_SIZE(firmware_info.block.product_id) - 1] = '\0';

        if (firmware_info.block.version >= 3) {
            if (strcmp(OPTIONS_PRODUCT_ID(), firmware_info.block.product_id) != 0) {
                if (log) {
                    logger.printfln("Failed to update: Firmware is for %s but this is %s!",
                                    firmware_info.block.product_id, OPTIONS_PRODUCT_ID());
                }

                return InstallState::WrongFirmwareType;
            }
        }
        else if (strcmp(OPTIONS_PRODUCT_NAME(), firmware_info.block.product_name) != 0) {
            if (log) {
                logger.printfln("Failed to update: Firmware is for a %s but this is a %s!",
                                firmware_info.block.product_name, OPTIONS_PRODUCT_NAME());
            }

            return InstallState::WrongFirmwareType;
        }

        SemanticVersion fw_version{firmware_info.block.fw_version_major, firmware_info.block.fw_version_minor, firmware_info.block.fw_version_patch,
                                   firmware_info.block.fw_version_beta, firmware_info.block.fw_build_timestamp};

        if (detect_downgrade && fw_version.compare(build_version) < 0) {
            if (log) {
                logger.printfln("Firmware is a downgrade!");
            }

            if (json_ptr != nullptr) {
                char fw_version_str[SEMANTIC_VERSION_MAX_STRING_LENGTH] = "<unknown>";

                fw_version.to_string(fw_version_str, ARRAY_SIZE(fw_version_str));

                json_ptr->addObject();
                json_ptr->addMemberNumber("error", static_cast<uint8_t>(InstallState::Downgrade));
                json_ptr->addMemberString("firmware_version", fw_version_str);
                json_ptr->addMemberString("installed_version", build_version_full_str());
                json_ptr->endObject();
                json_ptr->end();
            }

            return InstallState::Downgrade;
        }
    }

    return InstallState::InProgress;
}

InstallState FirmwareUpdate::handle_firmware_chunk(size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len, size_t complete_len, bool is_complete, TFJsonSerializer *json_ptr)
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

    if (is_complete) {
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

static void boot_other_partition(const char *other_partition_label, String &errmsg)
{
    const esp_partition_t *running_partition = esp_ota_get_running_partition();

    if (running_partition == nullptr) {
        errmsg = "Could not get running partition";
        return;
    }

    if (other_partition_label != nullptr) {
        if (strcmp(running_partition->label, other_partition_label) == 0) {
            errmsg = String("Partition ") + running_partition->label + (" is already running");
            return;
        }
    }
    else if (strcmp(running_partition->label, "app0") == 0) {
        other_partition_label = "app1";
    }
    else if (strcmp(running_partition->label, "app1") == 0) {
        other_partition_label = "app0";
    }
    else {
        errmsg = String("Unexpected running partition: ") + running_partition->label;
        return;
    }

    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP,
                                                     ESP_PARTITION_SUBTYPE_ANY,
                                                     nullptr);

    while (it != nullptr) {
        const esp_partition_t *partition = esp_partition_get(it);

        if (strcmp(partition->label, other_partition_label) == 0) {
            esp_partition_iterator_release(it);

            esp_err_t err = esp_ota_set_boot_partition(partition);

            if (err != ESP_OK) {
                String err_str;

                if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
                    err_str = "firmware is missing or invalid";
                }
                else {
                    err_str = String(err);
                }

                errmsg = String("Could not set ") + partition->label + (" as boot partition: ") + err_str;
            }
            else {
                logger.printfln("Booting %s partition", other_partition_label);
                trigger_reboot("booting other partition", 1_s);
            }

            return;
        }

        it = esp_partition_next(it);
    }

    errmsg = String("Other partition not found: ") + other_partition_label;
}

void FirmwareUpdate::register_urls()
{
    if (strlen(OPTIONS_FIRMWARE_UPDATE_UPDATE_URL()) > 0) {
        api.addPersistentConfig("firmware_update/config", &config);
    }
    else {
        api.addState("firmware_update/config", &config);
    }

    api.addState("firmware_update/state", &state);

    api.addCommand("firmware_update/check_for_update", Config::Null(), {}, [this](String &/*errmsg*/) {
        check_for_update();
    }, true);

    api.addCommand("firmware_update/install_firmware", &install_firmware_config, {}, [this](String &/*errmsg*/) {
#if signature_sodium_public_key_length == 0
        logger.printfln("Installing firmware from URL is not supported (running firmware is unsigned)");

        state.get("install_state")->updateEnum(InstallState::NotSupported);
        state.get("install_progress")->updateUint(0);
#else
        const String &version_str = install_firmware_config.get("version")->asString();
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

        if (firmware_url.setCapacity(update_url.length() + options_product_id_length + firmware_url_infix_len + firmware_url_version_len + firmware_url_suffix_len) == 0) {
            logger.printfln("Could not build firmware URL");
            state.get("install_state")->updateEnum(InstallState::InternalError);
            state.get("install_progress")->updateUint(0);
            return;
        }

        firmware_url.puts(update_url.c_str(), update_url.length());
        firmware_url.puts(OPTIONS_PRODUCT_ID(), options_product_id_length);
        firmware_url.puts(firmware_url_infix, firmware_url_infix_len);
        firmware_url.printf("%u_%u_%u", version.major, version.minor, version.patch);

        if (version.beta != 255) {
            firmware_url.printf("_beta_%u", version.beta);
        }

        firmware_url.printf("_%lx", version.timestamp);
        firmware_url.puts(firmware_url_suffix, firmware_url_suffix_len);

        char *firmware_url_ptr = firmware_url.take();

        install_firmware(firmware_url_ptr);

        free(firmware_url_ptr);
#endif
    }, true);

    api.addCommand("firmware_update/override_signature", &override_signature, {}, [this](String &errmsg) {
#if signature_sodium_public_key_length != 0
        char json_buf[64] = "";
        TFJsonSerializer json{json_buf, sizeof(json_buf)};

        if (signature_override_cookie == 0) {
            errmsg = "No update pending";
            return;
        }

        uint32_t cookie = override_signature.get("cookie")->asUint();

        if (signature_override_cookie != cookie) {
            errmsg = "Wrong signature override cookie";
            return;
        }

        logger.printfln("Overriding failed signature verification");

        signature_override_cookie = 0;

        if (!Update.end(true) || Update.hasError()) {
            errmsg = "Failed to apply update";
            logger.printfln("%s: %s", errmsg.c_str(), Update.errorString());
            return;
        }

        trigger_reboot("firmware update", 1_s);
#else
        errmsg = "Signature verification is disabled";
#endif
    }, true);

    api.addCommand("firmware_update/reboot_app0", Config::Null(), {}, [this](String &errmsg) {
        boot_other_partition("app0", errmsg);
    }, true);

    api.addCommand("firmware_update/reboot_app1", Config::Null(), {}, [this](String &errmsg) {
        boot_other_partition("app1", errmsg);
    }, true);

    api.addCommand("firmware_update/reboot_other", Config::Null(), {}, [this](String &errmsg) {
        boot_other_partition(nullptr, errmsg);
    }, true);

    api.addCommand("firmware_update/clear_rolled_back_version", Config::Null(), {}, [this](String &errmsg) {
        if (change_update_partition_from_aborted_to_invalid() > 0) {
            state.get("rolled_back_version")->updateString("");
        }
    }, true);

    api.addCommand("firmware_update/validate", Config::Null(), {}, [this](String &errmsg) {
        change_running_partition_from_pending_verify_to_valid();
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

            return request.send(400, "application/json", json_buf, json.buf_strlen);
        }

        return request.send(200, "text/plain", "Check OK");
    },
    [this](WebServerRequest request, String filename, size_t offset, uint8_t *data, size_t len, size_t remaining) {
        if (is_vehicle_blocking_update()) {
            char json_buf[64] = "";
            TFJsonSerializer json{json_buf, sizeof(json_buf)};

            install_state_to_json_error(InstallState::VehicleConnected, &json);
            request.send(400, "application/json", json_buf, json.buf_strlen);
            return false;
        }

        if (offset == 0) {
            bool ready = false;

            while (!ready) {
                auto result = task_scheduler.await([this, &ready](){
                    check_firmware_in_progress = true;
                    ready = true;

                    if (check_for_update_in_progress || install_firmware_in_progress) {
                        https_client.abort_async();
                        ready = false;
                    }
                });

                if (result != TaskScheduler::AwaitResult::Done) {
                    return false;
                }

                delay(100); // wait for other operations to react
            }

            firmware_info.reset();
        }

        if (offset + len > FIRMWARE_INFO_LENGTH) {
            char json_buf[64] = "";
            TFJsonSerializer json{json_buf, sizeof(json_buf)};

            install_state_to_json_error(InstallState::InfoPageTooBig, &json);
            request.send(400, "application/json", json_buf, json.buf_strlen);
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
        trigger_reboot("firmware update", 1_s);
        task_scheduler.await([this](){flash_firmware_in_progress = false;});
        return request.send(200, "text/plain", "Update OK");
    },
    [this](WebServerRequest request, String filename, size_t offset, uint8_t *data, size_t len, size_t remaining) {
        if (offset == 0) {
            bool ready = false;

            while (!ready) {
                auto result = task_scheduler.await([this, &ready](){
                    flash_firmware_in_progress = true;
                    ready = true;

                    if (check_for_update_in_progress || install_firmware_in_progress) {
                        https_client.abort_async();
                        ready = false;
                    }
                });

                if (result != TaskScheduler::AwaitResult::Done) {
                    return false;
                }

                delay(100); // wait for other operations to react
            }

            logger.printfln("Installing firmware from file upload");
        }

        char json_buf[256] = "";
        TFJsonSerializer json{json_buf, sizeof(json_buf)};

        InstallState result = handle_firmware_chunk(offset, data, len, request.contentLength(), remaining == 0, &json);

        if (result != InstallState::InProgress) {
            if (json_buf[0] == '\0') {
                install_state_to_json_error(result, &json);
            }

            request.send(400, "application/json", json_buf, json.buf_strlen);
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

void FirmwareUpdate::pre_reboot()
{
    change_running_partition_from_pending_verify_to_new();
}

static bool read_ota_data(size_t ota_index, esp_ota_select_entry_t *ota_data, bool silent)
{
    const esp_partition_t *ota_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, nullptr);

    if (ota_partition == nullptr) {
        if (!silent) {
            logger.printfln("Could not find OTA partition");
        }

        return false;
    }

    esp_err_t err = esp_partition_read(ota_partition, SPI_FLASH_SEC_SIZE * ota_index, ota_data, sizeof(esp_ota_select_entry_t));

    if (err != ESP_OK) {
        if (!silent) {
            logger.printfln("Could not read OTA partition page %d: %d", ota_index, err);
        }

        return false;
    }

    return true;
}

static bool write_ota_data(size_t ota_index, esp_ota_select_entry_t *ota_data, bool silent)
{
    const esp_partition_t *ota_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_OTA, nullptr);

    if (ota_partition == nullptr) {
        if (!silent) {
            logger.printfln("Could not find OTA partition");
        }

        return false;
    }

    esp_err_t err = esp_partition_erase_range(ota_partition, SPI_FLASH_SEC_SIZE * ota_index, SPI_FLASH_SEC_SIZE);

    if (err != ESP_OK) {
        if (!silent) {
            logger.printfln("Could not erase OTA partition page %d: %d", ota_index, err);
        }

        return false;
    }

    err = esp_partition_write(ota_partition, SPI_FLASH_SEC_SIZE * ota_index, ota_data, sizeof(esp_ota_select_entry_t));

    if (err != ESP_OK) {
        if (!silent) {
            logger.printfln("Could not write OTA partition page %d: %d", ota_index, err);
        }

        return false;
    }

    return true;
}

int FirmwareUpdate::change_partition_ota_state_from_to(const esp_partition_t *partition, esp_ota_img_states_t old_ota_state, esp_ota_img_states_t new_ota_state, bool silent)
{
    size_t ota_index;
    const char *app_state_key;

    if (strcmp(partition->label, "app0") == 0) {
        ota_index = 0;
        app_state_key = "app0_state";
    }
    else if (strcmp(partition->label, "app1") == 0) {
        ota_index = 1;
        app_state_key = "app1_state";
    }
    else {
        if (!silent) {
            logger.printfln("Unexpected partition: %s", partition->label);
        }

        return -1;
    }

    esp_ota_select_entry_t ota_data;

    if (!read_ota_data(ota_index, &ota_data, silent)) {
        if (!silent) {
            logger.printfln("Could not read %s partition OTA data", partition->label);
        }

        return -1;
    }

    if (ota_data.ota_state != old_ota_state) {
        return 0;
    }

    ota_data.ota_state = new_ota_state;

    if (!write_ota_data(ota_index, &ota_data, silent)) {
        if (!silent) {
            logger.printfln("Could not change %s partition OTA state from %s to %s",
                            partition->label,
                            get_esp_ota_img_state_name(static_cast<esp_ota_img_states_t>(old_ota_state)),
                            get_esp_ota_img_state_name(static_cast<esp_ota_img_states_t>(new_ota_state)));
        }

        return -1;
    }

    state.get(app_state_key)->updateUint(new_ota_state);

    if (!silent) {
        logger.printfln("Changed %s partition OTA state from %s to %s",
                        partition->label,
                        get_esp_ota_img_state_name(static_cast<esp_ota_img_states_t>(old_ota_state)),
                        get_esp_ota_img_state_name(static_cast<esp_ota_img_states_t>(new_ota_state)));
    }

    return 1;
}

int FirmwareUpdate::change_running_partition_from_pending_verify_to_valid(bool silent)
{
    const esp_partition_t *running_partition = esp_ota_get_running_partition();

    if (running_partition == nullptr) {
        if (!silent) {
            logger.printfln("Could not get running partition");
        }

        return -1;
    }

    return change_partition_ota_state_from_to(running_partition, ESP_OTA_IMG_PENDING_VERIFY, ESP_OTA_IMG_VALID, silent);
}

int FirmwareUpdate::change_running_partition_from_pending_verify_to_new(bool silent)
{
    const esp_partition_t *running_partition = esp_ota_get_running_partition();

    if (running_partition == nullptr) {
        if (!silent) {
            logger.printfln("Could not get running partition");
        }

        return -1;
    }

    return change_partition_ota_state_from_to(running_partition, ESP_OTA_IMG_PENDING_VERIFY, ESP_OTA_IMG_NEW, silent);
}

int FirmwareUpdate::change_update_partition_from_aborted_to_invalid(bool silent)
{
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(nullptr);

    if (update_partition == nullptr) {
        if (!silent) {
            logger.printfln("Could not get update partition");
        }

        return -1;
    }

    return change_partition_ota_state_from_to(update_partition, ESP_OTA_IMG_ABORTED, ESP_OTA_IMG_INVALID, silent);
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

    if (index_url.setCapacity(update_url.length() + options_product_id_length + index_url_suffix_len) == 0) {
        logger.printfln("Could not build firmware index URL");
        state.get("check_state")->updateEnum(CheckState::InternalError);
        return;
    }

    index_url.puts(update_url.c_str(), update_url.length());
    index_url.puts(OPTIONS_PRODUCT_ID(), options_product_id_length);
    index_url.puts(index_url_suffix, index_url_suffix_len);

    char *index_url_ptr = index_url.take();

    index_buf_used = 0;
    //last_version_timestamp = time(nullptr);
    check_for_update_in_progress = true;

    https_client.download_async(index_url_ptr, cert_id, [this](AsyncHTTPSClientEvent *event) {
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

            case AsyncHTTPSClientError::Timeout:
                logger.printfln("Update server %s did not respond", update_url.c_str());
                state.get("check_state")->updateEnum(CheckState::NoResponse);
                break;

            case AsyncHTTPSClientError::ShortRead:
                logger.printfln("Firmware index download ended prematurely");
                state.get("check_state")->updateEnum(CheckState::DownloadShortRead);
                break;

            case AsyncHTTPSClientError::HTTPError: {
                char buf[204];
                translate_HTTPError_detailed(event->error_handle, buf, ARRAY_SIZE(buf), true);
                logger.printfln("Firmware index download failed: %s", buf);
                state.get("check_state")->updateEnum(CheckState::DownloadError);
                break;
            }
            case AsyncHTTPSClientError::HTTPClientInitFailed:
                logger.printfln("Error while creating HTTP client");
                state.get("check_state")->updateEnum(CheckState::HTTPClientInitFailed);
                break;

            case AsyncHTTPSClientError::HTTPClientError:
                logger.printfln("Error while downloading firmware index: %s (0x%lX)", esp_err_to_name(event->error_http_client), static_cast<uint32_t>(event->error_http_client));
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

    free(index_url_ptr);
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
                if (version.compare(build_version) <= 0) {
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
    logger.printfln("Installing firmware from URL is not supported (running firmware is unsigned)");

    state.get("install_state")->updateEnum(InstallState::NotSupported);
    state.get("install_progress")->updateUint(0);
#else
    logger.printfln("Installing firmware from URL: %s", url);

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

            case AsyncHTTPSClientError::Timeout:
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
            if (event->data_complete_len < 0) {
                logger.printfln("Firmware file size is unknown");
                state.get("install_state")->updateEnum(InstallState::FirmwareSizeUnknown);
                https_client.abort_async();
                return;
            }

            if (event->data_complete_len <= FIRMWARE_OFFSET) {
                logger.printfln("Firmware file is too small: %u", event->data_complete_len);
                state.get("install_state")->updateEnum(InstallState::FirmwareTooSmall);
                https_client.abort_async();
                return;
            }

            result = handle_firmware_chunk(event->data_chunk_offset, (uint8_t *)event->data_chunk, event->data_chunk_len, (size_t)event->data_complete_len, event->data_is_complete, nullptr);

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
            trigger_reboot("firmware update", 1_s);
            install_firmware_in_progress = false;
            break;
        }
    });
#endif
}

void FirmwareUpdate::read_app_partition_state()
{
    bool app0_running = false;
    esp_ota_img_states_t app0_state = ESP_OTA_IMG_INVALID;
    const char *app0_state_name = "<unknown>";
    build_custom_app_desc_t app0_custom_desc;
    char app0_version[SEMANTIC_VERSION_MAX_STRING_LENGTH] = "<unknown>";

    bool app1_running = false;
    esp_ota_img_states_t app1_state = ESP_OTA_IMG_INVALID;
    const char *app1_state_name = "<unknown>";
    build_custom_app_desc_t app1_custom_desc;
    char app1_version[SEMANTIC_VERSION_MAX_STRING_LENGTH] = "<unknown>";

    const esp_partition_t *running_partition = esp_ota_get_running_partition();

    if (running_partition == nullptr) {
        logger.printfln("Could not get running partition");
    }
    else {
        state.get("running_partition")->updateString(running_partition->label);

        if (strcmp(running_partition->label, "app0") == 0) {
            app0_running = true;
        }
        else if (strcmp(running_partition->label, "app1") == 0) {
            app1_running = true;
        }
        else {
            logger.printfln("Unexpected running partition: %s", running_partition->label);
        }
    }

    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP,
                                                     ESP_PARTITION_SUBTYPE_ANY,
                                                     nullptr);

    while (it != nullptr) {
        const esp_partition_t *partition = esp_partition_get(it);

        esp_ota_img_states_t ota_state;
        esp_err_t err = esp_ota_get_state_partition(partition, &ota_state);

        if (err != ESP_OK) {
            logger.printfln("Could not get %s partition state: %d", partition->label, err);
        }
        else if (strcmp(partition->label, "app0") == 0) {
            app0_state_name = get_partition_ota_state_name(partition, &app0_state);
            state.get("app0_state")->updateUint(app0_state);
            read_custom_app_desc(partition, &app0_custom_desc, app0_version, ARRAY_SIZE(app0_version));
            state.get("app0_version")->updateString(app0_version);
        }
        else if (strcmp(partition->label, "app1") == 0) {
            app1_state_name = get_partition_ota_state_name(partition, &app1_state);
            state.get("app1_state")->updateUint(app1_state);
            read_custom_app_desc(partition, &app1_custom_desc, app1_version, ARRAY_SIZE(app1_version));
            state.get("app1_version")->updateString(app1_version);
        }
        else {
            logger.printfln("Unexpected app partition: %s", partition->label);
        }

        it = esp_partition_next(it);
    }

    const char *rolled_back_version = nullptr;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(nullptr);

    if (update_partition == nullptr) {
        logger.printfln("Could not get update partition");
    }
    else if (strcmp(update_partition->label, "app0") == 0) {
        if (app0_state == ESP_OTA_IMG_ABORTED) {
            rolled_back_version = app0_version;
        }
    }
    else if (strcmp(update_partition->label, "app1") == 0) {
        if (app1_state == ESP_OTA_IMG_ABORTED) {
            rolled_back_version = app1_version;
        }
    }
    else {
        logger.printfln("Unexpected update partition: %s", update_partition->label);
    }

    logger.printfln("Partitions: app0 (%s%s, %s), app1 (%s%s, %s)",
                    app0_state_name,
                    app0_running ? ", running" : "",
                    app0_version,
                    app1_state_name,
                    app1_running ? ", running" : "",
                    app1_version);

    if (rolled_back_version != nullptr) {
        logger.printfln("Firmware %s rolled back to %s", rolled_back_version, build_version_full_str());
        state.get("rolled_back_version")->updateString(rolled_back_version);
    }

    // if a power cycle happens while the running app partition is in pending-verify state then it gets
    // marked as aborted, even if the firmware is actually stable. this can happen to both app partitions.
    // in this case the bootloader will boot the aborted app partition anyway, as it doesn't have a better
    // choice. but the system is now stuck on one of the app partitions. if the firmware in the running app
    // partition is unstable and crashes the bootloader will not try the other app partition, even if that
    // one might actually contain a stable firmware. changing from aborted/aborted app partition state to
    // pending-verify/new results in the bootloader chosing the other partition if the running app partition
    // crashes. this way the system can find the stable firmware, if there is one.
    if (app0_state == ESP_OTA_IMG_ABORTED && app1_state == ESP_OTA_IMG_ABORTED) {
        logger.printfln("Trying to recover from an aborted/aborted app partition state");

        if (running_partition != nullptr) {
            change_partition_ota_state_from_to(running_partition, ESP_OTA_IMG_ABORTED, ESP_OTA_IMG_PENDING_VERIFY, false);
        }

        if (update_partition != nullptr) {
            change_partition_ota_state_from_to(update_partition, ESP_OTA_IMG_ABORTED, ESP_OTA_IMG_NEW, false);
        }
    }
}
