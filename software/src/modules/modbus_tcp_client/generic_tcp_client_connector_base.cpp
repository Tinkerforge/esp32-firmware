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

#define EVENT_LOG_PREFIX "modbus_tcp_clnt"

#include "generic_tcp_client_connector_base.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

#include "gcc_warnings.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wsuggest-final-methods"
#endif

void GenericTCPClientConnectorBase::start_connection()
{
    keep_connected = true;

    connect_internal();
}

void GenericTCPClientConnectorBase::stop_connection()
{
    keep_connected = false;

    disconnect_internal();
}

void GenericTCPClientConnectorBase::connect_callback_common(TFGenericTCPClientConnectResult result, int error_number, TFGenericTCPClientPoolShareLevel share_level)
{
    if (result == TFGenericTCPClientConnectResult::Connected) {
        const char *shared;

        switch (share_level) {
        case  TFGenericTCPClientPoolShareLevel::Undefined:
        case  TFGenericTCPClientPoolShareLevel::Primary:
            shared = "Connected to";
            break;

        case  TFGenericTCPClientPoolShareLevel::Secondary:
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

        connect_backoff = 1_s;
        last_connect_result = TFGenericTCPClientConnectResult::Connected;
        last_connect_error_number = 0;
        resolve_error_printed = false;

        connect_callback();
    }
    else {
        if (last_connect_result != result || last_connect_error_number != error_number) {
            char buf[256] = "";

            format_connect_error(result, error_number, share_level, host.c_str(), port, buf, sizeof(buf));
            logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len, "%s%s", event_log_message_prefix, buf);
        }

        if (result == TFGenericTCPClientConnectResult::ResolveFailed) {
            task_scheduler.scheduleOnce([this]() {
                if (keep_connected) {
                    connect_internal();
                }
            }, 10_s);
        }
        else {
            task_scheduler.scheduleOnce([this]() {
                if (keep_connected) {
                    connect_internal();
                }
            }, connect_backoff);

            connect_backoff += connect_backoff;

            if (connect_backoff > 16_s) {
                connect_backoff = 16_s;
            }
        }
    }

    last_connect_result = result;
    last_connect_error_number = error_number;
}

void GenericTCPClientConnectorBase::disconnect_callback_common(TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPClientPoolShareLevel share_level)
{
    char buf[256] = "";

    format_disconnect_reason(reason, error_number, share_level, host.c_str(), port, buf, sizeof(buf));
    logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len, "%s%s", event_log_message_prefix, buf);

    disconnect_callback();

    if (keep_connected) {
        task_scheduler.scheduleOnce([this]() {
            connect_internal();
        }, 5_s);
    }
}

void GenericTCPClientConnectorBase::format_connect_error(TFGenericTCPClientConnectResult result, int error_number, TFGenericTCPClientPoolShareLevel share_level,
                                                         const char *host, uint16_t port, char *buf, size_t buf_len)
{
    const char *shared;

    switch (share_level) {
    case  TFGenericTCPClientPoolShareLevel::Undefined:
    case  TFGenericTCPClientPoolShareLevel::Primary:
        shared = "";
        break;

    case  TFGenericTCPClientPoolShareLevel::Secondary:
        shared = " (shared connection)";
        break;

    default:
        shared = " <unknown>";
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
                     "Could not resolve hostname %s, no DNS server available%s",
                     host, shared);
        }
        else if (error_number >= 0) {
            snprintf(buf, buf_len,
                     "Could not resolve hostname %s%s: %s (%d)",
                     host, shared,
                     strerror(error_number), error_number);
        }
        else {
            snprintf(buf, buf_len,
                     "Could not resolve hostname %s%s",
                     host, shared);
        }
    }
    else if (error_number >= 0) {
        snprintf(buf, buf_len,
                 "Could not connect to %s:%u%s: %s / %s (%d)",
                 host, port, shared,
                 get_tf_generic_tcp_client_connect_result_name(result),
                 strerror(error_number), error_number);
    }
    else {
        snprintf(buf, buf_len,
                 "Could not connect to %s:%u%s: %s",
                 host, port, shared,
                 get_tf_generic_tcp_client_connect_result_name(result));
    }
}

void GenericTCPClientConnectorBase::format_disconnect_reason(TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPClientPoolShareLevel share_level,
                                                             const char *host, uint16_t port, char *buf, size_t buf_len)
{
    const char *shared;

    switch (share_level) {
    case  TFGenericTCPClientPoolShareLevel::Undefined:
    case  TFGenericTCPClientPoolShareLevel::Primary:
        shared = "Disconnected from";
        break;

    case  TFGenericTCPClientPoolShareLevel::Secondary:
        shared = "Unshared existing connection to";
        break;

    default:
        shared = "<Unknown>";
        break;
    }

    if (reason == TFGenericTCPClientDisconnectReason::Requested) {
        snprintf(buf, buf_len,
                 "%s %s:%u",
                 shared, host, port);
    }
    else if (error_number >= 0) {
        snprintf(buf, buf_len,
                 "%s %s:%u: %s / %s (%d)",
                 shared, host, port,
                 get_tf_generic_tcp_client_disconnect_reason_name(reason),
                 strerror(error_number), error_number);
    }
    else {
        snprintf(buf, buf_len,
                 "%s %s:%u: %s",
                 shared, host, port,
                 get_tf_generic_tcp_client_disconnect_reason_name(reason));
    }
}
