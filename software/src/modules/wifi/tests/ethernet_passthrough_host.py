#!/usr/bin/env -S uv run --script
"""Exercise the production bridge on a host with fake drivers and queued lwIP input.

Run: python3 src/modules/wifi/tests/ethernet_passthrough_host.py
Requires g++; no device or PlatformIO libraries are needed.
"""

from pathlib import Path
import shutil
import subprocess
import tempfile


STUBS = r'''
#pragma once
#include <cstdint>
#include <cstring>
#include <deque>
#include <vector>
#include <cassert>
inline uint32_t fake_now = 0;
inline void (*timer_handler)(void *) = nullptr;
inline void *timer_arg = nullptr;
inline uint32_t sys_now() { return fake_now; }
inline void sys_timeout(unsigned, void (*fn)(void *), void *arg) { assert(!timer_handler); timer_handler = fn; timer_arg = arg; }
inline void sys_untimeout(void (*)(void *), void *) { timer_handler = nullptr; }
inline void tick() { fake_now += 5; auto fn = timer_handler; timer_handler = nullptr; if (fn) fn(timer_arg); }
inline void run_timers() { for (int i = 0; timer_handler && i < 1000; ++i) tick(); assert(!timer_handler); }
using err_t = int;
constexpr int ERR_OK = 0, ERR_IF = -12, ERR_MEM = -1, ERR_BUF = -2, ERR_ARG = -16, ESP_OK = 0;
using esp_err_t = int;
constexpr int ESP_ERR_NO_MEM = 0x101, ESP_ERR_INVALID_ARG = 0x102, ESP_ERR_INVALID_STATE = 0x103;
constexpr int ESP_ERR_ESP_NETIF_TX_FAILED = 0x500e;
inline int copying_tx_error = ESP_OK;
inline unsigned copying_tx_calls = 0;
constexpr int PBUF_RAW_TX = 0, PBUF_RAM = 0;
constexpr int ESP_WIFI_MAX_CONN_NUM = 15;
struct wifi_sta_info_t { uint8_t mac[6]; };
struct wifi_sta_list_t { wifi_sta_info_t sta[ESP_WIFI_MAX_CONN_NUM]; int num = 0; };
inline wifi_sta_list_t associated{};
inline bool fail_clients = false;
inline int esp_wifi_ap_get_sta_list(wifi_sta_list_t *result) {
    if (fail_clients) return ERR_IF;
    *result = associated;
    return ESP_OK;
}
constexpr int ETH_CMD_S_PROMISCUOUS = 1;
struct pbuf {
    uint16_t tot_len;
    std::vector<uint8_t> bytes;
    bool rx_buffer = false;
    unsigned refs = 1;
    void *payload = nullptr;
    bool bridge_copy = false;
};
inline int live_rx_buffers = 0, live_copies = 0;
inline int by_ref_calls = 0;
inline bool fail_copy = false, fail_wifi_tx = false;
inline pbuf *pbuf_clone(int, int, pbuf *packet) {
    if (fail_copy) return nullptr;
    auto *copy = new pbuf{packet->tot_len, packet->bytes};
    copy->payload = copy->bytes.data();
    copy->bridge_copy = true;
    ++live_copies;
    return copy;
}
struct netif;
using netif_input_fn = err_t (*)(pbuf *, netif *);
using netif_linkoutput_fn = err_t (*)(netif *, pbuf *);
struct netif {
    netif_input_fn input;
    netif_linkoutput_fn linkoutput;
    uint8_t hwaddr[6];
    bool up = true;
    bool link = true;
};
struct esp_netif_obj { netif *impl; };
using esp_netif_t = esp_netif_obj;
inline esp_netif_t *esp_netif_get_handle_from_netif_impl(void *port) {
    static esp_netif_t handle{};
    handle.impl = static_cast<netif *>(port);
    return &handle;
}
esp_err_t esp_netif_transmit(esp_netif_t *, void *, size_t);
inline bool netif_is_up(netif *port) { return port->up; }
inline bool netif_is_link_up(netif *port) { return port->link; }
inline void *esp_netif_get_netif_impl(esp_netif_obj *port) { return port->impl; }
struct FakeETH { void *handle() { return this; } };
inline FakeETH ETH;
inline bool promiscuous = false, fail_ioctl = false, fail_api = false;
inline int freed = 0;
inline netif *local_port = nullptr;
inline std::vector<std::vector<uint8_t>> local_frames, wired_frames, wifi_frames;
inline int esp_eth_ioctl(void *, int, bool *enabled) {
    if (fail_ioctl) return ERR_IF;
    promiscuous = *enabled;
    return ESP_OK;
}
inline int pbuf_copy_partial(pbuf *p, void *dst, int length, int offset) {
    int n = length < p->tot_len - offset ? length : p->tot_len - offset;
    memcpy(dst, p->bytes.data() + offset, n);
    return n;
}
inline void pbuf_free(pbuf *p) {
    assert(p->refs > 0);
    if (--p->refs == 0) {
        ++freed;
        if (p->rx_buffer) --live_rx_buffers;
        if (p->bridge_copy) --live_copies;
        delete p;
    }
}
inline err_t ethernet_input(pbuf *p, netif *port) {
    local_port = port;
    local_frames.push_back(p->bytes);
    // lwIP can turn an incoming ARP request into a reply in the same buffer.
    if (!p->bytes.empty()) p->bytes[0] ^= 0x10;
    pbuf_free(p);
    return ERR_OK;
}
struct Pending { pbuf *packet; netif *port; netif_input_fn handler; };
inline std::deque<Pending> pending;
inline err_t tcpip_inpkt(pbuf *p, netif *port, netif_input_fn handler) {
    pending.push_back({p, port, handler});
    return ERR_OK;
}
inline unsigned synchronous_callbacks = 0;
inline err_t tcpip_callback_wait(void (*fn)(void *), void *data) {
    if (fail_api) { fail_api = false; return ERR_IF; }
    // Packet delivery may happen ahead of a queued callback. Match TCP/IP
    // queue ordering instead of executing all control calls ahead of packets.
    while (!pending.empty()) {
        auto item = pending.front();
        pending.pop_front();
        assert(item.handler(item.packet, item.port) == ERR_OK);
    }
    fn(data);
    ++synchronous_callbacks;
    return ERR_OK; // Synchronous completion, as required by the public API.
}
'''


