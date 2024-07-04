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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "digest_auth.h"

void Authentication::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enable_auth", Config::Bool(false)},
        {"username", Config::Str("", 0, 32)},
        {"digest_hash", Config::Str("", 0, 32)},
    }), [this](Config &update, ConfigSource source) -> String {
        if (update.get("enable_auth")->asBool() && update.get("digest_hash")->asString().isEmpty())
            return "Authentication can not be enabled if no password/digest hash is set.";

        // Theoretically authentication works without a username; not in Firefox with xhr.open(..., username, password);
        if (update.get("enable_auth")->asBool() && update.get("username")->asString().isEmpty())
            return "Authentication can not be enabled if no username is set.";

        if (update.get("username")->asString() != this->config.get("username")->asString() && update.get("digest_hash")->asString() == this->config.get("digest_hash")->asString())
            return "To change the username the digest hash also has to be updated.";

        return "";
    }};
}

void Authentication::setup()
{
    api.restorePersistentConfig("authentication/config", &config);

    if (config.get("enable_auth")->asBool()) {
        String user = config.get("username")->asString(); // Create copies of possibly emphemeral Strings from config.
        String digest_hash = config.get("digest_hash")->asString();

        server.onAuthenticate_HTTPThread([user, digest_hash](WebServerRequest req) -> bool {
            String auth = req.header("Authorization");
            if (auth.isEmpty()) {
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
    api.addPersistentConfig("authentication/config", &config, {"digest_hash"});
}
