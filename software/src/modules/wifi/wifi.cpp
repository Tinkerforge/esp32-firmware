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

#include "wifi.h"

#include "module_dependencies.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_eap_client.h>
#include <TFJson.h>

#include "event_log_prefix.h"
#include "tools.h"
#include "tools/net.h"
#include "build.h"
#include "tools/string_builder.h"

// result line: {"ssid":"%s","bssid":"%s","rssi":%d,"channel":%d,"encryption":%d}
// worst case length ~140
#define MAX_SCAN_RESULT_LENGTH 145

extern char local_uid_str[32];
extern char passphrase[20];

void Wifi::pre_setup()
{
    ap_config = ConfigRoot{Config::Object({
        {"enable_ap", Config::Bool(true)},
        {"ap_fallback_only", Config::Bool(false)},
        {"ssid", Config::Str("", 0, 32)},
        {"hide_ssid", Config::Bool(false)},
        {"passphrase", Config::Str("", 8, 64)}, // Blank passphrase will be replaced with default passphrase in setup. | FIXME: check if there are only ASCII characters or hex digits (for PSK) here.
        {"channel", Config::Uint(0, 0, 13)},
        {"ip", Config::Str("10.0.0.1", 7, 15)},
        {"gateway", Config::Str("10.0.0.1", 7, 15)},
        {"subnet", Config::Str("255.255.255.0", 7, 15)}
    }), [](Config &cfg, ConfigSource source) -> String {
        IPAddress ip_addr, subnet_mask, gateway_addr;
        if (!ip_addr.fromString(cfg.get("ip")->asUnsafeCStr()))
            return "Failed to parse \"ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!gateway_addr.fromString(cfg.get("gateway")->asUnsafeCStr()))
            return "Failed to parse \"gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!subnet_mask.fromString(cfg.get("subnet")->asUnsafeCStr()))
            return "Failed to parse \"subnet\": Expected format is dotted decimal, i.e. 255.255.255.0";

        if (!is_valid_subnet_mask(subnet_mask))
            return "Invalid subnet mask passed: Expected format is 255.255.255.0";

        uint8_t cidr = WiFiGenericClass::calculateSubnetCIDR(subnet_mask);
        if (cidr < 24 || cidr > 30)
            return "Invalid subnet mask passed: Subnet mask must be at least /30 and not bigger than /24.";

        if (ip_addr != IPAddress(0,0,0,0) && is_in_subnet(ip_addr, subnet_mask, IPAddress(127,0,0,1)))
            return "Invalid IP or subnet mask passed: This configuration would route localhost (127.0.0.1) to the WiFi AP.";

        if (gateway_addr != IPAddress(0,0,0,0) && !is_in_subnet(ip_addr, subnet_mask, gateway_addr))
            return "Invalid IP, subnet mask, or gateway passed: IP and gateway are not in the same network according to the subnet mask.";

        return "";
    }};
    state = Config::Object({
        {"connection_state", Config::Enum(WifiState::NotConfigured)},
        {"connection_start", Config::Uint(0)},
        {"connection_end", Config::Uint(0)},
        {"ap_state", Config::Int(0)},
        {"ap_bssid", Config::Str("", 0, 20)},
        {"ap_sta_count", Config::Uint(0)},
        {"sta_ip", Config::Str("0.0.0.0", 7, 15)},
        {"sta_subnet", Config::Str("0.0.0.0", 7, 15)},
        {"sta_rssi", Config::Int8(-127)},
        {"sta_bssid", Config::Str("", 0, 20)}
    });

    eap_config_prototypes.push_back({EapConfigID::None, *Config::Null()});

    // Max len of identity is currently limited by arduino.
    eap_config_prototypes.push_back({EapConfigID::TLS, Config::Object({
        {"ca_cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
        {"identity", Config::Str("", 0, 64)},
        {"client_cert_id", Config::Int(0, 0, MAX_CERT_ID)},
        {"client_key_id", Config::Int(0, 0, MAX_CERT_ID)}
    })});

    eap_config_prototypes.push_back({EapConfigID::PEAP_TTLS, Config::Object({
        {"ca_cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
        {"identity", Config::Str("", 0, 64)},
        {"username", Config::Str("", 0, 64)},
        {"password", Config::Str("", 0, 64)},
        {"client_cert_id", Config::Int(-1, -1, MAX_CERT_ID)},
        {"client_key_id", Config::Int(-1, -1, MAX_CERT_ID)}
    })});

    Config eap_config_union = Config::Union<EapConfigID>(
        *Config::Null(),
        EapConfigID::None,
        eap_config_prototypes.data(),
        eap_config_prototypes.size());

    sta_config = ConfigRoot{Config::Object({
        {"enable_sta", Config::Bool(false)},
        {"ssid", Config::Str("", 0, 32)},
        {"bssid", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0)
                },
                Config::get_prototype_uint8_0(),
                6,
                6,
                Config::type_id<Config::ConfUint>()
            )
        },
        {"bssid_lock", Config::Bool(false)},
        {"enable_11b", Config::Bool(false)},
        {"passphrase", Config::Str("", 0, 64)},
        {"ip", Config::Str("0.0.0.0", 7, 15)},
        {"gateway", Config::Str("0.0.0.0", 7, 15)},
        {"subnet", Config::Str("0.0.0.0", 7, 15)},
        {"dns", Config::Str("0.0.0.0", 7, 15)},
        {"dns2", Config::Str("0.0.0.0", 7, 15)},
        {"wpa_eap_config", eap_config_union}
    }), [](Config &cfg, ConfigSource source) -> String {
        const String &phrase = cfg.get("passphrase")->asString();
        if (phrase.length() > 0 && phrase.length() < 8)
            return "Passphrase too short. Must be at least 8 characters, or zero if open network.";
        // Fixme: Check if only hex if exactly 64 bytes long: then it's a PSK instead of a passphrase.

        IPAddress ip_addr, subnet_mask, gateway_addr, unused;

        if (!ip_addr.fromString(cfg.get("ip")->asUnsafeCStr()))
            return "Failed to parse \"ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!gateway_addr.fromString(cfg.get("gateway")->asUnsafeCStr()))
            return "Failed to parse \"gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!subnet_mask.fromString(cfg.get("subnet")->asUnsafeCStr()))
            return "Failed to parse \"subnet\": Expected format is dotted decimal, i.e. 255.255.255.0";

        if (!is_valid_subnet_mask(subnet_mask))
            return "Invalid subnet mask passed: Expected format is 255.255.255.0";

        if (ip_addr != IPAddress(0,0,0,0) && is_in_subnet(ip_addr, subnet_mask, IPAddress(127,0,0,1)))
            return "Invalid IP or subnet mask passed: This configuration would route localhost (127.0.0.1) to the WiFi STA interface.";

        if (gateway_addr != IPAddress(0,0,0,0) && !is_in_subnet(ip_addr, subnet_mask, gateway_addr))
            return "Invalid IP, subnet mask, or gateway passed: IP and gateway are not in the same network according to the subnet mask.";

        if (!unused.fromString(cfg.get("dns")->asUnsafeCStr()))
            return "Failed to parse \"dns\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(cfg.get("dns2")->asUnsafeCStr()))
            return "Failed to parse \"dns2\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (cfg.get("wpa_eap_config")->getTag<EapConfigID>() == EapConfigID::PEAP_TTLS) {
            int client_cert_id = cfg.get("wpa_eap_config")->get()->get("client_cert_id")->asInt();
            int client_key_id = cfg.get("wpa_eap_config")->get()->get("client_key_id")->asInt();

            if ((client_cert_id != -1 && client_key_id == -1) || (client_cert_id == -1 && client_key_id != -1)) {
                return "Must provide both, a client certificate and a client key";
            }
        }

        return "";
    }};
}

