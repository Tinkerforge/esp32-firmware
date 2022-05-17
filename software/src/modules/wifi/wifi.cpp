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

#include "task_scheduler.h"
#include "tools.h"
#include "api.h"
#include "event_log.h"
#include "web_server.h"

#include "modules.h"
#include "build.h"

extern EventLog logger;

extern TaskScheduler task_scheduler;
extern WebServer server;
extern char local_uid_str[7];
extern char passphrase[20];

extern API api;

Wifi::Wifi()
{
    wifi_ap_config = ConfigRoot(Config::Object({
        {"enable_ap", Config::Bool(true)},
        {"ap_fallback_only", Config::Bool(false)},
        {"ssid", Config::Str("", 0, 32)},
        {"hide_ssid", Config::Bool(false)},
        {"passphrase", Config::Str("this-will-be-replaced-in-setup", 8, 64)},//FIXME: check if there are only ASCII characters or hex digits (for PSK) here.
        {"channel", Config::Uint(0, 0, 13)},
        {"ip", Config::Str("10.0.0.1", 7, 15)},
        {"gateway", Config::Str("10.0.0.1", 7, 15)},
        {"subnet", Config::Str("255.255.255.0", 7, 15)}
    }), [](Config &cfg) -> String {
        const char *ip = cfg.get("ip")->asCStr();
        const char *gateway = cfg.get("gateway")->asCStr();
        const char *subnet = cfg.get("subnet")->asCStr();

        IPAddress unused;
        if (!unused.fromString(ip))
            return "Failed to parse \"ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(gateway))
            return "Failed to parse \"gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(subnet))
            return "Failed to parse \"subnet\": Expected format is dotted decimal, i.e. 10.0.0.1";

        return "";
    });

    wifi_sta_config = ConfigRoot(Config::Object({
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
        {"passphrase", Config::Str("", 8, 64)},
        {"ip", Config::Str("0.0.0.0", 7, 15)},
        {"gateway", Config::Str("0.0.0.0", 7, 15)},
        {"subnet", Config::Str("0.0.0.0", 7, 15)},
        {"dns", Config::Str("0.0.0.0", 7, 15)},
        {"dns2", Config::Str("0.0.0.0", 7, 15)},
    }), [](Config &cfg) -> String {
        const String &value = cfg.get("passphrase")->asString();
        if (value.length() > 0 && value.length() < 8)
            return "Passphrase too short. Must be at least 8 characters, or zero if open network.";
        // Fixme: Check if only hex if exactly 64 bytes long: then it's a PSK instead of a passphrase.

        const char *ip = cfg.get("ip")->asCStr();
        const char *gateway = cfg.get("gateway")->asCStr();
        const char *subnet = cfg.get("subnet")->asCStr();
        const char *dns = cfg.get("dns")->asCStr();
        const char *dns2 = cfg.get("dns2")->asCStr();

        IPAddress unused;
        if (!unused.fromString(ip))
            return "Failed to parse \"ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(gateway))
            return "Failed to parse \"gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(subnet))
            return "Failed to parse \"subnet\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(dns))
            return "Failed to parse \"dns\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(dns2))
            return "Failed to parse \"dns2\": Expected format is dotted decimal, i.e. 10.0.0.1";

        return "";
    });

    wifi_state = Config::Object({
        {"connection_state", Config::Int(0)},
        {"ap_state", Config::Int(0)},
        {"ap_bssid", Config::Str("", 0, 20)},
        {"sta_ip", Config::Str("0.0.0.0", 7, 15)},
        {"sta_rssi", Config::Int8(0)},
        {"sta_bssid", Config::Str("", 0, 20)}
    });

    wifi_scan_config = Config::Null();
}

float rssi_to_weight(int rssi) {
    return pow(2, (float)(128 + rssi) / 10);
}

