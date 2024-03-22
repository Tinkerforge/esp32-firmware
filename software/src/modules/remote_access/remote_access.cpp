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

#include "module_dependencies.h"

#include "remote_access.h"
#include "api.h"
#include "event_log.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "build.h"
#include "esp_http_client.h"
#include "task_scheduler.h"
#include <WiFi.h>

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);
extern char local_uid_str[32];
extern uint32_t local_uid_num;

enum ManagementCommandId {
    Connect,
    Disconnect,
};

struct ManagementCommand {
    ManagementCommandId command_id;
    int32_t connection_no;
    uint8_t connection_uuid[16];
};

struct ManagementResponse {
    uint32_t charger_id;
    int32_t connection_no;
    uint8_t connection_uuid[16];
};

static int create_sock_and_send_to(const void *payload, size_t payload_len, const char *dest_host, uint16_t port, uint16_t *local_port) {
    struct sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));

    ip_addr_t ip;
    int ret = dns_gethostbyname_addrtype(dest_host, &ip, nullptr, nullptr, LWIP_DNS_ADDRTYPE_IPV4);
    if (ret == ERR_VAL) {
        logger.printfln("No DNS server is configured!");
    }

    if (ret != ESP_OK || ip.type != IPADDR_TYPE_V4) {
        return -1;
    }

    dest_addr.sin_addr.s_addr = ip.u_addr.ip4.addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        logger.printfln("Remote Access: failed to send management frame");
        return sock;
    }
    ret = fcntl(sock, F_SETFL, O_NONBLOCK);
    if (ret == -1) {
        logger.printfln("Setting socket to non_blocking caused and error: (%i)%s", errno, strerror_r(errno, nullptr, 0));
        close(sock);
    }

    if (local_port != nullptr) {
        struct sockaddr_in local_addr;
        bzero(&local_addr, sizeof(local_addr));

        local_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
        local_addr.sin_family = AF_INET;
        local_addr.sin_port = htons(*local_port);
        ret = bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr));
        if (ret == -1) {
            logger.printfln("Binding socket to port %u caused and error: (%i)%s", *local_port, errno, strerror_r(errno, nullptr, 0));
            close(sock);
        }
    }

    ret = sendto(sock, payload, payload_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    logger.printfln("Sendto returned %i", ret);
    close(sock);
    return ret;
}

