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

#include <algorithm>
#include <vector>
#include <mbedtls/base64.h>
#include <mbedtls/platform_util.h>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"

Ocpp::Iso15118SeccChain::~Iso15118SeccChain()
{
    if (key_pem != nullptr) {
        mbedtls_platform_zeroize(key_pem.get(), key_pem_capacity);
    }
}
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

#if MODULE_ISO15118_AVAILABLE()
    // Fill ISO15118Ctrlr.ProtocolSupported instances from the SAP protocol list
    cp21->device_model.iso15118_pnc_supported = iso15118.supports_pnc();
    for (size_t i = 0; i < ARRAY_SIZE(iso15118_supported_protocols) && i < OCPP21_SUPPORTED_PROTOCOLS; i++) {
        snprintf(cp21->device_model.protocol_supported[i], sizeof(cp21->device_model.protocol_supported[i]), "%s", iso15118_supported_protocols[i]);
    }
#endif

    // The TLS file locators are interpreted by the platform:
    // "certid:<n>" refers to a certificate of the certs module, nullptr
    // means verification against the bundled roots.
    char ca_locator[16];
    PlatformTlsConfig tls;
    const bool enable_auth = config.get("enable_auth")->asBool();
    int32_t security_profile = 1;
    if (is_tls) {
        if (enable_auth) {
            security_profile = 2;
        }
        int8_t cert_id = config.get("cert_id")->asInt();
        if (cert_id >= 0) {
            snprintf(ca_locator, sizeof(ca_locator), "certid:%d", cert_id);
            tls.ca_cert_file = ca_locator;
        }
    }

    const char *pass = nullptr;
    if (enable_auth) {
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
        cp21->register15118EVCertificateResult([](bool accepted, const char *exi_response, int32_t remaining_contracts, void *user_data) {
            static_cast<Ocpp *>(user_data)->on_ev_cert_result(accepted, exi_response, remaining_contracts);
        }, this);

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

const Ocpp21::CertEntry *Ocpp::best_iso15118_secc_chain(bool iso20, bool *valid_out)
{
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
    if (valid_out != nullptr) {
        *valid_out = best_valid;
    }
    return best;
}

// Enumerates up to capacity chains for ISO 15118-2 (iso20=false) or
// ISO 15118-20 (iso20=true), freshest not_before first. Returns the
// number written to chains_out.
size_t Ocpp::get_iso15118_secc_chains(bool iso20, Iso15118SeccChain *chains_out, size_t capacity)
{
    if (!cp21 || !client_started || chains_out == nullptr || capacity == 0) {
        return 0;
    }

    auto group = iso20 ? Ocpp21::CertGroup::V2G20Chain : Ocpp21::CertGroup::V2GChain;
    time_t now = platform_get_system_time(cp21->connection.platform_ctx);
    std::vector<const Ocpp21::CertEntry *> candidates;
    for (const auto &e : cp21->cert_store.all()) {
        if (e.group != group || !e.has_anchor || e.not_before > now || now > e.not_after) {
            continue;
        }
        if (iso20 &&
            ((e.public_key_curve == OcppCurve21::Ed448 && !cp21->device_model.v2g20_use_ed448) ||
             (e.public_key_curve == OcppCurve21::Secp521r1 && !cp21->device_model.v2g20_use_secp521r1) ||
             (e.public_key_curve != OcppCurve21::Ed448 && e.public_key_curve != OcppCurve21::Secp521r1))) {
            continue;
        }
        candidates.push_back(&e);
    }
    std::sort(candidates.begin(), candidates.end(), [](const Ocpp21::CertEntry *a, const Ocpp21::CertEntry *b) {
        return a->not_before != b->not_before ? a->not_before > b->not_before : a->id < b->id;
    });

    static constexpr size_t key_pem_max = 2048;
    size_t count = 0;
    for (const auto *entry : candidates) {
        if (count >= capacity) {
            break;
        }

        Iso15118SeccChain candidate;
        candidate.chain_pem = heap_alloc_array<char>(OCPP21_CERT_PEM_MAX + 1);
        candidate.key_pem = heap_alloc_array<char>(key_pem_max + 1);
        candidate.key_pem_capacity = candidate.key_pem == nullptr ? 0 : key_pem_max + 1;
        std::string root = cp21->cert_store.loadRootByHash(entry->anchor_root);
        candidate.root_pem = heap_alloc_array<char>(root.size() + 1);
        if (candidate.chain_pem == nullptr || candidate.key_pem == nullptr || root.empty() || candidate.root_pem == nullptr) {
            continue;
        }
        if (cp21->cert_store.readPem(*entry, candidate.chain_pem.get(), OCPP21_CERT_PEM_MAX + 1) == 0) {
            continue;
        }
        const char *root_ptr = candidate.root_pem.get();
        memcpy(candidate.root_pem.get(), root.c_str(), root.size() + 1);
        if (platform_verify_chain21(candidate.chain_pem.get(), &root_ptr, 1, now, nullptr) != OcppChainVerifyResult21::Ok) {
            continue;
        }
        size_t key_len = platform_read_file(cp21->cert_store.keyPath(entry->id).c_str(), candidate.key_pem.get(), key_pem_max);
        if (key_len == 0) {
            continue;
        }
        candidate.key_pem[key_len] = '\0';
        candidate.chain_id = entry->id;
        chains_out[count++] = std::move(candidate);
    }
    return count;
}

bool Ocpp::get_iso15118_secc_chain(bool iso20, std::unique_ptr<char[]> *chain_pem_out, std::unique_ptr<char[]> *key_pem_out)
{
    if (!cp21 || !client_started) {
        return false;
    }

    const Ocpp21::CertEntry *best = best_iso15118_secc_chain(iso20, nullptr);
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
    key[key_len] = '\0';

    *chain_pem_out = std::move(chain);
    *key_pem_out = std::move(key);
    return true;
}

std::unique_ptr<char[]> Ocpp::get_iso15118_root_bundle(RootGroup group)
{
    if (!cp21 || !client_started) {
        return nullptr;
    }

    Ocpp21::CertGroup cert_group;
    switch (group) {
        case RootGroup::OEM: cert_group = Ocpp21::CertGroup::OEMRoot; break;
        case RootGroup::MO:  cert_group = Ocpp21::CertGroup::MORoot;  break;
        case RootGroup::V2G:
        default:             cert_group = Ocpp21::CertGroup::V2GRoot; break;
    }
    size_t count = 0;
    for (const auto &e : cp21->cert_store.all()) {
        if (e.group == cert_group) {
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
        if (e.group != cert_group) {
            continue;
        }
        used += cp21->cert_store.readPem(e, bundle.get() + used, OCPP21_ROOT_PEM_MAX + 1);
    }
    if (used == 0) {
        return nullptr;
    }
    return bundle;
}

bool Ocpp::get_iso15118_ctrlr(Iso15118CtrlrValues *out)
{
    if (!cp21 || !client_started) {
        return false;
    }

    out->enforce_tls = cp21->device_model.enforce_tls_enabled;
    out->pwm_charging_fallback_timeout_s = cp21->device_model.pwm_charging_fallback_timeout_s;
    snprintf(out->evse_id, sizeof(out->evse_id), "%s", cp21->device_model.iso15118_evse_id);
    return true;
}

bool Ocpp::is_iso15118_enabled()
{
    return !cp21 || !client_started || cp21->device_model.iso15118_enabled;
}

bool Ocpp::is_iso15118_store_live()
{
    return cp21 && client_started;
}

bool Ocpp::is_iso20_suite_enabled(OcppCurve21 curve) const
{
    if (!cp21 || !client_started) {
        return false;
    }
    switch (curve) {
        case OcppCurve21::Secp521r1:
            return cp21->device_model.v2g20_use_secp521r1;
        case OcppCurve21::Ed448:
            return cp21->device_model.v2g20_use_ed448;
        case OcppCurve21::Secp256r1:
        case OcppCurve21::Unknown:
            return false;
    }
    return false;
}

bool Ocpp::is_iso20_ocsp_required() const
{
    return !private_environment_waives_iso15118_ocsp();
}

constexpr bool Ocpp::supports_iso15118_pnc()
{
#if MODULE_ISO15118_AVAILABLE()
    return iso15118.supports_pnc();
#else
    return false;
#endif
}

bool Ocpp::private_environment_waives_iso15118_ocsp() const
{
    return cp21 && client_started && cp21->device_model.private_environment_enabled && !supports_iso15118_pnc();
}

// HUB20-532-002: TLS 1.3 with the -20 chain is only offered with a
// time valid chain and Good OCSP status for every chain
// certificate. Private mode waives OCSP only in builds without PnC.
bool Ocpp::is_iso20_tls_ready()
{
    if (!cp21 || !client_started) {
        return false;
    }

    bool valid = false;
    const Ocpp21::CertEntry *best = best_iso15118_secc_chain(true, &valid);
    if (best == nullptr || !valid) {
        return false;
    }
    if (private_environment_waives_iso15118_ocsp()) {
        return true;
    }
    return cp21->seccChainOcspStatus(best->id) == OcppOcspStatus21::Good;
}

bool Ocpp::is_iso20_tls_ready(uint32_t chain_id)
{
    if (!cp21 || !client_started) {
        return false;
    }
    const Ocpp21::CertEntry *chain = cp21->cert_store.findSeccChainById(chain_id);
    time_t now = platform_get_system_time(cp21->connection.platform_ctx);
    if (chain == nullptr || chain->group != Ocpp21::CertGroup::V2G20Chain || !chain->has_anchor ||
        chain->not_before > now || now > chain->not_after) {
        return false;
    }
    if (private_environment_waives_iso15118_ocsp()) {
        return true;
    }
    return cp21->seccChainOcspStatus(chain_id) == OcppOcspStatus21::Good;
}

// Raw OCSP response for one certificate of an explicitly selected SECC
// chain (0 = leaf), copied so the caller owns it.
bool Ocpp::get_iso15118_ocsp_staple(uint32_t chain_id, uint8_t cert_idx, std::unique_ptr<uint8_t[]> *der_out, size_t *der_len_out)
{
    if (!cp21 || !client_started) {
        return false;
    }

    const uint8_t *der = nullptr;
    size_t der_len = 0;
    if (!cp21->seccChainOcspResponse(chain_id, cert_idx, &der, &der_len)) {
        return false;
    }

    auto copy = heap_alloc_array<uint8_t>(der_len);
    if (copy == nullptr) {
        return false;
    }
    memcpy(copy.get(), der, der_len);
    *der_out = std::move(copy);
    *der_len_out = der_len;
    return true;
}

bool Ocpp::get_iso15118_ocsp_staple(uint8_t cert_idx, std::unique_ptr<uint8_t[]> *der_out, size_t *der_len_out)
{
    if (!cp21 || !client_started) {
        return false;
    }
    const Ocpp21::CertEntry *best = best_iso15118_secc_chain(true, nullptr);
    if (best == nullptr) {
        return false;
    }
    return get_iso15118_ocsp_staple(best->id, cert_idx, der_out, der_len_out);
}

static size_t cert_der_to_pem(const uint8_t *der, size_t der_len, char *pem, size_t pem_cap)
{
    size_t b64_cap = (der_len + 2) / 3 * 4 + 1;
    auto b64 = heap_alloc_array<uint8_t>(b64_cap);
    size_t b64_len = 0;

    if (b64 == nullptr || mbedtls_base64_encode(b64.get(), b64_cap, &b64_len, der, der_len) != 0) {
        return 0;
    }

    const char *begin = "-----BEGIN CERTIFICATE-----\n";
    const char *end = "-----END CERTIFICATE-----\n";
    size_t need = strlen(begin) + strlen(end) + b64_len + (b64_len + 63) / 64 + 1;
    if (need > pem_cap) {
        return 0;
    }

    size_t pos = static_cast<size_t>(snprintf(pem, pem_cap, "%s", begin));
    for (size_t i = 0; i < b64_len; i += 64) {
        size_t chunk = std::min<size_t>(64, b64_len - i);
        memcpy(pem + pos, b64.get() + i, chunk);
        pos += chunk;
        pem[pos++] = '\n';
    }
    pos += static_cast<size_t>(snprintf(pem + pos, pem_cap - pos, "%s", end));
    return pos;
}

bool Ocpp::request_iso15118_vehicle_chain_status(const VehicleChainCertDer *chain, size_t chain_len, const uint8_t *root_der, size_t root_len)
{
    vehicle_chain_count = 0;
    vehicle_chain_response_received = false;
    vehicle_chain_request_failed = true; // Cleared once the request is under way or served from cache

    if (!cp21 || !client_started) {
        return false;
    }
    if (private_environment_waives_iso15118_ocsp()) {
        vehicle_chain_request_failed = false;
        vehicle_chain_response_received = true;
        return true;
    }
    if (chain == nullptr || chain_len == 0 || chain_len > OCPP21_VEHICLE_OCSP_CACHE_SIZE || root_der == nullptr) {
        return false;
    }

    // One PEM bundle: Chain leaf first, the anchoring root appended as issuer of the topmost certificate.
    static constexpr size_t pem_max = 4096;
    size_t bundle_cap = (chain_len + 1) * pem_max;
    auto bundle = heap_alloc_array<char>(bundle_cap);
    if (bundle == nullptr) {
        return false;
    }
    size_t used = 0;
    for (size_t i = 0; i < chain_len; ++i) {
        size_t len = cert_der_to_pem(chain[i].der, chain[i].len, bundle.get() + used, bundle_cap - used);
        if (len == 0) {
            return false;
        }
        used += len;
    }
    if (cert_der_to_pem(root_der, root_len, bundle.get() + used, bundle_cap - used) == 0) {
        return false;
    }

    char urls[OCPP21_VEHICLE_OCSP_CACHE_SIZE][256];
    const char *url_ptrs[OCPP21_VEHICLE_OCSP_CACHE_SIZE];
    for (size_t i = 0; i < chain_len; ++i) {
        if (!platform_cert_hash_data21(bundle.get(), i, bundle.get(), i + 1, &vehicle_chain_hashes[i])) {
            return false;
        }
        if (!platform_cert_ocsp_url21(bundle.get(), i, urls[i], sizeof(urls[i]))) {
            urls[i][0] = '\0';
        }
        url_ptrs[i] = urls[i];
    }
    vehicle_chain_count = chain_len;

    // HUB20-432-003/007: The request may only be skipped when every chain
    // certificate has a currently valid cached status.
    // Otherwise the full chain is requested.
    time_t now = platform_get_system_time(cp21->connection.platform_ctx);
    bool all_cached = true;
    for (size_t i = 0; i < chain_len; ++i) {
        const Ocpp21::VehicleOcspStatus *entry = cp21->vehicleChainStatus(vehicle_chain_hashes[i]);
        if (entry == nullptr || entry->next_update <= now) {
            all_cached = false;
            break;
        }
    }
    if (all_cached) {
        vehicle_chain_request_failed = false;
        vehicle_chain_response_received = true;
        return true;
    }

    if (!cp21->requestVehicleChainStatus(vehicle_chain_hashes, url_ptrs, chain_len)) {
        return false;
    }
    vehicle_chain_request_failed = false;
    return true;
}

void Ocpp::on_vehicle_chain_status_result(bool response_received)
{
    if (response_received) {
        vehicle_chain_response_received = true;
    } else {
        vehicle_chain_request_failed = true;
    }
}

Ocpp::VehicleChainCheck Ocpp::get_iso15118_vehicle_chain_check()
{
    if (!cp21 || !client_started) {
        return VehicleChainCheck::NotRequired;
    }
    if (private_environment_waives_iso15118_ocsp()) {
        return VehicleChainCheck::NotRequired;
    }
    if (vehicle_chain_count == 0 || vehicle_chain_request_failed) {
        return VehicleChainCheck::Unknown;
    }
    if (!vehicle_chain_response_received) {
        return VehicleChainCheck::Pending;
    }

    // HUB20-432-008/009/010: Revoked dominates, everything not currently cached as Good counts as Unknown.
    using Status = Ocpp21::GetCertificateChainStatusResponseCertificateStatusEntryEntriesStatus;
    time_t now = platform_get_system_time(cp21->connection.platform_ctx);
    VehicleChainCheck result = VehicleChainCheck::Good;
    for (size_t i = 0; i < vehicle_chain_count; ++i) {
        const Ocpp21::VehicleOcspStatus *entry = cp21->vehicleChainStatus(vehicle_chain_hashes[i]);
        if (entry != nullptr && entry->status == Status::REVOKED) {
            return VehicleChainCheck::Revoked;
        }
        if (entry == nullptr || entry->next_update <= now || entry->status != Status::GOOD) {
            result = VehicleChainCheck::Unknown;
        }
    }
    return result;
}

// M01.FR.01/02/03, M02: forward the EV CertificateInstallationReq or
// CertificateUpdateReq EXI stream to the CSMS. maximumContractCertificateChains is only passed for -20.
bool Ocpp::request_iso15118_ev_certificate(bool iso20, bool update, const uint8_t *exi, size_t exi_len, int32_t max_contract_chains)
{
    ev_cert_status = EvCertStatus::Failed;
    ev_cert_exi = nullptr;
    ev_cert_exi_len = 0;
    ev_cert_remaining = 0;

    if (!supports_iso15118_pnc() || !cp21 || !client_started || (exi == nullptr) || (exi_len == 0)) {
        return false;
    }

    size_t b64_cap = (exi_len + 2) / 3 * 4 + 1;
    auto b64 = heap_alloc_array<char>(b64_cap);
    size_t b64_len = 0;
    if (b64 == nullptr || mbedtls_base64_encode(reinterpret_cast<uint8_t *>(b64.get()), b64_cap, &b64_len, exi, exi_len) != 0) {
        return false;
    }
    b64.get()[b64_len] = '\0';

    const char *schema = iso20 ? "urn:iso:std:iso:15118:-20:CommonMessages" : "urn:iso:15118:2:2013:MsgDef";
    if (!cp21->request15118EVCertificate(schema, update, b64.get(), iso20 ? max_contract_chains : -1)) {
        return false;
    }
    ev_cert_status = EvCertStatus::Pending;
    return true;
}

void Ocpp::on_ev_cert_result(bool accepted, const char *exi_response, int32_t remaining_contracts)
{
    if (ev_cert_status != EvCertStatus::Pending) {
        return;
    }
    ev_cert_status = EvCertStatus::Failed;
    if (!accepted || (exi_response == nullptr)) {
        return;
    }

    size_t b64_len = strlen(exi_response);
    size_t exi_cap = (b64_len / 4 + 1) * 3;
    auto exi = heap_alloc_array<uint8_t>(exi_cap);
    size_t exi_len = 0;
    if (exi == nullptr || mbedtls_base64_decode(exi.get(), exi_cap, &exi_len, reinterpret_cast<const uint8_t *>(exi_response), b64_len) != 0 || exi_len == 0) {
        return;
    }

    ev_cert_exi = std::move(exi);
    ev_cert_exi_len = exi_len;
    ev_cert_remaining = remaining_contracts;
    ev_cert_status = EvCertStatus::Accepted;
}

Ocpp::EvCertStatus Ocpp::get_iso15118_ev_cert_status()
{
    if (!cp21 || !client_started) {
        return EvCertStatus::Failed;
    }
    return ev_cert_status;
}


void Ocpp::reset_iso15118_ev_cert_response()
{
    ev_cert_status = EvCertStatus::Idle;
    ev_cert_exi = nullptr;
    ev_cert_exi_len = 0;
    ev_cert_remaining = 0;
}


bool Ocpp::take_iso15118_ev_cert_response(std::unique_ptr<uint8_t[]> *exi_out, size_t *exi_len_out, int32_t *remaining_out)
{
    if ((ev_cert_status != EvCertStatus::Accepted) || (ev_cert_exi == nullptr)) {
        return false;
    }
    *exi_out = std::move(ev_cert_exi);
    *exi_len_out = ev_cert_exi_len;
    *remaining_out = ev_cert_remaining;
    ev_cert_exi_len = 0;
    ev_cert_status = EvCertStatus::Idle;
    return true;
}

bool Ocpp::is_iso15118_contract_install_enabled()
{
    return supports_iso15118_pnc() && cp21 && client_started && cp21->device_model.iso15118_enabled && cp21->device_model.contract_cert_install_enabled;
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
