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
#include "generated/module_dependencies.h"
#include "build.h"
#include "options.h"
#include "tools.h"
#include "tools/fs.h"

#include "common/Platform.h"
#include "ocpp16/Configuration16.h"

static void reset_state(Config *state) {
    state->get("charge_point_state")->updateUint(0);
    state->get("charge_point_status")->updateUint(0);
    state->get("next_profile_eval")->updateInt52(0);
    state->get("connector_state")->updateUint(0);
    state->get("connector_status")->updateUint(0);
    state->get("tag_id")->updateString("");
    state->get("parent_tag_id")->updateString("");
    state->get("tag_expiry_date")->updateInt(0);
    state->get("tag_timeout")->updateUint(0);
    state->get("cable_timeout")->updateUint(0);
    state->get("last_rejected_tag")->updateString("");
    state->get("last_rejected_tag_reason")->updateUint(0);
    state->get("txn_id")->updateInt(INT32_MAX);
    state->get("txn_start_time")->updateInt52(0);
    state->get("current")->updateUint(0);
    state->get("txn_with_invalid_id")->updateBool(false);
    state->get("unavailable_requested")->updateBool(false);
    state->get("message_in_flight_type")->updateUint(0);
    state->get("message_in_flight_id_high")->updateUint(0);
    state->get("message_in_flight_id_low")->updateUint(0);
    state->get("message_in_flight_len")->updateUint(0);
    state->get("message_timeout")->updateUint(0);
    state->get("txn_msg_retry_timeout")->updateUint(0);
    state->get("message_queue_depth")->updateUint(0);
    state->get("status_queue_depth")->updateUint(0);
    state->get("txn_msg_queue_depth")->updateUint(0);
    state->get("connected")->updateBool(false);
    state->get("connected_change_time")->updateInt52(0);
    state->get("last_ping_sent")->updateUint(0);
    state->get("pong_timeout")->updateUint(0);
}

void Ocpp::pre_setup()
{
    trace_buf_idx = logger.alloc_trace_buffer("ocpp", 128 * 1024);

    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"protocol", Config::Enum(OcppProtocolVersion::V16, OcppProtocolVersion::V16, OcppProtocolVersion::V21)},
        {"url", Config::Str("", 0, 128)},
        {"identity", Config::Str("", 0, 64)},
        {"enable_auth",Config::Bool(false)},
        {"pass", Config::Str("", 0, 64)},
        {"cert_id", Config::Int(-1, -1, MAX_CERT_ID)}
    }), [this](Config &cfg, ConfigSource source) -> String {
#ifndef OCPP_STATE_CALLBACKS
#error "OCPP_STATE_CALLBACKS are required to detect a running transaction!"
#endif
        if (this->state.get("txn_id")->asInt32() != INT32_MAX)
            return "OCPP config may not be updated while a transaction is in progress! Unplug the vehicle.";

        if (source != ConfigSource::File)
            task_scheduler.scheduleOnce([this](){ this->apply_config(); });
        return "";
    }};

    change_configuration = Config::Object({
        {"key", Config::Str("", 0, 64)},
        {"value", Config::Str("", 0, 500)}
    });


    state = Config::Object({
        {"charge_point_state", Config::Uint8(0)},
        {"charge_point_status", Config::Uint8(0)},
        {"next_profile_eval", Config::Uptime()},

        {"connector_state", Config::Uint8(0)},
        {"connector_status", Config::Uint8(0)},
        {"tag_id", Config::Str("", 0, 21)},
        {"parent_tag_id", Config::Str("", 0, 21)},
        {"tag_expiry_date", Config::Int32(0)},
        {"tag_timeout", Config::Uint32(0)},
        {"cable_timeout", Config::Uint32(0)},
        {"last_rejected_tag", Config::Str("", 0, 21)},
        {"last_rejected_tag_reason", Config::Uint8(0)},
        {"txn_id", Config::Int32(INT32_MAX)},
        {"txn_start_time", Config::Uptime()},
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
        {"connected_change_time", Config::Uptime()},
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
        {config_keys[(size_t)ConfigKey::MaxChargingProfilesInstalled], Config::Str("", 0, INT_LEN)},

        // "Signed Meter Values in OCPP"
        {config_keys[(size_t)ConfigKey::MeterPublicKey1], Config::Str("", 0, STR_LEN)},
        {config_keys[(size_t)ConfigKey::PublicKeyWithSignedMeterValue], Config::Str("", 0, STR_LEN)},
        {config_keys[(size_t)ConfigKey::SampledDataSignReadings], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::StartTxnSampledData], Config::Str("", 0, STR_LEN)},
        {config_keys[(size_t)ConfigKey::SampledDataSignStartedReadings], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::SampledDataSignUpdatedReadings], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::AlignedDataSignReadings], Config::Str("", 0, BOOL_LEN)},
        {config_keys[(size_t)ConfigKey::AlignedDataSignUpdatedReadings], Config::Str("", 0, BOOL_LEN)}
    });
}

