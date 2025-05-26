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

#define ETH_PHY_TYPE  ETH_PHY_KSZ8081
#define ETH_PHY_ADDR  0
#define ETH_PHY_MDC   23
#define ETH_PHY_MDIO  18
#define ETH_PHY_POWER 5
#define ETH_CLK_MODE  ETH_CLOCK_GPIO0_IN

#include "module_dependencies.h"

#include <ETH.h>
#include <WiFiGeneric.h> // for calculateSubnetCIDR
#include <esp_eth.h>

#include "event_log_prefix.h"

#include "build.h"
#include "tools.h"
#include "tools/net.h"

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
        {"connection_state", Config::Enum(EthernetState::NotConfigured)},
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
        auto connected_for = now_us() - last_connected;
        logger.printfln("Was connected for %lu seconds.", connected_for.to<seconds_t>().as<uint32_t>());
    }
}

static void eth_async_begin(void *)
{
    if (!ETH.begin()) {
        logger.printfln("Async start failed. PHY broken?");
    }
    vTaskDelete(NULL); // exit RTOS task
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

    connection_state = EthernetState::NotConnected;
    state.get("connection_state")->updateEnum(connection_state);

    Network.begin();

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t /*info*/) {
            logger.printfln("Started");

#if MODULE_NETWORK_AVAILABLE()
            const String &hostname = network.get_hostname();
#else
            String hostname{BUILD_HOST_PREFIX};
            hostname.concat("-", 1);
            hostname.concat(local_uid_str);
#endif
            ETH.setHostname(hostname.c_str()); // Underlying API creates a copy.

            connection_state = EthernetState::NotConnected;

            task_scheduler.scheduleOnce([this]() {
                state.get("connection_state")->updateEnum(connection_state);
            });
        },
        ARDUINO_EVENT_ETH_START);

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t /*info*/) {
            const uint16_t link_speed = ETH.linkSpeed();
            if (link_speed < 100) {
                // A 10MBit link sometimes chokes on link-up and won't be able to send any data.
                // It usually needs at least an additional 19ms to settle before configuring an IP.
                // Give it 40ms to be safe.
                delay(40);
            }

            const IPAddress ip{config_in_use.get("ip")->asUnsafeCStr()};
            if (static_cast<uint32_t>(ip) != 0) {
                ETH.config(ip,
                           {config_in_use.get("gateway")->asUnsafeCStr()},
                           {config_in_use.get("subnet" )->asUnsafeCStr()},
                           {config_in_use.get("dns"    )->asUnsafeCStr()},
                           {config_in_use.get("dns2"   )->asUnsafeCStr()});
            } else {
                ETH.config();
            }

            uint8_t mac[6];
            if (!ETH.macAddress(mac)) {
                memset(mac, 0, sizeof(mac));
            }
            const bool full_duplex = ETH.fullDuplex();

            logger.printfln("Connected: %hu Mbps %s Duplex, MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                            link_speed,
                            full_duplex ? "Full" : "Half",
                            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

            connection_state = EthernetState::Connecting;

            task_scheduler.scheduleOnce([this, link_speed, full_duplex]() {
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

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t info) {
            const esp_netif_ip_info_t &ip_info = info.got_ip.ip_info;
            char ip_str[16];
            tf_ip4addr_ntoa(&ip_info.ip, ip_str, ARRAY_SIZE(ip_str));
            char gw_str[16];
            tf_ip4addr_ntoa(&ip_info.gw, gw_str, ARRAY_SIZE(gw_str));
            const uint32_t subnet = ip_info.netmask.addr;

            logger.printfln("Got IP address: %s/%i, GW %s", ip_str, __builtin_clz(~ntohl(subnet)), gw_str);

            const micros_t now = now_us();
            was_connected = true;
            last_connected = now;
            const uint32_t now_ms = now.to<millis_t>().as<uint32_t>();

            connection_state = EthernetState::Connected;

            const String ip_string{ip_str};
            task_scheduler.scheduleOnce([this, now_ms, ip_string, subnet]() {
                char subnet_str[16];
                tf_ip4addr_ntoa(&subnet, subnet_str, ARRAY_SIZE(subnet_str));

                state.get("ip"    )->updateString(ip_string);
                state.get("subnet")->updateString(subnet_str);
                state.get("connection_state")->updateEnum(connection_state);
                state.get("connection_start")->updateUint(now_ms);
            });
        },
        ARDUINO_EVENT_ETH_GOT_IP);

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t /*info*/) {
            logger.printfln("Got IPv6 address: TODO PRINT ADDRESS.");
        },
        ARDUINO_EVENT_ETH_GOT_IP6);

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t /*info*/) {
            logger.printfln("Lost IP address.");
            this->print_con_duration();

            uint32_t now_ms = now_us().to<millis_t>().as<uint32_t>();

            // Restart DHCP to make sure that the GOT_IP event fires when receiving the same address as before.
            ETH.config();

            connection_state = EthernetState::Connecting;

            task_scheduler.scheduleOnce([this, now_ms]() {
                state.get("connection_state")->updateEnum(connection_state);
                state.get("ip")->updateString("0.0.0.0");
                state.get("subnet")->updateString("0.0.0.0");
                state.get("connection_end")->updateUint(now_ms);
            });
        },
        ARDUINO_EVENT_ETH_LOST_IP);

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t /*info*/) {
            logger.printfln("Disconnected");
            this->print_con_duration();

            uint32_t now_ms = now_us().to<millis_t>().as<uint32_t>();

            connection_state = EthernetState::NotConnected;

            task_scheduler.scheduleOnce([this, now_ms]() {
                state.get("connection_state")->updateEnum(connection_state);
                state.get("ip")->updateString("0.0.0.0");
                state.get("subnet")->updateString("0.0.0.0");
                state.get("connection_end")->updateUint(now_ms);
            });
        },
        ARDUINO_EVENT_ETH_DISCONNECTED);

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t /*info*/) {
            logger.printfln("Stopped");
            this->print_con_duration();

            uint32_t now_ms = now_us().to<millis_t>().as<uint32_t>();

            connection_state = EthernetState::NotConnected;

            task_scheduler.scheduleOnce([this, now_ms]() {
                state.get("connection_state")->updateEnum(connection_state);
                state.get("connection_end")->updateUint(now_ms);
            });
        },
        ARDUINO_EVENT_ETH_STOP);

    ETH.setTaskStackSize(2048);

    const BaseType_t ret = xTaskCreatePinnedToCore(eth_async_begin, "eth_async_begin", 2560, nullptr, ESP_TASK_PRIO_MAX - 2, nullptr, 1);
    if (ret == pdPASS) {
        logger.printfln("Starting");
    } else {
        logger.printfln("eth_async_begin task could not be created: %s (0x%lx)", esp_err_to_name(ret), static_cast<uint32_t>(ret));
        if (!ETH.begin()) {
            logger.printfln("Start failed. PHY broken?");
        }
    }
}

void Ethernet::register_urls()
{
    api.addPersistentConfig("ethernet/config", &config);
    api.addState("ethernet/state", &state);
    api.addCommand("ethernet/force_reset", Config::Null(), {}, [this](String &/*errmsg*/) {
        esp_eth_stop(ETH._eth_handle);
        pinMode(5, OUTPUT);
        digitalWrite(5, LOW);
        delay(100);
        digitalWrite(5, HIGH);
        esp_eth_start(ETH._eth_handle);
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
