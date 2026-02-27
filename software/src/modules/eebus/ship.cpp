/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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

#include <esp_crt_bundle.h>
#include <esp_http_server.h>

#include "build.h"
#include "event_log_prefix.h"

#include "cert_generator.h"
#include "module_dependencies.h"
#include "tools.h"
#include "tools/hexdump.h"
#include "tools/net.h"

static constexpr uint16_t SHIP_PORT = 4712;

static const char cert_begin[] = "-----BEGIN CERTIFICATE-----";

// Don't inline, keep buffers from hogging the stack.
[[gnu::noinline]]
static String extract_subject_key_id_hex(const Cert &crt)
{
    if (!crt.is_loaded()) {
        logger.printfln("Cannot extract subject key: Certificate not loaded");
        return String{};
    }

    const uint8_t *crt_data;
    size_t crt_len;
    const uint8_t *key_data;
    size_t key_len;

    crt.get_data(&crt_data, &crt_len, &key_data, &key_len);

    if (crt_len > std::size(cert_begin) && strncmp(reinterpret_cast<const char *>(crt_data), cert_begin, std::size(cert_begin) - 1) == 0) {
        crt_len++; // Include null-termination of PEM certificate
    }

    mbedtls_x509_crt x509_crt;

    mbedtls_x509_crt_init(&x509_crt);

    char ship_ski[41];
    size_t ship_ski_len;

    const int ret = mbedtls_x509_crt_parse(&x509_crt, crt_data, crt_len);
    if (ret != 0) {
        logger.printfln("Failed to parse certificate to retrieve SKI: -0x%04x", static_cast<unsigned>(-ret));
        eebus.trace_fmtln("Failed to parse certificate to retrieve SKI: -0x%04x", static_cast<unsigned>(-ret));
        ship_ski_len = 0;
    } else {
        ship_ski_len = hexdump(x509_crt.subject_key_id.p, x509_crt.subject_key_id.len, ship_ski, std::size(ship_ski), HexdumpCase::Lower);

        if (ship_ski_len != 40) {
            logger.printfln("SKI from certificate has unexpected length: %zu", ship_ski_len);
        }
    }

    mbedtls_x509_crt_free(&x509_crt);

    return String{ship_ski, ship_ski_len};
}

void Ship::pre_setup()
{
    web_sockets.pre_setup();
}

void Ship::setup()
{
    // The extra port for SHIP connections must always be registered during setup, whether or not EEBUS is enabled.
    // This means that a restart is required to change the EEBUS certificate.

    WebServerExtraPortData *extra_ship_port = static_cast<WebServerExtraPortData *>(malloc(sizeof(WebServerExtraPortData)));
    *extra_ship_port = {
        .port = SHIP_PORT,
        .supports_user_authentication = false, // SHIP devices won't have local user credentials.
        .transport_mode = TransportMode::Secure,
        .cert_info =
            {
                .cert_id = static_cast<int16_t>(eebus.config.get("cert_id")->asInt()),
                .key_id = static_cast<int16_t>(eebus.config.get("key_id")->asInt()),
                .cert_path = "/eebus/cert",
                .key_path = "/eebus/key",
                .generator_fn = eebus_ship_certificate_generator_fn,
            },
        .next = nullptr,
    };

    server.register_extra_port(extra_ship_port);
    peer_handler.initialize_from_config();
}

void Ship::enable_ship()
{
    if (eebus.config.get("enable")->asBool()) {
        setup_wss();
        if (eebus.initialized) {
            setup_mdns();
        } else {
            // MDNS might not be set up properly if its done too early during startup
            task_scheduler.scheduleOnce(
                [this]() {
                    setup_mdns();
                },
                1_s);
        }
    }
}

