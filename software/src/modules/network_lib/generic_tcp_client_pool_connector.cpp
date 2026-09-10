/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include "generic_tcp_client_pool_connector.h"

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"

#include "gcc_warnings.h"

void GenericTCPClientPoolConnector::format_connect_error(TFGenericTCPClientConnectResult result, int error_number, TFGenericTCPClientPoolShareLevel share_level,
                                                         const char *host, uint16_t port, char *buf, size_t buf_len, Language language /*= Language::English*/)
{
    const char *shared;

    switch (share_level) {
    case TFGenericTCPClientPoolShareLevel::Undefined:
    case TFGenericTCPClientPoolShareLevel::Primary:
        shared = "";
        break;

    case TFGenericTCPClientPoolShareLevel::Secondary:
        shared = language == Language::English ? " (shared connection)" : " (geteilte Verbindung)";
        break;

    default:
        shared = language == Language::English ? " (<unknown>)" : " (<unbekannt>)";
        break;
    }

    if (result == TFGenericTCPClientConnectResult::Connected) {
        if (buf_len > 0) {
            *buf = '\0';
        }
    }
    else if (result == TFGenericTCPClientConnectResult::ResolveFailed) {
        if (error_number == EINVAL) {
            snprintf(buf, buf_len,
                     language == Language::English
                     ? "Could not resolve hostname %s, no DNS server available%s"
                     : "Konnte Hostname %s nicht auflösen, kein DNS Server verfügbar%s",
                     host, shared);
        }
        else if (error_number >= 0) {
            snprintf(buf, buf_len,
                     language == Language::English
                     ? "Could not resolve hostname %s%s: %s (%d)"
                     : "Konnte Hostname %s nicht auflösen%s: %s (%d)",
                     host, shared,
                     strerror(error_number), error_number);
        }
        else {
            snprintf(buf, buf_len,
                     language == Language::English
                     ? "Could not resolve hostname %s%s"
                     : "Konnte Hostname %s nicht auflösen%s",
                     host, shared);
        }
    }
    else if (error_number >= 0) {
        snprintf(buf, buf_len,
                 language == Language::English
                 ? "Could not connect to %s:%u%s: %s / %s (%d)"
                 : "Konnte nicht zu %s:%u verbinden%s: %s / %s (%d)",
                 host, port, shared,
                 get_tf_generic_tcp_client_connect_result_name(result),
                 strerror(error_number), error_number);
    }
    else {
        snprintf(buf, buf_len,
                 language == Language::English
                 ? "Could not connect to %s:%u%s: %s"
                 : "Konnte nicht zu %s:%u verbinden%s: %s",
                 host, port, shared,
                 get_tf_generic_tcp_client_connect_result_name(result));
    }
}

void GenericTCPClientPoolConnector::format_disconnect_reason(TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPClientPoolShareLevel share_level,
                                                             const char *host, uint16_t port, char *buf, size_t buf_len, Language language /*= Language::English*/)
{
    const char *shared;

    switch (share_level) {
    case TFGenericTCPClientPoolShareLevel::Undefined:
    case TFGenericTCPClientPoolShareLevel::Primary:
        shared = "";
        break;

    case TFGenericTCPClientPoolShareLevel::Secondary:
        shared = language == Language::English ? " (shared connection)" : " (geteilte Verbindung)";
        break;

    default:
        shared = language == Language::English ? " (<unknown>)" : " (<unbekannt>)";
        break;
    }

    if (reason == TFGenericTCPClientDisconnectReason::Requested) {
        snprintf(buf, buf_len,
                 language == Language::English
                 ? "Disconnected from %s:%u%s"
                 : "Verbindung zu %s:%u getrennt%s",
                 host, port, shared);
    }
    else if (reason == TFGenericTCPClientDisconnectReason::Forced) {
        snprintf(buf, buf_len,
                 language == Language::English
                 ? "Disconnected from %s:%u by force%s"
                 : "Verbindung zu %s:%u durch Zwang getrennt%s",
                 host, port, shared);
    }
    else if (error_number >= 0) {
        snprintf(buf, buf_len,
                 language == Language::English
                 ? "Disconnected from %s:%u%s: %s / %s (%d)"
                 : "Verbindung zu %s:%u getrennt%s: %s / %s (%d)",
                 host, port, shared,
                 get_tf_generic_tcp_client_disconnect_reason_name(reason),
                 strerror(error_number), error_number);
    }
    else {
        snprintf(buf, buf_len,
                 language == Language::English
                 ? "Disconnected from %s:%u%s: %s"
                 : "Verbindung zu %s:%u getrennt%s: %s",
                 host, port, shared,
                 get_tf_generic_tcp_client_disconnect_reason_name(reason));
    }
}

