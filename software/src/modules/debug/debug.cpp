/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "debug.h"

#include <Arduino.h>
#include "LittleFS.h"

#include "esp_core_dump.h"

#include "api.h"
#include "tools.h"
#include "task_scheduler.h"

#include "gcc_warnings.h"

void Debug::pre_setup()
{
}

void Debug::setup()
{
    state = Config::Object({
        {"uptime", Config::Uint32(0)},
        {"free_heap", Config::Uint32(0)},
        {"largest_free_heap_block", Config::Uint32(0)},
        {"free_psram", Config::Uint32(0)},
        {"largest_free_psram_block", Config::Uint32(0)}
    });

    task_scheduler.scheduleWithFixedDelay([this](){
        state.get("uptime")->updateUint(millis());
        state.get("free_heap")->updateUint(heap_caps_get_free_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        state.get("largest_free_heap_block")->updateUint(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
        state.get("free_psram")->updateUint(heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        state.get("largest_free_psram_block")->updateUint(heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    }, 1000, 1000);

    initialized = true;
}

void Debug::register_urls()
{
    api.addState("debug/state", &state, {}, 1000);

    server.on("/debug/crash", HTTP_GET, [this](WebServerRequest req) {
        task_scheduler.scheduleOnce([]() {
            assert(0);
        }, 1000);
        return req.send(200, "text/plain", "ok");
    });
#ifdef DEBUG_FS_ENABLE
    server.on("/debug/fs/*", HTTP_GET, [this](WebServerRequest request) {
        String path = request.uri().substring(ARRAY_SIZE("/debug/fs") - 1);
        if (path.length() > 1 && path[path.length() - 1] == '/')
            path = path.substring(0, path.length() - 1);

        if (!LittleFS.exists(path))
            return request.send(404, "text/plain", ("File " + path + " not found").c_str());

        File f = LittleFS.open(path);
        if (!f.isDirectory()) {
            char buf[256];
            request.beginChunkedResponse(200, "text/plain");
            while(f.available()) {
                size_t read = f.read(reinterpret_cast<uint8_t *>(buf), ARRAY_SIZE(buf));
                request.sendChunk(buf, static_cast<ssize_t>(read));
            }
            return request.endChunkedResponse();
        } else {
            request.beginChunkedResponse(200, "text/html");
            String header = "<h1>" + String(f.path()) + "</h1><br>";
            request.sendChunk(header.c_str(), static_cast<ssize_t>(header.length()));

            if (path.length() > 1) {
                int idx = path.lastIndexOf('/');
                String up = "<a href=\"/debug/fs" + path.substring(0, static_cast<unsigned int>(idx + 1)) + "\">..</a><br>";
                request.sendChunk(up.c_str(), static_cast<ssize_t>(up.length()));
            }

            File file = f.openNextFile();
            while(file) {
                String s = "<a href=\"/debug/fs" + String(file.path()) + "\">"+ file.name() +"</a><br>";
                request.sendChunk(s.c_str(), static_cast<ssize_t>(s.length()));
                file = f.openNextFile();
            }

            return request.endChunkedResponse();
        }
    });

    server.on("/debug/fs/*", HTTP_DELETE, [this](WebServerRequest request) {
        String path = request.uri().substring(ARRAY_SIZE("/debug/fs") - 1);
        if (path.length() > 1 && path[path.length() - 1] == '/')
            path = path.substring(0, path.length() - 1);

        if (!LittleFS.exists(path))
            return request.send(404, "text/plain", ("File " + path + " not found").c_str());

        File f = LittleFS.open(path);
        if (!f.isDirectory()) {
            f.close();
            LittleFS.remove(path);
            return request.send(200, "text/plain", ("File " + path + " deleted").c_str());
        } else {
            f.close();
            remove_directory(path.c_str());
            return request.send(200, "text/plain", ("Directory " + path + " and all contents deleted").c_str());
        }
    });

    server.on("/debug/fs/*", HTTP_PUT, [this](WebServerRequest request) {
        String path = request.uri().substring(ARRAY_SIZE("/debug/fs") - 1);
        bool create_directory = path.length() > 1 && path[path.length() - 1] == '/';
        if (create_directory)
            path = path.substring(0, path.length() - 1);

        if (LittleFS.exists(path)) {
            File f = LittleFS.open(path);
            if (!f.isDirectory() && create_directory)
                return request.send(400, "text/plain", ("File " + path + " already exists and is not a directory").c_str());
            if (f.isDirectory() && !create_directory)
                return request.send(400, "text/plain", ("Directory " + path + " already exists").c_str());
            if (f.isDirectory())
                return request.send(200, "text/plain", ("Directory " + path + " already exists").c_str());
            else {
                f.close();
                LittleFS.remove(path);
            }
        }

        if (create_directory) {
            LittleFS.mkdir(path);
            return request.send(200, "text/plain", ("Directory " + path + " created").c_str());
        }

        File f = LittleFS.open(path, "w");
        char *payload = request.receive();
        f.write(reinterpret_cast<uint8_t *>(payload), request.contentLength());
        free(payload);
        return request.send(200, "text/plain", ("File " + path + " created.").c_str());
    });
#endif
}
