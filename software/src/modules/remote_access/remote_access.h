/* esp32-firmware
 * Copyright (C) 2024 Frederic Henrichs <frederic@tinkerforge.com>
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

#pragma once

#include <WireGuard-ESP32.h>
#include <esp_http_client.h>
#include <queue>
#include <memory>

#include "module.h"
#include "config.h"
#include "async_https_client.h"
#include "ping/ping_sock.h"
#include "registration_state.enum.h"
#include "language.h"
#include "remote_access_packets.h"

#define MAX_USER_CONNECTIONS 5

struct HttpResponse {
    int status;
    char *cookie = nullptr;
    String body;
    uint64_t data_read = 0;
};

struct Connection {
    uint8_t id;
    std::unique_ptr<WireGuard> conn = nullptr;
    bool in_progress = false;
};

struct WgKey {
    String priv;
    String psk;
    String pub;
};

class RemoteAccess final : public IModule {
public:
    RemoteAccess() {};

    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    bool is_connected_local_ip(const IPAddress &address);

    Config &get_ping_state();
    micros_t get_ping_start();

    void update_peer_info(uint8_t connection_idx, uint8_t peer_index, bool up, const ip_addr_t *addr, uint16_t port);

    uint16_t send_port = 0;

    // Opens a TCP connection to the management server for streaming a charge log.
    // Sends RequestChargeLogSend and MetadataForChargeLog, waits for Ack.
    // Returns a connected TCP socket fd on success, or negative error code.
    // Must be called from a non-main thread (e.g., FreeRTOS task).
    int begin_charge_log_send(const char *filename, size_t filename_len,
                              const char *display_name, size_t display_name_len,
                              const char *user_uuid_str,
                              Language language);

    // Closes the TCP socket and waits for the server's final Ack.
    // Returns 0 on success, negative error code on failure.
    // Must be called from the same thread as begin_charge_log_send.
    int end_charge_log_send(int tcp_sock);

    // Release the inner socket back to run_management.
    void release_inner_socket();

private:
    void resolve_management();
    void connect_management();
    uint8_t get_connection(int32_t conn_id);
    void connect_remote_access(uint8_t i, uint16_t local_port);
    void close_all_remote_connections();
    void run_management();
    void handle_response_chunk(const AsyncHTTPSClientEvent *event);
    void run_request_with_next_stage(const String &url, esp_http_client_method_t method, const char *body, size_t body_size, const Config &next_config, std::function<void(const Config &config)> &&next_stage);
    void get_login_salt(const Config &user_config);
    void parse_login_salt();
    void get_secret(const Config &user_config);
    void parse_secret();
    void parse_registration(const Config &user_config, std::queue<WgKey> &keys, const String &public_key);
    void parse_add_user(std::queue<WgKey> &key_cache, const String &pub_key, const String &email, uint8_t next_user_id);
    void login(const Config &user_config, const String &login_key);
    void update_registration_state(RegistrationState state, const String &message = String());
    void update_connection_state(uint8_t conn_idx, uint8_t user, uint8_t connection, uint8_t state_value);
    void management_auth_fail();
    void request_cleanup();
    void cleanup_after();
    bool user_already_registered(const String &email);
    void setup_inner_socket();
    int start_ping();
    int stop_ping();
    int send_charge_log_metadata(const char *filename, size_t filename_len, const char *display_name, size_t display_name_len, int user_id, Language language);

    // Polls inner_socket for an Ack or Nack management packet.
    // Returns: 1 for Ack, -1 for Nack (nack_reason set), 0 for timeout.
    int poll_for_mgmt_response(uint32_t timeout_ms, uint8_t *nack_reason);

    std::unique_ptr<WireGuard> management = nullptr;
    Connection remote_connections[MAX_USER_CONNECTIONS] = {};

    String jwt;
    int inner_socket = -1;
    uint16_t in_seq_number = 0;
    bool management_request_done = false;
    bool management_request_failed = false;
    bool management_request_allowed = true;
    bool management_auth_failed = false;
    bool charge_log_sending = false;
    micros_t last_mgmt_alive = 0_us;
    uint64_t task_id = 0;

    esp_ping_handle_t ping = nullptr;
    micros_t ping_start = 0_us;

    std::unique_ptr<AsyncHTTPSClient> https_client;
    String response_body;
    std::unique_ptr<uint8_t[]> encrypted_secret = nullptr;
    std::unique_ptr<uint8_t[]> secret_nonce = nullptr;

    ConfigRoot config;
    ConfigRoot connection_state;
    ConfigRoot registration_state;
    Config     users_config_prototype;
    ConfigRoot registration_config;
    ConfigRoot ping_state;
};

struct PingArgs {
    uint32_t packets_sent;
    uint32_t packets_received;
};
