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

#include "ship_rx_queue.h"

ShipRxQueue::PushResult ShipRxQueue::push(RxEvent &&event)
{
    std::lock_guard<std::mutex> lock{mutex};
    if (count < QUEUE_SIZE && bytes + event.data_len <= MAX_BYTES) {
        const PushResult result = count == 0 ? PushResult::PushedToEmpty : PushResult::Pushed;
        bytes += event.data_len;
        events[(head + count) % QUEUE_SIZE] = std::move(event);
        count++;
        return result;
    }
    return PushResult::Dropped;
}

bool ShipRxQueue::pop(RxEvent *event)
{
    std::lock_guard<std::mutex> lock{mutex};
    if (count == 0) {
        return false;
    }
    *event = std::move(events[head]);
    head = (head + 1) % QUEUE_SIZE;
    count--;
    bytes -= event->data_len;
    return true;
}
