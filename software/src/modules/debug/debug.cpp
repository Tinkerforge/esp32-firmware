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

#include <esp_debug_helpers.h>
#include <esp_system.h>
#include <esp_task.h>
#include <esp_flash.h>
#include <LittleFS.h>
#include <lwipopts.h>
#include <soc/rtc.h>
#include <soc/spi_reg.h>
#include <freertos/task.h>

// lwip_socket_dbg_get_socket() in sockets_priv.h is not declared extern "C".
extern "C" {
#include <lwip/priv/sockets_priv.h>
}

#include "event_log_prefix.h"
#include "config/slot_allocator.h"
#include "module_dependencies.h"
#include "backtrace.h"
#include "tools/string_builder.h"
#include "tools/memory.h"
#include "tools/fs.h"

#include "gcc_warnings.h"

static float benchmark_area(uint32_t *start_address, size_t max_length);
static void get_spi_settings(uint32_t spi_num, uint32_t apb_clk, uint32_t *spi_clk, uint32_t *dummy_cyclelen, const char **spi_mode);

[[gnu::noinline]]
static void malloc_failed_log_detailed(size_t size, uint32_t caps, const char *function_name, const char *task_name)
{
    multi_heap_info_t ram_info;
    heap_caps_get_info(&ram_info, caps);

    char backtrace_buf[384];
    size_t backtrace_len = strn_backtrace(backtrace_buf, sizeof(backtrace_buf), 1);

    logger.printfln("malloc_failed_hook sz=%u frBl=%u frTot=%u caps=0x%lx fn=%s t=%s", size, ram_info.largest_free_block, ram_info.total_free_bytes, caps, function_name, task_name);
    logger.print_plain(backtrace_buf, backtrace_len);
    logger.print_plain("\n", 1);
}

// Called on affected task's stack, which might be small.
static void malloc_failed_hook(size_t size, uint32_t caps, const char *function_name)
{
    const char *task_name = pcTaskGetName(xTaskGetCurrentTaskHandle());

    if (strcmp(task_name, "loopTask") == 0 || strcmp(task_name, "httpd") == 0 || strcmp(task_name, "wifi") == 0) {
        malloc_failed_log_detailed(size, caps, function_name, task_name);
    } else {
        logger.print_timestamp();
        logger.print_plain("malloc_failed_hook from other task\n", 35);
        logger.print_plain(task_name, strlen(task_name));
        logger.print_plain("\n", 1);

        esp_backtrace_print(INT32_MAX);
    }
}

static constexpr int CONFIG_TYPES = 9;

static void (*const slot_debug_info_fns[CONFIG_TYPES])(SlotDebugInfo *slot_info) = {
    &get_slot_debug_info<Config::ConfUint>,
    &get_slot_debug_info<Config::ConfInt>,
    &get_slot_debug_info<Config::ConfFloat>,
    &get_slot_debug_info<Config::ConfString>,
    &get_slot_debug_info<Config::ConfArray>,
    &get_slot_debug_info<Config::ConfObject>,
    &get_slot_debug_info<Config::ConfUnion>,
    &get_slot_debug_info<Config::ConfUint53>,
    &get_slot_debug_info<Config::ConfInt52>,
};

