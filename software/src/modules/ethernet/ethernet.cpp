/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "event_log_prefix.h"

#include "build.h"
#include "options.h"
#include "tools/printf.h"
#include "tools/malloc.h"
#include "tools/net.h"

#include "gcc_warnings.h"

extern char local_uid_str[32];

static constexpr uint8_t IGMP_MAC[6] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x01};

// Generation counter for async ETH.begin() tasks. Incremented on every
// enable or disable so that stale tasks from a prior enable/disable cycle
// see a mismatch and skip the ETH.begin() call.
static std::atomic<uint32_t> eth_begin_generation{0};

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
    [this](Config &cfg, ConfigSource source) -> String {
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

        task_scheduler.scheduleOnce([this]() {
            this->apply_config();
        });

        return "";
    }};

    state = Config::Object({
        {"connection_state", Config::Enum(EthernetState::NotConfigured)},
        {"connection_start", Config::Uptime()},
        {"connection_end", Config::Uptime()},
        {"mac", Config::Str("", 0, 17)},
        {"ip", Config::Str("0.0.0.0", 7, 15)},
        {"subnet", Config::Str("0.0.0.0", 7, 15)},
        {"full_duplex", Config::Bool(false)},
        {"link_speed", Config::Uint8(0)}
    });
}

void Ethernet::print_con_duration()
{
    if (runtime_data->was_connected) {
        runtime_data->was_connected = false;
        const uint64_t connected_for_s = (now_us() - runtime_data->last_connected).to<seconds_t>().as<uint64_t>();
        logger.printfln("Was connected for %llu seconds.", connected_for_s);
    }
}

static void eth_async_begin(void *arg)
{
    const uint32_t my_generation = reinterpret_cast<uintptr_t>(arg);

    if (eth_begin_generation.load() == my_generation) {
        if (!ETH.begin()) {
            logger.printfln("Async start failed. PHY broken?");
        }
    } else {
        logger.printfln("Async start cancelled (generation mismatch: mine=%lu, current=%lu).",
                        my_generation,
                        eth_begin_generation.load());
    }

    vTaskDelete(NULL); // exit RTOS task
}

