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
#include "esp_system.h"
#include "LittleFS.h"

#include "api.h"
#include "task_scheduler.h"

#include "gcc_warnings.h"

#define BENCHMARK_BLOCKSIZE 32768

static float benchmark_area(uint8_t *start_address, size_t max_length)
{
    uint8_t *buffer = static_cast<uint8_t *>(heap_caps_malloc(BENCHMARK_BLOCKSIZE, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL));
    if (!buffer) {
        logger.printfln("debug: Can't malloc %i bytes for benchmark buffer.", BENCHMARK_BLOCKSIZE);
        return 0;
    }

    size_t blocks = max_length / BENCHMARK_BLOCKSIZE;
    size_t test_length = blocks * BENCHMARK_BLOCKSIZE;

    micros_t start_time = now_us();
    while (blocks > 0) {
        memcpy(buffer, start_address, BENCHMARK_BLOCKSIZE);
        start_address += BENCHMARK_BLOCKSIZE;
        blocks--;
    }
    micros_t runtime = now_us() - start_time;
    uint32_t runtime32 = static_cast<uint32_t>(static_cast<int64_t>(runtime));
    float runtime_f = static_cast<float>(runtime32);
    float test_length_f = static_cast<float>(test_length);
    float speed_MiBps = (test_length_f * 1000000.0F) / (runtime_f * 1024 * 1024);

    free(buffer);

    return speed_MiBps;
}

extern uint8_t _text_start;

void Debug::pre_setup()
{
    size_t internal_heap_size = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    size_t dram_heap_size     = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    size_t iram_heap_size     = internal_heap_size - dram_heap_size;
    size_t psram_heap_size    = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);

    size_t psram_size = 0;
#if defined(BOARD_HAS_PSRAM)
    psram_size = 4 * 1024 * 1024;
#endif

    String flash_mode;
    switch (esp_flash_default_chip->read_mode) {
        case SPI_FLASH_SLOWRD:  flash_mode = "slowrd";  break;
        case SPI_FLASH_FASTRD:  flash_mode = "fastrd";  break;
        case SPI_FLASH_DOUT:    flash_mode = "dout";    break;
        case SPI_FLASH_DIO:     flash_mode = "dio";     break;
        case SPI_FLASH_QOUT:    flash_mode = "qout";    break;
        case SPI_FLASH_QIO:     flash_mode = "qio";     break;
        case SPI_FLASH_OPI_STR: flash_mode = "opi_str"; break;
        case SPI_FLASH_OPI_DTR: flash_mode = "opi_dtr"; break;
        case SPI_FLASH_READ_MODE_MAX:
        default: flash_mode = static_cast<int>(esp_flash_default_chip->read_mode);
    }

    float psram_speed = 0;
#if defined(BOARD_HAS_PSRAM)
    psram_speed = benchmark_area(reinterpret_cast<uint8_t *>(0x3FB00000), 128*1024); // 128KiB inside the fourth MiB
#endif

    float flash_speed = benchmark_area(&_text_start, 128*1024); // 128KiB at the beginning of the code

    state_fast = Config::Object({
        {"uptime",     Config::Uint32(0)},
        {"free_dram",  Config::Uint32(0)},
        {"free_iram",  Config::Uint32(0)},
        {"free_psram", Config::Uint32(0)},
        {"heap_check_time_avg", Config::Uint32(0)},
        {"heap_check_time_max", Config::Uint32(0)},
        {"cpu_usage",  Config::Float(0)},
    });

    state_slow = Config::Object({
        {"largest_free_dram_block",  Config::Uint32(0)},
        {"largest_free_psram_block", Config::Uint32(0)},
        {"heap_dram",  Config::Uint32(dram_heap_size)},
        {"heap_iram",  Config::Uint32(iram_heap_size)},
        {"heap_psram", Config::Uint32(psram_heap_size)},
        {"psram_size", Config::Uint32(psram_size)},
        {"heap_integrity_ok", Config::Bool(true)},
        {"main_stack_hwm", Config::Uint32(0)},
        {"flash_mode", Config::Str(flash_mode, 0, 8)},
        {"flash_benchmark", Config::Float(flash_speed)},
        {"psram_benchmark", Config::Float(psram_speed)},
    });
}