TEST = r'''
#include "stubs.h"
#include "ethernet_passthrough.h"

err_t original_input(pbuf *packet, netif *port) { return ethernet_input(packet, port); }
err_t wired_output(netif *, pbuf *packet) { wired_frames.push_back(packet->bytes); return ERR_OK; }
struct Retained { pbuf *packet; std::vector<uint8_t> bytes; };
std::vector<Retained> wifi_pending;
err_t wifi_output(netif *, pbuf *packet) {
    ++by_ref_calls;
    assert(!packet->rx_buffer);
    if (fail_wifi_tx || wifi_pending.size() >= 8) return ERR_MEM;
    wifi_frames.push_back(packet->bytes);
    ++packet->refs; // Match esp_wifi_internal_tx_by_ref on PSRAM builds.
    wifi_pending.push_back({packet, packet->bytes});
    return ERR_OK;
}
esp_err_t esp_netif_transmit(esp_netif_t *driver, void *data, size_t length) {
    ++copying_tx_calls;
    assert(driver->impl->linkoutput == wifi_output); // Correct AP I/O driver.
    if (fail_wifi_tx) return ESP_ERR_NO_MEM;
    if (copying_tx_error != ESP_OK) return copying_tx_error;
    auto *bytes = static_cast<uint8_t *>(data);
    wifi_frames.emplace_back(bytes, bytes + length); // Driver consumes a copy now.
    return ESP_OK; // No pbuf reference is retained in the legacy cache.
}

void complete_wifi_tx() {
    for (const auto &item : wifi_pending) {
        assert(item.packet->bytes == item.bytes); // Still the original frame.
        pbuf_free(item.packet);
    }
    wifi_pending.clear();
}

void drain()
{
    while (!pending.empty()) {
        auto item = pending.front();
        pending.pop_front();
        assert(item.handler(item.packet, item.port) == ERR_OK);
    }
}

std::vector<uint8_t> frame(const uint8_t *destination, uint16_t type)
{
    std::vector<uint8_t> bytes(64, 0x42);
    memcpy(bytes.data(), destination, 6);
    const uint8_t source[] = {2, 0, 0, 0, 0, 3};
    memcpy(bytes.data() + 6, source, sizeof(source));
    bytes[12] = type >> 8;
    bytes[13] = type & 0xff;
    return bytes;
}

void receive(netif &port, const std::vector<uint8_t> &bytes)
{
    ++live_rx_buffers;
    assert(port.input(new pbuf{static_cast<uint16_t>(bytes.size()), bytes, true}, &port) == ERR_OK);
    drain();
    run_timers();
    assert(live_rx_buffers == 0 && live_copies == 0);
    assert(by_ref_calls == 0 && wifi_pending.empty());
}

void clear_frames() { complete_wifi_tx(); wired_frames.clear(); wifi_frames.clear(); local_frames.clear(); }

int main()
{
    netif wired{original_input, wired_output, {2, 0, 0, 0, 0, 1}};
    netif wifi{original_input, wifi_output, {2, 0, 0, 0, 0, 2}};
    esp_netif_obj eth{&wired}, ap{&wifi};
    const uint8_t client[] = {2, 0, 0, 0, 0, 3};
    const uint8_t broadcast[] = {255, 255, 255, 255, 255, 255};
    const uint8_t multicast[] = {0x33, 0x33, 0, 0, 0, 1};
    const uint8_t control[] = {1, 0x80, 0xc2, 0, 0, 0};

    // Reproduce ETH.netif() becoming visible before asynchronous ETH.begin()
    // initializes its callbacks. All phases reuse exactly the same netif object.
    wired.input = nullptr;
    wired.linkoutput = nullptr;
    wired.up = false;
    assert(!ethernet_passthrough::start(&eth, &ap));
    assert(wifi.input == original_input && !promiscuous);
    wired.input = original_input;
    assert(!ethernet_passthrough::start(&eth, &ap));
    assert(wifi.input == original_input && wired.linkoutput == nullptr);
    wired.linkoutput = wired_output;
    assert(!ethernet_passthrough::start(&eth, &ap)); // callbacks ready, netif not yet up
    wired.up = true;
    wifi.up = false;
    assert(!ethernet_passthrough::start(&eth, &ap));
    wifi.up = true;

    assert(!ethernet_passthrough::start(nullptr, &ap));
    fail_ioctl = true;
    assert(!ethernet_passthrough::start(&eth, &ap));
    assert(wired.input == original_input && !promiscuous);
    fail_ioctl = false;
    fail_api = true;
    assert(!ethernet_passthrough::start(&eth, &ap));
    assert(wired.input == original_input && !promiscuous);

    associated.num = 1;
    memcpy(associated.sta[0].mac, client, 6);
    assert(ethernet_passthrough::start(&eth, &ap));
    assert(ethernet_passthrough::start(&eth, &ap)); // Idempotent periodic updates.
    assert(promiscuous);

    // A driver reinitialization may replace callbacks without changing the
    // netif pointer. A subsequent poll must reinstall and capture valid hooks.
    wired.input = original_input;
    wired.linkoutput = wired_output;
    promiscuous = false; // The driver can reset its hardware frame filter too.
    assert(ethernet_passthrough::start(&eth, &ap));
    assert(wired.input != original_input && wired.linkoutput != wired_output && promiscuous);

    for (uint16_t type : {0x0800, 0x0806, 0x86dd, 0x8100, 0x88b5}) {
        clear_frames();
        auto bytes = frame(client, type);
        receive(wifi, bytes);
        assert(wired_frames == std::vector<std::vector<uint8_t>>{bytes});
        assert(wifi_frames.empty() && local_frames.empty());
        clear_frames();
        receive(wired, bytes);
        assert(wifi_frames == std::vector<std::vector<uint8_t>>{bytes});
        assert(wired_frames.empty() && local_frames.empty());
    }
    for (auto destination : {broadcast, multicast}) {
        for (auto port : {&wired, &wifi}) {
            clear_frames();
            auto bytes = frame(destination, 0x86dd);
            receive(*port, bytes);
            assert(local_frames == std::vector<std::vector<uint8_t>>{bytes} && local_port == &wired);
            assert((port == &wifi ? wired_frames : wifi_frames) == std::vector<std::vector<uint8_t>>{bytes});
            assert((port == &wifi ? wifi_frames : wired_frames).empty());
        }
    }
    for (auto destination : {wired.hwaddr, wifi.hwaddr}) {
        clear_frames();
        receive(wifi, frame(destination, 0x0800));
        assert(local_frames.size() == 1 && local_port == &wired);
        assert(wired_frames.empty() && wifi_frames.empty());
    }
    clear_frames();
    auto reply = frame(client, 0x0806);
    pbuf response{static_cast<uint16_t>(reply.size()), reply};
    wired.linkoutput(&wired, &response);
    run_timers();
    assert(wired_frames == std::vector<std::vector<uint8_t>>{reply} && wifi_frames == std::vector<std::vector<uint8_t>>{reply});
    response.bytes[0] ^= 0x10; // The caller may reuse its buffer after output.

    clear_frames();
    receive(wifi, frame(control, 0x0800));
    receive(wired, std::vector<uint8_t>(5));
    assert(wired_frames.empty() && wifi_frames.empty() && local_frames.empty());
    wired.link = false;
    receive(wifi, frame(client, 0x0800));
    assert(wired_frames.empty());
    wired.link = true;
    wifi.link = false;
    receive(wired, frame(client, 0x0800));
    assert(wifi_frames.empty());
    wifi.link = true;

    // Hundreds of connect/disconnect cycles: don't transmit LAN frames while
    // empty, don't keep stale MACs, and never pin Ethernet RX buffers in WiFi.
    for (int connection = 0; connection < 200; ++connection) {
        associated.num = 0;
        assert(ethernet_passthrough::start(&eth, &ap));
        clear_frames();
        receive(wired, frame(broadcast, 0x0800));
        assert(wifi_frames.empty());

        associated.num = 1;
        memcpy(associated.sta[0].mac, client, 6);
        associated.sta[0].mac[5] = 4;
        assert(ethernet_passthrough::start(&eth, &ap));
        clear_frames();
        receive(wired, frame(client, 0x0800));
        assert(wifi_frames.empty()); // Disconnected client must not get LAN TX.
        auto new_client = frame(associated.sta[0].mac, 0x0800);
        receive(wired, new_client);
        assert(wifi_frames == std::vector<std::vector<uint8_t>>{new_client});
        clear_frames();
        receive(wired, frame(broadcast, 0x0806)); // Asynchronous TX + local ARP.
        assert(wifi_frames.size() == 1 && local_frames.size() == 1);
    }

    // A station's first DHCP packet may precede the scheduled association event.
    associated.num = 0;
    assert(ethernet_passthrough::start(&eth, &ap));
    clear_frames();
    receive(wifi, frame(broadcast, 0x0800));
    receive(wired, frame(client, 0x0800));
    assert(wifi_frames.size() == 1);

    clear_frames();
    fail_copy = true;
    receive(wired, frame(client, 0x0800));
    assert(wifi_frames.empty() && live_rx_buffers == 0);
    fail_copy = false;
    fail_wifi_tx = true;
    receive(wired, frame(client, 0x0800));
    assert(wifi_frames.empty() && live_rx_buffers == 0);
    fail_wifi_tx = false;
    receive(wired, frame(client, 0x0800));
    assert(wifi_frames.size() == 1); // Recovery after transient TX/memory errors.

    // More than the legacy 8-static/128-cached TX slots, without servicing the
    // reference-backed cache. The copying API must not put any packets there.
    clear_frames();
    for (int packet = 0; packet < 2000; ++packet)
        receive(wired, frame(client, 0x0800));
    assert(wifi_frames.size() == 2000 && by_ref_calls == 0 && live_copies == 0);

    // Burst saturation must remain bounded and must not keep calling a full
    // driver for each arriving Ethernet frame. Only timer ticks retry it.
    copying_tx_error = ESP_ERR_NO_MEM;
    auto before_calls = copying_tx_calls;
    auto before_frames = wifi_frames.size();
    for (int packet = 0; packet < 351; ++packet) {
        assert(wired.input(new pbuf{64, frame(client, 0x0800)}, &wired) == ERR_OK);
        drain();
    }
    assert(live_copies == 32 && copying_tx_calls == before_calls);
    tick();
    assert(live_copies == 32 && copying_tx_calls == before_calls + 1);
    copying_tx_error = ESP_OK;
    run_timers();
    assert(live_copies == 0 && wifi_frames.size() == before_frames + 32);

    // A permanently full driver expires queued packets; stop cancels timers
    // and frees retained copies even with a retry pending.
    copying_tx_error = ESP_ERR_NO_MEM;
    receive(wired, frame(client, 0x0800));
    assert(live_copies == 0 && !timer_handler);
    assert(wired.input(new pbuf{64, frame(client, 0x0800)}, &wired) == ERR_OK);
    drain();
    assert(live_copies == 1 && timer_handler);
    assert(ethernet_passthrough::stop());
    assert(live_copies == 0 && !timer_handler);
    assert(ethernet_passthrough::start(&eth, &ap));

    copying_tx_error = ESP_OK;
    receive(wifi, frame(broadcast, 0x0800)); // Learn the re-associated client.
    copying_tx_error = ESP_ERR_INVALID_ARG;
    receive(wired, frame(broadcast, 0x0800));
    copying_tx_error = ESP_ERR_ESP_NETIF_TX_FAILED;
    receive(wired, frame(client, 0x0800));
    copying_tx_error = ESP_ERR_INVALID_STATE;
    receive(wired, frame(client, 0x0800));
    copying_tx_error = ESP_OK;

    // A queued frame must remain safe if the mode changes before processing it.
    wifi.input(new pbuf{64, reply}, &wifi);
    ethernet_passthrough::stop();
    assert(!promiscuous && wired.input == original_input && wifi.input == original_input);
    assert(wired.linkoutput == wired_output);
    drain();
    // The queued frame precedes stop in the TCP/IP mailbox and is processed
    // while the bridge is still active, before stop restores the callbacks.
    assert(wired.input == original_input && wifi.input == original_input);
    ethernet_passthrough::stop();
    assert(ethernet_passthrough::start(&eth, &ap));
    ethernet_passthrough::stop();
    complete_wifi_tx();
    assert(synchronous_callbacks > 0);
    assert(freed > 0 && pending.empty() && live_rx_buffers == 0);
    assert(ethernet_passthrough::start(&eth, &ap));
    fail_clients = true;
    assert(!ethernet_passthrough::start(&eth, &ap));
    assert(!promiscuous && wired.input == original_input && wifi.input == original_input);
    fail_clients = false;
    assert(ethernet_passthrough::start(&eth, &ap));
    fail_api = true;
    assert(!ethernet_passthrough::stop());
    assert(wired.input != original_input && wifi.input != original_input && promiscuous);
    assert(ethernet_passthrough::stop());
}
'''


def main():
    here = Path(__file__).resolve().parent
    with tempfile.TemporaryDirectory(prefix='ethernet-passthrough-') as directory:
        root = Path(directory)
        (root / 'stubs.h').write_text(STUBS)
        for header in ('ETH.h', 'esp_wifi.h', 'esp_netif_net_stack.h', 'lwip/netif.h',
                       'lwip/tcpip.h', 'lwip/timeouts.h', 'netif/ethernet.h'):
            target = root / header
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_text('#include "stubs.h"\n')
        (root / 'generated').mkdir()
        (root / 'generated/module_dependencies.h').write_text('#define MODULE_ETHERNET_AVAILABLE() 1\n')
        for source in ('ethernet_passthrough.cpp', 'ethernet_passthrough.h'):
            shutil.copyfile(here.parent / source, root / source)
        (root / 'test.cpp').write_text(TEST)
        executable = root / 'test'
        subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror',
                        '-fsanitize=address,undefined', '-fno-omit-frame-pointer',
                        '-I', str(root), str(root / 'ethernet_passthrough.cpp'),
                        str(root / 'test.cpp'), '-o', str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print('Ethernet passthrough frame and lifecycle tests passed')


if __name__ == '__main__':
    main()