void RemoteAccess::pre_setup() {
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"email", Config::Str("", 0, 64)},
        {"password", Config::Str("", 0, 64)},
        {"relay_host", Config::Str("", 0, 64)},
        {"relay_host_port", Config::Uint16(443)},
        {"self_signed_cert_id", Config::Int8(-1)}
    })};

    management_connection = ConfigRoot{Config::Object({
        {"internal_ip",      Config::Str("0.0.0.0", 7, 15)},
        {"internal_subnet",  Config::Str("0.0.0.0", 7, 15)},
        {"internal_gateway", Config::Str("0.0.0.0", 7, 15)},

        {"remote_internal_ip", Config::Str("0.0.0.0", 7, 15)},
        {"remote_host", Config::Str("", 0, 64)},
        {"remote_port", Config::Uint16(51820)},

        {"local_port", Config::Uint16(51820)},

        {"private_key",       Config::Str("", 0, 44)},
        {"remote_public_key", Config::Str("", 0, 44)},
        }), [](Config &cfg, ConfigSource source) -> String {
            IPAddress unused;

            if (!unused.fromString(cfg.get("internal_ip")->asEphemeralCStr()))
                return "Failed to parse \"internal_ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

            if (!unused.fromString(cfg.get("internal_subnet")->asEphemeralCStr()))
                return "Failed to parse \"internal_subnet\": Expected format is dotted decimal, i.e. 10.0.0.1";

            if (!unused.fromString(cfg.get("internal_gateway")->asEphemeralCStr()))
                return "Failed to parse \"internal_gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

            if (!unused.fromString(cfg.get("remote_internal_ip")->asEphemeralCStr()))
                return "Failed to parse \"remote_internal_ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

            const String &private_key = cfg.get("private_key")->asString();
            String result = check_key(private_key, true);
            if (!result.isEmpty())
                return "\"private_key\"" + result;

            const String &remote_public_key = cfg.get("remote_public_key")->asString();
            result = check_key(remote_public_key, true);
            if (!result.isEmpty())
                return "\"remote_public_key\"" + result;


            return "";
        }};

    remote_connection_config = ConfigRoot {
        Config::Object({
            {"connections", Config::Array({},
                new Config{
                    Config::Object({
                        {"internal_ip",      Config::Str("0.0.0.0", 7, 15)},
                        {"internal_subnet",  Config::Str("0.0.0.0", 7, 15)},
                        {"internal_gateway", Config::Str("0.0.0.0", 7, 15)},

                        {"remote_internal_ip", Config::Str("0.0.0.0", 7, 15)},
                        {"remote_host", Config::Str("", 0, 64)},
                        {"remote_port", Config::Uint16(51820)},

                        {"local_port", Config::Uint16(51820)},

                        {"private_key",       Config::Str("", 0, 44)},
                        {"remote_public_key", Config::Str("", 0, 44)},
                    })
                }, 5, 5, Config::type_id<Config::ConfObject>())
            }
        })
    };

    connection_state = ConfigRoot{
        Config::Object({
            {"management_connection_state", Config::Uint8(0)},
            {"remote_connection_states", Config::Array({
                Config::Object({
                    {"connection_state", Config::Uint8(0)}
                }),
                Config::Object({
                    {"connection_state", Config::Uint8(0)}
                }),
                Config::Object({
                    {"connection_state", Config::Uint8(0)}
                }),
                Config::Object({
                    {"connection_state", Config::Uint8(0)}
                }),
                Config::Object({
                    {"connection_state", Config::Uint8(0)}
                })
            },
                new Config {
                    Config::Object({
                        {"connection_state", Config::Uint8(0)}
                    })
                }, 5, 5, Config::type_id<Config::ConfObject>())}
        })
    };
}

void RemoteAccess::setup() {
    api.restorePersistentConfig("remote_access/config", &config);
    api.restorePersistentConfig("remote_access/management_connection", &management_connection);
    api.restorePersistentConfig("remote_access/remote_connection_config", &remote_connection_config);
    initialized = true;

    if (!config.get("enable")->asBool())
        return;

    logger.printfln("Remote Access is enabled trying to connect");
}

void RemoteAccess::register_urls() {
    api.addPersistentConfig("remote_access/config", &config, {
        "password"
    });
    api.addPersistentConfig("remote_access/management_connection", &management_connection, {
        "private_key",
        "remote_public_key",
    });

    api.addPersistentConfig("remote_access/remote_connection_config", &remote_connection_config, {
        "private_key"
    });

    api.addState("remote_access/state", &connection_state);

    api.addCommand("remote_access/test", Config::Null(), {}, [this]() {
        logger.printfln("Henlo");
        this->login();
        this->resolve_management();
        this->connect_management();
    }, true);

    api.addCommand("remote_access/test1", Config::Null(), {}, [this]() {
        logger.printfln("bla");
        this->login();
        this->resolve_management();
    }, true);
    api.addCommand("remote_access/test2", Config::Null(), {}, [this]() {
        logger.printfln("asd");
        this->login();
        this->resolve_management();
        for (size_t i = 0; i < 5; i++) {
            this->connect_remote_access(i);
        }
    }, true);

    if (!config.get("enable")->asBool()) {
        return;
    }

    task_scheduler.scheduleOnce([this]() {
        this->login();
        this->resolve_management();
        this->connect_management();
        // for (size_t i = 0; i < 5; i++) {
        //     this->connect_remote_access(i);
        // }
    }, 5000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->login();
        this->resolve_management();
        if (!management.is_peer_up(nullptr, nullptr)) {
            return;
        }
    }, 1000 * 10, 1000 * 10);
}