void GenericTCPClientPoolConnector::start_connection()
{
    if (keep_connected) {
        return;
    }

    keep_connected = true;

    connect(0_ms);
}

void GenericTCPClientPoolConnector::stop_connection()
{
    if (!keep_connected) {
        return;
    }

    keep_connected = false;

    disconnect(false);
}

void GenericTCPClientPoolConnector::force_reconnect()
{
    if (!keep_connected) {
        return;
    }

    disconnect(true);
}

void GenericTCPClientPoolConnector::connect_helper()
{
    if (shared_client != nullptr) {
        return;
    }

    pool->acquire(host.c_str(), port, &shared_client,
    [this](TFGenericTCPClientConnectResult result, int error_number, TFGenericTCPSharedClient *shared_client_, TFGenericTCPClientPoolShareLevel share_level) {
        if (result == TFGenericTCPClientConnectResult::NonReentrant) {
            esp_system_abort("TFGenericTCPClientPool acquire was called in non-reentrant context");
        }

        shared_client = shared_client_;

        bool report_result = false;

        if (result == TFGenericTCPClientConnectResult::Connected) {
            const char *shared;

            switch (share_level) {
            case TFGenericTCPClientPoolShareLevel::Undefined:
            case TFGenericTCPClientPoolShareLevel::Primary:
                shared = "Connected to";
                break;

            case TFGenericTCPClientPoolShareLevel::Secondary:
                shared = "Shared existing connection to";
                break;

            default:
                shared = "<Unknown>";
                break;
            }

            logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                     "%s%s %s:%u",
                                     event_log_message_prefix,
                                     shared, host.c_str(), port);

            report_result = true;
            connect_backoff = 1_s;
            last_connect_result = TFGenericTCPClientConnectResult::Connected;
            last_connect_error_number = 0;
            resolve_error_printed = false;
        }
        else {
            if (last_connect_result != result || last_connect_error_number != error_number) {
                char buf[256] = "";

                format_connect_error(result, error_number, share_level, host.c_str(), port, buf, sizeof(buf));
                logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len, "%s%s", event_log_message_prefix, buf);

                report_result = true;
            }

            if (result == TFGenericTCPClientConnectResult::ResolveFailed) {
                if (keep_connected) {
                    connect(10_s);
                }
            }
            else {
                if (keep_connected) {
                    connect(connect_backoff);
                }

                connect_backoff += connect_backoff;

                if (connect_backoff > 16_s) {
                    connect_backoff = 16_s;
                }
            }
        }

        if (report_result) {
            connect_callback(result, share_level);
        }

        last_connect_result = result;
        last_connect_error_number = error_number;
    },
    [this](TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPSharedClient *shared_client_, TFGenericTCPClientPoolShareLevel share_level) {
        shared_client = nullptr;

        char buf[256] = "";

        format_disconnect_reason(reason, error_number, share_level, host.c_str(), port, buf, sizeof(buf));
        logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len, "%s%s", event_log_message_prefix, buf);

        disconnect_callback(reason, share_level);

        if (keep_connected) {
            connect(5_s);
        }
    });
}

void GenericTCPClientPoolConnector::connect(millis_t delay)
{
    task_scheduler.cancel(connect_task_id);
    connect_task_id = 0;

    if (delay > 0_ms) {
        connect_task_id = task_scheduler.scheduleOnce([this]() {
            connect_task_id = 0;

            connect_helper();
        }, delay);
    }
    else {
        connect_helper();
    }
}

void GenericTCPClientPoolConnector::disconnect(bool force)
{
    task_scheduler.cancel(connect_task_id);
    connect_task_id = 0;

    if (shared_client == nullptr) {
        return;
    }

    switch (pool->release(shared_client, force)) {
    case TFGenericTCPClientDisconnectResult::NonReentrant:
        esp_system_abort("TFGenericTCPClientPool release was called in non-reentrant context");

    case TFGenericTCPClientDisconnectResult::NotConnected:
        esp_system_abort("TFGenericTCPClientPool release was called while not connected");

    case TFGenericTCPClientDisconnectResult::Disconnected:
        break;

    default:
        esp_system_abort("TFGenericTCPClientPool release returned unknown result");
    }
}
