#!/usr/bin/env -S uv run --script
"""Test the production NAT reply routing hook, including linker wrapping.

Run with python3; requires g++, no device or PlatformIO installation.
"""

from pathlib import Path
import shutil
import subprocess
import tempfile


STUBS = r'''
#pragma once
#include <cstdint>
struct ip4_addr_t { uint32_t addr; };
struct netif {
    netif *next;
    ip4_addr_t ip, mask;
    bool up = true, link = true;
#if IP_NAPT
    bool napt = false;
#endif
};
extern netif *netif_list;
#define NETIF_FOREACH(port) for ((port) = netif_list; (port); (port) = (port)->next)
inline bool netif_is_up(netif *port) { return port->up; }
inline bool netif_is_link_up(netif *port) { return port->link; }
inline const ip4_addr_t *netif_ip4_addr(netif *port) { return &port->ip; }
inline const ip4_addr_t *netif_ip4_netmask(netif *port) { return &port->mask; }
inline bool ip4_addr_isany(const ip4_addr_t *ip) { return ip->addr == 0; }
inline bool ip4_addr_eq(const ip4_addr_t *a, const ip4_addr_t *b) { return a->addr == b->addr; }
inline bool ip4_addr_netcmp(const ip4_addr_t *a, const ip4_addr_t *b, const ip4_addr_t *mask) {
    return (a->addr & mask->addr) == (b->addr & mask->addr);
}
inline bool ip4_addr_ismulticast(const ip4_addr_t *ip) { return (ip->addr & 0xf0000000) == 0xe0000000; }
inline bool ip4_addr_isbroadcast(const ip4_addr_t *ip, netif *port) {
    return ip->addr == 0xffffffff || (ip->addr | port->mask.addr) == 0xffffffff;
}
extern "C" netif *ip4_route_src_hook(const ip4_addr_t *, const ip4_addr_t *);
extern int fallback_calls;
'''

ORIGINAL = r'''
#include "lwip/netif.h"
netif *netif_list = nullptr;
int fallback_calls = 0;
// Match ESP-IDF's source-first default hook, in a separate translation unit.
extern "C" netif *ip4_route_src_hook(const ip4_addr_t *source, const ip4_addr_t *) {
    ++fallback_calls;
    if (source && !ip4_addr_isany(source)) {
        netif *port;
        NETIF_FOREACH(port) {
            if (netif_is_up(port) && netif_is_link_up(port) && ip4_addr_eq(source, &port->ip))
                return port;
        }
    }
    return nullptr;
}
'''

TEST = r'''
#include "lwip/netif.h"
#include <cassert>
int main() {
    netif ap{nullptr, {0x0a000001}, {0xffffff00}};
    netif sta{&ap, {0xac100002}, {0xffffff00}};
    netif eth{&sta, {0xc0a80105}, {0xffffff00}};
    netif_list = &eth;
    const ip4_addr_t client{0x0a000002}, internet{0x08080808};
#if IP_NAPT
    const ip4_addr_t any{0}, multicast{0xef000001}, limited_broadcast{0xffffffff}, ap_broadcast{0x0a0000ff};
    ap.napt = true;
    // Reproduces HTTP/HTTPS/WebSocket reply routing from the Ethernet IP.
    assert(ip4_route_src_hook(&eth.ip, &client) == &ap);
    assert(ip4_route_src_hook(&sta.ip, &client) == &ap);
    assert(ip4_route_src_hook(&ap.ip, &client) == &ap);
    assert(fallback_calls == 0); // Verifies --wrap is actually intercepting calls.

    // Forwarded client traffic and NAT-translated replies retain normal routing.
    assert(ip4_route_src_hook(&client, &internet) == nullptr);
    assert(ip4_route_src_hook(&internet, &client) == nullptr);
    assert(ip4_route_src_hook(nullptr, &client) == nullptr);
    assert(ip4_route_src_hook(&any, &client) == nullptr);
    assert(ip4_route_src_hook(&eth.ip, &internet) == &eth);
    assert(ip4_route_src_hook(&sta.ip, &internet) == &sta);
    assert(ip4_route_src_hook(&eth.ip, &multicast) == &eth);
    assert(ip4_route_src_hook(&eth.ip, &limited_broadcast) == &eth);
    assert(ip4_route_src_hook(&eth.ip, &ap_broadcast) == &eth);
    assert(ip4_route_src_hook(&eth.ip, &ap.ip) == &eth);
    assert(fallback_calls == 10);

    // Stop/restart NAT, switch to passthrough, and change the AP subnet.
    ap.napt = false;
    assert(ip4_route_src_hook(&eth.ip, &client) == &eth);
    ap.napt = true;
    ap.up = false;
    assert(ip4_route_src_hook(&eth.ip, &client) == &eth);
    ap.up = true;
    ap.link = false;
    assert(ip4_route_src_hook(&eth.ip, &client) == &eth);
    ap.link = true;
    ap.ip.addr = 0;
    assert(ip4_route_src_hook(&eth.ip, &client) == &eth);
    ap.ip.addr = 0x0a000101;
    assert(ip4_route_src_hook(&eth.ip, &client) == &eth);
    const ip4_addr_t new_client{0x0a000102};
    assert(ip4_route_src_hook(&eth.ip, &new_client) == &ap);
#else
    // Libraries without NAPT keep their original behavior; struct netif has no
    // napt member in this build, just as with the pinned non-NAPT libraries.
    assert(ip4_route_src_hook(&eth.ip, &client) == &eth);
    assert(ip4_route_src_hook(&eth.ip, &internet) == &eth);
    assert(ip4_route_src_hook(nullptr, &client) == nullptr);
    assert(fallback_calls == 3);
#endif
}
'''


def main():
    here = Path(__file__).resolve().parent
    with tempfile.TemporaryDirectory(prefix='nat-routing-') as directory:
        root = Path(directory)
        (root / 'lwip').mkdir()
        (root / 'lwip/netif.h').write_text(STUBS)
        (root / 'generated').mkdir()
        (root / 'generated/module_dependencies.h').write_text('#define MODULE_ETHERNET_AVAILABLE() 1\n')
        shutil.copyfile(here.parent / 'ethernet_nat_routing.cpp', root / 'routing.cpp')
        (root / 'original.cpp').write_text(ORIGINAL)
        (root / 'test.cpp').write_text(TEST)
        for napt in (0, 1):
            executable = root / f'test-{napt}'
            subprocess.run(['g++', '-std=c++17', '-Wall', '-Wextra', '-Werror',
                            '-fsanitize=undefined', f'-DIP_NAPT={napt}', '-I', str(root),
                            str(root / 'routing.cpp'), str(root / 'original.cpp'),
                            str(root / 'test.cpp'), '-Wl,--wrap=ip4_route_src_hook',
                            '-o', str(executable)], check=True)
            subprocess.run([str(executable)], check=True)
    print('NAT reply routing and linker wrapping tests passed (NAPT enabled and disabled)')


if __name__ == '__main__':
    main()
