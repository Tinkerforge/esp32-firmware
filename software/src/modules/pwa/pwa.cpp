/* esp32-firmware
 * Copyright (C) 2023 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "pwa.h"

#include "module_dependencies.h"
#include "manifest.embedded.h"

void Pwa::register_urls()
{
    server.on("/manifest.json", HTTP_GET, [](WebServerRequest request) {
        String response = device_name.display_name.get("display_name")->asString();
        response += "\"}";
        request.beginChunkedResponse(200, "application/json");
        request.sendChunk(manifest_data, manifest_length);
        request.sendChunk(response.c_str(), response.length());
        return request.endChunkedResponse();
    });
}
