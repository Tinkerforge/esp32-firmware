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

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"
#include "task_scheduler.h"
#include "modules.h"

#if MODULE_EVSE_AVAILABLE()
#include "bindings/bricklet_evse.h"
#endif
#if MODULE_EVSE_V2_AVAILABLE()
#include "bindings/bricklet_evse_v2.h"
#endif

#if MODULE_ESP32_ETHERNET_BRICK_AVAILABLE()
#define AUTHORIZED_TAG_LIST_LENGTH 16
#else
#define AUTHORIZED_TAG_LIST_LENGTH 8
#endif

#define IND_ACK 1001
#define IND_NACK 1002
#define IND_NAG 1003

#define TOKEN_LIFETIME_MS 30000
#define DETECTION_THRESHOLD_MS 1000

extern EventLog logger;

extern TaskScheduler task_scheduler;

extern API api;

void NFC::pre_setup()
{
    seen_tags = Config::Array(
        {},
        new Config{Config::Object({
            {"tag_type", Config::Uint8(0)},
            {"tag_id", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)},
            {"last_seen", Config::Uint32(0)}
        })},
        0, TAG_LIST_LENGTH,
        Config::type_id<Config::ConfObject>()
    );

    config = ConfigRoot(Config::Object({
        {"authorized_tags", Config::Array(
            {},
            new Config{Config::Object({
                {"user_id", Config::Uint8(0)},
                {"tag_type", Config::Uint(0, 0, 4)},
                {"tag_id", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)}
            })},
            0, AUTHORIZED_TAG_LIST_LENGTH,
            Config::type_id<Config::ConfObject>())
        }
    }), [this](Config &cfg) -> String {
        Config *tags = (Config *)cfg.get("authorized_tags");
        for(int tag = 0; tag < tags->count(); ++tag) {
            String id_copy = tags->get(tag)->get("tag_id")->asString();
            id_copy.toUpperCase();
            tags->get(tag)->get("tag_id")->updateString(id_copy);

            for(int i = 0; i < id_copy.length(); ++i) {
                char c = id_copy.charAt(i);
                if ((i % 3 != 2) && ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')))
                    continue;
                if (i % 3 == 2 && c == ':')
                    continue;
                return "Tag ID contains unexpected character. Expected format is hex bytes separated by colons. For example \"01:23:ab:3d\".";
            }
        }

        return "";
    });

    inject_tag = ConfigRoot(Config::Object({
        {"tag_type", Config::Uint(0, 0, 4)},
        {"tag_id", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)}
    }), [this](Config &cfg) -> String {
        String id_copy = cfg.get("tag_id")->asString();
        id_copy.toUpperCase();
        cfg.get("tag_id")->updateString(id_copy);

        for(int i = 0; i < id_copy.length(); ++i) {
            char c = id_copy.charAt(i);
            if ((i % 3 != 2) && ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F')))
                continue;
            if (i % 3 == 2 && c == ':')
                continue;
            return "Tag ID contains unexpected character. Expected format is hex bytes separated by colons. For example \"01:23:ab:3d\".";
        }

        return "";
    });
}

void NFC::setup_nfc()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    int result = tf_nfc_set_mode(&device, TF_NFC_MODE_SIMPLE);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("NFC set mode failed (rc %d). Disabling NFC support.", result);
        }
        return;
    }

    // Clear tag list
    result = tf_nfc_simple_get_tag_id(&device, 255, nullptr, nullptr, nullptr, nullptr);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("Clearing NFC tag list failed (rc %d). Disabling NFC support.", result);
        }
        return;
    }

    initialized = true;
    api.addFeature("nfc");
}

void NFC::check_nfc_state()
{
    uint8_t mode = 0;
    int result = tf_nfc_get_mode(&device, &mode);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("Failed to get NFC mode, rc: %d", result);
        }
        return;
    }
    if (mode != TF_NFC_MODE_SIMPLE) {
        logger.printfln("NFC mode invalid. Did the bricklet reset?");
        setup_nfc();
    }
}

uint8_t NFC::get_user_id(tag_info_t *tag, uint8_t *tag_idx)
{
    if (tag->last_seen >= TOKEN_LIFETIME_MS)
        return false;

    Config *auth_tags = (Config *)config_in_use.get("authorized_tags");

    for (uint8_t auth_tag_idx = 0; auth_tag_idx < auth_tags->count(); ++auth_tag_idx) {
        Config *auth_tag = (Config *)auth_tags->get(auth_tag_idx);

        if (auth_tag->get("tag_type")->asUint() == tag->tag_type && auth_tag->get("tag_id")->asString() == tag->tag_id) {
            *tag_idx = auth_tag_idx;
            return auth_tag->get("user_id")->asUint();
        }
    }
    return 0;
}

