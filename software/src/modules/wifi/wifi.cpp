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

#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_wpa2.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "build.h"
#include "string_builder.h"

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
        {"passphrase", Config::Str("this-will-be-replaced-in-setup", 8, 64)},//FIXME: check if there are only ASCII characters or hex digits (for PSK) here.
        {"channel", Config::Uint(0, 0, 13)},
        {"ip", Config::Str("10.0.0.1", 7, 15)},
        {"gateway", Config::Str("10.0.0.1", 7, 15)},
        {"subnet", Config::Str("255.255.255.0", 7, 15)}
    }), [](Config &cfg, ConfigSource source) -> String {
        IPAddress ip_addr, subnet_mask, gateway_addr;
        if (!ip_addr.fromString(cfg.get("ip")->asEphemeralCStr()))
            return "Failed to parse \"ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!gateway_addr.fromString(cfg.get("gateway")->asEphemeralCStr()))
            return "Failed to parse \"gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!subnet_mask.fromString(cfg.get("subnet")->asEphemeralCStr()))
            return "Failed to parse \"subnet\": Expected format is dotted decimal, i.e. 10.0.0.1";

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
        {"connection_state", Config::Uint((uint)WifiState::NotConfigured)},
        {"connection_start", Config::Uint(0)},
        {"connection_end", Config::Uint(0)},
        {"ap_state", Config::Int(0)},
        {"ap_bssid", Config::Str("", 0, 20)},
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
                new Config{Config::Uint8(0)},
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

        if (!ip_addr.fromString(cfg.get("ip")->asEphemeralCStr()))
            return "Failed to parse \"ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!gateway_addr.fromString(cfg.get("gateway")->asEphemeralCStr()))
            return "Failed to parse \"gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!subnet_mask.fromString(cfg.get("subnet")->asEphemeralCStr()))
            return "Failed to parse \"subnet\": Expected format is dotted decimal, i.e. 255.255.255.0";

        if (!is_valid_subnet_mask(subnet_mask))
            return "Invalid subnet mask passed: Expected format is 255.255.255.0";

        if (ip_addr != IPAddress(0,0,0,0) && is_in_subnet(ip_addr, subnet_mask, IPAddress(127,0,0,1)))
            return "Invalid IP or subnet mask passed: This configuration would route localhost (127.0.0.1) to the WiFi STA interface.";

        if (gateway_addr != IPAddress(0,0,0,0) && !is_in_subnet(ip_addr, subnet_mask, gateway_addr))
            return "Invalid IP, subnet mask, or gateway passed: IP and gateway are not in the same network according to the subnet mask.";

        if (!unused.fromString(cfg.get("dns")->asEphemeralCStr()))
            return "Failed to parse \"dns\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(cfg.get("dns2")->asEphemeralCStr()))
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

float rssi_to_weight(int rssi)
{
    return pow(2, (float)(128 + rssi) / 10);
}

void apply_weight(float *channels, int channel, float weight)
{
    for (int i = MAX(1, channel - 2); i <= MIN(13, channel + 2); ++i) {
        if (i == channel - 2 || i == channel + 2)
            channels[i] += weight / 2;
        else
            channels[i] += weight;
    }
}

void Wifi::apply_soft_ap_config_and_start()
{
    static uint32_t scan_start_time = 0;
    static int channel_to_use = ap_config_in_use.get("channel")->asUint();

    // We don't want apply_soft_ap_config_and_start
    // to be called over and over from the fallback AP task
    // if we are still scanning for a channel.
    soft_ap_running = true;

    if (channel_to_use == 0) {
        if (scan_start_time == 0) {
            logger.printfln("Starting scan to select unoccupied channel for soft AP");
            WiFi.scanDelete();
            WiFi.scanNetworks(true, true);
            scan_start_time = millis();
            task_scheduler.scheduleOnce([this]() {
                this->apply_soft_ap_config_and_start();
            }, 500);
            return;
        } else { // Scan already started
            int network_count = WiFi.scanComplete();

            if (network_count == WIFI_SCAN_RUNNING && !deadline_elapsed(scan_start_time + 10000)) {
                task_scheduler.scheduleOnce([this]() {
                    this->apply_soft_ap_config_and_start();
                }, 500);
                return;
            }

            if (network_count >= 0) { // Scan finished successfully
                float channels[14] = {0}; // Don't use 0, channels are one-based.
                float channels_smeared[14] = {0}; // Don't use 0, channels are one-based.
                for (int i = 0; i < network_count; ++i) {
                    wifi_ap_record_t *info = (wifi_ap_record_t *)WiFi.getScanInfoByIndex(i);

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

                int min = 1;
                for (int i = 1; i <= 13; ++i) {
                    if (channels_smeared[i] < channels_smeared[min])
                        min = i;
                }
                logger.printfln("Selecting channel %d for soft AP", min);
                channel_to_use = min;
            }

            // If channel_to_use is still 0, either a timeout occurred or the scan failed.
            if (channel_to_use == 0) {
                channel_to_use = (esp_random() % 4) * 4 + 1;
                logger.printfln("Channel selection scan timeout elapsed! Randomly selected channel %i", channel_to_use);
            }
        }
    }

    IPAddress ip, gateway, subnet;
    ip.fromString(ap_config_in_use.get("ip")->asEphemeralCStr());
    gateway.fromString(ap_config_in_use.get("gateway")->asEphemeralCStr());
    subnet.fromString(ap_config_in_use.get("subnet")->asEphemeralCStr());

    WiFi.softAP(ap_config_in_use.get("ssid")->asEphemeralCStr(),
                ap_config_in_use.get("passphrase")->asEphemeralCStr(),
                channel_to_use,
                ap_config_in_use.get("hide_ssid")->asBool());

    int ap_config_attempts = 0;
    do {
        if (!WiFi.softAPConfig(ip, gateway, subnet)) {
            logger.printfln("softAPConfig() failed. Try different Access Point settings.");
        }
        ++ap_config_attempts;
    } while (ip != WiFi.softAPIP());

    WiFi.setSleep(false);

    if (ap_config_attempts != 1) {
        logger.printfln("Had to configure soft AP IP address %d times.", ap_config_attempts);
    }
    logger.printfln("Soft AP started");
    logger.printfln_plain("    SSID: %s", ap_config_in_use.get("ssid")->asEphemeralCStr());
    logger.printfln_plain("    MAC address: %s", WiFi.softAPmacAddress().c_str());
    logger.printfln_plain("    IP address: %s", ip.toString().c_str());
}

bool Wifi::apply_sta_config_and_connect()
{
    return apply_sta_config_and_connect(get_connection_state());
}

bool Wifi::apply_sta_config_and_connect(WifiState current_state)
{
    if (current_state == WifiState::Connected) {
        return false;
    }

    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(false, true);

    const char *ssid = sta_config_in_use.get("ssid")->asEphemeralCStr();

    uint8_t bssid[6];
    sta_config_in_use.get("bssid")->fillUint8Array(bssid, ARRAY_SIZE(bssid));

    const char *passphrase = sta_config_in_use.get("passphrase")->asEphemeralCStr();
    bool bssid_lock = sta_config_in_use.get("bssid_lock")->asBool();

    IPAddress ip, subnet, gateway, dns, dns2;

    ip.fromString(sta_config_in_use.get("ip")->asEphemeralCStr());
    subnet.fromString(sta_config_in_use.get("subnet")->asEphemeralCStr());
    gateway.fromString(sta_config_in_use.get("gateway")->asEphemeralCStr());
    dns.fromString(sta_config_in_use.get("dns")->asEphemeralCStr());
    dns2.fromString(sta_config_in_use.get("dns2")->asEphemeralCStr());

    if (ip != 0) {
        WiFi.config(ip, gateway, subnet, dns, dns2);
    } else {
        WiFi.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
    }

    logger.printfln("Connecting to %s", ssid);
    EapConfigID eap_config_id = static_cast<EapConfigID>(sta_config_in_use.get("wpa_eap_config")->as<OwnedConfig::OwnedConfigUnion>()->tag);
    switch (eap_config_id) {
        case EapConfigID::None:
            WiFi.begin(ssid, passphrase, 0, bssid_lock ? bssid : nullptr, true, (uint8_t)3);
            break;

            case EapConfigID::TLS:
                {
                    WiFi.begin(ssid,
                            wpa2_auth_method_t::WPA2_AUTH_TLS,
                            eap_identity.c_str(),
                            nullptr,
                            nullptr,
                            (char *)ca_cert.get(),
                            ca_cert_len,
                            (char *)client_cert.get(),
                            client_cert_len,
                            (char *)client_key.get(),
                            client_key_len,
                            "",
                            0,
                            bssid_lock ? bssid : nullptr,
                            true,
                            3);
                }
                break;
        /**
         * The auth method can be hardcoded here because arduino is not controlling the actual method and we need to
         * set the username and password.
         * The user cert and key are unused because using them breaks it atm.
        */
        case EapConfigID::PEAP_TTLS:
            WiFi.begin(ssid,
                    wpa2_auth_method_t::WPA2_AUTH_PEAP,
                    eap_identity.c_str(),
                    eap_username.c_str(),
                    eap_password.c_str(),
                    (char *)ca_cert.get(),
                    ca_cert_len,
                    nullptr,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    0,
                    bssid_lock ? bssid : nullptr,
                    true,
                    3);
            break;

        default:
            break;
    }
    WiFi.setSleep(false);
    return true;
}

#define DEFAULT_REASON_STRING(X) case X: return "Unknown reason (" #X ")";

const char *reason2str(uint8_t reason)
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
    String default_ap_ssid = String(BUILD_HOST_PREFIX) + "-" + local_uid_str;
    String default_ap_passphrase = String(passphrase);

    if (!api.restorePersistentConfig("wifi/sta_config", &sta_config)) {
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

    if (!api.restorePersistentConfig("wifi/ap_config", &ap_config)) {
#ifdef DEFAULT_WIFI_AP_ENABLE
        ap_config.get("enable_ap")->updateBool(DEFAULT_WIFI_AP_ENABLE);
#endif
#ifdef DEFAULT_WIFI_AP_FALLBACK_ONLY
        ap_config.get("ap_fallback_only")->updateBool(DEFAULT_WIFI_AP_FALLBACK_ONLY);
#endif
#ifdef DEFAULT_WIFI_AP_SSID
        ap_config.get("ssid")->updateString(String(DEFAULT_WIFI_AP_SSID));
#else
        ap_config.get("ssid")->updateString(default_ap_ssid);
#endif
#ifdef DEFAULT_WIFI_AP_PASSPHRASE
        ap_config.get("passphrase")->updateString(String(DEFAULT_WIFI_AP_PASSPHRASE));
#else
        ap_config.get("passphrase")->updateString(default_ap_passphrase);
#endif
    }

    ap_config_in_use = ap_config.get_owned_copy();
    sta_config_in_use = sta_config.get_owned_copy();

    WiFi.persistent(false);
    WiFi.disableSTA11b(!sta_config_in_use.get("enable_11b")->asBool());

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            uint8_t reason_code = info.wifi_sta_disconnected.reason;
            const char *reason = reason2str(reason_code);
            if (!this->was_connected) {
                logger.printfln("Failed to connect to %s: %s (%u)", sta_config_in_use.get("ssid")->asEphemeralCStr(), reason, reason_code);
            } else {
                uint32_t now = millis();
                uint32_t connected_for = now - last_connected_ms;

                // FIXME: Use a better way of time keeping here.
                if (connected_for < 0x7FFFFFFF)
                    logger.printfln("Disconnected from %s: %s (%u). Was connected for %u seconds.", sta_config_in_use.get("ssid")->asEphemeralCStr(), reason, reason_code, connected_for / 1000);
                else
                    logger.printfln("Disconnected from %s: %s (%u). Was connected for a long time.", sta_config_in_use.get("ssid")->asEphemeralCStr(), reason, reason_code);

                task_scheduler.scheduleOnce([this, now](){
                    state.get("connection_end")->updateUint(now);
                }, 0);
            }

            task_scheduler.scheduleOnce([this](){
                state.get("sta_ip")->updateString("0.0.0.0");
                state.get("sta_subnet")->updateString("0.0.0.0");
                state.get("sta_bssid")->updateString("");
            }, 0);

            this->was_connected = false;
        },
        ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            this->was_connected = true;

            logger.printfln("Connected to %s, BSSID %s", WiFi.SSID().c_str(), WiFi.BSSIDstr().c_str());
            auto now = millis();
            last_connected_ms = now;

            task_scheduler.scheduleOnce([this, now](){
                state.get("connection_start")->updateUint(now);
            }, 0);
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
            }, 0);
        },
        ARDUINO_EVENT_WIFI_STA_GOT_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Got IPv6 address: %s.", WiFi.localIPv6().toString().c_str());
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
            }, 0);
        },
        ARDUINO_EVENT_WIFI_STA_LOST_IP);

    bool enable_ap = ap_config_in_use.get("enable_ap")->asBool();
    bool enable_sta = sta_config_in_use.get("enable_sta")->asBool();
    bool ap_fallback_only = ap_config_in_use.get("ap_fallback_only")->asBool();

    // For some reason WiFi.setHostname only writes a temporary buffer that is passed to the IDF when calling WiFi.mode.
    // As we have the same hostname for STA and AP, it is sufficient to set the hostname here once and never call WiFi.softAPsetHostname.