void Ship::disable_ship()
{
    eebus.trace_fmtln("disable_ship start");
    for (auto &ship_connection : eebus.ship.ship_connections) {
        ship_connection->schedule_close(0_ms, "EEBUS disabled");
    }
    // If mDNS is not started now,
    // it was not started while enabling ship
    // -> We don't have to remove the service in this case
    if (network.is_mdns_started() && mdns_service_exists("_ship", "_tcp", NULL)) {
        mdns_service_remove("_ship", "_tcp");
    }

    eebus.trace_fmtln("disable_ship end");
}

void Ship::setup_wss()
{
    if (wss_registered) {
        return;
    }

    eebus.trace_fmtln("setup_wss_server start");

    if (!cert.is_loaded()) {
        const cert_load_info cert_info = {
            .cert_id = static_cast<int16_t>(eebus.config.get("cert_id")->asInt()),
            .key_id = static_cast<int16_t>(eebus.config.get("key_id")->asInt()),
            .cert_path = "/eebus/cert",
            .key_path = "/eebus/key",
            .generator_fn = eebus_ship_certificate_generator_fn,
        };
        cert.load_external_with_internal_fallback(&cert_info);

        const String ship_ski = extract_subject_key_id_hex(cert);

        if (ship_ski.isEmpty()) {
            return;
        }

        eebus.set_own_ski(ship_ski);
    }

    // Websocket initial connection handler
    web_sockets.onConnect_HTTPThread([this](WebSocketsClient *ws_client) {
        if (!eebus.is_enabled()) {
            return false;
        }

        const String peer_ip = tf_peer_address_of_sockfd(ws_client->getFd()).toString();
        CoolString peer_ski = "unknown";

        std::shared_ptr<ShipNode> node = peer_handler.get_peer_by_ip(peer_ip);
        if (node != nullptr) {
            peer_ski = node.get()->txt_ski;
            peer_handler.update_ip_by_ski(peer_ski, peer_ip, true); // Force the Ip to the front as its actually being used
        } else {
            eebus.trace_fmtln("New incoming SHIP connection from unknown peer %s", peer_ip.c_str());
            peer_handler.update_ip_by_ip(peer_ip, peer_ip);
            node = peer_handler.get_peer_by_ip(peer_ip);
        }
        peer_handler.update_state_by_ip(peer_ip, NodeState::Connected);
        eebus.trace_fmtln("WebSocketsClient connected from %s with SKI %s", peer_ip.c_str(), peer_ski.c_str());
        ship_connections.push_back(std::move(make_unique_psram<ShipConnection>(ws_client, node)));
        logger.printfln("New SHIP Client connected from %s", node->node_name().c_str());

        if (ws_client->setCtx(ship_connections.back().get()) != nullptr) {
            esp_system_abort("Clobbered previously set WebSocketsClient context");
        }
        task_scheduler.scheduleOnce(
            []() {
                eebus.ship.discover_ship_peers();
                eebus.update_peers_state();
            },
            2_s);

        return true;
    });

    web_sockets.onDisconnect_HTTPThread([this](WebSocketsClient *client, bool /*clean_close*/) {
        ShipConnection *ship_connection = static_cast<ShipConnection *>(client->getCtx());

        if (ship_connection == nullptr) {
            // Connection closed by us, ship connection context already gone.
            return;
        }

        client->setCtx(nullptr);
        ship_connection->ws_client = nullptr; // Connection already closed, can't use it anymore.
        ship_connection->schedule_close(0_ms, "Websocket disconnected");
    });

    // Websocket data received handler
    web_sockets.onBinaryDataReceived_HTTPThread([this](WebSocketsClient *client, httpd_ws_frame_t *ws_pkt) {
        if (!eebus.is_enabled()) {
            eebus.trace_fmtln("Error while receiving Websocket packet: EEBUS not enabled");
            return;
        }
        ShipConnection *ship_connection = static_cast<ShipConnection *>(client->getCtx());

        if (ship_connection == nullptr) {
            eebus.trace_fmtln("Error while receiving Websocket packet: No ShipConnection attached to WS client with fd %d", client->getFd());
            return;
        }
        ship_connection->frame_received(ws_pkt);
    });

    // Start websocket on the HTTPS server
    web_sockets.start("/ship/", "/eebus/ws", "ship", SHIP_PORT);
    logger.printfln("EEBUS SHIP started up and accepting connections");

    wss_registered = true;
}

