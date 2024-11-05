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

#include <Arduino.h>
#include <TFGenericTCPClient.h>

#include "tools.h"

class GenericTCPClientConnectorBase
{
protected:
    GenericTCPClientConnectorBase(const char *event_log_prefix_override_) :
        event_log_prefix_override(event_log_prefix_override_),
        event_log_prefix_override_len(strlen(event_log_prefix_override_)) {}
    virtual ~GenericTCPClientConnectorBase() {}

    void start_connection();
    void stop_connection();
    void force_reconnect();

    void connect_callback_common(TFGenericTCPClientConnectResult result, int error_number);
    void disconnect_callback_common(TFGenericTCPClientDisconnectReason reason, int error_number);

    virtual void connect_callback() = 0;
    virtual void disconnect_callback() = 0;

    const char *event_log_prefix_override;
    size_t event_log_prefix_override_len;
    String host_name;
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
