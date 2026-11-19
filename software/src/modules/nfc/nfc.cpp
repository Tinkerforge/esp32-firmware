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

#include "nfc.h"

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "bindings/errors.h"
#include "tools/hexdump.h"

#include "gcc_warnings.h"

#if defined(BOARD_HAS_PSRAM)
#define MAX_AUTHORIZED_TAGS 32
#else
#define MAX_AUTHORIZED_TAGS 16
#endif

#define DETECTION_THRESHOLD_MS 2000

static constexpr size_t TAG_LIST_LENGTH = 9;

static bool operator==(const NFC::tag_t& a, const NFC::tag_t& b) {
    return a.type == b.type && a.id_length == b.id_length && memcmp(a.id_bytes, b.id_bytes, a.id_length) == 0;
}

static const char *lookup = "0123456789ABCDEF";

static void id_to_string(char str[NFC_TAG_ID_STRING_LENGTH + 1], const NFC::tag_t *tag) {
    if (tag->id_length == 0) {
        str[0] = '\0';
        return;
    }

    for (size_t i = 0; i < static_cast<size_t>(tag->id_length); i++) {
        const uint8_t b = tag->id_bytes[i];
        *str++ = lookup[b >> 4];
        *str++ = lookup[b & 0xF];
        *str++ = ':';
    }

    *--str = '\0'; // Overwrite trailing colon
}

static void id_to_string_without_separator(char str[NFC_TAG_ID_STRING_WITHOUT_SEPARATOR_LENGTH + 1], const NFC::tag_t *tag) {
    for (size_t i = 0; i < static_cast<size_t>(tag->id_length); i++) {
        const uint8_t b = tag->id_bytes[i];
        *str++ = lookup[b >> 4];
        *str++ = lookup[b & 0xF];
    }

    *str = '\0';
}

static uint8_t hex_digit_to_byte(char digit)
{
    if (digit >= '0' && digit <= '9')
        return digit - '0';

    digit |= 0x60;
    if (digit >= 'a' && digit <= 'f')
        return digit - 'a' + 10;

    return 0xFF;
}

static uint8_t string_to_id(uint8_t buf[NFC_TAG_ID_LENGTH], const char *str, size_t str_len = std::numeric_limits<size_t>::max()) {
    if (str_len == std::numeric_limits<size_t>::max())
        str_len = strnlen(str, NFC_TAG_ID_STRING_LENGTH);

    const uint8_t bytes = static_cast<uint8_t>((str_len + 1) / 3);

    for (uint8_t b = 0; b < bytes; ++b) {
        buf[b] = hex_digit_to_byte(str[b * 3]) << 4 | hex_digit_to_byte(str[b * 3 + 1]);
    }
    return bytes;
}

