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

#include <ctype.h>
#include <string.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "options.h"
#include "tools/fs.h"

#include "ocpp/Configuration.h"

extern char local_uid_str[32];

void Ocpp::pre_setup()
{
    trace_buf_idx = logger.alloc_trace_buffer("ocpp", 1 << 17);

    config = Config::Object({
        {"enable", Config::Bool(false)},
        {"url", Config::Str("", 0, 128)},
        {"identity", Config::Str("", 0, 64)},
        {"enable_auth",Config::Bool(false)},
        {"pass", Config::Str("", 0, 64)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)}
    });

    change_configuration = Config::Object({
        {"key", Config::Str("", 0, 64)},
        {"value", Config::Str("", 0, 500)}
    });

#ifdef OCPP_STATE_CALLBACKS
    state = Config::Object({
        {"charge_point_state", Config::Uint8(0)},
        {"charge_point_status", Config::Uint8(0)},
        {"next_profile_eval", Config::Int52(0)},

        {"connector_state", Config::Uint8(0)},
        {"connector_status", Config::Uint8(0)},
        {"tag_id", Config::Str("", 0, 21)},
        {"parent_tag_id", Config::Str("", 0, 21)},
        {"tag_expiry_date", Config::Int32(0)},
        {"tag_timeout", Config::Uint32(0)},
        {"cable_timeout", Config::Uint32(0)},
        {"last_rejected_tag", Config::Str("", 0, 21)},
        {"last_rejected_tag_reason", Config::Uint8(0)},
        {"txn_id", Config::Int32(0)},
        {"txn_start_time", Config::Int52(0)},
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
        {"connected_change_time", Config::Int52(0)},
        {"last_ping_sent", Config::Uint32(0)},
        {"pong_timeout", Config::Uint32(0)},
    });

    const uint16_t BOOL_LEN = 5;
    const uint16_t INT_LEN = 11;
    const uint16_t STR_LEN = 500;

    configuration = Config::Object({
        {config_keys[(size_t)ConfigKey::AuthorizeRemoteTxRequests], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::ClockAlignedDataInterval], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::ConnectionTimeOut], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::ConnectorPhaseRotation], Config::Str("", 0, STR_LEN)},
        {config_keys[(size_t)ConfigKey::ConnectorPhaseRotationMaxLength], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::GetConfigurationMaxKeys], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::HeartbeatInterval], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::LocalAuthorizeOffline], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::LocalPreAuthorize], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::MessageTimeout], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::MeterValuesAlignedData], Config::Str("", 0, STR_LEN)},
        {config_keys[(size_t)ConfigKey::MeterValuesAlignedDataMaxLength], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::MeterValuesSampledData], Config::Str("", 0, STR_LEN)},
        {config_keys[(size_t)ConfigKey::MeterValuesSampledDataMaxLength], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::MeterValueSampleInterval], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::NumberOfConnectors], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::ResetRetries], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::StopTransactionOnEVSideDisconnect], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::StopTransactionOnInvalidId], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::StopTransactionMaxMeterValues], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::StopTxnAlignedData], Config::Str("", 0, STR_LEN)},
        {config_keys[(size_t)ConfigKey::StopTxnAlignedDataMaxLength], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::StopTxnSampledData], Config::Str("", 0, STR_LEN)},
        {config_keys[(size_t)ConfigKey::StopTxnSampledDataMaxLength], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::SupportedFeatureProfiles], Config::Str("", 0, STR_LEN)},
        {config_keys[(size_t)ConfigKey::TransactionMessageAttempts], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::TransactionMessageRetryInterval], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::UnlockConnectorOnEVSideDisconnect], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::WebSocketPingInterval], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::ChargeProfileMaxStackLevel], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::ChargingScheduleAllowedChargingRateUnit], Config::Str("", 0, STR_LEN)},
        {config_keys[(size_t)ConfigKey::ChargingScheduleMaxPeriods], Config::Str("", 0, INT_LEN)},
        {config_keys[(size_t)ConfigKey::ConnectorSwitch3to1PhaseSupported], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::MaxChargingProfilesInstalled], Config::Str("", 0, INT_LEN)}
    });
#endif
}

bool Ocpp::start_client()
{
    if (!config_in_use.get("enable_auth")->asBool()) {
        return cp->start(config_in_use.get("url")->asEphemeralCStr(), config_in_use.get("identity")->asEphemeralCStr(), nullptr, 0, BasicAuthPassType::NONE);
    }

    const String &pass = config_in_use.get("pass")->asString();
    return cp->start(config_in_use.get("url")->asEphemeralCStr(), config_in_use.get("identity")->asEphemeralCStr(), (const uint8_t *)pass.c_str(), pass.length(), BasicAuthPassType::TRY_BOTH);
}

void Ocpp::setup()
{
    initialized = true;
    if (!api.restorePersistentConfig("ocpp/config", &config)) {
        config.get("identity")->updateString(String(OPTIONS_HOSTNAME_PREFIX()) + "-" + local_uid_str);
    }

    if (!config.get("enable")->asBool() || config.get("url")->asString().length() == 0)
        return;

    config_in_use = config;

    cp = std::unique_ptr<OcppChargePoint>(new OcppChargePoint());

    task_scheduler.scheduleOnce([this](){
        if (!start_client()) {
            state.get("charge_point_state")->updateUint((uint32_t)OcppState::Faulted);
            logger.printfln("Failed to start OCPP client. Check configuration!");
            return;
        }

        task_scheduler.scheduleWithFixedDelay([this](){
            cp->tick();
        }, 100_ms, 100_ms);
    }, 5_s);
}

void Ocpp::register_urls()
{
    api.addPersistentConfig("ocpp/config", &config, {"pass"});
#ifdef OCPP_STATE_CALLBACKS
    api.addState("ocpp/state", &state);
    api.addState("ocpp/configuration", &configuration);
#endif
    api.addCommand("ocpp/reset", Config::Null(), {}, [](String &/*errmsg*/) {
        remove_directory("/ocpp");
    }, true);

#ifdef OCPP_DEBUG
    api.addFeature("ocpp_debug");
    api.addCommand("ocpp/change_configuration", &change_configuration, {}, [this](String &/*errmsg*/) {
        auto status = cp->changeConfig(change_configuration.get("key")->asEphemeralCStr(), change_configuration.get("value")->asEphemeralCStr());
        logger.printfln("Change config %s status %s", change_configuration.get("key")->asEphemeralCStr(), ChangeConfigurationResponseStatusStrings[(size_t) status]);
    }, true);
#endif
}

#if MODULE_NFC_AVAILABLE()
bool Ocpp::on_tag_seen(const char *tag_id)
{
    if (tag_seen_cb == nullptr)
        return false;

    // We have to remove the separating ':'s from the tag_id.
    // OCPP expectes IDs that map to physical tag IDs to contain only the hex-bytes.
    char buf[NFC_TAG_ID_STRING_LENGTH + 1] = {};
    remove_separator(tag_id, buf);

    tag_seen_cb(1, buf, tag_seen_cb_user_data);
    return true;
}
#endif

void Ocpp::pre_reboot() {
    if (cp) {
        cp->stop();
    }
}
