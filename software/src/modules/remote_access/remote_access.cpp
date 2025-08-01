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
#include "options.h"
#include "esp_tls.h"
#include "esp_tls_errors.h"
#include "event_log_prefix.h"
#include "tools.h"
#include "tools/dns.h"

extern "C" esp_err_t esp_crt_bundle_attach(void *conf);
extern char local_uid_str[32];
extern uint32_t local_uid_num;

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

static int create_sock_and_send_to(const void *payload, size_t payload_len, const ip_addr_t ip, uint16_t port, uint16_t local_port)
{
    sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_addr.s_addr = ip.u_addr.ip4.addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        logger.printfln("Remote Access: failed to send management frame");
        return sock;
    }
    int ret = fcntl(sock, F_SETFL, O_NONBLOCK);
    if (ret == -1) {
        logger.printfln("Setting socket to non_blocking caused and error: %s (%i)", strerror(errno), errno);
        close(sock);
        return -1;
    }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));

    //local_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); // address already set by memset
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(local_port);
    ret = bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if (ret == -1) {
        logger.printfln("Binding socket to port %u caused and error: %s (%i)", local_port, strerror(errno), errno);
        close(sock);
        return -1;
    }

    ret = sendto(sock, payload, payload_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    close(sock);
    return ret;
}