#if MODULE_NETWORK_AVAILABLE()
    WiFi.setHostname(network.config.get("hostname")->asEphemeralCStr());
#else
    WiFi.setHostname((String(BUILD_HOST_PREFIX) + "-" + local_uid_str).c_str());
#endif

    if (enable_sta && enable_ap) {
        WiFi.mode(WIFI_AP_STA);
    } else if (enable_ap) {
        WiFi.mode(WIFI_AP);
    } else {
        WiFi.mode(WIFI_STA);
    }

    WiFi.setAutoReconnect(false);
    WiFi.disconnect(false, true);

    wifi_country_t config;
    config.cc[0] = 'D';
    config.cc[1] = 'E';
    config.cc[2] = ' ';
    config.schan = 1;
    config.nchan = 13;
    config.policy = WIFI_COUNTRY_POLICY_AUTO;
    esp_wifi_set_country(&config);

    esp_wifi_set_ps(WIFI_PS_NONE);

    // We don't need the additional speed of HT40 and it only causes more errors.
    // Always disable on both interfaces but only print warnings for interfaces we care about.
    esp_err_t err;
    err = esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT20);
    if (enable_sta && err != ESP_OK)
        logger.printfln("Setting HT20 for station failed: %i", err);

    err = esp_wifi_set_bandwidth(WIFI_IF_AP, WIFI_BW_HT20);
    if (enable_ap && err != ESP_OK)
        logger.printfln("Setting HT20 for AP failed: %i", err);

    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    state.get("ap_bssid")->updateString(WiFi.softAPmacAddress());

    if (enable_ap && !ap_fallback_only) {
        apply_soft_ap_config_and_start();
    } else {
        LogSilencer ls;
        WiFi.softAPdisconnect(true);
    }

    if (enable_ap) {
        task_scheduler.scheduleWithFixedDelay([this]() {
            state.get("ap_state")->updateInt(get_ap_state());
        }, 5000, 5000);
    }

    if (enable_sta) {
        task_scheduler.scheduleWithFixedDelay([this]() {
            WifiState connection_state = get_connection_state();
            state.get("connection_state")->updateEnum(connection_state);

            int rssi = -127;
            esp_wifi_sta_get_rssi(&rssi); // Ignore failure, rssi is still -127.
            state.get("sta_rssi")->updateInt(rssi);

            static int tries = 0;
            if (tries < 3 || tries % 3 == 2)
                if (!apply_sta_config_and_connect(connection_state))
                    tries = 0;
            tries++;
        }, 0, 10000);
    }

    if (ap_fallback_only) {
        task_scheduler.scheduleWithFixedDelay([this]() {
            bool connected = state.get("connection_state")->asEnum<WifiState>() == WifiState::Connected;

#if MODULE_ETHERNET_AVAILABLE()
            connected = connected || ethernet.get_connection_state() == EthernetState::Connected;
#endif

            static int stop_soft_ap_runs = 0;

            if (!connected)
                stop_soft_ap_runs = 0;

            // Start softAP immediately, but stop it only if
            // we got a connection in the first 30 seconds after start-up
            // or we had a connection for 5 minutes.

            if (connected && soft_ap_running) {
                ++stop_soft_ap_runs;
                if (now_us() < 30_s || stop_soft_ap_runs > 5 * 6) {
                    logger.printfln("Network connected. Stopping soft AP");
                    WiFi.softAPdisconnect(true);
                    soft_ap_running = false;
                }
            } else if (!connected && !soft_ap_running) {
                apply_soft_ap_config_and_start();
            }
        },
        enable_sta
#if MODULE_ETHERNET_AVAILABLE()
        || (ethernet.is_enabled() && ethernet.get_connection_state() != EthernetState::NotConnected)
#endif
        ? 30 * 1000 : 1000, 10 * 1000);
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

    initialized = true;
}