static esp_err_t http_event_handle(esp_http_client_event *evt) {
    char **cookie = reinterpret_cast<char **>(evt->user_data);
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        logger.printfln("Management: error while resolving");
        break;

    case HTTP_EVENT_ON_HEADER:
        if (!strcmp("set-cookie", evt->header_key)) {
            *cookie = strdup(evt->header_value);
        }
        break;

    default:
        break;
    }
    return ESP_OK;
}

static String parse_cookie(String cookie) {
    size_t start = cookie.indexOf('=') + 1;
    size_t end = cookie.indexOf(';');
    return cookie.substring(start, end);
}

void RemoteAccess::login() {
    size_t cert_len = 0;
    int self_signed_cert_id = config.get("self_signed_cert_id")->asInt();
    std::unique_ptr<unsigned char[]> cert = nullptr;
    if (self_signed_cert_id >= 0) {
        cert = certs.get_cert(static_cast<uint8_t>(self_signed_cert_id), &cert_len);
        if (cert == nullptr) {
            logger.printfln("Management: Failed to get self signed cert");
            return;
        }
    }

    CoolString relay_host = config.get("relay_host")->asString();
    uint32_t relay_host_port = config.get("relay_host_port")->asUint();
    CoolString login_url = "https://";
    login_url += relay_host;
    login_url += ":";
    login_url += relay_host_port;
    login_url += "/api/auth/login";

    char *cookie = nullptr;

    esp_http_client_config_t http_config;
    bzero(&http_config, sizeof(esp_http_client_config_t));

    http_config.url = login_url.c_str();
    http_config.event_handler = http_event_handle;
    http_config.method = HTTP_METHOD_POST;
    http_config.user_data = &cookie;

    if (cert == nullptr) {
        http_config.crt_bundle_attach = esp_crt_bundle_attach;
        http_config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    } else {
        http_config.cert_pem = (const char *)cert.get();
        http_config.skip_cert_common_name_check = true;
    }

    esp_http_client_handle_t client = esp_http_client_init(&http_config);

    CoolString login_data = "{\"email\":\"";
    login_data += config.get("email")->asString() + "\",";
    login_data += "\"password\":\"" + config.get("password")->asString() + "\"}";



    esp_err_t ret = esp_http_client_set_post_field(client, login_data.c_str(), login_data.length());
    if (ret != ESP_OK) {
        logger.printfln("Failed to set post data: %i", ret);
        esp_http_client_cleanup(client);
        return;
    }

    ret = esp_http_client_perform(client);
    if (ret != ESP_OK) {
        logger.printfln("Failed to send request: %i", ret);
    }

    if (cookie != nullptr) {
        jwt = parse_cookie(String(cookie));
        free(cookie);
    }

    esp_http_client_cleanup(client);
}

void RemoteAccess::resolve_management() {
    size_t cert_len = 0;
    int self_signed_cert_id = config.get("self_signed_cert_id")->asInt();
    std::unique_ptr<unsigned char[]> cert = nullptr;
    if (self_signed_cert_id >= 0) {
        cert = certs.get_cert(static_cast<uint8_t>(self_signed_cert_id), &cert_len);
        if (cert == nullptr) {
            logger.printfln("Management: Failed to get self signed cert");
            return;
        }
    }

    CoolString relay_host = config.get("relay_host")->asString();
    uint32_t relay_host_port = config.get("relay_host_port")->asUint();
    CoolString url = "https://";
    url += relay_host;
    url += ":";
    url += relay_host_port;
    url += "/api/management";

    esp_http_client_config_t http_config;
    bzero(&http_config, sizeof(esp_http_client_config_t));

    http_config.url = url.c_str();
    http_config.event_handler = http_event_handle;
    http_config.method = HTTP_METHOD_PUT;

    if (cert == nullptr) {
        http_config.crt_bundle_attach = esp_crt_bundle_attach;
        http_config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    } else {
        http_config.cert_pem = (const char *)cert.get();
        http_config.skip_cert_common_name_check = true;
    }

    esp_http_client_handle_t client = esp_http_client_init(&http_config);

    CoolString management_data = "{\"id\":";
    management_data += String(local_uid_num) + "}";

    esp_err_t ret = esp_http_client_set_post_field(client, management_data.c_str(), management_data.length());
    if (ret != ESP_OK) {
        logger.printfln("Failed to set post data: %i", ret);
        esp_http_client_cleanup(client);
        return;
    }
    String access_token = "access_token=";
    access_token += jwt + ";";
    esp_http_client_set_header(client, "Cookie", access_token.c_str());
    esp_http_client_set_header(client, "Content-Type", "application/json");

    ret = esp_http_client_perform(client);
    if (ret != ESP_OK) {
        logger.printfln("Failed to send request: %i", ret);
    }

    logger.printfln("management response code: %i", esp_http_client_get_status_code(client));
    esp_http_client_cleanup(client);
}

