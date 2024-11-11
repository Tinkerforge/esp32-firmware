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
#include <sodium.h>

#include "module.h"
#include "config.h"
#include "semantic_version.h"
#include "async_https_client.h"
#include "signature_verify.embedded.h"
#include "install_state.enum.h"

struct TFJsonSerializer;

template <typename T>
class BlockReader
{
public:
    BlockReader(size_t block_offset, size_t block_len, uint32_t expected_magic_0, uint32_t expected_magic_1) :
        block_offset(block_offset), block_len(block_len), expected_magic_0(expected_magic_0), expected_magic_1(expected_magic_1) {}

    void reset();
    bool handle_chunk(size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len);

    size_t block_offset;
    size_t block_len;

    uint32_t expected_magic_0;
    uint32_t expected_magic_1;

    union {
        T block;
        struct {
            uint32_t actual_magic_0;
            uint32_t actual_magic_1;
        };
    };

    size_t read_block_len = 0;
    bool block_found = false;

    uint32_t actual_checksum = 0;
    uint32_t expected_checksum = 0;
    size_t read_expected_checksum_len = 0;
};

class FirmwareUpdate final : public IModule
{
public:
    FirmwareUpdate();
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    bool vehicle_connected = false;

    void handle_index_data(const void *data, size_t data_len);

private:
    bool is_vehicle_blocking_update() const;
    InstallState handle_firmware_chunk(size_t chunk_offset, uint8_t *chunk, size_t chunk_len, size_t complete_len, bool is_complete, TFJsonSerializer *json_ptr);
    InstallState check_firmware_info(bool detect_downgrade, bool log, TFJsonSerializer *json_ptr);
    void check_for_update();
    void install_firmware(const char *url);

    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot install_firmware_config;
    ConfigRoot override_signature;

    bool check_firmware_in_progress = false;
    bool flash_firmware_in_progress = false;

    struct firmware_info_t {
        uint32_t magic[2] = {0};
        char firmware_name[61] = {0};
        uint8_t fw_version[3] = {0};
        uint32_t fw_build_time = 0;
        uint8_t fw_version_beta = 0;
    };

    BlockReader<firmware_info_t> firmware_info;

#if signature_sodium_public_key_length != 0
    struct signature_info_t {
        uint32_t magic[2] = {0};
        char publisher[64] = {0};
        unsigned char signature[crypto_sign_BYTES] = {0};
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
