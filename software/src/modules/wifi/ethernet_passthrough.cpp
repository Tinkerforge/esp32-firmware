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

#include "generated/module_dependencies.h"

#if MODULE_ETHERNET_AVAILABLE()
#include "ethernet_passthrough.h"

#include <cstring>
#include <ETH.h>
#include <esp_wifi.h>
#include <esp_netif_net_stack.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <lwip/timeouts.h>
#include <netif/ethernet.h>

namespace {
// All bridge state and frame processing belong to the TCP/IP thread. The
// driver-facing input function only queues packets, just like tcpip_input.
netif *eth_port = nullptr;
netif *ap_port = nullptr;
netif_input_fn eth_input = nullptr;
netif_input_fn ap_input = nullptr;
netif_linkoutput_fn eth_output = nullptr;
bool owns_promiscuous = false;
uint8_t client_macs[ESP_WIFI_MAX_CONN_NUM][6];
uint8_t client_count = 0;

bool is_client(const uint8_t *mac)
{
    for (uint8_t i = 0; i < client_count; ++i) {
        if (memcmp(mac, client_macs[i], 6) == 0)
            return true;
    }
    return false;
}

void learn_client(const uint8_t *mac)
{
    // A station can send DHCP before its connected event reaches the scheduler.
    // Reception on the AP is already restricted to associated stations.
    if (!(mac[0] & 1) && !is_client(mac) && client_count < ESP_WIFI_MAX_CONN_NUM)
        memcpy(client_macs[client_count++], mac, 6);
}

// Bounded queue serviced by lwIP timers: never sleep in the TCP/IP thread.
// A rejected frame stays owned here until accepted, expired, or bridge teardown.
constexpr unsigned tx_capacity = 32;
constexpr unsigned tx_interval_ms = 5;
constexpr unsigned tx_budget = 4;
constexpr uint32_t tx_lifetime_ms = 250;
struct Pending { pbuf *packet; uint32_t queued_at; };
Pending pending[tx_capacity]{};
unsigned tx_head = 0, tx_count = 0;
bool tx_timer = false;

esp_err_t transmit_wifi(pbuf *copy)
{
    esp_netif_t *driver = esp_netif_get_handle_from_netif_impl(ap_port);
    // The copying callback consumes the bytes synchronously. AP linkoutput
    // uses the PSRAM by-reference path, which can retain Ethernet RX buffers.
    return driver ? esp_netif_transmit(driver, copy->payload, copy->tot_len) : ESP_ERR_INVALID_STATE;
}

void pump_wifi(void *);

void schedule_wifi()
{
    if (!tx_timer && tx_count) {
        tx_timer = true;
        sys_timeout(tx_interval_ms, pump_wifi, nullptr);
    }
}

void discard_head()
{
    pbuf_free(pending[tx_head].packet);
    tx_head = (tx_head + 1) % tx_capacity;
    --tx_count;
}

void pump_wifi(void *)
{
    tx_timer = false;
    // Limit each turn so receive processing and the WiFi task can make progress.
    for (unsigned budget = 0; tx_count && budget < tx_budget; ++budget) {
        auto &entry = pending[tx_head];
        const auto *destination = static_cast<const uint8_t *>(entry.packet->payload);
        if (!ap_port || !netif_is_link_up(ap_port) || !client_count
            || (!(destination[0] & 1) && !is_client(destination))) {
            discard_head();
            continue;
        }
        if (static_cast<uint32_t>(sys_now() - entry.queued_at) >= tx_lifetime_ms) {
            discard_head();
            continue;
        }
        if (transmit_wifi(entry.packet) == ESP_ERR_NO_MEM) {
            break;
        }
        discard_head();
    }
    schedule_wifi();
}

void clear_wifi_queue()
{
    sys_untimeout(pump_wifi, nullptr);
    tx_timer = false;
    while (tx_count)
        discard_head();
}

void send_wifi(pbuf *packet)
{
    uint8_t destination[6];
    if (!netif_is_link_up(ap_port) || client_count == 0
        || pbuf_copy_partial(packet, destination, sizeof(destination), 0) != sizeof(destination)
        || (!(destination[0] & 1) && !is_client(destination))) {
        return;
    }
    if (tx_count == tx_capacity) {
        return;
    }
    // Private contiguous copy: neither ETH RX nor local ARP processing owns it.
    pbuf *copy = pbuf_clone(PBUF_RAW_TX, PBUF_RAM, packet);
    if (!copy) {
        return;
    }
    // Always pace submissions. Ethernet can outrun the eight static WiFi TX
    // buffers, including before the first NO_MEM response is visible.
    pending[(tx_head + tx_count) % tx_capacity] = {copy, sys_now()};
    ++tx_count;
    schedule_wifi();
}

void send_ethernet(pbuf *packet)
{
    if (!netif_is_link_up(eth_port))
        return;
    if (eth_output)
        eth_output(eth_port, packet);
}

bool local_destination(const uint8_t *destination)
{
    return memcmp(destination, eth_port->hwaddr, 6) == 0
        || memcmp(destination, ap_port->hwaddr, 6) == 0;
}

bool reserved_destination(const uint8_t *destination)
{
    // IEEE link-local control frames (including spanning tree) never cross a
    // bridge. There is no spanning tree implementation: avoid network loops.
    static const uint8_t prefix[] = {0x01, 0x80, 0xc2, 0x00, 0x00};
    return memcmp(destination, prefix, sizeof(prefix)) == 0 && destination[5] <= 0x0f;
}

err_t bridge_input(pbuf *packet, netif *incoming)
{
    if (!eth_port || (incoming != eth_port && incoming != ap_port))
        return ethernet_input(packet, incoming);

    uint8_t destination[6];
    if (packet->tot_len < 14 || pbuf_copy_partial(packet, destination, sizeof(destination), 0) != sizeof(destination)
        || reserved_destination(destination)) {
        pbuf_free(packet);
        return ERR_OK;
    }

    if (incoming == ap_port) {
        uint8_t source[6];
        if (pbuf_copy_partial(packet, source, sizeof(source), 6) == sizeof(source))
            learn_client(source);
    }

    const bool multicast = (destination[0] & 1) != 0;
    const bool local = local_destination(destination);
    if (!local) {
        // Forward the original Ethernet frame, including source MAC and all
        // EtherTypes. In particular DHCP, ARP, IPv6 NDP and multicast survive.
        if (incoming == ap_port)
            send_ethernet(packet);
        else
            send_wifi(packet);
    }

    if (local || multicast) {
        // Both ports use Ethernet's IP stack. This also makes the device's
        // Ethernet management address reachable from wireless clients.
        return ethernet_input(packet, eth_port);
    }
    pbuf_free(packet);
    return ERR_OK;
}

err_t queue_input(pbuf *packet, netif *incoming)
{
    return tcpip_inpkt(packet, incoming, bridge_input);
}

err_t host_output(netif *outgoing, pbuf *packet)
{
    // Locally generated frames must reach wireless clients too (ARP replies,
    // DHCP, IPv6 and replies from the web server). Never hairpin transit frames.
    send_wifi(packet);
    if (!eth_output)
        return ERR_IF;
    return eth_output(outgoing, packet);
}

// Use the public synchronous API and its dedicated completion semaphore.
// All callback inspection/installation is serialized with packet processing.
struct Change {
    netif *ethernet;
    netif *ap;
    wifi_sta_list_t clients;
    bool ready = false;
    bool refresh_promiscuous = false;
};

void change_ports(void *data)
{
    auto *change = static_cast<Change *>(data);
    const bool same_ports = eth_port == change->ethernet && ap_port == change->ap;

    if (change->ethernet) {
        // ETH.netif() exists before asynchronous ETH.begin() has called
        // netif_add()/ethernetif_init(). Capturing linkoutput at that point
        // saves nullptr, and later initialization overwrites our input hooks.
        // Check actual readiness on the TCP/IP thread, not just pointer identity
        // or an Arduino START event (which is delivered on another task).
        if (!netif_is_up(change->ethernet) || !netif_is_up(change->ap)
            || !change->ethernet->input || !change->ethernet->linkoutput
            || !change->ap->input || !change->ap->linkoutput) {
            return;
        }
    }

    client_count = 0;
    for (int i = 0; i < change->clients.num && i < ESP_WIFI_MAX_CONN_NUM; ++i)
        learn_client(change->clients.sta[i].mac);

    if (same_ports && (!eth_port || (eth_port->input == queue_input
        && eth_port->linkoutput == host_output && ap_port->input == queue_input))) {
        change->ready = !eth_port || eth_output != nullptr;
        return;
    }

    clear_wifi_queue();
    if (eth_port) {
        // Initialization/restart can replace callbacks on the same netif.
        // Restore only hooks that we still own; preserve new driver callbacks.
        if (eth_port->input == queue_input)
            eth_port->input = eth_input;
        if (eth_port->linkoutput == host_output)
            eth_port->linkoutput = eth_output;
        if (ap_port->input == queue_input)
            ap_port->input = ap_input;
    }
    eth_port = change->ethernet;
    ap_port = change->ap;
    if (eth_port) {
        eth_input = eth_port->input;
        eth_output = eth_port->linkoutput;
        ap_input = ap_port->input;
        eth_port->input = queue_input;
        ap_port->input = queue_input;
        eth_port->linkoutput = host_output;
        change->refresh_promiscuous = true;
    }
    change->ready = true;
}
}

