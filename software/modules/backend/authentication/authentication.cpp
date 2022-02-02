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
        {"username", Config::Str("", 0, 64)},
        {"password", Config::Str("", 0, 64)},
    }), [](Config &update) {
        if (update.get("enable_auth")->asBool() && update.get("password")->asString() == "")
            return String("Authentication can not be enabled if no password is set.");

        if (update.get("enable_auth")->asBool() && update.get("username")->asString() == "")
            return String("Authentication can not be enabled if no username is set.");

        if (!update.get("enable_auth")->asBool() && update.get("password")->asString() != "")
            update.get("password")->updateString("");

        return String("");
    }};
}

void Authentication::setup()
{
    api.restorePersistentConfig("authentication/config", &authentication_config);

    if (authentication_config.get("enable_auth")->asBool()) {
        String user = authentication_config.get("username")->asString();
        String pass = authentication_config.get("password")->asString();

        server.setAuthentication([user, pass](WebServerRequest req) -> bool {
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

            return checkDigestAuthentication(fields, req.methodString(), user.c_str(), pass.c_str(), DEFAULT_REALM, false, nullptr, nullptr, nullptr);
        });
        logger.printfln("Web interface authentication enabled.");
    }

    initialized = true;
}

void Authentication::register_urls()
{
    api.addPersistentConfig("authentication/config", &authentication_config, {"password"}, 10000);
}

void Authentication::loop()
{

}
