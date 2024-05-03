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

#include "module.h"
#include "config.h"

#include <WireGuard-ESP32.h>
#include "esp_http_client.h"

class RemoteAccess final : public IModule {
public:
    RemoteAccess() {};

    void pre_setup() override;
    void setup() override;
    void register_urls() override;

private:
    void resolve_management();
    void login();
    void connect_management();
    void connect_remote_access(uint8_t i);
    void run_management();
    int setup_inner_socket();
    String make_http_request(const char *url, esp_http_client_method_t method, const char *payload, size_t payload_size, std::vector<std::pair<CoolString, CoolString>> *headers, esp_err_t *ret_error);

    WireGuard management;
    WireGuard remote_connections[5];

    String jwt;
    int inner_socket = -1;

    ConfigRoot config;
    ConfigRoot management_connection;
    ConfigRoot remote_connection_config;
    ConfigRoot connection_state;
    ConfigRoot register_config;
};