void Ship::connect_trusted_peers()
{
#ifdef EEBUS_SHIP_AUTOCONNECT

    size_t peer_count = eebus.config.get("peers")->count();
    eebus.trace_fmtln("connect_trusted_peers start, %d peers configured", peer_count);
    int trusted_peer_count = 0;
    for (size_t i = 0; i < peer_count; i++) {
        auto peer = eebus.config.get("peers")->get(i);
        if (peer->get("trusted")->asBool() && peer->get("state")->asEnum<NodeState>() == NodeState::Discovered) {
            trusted_peer_count++;
            tf_websocket_client_config_t websocket_cfg = {};
            CoolString peer_ski = peer->get("ski")->asString();
            CoolString ip = peer->get("ip")->asString();
            websocket_cfg.host = ip.c_str();
            websocket_cfg.port = peer->get("port")->asUint();
            websocket_cfg.path = peer->get("wss_path")->asString().c_str();

            websocket_cfg.crt_bundle_attach = esp_crt_bundle_attach;
            websocket_cfg.use_global_ca_store = true;
            websocket_cfg.transport = WEBSOCKET_TRANSPORT_OVER_SSL;
            websocket_cfg.cert_pem = nullptr;

            // The pointer is stored and not the data so the data needs to be valid for the duration of the connection.
            cert.get_data(reinterpret_cast<const uint8_t **>(&websocket_cfg.client_cert), &websocket_cfg.client_cert_len, reinterpret_cast<const uint8_t **>(&websocket_cfg.client_key), &websocket_cfg.client_key_len);

            websocket_cfg.disable_auto_reconnect = true;

            websocket_cfg.subprotocol = "ship"; // SHIP 10.2
            websocket_cfg.skip_cert_common_name_check = true;
            websocket_cfg.cert_common_name = NULL;

            // An error still occurs here because something is wrong with the cert
            ship_connections.push_back(std::move(make_unique_psram<ShipConnection>(websocket_cfg, node)));
        }
    }
    logger.printfln("EEBUS SHIP: %d trusted peers configured", trusted_peer_count);
#endif
}

