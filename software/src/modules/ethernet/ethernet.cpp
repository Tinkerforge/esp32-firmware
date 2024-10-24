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

#include "ethernet.h"

#define ETH_ADDR        0
#define ETH_POWER_PIN   5
#define ETH_TYPE        ETH_PHY_KSZ8081

#include <ETH.h>
#include <esp_eth.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools.h"

extern char local_uid_str[32];

void Ethernet::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable_ethernet", Config::Bool(true)},
        {"ip", Config::Str("0.0.0.0", 7, 15)},
        {"gateway", Config::Str("0.0.0.0", 7, 15)},
        {"subnet", Config::Str("0.0.0.0", 7, 15)},
        {"dns", Config::Str("0.0.0.0", 7, 15)},
        {"dns2", Config::Str("0.0.0.0", 7, 15)},
    }),
    [](Config &cfg, ConfigSource source) -> String {
        IPAddress ip_addr, subnet_mask, gateway_addr, unused;

        if (!ip_addr.fromString(cfg.get("ip")->asEphemeralCStr()))
            return "Failed to parse \"ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!gateway_addr.fromString(cfg.get("gateway")->asEphemeralCStr()))
            return "Failed to parse \"gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!subnet_mask.fromString(cfg.get("subnet")->asEphemeralCStr()))
            return "Failed to parse \"subnet\": Expected format is dotted decimal, i.e. 255.255.255.0";

        if (!is_valid_subnet_mask(subnet_mask))
            return "Invalid subnet mask passed: Expected format is 255.255.255.0";

        if (ip_addr != IPAddress(0, 0, 0, 0) && is_in_subnet(ip_addr, subnet_mask, IPAddress(127, 0, 0, 1)))
            return "Invalid IP or subnet mask passed: This configuration would route localhost (127.0.0.1) to the ethernet interface.";

        if (gateway_addr != IPAddress(0, 0, 0, 0) && !is_in_subnet(ip_addr, subnet_mask, gateway_addr))
            return "Invalid IP, subnet mask, or gateway passed: IP and gateway are not in the same network according to the subnet mask.";

        if (!unused.fromString(cfg.get("dns")->asEphemeralCStr()))
            return "Failed to parse \"dns\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(cfg.get("dns2")->asEphemeralCStr()))
            return "Failed to parse \"dns2\": Expected format is dotted decimal, i.e. 10.0.0.1";

        return "";
    }};

    state = Config::Object({
        {"connection_state", Config::Enum(EthernetState::NotConfigured, EthernetState::NotConfigured, EthernetState::Connected)},
        {"connection_start", Config::Uint(0)},
        {"connection_end", Config::Uint(0)},
        {"ip", Config::Str("0.0.0.0", 7, 15)},
        {"subnet", Config::Str("0.0.0.0", 7, 15)},
        {"full_duplex", Config::Bool(false)},
        {"link_speed", Config::Uint8(0)}
    });
}

void Ethernet::print_con_duration()
{
    if (was_connected) {
        was_connected = false;
        uint32_t now = millis();
        uint32_t connected_for = now - last_connected;
        if (connected_for < 0x7FFFFFFF) {
            logger.printfln("Was connected for %u seconds.", connected_for / 1000);
        } else {
            logger.printfln("Was connected for a long time.");
        }
    }
}

