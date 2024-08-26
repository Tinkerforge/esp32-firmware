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

#include "remote_access.h"

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/pbuf.h>
#include <lwip/netdb.h>
#include <WiFi.h>
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>
#include <sodium.h>
#include <esp_http_client.h>
#include <LittleFS.h>
#include <TFJson.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "tools.h"
#include "esp_tls_errors.h"
#include "esp_tls.h"

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);
extern char local_uid_str[32];
extern uint32_t local_uid_num;

#define MAX_KEYS_PER_USER 5
#define MAX_USERS 1
#define WG_KEY_LENGTH 44
#define KEY_SIZE (3 * WG_KEY_LENGTH)
#define KEY_DIRECTORY "/remote-access-keys"

static inline String get_key_path(uint8_t user_id, uint8_t key_id) {
    return String(KEY_DIRECTORY "/") + user_id + "_" + key_id;
}

void remove_key(uint8_t user_id, uint8_t key_id) {
    auto path = get_key_path(user_id, key_id);
    if (!LittleFS.exists(path))
        return;

    LittleFS.remove(path);
}

void store_key(uint8_t user_id, uint8_t key_id, const char *pri, const char *psk, const char *pub) {
    File f = LittleFS.open(get_key_path(user_id, key_id), "w+");
    // TODO: more robust writing
    f.write((const uint8_t *)pri, WG_KEY_LENGTH);
    f.write((const uint8_t *)psk, WG_KEY_LENGTH);
    f.write((const uint8_t *)pub, WG_KEY_LENGTH);
}

static bool get_key(uint8_t user_id, uint8_t key_id, char *pri, char *psk, char *pub)
{
    String path = get_key_path(user_id, key_id);

    if (!LittleFS.exists(path)) {
        goto fail;
    }

    {
        File f = LittleFS.open(path, "r");

        if (f.size() != KEY_SIZE) {
            goto fail;
        }

        if (f.read((uint8_t *)pri, WG_KEY_LENGTH) != WG_KEY_LENGTH) {
            goto fail;
        }
        if (f.read((uint8_t *)psk, WG_KEY_LENGTH) != WG_KEY_LENGTH) {
            goto fail;
        }
        if (f.read((uint8_t *)pub, WG_KEY_LENGTH) != WG_KEY_LENGTH) {
            goto fail;
        }
    }

    pri[WG_KEY_LENGTH] = '\0';
    psk[WG_KEY_LENGTH] = '\0';
    pub[WG_KEY_LENGTH] = '\0';

    return true;

fail:
    pri[0] = '\0';
    psk[0] = '\0';
    pub[0] = '\0';
    return false;
}

static int create_sock_and_send_to(const void *payload, size_t payload_len, const char *dest_host, uint16_t port, uint16_t *local_port) {
    struct sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));

    ip_addr_t ip;
    int ret = dns_gethostbyname_addrtype_lwip_ctx(dest_host, &ip, nullptr, nullptr, LWIP_DNS_ADDRTYPE_IPV4);
    if (ret == ERR_VAL) {
        logger.printfln("No DNS server is configured!");
    } else if (ret != ERR_OK) {
        logger.printfln("Error during DNS resolve!");
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
    close(sock);
    return ret;
}

void RemoteAccess::pre_setup() {
    config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"email", Config::Str("", 0, 64)},
        {"password", Config::Str("", 0, 32)},
        {"relay_host", Config::Str("my.warp-charger.com", 0, 64)},
        {"relay_host_port", Config::Uint16(443)}, //relay_host_port -> relay_port
        {"cert_id", Config::Int8(-1)}
    })};

    Config *cs = new Config {
        Config::Object({
            {"connection_state", Config::Uint8(0)}
        })
    };

    // TODO
    connection_state = ConfigRoot{
        Config::Object({
            {"management_connection_state", Config::Uint8(0)},
            {"remote_connection_states", Config::Array({
                    *cs,*cs,*cs,*cs,*cs
                },
                cs, 5, 5, Config::type_id<Config::ConfObject>())}
        })
    };
}

void RemoteAccess::setup() {
    LittleFS.mkdir(KEY_DIRECTORY);

    api.restorePersistentConfig("remote_access/config", &config);
    initialized = true;

    if (!config.get("enable")->asBool())
        return;

    logger.printfln("Remote Access is enabled trying to connect");
}