void Ethernet::setup()
{
    api.addFeature("ethernet");
    api.restorePersistentConfig("ethernet/config", &config);

    initialized = true;

    runtime_data = static_cast<decltype(runtime_data)>(malloc_psram_or_dram(sizeof(*runtime_data)));

    if (!runtime_data) {
        logger.printfln("Failed to allocate memory for Ethernet runtime data");
        return;
    }

    runtime_data->was_connected = false;

    Network.begin();

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t /*info*/) {
            const uint32_t t_startup = (now_us() - this->runtime_data->last_connected).as<uint32_t>() / 1000;
            logger.printfln("Started after %lums", t_startup);

#if MODULE_NETWORK_AVAILABLE()
            const String &hostname = network.get_hostname();
#else
            String hostname{OPTIONS_HOSTNAME_PREFIX()};
            hostname.concat("-", 1);
            hostname.concat(local_uid_str);
#endif
            ETH.setHostname(hostname.c_str()); // Underlying API creates a copy.
            ETH.setRoutePrio(110); // Prefer Ethernet over WiFi, which has priority 100.

            // Manually add a MAC filter to accept IGMP packets because lwIP is bugged and doesn't do it.
            const esp_err_t err = esp_eth_ioctl(ETH.handle(), ETH_CMD_ADD_MAC_FILTER, const_cast<uint8_t *>(IGMP_MAC));

            if (err != ESP_OK) {
                logger.printfln("Setting IGMP filter failed: %s (%04X)", esp_err_to_name(err), static_cast<unsigned>(err));
            }

            this->runtime_data->connection_state = EthernetState::NotConnected;

            uint8_t mac[6];
            if (ETH.macAddress(mac) == nullptr) {
                memset(mac, 0, std::size(mac));
            }

            task_scheduler.scheduleOnce([this, mac]() {
                char mac_str[18];
                const size_t len = snprintf_u(mac_str, std::size(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

                state.get("mac")->updateString(String{mac_str, len});
                state.get("connection_state")->updateEnum(this->runtime_data->connection_state);
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

            if (this->runtime_data->ip.addr != 0) {
                ETH.config({this->runtime_data->ip.addr     },
                           {this->runtime_data->gateway.addr},
                           {tf_ip4addr_cidr2mask(this->runtime_data->subnet_cidr).addr},
                           {this->runtime_data->dns.addr    },
                           {this->runtime_data->dns2.addr   });
            } else {
                ETH.config();
            }

            const bool full_duplex = ETH.fullDuplex();

            logger.printfln("Connected: %hu Mbps, %s Duplex",
                            link_speed,
                            full_duplex ? "Full" : "Half");

            this->runtime_data->connection_state = EthernetState::Connecting;

            task_scheduler.scheduleOnce([this, link_speed, full_duplex]() {
                state.get("connection_state")->updateEnum(this->runtime_data->connection_state);
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
            char ip_str[INET_ADDRSTRLEN];
            tf_ip4addr_ntoa(&ip_info.ip, ip_str, ARRAY_SIZE(ip_str));
            char gw_str[INET_ADDRSTRLEN];
            tf_ip4addr_ntoa(&ip_info.gw, gw_str, ARRAY_SIZE(gw_str));
            const uint32_t subnet = ip_info.netmask.addr;
            const bool was_already_connected = this->runtime_data->connection_state == EthernetState::Connected;
            const bool ip_changed = info.got_ip.ip_changed;

            logger.printfln("Got IP address: %s/%i, GW %s", ip_str, tf_ip4addr_mask2cidr(ip4_addr_t{subnet}), gw_str);

            const micros_t now = now_us();
            this->runtime_data->was_connected = true;
            this->runtime_data->last_connected = now;

            this->runtime_data->connection_state = EthernetState::Connected;

            const String ip_string{ip_str};
            task_scheduler.scheduleOnce([this, now, ip_string, subnet, was_already_connected, ip_changed]() {
                char subnet_str[INET_ADDRSTRLEN];
                tf_ip4addr_ntoa(&subnet, subnet_str, ARRAY_SIZE(subnet_str));

                state.get("ip"    )->updateString(ip_string);
                state.get("subnet")->updateString(subnet_str);

                if (was_already_connected && ip_changed) {
                    // When the IP changes while already connected (e.g. static IP reconfigured
                    // at runtime), ESP-IDF only fires GOT_IP with ip_changed=true but no LOST_IP.
                    // Set Connecting now so the event system's next polling cycle propagates it to
                    // network.cpp (which fires on_network_connected(false), making modules release
                    // old connections).
                    state.get("connection_state")->updateEnum(EthernetState::Connecting);
                    this->reconnect_deadline = calculate_deadline(1_s);
                    this->reconnect_connection_start = now;
                } else {
                    state.get("connection_state")->updateEnum(this->runtime_data->connection_state);
                    state.get("connection_start")->updateUptime(now);
                }
            });
        },
        ARDUINO_EVENT_ETH_GOT_IP);

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t info) {
            char ip6_str[INET6_ADDRSTRLEN];
            tf_ip6addr_ntoa(&info.got_ip6.ip6_info.ip, ip6_str, ARRAY_SIZE(ip6_str));
            logger.printfln("Got IPv6 address: %s", ip6_str);
        },
        ARDUINO_EVENT_ETH_GOT_IP6);

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t /*info*/) {
            logger.printfln("Lost IP address.");
            this->print_con_duration();

            auto now = now_us();

            // Restart DHCP, if it's enabled, to make sure that the GOT_IP event fires when receiving the same address as before.
            if (this->runtime_data->ip.addr == 0) {
                ETH.config();
            }

            this->runtime_data->connection_state = EthernetState::Connecting;

            task_scheduler.scheduleOnce([this, now]() {
                this->reconnect_deadline = 0_us; // Cancel pending IP-change reconnect
                state.get("connection_state")->updateEnum(this->runtime_data->connection_state);
                state.get("ip")->updateString("0.0.0.0");
                state.get("subnet")->updateString("0.0.0.0");
                state.get("connection_end")->updateUptime(now);
            });
        },
        ARDUINO_EVENT_ETH_LOST_IP);

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t /*info*/) {
            logger.printfln("Disconnected");
            this->print_con_duration();

            auto now = now_us();

            this->runtime_data->connection_state = EthernetState::NotConnected;

            task_scheduler.scheduleOnce([this, now]() {
                this->reconnect_deadline = 0_us; // Cancel pending IP-change reconnect
                state.get("connection_state")->updateEnum(this->runtime_data->connection_state);
                state.get("ip")->updateString("0.0.0.0");
                state.get("subnet")->updateString("0.0.0.0");
                state.get("connection_end")->updateUptime(now);
            });
        },
        ARDUINO_EVENT_ETH_DISCONNECTED);

    Network.onEvent([this](arduino_event_id_t /*event*/, arduino_event_info_t /*info*/) {
            logger.printfln("Stopped");
            this->print_con_duration();

            uint32_t now_ms = now_us().to<millis_t>().as<uint32_t>();

            // If eth_started is false, we're being shut down by apply_config() for a disable.
            this->runtime_data->connection_state = this->eth_started
                ? EthernetState::NotConnected
                : EthernetState::NotConfigured;

            task_scheduler.scheduleOnce([this, now_ms]() {
                this->reconnect_deadline = 0_us; // Cancel pending IP-change reconnect
                state.get("connection_state")->updateEnum(this->runtime_data->connection_state);
                state.get("connection_end")->updateUint(now_ms);
            });
        },
        ARDUINO_EVENT_ETH_STOP);

    ETH.setTaskStackSize(2304);

    apply_config();
}

