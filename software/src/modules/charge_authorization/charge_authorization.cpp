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
    auth_prototypes[0] = {CMAuthType::None, *Config::Null()};
    auth_prototypes[1] = {CMAuthType::Lost, *Config::Null()};
    auth_prototypes[2] = {CMAuthType::NFC, Config::Object({
        {"tag_id", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)},
        {"tag_type", Config::Uint8(0)}
    })};
    auth_prototypes[3] = {CMAuthType::InjectedNFC, Config::Object({
        {"tag_id", Config::Str("", 0, NFC_TAG_ID_STRING_LENGTH)},
        {"tag_type", Config::Uint8(0)}
    })};


    last_seen_authentications = Config::Tuple(
        LAST_AUTH_LIST_LENGTH,
        Config::Object({
            {"seen_at", Config::Uptime()},
            {"auth_info", Config::Union<CMAuthType>(
                *Config::Null(),
                CMAuthType::None,
                auth_prototypes,
                ARRAY_SIZE(auth_prototypes)
            )}
        })
    );
}

void ChargeAuthorization::setup()
{
    initialized = true;
}

void ChargeAuthorization::notify_auth(int16_t user_id, millis_t last_seen, CMAuthType auth_type, int action, Config::ConfVariant auth_info)
{
    auto seen_at = now_us() - last_seen;

    size_t insert_at = 0;

    for (size_t i = 0; i < LAST_AUTH_LIST_LENGTH; ++i) {
        Config *auth = static_cast<Config *>(last_seen_authentications.get(i));
        // Empty slots have a seen_at timestamp of 0.
        // This is always less than now_us() - last_seen except if a tag was seen before the ESP started.
        // It's fine to ignore it in this case.
        if (auth->get("seen_at")->asUptime() >= seen_at)
            continue;

        insert_at = i;
        break;
    }

    for (int i = LAST_AUTH_LIST_LENGTH - 1; i > insert_at; --i) {
        Config *src = static_cast<Config *>(last_seen_authentications.get(i - 1));
        Config *dst = static_cast<Config *>(last_seen_authentications.get(i));
        *dst = *src;
    }

    Config *dst = static_cast<Config *>(last_seen_authentications.get(insert_at));
    dst->get("seen_at")->updateUptime(seen_at);
    dst->get("auth_info")->changeUnionVariant(auth_type);
    static_cast<Config *>(dst->get("auth_info")->get())->value = auth_info;

    bool blink_handled = false;
#if MODULE_OCPP_AVAILABLE()
    if (auth_type == CMAuthType::NFC || auth_type == CMAuthType::InjectedNFC) {
        // TODO: this is a hack but good enough for now.
        char buf_ocpp[NFC_TAG_ID_STRING_WITHOUT_SEPARATOR_LENGTH + 1];
        nfc.get_last_tag_seen(nullptr, nullptr, buf_ocpp);
        blink_handled = ocpp.on_tag_seen(buf_ocpp);
    }
#endif

#if MODULE_EVSE_LED_AVAILABLE()
    if (!blink_handled)
        evse_led.set_module(user_id >= 0 ? EvseLed::Blink::Ack : EvseLed::Blink::Nack, 2000);
#else
    (void) blink_handled;
#endif

#if MODULE_EVSE_COMMON_AVAILABLE()
    if (user_id >= 0)
        users.trigger_charge_action(user_id, auth_type, auth_info, action, 3_s, /*TODO: De-hack */nfc.get_deadtime_post_start());

    // TODO: Maybe let notify_new_auth return blink_handled so that central user auth wins against local?
    evse_common.notify_new_auth();
#endif
}

void ChargeAuthorization::register_urls()
{
    api.addState("charge_authorization/last_seen", &last_seen_authentications, {}, {"tag_id"});
}

int16_t ChargeAuthorization::find_user(const cm_auth_info &info)
{
    switch (info.auth_method) {
        case CMAuthType::NFC:
        case CMAuthType::InjectedNFC: {
            NFC::tag_t tag;
            tag.type = info.tag_type;
            tag.id_length = info.tag_id_len;

            static_assert(sizeof(tag.id_bytes) == sizeof(info.tag_id), "Tag ID size mismatch");
            memcpy(tag.id_bytes, info.tag_id, sizeof(info.tag_id));

            return nfc.get_user_id(tag);
        }
        case CMAuthType::None:
        case CMAuthType::Lost:
            return -1;
    }
    return -1;
}