static std::unique_ptr<char []> decode_base64(const CoolString &input, size_t buffer_size) {
    std::unique_ptr<char []> out = heap_alloc_array<char>(buffer_size);
    size_t _unused = 0;
    int ret = mbedtls_base64_decode((unsigned char *)out.get(), buffer_size, &_unused, (unsigned char *)input.c_str(), input.length());
    if (ret != 0) {
        logger.printfln("Error while decoding: %i", ret);
    }
    return out;
}

void RemoteAccess::register_urls() {
    api.addState("remote_access/config", &config, {
        "password"
    });

    api.addState("remote_access/state", &connection_state);

    server.on("/remote_access/register", HTTP_PUT, [this](WebServerRequest request) {
        // TODO: Maybe don't run the registration in the request handler. Start a task instead?

        auto content_len = request.contentLength();
        std::unique_ptr<char[]> req_body = heap_alloc_array<char>(content_len);
        if (request.receive(req_body.get(), content_len) <= 0) {
            // TODO: Fix error codes?
            return request.send(500);
        }

        // TODO: use TFJsonDeserializer?
        StaticJsonDocument<768> doc;

        {
            DeserializationError error = deserializeJson(doc, req_body.get(), content_len);

            if (error) {
                // TODO: Fix error codes?
                return request.send(500);
            }
        }

        ConfigRoot new_config = this->config;

        {
            String error = new_config.update_from_json(doc["config"], true, ConfigSource::API);
            if (error != "") {
                return request.send(400, "text/plain", error.c_str());
            }
        }

        if (!new_config.get("enable")->asBool()) {
            config = new_config;
            API::writeConfig("remote_access/config", &config);

            remove_key(0, 0);
            for(int user_id = 1; user_id < MAX_USERS + 1; ++user_id) // user 0 is the management connection
                for(int key_id = 0; i < MAX_KEYS_PER_USER; ++key_id)
                    remove_key(user_id, key_id);

            return request.send(200); // TODO result json?
        }

        std::unique_ptr<char[]> ptr = heap_alloc_array<char>(5000);

        // TODO: Should we validate the secret{,_nonce,_key} lengths before decoding?
        // Also validate the decoded lengths!
        // TODO: The decode_base64 takes a string, so doc["secret..."] is probably copied here
        std::unique_ptr<char[]> encrypted_secret = decode_base64(doc["secret"],       32 + crypto_secretbox_MACBYTES);
        std::unique_ptr<char[]> secret_nonce     = decode_base64(doc["secret_nonce"], crypto_secretbox_NONCEBYTES);
        std::unique_ptr<char[]> secret_key       = decode_base64(doc["secret_key"],   crypto_secretbox_KEYBYTES);

        if (sodium_init() < 0) {
            return request.send(500);
        }
        char secret[32];
        int ret = crypto_secretbox_open_easy((unsigned char *)secret, (unsigned char *)encrypted_secret.get(), 32 + crypto_secretbox_MACBYTES, (unsigned char*)secret_nonce.get(), (unsigned char*)secret_key.get());
        if (ret != 0) {
            logger.printfln("Failed to decrypt secret");
            return request.send(500);
        }

        unsigned char pk[crypto_box_PUBLICKEYBYTES];
        ret = crypto_scalarmult_base(pk, (unsigned char *)secret);
        if (ret < 0) {
            return request.send(500);
        }

        TFJsonSerializer serializer = TFJsonSerializer(ptr.get(), 5000);
        serializer.addObject();
        serializer.addMemberArray("keys");
        std::unique_ptr<char[]> buf = heap_alloc_array<char>(50);
        {
            int i = 0;
            // JsonArray already has reference semantics. No need for &.
            for (const auto key : doc["keys"].as<JsonArray>()) {
                serializer.addObject();
                // TODO optimize
                std::snprintf(buf.get(), 50, "10.123.%i.2", i);
                serializer.addMemberString("charger_address", buf.get());
                serializer.addMemberString("charger_public", key["charger_public"]);
                serializer.addMemberNumber("connection_no", i);
                std::snprintf(buf.get(), 50, "10.123.%i.3", i);
                serializer.addMemberString("web_address", buf.get());
                CoolString wg_key = CoolString{key["web_private"].as<String>()};

                // TODO optimize: this is always 44 bytes + crypto_box_SEALBYTES -> stack alloc
                std::unique_ptr<char[]> output = heap_alloc_array<char>(crypto_box_SEALBYTES + wg_key.length());

                ret = crypto_box_seal((unsigned char *)output.get(), (unsigned char *)wg_key.c_str(), wg_key.length(), pk);
                if (ret < 0) {
                    return request.send(500);
                }

                // TODO: maybe base64 encode?
                serializer.addMemberArray("web_private");
                for (size_t a = 0; a < crypto_box_SEALBYTES + wg_key.length(); a++) {
                    serializer.addNumber(output[a]);
                }
                serializer.endArray();

                CoolString psk = key["psk"];
                std::unique_ptr<char[]> encrypted_psk = heap_alloc_array<char>(crypto_box_SEALBYTES + psk.length());
                ret = crypto_box_seal((unsigned char*)encrypted_psk.get(), (unsigned char*)psk.c_str(), psk.length(), pk);
                if (ret < 0) {
                    return request.send(500);
                }

                // TODO: maybe base64 encode?
                serializer.addMemberArray("psk");
                for (size_t a = 0; a < crypto_box_SEALBYTES + psk.length(); a++) {
                    serializer.addNumber(encrypted_psk[a]);
                }
                serializer.endArray();

                serializer.endObject();
                i++;

                // Ignore rest of keys if there were sent more than we support.
                if (i == MAX_KEYS_PER_USER)
                    break;
            }
        }
        serializer.endArray();

        serializer.addMemberObject("charger");

        serializer.addMemberString("charger_pub", doc["mgmt_charger_public"]);
        serializer.addMemberString("id", local_uid_str);

        CoolString name = api.getState("info/display_name")->get("display_name")->asString();
        std::unique_ptr<char[]> encrypted_name = heap_alloc_array<char>(crypto_box_SEALBYTES + name.length());
        crypto_box_seal((unsigned char *)encrypted_name.get(), (unsigned char *)name.c_str(), name.length(), (unsigned char *)pk);

        serializer.addMemberArray("name");
        for (size_t a = 0; a < crypto_box_SEALBYTES + name.length(); a++) {
            serializer.addNumber(encrypted_name[a]);
        }
        serializer.endArray();

        serializer.addMemberString("wg_charger_ip", "10.123.123.2");
        serializer.addMemberString("wg_server_ip", "10.123.123.3");
        serializer.addMemberString("psk", doc["mgmt_psk"]);

        serializer.endObject();
        serializer.endObject();
        size_t size = serializer.end();

        login(&new_config, CoolString{doc["login_key"].as<String>()});

        CoolString relay_host = new_config.get("relay_host")->asString();
        uint32_t relay_host_port = new_config.get("relay_host_port")->asUint();
        CoolString url = "https://";
        url += relay_host;
        url += ":";
        url += relay_host_port;
        url += "/api/charger/add";

        CoolString access_token = "access_token=";
        access_token += jwt + ";";

        std::vector<std::pair<CoolString, CoolString>> headers;
        headers.push_back(std::pair<CoolString, CoolString>(CoolString("Cookie"), access_token));
        headers.push_back(std::pair<CoolString, CoolString>(CoolString("Content-Type"), CoolString("application/json")));
        esp_err_t err;

        HttpResponse response = make_http_request(url.c_str(), HTTP_METHOD_PUT, ptr.get(), size, &headers, &err, &new_config);
        if (err != ESP_OK) {
            return request.send(500);
        } else if (response.status != 200) {
            return request.send(response.status, "text/plain", response.body.c_str(), response.body.length());
        }

        StaticJsonDocument<32> resp_doc;

        DeserializationError error = deserializeJson(resp_doc, response.body.begin(), response.body.length());

        if (error) {
            return request.send(500);
        }

        const char* charger_password = resp_doc["charger_password"];
        const char* management_pub = resp_doc["management_pub"];

        store_key(0, 0, doc["mgmt_charger_private"], doc["mgmt_psk"], management_pub);

        {
            int i = 0;
            for (const auto key : doc["keys"].as<JsonArray>()) {
                store_key(1, i,
                    key["charger_private"],
                    key["psk"],
                    key["web_public"]);
                ++i;
                // Ignore rest of keys if there were sent more than we support.
                if (i == MAX_KEYS_PER_USER)
                    break;
            }
        }

        config = new_config;
        config.get("password")->updateString(charger_password);
        API::writeConfig("remote_access/config", &config);

        return request.send(200);
    });

    if (!config.get("enable")->asBool()) {
        return;
    }

    task_scheduler.scheduleOnce([this]() {
        this->resolve_management();
        this->connect_management();
        this->connection_state.get("management_connection_state")->updateUint(1);
    }, 5000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (!this->management_request_done) {
            this->resolve_management();
        }
    }, 1000 * 10, 1000 * 10);

    task_scheduler.scheduleWithFixedDelay([this]() {
        for (int i = 0; i < 5; i++) {
            uint32_t state = 0;
            if (this->remote_connections[i] != nullptr) {
                state = this->remote_connections[i]->is_peer_up(nullptr, nullptr) ? 2 : 1;
            }

            if (this->connection_state.get("remote_connection_states")->get(i)->get("connection_state")->updateUint(state)) {
                if (state == 2) {
                    logger.printfln("Connection %i connected", i);
                } else if (state == 1) {
                    logger.printfln("Connection %i disconnected", i);
                }
            }
        }
    }, 1000, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        uint32_t state = 0;
        if (management != nullptr) {
            state = management->is_peer_up(nullptr, nullptr) ? 2 : 1;
        }

        if (this->connection_state.get("management_connection_state")->updateUint(state)) {
            if (state == 2) {
                logger.printfln("Management connection connected");
            } else {
                this->management_request_done = false;
                in_seq_number = 0;
                logger.printfln("Management connection disconnected");
            }
        }
    }, 1000, 1000);
}