void RemoteAccess::pre_setup()
{
    users_config_prototype = Config::Object({
        {"id",Config::Uint8(0)},
        {"email", Config::Str("", 0, 64)},
        {"uuid", Config::Str("", 0, 36)},
        {"public_key", Config::Str("", 0, 44)},
    });

    // We cant reset this because it is a state since it contains values (uuid, password, users) that should
    // never ever be writable by a user. The only way to fix this is to completely separate the
    // "normal" config from the config that stores the chargers credentials.
    config = Config::Object({
        {"uuid", Config::Str("", 0, 36)},
        {"enable", Config::Bool(false)},
        {"password", Config::Str("", 0, 32)},
        {"relay_host", Config::Str(OPTIONS_REMOTE_ACCESS_HOST(), 0, 64)},
        {"relay_port", Config::Uint16(443)},
        {"cert_id", Config::Int8(-1)},
        {"users", Config::Array({}, &users_config_prototype, 0, OPTIONS_REMOTE_ACCESS_MAX_USERS(), Config::type_id<Config::ConfObject>())},
    });

    registration_config = Config::Object({
        {"enable", Config::Bool(false)},
        {"relay_host", Config::Str("", 0, 32)},
        {"relay_port", Config::Uint16(443)},
        {"email", Config::Str("", 0, 64)},
        {"cert_id", Config::Int8(-1)},
    });

    connection_state_prototype = Config::Object({
        {"state", Config::Uint8(1)},
        {"user", Config::Uint8(255)},
        {"connection", Config::Uint8(255)},
        // unix timestamp
        {"last_state_change", Config::Uint53(0)},
    });

    ping_state = Config::Object({
        {"packets_sent", Config::Uint32(0)},
        {"packets_received", Config::Uint32(0)},
        {"time_elapsed_ms", Config::Uint32(0)},
    });

    connection_state = Config::Array({},
        &connection_state_prototype,
        MAX_USER_CONNECTIONS + 1,
        MAX_USER_CONNECTIONS + 1,
        Config::type_id<Config::ConfUint>());

    connection_state.reserve(MAX_USER_CONNECTIONS + 1);

    for (int i = 0; i < MAX_USER_CONNECTIONS + 1; ++i) {
        connection_state.add()->get("state")->updateUint(1); // Set the default here so that the generic prototype can be used.
    }

    registration_state = Config::Object({{"state", Config::Enum<RegistrationState>(RegistrationState::None)}, {"message", Config::Str("", 0, 64)}});
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

static std::unique_ptr<uint8_t[]> decode_base64(const String &input, size_t buffer_size)
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
    api.addState("remote_access/ping_state", &ping_state);

    server.on("/remote_access/reset_registration_state", HTTP_PUT, [this](WebServerRequest request) {
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::None);
        registration_state.get("message")->clearString();
        return request.send(200);
    });

    api.addCommand(
        "remote_access/start_ping",
        Config::Null(),
        {},
        [this](String & err) {
            if (ping != nullptr) {
                err = "Ping already started";
                return;
            }
            int start_err = start_ping();
            if (start_err != 0) {
                err = "Failed to start ping: " + String(start_err);
            }
        },
        true);

    api.addCommand(
        "remote_access/stop_ping",
        Config::Null(),
        {},
        [this](String & /*errmsg*/) {
            stop_ping();
        },
        true);

    api.addCommand(
        "remote_access/config_update",
        &registration_config,
        {"password", "email"},
        [this](String & /*errmsg*/) {
            config.get("enable")->updateBool(registration_config.get("enable")->asBool());
            config.get("relay_host")->updateString(registration_config.get("relay_host")->asString());
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

            for (uint32_t user = 0; user < OPTIONS_REMOTE_ACCESS_MAX_USERS() + 1; user++) {
                for (int i = 0; i < OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER(); i++) {
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
            if (!error.isEmpty()) {
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
            if (!error.isEmpty()) {
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
                snprintf(err_str, sizeof(err_str), "Failed to deserialize request body: %s", error.c_str());
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", err_str);
            }
        }

        {
            String error = registration_config.update_from_json(doc["config"], true, ConfigSource::API);
            if (!error.isEmpty()) {
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", error.c_str());
            }
        }

        const String login_key = doc["login_key"];
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
                snprintf(err_str, sizeof(err_str), "Failed to deserialize request body: %s", error.c_str());
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", err_str);
            }
        }

        {
            String error = registration_config.update_from_json(doc["config"], true, ConfigSource::API);
            if (!error.isEmpty()) {
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", error.c_str());
            }
        }

        if (!registration_config.get("enable")->asBool()) {
            this->request_cleanup();
            return request.send(400, "text/plain; charset=utf-8", "Calling register without enable beeing true is not supported anymore");
        }

        const String &note = doc["note"];
        size_t encrypted_note_size = crypto_box_SEALBYTES + note.length();
        size_t bs64_note_size = 4 * (encrypted_note_size / 3) + 5;
        const String &name = api.getState("info/display_name")->get("display_name")->asString();
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
        {
            int i = 0;
            // JsonArray already has reference semantics. No need for &.
            for (const auto key : doc["keys"].as<JsonArray>()) {
                serializer.addObject();
                serializer.addMemberStringF("charger_address", "10.123.%i.2", i);
                serializer.addMemberString("charger_public", key["charger_public"]);
                serializer.addMemberNumber("connection_no", (int32_t)i);
                serializer.addMemberStringF("web_address", "10.123.%i.3", i);
                const String wg_key = key["web_private"];

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

                const String psk = key["psk"];
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
                if (i == OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER())
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
        const String &uuid = doc["user_uuid"];
        const String &token = doc["auth_token"];
        if (uuid != "null" && token != "null") {
            serializer.addMemberString("user_id", uuid.c_str());
            serializer.addMemberString("token", token.c_str());
            snprintf(url,
                    sizeof(url),
                    "https://%s:%lu/api/add_with_token",
                    registration_config.get("relay_host")->asEphemeralCStr(),
                    registration_config.get("relay_port")->asUint());
        } else {
            snprintf(url,
                    sizeof(url),
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
                if (i == OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER())
                    break;
            }
        }

        registration_state.get("message")->clearString();
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::InProgress);

        uint8_t public_key[50];
        mbedtls_base64_encode(public_key, sizeof(public_key), &olen, (uint8_t *)pk, crypto_box_PUBLICKEYBYTES);
        auto next_stage = [this, key_cache, public_key, olen](const Config &cfg) {
            const String pub_key((char *)public_key, olen);
            this->parse_registration(cfg, key_cache, pub_key);
        };
        this->run_request_with_next_stage(url, HTTP_METHOD_PUT, ptr.get(), size, registration_config, std::move(next_stage));

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
        for (uint32_t i = 1; i < OPTIONS_REMOTE_ACCESS_MAX_USERS() + 1; i++) {
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
                snprintf(err_str, sizeof(err_str), "Failed to deserialize request body: %s", error.c_str());
                this->request_cleanup();
                return request.send(400, "text/plain; charset=utf-8", err_str);
            }
        }

        const String &email = doc["email"];
        const String &uuid = doc["user_uuid"];
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
        const String &name = api.getState("info/display_name")->get("display_name")->asString();
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
        int i = 0;
        for (auto key : doc["wg_keys"].as<JsonArray>()) {
            serializer.addObject();
            int connection_number = (next_user_id - 1) * OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER() + i;
            serializer.addMemberStringF("charger_address", "10.123.%i.2", connection_number);
            serializer.addMemberStringF("web_address", "10.123.%i.3", connection_number);
            serializer.addMemberString("charger_public", key["charger_public"]);

            const String psk = key["psk"];
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

            const String web_private = key["web_private"];
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
                 sizeof(url),
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
            if (a == OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER())
                break;
        }

        uint8_t public_key[50];
        mbedtls_base64_encode(public_key, sizeof(public_key), &olen, (uint8_t *)pk.get(), crypto_box_PUBLICKEYBYTES);
        const String pub_key(public_key, olen);

        if (https_client == nullptr) {
            https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
        }

        https_client->set_header("Content-Type", "application/json");
        auto next_stage = [this, key_cache, pub_key, next_user_id, email](const Config &/*cfg*/) {
            this->parse_add_user(key_cache, pub_key, email, next_user_id);
        };

        this->run_request_with_next_stage(url, HTTP_METHOD_PUT, json.get(), size, config, std::move(next_stage));

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
                snprintf(err_str, sizeof(err_str), "Failed to deserialize request body: %s", error.c_str());
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
        for (int i = 0; i < OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER(); i++) {
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
            TFJsonSerializer serializer{json, sizeof(json)};
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
                     sizeof(url),
                     "https://%s:%lu/api/selfdestruct",
                     config.get("relay_host")->asEphemeralCStr(),
                     config.get("relay_port")->asUint());
            run_request_with_next_stage(url, HTTP_METHOD_DELETE, json, json_size, config, [this](const Config &/*cfg*/) {
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
            for (size_t i = 0; i < MAX_USER_CONNECTIONS; i++) {
                uint32_t state = 1;
                if (this->remote_connections[i].conn != nullptr) {
                    state = this->remote_connections[i].conn->is_peer_up(nullptr, nullptr) ? 2 : 1;
                }

                auto conn_state = this->connection_state.get(i + 1); // 0 is the management connection
                if (conn_state->get("state")->updateUint(state)) {
                    uint32_t conn = conn_state->get("connection")->asUint();
                    uint32_t user = conn_state->get("user")->asUint();
                    conn_state->get("last_state_change")->updateUint53((uint64_t)now.tv_sec);
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

            auto mgmt_state = this->connection_state.get(0);
            if (mgmt_state->get("state")->updateUint(state)) {
                mgmt_state->get("last_state_change")->updateUint53((uint64_t)now.tv_sec);
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

    network.on_network_connected([this](const Config *connected) {
        if (connected->asBool()) {
            // Start task if not scheduled yet.
            if (!this->task_id) {
                this->task_id = task_scheduler.scheduleWithFixedDelay([this]() {
                    if (!this->management_request_done && !this->management_auth_failed) {
                        this->resolve_management();
                    }
                }, 30_s);
            }
        } else {
            // Cancel task if currently scheduled.
            if (this->task_id) {
                task_scheduler.cancel(this->task_id);
                this->task_id = 0;
            }
        }
        return EventResult::OK;
    });
}

bool RemoteAccess::user_already_registered(const String &email)
{
    for (const auto &user : config.get("users")) {
        if (email == user.get("email")->asString()) {
            return true;
        }
    }

    return false;
}

void RemoteAccess::run_request_with_next_stage(const char *url,
                                               esp_http_client_method_t method,
                                               const char *body,
                                               int body_size,
                                               const Config &config,
                                               std::function<void(const Config &config)> &&next_stage)
{
    response_body.clear();

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
                                if (event->error_http_status == 401) {
                                    this->management_auth_failed = true;
                                    logger.printfln("Management authentication failed (401) - not reconnecting");
                                    this->config.get("enable")->updateBool(false);
                                    api.writeConfig("remote_access/config", &this->config);
                                }
                            }
                        } else {
                            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
                            {
                                char err_buf[64];
                                snprintf(err_buf, sizeof(err_buf), "Received status-code %i", (int)event->error_http_status);
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
                                snprintf(err_buf, sizeof(err_buf), "Error code %i", (int)event->error);
                                registration_state.get("message")->updateString(err_buf);
                            }
                        }
                        this->cleanup_after();
                        break;
                }
                response_body.clear();
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

            case AsyncHTTPSClientEventType::Redirect:
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

void RemoteAccess::get_login_salt(const Config &config)
{
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::InProgress);
    registration_state.get("message")->clearString();
    char url[196];
    snprintf(url,
            sizeof(url),
            "https://%s:%lu/api/auth/get_login_salt?email=%s",
            config.get("relay_host")->asEphemeralCStr(),
            config.get("relay_port")->asUint(),
            config.get("email")->asEphemeralCStr());

    https_client = std::unique_ptr<AsyncHTTPSClient>{new AsyncHTTPSClient(true)};
    run_request_with_next_stage(url, HTTP_METHOD_GET, nullptr, 0, config, [this](const Config &/*cfg*/) {
        this->parse_login_salt();
    });
}

void RemoteAccess::parse_login_salt()
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
        response_body.clear();
    }

    char base64[65] = {};
    size_t bytes_written;
    if (mbedtls_base64_encode((uint8_t *)base64, sizeof(base64), &bytes_written, login_salt, 48)) {
        registration_state.get("message")->updateString("Error while encoding login-salt");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        this->request_cleanup();
        return;
    }
    registration_state.get("message")->updateString(String(base64));
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
}

void RemoteAccess::login(const Config &config, const String &login_key)
{
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::InProgress);
    registration_state.get("message")->clearString();
    char url[128];
    snprintf(url, sizeof(url), "https://%s:%lu/api/auth/login", config.get("relay_host")->asEphemeralCStr(), config.get("relay_port")->asUint());

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

    run_request_with_next_stage(url, HTTP_METHOD_POST, body.c_str(), body.length(), config, [this](const Config &/*cfg*/) {
        registration_state.get("message")->clearString();
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
        response_body.clear();
    });
}

