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
    if (network.is_mdns_started()) {
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
                            eebus.update_peers_config();
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

    // SHIP 7.2 Service Name
    mdns_service_add(NULL, "_ship", "_tcp", SHIP_PORT, NULL, 0);

    // SHIP 7.3.2 TXT Record
    // Mandatory Fields
    mdns_service_txt_item_set("_ship", "_tcp", "txtvers", "1");

    mdns_service_txt_item_set("_ship", "_tcp", "id", eebus.get_eebus_name().c_str());
    // ManufaturerName-Model-UniqueID (max 63 bytes)
    mdns_service_txt_item_set("_ship", "_tcp", "path", "/ship/");
    mdns_service_txt_item_set("_ship", "_tcp", "ski", eebus.state.get("ski")->asEphemeralCStr());
    // 40 byte hexadecimal digits representing the 160 bit SKI value

    mdns_service_txt_item_set("_ship", "_tcp", "register", "false");
    // Optional Fields
    mdns_service_txt_item_set("_ship", "_tcp", "brand", OPTIONS_MANUFACTURER_USER_AGENT());
    mdns_service_txt_item_set("_ship", "_tcp", "model", api.getState("info/name")->get("type")->asEphemeralCStr());
    mdns_service_txt_item_set("_ship", "_tcp", "type", EEBUS_DEVICE_TYPE); // Or EVSE?

    eebus.trace_fmtln("setup_mdns() done");
}

