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

#ifdef DEBUG_FS_ENABLE
#include <freertos/task.h>
#endif

WebSocketsClient::~WebSocketsClient()
{
    if (ctx != nullptr) {
        free(ctx);
        ctx = nullptr;
    }
}

bool WebSocketsClient::sendOwnedNoFreeBlocking_HTTPThread(char *payload, size_t payload_len, httpd_ws_type_t ws_type)
{
#ifdef DEBUG_FS_ENABLE
    const char *task_name = pcTaskGetName(nullptr);

    if (strcmp(task_name, "httpd") != 0) {
        esp_system_abort("sendOwnedNoFreeBlocking_HTTPThread can only be called from the HTTP thread");
    }
#endif

    httpd_ws_frame ws_pkt = {
        .final      = true, // Doesn't matter, frame is not fragmented.
        .fragmented = false,
        .type       = ws_type,
        .payload    = reinterpret_cast<uint8_t *>(payload),
        .len        = payload_len,
    };

    return ws->send_ws_item_direct(fd, &ws_pkt);
}

void WebSocketsClient::close_HTTPThread()
{
    ws->keepAliveCloseDead_HTTPThread(fd);
}

void *WebSocketsClient::setCtx(void *ctx_)
{
    void *old_ctx = ctx;
    ctx = ctx_;
    return old_ctx;
}
