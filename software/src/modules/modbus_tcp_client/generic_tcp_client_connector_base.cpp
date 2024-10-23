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

void GenericTCPClientConnectorBase::force_reconnect()
{
    stop_connection();
    start_connection();
}

void GenericTCPClientConnectorBase::connect_callback_common(TFGenericTCPClientConnectResult result, int error_number)
{
    if (result == TFGenericTCPClientConnectResult::Connected) {
        logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                 "Connected to %s:%u", host_name.c_str(), port);

        connect_backoff = 1_s;
        last_connect_result = TFGenericTCPClientConnectResult::Connected;
        last_connect_error_number = 0;
        resolve_error_printed = false;

        connect_callback();
    }
    else {
        if (last_connect_result != result || last_connect_error_number != error_number) {
            if (result == TFGenericTCPClientConnectResult::ResolveFailed) {
                if (error_number == EINVAL) {
                    logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                            "Could not resolve hostname %s, no DNS server available", host_name.c_str());
                }
                else if (error_number >= 0) {
                    logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                            "Could not resolve hostname %s: %s (%d)",
                                            host_name.c_str(),
                                            strerror(error_number), error_number);
                }
                else {
                    logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                            "Could not resolve hostname %s",
                                            host_name.c_str());
                }
            }
            else if (error_number >= 0) {
                logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                         "Could not connect to %s:%u: %s / %s (%d)",
                                         host_name.c_str(), port,
                                         get_tf_generic_tcp_client_connect_result_name(result),
                                         strerror(error_number), error_number);
            }
            else {
                logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                         "Could not connect to %s:%u: %s",
                                         host_name.c_str(), port,
                                         get_tf_generic_tcp_client_connect_result_name(result));
            }
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

void GenericTCPClientConnectorBase::disconnect_callback_common(TFGenericTCPClientDisconnectReason reason, int error_number)
{
    if (reason == TFGenericTCPClientDisconnectReason::Requested) {
        logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                 "Disconnected from %s:%u", host_name.c_str(), port);
    }
    else if (error_number >= 0) {
        logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                 "Disconnected from %s:%u: %s / %s (%d)",
                                 host_name.c_str(), port,
                                 get_tf_generic_tcp_client_disconnect_reason_name(reason),
                                 strerror(error_number), error_number);
    }
    else {
        logger.printfln_prefixed(event_log_prefix_override, event_log_prefix_override_len,
                                 "Disconnected from %s:%u: %s",
                                 host_name.c_str(), port,
                                 get_tf_generic_tcp_client_disconnect_reason_name(reason));
    }

    disconnect_callback();

    if (keep_connected) {
        task_scheduler.scheduleOnce([this]() {
            connect_internal();
        });
    }
}
