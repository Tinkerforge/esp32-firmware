/* esp32-firmware
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

#include "ship.h"

#include <algorithm>
#include <mdns.h>
#include <utility>
#include <vector>

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "eebus.h"
#include "tools.h"
#include "tools/net.h"

#include "options.h"
#include "gcc_warnings.h"

void Ship::setup_mdns()
{
#if EEBUS_TRACE_SUPER_VERBOSE()
    eebus.trace_fmtln("setup_mdns() start");
#endif
    if (!network.is_mdns_started()) {
        logger.printfln("SHIP mDNS setup failed.");
        eebus.trace_fmtln("setup_mdns() failed; mDNS not started");
        return;
    }

    // SHIP 7.2 Service Name
    int ret = mdns_service_add(NULL, "_ship", "_tcp", SHIP_PORT, NULL, 0);
    if (ret != ESP_OK) {
        logger.printfln("SHIP mDNS setup failed");
        eebus.trace_fmtln("setup_mdns() failed; mdns_service_add returned %d", ret);
        return;
    }

    // SHIP 7.3.2 TXT Record
    const String id = eebus.get_eebus_name();
    const struct {
        const char *key;
        const char *value;
    } mandatory_items[] = {
        {"txtvers", "1"},
        {"id", id.c_str()}, // ManufacturerName-Model-UniqueID (max 63 bytes)
        {"path", "/ship/"},
        {"ski", eebus.state.get("ski")->asEphemeralCStr()}, // 40 hexadecimal digits representing the 160 bit SKI value
        {"register", "false"},
    };
    for (const auto &item : mandatory_items) {
        ret = mdns_service_txt_item_set("_ship", "_tcp", item.key, item.value);
        if (ret != ESP_OK) {
            logger.printfln("SHIP mDNS setup failed");
            eebus.trace_fmtln("setup_mdns() failed; mdns_service_txt_item_set for %s returned %d", item.key, ret);
            return;
        }
    }

    // Optional Fields
    // [SRIP-220/2] Whitespace is prohibited in TXT values, [SRIP-220/3] recommends '-' instead.
    // Keep in sync with the QR code in web/src/modules/eebus/main.tsx.
    String brand = OPTIONS_MANUFACTURER();
    String model = OPTIONS_PRODUCT_NAME();
    brand.replace(' ', '-');
    model.replace(' ', '-');
    mdns_service_txt_item_set("_ship", "_tcp", "brand", brand.c_str());
    mdns_service_txt_item_set("_ship", "_tcp", "model", model.c_str());
    mdns_service_txt_item_set("_ship", "_tcp", "type", EEBUS_DEVICE_TYPE);
    mdns_service_txt_item_set("_ship", "_tcp", "cat", EEBUS_DEVICE_CATEGORY); // [SRIP-220/23]

#if EEBUS_TRACE_SUPER_VERBOSE()
    eebus.trace_fmtln("setup_mdns() done");
#endif
}

void Ship::check_mdns_results_cb(mdns_search_once_t *)
{
    task_scheduler.scheduleOnce([]() {
        if (!eebus.ship.is_enabled || !eebus.is_enabled()) {
            return;
        }
        eebus.ship.check_mdns_results();
    });
}
void Ship::check_mdns_results()
{
    if (!is_enabled || !eebus.is_enabled() || mdns_scan == nullptr) {
        return;
    }

    mdns_result_t *results;
    auto query_results = mdns_query_async_get_results(mdns_scan, 0, &results, nullptr);
    mdns_query_async_delete(mdns_scan);
    mdns_scan = nullptr;

    if (!query_results) {
        eebus.trace_fmtln("EEBUS MDNS: 0 results found!");
        update_discovery_state(ShipDiscoveryState::ScanDone);
        return;
    }
    // ESP-IDF may attach the resolved A/AAAA records to only one of several
    // results sharing a hostname, so collect all addresses per hostname first.
    std::vector<std::pair<String, std::vector<String>>> host_addresses;
    for (mdns_result_t *current = results; current != nullptr; current = current->next) {
        if (current->hostname == nullptr || current->addr == nullptr) {
            continue;
        }
        std::vector<String> *ips = nullptr;
        for (auto &entry : host_addresses) {
            if (entry.first == current->hostname) {
                ips = &entry.second;
                break;
            }
        }
        if (ips == nullptr) {
            host_addresses.emplace_back(current->hostname, std::vector<String>{});
            ips = &host_addresses.back().second;
        }
        char buf[INET6_ADDRSTRLEN];
        for (mdns_ip_addr_t *addr = current->addr; addr != nullptr; addr = addr->next) {
            tf_ipaddr_ntoa(&addr->addr, buf, sizeof(buf));
            String ip{buf};
            if (std::find(ips->begin(), ips->end(), ip) == ips->end()) {
                ips->push_back(ip);
            }
        }
    }

    // Refresh known peers before admitting any new ones.
    // A peer returning after a missed scan must not be evicted just because its result appears later.
    for (mdns_result_t *result = results; result != nullptr; result = result->next) {
        const char *ski = nullptr;
        bool has_version = false;
        bool has_id = false;
        bool has_path = false;
        for (size_t i = 0; i < result->txt_count; ++i) {
            const mdns_txt_item_t &txt = result->txt[i];
            if (txt.key == nullptr || txt.value == nullptr) {
                continue;
            }
            if (strcmp(txt.key, "ski") == 0) {
                ski = txt.value;
            } else if (strcmp(txt.key, "txtvers") == 0) {
                has_version = txt.value[0] != '\0';
            } else if (strcmp(txt.key, "id") == 0) {
                has_id = txt.value[0] != '\0';
            } else if (strcmp(txt.key, "path") == 0) {
                has_path = txt.value[0] != '\0';
            }
        }
        if (ski != nullptr && has_version && has_id && has_path) {
            if (auto *peer = peer_handler.get_peer_by_ski(ski)) {
                peer->missed_scans = 0;
            }
        }
    }

    mdns_result_t *current = results;
    std::vector<String> seen_skis;
    while (current) {
        std::vector<String> ip_addresses{};
        if (current->hostname != nullptr) {
            for (const auto &entry : host_addresses) {
                if (entry.first == current->hostname) {
                    ip_addresses = entry.second;
                    break;
                }
            }
        }
        // Results without addresses are not skipped: the TXT/SRV data still
        // identifies the peer by SKI and the IP may be learned elsewhere.
        String txt_vers;
        String txt_id;
        String txt_wss_path;
        String txt_ski;
        bool txt_autoregister = false;
        String txt_brand;
        String txt_model;
        String txt_type;
        String dns_name;
        uint16_t port;

        for (size_t i = 0; i < current->txt_count; i++) {
            mdns_txt_item_t *txt = &current->txt[i];
            if (txt->key == NULL || txt->value == NULL) {
                continue;
            }
            // mandatory fields
            if (strcmp(txt->key, "txtvers") == 0) {
                txt_vers = txt->value;
            } else if (strcmp(txt->key, "id") == 0) {
                txt_id = txt->value;
            } else if (strcmp(txt->key, "path") == 0) {
                txt_wss_path = txt->value;
            } else if (strcmp(txt->key, "ski") == 0) {
                txt_ski = txt->value;
            } else if (strcmp(txt->key, "register") == 0) {
                txt_autoregister = strcmp(txt->value, "true") == 0;
                // optional fields
            } else if (strcmp(txt->key, "brand") == 0) {
                txt_brand = txt->value;
            } else if (strcmp(txt->key, "model") == 0) {
                txt_model = txt->value;
            } else if (strcmp(txt->key, "type") == 0) {
                txt_type = txt->value;
            }
        }
        if (txt_model.length() < 1 && current->instance_name != nullptr)
            txt_model = current->instance_name;
        if (current->hostname != nullptr) {
            dns_name = String(current->hostname) + ".local";
        }
        port = current->port;
        if (txt_vers.isEmpty() || txt_id.isEmpty() || txt_wss_path.isEmpty() || txt_ski.isEmpty()) {
            eebus.trace_fmtln("Peer %s missing mandatory TXT records, skipping", dns_name.isEmpty() ? (ip_addresses.empty() ? "<unknown>" : ip_addresses.front().c_str()) : dns_name.c_str());
            current = current->next;
            continue;
        }

        // Match by SKI only: the SKI is the authoritative peer identity, not the IP.
        // A new SKI on a known IP is treated as a new peer; an IP may legitimately
        // be shared between several peers.
        auto *peer = peer_handler.get_or_create_by_ski(txt_ski, ShipPeerHandler::Source::MdnsDiscovery);
        if (peer == nullptr) {
            eebus.trace_fmtln("Ignoring discovered peer %s: peer limit reached", dns_name.isEmpty() ? txt_ski.c_str() : dns_name.c_str());
            current = current->next;
            continue;
        }
        seen_skis.push_back(txt_ski);
        peer->txt_vers = txt_vers;
        peer->txt_id = txt_id;
        peer->txt_wss_path = txt_wss_path;
        peer->txt_model = txt_model;
        peer->txt_type = txt_type;
        peer->txt_brand = txt_brand;
        peer->txt_autoregister = txt_autoregister;
        peer->dns_name = dns_name;
        peer->port = port;
        for (const String &ip : ip_addresses) {
            peer_handler.update_ip_by_ski(txt_ski, ip);
        }
        if (peer->state == NodeState::Disconnected || peer->state == NodeState::LoadedFromConfig) {
            peer->state = NodeState::Discovered;
        }

        current = current->next;
    }

    mdns_query_results_free(results);
    peer_handler.age_peers(seen_skis);
    update_discovery_state(ShipDiscoveryState::ScanDone);
    eebus.update_peers_state();
    connect_trusted_peers();
}
void Ship::update_discovery_state(ShipDiscoveryState new_state)
{
    discovery_state = new_state;
    eebus.state.get("discovery_state")->updateEnum(new_state);
}

void Ship::discover_ship_peers()
{
    if (!is_enabled || !eebus.is_enabled()) {
        return;
    }

    if (discovery_state == ShipDiscoveryState::Scanning) {
        return;
    }
    update_discovery_state(ShipDiscoveryState::Scanning);

#if EEBUS_TRACE_SUPER_VERBOSE()
    eebus.trace_fmtln("discover_ship_peers start");
#endif
    if (!network.is_mdns_started()) {
        logger.printfln("MDNS Query Failed: mDNS is disabled or failed to start");
        eebus.trace_fmtln("EEBUS MDNS Query Failed; mDNS not started");
        update_discovery_state(ShipDiscoveryState::Error);
        return;
    }

    const char *service = "_ship";
    const char *proto = "_tcp";
    mdns_scan = mdns_query_async_new(NULL, service, proto, MDNS_TYPE_PTR, 1000, INT8_MAX, &check_mdns_results_cb);

    if (!mdns_scan) {
        logger.printfln("MDNS Query Failed.");
        update_discovery_state(ShipDiscoveryState::Error);
    }
}
