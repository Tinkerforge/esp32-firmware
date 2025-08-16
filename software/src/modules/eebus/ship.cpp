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

#include <esp_https_server.h>

#include "build.h"
#include "event_log_prefix.h"

#include "module_dependencies.h"
#include "tools.h"

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);

void Ship::pre_setup()
{
    web_sockets.pre_setup(); // Moved to setup_wss(), is this needed this early?
}

void Ship::setup()
{
    if (eebus.config.get("enable")->asBool()) {
#ifdef SHIP_USE_INTERNAL_CERTS
        eebus.state.get("ski")->updateString(ship_ski);
#endif
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
    logger.tracefln(eebus.trace_buffer_index, "disable_ship start");
    for (ShipConnection &conn : ship_connections) {
        conn.schedule_close(0_ms);
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

    logger.tracefln(eebus.trace_buffer_index, "disable_ship end");
}

void Ship::setup_wss()
{
    logger.tracefln(eebus.trace_buffer_index, "setup_wss_server start"); // TODO Move to tracelog

    // HTTPS server configuration.
    // This HTTPS server is just used to provide the send/recv for a secure websocket.
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();

    // HTTPD config
    // config.httpd.stack_size           = TODO;
    // config.httpd.max_uri_handlers     = TODO;
    config.httpd.lru_purge_enable = true;
    config.httpd.global_user_ctx = this;
    // httpd_stop calls free on the pointer passed as global_user_ctx if we don't override the free_fn.
    config.httpd.global_user_ctx_free_fn = [](void *foo) {};
    config.httpd.max_open_sockets = 3;
    config.httpd.enable_so_linger = true;
    config.httpd.linger_timeout = 100;
    // TODO: We could implement a mechanism that makes sure the ctrl ports are unique.
    //       By default, the ctrl port is 32768. If we have multiple instances of the
    //       httpd server running, we need to increment the ctrl port each time.
    config.httpd.ctrl_port = 32769;

    // SSL config
    config.transport_mode = HTTPD_SSL_TRANSPORT_SECURE;
    config.port_secure = 4712;
    config.port_insecure = 0;

    // Lambda to parse the X509 certificate
    auto parse_x509_crt = [](const unsigned char *buf, size_t buflen) -> int {
        mbedtls_x509_crt x509_crt;
        mbedtls_x509_crt_init(&x509_crt);
        int ret = mbedtls_x509_crt_parse(&x509_crt, buf, buflen);
        if (ret != 0) {
            logger.printfln(" An error occurred while setting up the SHIP Websocket");
            logger.tracefln(eebus.trace_buffer_index, "mbedtls_x509_crt_parse failed: 0x%04x", ret);
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

        // TODO: This is only for debugging, remove later
        //cert->log();

        if (parse_x509_crt(cert->crt, cert->crt_length) != 0) {
            logger.printfln("An error occured while starting EEBUS SHIP Server");
            logger.tracefln(eebus.trace_buffer_index, "parse_x509_crt != 0");
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

        for (size_t i = 0; i < eebus.config.get("peers")->count(); i++) {
            if (peer_ip.find(eebus.config.get("peers")->get(i)->get("ip")->asString().c_str()) != std::string::npos) {
                peer_ski = eebus.config.get("peers")->get(i)->get("ski")->asString();
                break;
            }
        }
        logger.tracefln(eebus.trace_buffer_index,
                        "WebSocketsClient connected from %s:%d with SKI %s",
                        client_ip,
                        ntohs(addr.sin6_port),
                        peer_ski.c_str());
        ship_connections.push_back(ShipConnection{ws_client, ShipConnection::Role::Server, peer_ski});
        logger.printfln("New SHIP Client connected");

        return true;
    });

    web_sockets.onBinaryDataReceived_HTTPThread([this](const int fd, httpd_ws_frame_t *ws_pkt) {
        if (!eebus.config.get("enable")->asBool()) {
            return;
        }
        for (auto &ship_connection : ship_connections) {
            if (ship_connection.ws_client.fd == fd) {
                ship_connection.frame_received(ws_pkt);
                return;
            }
        }

        logger.tracefln(eebus.trace_buffer_index, "Error while receiving Websocket packet: No ShipConnection found for fd %d", fd);
    });

    // Start websocket on the HTTPS server

    web_sockets.start("/ship/", nullptr, httpd, "ship");

    logger.printfln("EEBUS SHIP started up and accepting connections");
}

void Ship::setup_mdns()
{
    logger.tracefln(eebus.trace_buffer_index, "setup_mdns() start");

    // SHIP 7.2 Service Name
    mdns_service_add(NULL, "_ship", "_tcp", 4712, NULL, 0);

    // SHIP 7.3.2 TXT Record
    // Mandatory Fields
    mdns_service_txt_item_set("_ship", "_tcp", "txtvers", "1");
    // TODO: Use UID instead of 12345

    mdns_service_txt_item_set("_ship", "_tcp", "id", eebus.get_eebus_name().c_str()); // ManufaturerName-Model-UniqueID (max 63 bytes)
    mdns_service_txt_item_set("_ship", "_tcp", "path", "/ship/");
    mdns_service_txt_item_set("_ship",
                              "_tcp",
                              "ski",
                              eebus.state.get("ski")->asEphemeralCStr()); // 40 byte hexadecimal digits representing the 160 bit SKI value

    mdns_service_txt_item_set("_ship", "_tcp", "register", "false");
    // Optional Fields
    mdns_service_txt_item_set("_ship", "_tcp", "brand", EEBUS_DEVICE_MANUFACTURER);
    mdns_service_txt_item_set("_ship", "_tcp", "model", EEBUS_DEVICE_MODEL);
    mdns_service_txt_item_set("_ship", "_tcp", "type", EEBUS_DEVICE_TYPE); // Or EVSE?

    logger.tracefln(eebus.trace_buffer_index, "setup_mdns() done");

}

Ship_Discovery_State Ship::discover_ship_peers()
{
    if (discovery_state == Ship_Discovery_State::SCANNING) {
        return discovery_state;
    }

    auto update_discovery_state = [this](Ship_Discovery_State state) {
        this->discovery_state = state;
        eebus.state.get("discovery_state")->updateUint(static_cast<uint8_t>(state));
    };

    update_discovery_state(Ship_Discovery_State::SCANNING);

    logger.tracefln(eebus.trace_buffer_index, "discover_ship_peers start");
    logger.printfln("EEBUS MDNS Discovery started");

    const char *service = "_ship";
    const char *proto = "_tcp";
    mdns_result_t *results = NULL;
    esp_err_t err = mdns_query_ptr(service, proto, 3000, 20, &results);
    if (err) {
        logger.printfln("EEBUS MDNS Query Failed.");
        logger.tracefln(eebus.trace_buffer_index, "EEBUS MDNS Query Failed. Error %d", err);
        update_discovery_state(Ship_Discovery_State::ERROR);
        return discovery_state;
    }
    if (!results) {
        logger.printfln("EEBUS MDNS: No results found!");
        logger.tracefln(eebus.trace_buffer_index, "EEBUS MDNS: 0 results found!");
        update_discovery_state(Ship_Discovery_State::SCAN_DONE);
        return discovery_state;
    }
    mdns_results.clear();
    while (results) {
        ShipNode ship_node;
        ship_node.dns_name = results->hostname;
        ship_node.port = results->port;

        // TODO: Maybe make some security checks? So no harmful data is ingested. Or does mdns library sanizite the data?
        for (int i = 0; i < results->txt_count; i++) {
            mdns_txt_item_t *txt = &results->txt[i];
            if (txt->key == NULL || txt->value == NULL) {
                continue;
            }
            // mandatory fields
            if (strcmp(txt->key, "txtvers") == 0) {
                ship_node.txt_vers = txt->value;
            } else if (strcmp(txt->key, "id") == 0) {
                ship_node.txt_id = txt->value;
            } else if (strcmp(txt->key, "path") == 0) {
                ship_node.txt_wss_path = txt->value;
            } else if (strcmp(txt->key, "ski") == 0) {
                ship_node.txt_ski = txt->value;
            } else if (strcmp(txt->key, "register") == 0) {
                ship_node.txt_autoregister = strcmp(txt->value, "true") == 0;
                // Optional Fields
            } else if (strcmp(txt->key, "brand") == 0) {
                ship_node.txt_brand = txt->value;
            } else if (strcmp(txt->key, "model") == 0) {
                ship_node.txt_model = txt->value;
            } else if (strcmp(txt->key, "type") == 0) {
                ship_node.txt_type = txt->value;
            }
        }
        // Add IP adress to ship_node. Can be multiple IPv4 and IPv6 addresses
        while (results->addr) {
            esp_ip_addr_t ip = results->addr->addr;
            if (ip.type == IPADDR_TYPE_V4) {
                ship_node.ip_addresses.push_back(IPAddress(ip.u_addr.ip4.addr));
            } else {
                //ship_node.ip_addresses.push_back(IPAddress(ip.u_addr.ip6.addr));
                // TODO: Add IPv6 support. Convert the IP type from esp_ip6_addr to IPAddress
            }
            results->addr = results->addr->next;
        }

        mdns_results.push_back(ship_node);

        results = results->next;
    }

    logger.printfln("EEBUS MDNS Discovery: Found %d results", mdns_results.size());

    mdns_query_results_free(results);
    update_discovery_state(Ship_Discovery_State::SCAN_DONE);
    return discovery_state;
}

void Ship::print_skis(StringBuilder *sb)
{
    for (uint16_t i = 0; i < mdns_results.size(); i++) {
        mdns_results[i].as_json(sb);
        sb->putc(',');
    }
}

void Ship::remove(const ShipConnection &ship_connection)
{
    ship_connections.erase(std::remove(ship_connections.begin(), ship_connections.end(), ship_connection), ship_connections.end());
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

    StringBuilder ip_sb;
    ip_sb.putc('[');
    for (IPAddress ip : ip_addresses) {
        ip_sb.puts(ip.toString().c_str());
        ip_sb.putc(',');
    }
    ip_sb.putc(']');
    json.addMemberString("ip_addresses", ip_sb.getPtr());
    json.end();
    sb->puts(json_buf);
}
