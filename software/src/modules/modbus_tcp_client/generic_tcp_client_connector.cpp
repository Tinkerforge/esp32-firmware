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

#include "generic_tcp_client_connector.h"

#include "gcc_warnings.h"

void GenericTCPClientConnector::connect_internal()
{
    client->connect(host_name.c_str(), port,
    [this](TFGenericTCPClientConnectResult result, int error_number) {
        if (result == TFGenericTCPClientConnectResult::Connected) {
            connected_client = shared_client;
        }

        connect_callback_common(result, error_number);
    },
    [this](TFGenericTCPClientDisconnectReason reason, int error_number) {
        connected_client = nullptr;

        disconnect_callback_common(reason, error_number);
    });
}

void GenericTCPClientConnector::disconnect_internal()
{
    if (connected_client != nullptr) {
        client->disconnect();
        connected_client = nullptr;
    }
}