static esp_err_t http_event_handle(esp_http_client_event *evt) {
    HttpResponse *response = reinterpret_cast<HttpResponse*>(evt->user_data);
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        {
            ESP_LOGI(TAG, "HTTP_EVENT_ERROR\n");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error(*(esp_tls_error_handle_t *)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x\n", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x\n", mbedtls_err);
            }
        }
        break;

    case HTTP_EVENT_ON_HEADER:
        if (!strcmp("set-cookie", evt->header_key) && !strncmp("access_token", evt->header_value, 12)) {
            response->cookie = strdup(evt->header_value);
        }
        break;

    case HTTP_EVENT_ON_DATA:
        response->body += String((char*)evt->data, evt->data_len);
        break;

    default:
        break;
    }
    return ESP_OK;
}

static String parse_cookie(const String &cookie) {
    size_t start = cookie.indexOf('=') + 1;
    size_t end = cookie.indexOf(';');
    return cookie.substring(start, end);
}

HttpResponse RemoteAccess::make_http_request(const char *url, esp_http_client_method_t method, const char *payload, size_t payload_size, std::vector<std::pair<CoolString, CoolString>> *headers, esp_err_t *ret_error, Config *config) {
    esp_err_t err = ESP_OK;
    size_t cert_len = 0;
    int cert_id = config->get("cert_id")->asInt();
    std::unique_ptr<unsigned char[]> cert = nullptr;

    HttpResponse response;
    if (cert_id >= 0) {
        cert = certs.get_cert(static_cast<uint8_t>(cert_id), &cert_len);
        if (cert == nullptr) {
            logger.printfln("Management: Failed to get self signed cert");
            *ret_error = ESP_FAIL;
            response.body = "Failed to get self signed cert";
            response.status = 500;
            return response;
        }
    }
    esp_http_client_config_t http_config;
    bzero(&http_config, sizeof(esp_http_client_config_t));

    http_config.url = url;
    http_config.event_handler = http_event_handle;
    http_config.method = method;
    http_config.user_data = &response;
    http_config.is_async = true;
    http_config.timeout_ms = 15000;

    auto deadline = now_us() + 15_usec * 1000_usec * 1000_usec;

    if (cert == nullptr) {
        http_config.crt_bundle_attach = esp_crt_bundle_attach;
        http_config.transport_type = HTTP_TRANSPORT_OVER_SSL;
    } else {
        http_config.cert_pem = (const char *)cert.get();
        http_config.skip_cert_common_name_check = true;
    }

    esp_http_client_handle_t client = esp_http_client_init(&http_config);

    if (payload != nullptr && payload_size > 0) {
        err = esp_http_client_set_post_field(client, payload, payload_size);
        if (err != ESP_OK) {
            logger.printfln("Failed to set post data: %i", err);
            esp_http_client_cleanup(client);
            *ret_error = err;
            response.body = "Failed to set post data";
            response.status = 500;
            return response;
        }
    }

    if (headers != nullptr) {
        for (auto &header : *headers) {
            err = esp_http_client_set_header(client, header.first.c_str(), header.second.c_str());
            if (err != ESP_OK) {
                logger.printfln("Failed to set header");
                *ret_error = err;
                response.body = "Failed to set header";
                response.status = 500;
                return response;
            }
        }
    }

    do {
        err = esp_http_client_perform(client);
        // TODO: improve http timeout handling.
        if (deadline_elapsed(deadline)) {
            err = ESP_FAIL;
        }
    } while (err == ESP_ERR_HTTP_EAGAIN);

    if (err != ESP_OK) {
        logger.printfln("Failed to send request with error %i: %s", errno, strerror_r(errno, nullptr, 0));
        esp_http_client_cleanup(client);
        *ret_error = err;
        return response;
    }
    response.body += "\0";

    if (response.cookie != nullptr) {
        jwt = parse_cookie(String(response.cookie));
        free(response.cookie);
    }

    int status = esp_http_client_get_status_code(client);
    if (status < 200 || status > 299) {
        // TODO: Dont log nginx error page!
        logger.printfln("Request to %s failed with code %i: %s", url, status, response.body.c_str());
    }
    response.status = status;

    esp_http_client_cleanup(client);
    *ret_error = err;
    return response;
}

