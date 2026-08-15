/* esp32-firmware
 * Copyright (C) 2024-2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "build.h"
#include "esp_httpd_priv.h"
#include "event_log_prefix.h"
#include <esp_http_server.h>
#include <esp_tls.h>
#include <mbedtls/ssl.h>

#include "cert_generator.h"
#include "generated/module_dependencies.h"
#include "tools.h"
#include "tools/hexdump.h"
#include "tools/net.h"

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

/// Extract the SKI (Subject Key Identifier) from the TLS client certificate presented during the handshake.
/// Returns an empty string if no client certificate was presented or the SKI could not be extracted.
/// @param httpd The httpd handle (httpd_handle_t, actually struct httpd_data*)
/// @param sockfd The socket file descriptor of the client connection
[[gnu::noinline]]
static String extract_peer_ski_from_tls(httpd_handle_t httpd, int sockfd)
{
    struct httpd_data *hd = static_cast<struct httpd_data *>(httpd);
    struct sock_db *session = httpd_sess_get(hd, sockfd);
    if (session == nullptr || session->transport_ctx == nullptr) {
        return String{};
    }

    struct httpd_ssl_transport_ctx {
        esp_tls_t *tls;
        void *global_ctx;
    };

    auto *transport = static_cast<httpd_ssl_transport_ctx *>(session->transport_ctx);
    if (transport->tls == nullptr) {
        return String{};
    }

    void *ssl_ctx_void = esp_tls_get_ssl_context(transport->tls);
    if (ssl_ctx_void == nullptr) {
        return String{};
    }

    mbedtls_ssl_context *ssl = static_cast<mbedtls_ssl_context *>(ssl_ctx_void);
    const mbedtls_x509_crt *peer_cert = mbedtls_ssl_get_peer_cert(ssl);
    if (peer_cert == nullptr) {
        eebus.trace_fmtln("No client certificate presented during TLS handshake");
        return String{};
    }

    // Extract the Subject Key Identifier from the peer certificate
    char ski_hex[41];
    size_t ski_len = hexdump(peer_cert->subject_key_id.p, peer_cert->subject_key_id.len, ski_hex, std::size(ski_hex), HexdumpCase::Lower);

    if (ski_len != 40) {
        eebus.trace_fmtln("Peer certificate SKI has unexpected length: %zu", ski_len);
        return String{};
    }

    return String{ski_hex, ski_len};
}

#ifdef EEBUS_SHIP_AUTOCONNECT
static esp_err_t eebus_client_crt_bundle_attach(void *conf)
{
    mbedtls_ssl_config *ssl_conf = (mbedtls_ssl_config *)conf;
    mbedtls_ssl_conf_authmode(ssl_conf, MBEDTLS_SSL_VERIFY_NONE);
    return ESP_OK; // Don't chain to real bundle — we don't need CA certs
}
#endif

void Ship::pre_setup()
{
    web_sockets.pre_setup();
}

void Ship::setup()
{
    // The extra port for SHIP connections must always be registered during setup, whether or not EEBUS is enabled.
    // This means that a restart is required to change the EEBUS certificate.
    // TODO: Add functionality to make EEBUS use the central cert store
    WebServerExtraPortData *extra_ship_port = static_cast<WebServerExtraPortData *>(malloc(sizeof(WebServerExtraPortData)));
    *extra_ship_port = {
        .port = SHIP_PORT,
        .supports_user_authentication = false, // SHIP devices won't have local user credentials.
        .request_client_cert = true,           // Request client certificate for peer SKI identification.
        .transport_mode = TransportMode::Secure,
        .cert_info =
            {
                .cert_id = -1,
                .key_id = -1,
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
    is_enabled = true;
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
        logger.printfln("Connecting to known EEBUS peers in 30 seconds");
        autoconnect_timer = task_scheduler.scheduleOnce(
            [this]() {
                discover_ship_peers();
            },
            30_s); // Initial Timeout is 30s after that EEBUS_SHIP_AUTOCONNECT_INTERVAL should be used
    }
}

void Ship::disable_ship()
{
    eebus.trace_fmtln("disable_ship start");
    is_enabled = false;

    if (mdns_scan != nullptr) {
        mdns_query_async_delete(mdns_scan);
        mdns_scan = nullptr;
    }

    task_scheduler.cancel(autoconnect_timer);
    autoconnect_timer = 0;

    for (auto &ship_connection : eebus.ship.ship_connections) {
        ship_connection->schedule_close(0_ms, "EEBUS disabled");
    }
    // If mDNS is not started, the service was never announced and doesn't need to be removed
    if (network.is_mdns_started() && mdns_service_exists("_ship", "_tcp", NULL)) {
        mdns_service_remove("_ship", "_tcp");
    }
    update_discovery_state(ShipDiscoveryState::Ready);

    eebus.trace_fmtln("disable_ship end");
}

void Ship::setup_wss()
{
    if (wss_registered) {
        return;
    }

#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("setup_wss_server start");
#endif
    if (!cert.is_loaded()) {
        const cert_load_info cert_info = {
            .cert_id = -1,
            .key_id = -1,
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

    // Only copy data and push events on the http tasks
    web_sockets.onConnect_HTTPThread([this](WebSocketsClient *ws_client) {
        if (!eebus.is_enabled()) {
            return false;
        }

        const int fd = ws_client->getFd();
        RxEvent event;
        event.type = RxEvent::Type::Connected;
        event.role = ShipConnection::Role::Server;
        event.fd = fd;
        event.peer_ip = tf_peer_address_of_sockfd(fd).toString();
        // The TLS session is only guaranteed to be accessible on the httpd task.
        event.tls_ski = extract_peer_ski_from_tls(web_sockets.get_httpd_handle(), fd);
        push_rx_event(std::move(event));
        return true;
    });

    web_sockets.onDisconnect_HTTPThread([this](WebSocketsClient *client, bool /*clean_close*/) {
        RxEvent event;
        event.type = RxEvent::Type::Disconnected;
        event.role = ShipConnection::Role::Server;
        event.fd = client->getFd();
        event.reason = "Websocket disconnected";
        push_rx_event(std::move(event));
    });

    web_sockets.onBinaryDataReceived_HTTPThread([this](WebSocketsClient *client, httpd_ws_frame_t *ws_pkt) {
        RxEvent event;
        event.type = RxEvent::Type::Frame;
        event.role = ShipConnection::Role::Server;
        event.fd = client->getFd();
        event.fin = ws_pkt->final;
        event.data_len = ws_pkt->len;
        event.data.reset(static_cast<uint8_t *>(malloc_psram(ws_pkt->len)));
        if (!event.data) {
            eebus.trace_fmtln("onBinaryDataReceived: PSRAM allocation failed, dropping frame (len %zu)", ws_pkt->len);
            return;
        }
        memcpy(event.data.get(), ws_pkt->payload, ws_pkt->len);
        push_rx_event(std::move(event));
    });

    web_sockets.start("/ship/", "/eebus/ws", "ship", SHIP_PORT);
    logger.printfln("SHIP started up and accepting connections");

    wss_registered = true;
}