void apply_weight(float *channels, int channel, float weight) {
    for(int i = MAX(1, channel - 2); i <= MIN(13, channel + 2); ++i) {
        if (i == channel - 2 || i == channel + 2)
            channels[i] += weight / 2;
        else
            channels[i] += weight;
    }
}

void Wifi::apply_soft_ap_config_and_start()
{
    static uint32_t scan_start_time = 0;
    static int channel_to_use = wifi_ap_config_in_use.get("channel")->asUint();

    if (channel_to_use == 0 && scan_start_time == 0) {
        logger.printfln("Starting scan to select unoccupied channel for soft AP.");
        WiFi.scanDelete();
        WiFi.scanNetworks(true, true);
        scan_start_time = millis();
        task_scheduler.scheduleOnce([this](){
            this->apply_soft_ap_config_and_start();
        }, 500);
        return;
    }

    if (channel_to_use == 0 && scan_start_time != 0 && !deadline_elapsed(scan_start_time + 6000)) {
        int network_count = WiFi.scanComplete();

        if (network_count == WIFI_SCAN_RUNNING) {
            task_scheduler.scheduleOnce([this](){
                this->apply_soft_ap_config_and_start();
            }, 500);
            return;
        }

        if (network_count == WIFI_SCAN_FAILED) {
            task_scheduler.scheduleOnce([this](){
                this->apply_soft_ap_config_and_start();
            }, 500);
        }

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

        memcpy(channels_smeared, channels, sizeof(channels_smeared) / sizeof(channels_smeared[0]));

        for(int i = 1; i <= 13; ++i) {
            if (i > 1)
                channels_smeared[i] += channels[i-1];
            if (i < 13)
                channels_smeared[i] += channels[i+1];
        }

        int min = 1;
        for(int i = 1; i <= 13; ++i) {
            if (channels_smeared[i] < channels_smeared[min])
                min = i;
        }
        logger.printfln("Selecting channel %d for softAP", min);
        channel_to_use = min;
    }
    if (channel_to_use == 0 && scan_start_time != 0 && deadline_elapsed(scan_start_time + 7000)) {
        channel_to_use = (esp_random() % 4) * 4 + 1;
        logger.printfln("Channel selection scan timeout elapsed! Randomly selected channel %u", channel_to_use);
    }


    IPAddress ip, gateway, subnet;
    ip.fromString(wifi_ap_config_in_use.get("ip")->asCStr());
    gateway.fromString(wifi_ap_config_in_use.get("gateway")->asCStr());
    subnet.fromString(wifi_ap_config_in_use.get("subnet")->asCStr());

    int counter = 0;
    while (ip != WiFi.softAPIP()) {
        WiFi.softAPConfig(ip, gateway, subnet);
        ++counter;
    }
    logger.printfln("Had to configure soft AP IP address %d times.", counter);
    logger.printfln("Wifi soft AP started");
    logger.printfln("    SSID: %s", wifi_ap_config_in_use.get("ssid")->asString().c_str());

    WiFi.softAP(wifi_ap_config_in_use.get("ssid")->asString().c_str(),
                wifi_ap_config_in_use.get("passphrase")->asString().c_str(),
                channel_to_use,
                wifi_ap_config_in_use.get("hide_ssid")->asBool());
    WiFi.setSleep(false);

    soft_ap_running = true;
    IPAddress myIP = WiFi.softAPIP();
    logger.printfln("    MAC address: %s", WiFi.softAPmacAddress().c_str());
    logger.printfln("    IP address: %s", myIP.toString().c_str());
}