static float rssi_to_weight(int rssi)
{
    return powf(2, static_cast<float>(128 + rssi) / 10);
}

static void apply_weight(float *channels, int channel, float weight)
{
    for (int i = std::max(1, channel - 2); i <= std::min(13, channel + 2); ++i) {
        if (i == channel - 2 || i == channel + 2)
            channels[i] += weight / 2;
        else
            channels[i] += weight;
    }
}

void Wifi::apply_soft_ap_config_and_start()
{
    // We don't want apply_soft_ap_config_and_start
    // to be called over and over from the fallback AP task
    // if we are still scanning for a channel.
    runtime_ap->soft_ap_running = true;

    if (runtime_ap->channel == 0) {
        if (runtime_ap->scan_start_time_s == 0) {
            logger.printfln("Starting scan to select unoccupied channel for soft AP");
            WiFi.scanDelete();
            WiFi.scanNetworks(true, true);
            runtime_ap->scan_start_time_s = std::max(now_us().to<seconds_t>().as<uint32_t>(), 1ul);
            task_scheduler.scheduleOnce([this]() {
                this->apply_soft_ap_config_and_start();
            }, 500_ms);
            return;
        } else { // Scan already started
            int16_t network_count = WiFi.scanComplete();

            if (network_count == WIFI_SCAN_RUNNING && !deadline_elapsed(seconds_t{runtime_ap->scan_start_time_s + 10})) {
                task_scheduler.scheduleOnce([this]() {
                    this->apply_soft_ap_config_and_start();
                }, 500_ms);
                return;
            }

            if (network_count >= 0) { // Scan finished successfully
                float channels[14] = {0}; // Don't use 0, channels are one-based.
                float channels_smeared[14] = {0}; // Don't use 0, channels are one-based.
                for (int16_t i = 0; i < network_count; ++i) {
                    const wifi_ap_record_t *info = static_cast<decltype(info)>(WiFi.getScanInfoByIndex(i));

                    int channel = info->primary;
                    float weight = rssi_to_weight(info->rssi);
                    apply_weight(channels, channel, weight);

                    if (info->second == WIFI_SECOND_CHAN_ABOVE) {
                        apply_weight(channels, channel + 4, weight);
                    } else if (info->second == WIFI_SECOND_CHAN_BELOW) {
                        apply_weight(channels, channel - 4, weight);
                    }
                }

                memcpy(channels_smeared, channels, ARRAY_SIZE(channels_smeared));

                for (int i = 1; i <= 13; ++i) {
                    if (i > 1)
                        channels_smeared[i] += channels[i - 1];
                    if (i < 13)
                        channels_smeared[i] += channels[i + 1];
                }

                uint32_t min = 1;
                for (uint32_t i = 1; i <= 13; ++i) {
                    if (channels_smeared[i] < channels_smeared[min])
                        min = i;
                }
                logger.printfln("Selecting channel %lu for soft AP", min);
                runtime_ap->channel = min & 0xF;
            }

            // If channel_to_use is still 0, either a timeout occurred or the scan failed.
            if (runtime_ap->channel == 0) {
                runtime_ap->channel = ((esp_random() % 4) * 4 + 1) & 0xF;
                logger.printfln("Channel selection scan timeout elapsed! Randomly selected channel %hhu", runtime_ap->channel);
            }
        }
    }

    // Convert addresses before enableAP in order to keep the time between enableAP and softAPConfig as short as possible.
    const IPAddress ip{runtime_ap->ip_ssid_passphrase};
    const IPAddress subnet{tf_ip4addr_cidr2mask(runtime_ap->subnet_cidr).addr};

    // AP must be enabled before the bandwidth can be set.
    if (!WiFi.enableAP(true)) {
        logger.printfln("AP enable failed");
        return;
    }

    if (!WiFi.AP.waitStatusBits(ESP_NETIF_STARTED_BIT, 1000)) {
        logger.printfln("AP netif not started after 1s. Expect problems.");
    }

    // We don't need the additional speed of HT40 and it only causes more errors.
    // Must be set directly after enabling the interface because it might clobber some other settings.
    esp_err_t err = esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);
    if (err != ESP_OK) {
        logger.printfln("Setting HT20 for AP failed: %s (%i)", esp_err_to_name(err), err);
    }

    WiFi.AP.create(runtime_ap->ip_ssid_passphrase + runtime_ap->ssid_offset,
                   runtime_ap->ip_ssid_passphrase + runtime_ap->passphrase_offset,
                   runtime_ap->channel,
                   runtime_ap->hide_ssid);

    if (!WiFi.softAPConfig(ip, {runtime_ap->gateway.addr}, subnet)) {
        logger.printfln("softAPConfig failed");
    }

    WiFi.setSleep(false);

    uint8_t bssid[6];
    if (!WiFi.AP.macAddress(bssid)) {
        memset(bssid, 0, sizeof(bssid));
    }
    char bssid_str[18];
    const size_t bssid_strlen = snprintf_u(bssid_str, sizeof(bssid_str), "%02X:%02X:%02X:%02X:%02X:%02X", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

    state.get("ap_bssid")->updateString(String{bssid_str, bssid_strlen});

    logger.printfln("Soft AP started");
    logger.printfln_continue("SSID: %s", runtime_ap->ip_ssid_passphrase + runtime_ap->ssid_offset);
    logger.printfln_continue("MAC address: %s", bssid_str);
    logger.printfln_continue("IP address: %s", runtime_ap->ip_ssid_passphrase);
}