void NFC::pre_setup()
{
    seen_tags_state = Config::Tuple(TAG_LIST_LENGTH, Config::Object({
        {"tag_type",  Config::Uint8(0)},
        {"tag_id",    Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)},
        {"last_seen", Config::Uint32(0)}
    }));

    config_authorized_tags_prototype = Config::Object({
        {"user_id", Config::Uint8(0)},
        {"tag_type", Config::Uint8(0, 6)},
        {"tag_id", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)}
    });

    config = ConfigRoot{Config::Object({
        {"authorized_tags", Config::Array(
            {},
            &config_authorized_tags_prototype,
            0, MAX_AUTHORIZED_TAGS,
            Config::type_id<Config::ConfObject>())
        },
        {"deadtime_post_start", Config::Uint32(30)}
    }), [this](Config &cfg, ConfigSource source) -> String {
        Config *tags = static_cast<Config *>(cfg.get("authorized_tags"));
        auto tags_count = tags->count();

        // Check tag_id format
        for (size_t tag = 0; tag < tags_count; ++tag) {
            String id_copy = tags->get(tag)->get("tag_id")->asString();
            id_copy.toUpperCase();
            tags->get(tag)->get("tag_id")->updateString(id_copy);

            if (id_copy.length() != 0 && id_copy.length() % 3 != 2)
                return "Tag ID has unexpected length. Expected format is uppercase hex bytes separated by colons. For example \"01:23:AB:3D\".";

            for (size_t i = 0; i < id_copy.length(); ++i) {
                char c = id_copy.charAt(i);
                if ((i % 3 != 2) && ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')))
                    continue;
                if (i % 3 == 2 && c == ':')
                    continue;
                return "Tag ID contains unexpected character. Expected format is uppercase hex bytes separated by colons. For example \"01:23:AB:3D\".";
            }
        }

        // Add more validation above this block!
        if (source == ConfigSource::File) {
            // The validation below was missing in old firmwares.
            // To make sure a config that is stored in the ESP's flash
            // can be loaded on start-up, fix the mappings instead of
            // returning an error.
            bool update_file = false;
            for (size_t tag = 0; tag < tags_count; ++tag) {
                uint8_t user_id = tags->get(tag)->get("user_id")->asUint8();
                if (!users.is_user_configured(user_id)) {
                    logger.printfln("Fixing NFC tag %s referencing a deleted user.", tags->get(tag)->get("tag_id")->asEphemeralCStr());
                    tags->get(tag)->get("user_id")->updateUint(0);
                    update_file = true;
                }
            }
            if (update_file)
                API::writeConfig("nfc/config", &cfg);

        } else {
            // Check user_id_mappings
            for (size_t tag = 0; tag < tags_count; ++tag) {
                uint8_t user_id = tags->get(tag)->get("user_id")->asUint8();
                if (!users.is_user_configured(user_id))
                    return String("Unknown user with ID ") + static_cast<int>(user_id) + ".";
            }

            // Check for duplicated tag_id+type entries
            for (size_t tag = 0; tag < tags_count; ++tag) {
                const String &tag_id = tags->get(tag)->get("tag_id")->asString();
                auto tag_type = tags->get(tag)->get("tag_type")->asUint8();
                for (size_t tag2 = tag + 1; tag2 < tags_count; ++tag2) {
                    const String &tag2_id = tags->get(tag2)->get("tag_id")->asString();
                    auto tag2_type = tags->get(tag2)->get("tag_type")->asUint8();

                    if (tag_id == tag2_id && tag_type == tag2_type) {
                        return "Tag ID " + tags->get(tag)->get("tag_id")->asString() + " is used multiple times with same tag type.";
                    }
                }
            }
        }

        // NFC::setup() calls setup_auth_tags after restoring the config.
        if (source != ConfigSource::File)
            task_scheduler.scheduleOnce([this](){this->setup_auth_tags();});
        return "";
    }};

    inject_tag = ConfigRoot{Config::Object({
        {"tag_type", Config::Uint8(0, 6)},
        {"tag_id", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)}
    }), [this](Config &cfg, ConfigSource source) -> String {
        String id_copy = cfg.get("tag_id")->asString();
        id_copy.toUpperCase();
        cfg.get("tag_id")->updateString(id_copy);

        if (id_copy.length() != 0 && id_copy.length() % 3 != 2)
            return "Tag ID has unexpected length. Expected format is uppercase hex bytes separated by colons. For example \"01:23:AB:3D\".";

        for (size_t i = 0; i < id_copy.length(); ++i) {
            char c = id_copy.charAt(i);
            if ((i % 3 != 2) && ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')))
                continue;
            if (i % 3 == 2 && c == ':')
                continue;
            return "Tag ID contains unexpected character. Expected format is uppercase hex bytes separated by colons. For example \"01:23:AB:3D\".";
        }

        return "";
    }};

#if MODULE_AUTOMATION_AVAILABLE()
    automation.register_trigger(
        AutomationTriggerID::NFC,
        Config::Object({
            {"tag_type", Config::Uint8(0, 6)},
            {"tag_id", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)},
            // Index into the charge manager's charger list, or -1 to match tags seen by any charger.
            {"charger", Config::Int8(-1)}
        })
    );

    automation.register_action(
        AutomationActionID::NFCInjectTag,
        Config::Object({
            {"tag_type", Config::Uint8(0, 6)},
            {"tag_id", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)},
            {"action", Config::Uint8(0, 2)}
        }),
        [this](const Config *cfg) {
            inject_tag.get("tag_type")->updateUint(cfg->get("tag_type")->asUint8());
            inject_tag.get("tag_id")->updateString(cfg->get("tag_id")->asString());
            tag_injected(cfg->get("action")->asUint8());
        }
    );
