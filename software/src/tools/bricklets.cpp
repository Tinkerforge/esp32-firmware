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

static bool wait_for_bootloader_mode(TF_Unknown *bricklet, int target_mode)
{
    uint8_t mode = 255;

    for (int i = 0; i < 10; ++i) {
        if (tf_unknown_get_bootloader_mode(bricklet, &mode) != TF_E_OK) {
            continue;
        }

        if (mode == target_mode) {
            break;
        }

        delay(250);
    }

    return mode == target_mode;
}

static bool flash_plugin(TF_Unknown *bricklet, const uint8_t *firmware, size_t firmware_len, int regular_plugin_upto)
{
    logger.printfln_continue("Setting bootloader mode to bootloader.");
    tf_unknown_set_bootloader_mode(bricklet, 0, nullptr);
    logger.printfln_continue("Waiting for bootloader...");

    if (!wait_for_bootloader_mode(bricklet, 0)) {
        logger.printfln_continue("Timed out, flashing failed");
        return false;
    }

    logger.printfln_continue("Device is in bootloader, flashing...");

    int num_packets = firmware_len / 64;
    int last_packet = 0;
    bool write_footer = false;

    if (regular_plugin_upto >= firmware_len - 64 * 4) {
        last_packet = num_packets;
    } else {
        last_packet = ((regular_plugin_upto / 256) + 1) * 4;
        write_footer = true;
    }

    for (int position = 0; position < last_packet; ++position) {
        int start = position * 64;

        if (tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
            if (tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                logger.printfln_continue("Failed to set firmware pointer to %d", start);
                return false;
            }
        }

        if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
            if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                logger.printfln_continue("Failed to write firmware at %d", start);
                return false;
            }
        }
    }

    if (write_footer) {
        for (int position = num_packets - 4; position < num_packets; ++position) {
            int start = position * 64;

            if (tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                if (tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                    logger.printfln_continue("(Footer) Failed to set firmware pointer to %d", start);
                    return false;
                }
            }

            if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                if (tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                    logger.printfln_continue("(Footer) Failed to write firmware at %d", start);
                    return false;
                }
            }
        }
    }

    logger.printfln_continue("Device flashed successfully.");

    return true;
}

static bool flash_firmware(TF_Unknown *bricklet, const uint8_t *firmware, size_t firmware_len)
{
    int regular_plugin_upto = -1;

    for (int i = firmware_len - 13; i >= 4; --i) {
        if (firmware[i] == 0x12
         && firmware[i - 1] == 0x34
         && firmware[i - 2] == 0x56
         && firmware[i - 3] == 0x78) {
             regular_plugin_upto = i;
             break;
         }
    }

    if (regular_plugin_upto == -1) {
        logger.printfln_continue("Firmware end marker not found. Is this a valid firmware?");
        return false;
    }

    if (!flash_plugin(bricklet, firmware, firmware_len, regular_plugin_upto)) {
        return false;
    }

    logger.printfln_continue("Setting bootloader mode to firmware.");

    uint8_t ret_status = 0;

    tf_unknown_set_bootloader_mode(bricklet, 1, &ret_status);

    if (ret_status != 0 && ret_status != 2) {
        logger.printfln_continue("Failed to set bootloader mode to firmware. status %d.", ret_status);

        if (ret_status != 5) {
            return false;
        }

        logger.printfln_continue("Status is 5, retrying.");

        if (!flash_plugin(bricklet, firmware, firmware_len, firmware_len)) {
            return false;
        }

        ret_status = 0;

        logger.printfln_continue("Setting bootloader mode to firmware.");
        tf_unknown_set_bootloader_mode(bricklet, 1, &ret_status);

        if (ret_status != 0 && ret_status != 2) {
            logger.printfln_continue("(Second attempt) Failed to set bootloader mode to firmware. status %d.", ret_status);
            return false;
        }
    }

    logger.printfln_continue("Waiting for firmware...");

    if (!wait_for_bootloader_mode(bricklet, 1)) {
        logger.printfln_continue("Timed out, flashing failed");
        return false;
    }

    logger.printfln_continue("Firmware flashed successfully");

    return true;
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