bool Wifi::apply_sta_config_and_connect()
{
    if (get_connection_state() == WifiState::Connected) {
        return false;
    }

    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(false, true);
    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);

    const char *ssid = sta_config_in_use.get("ssid")->asEphemeralCStr();
    const char *passphrase = sta_config_in_use.get("passphrase")->asEphemeralCStr();
    const bool bssid_lock = sta_config_in_use.get("bssid_lock")->asBool();

    uint8_t bssid[6];
    if (bssid_lock) {
        sta_config_in_use.get("bssid")->fillUint8Array(bssid, ARRAY_SIZE(bssid));
    }

    const IPAddress ip{sta_config_in_use.get("ip")->asUnsafeCStr()};
    if (static_cast<uint32_t>(ip) != 0) {
        WiFi.config(ip,
                    {sta_config_in_use.get("gateway")->asUnsafeCStr()},
                    {sta_config_in_use.get("subnet" )->asUnsafeCStr()},
                    {sta_config_in_use.get("dns"    )->asUnsafeCStr()},
                    {sta_config_in_use.get("dns2"   )->asUnsafeCStr()});
    } else {
        WiFi.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
    }

    if (bssid_lock) {
        logger.printfln("Connecting to '%s', locked to BSSID %02X:%02X:%02X:%02X:%02X:%02X", ssid, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    } else {
        logger.printfln("Connecting to '%s'", ssid);
    }
    EapConfigID eap_config_id = static_cast<EapConfigID>(sta_config_in_use.get("wpa_eap_config")->as<OwnedConfig::OwnedConfigUnion>()->tag);
    switch (eap_config_id) {
        case EapConfigID::None:
            WiFi.setMinSecurity(WIFI_AUTH_WPA_WPA2_PSK);
            WiFi.begin(ssid, passphrase, 0, bssid_lock ? bssid : nullptr, true);
            break;

        case EapConfigID::TLS:
            WiFi.setMinSecurity(WIFI_AUTH_WPA2_ENTERPRISE);
            WiFi.begin(ssid,
                    wpa2_auth_method_t::WPA2_AUTH_TLS,
                    eap_identity.c_str(),
                    nullptr,
                    nullptr,
                    (const char *)ca_cert.get(),
                    //ca_cert_len,
                    (const char *)client_cert.get(),
                    //client_cert_len,
                    (const char *)client_key.get(),
                    //client_key_len,
                    //"",
                    //0,
                    -1,
                    0,
                    bssid_lock ? bssid : nullptr,
                    true);
            break;
        /**
         * The auth method can be hardcoded here because arduino is not controlling the actual method and we need to
         * set the username and password.
         * The user cert and key are unused because using them breaks it atm.
        */
        case EapConfigID::PEAP_TTLS:
            WiFi.setMinSecurity(WIFI_AUTH_WPA2_ENTERPRISE);
            WiFi.begin(ssid,
                    wpa2_auth_method_t::WPA2_AUTH_PEAP,
                    eap_identity.c_str(),
                    eap_username.c_str(),
                    eap_password.c_str(),
                    (const char *)ca_cert.get(),
                    //ca_cert_len,
                    nullptr,
                    //0,
                    nullptr,
                    //0,
                    //nullptr,
                    //0,
                    -1,
                    0,
                    bssid_lock ? bssid : nullptr,
                    true);
            break;

        default:
            break;
    }
    WiFi.setSleep(false);
    return true;
}

#define DEFAULT_REASON_STRING(X) case X: return "Unknown reason (" #X ")";

