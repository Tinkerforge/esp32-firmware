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
        {"relay_port", Config::Uint16(443)},
        {"cert_id", Config::Int8(-1)}
    })};

    Config *cs = new Config {Config::Uint8(1)};

    connection_state = Config::Array(
                {},
                cs,
                MAX_USERS * MAX_KEYS_PER_USER + 1,
                MAX_USERS * MAX_KEYS_PER_USER + 1,
                Config::type_id<Config::ConfUint>()
    );
    for(int i = 0; i < MAX_USERS * MAX_KEYS_PER_USER + 1; ++i) {
        connection_state.add();
    }

    registration_state = Config::Object({
        {"state", Config::Uint8(0)},
        {"message", Config::Str("", 0, 64)}
    });
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
    if (out == nullptr) {
        return out;
    }
    size_t _unused = 0;
    int ret = mbedtls_base64_decode((unsigned char *)out.get(), buffer_size, &_unused, (unsigned char *)input.c_str(), input.length());
    if (ret != 0) {
        logger.printfln("Error while decoding: %i", ret);
    }
    return out;
}

void RemoteAccess::handle_response_chunk(const AsyncHTTPSClientEvent *event) {
    if (response_body.length() == 0) {
        response_body.reserve(event->data_complete_len);
    }
    response_body.concat((const uint8_t*)event->data_chunk, (unsigned int)event->data_chunk_len);
}

