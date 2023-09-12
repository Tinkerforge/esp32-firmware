/* esp32-firmware
 * Copyright (C) 2022 Erik Fleckstein <erik@tinkerforge.com>
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

#include "ocpp.h"

#include "build.h"
#include "api.h"
#include "task_scheduler.h"
#include "module_dependencies.h"

#include <ctype.h>
#include <string.h>

extern char local_uid_str[32];

void Ocpp::pre_setup()
{
    config = Config::Object({
        {"enable", Config::Bool(false)},
        {"url", Config::Str("", 0, 128)},
        {"identity", Config::Str("", 0, 64)},
        {"enable_auth",Config::Bool(false)},
        {"pass", Config::Str("", 0, 64)}
    });

    change_configuration = Config::Object({
        {"key", Config::Str("", 0, 64)},
        {"value", Config::Str("", 0, 500)}
    });

#ifdef OCPP_STATE_CALLBACKS
    state = Config::Object({
        {"charge_point_state", Config::Uint8(0)},
        {"charge_point_status", Config::Uint8(0)},
        {"next_profile_eval", Config::Int32(0)},

        {"connector_state", Config::Uint8(0)},
        {"connector_status", Config::Uint8(0)},
        {"tag_id", Config::Str("", 0, 21)},
        {"parent_tag_id", Config::Str("", 0, 21)},
        {"tag_expiry_date", Config::Int32(0)},
        {"tag_timeout", Config::Uint32(0)},
        {"cable_timeout", Config::Uint32(0)},
        {"txn_id", Config::Int32(0)},
        {"txn_confirmed_time", Config::Int32(0)},
        {"txn_start_time", Config::Int32(0)},
        {"current", Config::Uint32(0)},
        {"txn_with_invalid_id", Config::Bool(false)},
        {"unavailable_requested", Config::Bool(false)},

        {"message_in_flight_type", Config::Uint8(0)},
        {"message_in_flight_id_high", Config::Uint32(0)},
        {"message_in_flight_id_low", Config::Uint32(0)},
        {"message_in_flight_len", Config::Uint32(0)},
        {"message_timeout", Config::Uint32(0)},
        {"txn_msg_retry_timeout", Config::Uint32(0)},
        {"message_queue_depth", Config::Uint8(0)},
        {"status_queue_depth", Config::Uint8(0)},
        {"txn_msg_queue_depth", Config::Uint8(0)},
        {"connected", Config::Bool(false)},
        {"connected_change_time", Config::Uint32(0)},
        {"last_ping_sent", Config::Uint32(0)},
        {"pong_deadline", Config::Uint32(0)},
    });

    configuration = Config::Object({
        {"AuthorizeRemoteTxRequests", Config::Str("", 0, 500)},
        {"ClockAlignedDataInterval", Config::Str("", 0, 500)},
        {"ConnectionTimeOut", Config::Str("", 0, 500)},
        {"ConnectorPhaseRotation", Config::Str("", 0, 500)},
        {"ConnectorPhaseRotationMaxLength", Config::Str("", 0, 500)},
        {"GetConfigurationMaxKeys", Config::Str("", 0, 500)},
        {"HeartbeatInterval", Config::Str("", 0, 500)},
        {"LocalAuthorizeOffline", Config::Str("", 0, 500)},
        {"LocalPreAuthorize", Config::Str("", 0, 500)},
        {"MessageTimeout", Config::Str("", 0, 500)},
        {"MeterValuesAlignedData", Config::Str("", 0, 500)},
        {"MeterValuesAlignedDataMaxLength", Config::Str("", 0, 500)},
        {"MeterValuesSampledData", Config::Str("", 0, 500)},
        {"MeterValuesSampledDataMaxLength", Config::Str("", 0, 500)},
        {"MeterValueSampleInterval", Config::Str("", 0, 500)},
        {"NumberOfConnectors", Config::Str("", 0, 500)},
        {"ResetRetries", Config::Str("", 0, 500)},
        {"StopTransactionOnEVSideDisconnect", Config::Str("", 0, 500)},
        {"StopTransactionOnInvalidId", Config::Str("", 0, 500)},
        {"StopTransactionMaxMeterValues", Config::Str("", 0, 500)},
        {"StopTxnAlignedData", Config::Str("", 0, 500)},
        {"StopTxnAlignedDataMaxLength", Config::Str("", 0, 500)},
        {"StopTxnSampledData", Config::Str("", 0, 500)},
        {"StopTxnSampledDataMaxLength", Config::Str("", 0, 500)},
        {"SupportedFeatureProfiles", Config::Str("", 0, 500)},
        {"TransactionMessageAttempts", Config::Str("", 0, 500)},
        {"TransactionMessageRetryInterval", Config::Str("", 0, 500)},
        {"UnlockConnectorOnEVSideDisconnect", Config::Str("", 0, 500)},
        {"WebSocketPingInterval", Config::Str("", 0, 500)},
        {"ChargeProfileMaxStackLevel", Config::Str("", 0, 500)},
        {"ChargingScheduleAllowedChargingRateUnit", Config::Str("", 0, 500)},
        {"ChargingScheduleMaxPeriods", Config::Str("", 0, 500)},
        {"ConnectorSwitch3to1PhaseSupported", Config::Str("", 0, 500)},
        {"MaxChargingProfilesInstalled", Config::Str("", 0, 500)}
    });
#endif
}

static const char *lookup = "0123456789ABCDEFabcdef";

static uint8_t hex_digit_to_byte(char digit) {
    for(size_t i = 0; i < strlen(lookup); ++i) {
        if (lookup[i] == digit)
            return i > 15 ? (i - 6) : i;
    }
    return 0xFF;
}

void Ocpp::setup()
{
    initialized = true;
    if (!api.restorePersistentConfig("ocpp/config", &config)) {
        config.get("identity")->updateString(String(BUILD_HOST_PREFIX) + "-" + local_uid_str);
    }

    config_in_use = config;

    if (!config.get("enable")->asBool() || config.get("url")->asString().length() == 0)
        return;

    cp = std::unique_ptr<OcppChargePoint>(new OcppChargePoint());

    task_scheduler.scheduleOnce([this](){
        // Make sure every code path calls cp->start!

        task_scheduler.scheduleWithFixedDelay([this](){
            cp->tick();
        }, 100, 100);

        if (!config_in_use.get("enable_auth")->asBool()) {
            cp->start(config_in_use.get("url")->asEphemeralCStr(), config_in_use.get("identity")->asEphemeralCStr(), nullptr, 0);
            return;
        }

        String pass = config_in_use.get("pass")->asString();
        bool pass_is_hex = pass.length() == 40;
        if (pass_is_hex) {
            for(size_t i = 0; i < 40; ++i) {
                if (!isxdigit(pass[i])) {
                    pass_is_hex = false;
                    break;
                }
            }
        }

        if (!pass_is_hex) {
            cp->start(config.get("url")->asEphemeralCStr(), config_in_use.get("identity")->asEphemeralCStr(), (const uint8_t *)pass.c_str(), pass.length());
            return;
        }

        uint8_t pass_bytes[20] = {};
        for(size_t i = 0; i < 20; ++i) {
            pass_bytes[i] = hex_digit_to_byte(pass[2*i]) << 4 | hex_digit_to_byte(pass[2*i + 1]);
        }
        cp->start(config.get("url")->asEphemeralCStr(), config_in_use.get("identity")->asEphemeralCStr(), pass_bytes, 20);
    }, 5000);
}

void Ocpp::register_urls()
{
    api.addPersistentConfig("ocpp/config", &config, {"pass"}, 1000);
#ifdef OCPP_STATE_CALLBACKS
    api.addState("ocpp/state", &state, {}, 1000);
    api.addState("ocpp/configuration", &configuration, {}, 1000);
#endif
    api.addCommand("ocpp/reset", Config::Null(), {}, [](){
        remove_directory("/ocpp");
    }, true);

#ifdef OCPP_DEBUG
    api.addFeature("ocpp_debug");
    api.addCommand("ocpp/change_configuration", &change_configuration, {}, [this](){
        auto status = cp->changeConfig(change_configuration.get("key")->asEphemeralCStr(), change_configuration.get("value")->asEphemeralCStr());
        logger.printfln("Change config %s status %s", change_configuration.get("key")->asEphemeralCStr(), ChangeConfigurationResponseStatusStrings[(size_t) status]);
    }, true);
#endif
}

#if MODULE_NFC_AVAILABLE()
void Ocpp::on_tag_seen(const char *tag_id) {
    if (tag_seen_cb == nullptr)
        return;

    // We have to remove the separating ':'s from the tag_id.
    // OCPP expectes IDs that map to physical tag IDs to contain only the hex-bytes.
    char buf[NFC_TAG_ID_STRING_LENGTH + 1] = {};
    remove_separator(tag_id, buf);

    tag_seen_cb(1, buf, tag_seen_cb_user_data);
}
#endif
