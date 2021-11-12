/* esp32-lib
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

#include "tools.h"

#include "bindings/errors.h"

#include <Arduino.h>

#include "SPIFFS.h"
#include "esp_spiffs.h"

#include <soc/efuse_reg.h>
#include "bindings/base58.h"
#include "bindings/bricklet_unknown.h"
#include "event_log.h"

extern EventLog logger;

bool deadline_elapsed(uint32_t deadline_ms) {
    uint32_t now = millis();

    return ((uint32_t)(now - deadline_ms)) < (UINT32_MAX / 2);
}

bool find_uid_by_did(TF_HalContext *hal, uint16_t device_id, char uid[7]) {
    char pos;
    uint16_t did;
    for (size_t i = 0; tf_hal_get_device_info(hal, i, uid, &pos, &did) == TF_E_OK; ++i) {
        if (did == device_id) {
            return true;
        }
    }
    return false;
}

bool find_uid_by_did_at_port(TF_HalContext *hal, uint16_t device_id, char port, char uid[7]) {
    char pos;
    uint16_t did;
    for (size_t i = 0; tf_hal_get_device_info(hal, i, uid, &pos, &did) == TF_E_OK; ++i) {
        if (did == device_id && port == pos) {
            return true;
        }
    }
    return false;
}

String update_config(Config &cfg, String config_name, JsonVariant &json) {
    String error = cfg.update_from_json(json);

    String tmp_path = String("/") + config_name + ".json.tmp";
    String path = String("/") + config_name + ".json";

    if (error == "") {
        if (SPIFFS.exists(tmp_path))
            SPIFFS.remove(tmp_path);

        File file = SPIFFS.open(tmp_path, "w");
        cfg.save_to_file(file);
        file.close();

        if (SPIFFS.exists(path))
            SPIFFS.remove(path);

        SPIFFS.rename(tmp_path, path);
    } else {
        logger.printfln("Failed to update %s: %s", path.c_str(), error.c_str());
    }
    return error;
}


void read_efuses(uint32_t *ret_uid_numeric, char *ret_uid_string, char *ret_passphrase_string) {
    uint32_t blocks[8] = {0};

    for(int32_t block3Address=EFUSE_BLK3_RDATA0_REG, i = 0; block3Address<=EFUSE_BLK3_RDATA7_REG; block3Address+=4, ++i)
    {
        blocks[i] = REG_GET_FIELD(block3Address, EFUSE_BLK3_DOUT0);
    }

    uint32_t passphrase[4] = {0};
    uint32_t uid = 0;

    /*
    EFUSE_BLK_3 is 256 bit (32 byte, 8 blocks) long and organized as follows:
    block 0:
        Custom MAC CRC + MAC bytes 0 to 2
    block 1:
        Custom MAC bytes 3 to 5
        byte 3 - Wifi passphrase chunk 0 byte 0
    block 2:
        byte 0 - Wifi passphrase chunk 0 byte 1
        byte 1 - Wifi passphrase chunk 0 byte 2
        byte 2 - Wifi passphrase chunk 1 byte 0
        byte 3 - Wifi passphrase chunk 1 byte 1
    block 3:
        ADC 1 calibration data
    block 4:
        ADC 2 calibration data
    block 5:
        byte 0 - Wifi passphrase chunk 1 byte 2
        byte 1 - Wifi passphrase chunk 2 byte 0
        byte 2 - Wifi passphrase chunk 2 byte 1
        byte 3 - Custom MAC Version
    block 6:
        byte 0 - Wifi passphrase chunk 2 byte 2
        byte 1 - Wifi passphrase chunk 3 byte 0
        byte 2 - Wifi passphrase chunk 3 byte 1
        byte 3 - Wifi passphrase chunk 3 byte 2
    block 7:
        UID
    */

    passphrase[0] = ((blocks[1] & 0xFF000000) >> 24) | ((blocks[2] & 0x0000FFFF) << 8);
    passphrase[1] = ((blocks[2] & 0xFFFF0000) >> 16) | ((blocks[5] & 0x000000FF) << 16);
    passphrase[2] = ((blocks[5] & 0x00FFFF00) >> 8)  | ((blocks[6] & 0x000000FF) << 16);
    passphrase[3] =  (blocks[6] & 0xFFFFFF00) >> 8;
    uid = blocks[7];

    char buf[7] = {0};
    for(int i = 0; i < 4; ++i) {
        if(i != 0)
            ret_passphrase_string[i * 5 - 1] = '-';

        tf_base58_encode(passphrase[i], buf);
        if(strnlen(buf, sizeof(buf)/sizeof(buf[0])) != 4) {
            logger.printfln("efuse error: malformed passphrase!");
        } else {
            memcpy(ret_passphrase_string + i * 5, buf, 4);
        }
    }
    tf_base58_encode(uid, ret_uid_string);
}

