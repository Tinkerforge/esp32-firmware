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

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"

#include "LittleFS.h"

#include "modules.h"

#include <mbedtls/pem.h>
#include <mbedtls/error.h>

void Certs::pre_setup() {
    state = Config::Object({
        {"certs", Config::Array({},
            new Config(Config::Object({
                {"id", Config::Uint(0, 0, MAX_CERTS)},
                {"name", Config::Str("", 0, MAX_CERT_NAME)},
            })),
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
    }), [](Config &cfg) -> String {
        const auto &cert = cfg.get("cert")->asString();
        if (cert.length() == 0)
            return "";

        mbedtls_pem_context ctx;
        mbedtls_pem_init(&ctx);
        defer {mbedtls_pem_free(&ctx);};

        size_t ignored;
        auto result = mbedtls_pem_read_buffer(
                        &ctx,
                        "-----BEGIN CERTIFICATE-----",
                        "-----END CERTIFICATE-----",
                        (const unsigned char *)cert.c_str(),
                        nullptr, 0,
                        &ignored);
        if (result != 0) {
            char buf[256] = {0};
            mbedtls_strerror(result, buf, sizeof(buf));
            return String("Failed to parse certificate: ") + buf;
        }

        return "";
    }};
}

void Certs::update_state() {
    state.get("certs")->removeAll();

    for(uint8_t i = 0; i < MAX_CERTS; ++i) {
        String path = String("/certs/") + i;

        if (!LittleFS.exists(path) || !LittleFS.exists(path + "_name"))
            continue;

        auto new_cfg = certs.state.get("certs")->add();
        new_cfg->get("id")->updateUint(i);

        File f = LittleFS.open(path + "_name", "r");
        char name[MAX_CERT_NAME + 1] = {0};
        size_t written = f.readBytes(name, MAX_CERT_NAME);
        String cert_name{name, written};
        new_cfg->get("name")->updateString(cert_name);
    }
}

void Certs::setup() {
    LittleFS.mkdir("/certs");
    update_state();

    initialized = true;
}

void Certs::register_urls()
{
    api.addState("certs/state", &state, {}, 1000);

    api.addCommand("certs/add", &add, {}, [this](String &error) {
        error = "";

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
            File f = LittleFS.open(String("/certs/") + cert_id + "_name", "w");
            auto cert_name = add.get("name")->asString();
            f.write((const uint8_t *) cert_name.c_str(), cert_name.length());
        }

        {
            File f = LittleFS.open(String("/certs/") + cert_id, "w");
            // TODO: more robust writing
            auto &cert = add.get("cert")->asString();
            size_t written = f.write((const uint8_t *) cert.c_str(), cert.length());

            logger.printfln("Written %u; size %u", written, cert.length());
        }

        // Cert is written into flash. Drop from config to free memory.
        add.get("cert")->clearString();

        this->update_state();
    }, true);

    api.addCommand("certs/modify", &add, {}, [this](String &error) {
        error = "";

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
            File f = LittleFS.open(String("/certs/") + cert_id + "_name", "w");
            auto cert_name = add.get("name")->asString();
            f.write((const uint8_t *) cert_name.c_str(), cert_name.length());
        }

        if (add.get("cert")->asString().length() != 0) {
            File f = LittleFS.open(String("/certs/") + cert_id, "w");

            auto cert_name = add.get("name")->asString();
            cert_name.replace('\n', ' ');
            cert_name += '\n';
            f.write((const uint8_t *) cert_name.c_str(), cert_name.length());

            // TODO: more robust writing
            auto &cert = add.get("cert")->asString();
            size_t written = f.write((const uint8_t *) cert.c_str(), cert.length());

            logger.printfln("Written %u; size %u", written, cert.length());
        }

        // Cert is written into flash. Drop from config to free memory.
        add.get("cert")->clearString();

        this->update_state();
    }, true);

    api.addCommand("certs/remove", &remove, {}, [this](String &error) {
        error = "";

        uint8_t cert_id = remove.get("id")->asUint();

        String path = String("/certs/") + cert_id;

        if (!LittleFS.exists(path)) {
            error = String("No cert with ID ") + cert_id + " found!";
            return;
        }

        LittleFS.remove(path);
        LittleFS.remove(path + "_name");
        this->update_state();
    }, true);
}

std::unique_ptr<unsigned char[]> Certs::get_cert(uint8_t id, size_t *out_cert_len) {
    String path = String("/certs/") + id;

    if (!LittleFS.exists(path))
        return nullptr;

    File f = LittleFS.open(path, "r");
    auto result = heap_alloc_array<unsigned char>(f.size());
    size_t buf_size = f.size();
    while (f.available())
        buf_size -= f.read(result.get(), buf_size);

    *out_cert_len = f.size();
    return result;
}