void RemoteAccess::login(Config *config, const CoolString &login_key_base64) {
    CoolString relay_host = config->get("relay_host")->asString();
    uint32_t relay_host_port = config->get("relay_host_port")->asUint();
    CoolString login_url = "https://";
    login_url += relay_host;
    login_url += ":";
    login_url += relay_host_port;
    login_url += "/api/auth/login";

    std::unique_ptr<char[]> login_key = decode_base64(login_key_base64, 24);
    std::unique_ptr<char[]> json = heap_alloc_array<char>(250);

    TFJsonSerializer serializer = TFJsonSerializer(json.get(), 250);
    serializer.addObject();
    serializer.addMemberString("email", config->get("email")->asEphemeralCStr());
    serializer.addMemberArray("login_key");
    for (int i = 0; i < 24; i++) {
        serializer.addNumber(login_key.get()[i]);
    }
    serializer.endArray();
    serializer.endObject();
    size_t json_size = serializer.end();

    esp_err_t err;
    make_http_request(login_url.c_str(), HTTP_METHOD_POST, json.get(), json_size, nullptr, &err, config);
}

void RemoteAccess::resolve_management() {
    size_t cert_len = 0;
    int cert_id = config.get("cert_id")->asInt();
    std::unique_ptr<unsigned char[]> cert = nullptr;
    if (cert_id >= 0) {
        cert = certs.get_cert(static_cast<uint8_t>(cert_id), &cert_len);
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

    std::unique_ptr<char[]> json = heap_alloc_array<char>(256);
    TFJsonSerializer serializer = TFJsonSerializer(json.get(), 250);
    serializer.addObject();
    serializer.addMemberNumber("id", local_uid_num);
    serializer.addMemberString("password", config.get("password")->asEphemeralCStr());
        serializer.addMemberObject("data");
            serializer.addMemberObject("V1");
                serializer.addMemberNumber("port", network.config.get("web_server_port")->asUint());
                serializer.addMemberString("firmware_version", BUILD_VERSION_STRING);
                //TODO: Adapt this once we support more than one user.
                serializer.addMemberArray("configured_connections");
                    for (int i = 0; i < 5; i++) {
                        serializer.addNumber(i);
                    }
                serializer.endArray();
            serializer.endObject();
        serializer.endObject();
    serializer.endObject();
    size_t len = serializer.end();

    std::vector<std::pair<CoolString, CoolString>> headers;
    headers.push_back(std::pair<CoolString, CoolString>(CoolString("Content-Type"), CoolString("application/json")));
    esp_err_t err;
    HttpResponse response = make_http_request(url.c_str(), HTTP_METHOD_PUT, json.get(), len, &headers, &err, &config);
    if (response.status == 200 && err == 0) {
        management_request_done = true;
    }
}

static int management_filter_in(struct pbuf* packet) {
    // When this function is called it is already ensured that the payload contains a valid ip packet.
    uint8_t *payload = (uint8_t*)packet->payload;

    if (payload[9] != 0x11) {
        logger.printfln("Management blocked invalid incoming packet with protocol: 0x%X", payload[9]);
        return -1;
    }

    int header_len = (payload[0] & 0xF) * 4;
    if (packet->len - (header_len + 8) != sizeof(management_command_packet)) {
        logger.printfln("Management blocked invalid incoming packet of size: %i", (packet->len - (header_len + 8)));
        return -1;
    }

    int dest_port = payload[header_len] << 8;
    dest_port |= payload[header_len + 1];
    if (dest_port != 12345) {
        logger.printfln("Management blocked invalid incoming packet with destination port: %i.", dest_port);
        return -1;
    }
    return 0;
}

static int management_filter_out(struct pbuf* packet) {
    uint8_t *payload = (uint8_t*)packet->payload;

    if (payload[9] == 0x1) {
        return 0;
    } else {
        logger.printfln("Management blocked outgoing packet");
    }
    return -1;
}

bool port_valid(uint16_t port) {
    bool port_valid = false;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return port_valid;
    }

    struct sockaddr_in local_addr;
    bzero(&local_addr, sizeof(local_addr));

    local_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(port);
    int ret = bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if (ret == 0) {
        port_valid = true;
    }
    close(sock);
    return port_valid;
}

