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
#include <esp_https_server.h>

#include "build.h"
#include "esp_transport_ssl.h"
#include "event_log_prefix.h"

#include "module_dependencies.h"
#include "tools.h"

static constexpr uint16_t SHIP_PORT = 4712;

void Ship::pre_setup()
{
    web_sockets.pre_setup();
}

void Ship::setup()
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
        ship_connection->schedule_close(0_ms);
    }
    mdns_service_remove("_ship", "_tcp");
    // Delay closing the socket and httpd server so the connections can all be closed
    task_scheduler.scheduleOnce(
        [this]() {
            web_sockets.stop();
            httpd_ssl_stop(httpd);
            httpd = nullptr;
        },
        100_ms);

    eebus.trace_fmtln("disable_ship end");
}

void Ship::setup_wss()
{
    eebus.trace_fmtln("setup_wss_server start");

    // HTTPS server configuration.
    // This HTTPS server is just used to provide the send/recv for a secure websocket.
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();

    // HTTPD config
    // config.httpd.stack_size           = TODO;
    // config.httpd.max_uri_handlers     = TODO;
    config.httpd.lru_purge_enable = true;
    config.httpd.global_user_ctx = this;
    // httpd_stop calls free on the pointer passed as global_user_ctx if we don't override the free_fn.
    config.httpd.global_user_ctx_free_fn = [](void *foo) {
    };
    config.httpd.max_open_sockets = 3;
    config.httpd.enable_so_linger = true;
    config.httpd.linger_timeout = 100;
    // TODO: We could implement a mechanism that makes sure the ctrl ports are unique.
    //       By default, the ctrl port is 32768. If we have multiple instances of the
    //       httpd server running, we need to increment the ctrl port each time.
    config.httpd.ctrl_port = 32769;

    // SSL config
    config.transport_mode = HTTPD_SSL_TRANSPORT_SECURE;
    config.port_secure = SHIP_PORT;
    config.port_insecure = 0;

    // Lambda to parse the X509 certificate
    auto parse_x509_crt = [](const unsigned char *buf, size_t buflen) -> int {
        mbedtls_x509_crt x509_crt;
        mbedtls_x509_crt_init(&x509_crt);
        int ret = mbedtls_x509_crt_parse(&x509_crt, buf, buflen);
        if (ret != 0) {
            logger.printfln(" An error occurred while setting up the SHIP Websocket. EEBUS Failed to start");
            eebus.trace_fmtln("mbedtls_x509_crt_parse failed: 0x%04x", ret);
        } else {
            char ship_ski[64] = {0};
            for (size_t i = 0; i < x509_crt.subject_key_id.len; i++) {
                sprintf(&ship_ski[i * 2], "%02x", x509_crt.subject_key_id.p[i]);
            }
            eebus.state.get("ski")->updateString(ship_ski);
        }

        return ret;
    };

    const int32_t cert_id = eebus.config.get("cert_id")->asInt();
    const int32_t key_id = eebus.config.get("key_id")->asInt();

    // If both cert and key are set externally, we use them.
    // Oterwise we generate and use a self-signed certificate.
    if (cert_id != -1 && key_id != -1) {
        size_t cert_crt_len = 0;
        auto cert_crt = certs.get_cert(cert_id, &cert_crt_len);
        if (cert_crt == nullptr) {
            logger.printfln("Certificate with ID %ld is not available", cert_id);
            return;
        }

        size_t cert_key_len = 0;
        auto cert_key = certs.get_cert(key_id, &cert_key_len);
        if (cert_key == nullptr) {
            logger.printfln("Certificate with ID %ld is not available", key_id);
            return;
        }

        if (!parse_x509_crt(cert_crt.get(), cert_crt_len + 1)) {
            return;
        }

        config.servercert = cert_crt.release();
        config.servercert_len = cert_crt_len + 1; // +1 since the length must include the null terminator
        config.prvtkey_pem = cert_key.release();
        config.prvtkey_len = cert_key_len + 1; // +1 since the length must include the null terminator
    } else {
        if (cert == nullptr) {
            cert = make_unique_psram<Cert>();
        }
        if (!cert->read()) {
            logger.printfln("Failed to read self-signed certificate");
            return;
        }

        if (parse_x509_crt(cert->crt, cert->crt_length) != 0) {
            logger.printfln("An error occured while starting EEBUS SHIP Server");
            eebus.trace_fmtln("parse_x509_crt != 0");
            return;
        }

        config.servercert = cert->crt;
        config.servercert_len = cert->crt_length;
        config.prvtkey_pem = cert->key;
        config.prvtkey_len = cert->key_length;
    }

    // Start HTTPS server
    esp_err_t ret = httpd_ssl_start(&httpd, &config);
    if (ESP_OK != ret) {
        logger.printfln("Error starting EEBUS HTTPS server: %d", ret);
    }

    // Websocket initial connection handler
    web_sockets.onConnect_HTTPThread([this](WebSocketsClient ws_client) {
        if (!eebus.config.get("enable")->asBool()) {
            return false;
        }

        sockaddr_in6 addr;
        socklen_t addr_len = sizeof(addr);
        getpeername(ws_client.fd, (struct sockaddr *)&addr, &addr_len);

        char client_ip[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, &addr.sin6_addr, client_ip, sizeof(client_ip));

        CoolString peer_ski = "unknown";
        std::string peer_ip = client_ip;
        // need to strip out the IPv6 prefix if present
        std::string ip_ip6_start = "::FFFF:";
        auto peer_ip_pos = peer_ip.find(ip_ip6_start);
        if (peer_ip_pos != std::string::npos) {
            peer_ip = peer_ip.erase(peer_ip_pos, ip_ip6_start.length());
        }

        auto node = peer_handler.get_peer_by_ip(peer_ip.c_str());
        if (node != nullptr) {
            peer_ski = node->txt_ski;
        } else {
            eebus.trace_fmtln("New incoming SHIP connection from unknown peer %s", peer_ip.c_str());
            peer_handler.update_dns_name_by_ip(peer_ip.c_str(), peer_ip.c_str());
        }
        peer_handler.update_state_by_ip(peer_ip.c_str(), NodeState::Connected);
        eebus.trace_fmtln("WebSocketsClient connected from %s:%d with SKI %s", peer_ip.c_str(), ntohs(addr.sin6_port), peer_ski.c_str());

        ship_connections.push_back(std::move(make_unique_psram<ShipConnection>(ws_client, peer_ski)));
        logger.printfln("New SHIP Client connected from %s", peer_ip.c_str());

        return true;
    });

    // Websocket data received handler
    web_sockets.onBinaryDataReceived_HTTPThread([this](const int fd, httpd_ws_frame_t *ws_pkt) {
        if (!eebus.is_enabled()) {
            eebus.trace_fmtln("Error while receiving Websocket packet: EEBUS not enabled");

            return;
        }
        for (auto &ship_connection : ship_connections) {
            if (ship_connection->ws_client.fd == fd) {
                ship_connection->frame_received(ws_pkt);
                return;
            }
        }
        eebus.trace_fmtln("Error while receiving Websocket packet: No ShipConnection found for fd %d", fd);
    });

    // Start websocket on the HTTPS server
    web_sockets.start("/ship/", nullptr, httpd, "ship");
    logger.printfln("EEBUS SHIP started up and accepting connections");
}

