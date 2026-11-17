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

#include "module.h"
#include "config.h"
#include "bindings/bricklet_nfc.h"

#include "generated/module_available.h"

#if MODULE_AUTOMATION_AVAILABLE()
#include "modules/automation/automation_backend.h"
#endif

#include "options.h"

// in bytes
#define NFC_TAG_ID_LENGTH 10
// For hex strings: two chars per byte plus a separator between each byte
#define NFC_TAG_ID_STRING_LENGTH (NFC_TAG_ID_LENGTH * 3 - 1)
#define NFC_TAG_ID_STRING_WITHOUT_SEPARATOR_LENGTH (NFC_TAG_ID_LENGTH * 2)

class INfcBackend
{
public:
    INfcBackend() {}
    virtual ~INfcBackend() {}

    // Get tag at given index. Returns true on success, false on error.
    virtual bool get_tag_id(uint8_t index, uint8_t *tag_type, uint8_t *tag_id, uint8_t *tag_id_length, uint32_t *last_seen) = 0;

    virtual void reset() {}
};

class NFC final : public IModule
#if MODULE_AUTOMATION_AVAILABLE()
          , public IAutomationBackend
#endif
{
public:
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    struct tag_t {
        uint8_t type;
        uint8_t id_length;
        uint8_t id_bytes[NFC_TAG_ID_LENGTH];
    };

    struct tag_info_t {
        millis_t last_seen_at_ms; // Absolute uptime
        uint32_t last_seen;       // Relative before now
        tag_t tag;
    };

    int16_t get_user_id(const tag_t &tag);

    void remove_user(uint8_t user_id);

    bool get_last_tag_seen(tag_info_t *info, char id_with_separator[NFC_TAG_ID_STRING_LENGTH + 1], char id_without_separator[NFC_TAG_ID_STRING_WITHOUT_SEPARATOR_LENGTH + 1]);

    struct automation_trigger_data_t {
        const tag_t *tag;
        int8_t charger_index;
    };

    void local_tag_seen(const tag_t *tag);
    void remote_tag_seen(uint8_t tag_type, const uint8_t *tag_id, uint8_t tag_id_length, int8_t charger_index);

#if MODULE_AUTOMATION_AVAILABLE()
    bool has_triggered(const Config *conf, void *data) override;
#endif

    micros_t get_deadtime_post_start() const { return this->deadtime_post_start; }

private:
    ConfigRoot seen_tags_state;

    Config config_authorized_tags_prototype;
    ConfigRoot config;

    ConfigRoot inject_tag;
    Config auth_info;

    tag_info_t *seen_tags = nullptr;

    struct auth_tag_t {
        uint8_t user_id;
        tag_t tag;
    };

    std::unique_ptr<auth_tag_t[]> auth_tags = nullptr;
    size_t auth_tag_count = 0;

    micros_t deadtime_post_start = 0_us;

    void setup_auth_tags();
    bool setup_seen_tags();

    void tag_injected(int action);
    void tag_seen(const tag_t *tag, int injection_action);
};

#include "options_end.h"
#include "generated/module_available_end.h"