bool Ocpp::start_client()
{
    if (!config.get("enable_auth")->asBool()) {
        return cp->start(config.get("url")->asEphemeralCStr(), config.get("identity")->asEphemeralCStr(), nullptr, 0, BasicAuthPassType::NONE);
    }

    const String &pass = config.get("pass")->asString();
    return cp->start(config.get("url")->asEphemeralCStr(), config.get("identity")->asEphemeralCStr(), (const uint8_t *)pass.c_str(), pass.length(), BasicAuthPassType::TRY_BOTH);
}

bool Ocpp::start_client_21()
{
    const String &url = config.get("url")->asString();
    bool is_tls = url.startsWith("wss://");

    // The TLS file locators are interpreted by the platform:
    // "certid:<n>" refers to a certificate of the certs module, nullptr
    // means verification against the bundled roots.
    char ca_locator[16];
    PlatformTlsConfig tls;
    int32_t security_profile = 1;
    if (is_tls) {
        security_profile = 2;
        int8_t cert_id = config.get("cert_id")->asInt();
        if (cert_id >= 0) {
            snprintf(ca_locator, sizeof(ca_locator), "certid:%d", cert_id);
            tls.ca_cert_file = ca_locator;
        }
    }

    const char *pass = nullptr;
    if (config.get("enable_auth")->asBool()) {
        pass = config.get("pass")->asEphemeralCStr();
    }

    return cp21->start(url.c_str(), config.get("identity")->asEphemeralCStr(), pass, security_profile, is_tls ? &tls : nullptr);
}

void Ocpp::apply_config() {
    task_scheduler.cancel(task_id);
    task_id = 0;

    if (cp && client_started) {
        cp->stop();
        reset_state(&state);
    }

    if (cp21 && client_started) {
        cp21->stop();
        reset_state(&state);
    }

    cp = nullptr;
    cp21 = nullptr;
    platform_cert_store_changed21(nullptr);

    if (!config.get("enable")->asBool() || config.get("url")->asString().length() == 0) {
        return;
    }

    if (config.get("protocol")->asEnum<OcppProtocolVersion>() == OcppProtocolVersion::V21) {
        cp21 = std::unique_ptr<Ocpp21::ChargePoint>(new Ocpp21::ChargePoint());

        client_started = start_client_21();
        if (!client_started) {
            state.get("charge_point_state")->updateUint((uint32_t)OcppState::Faulted);
            logger.printfln("Failed to start OCPP 2.1 client. Check configuration!");
            cp21 = nullptr;
            return;
        }

        platform_cert_store_changed21(nullptr);
        task_id = task_scheduler.scheduleWithFixedDelay([this](){
            cp21->tick();
            // The 2.1 stack has no state callbacks yet: poll the
            // connection and registration state instead.
            state.get("connected")->updateBool(platform_ws_connected(cp21->connection.platform_ctx));
            state.get("charge_point_state")->updateUint((uint32_t)cp21->state);
        }, 100_ms, 100_ms);
        return;
    }

    cp = std::unique_ptr<OcppChargePoint>(new OcppChargePoint());

    client_started = start_client();
    if (!client_started) {
        state.get("charge_point_state")->updateUint((uint32_t)OcppState::Faulted);
        logger.printfln("Failed to start OCPP client. Check configuration!");
        cp = nullptr;
        return;
    }

    task_id = task_scheduler.scheduleWithFixedDelay([this](){
        cp->tick();
    }, 100_ms, 100_ms);
}