// Callable from any task
bool Ship::push_rx_event(RxEvent &&event)
{
    switch (rx_queue.push(std::move(event))) {
        case ShipRxQueue::PushResult::PushedToEmpty:
            task_scheduler.scheduleOnce([this]() {
                drain_rx_events();
            });
            [[fallthrough]];
        case ShipRxQueue::PushResult::Pushed:
            return true;
        default:
            eebus.trace_fmtln("push_rx_event: queue full, dropping event (type %u, len %zu)", static_cast<unsigned>(event.type), event.data_len);
            return false;
    }
}

void Ship::drain_rx_events()
{
    RxEvent event;
    while (rx_queue.pop(&event)) {
        process_rx_event(event);
    }
}

ShipConnection *Ship::find_connection(const RxEvent &event)
{
    for (const auto &conn : ship_connections) {
        if (conn->closing_scheduled || conn->role != event.role) {
            continue;
        }
        if (event.role == ShipConnection::Role::Server ? conn->ws_fd == event.fd : conn->ws_server == event.ws_handle) {
            return conn.get();
        }
    }
    return nullptr;
}

void Ship::process_rx_event(RxEvent &event)
{
    if (event.type == RxEvent::Type::Connected) {
        handle_connected(event);
        return;
    }

    ShipConnection *conn = find_connection(event);
    if (conn == nullptr) {
        eebus.trace_fmtln("process_rx_event: no connection for event (type %u)", static_cast<unsigned>(event.type));
        return;
    }

    switch (event.type) {
        case RxEvent::Type::Frame:
            conn->process_frame(event.data.get(), event.data_len, event.fin);
            break;
        case RxEvent::Type::Disconnected:
            conn->ws_fd = -1; // Socket is already gone, don't close it again.
            conn->schedule_close(0_ms, event.reason);
            break;
        case RxEvent::Type::ClientConnected:
            conn->start_client_confirm();
            // SHIP v1.1.0 12.2.3: check for duplicates right after the TLS connection is established
            resolve_duplicate_connections(conn->peer_node);
            break;
        default:
            break;
    }
}