uint16_t find_next_free_port(uint16_t port) {
    while (!port_valid(port)) {
        port++;
    }
    return port;
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

    internal_ip.fromString("10.123.123.2");
    internal_subnet.fromString("255.255.255.0");
    internal_gateway.fromString("10.123.123.1");
    allowed_ip.fromString("0.0.0.0");
    allowed_subnet.fromString("0.0.0.0");
    local_port = 51820;

    // WireGuard decodes those (base64 encoded) keys and stores them.
    char private_key[WG_KEY_LENGTH + 1];
    char psk[WG_KEY_LENGTH + 1];
    char remote_public_key[WG_KEY_LENGTH + 1];

    if (!get_key(0, 0, private_key, psk, remote_public_key)) {
        logger.printfln("Can't connect to management server: no WireGuard key installed!");
        return;
    }

    // Only used for BLOCKING! DNS resolve. TODO Make this non-blocking in Wireguard-ESP32-Arduino/src/WireGuard.cpp!
    auto remote_host = config.get("relay_host")->asEphemeralCStr();

    logger.printfln("Connecting to Management WireGuard peer %s:%u", remote_host, 51820);

    if (management != nullptr) {
        delete management;
    }
    management = new WireGuard();

    local_port = find_next_free_port(local_port);
    this->setup_inner_socket();
    management->begin(internal_ip,
             internal_subnet,
             local_port,
             internal_gateway,
             private_key,
             remote_host,
             remote_public_key,
             51820,
             allowed_ip,
             allowed_subnet,
             false,
             psk,
             &management_filter_in,
             &management_filter_out);

    task_scheduler.scheduleWithFixedDelay([this]() {
        this->run_management();
    }, 0, 250);
}

