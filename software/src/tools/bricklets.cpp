/* esp32-firmware
 * Copyright (C) 2024 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include "bricklets.h"

#include "bindings/bricklet_unknown.h"
#include "bindings/errors.h"
#include "event_log_prefix.h"
#include "main_dependencies.h"

#include "gcc_warnings.h"

extern TF_HAL hal;

int check(int rc, const char *msg)
{
    if (rc >= 0) {
        return rc;
    }

    logger.printfln("%lu Failed to %s rc: %s", millis(), msg, tf_hal_strerror(rc));
    delay(10);

    return rc;
}

static bool wait_for_bootloader_mode(TF_Unknown *bricklet, uint8_t target_mode)
{
    uint8_t mode = 255;

    for (int i = 0; i < 10; ++i) {
        if (tf_unknown_get_bootloader_mode(bricklet, &mode) != TF_E_OK) {
            continue;
        }

        if (mode == target_mode) {
            return true;
        }

        delay(250);
    }

    return false;
}

static constexpr const size_t XMC1_PAGE_SIZE = 256;
static constexpr const size_t MAX_SKIPPED_ZERO_BYTES = 20 * 1024; // Theoretical worst-case maximum possible is ~23KiB.
static constexpr const size_t MAX_SKIPPED_PAGES = MAX_SKIPPED_ZERO_BYTES / XMC1_PAGE_SIZE;

static bool flash_plugin(TF_Unknown *bricklet, const uint8_t *firmware, size_t firmware_len, bool allow_skipping)
{
    logger.printfln_continue("Setting bootloader mode to bootloader.");
    tf_unknown_set_bootloader_mode(bricklet, TF_UNKNOWN_BOOTLOADER_MODE_BOOTLOADER, nullptr);

    logger.printfln_continue("Waiting for bootloader...");

    if (!wait_for_bootloader_mode(bricklet, TF_UNKNOWN_BOOTLOADER_MODE_BOOTLOADER)) {
        logger.printfln_continue("Timed out, flashing failed");
        return false;
    }

    logger.printfln_continue("Device is in bootloader, flashing...");

    size_t skipped_pages = std::numeric_limits<decltype(skipped_pages)>::max(); // Initialize with max to force writing the first page.

    for (size_t position = 0; position < firmware_len; position += XMC1_PAGE_SIZE) {
        // Don't allow skipping the last page.
        if (allow_skipping && skipped_pages < MAX_SKIPPED_PAGES && firmware_len - position > XMC1_PAGE_SIZE) {
            bool all_zero = true;

            for (size_t i = 0; i < XMC1_PAGE_SIZE; i++) {
                if (firmware[position + i] != 0) {
                    all_zero = false;
                    break;
                }
            }

            if (all_zero) {
                skipped_pages++;
                continue;
            }
        }

        skipped_pages = 0;

        for (size_t subpage_offset = 0; subpage_offset < XMC1_PAGE_SIZE; subpage_offset += 64) {
            size_t subpage_position = position + subpage_offset;

            for (size_t i = 0;; i++) {
                int rc = tf_unknown_set_write_firmware_pointer(bricklet, subpage_position);
                if (rc != TF_E_OK) {
                    logger.printfln_continue("Failed to set firmware pointer to %zu (%i)", subpage_position, rc);
                    if (i < 2) { // i is fails, not attempts
                        continue;
                    }
                }

                rc = tf_unknown_write_firmware(bricklet, firmware + subpage_position, nullptr);
                if (rc != TF_E_OK) {
                    logger.printfln_continue("Failed to write firmware at %zu (%i)", subpage_position, rc);
                    if (i < 2) { // i is fails, not attempts
                        continue;
                    }
                }

                // Block ok
                break;
            }
        }
    }

    logger.printfln_continue("Device flashed successfully.");

    return true;
}

static bool flash_firmware(TF_Unknown *bricklet, const uint8_t *firmware, size_t firmware_len)
{
    //int regular_plugin_upto = -1;

    //for (int i = firmware_len - 13; i >= 4; --i) {
    //    if (firmware[i] == 0x12
    //     && firmware[i - 1] == 0x34
    //     && firmware[i - 2] == 0x56
    //     && firmware[i - 3] == 0x78) {
    //         regular_plugin_upto = i;
    //         break;
    //     }
    //}

    //if (regular_plugin_upto == -1) {
    //    logger.printfln_continue("Firmware end marker not found. Is this a valid firmware?");
    //    return false;
    //}

    if (firmware_len % XMC1_PAGE_SIZE != 0) {
        logger.printfln("Firmware size is not a multiple of the page size (%zu)", firmware_len);
        return false;
    }

    size_t attempt;
    for (attempt = 0; attempt < 2; attempt++) {
        bool allow_skipping = attempt == 0;

        if (!flash_plugin(bricklet, firmware, firmware_len, allow_skipping)) {
            continue;
        }

        logger.printfln_continue("Setting bootloader mode to firmware.");

        uint8_t ret_status = 0;
        tf_unknown_set_bootloader_mode(bricklet, TF_UNKNOWN_BOOTLOADER_MODE_FIRMWARE, &ret_status);

        if (ret_status != TF_UNKNOWN_BOOTLOADER_STATUS_OK && ret_status != TF_UNKNOWN_BOOTLOADER_STATUS_NO_CHANGE) {
            logger.printfln_continue("Failed to set bootloader mode to firmware. Attempt %zu, status %hhu.", attempt + 1, ret_status);
            continue;
        }

        logger.printfln_continue("Waiting for firmware...");

        if (!wait_for_bootloader_mode(bricklet, TF_UNKNOWN_BOOTLOADER_MODE_FIRMWARE)) {
            logger.printfln_continue("Timed out");
            continue;
        }

        logger.printfln_continue("Firmware flashed successfully");

        return true;
    }

    logger.printfln("Flashing failed after %zu attempts", attempt);

    return false;
}

#define FIRMWARE_MAJOR_OFFSET 10
#define FIRMWARE_MINOR_OFFSET 11
#define FIRMWARE_PATCH_OFFSET 12

int ensure_matching_firmware(TF_TFP *tfp, const char *name, const char *purpose, const uint8_t *firmware, size_t firmware_len, bool force)
{
    TFPSwap tfp_swap(tfp);
    TF_Unknown bricklet;
    auto old_timeout = tf_hal_get_timeout(&hal);
    defer {tf_hal_set_timeout(&hal, old_timeout);};
    tf_hal_set_timeout(&hal, 2500 * 1000);


    int rc = tf_unknown_create(&bricklet, tfp);
    defer {tf_unknown_destroy(&bricklet);};

    if (rc != TF_E_OK) {
        logger.printfln("%s init failed (rc %d).", name, rc);
        return -1;
    }

    uint8_t firmware_version[3] = {0};

    rc = tf_unknown_get_identity(&bricklet, nullptr, nullptr, nullptr, nullptr, firmware_version, nullptr);

    if (rc != TF_E_OK) {
        logger.printfln("%s get identity failed (rc %d).", name, rc);
        return -1;
    }

    uint8_t embedded_firmware_version[3] = {
        firmware[firmware_len - FIRMWARE_MAJOR_OFFSET],
        firmware[firmware_len - FIRMWARE_MINOR_OFFSET],
        firmware[firmware_len - FIRMWARE_PATCH_OFFSET],
    };

    bool flash_required = force;

    for (int i = 0; i < 3; ++i) {
        // Intentionally use != here: we also want to downgrade the bricklet firmware if the ESP firmware embeds an older one.
        // This makes sure, that the interfaces fit.
        flash_required |= firmware_version[i] != embedded_firmware_version[i];
    }

    uint8_t mode;
    tf_unknown_get_bootloader_mode(&bricklet, &mode);
    flash_required |= mode != TF_UNKNOWN_BOOTLOADER_MODE_FIRMWARE;

    if (flash_required) {
        if (force) {
            logger.printfln("Forcing %s firmware update to %d.%d.%d. Flashing firmware...",
                             name,
                             embedded_firmware_version[0], embedded_firmware_version[1], embedded_firmware_version[2]);
        } else {
            logger.printfln("%s firmware is %d.%d.%d not the expected %d.%d.%d. Flashing firmware...",
                             name,
                             firmware_version[0], firmware_version[1], firmware_version[2],
                             embedded_firmware_version[0], embedded_firmware_version[1], embedded_firmware_version[2]);
        }

        if (!flash_firmware(&bricklet, firmware, firmware_len)) {
            logger.printfln("%s flashing failed.", name);
            return -1;
        }
    }

    return 0;
}