void Ship::connect_trusted_peers()
{
#ifndef EEBUS_SHIP_AUTOCONNECT
    return;
#endif
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
            websocket_cfg.client_cert = reinterpret_cast<const char *>(cert->crt);
            websocket_cfg.client_cert_len = cert->crt_length;
            websocket_cfg.client_key = reinterpret_cast<const char *>(cert->key);
            websocket_cfg.client_key_len = cert->key_length;
            websocket_cfg.disable_auto_reconnect = true;

            websocket_cfg.subprotocol = "ship"; // SHIP 10.2
            websocket_cfg.skip_cert_common_name_check = true;
            websocket_cfg.cert_common_name = NULL;

            // An error still occurs here because something is wrong with the cert
            ship_connections.push_back(std::move(make_unique_psram<ShipConnection>(websocket_cfg, peer_ski)));

        }
    }
    logger.printfln("EEBUS SHIP: %d trusted peers configured", trusted_peer_count);
}

void Ship::setup_mdns()
{
    eebus.trace_fmtln("setup_mdns() start");

    // SHIP 7.2 Service Name
    mdns_service_add(NULL, "_ship", "_tcp", SHIP_PORT, NULL, 0);

    // SHIP 7.3.2 TXT Record
    // Mandatory Fields
    mdns_service_txt_item_set("_ship", "_tcp", "txtvers", "1");

    mdns_service_txt_item_set("_ship", "_tcp", "id", eebus.get_eebus_name().c_str()); // ManufaturerName-Model-UniqueID (max 63 bytes)
    mdns_service_txt_item_set("_ship", "_tcp", "path", "/ship/");
    mdns_service_txt_item_set("_ship", "_tcp", "ski", eebus.state.get("ski")->asEphemeralCStr()); // 40 byte hexadecimal digits representing the 160 bit SKI value

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

    const char *service = "_ship";
    const char *proto = "_tcp";
    mdns_result_t *results = NULL;
    esp_err_t err = mdns_query_ptr(service, proto, 3000, 20, &results);
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
        //ShipNode ship_node;
        //ship_node.dns_name = results->hostname;
        //ship_node.port = results->port;
        String ip_address{};
        while (results->addr) {
            if (ip_address.length() > 0) {
                eebus.trace_fmtln("Error in EEBUS MDNS: More than one IP Address found, this might cause issues.");
            }
            esp_ip_addr_t ip = results->addr->addr;
            if (ip.type == IPADDR_TYPE_V4) {
                ip_address = IPAddress(ip.u_addr.ip4.addr).toString().c_str();
            } else {
                eebus.trace_fmtln("MDNS returned ipv6 address");
                //ship_node.ip_address.push_back(IPAddress(ip.u_addr.ip6.addr));
                // TODO: Add IPv6 support. Convert the IP type from esp_ip6_addr to IPAddress
            }
            results->addr = results->addr->next;
        }
        if (ip_address.length() < 1) {
            continue;
        }
        // TODO: Maybe make some security checks? So no harmful data is ingested. Or does mdns library sanizite the data?
        for (int i = 0; i < results->txt_count; i++) {
            mdns_txt_item_t *txt = &results->txt[i];
            if (txt->key == NULL || txt->value == NULL) {
                continue;
            }
            // mandatory fields
            if (strcmp(txt->key, "txtvers") == 0) {
                peer_handler.update_vers_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "id") == 0) {
                peer_handler.update_id_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "path") == 0) {
                peer_handler.update_id_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "ski") == 0) {
                peer_handler.update_ski_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "register") == 0) {
                peer_handler.update_autoregister_by_ip(ip_address, strcmp(txt->value, "true") == 0);
                // Optional Fields
            } else if (strcmp(txt->key, "brand") == 0) {
                peer_handler.update_brand_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "model") == 0) {
                peer_handler.update_model_by_ip(ip_address, txt->value);
            } else if (strcmp(txt->key, "type") == 0) {
                peer_handler.update_type_by_ip(ip_address, txt->value);
            }
            if (peer_handler.get_peer_by_ip(ip_address)->state != NodeState::Connected)
                peer_handler.update_state_by_ip(ip_address, NodeState::Discovered);
        }
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
        peers[i].as_json(sb);
        sb->putc(',');
    }
}