void Ethernet::setup()
{
    api.addFeature("ethernet");
    api.restorePersistentConfig("ethernet/config", &config);

    config_in_use = config.get_owned_copy();

    connection_state = EthernetState::NotConfigured;
    state.get("connection_state")->updateEnum(connection_state);

    initialized = true;

    if (!config_in_use.get("enable_ethernet")->asBool())
        return;

#if MODULE_NETWORK_AVAILABLE()
    hostname = network.config.get("hostname")->asString();
#else
    hostname = String(BUILD_HOST_PREFIX) + "-" + local_uid_str;
#endif

    connection_state = EthernetState::NotConnected;
    state.get("connection_state")->updateEnum(connection_state);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Started");
            ETH.setHostname(hostname.c_str());

            connection_state = EthernetState::NotConnected;

            task_scheduler.scheduleOnce([this](){
                state.get("connection_state")->updateEnum(connection_state);
            });
        },
        ARDUINO_EVENT_ETH_START);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            uint32_t link_speed = ETH.linkSpeed();
            bool full_duplex    = ETH.fullDuplex();
            logger.printfln("Connected: %u Mbps %s Duplex, MAC: %s", link_speed, full_duplex ? "Full" : "Half", ETH.macAddress().c_str());

            IPAddress ip, subnet, gateway, dns, dns2;
            ip.fromString(config_in_use.get("ip")->asEphemeralCStr());
            subnet.fromString(config_in_use.get("subnet")->asEphemeralCStr());
            gateway.fromString(config_in_use.get("gateway")->asEphemeralCStr());
            dns.fromString(config_in_use.get("dns")->asEphemeralCStr());
            dns2.fromString(config_in_use.get("dns2")->asEphemeralCStr());

            if (link_speed < 100)
                delay(40); // 10MBit usually needs at least 19ms extra. Give it 40ms to be safe.

            if (ip != 0) {
                ETH.config(ip, gateway, subnet, dns, dns2);
            } else {
                ETH.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
            }

            connection_state = EthernetState::Connecting;

            task_scheduler.scheduleOnce([this, link_speed, full_duplex](){
                state.get("connection_state")->updateEnum(connection_state);
                state.get("link_speed" )->updateUint(link_speed);
                state.get("full_duplex")->updateBool(full_duplex);

#if MODULE_WIFI_AVAILABLE()
                if (wifi.is_sta_enabled()) {
                    logger.printfln("Warning: Ethernet is connected and WiFi station is enabled at the same time.");
                    logger.printfln_continue("     This can lead to connectivity issues and is not recommended.");
                }
#endif
            });
        },
        ARDUINO_EVENT_ETH_CONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            auto ip = ETH.localIP().toString();
            auto subnet = ETH.subnetMask();
            logger.printfln("Got IP address: %s/%u", ip.c_str(), WiFiGenericClass::calculateSubnetCIDR(subnet));

            uint32_t now = millis();
            was_connected = true;
            last_connected = now;

            connection_state = EthernetState::Connected;

            task_scheduler.scheduleOnce([this, now, ip, subnet](){
                state.get("connection_state")->updateEnum(connection_state);
                state.get("ip")->updateString(ip);
                state.get("subnet")->updateString(subnet.toString());
                state.get("connection_start")->updateUint(now);
            });
        },
        ARDUINO_EVENT_ETH_GOT_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Got IPv6 address: %s.", ETH.localIPv6().toString().c_str());
        },
        ARDUINO_EVENT_ETH_GOT_IP6);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Lost IP address.");
            this->print_con_duration();

            uint32_t now = millis();

            connection_state = EthernetState::Connecting;

            task_scheduler.scheduleOnce([this, now](){
                state.get("connection_state")->updateEnum(connection_state);
                state.get("ip")->updateString("0.0.0.0");
                state.get("subnet")->updateString("0.0.0.0");
                state.get("connection_end")->updateUint(now);
            });
        },
        ARDUINO_EVENT_ETH_LOST_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Disconnected");
            this->print_con_duration();

            uint32_t now = millis();

            connection_state = EthernetState::NotConnected;

            task_scheduler.scheduleOnce([this, now](){
                state.get("connection_state")->updateEnum(connection_state);
                state.get("ip")->updateString("0.0.0.0");
                state.get("subnet")->updateString("0.0.0.0");
                state.get("connection_end")->updateUint(now);
            });
        },
        ARDUINO_EVENT_ETH_DISCONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Stopped");
            this->print_con_duration();

            uint32_t now = millis();

            connection_state = EthernetState::NotConnected;

            task_scheduler.scheduleOnce([this, now](){
                state.get("connection_state")->updateEnum(connection_state);
                state.get("connection_end")->updateUint(now);
            });
        },
        ARDUINO_EVENT_ETH_STOP);

    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_TYPE);
}

void Ethernet::register_urls()
{
    api.addPersistentConfig("ethernet/config", &config);
    api.addState("ethernet/state", &state);
    api.addCommand("ethernet/force_reset", Config::Null(), {}, [this](String &/*errmsg*/) {
        esp_eth_stop(ETH.eth_handle);
        pinMode(5, OUTPUT);
        digitalWrite(5, LOW);
        delay(100);
        digitalWrite(5, HIGH);
        esp_eth_start(ETH.eth_handle);
    }, true);
}

EthernetState Ethernet::get_connection_state() const
{
    if (!initialized)
        return EthernetState::NotConfigured;
    return this->connection_state;
}

bool Ethernet::is_enabled() const
{
    return config_in_use.get("enable_ethernet")->asBool();
}
