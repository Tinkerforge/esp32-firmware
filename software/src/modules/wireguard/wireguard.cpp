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

#include <WiFi.h>
#include <mbedtls/base64.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"

String check_key(const String &key, bool enable)
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
        {"enable", Config::Bool(false)},
        {"make_default_interface", Config::Bool(true)},

        {"internal_ip",      Config::Str("0.0.0.0", 7, 15)},
        {"internal_subnet",  Config::Str("0.0.0.0", 7, 15)},
        {"internal_gateway", Config::Str("0.0.0.0", 7, 15)},

        {"remote_host", Config::Str("", 0, 64)},
        {"remote_port", Config::Uint16(51820)},

        {"local_port", Config::Uint16(51820)},

        {"private_key",       Config::Str("", 0, 44)},
        {"remote_public_key", Config::Str("", 0, 44)},
        {"preshared_key",     Config::Str("", 0, 44)},

        {"allowed_ip",     Config::Str("0.0.0.0", 7, 15)},
        {"allowed_subnet", Config::Str("0.0.0.0", 7, 15)}
    }), [](Config &cfg, ConfigSource source) -> String {
        IPAddress unused;

        if (!unused.fromString(cfg.get("internal_ip")->asEphemeralCStr()))
            return "Failed to parse \"internal_ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(cfg.get("internal_subnet")->asEphemeralCStr()))
            return "Failed to parse \"internal_subnet\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(cfg.get("internal_gateway")->asEphemeralCStr()))
            return "Failed to parse \"internal_gateway\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(cfg.get("allowed_ip")->asEphemeralCStr()))
            return "Failed to parse \"allowed_ip\": Expected format is dotted decimal, i.e. 10.0.0.1";

        if (!unused.fromString(cfg.get("allowed_subnet")->asEphemeralCStr()))
            return "Failed to parse \"allowed_subnet\": Expected format is dotted decimal, i.e. 10.0.0.1";


        bool enable = cfg.get("enable")->asBool();
        const String &private_key = cfg.get("private_key")->asString();
        String result = check_key(private_key, enable);
        if (!result.isEmpty())
            return "\"private_key\"" + result;

        const String &remote_public_key = cfg.get("remote_public_key")->asString();
        result = check_key(remote_public_key, enable);
        if (!result.isEmpty())
            return "\"remote_public_key\"" + result;

        const String &preshared_key = cfg.get("preshared_key")->asString();

        // pass false because an empty preshared_key is always allowed.
        result = check_key(preshared_key, false);
        if (!result.isEmpty())
            return "\"preshared_key\"" + result;

        return "";
    }};

    state = Config::Object({
        {"connection_start", Config::Uint(0)},
        {"connection_end", Config::Uint(0)},
        {"state", Config::Uint(0)} // 0 not configured, 1 waiting for time sync 2 not connected 3 connected
    });
}

void Wireguard::start_wireguard()
{
    static bool done = false;
    if (done)
        return;

    struct timeval tv;
    if (!rtc.clock_synced(&tv))
        return;

    done = true;

    IPAddress internal_ip;
    IPAddress internal_subnet;
    IPAddress internal_gateway;
    IPAddress allowed_ip;
    IPAddress allowed_subnet;
    uint16_t local_port;

    internal_ip.fromString(config.get("internal_ip")->asEphemeralCStr());
    internal_subnet.fromString(config.get("internal_subnet")->asEphemeralCStr());
    internal_gateway.fromString(config.get("internal_gateway")->asEphemeralCStr());
    allowed_ip.fromString(config.get("allowed_ip")->asEphemeralCStr());
    allowed_subnet.fromString(config.get("allowed_subnet")->asEphemeralCStr());
    local_port = config.get("local_port")->asUint();

    private_key = config.get("private_key")->asString(); // Local copy of ephemeral conf String. The network interface created by WG might hold a reference to the C string.
    remote_host = config.get("remote_host")->asString(); // Local copy of ephemeral conf String. lwip_getaddrinfo() might hold a reference to the C string.

    logger.printfln("Got NTP sync. Connecting to WireGuard peer %s:%u", remote_host.c_str(), config.get("remote_port")->asUint());

    wg.begin(internal_ip,
             internal_subnet,
             local_port,
             internal_gateway,
             private_key.c_str(),
             remote_host.c_str(),
             config.get("remote_public_key")->asEphemeralCStr(),
             config.get("remote_port")->asUint(),
             allowed_ip,
             allowed_subnet,
             config.get("make_default_interface")->asBool(),
             config.get("preshared_key")->asString().length() > 0 ? config.get("preshared_key")->asEphemeralCStr() : nullptr);

    task_scheduler.scheduleWithFixedDelay([this]() {
        bool up = wg.is_peer_up(nullptr, nullptr);

        if(state.get("state")->updateUint(up ? 3 : 2))
        {
            if (up) {
                logger.printfln("Wireguard connection established");
                last_connected_ms = millis();
                state.get("connection_start")->updateUint(last_connected_ms);
            } else {
                uint32_t now = millis();
                uint32_t connected_for = millis() - this->last_connected_ms;
                state.get("connection_end")->updateUint(now);
                if (connected_for < 0x7FFFFFFF) {
                    logger.printfln("Wireguard connection lost. Was connected for %u seconds.", connected_for / 1000);
                } else {
                    logger.printfln("Wireguard connection lost. Was connected for a long time.");
                }
            }
        }
    }, 1_s, 1_s);

    state.get("state")->updateUint(2);
}

void Wireguard::setup()
{
    api.restorePersistentConfig("wireguard/config", &config);

    initialized = true;

    if (!config.get("enable")->asBool())
        return;

    logger.printfln("WireGuard enabled. Waiting for NTP time sync.");

    state.get("state")->updateUint(1);

    task_scheduler.scheduleWithFixedDelay([this](){
        start_wireguard();
    }, 1_s, 1_s);
}

void Wireguard::register_urls()
{
    api.addPersistentConfig("wireguard/config", &config, {
        "private_key",
        "remote_public_key",
        "preshared_key"
    });

    api.addState("wireguard/state", &state);
}

bool Wireguard::port_used(uint32_t port) {
    uint32_t self_port = config.get("local_port")->asUint();
    return port == self_port;
}
