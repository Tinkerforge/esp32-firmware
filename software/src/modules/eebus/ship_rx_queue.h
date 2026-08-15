/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include <mutex>

#include "ship_connection.h"
#include "tools/malloc.h"

// Cross-task handoff queue for websocket events. The httpd task (server role) and the
// websocket client task (client role) push events. Ship dispatches them in FIFO order on the
// main task. This is the only cross-task synchronization point of the EEBus module.
// Everything else runs on the main task.
class ShipRxQueue
{
public:
    struct RxEvent {
        enum class Type : uint8_t {
            None,
            Connected,       // Server role: incoming websocket connection accepted
            ClientConnected, // Client role: outgoing websocket connection established
            Frame,           // Websocket frame received
            Disconnected,    // Websocket connection lost or peer sent close
        };

        Type type = Type::None;
        ShipConnection::Role role = ShipConnection::Role::Server;
        int fd = -1;                                      // Connection key, server role
        tf_websocket_client_handle_t ws_handle = nullptr; // Connection key, client role (never dereferenced)
        String peer_ip;                                   // Connected
        String tls_ski;                                   // Connected
        String reason;                                    // Disconnected
        unique_ptr_any<uint8_t> data;                     // Frame payload, PSRAM
        size_t data_len = 0;
        bool fin = false;                                 // Frame
    };

    enum class PushResult : uint8_t {
        Pushed,
        PushedToEmpty, // Pushed into a previously empty queue: the consumer needs a wakeup
        Dropped,       // Queue full
    };

    PushResult push(RxEvent &&event);
    bool pop(RxEvent *event);

private:
    static constexpr size_t QUEUE_SIZE = 16;
    static constexpr size_t MAX_BYTES = 128 * 1024;

    std::mutex mutex;
    RxEvent events[QUEUE_SIZE]; // Ring buffer, FIFO
    size_t head = 0;
    size_t count = 0;
    size_t bytes = 0;
};