static const char *reason2str(uint8_t reason)
{
    switch (reason) {
        case WIFI_REASON_NO_AP_FOUND:
            return "Access Point not found. Is the reception too poor or the SSID incorrect?";
        case WIFI_REASON_AUTH_FAIL:
            return "Authentication failed. Is the passphrase correct?";
        case WIFI_REASON_ASSOC_FAIL:
            return "Assiociation failed.";
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_BEACON_TIMEOUT:
            return "Reception too poor";

        DEFAULT_REASON_STRING(WIFI_REASON_UNSPECIFIED)
        DEFAULT_REASON_STRING(WIFI_REASON_AUTH_EXPIRE)
        DEFAULT_REASON_STRING(WIFI_REASON_AUTH_LEAVE)
        DEFAULT_REASON_STRING(WIFI_REASON_ASSOC_EXPIRE)
        DEFAULT_REASON_STRING(WIFI_REASON_ASSOC_TOOMANY)
        DEFAULT_REASON_STRING(WIFI_REASON_NOT_AUTHED)
        DEFAULT_REASON_STRING(WIFI_REASON_NOT_ASSOCED)
        DEFAULT_REASON_STRING(WIFI_REASON_ASSOC_LEAVE)
        DEFAULT_REASON_STRING(WIFI_REASON_ASSOC_NOT_AUTHED)
        DEFAULT_REASON_STRING(WIFI_REASON_DISASSOC_PWRCAP_BAD)
        DEFAULT_REASON_STRING(WIFI_REASON_DISASSOC_SUPCHAN_BAD)
        DEFAULT_REASON_STRING(WIFI_REASON_BSS_TRANSITION_DISASSOC)
        DEFAULT_REASON_STRING(WIFI_REASON_IE_INVALID)
        DEFAULT_REASON_STRING(WIFI_REASON_MIC_FAILURE)
        DEFAULT_REASON_STRING(WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT)
        DEFAULT_REASON_STRING(WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT)
        DEFAULT_REASON_STRING(WIFI_REASON_IE_IN_4WAY_DIFFERS)
        DEFAULT_REASON_STRING(WIFI_REASON_GROUP_CIPHER_INVALID)
        DEFAULT_REASON_STRING(WIFI_REASON_PAIRWISE_CIPHER_INVALID)
        DEFAULT_REASON_STRING(WIFI_REASON_AKMP_INVALID)
        DEFAULT_REASON_STRING(WIFI_REASON_UNSUPP_RSN_IE_VERSION)
        DEFAULT_REASON_STRING(WIFI_REASON_INVALID_RSN_IE_CAP)
        DEFAULT_REASON_STRING(WIFI_REASON_802_1X_AUTH_FAILED)
        DEFAULT_REASON_STRING(WIFI_REASON_CIPHER_SUITE_REJECTED)

        DEFAULT_REASON_STRING(WIFI_REASON_INVALID_PMKID)

        DEFAULT_REASON_STRING(WIFI_REASON_CONNECTION_FAIL)
        DEFAULT_REASON_STRING(WIFI_REASON_AP_TSF_RESET)
        DEFAULT_REASON_STRING(WIFI_REASON_ROAMING)

        default:
            return "Unknown reason.";
    }
}