bool Ocpp::get_iso15118_secc_chain(bool iso20, std::unique_ptr<char[]> *chain_pem_out, std::unique_ptr<char[]> *key_pem_out)
{
    if (!cp21 || !client_started) {
        return false;
    }

    // One chain per anchoring V2G root can be stored.
    // Prefer a chain that is currently valid and the freshest one on a tie.
    auto group = iso20 ? Ocpp21::CertGroup::V2G20Chain : Ocpp21::CertGroup::V2GChain;
    time_t now = platform_get_system_time(cp21->connection.platform_ctx);
    const Ocpp21::CertEntry *best = nullptr;
    bool best_valid = false;
    for (const auto &e : cp21->cert_store.all()) {
        if (e.group != group) {
            continue;
        }
        bool valid = e.not_before <= now && now <= e.not_after;
        if (best == nullptr || (valid && !best_valid) || (valid == best_valid && e.not_before > best->not_before)) {
            best = &e;
            best_valid = valid;
        }
    }
    if (best == nullptr) {
        return false;
    }

    auto chain = heap_alloc_array<char>(OCPP21_CERT_PEM_MAX + 1);
    if (chain == nullptr || cp21->cert_store.readPem(*best, chain.get(), OCPP21_CERT_PEM_MAX + 1) == 0) {
        return false;
    }

    static constexpr size_t key_pem_max = 2048;
    auto key = heap_alloc_array<char>(key_pem_max + 1);
    if (key == nullptr) {
        return false;
    }
    size_t key_len = platform_read_file(cp21->cert_store.keyPath(best->id).c_str(), key.get(), key_pem_max);
    if (key_len == 0) {
        return false;
    }
    key.get()[key_len] = '\0';

    *chain_pem_out = std::move(chain);
    *key_pem_out = std::move(key);
    return true;
}

std::unique_ptr<char[]> Ocpp::get_iso15118_root_bundle(bool oem)
{
    if (!cp21 || !client_started) {
        return nullptr;
    }

    auto group = oem ? Ocpp21::CertGroup::OEMRoot : Ocpp21::CertGroup::V2GRoot;
    size_t count = 0;
    for (const auto &e : cp21->cert_store.all()) {
        if (e.group == group) {
            ++count;
        }
    }
    if (count == 0) {
        return nullptr;
    }

    auto bundle = heap_alloc_array<char>(count * OCPP21_ROOT_PEM_MAX + 1);
    if (bundle == nullptr) {
        return nullptr;
    }
    size_t used = 0;
    for (const auto &e : cp21->cert_store.all()) {
        if (e.group != group) {
            continue;
        }
        used += cp21->cert_store.readPem(e, bundle.get() + used, OCPP21_ROOT_PEM_MAX + 1);
    }
    if (used == 0) {
        return nullptr;
    }
    return bundle;
}

void Ocpp::setup()
{
    initialized = true;
    if (!api.restorePersistentConfig("ocpp/config", &config)) {
        config.get("identity")->updateString(esp32_common.get_default_name());
    }

    // Should we use on_network_connected here?
    task_scheduler.scheduleOnce([this](){ this->apply_config(); }, 5_s);
}

void Ocpp::register_urls()
{
    api.addPersistentConfig("ocpp/config", &config, {"pass"});
    api.addState("ocpp/state", &state);
    api.addState("ocpp/configuration", &configuration);

    api.addCommand("ocpp/reset", Config::Null(), {}, [](Language /*language*/, String &/*errmsg*/) {
        remove_directory("/ocpp");
    }, true);

#ifdef OCPP_DEBUG
    api.addFeature("ocpp_debug");
    api.addCommand("ocpp/change_configuration", &change_configuration, {}, [this](Language /*language*/, String &/*errmsg*/) {
        if (!cp)
            return;

        auto status = cp->changeConfig(change_configuration.get("key")->asEphemeralCStr(), change_configuration.get("value")->asEphemeralCStr());
        logger.printfln("Change config %s status %s", change_configuration.get("key")->asEphemeralCStr(), ChangeConfigurationResponseStatusStrings[(size_t) status]);
    }, true);
#endif
}

#if MODULE_NFC_AVAILABLE()
bool Ocpp::on_tag_seen(const char *tag_id)
{
    if (tag_seen_cb == nullptr) {
        return false;
    }

    tag_seen_cb(1, tag_id, tag_seen_cb_user_data);
    return true;
}
#endif

void Ocpp::pre_reboot() {
    if (cp) {
        cp->stop();
        cp = nullptr;
    }
    if (cp21) {
        cp21->stop();
        cp21 = nullptr;
    }
}