void Ship::setup_mdns()
{
    eebus.trace_fmtln("setup_mdns() start");

    if (!network.is_mdns_started()) {
        logger.printfln("EEBUS Ship mDNS setup failed: mDNS is disabled or failed to start.");
        eebus.trace_fmtln("setup_mdns() failed; mDNS not started");
        return;
    }
    int ret = 0;

    // SHIP 7.2 Service Name
    ret = mdns_service_add(NULL, "_ship", "_tcp", SHIP_PORT, NULL, 0);
    if (ret != ESP_OK) {
        logger.printfln("EEBUS Ship mDNS setup failed: Failed to add mDNS service. Error %d", ret);
        eebus.trace_fmtln("setup_mdns() failed; mdns_service_add returned %d", ret);
        return;
    }
    // SHIP 7.3.2 TXT Record
    // Mandatory Fields
    ret = mdns_service_txt_item_set("_ship", "_tcp", "txtvers", "1");
    if (ret != ESP_OK) {
        logger.printfln("EEBUS Ship mDNS setup failed: Failed to set txtvers TXT record. Error %d", ret);
        eebus.trace_fmtln("setup_mdns() failed; mdns_service_txt_item_set for txtvers returned %d", ret);
        return;
    }
    ret = mdns_service_txt_item_set("_ship", "_tcp", "id", eebus.get_eebus_name().c_str());
    if (ret != ESP_OK) {
        logger.printfln("EEBUS Ship mDNS setup failed: Failed to set id TXT record. Error %d", ret);
        eebus.trace_fmtln("setup_mdns() failed; mdns_service_txt_item_set for id returned %d", ret);
        return;
    }
    // ManufaturerName-Model-UniqueID (max 63 bytes)
    ret = mdns_service_txt_item_set("_ship", "_tcp", "path", "/ship/");
    if (ret != ESP_OK) {
        logger.printfln("EEBUS Ship mDNS setup failed: Failed to set path TXT record. Error %d", ret);
        eebus.trace_fmtln("setup_mdns() failed; mdns_service_txt_item_set for path returned %d", ret);
        return;
    }
    ret = mdns_service_txt_item_set("_ship", "_tcp", "ski", eebus.state.get("ski")->asEphemeralCStr());
    if (ret != ESP_OK) {
        logger.printfln("EEBUS Ship mDNS setup failed: Failed to set ski TXT record. Error %d", ret);
        eebus.trace_fmtln("setup_mdns() failed; mdns_service_txt_item_set for ski returned %d", ret);
        return;
    }
    // 40 byte hexadecimal digits representing the 160 bit SKI value

    ret = mdns_service_txt_item_set("_ship", "_tcp", "register", "false");
    if (ret != ESP_OK) {
        logger.printfln("EEBUS Ship mDNS setup failed: Failed to set register TXT record. Error %d", ret);
        eebus.trace_fmtln("setup_mdns() failed; mdns_service_txt_item_set for register returned %d", ret);
        return;
    }
    // Optional Fields
    mdns_service_txt_item_set("_ship", "_tcp", "brand", OPTIONS_MANUFACTURER_USER_AGENT());
    mdns_service_txt_item_set("_ship", "_tcp", "model", OPTIONS_PRODUCT_NAME());
    mdns_service_txt_item_set("_ship", "_tcp", "type", EEBUS_DEVICE_TYPE); // Or EVSE?

    eebus.trace_fmtln("setup_mdns() done");
}