void Wifi::setup()
{
    initialized = true;

    if (!api.restorePersistentConfig("wifi/sta_config", &sta_config, API::SavedDefaultConfig::Keep)) {
#ifdef DEFAULT_WIFI_STA_ENABLE
        sta_config.get("enable_sta")->updateBool(DEFAULT_WIFI_STA_ENABLE);
#endif
#ifdef DEFAULT_WIFI_STA_SSID
        sta_config.get("ssid")->updateString(String(DEFAULT_WIFI_STA_SSID));
#endif
#ifdef DEFAULT_WIFI_STA_PASSPHRASE
        sta_config.get("passphrase")->updateString(String(DEFAULT_WIFI_STA_PASSPHRASE));
#endif
    }

    if (!api.restorePersistentConfig("wifi/ap_config", &ap_config, API::SavedDefaultConfig::Keep)) {
#ifdef DEFAULT_WIFI_AP_ENABLE
        ap_config.get("enable_ap")->updateBool(DEFAULT_WIFI_AP_ENABLE);
#endif
#ifdef DEFAULT_WIFI_AP_FALLBACK_ONLY
        ap_config.get("ap_fallback_only")->updateBool(DEFAULT_WIFI_AP_FALLBACK_ONLY);
#endif
#ifdef DEFAULT_WIFI_AP_SSID
        ap_config.get("ssid")->updateString(String(DEFAULT_WIFI_AP_SSID));
#else
        String default_ap_ssid{BUILD_HOST_PREFIX};
        default_ap_ssid.concat("-", 1);
        default_ap_ssid.concat(local_uid_str);
        ap_config.get("ssid")->updateString(default_ap_ssid);
#endif
#ifdef DEFAULT_WIFI_AP_PASSPHRASE
        ap_config.get("passphrase")->updateString(String(DEFAULT_WIFI_AP_PASSPHRASE));
#else
        ap_config.get("passphrase")->updateString(passphrase); // Default AP passphrase
#endif
    }

    if (ap_config.get("enable_ap")->asBool()) {
        const String &ip   = ap_config.get("ip"        )->asString();
        const String &ssid = ap_config.get("ssid"      )->asString();
        const String &pass = ap_config.get("passphrase")->asString();

        const size_t ip_len_term   = ip.length()   + 1; // Include null-termination.
        const size_t ssid_len_term = ssid.length() + 1;
        const size_t pass_len_term = pass.length() + 1;

        runtime_ap = static_cast<decltype(runtime_ap)>(malloc_psram_or_dram(offsetof(struct ap_runtime, ip_ssid_passphrase) + ip_len_term + ssid_len_term + pass_len_term));

        if (!runtime_ap) {
            logger.printfln("Failed to allocate memory for WiFi AP runtime data");
        } else {
            ip4addr_aton(ap_config.get("gateway")->asUnsafeCStr(), &runtime_ap->gateway);

            ip4_addr_t subnet;
            ip4addr_aton(ap_config.get("subnet" )->asUnsafeCStr(), &subnet);
            runtime_ap->subnet_cidr = tf_ip4addr_mask2cidr(subnet);

            runtime_ap->ap_fallback_only = ap_config.get("ap_fallback_only")->asBool();
            runtime_ap->hide_ssid        = ap_config.get("hide_ssid"       )->asBool();
            runtime_ap->channel          = ap_config.get("channel"         )->asUint() & 0xF;

            memcpy(runtime_ap->ip_ssid_passphrase, ip.c_str(), ip_len_term);

            runtime_ap->ssid_offset = static_cast<uint8_t>(ip_len_term);
            memcpy(runtime_ap->ip_ssid_passphrase + runtime_ap->ssid_offset, ssid.c_str(), ssid_len_term);

            runtime_ap->passphrase_offset = static_cast<uint8_t>(runtime_ap->ssid_offset + ssid_len_term);
            memcpy(runtime_ap->ip_ssid_passphrase + runtime_ap->passphrase_offset, pass.c_str(), pass_len_term);

            runtime_ap->soft_ap_running   = false;
            runtime_ap->scan_start_time_s = 0;
            runtime_ap->stop_soft_ap_runs = 0;
        }
    }

    sta_config_in_use = sta_config.get_owned_copy();
    const bool enable_sta = sta_config_in_use.get("enable_sta")->asBool();

    if (enable_sta) {
        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
                uint8_t reason_code = info.wifi_sta_disconnected.reason;
                const char *reason = reason2str(reason_code);
                if (!this->was_connected) {
                    logger.printfln("Failed to connect to '%s': %s (%u)", sta_config_in_use.get("ssid")->asEphemeralCStr(), reason, reason_code);
                } else {
                    auto now = now_us();
                    auto connected_for = now - last_connected;
                    logger.printfln("Disconnected from '%s': %s (%u). Was connected for %lu seconds.", sta_config_in_use.get("ssid")->asEphemeralCStr(), reason, reason_code, connected_for.to<seconds_t>().as<uint32_t>());

                    uint32_t now_ms = now.to<millis_t>().as<uint32_t>();
                    task_scheduler.scheduleOnce([this, now_ms](){
                        state.get("connection_end")->updateUint(now_ms);
                    });
                }

                task_scheduler.scheduleOnce([this](){
                    state.get("sta_ip")->updateString("0.0.0.0");
                    state.get("sta_subnet")->updateString("0.0.0.0");
                    state.get("sta_bssid")->updateString("");
                });

                this->was_connected = false;
            },
            ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
                wifi_ap_record_t wifi_info;
                if (esp_wifi_sta_get_ap_info(&wifi_info) != ESP_OK) {
                    logger.printfln("Connected to WiFi");
                } else {
                    char buf[128];
                    StringWriter sw(buf, ARRAY_SIZE(buf));
                    sw.printf("'%s', ", reinterpret_cast<const char *>(wifi_info.ssid));

                    if (wifi_info.phy_11a)       sw.puts("a+");
                    if (wifi_info.phy_11b)       sw.puts("b+");
                    if (wifi_info.phy_11g)       sw.puts("g+");
                    if (wifi_info.phy_11n)       sw.puts("n+");
                    if (wifi_info.phy_11ac)      sw.puts("ac+");
                    if (wifi_info.phy_11ax)      sw.puts("ax+");
                    sw.setLength(sw.getLength() - 1); // Remove trailing plus or space if no mode was added.

                    sw.printf(" ch.%hhu", wifi_info.primary);

                    if      (wifi_info.bandwidth == WIFI_BW_HT20)   sw.puts(" HT20");
                    else if (wifi_info.bandwidth == WIFI_BW_HT40)   sw.puts(" HT40");
                    else if (wifi_info.bandwidth == WIFI_BW80)      sw.puts(" VHT80");
                    else if (wifi_info.bandwidth == WIFI_BW160)     sw.puts(" VHT160");
                    else if (wifi_info.bandwidth == WIFI_BW80_BW80) sw.puts(" VHT80+80");

                    if (wifi_info.phy_lr)        sw.puts(" lr");
                    if (wifi_info.wps)           sw.puts(" WPS");
                    if (wifi_info.ftm_responder) sw.puts(" FTMr");
                    if (wifi_info.ftm_initiator) sw.puts(" FTMi");

                    sw.puts(" [");
                    if (wifi_info.country.cc[0] != '\0') {
                        sw.printf("%-2.2s", wifi_info.country.cc);
                    }
                    const char oix = wifi_info.country.cc[2];
                    switch(oix) {
                        case '\0':
                            break;
                        case ' ':
                        case 'O':
                        case 'I':
                        case 'X':
                            sw.putc(oix);
                            break;
                        default:
                            sw.printf("%u", oix);
                    }

                    sw.printf("] %hhidBm, BSSID %02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX",
                            wifi_info.rssi,
                            wifi_info.bssid[0], wifi_info.bssid[1], wifi_info.bssid[2], wifi_info.bssid[3], wifi_info.bssid[4], wifi_info.bssid[5]);

                    logger.printfln("Connected to %s", buf);
                }

                this->was_connected = true;
                this->last_connected = now_us();

                uint32_t now_ms = this->last_connected.to<millis_t>().as<uint32_t>();
                task_scheduler.scheduleOnce([this, now_ms]() {
                    state.get("connection_start")->updateUint(now_ms);
                });
            },
            ARDUINO_EVENT_WIFI_STA_CONNECTED);

        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
                // Sometimes the ARDUINO_EVENT_WIFI_STA_CONNECTED is not fired.
                // Instead we get the ARDUINO_EVENT_WIFI_STA_GOT_IP twice?
                // Make sure that the state is set to connected here,
                // or else MQTT will never attempt to connect.
                this->was_connected = true;

                auto ip = WiFi.localIP().toString();
                auto subnet = WiFi.subnetMask();
                logger.printfln("Got IP address: %s/%u. Own MAC address: %s", ip.c_str(), WiFiGenericClass::calculateSubnetCIDR(subnet), WiFi.macAddress().c_str());
                task_scheduler.scheduleOnce([this, ip, subnet](){
                    state.get("sta_ip")->updateString(ip);
                    state.get("sta_subnet")->updateString(subnet.toString());
                    state.get("sta_bssid")->updateString(WiFi.BSSIDstr());
                });
            },
            ARDUINO_EVENT_WIFI_STA_GOT_IP);

        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
                logger.printfln("Got IPv6 address: TODO PRINT ADDRESS.");
            },
            ARDUINO_EVENT_WIFI_STA_GOT_IP6);

        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
                if(!this->was_connected)
                    return;

                this->was_connected = false;

                logger.printfln("Lost IP. Forcing disconnect and reconnect of WiFi");
                WiFi.disconnect(false, true);

                task_scheduler.scheduleOnce([this](){
                    state.get("sta_ip")->updateString("0.0.0.0");
                    state.get("sta_subnet")->updateString("0.0.0.0");
                    state.get("sta_bssid")->updateString("");
                });
            },
            ARDUINO_EVENT_WIFI_STA_LOST_IP);
    }

    if (runtime_ap) {
        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
                logger.printfln("STA with MAC address %02X:%02X:%02X:%02X:%02X:%02X connected to AP",
                                info.wifi_ap_staconnected.mac[0],
                                info.wifi_ap_staconnected.mac[1],
                                info.wifi_ap_staconnected.mac[2],
                                info.wifi_ap_staconnected.mac[3],
                                info.wifi_ap_staconnected.mac[4],
                                info.wifi_ap_staconnected.mac[5]);

                task_scheduler.scheduleOnce([this]() {
                    auto ap_sta_count = state.get("ap_sta_count");
                    ap_sta_count->updateUint(ap_sta_count->asUint() + 1);
                });

            },
            ARDUINO_EVENT_WIFI_AP_STACONNECTED);

        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
                logger.printfln("STA with MAC address %02X:%02X:%02X:%02X:%02X:%02X disconnected from AP",
                                info.wifi_ap_staconnected.mac[0],
                                info.wifi_ap_staconnected.mac[1],
                                info.wifi_ap_staconnected.mac[2],
                                info.wifi_ap_staconnected.mac[3],
                                info.wifi_ap_staconnected.mac[4],
                                info.wifi_ap_staconnected.mac[5]);

                task_scheduler.scheduleOnce([this]() {
                    auto ap_sta_count = state.get("ap_sta_count");
                    uint32_t ap_sta_count_value = ap_sta_count->asUint();

                    if (ap_sta_count_value == 0) {
                        logger.printfln("Unexpected disconnect from AP, STA count was already zero");
                    }
                    else {
                        ap_sta_count->updateUint(ap_sta_count_value - 1);
                    }
                });
            },
            ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
    }

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            const wifi_event_sta_scan_done_t *scan_info = &info.wifi_scan_done;

            if (scan_info->status != 0) {
                logger.printfln("Scan failed");
                return;
            }

            const size_t network_count = scan_info->number;
            logger.printfln("%zu networks found", network_count);