bool Wifi::apply_sta_config_and_connect()
{
    if (get_connection_state() == WifiState::CONNECTED) {
        return false;
    }

    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(false, true);

    String ssid = wifi_sta_config_in_use.get("ssid")->asString();

    uint8_t bssid[6];
    wifi_sta_config_in_use.get("bssid")->fillArray<uint8_t, Config::ConfUint>(bssid, 6 * sizeof(bssid));

    String passphrase = wifi_sta_config_in_use.get("passphrase")->asString();
    bool bssid_lock = wifi_sta_config_in_use.get("bssid_lock")->asBool();

    IPAddress ip, subnet, gateway, dns, dns2;

    ip.fromString(wifi_sta_config_in_use.get("ip")->asCStr());
    subnet.fromString(wifi_sta_config_in_use.get("subnet")->asCStr());
    gateway.fromString(wifi_sta_config_in_use.get("gateway")->asCStr());
    dns.fromString(wifi_sta_config_in_use.get("dns")->asCStr());
    dns2.fromString(wifi_sta_config_in_use.get("dns2")->asCStr());

    WiFi.begin(ssid.c_str(), passphrase.c_str(), 0, bssid_lock ? bssid : nullptr, false);

    if (ip != 0) {
        WiFi.config(ip, gateway, subnet, dns, dns2);
    } else {
        WiFi.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
    }

    logger.printfln("Wifi connecting to %s", wifi_sta_config_in_use.get("ssid")->asString().c_str());

    WiFi.begin(ssid.c_str(), passphrase.c_str(), 0, bssid_lock ? bssid : nullptr, true);
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
    String default_ssid = String(BUILD_HOST_PREFIX) + String("-") + String(local_uid_str);
    String default_passphrase = String(passphrase);

    api.restorePersistentConfig("wifi/sta_config", &wifi_sta_config);

    if (!api.restorePersistentConfig("wifi/ap_config", &wifi_ap_config)) {
        wifi_ap_config.get("ssid")->updateString(default_ssid);
        wifi_ap_config.get("passphrase")->updateString(default_passphrase);
    }

    wifi_ap_config_in_use = wifi_ap_config;
    wifi_sta_config_in_use = wifi_sta_config;

    WiFi.persistent(false);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            uint8_t reason_code = info.wifi_sta_disconnected.reason;
            const char *reason = reason2str(reason_code);
            if (!this->was_connected) {
                logger.printfln("Wifi failed to connect to %s: %s (%u)", wifi_sta_config_in_use.get("ssid")->asString().c_str(), reason, reason_code);
            } else {
                uint32_t now = millis();
                uint32_t connected_for = now - last_connected_ms;

                // FIXME: Use a better way of time keeping here.
                if (connected_for < 0x7FFFFFFF)
                    logger.printfln("Wifi disconnected from %s: %s (%u). Was connected for %d seconds.", wifi_sta_config_in_use.get("ssid")->asString().c_str(), reason, reason_code, connected_for / 1000);
                else
                    logger.printfln("Wifi disconnected from %s: %s (%u). Was connected for a long time.", wifi_sta_config_in_use.get("ssid")->asString().c_str(), reason, reason_code);

            }
            this->was_connected = false;

            wifi_state.get("sta_ip")->updateString("0.0.0.0");
            wifi_state.get("sta_bssid")->updateString("");
        },
        ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            this->was_connected = true;

            logger.printfln("Wifi connected to %s", WiFi.SSID().c_str());
            last_connected_ms = millis();
        },
        ARDUINO_EVENT_WIFI_STA_CONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            // Sometimes the ARDUINO_EVENT_WIFI_STA_CONNECTED is not fired.
            // Instead we get the ARDUINO_EVENT_WIFI_STA_GOT_IP twice?
            // Make sure that the state is set to connected here,
            // or else MQTT will never attempt to connect.
            this->was_connected = true;

            auto ip = WiFi.localIP().toString();
            logger.printfln("Wifi MAC address: %s", WiFi.macAddress().c_str());
            logger.printfln("Wifi got IP address: %s. Connected to BSSID %s", ip.c_str(), WiFi.BSSIDstr().c_str());
            wifi_state.get("sta_ip")->updateString(ip);
            wifi_state.get("sta_bssid")->updateString(WiFi.BSSIDstr());

            api.wifiAvailable();
        },
        ARDUINO_EVENT_WIFI_STA_GOT_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Wifi got IPv6 address: %s.", WiFi.localIPv6().toString().c_str());
        },
        ARDUINO_EVENT_WIFI_STA_GOT_IP6);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
        if(!this->was_connected)
            return;

        this->was_connected = false;

        logger.printfln("Wifi lost IP. Forcing disconnect and reconnect of WiFi");
        wifi_state.get("sta_ip")->updateString("0.0.0.0");
        wifi_state.get("sta_bssid")->updateString("");

        WiFi.disconnect(false, true);
    }, ARDUINO_EVENT_WIFI_STA_LOST_IP);

    bool enable_ap = wifi_ap_config_in_use.get("enable_ap")->asBool();
    bool enable_sta = wifi_sta_config_in_use.get("enable_sta")->asBool();
    bool ap_fallback_only = wifi_ap_config_in_use.get("ap_fallback_only")->asBool();

    // For some reason WiFi.setHostname only writes a temporary buffer that is passed to the IDF when calling WiFi.mode.
    // As we have the same hostname for STA and AP, it is sufficient to set the hostname here once and never call WiFi.softAPsetHostname.
    WiFi.setHostname(network.config.get("hostname")->asCStr());

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

    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    wifi_state.get("ap_bssid")->updateString(WiFi.softAPmacAddress());

    if (enable_ap && !ap_fallback_only) {
        apply_soft_ap_config_and_start();
    } else {
        WiFi.softAPdisconnect(true);
    }

    if (enable_sta) {
        task_scheduler.scheduleWithFixedDelay([this](){
            static int backoff = 1;
            static int backoff_counter = 0;

            if (backoff_counter > 0) {
                --backoff_counter;
                return;
            }

            if (!apply_sta_config_and_connect()) {
                // We are already connected. Reset exponential backoff
                backoff = 1;
                backoff_counter = 0;
            } else {
                if (backoff <= 32)
                    backoff *= 2;
                backoff_counter = backoff;
            }
        }, 0, 5000);
    }

    task_scheduler.scheduleWithFixedDelay([this](){
        wifi_state.get("sta_rssi")->updateInt(WiFi.RSSI());
    }, 5000, 5000);

    initialized = true;
}