void Ship::remove(const ShipConnection &ship_connection)
{
    // ship_connections is a vector of unique_ptr, so comparing with a reference won't work directly.
    ship_connections.erase(
        std::remove_if(
            ship_connections.begin(),
            ship_connections.end(),
            [&ship_connection](const unique_ptr_any<ShipConnection> &ptr) {
                return ptr.get() == &ship_connection;
            }
            ),
        ship_connections.end()
        );
    // The unique_ptr will be destroyed here and the memory will be freed.
}

void ShipNode::as_json(StringBuilder *sb)
{
    char json_buf[1024]; //TODO: Use 1024 for now, change later to dynamic size depending on struct size
    TFJsonSerializer json(json_buf, sizeof(json_buf));
    json.addMemberString("name", dns_name.c_str());
    json.addMemberString("id", txt_id.c_str());
    json.addMemberString("ws_path", txt_wss_path.c_str());
    json.addMemberString("ski", txt_ski.c_str());
    json.addMemberBoolean("allow_autoregister", txt_autoregister);
    json.addMemberString("device_manufacturer", txt_brand.c_str());
    json.addMemberString("device_model", txt_model.c_str());
    json.addMemberString("device_type", txt_type.c_str());
    json.addMemberBoolean("trusted", trusted);
    json.addMemberNumber("port", port);
    json.addMemberNumber("state", (uint8_t)state);
    json.addMemberString("ip_address", ip_address.c_str());
    json.end();
    sb->puts(json_buf);
}

