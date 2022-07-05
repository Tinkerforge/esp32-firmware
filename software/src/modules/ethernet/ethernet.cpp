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

extern API api;
extern TaskScheduler task_scheduler;
extern char local_uid_str[7];

Ethernet::Ethernet()
{
    ethernet_config = ConfigRoot(Config::Object({
        {"enable_ethernet", Config::Bool(true)},
        {"ip", Config::Str("0.0.0.0", 7, 15)},
        {"gateway", Config::Str("0.0.0.0", 7, 15)},
        {"subnet", Config::Str("0.0.0.0", 7, 15)},
        {"dns", Config::Str("0.0.0.0", 7, 15)},
        {"dns2", Config::Str("0.0.0.0", 7, 15)},
    }), [](Config &cfg) -> String {
        const char *ip = cfg.get("ip")->asCStr();
        const char *gateway = cfg.get("gateway")->asCStr();
        const char *subnet = cfg.get("subnet")->asCStr();
        const char *dns = cfg.get("dns")->asCStr();
        const char *dns2 = cfg.get("dns2")->asCStr();

        IPAddress ip_addr, subnet_mask, gateway_addr, unused;

        if (!ip_addr.fromString(ip))
            return "Failed to parse \"ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!gateway_addr.fromString(gateway))
            return "Failed to parse \"gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!subnet_mask.fromString(subnet))
            return "Failed to parse \"subnet\": Expected format is dotted decimal, i.e. 255.255.255.0";

        if (!is_valid_subnet_mask(subnet_mask))
            return "Invalid subnet mask passed: Expected format is 255.255.255.0";

        if (ip_addr != IPAddress(0,0,0,0) && is_in_subnet(ip_addr, subnet_mask, IPAddress(127,0,0,1)))
            return "Invalid IP or subnet mask passed: This configuration would route localhost (127.0.0.1) to the ethernet interface.";

        if (gateway_addr != IPAddress(0,0,0,0) && !is_in_subnet(ip_addr, subnet_mask, gateway_addr))
            return "Invalid IP, subnet mask, or gateway passed: IP and gateway are not in the same network according to the subnet mask.";

        if (!unused.fromString(dns))
            return "Failed to parse \"dns\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(dns2))
            return "Failed to parse \"dns2\": Expected format is dotted decimal, i.e. 10.0.0.1";

        return "";
    });

    ethernet_state = Config::Object({
        {"connection_state", Config::Uint(0)},
        {"ip", Config::Str("0.0.0.0", 7, 15)},
        {"full_duplex", Config::Bool(false)},
        {"link_speed", Config::Uint8(0)}
    });

    ethernet_force_reset = Config::Null();
}

void Ethernet::setup()
{
    api.addFeature("ethernet");
    api.restorePersistentConfig("ethernet/config", &ethernet_config);

    ethernet_config_in_use = ethernet_config;

    ethernet_state.get("connection_state")->updateUint(0);

    initialized = true;

    if (!ethernet_config_in_use.get("enable_ethernet")->asBool())
        return;

    ethernet_state.get("connection_state")->updateUint(1);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet started");
            ETH.setHostname(network.config.get("hostname")->asCStr());
            ethernet_state.get("connection_state")->updateUint(1);
        },
        ARDUINO_EVENT_ETH_START);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet connected");
            ethernet_state.get("connection_state")->updateUint(2);
        },
        ARDUINO_EVENT_ETH_CONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet MAC: %s, IPv4: %s, %s Duplex, %u Mbps", ETH.macAddress().c_str(), ETH.localIP().toString().c_str(), ETH.fullDuplex() ? "Full" : "Half", ETH.linkSpeed());
            ethernet_state.get("connection_state")->updateUint(3);
            ethernet_state.get("full_duplex")->updateBool(ETH.fullDuplex());
            ethernet_state.get("link_speed")->updateUint(ETH.linkSpeed());

            auto ip = ETH.localIP().toString();
            logger.printfln("Ethernet got IP address: %s", ip.c_str());
            ethernet_state.get("ip")->updateString(ip);

            api.wifiAvailable();
        },
        ARDUINO_EVENT_ETH_GOT_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet got IPv6 address: %s.", ETH.localIPv6().toString().c_str());
        },
        ARDUINO_EVENT_ETH_GOT_IP6);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet lost IP address.");
            ethernet_state.get("connection_state")->updateUint(2);

            ethernet_state.get("ip")->updateString("0.0.0.0");
        },
        ARDUINO_EVENT_ETH_LOST_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet disconnected");
            ethernet_state.get("connection_state")->updateUint(1);

            ethernet_state.get("ip")->updateString("0.0.0.0");
        },
        ARDUINO_EVENT_ETH_DISCONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
           logger.printfln("Ethernet stopped");
           ethernet_state.get("connection_state")->updateUint(1);
        },
        ARDUINO_EVENT_ETH_STOP);

    IPAddress ip, subnet, gateway, dns, dns2;

    ip.fromString(ethernet_config_in_use.get("ip")->asCStr());
    subnet.fromString(ethernet_config_in_use.get("subnet")->asCStr());
    gateway.fromString(ethernet_config_in_use.get("gateway")->asCStr());
    dns.fromString(ethernet_config_in_use.get("dns")->asCStr());
    dns2.fromString(ethernet_config_in_use.get("dns2")->asCStr());

    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_TYPE);

    if (ip != 0) {
        ETH.config(ip, gateway, subnet, dns, dns2);
    } else {
        ETH.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
    }
}

void Ethernet::register_urls()
{
    api.addPersistentConfig("ethernet/config", &ethernet_config, {}, 1000);
    api.addState("ethernet/state", &ethernet_state, {}, 1000);
    api.addCommand("ethernet/force_reset", &ethernet_force_reset, {}, [this](){
        esp_eth_stop(ETH.eth_handle);
        pinMode(5, OUTPUT);
        digitalWrite(5, LOW);
        delay(100);
        digitalWrite(5, HIGH);
        esp_eth_start(ETH.eth_handle);
    }, true);
}

void Ethernet::loop()
{
}

EthernetState Ethernet::get_connection_state()
{
    if (!initialized)
        return EthernetState::NOT_CONFIGURED;
    return (EthernetState)ethernet_state.get("connection_state")->asUint();
}