void RemoteAccess::connect_remote_access(uint8_t i, uint16_t local_port) {
    struct timeval tv;
    if (!clock_synced(&tv)) {
        task_scheduler.scheduleOnce([this, i, local_port]() {
            this->connect_remote_access(i, local_port);
        }, 5000);
        return;
    }

    IPAddress internal_ip;
    IPAddress internal_subnet;
    IPAddress internal_gateway;
    IPAddress allowed_ip;
    IPAddress allowed_subnet;

    std::unique_ptr<char[]> buf = heap_alloc_array<char>(50);

    // TODO: make this more efficient. For example (internal_ip.fromString("10.123.0.2") | i << 8). Maybe there is a macro that converts dotted decimal form to an int?
    snprintf(buf.get(), 50, "10.123.%u.2", i);
    internal_ip.fromString(buf.get());
    internal_subnet.fromString("255.255.255.0");

    // TODO: make this more efficient, see above.
    snprintf(buf.get(), 50, "10.123.%u.1", i);
    internal_gateway.fromString(buf.get());
    allowed_ip.fromString("0.0.0.0");
    allowed_subnet.fromString("0.0.0.0");

    // WireGuard decodes those (base64 encoded) keys and stores them.
    char private_key[WG_KEY_LENGTH + 1];
    char psk[WG_KEY_LENGTH + 1];
    char remote_public_key[WG_KEY_LENGTH + 1];

    if (!get_key(1, i, private_key, psk, remote_public_key)) {
        logger.printfln("Can't connect to web interface: no WireGuard key installed!");
        return;
    }

    // Only used for BLOCKING! DNS resolve. TODO Make this non-blocking in Wireguard-ESP32-Arduino/src/WireGuard.cpp!
    auto remote_host = config.get("relay_host")->asEphemeralCStr();

    logger.printfln("Connecting: private %s psk %s pub %s", private_key, psk, remote_public_key);

    if (remote_connections[i] != nullptr) {
        delete remote_connections[i];
    }
    remote_connections[i] = new WireGuard();

    remote_connections[i]->begin(internal_ip,
             internal_subnet,
             local_port,
             internal_gateway,
             private_key,
             remote_host,
             remote_public_key,
             51820,
             allowed_ip,
             allowed_subnet,
             false,
             psk);
}

