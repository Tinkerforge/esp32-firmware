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

#include "ethernet.h"

#define ETH_ADDR        0
#define ETH_POWER_PIN   5
#define ETH_TYPE        ETH_PHY_KSZ8081

#include <ETH.h>

#include "api.h"
#include "task_scheduler.h"

extern API api;
extern TaskScheduler task_scheduler;
extern char uid[7];

Ethernet::Ethernet()
{
    ethernet_config = Config::Object({
        {"enable_ethernet", Config::Bool(true)},
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

    ethernet_state = Config::Object({
        {"connection_state", Config::Uint(0)},
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
        {"full_duplex", Config::Bool(false)},
        {"link_speed", Config::Uint8(0)}
    });
}
/*
bool eth_connected = false;

void WiFiEvent(WiFiEvent_t event)
{
  switch (event) {
    case SYSTEM_EVENT_ETH_START:
      Serial.println("ETH Started");
      //set eth hostname here
      ETH.setHostname("esp32-ethernet");
      break;
    case SYSTEM_EVENT_ETH_CONNECTED:
      Serial.println("ETH Connected");
      break;
    case SYSTEM_EVENT_ETH_GOT_IP:
      Serial.print("ETH MAC: ");
      Serial.print(ETH.macAddress());
      Serial.print(", IPv4: ");
      Serial.print(ETH.localIP());
      if (ETH.fullDuplex()) {
        Serial.print(", FULL_DUPLEX");
      }
      Serial.print(", ");
      Serial.print(ETH.linkSpeed());
      Serial.println("Mbps");
      eth_connected = true;
      break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case SYSTEM_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default:
      break;
  }
}*/

void Ethernet::setup()
{
    String default_hostname = String(__HOST_PREFIX__) + String("-") + String(uid);

    if(!api.restorePersistentConfig("ethernet/config", &ethernet_config)) {
        ethernet_config.get("hostname")->updateString(default_hostname);
    }

    ethernet_config_in_use = ethernet_config;

    ethernet_state.get("connection_state")->updateUint(0);

    initialized = true;

    if (!ethernet_config_in_use.get("enable_ethernet")->asBool())
        return;

    ethernet_state.get("connection_state")->updateUint(1);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet started");
            ETH.setHostname(ethernet_config_in_use.get("hostname")->asString().c_str());
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

            auto ip = ETH.localIP();
            logger.printfln("Ethernet got IP address: %u.%u.%u.%u.", ip[0], ip[1], ip[2], ip[3]);
            ethernet_state.get("ip")->get(0)->updateUint(ip[0]);
            ethernet_state.get("ip")->get(1)->updateUint(ip[1]);
            ethernet_state.get("ip")->get(2)->updateUint(ip[2]);
            ethernet_state.get("ip")->get(3)->updateUint(ip[3]);
            api.wifiAvailable();
        },
        ARDUINO_EVENT_ETH_GOT_IP);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            logger.printfln("Ethernet disconnected");
            ethernet_state.get("connection_state")->updateUint(1);

            ethernet_state.get("ip")->get(0)->updateUint(0);
            ethernet_state.get("ip")->get(1)->updateUint(0);
            ethernet_state.get("ip")->get(2)->updateUint(0);
            ethernet_state.get("ip")->get(3)->updateUint(0);
        },
        ARDUINO_EVENT_ETH_DISCONNECTED);

    WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
           logger.printfln("Ethernet stopped");
           ethernet_state.get("connection_state")->updateUint(1);
        },
        ARDUINO_EVENT_ETH_STOP);

    uint8_t ip[4], subnet[4], gateway[4], dns[4], dns2[4];
    ethernet_config_in_use.get("ip")->fillUint8Array(ip, 4);
    ethernet_config_in_use.get("subnet")->fillUint8Array(subnet, 4);
    ethernet_config_in_use.get("gateway")->fillUint8Array(gateway, 4);
    ethernet_config_in_use.get("dns")->fillUint8Array(dns, 4);
    ethernet_config_in_use.get("dns2")->fillUint8Array(dns2, 4);

    ETH.begin(ETH_ADDR, ETH_POWER_PIN, ETH_PHY_MDC, ETH_PHY_MDIO, ETH_TYPE);

    if(ip != 0) {
        ETH.config(ip, gateway, subnet, dns, dns2);
    } else {
        ETH.config((uint32_t)0, (uint32_t)0, (uint32_t)0);
    }
}

void Ethernet::register_urls()
{
    api.addPersistentConfig("ethernet/config", &ethernet_config, {}, 1000);
    api.addPersistentConfig("ethernet/state", &ethernet_state, {}, 1000);
}

void Ethernet::loop()
{

}

EthernetState Ethernet::get_connection_state()
{
    if(!initialized)
        return EthernetState::NOT_CONFIGURED;
    return (EthernetState) ethernet_state.get("connection_state")->asUint();
}