void RemoteAccess::get_secret(const Config &config)
{
    registration_state.get("message")->clearString();
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::InProgress);
    char url[128];
    snprintf(url,
             sizeof(url),
             "https://%s:%lu/api/user/get_secret",
             config.get("relay_host")->asEphemeralCStr(),
             config.get("relay_port")->asUint());

    run_request_with_next_stage(url, HTTP_METHOD_GET, nullptr, 0, config, [this](const Config &/*cfg*/) {
        this->parse_secret();
    });
}

void RemoteAccess::parse_secret()
{
    StaticJsonDocument<2048> doc;

    {
        DeserializationError error = deserializeJson(doc, response_body.c_str());
        if (error) {
            char err_str[64];
            snprintf(err_str, sizeof(err_str), "Error while deserializing Secret: %s", error.c_str());
            registration_state.get("message")->updateString(err_str);
            registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
            this->request_cleanup();
            return;
        }
        response_body.clear();
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
    if (mbedtls_base64_encode(encoded_secret_salt, sizeof(encoded_secret_salt), &olen, secret_salt, 48) != 0) {
        registration_state.get("message")->updateString("Error while encoding secret-salt");
        registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Error);
        this->request_cleanup();
        return;
    }

    registration_state.get("message")->updateString((char *)encoded_secret_salt);
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
}