int check(int rc,const char *msg) {
    if (rc >= 0)
        return rc;
    logger.printfln("%lu Failed to %s rc: %s", millis(), msg, tf_hal_strerror(rc));
    delay(10);
    return rc;
}

bool mount_or_format_spiffs() {
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 10,
      .format_if_mount_failed = false
    };

    esp_err_t err = esp_vfs_spiffs_register(&conf);
    if(err == ESP_FAIL) {
        logger.printfln("SPIFFS is not formatted. Formatting now. This will take about 30 seconds.");
        SPIFFS.format();
    } else {
        esp_vfs_spiffs_unregister(NULL);
    }

    return SPIFFS.begin(false);
}

String read_or_write_config_version(String &firmware_version) {
    String spiffs_version = firmware_version;

    if (SPIFFS.exists("/spiffs.json")) {
        const size_t capacity = JSON_OBJECT_SIZE(1) + 60;
        StaticJsonDocument<capacity> doc;

        File file = SPIFFS.open("/spiffs.json", "r");
        deserializeJson(doc, file);
        file.close();

        spiffs_version = doc["spiffs"].as<const char*>();
    } else {
        File file = SPIFFS.open("/spiffs.json", "w");
        file.printf("{\"spiffs\": \"%s\"}", firmware_version.c_str());
        file.close();
    }

    return spiffs_version;
}


