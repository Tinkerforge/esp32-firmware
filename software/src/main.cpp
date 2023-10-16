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

#include "index_html.embedded.h"

#include "bindings/hal_common.h"
#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "web_server.h"
#include "build.h"
#include "module.h"
#include "modules_main.h"
#include "tools.h"

#include "gcc_warnings.h"

BootStage boot_stage = BootStage::STATIC_INITIALIZATION;

struct loop_chain {
    struct loop_chain *next;
    IModule *imodule;
};

static struct loop_chain *loop_chain = nullptr;

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

bool firmware_update_allowed = true;

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

static void register_default_urls(void) {
    server.on_HTTPThread("/", HTTP_GET, [](WebServerRequest request) {
        return send_index_html(request);
    });


    api.addCommand("reboot", Config::Null(), {}, []() {
        trigger_reboot("API");
    }, true);

    api.addState("info/modules", &modules, {}, 10000);

    server.on_HTTPThread("/force_reboot", HTTP_GET, [](WebServerRequest request) {
        ESP.restart();
        return request.send(200, "text/plain", "Forced reboot.");
    });

    server.onNotAuthorized_HTTPThread([](WebServerRequest request) {
        if (request.uri() == "/") {
            // Safari does not support an unauthenticated login page and an authenticated main page on the same url,
            // as it does not proactively send the credentials if the same url is known to have an unauthenticated
            // version.
            if (is_safari(request.header("User-Agent"))) {
                return request.requestAuthentication();
            }

            return send_index_html(request);
        } else if (request.uri() == "/login_state") {
            // Same reasoning as above. If we don't force Safari, it does not send credentials, which breaks the login_state check.
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

    api.registerDebugUrl();
}

void setup(void) {
    set_main_task_handle();

    boot_stage = BootStage::PRE_INIT;
    Serial.begin(BUILD_MONITOR_SPEED);

    logger.pre_init();

    logger.printfln("    **** TINKERFORGE " BUILD_DISPLAY_NAME_UPPER " V%s ****", build_version_full_str());
    logger.printfln("         %uK RAM SYSTEM   %u HEAP BYTES FREE", ESP.getHeapSize() / 1024, ESP.getFreeHeap());
    logger.printfln("READY.");

    logger.printfln("Last reset reason was: %s", tf_reset_reason());

    std::vector<IModule*> imodules;
    modules_get_imodules(&imodules);

    config_pre_init();

    for (IModule *imodule : imodules) {
        imodule->pre_init();
    }

    if(!mount_or_format_spiffs()) {
        logger.printfln("Failed to mount SPIFFS.");
    }

    boot_stage = BootStage::PRE_SETUP;

    task_scheduler.pre_setup();
    api.pre_setup();
    logger.pre_setup();

    for (IModule *imodule : imodules) {
        imodule->pre_setup();
    }

    boot_stage = BootStage::SETUP;

    // Setup task scheduler before API: The API setup can run migrations that want to start tasks.
    task_scheduler.setup();
    api.setup();

    for (IModule *imodule : imodules) {
        imodule->setup();
    }

    modules = modules_get_init_config();

    config_post_setup();

    server.start();

    boot_stage = BootStage::REGISTER_URLS;

    register_default_urls();
    logger.register_urls();
    task_scheduler.register_urls();

    for (IModule *imodule : imodules) {
        imodule->register_urls();
    }

    boot_stage = BootStage::REGISTER_EVENTS;

    for (IModule *imodule : imodules) {
        imodule->register_events();
    }

    // Ignore non-overridden empty loop functions.
    // Add all overridden loop functions to a circular list for round-robin execution.
    struct loop_chain **next_chain_ptr = &loop_chain;
    for (IModule *imodule : imodules) {
        if (is_module_loop_overridden(imodule)) {
            *next_chain_ptr = static_cast<struct loop_chain*>(malloc(sizeof(struct loop_chain)));
            (*next_chain_ptr)->imodule = imodule;
            next_chain_ptr = &(*next_chain_ptr)->next;
        }
    }
    *next_chain_ptr = loop_chain; // Close loop. Overwrites loop_chain with itself if the loop is empty.

    boot_stage = BootStage::LOOP;
}

void loop(void) {
    tf_hal_tick(&hal, 0);
    task_scheduler.loop();

    // Round-robin for modules' loop functions, to prioritize HAL ticks and scheduler.
    if (loop_chain != nullptr) {
        loop_chain->imodule->loop();
        loop_chain = loop_chain->next;
    }
}
