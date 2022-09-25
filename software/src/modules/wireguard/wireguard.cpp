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
#include "wireguard.h"

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"

#include <WiFi.h>

extern API api;
extern EventLog logger;
extern TaskScheduler task_scheduler;

#include "mbedtls/base64.h"

String check_key(String key, bool enable)
{
    if (key.length() > 0) {
        if (key.length() != 44)
            return " had unexpected length. Expecting a base64 encoded string of length 44.";

        size_t written;
        if (mbedtls_base64_decode(nullptr, 0, &written, (const unsigned char *)key.c_str(), key.length()) == MBEDTLS_ERR_BASE64_INVALID_CHARACTER)
            return " contained unexpected character. Expecting a base64 encoded string.";
        if (written != 32)
            return " decodes to a key of unexpected length. Expecting a base64 encoded string that decodes to a 32 byte long key.";
    } else if (enable) {
        return " is empty, but Wireguard shall be enabed. Set this key to be able to enable Wireguard.";
    }
    return "";
}

void Wireguard::pre_setup()
{
    config = ConfigRoot{Config::Object({
        {"enabled", Config::Bool(false)},
        {"make_default_interface", Config::Bool(true)},

        {"internal_ip",      Config::Str("0.0.0.0", 7, 15)},
        {"internal_subnet",  Config::Str("0.0.0.0", 7, 15)},
        {"internal_gateway", Config::Str("0.0.0.0", 7, 15)},

        {"remote_host", Config::Str("", 0, 64)},
        {"remote_port", Config::Uint16(51820)},

        {"private_key",       Config::Str("", 0, 44)},
        {"remote_public_key", Config::Str("", 0, 44)},
        {"preshared_key",     Config::Str("", 0, 44)},

        {"allowed_ip",     Config::Str("0.0.0.0", 7, 15)},
        {"allowed_subnet", Config::Str("0.0.0.0", 7, 15)}
    }), [](Config &cfg) -> String {
        const char *internal_ip = cfg.get("internal_ip")->asCStr();
        const char *internal_subnet = cfg.get("internal_subnet")->asCStr();
        const char *internal_gateway = cfg.get("internal_gateway")->asCStr();
        const char *allowed_ip = cfg.get("allowed_ip")->asCStr();
        const char *allowed_subnet = cfg.get("allowed_subnet")->asCStr();

        IPAddress unused;

        if (!unused.fromString(internal_ip))
            return "Failed to parse \"internal_ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(internal_subnet))
            return "Failed to parse \"internal_subnet\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(internal_gateway))
            return "Failed to parse \"internal_gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(allowed_ip))
            return "Failed to parse \"allowed_ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(allowed_subnet))
            return "Failed to parse \"allowed_subnet\": Expected format is dotted decimal, i.e. 10.0.0.1";


        bool enable = cfg.get("enabled")->asBool();
        String private_key = cfg.get("private_key")->asString();
        String result = check_key(private_key, enable);
        if (result != "")
            return String("\"private_key\"") + result;

        String remote_public_key = cfg.get("remote_public_key")->asString();
        result = check_key(remote_public_key, enable);
        if (result != "")
            return String("\"remote_public_key\"") + result;

        String preshared_key = cfg.get("preshared_key")->asString();

        // pass false because an empty preshared_key is always allowed.
        result = check_key(preshared_key, false);
        if (result != "")
            return String("\"preshared_key\"") + result;

        return "";
    }};

    state = Config::Object({
        {"state", Config::Uint(0)} // 0 not configured, 1 waiting for time sync 2 not connected 3 connected
    });
}

void Wireguard::start_wireguard()
{
    static bool done = false;
    if (done)
        return;

    struct timeval tv;
    if (!clock_synced(&tv))
        return;

    done = true;

    IPAddress internal_ip;
    IPAddress internal_subnet;
    IPAddress internal_gateway;
    IPAddress allowed_ip;
    IPAddress allowed_subnet;

    internal_ip.fromString(config.get("internal_ip")->asCStr());
    internal_subnet.fromString(config.get("internal_subnet")->asCStr());
    internal_gateway.fromString(config.get("internal_gateway")->asCStr());
    allowed_ip.fromString(config.get("allowed_ip")->asCStr());
    allowed_subnet.fromString(config.get("allowed_subnet")->asCStr());

    logger.printfln("Got NTP sync. Connecting to WireGuard peer %s:%u", config.get("remote_host")->asCStr(), config.get("remote_port")->asUint());

    wg.begin(internal_ip,
             internal_subnet,
             internal_gateway,
             config.get("private_key")->asCStr(),
             config.get("remote_host")->asCStr(),
             config.get("remote_public_key")->asCStr(),
             config.get("remote_port")->asUint(),
             allowed_ip,
             allowed_subnet,
             config.get("make_default_interface")->asBool(),
             config.get("preshared_key")->asString().length() > 0 ? config.get("preshared_key")->asCStr() : nullptr);

    task_scheduler.scheduleWithFixedDelay([this]() {
        bool up = wg.is_peer_up(nullptr, nullptr);

        if(state.get("state")->updateUint(up ? 3 : 2))
            logger.printfln("WireGuard connection %s", up ? "established" : "lost");
    }, 1000, 1000);

    state.get("state")->updateUint(2);
}

void Wireguard::setup()
{
    api.restorePersistentConfig("wireguard/config", &config);

    initialized = true;

    if (!config.get("enabled")->asBool())
        return;

    logger.printfln("WireGuard enabled. Waiting for NTP time sync.");

    state.get("state")->updateUint(1);

    task_scheduler.scheduleWithFixedDelay([this](){
        start_wireguard();
    }, 1000, 1000);
}

void Wireguard::register_urls()
{
    api.addPersistentConfig("wireguard/config", &config, {
        "private_key",
        "remote_public_key",
        "preshared_key"
    }, 1000);

    api.addState("wireguard/state", &state, {}, 1000);
}

void Wireguard::loop()
{

}