void RemoteAccess::connect_management() {
    static bool done = false;
    if (done)
        return;

    struct timeval tv;
    if (!clock_synced(&tv)) {
        task_scheduler.scheduleOnce([this]() {
            this->connect_management();
        }, 5000);
        return;
    }

    done = true;

    IPAddress internal_ip;
    IPAddress internal_subnet;
    IPAddress internal_gateway;
    IPAddress allowed_ip;
    IPAddress allowed_subnet;
    uint16_t local_port;

    internal_ip.fromString(management_connection.get("internal_ip")->asEphemeralCStr());
    internal_subnet.fromString(management_connection.get("internal_subnet")->asEphemeralCStr());
    internal_gateway.fromString(management_connection.get("internal_gateway")->asEphemeralCStr());
    allowed_ip.fromString("0.0.0.0");
    allowed_subnet.fromString("0.0.0.0");
    local_port = management_connection.get("local_port")->asUint();

    String private_key = management_connection.get("private_key")->asString(); // Local copy of ephemeral conf String. The network interface created by WG might hold a reference to the C string.
    String remote_host = management_connection.get("remote_host")->asString(); // Local copy of ephemeral conf String. lwip_getaddrinfo() might hold a reference to the C string.

    logger.printfln("Connecting to Management WireGuard peer %s:%u", remote_host.c_str(), management_connection.get("remote_port")->asUint());

    management.begin(internal_ip,
             internal_subnet,
             local_port,
             internal_gateway,
             private_key.c_str(),
             remote_host.c_str(),
             management_connection.get("remote_public_key")->asEphemeralCStr(),
             management_connection.get("remote_port")->asUint(),
             allowed_ip,
             allowed_subnet,
             false,
             nullptr);

    task_scheduler.scheduleWithFixedDelay([this]() {
        bool up = management.is_peer_up(nullptr, nullptr);

        if (this->connection_state.get("management_connection_state")->updateUint(up ? 2 : 1)) {
            if (up) {
                this->setup_inner_socket();
                logger.printfln("Management connection connected");
            } else {
                close(this->inner_socket);
                this->inner_socket = -1;
                logger.printfln("Management connection disconnected");
            }
        }
    }, 1000, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->run_management();
    }, 1000, 250);
}

