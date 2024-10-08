/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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
#include "certs.h"

#include <LittleFS.h>
#include <mbedtls/pem.h>
#include <mbedtls/error.h>

#include "module_dependencies.h"

#define CERT_DIRECTORY "/certs"

static inline String get_cert_path(uint8_t cert_id) {
    return String(CERT_DIRECTORY "/") + cert_id;
}

static inline String get_cert_name_path(uint8_t cert_id) {
    return String(CERT_DIRECTORY "/") + cert_id + "_name";
}

void Certs::pre_setup()
{
    state_certs_prototype = Config::Object({
        {"id", Config::Uint(0, 0, MAX_CERTS)},
        {"name", Config::Str("", 0, MAX_CERT_NAME)},
    });

    state = Config::Object({
        {"certs", Config::Array({},
            &state_certs_prototype,
            0, MAX_CERTS, Config::type_id<Config::ConfObject>())
        }
    });

    remove = Config::Object({
        {"id", Config::Uint(0, 0, MAX_CERTS)}
    });

    add = ConfigRoot{Config::Object({
        {"id", Config::Uint(0, 0, MAX_CERTS)},
        {"name", Config::Str("", 0, 32)},
        {"cert", Config::Str("", 0, MAX_CERT_SIZE)}
    }), [](Config &cfg, ConfigSource source) -> String {
        const auto &cert = cfg.get("cert")->asString();
        if (cert.length() == 0)
            return "";

        mbedtls_pem_context ctx;
        mbedtls_pem_init(&ctx);
        defer {mbedtls_pem_free(&ctx);};

        size_t ignored;
        int result = mbedtls_pem_read_buffer(
                        &ctx,
                        "-----BEGIN CERTIFICATE-----",
                        "-----END CERTIFICATE-----",
                        (const unsigned char *)cert.c_str(),
                        nullptr, 0,
                        &ignored);

        mbedtls_pem_context key_ctx;
        mbedtls_pem_init(&key_ctx);

        int key_res = mbedtls_pem_read_buffer(
                        &key_ctx,
                        "-----BEGIN PRIVATE KEY-----",
                        "-----END PRIVATE KEY-----",
                        (const unsigned char *)cert.c_str(),
                        nullptr, 0,
                        &ignored);
        mbedtls_pem_free(&key_ctx);

        int second_key_res = mbedtls_pem_read_buffer(
                        &key_ctx,
                        "-----BEGIN RSA PRIVATE KEY-----",
                        "-----END RSA PRIVATE KEY-----",
                        (const unsigned char *)cert.c_str(),
                        nullptr, 0,
                        &ignored);
        mbedtls_pem_free(&key_ctx);

        int third_key_res = mbedtls_pem_read_buffer(
                        &key_ctx,
                        "-----BEGIN EC PRIVATE KEY-----",
                        "-----END EC PRIVATE KEY-----",
                        (const unsigned char *)cert.c_str(),
                        nullptr, 0,
                        &ignored);
        mbedtls_pem_free(&key_ctx);

        if ((result != 0) && (key_res != 0) && (second_key_res != 0) && (third_key_res != 0)) {
            if (result != 0) {
                char buf[256] = {0};
                mbedtls_strerror(result, buf, sizeof(buf));
                return String("Failed to parse certificate: ") + buf;
            }
            if (key_res != 0) {
                char buf[256] = {0};
                mbedtls_strerror(key_res, buf, sizeof(buf));
                return String("Failed to parse private key: ") + buf;
            }
            if (second_key_res != 0) {
                char buf[256] = {0};
                mbedtls_strerror(second_key_res, buf, sizeof(buf));
                return String("Failed to parse RSA private key: ") + buf;
            }
            if (third_key_res != 0) {
                char buf[256] = {0};
                mbedtls_strerror(third_key_res, buf, sizeof(buf));
                return String("Failed to parse EC private key: ") + buf;
            }
        }
        return "";
    }};
}