// FIXME: use TFJson on top of StringBuilder
void Wifi::get_scan_results(StringBuilder *sb, int network_count)
{
    sb->putc('[');

    for (int i = 0; i < network_count; ++i) {
        if (i > 0) {
            sb->putc(',');
        }

        sb->printf("{\"ssid\":\"%s\",\"bssid\":\"%s\",\"rssi\":%d,\"channel\":%d,\"encryption\":%d}",
                   WiFi.SSID(i).c_str(), WiFi.BSSIDstr(i).c_str(), WiFi.RSSI(i), WiFi.channel(i), WiFi.encryptionType(i));
    }

    sb->putc(']');
}

void Wifi::check_for_scan_completion()
{
    int network_count = WiFi.scanComplete();

    if (network_count == WIFI_SCAN_RUNNING) {
        logger.printfln("Scan in progress...");
        task_scheduler.scheduleOnce([this]() {
            this->check_for_scan_completion();
        }, 500);
        return;
    }

    if (network_count < 0) {
        logger.printfln("Scan failed.");
        return;
    }

    logger.printfln("Scan done. %d networks.", network_count);

#if MODULE_WS_AVAILABLE()
    StringBuilder sb;

    if (ws.pushRawStateUpdateBegin(&sb, MAX_SCAN_RESULT_LENGTH * network_count + 2, "wifi/scan_results")) {
        get_scan_results(&sb, network_count);
        ws.pushRawStateUpdateEnd(&sb);
    }
#endif
}