#if MODULE_WS_AVAILABLE()
            if (!ws.haveActiveClient()) {
                return;
            }

            StringBuilder sb;

            if (ws.pushRawStateUpdateBegin(&sb, MAX_SCAN_RESULT_LENGTH * network_count + 2, "wifi/scan_results")) {
                get_scan_results(&sb, network_count);
                WiFi.scanDelete();
                ws.pushRawStateUpdateEnd(&sb);
            }
#endif
        },
        ARDUINO_EVENT_WIFI_SCAN_DONE);

    // For some reason WiFi.setHostname only writes a temporary buffer that is passed to the IDF when calling WiFi.mode.
    // As we have the same hostname for STA and AP, it is sufficient to set the hostname here once and never call WiFi.softAPsetHostname.
#if MODULE_NETWORK_AVAILABLE()
    WiFi.setHostname(network.get_hostname().c_str());
#else
    WiFi.setHostname((String(BUILD_HOST_PREFIX) + "-" + local_uid_str).c_str());
#endif

    WiFi.persistent(false);
    WiFi.disableSTA11b(!sta_config_in_use.get("enable_11b")->asBool());

    // STA mode is always on so that we can scan for WiFis and the HWRNG has entropy.
    if (!WiFi.mode(WIFI_STA)) {
        logger.printfln("WiFi STA enable failed");
        return;
    }
    WiFi.setAutoReconnect(false);

    // Check if WiFi connected automatically and erase configuration in that case.
    if (WiFi.status() != WL_STOPPED) {
        WiFi.disconnect(false, true);
    }

    // We don't need the additional speed of HT40 and it only causes more errors.
    // Cannot be set in apply_sta_config_and_connect() and must be set directly after enabling station mode
    // because it apparently clobbers some interface settings, including IGMP memberships.
    esp_err_t err = esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20);
    if (err != ESP_OK) {
        logger.printfln("Setting HT20 for station failed: %s (%i)", esp_err_to_name(err), err);
    }

    esp_wifi_set_country_code("DE", true);
    esp_wifi_set_ps(WIFI_PS_NONE);

    // Request max TX power. The actual power will be limited by the country setting above.
    WiFi.setTxPower(WIFI_POWER_21dBm);

    if (runtime_ap) {
        if (!runtime_ap->ap_fallback_only) {
            apply_soft_ap_config_and_start();
        } else {
            task_scheduler.scheduleWithFixedDelay([this]() {
                bool connected = this->state.get("connection_state")->asEnum<WifiState>() == WifiState::Connected;

#if MODULE_ETHERNET_AVAILABLE()
                connected = connected || ethernet.get_connection_state() == EthernetState::Connected;
#endif

                // Start softAP immediately, but stop it only if
                // we got a connection in the first 30 seconds after start-up
                // or we had a connection for 5 minutes.

                if (connected == this->runtime_ap->soft_ap_running) {
                    if (this->runtime_ap->soft_ap_running) {
                        ++this->runtime_ap->stop_soft_ap_runs;
                        if (now_us() < 30_s || this->runtime_ap->stop_soft_ap_runs > 5 * 6) { // 5 * 6 * 10_s task delay = 5 minutes
                            logger.printfln("Network connected. Stopping soft AP");
                            WiFi.softAPdisconnect(true);
                            this->runtime_ap->soft_ap_running = false;
                        }
                    } else {
                        this->runtime_ap->stop_soft_ap_runs = 0;

                        apply_soft_ap_config_and_start();
                    }
                }
            },
            enable_sta
#if MODULE_ETHERNET_AVAILABLE()
            || (ethernet.is_enabled() && ethernet.get_connection_state() != EthernetState::NotConnected)
#endif
            ? 30_s : 6_s, 10_s); // When Ethernet is not connected yet, wait 6 seconds: 4s for link up, 1s for DHCP, 1s for tolerance.
        }

        task_scheduler.scheduleWithFixedDelay([this]() {
            state.get("ap_state")->updateInt(get_ap_state());
        }, 5_s, 5_s);
    }

    if (enable_sta) {
        task_scheduler.scheduleWithFixedDelay([this]() {
            state.get("connection_state")->updateEnum(get_connection_state());
        }, 1_s);

        task_scheduler.scheduleWithFixedDelay([this]() {
            int rssi = -127;
            esp_wifi_sta_get_rssi(&rssi); // Ignore failure, rssi is still -127.
            state.get("sta_rssi")->updateInt(rssi);

            static int tries = 0; // TODO move to runtime
            if (tries < 3 || tries % 3 == 2)
                if (!apply_sta_config_and_connect())
                    tries = 0;
            tries++;
        }, 10_s, 10_s);
    }

    EapConfigID eap_config_id = static_cast<EapConfigID>(sta_config_in_use.get("wpa_eap_config")->as<OwnedConfig::OwnedConfigUnion>()->tag);
    if (eap_config_id != EapConfigID::None) {
        const OwnedConfig::OwnedConfigWrap &eap_config = sta_config_in_use.get("wpa_eap_config")->get();
        int ca_id = eap_config->get("ca_cert_id")->asInt();
        int client_cert_id = eap_config->get("client_cert_id")->asInt();
        int client_key_id = eap_config->get("client_key_id")->asInt();
        eap_identity = eap_config->get("identity")->asString();
        if (ca_id != -1) {
            ca_cert = certs.get_cert(ca_id, &ca_cert_len);
        }
        switch (eap_config_id) {
            case EapConfigID::TLS:
            {
                client_cert = certs.get_cert(client_cert_id, &client_cert_len);
                client_key = certs.get_cert(client_key_id, &client_key_len);

                const CoolString &tmp_identity = eap_config->get("identity")->asString();
                if (tmp_identity.length() > 0) {
                    eap_identity = tmp_identity;
                } else {
                    eap_identity = "anonymous";
                }
            }
                break;

            case EapConfigID::PEAP_TTLS:
                eap_username = eap_config->get("username")->asString();
                eap_password = eap_config->get("password")->asString();

                if (client_cert_id != -1) {
                    client_cert = certs.get_cert(eap_config->get("client_cert_id")->asInt(), &client_cert_len);
                }
                if (client_key_id != -1) {
                    client_key = certs.get_cert(eap_config->get("client_key_id")->asInt(), &client_key_len);
                }

                break;

            default:
                break;
        }
    }

    if (enable_sta) {
        wl_status_t old_status = WL_STOPPED;
        wl_status_t status;
        micros_t state_deadline;

        // The WiFi status can change a few times after enabling station mode.
        // Wait for it to be in a stable disconnected state for 10ms.
        // Don't hang if this takes longer than 1s. It usually takes only 12ms.
        micros_t check_deadline = now_us() + 1_s;
        for (;;) {
            status = WiFi.STA.status();
            if (status != old_status) {
                if (status == WL_DISCONNECTED) {
                    state_deadline = now_us() + 10_ms;
                }
                old_status = status;
            }

            if (status == WL_DISCONNECTED && deadline_elapsed(state_deadline)) {
                break;
            }

            if (deadline_elapsed(check_deadline)) {
                logger.printfln("WiFi stuck in state %i? Continuing...", status);
                break;
            }
        }

        if (!apply_sta_config_and_connect()) {
            logger.printfln("Couldn't apply STA config and connect during setup");
        }
    }
}