#endif

    auth_info = Config::Object({
        {"tag_type", Config::Uint8(0)},
        {"tag_id", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)}
    });
}

int16_t NFC::get_user_id(const tag_t &tag)
{
    // NFC does not know about the anonymous user (that has ID 0)
    // return -1 instead if this tag was not found or it was found
    // but not assigned to any user.
    for (uint8_t i = 0; i < auth_tag_count; ++i) {
        const auto &auth_tag = auth_tags[i];

        if (auth_tag.tag == tag)
            return auth_tag.user_id == 0 ? -1 : auth_tag.user_id;
    }
    return -1;
}

void NFC::remove_user(uint8_t user_id)
{
    Config *tags = static_cast<Config *>(config.get("authorized_tags"));

    bool changed = false;
    for (size_t i = tags->count(); i-- > 0;) {
        if (tags->get(i)->get("user_id")->asUint8() == user_id) {
            tags->remove(i);
            changed = true;
        }
    }

    if (changed) {
        API::writeConfig("nfc/config", &config);
    }
}

void NFC::remote_tag_seen(uint8_t tag_type, const uint8_t *tag_id, uint8_t tag_id_length, int8_t charger_index)
{
#if MODULE_AUTOMATION_AVAILABLE()
    tag_t tag;
    tag.type = tag_type;
    tag.id_length = std::min(tag_id_length, static_cast<uint8_t>(NFC_TAG_ID_LENGTH));
    memcpy(tag.id_bytes, tag_id, tag.id_length);

    automation_trigger_data_t data;
    data.tag = &tag;
    data.charger_index = charger_index;

    automation.trigger(AutomationTriggerID::NFC, &data, this);
#else
    (void)tag_type;
    (void)tag_id;
    (void)tag_id_length;
    (void)charger_index;
#endif
}

void NFC::setup_auth_tags()
{
    const Config *auth_tags_cfg = static_cast<Config *>(config.get("authorized_tags"));

    // Allways set deadtime, even if there are not tags configured, this is used for autocharge too (ev mac instead of nfc tag).
    this->deadtime_post_start = seconds_t{config.get("deadtime_post_start")->asUint32()};

    const size_t new_count = auth_tags_cfg->count();
    const bool auth_tag_count_changed = auth_tag_count != new_count;

    auth_tag_count = new_count;
    if (auth_tag_count == 0) {
        auth_tags = nullptr;
        return;
    }

    if (auth_tag_count_changed) {
        auth_tags.reset(); // Avoid concurrent allocations
        auth_tags = heap_alloc_array<auth_tag_t>(auth_tag_count);
    }

    for (size_t i = 0; i < auth_tag_count; ++i) {
        const auto tag = auth_tags_cfg->get(i);
        auth_tag_t *auth_tag = auth_tags.get() + i;

        auth_tag->tag.type = tag->get("tag_type")->asUint8();
        auth_tag->user_id  = tag->get("user_id" )->asUint8();

        const String &id = tag->get("tag_id")->asString();
        auth_tag->tag.id_length = string_to_id(auth_tag->tag.id_bytes, id.c_str(), id.length());
    }
}

void NFC::setup()
{
    api.restorePersistentConfig("nfc/config", &config);
    setup_auth_tags();

    initialized = true;
}

void NFC::register_urls()
{
    api.addState("nfc/seen_tags", &seen_tags_state, {}, {"tag_id"});
    api.addPersistentConfig("nfc/config", &config, {}, {"tag_id"});

#if MODULE_EVSE_COMMON_AVAILABLE()
    api.addCommand("nfc/inject_tag", &inject_tag, {"tag_id"}, [this](Language /*language*/, String &/*errmsg*/) {
        tag_injected(TRIGGER_CHARGE_ANY);
    }, true);

    api.addCommand("nfc/inject_tag_start", &inject_tag, {"tag_id"}, [this](Language /*language*/, String &/*errmsg*/) {
        tag_injected(TRIGGER_CHARGE_START);
    }, true);

    api.addCommand("nfc/inject_tag_stop", &inject_tag, {"tag_id"}, [this](Language /*language*/, String &/*errmsg*/) {
        tag_injected(TRIGGER_CHARGE_STOP);
    }, true);
#endif
}

