/* esp32-brick
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

#include <ESPmDNS.h>

#include <esp_wifi.h>

#include "task_scheduler.h"
#include "tools.h"
#include "api.h"
#include "event_log.h"
#include "web_server.h"
#include "WiFi.h"

#include "modules.h"

extern EventLog logger;

extern TaskScheduler task_scheduler;
extern WebServer server;
extern char uid[7];
extern char passphrase[20];

extern API api;

Wifi::Wifi() {
    wifi_ap_config = Config::Object({
        {"enable_ap", Config::Bool(true)},
        {"ap_fallback_only", Config::Bool(false)},
        {"ssid",  Config::Str("esp-brick", 32)},
        {"hide_ssid", Config::Bool(false)},
        {"passphrase", Config::Str("0123456789", 64, [](Config::ConfString &s) {
                return (s.value.length() >= 8 && s.value.length() <= 63) || //FIXME: check if there are only ASCII characters here.
                    (s.value.length() == 64) ? String("") : String("passphrase must be of length 8 to 63, or 64 if PSK."); //FIXME: check if there are only hex digits here.
            })
        },
        {"hostname", Config::Str("esp-brick", 32)},
        {"channel", Config::Uint(1, 1, 13)},
        {"ip", Config::Array({
                Config::Uint8(10),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(1),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"gateway", Config::Array({
                Config::Uint8(10),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(1),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"subnet", Config::Array({
                Config::Uint8(255),
                Config::Uint8(255),
                Config::Uint8(255),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
    });
    wifi_sta_config = Config::Object({
        {"enable_sta", Config::Bool(false)},
        {"ssid", Config::Str("", 32)},
        {"bssid", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0)
                },
                Config::Uint8(0),
                6,
                6,
                Config::type_id<Config::ConfUint>()
            )
        },
        {"bssid_lock", Config::Bool(false)},
        {"passphrase", Config::Str("", 64, [](Config::ConfString &s) {
                return s.value.length() == 0 ||
                    (s.value.length() >= 8 && s.value.length() <= 63) || //FIXME: check if there are only ASCII characters here.
                    (s.value.length() == 64) ? String("") : String("passphrase must be of length zero, or 8 to 63, or 64 if PSK."); //FIXME: check if there are only hex digits here.
            })
        },
        {"hostname", Config::Str("wallbox", 32)},
        {"ip", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"gateway", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"subnet", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"dns", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"dns2", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
    });

    wifi_state = Config::Object({
        {"connection_state", Config::Int(0)},
        {"ap_state", Config::Int(0)},
        {"ap_bssid", Config::Str("", 20)},
        {"sta_ip", Config::Array({
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                Config::Uint8(0),
                },
                Config::Uint8(0),
                4,
                4,
                Config::type_id<Config::ConfUint>()
            )},
        {"sta_rssi", Config::Int8(0)},
        {"sta_bssid", Config::Str("", 20)}
    });

    wifi_scan_config = Config::Null();
}

void Wifi::apply_soft_ap_config_and_start() {
    uint8_t ip[4];
    uint8_t gateway[4];
    uint8_t subnet[4];

    wifi_ap_config_in_use.get("ip")->fillArray<uint8_t, Config::ConfUint>(ip, 4);
    wifi_ap_config_in_use.get("gateway")->fillArray<uint8_t, Config::ConfUint>(gateway, 4);
    wifi_ap_config_in_use.get("subnet")->fillArray<uint8_t, Config::ConfUint>(subnet, 4);

    int counter = 0;
    while((ip[0] != WiFi.softAPIP()[0]) || (ip[1] != WiFi.softAPIP()[1]) || (ip[2] != WiFi.softAPIP()[2]) || (ip[3] != WiFi.softAPIP()[3])) {
        WiFi.softAPConfig(ip, gateway, subnet);
        ++counter;
    }
    logger.printfln("Had to configure softAP ip %d times.", counter);
    delay(2000);

    logger.printfln("Soft AP started.");
    logger.printfln("    SSID: %s", wifi_ap_config_in_use.get("ssid")->asString().c_str());
    logger.printfln("    hostname: %s", wifi_ap_config_in_use.get("hostname")->asString().c_str());

    WiFi.softAPsetHostname(wifi_ap_config_in_use.get("hostname")->asString().c_str());

    WiFi.softAP(wifi_ap_config_in_use.get("ssid")->asString().c_str(),
                wifi_ap_config_in_use.get("passphrase")->asString().c_str(),
                wifi_ap_config_in_use.get("channel")->asUint(),
                wifi_ap_config_in_use.get("hide_ssid")->asBool());
    WiFi.setSleep(false);

    soft_ap_running = true;
    IPAddress myIP = WiFi.softAPIP();
    logger.printfln("    IP: %u.%u.%u.%u", myIP[0], myIP[1], myIP[2], myIP[3]);
}

void Wifi::apply_sta_config_and_connect() {
    if (get_connection_state() == WifiState::CONNECTED) {
        return;
    }

    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect(false, true);

    String ssid = wifi_sta_config_in_use.get("ssid")->asString();

    uint8_t bssid[6];
    wifi_sta_config_in_use.get("bssid")->fillArray<uint8_t, Config::ConfUint>(bssid, 6 * sizeof(bssid));

    String passphrase = wifi_sta_config_in_use.get("passphrase")->asString();
    bool bssid_lock = wifi_sta_config_in_use.get("bssid_lock")->asBool();

    uint8_t ip[4], subnet[4], gateway[4], dns[4], dns2[4];
    wifi_sta_config_in_use.get("ip")->fillUint8Array(ip, 4);
    wifi_sta_config_in_use.get("subnet")->fillUint8Array(subnet, 4);
    wifi_sta_config_in_use.get("gateway")->fillUint8Array(gateway, 4);
    wifi_sta_config_in_use.get("dns")->fillUint8Array(dns, 4);
    wifi_sta_config_in_use.get("dns2")->fillUint8Array(dns2, 4);


    WiFi.begin(ssid.c_str(), passphrase.c_str(), 0, bssid_lock ? bssid : nullptr, false);

    if(ip != 0) {
        WiFi.config(ip, gateway, subnet, dns, dns2);
    } else {
        WiFi.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
    }

    WiFi.setHostname(wifi_sta_config_in_use.get("hostname")->asString().c_str());

    logger.printfln("Connecting to %s", wifi_sta_config_in_use.get("ssid")->asString().c_str());

    WiFi.begin(ssid.c_str(), passphrase.c_str(), 0, bssid_lock ? bssid : nullptr, true);
    WiFi.setSleep(false);
}

const char *reason2str(uint8_t reason) {
    switch(reason) {
        case WIFI_REASON_NO_AP_FOUND:
            return "Access Point not found. Is the reception too poor or the SSID incorrect?";
        case WIFI_REASON_AUTH_FAIL:
            return "Authentication failed. Is the passphrase correct?";
        case WIFI_REASON_ASSOC_FAIL:
            return "Assiociation failed.";
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
        case WIFI_REASON_BEACON_TIMEOUT:
            return "Reception too poor";
        default:
            return "Unknown reason.";
    }
}

void Wifi::setup()
{
    String default_hostname = String(__HOST_PREFIX__) + String("-") + String(uid);
    String default_passphrase = String(passphrase);

    if(!api.restorePersistentConfig("wifi/sta_config", &wifi_sta_config)) {
        wifi_sta_config.get("hostname")->updateString(default_hostname);
    }

    if(!api.restorePersistentConfig("wifi/ap_config", &wifi_ap_config)) {
        wifi_ap_config.get("hostname")->updateString(default_hostname);
        wifi_ap_config.get("ssid")->updateString(default_hostname);
        wifi_ap_config.get("passphrase")->updateString(default_passphrase);
    }

    wifi_ap_config_in_use = wifi_ap_config;
    wifi_sta_config_in_use = wifi_sta_config;

    WiFi.persistent(false);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            uint8_t reason_code = info.wifi_sta_disconnected.reason;
            const char *reason = reason2str(reason_code);
            if(!this->was_connected) {
                logger.printfln("Failed to connect to %s: %s (%u)", wifi_sta_config_in_use.get("ssid")->asString().c_str(), reason, reason_code);
            } else {
                logger.printfln("Disconnected from %s: %s (%u)", wifi_sta_config_in_use.get("ssid")->asString().c_str(), reason, reason_code);
            }
            this->was_connected = false;
        },
        ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            this->was_connected = true;

            logger.printfln("Connected to %s", WiFi.SSID().c_str());
        },
        ARDUINO_EVENT_WIFI_STA_CONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            // Sometimes the ARDUINO_EVENT_WIFI_STA_CONNECTED is not fired.
            // Instead we get the ARDUINO_EVENT_WIFI_STA_GOT_IP twice?
            // Make sure that the state is set to connected here,
            // or else MQTT will never attempt to connect.
            this->was_connected = true;

            auto ip = WiFi.localIP();
            logger.printfln("Got IP address: %u.%u.%u.%u. Connected to BSSID %s", ip[0], ip[1], ip[2], ip[3], WiFi.BSSIDstr().c_str());
            wifi_state.get("sta_ip")->get(0)->updateUint(ip[0]);
            wifi_state.get("sta_ip")->get(1)->updateUint(ip[1]);
            wifi_state.get("sta_ip")->get(2)->updateUint(ip[2]);
            wifi_state.get("sta_ip")->get(3)->updateUint(ip[3]);
            wifi_state.get("sta_bssid")->updateString(WiFi.BSSIDstr());

            api.wifiAvailable();
        },
        ARDUINO_EVENT_WIFI_STA_GOT_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
        if(!this->was_connected)
            return;

        this->was_connected = false;

        logger.printfln("Lost IP. Forcing disconnect and reconnect of WiFi");
        wifi_state.get("sta_ip")->get(0)->updateUint(0);
        wifi_state.get("sta_ip")->get(1)->updateUint(0);
        wifi_state.get("sta_ip")->get(2)->updateUint(0);
        wifi_state.get("sta_ip")->get(3)->updateUint(0);
        wifi_state.get("sta_bssid")->updateString("");

        WiFi.disconnect(false, true);
    }, ARDUINO_EVENT_WIFI_STA_LOST_IP);

    bool enable_ap = wifi_ap_config_in_use.get("enable_ap")->asBool();
    bool enable_sta = wifi_sta_config_in_use.get("enable_sta")->asBool();
    bool ap_fallback_only = wifi_ap_config_in_use.get("ap_fallback_only")->asBool();

    if (enable_sta && enable_ap) {
        WiFi.mode(WIFI_AP_STA);
    } else if (enable_ap) {
        WiFi.mode(WIFI_AP);
    } else if (enable_sta) {
        WiFi.mode(WIFI_STA);
    } else {
        WiFi.mode(WIFI_OFF);
    }

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
        task_scheduler.scheduleWithFixedDelay("wifi_connect", [this](){
            apply_sta_config_and_connect();
        }, 0, 9000);
    }

    /*use mdns for host name resolution*/
    if (!MDNS.begin(wifi_ap_config_in_use.get("hostname")->asString().c_str())) {
        logger.printfln("Error setting up mDNS responder!");
    } else {
        logger.printfln("mDNS responder started");
    }

    task_scheduler.scheduleWithFixedDelay("wifi_rssi", [this](){
        wifi_state.get("sta_rssi")->updateInt(WiFi.RSSI());
    }, 5000, 5000);

    initialized = true;
}