void Certs::update_state()
{
    state.get("certs")->removeAll();

    for (uint8_t i = 0; i < MAX_CERTS; ++i) {
        String path = get_cert_path(i);

        if (!LittleFS.exists(path) || !LittleFS.exists(path + "_name"))
            continue;

        auto new_cfg = state.get("certs")->add();
        new_cfg->get("id")->updateUint(i);

        File f = LittleFS.open(path + "_name", "r");
        char name[MAX_CERT_NAME + 1] = {0};
        size_t written = f.readBytes(name, MAX_CERT_NAME);
        String cert_name{name, written};
        new_cfg->get("name")->updateString(cert_name);
    }
}

void Certs::setup()
{
    LittleFS.mkdir(CERT_DIRECTORY);
    update_state();

    initialized = true;
}

void Certs::register_urls()
{
    api.addState("certs/state", &state);

    api.addCommand("certs/add", &add, {}, [this](String &error) {
        if (add.get("cert")->asString().length() == 0) {
            error = "Adding an empty certificate is not allowed. Did you mean to call certs/modify?";
            return;
        }

        uint8_t cert_id = add.get("id")->asUint();

        for (const auto &cert: state.get("certs")) {
            if (cert.get("id")->asUint() == cert_id) {
                error = String("A certificate with ID ") + cert_id + " does already exist! Did you mean to call certs/modify?";
                return;
            }
        }

        {
            File f = LittleFS.open(get_cert_name_path(cert_id), "w");
            auto cert_name = add.get("name")->asString();
            f.write((const uint8_t *) cert_name.c_str(), cert_name.length());
        }

        {
            File f = LittleFS.open(get_cert_path(cert_id), "w");
            // TODO: more robust writing
            auto &cert = add.get("cert")->asString();
            f.write((const uint8_t *) cert.c_str(), cert.length());
        }

        // Cert is written into flash. Drop from config to free memory.
        add.get("cert")->clearString();

        this->update_state();
    }, true);

    api.addCommand("certs/modify", &add, {}, [this](String &error) {
        uint8_t cert_id = add.get("id")->asUint();
        bool found = false;
        for (const auto &cert: state.get("certs")) {
            if (cert.get("id")->asUint() == cert_id) {
                found = true;
                break;
            }
        }

        if (!found) {
            error = String("No cert with ID ") + cert_id + " found! Did you mean to call certs/add?";
        }

        {
            File f = LittleFS.open(get_cert_name_path(cert_id), "w");
            auto cert_name = add.get("name")->asString();
            f.write((const uint8_t *) cert_name.c_str(), cert_name.length());
        }

        if (add.get("cert")->asString().length() != 0) {
            File f = LittleFS.open(get_cert_path(cert_id), "w");
            // TODO: more robust writing
            auto &cert = add.get("cert")->asString();
            f.write((const uint8_t *) cert.c_str(), cert.length());
        }

        // Cert is written into flash. Drop from config to free memory.
        add.get("cert")->clearString();

        this->update_state();
    }, true);

    api.addCommand("certs/remove", &remove, {}, [this](String &error) {
        uint8_t cert_id = remove.get("id")->asUint();

        String path = get_cert_path(cert_id);

        if (!LittleFS.exists(path)) {
            error = String("No cert with ID ") + cert_id + " found!";
            return;
        }

        LittleFS.remove(path);
        LittleFS.remove(path + "_name");
        this->update_state();
    }, true);
}

std::unique_ptr<unsigned char[]> Certs::get_cert(uint8_t cert_id, size_t *out_cert_len)
{
    String path = get_cert_path(cert_id);

    if (!LittleFS.exists(path)) {
        return nullptr;
    }

    File f = LittleFS.open(path, "r");
    // Allocate one byte more so that the cert is also null-terminated.
    // Some ESP-IDF APIs need both a null-terminated string _and_ passing the strings length.
    auto result = heap_alloc_array<unsigned char>(f.size() + 1);
    size_t buf_size = f.size();
    while (f.available())
        buf_size -= f.read(result.get(), buf_size);

    *out_cert_len = f.size();
    result[*out_cert_len] = 0;
    return result;
}