ShipPeerHandler::ShipPeerHandler()
{
    peers.clear();
}

ShipNode *ShipPeerHandler::get_peer_by_ski(const String &ski)
{
    auto it = std::find_if(peers.begin(),
                           peers.end(),
                           [&ski](const ShipNode &n) {
                               return n.txt_ski == ski;
                           });
    if (it != peers.end())
        return &*it;
    return nullptr;
}

void ShipPeerHandler::remove_peer_by_ski(const String &ski)
{
    peers.erase(
        std::ranges::remove_if(peers,
                               [&ski](const ShipNode &n) {
                                   return n.txt_ski == ski;
                               }).begin(),
        peers.end());
}

void ShipPeerHandler::update_ip_by_ski(const String &ski, const String &ip)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->ip_address = ip;
    } else {
        new_peer_from_ski(ski);
        update_ip_by_ski(ski, ip);
    }
}

void ShipPeerHandler::update_port_by_ski(const String &ski, uint16_t port)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->port = port;
    } else {
        new_peer_from_ski(ski);
        update_port_by_ski(ski, port);
    }
}

void ShipPeerHandler::update_trusted_by_ski(const String &ski, bool trusted)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->trusted = trusted;
    } else {
        new_peer_from_ski(ski);
        update_trusted_by_ski(ski, trusted);
    }
}

ShipNode *ShipPeerHandler::get_peer_by_ip(const String &ip)
{
    auto it = std::find_if(peers.begin(),
                           peers.end(),
                           [&ip](const ShipNode &n) {
                               return n.ip_address == ip;
                           });
    if (it != peers.end())
        return &*it;
    return nullptr;
}

void ShipPeerHandler::remove_peer_by_ip(const String &ip)
{
    peers.erase(
        std::ranges::remove_if(peers,
                               [&ip](const ShipNode &n) {
                                   return n.ip_address == ip;
                               }).begin(),
        peers.end());
}

/* --- update by ski --- */
void ShipPeerHandler::update_state_by_ski(const String &ski, NodeState state)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->state = state;
    } else {
        new_peer_from_ski(ski);
        update_state_by_ski(ski, state);
    }
}

void ShipPeerHandler::update_dns_name_by_ski(const String &ski, const String &dns_name)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->dns_name = dns_name;
    } else {
        new_peer_from_ski(ski);
        update_dns_name_by_ski(ski, dns_name);
    }
}

void ShipPeerHandler::update_vers_by_ski(const String &ski, const String &txt_vers)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->txt_vers = txt_vers;
    } else {
        new_peer_from_ski(ski);
        update_vers_by_ski(ski, txt_vers);
    }
}

void ShipPeerHandler::update_id_by_ski(const String &ski, const String &txt_id)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->txt_id = txt_id;
    } else {
        new_peer_from_ski(ski);
        update_id_by_ski(ski, txt_id);
    }
}

void ShipPeerHandler::update_wss_path_by_ski(const String &ski, const String &txt_wss_path)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->txt_wss_path = txt_wss_path;
    } else {
        new_peer_from_ski(ski);
        update_wss_path_by_ski(ski, txt_wss_path);
    }
}