bool NFC::setup_seen_tags()
{
    assert(seen_tags == nullptr);

    const size_t alloc_size = TAG_LIST_LENGTH * sizeof(tag_info_t);
    seen_tags = static_cast<decltype(seen_tags)>(perm_aligned_alloc(alignof(tag_info_t), alloc_size, RAM::DRAM));

    if (seen_tags == nullptr) {
        logger.printfln("No memory for tag list");
        return false;
    }

    memset(seen_tags, 0, alloc_size);

    // Task for updating relative timestamps in state
    task_scheduler.scheduleUncancelable([this]() {
        bool have_now = false;
        millis_t now_ms;

        for (size_t i = 0; i < TAG_LIST_LENGTH; i++) {
            tag_info_t *tag_info = seen_tags + i;

            if (tag_info->last_seen == 0) {
                break;
            }

            if (!have_now) {
                now_ms = now_us().to<millis_t>();
                have_now = true;
            }

            Config *tag_state = static_cast<Config *>(seen_tags_state.get(i));
            const millis_t last_seen_ms = now_ms - tag_info->last_seen_at_ms;

            if (static_cast<uint32_t>(last_seen_ms.as<int64_t>() >> 32) == 0) {
                // last_seen_ms fits into uint32_t
                tag_info->last_seen = last_seen_ms.as<uint32_t>();
            } else {
                // uint32_t overflow after 49 days, evict tag
                memset(tag_info, 0, sizeof(*tag_info));
                tag_state->get("tag_type")->updateUint(0);
                tag_state->get("tag_id")->clearString();
            }

            tag_state->get("last_seen")->updateUint(tag_info->last_seen);
        }
    }, 1_s, 1_s);

    return true;
}

bool NFC::get_last_tag_seen(tag_info_t *info, char id_with_separator[NFC_TAG_ID_STRING_LENGTH + 1], char id_without_separator[NFC_TAG_ID_STRING_WITHOUT_SEPARATOR_LENGTH + 1]) {
    if (seen_tags == nullptr) {
        return false;
    }

    const tag_info_t *tag = seen_tags + 0; // First tag is most recently seen.

    if (tag->last_seen == 0) {
        return false;
    }

    if (info != nullptr) {
        *info = *tag;
    }

    if (id_with_separator != nullptr) {
        id_to_string(id_with_separator, &tag->tag);
    }

    if (id_without_separator != nullptr) {
        id_to_string_without_separator(id_without_separator, &tag->tag);
    }

    return true;
}

void NFC::tag_injected(int action)
{
    tag_t tag;

    tag.type = inject_tag.get("tag_type")->asUint8();

    const String &tag_id_str = inject_tag.get("tag_id")->asString();
    tag.id_length = string_to_id(tag.id_bytes, tag_id_str.c_str(), tag_id_str.length());

    tag_seen(&tag, action);
}

void NFC::local_tag_seen(const tag_t *tag)
{
    if (tag->type == 4 && tag->id_length == 4) {
        // NFC Forum Type 4 tags must have an ID length of 7.
        // Smartphones without the WARP app present random type 4 tags with an ID length of 4 and can thus be filtered out.
        logger.printfln("Ignoring Smartphone without WARP app");
        return;
    }

    tag_seen(tag, -1);
}