void Debug::setup()
{
    task_scheduler.scheduleWithFixedDelay([this](){
        size_t free_internal = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

        multi_heap_info_t dram_info;
        multi_heap_info_t psram_info;
        heap_caps_get_info(&dram_info,  MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        heap_caps_get_info(&psram_info, MALLOC_CAP_SPIRAM);

        state_fast.get("uptime")->updateUint(millis());
        state_fast.get("free_dram")->updateUint(dram_info.total_free_bytes);
        state_fast.get("free_iram")->updateUint(free_internal - dram_info.total_free_bytes);
        state_fast.get("free_psram")->updateUint(psram_info.total_free_bytes);

        state_slow.get("largest_free_dram_block")->updateUint(dram_info.largest_free_block);
        state_slow.get("largest_free_psram_block")->updateUint(psram_info.largest_free_block);

        state_slow.get("main_stack_hwm")->updateUint(uxTaskGetStackHighWaterMark(nullptr));

        uint32_t runtime_avg;
        if (this->integrity_check_runs == 0) {
            runtime_avg = 0;
        } else {
            runtime_avg = this->integrity_check_runtime_sum / this->integrity_check_runs;
        }
        state_fast.get("heap_check_time_avg")->updateUint(runtime_avg);
        state_fast.get("heap_check_time_max")->updateUint(this->integrity_check_runtime_max);

        micros_t now = now_us();
        uint32_t time_since_last_update_us = static_cast<uint32_t>(static_cast<int64_t>(now - this->last_state_update));
        float heap_check_cpu_usage = static_cast<float>(this->integrity_check_runtime_sum) / static_cast<float>(time_since_last_update_us);
        state_fast.get("cpu_usage")->updateFloat(1 - heap_check_cpu_usage);
        this->last_state_update = now;

        this->integrity_check_runs = 0;
        this->integrity_check_runtime_sum = 0;
        this->integrity_check_runtime_max = 0;
    }, 1000, 1000);

    last_state_update = now_us();

    initialized = true;
}

void Debug::register_urls()
{
    api.addState("debug/state_fast", &state_fast);
    api.addState("debug/state_slow", &state_slow);

    server.on_HTTPThread("/debug/crash", HTTP_GET, [this](WebServerRequest req) {
        assert(0);
        return req.send(200, "text/plain", "ok");
    });
#ifdef DEBUG_FS_ENABLE
    server.on_HTTPThread("/debug/fs/*", HTTP_GET, [this](WebServerRequest request) {
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
                String s = "<a href=\"/debug/fs" + String(file.path()) + "\">"+ file.name() +"</a><button type=\"button\" onclick=\"fetch('/debug/fs" + String(file.path()) + "', {method: 'DELETE'})\">Delete</button><br>";
                request.sendChunk(s.c_str(), static_cast<ssize_t>(s.length()));
                file = f.openNextFile();
            }

            return request.endChunkedResponse();
        }
    });

    server.on_HTTPThread("/debug/fs/*", HTTP_DELETE, [this](WebServerRequest request) {
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

    server.on_HTTPThread("/debug/fs/*", HTTP_PUT, [this](WebServerRequest request) {
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

void Debug::loop()
{
    micros_t start = now_us();
    bool check_ok = heap_caps_check_integrity_all(integrity_check_print_errors);
    uint32_t runtime = static_cast<uint32_t>(static_cast<int64_t>(now_us() - start));

    integrity_check_runs++;
    integrity_check_runtime_sum += runtime;

    if (runtime > integrity_check_runtime_max) {
        integrity_check_runtime_max = runtime;
    }

    if (!check_ok) {
        state_slow.get("heap_integrity_ok")->updateBool(false);
        integrity_check_print_errors = false;
    }
}