void Ship::check_mdns_results_cb(mdns_search_once_t *)
{
    task_scheduler.scheduleOnce([]() {
        eebus.ship.check_mdns_results();
    });
}
void Ship::check_mdns_results()
{
    mdns_result_t *results;
    auto query_results = mdns_query_async_get_results(mdns_scan,0, &results, nullptr);

    if (!query_results) {
        eebus.trace_fmtln("EEBUS MDNS: 0 results found!");
        update_discovery_state(ShipDiscoveryState::ScanDone);
        return;
    }
    while (results) {
        String ip_address = "";
        std::vector<String> ip_addresses{};
        char buf[INET6_ADDRSTRLEN];
        while (results->addr) {
            tf_ipaddr_ntoa(&results->addr->addr, buf, sizeof(buf));
            ip_addresses.push_back(String(buf));
            results->addr = results->addr->next;
        }
        if (ip_addresses.empty()) {
            results = results->next;
            continue;
        }
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

        for (int i = 0; i < results->txt_count; i++) {
            mdns_txt_item_t *txt = &results->txt[i];
            if (txt->key == NULL || txt->value == NULL) {
                continue;
            }
            // mandatory fields
            if (strcmp(txt->key, "txtvers") == 0) {
                txt_vers = txt->value;
                //peer_handler.update_vers_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "id") == 0) {
                txt_id = txt->value;
                //peer_handler.update_id_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "path") == 0) {
                txt_wss_path = txt->value;
                //peer_handler.update_wss_path_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "ski") == 0) {
                txt_ski = txt->value;
                //peer_handler.update_ski_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "register") == 0) {
                txt_autoregister = strcmp(txt->value, "true") == 0;
                // peer_handler.update_autoregister_by_ip(ip_address, strcmp(txt->value, "true") == 0);
                // Optional Fields
            } else if (strcmp(txt->key, "brand") == 0) {
                txt_brand = txt->value;
                //peer_handler.update_brand_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "model") == 0) {
                txt_model = txt->value;
                //peer_handler.update_model_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "type") == 0) {
                txt_type = txt->value;
                //peer_handler.update_type_by_ip(ip_address, txt->value);
            }
        }
        if (txt_model.length() < 1)
            txt_model = results->instance_name;
        dns_name = String(results->hostname) + ".local";
        port = results->port;
        if (txt_vers.isEmpty() || txt_id.isEmpty() || txt_wss_path.isEmpty() || txt_ski.isEmpty()) {
            eebus.trace_fmtln("Peer with IP %s missing mandatory TXT records, skipping", ip_addresses.front().c_str());
            results = results->next;
            continue;
        }

        // Check if we already have a peer with this SKI
        auto existing_ski_peer = peer_handler.get_peer_by_ski(txt_ski);

        // Check if we have a peer with matching IP but different/empty SKI (from SHIP connection)
        // If so, merge them by updating the existing peer's SKI instead of creating a duplicate
        for (const String &ip : ip_addresses) {
            auto ip_peer = peer_handler.get_peer_by_ip(ip);
            if (ip_peer != nullptr && ip_peer->txt_ski != txt_ski) {
                // Found a peer with matching IP but different SKI - merge them
                eebus.trace_fmtln("Merging peer with IP %s: updating SKI from %s to %s", ip.c_str(), ip_peer->txt_ski.c_str(), txt_ski.c_str());
                ip_peer->txt_ski = txt_ski;
                // Mark as discovered (non-persistent) since it came from mDNS
                ip_peer->persistent = false;
                existing_ski_peer = ip_peer;
                break;
            }
        }

        // This is pretty slow as we have to search for the peer again everytime, could be optimized by adding, getting and then setting the values
        auto *peer = peer_handler.get_or_create_by_ski(txt_ski);
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
        if (peer->state != NodeState::Connected) {
            peer->state = NodeState::Discovered;
        }

        results = results->next;
    }

    mdns_query_results_free(results);
    update_discovery_state(ShipDiscoveryState::ScanDone);
    eebus.update_peers_state();
}
void Ship::update_discovery_state(ShipDiscoveryState state)
{
    eebus.state.get("discovery_state")->updateEnum(state);
}

void Ship::discover_ship_peers()
{
    if (discovery_state == ShipDiscoveryState::Scanning) {
        return;
    }
    update_discovery_state(ShipDiscoveryState::Scanning);

    eebus.trace_fmtln("discover_ship_peers start");

    if (!network.is_mdns_started()) {
        logger.printfln("EEBUS MDNS Query Failed: mDNS is disabled or failed to start");
        eebus.trace_fmtln("EEBUS MDNS Query Failed; mDNS not started");
        update_discovery_state(ShipDiscoveryState::Error);
        return;
    }

    const char *service = "_ship";
    const char *proto = "_tcp";
    mdns_scan = mdns_query_async_new(NULL, service, proto, MDNS_TYPE_PTR, 1000, INT8_MAX, &check_mdns_results_cb);

    if (!mdns_scan) {
        logger.printfln("EEBUS MDNS Query Failed.");
        update_discovery_state(ShipDiscoveryState::Error);
    }
}

void Ship::print_skis(StringBuilder *sb)
{
    auto peers = peer_handler.get_peers();
    for (uint16_t i = 0; i < peers.size(); i++) {
        peers[i]->as_json(sb);
        sb->putc(',');
    }
}

void Ship::remove(const ShipConnection &ship_connection)
{
    // ship_connections is a vector of unique_ptr, so comparing with a reference won't work directly.
    ship_connections.erase(std::remove_if(ship_connections.begin(),
                                          ship_connections.end(),
                                          [&ship_connection](const unique_ptr_any<ShipConnection> &ptr) {
                                              return ptr.get() == &ship_connection;
                                          }),
                           ship_connections.end());
    // The unique_ptr will be destroyed here and the memory will be freed.
}

