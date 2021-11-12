/* esp32-lib
 * Copyright (C) 2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "config.h"
#include "bindings/hal_common.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "web_server.h"

extern EventLog logger;

extern TF_HalContext hal;
extern WebServer server;

extern API api;

#define BOOTLOADER_MODE_FIRMWARE 1
#define FIRMWARE_DEVICE_IDENTIFIER_OFFSET 8

template <typename DeviceT,
          const uint8_t *firmware,
          const size_t firmware_len,
          int (*init_function)(DeviceT *, const char*, TF_HalContext *),
          int (*get_bootloader_mode_function)(DeviceT *, uint8_t *),
          int (*reset_function)(DeviceT *)>
class DeviceModule {
public:
    DeviceModule(const char *url_prefix,
                 const char *device_name,
                 const char *module_name,
                 std::function<void(void)> setup_function) :
        device_reflash(Config::Null()),
        device_reset(Config::Null()),
        url_prefix(url_prefix),
        device_name(device_name),
        module_name(module_name),
        setup_function(setup_function)
        {}

    uint16_t get_did() {
        return firmware[firmware_len - FIRMWARE_DEVICE_IDENTIFIER_OFFSET] | (firmware[firmware_len - FIRMWARE_DEVICE_IDENTIFIER_OFFSET + 1] << 8);
    }

    bool setup_device() {
        char uid[7] = {0};
        if (!find_uid_by_did(&hal, get_did(), uid)) {
            logger.printfln("No %s bricklet found. Disabling %s support.", device_name, module_name);
            return false;
        }
        this->device_found = true;

        int result = ensure_matching_firmware(&hal, uid, device_name, module_name, firmware, firmware_len, &logger);
        if(result != 0) {
            logger.printfln("Flashing failed (%d)", result);
            return false;
        }

        result = init_function(&device, uid, &hal);
        if(result != TF_E_OK) {
            logger.printfln("Failed to initialize %s bricklet (%d). Disabling %s support.", device_name, result, module_name);
            return false;
        }

        return true;
    }

    void register_urls(){
        api.addCommand(url_prefix + "/reflash", &device_reflash, {}, [this](){
            char uid[7] = {0};
            find_uid_by_did(&hal, get_did(), uid);
            ensure_matching_firmware(&hal, uid, device_name, module_name, firmware, firmware_len, &logger, true);
        }, true);

        api.addCommand(url_prefix + "/reset", &device_reset, {}, [this](){
            reset_function(&device);
            initialized = false;
        }, true);
    }

    void loop() {
        if(device_found && !initialized && deadline_elapsed(last_check + 10000)) {
            last_check = millis();
            if(!is_in_bootloader(TF_E_TIMEOUT))
                //setup_device();
                setup_function();
        }
    }

    bool is_in_bootloader(int rc) {
        if(rc != TF_E_TIMEOUT && rc != TF_E_NOT_SUPPORTED)
            return false;

        uint8_t mode;
        int bootloader_rc = get_bootloader_mode_function(&device, &mode);
        if(bootloader_rc != TF_E_OK) {
            return false;
        }

        if(mode != BOOTLOADER_MODE_FIRMWARE) {
            initialized = false;
        }

        return mode != BOOTLOADER_MODE_FIRMWARE;
    }

    bool device_found = false;
    bool initialized = false;

    Config device_reflash;
    Config device_reset;

    DeviceT device;

    String url_prefix;
    const char *device_name;
    const char *module_name;
    std::function<void(void)> setup_function;
    uint32_t last_check = 0;
};
