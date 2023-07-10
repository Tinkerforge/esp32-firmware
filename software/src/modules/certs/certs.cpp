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

void Certs::pre_setup() {
    state = Config::Object({
        {"certs", Config::Array({},
            new Config(Config::Object({
                {"id", Config::Uint(0, 0, MAX_CERTS)},
                {"name", Config::Str("", 0, 32)},
                {"size", Config::Uint16(0)},
            })),
            0, MAX_CERTS, Config::type_id<Config::ConfObject>())
        }
    });

    remove = Config::Object({
        {"cert_id", Config::Uint(0, 0, 8)}
    });
}

void Certs::update_state() {
    state.get("certs")->removeAll();

    for(uint8_t i = 0; i < MAX_CERTS; ++i) {
        String path = String("/certs/") + i;
        logger.printfln("Checking %s", path.c_str());
        if (!LittleFS.exists(path))
            continue;
        File f = LittleFS.open(path, "r");
        auto new_cfg = certs.state.get("certs")->add();
        new_cfg->get("name")->updateString(f.name());
        new_cfg->get("size")->updateUint(f.size());
        new_cfg->get("id")->updateUint(i);
    }
}

void Certs::setup() {
    api.restorePersistentConfig("certs/config", &config);
    config_in_use = config;

    LittleFS.mkdir("/certs");
    update_state();
}

void Certs::register_urls()
{
    api.addState("certs/state", &state, {}, 1000);
    api.addPersistentConfig("certs/config", &config, {}, 1000);

    api.addRawCommand("certs/add", [this](char *payload, size_t size) {
        if (size < 3)
            return "Too short";

        if (size > MAX_CERT_SIZE + 2)
            return "Too long";

        if (payload[1] != ';' || !isdigit(payload[0]))
            return "Wrong format";

        uint8_t cert_num = payload[0] - '0';

        if (cert_num >= MAX_CERTS)
            return "cert_id too high";

        File f = LittleFS.open(String("/certs/") + payload[0], "w");

        // Skip header
        payload += 2;
        size -= 2;

        // TODO: more robust writing
        size_t written = f.write((const uint8_t *)payload, size);
        logger.printfln("Written %u; size %u", written, size);

        this->update_state();
        return "";
    }, true);

    api.addCommand("certs/remove", &remove, {}, [this](){
        String path = String("/certs/") + remove.get("cert_id")->asUint();
        logger.printfln("Removing %s", path.c_str());

        if (!LittleFS.exists(path))
            return "Not found";

        LittleFS.remove(path);
        this->update_state();
        return "";
    }, true);
}

std::unique_ptr<unsigned char[]> Certs::get_cert(uint8_t cert_id, size_t *out_cert_len) {
    String path = String("/certs/") + cert_id;

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