void ShipNode::as_json(StringBuilder *sb)
{
    size_t strs_len = dns_name.length() + txt_id.length() + txt_wss_path.length() + txt_ski.length() + txt_brand.length() + txt_model.length() + txt_type.length();
    size_t ips_len = 0;
    for (const String &ip : ip_address) {
        ips_len += ip.length();
    }
    const size_t capacity = JSON_OBJECT_SIZE(12) + JSON_ARRAY_SIZE(ip_address.size()) + strs_len + ips_len + 128;
    DynamicJsonDocument doc(capacity);
    doc["name"] = dns_name.c_str();
    doc["id"] = txt_id.c_str();
    doc["ws_path"] = txt_wss_path.c_str();
    doc["ski"] = txt_ski.c_str();
    doc["allow_autoregister"] = txt_autoregister;
    doc["device_manufacturer"] = txt_brand.c_str();
    doc["device_model"] = txt_model.c_str();
    doc["device_type"] = txt_type.c_str();
    doc["trusted"] = trusted;
    doc["port"] = port;
    doc["state"] = static_cast<uint8_t>(state);

    JsonArray arr = doc.createNestedArray("ip_address");
    for (const String &ip : ip_address) {
        arr.add(ip.c_str());
    }

    size_t len = measureJson(doc);
    char *buf = new char[len + 1];
    serializeJson(doc, buf, len + 1);

    sb->puts(buf);
    delete[] buf;
}

String ShipNode::ip_address_as_string() const
{
    String ip_concat;
    size_t len = ip_address.size();
    if (len > 3) {
        len = 3;
    } // Maximum return of the first 3 ips
    for (size_t i = 0; i < len; ++i) {
        if (i > 0) {
            ip_concat += ";";
        }
        ip_concat += ip_address[i];
    }
    return ip_concat;
}

String ShipNode::node_name() const
{

    if (!dns_name.isEmpty() && !txt_ski.isEmpty()) {
        return dns_name + ", SKI: " + txt_ski;
    }
    if (!ip_address.empty()) {
        return ip_address.front();
    }
    return "";
}

ShipPeerHandler::ShipPeerHandler()
{
    peers.clear();
}

std::shared_ptr<ShipNode> ShipPeerHandler::get_peer_by_ski(const String &ski)
{
    auto it = std::find_if(peers.begin(), peers.end(), [&ski](const std::shared_ptr<ShipNode> &n) {
        return n->txt_ski == ski;
    });
    if (it != peers.end())
        return *it;
    return nullptr;
}

void ShipPeerHandler::remove_peer_by_ski(const String &ski)
{
    peers.erase(std::ranges::remove_if(peers,
                                       [&ski](const std::shared_ptr<ShipNode> &n) {
                                           return n->txt_ski == ski;
                                       })
                    .begin(),
                peers.end());
}

ShipNode *ShipPeerHandler::get_or_create_by_ski(const String &ski)
{
    if (auto peer = get_peer_by_ski(ski))
        return peer.get();
    new_peer_from_ski(ski);
    return peers.back().get();
}

void ShipPeerHandler::update_ip_by_ski(const String &ski, const String &ip, const bool force_front)
{
    auto *peer = get_or_create_by_ski(ski);
    if (!peer->contains_ip(ip)) {
        if (force_front) {
            peer->ip_address.insert(peer->ip_address.begin(), ip);
        } else {
            peer->ip_address.push_back(ip);
        }
    } else if (force_front && !peer->ip_address.empty() && peer->ip_address[0] != ip) {
        // Move existing ip to front
        auto it = std::find(peer->ip_address.begin(), peer->ip_address.end(), ip);
        if (it != peer->ip_address.end()) {
            std::rotate(peer->ip_address.begin(), it, it + 1);
        }
    }
}

