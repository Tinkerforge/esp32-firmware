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

#include "api.h"
#include "task_scheduler.h"
#include "build.h"
#include "modules.h"
#include "tools.h"

extern char local_uid_str[32];

void Ethernet::pre_setup()
{
    ethernet_config = ConfigRoot(Config::Object({
        {"enable_ethernet", Config::Bool(true)},
        {"ip", Config::Str("0.0.0.0", 7, 15)},
        {"gateway", Config::Str("0.0.0.0", 7, 15)},
        {"subnet", Config::Str("0.0.0.0", 7, 15)},
        {"dns", Config::Str("0.0.0.0", 7, 15)},
        {"dns2", Config::Str("0.0.0.0", 7, 15)},
    }), [](Config &cfg) -> String {
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
            return "Invalid IP or subnet mask passed: This configuration would route localhost (127.0.0.1) to the ethernet interface.";

        if (gateway_addr != IPAddress(0,0,0,0) && !is_in_subnet(ip_addr, subnet_mask, gateway_addr))
            return "Invalid IP, subnet mask, or gateway passed: IP and gateway are not in the same network according to the subnet mask.";

        if (!unused.fromString(cfg.get("dns")->asEphemeralCStr()))
            return "Failed to parse \"dns\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(cfg.get("dns2")->asEphemeralCStr()))
            return "Failed to parse \"dns2\": Expected format is dotted decimal, i.e. 10.0.0.1";

        return "";
    });

    ethernet_state = Config::Object({
        {"connection_state", Config::Uint((uint)EthernetState::NOT_CONFIGURED)},
        {"connection_start", Config::Uint(0)},
        {"connection_end", Config::Uint(0)},
        {"ip", Config::Str("0.0.0.0", 7, 15)},
        {"full_duplex", Config::Bool(false)},
        {"link_speed", Config::Uint8(0)}
    });
}

void Ethernet::print_con_duration() {
    if (was_connected) {
        was_connected = false;
        uint32_t now = millis();
        uint32_t connected_for = now - last_connected;
        ethernet_state.get("connection_end")->updateUint(now);
        if (connected_for < 0x7FFFFFFF) {
            logger.printfln("Ethernet was connected for %u seconds.", connected_for / 1000);
        } else {
            logger.printfln("Ethernet was connected for a long time.");
        }
    }
}

void Ethernet::setup()
{
    api.addFeature("ethernet");
    api.restorePersistentConfig("ethernet/config", &ethernet_config);

    ethernet_config_in_use = ethernet_config;

    ethernet_state.get("connection_state")->updateUint((uint)EthernetState::NOT_CONFIGURED);

    initialized = true;

    if (!ethernet_config_in_use.get("enable_ethernet")->asBool())
        return;

    ethernet_state.get("connection_state")->updateUint((uint)EthernetState::NOT_CONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet started");
#if MODULE_NETWORK_AVAILABLE()
            ETH.setHostname(network.config.get("hostname")->asEphemeralCStr());
#else
            ETH.setHostname((String(BUILD_HOST_PREFIX) + "-" + local_uid_str).c_str());
#endif
            ethernet_state.get("connection_state")->updateUint((uint)EthernetState::NOT_CONNECTED);
        },
        ARDUINO_EVENT_ETH_START);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            uint32_t link_speed = ETH.linkSpeed();
            bool full_duplex    = ETH.fullDuplex();
            logger.printfln("Ethernet connected: %u Mbps %s Duplex, MAC: %s", link_speed, full_duplex ? "Full" : "Half", ETH.macAddress().c_str());
            ethernet_state.get("connection_state")->updateUint((uint)EthernetState::CONNECTING);
            ethernet_state.get("link_speed" )->updateUint(link_speed);
            ethernet_state.get("full_duplex")->updateBool(full_duplex);

            IPAddress ip, subnet, gateway, dns, dns2;
            ip.fromString(ethernet_config_in_use.get("ip")->asEphemeralCStr());
            subnet.fromString(ethernet_config_in_use.get("subnet")->asEphemeralCStr());
            gateway.fromString(ethernet_config_in_use.get("gateway")->asEphemeralCStr());
            dns.fromString(ethernet_config_in_use.get("dns")->asEphemeralCStr());
            dns2.fromString(ethernet_config_in_use.get("dns2")->asEphemeralCStr());

            if (link_speed < 100)
                delay(40); // 10MBit usually needs at least 19ms extra. Give it 40ms to be safe.

            if (ip != 0) {
                ETH.config(ip, gateway, subnet, dns, dns2);
            } else {
                ETH.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
            }
        },
        ARDUINO_EVENT_ETH_CONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            auto ip = ETH.localIP().toString();
            logger.printfln("Ethernet got IP address: %s", ip.c_str());
            ethernet_state.get("connection_state")->updateUint((uint)EthernetState::CONNECTED);
            ethernet_state.get("ip")->updateString(ip);

            was_connected = true;
            last_connected = millis();
            ethernet_state.get("connection_start")->updateUint(last_connected);
        },
        ARDUINO_EVENT_ETH_GOT_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet got IPv6 address: %s.", ETH.localIPv6().toString().c_str());
        },
        ARDUINO_EVENT_ETH_GOT_IP6);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet lost IP address.");
            ethernet_state.get("connection_state")->updateUint((uint)EthernetState::CONNECTING);

            ethernet_state.get("ip")->updateString("0.0.0.0");
            this->print_con_duration();
        },
        ARDUINO_EVENT_ETH_LOST_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet disconnected");
            ethernet_state.get("connection_state")->updateUint((uint)EthernetState::NOT_CONNECTED);

            ethernet_state.get("ip")->updateString("0.0.0.0");
            this->print_con_duration();
        },
        ARDUINO_EVENT_ETH_DISCONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
           logger.printfln("Ethernet stopped");
           ethernet_state.get("connection_state")->updateUint((uint)EthernetState::NOT_CONNECTED);
            this->print_con_duration();
        },
        ARDUINO_EVENT_ETH_STOP);

    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_TYPE);
}

void Ethernet::register_urls()
{
    api.addPersistentConfig("ethernet/config", &ethernet_config, {}, 1000);
    api.addState("ethernet/state", &ethernet_state, {}, 1000);
    api.addCommand("ethernet/force_reset", Config::Null(), {}, [this](){
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
        return EthernetState::NOT_CONFIGURED;
    return (EthernetState)ethernet_state.get("connection_state")->asUint();
}

bool Ethernet::is_enabled() const
{
    return ethernet_config_in_use.get("enable_ethernet")->asBool();
}
