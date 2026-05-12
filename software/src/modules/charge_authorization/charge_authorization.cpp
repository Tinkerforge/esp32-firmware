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

#include "charge_authorization.h"
#include "generated/module_dependencies.h"
#include "modules/cm_networking/generated/cm_auth_type.enum.h"

void ChargeAuthorization::pre_setup()
{
    last_seen_authentications = Config::Tuple({});
}

void ChargeAuthorization::setup()
{
    auth_prototypes[0] = {CMAuthType::None, *Config::Null()};
    auth_prototypes[1] = {CMAuthType::NFC, Config::Object({
        {"auth_string", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)},
        {"additional_data", Config::Uint8(0)},
        {"last_seen", Config::Uint32(0)}
    })};

    last_seen_authentications.replace(LAST_AUTH_LIST_LENGTH, Config::Union<CMAuthType>(
        *Config::Null(),
        CMAuthType::None,
        auth_prototypes,
        ARRAY_SIZE(auth_prototypes)
    ));

    initialized = true;
}

void ChargeAuthorization::register_urls()
{
    api.addState("charge_authorization/last_seen", &last_seen_authentications, {}, {"auth_string"});
    
    task_scheduler.scheduleUncancelable([this] {
        // Collect valid tags from indices 0 to TAG_LIST_LENGTH-2 (already sorted newest-first by NFC module)
        size_t valid_indices[TAG_LIST_LENGTH];
        size_t valid_count = 0;

        for (size_t i = 0; i < TAG_LIST_LENGTH - 1; ++i) {
            const Config *tag = static_cast<const Config *>(nfc.seen_tags.get(i));
            if (tag->get("last_seen")->asUint() == 0)
                continue;
            if (tag->get("tag_id")->asString().length() == 0)
                continue;
            valid_indices[valid_count++] = i;
        }

        // The last index may contain an injected tag; insert it into the correct sorted position
        constexpr size_t last_idx = TAG_LIST_LENGTH - 1;
        const Config *last_tag = static_cast<const Config *>(nfc.seen_tags.get(last_idx));
        if (last_tag->get("last_seen")->asUint() != 0 && last_tag->get("tag_id")->asString().length() != 0) {
            uint32_t last_seen = last_tag->get("last_seen")->asUint();
            size_t insert_pos = valid_count;
            for (size_t i = 0; i < valid_count; ++i) {
                if (static_cast<const Config *>(nfc.seen_tags.get(valid_indices[i]))->get("last_seen")->asUint() > last_seen) {
                    insert_pos = i;
                    break;
                }
            }
            for (size_t i = valid_count; i > insert_pos; --i) {
                valid_indices[i] = valid_indices[i - 1];
            }
            valid_indices[insert_pos] = last_idx;
            ++valid_count;
        }

        // Fill up to LAST_AUTH_LIST_LENGTH entries; clear the rest
        for (size_t i = 0; i < LAST_AUTH_LIST_LENGTH; ++i) {
            Config *auth = static_cast<Config *>(last_seen_authentications.get(i));
            if (i < valid_count) {
                const Config *tag = static_cast<const Config *>(nfc.seen_tags.get(valid_indices[i]));
                auth->changeUnionVariant(CMAuthType::NFC);
                auth->get()->get("auth_string")->updateString(tag->get("tag_id")->asString());
                auth->get()->get("additional_data")->updateUint(tag->get("tag_type")->asUint());
                auth->get()->get("last_seen")->updateUint(tag->get("last_seen")->asUint());
            } else {
                auth->changeUnionVariant(CMAuthType::None);
            }
        }
    }, 1_s);
}

int16_t ChargeAuthorization::find_user(const cm_auth_info &info)
{
    if (info.auth_method != CMAuthType::NFC)
        return -1;

    NFC::tag_t tag;
    tag.type = info.tag_type;
    tag.id_length = info.tag_id_len;

    static_assert(sizeof(tag.id_bytes) == sizeof(info.tag_id), "Tag ID size mismatch");
    memcpy(tag.id_bytes, info.tag_id, sizeof(info.tag_id));

    return nfc.get_user_id(tag);
}