void RemoteAccess::connect_remote_access(uint8_t i) {
    const Config *conf = static_cast<Config *>(remote_connection_config.get("connections")->get(i));

    struct timeval tv;
    if (!clock_synced(&tv)) {
        task_scheduler.scheduleOnce([this, i]() {
            this->connect_remote_access(i);
        }, 5000);
        return;
    }

    IPAddress internal_ip;
    IPAddress internal_subnet;
    IPAddress internal_gateway;
    IPAddress allowed_ip;
    IPAddress allowed_subnet;
    uint16_t local_port;

    internal_ip.fromString(conf->get("internal_ip")->asEphemeralCStr());
    internal_subnet.fromString(conf->get("internal_subnet")->asEphemeralCStr());
    internal_gateway.fromString(conf->get("internal_gateway")->asEphemeralCStr());
    allowed_ip.fromString("0.0.0.0");
    allowed_subnet.fromString("0.0.0.0");
    local_port = conf->get("local_port")->asUint();

    String private_key = conf->get("private_key")->asString(); // Local copy of ephemeral conf String. The network interface created by WG might hold a reference to the C string.
    String remote_host = conf->get("remote_host")->asString(); // Local copy of ephemeral conf String. lwip_getaddrinfo() might hold a reference to the C string.


    remote_connections[i].begin(internal_ip,
             internal_subnet,
             local_port,
             internal_gateway,
             private_key.c_str(),
             remote_host.c_str(),
             conf->get("remote_public_key")->asEphemeralCStr(),
             conf->get("remote_port")->asUint(),
             allowed_ip,
             allowed_subnet,
             false,
             nullptr);

    task_scheduler.scheduleWithFixedDelay([this, i]() {
        bool up = remote_connections[i].is_peer_up(nullptr, nullptr);

        if (this->connection_state.get("remote_connection_states")->get(i)->get("connection_state")->updateUint(up ? 2 : 1)) {
            if (up) {
                logger.printfln("Connection %i connected", i);
            } else {
                logger.printfln("Connection %i disconnected", i);
            }
        }
    }, 1000, 1000);
}

int RemoteAccess::setup_inner_socket() {
    logger.printfln("Setting up inner socket.");
    if (inner_socket < 0) {
        close(inner_socket);
    }

    inner_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (inner_socket < 0) {
        return inner_socket;
    }

    struct sockaddr_in local_addr;
    bzero(&local_addr, sizeof(local_addr));

    local_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(12345);
    int ret = bind(inner_socket, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if (ret == -1) {
        logger.printfln("Binding socket to port 12345 caused and error: (%i)%s", errno, strerror_r(errno, nullptr, 0));
        close(inner_socket);
    }

    ret = fcntl(inner_socket, F_SETFL, O_NONBLOCK);
    if (ret == -1) {
        logger.printfln("Setting socket to non_blocking caused and error: (%i)%s", errno, strerror_r(errno, nullptr, 0));
        close(inner_socket);
        inner_socket = -1;
    }

    return 0;
}

void RemoteAccess::run_management() {
    if (inner_socket < 0) {
        return;
    }

    uint8_t buf[sizeof(ManagementCommand)];
    struct sockaddr from;
    socklen_t from_len = sizeof(sockaddr);
    int ret = recvfrom(inner_socket, &buf, sizeof(ManagementCommand), 0, &from, &from_len);

    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        close(inner_socket);
        inner_socket = -1;
        return;
    }

    logger.printfln("received %i bytes", ret);

    if (ret != sizeof(ManagementCommand)) {
        logger.printfln("Didnt receive Management command.");
        return;
    }

    ManagementCommand *command = reinterpret_cast<ManagementCommand *>(buf);
    if (static_cast<uint32_t>(command->connection_no) > 5) {
        return;
    }

    logger.printfln("%i, %i", command->command_id, command->connection_no);

    switch (command->command_id) {
        case ManagementCommandId::Connect:
            {
                uint32_t local_port = remote_connection_config.get("connections")->get(command->connection_no)->get("local_port")->asUint();
                ManagementResponse response;
                response.charger_id = local_uid_num;
                response.connection_no = command->connection_no;
                memcpy(&response.connection_uuid, &command->connection_uuid, 16);

                CoolString remote_host = config.get("relay_host")->asString();
                create_sock_and_send_to(&response, sizeof(response), remote_host.c_str(), 51820, (uint16_t *)&local_port);
            }
            connect_remote_access(command->connection_no);
            break;

        case ManagementCommandId::Disconnect:
            remote_connections[command->connection_no].end();
            break;
    }
}
