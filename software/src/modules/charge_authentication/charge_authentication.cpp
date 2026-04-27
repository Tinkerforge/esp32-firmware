/* esp32-firmware
 * Copyright (C) 2026 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "charge_authentication.h"
#include "generated/module_dependencies.h"
#include "modules/cm_networking/generated/cm_auth_type.enum.h"

void ChargeAuthentication::pre_setup()
{
    last_seen_authentications = Config::Tuple({});
}

void ChargeAuthentication::setup()
{
    last_seen_authentications.replace(LAST_AUTH_LIST_LENGTH, Config::Object({
        {"type",      Config::Enum(CMAuthType::None)},
        /**
         * In case of NFC: Token-String
         */
        {"auth_string", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)},
        /**
         * Currently only used in NFC as Tag-Type
         */
        {"additional_data", Config::Uint8(0)},
        {"last_seen", Config::Uint32(0)}
    }));

    initialized = true;
}

void ChargeAuthentication::register_urls()
{
    api.addState("charge_authentication/last_seen_authentications", &last_seen_authentications, {}, {"auth_string"});
    
    task_scheduler.scheduleUncancelable([this] {
        // Collect indices of valid (seen) NFC tags
        size_t valid_indices[TAG_LIST_LENGTH];
        size_t valid_count = 0;

        for (size_t i = 0; i < TAG_LIST_LENGTH; ++i) {
            const Config *tag = static_cast<const Config *>(nfc.seen_tags.get(i));
            if (tag->get("last_seen")->asUint() == 0)
                continue;
            if (tag->get("tag_id")->asString().length() == 0)
                continue;
            valid_indices[valid_count++] = i;
        }

        // Insertion sort by last_seen ascending (smallest value = most recently seen)
        for (size_t i = 1; i < valid_count; ++i) {
            size_t key_idx = valid_indices[i];
            uint32_t key_last_seen = static_cast<const Config *>(nfc.seen_tags.get(key_idx))->get("last_seen")->asUint();
            int j = (int)i - 1;
            while (j >= 0 && static_cast<const Config *>(nfc.seen_tags.get(valid_indices[j]))->get("last_seen")->asUint() > key_last_seen) {
                valid_indices[j + 1] = valid_indices[j];
                --j;
            }
            valid_indices[j + 1] = key_idx;
        }

        // Fill up to LAST_AUTH_LIST_LENGTH entries; clear the rest
        for (size_t i = 0; i < LAST_AUTH_LIST_LENGTH; ++i) {
            Config *auth = static_cast<Config *>(last_seen_authentications.get(i));
            if (i < valid_count) {
                const Config *tag = static_cast<const Config *>(nfc.seen_tags.get(valid_indices[i]));
                auth->get("type")->updateEnum(CMAuthType::NFC);
                auth->get("auth_string")->updateString(tag->get("tag_id")->asString());
                auth->get("additional_data")->updateUint(tag->get("tag_type")->asUint());
                auth->get("last_seen")->updateUint(tag->get("last_seen")->asUint());
            } else {
                auth->get("type")->updateEnum(CMAuthType::None);
                auth->get("auth_string")->updateString("");
                auth->get("additional_data")->updateUint(0);
                auth->get("last_seen")->updateUint(0);
            }
        }
    }, 1_s);
}

int16_t ChargeAuthentication::find_user(const cm_auth_info &info)
{
    if (info.auth_method != USERS_AUTH_METHOD_NFC)
        return -1;

    NFC::tag_t tag;
    tag.type = info.tag_type;
    tag.id_length = info.tag_id_len;

    static_assert(sizeof(tag.id_bytes) == sizeof(info.tag_id), "Tag ID size mismatch");
    memcpy(tag.id_bytes, info.tag_id, sizeof(info.tag_id));

    return nfc.get_user_id(tag);
}