void RemoteAccess::parse_registration(const Config &new_config, std::queue<WgKey> keys, const String &public_key)
{
    StaticJsonDocument<256> resp_doc;
    DeserializationError error = deserializeJson(resp_doc, response_body.begin(), response_body.length());

    if (error) {
        char err_str[64];
        snprintf(err_str, sizeof(err_str), "Error while deserializing registration response: %s", error.c_str());
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

    for (int i = 0; !keys.empty() && i < OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER(); i++, keys.pop()) {
        WgKey &key = keys.front();
        store_key(1, i, key.priv.c_str(), key.psk.c_str(), key.pub.c_str());
    }

    this->config.get("relay_host")->updateString(new_config.get("relay_host")->asString());
    this->config.get("relay_port")->updateUint(new_config.get("relay_port")->asUint());
    this->config.get("enable")->updateBool(new_config.get("enable")->asBool());
    this->config.get("cert_id")->updateInt(new_config.get("cert_id")->asInt());
    this->config.get("password")->updateString(charger_password);
    this->config.get("uuid")->updateString(resp_doc["charger_uuid"]);

    auto users = this->config.get("users");
    if (users->count() != 0) {
        esp_system_abort("Expected 'users' to be empty");
    }
    auto new_user = users->add();
    new_user->get("email")->updateString(new_config.get("email")->asString());
    new_user->get("id")->updateUint(1);
    new_user->get("public_key")->updateString(public_key);

    API::writeConfig("remote_access/config", &this->config);
    registration_state.get("message")->clearString();
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
    this->request_cleanup();
}

void RemoteAccess::parse_add_user(std::queue<WgKey> key_cache, const String &pub_key, const String &email, uint32_t next_user_id)
{
    for (int i = 0; !key_cache.empty() && i < OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER(); i++, key_cache.pop()) {
        const WgKey &key = key_cache.front();
        store_key(next_user_id, i, key.priv.c_str(), key.psk.c_str(), key.pub.c_str());
    }

    auto new_user = this->config.get("users")->add();
    new_user->get("email")->updateString(email);
    new_user->get("id")->updateUint(next_user_id);
    new_user->get("public_key")->updateString(pub_key);
    api.writeConfig("remote_access/config", &this->config);
    registration_state.get("message")->clearString();
    registration_state.get("state")->updateEnum<RegistrationState>(RegistrationState::Success);
}

void RemoteAccess::resolve_management()
{
    if (!this->management_request_allowed) {
        return;
    }

    char url[128];
    snprintf(url,
            sizeof(url),
            "https://%s:%lu/api/management",
            config.get("relay_host")->asEphemeralCStr(),
            config.get("relay_port")->asUint());

    const String &uuid = config.get("uuid")->asString();
    const String &name = api.getState("info/display_name")->get("display_name")->asString();
    size_t encrypted_name_size = name.length() + crypto_box_SEALBYTES;
    size_t encoded_name_size = 4 * (encrypted_name_size / 3) + 5;

    char json[1000];
    TFJsonSerializer serializer{json, sizeof(json)};
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
            for (int i = 0; i < OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER(); i++) {
                serializer.addNumber((user_id * OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER()) + i);
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
            const String &user_id = user.get("uuid")->asString();
            if (user_id.length() != 0) {
                serializer.addMemberString("user_id", user_id.c_str());
            } else {
                serializer.addMemberString("email", user.get("email")->asEphemeralCStr());
            }

            const String &public_key = user.get("public_key")->asString();
            if (public_key.length() != 0) {
                auto key = decode_base64(public_key, 32);

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
    auto callback = [this, old_api](const Config &/*cfg*/) {
        this->management_request_failed = false;
        this->management_request_allowed = true;
        this->management_auth_failed = false;

        if (old_api) {
            StaticJsonDocument<250> resp;
            {
                DeserializationError error = deserializeJson(resp, response_body.c_str());
                if (error) {
                    char err_str[64];
                    snprintf(err_str, sizeof(err_str), "Error while deserializing management response: %s", error.c_str());
                    this->request_cleanup();
                    return;
                }
                response_body.clear();
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
                snprintf(err_str, sizeof(err_str), "Error while deserializing management response: %s", error.c_str());
                this->request_cleanup();
                return;
            }
            response_body.clear();
        }

        bool changed = false;
        auto users = config.get("users");
        for (int idx = users->count() - 1; idx >= 0; idx--) {
            const String &user_email = doc["configured_users_emails"][idx];
            const String &user_uuid = doc["configured_users_uuids"][idx];
            auto user = users->get(idx);

            if (doc["configured_users"][idx] == 0) {
                uint32_t user_id = user->get("id")->asUint();
                for (int i = 0; i < OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER(); i++) {
                    remove_key(user_id, i);
                }
                users->remove(idx);
                changed = true;
            } else if (user_email != "null") {
                changed |= user->get("email")->updateString(user_email);
                changed |= user->get("uuid")->updateString(user_uuid);
            }
        }

        if (changed) {
            api.writeConfig("remote_access/config", &config);
        }

        this->management_request_done = true;
        this->last_mgmt_alive = now_us();
        this->request_cleanup();
        // Don't reconnect if authentication failed
        if (!this->management_auth_failed) {
            this->connect_management();
        }
        this->connection_state.get(0)->get("state")->updateUint(1);
    };
    this->management_request_allowed = false;
    run_request_with_next_stage(url, HTTP_METHOD_PUT, json, len, config, std::move(callback));
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
    memset(&local_addr, 0, sizeof(local_addr));

    //local_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); // address already set by memset
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
    response_body.clear();
    management_request_allowed = true;
}

void RemoteAccess::connect_management()
{
    static bool done = false;
    if (done) {
        logger.printfln("Attempted connect_management again despite being done");
        return;
    }

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

    const IPAddress internal_ip     { 10,123,123,2};
    const IPAddress internal_subnet {255,255,255,0};
    const IPAddress internal_gateway{ 10,123,123,1};
    const IPAddress allowed_ip      (IPv4); // 0.0.0.0
    const IPAddress allowed_subnet  (IPv4); // 0.0.0.0

    auto key_buf = heap_alloc_array<char>(3 * (WG_KEY_LENGTH + 1));
    char *private_key       = key_buf.get() + 0 * (WG_KEY_LENGTH + 1);
    char *psk               = key_buf.get() + 1 * (WG_KEY_LENGTH + 1);
    char *remote_public_key = key_buf.get() + 2 * (WG_KEY_LENGTH + 1);

    // WireGuard decodes those (base64 encoded) keys and stores them.
    if (!get_key(0, 0, private_key, psk, remote_public_key)) {
        logger.printfln("Can't connect to management server: no WireGuard key installed!");
        return;
    }

    // Only used for BLOCKING! DNS resolve. TODO Make this non-blocking in Wireguard-ESP32-Arduino/src/WireGuard.cpp!
    const char *remote_host = config.get("relay_host")->asUnsafeCStr();

    logger.printfln("Connecting to Management WireGuard peer %s:%u", remote_host, 51820);

    management = std::make_unique<WireGuard>();

    const uint16_t local_port = find_next_free_port(51820);
    this->setup_inner_socket();
    // management->begin copys the keys.
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

    const IPAddress internal_ip     { 10,123,  i,2};
    const IPAddress internal_subnet {255,255,255,0};
    const IPAddress internal_gateway{ 10,123,  i,1};
    const IPAddress allowed_ip      (IPv4); // 0.0.0.0
    const IPAddress allowed_subnet  (IPv4); // 0.0.0.0

    // WireGuard decodes those (base64 encoded) keys and stores them.
    char private_key[WG_KEY_LENGTH + 1];
    char psk[WG_KEY_LENGTH + 1];
    char remote_public_key[WG_KEY_LENGTH + 1];

    uint8_t conn_id = i % OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER();
    uint8_t user_id = i / OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER();
    if (!get_key(user_id + 1, conn_id, private_key, psk, remote_public_key)) {
        logger.printfln("Can't connect to web interface: no WireGuard key installed!");
        return;
    }

    // Only used for BLOCKING! DNS resolve. TODO Make this non-blocking in Wireguard-ESP32-Arduino/src/WireGuard.cpp!
    const char *remote_host = config.get("relay_host")->asUnsafeCStr();

    uint8_t conn_idx = get_connection(i);
    if (conn_idx == 255) {
        logger.printfln("No free conn found");
        return;
    }
    remote_connections[conn_idx].id = i;
    remote_connections[conn_idx].conn = std::make_unique<WireGuard>();

    remote_connections[conn_idx].conn->begin(internal_ip,
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

    auto conn_state = connection_state.get(conn_idx + 1);
    conn_state->get("user")->updateUint(user_id);
    conn_state->get("connection")->updateUint(conn_id);
}

void RemoteAccess::setup_inner_socket()
{
    if (inner_socket > 0) {
        return;
    }

    inner_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (inner_socket < 0) {
        logger.printfln("Failed to create inner socket: %s (%i)", strerror(errno), errno);
        return;
    }

    struct sockaddr_in local_addr;
    memset(&local_addr, 0, sizeof(local_addr));

    //local_addr.sin_addr.s_addr = inet_addr("0.0.0.0"); // address is already set by memset
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(12345);
    int ret = bind(inner_socket, (struct sockaddr *)&local_addr, sizeof(local_addr));
    if (ret == -1) {
        logger.printfln("Binding socket to port 12345 caused and error: %s (%i)", strerror(errno), errno);
        close(inner_socket);
        inner_socket = -1;
        return;
    }

    ret = fcntl(inner_socket, F_SETFL, O_NONBLOCK);
    if (ret == -1) {
        logger.printfln("Setting socket to non_blocking caused and error: %s (%i)", strerror(errno), errno);
        close(inner_socket);
        inner_socket = -1;
    }

    return;
}

uint8_t RemoteAccess::get_connection(uint8_t conn_id)
{
    uint8_t first_free_idx = 255;
    for (uint8_t i = 0; i < MAX_USER_CONNECTIONS; i++) {
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
    if (static_cast<uint32_t>(command->connection_no) >= OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER() * OPTIONS_REMOTE_ACCESS_MAX_USERS()) {
        return;
    }

    uint8_t conn_idx = get_connection(command->connection_no);
    if (conn_idx == 255) {
        logger.printfln("No free connection found");
        return;
    }
    auto &conn = remote_connections[conn_idx].conn;
    switch (command->command_id) {
        case management_command_id::Connect: {
            remote_connections[conn_idx].id = command->connection_no;
            if (conn != nullptr && conn->is_peer_up(nullptr, nullptr)) {
                return;
            }
            if (conn == nullptr) {
                uint32_t conn_id = command->connection_no % OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER();
                uint32_t user_id = command->connection_no / OPTIONS_REMOTE_ACCESS_MAX_KEYS_PER_USER();
                logger.printfln("Opening connection %lu for user %lu", conn_id, user_id);
            }

            uint16_t local_port = 51821;
            port_discovery_packet response;
            response.charger_id = local_uid_num;
            response.connection_no = command->connection_no;
            memcpy(&response.connection_uuid, &command->connection_uuid, 16);

            const String &remote_host = config.get("relay_host")->asString();
            if (conn != nullptr) {
                conn->end();
            }
            local_port = find_next_free_port(local_port);

            uint8_t conn_no = command->connection_no;
            dns_gethostbyname_addrtype_lwip_ctx_async(remote_host.c_str(), [this, response, local_port, conn_no](dns_gethostbyname_addrtype_lwip_ctx_async_data *data) {
                create_sock_and_send_to(&response, sizeof(response), data->addr, 51820, local_port);
                connect_remote_access(conn_no, local_port);
            }, LWIP_DNS_ADDRTYPE_IPV4);
        } break;

        case management_command_id::Disconnect:
            remote_connections[conn_idx].id = 255;
            if (conn == nullptr) {
                logger.printfln("Not found");
                break;
            }
            auto conn_state = connection_state.get(conn_idx + 1);
            logger.printfln("Closing connection %lu for user %lu",
                            conn_state->get("connection")->asUint(),
                            conn_state->get("user")->asUint());
            conn->end();
            conn = nullptr;
            conn_state->get("user")->updateUint(255);
            conn_state->get("connection")->updateUint(255);
            break;
    }
}

void RemoteAccess::close_all_remote_connections() {
    timeval now;
    if (!rtc.clock_synced(&now)) {
        now.tv_sec = 0;
    }
    for (uint8_t i = 0; i < MAX_USER_CONNECTIONS; i++) {
        if (remote_connections[i].conn != nullptr) {
            auto conn_state = connection_state.get(i + 1);
            logger.printfln("Closing connection %lu for user %lu",
                            conn_state->get("connection")->asUint(),
                            conn_state->get("user")->asUint());
            remote_connections[i].conn->end();
            remote_connections[i].conn = nullptr;
            remote_connections[i].id = 255;
            conn_state->get("user")->updateUint(255);
            conn_state->get("connection")->updateUint(255);
            conn_state->get("last_state_change")->updateUint53(now.tv_sec);
        }
    }
}

static void on_ping_success(esp_ping_handle_t handle, void *args) {
    uint8_t ttl;
    uint8_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;

    esp_ping_get_profile(handle, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(handle, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(handle, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(handle, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(handle, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    logger.printfln("Ping: seqno=%u, ttl=%u, elapsed_time=%lu ms, recv_len=%lu bytes", seqno, ttl, elapsed_time, recv_len);

    PingArgs *ping_args = static_cast<PingArgs *>(args);
    ping_args->packets_sent++;

    ping_args->packets_received++;
    task_scheduler.scheduleOnce(
        [ping_args]() {
            Config &ping_state = remote_access.get_ping_state();

            ping_state.get("packets_sent")->updateUint(ping_args->packets_sent);
            ping_state.get("packets_received")->updateUint(ping_args->packets_received);
            ping_state.get("time_elapsed_ms")->updateUint((now_us() - remote_access.get_ping_start()).to<millis_t>().as<uint32_t>());
        },
    0_ms);
}

static void on_ping_timeout(esp_ping_handle_t handle, void *args) {
    uint8_t seqno;
    ip_addr_t target_addr;

    esp_ping_get_profile(handle, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(handle, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    logger.printfln("Ping timeout: seqno=%u", seqno);

    PingArgs *ping_args = static_cast<PingArgs *>(args);
    ping_args->packets_sent++;
    task_scheduler.scheduleOnce(
        [ping_args]() {
            Config &ping_state = remote_access.get_ping_state();

            ping_state.get("packets_sent")->updateUint(ping_args->packets_sent);
            ping_state.get("packets_received")->updateUint(ping_args->packets_received);
            ping_state.get("time_elapsed_ms")->updateUint((now_us() - remote_access.get_ping_start()).to<millis_t>().as<uint32_t>());
        },
    0_ms);
}

static void on_ping_end(esp_ping_handle_t handle, void *args) {
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;

    esp_ping_get_profile(handle, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(handle, ESP_PING_PROF_REPLY, &received, sizeof(received));
    esp_ping_get_profile(handle, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
    logger.printfln("Ping end: packets transmitted=%lu, received=%lu, total_time_ms=%lu", transmitted, received, total_time_ms);

    PingArgs *ping_args = static_cast<PingArgs *>(args);
    task_scheduler.scheduleOnce(
        [ping_args, transmitted, received]() {
            Config &ping_state = remote_access.get_ping_state();

            auto elapsed = (now_us() - remote_access.get_ping_start()).to<millis_t>().as<uint32_t>();

            ping_state.get("packets_sent")->updateUint(transmitted);
            ping_state.get("packets_received")->updateUint(received);
            ping_state.get("time_elapsed_ms")->updateUint(elapsed);

            delete ping_args;
        },
    0_s);
}

int RemoteAccess::start_ping() {
    const char *host = config.get("relay_host")->asEphemeralCStr();

    dns_gethostbyname_addrtype_lwip_ctx_async(host, [this, host](dns_gethostbyname_addrtype_lwip_ctx_async_data *data) {
        PingArgs *ping_args = new PingArgs();

        esp_ping_config_t ping_config = ESP_PING_DEFAULT_CONFIG();
        ping_config.target_addr = data->addr;
        ping_config.count = ESP_PING_COUNT_INFINITE;

        esp_ping_callbacks_t cbs;
        cbs.on_ping_success = on_ping_success;
        cbs.on_ping_timeout = on_ping_timeout;
        cbs.on_ping_end = on_ping_end;
        cbs.cb_args = static_cast<void *>(ping_args);
        esp_ping_new_session(&ping_config, &cbs, &ping);
        esp_ping_start(ping);

        ping_start = millis();

        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &data->addr, str, sizeof(str));
        logger.printfln("Start pinging %s(%s)", host, str);
    }, LWIP_DNS_ADDRTYPE_IPV4);

    return 0;
}

int RemoteAccess::stop_ping() {
    if (ping != nullptr) {
        esp_ping_stop(ping);
        esp_ping_delete_session(ping);
        ping = nullptr;
    }

    return 0;
}

Config &RemoteAccess::get_ping_state() {
    return ping_state;
}

micros_t RemoteAccess::get_ping_start() {
    return ping_start;
}
