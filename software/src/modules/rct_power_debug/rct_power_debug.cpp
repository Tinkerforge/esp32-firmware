/* esp32-firmware
 * Copyright (C) 2025 Matthias Bolte <matthias@tinkerforge.com>
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

#include "rct_power_debug.h"

#include <stdarg.h>
#include <TFJson.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "modules/network_lib/generic_tcp_client_connector_base.h"

#include "gcc_warnings.h"

void RCTPowerDebug::pre_setup()
{
    read_config = Config::Object({
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(8899)},
        {"id", Config::Uint32(0)},
        {"timeout", Config::Uint32(2000)},
        {"cookie", Config::Uint32(0)},
    });
}

[[gnu::format(__printf__, 2, 3)]] static void report_errorf(uint32_t cookie, const char *fmt, ...);
static void report_errorf(uint32_t cookie, const char *fmt, ...)
{
    va_list args;
    char buf[256];
    TFJsonSerializer json{buf, sizeof(buf)};

    json.addObject();
    json.addMemberNumber("cookie", cookie);
    va_start(args, fmt);
    json.addMemberStringVF("error", fmt, args);
    json.addMemberNull("value");
    va_end(args);
    json.endObject();
    json.end();

    ws.pushRawStateUpdate(buf, "rct_power_debug/read_result");
}

static void report_result(uint32_t cookie, float value)
{
    char buf[256];
    TFJsonSerializer json{buf, sizeof(buf)};

    json.addObject();
    json.addMemberNumber("cookie", cookie);
    json.addMemberNull("error");
    json.addMemberNumber("value", value);
    json.endObject();
    json.end();

    ws.pushRawStateUpdate(buf, "rct_power_debug/read_result");
}

void RCTPowerDebug::register_urls()
{
    api.addCommand("rct_power_debug/read", &read_config, {}, [this](Language /*language*/, String &errmsg) {
        uint32_t cookie = read_config.get("cookie")->asUint();

        if (client != nullptr) {
            report_errorf(cookie, "Another read is already in progress");
            return;
        }

        const String &host = read_config.get("host")->asString();
        uint16_t port = read_config.get("port")->asUint16();
        uint32_t id = read_config.get("id")->asUint();
        millis_t timeout = millis_t{read_config.get("timeout")->asUint()};

        defer {
            // When done parsing the transaction, drop Strings from config to free memory.
            // This invalidates the "host" reference above, which will be copied by the
            // lambda before being cleared.
            read_config.get("host")->clearString();
        };

        rct_power_client.get_pool()->acquire(host.c_str(), port,
        [this, cookie, host, port, id, timeout](TFGenericTCPClientConnectResult connect_result, int error_number, TFGenericTCPSharedClient *shared_client, TFGenericTCPClientPoolShareLevel share_level) {
            if (connect_result != TFGenericTCPClientConnectResult::Connected) {
                char connect_error[256] = "";

                GenericTCPClientConnectorBase::format_connect_error(connect_result, error_number, share_level, host.c_str(), port, connect_error, sizeof(connect_error));
                report_errorf(cookie, "%s", connect_error);
                return;
            }

            client = shared_client;

            static_cast<TFRCTPowerSharedClient *>(client)->read(id, timeout,
            [this, cookie](TFRCTPowerClientTransactionResult result, float value) {
                if (result != TFRCTPowerClientTransactionResult::Success) {
                    report_errorf(cookie, "Read failed: %s (%d)",
                                  get_tf_rct_power_client_transaction_result_name(result),
                                  static_cast<int>(result));

                    release_client();
                    return;
                }

                report_result(cookie, value);
                release_client();
            });
        },
        [this](TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPSharedClient *shared_client, TFGenericTCPClientPoolShareLevel share_level) {
            client = nullptr;
        });
    }, true);
}

void RCTPowerDebug::release_client()
{
    task_scheduler.scheduleOnce([this]() {
        if (client != nullptr) {
            rct_power_client.get_pool()->release(client);
        }
    });
}