int RemoteAccess::setup_inner_socket() {
    if (inner_socket < 0) {
        close(inner_socket);
    } else if (inner_socket > 0) {
        return 0;
    }

    inner_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (inner_socket < 0) {
        logger.printfln("Failed to create inner socket: (%i)%s", errno, strerror_r(errno, nullptr, 0));
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

    uint8_t buf[sizeof(management_command_packet)];
    struct sockaddr from;
    socklen_t from_len = sizeof(sockaddr);
    int ret = recvfrom(inner_socket, &buf, sizeof(management_command_packet), 0, &from, &from_len);

    if (ret == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        close(inner_socket);
        inner_socket = 0;
        setup_inner_socket();
        return;
    } if (ret != sizeof(management_command_packet)) {
        logger.printfln("Didnt receive Management command.");
        return;
    }

    management_command_packet *command_packet = reinterpret_cast<management_command_packet *>(buf);
    if (command_packet->header.magic != 0x1234 || command_packet->header.length != sizeof(management_command)) {
        return;
    }

    if ((int16_t)(command_packet->header.seq_num - in_seq_number) > 0) {
        in_seq_number = command_packet->header.seq_num;
    } else {
        return;
    }

    management_command *command = &command_packet->command;
    if (static_cast<uint32_t>(command->connection_no) > 5) {
        return;
    }

    switch (command->command_id) {
        case management_command_id::Connect:
            {
                if (remote_connections[command->connection_no] != nullptr && remote_connections[command->connection_no]->is_peer_up(nullptr, nullptr)) {
                    return;
                }

                logger.printfln("Opening connection %u", command->connection_no);
                uint16_t local_port = 51821;
                port_discovery_packet response;
                response.charger_id = local_uid_num;
                response.connection_no = command->connection_no;
                memcpy(&response.connection_uuid, &command->connection_uuid, 16);

                CoolString remote_host = config.get("relay_host")->asString();
                if (remote_connections[command->connection_no] != nullptr) {
                    remote_connections[command->connection_no]->end();
                }
                local_port = find_next_free_port(local_port);
                create_sock_and_send_to(&response, sizeof(response), remote_host.c_str(), 51820, &local_port);
                connect_remote_access(command->connection_no, local_port);
            }
            break;

        case management_command_id::Disconnect:
            if (remote_connections[command->connection_no] == nullptr) {
                break;
            }
            logger.printfln("Closing connection %u", command->connection_no);
            remote_connections[command->connection_no]->end();
            delete remote_connections[command->connection_no];
            remote_connections[command->connection_no] = nullptr;
            break;
    }
}