void ShipPeerHandler::update_autoregister_by_ski(const String &ski, bool autoregister)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->txt_autoregister = autoregister;
    } else {
        new_peer_from_ski(ski);
        update_autoregister_by_ski(ski, autoregister);
    }
}

void ShipPeerHandler::update_brand_by_ski(const String &ski, const String &brand)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->txt_brand = brand;
    } else {
        new_peer_from_ski(ski);
        update_brand_by_ski(ski, brand);
    }
}

void ShipPeerHandler::update_model_by_ski(const String &ski, const String &model)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->txt_model = model;
    } else {
        new_peer_from_ski(ski);
        update_model_by_ski(ski, model);
    }
}

void ShipPeerHandler::update_type_by_ski(const String &ski, const String &type)
{
    if (const auto peer = get_peer_by_ski(ski)) {
        peer->txt_type = type;
    } else {
        new_peer_from_ski(ski);
        update_type_by_ski(ski, type);
    }
}

/* --- update by ip --- */
void ShipPeerHandler::update_port_by_ip(const String &ip, uint16_t port)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->port = port;
    } else {
        new_peer_from_ip(ip);
        update_port_by_ip(ip, port);
    }
}

void ShipPeerHandler::update_trusted_by_ip(const String &ip, bool trusted)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->trusted = trusted;
    } else {
        new_peer_from_ip(ip);
        update_trusted_by_ip(ip, trusted);
    }
}

void ShipPeerHandler::update_state_by_ip(const String &ip, NodeState state)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->state = state;
    } else {
        new_peer_from_ip(ip);
        update_state_by_ip(ip, state);
    }
}

void ShipPeerHandler::update_dns_name_by_ip(const String &ip, const String &dns_name)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->dns_name = dns_name;
    } else {
        new_peer_from_ip(ip);
        update_dns_name_by_ip(ip, dns_name);
    }
}

void ShipPeerHandler::update_vers_by_ip(const String &ip, const String &txt_vers)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->txt_vers = txt_vers;
    } else {
        new_peer_from_ip(ip);
        update_vers_by_ip(ip, txt_vers);
    }
}

void ShipPeerHandler::update_id_by_ip(const String &ip, const String &txt_id)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->txt_id = txt_id;
    } else {
        new_peer_from_ip(ip);
        update_id_by_ip(ip, txt_id);
    }
}

void ShipPeerHandler::update_wss_path_by_ip(const String &ip, const String &txt_wss_path)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->txt_wss_path = txt_wss_path;
    } else {
        new_peer_from_ip(ip);
        update_wss_path_by_ip(ip, txt_wss_path);
    }
}

void ShipPeerHandler::update_ski_by_ip(const String &ip, const String &txt_ski)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->txt_ski = txt_ski;
    } else {
        new_peer_from_ip(ip);
        update_ski_by_ip(ip, txt_ski);
    }
}

void ShipPeerHandler::update_autoregister_by_ip(const String &ip, bool autoregister)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->txt_autoregister = autoregister;
    } else {
        new_peer_from_ip(ip);
        update_autoregister_by_ip(ip, autoregister);
    }
}

void ShipPeerHandler::update_brand_by_ip(const String &ip, const String &brand)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->txt_brand = brand;
    } else {
        new_peer_from_ip(ip);
        update_brand_by_ip(ip, brand);
    }
}

void ShipPeerHandler::update_model_by_ip(const String &ip, const String &model)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->txt_model = model;
    } else {
        new_peer_from_ip(ip);
        update_model_by_ip(ip, model);
    }
}

void ShipPeerHandler::update_type_by_ip(const String &ip, const String &type)
{
    if (const auto peer = get_peer_by_ip(ip)) {
        peer->txt_type = type;
    } else {
        new_peer_from_ip(ip);
        update_type_by_ip(ip, type);
    }
}

void ShipPeerHandler::new_peer_from_ski(const String &ski)
{
    ShipNode node = {};
    node.txt_ski = ski;
    peers.push_back(node);
}

void ShipPeerHandler::new_peer_from_ip(const String &ip)
{
    ShipNode node = {};
    node.ip_address = ip;
    peers.push_back(node);
}