void Debug::pre_setup()
{
    heap_caps_register_failed_alloc_callback(malloc_failed_hook);

    size_t dram_heap_size     = heap_caps_get_total_size(MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    size_t iram_heap_size     = heap_caps_get_total_size(MALLOC_CAP_IRAM);
    size_t psram_heap_size    = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);

    size_t psram_size = 0;
#if defined(BOARD_HAS_PSRAM)
    psram_size = 4 * 1024 * 1024;
#endif

    rtc_cpu_freq_config_t cpu_freq_conf;
    rtc_clk_cpu_freq_get_config(&cpu_freq_conf);

    const float dram_speed   = static_cast<float>(cpu_freq_conf.freq_mhz * 1000000 * 4) / 1048576.0f; // DRAM speed cannot be measured accurately, but we know that it can deliver 4 bytes per CPU clock cycle.
    const float iram_speed   = benchmark_area(reinterpret_cast<uint32_t *>(0x40080000), 128*1024);
    const float rodata_speed = benchmark_area(&_rodata_start, 32*1024); // 32KiB at the beginning of the readonly-data
    const float text_speed   = benchmark_area(&_text_start,   32*1024); // 32KiB at the beginning of the code

    float psram_speed = 0;
#if defined(BOARD_HAS_PSRAM)
    psram_speed = benchmark_area(reinterpret_cast<uint32_t *>(0x3FB00000), 32*1024); // 32KiB inside the fourth MiB
#endif

    state_spi_bus_prototype = Config::Object({
        {"clk",          Config::Uint32(0)},
        {"dummy_cycles", Config::Uint8(0)},
        {"spi_mode",     Config::Str("", 0, 14)}
    });

    state_static = Config::Object({
        {"heap_dram",  Config::Uint32(dram_heap_size)},
        {"heap_iram",  Config::Uint32(iram_heap_size)},
        {"heap_psram", Config::Uint32(psram_heap_size)},
        {"psram_size", Config::Uint32(psram_size)},
        {"ipsock_max", Config::Uint8(CONFIG_LWIP_MAX_SOCKETS)},
        {"cpu_clk",    Config::Uint32(cpu_freq_conf.freq_mhz * 1000000)},
        {"apb_clk",    Config::Uint32(rtc_clk_apb_freq_get())},
        {"spi_buses",  Config::Array({},
            &state_spi_bus_prototype,
            4, 4
        )},
        {"dram_benchmark",   Config::Float(dram_speed)},
        {"iram_benchmark",   Config::Float(iram_speed)},
        {"psram_benchmark",  Config::Float(psram_speed)},
        {"rodata_benchmark", Config::Float(rodata_speed)},
        {"text_benchmark",   Config::Float(text_speed)},
    });

    state_static.get("spi_buses")->setCount(4);

    state_fast = Config::Object({
        {"uptime",     Config::Uint32(0)},
        {"free_dram",  Config::Uint32(0)},
        {"free_iram",  Config::Uint32(0)},
        {"free_psram", Config::Uint32(0)},
        {"heap_check_time_avg", Config::Uint32(0)},
        {"heap_check_time_max", Config::Uint32(0)},
        {"cpu_usage",  Config::Uint8(0)},
    });

    state_slow = Config::Object({
        {"largest_free_dram_block",  Config::Uint32(0)},
        {"largest_free_iram_block",  Config::Uint32(0)},
        {"largest_free_psram_block", Config::Uint32(0)},
        {"heap_integrity_ok", Config::Bool(true)},
        {"loop_max_us", Config::Uint32(0)},
        {"loop_max_fn_file", Config::Str("", 0, 0)},
        {"loop_max_fn_line", Config::Uint32(0)},
        {"min_free_dram", Config::Uint32(0)},
        {"min_free_iram", Config::Uint32(0)},
        {"min_free_psram", Config::Uint32(0)},
        {"conf_uint_buf_size", Config::Uint32(0)},
        {"conf_int_buf_size", Config::Uint32(0)},
        {"conf_float_buf_size", Config::Uint32(0)},
        {"conf_string_buf_size", Config::Uint32(0)},
        {"conf_array_buf_size", Config::Uint32(0)},
        {"conf_object_buf_size", Config::Uint32(0)},
        {"conf_union_buf_size", Config::Uint32(0)},
        {"conf_uint53_buf_size", Config::Uint32(0)},
        {"conf_int52_buf_size", Config::Uint32(0)},
        {"ipsock_cur", Config::Uint8(0)},
        {"ipsock_hwm", Config::Uint8(0)},
    });

    state_slots_prototype = Config::Array({},
        Config::get_prototype_uint16_0(),
        5, 5
    );

    state_slots = Config::Array({},
        &state_slots_prototype,
        CONFIG_TYPES, CONFIG_TYPES
    );

    state_slots.setCount(CONFIG_TYPES);

    for (size_t i = 0; i < CONFIG_TYPES; i++) {
        state_slots.get(i)->setCount(5);
    }

    state_hwm_prototype = Config::Object({
        {"task_name",  Config::Str("", 0, CONFIG_FREERTOS_MAX_TASK_NAME_LEN)},
        {"hwm",        Config::Uint16(0)},
        {"stack_size", Config::Uint16(0)},
    });

    state_hwm = Config::Array({},
        &state_hwm_prototype,
        0, 64
    );


    task_handles.reserve(16);
    register_task(xTaskGetCurrentTaskHandle(),      getArduinoLoopTaskStackSize());
    register_task(xTaskGetIdleTaskHandleForCore(0), sizeof(StackType_t) * configMINIMAL_STACK_SIZE);
    register_task(xTaskGetIdleTaskHandleForCore(1), sizeof(StackType_t) * configMINIMAL_STACK_SIZE);
    register_task(xTimerGetTimerDaemonTaskHandle(), sizeof(StackType_t) * configTIMER_TASK_STACK_DEPTH);
    register_task("esp_timer",                      ESP_TASK_TIMER_STACK);

// Copied from esp_ipc.c
#if defined(CONFIG_COMPILER_OPTIMIZATION_NONE) && CONFIG_COMPILER_OPTIMIZATION_NONE
#define IPC_STACK_SIZE (CONFIG_ESP_IPC_TASK_STACK_SIZE + 0x100)
#else
#define IPC_STACK_SIZE (CONFIG_ESP_IPC_TASK_STACK_SIZE)
#endif //CONFIG_COMPILER_OPTIMIZATION_NONE

    register_task("ipc0", IPC_STACK_SIZE);
    register_task("ipc1", IPC_STACK_SIZE);
}

