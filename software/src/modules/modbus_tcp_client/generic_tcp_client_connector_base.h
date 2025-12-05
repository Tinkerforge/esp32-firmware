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

#pragma once

#include <WString.h>
#include <TFGenericTCPClient.h>
#include <TFGenericTCPClientPool.h>

#include "tools.h"

class GenericTCPClientConnectorBase
{
public:
    static void format_connect_error(TFGenericTCPClientConnectResult result, int error_number, TFGenericTCPClientPoolShareLevel share_level, const char *host, uint16_t port, char *buf, size_t buf_len);
    static void format_disconnect_reason(TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPClientPoolShareLevel share_level, const char *host, uint16_t port, char *buf, size_t buf_len);

protected:
    GenericTCPClientConnectorBase(const char *event_log_prefix_override_, const char *event_log_message_prefix_) :
        event_log_prefix_override(event_log_prefix_override_),
        event_log_prefix_override_len(strlen(event_log_prefix_override_)),
        event_log_message_prefix(event_log_message_prefix_) {}
    virtual ~GenericTCPClientConnectorBase() {}

    void start_connection();
    void stop_connection();

    void connect_callback_common(TFGenericTCPClientConnectResult result, int error_number, TFGenericTCPClientPoolShareLevel share_level);
    void disconnect_callback_common(TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPClientPoolShareLevel share_level);

    virtual void connect_callback() = 0;
    virtual void disconnect_callback() = 0;
    virtual void force_reconnect() = 0;

    const char *event_log_prefix_override;
    size_t event_log_prefix_override_len;
    const char *event_log_message_prefix;
    String host;
    uint16_t port = 0;
    TFGenericTCPSharedClient *connected_client = nullptr;

private:
    virtual void connect_internal() = 0;
    virtual void disconnect_internal() = 0;
    bool keep_connected = false;
    millis_t connect_backoff = 1_s;
    TFGenericTCPClientConnectResult last_connect_result = TFGenericTCPClientConnectResult::Connected;
    int last_connect_error_number = 0;
    bool resolve_error_printed = false;
};
