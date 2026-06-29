/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "ove_r37.h"

#include <string.h>
#include <mbedtls/sha256.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "options.h"

#include "generated/ove_r37_state.enum.h"

#include "gcc_warnings.h"

static bool ove_r37_check_password(const String &password)
{
    unsigned char hash[32];

    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0 /*use SHA256*/);
    mbedtls_sha256_update(&ctx, reinterpret_cast<const unsigned char *>(password.c_str()), password.length());
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);

    char hex[65];
    for (int i = 0; i < 32; ++i)
        snprintf(hex + i * 2, 3, "%02x", hash[i]);
    hex[64] = '\0';

    return strcmp(hex, OPTIONS_OVE_R37_ELECTRICIAN_PASSWORD_SHA256()) == 0;
}

void OveR37::pre_setup()
{
    config = Config::Object({
        {"enabled", Config::Bool(false)},
        {"undervoltage_threshold", Config::Uint16(800)},        // 1/1000 pu (0.80 pu) (5.9.8)
        {"undervoltage_observation_time", Config::Uint16(3000)},// in ms (5.9.8)
        {"reconnect_wait_time", Config::Uint16(60)},            // in s, 0..300 (5.7.4.2)
        {"start_delay", Config::Uint16(0)},                     // in s, 0..300 (5.9.2 B)
    });

    config_update = Config::Object({
        {"enabled", Config::Bool(false)},
        {"undervoltage_threshold", Config::Uint(800, 0, 1000)},
        {"undervoltage_observation_time", Config::Uint16(3000)},
        {"reconnect_wait_time", Config::Uint(60, 0, 300)},
        {"start_delay", Config::Uint(0, 0, 300)},
        {"password", Config::Str("", 0, 64)}, // electrician password (write-only, not echoed back)
    });

    state = Config::Object({
        {"state", Config::Enum(OveR37State::Disabled)},
        {"trip_reason", Config::Uint8(0)},
        {"flags", Config::Uint8(0)},
    });
}

void OveR37::setup()
{
    initialized = true;
}

void OveR37::register_urls()
{
    api.addState("ove_r37/config", &config);
    api.addState("ove_r37/state", &state);

    api.addCommand("ove_r37/config_update", &config_update, {"password"}, [this](Language /*language*/, String &errmsg) {
        // The OVE R37 parameters affect grid-support behaviour and must only be
        // changeable by an electrician (OVE R 37 5.9.5 manipulation security).
        if (!ove_r37_check_password(config_update.get("password")->asString())) {
            config_update.get("password")->updateString("");
            errmsg = "Wrong electrician password";
            return;
        }
        config_update.get("password")->updateString("");

        int rc = evse_v2.set_ove_r37_configuration(
            config_update.get("enabled")->asBool(),
            static_cast<uint16_t>(config_update.get("undervoltage_threshold")->asUint()),
            static_cast<uint16_t>(config_update.get("undervoltage_observation_time")->asUint()),
            static_cast<uint16_t>(config_update.get("reconnect_wait_time")->asUint()),
            static_cast<uint16_t>(config_update.get("start_delay")->asUint())
        );

        if (rc != TF_E_OK) {
            errmsg = "Failed to write OVE R37 configuration to EVSE";
            return;
        }

        update_config_from_bricklet();
    }, false);
}

bool OveR37::update_config_from_bricklet()
{
    bool enabled;
    uint16_t undervoltage_threshold, undervoltage_observation_time, reconnect_wait_time, start_delay;

    if (evse_v2.get_ove_r37_configuration(&enabled, &undervoltage_threshold, &undervoltage_observation_time, &reconnect_wait_time, &start_delay) != TF_E_OK) {
        return false;
    }

    config.get("enabled")->updateBool(enabled);
    config.get("undervoltage_threshold")->updateUint(undervoltage_threshold);
    config.get("undervoltage_observation_time")->updateUint(undervoltage_observation_time);
    config.get("reconnect_wait_time")->updateUint(reconnect_wait_time);
    config.get("start_delay")->updateUint(start_delay);

    return true;
}

void OveR37::update_state_from_all_data(uint8_t ove_r37_state, uint8_t trip_reason, uint8_t flags)
{
    if (!config_read) {
        config_read = update_config_from_bricklet();
    }

    state.get("state")->updateEnum(static_cast<OveR37State>(ove_r37_state));
    state.get("trip_reason")->updateUint(trip_reason);
    state.get("flags")->updateUint(flags);
}