void Wifi::get_scan_results(StringBuilder *sb, size_t network_count)
{
    sb->putc('[');

    for (size_t i = 0; i < network_count; ++i) {
        const wifi_ap_record_t *ap_record = reinterpret_cast<wifi_ap_record_t *>(WiFi.getScanInfoByIndex(static_cast<int>(i)));
        if (!ap_record) {
            continue;
        }

        if (i > 0) {
            sb->putc(',');
        }

        char json_buf[2 + 64] = ""; // use twice the maximum SSID length to have room for escaping
        TFJsonSerializer json{json_buf, sizeof(json_buf)};

        json.addString(reinterpret_cast<const char *>(ap_record->ssid));
        json.end();

        const uint8_t *bssid = ap_record->bssid;

        sb->printf("{\"ssid\":%s,\"bssid\":\"%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\",\"rssi\":%hhi,\"channel\":%hhu,\"encryption\":%i}",
                   json_buf,
                   bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5],
                   ap_record->rssi,
                   ap_record->primary,
                   ap_record->authmode);
    }

    sb->putc(']');
}

void Wifi::start_scan()
{
    // Abort if a scan is running. This is safe, because
    // the state will change to SCAN_FAILED if it timed out.
    if (WiFi.scanComplete() == WIFI_SCAN_RUNNING)
        return;

    logger.printfln("Scanning for WiFi networks...");
    WiFi.scanDelete();

    // WIFI_SCAN_FAILED also means the scan is done.
    if (WiFi.scanComplete() != WIFI_SCAN_FAILED)
        return;

    if (WiFi.scanNetworks(true, true) != WIFI_SCAN_FAILED) {
        return;
    }

    logger.printfln("Starting WiFi scan failed. Maybe a connection attempt is running concurrently. Retrying once in 6 seconds.");
    task_scheduler.scheduleOnce([this]() {
        if (WiFi.scanNetworks(true, true) == WIFI_SCAN_FAILED) {
            logger.printfln("Second scan attempt failed. Giving up.");
            return;
        }
    }, 6_s);
}