String Wifi::get_scan_results() {
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

    //result line: {"ssid": "%s", "bssid": "%s", "rssi": %d, "channel": %d, "encryption": %d}
    //worst case length ~ 140
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
        if(i != network_count - 1)
            result += ",";
    }
    result += "]";
    return result;
}

void Wifi::check_for_scan_completion() {
    String result = this->get_scan_results();

    if (result == "scan in progress") {
        logger.printfln("Scan in progress...");
        task_scheduler.scheduleOnce("wifi_scan_check_complete", [this]() {
            this->check_for_scan_completion();
        }, 500);
        return;
    }
    logger.printfln("Scan done. %d networks.", WiFi.scanComplete());

#ifdef MODULE_WS_AVAILABLE
    ws.pushStateUpdate(this->get_scan_results(), "wifi/scan_results");
#endif
}

void Wifi::register_urls()
{
    api.addState("wifi/state", &wifi_state, {}, 1000);

    api.addCommand("wifi/scan", &wifi_scan_config, {}, [this](){
        // Abort if a scan is running. This is save, because
        // the state will change to SCAN_FAILED if it timed out.
        if (WiFi.scanComplete() == WIFI_SCAN_RUNNING)
            return;

        logger.printfln("Scanning for wifis...");
        WiFi.scanDelete();

        // WIFI_SCAN_FAILED also means the scan is done.
        if(WiFi.scanComplete() == WIFI_SCAN_FAILED){
            WiFi.scanNetworks(true, true);
        }

        task_scheduler.scheduleOnce("wifi_scan_check_complete", [this]() {
            this->check_for_scan_completion();
        }, 500);
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
#ifdef MODULE_ETHERNET_AVAILABLE
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

WifiState Wifi::get_connection_state() {
    if (!wifi_sta_config_in_use.get("enable_sta")->asBool())
        return WifiState::NOT_CONFIGURED;

    switch(WiFi.status()) {
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

int Wifi::get_ap_state() {
    bool enable_ap = wifi_ap_config_in_use.get("enable_ap")->asBool();
    bool ap_fallback = wifi_ap_config_in_use.get("ap_fallback_only")->asBool();
    if(!enable_ap)
        return 0;
    if(!ap_fallback)
        return 1;
    if(!soft_ap_running)
        return 2;

    return 3;
}
