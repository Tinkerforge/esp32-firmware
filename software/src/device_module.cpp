/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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

#include "device_module.h"

#include "bindings/base58.h"
#include "bindings/bricklet_unknown.h"
#include "bindings/errors.h"
#include "event_log_prefix.h"
#include "main_dependencies.h"
#include "tools/bricklets.h"

#include "gcc_warnings.h"

extern TF_HAL hal;

#define BOOTLOADER_MODE_FIRMWARE 1
#define FIRMWARE_DEVICE_IDENTIFIER_OFFSET 8

void device_module_run_on_hal_owner(std::function<void(void)> &&fn)
{
#if MODULE_IO_SCHEDULER_AVAILABLE()
    (void)io_scheduler.await(std::move(fn));
#else
    fn();
#endif
}

void DeviceModuleBase::pre_setup()
{
    identity = Config::Object({
        {"uid", Config::Str("", 0, 8)},
        {"connected_uid", Config::Str("", 0, 8)},
        {"position", Config::Str("", 0, 1)},
        {"hw_version", Config::Str("", 0, 12)},
        {"fw_version", Config::Str("", 0, 12)},
        {"device_identifier", Config::Uint16(123)}
    });
}

void DeviceModuleBase::register_urls()
{
    String url_prefix_str(url_prefix);

    api.addCommand(url_prefix_str + "/reflash", Config::Null(), {}, [this](Language /*language*/, String &/*errmsg*/) {
        auto reflash = [this]() {
            uint16_t device_id = get_device_id();
            TF_TFP *tfp = tf_hal_get_tfp(&hal, nullptr, nullptr, &device_id, false);

            if (tfp != nullptr) {
                ensure_matching_firmware(tfp, device_name, module_name, firmware, firmware_len, true);
            }
        };

#if MODULE_IO_SCHEDULER_AVAILABLE()
        // Flashing a Bricklet can take a while. Use a generous timeout.
        (void)io_scheduler.await(std::move(reflash), 5_min);
#else
        reflash();
#endif
    }, true);

    api.addCommand(url_prefix_str + "/reset", Config::Null(), {}, [this](Language /*language*/, String &/*errmsg*/) {
        reset(); // Runs on the HAL-owning task, see DeviceModule::reset().

        initialized = false;
    }, true);

    api.addState(url_prefix_str + "/identity", &identity);
}

uint16_t DeviceModuleBase::get_device_id()
{
    return static_cast<uint16_t>(firmware[firmware_len - FIRMWARE_DEVICE_IDENTIFIER_OFFSET] | (firmware[firmware_len - FIRMWARE_DEVICE_IDENTIFIER_OFFSET + 1] << 8));
}

void DeviceModuleBase::update_identity(TF_TFP *tfp)
{
    struct IdentityData {
        char uid[8] = {};
        char connected_uid[8] = {};
        char position = '\0';
        uint8_t hw_version[3] = {};
        uint8_t fw_version[3] = {};
        uint16_t device_identifier = 0;
    } data;

    TFPSwap swap(tfp);
    TF_Unknown unknown;

    int rc = tf_unknown_create(&unknown, tfp);
    defer {tf_unknown_destroy(&unknown);};

    if (rc != TF_E_OK) {
        logger.printfln("Creation of unknown device failed with rc %i", rc);
        return;
    }

    rc = tf_unknown_get_identity(&unknown, data.uid, data.connected_uid, &data.position, data.hw_version, data.fw_version, &data.device_identifier);
    if (rc != TF_E_OK) {
        logger.printfln("Getting identity of unknown device failed with rc %i", rc);
    }

    // During the boot setup this runs directly, after that from io task.
    auto publish = [this, data]() {
        String value(data.uid);
        identity.get("uid")->updateString(value);

        value = String(data.connected_uid);
        identity.get("connected_uid")->updateString(value);

        value = String(data.position);
        identity.get("position")->updateString(value);

        value = data.hw_version[0] + String(".") + data.hw_version[1] + "." + data.hw_version[2];
        identity.get("hw_version")->updateString(value);

        value = data.fw_version[0] + String(".") + data.fw_version[1] + "." + data.fw_version[2];
        identity.get("fw_version")->updateString(value);

        identity.get("device_identifier")->updateUint(data.device_identifier);
    };

    if (running_in_main_task()) {
        publish();
    } else {
        task_scheduler.scheduleOnce(std::move(publish));
    }
}

bool DeviceModuleBase::is_in_bootloader(int rc)
{
    if (rc != TF_E_TIMEOUT && rc != TF_E_NOT_SUPPORTED) {
        return false;
    }

    uint8_t mode;
    int bootloader_rc;

#if MODULE_IO_SCHEDULER_AVAILABLE()
    bootloader_rc = TF_E_TIMEOUT;

    if (!io_scheduler.await([this, &mode, &bootloader_rc]() { bootloader_rc = get_bootloader_mode(&mode); })) {
        return false;
    }
#else
    bootloader_rc = get_bootloader_mode(&mode);
#endif

    if (bootloader_rc != TF_E_OK) {
        return false;
    }

    if (mode == BOOTLOADER_MODE_FIRMWARE) {
        return false;
    }

    initialized = false;

    return true;
}

bool DeviceModuleBase::setup_device()
{
    // Destroy here in case the device was already initialized once.
    // If we don't destroy it here, tf_hal_get_tfp will not return
    // the TFP context because it is still marked as in use.
    destroy();

    uint16_t device_id = get_device_id();
    TF_TFP *tfp = tf_hal_get_tfp(&hal, nullptr, nullptr, &device_id, true);

    if (!log_message_printed) {
        if (tfp == nullptr && mandatory)
            logger.printfln("No %s Bricklet found. Disabling %s support.", device_name, module_name);
        else if (tfp != nullptr && !mandatory)
            logger.printfln("%s Bricklet found. Enabling %s support.", device_name, module_name);
    }
    log_message_printed = true;

    if (tfp == nullptr)
        return false;

    device_found = true;

    int result = ensure_matching_firmware(tfp, device_name, module_name, firmware, firmware_len, false);

    if (result != 0) {
        logger.printfln("Flashing %s Bricklet failed (%d)", device_name, result);
        logger.printfln("Retrying once.");
        result = ensure_matching_firmware(tfp, device_name, module_name, firmware, firmware_len, false);
        if (result != 0) {
            logger.printfln("Flashing %s Bricklet failed twice (%d). Disabling completely.", device_name, result);
            device_found = false;
            return false;
        }
    }

    if (!task_started) {
#if MODULE_IO_SCHEDULER_AVAILABLE()
        io_scheduler.driveUncancelable(
            [this]() { return !initialized; },
            [this]() {
                if (!initialized) {
                    if (!is_in_bootloader(TF_E_TIMEOUT)) {
                        setup_function();
                    }
                }
            },
            nullptr,
            0_ms, 10_s);
#else
        task_scheduler.scheduleUncancelable([this]() {
            if (!initialized) {
                if (!is_in_bootloader(TF_E_TIMEOUT)) {
                    setup_function();
                }
            }
        }, 10_s);
#endif
        task_started = true;
    }

    char uid[7] = {0};

    tf_base58_encode(tfp->uid_num, uid);

    result = init(uid, &hal);

    if (result != TF_E_OK) {
        logger.printfln("Failed to initialize %s Bricklet (%d). Disabling %s support.", device_name, result, module_name);
        return false;
    }

    update_identity(tfp);
    return true;
}
