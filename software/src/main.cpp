/* esp32-firmware
 * Copyright (C) 2020-2023 Erik Fleckstein <erik@tinkerforge.com>
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

#include <Arduino.h>

#include <stdio.h>
#include <string.h>

#include "event_log_prefix.h"
#include "main_dependencies.h"
#include "modules.h"
#include "index_html.embedded.h"
#include "bindings/hal_common.h"
#include "build.h"
#include "tools.h"

#include "gcc_warnings.h"

BootStage boot_stage = BootStage::STATIC_INITIALIZATION;

static IModule **loop_chain = nullptr;
static size_t loop_chain_size = 0;
static size_t loop_chain_head = 0;

static bool is_module_loop_overridden(const IModule *imodule) {
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    #pragma GCC diagnostic ignored "-Wpedantic"
    #pragma GCC diagnostic ignored "-Wpmf-conversions"
    // GCC pointer to member function magic
    // http://www.cs.fsu.edu/~baker/ada/gnat/html/gcc_6.html#SEC151
    // https://stackoverflow.com/questions/3068144/print-address-of-virtual-member-function
    void (IModule::*loop_ptr)() = &IModule::loop;
    if ((void*)(imodule->*loop_ptr) != (void*)(&IModule::loop)) { // Get pointer to member function and compare with pointer to base class function.
        return true;
    } else {
        return false;
    }
    #pragma GCC diagnostic pop
#else
    (void*)imodule;
    return true;
#endif
}

// declared and initialized by board module
extern TF_HAL hal;
// initialized by board module
uint32_t local_uid_num = 0;
char local_uid_str[32] = {0};
char passphrase[20] = {0};
int8_t blue_led_pin = -1;
int8_t green_led_pin = -1;
int8_t button_pin = -1;
// filled by board module

ConfigRoot modules;

static bool is_safari(const String &user_agent) {
    return user_agent.indexOf("Safari/") >= 0 &&
           user_agent.indexOf("Version/") >= 0 &&
           user_agent.indexOf("Chrome/") == -1 &&
           user_agent.indexOf("Chromium/") == -1;
}

static WebServerRequestReturnProtect send_index_html(WebServerRequest &request) {
    request.addResponseHeader("Content-Encoding", "gzip");
    request.addResponseHeader("ETag", build_timestamp_hex_str());
    request.addResponseHeader("X-Clacks-Overhead", "GNU Terry Pratchett");

    if (request.header("If-None-Match") == build_timestamp_hex_str()) {
        return request.send(304);
    }

    return request.send(200, "text/html; charset=utf-8", index_html_data, index_html_length);
}

static void register_default_urls() {
    server.on_HTTPThread("/", HTTP_GET, [](WebServerRequest request) {
        return send_index_html(request);
    });

    api.addCommand("reboot", Config::Null(), {}, []() {
        trigger_reboot("API", 1000);
    }, true);

    api.addState("info/modules", &modules);

    server.on_HTTPThread("/force_reboot", HTTP_GET, [](WebServerRequest request) {
        ESP.restart();
        return request.send(200, "text/plain", "Forced reboot.");
    });

    server.onNotAuthorized_HTTPThread([](WebServerRequest request) {
        if (request.uri() == "/") {
            return send_index_html(request);
        } else if (request.uri() == "/login_state") {
            // Force Safari to send credentials proactively.
            // This still is broken for the ws:// handler,
            // however there seems to be no way to force safari to proactively send credentials for it.
            // See https://bugs.webkit.org/show_bug.cgi?id=80362
            // Pressing cancel instead of logging in works at least on macOS.
            if (is_safari(request.header("User-Agent"))) {
                return request.requestAuthentication();
            }

            return request.send(200, "text/plain", "Not logged in");
        } else {
            return request.requestAuthentication();
        }
    });

    server.on_HTTPThread("/credential_check", HTTP_GET, [](WebServerRequest request) {
        return request.send(200, "text/plain", "Credentials okay");
    });

    server.on_HTTPThread("/login_state", HTTP_GET, [](WebServerRequest request) {
        return request.send(200, "text/plain", "Logged in");
    });
}

#define PRE_REBOOT_MAX_DURATION (5 * 60 * 1000)

static const char *pre_reboot_message = "Pre-reboot stage lasted longer than five minutes";

#if !MODULE_WATCHDOG_AVAILABLE()

static void pre_reboot_task(void *arg)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
    // portTICK_PERIOD_MS expands to an old style cast.
    vTaskDelay(PRE_REBOOT_MAX_DURATION / portTICK_PERIOD_MS);
#pragma GCC diagnostic pop

    esp_system_abort(pre_reboot_message);
}

static void task_creation_failed(int error_code)
{
    char msg[48];
    msg[0] = 0;
    snprintf(msg, ARRAY_SIZE(msg), "Failed to create pre-reboot task: %i", error_code);
    esp_system_abort(msg);
}

#endif

static void pre_reboot_helper()
{
    std::vector<IModule *> imodules;
    modules_get_imodules(&imodules);

    for (IModule *imodule : imodules) {
        imodule->pre_reboot();
    }

    delay(1500);
}

static void pre_reboot()
{
    boot_stage = BootStage::PRE_REBOOT;

    if (running_in_main_task()) {
#if MODULE_WATCHDOG_AVAILABLE()
        watchdog.add("pre_reboot", pre_reboot_message, PRE_REBOOT_MAX_DURATION, 0, true);
#else
        auto err = xTaskCreatePinnedToCore(
            pre_reboot_task,
            "pre_reboot_task",
            640,
            nullptr,
            ESP_TASK_PRIO_MAX,
            nullptr,
            1);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
        // pdPASS expands to an old-style cast that is also useless
        if (err != pdPASS) {
            task_creation_failed(err);
        }
#pragma GCC diagnostic pop
#endif

        pre_reboot_helper();
    }
    else {
        if (task_scheduler.await(pre_reboot_helper, PRE_REBOOT_MAX_DURATION) == TaskScheduler::AwaitResult::Timeout) {
            esp_system_abort(pre_reboot_message);
        }
    }
}

#if MODULE_WATCHDOG_AVAILABLE()
static int watchdog_handle;
#endif

void setup()
{
    set_main_task_handle();

    boot_stage = BootStage::PRE_INIT;

    // Technically the serial console is already active, because the ESP's ROM bootloader prints some messages.
    // However if BUILD_MONITOR_SPEED is not the ROM bootloader's preferred speed, this call will change the speed.
    Serial.begin(BUILD_MONITOR_SPEED);

    config_pre_init();

    std::vector<IModule *> imodules;
    modules_get_imodules(&imodules);

    for (IModule *imodule : imodules) {
        imodule->pre_init();
    }

    if (esp_register_shutdown_handler(pre_reboot) != ESP_OK) {
        logger.printfln("Failed to register reboot handler");
    }

    if (!mount_or_format_spiffs()) {
        logger.printfln("Failed to mount SPIFFS.");
    }

    boot_stage = BootStage::PRE_SETUP;

    for (IModule *imodule : imodules) {
        imodule->pre_setup();
    }

    boot_stage = BootStage::SETUP;

    for (IModule *imodule : imodules) {
        imodule->setup();
    }

    modules = modules_get_init_config();

    logger.post_setup();
    config_post_setup();
    server.post_setup();

    boot_stage = BootStage::REGISTER_URLS;

    register_default_urls();

    for (IModule *imodule : imodules) {
        imodule->register_urls();
    }

    boot_stage = BootStage::REGISTER_EVENTS;

    for (IModule *imodule : imodules) {
        imodule->register_events();
    }

    // Ignore non-overridden empty loop functions.
    for (IModule *imodule : imodules) {
        if (is_module_loop_overridden(imodule)) {
            ++loop_chain_size;
        }
    }

    // Add all overridden loop functions to a circular list for round-robin execution.
    if (loop_chain_size > 0) {
        loop_chain = static_cast<IModule **>(malloc(sizeof(IModule*) * loop_chain_size));
        size_t loop_chain_used = 0;
        for (IModule *imodule : imodules) {
            if (is_module_loop_overridden(imodule)) {
                loop_chain[loop_chain_used] = imodule;
                ++loop_chain_used;
            }
        }
    }

#if MODULE_WATCHDOG_AVAILABLE()
    watchdog_handle = watchdog.add("main_loop", "Main thread blocked", 30000, 0, true);
#endif

    boot_stage = BootStage::LOOP;
}

void loop() {
#if MODULE_WATCHDOG_AVAILABLE()
    watchdog.reset(watchdog_handle);
#endif

    tf_hal_tick(&hal, 0);
    task_scheduler.custom_loop();

    // Round-robin for modules' loop functions, to prioritize HAL ticks and scheduler.
    if (loop_chain != nullptr) {
        loop_chain[loop_chain_head]->loop();
        loop_chain_head = loop_chain_head + 1;
        if (loop_chain_head >= loop_chain_size) {
            loop_chain_head = 0;
        }
    }
}
