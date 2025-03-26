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

#include "module_dependencies.h"

#include <LittleFS.h>
#include <TFJson.h>
#include <WiFi.h>
#include <esp_http_client.h>
#include <lwip/err.h>
#include <lwip/netdb.h>
#include <lwip/pbuf.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <mbedtls/aes.h>
#include <mbedtls/base64.h>
#include <sodium.h>

#include "build.h"
#include "esp_tls.h"
#include "esp_tls_errors.h"
#include "event_log_prefix.h"
#include "tools.h"
#include "tools/dns.h"

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);
extern char local_uid_str[32];
extern uint32_t local_uid_num;

#define MAX_KEYS_PER_USER 5
#define MAX_USERS 5
#define WG_KEY_LENGTH 44
#define KEY_SIZE (3 * WG_KEY_LENGTH)
#define KEY_DIRECTORY "/remote-access-keys"

static inline String get_key_path(uint8_t user_id, uint8_t key_id)
{
    return String(KEY_DIRECTORY "/") + user_id + "_" + key_id;
}

void remove_key(uint8_t user_id, uint8_t key_id)
{
    auto path = get_key_path(user_id, key_id);
    if (!LittleFS.exists(path))
        return;

    LittleFS.remove(path);
}

void store_key(uint8_t user_id, uint8_t key_id, const char *pri, const char *psk, const char *pub)
{
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

static int create_sock_and_send_to(const void *payload, size_t payload_len, const char *dest_host, uint16_t port, uint16_t *local_port)
{
    struct sockaddr_in dest_addr;
    bzero(&dest_addr, sizeof(dest_addr));

    ip_addr_t ip;
    int ret = dns_gethostbyname_addrtype_lwip_ctx(dest_host, &ip, nullptr, nullptr, LWIP_DNS_ADDRTYPE_IPV4);
    if (ret == ERR_VAL) {
        logger.printfln("No DNS server is configured!");
        return -1;
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
        return -1;
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
            return -1;
        }
    }

    ret = sendto(sock, payload, payload_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    close(sock);
    return ret;
}

void RemoteAccess::pre_setup()
{
    users_config_prototype =
        Config::Object({{"id",Config::Uint8(0)},
            {"email", Config::Str("", 0, 64)},
            {"uuid", Config::Str("", 0, 36)},
            {"public_key", Config::Str("", 0, 44)}
        });

    // We cant reset this because it is a state since it contains values (uuid, password, users) that should
    // never ever be writable by a user. The only way to fix this is to completely separate the
    // "normal" config from the config that stores the chargers credentials.
    config = ConfigRoot{
        Config::Object({{"uuid", Config::Str("", 0, 36)},
                        {"enable", Config::Bool(false)},
                        {"password", Config::Str("", 0, 32)},
                        {"relay_host", Config::Str(BUILD_REMOTE_ACCESS_HOST, 0, 64)},
                        {"relay_port", Config::Uint16(443)},
                        {"cert_id", Config::Int8(-1)},
                        {"users", Config::Array({}, &users_config_prototype, 0, MAX_USERS, Config::type_id<Config::ConfObject>())}})};

    registration_config = ConfigRoot{Config::Object({
        {"enable", Config::Bool(false)},
        {"relay_host", Config::Str("", 0, 32)},
        {"relay_port", Config::Uint16(443)},
        {"email", Config::Str("", 0, 64)},
        {"cert_id", Config::Int8(-1)},
    })};

    connection_state_prototype = Config::Object({
        {"state", Config::Uint8(1)},
        {"user", Config::Uint8(255)},
        {"connection", Config::Uint8(255)},
        // unix timestamp
        {"last_state_change", Config::Uint53(0)},
    });

    connection_state =
        Config::Array({}, &connection_state_prototype, MAX_KEYS_PER_USER + 1, MAX_KEYS_PER_USER + 1, Config::type_id<Config::ConfUint>());
    for (int i = 0; i < MAX_KEYS_PER_USER + 1; ++i) {
        connection_state.add()->get("state")->updateUint(1); // Set the default here so that the generic prototype can be used.
    }

    registration_state = Config::Object({{"state", Config::Uint8(0)}, {"message", Config::Str("", 0, 64)}});
}

void RemoteAccess::setup()
{
    LittleFS.mkdir(KEY_DIRECTORY);

    api.restorePersistentConfig("remote_access/config", &config);
    initialized = true;

    if (!config.get("enable")->asBool())
        return;

    logger.printfln("Remote Access is enabled trying to connect");
}

static std::unique_ptr<uint8_t[]> decode_base64(const CoolString &input, size_t buffer_size)
{
    std::unique_ptr<uint8_t[]> out = heap_alloc_array<uint8_t>(buffer_size);
    if (out == nullptr) {
        return out;
    }
    size_t _unused = 0;
    int ret = mbedtls_base64_decode(out.get(), buffer_size, &_unused, (unsigned char *)input.c_str(), input.length());
    if (ret != 0) {
        logger.printfln("Error while decoding: %i", ret);
        return nullptr;
    }
    return out;
}

void RemoteAccess::handle_response_chunk(const AsyncHTTPSClientEvent *event)
{
    if (response_body.length() == 0) {
        if (event->data_complete_len < 0) {
            response_body.reserve(1024);
        } else {
            response_body.reserve(event->data_complete_len);
        }
    }
    response_body.concat((const uint8_t *)event->data_chunk, (unsigned int)event->data_chunk_len);
}

void RemoteAccess::register_urls()
{
    api.addState("remote_access/config", &config, {"password"}, {"email"});

    api.addState("remote_access/state", &connection_state);
    api.addState("remote_access/registration_state", &registration_state);

    server.on("/remote_access/reset_registration_state", HTTP_PUT, [this](WebServerRequest request) {
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::None);
        registration_state.get("message")->updateString("");
        return request.send(200);
    });

    api.addCommand(
        "remote_access/config_update",
        &registration_config,
        {"password", "email"},
        [this](String & /*errmsg*/) {
            config.get("enable")->updateBool(registration_config.get("enable")->asBool());
            config.get("relay_host")->updateString(registration_config.get("relay_host")->asEphemeralCStr());
            config.get("relay_port")->updateUint(registration_config.get("relay_port")->asUint());
            config.get("cert_id")->updateInt(registration_config.get("cert_id")->asInt());
            api.writeConfig("remote_access/config", &config);
        },
        false);

    api.addCommand(
        "remote_access/config_reset",
        Config::Null(),
        {},
        [this](String & /*errmsg*/) {
            API::removeConfig("remote_access/config");

            for (uint32_t user = 0; user < MAX_USERS + 1; user++) {
                for (int i = 0; i < MAX_KEYS_PER_USER; i++) {
                    remove_key(user, i);
                }
            }
        },
        true);

    server.on("/remote_access/get_login_salt", HTTP_PUT, [this](WebServerRequest request) {
        this->management_request_allowed = false;
        size_t content_len = request.contentLength();
        std::unique_ptr<char[]> req_body = heap_alloc_array<char>(content_len);
        if (req_body == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        if (request.receive(req_body.get(), content_len) <= 0) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Failed to read request body");
        }

        {
            String error = registration_config.update_from_cstr(req_body.get(), content_len);
            if (error != "") {
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", error.c_str());
            }
        }

        this->get_login_salt(registration_config);
        return request.send(200);
    });

    server.on("/remote_access/get_secret_salt", HTTP_PUT, [this](WebServerRequest request) {
        size_t content_len = request.contentLength();
        std::unique_ptr<char[]> req_body = heap_alloc_array<char>(content_len);
        if (req_body == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        if (request.receive(req_body.get(), content_len) <= 0) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Failed to read request body");
        }

        {
            String error = registration_config.update_from_cstr(req_body.get(), content_len);
            if (error != "") {
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", error.c_str());
            }
        }

        this->get_secret(registration_config);

        return request.send(200);
    });

    server.on("/remote_access/login", HTTP_PUT, [this](WebServerRequest request) {
        auto content_len = request.contentLength();
        std::unique_ptr<char[]> req_body = heap_alloc_array<char>(content_len);
        if (req_body == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        if (request.receive(req_body.get(), content_len) <= 0) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Failed to read request body");
        }

        // TODO: use TFJsonDeserializer?
        StaticJsonDocument<768> doc;

        {
            DeserializationError error = deserializeJson(doc, req_body.get(), content_len);

            if (error) {
                char err_str[64];
                snprintf(err_str, 64, "Failed to deserialize request body: %s", error.c_str());
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", err_str);
            }
        }

        {
            String error = registration_config.update_from_json(doc["config"], true, ConfigSource::API);
            if (error != "") {
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", error.c_str());
            }
        }

        CoolString login_key = doc["login_key"];
        this->login(registration_config, login_key);

        return request.send(200);
    });

    server.on("/remote_access/register", HTTP_PUT, [this](WebServerRequest request) {
        if (config.get("users")->count() != 0) {
            config.get("users")->removeAll();
        }

        // TODO: Maybe don't run the registration in the request handler. Start a task instead?
        auto content_len = request.contentLength();
        std::unique_ptr<char[]> req_body = heap_alloc_array<char>(content_len);
        if (req_body == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        if (request.receive(req_body.get(), content_len) <= 0) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Failed to read request body");
        }

        // TODO: use TFJsonDeserializer?
        StaticJsonDocument<768> doc;

        {
            DeserializationError error = deserializeJson(doc, req_body.get(), content_len);

            if (error) {
                char err_str[64];
                snprintf(err_str, 64, "Failed to deserialize request body: %s", error.c_str());
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", err_str);
            }
        }

        {
            String error = registration_config.update_from_json(doc["config"], true, ConfigSource::API);
            if (error != "") {
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", error.c_str());
            }
        }

        if (!registration_config.get("enable")->asBool()) {
            this->request_cleanup();
            return request.send(400, "text/plain; charset=utf-8", "Calling register without enable beeing true is not supported anymore");
        }

        const CoolString &note = doc["note"];
        size_t encrypted_note_size = crypto_box_SEALBYTES + note.length();
        size_t bs64_note_size = 4 * (encrypted_note_size / 3) + 5;
        const CoolString &name = api.getState("info/display_name")->get("display_name")->asString();
        size_t encrypted_name_size = crypto_box_SEALBYTES + name.length();
        size_t bs64_name_size = 4 * (encrypted_name_size / 3) + 5;
        size_t json_size = 5000 + bs64_name_size + bs64_note_size;
        std::unique_ptr<char[]> ptr = heap_alloc_array<char>(json_size);
        if (ptr == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }

        if (sodium_init() < 0) {
            this->request_cleanup();
            logger.printfln("Failed to initialize libsodium");
            return request.send(500, "text/plain; charset=utf-8", "Failed to initialize crypto");
        }

        unsigned char pk[crypto_box_PUBLICKEYBYTES];
        if (!doc["secret_key"].isNull()) {
            // TODO: Should we validate the secret{,_nonce,_key} lengths before decoding?
            // Also validate the decoded lengths!
            std::unique_ptr<uint8_t[]> secret_key = decode_base64(doc["secret_key"], crypto_secretbox_KEYBYTES);
            if (secret_key == nullptr) {
                this->request_cleanup();
                return request.send(500, "text/plain; charset=utf-8", "Low memory");
            }

            char secret[crypto_box_SECRETKEYBYTES];
            int ret = crypto_secretbox_open_easy((unsigned char *)secret,
                                                (unsigned char *)encrypted_secret.get(),
                                                crypto_box_SECRETKEYBYTES + crypto_secretbox_MACBYTES,
                                                (unsigned char *)secret_nonce.get(),
                                                (unsigned char *)secret_key.get());
            if (ret != 0) {
                this->request_cleanup();
                logger.printfln("Failed to decrypt secret");
                return request.send(500, "text/plain; charset=utf-8", "Failed to decrypt secret");
            }

            ret = crypto_scalarmult_base(pk, (unsigned char *)secret);
            if (ret < 0) {
                this->request_cleanup();
                logger.printfln("Failed to derive public-key");
                return request.send(500, "text/plain; charset=utf-8", "Failed to derive public-key");
            }
        } else if (!doc["public_key"].isNull()) {
            std::unique_ptr<uint8_t[]> public_key = decode_base64(doc["public_key"], crypto_box_PUBLICKEYBYTES);
            if (public_key == nullptr) {
                this->request_cleanup();
                return request.send(500, "text/plain; charset=utf-8", "Low memory");
            }
            memcpy(pk, public_key.get(), crypto_box_PUBLICKEYBYTES);
        } else {
            this->request_cleanup();
            return request.send(400, "text/plain; charset=utf-8", "No key provided");
        }

        TFJsonSerializer serializer{ptr.get(), json_size};
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
                serializer.addMemberNumber("connection_no", (int32_t)i);
                std::snprintf(buf, 50, "10.123.%i.3", i);
                serializer.addMemberString("web_address", buf);
                CoolString wg_key = CoolString{key["web_private"].as<String>()};

                // TODO optimize: this is always 44 bytes + crypto_box_SEALBYTES -> stack alloc
                unsigned char output[44 + crypto_box_SEALBYTES];

                int ret = crypto_box_seal(output, (unsigned char *)wg_key.c_str(), wg_key.length(), pk);
                if (ret < 0) {
                    this->request_cleanup();
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
                unsigned char encrypted_psk[44 + crypto_box_SEALBYTES];
                ret = crypto_box_seal(encrypted_psk, (unsigned char *)psk.c_str(), psk.length(), pk);
                if (ret < 0) {
                    this->request_cleanup();
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
        serializer.addMemberString("uid", local_uid_str);
        serializer.addMemberString("wg_charger_ip", "10.123.123.2");
        serializer.addMemberString("wg_server_ip", "10.123.123.3");
        serializer.addMemberString("psk", doc["mgmt_psk"]);

        serializer.endObject();

        std::unique_ptr<uint8_t[]> encrypted_name = heap_alloc_array<uint8_t>(encrypted_name_size);
        if (encrypted_name == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        crypto_box_seal(encrypted_name.get(), (unsigned char *)name.c_str(), name.length(), (unsigned char *)pk);
        auto bs64_name = heap_alloc_array<char>(bs64_name_size);
        if (bs64_name == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        size_t olen;
        mbedtls_base64_encode((uint8_t *)bs64_name.get(), bs64_name_size, &olen, encrypted_name.get(), encrypted_name_size);

        serializer.addMemberString("name", bs64_name.get());

        auto encrypted_note = heap_alloc_array<uint8_t>(encrypted_note_size);
        if (encrypted_note == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        crypto_box_seal(encrypted_note.get(), (uint8_t *)note.c_str(), note.length(), pk);
        auto bs64_note = heap_alloc_array<char>(bs64_note_size);
        if (bs64_note == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        mbedtls_base64_encode((uint8_t *)bs64_note.get(), bs64_note_size, &olen, encrypted_note.get(), encrypted_note_size);

        serializer.addMemberString("note", bs64_note.get());

        char url[128];
        const CoolString &uuid = doc["user_uuid"];
        const CoolString &token = doc["auth_token"];
        if (uuid != "null" && token != "null") {
            serializer.addMemberString("user_id", uuid.c_str());
            serializer.addMemberString("token", token.c_str());
            snprintf(url,
                    128,
                    "https://%s:%lu/api/add_with_token",
                    registration_config.get("relay_host")->asEphemeralCStr(),
                    registration_config.get("relay_port")->asUint());
        } else {
            snprintf(url,
                    128,
                    "https://%s:%lu/api/charger/add",
                    registration_config.get("relay_host")->asEphemeralCStr(),
                    registration_config.get("relay_port")->asUint());
        }

        serializer.endObject();
        size_t size = serializer.end();

        std::queue<WgKey> key_cache;

        key_cache.push(WgKey{
            doc["mgmt_charger_private"],
            doc["mgmt_psk"],
            "",
        });

        {
            int i = 0;
            for (const auto key : doc["keys"].as<JsonArray>()) {
                WgKey k = {key["charger_private"], key["psk"], key["web_public"]};
                key_cache.push(WgKey{key["charger_private"], key["psk"], key["web_public"]});
                ++i;
                // Ignore rest of keys if there were sent more than we support.
                if (i == MAX_KEYS_PER_USER)
                    break;
            }
        }

        uint8_t public_key[50];
        mbedtls_base64_encode(public_key, 50, &olen, (uint8_t *)pk, crypto_box_PUBLICKEYBYTES);
        auto next_stage = [this, key_cache, public_key, olen](ConfigRoot cfg) {
            CoolString pub_key((char *)public_key, olen);
            this->parse_registration(cfg, key_cache, pub_key);
        };
        this->run_request_with_next_stage(url, HTTP_METHOD_PUT, ptr.get(), size, registration_config, next_stage);

        return request.send(200);
    });

    server.on("/remote_access/add_user", HTTP_PUT, [this](WebServerRequest request) {
        size_t content_len = request.contentLength();
        std::unique_ptr<char[]> req_body = heap_alloc_array<char>(content_len);
        if (req_body == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }
        if (request.receive(req_body.get(), content_len) <= 0) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Failed to read request body");
        }

        uint32_t next_user_id;
        for (uint32_t i = 1; i < MAX_USERS + 1; i++) {
            next_user_id = i;
            bool found = false;
            for (auto &user : config.get("users")) {
                if (user.get("id")->asUint() == i) {
                    found = true;
                }
            }
            if (!found) {
                break;
            }
        }

        // TODO: use TFJsonDeserializer?
        StaticJsonDocument<768> doc;
        {
            DeserializationError error = deserializeJson(doc, req_body.get(), content_len);

            if (error) {
                char err_str[64];
                snprintf(err_str, 64, "Failed to deserialize request body: %s", error.c_str());
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", err_str);
            }
        }

        const CoolString &email = doc["email"];
        const CoolString &uuid = doc["user_uuid"];
        if ((email == "null" && uuid == "null") || this->user_already_registered(email)) {
            this->request_cleanup();
            return request.send(400, "text/plain; charset=utf-8", "User already exists or is empty");
        }

        std::unique_ptr<unsigned char[]> pk = nullptr;
        if (!doc["secret_key"].isNull()) {

            // TODO: Should we validate the secret{,_nonce,_key} lengths before decoding?
            // Also validate the decoded lengths!
            std::unique_ptr<uint8_t[]> secret_key = decode_base64(doc["secret_key"], crypto_secretbox_KEYBYTES);
            if (secret_key == nullptr) {
                this->request_cleanup();
                return request.send(500, "text/plain; charset=utf-8", "Low memory");
            }

            if (sodium_init() < 0) {
                this->request_cleanup();
                logger.printfln("Failed to initialize libsodium");
                return request.send(500, "text/plain; charset=utf-8", "Failed to initialize crypto");
            }

            char secret[crypto_box_SECRETKEYBYTES];
            int ret = crypto_secretbox_open_easy((unsigned char *)secret,
                                                (unsigned char *)encrypted_secret.get(),
                                                crypto_box_SECRETKEYBYTES + crypto_secretbox_MACBYTES,
                                                (unsigned char *)secret_nonce.get(),
                                                (unsigned char *)secret_key.get());
            if (ret != 0) {
                this->request_cleanup();
                logger.printfln("Failed to decrypt secret");
                return request.send(500, "text/plain; charset=utf-8", "Failed to decrypt secret");
            }
            pk = heap_alloc_array<unsigned char>(crypto_box_PUBLICKEYBYTES);
            ret = crypto_scalarmult_base(pk.get(), (unsigned char *)secret);
            if (ret < 0) {
                this->request_cleanup();
                logger.printfln("Failed to derive public-key");
                return request.send(500, "text/plain; charset=utf-8", "Failed to derive public-key");
            }
        } else if (!doc["public_key"].isNull()) {
            pk = decode_base64(doc["public_key"], crypto_box_PUBLICKEYBYTES);
            if (pk == nullptr) {
                this->request_cleanup();
                return request.send(500, "text/plain; charset=utf-8", "Low memory");
            }
        } else {
            this->request_cleanup();
            return request.send(400, "text/plain; charset=utf-8", "No public key provided");
        }

        const String &note = doc["note"];
        size_t encrypted_note_size = note.length() + crypto_box_SEALBYTES;
        size_t bs64_note_size = 4 * ((note.length() + crypto_box_SEALBYTES) / 3) + 5;
        const CoolString &name = api.getState("info/display_name")->get("display_name")->asString();
        size_t encrypted_name_size = name.length() + crypto_box_SEALBYTES;
        size_t bs64_name_size = 4 * (encrypted_name_size / 3) + 5;
        size_t json_size = 4500 + bs64_note_size + bs64_name_size;
        auto json = heap_alloc_array<char>(json_size);
        if (json == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }

        TFJsonSerializer serializer{json.get(), json_size};
        serializer.addObject();

        std::unique_ptr<uint8_t[]> encrypted_note = heap_alloc_array<uint8_t>(encrypted_note_size);
        if (encrypted_note == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }

        if (crypto_box_seal(encrypted_note.get(), (uint8_t *)note.c_str(), note.length(), pk.get())) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Failed to encrypt note");
        }

        auto bs64_note = heap_alloc_array<char>(bs64_note_size);
        size_t olen;
        mbedtls_base64_encode((uint8_t *)bs64_note.get(),
                              bs64_note_size,
                              &olen,
                              encrypted_note.get(),
                              note.length() + crypto_box_SEALBYTES);

        serializer.addMemberString("note", bs64_note.get());

        std::unique_ptr<uint8_t[]> encrypted_name = heap_alloc_array<uint8_t>(encrypted_name_size);
        if (encrypted_name == nullptr) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Low memory");
        }

        if (crypto_box_seal(encrypted_name.get(), (uint8_t *)name.c_str(), name.length(), pk.get())) {
            this->request_cleanup();
            return request.send(500, "text/plain; charset=utf-8", "Failed to encrypt name");
        }

        auto bs64_name = heap_alloc_array<char>(bs64_name_size);
        mbedtls_base64_encode((uint8_t *)bs64_name.get(), bs64_name_size, &olen, encrypted_name.get(), encrypted_name_size);

        serializer.addMemberString("charger_name", bs64_name.get());

        serializer.addMemberString("charger_id", config.get("uuid")->asEphemeralCStr());
        serializer.addMemberString("charger_password", config.get("password")->asEphemeralCStr());
        if (email != "null") {
            serializer.addMemberString("email", email.c_str());
        }
        if (uuid != "null") {
            serializer.addMemberString("user_uuid", uuid.c_str());
        }

        serializer.addMemberObject("user_auth");

        if (!doc["login_key"].isNull()) {
            serializer.addMemberString("LoginKey", doc["login_key"]);
        } else if (!doc["auth_token"].isNull()) {
            serializer.addMemberString("AuthToken", doc["auth_token"]);
        } else {
            this->request_cleanup();
            return request.send(400, "text/plain; charset=utf-8", "No login key or auth token provided");
        }
        serializer.endObject();

        serializer.addMemberArray("wg_keys");
        char buf[50];
        int i = 0;
        for (auto key : doc["wg_keys"].as<JsonArray>()) {
            serializer.addObject();
            int connection_number = (next_user_id - 1) * MAX_KEYS_PER_USER + i;
            std::snprintf(buf, 50, "10.123.%i.2", connection_number);
            serializer.addMemberString("charger_address", buf);
            std::snprintf(buf, 50, "10.123.%i.3", connection_number);
            serializer.addMemberString("web_address", buf);
            serializer.addMemberString("charger_public", key["charger_public"]);

            CoolString psk = key["psk"];
            uint8_t encrypted_psk[44 + crypto_box_SEALBYTES];
            if (crypto_box_seal(encrypted_psk, (uint8_t *)psk.c_str(), 44, pk.get())) {
                this->request_cleanup();
                return request.send(500, "text/plain; charset=utf-8", "Failed to encrypt psk");
            }
            serializer.addMemberArray("psk");
            for (int a = 0; a < 44 + crypto_box_SEALBYTES; a++) {
                serializer.addNumber(encrypted_psk[a]);
            }
            serializer.endArray();

            CoolString web_private = key["web_private"];
            uint8_t encrypted_web_private[44 + crypto_box_SEALBYTES];
            if (crypto_box_seal(encrypted_web_private, (uint8_t *)web_private.c_str(), 44, pk.get())) {
                this->request_cleanup();
                return request.send(500, "text/plain; charset=utf-8", "Failed to encrypt web_private");
            }
            serializer.addMemberArray("web_private");
            for (int a = 0; a < 44 + crypto_box_SEALBYTES; a++) {
                serializer.addNumber(encrypted_web_private[a]);
            }
            serializer.endArray();

            serializer.addMemberNumber("connection_no", (int32_t)connection_number);
            i++;
            serializer.endObject();
        }
        serializer.endArray();
        serializer.endObject();
        uint32_t size = serializer.end();

        char url[128];
        snprintf(url,
                 128,
                 "https://%s:%lu/api/allow_user",
                 config.get("relay_host")->asEphemeralCStr(),
                 config.get("relay_port")->asUint());

        std::queue<WgKey> key_cache;
        int a = 0;
        for (const auto key : doc["wg_keys"].as<JsonArray>()) {
            WgKey k = {key["charger_private"], key["psk"], key["web_public"]};
            key_cache.push(WgKey{key["charger_private"], key["psk"], key["web_public"]});
            ++a;
            // Ignore rest of keys if there were sent more than we support.
            if (a == MAX_KEYS_PER_USER)
                break;
        }

        uint8_t public_key[50];
        mbedtls_base64_encode(public_key, 50, &olen, (uint8_t *)pk.get(), crypto_box_PUBLICKEYBYTES);
        CoolString pub_key(public_key, olen);

        ConfigRoot user = config;

        if (https_client == nullptr) {
            https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
        }

        https_client->set_header("Content-Type", "application/json");
        auto next_stage = [this, key_cache, pub_key, next_user_id, email](ConfigRoot cfg) {
            this->parse_add_user(cfg, key_cache, pub_key, email, next_user_id);
        };

        this->run_request_with_next_stage(url, HTTP_METHOD_PUT, json.get(), size, user, next_stage);

        return request.send(200);
    });

    server.on("/remote_access/remove_user", HTTP_PUT, [this](WebServerRequest request) {
        size_t content_len = request.contentLength();
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

        uint32_t req_id = doc["id"];
        int idx = 0;
        for (const auto &user : config.get("users")) {
            if (user.get("id")->asUint() == req_id) {
                break;
            }
            idx++;
        }
        if (idx >= config.get("users")->count()) {
            return request.send(400, "text/plain; charset=utf-8", "User does not exist");
        }

        config.get("users")->remove(idx);
        for (int i = 0; i < MAX_KEYS_PER_USER; i++) {
            remove_key(req_id, i);
        }

        bool one_left = false;
        for (const auto &user : config.get("users")) {
            if (user.get("id")->asUint() != 255) {
                one_left = true;
                break;
            }
        }

        if (!one_left) {
            char json[256];
            TFJsonSerializer serializer{json, 256};
            serializer.addObject();
            serializer.addMemberString("uuid", config.get("uuid")->asEphemeralCStr());
            serializer.addMemberString("password", config.get("password")->asEphemeralCStr());
            serializer.endObject();
            size_t json_size = serializer.end();

            if (https_client == nullptr) {
                https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
            }
            https_client->set_header("Content-Type", "application/json");

            char url[256];
            snprintf(url,
                     256,
                     "https://%s:%lu/api/selfdestruct",
                     config.get("relay_host")->asEphemeralCStr(),
                     config.get("relay_port")->asUint());
            run_request_with_next_stage(url, HTTP_METHOD_DELETE, json, json_size, config, [this](ConfigRoot cfg) {
                this->request_cleanup();
            });

            remove_key(0, 0);

            config.get("enable")->updateBool(false);
        }

        API::writeConfig("remote_access/config", &config);

        return request.send(200);
    });

    if (!config.get("enable")->asBool()) {
        return;
    }

    task_scheduler.scheduleWithFixedDelay(
        [this]() {
            timeval now;
            if (!rtc.clock_synced(&now)) {
                now.tv_sec = 0;
            }
            for (size_t i = 0; i < MAX_KEYS_PER_USER; i++) {
                uint32_t state = 1;
                if (this->remote_connections[i].conn != nullptr) {
                    state = this->remote_connections[i].conn->is_peer_up(nullptr, nullptr) ? 2 : 1;
                }

                if (this->connection_state.get(i + 1)->get("state")->updateUint(state)) { // 0 is the management connection
                    uint32_t conn = this->connection_state.get(i + 1)->get("connection")->asUint();
                    uint32_t user = this->connection_state.get(i + 1)->get("user")->asUint();
                    this->connection_state.get(i + 1)->get("last_state_change")->updateUint53((uint64_t)now.tv_sec);
                    if (state == 2) {
                        logger.printfln("Connection %lu for user %lu connected", conn, user);
                    } else if (state == 1 && conn != 255 && user != 255) {
                        logger.printfln("Connection %lu for user %lu disconnected", conn, user);
                    }
                }
            }
        },
        1_s,
        1_s);

    task_scheduler.scheduleWithFixedDelay(
        [this]() {
            timeval now;
            if (!rtc.clock_synced(&now)) {
                return;
            }

            uint32_t state = 1;
            if (management != nullptr) {
                state = management->is_peer_up(nullptr, nullptr) ? 2 : 1;
            }
            if (state == 2) {
                this->last_mgmt_alive = now_us();
            }

            // Check if we got unlucky timing and management request ran
            // without the management connection getting connected afterwards
            if (deadline_elapsed(this->last_mgmt_alive + 60_s) && this->management_request_done) {
                logger.printfln("Management connection timed out");

                // Reset the timeout to prevent log and reconnect spamming
                this->last_mgmt_alive = now_us();
                this->management_request_done = false;
            }

            if (this->connection_state.get(0)->get("state")->updateUint(state)) {
                this->connection_state.get(0)->get("last_state_change")->updateUint53((uint64_t)now.tv_sec);
                if (state == 2) {
                    logger.printfln("Management connection connected");
                } else {
                    in_seq_number = 0;
                    this->management_request_done = false;
                    logger.printfln("Management connection disconnected");
                    // Close all remote access connections when management connection is lost
                    this->close_all_remote_connections();
                }
            }
        },
        1_s,
        1_s);
}

void RemoteAccess::register_events()
{
    if (!config.get("enable")->asBool())
        return;

    event.registerEvent("network/state", {"connected"}, [this](const Config *connected) {
        task_scheduler.cancel(this->task_id);

        if (connected->asBool()) {
            this->task_id = task_scheduler.scheduleWithFixedDelay(
                [this]() {
                    if (!this->management_request_done) {
                        this->resolve_management();
                    }
                },
                0_s,
                30_s);
        }
        return EventResult::OK;
    });
}

bool RemoteAccess::user_already_registered(const CoolString &email)
{
    for (const auto &user : config.get("users")) {
        if (email == user.get("email")->asEphemeralCStr()) {
            return true;
        }
    }

    return false;
}

void RemoteAccess::run_request_with_next_stage(const char *url,
                                               esp_http_client_method_t method,
                                               const char *body,
                                               int body_size,
                                               ConfigRoot config,
                                               std::function<void(ConfigRoot config)> &&next_stage)
{
    response_body = String();

    const String url_capture = String(url);
    std::function<void(AsyncHTTPSClientEvent * event)> callback = [this, next_stage, config, url_capture](AsyncHTTPSClientEvent *event) {
        switch (event->type) {
            case AsyncHTTPSClientEventType::Error:
                switch (event->error) {
                    case AsyncHTTPSClientError::HTTPStatusError: {
                        if (strstr(url_capture.c_str(), "/management") != nullptr) {
                            if (!this->management_request_failed) {
                                logger.printfln("Management request failed with HTTP-Error-Code %i", (int)event->error_http_status);
                                this->management_request_failed = true;
                            }
                        } else {
                            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
                            {
                                char err_buf[64];
                                snprintf(err_buf, 64, "Received status-code %i", (int)event->error_http_status);
                                registration_state.get("message")->updateString(err_buf);
                            }
                        }
                        this->cleanup_after();
                        break;
                    }
                    case AsyncHTTPSClientError::HTTPError:
                        break;

                    default:
                        if (strstr(url_capture.c_str(), "/management") != nullptr) {
                            if (!this->management_request_failed) {
                                logger.printfln("Management request failed with internal error: %s (%u)", translate_error(event), (uint8_t)event->error);
                                this->management_request_failed = true;
                            }
                        } else {
                            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
                            {
                                char err_buf[64];
                                snprintf(err_buf, 64, "Error code %i", (int)event->error);
                                registration_state.get("message")->updateString(err_buf);
                            }
                        }
                        this->cleanup_after();
                        break;
                }
                response_body = String();
                break;

            case AsyncHTTPSClientEventType::Aborted:
                registration_state.get("message")->updateString("Request was aborted");
                registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
                this->cleanup_after();
                break;
            case AsyncHTTPSClientEventType::Data:
                handle_response_chunk(event);
                break;
            case AsyncHTTPSClientEventType::Finished:
                task_scheduler.scheduleOnce([this, next_stage, config]() {
                    next_stage(config);
                });
                break;
        }
    };

    // https_client should never be a nullptr in normal operation but in case someone uses the api wrong
    // this ensures that we dont crash
    if (https_client == nullptr) {
        https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
    }
    https_client->fetch(url, config.get("cert_id")->asInt(), method, body, body_size, std::move(callback));
}

void RemoteAccess::get_login_salt(ConfigRoot config)
{
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::InProgress);
    registration_state.get("message")->updateString("");
    char url[196] = {};
    sprintf(url,
            "https://%s:%lu/api/auth/get_login_salt?email=%s",
            config.get("relay_host")->asEphemeralCStr(),
            config.get("relay_port")->asUint(),
            config.get("email")->asEphemeralCStr());

    https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
    run_request_with_next_stage(url, HTTP_METHOD_GET, nullptr, 0, config, [this](ConfigRoot cfg) {
        this->parse_login_salt(cfg);
    });
}

void RemoteAccess::parse_login_salt(ConfigRoot config)
{
    uint8_t login_salt[48];
    {
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, response_body.c_str(), response_body.length());
        if (error) {
            registration_state.get("message")->updateString("Error while deserializing login-salt");
            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
            this->request_cleanup();
            return;
        }
        for (int i = 0; i < 48; i++) {
            login_salt[i] = doc[i].as<uint8_t>();
        }
        response_body = "";
    }

    char base64[65] = {};
    size_t bytes_written;
    if (mbedtls_base64_encode((uint8_t *)base64, 65, &bytes_written, login_salt, 48)) {
        registration_state.get("message")->updateString("Error while encoding login-salt");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        this->request_cleanup();
        return;
    }
    registration_state.get("message")->updateString(String(base64));
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
}

void RemoteAccess::login(ConfigRoot config, CoolString &login_key)
{
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::InProgress);
    registration_state.get("message")->updateString("");
    char url[128] = {};
    sprintf(url, "https://%s:%lu/api/auth/login", config.get("relay_host")->asEphemeralCStr(), config.get("relay_port")->asUint());

    String body;
    DynamicJsonDocument doc(512);

    doc["email"] = config.get("email")->asString();
    uint8_t key[24] = {};
    size_t written;
    if (mbedtls_base64_decode(key, 24, &written, (uint8_t *)login_key.c_str(), login_key.length()) != 0) {
        registration_state.get("message")->updateString("Error while decoding login-salt");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        this->request_cleanup();
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

void RemoteAccess::get_secret(ConfigRoot config)
{
    registration_state.get("message")->updateString("");
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::InProgress);
    char url[128] = {};
    snprintf(url,
             128,
             "https://%s:%lu/api/user/get_secret",
             config.get("relay_host")->asEphemeralCStr(),
             config.get("relay_port")->asUint());

    run_request_with_next_stage(url, HTTP_METHOD_GET, nullptr, 0, config, [this](ConfigRoot cfg) {
        this->parse_secret(cfg);
    });
}

void RemoteAccess::parse_secret(ConfigRoot config)
{
    StaticJsonDocument<2048> doc;

    {
        DeserializationError error = deserializeJson(doc, response_body.c_str());
        if (error) {
            char err_str[64];
            snprintf(err_str, 64, "Error while deserializing Secret: %s", error.c_str());
            registration_state.get("message")->updateString(err_str);
            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
            this->request_cleanup();
            return;
        }
        response_body = "";
    }

    encrypted_secret = heap_alloc_array<uint8_t>(crypto_box_SECRETKEYBYTES + crypto_secretbox_MACBYTES);
    if (encrypted_secret == nullptr) {
        registration_state.get("message")->updateString("Low memory");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        this->request_cleanup();
        return;
    }
    for (int i = 0; i < crypto_box_SECRETKEYBYTES + crypto_secretbox_MACBYTES; i++) {
        encrypted_secret[i] = doc["secret"][i];
    }

    secret_nonce = heap_alloc_array<uint8_t>(crypto_secretbox_NONCEBYTES);
    if (encrypted_secret == nullptr) {
        registration_state.get("message")->updateString("Low memory");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        this->request_cleanup();
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
        this->request_cleanup();
        return;
    }

    registration_state.get("message")->updateString((char *)encoded_secret_salt);
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
}

void RemoteAccess::parse_registration(ConfigRoot new_config, std::queue<WgKey> keys, CoolString public_key)
{
    StaticJsonDocument<256> resp_doc;
    DeserializationError error = deserializeJson(resp_doc, response_body.begin(), response_body.length());

    if (error) {
        char err_str[64];
        snprintf(err_str, 64, "Error while deserializing registration response: %s", error.c_str());
        registration_state.get("message")->updateString(err_str);
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        this->request_cleanup();
        return;
    }

    const char *charger_password = resp_doc["charger_password"];
    const char *management_pub = resp_doc["management_pub"];

    WgKey &mgmt = keys.front();
    store_key(0, 0, mgmt.priv.c_str(), mgmt.psk.c_str(), management_pub);
    keys.pop();

    for (int i = 0; !keys.empty() && i < MAX_KEYS_PER_USER; i++, keys.pop()) {
        WgKey &key = keys.front();
        store_key(1, i, key.priv.c_str(), key.psk.c_str(), key.pub.c_str());
    }

    this->config.get("relay_host")->updateString(new_config.get("relay_host")->asString());
    this->config.get("relay_port")->updateUint(new_config.get("relay_port")->asUint());
    this->config.get("enable")->updateBool(new_config.get("enable")->asBool());
    this->config.get("cert_id")->updateInt(new_config.get("cert_id")->asInt());
    this->config.get("password")->updateString(charger_password);
    this->config.get("uuid")->updateString(resp_doc["charger_uuid"]);

    this->config.get("users")->add();
    this->config.get("users")->get(0)->get("email")->updateString(new_config.get("email")->asString());
    this->config.get("users")->get(0)->get("id")->updateUint(1);
    this->config.get("users")->get(0)->get("public_key")->updateString(public_key);

    API::writeConfig("remote_access/config", &this->config);
    registration_state.get("message")->updateString("");
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
    this->request_cleanup();
}

void RemoteAccess::parse_add_user(ConfigRoot cfg, std::queue<WgKey> key_cache, CoolString pub_key, CoolString email, uint32_t next_user_id)
{
    for (int i = 0; !key_cache.empty() && i < MAX_KEYS_PER_USER; i++, key_cache.pop()) {
        const WgKey &key = key_cache.front();
        store_key(next_user_id, i, key.priv.c_str(), key.psk.c_str(), key.pub.c_str());
    }

    this->config.get("users")->add();
    uint32_t last_idx = this->config.get("users")->count() - 1;
    this->config.get("users")->get(last_idx)->get("email")->updateString(email);
    this->config.get("users")->get(last_idx)->get("id")->updateUint(next_user_id);
    this->config.get("users")->get(last_idx)->get("public_key")->updateString(pub_key);
    api.writeConfig("remote_access/config", &this->config);
    registration_state.get("message")->updateString("");
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
}

void RemoteAccess::resolve_management()
{
    if (!this->management_request_allowed) {
        return;
    }

    CoolString relay_host = config.get("relay_host")->asString();
    uint32_t relay_port = config.get("relay_port")->asUint();
    CoolString url = "https://";
    url += relay_host;
    url += ":";
    url += relay_port;
    url += "/api/management";

    const CoolString &uuid = config.get("uuid")->asString();
    const CoolString &name = api.getState("info/display_name")->get("display_name")->asString();
    size_t encrypted_name_size = name.length() + crypto_box_SEALBYTES;
    size_t encoded_name_size = 4 * (encrypted_name_size / 3) + 5;

    char json[1000] = {};
    TFJsonSerializer serializer{json, 1000};
    serializer.addObject();

    bool old_api;
    if (uuid.length() == 0) {
        old_api = true;
        serializer.addMemberNumber("id", local_uid_num);
        serializer.addMemberString("password", config.get("password")->asEphemeralCStr());
        serializer.addMemberObject("data");
        serializer.addMemberObject("V1");
        serializer.addMemberNumber("port", network.get_web_server_port());
        serializer.addMemberString("firmware_version", build_version_full_str());
        serializer.addMemberArray("configured_connections");
        for (auto &user : config.get("users")) {
            uint32_t user_id = user.get("id")->asUint() - 1;
            for (int i = 0; i < MAX_KEYS_PER_USER; i++) {
                serializer.addNumber((user_id * MAX_KEYS_PER_USER) + i);
            }
        }
        serializer.endArray();
        serializer.endObject();
        serializer.endObject();
    } else {
        old_api = false;
        serializer.addMemberObject("data");
        serializer.addMemberObject("V2");
        serializer.addMemberString("id", config.get("uuid")->asEphemeralCStr());
        serializer.addMemberString("password", config.get("password")->asEphemeralCStr());
        serializer.addMemberNumber("port", network.get_web_server_port());
        serializer.addMemberString("firmware_version", build_version_full_str());
        serializer.addMemberArray("configured_users");
        for (auto &user : config.get("users")) {
            serializer.addObject();

            // Check if we already have the user_id and use it
            const CoolString &user_id = user.get("uuid")->asString();
            if (user_id.length() != 0) {
                serializer.addMemberString("user_id", user_id.c_str());
            } else {
            serializer.addMemberString("email", user.get("email")->asEphemeralCStr());
            }

            if (user.get("public_key")->asString().length() != 0) {
                auto key = decode_base64(user.get("public_key")->asString(), 32);

                auto encrypted_name = heap_alloc_array<uint8_t>(encrypted_name_size);
                crypto_box_seal(encrypted_name.get(), (uint8_t *)name.c_str(), name.length(), key.get());

                auto encoded_name = heap_alloc_array<uint8_t>(encoded_name_size);
                size_t olen;
                mbedtls_base64_encode(encoded_name.get(), encoded_name_size, &olen, encrypted_name.get(), encrypted_name_size);
                serializer.addMemberString("name", (char *)encoded_name.get());
            }
            serializer.endObject();
        }
        serializer.endArray();
        serializer.endObject();
        serializer.endObject();
    }

    serializer.endObject();
    size_t len = serializer.end();

    if (https_client == nullptr) {
        https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
    }
    https_client->set_header("Content-Type", "application/json");
    auto callback = [this, old_api](ConfigRoot cfg) {
        this->management_request_failed = false;
        this->management_request_allowed = true;

        if (old_api) {
            StaticJsonDocument<250> resp;
            {
                DeserializationError error = deserializeJson(resp, response_body.c_str());
                if (error) {
                    char err_str[64];
                    snprintf(err_str, 64, "Error while deserializing management response: %s", error.c_str());
                    this->request_cleanup();
                    return;
                }
                response_body = "";
            }
            config.get("uuid")->updateString(resp["uuid"]);
            api.writeConfig("remote_access/config", &config);

            this->request_cleanup();

            return;
        }

        StaticJsonDocument<1024> doc;
        {
            DeserializationError error = deserializeJson(doc, response_body.c_str());
            if (error) {
                char err_str[64];
                snprintf(err_str, 64, "Error while deserializing management response: %s", error.c_str());
                this->request_cleanup();
                return;
            }
            response_body = "";
        }

            bool changed = false;
        for (int idx = config.get("users")->count() - 1; idx >= 0; idx--) {
            const CoolString &user_email = doc["configured_users_emails"][idx];
            const CoolString &user_uuid = doc["configured_users_uuids"][idx];

            if (doc["configured_users"][idx] == 0) {
                uint32_t user_id = config.get("users")->get(idx)->get("id")->asUint();
                for (int i = 0; i < MAX_KEYS_PER_USER; i++) {
                    remove_key(user_id, i);
                }
                config.get("users")->remove(idx);
                changed = true;
            } else if (user_email != "null") {
                changed |= config.get("users")->get(idx)->get("email")->updateString(user_email);
                changed |= config.get("users")->get(idx)->get("uuid")->updateString(user_uuid);
            }

            }
            if (changed) {
                api.writeConfig("remote_access/config", &config);
        }

        this->management_request_done = true;
        this->last_mgmt_alive = now_us();
        this->request_cleanup();
        this->connect_management();
        this->connection_state.get(0)->get("state")->updateUint(1);
    };
    this->management_request_allowed = false;
    run_request_with_next_stage(url.c_str(), HTTP_METHOD_PUT, json, len, config, callback);
}

void RemoteAccess::cleanup_after()
{
    task_scheduler.scheduleOnce(
        [this] {
            this->request_cleanup();
        },
        0_s);
}

static int management_filter_in(struct pbuf *packet)
{
    // When this function is called it is already ensured that the payload contains a valid ip packet.
    uint8_t *payload = (uint8_t *)packet->payload;

    if (payload[9] != 0x11) {
        logger.printfln("Management blocked invalid incoming packet with protocol: 0x%X", payload[9]);
        return ERR_VAL;
    }

    int header_len = (payload[0] & 0xF) * 4;
    if (packet->len - (header_len + 8) != sizeof(management_command_packet)) {
        logger.printfln("Management blocked invalid incoming packet of size: %i", (packet->len - (header_len + 8)));
        return ERR_VAL;
    }

    int dest_port = payload[header_len] << 8;
    dest_port |= payload[header_len + 1];
    if (dest_port != 12345) {
        logger.printfln("Management blocked invalid incoming packet with destination port: %i.", dest_port);
        return ERR_VAL;
    }
    return ERR_OK;
}

static int management_filter_out(struct pbuf *packet)
{
    uint8_t *payload = (uint8_t *)packet->payload;

    if (payload[9] == 0x1) {
        return ERR_OK;
    } else {
        logger.printfln("Management blocked outgoing packet");
    }
    return ERR_VAL;
}

bool port_valid(uint16_t port)
{
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

uint16_t find_next_free_port(uint16_t port)
{
    while (!port_valid(port)) {
        port++;
    }
    return port;
}

void RemoteAccess::request_cleanup()
{
    https_client = nullptr;
    encrypted_secret = nullptr;
    secret_nonce = nullptr;
    response_body = "";
    management_request_allowed = true;
}

void RemoteAccess::connect_management()
{
    static bool done = false;
    if (done)
        return;

    struct timeval tv;
    if (!rtc.clock_synced(&tv)) {
        task_scheduler.scheduleOnce(
            [this]() {
                this->connect_management();
            },
            5_s);
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

    task_scheduler.scheduleWithFixedDelay(
        [this]() {
            this->run_management();
        },
        250_ms);
}

void RemoteAccess::connect_remote_access(uint8_t i, uint16_t local_port)
{
    struct timeval tv;
    if (!rtc.clock_synced(&tv)) {
        task_scheduler.scheduleOnce(
            [this, i, local_port]() {
                this->connect_remote_access(i, local_port);
            },
            5_s);
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

    uint8_t conn_id = i % MAX_KEYS_PER_USER;
    uint8_t user_id = i / MAX_KEYS_PER_USER;
    if (!get_key(user_id + 1, conn_id, private_key, psk, remote_public_key)) {
        logger.printfln("Can't connect to web interface: no WireGuard key installed!");
        return;
    }

    // Only used for BLOCKING! DNS resolve. TODO Make this non-blocking in Wireguard-ESP32-Arduino/src/WireGuard.cpp!
    auto remote_host = config.get("relay_host")->asEphemeralCStr();

    uint8_t conn_idx = get_connection(i);
    if (conn_idx == 255) {
        logger.printfln("No free conn found");
        return;
    }
    remote_connections[conn_idx].id = i;
    WireGuard **conn = &remote_connections[conn_idx].conn;
    if (*conn != nullptr) {
        delete *conn;
    }
    *conn = new WireGuard();

    (*conn)->begin(internal_ip,
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
    connection_state.get(conn_idx + 1)->get("user")->updateUint(user_id);
    connection_state.get(conn_idx + 1)->get("connection")->updateUint(conn_id);
}

void RemoteAccess::setup_inner_socket()
{
    if (inner_socket > 0) {
        return;
    }

    inner_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (inner_socket < 0) {
        logger.printfln("Failed to create inner socket: (%i)%s", errno, strerror_r(errno, nullptr, 0));
        return;
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
        inner_socket = -1;
        return;
    }

    ret = fcntl(inner_socket, F_SETFL, O_NONBLOCK);
    if (ret == -1) {
        logger.printfln("Setting socket to non_blocking caused and error: (%i)%s", errno, strerror_r(errno, nullptr, 0));
        close(inner_socket);
        inner_socket = -1;
    }

    return;
}

uint8_t RemoteAccess::get_connection(uint8_t conn_id)
{
    uint8_t first_free_idx = 255;
    for (uint8_t i = 0; i < 5; i++) {
        if (remote_connections[i].id == conn_id) {
            return i;
        } else if (remote_connections[i].conn == nullptr && first_free_idx == 255) {
            first_free_idx = i;
        }
    }

    if (first_free_idx != 255) {
        return first_free_idx;
    } else {
        return 255;
    }
}

void RemoteAccess::run_management()
{
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
        inner_socket = -1;
        setup_inner_socket();
        return;
    }
    if (ret != sizeof(management_command_packet)) {
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
    if (static_cast<uint32_t>(command->connection_no) >= MAX_KEYS_PER_USER * MAX_USERS) {
        return;
    }

    uint8_t conn_idx = get_connection(command->connection_no);
    if (conn_idx == 255) {
        logger.printfln("No free connection found");
        return;
    }
    WireGuard **conn = &remote_connections[conn_idx].conn;
    switch (command->command_id) {
        case management_command_id::Connect: {
            remote_connections[conn_idx].id = command->connection_no;
            if (conn == nullptr) {
                return;
            }
            if (*conn != nullptr && (*conn)->is_peer_up(nullptr, nullptr)) {
                return;
            }
            if ((*conn) == nullptr) {
                uint32_t conn_id = command->connection_no % MAX_KEYS_PER_USER;
                uint32_t user_id = command->connection_no / MAX_KEYS_PER_USER;
                logger.printfln("Opening connection %lu for user %lu", conn_id, user_id);
            }

            uint16_t local_port = 51821;
            port_discovery_packet response;
            response.charger_id = local_uid_num;
            response.connection_no = command->connection_no;
            memcpy(&response.connection_uuid, &command->connection_uuid, 16);

            CoolString remote_host = config.get("relay_host")->asString();
            if (*conn != nullptr) {
                (*conn)->end();
            }
            local_port = find_next_free_port(local_port);
            create_sock_and_send_to(&response, sizeof(response), remote_host.c_str(), 51820, &local_port);
            connect_remote_access(command->connection_no, local_port);
        } break;

        case management_command_id::Disconnect:
            remote_connections[conn_idx].id = 255;
            if (conn == nullptr || *conn == nullptr) {
                logger.printfln("Not found");
                break;
            }
            logger.printfln("Closing connection %lu for user %lu",
                            connection_state.get(conn_idx + 1)->get("connection")->asUint(),
                            connection_state.get(conn_idx + 1)->get("user")->asUint());
            (*conn)->end();
            delete *conn;
            *conn = nullptr;
            connection_state.get(conn_idx + 1)->get("user")->updateUint(255);
            connection_state.get(conn_idx + 1)->get("connection")->updateUint(255);
            break;
    }
}

void RemoteAccess::close_all_remote_connections() {
    timeval now;
    if (!rtc.clock_synced(&now)) {
        now.tv_sec = 0;
    }
    for (uint8_t i = 0; i < 5; i++) {
        if (remote_connections[i].conn != nullptr) {
            logger.printfln("Closing connection %lu for user %lu",
                            connection_state.get(i + 1)->get("connection")->asUint(),
                            connection_state.get(i + 1)->get("user")->asUint());
            remote_connections[i].conn->end();
            delete remote_connections[i].conn;
            remote_connections[i].conn = nullptr;
            remote_connections[i].id = 255;
            connection_state.get(i + 1)->get("user")->updateUint(255);
            connection_state.get(i + 1)->get("connection")->updateUint(255);
            connection_state.get(i + 1)->get("last_state_change")->updateUint53(now.tv_sec);
        }
    }
}