void set_led(int16_t mode)
{
    static int16_t last_mode = -1;
    static uint32_t last_set = 0;

    if (last_mode == mode && !deadline_elapsed(last_set + 2500))
        return;

    // sorted by priority
    switch (mode) {
        case IND_ACK:
            break;
        case IND_NACK:
            if (last_mode == IND_ACK && !deadline_elapsed(last_set + 2340))
                return;
            break;
        case IND_NAG:
        case -1:
            if ((last_mode == IND_ACK && !deadline_elapsed(last_set + 2340))
                || (last_mode == IND_NACK && !deadline_elapsed(last_set + 3536)))
                return;
            break;
        default:
            break;
    }

#if MODULE_EVSE_AVAILABLE()
    tf_evse_set_indicator_led(&evse.device, mode, mode != IND_NACK ? 2620 : 3930, nullptr);
#endif
#if MODULE_EVSE_V2_AVAILABLE()
    tf_evse_v2_set_indicator_led(&evse_v2.device, mode, mode != IND_NACK ? 2620 : 3930, nullptr);
#endif

    last_mode = mode;
    last_set = millis();
}

void NFC::handle_event(tag_info_t *tag, bool found, bool injected)
{
    uint8_t idx = 0;
    uint8_t user_id = get_user_id(tag, &idx);

    if (user_id != 0) {
        if (found) {
            // Found a new authorized tag. Create/overwrite auth token. Overwrite blink state even if we previously saw a not authorized tag.
            auth_token = idx;
            auth_token_seen = millis();
            blink_state = IND_ACK;
            users.trigger_charge_action(user_id, injected ? CHARGE_TRACKER_AUTH_TYPE_NFC_INJECTION : CHARGE_TRACKER_AUTH_TYPE_NFC, Config::Object({
                    {"tag_type", Config::Uint8(tag->tag_type)},
                    {"tag_id", Config::Str(tag->tag_id)}}).value,
                    injected ? tag_injection_action : TRIGGER_CHARGE_ANY);
        } else if (auth_token == idx) {
            // Lost an authorized tag. If we still have it's auth token, extend the token's validity.
            //auth_token_seen = millis();
        }
    } else if (found) {
        // Found a not authorized tag. Blink NACK but only if we did not see an authorized token
        if (blink_state == -1) {
            blink_state = IND_NACK;
        }
    }
}

void NFC::handle_evse()
{
    static Config *evse_state = api.getState("evse/state", false);
    static Config *evse_slots = api.getState("evse/slots", false);

    if (evse_state == nullptr || evse_slots == nullptr)
        return;

    bool waiting_for_start = (evse_state->get("iec61851_state")->asUint() == 1)
                          && (evse_slots->get(CHARGING_SLOT_USER)->get("active")->asBool())
                          && (evse_slots->get(CHARGING_SLOT_USER)->get("max_current")->asUint() == 0);

    if (blink_state != -1) {
        set_led(blink_state);
        blink_state = -1;
    } else
        set_led(waiting_for_start ? IND_NAG : -1);
}

const char *lookup = "0123456789ABCDEF";

void tag_id_bytes_to_string(const uint8_t *tag_id, uint8_t tag_id_len, char buf[NFC_TAG_ID_STRING_LENGTH + 1])
{
    for (int i = 0; i < tag_id_len; ++i) {
        uint8_t b = tag_id[i];
        uint8_t hi = (b & 0xF0) >> 4;
        uint8_t lo = b & 0x0F;
        buf[3 * i] = lookup[hi];
        buf[3 * i + 1] = lookup[lo];
        buf[3 * i + 2] = ':';
    }
    if (tag_id_len == 0)
        buf[0] = '\0';
    else
        buf[3 * tag_id_len - 1] = '\0';
}