void Ship::handle_connected(const RxEvent &event)
{
    if (!is_enabled || !eebus.is_enabled()) {
        ws_close(event.fd);
        return;
    }

    // The SKI from the TLS client certificate is the only reliable peer identity
    // (multiple SHIP peers may share one host IP). SHIP 9.1 requires client certificates.
    if (event.tls_ski.isEmpty()) {
        eebus.trace_fmtln("Rejecting incoming SHIP connection from %s: no SKI from TLS client cert", event.peer_ip.c_str());
        logger.printfln("Rejected SHIP connection from %s: no TLS client certificate", event.peer_ip.c_str());
        ws_close(event.fd);
        return;
    }

    if (peer_handler.get_peer_by_ski(event.tls_ski) != nullptr) {
        eebus.trace_fmtln("Incoming SHIP connection from %s identified by TLS cert SKI %s", event.peer_ip.c_str(), event.tls_ski.c_str());
    } else {
        eebus.trace_fmtln("New incoming SHIP connection from %s with unknown SKI %s (from TLS cert)", event.peer_ip.c_str(), event.tls_ski.c_str());
    }
    ShipNode *node = peer_handler.get_or_create_by_ski(event.tls_ski);
    peer_handler.update_ip_by_ski(event.tls_ski, event.peer_ip, true);

    node->state = NodeState::Connected;
    eebus.trace_fmtln("WebSocketsClient connected from %s with SKI %s", event.peer_ip.c_str(), node->txt_ski.c_str());
    ship_connections.push_back(make_unique_psram<ShipConnection>(event.fd, node));
    logger.printfln("New SHIP Client connected from %s", node->node_name().c_str());

    // SHIP v1.1.0 12.2.3: check for duplicates right after the peer is identified
    resolve_duplicate_connections(node);

    task_scheduler.scheduleOnce([]() {
        eebus.ship.discover_ship_peers();
        eebus.update_peers_state();
    }, 2_s);
}

