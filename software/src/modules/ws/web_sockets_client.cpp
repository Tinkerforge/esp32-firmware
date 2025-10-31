/* esp32-firmware
 * Copyright (C) 2025 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "module_dependencies.h"
#include "web_sockets.h"
#include "web_sockets_client.h"

WebSocketsClient::~WebSocketsClient()
{
    if (ctx != nullptr) {
        free(ctx);
        ctx = nullptr;
    }
}

bool WebSocketsClient::sendOwnedNoFreeBlocking_HTTPThread(char *payload, size_t payload_len, httpd_ws_type_t ws_type)
{
    ws_work_item wi{{this->fd, -1, -1, -1, -1}, payload, payload_len, ws_type};
    bool result = ws->send_ws_work_item(&wi);
    return result;
}

void WebSocketsClient::close_HTTPThread()
{
    ws->keepAliveCloseDead(fd);
}

void *WebSocketsClient::setCtx(void *ctx_)
{
    void *old_ctx = ctx;
    ctx = ctx_;
    return old_ctx;
}