String Wifi::get_scan_results()
{
    int network_count = WiFi.scanComplete();

    if (network_count == WIFI_SCAN_RUNNING) {
        return "scan in progress";
    }

    if (network_count < 0) {
        return "scan failed";
    }

    if (network_count == 0) {
        return "[]";
    }

    // result line: {"ssid": "%s", "bssid": "%s", "rssi": %d, "channel": %d, "encryption": %d}
    // worst case length ~ 140
    String result;
    result.reserve(145 * network_count);
    logger.printfln("%d networks found", network_count);
    result += "[";

    for (int i = 0; i < network_count; ++i) {
        // Print SSID and RSSI for each network found
        result += "{\"ssid\": \"";
        result += WiFi.SSID(i);
        result += "\", \"bssid\": \"";
        result += WiFi.BSSIDstr(i);
        result += "\", \"rssi\": ";
        result += WiFi.RSSI(i);
        result += ", \"channel\": ";
        result += WiFi.channel(i);
        result += ", \"encryption\": ";
        result += WiFi.encryptionType(i);
        result += "}";
        if (i != network_count - 1)
            result += ",";
    }
    result += "]";
    return result;
}

void Wifi::check_for_scan_completion()
{
    String result = this->get_scan_results();

    if (result == "scan in progress") {
        logger.printfln("Scan in progress...");
        task_scheduler.scheduleOnce([this]() {
            this->check_for_scan_completion();
        }, 500);
        return;
    }

    if (result == "scan failed") {
        logger.printfln("Scan failed.");
        return;
    }

    logger.printfln("Scan done. %d networks.", WiFi.scanComplete());

#if MODULE_WS_AVAILABLE()
    ws.pushRawStateUpdate(this->get_scan_results(), "wifi/scan_results");
#endif
}