// SHIP v1.1.0 12.2.3: Prevent double connections with SKI comparison.
void Ship::resolve_duplicate_connections(const ShipNode *node)
{
    if (node == nullptr || node->txt_ski.isEmpty()) {
        return;
    }

    size_t count = 0;
    for (const auto &conn : ship_connections) {
        if (!conn->closing_scheduled && conn->peer_node == node) {
            count++;
        }
    }
    if (count < 2) {
        return;
    }

    String own_ski = eebus.state.get("ski")->asString();
    String peer_ski = node->txt_ski;
    own_ski.toLowerCase();
    peer_ski.toLowerCase();

    if (own_ski == peer_ski) {
        // Both sides would decide identically, so the tie-break cannot work.
        // This happens when two peers (mis)use the same certificate.
        logger.printfln("SHIP: Duplicate connections to %s with SKI equal to our own, cannot resolve. Are two peers sharing one certificate?", node->node_name().c_str());
        return;
    }

    // SKIs are equal-length hex strings, so lexicographic comparison of the
    // normalized strings equals numeric comparison of the 160 bit SKI values.
    if (own_ski > peer_ski) {
        eebus.trace_fmtln("SHIP 12.2.3: Duplicate connections to %s, we have the bigger SKI, keeping only the most recent connection", node->node_name().c_str());
        close_all_but_most_recent(node);
    } else {
        // Smaller SKI: give the bigger-SKI peer 3 seconds to resolve the duplicate.
        // (We skip the spec's optional WebSocket ping liveness check and simply
        // close the older connections, which the spec permits.)
        // Capture the SKI, not the node: the node may be erased within the 3 seconds.
        eebus.trace_fmtln("SHIP 12.2.3: Duplicate connections to %s, waiting 3s for the bigger-SKI peer to resolve", node->node_name().c_str());
        task_scheduler.scheduleOnce([this, ski = node->txt_ski]() {
            if (!is_enabled || !eebus.is_enabled()) {
                return;
            }
            const ShipNode *node = peer_handler.get_peer_by_ski(ski);
            if (node == nullptr) {
                return; // The peer was removed in the meantime.
            }
            size_t remaining = 0;
            for (const auto &conn : ship_connections) {
                if (!conn->closing_scheduled && conn->peer_node == node) {
                    remaining++;
                }
            }
            if (remaining < 2) {
                return; // The peer resolved the duplicate in time.
            }
            eebus.trace_fmtln("SHIP 12.2.3: Bigger-SKI peer %s did not resolve duplicate connections within 3s, closing older connections", node->node_name().c_str());
            close_all_but_most_recent(node);
        }, 3_s);
    }
}

void Ship::close_all_but_most_recent(const ShipNode *node)
{
    // ship_connections is in creation order: the last match is the most recent.
    ShipConnection *most_recent = nullptr;
    for (const auto &conn : ship_connections) {
        if (!conn->closing_scheduled && conn->peer_node == node) {
            most_recent = conn.get();
        }
    }

    for (const auto &conn : ship_connections) {
        if (conn->closing_scheduled || conn->peer_node != node || conn.get() == most_recent) {
            continue;
        }
        eebus.trace_fmtln("SHIP 12.2.3: Closing duplicate connection to %s (role=%s, established=%d)", node->node_name().c_str(), conn->role == ShipConnection::Role::Client ? "client" : "server", conn->connection_established ? 1 : 0);
        if (conn->connection_established) {
            // Already in the data exchange state: terminate gracefully (SHIP 13.4.8).
            conn->initiate_termination("Duplicate connection resolved per SHIP 12.2.3");
        } else {
            conn->schedule_close(0_ms, "Duplicate connection resolved per SHIP 12.2.3");
        }
    }
}

// Callable from the main task
bool Ship::ws_send(int fd, const char *payload, size_t payload_len)
{
    return web_sockets.sendToClient(payload, payload_len, fd, HTTPD_WS_TYPE_BINARY);
}

void Ship::ws_close(int fd)
{
    web_sockets.closeClient_async(fd);
}