void NFC::tag_seen(const tag_t *new_tag, int injection_action)
{
    if (seen_tags == nullptr && !setup_seen_tags()) {
        return;
    }

    if (new_tag->id_length == 0 || new_tag->id_length > std::size(new_tag->id_bytes)) {
        return;
    }

    size_t old_tag_count     = TAG_LIST_LENGTH;
    size_t found_in_position = TAG_LIST_LENGTH;

    for (size_t i = 0; i < TAG_LIST_LENGTH; i++) {
        const tag_info_t *old_tag_info = seen_tags + i;

        if (old_tag_info->last_seen == 0) {
            old_tag_count = i;
            break;
        }

        const tag_t *tag = &old_tag_info->tag;

        if (memcmp(tag, new_tag, offsetof(tag_t, id_bytes) + new_tag->id_length) == 0) {
            found_in_position = i;
            break;
        }
    }

    const millis_t now_ms = now_us().to<millis_t>();
    const bool seen_new = (found_in_position >= TAG_LIST_LENGTH) || (now_ms - seen_tags[found_in_position].last_seen_at_ms > millis_t{DETECTION_THRESHOLD_MS});

    size_t tags_to_move = 0;

    if (found_in_position != 0) {
        // Move the tags before this or all old ones, drop the last one if full.
        tags_to_move = std::min(found_in_position, std::min(old_tag_count, TAG_LIST_LENGTH - 1));

        if (tags_to_move > 0) {
            memmove(seen_tags + 1, seen_tags + 0, tags_to_move * sizeof(seen_tags[0]));
        }

        seen_tags[0].tag = *new_tag;
    }

    tag_info_t *tag_info = seen_tags + 0; // Seen tag is now first in list
    tag_info->last_seen_at_ms = now_ms;
    tag_info->last_seen = 1; // Must be != 0

    if (found_in_position == 0) {
        seen_tags_state.get(0)->get("last_seen")->updateUint(1);
    } else {
        const size_t tags_to_sync = tags_to_move + 1;
        CoolString id_string;

        for (size_t i = 0; i < tags_to_sync; i++) {
            const tag_info_t *tag = seen_tags + i;

            if (tag->tag.id_length == 0) {
                id_string.setLength(0);
            } else {
                const size_t char_count = static_cast<size_t>(tag->tag.id_length) * 3 - 1;

                id_string.reserve(char_count);
                id_to_string(id_string.begin(), &tag->tag);
                id_string.setLength(static_cast<int>(char_count));
            }

            Config *tag_state = static_cast<Config *>(seen_tags_state.get(i));
            tag_state->get("tag_id"   )->updateString(id_string);
            tag_state->get("tag_type" )->updateUint(tag->tag.type);
            tag_state->get("last_seen")->updateUint(tag->last_seen);
        }
    }

    if (!seen_new) {
        return;
    }

    logger.printfln("Seen NFC tag type %hhu", tag_info->tag.type);

#if MODULE_AUTOMATION_AVAILABLE()
    {
        automation_trigger_data_t data;
        data.tag = &tag_info->tag;
#if MODULE_CHARGE_MANAGER_AVAILABLE()
        data.charger_index = charge_manager.get_charger_state_index(charge_manager.find_charger_state(esp32_common.get_uid_num()));
#else
        data.charger_index = -1;
#endif
        automation.trigger(AutomationTriggerID::NFC, &data, this);
    }
#endif

#if MODULE_CHARGE_AUTHORIZATION_AVAILABLE()
    {
        const int16_t user_id = get_user_id(tag_info->tag);

        auth_info.get("tag_type")->updateUint(tag_info->tag.type);

        char buf[NFC_TAG_ID_STRING_LENGTH + 1];
        id_to_string(buf, &tag_info->tag);
        auth_info.get("tag_id")->updateString(buf);

        charge_authorization.notify_auth(
            user_id,
            millis_t{tag_info->last_seen},
            injection_action >= 0 ? CMAuthType::InjectedNFC : CMAuthType::NFC,
#if MODULE_EVSE_COMMON_AVAILABLE()
            injection_action >= 0 ? injection_action : TRIGGER_CHARGE_ANY,
#else
            0,
#endif
            auth_info.value);
    }
#else
    (void)injection_action;
#endif
}

#if MODULE_AUTOMATION_AVAILABLE()
bool NFC::has_triggered(const Config *conf, void *data)
{
    const Config *cfg = static_cast<const Config *>(conf->get());
    const automation_trigger_data_t *trigger_data = static_cast<automation_trigger_data_t *>(data);

    if (conf->getTag<AutomationTriggerID>() != AutomationTriggerID::NFC) {
        return false;
    }

    const int8_t rule_charger = cfg->get("charger")->asInt8();
    if ((rule_charger != -1) && (rule_charger != trigger_data->charger_index)) {
        return false;
    }

    if (cfg->get("tag_type")->asUint8() != trigger_data->tag->type) {
        return false;
    }

    const String &rule_tag_id_str = cfg->get("tag_id")->asString();

    if (rule_tag_id_str.length() != static_cast<size_t>(trigger_data->tag->id_length) * 3 - 1) {
        return false;
    }

    char tag_id_str[NFC_TAG_ID_STRING_LENGTH + 1];
    id_to_string(tag_id_str, trigger_data->tag);

    return rule_tag_id_str == tag_id_str;
}
#endif