void Wifi::register_urls()
{
    api.addState("wifi/state", &state);

    api.addCommand("wifi/scan", Config::Null(), {}, [this](String &/*errmsg*/) {
        start_scan();
    }, true);

    server.on("/wifi/scan_results", HTTP_GET, [this](WebServerRequest request) {
        int16_t network_count = WiFi.scanComplete();

        if (network_count == WIFI_SCAN_RUNNING) {
            return request.send(200, "text/plain; charset=utf-8", "scan in progress");
        }

        if (network_count < 0) {
            return request.send(200, "text/plain; charset=utf-8", "scan failed");
        }

        StringBuilder sb;

        if (!sb.setCapacity(MAX_SCAN_RESULT_LENGTH * static_cast<size_t>(network_count) + 2)) {
            return request.send(200, "text/plain; charset=utf-8", "scan out of memory");
        }

        get_scan_results(&sb, static_cast<size_t>(network_count));

        return request.send(200, "application/json; charset=utf-8", sb.getPtr(), static_cast<ssize_t>(sb.getLength()));
    });

    api.addPersistentConfig("wifi/sta_config", &sta_config, {"passphrase", "password"});
    api.addPersistentConfig("wifi/ap_config", &ap_config, {"passphrase"});
}

WifiState Wifi::get_connection_state() const
{
    if (!sta_config_in_use.get("enable_sta")->asBool())
        return WifiState::NotConfigured;

    switch (WiFi.status()) {
        case WL_CONNECT_FAILED:
        case WL_CONNECTION_LOST:
        case WL_DISCONNECTED:
        case WL_NO_SSID_AVAIL:
            return WifiState::NotConnected;
        case WL_SCAN_COMPLETED: // Part of the enum but apparently never set.
        case WL_CONNECTED:
            return WifiState::Connected;
        case WL_STOPPED:
        case WL_NO_SHIELD:
            return WifiState::NotConfigured;
        case WL_IDLE_STATUS:
            return WifiState::Connecting;
        default:
            esp_system_abort("Invalid WiFi status");
    }
}

bool Wifi::is_sta_enabled() const
{
    return sta_config_in_use.get("enable_sta")->asBool();
}

int Wifi::get_ap_state()
{
    if (runtime_ap == nullptr)
        return 0; // Disabled

    if (!runtime_ap->ap_fallback_only)
        return 1; // Enabled

    if (!runtime_ap->soft_ap_running)
        return 2; // Fallback inactive

    return 3; // Fallback active
}

int Wifi::get_sta_rssi() const
{
    return state.get("sta_rssi")->asInt();
}

const char* Wifi::get_ap_ssid() const
{
    if (runtime_ap) {
        return runtime_ap->ip_ssid_passphrase + runtime_ap->ssid_offset;
    } else {
        return ap_config.get("ssid")->asUnsafeCStr();
    }
}

const char* Wifi::get_ap_ip() const
{
    if (runtime_ap) {
        return runtime_ap->ip_ssid_passphrase;
    } else {
        return ap_config.get("ip")->asUnsafeCStr();
    }
}

const char* Wifi::get_ap_passphrase() const
{
    if (runtime_ap) {
        return runtime_ap->ip_ssid_passphrase + runtime_ap->passphrase_offset;
    } else {
        return ap_config.get("passphrase")->asUnsafeCStr();
    }
}
