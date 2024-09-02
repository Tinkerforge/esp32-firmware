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

class GenericTCPClientConnector
{
protected:
    GenericTCPClientConnector(const char *event_log_prefix_override_, TFGenericTCPClient *client_) :
        event_log_prefix_override(event_log_prefix_override_), event_log_prefix_override_len(strlen(event_log_prefix_override_)), client(client_) {}
    virtual ~GenericTCPClientConnector() = default;

    virtual void start_connection();
    virtual void stop_connection();

    void start_connection_common();
    void connect_callback_common(TFGenericTCPClientConnectResult result, int error_number);
    void disconnect_callback_common(TFGenericTCPClientDisconnectReason reason, int error_number);

    String host_name;
    uint16_t port = 0;

    TFGenericTCPClient *client_ptr = nullptr;

private:
    virtual void connect_callback() = 0;
    virtual void disconnect_callback() = 0;

    const char *event_log_prefix_override;
    size_t event_log_prefix_override_len;
    TFGenericTCPClient *client;
    uint32_t connect_backoff_ms = 1000;
    TFGenericTCPClientConnectResult last_connect_result = TFGenericTCPClientConnectResult::Connected;
    int last_connect_error_number = 0;
    bool resolve_error_printed = false;
};
