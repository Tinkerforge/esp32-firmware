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

#include "build.h"
#include "esp_http_client.h"
#include "task_scheduler.h"
#include <WiFi.h>

extern char local_uid_str[32];

void RemoteAccess::pre_setup() {
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"email", Config::Str("", 0, 64)},
        {"password", Config::Str("", 0, 64)},
        {"relay_host", Config::Str("", 0, 64)},
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
}

void RemoteAccess::setup() {
    api.restorePersistentConfig("remote_access/config", &config);
    api.restorePersistentConfig("remote_access/management_connection", &management_connection);
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
    std::unique_ptr<unsigned char[]> cert = certs.get_cert(0, &cert_len);
    if (cert == nullptr) {
        logger.printfln("Management: Failed to get cert");
        return;
    }

    CoolString relay_host = config.get("relay_host")->asString();
    CoolString login_url = "https://";
    login_url += relay_host;
    login_url += "/api/auth/login";

    char *cookie = nullptr;

    esp_http_client_config_t http_config;
    http_config.url = login_url.c_str();
    http_config.event_handler = http_event_handle;
    http_config.cert_pem = (const char *)cert.get();
    http_config.skip_cert_common_name_check = true;
    http_config.method = HTTP_METHOD_POST;
    http_config.user_data = &cookie;

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
    std::unique_ptr<unsigned char[]> cert = certs.get_cert(0, &cert_len);
    if (cert == nullptr) {
        logger.printfln("Management: Failed to get cert");
        return;
    }

    CoolString relay_host = config.get("relay_host")->asString();
    CoolString login_url = "https://";
    login_url += relay_host;
    login_url += "/api/management";

    esp_http_client_config_t http_config;
    http_config.url = login_url.c_str();
    http_config.event_handler = http_event_handle;
    http_config.cert_pem = (const char *)cert.get();
    http_config.skip_cert_common_name_check = true;
    http_config.method = HTTP_METHOD_PUT;

    esp_http_client_handle_t client = esp_http_client_init(&http_config);

    CoolString management_data = "{\"id\":\"";
    management_data += String(local_uid_str) + "\"}";

    logger.printfln("data: %s", management_data.c_str());

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
    if (!clock_synced(&tv))
        return;

    done = true;

    IPAddress internal_ip;
    IPAddress internal_subnet;
    IPAddress internal_gateway;
    IPAddress allowed_ip;
    IPAddress allowed_subnet;
    uint16_t local_port;

    printf("%s\n", management_connection.to_string().c_str());

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

        if (up) {
            logger.printfln("Bla");
        }
    }, 1000, 1000);
}