ShipDiscoveryState Ship::discover_ship_peers()
{
    if (discovery_state == ShipDiscoveryState::Scanning) {
        return discovery_state;
    }

    auto update_discovery_state = [this](ShipDiscoveryState state) {
        eebus.state.get("discovery_state")->updateEnum(state);
    };

    update_discovery_state(ShipDiscoveryState::Scanning);

    eebus.trace_fmtln("discover_ship_peers start");
    logger.printfln("EEBUS MDNS Discovery started");

    auto ip_to_string = [](const esp_ip_addr_t &ip) -> String {
        char buf[80];
        if (ip.type == ESP_IPADDR_TYPE_V4) {
            snprintf(buf, sizeof(buf), IPSTR, IP2STR(&ip.u_addr.ip4));
        } else {
            const uint8_t *bytes = reinterpret_cast<const uint8_t *>(&ip.u_addr.ip6);
            uint16_t hextets[8];
            for (int i = 0; i < 8; ++i) {
                hextets[i] = (static_cast<uint16_t>(bytes[2 * i]) << 8) | bytes[2 * i + 1];
            }

            // Find longest sequence of zero hextets (for :: compression)
            int best_start = -1;
            int best_len = 0;
            for (int i = 0; i < 8;) {
                if (hextets[i] == 0) {
                    int j = i;
                    while (j < 8 && hextets[j] == 0)
                        ++j;
                    int len = j - i;
                    if (len > best_len) {
                        best_start = i;
                        best_len = len;
                    }
                    i = j;
                } else {
                    ++i;
                }
            }
            if (best_len < 2) {
                best_start = -1; // do not compress sequences shorter than 2
                best_len = 0;
            }

            // Build shortened IPv6 string
            char *p = buf;
            size_t left = sizeof(buf);
            for (int i = 0; i < 8; ++i) {
                if (i == best_start) {
                    int n = snprintf(p, left, "::");
                    if (n < 0)
                        break;
                    p += n;
                    left = (left > (size_t)n ? left - n : 0);
                    i += best_len - 1;
                    continue;
                }

                int n = snprintf(p, left, "%x", hextets[i]); // no leading zeros, lowercase
                if (n < 0)
                    break;
                p += n;
                left = (left > (size_t)n ? left - n : 0);

                // Append ':' if not at end and next part isn't the compressed block
                if (i != 7 && !(best_start != -1 && i + 1 == best_start)) {
                    if (left > 1) {
                        *p = ':';
                        ++p;
                        --left;
                    }
                }
            }
            buf[sizeof(buf) - 1] = '\0';
        }
        return String(buf);
    };

    if (!network.is_mdns_started()) {
        logger.printfln("EEBUS MDNS Query Failed: mDNS is disabled or failed to start");
        eebus.trace_fmtln("EEBUS MDNS Query Failed; mDNS not started");
        update_discovery_state(ShipDiscoveryState::Error);
        return discovery_state;
    }

    const char *service = "_ship";
    const char *proto = "_tcp";
    mdns_result_t *results = NULL;
    esp_err_t err = mdns_query_ptr(service, proto, 1000, 20, &results);
    if (err) {
        logger.printfln("EEBUS MDNS Query Failed.");
        eebus.trace_fmtln("EEBUS MDNS Query Failed. Error %d", err);
        update_discovery_state(ShipDiscoveryState::Error);
        return discovery_state;
    }
    if (!results) {
        logger.printfln("EEBUS MDNS: No results found!");
        eebus.trace_fmtln("EEBUS MDNS: 0 results found!");
        update_discovery_state(ShipDiscoveryState::ScanDone);
        return discovery_state;
    }
    while (results) {
        String ip_address = "";
        String first_ip = "";
        while (results->addr) {
            ip_address = ip_to_string(results->addr->addr);
            if (first_ip.length() < 1) {
                first_ip = ip_address;
            }
            peer_handler.update_ip_by_ip(first_ip, ip_address);
            /*
            if (existing_node == nullptr) {
                existing_node = peer_handler.get_peer_by_ip(ip_address);
                logger.printfln("found existing node: %d", existing_node == nullptr);
                if (existing_node == nullptr) {
                    peer_handler.new_peer_from_ip(ip_address);
                    existing_node = peer_handler.get_peer_by_ip(ip_address);
                } else {
                    existing_node->ip_address.push_back(ip_address);
                }
            } else {
                if (!existing_node->contains_ip(ip_address)) {
                    existing_node->ip_address.push_back(ip_address);
                }
            }*/

            results->addr = results->addr->next;
        }
        ShipNode *existing_node = peer_handler.get_peer_by_ip(ip_address).get();
        if (ip_address.length() < 1 || existing_node == nullptr) {
            results = results->next;
            continue;
        }

        for (int i = 0; i < results->txt_count; i++) {
            mdns_txt_item_t *txt = &results->txt[i];
            if (txt->key == NULL || txt->value == NULL) {
                continue;
            }
            // mandatory fields
            if (strcmp(txt->key, "txtvers") == 0) {
                existing_node->txt_vers = txt->value;
                //peer_handler.update_vers_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "id") == 0) {
                existing_node->txt_id = txt->value;
                //peer_handler.update_id_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "path") == 0) {
                existing_node->txt_wss_path = txt->value;
                //peer_handler.update_wss_path_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "ski") == 0) {
                existing_node->txt_ski = txt->value;
                //peer_handler.update_ski_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "register") == 0) {
                existing_node->txt_autoregister = strcmp(txt->value, "true") == 0;
                // peer_handler.update_autoregister_by_ip(ip_address, strcmp(txt->value, "true") == 0);
                // Optional Fields
            } else if (strcmp(txt->key, "brand") == 0) {
                existing_node->txt_brand = txt->value;
                //peer_handler.update_brand_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "model") == 0) {
                existing_node->txt_model = txt->value;
                //peer_handler.update_model_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "type") == 0) {
                existing_node->txt_type = txt->value;
                //peer_handler.update_type_by_ip(ip_address, txt->value);
            }
            if (peer_handler.get_peer_by_ip(ip_address).get()->state != NodeState::Connected) {
                existing_node->state = NodeState::Discovered;
                //peer_handler.update_state_by_ip(ip_address, NodeState::Discovered);
            }
        }
        if (existing_node->txt_model.length() < 1)
            existing_node->txt_model = results->instance_name;
        existing_node->dns_name = String(results->hostname) + ".local";
        existing_node->port = results->port;

        results = results->next;
    }
    logger.printfln("EEBUS MDNS Discovery: Found %zu results", peer_handler.get_peers().size());

    mdns_query_results_free(results);
    update_discovery_state(ShipDiscoveryState::ScanDone);
    return discovery_state;
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
    size_t strs_len = dns_name.length() + txt_id.length() + txt_wss_path.length() + txt_ski.length()
                    + txt_brand.length() + txt_model.length() + txt_type.length();
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
    for (size_t i = 0; i < ip_address.size(); ++i) {
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

void ShipPeerHandler::update_ip_by_ski(const String &ski, const String &ip)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        if (!peer.get()->contains_ip(ip))
            peer.get()->ip_address.push_back(ip);
    } else {
        new_peer_from_ski(ski);
        update_ip_by_ski(ski, ip);
    }
}