static bool wait_for_bootloader_mode(TF_Unknown *bricklet, int target_mode) {
    uint8_t mode = 255;
    for(int i = 0; i < 10; ++i) {
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

static bool flash_plugin(TF_Unknown *bricklet, const uint8_t *firmware, size_t firmware_len, int regular_plugin_upto, EventLog *logger) {
    logger->printfln("    Setting bootloader mode to bootloader.");
    tf_unknown_set_bootloader_mode(bricklet, 0, nullptr);
    logger->printfln("    Waiting for bootloader...");
    if(!wait_for_bootloader_mode(bricklet, 0)) {
        logger->printfln("    Timed out, flashing failed");
        return false;
    }
    logger->printfln("    Device is in bootloader, flashing...");

    int num_packets = firmware_len / 64;

    int last_packet = 0;
    bool write_footer = false;
    if (regular_plugin_upto >= firmware_len - 64 * 4) {
        last_packet = num_packets;
    } else {
        last_packet = ((regular_plugin_upto / 256) + 1) * 4;
        write_footer = true;
    }

    for(int position = 0; position < last_packet; ++position) {
        int start = position * 64;
        if(tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
            if(tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                logger->printfln("    Failed to set firmware pointer to %d", start);
                return false;
            }
        }

        if(tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
            if(tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                logger->printfln("    Failed to write firmware at %d", start);
                return false;
            }
        }
    }

    if (write_footer) {
        for(int position = num_packets - 4; position < num_packets; ++position) {
            int start = position * 64;
            if(tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                if(tf_unknown_set_write_firmware_pointer(bricklet, start) != TF_E_OK) {
                    logger->printfln("    (Footer) Failed to set firmware pointer to %d", start);
                    return false;
                }
            }

            if(tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                if(tf_unknown_write_firmware(bricklet, const_cast<uint8_t *>(firmware + start), nullptr) != TF_E_OK) {
                    logger->printfln("    (Footer) Failed to write firmware at %d", start);
                    return false;
                }
            }
        }
    }
    logger->printfln("    Device flashed successfully.");
    return true;
}

static bool flash_firmware(TF_Unknown *bricklet, const uint8_t *firmware, size_t firmware_len, EventLog *logger) {
    int regular_plugin_upto = -1;
    for(int i = firmware_len - 13; i >= 4; --i) {
        if (firmware[i] == 0x12
         && firmware[i - 1] == 0x34
         && firmware[i - 2] == 0x56
         && firmware[i - 3] == 0x78) {
             regular_plugin_upto = i;
             break;
         }
    }

    if (regular_plugin_upto == -1) {
        logger->printfln("    Firmware end marker not found. Is this a valid firmware?");
        return false;
    }

    if(!flash_plugin(bricklet, firmware, firmware_len, regular_plugin_upto, logger)) {
        return false;
    }

    logger->printfln("    Setting bootloader mode to firmware.");
    uint8_t ret_status = 0;
    tf_unknown_set_bootloader_mode(bricklet, 1, &ret_status);
    if (ret_status != 0 && ret_status != 2) {
        logger->printfln("    Failed to set bootloader mode to firmware. status %d.", ret_status);
        if (ret_status != 5) {
            return false;
        }
        logger->printfln("    Status is 5, retrying.");
        if(!flash_plugin(bricklet, firmware, firmware_len, regular_plugin_upto, logger)) {
            return false;
        }

        ret_status = 0;
        logger->printfln("    Setting bootloader mode to firmware.");
        tf_unknown_set_bootloader_mode(bricklet, 1, &ret_status);
        if (ret_status != 0 && ret_status != 2) {
            logger->printfln("    (Second attempt) Failed to set bootloader mode to firmware. status %d.", ret_status);
            return false;
        }
    }
    logger->printfln("    Waiting for firmware...");
    if(!wait_for_bootloader_mode(bricklet, 1)) {
        logger->printfln("    Timed out, flashing failed");
        return false;
    }
    logger->printfln("    Firmware flashed successfully");
    return true;
}

#define FIRMWARE_MAJOR_OFFSET 10
#define FIRMWARE_MINOR_OFFSET 11
#define FIRMWARE_PATCH_OFFSET 12

int ensure_matching_firmware(TF_HalContext *hal, const char *uid, const char* name, const char *purpose, const uint8_t *firmware, size_t firmware_len, EventLog *logger, bool force) {
    TF_Unknown bricklet;

    uint32_t numeric_uid;
    int rc = tf_base58_decode(uid, &numeric_uid);
    if (rc != TF_E_OK) {
        return rc;
    }

    uint8_t port_id;
    uint8_t inventory_index;
    rc = tf_hal_get_port_id(hal, numeric_uid, &port_id, &inventory_index);
    if (rc < 0) {
        return rc;
    }

    auto result = tf_unknown_create(&bricklet, uid, hal, port_id, inventory_index);
    if(result != TF_E_OK) {
        logger->printfln("%s init failed (rc %d). Disabling %s support.", name, result, purpose);
        return -1;
    }

    uint8_t firmware_version[3] = {0};

    result = tf_unknown_get_identity(&bricklet, nullptr, nullptr, nullptr, nullptr, firmware_version, nullptr);
    if(result != TF_E_OK) {
        logger->printfln("%s get identity failed (rc %d). Disabling %s support.", name, result, purpose);
        return -1;
    }

    uint8_t embedded_firmware_version[3] = {
        firmware[firmware_len - FIRMWARE_MAJOR_OFFSET],
        firmware[firmware_len - FIRMWARE_MINOR_OFFSET],
        firmware[firmware_len - FIRMWARE_PATCH_OFFSET],
    };

    bool flash_required = force;
    for(int i = 0; i < 3; ++i) {
        // Intentionally use != here: we also want to downgrade the bricklet firmware if the ESP firmware embeds an older one.
        // This makes sure, that the interfaces fit.
        flash_required |= firmware_version[i] != embedded_firmware_version[i];
    }

    if (flash_required) {
        logger->printfln("%s firmware is %d.%d.%d not the expected %d.%d.%d. Flashing firmware...",
                      name,
                      firmware_version[0], firmware_version[1], firmware_version[2],
                      embedded_firmware_version[0], embedded_firmware_version[1], embedded_firmware_version[2]);
        if(!flash_firmware(&bricklet, firmware, firmware_len, logger)) {
            logger->printfln("%s flashing failed. Disabling %s support.", name, purpose);
            return -1;
        }
    }
    tf_unknown_destroy(&bricklet);
    return 0;
}