void Wifi::start_scan()
{
    // Abort if a scan is running. This is safe, because
    // the state will change to SCAN_FAILED if it timed out.
    if (WiFi.scanComplete() == WIFI_SCAN_RUNNING)
        return;

    logger.printfln("Scanning for wifis...");
    WiFi.scanDelete();

    // WIFI_SCAN_FAILED also means the scan is done.
    if (WiFi.scanComplete() != WIFI_SCAN_FAILED)
        return;

    if (WiFi.scanNetworks(true, true) != WIFI_SCAN_FAILED) {
        task_scheduler.scheduleOnce([this]() {
            this->check_for_scan_completion();
        }, 2000);
        return;
    }

    logger.printfln("Starting WiFi scan failed. Maybe a connection attempt is running concurrently. Retrying once in 6 seconds.");
    task_scheduler.scheduleOnce([this](){
        if (WiFi.scanNetworks(true, true) == WIFI_SCAN_FAILED) {
            logger.printfln("Second scan attempt failed. Giving up.");
            return;
        }

        task_scheduler.scheduleOnce([this]() {
            this->check_for_scan_completion();
        }, 2000);
    }, 6000);
}

void Wifi::register_urls()
{
    api.addState("wifi/state", &state);

    api.addCommand("wifi/scan", Config::Null(), {}, [this](){
        start_scan();
    }, true);

    server.on("/wifi/scan_results", HTTP_GET, [this](WebServerRequest request) {
        int network_count = WiFi.scanComplete();

        if (network_count == WIFI_SCAN_RUNNING) {
            return request.send(200, "text/plain; charset=utf-8", "scan in progress");
        }

        if (network_count < 0) {
            return request.send(200, "text/plain; charset=utf-8", "scan failed");
        }

        StringBuilder sb;

        if (!sb.setCapacity(MAX_SCAN_RESULT_LENGTH * network_count + 2)) {
            return request.send(200, "text/plain; charset=utf-8", "scan out of memory");
        }

        get_scan_results(&sb, network_count);

        return request.send(200, "application/json; charset=utf-8", sb.getPtr());
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
        case WL_CONNECTED:
            return WifiState::Connected;
        case WL_NO_SHIELD:
            return WifiState::NotConfigured;
        case WL_IDLE_STATUS:
            return WifiState::Connecting;
        default:
            // this will only be reached with WL_SCAN_COMPLETED, but this value is never set
            return WifiState::Connected;
    }
}

bool Wifi::is_sta_enabled() const
{
    return sta_config_in_use.get("enable_sta")->asBool();
}

int Wifi::get_ap_state()
{
    bool enable_ap = ap_config_in_use.get("enable_ap")->asBool();
    if (!enable_ap)
        return 0;

    bool ap_fallback = ap_config_in_use.get("ap_fallback_only")->asBool();
    if (!ap_fallback)
        return 1;

    if (!soft_ap_running)
        return 2;

    return 3;
}

int Wifi::get_sta_rssi() const
{
    return state.get("sta_rssi")->asInt();
}

const char* Wifi::get_ap_ssid() const
{
    return ap_config_in_use.get("ssid")->asEphemeralCStr();
}

const char* Wifi::get_ap_ip() const
{
    return ap_config_in_use.get("ip")->asEphemeralCStr();
}

const char* Wifi::get_ap_passphrase() const
{
    return ap_config_in_use.get("passphrase")->asEphemeralCStr();
}