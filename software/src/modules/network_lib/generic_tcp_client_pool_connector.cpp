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

#include "gcc_warnings.h"

void GenericTCPClientPoolConnector::force_reconnect()
{
    disconnect_internal(true);
}

void GenericTCPClientPoolConnector::connect_internal()
{
    if (connected_client != nullptr) {
        return;
    }

    pool->acquire(host.c_str(), port,
    [this](TFGenericTCPClientConnectResult result, int error_number, TFGenericTCPSharedClient *shared_client, TFGenericTCPClientPoolShareLevel share_level) {
        if (result == TFGenericTCPClientConnectResult::NonReentrant) {
            esp_system_abort("TFGenericTCPClientPool acquire was called in non-reentrant context");
        }

        if (result == TFGenericTCPClientConnectResult::Connected) {
            connected_client = shared_client;
        }

        connect_callback_common(result, error_number, share_level);
    },
    [this](TFGenericTCPClientDisconnectReason reason, int error_number, TFGenericTCPSharedClient *shared_client, TFGenericTCPClientPoolShareLevel share_level) {
        connected_client = nullptr;

        disconnect_callback_common(reason, error_number, share_level);
    });
}

void GenericTCPClientPoolConnector::disconnect_internal()
{
    disconnect_internal(false);
}

void GenericTCPClientPoolConnector::disconnect_internal(bool force_disconnect)
{
    if (connected_client == nullptr) {
        return;
    }

    switch (pool->release(connected_client, force_disconnect)) {
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