void ShipPeerHandler::update_port_by_ski(const String &ski, uint16_t port)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer.get()->port = port;
    } else {
        new_peer_from_ski(ski);
        update_port_by_ski(ski, port);
    }
}

void ShipPeerHandler::update_trusted_by_ski(const String &ski, bool trusted)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer.get()->trusted = trusted;
    } else {
        new_peer_from_ski(ski);
        update_trusted_by_ski(ski, trusted);
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

/* --- update by ski --- */
void ShipPeerHandler::update_state_by_ski(const String &ski, NodeState state)
{
    if (const auto peer = get_peer_by_ski(ski).get()) {
        peer->state = state;
    } else {
        new_peer_from_ski(ski);
        update_state_by_ski(ski, state);
    }
}

void ShipPeerHandler::update_dns_name_by_ski(const String &ski, const String &dns_name)
{
    if (const auto peer = get_peer_by_ski(ski).get()) {
        peer->dns_name = dns_name;
    } else {
        new_peer_from_ski(ski);
        update_dns_name_by_ski(ski, dns_name);
    }
}

void ShipPeerHandler::update_vers_by_ski(const String &ski, const String &txt_vers)
{
    if (const auto peer = get_peer_by_ski(ski).get()) {
        peer->txt_vers = txt_vers;
    } else {
        new_peer_from_ski(ski);
        update_vers_by_ski(ski, txt_vers);
    }
}

void ShipPeerHandler::update_id_by_ski(const String &ski, const String &txt_id)
{
    if (const auto peer = get_peer_by_ski(ski).get()) {
        peer->txt_id = txt_id;
    } else {
        new_peer_from_ski(ski);
        update_id_by_ski(ski, txt_id);
    }
}

void ShipPeerHandler::update_wss_path_by_ski(const String &ski, const String &txt_wss_path)
{
    if (const auto peer = get_peer_by_ski(ski).get()) {
        peer->txt_wss_path = txt_wss_path;
    } else {
        new_peer_from_ski(ski);
        update_wss_path_by_ski(ski, txt_wss_path);
    }
}

void ShipPeerHandler::update_autoregister_by_ski(const String &ski, bool autoregister)
{
    if (const auto peer = get_peer_by_ski(ski).get()) {
        peer->txt_autoregister = autoregister;
    } else {
        new_peer_from_ski(ski);
        update_autoregister_by_ski(ski, autoregister);
    }
}

void ShipPeerHandler::update_brand_by_ski(const String &ski, const String &brand)
{
    if (const auto peer = get_peer_by_ski(ski).get()) {
        peer->txt_brand = brand;
    } else {
        new_peer_from_ski(ski);
        update_brand_by_ski(ski, brand);
    }
}

void ShipPeerHandler::update_model_by_ski(const String &ski, const String &model)
{
    if (const auto peer = get_peer_by_ski(ski).get()) {
        peer->txt_model = model;
    } else {
        new_peer_from_ski(ski);
        update_model_by_ski(ski, model);
    }
}

void ShipPeerHandler::update_type_by_ski(const String &ski, const String &type)
{
    if (const auto peer = get_peer_by_ski(ski).get()) {
        peer->txt_type = type;
    } else {
        new_peer_from_ski(ski);
        update_type_by_ski(ski, type);
    }
}

/* --- update by ip --- */
void ShipPeerHandler::update_port_by_ip(const String &ip, uint16_t port)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->port = port;
    } else {
        new_peer_from_ip(ip);
        update_port_by_ip(ip, port);
    }
}