void Wifi::start_scan() {
    // Abort if a scan is running. This is save, because
    // the state will change to SCAN_FAILED if it timed out.
    if (WiFi.scanComplete() == WIFI_SCAN_RUNNING)
        return;

    logger.printfln("Scanning for wifis...");
    WiFi.scanDelete();

    // WIFI_SCAN_FAILED also means the scan is done.
    if(WiFi.scanComplete() != WIFI_SCAN_FAILED)
        return;

    if (WiFi.scanNetworks(true, true) != WIFI_SCAN_FAILED) {
        task_scheduler.scheduleOnce([this]() {
            this->check_for_scan_completion();
        }, 500);
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
        }, 500);
    }, 6000);
}

void Wifi::register_urls()
{
    api.addState("wifi/state", &wifi_state, {}, 1000);

    api.addCommand("wifi/scan", &wifi_scan_config, {}, [this](){
        start_scan();
    }, true);

    server.on("/wifi/scan_results", HTTP_GET, [this](WebServerRequest request) {
        int network_count = WiFi.scanComplete();
        String result = this->get_scan_results();

        if (network_count < 0) {
            request.send(200, "text/plain; charset=utf-8", result.c_str());
        }

        logger.printfln("scan done");
        request.send(200, "application/json; charset=utf-8", result.c_str());
    });

    api.addPersistentConfig("wifi/sta_config", &wifi_sta_config, {"passphrase"}, 1000);
    api.addPersistentConfig("wifi/ap_config", &wifi_ap_config, {"passphrase"}, 1000);
}

void Wifi::loop()
{
    auto connection_state = get_connection_state();
    wifi_state.get("connection_state")->updateInt((int)connection_state);
    wifi_state.get("ap_state")->updateInt(get_ap_state());

    bool ap_fallback_only = wifi_ap_config_in_use.get("enable_ap")->asBool() && wifi_ap_config_in_use.get("ap_fallback_only")->asBool();
    bool ethernet_connected = false;
#if MODULE_ETHERNET_AVAILABLE()
    ethernet_connected = ethernet.get_connection_state() == EthernetState::CONNECTED;
#endif
    bool connected = (wifi_sta_config_in_use.get("enable_sta")->asBool() && connection_state == WifiState::CONNECTED) || ethernet_connected;

    if (!connected && ap_fallback_only && !soft_ap_running) {
        apply_soft_ap_config_and_start();
    }

    if (connected && ap_fallback_only && soft_ap_running) {
        logger.printfln("Network connected. Stopping soft AP");
        WiFi.softAPdisconnect(true);
        soft_ap_running = false;
    }
}

WifiState Wifi::get_connection_state()
{
    if (!wifi_sta_config_in_use.get("enable_sta")->asBool())
        return WifiState::NOT_CONFIGURED;

    switch (WiFi.status()) {
        case WL_CONNECT_FAILED:
        case WL_CONNECTION_LOST:
        case WL_DISCONNECTED:
        case WL_NO_SSID_AVAIL:
            return WifiState::NOT_CONNECTED;
        case WL_CONNECTED:
            return WifiState::CONNECTED;
        case WL_NO_SHIELD:
            return WifiState::NOT_CONFIGURED;
        case WL_IDLE_STATUS:
            return WifiState::CONNECTING;
        default:
            // this will only be reached with WL_SCAN_COMPLETED, but this value is never set
            return WifiState::CONNECTED;
    }
}

int Wifi::get_ap_state()
{
    bool enable_ap = wifi_ap_config_in_use.get("enable_ap")->asBool();
    bool ap_fallback = wifi_ap_config_in_use.get("ap_fallback_only")->asBool();
    if (!enable_ap)
        return 0;
    if (!ap_fallback)
        return 1;
    if (!soft_ap_running)
        return 2;

    return 3;
}
