/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "authentication.h"

#include <Arduino.h>

#include "api.h"
#include "event_log.h"
#include "web_server.h"

#include "digest_auth.h"

extern WebServer server;
extern API api;
extern EventLog logger;

Authentication::Authentication()
{
    authentication_config = ConfigRoot{Config::Object({
        {"enable_auth", Config::Bool(false)},
        {"username", Config::Str("", 0, 32)},
        {"digest_hash", Config::Str("", 0, 32)},
    }), [](Config &update) {
        if (update.get("enable_auth")->asBool() && update.get("digest_hash")->asString() == "")
            return String("Authentication can not be enabled if no password/digest hash is set.");

        if (update.get("enable_auth")->asBool() && update.get("username")->asString() == "")
            return String("Authentication can not be enabled if no username is set.");

        if (!update.get("enable_auth")->asBool() && update.get("digest_hash")->asString() != "")
            update.get("digest_hash")->updateString("");

        return String("");
    }};
}

void Authentication::setup()
{
    api.restorePersistentConfig("authentication/config", &authentication_config);

    if (authentication_config.get("enable_auth")->asBool()) {
        String user = authentication_config.get("username")->asString();
        String digest_hash = authentication_config.get("digest_hash")->asString();

        server.setAuthentication([user, digest_hash](WebServerRequest req) -> bool {
            String auth = req.header("Authorization");
            if (auth == "") {
                return false;
            }

            if (!auth.startsWith("Digest ")) {
                return false;
            }

            auth = auth.substring(7);
            AuthFields fields = parseDigestAuth(auth.c_str());

            if (fields.username != user)
                return false;

            return checkDigestAuthentication(fields, req.methodString(), user.c_str(), digest_hash.c_str(), DEFAULT_REALM, true, nullptr, nullptr, nullptr);
        });
        logger.printfln("Web interface authentication enabled.");
    }

    initialized = true;
}

void Authentication::register_urls()
{
    api.addPersistentConfig("authentication/config", &authentication_config, {"digest_hash"}, 10000);
}

void Authentication::loop()
{
}