void Debug::setup()
{
    task_scheduler.scheduleWithFixedDelay([this]() {
        multi_heap_info_t dram_info;
        multi_heap_info_t iram_info;
        multi_heap_info_t psram_info;
        heap_caps_get_info(&dram_info,  MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        heap_caps_get_info(&iram_info,  MALLOC_CAP_IRAM);
        heap_caps_get_info(&psram_info, MALLOC_CAP_SPIRAM);

        state_fast.get("uptime")->updateUint(now_us().to<millis_t>().as<uint32_t>());
        state_fast.get("free_dram")->updateUint(dram_info.total_free_bytes);
        state_fast.get("free_iram")->updateUint(iram_info.total_free_bytes);
        state_fast.get("free_psram")->updateUint(psram_info.total_free_bytes);

        state_slow.get("largest_free_dram_block")->updateUint(dram_info.largest_free_block);
        state_slow.get("largest_free_iram_block")->updateUint(iram_info.largest_free_block);
        state_slow.get("largest_free_psram_block")->updateUint(psram_info.largest_free_block);
        state_slow.get("min_free_dram")->updateUint(dram_info.minimum_free_bytes);
        state_slow.get("min_free_iram")->updateUint(iram_info.minimum_free_bytes);
        state_slow.get("min_free_psram")->updateUint(psram_info.minimum_free_bytes);

        state_slow.get("conf_uint_buf_size"  )->updateUint(get_allocated_slot_memory<Config::ConfUint>());
        state_slow.get("conf_int_buf_size"   )->updateUint(get_allocated_slot_memory<Config::ConfInt>());
        state_slow.get("conf_float_buf_size" )->updateUint(get_allocated_slot_memory<Config::ConfFloat>());
        state_slow.get("conf_string_buf_size")->updateUint(get_allocated_slot_memory<Config::ConfString>());
        state_slow.get("conf_array_buf_size" )->updateUint(get_allocated_slot_memory<Config::ConfArray>());
        state_slow.get("conf_object_buf_size")->updateUint(get_allocated_slot_memory<Config::ConfObject>());
        state_slow.get("conf_union_buf_size" )->updateUint(get_allocated_slot_memory<Config::ConfUnion>());
        state_slow.get("conf_uint53_buf_size")->updateUint(get_allocated_slot_memory<Config::ConfUint53>());
        state_slow.get("conf_int52_buf_size" )->updateUint(get_allocated_slot_memory<Config::ConfInt52>());

        for (size_t i = 0; i < CONFIG_TYPES; i++) {
            SlotDebugInfo slot_info;
            slot_debug_info_fns[i](&slot_info);
            auto arr = state_slots.get(i);
            arr->get(0)->updateUint(slot_info.used_slots);
            arr->get(1)->updateUint(slot_info.slots_hwm);
            arr->get(2)->updateUint(slot_info.allocated_slots);
            arr->get(3)->updateUint(slot_info.first_free_slot);
            arr->get(4)->updateUint(slot_info.last_used_slot);
        }

        if (dram_info.largest_free_block < 2000) {
            logger.printfln("Heap full. Largest block is %u bytes.", dram_info.largest_free_block);
        }

        size_t task_count = this->task_handles.size();
        for (size_t i = 0; i < task_count; i++) {
            uint32_t hwm = uxTaskGetStackHighWaterMark(this->task_handles[i]);
            Config *conf_task_hwm = static_cast<Config *>(this->state_hwm.get(i));
            if (conf_task_hwm->get("hwm")->updateUint(hwm) && hwm < 256) {
                const char *task_name = conf_task_hwm->get("task_name")->asUnsafeCStr();
                if (hwm < 40 || strcmp(task_name, "watchdog_task") != 0) {
                    logger.printfln("HWM of task '%s' changed: %lu", task_name, hwm);
                }
            }
        }

        uint32_t runtime_avg;
        if (this->integrity_check_runs == 0) {
            runtime_avg = 0;
        } else {
            runtime_avg = this->integrity_check_runtime_sum / this->integrity_check_runs;
        }
        state_fast.get("heap_check_time_avg")->updateUint(runtime_avg);
        state_fast.get("heap_check_time_max")->updateUint(this->integrity_check_runtime_max);

        micros_t now = now_us();
        uint32_t time_since_last_update_us = (now - this->last_state_update).as<uint32_t>();
        uint32_t integrity_check_cpu_usage = 100 * this->integrity_check_runtime_sum / time_since_last_update_us;
        state_fast.get("cpu_usage")->updateUint(100 - integrity_check_cpu_usage);
        this->last_state_update = now;

        this->integrity_check_runs = 0;
        this->integrity_check_runtime_sum = 0;
        this->integrity_check_runtime_max = 0;
    }, 1_s, 1_s);

    task_scheduler.scheduleWithFixedDelay([this]() {
        uint8_t lwip_socks_in_use = 0;
        // NUM_SOCKETS is expected to be CONFIG_LWIP_MAX_SOCKETS, but NUM_SOCKETS is what lwIP uses internally.
        for (int i = 0; i < NUM_SOCKETS; i++) {
            // Unprotected access on lwIP sockets. Dereferencing the socket pointer is safe because
            // all socket structs live in a static array. The conn pointer might change at any time,
            // but unprotected read access should be fine because this only gets a momentary snapshot
            // of sockets in use anyway.
            lwip_sock *sock = lwip_socket_dbg_get_socket(LWIP_SOCKET_OFFSET + i);
            if (sock && sock->conn) {
                lwip_socks_in_use++;
            }
        }
        if (lwip_socks_in_use > lwip_sockets_hwm) {
            lwip_sockets_hwm = lwip_socks_in_use;
            state_slow.get("ipsock_hwm")->updateUint(lwip_sockets_hwm);
        }
        state_slow.get("ipsock_cur")->updateUint(lwip_socks_in_use);
    }, 0_ms, 100_ms);

    last_state_update = now_us();

    initialized = true;
}

#ifdef DEBUG_FS_ENABLE
const char * const fs_browser_header = "<script>"
"if (!window.location.toString().endsWith('/')) {"
    "window.location = window.location + '/';"
"}"
"async function uploadFile() {"
    "let file = document.getElementById('upload').files[0];"
    "await fetch(window.location + (window.location.toString().endsWith('/') ? '' : '/') + file.name, {"
        "method: 'PUT',"
        "credentials: 'same-origin',"
        "body: file"
    "});"
    "window.location.reload();"
"}"
"async function createFile() {"
    "let filename = document.getElementById('newFileName').value;"
    "let content = document.getElementById('newFileContent').value;"
    "await fetch(window.location + (window.location.toString().endsWith('/') ? '' : '/') + filename, {"
        "method: 'PUT',"
        "credentials: 'same-origin',"
        "body: content"
    "});"
    "window.location.reload();"
"}"
"async function createDirectory() {"
    "let dirname = document.getElementById('dirname').value;"
    "await fetch(window.location + (window.location.toString().endsWith('/') ? '' : '/') + dirname + (dirname.endsWith('/') ? '' : '/'), {"
        "method: 'PUT',"
        "credentials: 'same-origin'"
    "});"
    "window.location.reload();"
"}"
"async function deleteFile(name) {"
    "if (!window.confirm('Delete file ' + name + ' ?'))"
        "return;"
    "await fetch(window.location + name, {method: 'DELETE'});"
    "window.location.reload();"
"}"
"</script>";

const char * const fs_browser_footer =
"<br>"
"<hr>"
"<br>"
"<div>"
    "<label for=upload>Create directory</label>&nbsp;&nbsp;&nbsp;"
    "<input type=text id=dirname placeholder='Directory name'>&nbsp;&nbsp;&nbsp;"
    "<button type=button onClick=createDirectory()>Create</button>"
"</div>"
"<br>"
"<hr>"
"<br>"
"<div>"
    "<label for=upload>Upload file</label>&nbsp;&nbsp;&nbsp;"
    "<input type=file id=upload>"
    "<button type=button onClick=uploadFile()>Upload</button>"
"</div>"
"<br>"
"<hr>"
"<br>"
"<div>"
    "<label for=create>Create new file</label>&nbsp;&nbsp;&nbsp;"
    "<input type=text id=newFileName placeholder='File name'>"
    "<br>"
    "<textarea id=newFileContent placeholder='File content' cols=80 rows=25></textarea>"
    "<br>"
    "<button type=button onClick=createFile()>Create file</button>"
"</div>";


static WebServerRequestReturnProtect browse_get(WebServerRequest request, String path) {
    if (path.length() > 1 && path[path.length() - 1] == '/')
        path = path.substring(0, path.length() - 1);

    if (!LittleFS.exists(path))
        return request.send(404, "text/plain", ("File " + path + " not found").c_str());

    File f = LittleFS.open(path);
    if (!f.isDirectory()) {
        char buf[256];
        request.beginChunkedResponse(200);
        while (f.available()) {
            size_t read = f.read(reinterpret_cast<uint8_t *>(buf), ARRAY_SIZE(buf));
            request.sendChunk(buf, static_cast<ssize_t>(read));
        }
        return request.endChunkedResponse();
    } else {
        request.beginChunkedResponse(200, "text/html; charset=utf-8");
        request.sendChunk(fs_browser_header, strlen(fs_browser_header));
        String header = "<h1>" + String(f.path()) + "</h1><br>\n";
        request.sendChunk(header.c_str(), static_cast<ssize_t>(header.length()));

        if (path.length() > 1) {
            String up = "<button type=button onclick=\"\" style=\"visibility: hidden;\">Delete</button>&nbsp;&nbsp;&nbsp;<a href='..'>..</a><br>\n";

            request.sendChunk(up.c_str(), static_cast<ssize_t>(up.length()));
        }

        File file = f.openNextFile();
        while(file) {
            String s = "<button type=button onclick=\"deleteFile('/" + String(file.name()) + "')\">Delete</button>&nbsp;&nbsp;&nbsp;<a href=" + String(file.name()) + (file.isDirectory() ? "/" : "") + ">"+ file.name() + (file.isDirectory() ? "/" : "") +"</a><br>\n";
            request.sendChunk(s.c_str(), static_cast<ssize_t>(s.length()));
            file = f.openNextFile();
        }

        request.sendChunk(fs_browser_footer, strlen(fs_browser_footer));

        return request.endChunkedResponse();
    }
}

static WebServerRequestReturnProtect browse_delete(WebServerRequest request, String path) {
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
}

static WebServerRequestReturnProtect browse_put(WebServerRequest request, String path) {
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

    auto size = request.contentLength();
    auto payload = heap_alloc_array<char>(size);
    if (request.receive(payload.get(), size) < 0)
        return request.send(500, "text/plain", "failed to receive");

    f.write(reinterpret_cast<uint8_t *>(payload.get()), size);
    return request.send(200, "text/plain", ("File " + path + " created.").c_str());
}
#endif

void Debug::register_urls()
{
    api.addState("debug/state_static", &state_static);
    api.addState("debug/state_fast", &state_fast);
    api.addState("debug/state_slow", &state_slow);
    api.addState("debug/state_slots", &state_slots);
    api.addState("debug/state_hwm", &state_hwm);

#ifdef DEBUG_FS_ENABLE
    server.on_HTTPThread("/debug/crash", HTTP_GET, [](WebServerRequest req) {
        esp_system_abort("Crash requested");
        return req.send(200);
    });
#endif

    server.on_HTTPThread("/debug/state_sizes", HTTP_GET, [](WebServerRequest req) {
        char buf[3072]; // on httpd stack, which is large enough
        StringWriter sw(buf, sizeof(buf));
        task_scheduler.await([&sw]() {
            for (const auto &reg : api.states) {
                sw.printf("%4u %s\n", reg.config->string_length(), reg.path);
            }
        });
        return req.send(200, "text/plain", sw.getPtr(), static_cast<ssize_t>(sw.getLength()));
    });

#ifdef DEBUG_FS_ENABLE
    server.on_HTTPThread("/debug/rtos_tasks", HTTP_GET, [](WebServerRequest req) {
        char buf[2048]; // "This buffer is assumed to be large enough to contain the generated report. Approximately 40 bytes per task should be sufficient." - vTaskGetRunTimeStats@FreeRTOS
        vTaskGetRunTimeStats(buf);
        return req.send(200, "text/plain", buf, HTTPD_RESP_USE_STRLEN);
    });

    server.on_HTTPThread("/debug/fs/*", HTTP_GET, [this](WebServerRequest request) {
        String path = request.uri().substring(ARRAY_SIZE("/debug/fs") - 1);

        return browse_get(request, path);
    });

    server.on_HTTPThread("/debug/fs/*", HTTP_DELETE, [this](WebServerRequest request) {
        String path = request.uri().substring(ARRAY_SIZE("/debug/fs") - 1);

        return browse_delete(request, path);
    });

    server.on_HTTPThread("/debug/fs/*", HTTP_PUT, [this](WebServerRequest request) {
        String path = request.uri().substring(ARRAY_SIZE("/debug/fs") - 1);

        return browse_put(request, path);
    });

    server.on_HTTPThread("/debug/fs", HTTP_GET, [this](WebServerRequest request) {
        return browse_get(request, "/");
    });

    server.on_HTTPThread("/debug/fs", HTTP_DELETE, [this](WebServerRequest request) {
        return browse_delete(request, "/");
    });

    server.on_HTTPThread("/debug/fs", HTTP_PUT, [this](WebServerRequest request) {
        return browse_put(request, "/");
    });
#endif
}

void Debug::register_events()
{
    // Query SPI buses and tasks here after everything else has been set up.

    uint32_t apb_clk = state_static.get("apb_clk")->asUint();
    Config *conf_spi_buses = static_cast<Config *>(state_static.get("spi_buses"));

    for (uint32_t i = 0; i < 4; i++) {
        uint32_t spi_clk;
        uint32_t dummy_cyclelen;
        const char *spi_mode;

        get_spi_settings(i, apb_clk, &spi_clk, &dummy_cyclelen, &spi_mode);

        Config *conf = static_cast<Config *>(conf_spi_buses->get(i));
        conf->get("clk")->updateUint(spi_clk);
        conf->get("dummy_cycles")->updateUint(dummy_cyclelen);
        conf->get("spi_mode")->updateString(spi_mode);
    }

    register_task("tiT",            TCPIP_THREAD_STACKSIZE);
    register_task("emac_rx",        2048, Optional); // stack size set by us in ethernet.cpp
    register_task("wifi",           6656, Optional); // stack size observed at runtime from task creation
    register_task("sys_evt",        ESP_TASKD_EVENT_STACK); // created in WiFiGeneric.cpp
    register_task("arduino_events", 4096); // stack size from WiFiGeneric.cpp

    register_task("async_udp",       0, ExpectMissing);
    register_task("btm_rrm_t",       0, ExpectMissing);
    register_task("console_repl",    0, ExpectMissing);
    register_task("https_ota_task",  0, ExpectMissing);
    register_task("l2tap_clean_tas", 0, ExpectMissing);
    register_task("main",            0, ExpectMissing);
    register_task("ot_cli",          0, ExpectMissing);
    register_task("protocomm_conso", 0, ExpectMissing);
    register_task("sc_ack_send_tas", 0, ExpectMissing);
    register_task("tcpip_thread",    0, ExpectMissing);
    register_task("uart_event_task", 0, ExpectMissing);
    register_task("wpsT",            0, ExpectMissing);
}

#if defined(BOARD_HAS_PSRAM)
#define CHECK_PSRAM 1
#else
#define CHECK_PSRAM 0
#endif

void Debug::task_scheduler_idle_call()
{
    micros_t start = now_us();
    if (CHECK_PSRAM && check_psram_next) {
        psram_heap_valid = heap_caps_check_integrity(MALLOC_CAP_SPIRAM, integrity_check_print_errors);
        check_psram_next = false;
    } else {
        internal_heap_valid = heap_caps_check_integrity(MALLOC_CAP_INTERNAL, integrity_check_print_errors);
        if (CHECK_PSRAM) {
            check_psram_next = true;
        }
    }
    uint32_t runtime = (now_us() - start).as<uint32_t>();

    integrity_check_runs++;
    integrity_check_runtime_sum += runtime;

    if (runtime > integrity_check_runtime_max) {
        integrity_check_runtime_max = runtime;
    }

    if (!(internal_heap_valid & psram_heap_valid)) {
        state_slow.get("heap_integrity_ok")->updateBool(false);
        integrity_check_print_errors = false;
    }
}

extern const char *task_fn_file;
extern uint_least32_t task_fn_line;

void Debug::custom_loop() {
    micros_t start = now_us();
    uint32_t run = (start - last_run).as<uint32_t>();
    if (run > run_max && last_run != 0_us) {
        run_max = run;
        state_slow.get("loop_max_us")->updateUint(run_max);
        if (task_fn_file != nullptr) {
            state_slow.get("loop_max_fn_file")->updateString(task_fn_file);
            state_slow.get("loop_max_fn_line")->updateUint(task_fn_line);
        }
    }
    last_run = start;
}

void Debug::register_task(const char *task_name, uint32_t stack_size, TaskAvailability availability)
{
    size_t task_count = state_hwm.count();
    for (size_t i = 0; i < task_count; i++) {
        if (state_hwm.get(i)->get("task_name")->asString() == task_name) {
            logger.printfln("Can't register task: A task named '%s' is already registered.", task_name);
            return;
        }
    }

    TaskHandle_t handle = xTaskGetHandle(task_name);
    if (!handle) {
        if (availability == ExpectPresent) {
            logger.printfln("Can't find task '%s'", task_name);
        }
        return;
    }
    if (availability == ExpectMissing) {
        logger.printfln("Found task '%s'", task_name);
    }
    register_task(handle, stack_size);
}

void Debug::register_task(TaskHandle_t handle, uint32_t stack_size)
{
    if (!handle) {
        logger.printfln("register_task called with invalid handle.");
        return;
    }

    const char *task_name = pcTaskGetName(handle);
    if (!task_name) {
        logger.printfln("register_task couldn't find task.");
        return;
    }

    size_t task_count = task_handles.size();
    for (size_t i = 0; i < task_count; i++) {
        if (task_handles[i] == handle) {
            logger.printfln("Can't register task: TaskHandle of task '%s' already registered.", task_name);
            return;
        }
    }

    task_handles.push_back(handle);

    Config *conf = static_cast<Config *>(state_hwm.add());
    conf->get("task_name")->updateString(task_name);
    conf->get("hwm")->updateUint(uxTaskGetStackHighWaterMark(handle));
    conf->get("stack_size")->updateUint(stack_size);
}

void Debug::deregister_task(const char *task_name)
{
    size_t task_count = state_hwm.count();
    for (size_t i = 0; i < task_count; i++) {
        if (state_hwm.get(i)->get("task_name")->asString() == task_name) {
            deregister_task_internal(i);
            return;
        }
    }

    logger.printfln("Cannot deregister task: Name '%s' not found.", task_name);
}

void Debug::deregister_task(TaskHandle_t handle)
{
    if (!handle) {
        logger.printfln("deregister_task called with invalid handle.");
        return;
    }

    size_t task_count = task_handles.size();
    for (size_t i = 0; i < task_count; i++) {
        if (task_handles[i] == handle) {
            deregister_task_internal(i);
            return;
        }
    }

    logger.printfln("Cannot deregister task: Handle not found.");
}

void Debug::deregister_task_internal(size_t i)
{
    task_handles.erase(task_handles.begin() + static_cast<int>(i));
    state_hwm.remove(i);
}

static float benchmark_area(uint32_t *start_address, size_t max_length)
{
    uint32_t *end_address = start_address + (max_length / 4);

    micros_t start_time = now_us();
    while (start_address < end_address) {
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
        asm volatile ("" : /* No outputs */ : "r" (*start_address++));
    }
    micros_t runtime = now_us() - start_time;

    uint32_t runtime32 = runtime.as<uint32_t>();
    float runtime_f = static_cast<float>(runtime32);
    float test_length_f = static_cast<float>(max_length);
    float speed_MiBps = (test_length_f * 1000000.0F) / (runtime_f * 1024 * 1024);

    return speed_MiBps;
}

static void get_spi_settings(uint32_t spi_num, uint32_t apb_clk, uint32_t *spi_clk, uint32_t *dummy_cyclelen, const char **spi_mode)
{
    // Dummy cycles
    uint32_t spi_user1_reg = *reinterpret_cast<uint32_t *>(SPI_USER1_REG(spi_num));
    *dummy_cyclelen = spi_user1_reg >> SPI_USR_DUMMY_CYCLELEN_S & SPI_USR_DUMMY_CYCLELEN_V;

    // Mode
    uint32_t spi_ctrl_reg = *reinterpret_cast<uint32_t *>(SPI_CTRL_REG(spi_num));
    uint32_t fread_qio  = spi_ctrl_reg >> SPI_FREAD_QIO_S  & SPI_FREAD_QIO_V;
    uint32_t fread_dio  = spi_ctrl_reg >> SPI_FREAD_DIO_S  & SPI_FREAD_DIO_V;
    uint32_t fread_quad = spi_ctrl_reg >> SPI_FREAD_QUAD_S & SPI_FREAD_QUAD_V;
    uint32_t fread_dual = spi_ctrl_reg >> SPI_FREAD_DUAL_S & SPI_FREAD_DUAL_V;
    uint32_t mode_count = fread_qio + fread_dio + fread_quad + fread_dual;

    if (mode_count == 0) {
        *spi_mode = "four-wire";
    } else if (mode_count == 1) {
        if (fread_qio) {
            *spi_mode = "qio";
        } else if (fread_dio) {
            *spi_mode = "dio";
        } else if (fread_quad) {
            *spi_mode = "quad";
        } else {
            *spi_mode = "dual";
        }
    } else {
        *spi_mode = "invalid mode";
        logger.printfln("fread_qio=%lu fread_dio=%lu fread_quad=%lu fread_dual=%lu", fread_qio, fread_dio, fread_quad, fread_dual);
    }

    // Clock
    uint32_t spi_clock_reg = *reinterpret_cast<uint32_t *>(SPI_CLOCK_REG(spi_num));
    uint32_t clk_equ_sysclk = spi_clock_reg >> SPI_CLK_EQU_SYSCLK_S & SPI_CLK_EQU_SYSCLK_V;
    uint32_t clkdiv_pre     = spi_clock_reg >> SPI_CLKDIV_PRE_S     & SPI_CLKDIV_PRE_V;
    uint32_t clkcnt_n       = spi_clock_reg >> SPI_CLKCNT_N_S       & SPI_CLKCNT_N_V;
    uint32_t clkcnt_h       = spi_clock_reg >> SPI_CLKCNT_H_S       & SPI_CLKCNT_H_V;
    uint32_t clkcnt_l       = spi_clock_reg >> SPI_CLKCNT_L_S       & SPI_CLKCNT_L_V;

    *spi_clk = apb_clk / ((clkcnt_n + 1) * (clkdiv_pre + 1));

    // Clock check
    uint32_t clk_conf_sum = clkdiv_pre + clkcnt_n + clkcnt_h + clkcnt_l;
    if (clk_equ_sysclk == 0) {
        if (clk_conf_sum == 0) {
            *spi_mode = "zero clock";
        }
    } else {
        if (clk_conf_sum != 0) {
            *spi_mode = "invalid clock";
        }
    }
}