void Ethernet::register_urls()
{
    api.addPersistentConfig("ethernet/config", &config);
    api.addState("ethernet/state", &state);
}

void Ethernet::loop()
{
    if (reconnect_deadline != 0_us && deadline_elapsed(reconnect_deadline)) {
        reconnect_deadline = 0_us;
        state.get("connection_state")->updateEnum(this->runtime_data->connection_state);
        state.get("connection_start")->updateUptime(this->reconnect_connection_start);
    }
}

EthernetState Ethernet::get_connection_state() const
{
    if (!runtime_data) {
        return EthernetState::NotConfigured;
    }
    return runtime_data->connection_state;
}

bool Ethernet::is_enabled() const
{
    return eth_started;
}

void Ethernet::apply_config()
{
    const bool want_enabled = config.get("enable_ethernet")->asBool();

    if (want_enabled) {
        // Parse new IP settings from config.
        ip4_addr_t subnet_tmp;
        ip4addr_aton(config.get("ip"     )->asUnsafeCStr(), &runtime_data->ip);
        ip4addr_aton(config.get("gateway")->asUnsafeCStr(), &runtime_data->gateway);
        ip4addr_aton(config.get("dns"    )->asUnsafeCStr(), &runtime_data->dns);
        ip4addr_aton(config.get("dns2"   )->asUnsafeCStr(), &runtime_data->dns2);
        ip4addr_aton(config.get("subnet" )->asUnsafeCStr(), &subnet_tmp);
        runtime_data->subnet_cidr = tf_ip4addr_mask2cidr(subnet_tmp);

        if (!eth_started) {
            // Disabled -> Enabled: start the interface.
            runtime_data->connection_state = EthernetState::NotConnected;
            state.get("connection_state")->updateEnum(runtime_data->connection_state);

            runtime_data->last_connected = now_us();
            eth_started = true;
            const uint32_t gen = ++eth_begin_generation;

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    #pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

            const BaseType_t ret = xTaskCreatePinnedToCore(eth_async_begin, "eth_async_begin", 2560, reinterpret_cast<void *>(gen), uxTaskPriorityGet(nullptr) + 1, nullptr, 1);
            if (ret != pdPASS) {
                logger.printfln("eth_async_begin task could not be created: %s (0x%lx)", esp_err_to_name(ret), static_cast<uint32_t>(ret));
                if (!ETH.begin()) {
                    logger.printfln("Start failed. PHY broken?");
                }
            }

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
        } else {
            // Already enabled: IP/DNS change.
            // If connected or connecting, apply new IP settings immediately.
            // Otherwise the ETH_CONNECTED handler will pick up the new runtime_data values.
            if (runtime_data->connection_state == EthernetState::Connected
             || runtime_data->connection_state == EthernetState::Connecting) {
                if (runtime_data->ip.addr != 0) {
                    ETH.config({runtime_data->ip.addr     },
                               {runtime_data->gateway.addr},
                               {tf_ip4addr_cidr2mask(runtime_data->subnet_cidr).addr},
                               {runtime_data->dns.addr    },
                               {runtime_data->dns2.addr   });
                } else {
                    ETH.config();
                }
            }
        }
    } else if (eth_started) {
        // Enabled -> Disabled: requires a reboot to take effect.
        // The config is already saved; setup() will skip ETH.begin() on next boot.
        // The frontend enforces the reboot via a modal dialog.
        logger.printfln("Ethernet disabled in config. Reboot required to take effect.");
    } else {
        // Not enabled and not started (initial boot with ethernet disabled).
        runtime_data->connection_state = EthernetState::NotConfigured;
    }
}