std::shared_ptr<ShipNode> ShipPeerHandler::get_peer_by_ip(const String &ip)
{
    auto it = std::find_if(peers.begin(), peers.end(), [&ip](const std::shared_ptr<ShipNode> &n) {
        return std::any_of(n->ip_address.begin(), n->ip_address.end(), [&ip](const String &s) {
            return s == ip;
        });
    });
    if (it != peers.end())
        return *it;
    return nullptr;
}

void ShipPeerHandler::remove_peer_by_ip(const String &ip)
{
    peers.erase(std::ranges::remove_if(peers,
                                       [&ip](const std::shared_ptr<ShipNode> &n) {
                                           return std::any_of(n->ip_address.begin(), n->ip_address.end(), [&ip](const String &s) {
                                               return s == ip;
                                           });
                                       })
                    .begin(),
                peers.end());
}

ShipNode *ShipPeerHandler::get_or_create_by_ip(const String &ip)
{
    if (auto peer = get_peer_by_ip(ip))
        return peer.get();
    new_peer_from_ip(ip);
    return peers.back().get();
}

void ShipPeerHandler::update_state_by_ip(const String &ip, NodeState state)
{
    get_or_create_by_ip(ip)->state = state;
}

void ShipPeerHandler::update_ip_by_ip(const String &ip, const String &new_ip)
{
    auto *peer = get_or_create_by_ip(ip);
    if (!peer->contains_ip(new_ip)) {
        peer->ip_address.push_back(new_ip);
    }
}

void ShipPeerHandler::new_peer_from_ski(const String &ski)
{
    const std::shared_ptr<ShipNode> node = std::make_shared<ShipNode>();
    node->txt_ski = ski;
    node->persistent = false; // Discovered peers are non-persistent by default
    peers.push_back(node);
}

void ShipPeerHandler::new_peer_from_ip(const String &ip)
{
    const std::shared_ptr<ShipNode> node = std::make_shared<ShipNode>();
    node->ip_address.push_back(ip);
    node->persistent = false; // Discovered peers are non-persistent by default
    peers.push_back(node);
}
void ShipPeerHandler::initialize_from_config()
{
    auto config_peers = eebus.config.get("peers");

    const size_t peer_count = config_peers->count();
    for (size_t i = 0; i < peer_count; i++) {
        auto peer = config_peers->get(i);
        auto node = std::make_shared<ShipNode>();
        node->txt_ski = peer->get("ski")->asString();
        if (node->txt_ski.isEmpty()) {
            // Do not add empty SKIs
            continue;
        }
        // Check for duplicate SKIs - skip if peer already exists
        if (get_peer_by_ski(node->txt_ski) != nullptr) {
            continue;
        }
        node->trusted = peer->get("trusted")->asBool();
        node->port = static_cast<uint16_t>(peer->get("port")->asUint());
        node->state = NodeState::LoadedFromConfig;
        node->dns_name = peer->get("dns_name")->asString();
        node->txt_id = peer->get("id")->asString();
        node->txt_wss_path = peer->get("wss_path")->asString();
        node->txt_autoregister = peer->get("autoregister")->asBool();

        node->txt_brand = peer->get("model_brand")->asString();
        node->txt_model = peer->get("model_model")->asString();
        node->txt_type = peer->get("model_type")->asString();

        // Mark peers loaded from config as persistent
        node->persistent = true;

        String ip_list = peer->get("ip")->asString();
        size_t start = 0;
        int end = ip_list.indexOf(';');
        while (end != ip_list.lastIndexOf(';')) {
            node->ip_address.push_back(ip_list.substring(start, end));
            start = end + 1;
            end = ip_list.indexOf(';', start);
        }
        if (start < ip_list.length()) {
            node->ip_address.push_back(ip_list.substring(start));
        }

        peers.push_back(node);
    }
    // Update state API with all peers after loading from config
    eebus.update_peers_state();
}
