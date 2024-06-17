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

#include "config.h"

#include <stdint.h>
#include <esp_http_client.h>
#include <sodium.h>

#include "module.h"
#include "web_server.h"
#include "signature_public_key.embedded.h"

struct SemanticVersion {
    uint8_t major = 255;
    uint8_t minor = 255;
    uint8_t patch = 255;
    uint8_t beta = 255;
    uint32_t timestamp = 0;
};

class FirmwareUpdate final : public IModule
{
public:
    FirmwareUpdate(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    bool firmware_update_allowed = true;
    bool firmware_update_running = false;

    void handle_update_data(const void *data, size_t data_len);

private:
    bool handle_firmware_chunk(int command, std::function<void(const char *, const char *)> result_cb, size_t chunk_offset, uint8_t *chunk_data, size_t chunk_length, size_t remaining, size_t complete_len);
#if signature_public_key_length != 0
    void handle_signature_chunk(size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len);
#endif
    void reset_firmware_info();
    bool handle_firmware_info_chunk(size_t chunk_offset, uint8_t *chunk_data, size_t chunk_len);
    String check_firmware_info(bool firmware_info_found, bool detect_downgrade, bool log);
    void check_for_update();
    bool parse_version(const char *p, SemanticVersion *version) const;
    String format_version(SemanticVersion *version) const;

    struct firmware_info_t {
        uint32_t magic[2] = {0};
        char firmware_name[61] = {0};
        uint8_t fw_version[3] = {0};
        uint32_t fw_build_time = {0};
        uint8_t fw_version_beta = {0};
    };

    ConfigRoot config;
    ConfigRoot state;
    ConfigRoot install_firmware;

    firmware_info_t info;
    uint32_t info_offset = 0;
    uint32_t calculated_checksum = 0;
    uint32_t checksum = 0;
    uint32_t checksum_offset = 0;
    bool update_aborted = false;
    bool info_found = false;

#if signature_public_key_length != 0
    crypto_sign_state signature_state;
    unsigned char signature_data[crypto_sign_BYTES];
#endif

    String update_url;
    int cert_id = -1;
    std::unique_ptr<unsigned char[]> cert = nullptr;
    esp_http_client_handle_t http_client = nullptr;
    uint32_t last_update_begin;
    char update_buf[64 + 1];
    size_t update_buf_used;
    SemanticVersion beta_update;
    SemanticVersion release_update;
    SemanticVersion stable_update;
    uint8_t update_mask;
    bool update_complete;
    uint32_t last_non_beta_timestamp;
};