bool ethernet_passthrough::start(esp_netif_obj *ethernet, esp_netif_obj *ap)
{
    if (!ethernet || !ap || !ETH.handle())
        return false;
    auto *eth = static_cast<netif *>(esp_netif_get_netif_impl(ethernet));
    auto *wifi = static_cast<netif *>(esp_netif_get_netif_impl(ap));
    if (!eth || !wifi)
        return false;

    Change change{};
    change.ethernet = eth;
    change.ap = wifi;
    if (esp_wifi_ap_get_sta_list(&change.clients) != ESP_OK) {
        stop();
        return false;
    }
    if (tcpip_callback_wait(change_ports, &change) != ERR_OK) {
        stop();
        return false;
    }
    if (!change.ready)
        return false;

    // Driver reinitialization may also have reset the hardware frame filter.
    if (change.refresh_promiscuous)
        owns_promiscuous = false;
    // Receive frames addressed to clients, not just the ESP32's own MAC.
    if (!owns_promiscuous) {
        bool promiscuous = true;
        if (esp_eth_ioctl(ETH.handle(), ETH_CMD_S_PROMISCUOUS, &promiscuous) != ESP_OK) {
            stop();
            return false;
        }
        owns_promiscuous = true;
    }
    return true;
}

bool ethernet_passthrough::stop()
{
    Change change{};
    if (tcpip_callback_wait(change_ports, &change) != ERR_OK)
        return false;
    if (owns_promiscuous && ETH.handle()) {
        bool promiscuous = false;
        esp_eth_ioctl(ETH.handle(), ETH_CMD_S_PROMISCUOUS, &promiscuous);
    }
    owns_promiscuous = false;
    return true;
}
#endif