void ShipPeerHandler::update_trusted_by_ip(const String &ip, bool trusted)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->trusted = trusted;
    } else {
        new_peer_from_ip(ip);
        update_trusted_by_ip(ip, trusted);
    }
}

void ShipPeerHandler::update_state_by_ip(const String &ip, NodeState state)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->state = state;
    } else {
        new_peer_from_ip(ip);
        update_state_by_ip(ip, state);
    }
}

void ShipPeerHandler::update_dns_name_by_ip(const String &ip, const String &dns_name)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->dns_name = dns_name;
    } else {
        new_peer_from_ip(ip);
        update_dns_name_by_ip(ip, dns_name);
    }
}

void ShipPeerHandler::update_vers_by_ip(const String &ip, const String &txt_vers)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->txt_vers = txt_vers;
    } else {
        new_peer_from_ip(ip);
        update_vers_by_ip(ip, txt_vers);
    }
}

void ShipPeerHandler::update_id_by_ip(const String &ip, const String &txt_id)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->txt_id = txt_id;
    } else {
        new_peer_from_ip(ip);
        update_id_by_ip(ip, txt_id);
    }
}

void ShipPeerHandler::update_wss_path_by_ip(const String &ip, const String &txt_wss_path)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->txt_wss_path = txt_wss_path;
    } else {
        new_peer_from_ip(ip);
        update_wss_path_by_ip(ip, txt_wss_path);
    }
}

void ShipPeerHandler::update_ski_by_ip(const String &ip, const String &txt_ski)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->txt_ski = txt_ski;
    } else {
        new_peer_from_ip(ip);
        update_ski_by_ip(ip, txt_ski);
    }
}

void ShipPeerHandler::update_autoregister_by_ip(const String &ip, bool autoregister)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->txt_autoregister = autoregister;
    } else {
        new_peer_from_ip(ip);
        update_autoregister_by_ip(ip, autoregister);
    }
}

void ShipPeerHandler::update_brand_by_ip(const String &ip, const String &brand)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->txt_brand = brand;
    } else {
        new_peer_from_ip(ip);
        update_brand_by_ip(ip, brand);
    }
}

void ShipPeerHandler::update_model_by_ip(const String &ip, const String &model)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->txt_model = model;
    } else {
        new_peer_from_ip(ip);
        update_model_by_ip(ip, model);
    }
}

void ShipPeerHandler::update_type_by_ip(const String &ip, const String &type)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        peer->txt_type = type;
    } else {
        new_peer_from_ip(ip);
        update_type_by_ip(ip, type);
    }
}
void ShipPeerHandler::update_ip_by_ip(const String &ip, const String &new_ip)
{
    if (const auto peer = get_peer_by_ip(ip).get()) {
        if (!peer->contains_ip(new_ip)) {
            peer->ip_address.push_back(new_ip);
        }
    } else {
        new_peer_from_ip(ip);
        update_ip_by_ip(ip, new_ip);
    }
}

void ShipPeerHandler::new_peer_from_ski(const String &ski)
{
    const std::shared_ptr<ShipNode> node = std::make_shared<ShipNode>();
    node->txt_ski = ski;
    peers.push_back(node);
}

void ShipPeerHandler::new_peer_from_ip(const String &ip)
{
    const std::shared_ptr<ShipNode> node = std::make_shared<ShipNode>();
    node->ip_address.push_back(ip);
    peers.push_back(node);
}