void NFC::update_seen_tags()
{
    for (int i = 0; i < TAG_LIST_LENGTH - 1; ++i) {
        uint8_t buf[10] = {0};
        uint8_t tag_id_len = 0;
        int result = tf_nfc_simple_get_tag_id(&device, i, &new_tags[i].tag_type, buf, &tag_id_len, &new_tags[i].last_seen);
        if (result != TF_E_OK) {
            if (!is_in_bootloader(result)) {
                logger.printfln("Failed to get tag id %d, rc: %d", i, result);
            }
            continue;
        }

        tag_id_bytes_to_string(buf, tag_id_len, new_tags[i].tag_id);

        seen_tags.get(i)->get("tag_type")->updateUint(new_tags[i].tag_type);
        seen_tags.get(i)->get("tag_id")->updateString(new_tags[i].tag_id);
        seen_tags.get(i)->get("last_seen")->updateUint(new_tags[i].last_seen);
    }

    if (last_tag_injection == 0 || deadline_elapsed(last_tag_injection + 1000 * 60 * 60 * 24)) {
        last_tag_injection = 0;
        new_tags[TAG_LIST_LENGTH - 1].tag_type = 0;
        new_tags[TAG_LIST_LENGTH - 1].tag_id[0] = '\0';
        new_tags[TAG_LIST_LENGTH - 1].last_seen = 0;
    } else {
        new_tags[TAG_LIST_LENGTH - 1].tag_type = inject_tag.get("tag_type")->asUint();
        strncpy(new_tags[TAG_LIST_LENGTH - 1].tag_id, inject_tag.get("tag_id")->asCStr(), sizeof(new_tags[TAG_LIST_LENGTH - 1].tag_id));
        new_tags[TAG_LIST_LENGTH - 1].last_seen = millis() - last_tag_injection;
    }

    seen_tags.get(TAG_LIST_LENGTH - 1)->get("last_seen")->updateUint(new_tags[TAG_LIST_LENGTH - 1].last_seen);
    seen_tags.get(TAG_LIST_LENGTH - 1)->get("tag_type")->updateUint(new_tags[TAG_LIST_LENGTH - 1].tag_type);
    seen_tags.get(TAG_LIST_LENGTH - 1)->get("tag_id")->updateString(new_tags[TAG_LIST_LENGTH - 1].tag_id);

    // compare new list with old
    // tags that are not seen anymore are lost
    // tags that are seen again or are not in the old list are found
    for (int new_idx = 0; new_idx < TAG_LIST_LENGTH; ++new_idx) {
        if (new_tags[new_idx].last_seen == 0)
            continue;

        bool found = false;
        int old_idx;
        for (old_idx = 0; old_idx < TAG_LIST_LENGTH; ++old_idx) {
            if (old_tags[old_idx].last_seen == 0)
                continue;

            if(strncmp(old_tags[old_idx].tag_id,
                       new_tags[new_idx].tag_id,
                       NFC_TAG_ID_STRING_LENGTH) != 0)
                continue;

            found = true;
            break;
        }

        bool new_seen = new_tags[new_idx].last_seen < DETECTION_THRESHOLD_MS;

        if (!found && new_seen) {
            // found new tag
            handle_event(&new_tags[new_idx], true, new_idx == TAG_LIST_LENGTH - 1);
            continue;
        }

        bool old_seen = old_tags[old_idx].last_seen < DETECTION_THRESHOLD_MS;
        old_tags[old_idx].last_seen = 0;

        if (old_seen && !new_seen) {
            // lost old tag
            handle_event(&old_tags[old_idx], false, old_idx == TAG_LIST_LENGTH - 1);
            continue;
        }
        if (!old_seen && new_seen) {
            // found new tag
            handle_event(&new_tags[new_idx], true, new_idx == TAG_LIST_LENGTH - 1);
            continue;
        }
    }

    // tags that are also in the new list are marked with last_seen = 0
    // all other tags are displaced i.e. gone
    for (int old_idx = 0; old_idx < TAG_LIST_LENGTH; ++old_idx) {
        if (old_tags[old_idx].last_seen == 0)
            continue;

        handle_event(&old_tags[old_idx], false, old_idx == TAG_LIST_LENGTH - 1);
    }

    tag_info_t *tmp = old_tags;
    old_tags = new_tags;
    new_tags = tmp;
}

void NFC::setup()
{
    setup_nfc();
    if (!device_found)
        return;

    api.restorePersistentConfig("nfc/config", &config);
    config_in_use = config;

    for (int i = 0; i < TAG_LIST_LENGTH; ++i) {
        seen_tags.add();
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        this->check_nfc_state();
    }, 5 * 60 * 1000, 5 * 60 * 1000);

    task_scheduler.scheduleWithFixedDelay([this](){
        static uint32_t last_run = 0;
        if (deadline_elapsed(last_run + 300)) {
            last_run = millis();
            this->update_seen_tags();
        }
        this->handle_evse();
    }, 10, 10);
}

void NFC::register_urls()
{
    if (!device_found)
        return;

    api.addState("nfc/seen_tags", &seen_tags, {}, 1000);
    api.addPersistentConfig("nfc/config", &config, {}, 1000);
    api.addCommand("nfc/inject_tag", &inject_tag, {}, [this](){
        last_tag_injection = millis();
        tag_injection_action = TRIGGER_CHARGE_ANY;
        // 0 is the marker that no injection happened or the last one was handled.
        // Fake that we were one ms faster.
        if (last_tag_injection == 0)
            last_tag_injection -= 1;
    }, true);

    api.addCommand("nfc/inject_tag_start", &inject_tag, {}, [this](){
        last_tag_injection = millis();
        tag_injection_action = TRIGGER_CHARGE_START;
        // 0 is the marker that no injection happened or the last one was handled.
        // Fake that we were one ms faster.
        if (last_tag_injection == 0)
            last_tag_injection -= 1;
    }, true);

    api.addCommand("nfc/inject_tag_stop", &inject_tag, {}, [this](){
        last_tag_injection = millis();
        tag_injection_action = TRIGGER_CHARGE_STOP;
        // 0 is the marker that no injection happened or the last one was handled.
        // Fake that we were one ms faster.
        if (last_tag_injection == 0)
            last_tag_injection -= 1;
    }, true);

    this->DeviceModule::register_urls();
}

void NFC::loop()
{
    this->DeviceModule::loop();
}
