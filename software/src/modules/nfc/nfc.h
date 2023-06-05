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

#include "bindings/bricklet_nfc.h"

#include "build.h"
#include "config.h"
#include "device_module.h"
#include "nfc_bricklet_firmware_bin.embedded.h"

// in bytes
#define NFC_TAG_ID_LENGTH 10
// For hex strings: two chars per byte plus a separator between each byte
#define NFC_TAG_ID_STRING_LENGTH (NFC_TAG_ID_LENGTH * 3 - 1)

#define TAG_LIST_LENGTH 9

class NFC : public DeviceModule<TF_NFC,
                                nfc_bricklet_firmware_bin_data,
                                nfc_bricklet_firmware_bin_length,
                                tf_nfc_create,
                                tf_nfc_get_bootloader_mode,
                                tf_nfc_reset,
                                tf_nfc_destroy,
#ifdef BUILD_NAME_WARP2
                                true
#else
                                false
#endif

                                >
{
public:
    NFC() : DeviceModule("nfc", "NFC", "NFC", std::bind(&NFC::setup_nfc, this)) {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;

    struct tag_info_t {
        uint32_t last_seen;
        uint8_t tag_type;
        char tag_id[NFC_TAG_ID_STRING_LENGTH + 1]; // allow null terminator here
    };

    void update_seen_tags();
    void handle_event(tag_info_t *tag, bool lost_or_found, bool injected);
    void handle_evse();
    void setup_nfc();
    void check_nfc_state();
    uint8_t get_user_id(tag_info_t *tag, uint8_t *tag_idx);

    ConfigRoot config;
    ConfigRoot config_in_use;
    ConfigRoot seen_tags;
    ConfigRoot state;
    ConfigRoot inject_tag;
    uint32_t last_tag_injection = 0;
    int tag_injection_action = 0;

    uint32_t last_action_ms = 0;

    char uid[7] = {0};

    tag_info_t old_tag_buffer[TAG_LIST_LENGTH] = {};
    tag_info_t new_tag_buffer[TAG_LIST_LENGTH] = {};
    tag_info_t *old_tags = old_tag_buffer;
    tag_info_t *new_tags = new_tag_buffer;

    int auth_token = -1;
    uint32_t auth_token_seen = 0;
    int16_t blink_state = -1;
};
