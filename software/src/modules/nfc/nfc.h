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

#include "device_module.h"
#include "config.h"
#include "build.h"
#include "bindings/bricklet_nfc.h"
#include "module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

// in bytes
#define NFC_TAG_ID_LENGTH 10
// For hex strings: two chars per byte plus a separator between each byte
#define NFC_TAG_ID_STRING_LENGTH (NFC_TAG_ID_LENGTH * 3 - 1)

#define TAG_LIST_LENGTH 9

class NFC : public DeviceModule<TF_NFC,
                                tf_nfc_create,
                                tf_nfc_get_bootloader_mode,
                                tf_nfc_reset,
                                tf_nfc_destroy,
                                BUILD_IS_WARP2() | BUILD_IS_WARP3()
                                >
#if MODULE_AUTOMATION_AVAILABLE()
          , public IAutomationBackend
#endif
{
public:
    NFC();

    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;

    struct tag_info_t {
        uint32_t last_seen;
        uint8_t tag_type;
        char tag_id[NFC_TAG_ID_STRING_LENGTH + 1]; // allow null terminator here
    };

    struct auth_tag_t {
        uint8_t tag_type;
        uint8_t user_id;
        char tag_id[NFC_TAG_ID_STRING_LENGTH + 1]; // allow null terminator here
    };

    static_assert(sizeof(auth_tag_t::tag_id) == sizeof(tag_info_t::tag_id));

    void update_seen_tags();
    void tag_seen(tag_info_t *tag, bool injected);
    void setup_nfc();
    void check_nfc_state();
    uint8_t get_user_id(tag_info_t *tag, uint8_t *tag_idx);

    void remove_user(uint8_t user_id);

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

private:
    ConfigRoot config;
    ConfigRoot auth_info;

    micros_t deadtime_post_start = 0_us;
    size_t auth_tag_count = 0;
    std::unique_ptr<auth_tag_t[]> auth_tags = nullptr;
    void setup_auth_tags();

public:
    ConfigRoot seen_tags;
    ConfigRoot state;
    ConfigRoot inject_tag;
    uint32_t last_tag_injection = 0;
    int tag_injection_action = 0;

    tag_info_t *old_tags = nullptr;
    tag_info_t *new_tags = nullptr;
};