void Ship::connect_trusted_peers()
{
#ifdef EEBUS_SHIP_AUTOCONNECT
    if (!is_enabled || !eebus.is_enabled()) {
        return;
    }

    task_scheduler.cancel(autoconnect_timer);
    if (!cert.is_loaded()) {
        eebus.trace_fmtln("connect_trusted_peers: Certificate not loaded, skipping");
        return;
    }

    const auto &peers = peer_handler.get_peers();
#ifdef EEBUS_TRACE_SUPER_VERBOSE
    eebus.trace_fmtln("connect_trusted_peers start, %zu peers known", peers.size());
#endif
    int trusted_peer_count = 0;

    for (const auto &node : peers) {
        if (!is_enabled || !eebus.is_enabled()) {
            break;
        }

        if (!node->trusted || node->pending_removal) {
            continue;
        }
        if (node->state != NodeState::Discovered && node->state != NodeState::Disconnected) {
            continue;
        }
        if (node->ip_address.empty() || node->port == 0) {
            eebus.trace_fmtln("Skipping peer %s: no IP or port", node->node_name().c_str());
            continue;
        }
        if (node->txt_ski.isEmpty()) {
            eebus.trace_fmtln("Skipping peer %s: no SKI", node->node_name().c_str());
            continue;
        }

        // Check if we already have an active connection to this peer
        bool already_connected = false;
        for (const auto &conn : ship_connections) {
            if (conn->peer_node == node.get() && !conn->closing_scheduled) {
                already_connected = true;
                break;
            }
        }
        if (already_connected) {
            continue;
        }

        trusted_peer_count++;

        // Use the first (most recently used) IP address
        const String &ip = node->ip_address.front();
        const String &wss_path = node->txt_wss_path.isEmpty() ? String("/ship/") : node->txt_wss_path;

        tf_websocket_client_config_t websocket_cfg = {};
        cert.get_data(reinterpret_cast<const uint8_t **>(&websocket_cfg.client_cert), &websocket_cfg.client_cert_len, reinterpret_cast<const uint8_t **>(&websocket_cfg.client_key), &websocket_cfg.client_key_len);
        websocket_cfg.host = ip.c_str();
        websocket_cfg.port = node->port;
        websocket_cfg.path = wss_path.c_str();
        websocket_cfg.transport = WEBSOCKET_TRANSPORT_OVER_SSL;
        websocket_cfg.subprotocol = "ship"; // SHIP 10.2
        websocket_cfg.disable_auto_reconnect = true;
        websocket_cfg.crt_bundle_attach = eebus_client_crt_bundle_attach;
        websocket_cfg.task_stack = SHIP_CONNECTION_WS_CLIENT_TASK_STACK;
        websocket_cfg.task_name = "eebus_ws";

        eebus.trace_fmtln("Connecting to trusted peer %s at %s:%d%s", node->node_name().c_str(), ip.c_str(), node->port, wss_path.c_str());

        node->state = NodeState::Connecting;
        eebus.update_peers_state();

        ship_connections.push_back(make_unique_psram<ShipConnection>(websocket_cfg, node.get()));
        ship_connections.back()->start_client();
    }

    if (trusted_peer_count > 0) {
        logger.printfln("SHIP: Connecting to %d trusted peer(s)", trusted_peer_count);
    }
    if (is_enabled && eebus.is_enabled()) {
        autoconnect_timer = task_scheduler.scheduleOnce(
            [this]() {
                discover_ship_peers();
            },
            EEBUS_SHIP_AUTOCONNECT_INTERVAL);
    } else {
        autoconnect_timer = 0;
    }
#endif
}

void Ship::print_skis(StringBuilder *sb)
{
    for (const auto &node : peer_handler.get_peers()) {
        if (node->pending_removal) {
            continue;
        }
        node->as_json(sb);
        sb->putc(',');
    }
}

void Ship::remove(const ShipConnection &ship_connection)
{
    ShipNode *node = ship_connection.peer_node;

    ship_connections.erase(std::remove_if(ship_connections.begin(),
                                          ship_connections.end(),
                                          [&ship_connection](const unique_ptr_any<ShipConnection> &ptr) {
                                              return ptr.get() == &ship_connection;
                                          }),
                           ship_connections.end());

    // Erase a removed peer once its last connection is gone
    if (node != nullptr && node->pending_removal) {
        for (const auto &conn : ship_connections) {
            if (conn->peer_node == node) {
                return;
            }
        }
        peer_handler.erase_node(node);
    }
}

void Ship::close_connections_by_ski(const String &ski, const String &reason) const
{
    for (auto &conn : ship_connections) {
        if (conn->peer_node && conn->peer_node->txt_ski.equalsIgnoreCase(ski) && !conn->closing_scheduled) {
            if (conn->connection_established) {
                // Data exchange state: terminate gracefully (SHIP 13.4.8)
                conn->initiate_termination(reason);
            } else {
                conn->schedule_close(0_ms, reason);
            }
        }
    }
}

void Ship::notify_peer_updated(const String &ski) const
{
    for (auto &conn : ship_connections) {
        if (conn->peer_node && conn->peer_node->txt_ski.equalsIgnoreCase(ski) && !conn->closing_scheduled) {
            conn->notify_trust_changed();
        }
    }
}
