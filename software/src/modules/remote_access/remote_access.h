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

#include "module.h"
#include "config.h"
#include "async_https_client.h"
#include "ping/ping_sock.h"

struct HttpResponse {
    int status;
    char *cookie = nullptr;
    String body;
    uint64_t data_read = 0;
};

enum class RegistrationState {
    None,
    InProgress,
    Success,
    Error,
};

struct Connections {
    uint8_t id;
    WireGuard *conn;
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

    ConfigRoot &get_ping_state();
    uint32_t get_ping_start();

private:
    void resolve_management();
    void connect_management();
    uint8_t get_connection(uint8_t conn_id);
    void connect_remote_access(uint8_t i, uint16_t local_port);
    void close_all_remote_connections();
    void run_management();
    void handle_response_chunk(const AsyncHTTPSClientEvent *event);
    void run_request_with_next_stage(const char *url, esp_http_client_method_t method, const char *body, int body_size, ConfigRoot config, std::function<void(ConfigRoot config)> &&next_stage);
    void get_login_salt(ConfigRoot config);
    void parse_login_salt(ConfigRoot config);
    void get_secret(ConfigRoot config);
    void parse_secret(ConfigRoot config);
    void parse_registration(ConfigRoot config, std::queue<WgKey> keys, CoolString public_key);
    void parse_add_user(ConfigRoot cfg, std::queue<WgKey> key_cache, CoolString pub_key, CoolString email, uint32_t next_user_id);
    void login(ConfigRoot config, CoolString &login_key);
    void request_cleanup();
    void cleanup_after();
    bool user_already_registered(const CoolString &email);
    void setup_inner_socket();
    int start_ping();
    int stop_ping();
    WireGuard *management = nullptr;
    Connections remote_connections[5] = {};

    String jwt;
    int inner_socket = -1;
    uint16_t in_seq_number = 0;
    bool management_request_done = false;
    bool management_request_failed = false;
    bool management_request_allowed = true;
    micros_t last_mgmt_alive = 0_us;
    uint64_t task_id = 0;

    esp_ping_handle_t ping = nullptr;
    uint32_t ping_start = 0;

    std::unique_ptr<AsyncHTTPSClient> https_client;
    String response_body;
    std::unique_ptr<uint8_t[]> encrypted_secret = nullptr;
    std::unique_ptr<uint8_t[]> secret_nonce = nullptr;

    ConfigRoot config;
    ConfigRoot connection_state;
    ConfigRoot connection_state_prototype;
    ConfigRoot registration_state;
    ConfigRoot users_config_prototype;
    ConfigRoot registration_config;
    ConfigRoot ping_state;
};

enum management_command_id {
    Connect,
    Disconnect,
};

struct [[gnu::packed]] management_command {
    management_command_id command_id;
    int32_t connection_no;
    uint8_t connection_uuid[16];
};

struct [[gnu::packed]] port_discovery_packet {
    uint32_t charger_id;
    int32_t connection_no;
    uint8_t connection_uuid[16];
};

struct [[gnu::packed]] management_packet_header {
    uint16_t magic;
    uint16_t length;
    uint16_t seq_num;
    uint8_t version;
    /*
        0x00 - Management command
    */
    uint8_t type;
};

struct [[gnu::packed]] management_command_packet {
    struct management_packet_header header;
    struct management_command command;
};

struct PingArgs {
    RemoteAccess *that;
    uint32_t packets_sent;
    uint32_t packets_received;
};
