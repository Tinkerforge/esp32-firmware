/* esp32-firmware
 * Copyright (C) 2020-2026 Erik Fleckstein <erik@tinkerforge.com>
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

#include "nfc_bricklet.h"

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "bindings/errors.h"
#include "generated/nfc_bricklet_firmware_bin.embedded.h"

#include "gcc_warnings.h"

NFCBricklet::NFCBricklet() : DeviceModule(nfc_bricklet_firmware_bin_data,
                                          nfc_bricklet_firmware_bin_length,
                                          "nfc",
                                          "NFC",
                                          "NFC",
                                          [this](){this->setup_nfc();}) {}

void NFCBricklet::setup()
{
    setup_nfc();

    if (!device_found)
        return;

    api.addFeature("nfc");

#if MODULE_IO_SCHEDULER_AVAILABLE()
#define SCHEDULER io_scheduler
#else
#define SCHEDULER task_scheduler
#endif
    SCHEDULER.scheduleUncancelable([this]() {
        this->check_state();
    }, 30_s, 30_s); // DeviceModule polls bootloader mode every 10s; no point in using minutes for the NFC mode check.
}

void NFCBricklet::pre_reboot()
{
    // Unregister callback handler. The callback can't be disabled at this point
    // because the IO scheduler doesn't process requests during the pre-reboot stage.
    // Updating the callback (pointer) is atomic and should be safe without locking.
    tf_nfc_register_simple_tag_seen_callback(&device, nullptr, nullptr);
}

void NFCBricklet::setup_nfc()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    int result = tf_nfc_set_mode(&device, TF_NFC_MODE_SIMPLE);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("NFC set mode failed (rc %d). Disabling NFC support.", result);
        }
        return;
    }

    // Clear tag list
    result = tf_nfc_simple_get_tag_id(&device, 255, nullptr, nullptr, nullptr, nullptr);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("Clearing NFC tag list failed (rc %d). Disabling NFC support.", result);
        }
        return;
    }

    // Enable callback
    tf_nfc_register_simple_tag_seen_callback(&device, &static_tag_seen_cb, nullptr);

    result = tf_nfc_set_simple_tag_seen_callback_configuration(&device, 900);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("Enabling tag seen callback failed (rc %d). Disabling NFC support.", result);
        }
        return;
    }

    initialized = true;
}

bool NFCBricklet::get_tag_id(uint8_t index, uint8_t *tag_type, uint8_t *tag_id, uint8_t *tag_id_length, uint32_t *last_seen)
{
    int result = tf_nfc_simple_get_tag_id(&device, index, tag_type, tag_id, tag_id_length, last_seen);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.trace_bricklet_error(result, "Failed to get tag ID %d", index);
        }
        return false;
    }
    return true;
}

// Called by IO scheduler, if present.
void NFCBricklet::static_tag_seen_cb(struct TF_NFC * /*nfc*/, uint8_t tag_type, uint8_t tag_id_length, uint8_t tag_id_data[10], void * /*user_data*/)
{
    struct closure_t {
        aligned_storage<Task> task_buf;
        NFC::tag_t tag;
    };

    closure_t *closure = new(std::nothrow) closure_t;

    if (closure == nullptr) {
        logger.printfln("No memory for seen tag");
        return;
    }

    NFC::tag_t *tag = &closure->tag;

    tag->type = tag_type;
    tag->id_length = tag_id_length;
    memcpy(tag->id_bytes, tag_id_data, tag_id_length);

    // Transfer ownership, will free the whole closure.
    task_scheduler.scheduleOnceNoAlloc(&closure->task_buf, true, [tag]() {
        nfc.local_tag_seen(tag);
    });
}

void NFCBricklet::check_state()
{
    uint8_t mode = 0;
    int result = tf_nfc_get_mode(&device, &mode);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.trace_bricklet_error(result, "Failed to get NFC mode");
        }
        return;
    }
    if (mode != TF_NFC_MODE_SIMPLE) {
        logger.printfln("NFC mode invalid. Did the Bricklet reset?");
        setup_nfc();
    }
}