void RemoteAccess::register_urls() {
    api.addState("remote_access/config", &config, {
        "password"
    });

    api.addState("remote_access/state", &connection_state);
    api.addState("remote_access/registration_state", &registration_state);

    server.on("/remote_access/reset_registration_state", HTTP_PUT, [this](WebServerRequest request) {
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::None);
        registration_state.get("message")->updateString("");
        return request.send(200);
    });

    server.on("/remote_access/get_login_salt", HTTP_PUT, [this](WebServerRequest request) {
        size_t content_len = request.contentLength();
        std::unique_ptr<char[]> req_body = heap_alloc_array<char>(content_len);
        if (req_body == nullptr) {
            return request.send(500,  "text/plain; charset=utf-8", "Low memory");
        }
        if (request.receive(req_body.get(), content_len) <= 0) {
            return request.send(500, "text/plain; charset=utf-8", "Failed to read request body");
        }

        ConfigRoot new_config = config;
        {
            String error = new_config.update_from_cstr(req_body.get(), content_len);
            if (error != "") {
                return request.send(400, "text/plain; charset=utf-8", error.c_str());
            }
        }

        this->get_login_salt(new_config);
        return request.send(200);
    });

    server.on("/remote_access/get_secret_salt", HTTP_PUT, [this](WebServerRequest request) {
        size_t content_len = request.contentLength();
        std::unique_ptr<char[]> req_body = heap_alloc_array<char>(content_len);
        if (req_body == nullptr) {
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        if (request.receive(req_body.get(), content_len) <= 0) {
            return request.send(500, "text/plain; charset=utf-8", "Failed to read request body");
        }

        ConfigRoot new_config = config;
        {
            String error = new_config.update_from_cstr(req_body.get(), content_len);
            if (error != "") {
                return request.send(400, "text/plain; charset=utf-8", error.c_str());
            }
        }

        this->get_secret(new_config);

        return request.send(200);
    });

    server.on("/remote_access/login", HTTP_PUT, [this](WebServerRequest request) {
        auto content_len = request.contentLength();
        std::unique_ptr<char[]> req_body = heap_alloc_array<char>(content_len);
        if (req_body == nullptr) {
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        if (request.receive(req_body.get(), content_len) <= 0) {
            return request.send(500, "text/plain; charset=utf-8", "Failed to read request body");
        }

        // TODO: use TFJsonDeserializer?
        StaticJsonDocument<768> doc;

        {
            DeserializationError error = deserializeJson(doc, req_body.get(), content_len);

            if (error) {
                char err_str[64];
                snprintf(err_str, 64, "Failed to deserialize request body: %s", error.c_str());
                return request.send(400, "text/plain; charset=utf-8", err_str);
            }
        }

        ConfigRoot new_config = this->config;

        {
            String error = new_config.update_from_json(doc["config"], true, ConfigSource::API);
            if (error != "") {
                return request.send(400, "text/plain; charset=utf-8", error.c_str());
            }
        }

        CoolString login_key = doc["login_key"];
        this->login(new_config, login_key);

        return request.send(200);
    });

    server.on("/remote_access/register", HTTP_PUT, [this](WebServerRequest request) {
        // TODO: Maybe don't run the registration in the request handler. Start a task instead?
        auto content_len = request.contentLength();
        std::unique_ptr<char[]> req_body = heap_alloc_array<char>(content_len);
        if (req_body == nullptr) {
            https_client = nullptr;
            encrypted_secret = nullptr;
            secret_nonce = nullptr;
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        if (request.receive(req_body.get(), content_len) <= 0) {
            https_client = nullptr;
            encrypted_secret = nullptr;
            secret_nonce = nullptr;
            return request.send(500, "text/plain; charset=utf-8", "Failed to read request body");
        }

        // TODO: use TFJsonDeserializer?
        StaticJsonDocument<768> doc;

        {
            DeserializationError error = deserializeJson(doc, req_body.get(), content_len);

            if (error) {
                char err_str[64];
                snprintf(err_str, 64, "Failed to deserialize request body: %s", error.c_str());
                https_client = nullptr;
                encrypted_secret = nullptr;
                secret_nonce = nullptr;
                return request.send(400, "text/plain; charset=utf-8", err_str);
            }
        }

        ConfigRoot new_config = this->config;

        {
            String error = new_config.update_from_json(doc["config"], true, ConfigSource::API);
            if (error != "") {
                https_client = nullptr;
                encrypted_secret = nullptr;
                secret_nonce = nullptr;
                return request.send(400, "text/plain; charset=utf-8", error.c_str());
            }
        }

        if (!new_config.get("enable")->asBool()) {
            TFJsonSerializer test_serializer = TFJsonSerializer(nullptr, 0);
            test_serializer.addObject();
            test_serializer.addMemberNumber("id", local_uid_num);
            test_serializer.addMemberString("password", config.get("password")->asEphemeralCStr()),
            test_serializer.endObject();
            size_t json_size = test_serializer.end();

            std::unique_ptr<char[]> delete_buf = heap_alloc_array<char>(json_size + 1);

            TFJsonSerializer serializer = TFJsonSerializer(delete_buf.get(), json_size + 1);
            serializer.addObject();
            serializer.addMemberNumber("id", local_uid_num);
            serializer.addMemberString("password", config.get("password")->asEphemeralCStr()),
            serializer.endObject();
            size_t size = serializer.end();

            CoolString relay_host = config.get("relay_host")->asString();
            uint32_t relay_port = config.get("relay_port")->asUint();
            CoolString url = "https://";
            url += relay_host;
            url += ":";
            url += relay_port;
            url += "/api/selfdestruct";

            if (https_client == nullptr) {
                https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
            }
            https_client->set_header("Content-Type", "application/json");
            // Deregistering from the server is optional. Don't handle any request errors.
            auto callback = [this](ConfigRoot _) {
                https_client = nullptr;
                encrypted_secret = nullptr;
                secret_nonce = nullptr;
            };
            this->run_request_with_next_stage(url.c_str(), HTTP_METHOD_DELETE, delete_buf.get(), size, config, callback);

            config = new_config;
            API::writeConfig("remote_access/config", &config);

            remove_key(0, 0);
            for(int user_id = 1; user_id < MAX_USERS + 1; ++user_id) // user 0 is the management connection
                for(int key_id = 0; key_id < MAX_KEYS_PER_USER; ++key_id)
                    remove_key(user_id, key_id);

            return request.send(200); // TODO result json?
        }

        std::unique_ptr<char[]> ptr = heap_alloc_array<char>(5000);
        if (ptr == nullptr) {
            https_client = nullptr;
            encrypted_secret = nullptr;
            secret_nonce = nullptr;
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }

        // TODO: Should we validate the secret{,_nonce,_key} lengths before decoding?
        // Also validate the decoded lengths!
        std::unique_ptr<char[]> secret_key       = decode_base64(doc["secret_key"],   crypto_secretbox_KEYBYTES);
        if (secret_key == nullptr) {
            https_client = nullptr;
            encrypted_secret = nullptr;
            secret_nonce = nullptr;
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }

        if (sodium_init() < 0) {
            https_client = nullptr;
            encrypted_secret = nullptr;
            secret_nonce = nullptr;
            logger.printfln("Failed to initialize libsodium");
            return request.send(500, "text/plain; charset=utf-8", "Failed to initialize crypto");
        }
        char secret[crypto_box_SECRETKEYBYTES];
        int ret = crypto_secretbox_open_easy((unsigned char *)secret, (unsigned char *)encrypted_secret.get(), crypto_box_SECRETKEYBYTES + crypto_secretbox_MACBYTES, (unsigned char*)secret_nonce.get(), (unsigned char*)secret_key.get());
        if (ret != 0) {
            https_client = nullptr;
            encrypted_secret = nullptr;
            secret_nonce = nullptr;
            logger.printfln("Failed to decrypt secret");
            return request.send(500, "text/plain; charset=utf-8", "Failed to decrypt secret");
        }

        unsigned char pk[crypto_box_PUBLICKEYBYTES];
        ret = crypto_scalarmult_base(pk, (unsigned char *)secret);
        if (ret < 0) {
            https_client = nullptr;
            encrypted_secret = nullptr;
            secret_nonce = nullptr;
            logger.printfln("Failed to derive public-key");
            return request.send(500, "text/plain; charset=utf-8", "Failed to derive public-key");
        }

        TFJsonSerializer serializer = TFJsonSerializer(ptr.get(), 5000);
        serializer.addObject();
        serializer.addMemberArray("keys");
        char buf[50];
        {
            int i = 0;
            // JsonArray already has reference semantics. No need for &.
            for (const auto key : doc["keys"].as<JsonArray>()) {
                serializer.addObject();
                // TODO optimize
                std::snprintf(buf, 50, "10.123.%i.2", i);
                serializer.addMemberString("charger_address", buf);
                serializer.addMemberString("charger_public", key["charger_public"]);
                serializer.addMemberNumber("connection_no", i);
                std::snprintf(buf, 50, "10.123.%i.3", i);
                serializer.addMemberString("web_address", buf);
                CoolString wg_key = CoolString{key["web_private"].as<String>()};

                // TODO optimize: this is always 44 bytes + crypto_box_SEALBYTES -> stack alloc
                char output[44];

                ret = crypto_box_seal((unsigned char *)output, (unsigned char *)wg_key.c_str(), wg_key.length(), pk);
                if (ret < 0) {
                    https_client = nullptr;
                    encrypted_secret = nullptr;
                    secret_nonce = nullptr;
                    logger.printfln("Failed to encrypt Wireguard keys: %i", ret);
                    return request.send(500, "text/plain; charset=utf-8", "Failed to encrypt WireGuard keys.");
                }

                // TODO: maybe base64 encode?
                serializer.addMemberArray("web_private");
                for (size_t a = 0; a < crypto_box_SEALBYTES + wg_key.length(); a++) {
                    serializer.addNumber(output[a]);
                }
                serializer.endArray();

                CoolString psk = key["psk"];
                char encrypted_psk[44];
                ret = crypto_box_seal((unsigned char*)encrypted_psk, (unsigned char*)psk.c_str(), psk.length(), pk);
                if (ret < 0) {
                    https_client = nullptr;
                    encrypted_secret = nullptr;
                    secret_nonce = nullptr;
                    logger.printfln("Failed to encrypt psk: %i", ret);
                    return request.send(500, "text/plain; charset=utf-8", "Failed to encrypt psk");
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
        if (encrypted_name == nullptr) {
            https_client = nullptr;
            encrypted_secret = nullptr;
            secret_nonce = nullptr;
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
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

        char url[128];
        snprintf(url, 128, "https://%s:%u/api/charger/add", new_config.get("relay_host")->asEphemeralCStr(), new_config.get("relay_port")->asUint());

        std::queue<WgKey> key_cache;

        key_cache.push(WgKey {
            doc["mgmt_charger_private"],
            doc["mgmt_psk"],
            "",
        });

        {
            int i = 0;
            for (const auto key : doc["keys"].as<JsonArray>()) {
                WgKey k = {
                    key["charger_private"],
                    key["psk"],
                    key["web_public"]
                };
                key_cache.push(
                    WgKey {
                    key["charger_private"],
                    key["psk"],
                    key["web_public"]
                    }
                );
                ++i;
                // Ignore rest of keys if there were sent more than we support.
                if (i == MAX_KEYS_PER_USER)
                    break;
            }
        }

        auto next_stage = [this, key_cache](ConfigRoot cfg) {
            this->parse_registration(cfg, key_cache);
        };
        this->run_request_with_next_stage(url, HTTP_METHOD_PUT, ptr.get(), size, new_config, next_stage);

        return request.send(200);
    });

    if (!config.get("enable")->asBool()) {
        return;
    }

    task_scheduler.scheduleOnce([this]() {
        this->resolve_management();
        this->connect_management();
        this->connection_state.get(0)->updateUint(1);
    }, 5000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (!this->management_request_done) {
            this->resolve_management();
        }
    }, 1000 * 30, 1000 * 30);

    task_scheduler.scheduleWithFixedDelay([this]() {
        for (int i = 0; i < MAX_KEYS_PER_USER; i++) {
            uint32_t state = 1;
            if (this->remote_connections[i] != nullptr) {
                state = this->remote_connections[i]->is_peer_up(nullptr, nullptr) ? 2 : 1;
            }

            if (this->connection_state.get(i + 1)->updateUint(state)) { // 0 is the management connection
                if (state == 2) {
                    logger.printfln("Connection %i connected", i);
                } else if (state == 1) {
                    logger.printfln("Connection %i disconnected", i);
                }
            }
        }
    }, 1000, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        uint32_t state = 1;
        if (management != nullptr) {
            state = management->is_peer_up(nullptr, nullptr) ? 2 : 1;
        }

        if (this->connection_state.get(0)->updateUint(state)) {
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

void RemoteAccess::run_request_with_next_stage(const char *url, esp_http_client_method_t method, const char *body, int body_size, ConfigRoot config, std::function<void(ConfigRoot config)> next_stage) {
    response_body = String();

    std::function<void(AsyncHTTPSClientEvent *event)> callback = [this, next_stage, config](AsyncHTTPSClientEvent *event) {
            switch (event->type) {
                case AsyncHTTPSClientEventType::Error:
                    switch (event->error) {
                        case AsyncHTTPSClientError::HTTPStatusError: {
                            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
                            {
                                char err_buf[64];
                                snprintf(err_buf, 64, "Received status-code %i", (int)event->error_http_status);
                                registration_state.get("message")->updateString(err_buf);
                            }
                            break;
                        }
                        case AsyncHTTPSClientError::HTTPError:
                            break;

                        default:
                            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
                            {
                                char err_buf[64];
                                snprintf(err_buf, 64, "Error code %i", (int)event->error);
                                registration_state.get("message")->updateString(err_buf);
                            }
                    }
                    response_body = String();
                    break;

                case AsyncHTTPSClientEventType::Aborted:
                    registration_state.get("message")->updateString("Request was aborted");
                    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
                    break;
                case AsyncHTTPSClientEventType::Data:
                    handle_response_chunk(event);
                    break;
                case AsyncHTTPSClientEventType::Finished:
                    task_scheduler.scheduleOnce([this, next_stage, config]() {
                        next_stage(config);
                    }, 0);
                    break;
            }
        };

    // https_client should never be a nullptr in normal operation but in case someone uses the api wrong
    // this ensures that we dont crash
    if (https_client == nullptr) {
        https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
    }
    switch (method) {
        case HTTP_METHOD_GET:
            https_client->download_async(url, config.get("cert_id")->asInt(), callback);
            break;
        case HTTP_METHOD_POST:
            https_client->post_async(url, config.get("cert_id")->asInt(), body, body_size, callback);
            break;
        case HTTP_METHOD_PUT:
            https_client->put_async(url, config.get("cert_id")->asInt(), body, body_size, callback);
            break;
        case HTTP_METHOD_DELETE:
            https_client->delete_async(url, config.get("cert_id")->asInt(), body, body_size, callback);
            break;
        default:
            break;
    }
}

void RemoteAccess::get_login_salt(ConfigRoot config) {
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::InProgress);
    registration_state.get("message")->updateString("");
    char url[196] = {};
    sprintf(url, "https://%s:%u/api/auth/get_login_salt?email=%s", config.get("relay_host")->asEphemeralCStr(), config.get("relay_port")->asUint(), config.get("email")->asEphemeralCStr());
    std::function<void(ConfigRoot)> next_stage = [this] (ConfigRoot cfg) {
        this->parse_login_salt(cfg);
    };
    https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
    run_request_with_next_stage(url, HTTP_METHOD_GET, nullptr, 0, config, next_stage);
}

void RemoteAccess::parse_login_salt(ConfigRoot config) {
    uint8_t login_salt[48];
    {
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, response_body.c_str(), response_body.length());
        if (error) {
            registration_state.get("message")->updateString("Error while deserializing login-salt");
            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
            https_client = nullptr;
            return;
        }
        for (int i = 0; i < 48; i++) {
            if (i == 48) {
                break;
            }
            login_salt[i] = doc[i].as<uint8_t>();
        }
        response_body = "";
    }

    char base64[65] = {};
    size_t bytes_written;
    if (mbedtls_base64_encode((uint8_t*)base64, 65, &bytes_written, login_salt, 48)) {
        registration_state.get("message")->updateString("Error while encoding login-salt");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        https_client = nullptr;
        return;
    }
    registration_state.get("message")->updateString(String(base64));
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
}

void RemoteAccess::login(ConfigRoot config, CoolString &login_key) {
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::InProgress);
    registration_state.get("message")->updateString("");
    char url[128] = {};
    sprintf(url, "https://%s:%u/api/auth/login", config.get("relay_host")->asEphemeralCStr(), config.get("relay_port")->asUint());

    String body;
    DynamicJsonDocument doc(512);

    doc["email"] = config.get("email")->asString();
    uint8_t key[24] = {};
    size_t written;
    if (mbedtls_base64_decode(key, 24, &written, (uint8_t*)login_key.c_str(), login_key.length()) != 0) {
        registration_state.get("message")->updateString("Error while decoding login-salt");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        https_client = nullptr;
        return;
    }

    for (int i = 0; i < 24; i++) {
        doc["login_key"].add(key[i]);
    }
    serializeJson(doc, body);

    run_request_with_next_stage(url, HTTP_METHOD_POST, body.c_str(), body.length(), config, [this](ConfigRoot cfg) {
        registration_state.get("message")->updateString("");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
        response_body = "";
    });
}

void RemoteAccess::get_secret(ConfigRoot config) {
    registration_state.get("message")->updateString("");
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::InProgress);
    char url[128] = {};
    snprintf(url, 128, "https://%s:%u/api/user/get_secret", config.get("relay_host")->asEphemeralCStr(), config.get("relay_port")->asUint());

    run_request_with_next_stage(url, HTTP_METHOD_GET, nullptr, 0, config, [this](ConfigRoot cfg) {
        this->parse_secret(cfg);
    });
}

void RemoteAccess::parse_secret(ConfigRoot config) {
    StaticJsonDocument<2048> doc;

    {
        DeserializationError error = deserializeJson(doc, response_body.c_str());
        if (error) {
            char err_str[64];
            snprintf(err_str, 64, "Error while deserializing Secret: %s", error.c_str());
            registration_state.get("message")->updateString(err_str);
            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
            https_client = nullptr;
            return;
        }
        response_body = "";
    }

    encrypted_secret = heap_alloc_array<uint8_t>(crypto_box_SECRETKEYBYTES + crypto_secretbox_MACBYTES);
    if (encrypted_secret == nullptr) {
        registration_state.get("message")->updateString("Low memory");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        https_client = nullptr;
        return;
    }
    for (int i = 0; i < crypto_box_SECRETKEYBYTES + crypto_secretbox_MACBYTES; i++) {
        encrypted_secret[i] = doc["secret"][i];
    }

    secret_nonce = heap_alloc_array<uint8_t>(crypto_secretbox_NONCEBYTES);
    if (encrypted_secret == nullptr) {
        registration_state.get("message")->updateString("Low memory");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        https_client = nullptr;
        return;
    }
    for (int i = 0; i < crypto_secretbox_NONCEBYTES; i++) {
        secret_nonce[i] = doc["secret_nonce"][i];
    }

    uint8_t secret_salt[48];
    for (int i = 0; i < 48; i++) {
        secret_salt[i] = doc["secret_salt"][i];
    }
    uint8_t encoded_secret_salt[65] = {};
    size_t olen = 0;
    if (mbedtls_base64_encode(encoded_secret_salt, 65, &olen, secret_salt, 48) != 0) {
        registration_state.get("message")->updateString("Error while encoding secret-salt");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        https_client = nullptr;
        encrypted_secret = nullptr;
        secret_nonce = nullptr;
        return;
    }

    registration_state.get("message")->updateString((char *)encoded_secret_salt);
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
}

void RemoteAccess::parse_registration(ConfigRoot new_config, std::queue<WgKey> keys) {
        StaticJsonDocument<32> resp_doc;

        DeserializationError error = deserializeJson(resp_doc, response_body.begin(), response_body.length());

        if (error) {
            char err_str[64];
            snprintf(err_str, 64, "Error while deserializing registration response: %s", error.c_str());
            registration_state.get("message")->updateString(err_str);
            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
            https_client = nullptr;
            encrypted_secret = nullptr;
            secret_nonce = nullptr;
            response_body = "";
            return;
        }

        const char* charger_password = resp_doc["charger_password"];
        const char* management_pub = resp_doc["management_pub"];

        WgKey &mgmt = keys.front();
        store_key(0, 0, mgmt.priv.c_str(), mgmt.psk.c_str(), management_pub);
        keys.pop();

        for (int i = 0; !keys.empty() && i < MAX_KEYS_PER_USER; i++, keys.pop()) {
            WgKey &key = keys.front();
            store_key(1, i,
                key.priv.c_str(),
                key.psk.c_str(),
                key.pub.c_str());
        }

        this->config = new_config;
        this->config.get("password")->updateString(charger_password);
        API::writeConfig("remote_access/config", &this->config);
        registration_state.get("message")->updateString("");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
        https_client = nullptr;
        encrypted_secret = nullptr;
        secret_nonce = nullptr;
        response_body = "";
}

void RemoteAccess::resolve_management() {
    CoolString relay_host = config.get("relay_host")->asString();
    uint32_t relay_port = config.get("relay_port")->asUint();
    CoolString url = "https://";
    url += relay_host;
    url += ":";
    url += relay_port;
    url += "/api/management";

    char json[250] = {};
    TFJsonSerializer serializer = TFJsonSerializer(json, 250);
    serializer.addObject();
    serializer.addMemberNumber("id", local_uid_num);
    serializer.addMemberString("password", config.get("password")->asEphemeralCStr());
        serializer.addMemberObject("data");
            serializer.addMemberObject("V1");
                serializer.addMemberNumber("port", network.config.get("web_server_port")->asUint());
                serializer.addMemberString("firmware_version", BUILD_VERSION_STRING);
                //TODO: Adapt this once we support more than one user.
                serializer.addMemberArray("configured_connections");
                    for (int i = 0; i < MAX_KEYS_PER_USER; i++) {
                        serializer.addNumber(i);
                    }
                serializer.endArray();
            serializer.endObject();
        serializer.endObject();
    serializer.endObject();
    size_t len = serializer.end();

    if (https_client == nullptr) {
        https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
    }
    https_client->set_header("Content-Type", "application/json");
    auto callback = [this](ConfigRoot cfg) {
        management_request_done = true;
        https_client = nullptr;
    };
    run_request_with_next_stage(url.c_str(), HTTP_METHOD_PUT, json, len, config, callback);
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
    if (static_cast<uint32_t>(command->connection_no) >= MAX_KEYS_PER_USER) {
        return;
    }

    switch (command->command_id) {
        case management_command_id::Connect:
            {
                if (remote_connections[command->connection_no] != nullptr && remote_connections[command->connection_no]->is_peer_up(nullptr, nullptr)) {
                    return;
                }
                if (remote_connections[command->connection_no] == nullptr) {
                    logger.printfln("Opening connection %u", command->connection_no);
                }

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
