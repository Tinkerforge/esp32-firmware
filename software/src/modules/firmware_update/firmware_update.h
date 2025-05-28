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

#pragma once

#include <stdint.h>
#include <FS.h> // FIXME: without this include here there is a problem with the IPADDR_NONE define in <lwip/ip4_addr.h>
#include <esp_http_client.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>
#include <sodium.h>

#include "module.h"
#include "config.h"
#include "tools/semantic_version.h"
#include "async_https_client.h"
#include "signature_verify.embedded.h"
#include "install_state.enum.h"

struct TFJsonSerializer;

#define BLOCK_READER_MAGIC_LENGTH 7

template <typename T>
class BlockReader
{
public:
    BlockReader(size_t block_offset, size_t block_len, const uint8_t expected_magic[BLOCK_READER_MAGIC_LENGTH]);

    void reset();
    bool handle_chunk(size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len);

    size_t block_offset;
    size_t block_len;
    uint8_t expected_magic[BLOCK_READER_MAGIC_LENGTH];

    T block;
    size_t read_block_len;
    bool block_found;

    uint32_t actual_checksum;
    uint32_t expected_checksum;
    size_t read_expected_checksum_len;
};

class FirmwareUpdate final : public IModule
{
public:
    FirmwareUpdate();
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void pre_reboot() override;

    bool vehicle_connected = false;

    void handle_index_data(const void *data, size_t data_len);

    int change_running_partition_from_pending_verify_to_valid(bool silent = false);
    int change_running_partition_from_pending_verify_to_new(bool silent = false);
    int change_update_partition_from_aborted_to_invalid(bool silent = false);

private:
    int change_partition_ota_state_from_to(const esp_partition_t *partition, esp_ota_img_states_t old_ota_state, esp_ota_img_states_t new_ota_state, bool silent);
    bool is_vehicle_blocking_update() const;
    InstallState handle_firmware_chunk(size_t chunk_offset, uint8_t *chunk, size_t chunk_len, size_t complete_len, bool is_complete, TFJsonSerializer *json_ptr);
    InstallState check_firmware_info(bool detect_downgrade, bool log, TFJsonSerializer *json_ptr);
    void check_for_update();
    void install_firmware(const char *url);
    void read_app_partition_state();

    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot install_firmware_config;
    ConfigRoot override_signature;

    bool check_firmware_in_progress = false;
    bool flash_firmware_in_progress = false;

    struct firmware_info_t {
        uint8_t magic[BLOCK_READER_MAGIC_LENGTH] = {};
        uint8_t version = 0;
        char display_name[61] = {};
        uint8_t fw_version_major = 0;
        uint8_t fw_version_minor = 0;
        uint8_t fw_version_patch = 0;
        uint32_t fw_build_timestamp = 0;
        uint8_t fw_version_beta = 0; // since firmware info version 2, before it's 0xFF
        char name[61] = {}; // since firmware info version 3, before it's 0xFF
    };

    BlockReader<firmware_info_t> firmware_info;

#if signature_sodium_public_key_length != 0
    struct signature_info_t {
        uint8_t magic[BLOCK_READER_MAGIC_LENGTH] = {};
        uint8_t version = 0;
        char publisher[64] = {};
        unsigned char signature[crypto_sign_BYTES] = {};
    };

    BlockReader<signature_info_t> signature_info;
    crypto_sign_state signature_state;
    uint32_t signature_override_cookie = 0;
#endif

    String update_url;
    AsyncHTTPSClient https_client;
    int cert_id = -1;
    char index_buf[64 + 1];
    size_t index_buf_used;
    //uint32_t last_version_timestamp;
    bool check_for_update_in_progress = false;
    bool install_firmware_in_progress = false;
};
