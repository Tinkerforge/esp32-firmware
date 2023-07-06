#include "Arduino.h"

#include <stdint.h>
#include <string.h>

#include "bindings/hal_common.h"
#include "bindings/bricklet_evse_v2.h"
#include "modules/esp32_ethernet_brick/hal_arduino_esp32_ethernet_brick/hal_arduino_esp32_ethernet_brick.h"

#include "tools.h"
#include "api.h"

#include "modules.h"

#define BUTTON_MIN_PRESS_THRES 10000
#define BUTTON_MAX_PRESS_THRES 30000
#define BUTTON_IS_PRESSED 0xFFFFFFFF
#define FACTORY_RESET_DATA_MAGIC 0x0BADB007; //stage is written over the first 0. so this counts the number of bad boot(-up)s.

extern TF_HAL hal;

#define GREEN_LED 2
#define BLUE_LED 15

void evse_v2_button_recovery_handler() {
    int result = tf_hal_create(&hal);
    if (result != TF_E_OK)
        return;
    tf_hal_set_timeout(&hal, 100000);

    TF_EVSEV2 evse;
    result = tf_evse_v2_create(&evse, nullptr, &hal);
    if (result != TF_E_OK)
        return;

    uint32_t start = millis();

    pinMode(GREEN_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);

    uint32_t button_press_time = BUTTON_IS_PRESSED;
    bool first = true;
    while(button_press_time == BUTTON_IS_PRESSED && !deadline_elapsed(start + BUTTON_MAX_PRESS_THRES)) {
        // Handle first boot with new firmware (i.e. the firmware supporting get_button_press_boot_time is not flashed yet)
        if (tf_evse_v2_get_button_press_boot_time(&evse, false, &button_press_time) == TF_E_NOT_SUPPORTED) {
            button_press_time = 0;
            break;
        }

        if (first && button_press_time == BUTTON_IS_PRESSED) {
            logger.printfln("Button is pressed. Waiting for release.");
            first = false;
        } else {
            led_blink(BLUE_LED, 200, 1, 0);
            led_blink(GREEN_LED, 200, 1, 0);
        }
    }

    if (button_press_time == 0)
        return;

    if (deadline_elapsed(start + BUTTON_MAX_PRESS_THRES)) {
        logger.printfln("Button is pressed for more than 30 seconds. Assuming charger runs with front plate removed. Continuing normal boot.");
        return;
    }

    if (button_press_time < BUTTON_MIN_PRESS_THRES || button_press_time > BUTTON_MAX_PRESS_THRES){
        logger.printfln("Button was pressed for more %s. Continuing normal boot.", (button_press_time < BUTTON_MIN_PRESS_THRES) ? "less than 10 seconds" : "more than 30 seconds");
        return;
    }

    logger.printfln("Button was pressed for %.3f seconds on EVSE startup. Starting recovery routine.", button_press_time / 1000.0);

    uint8_t stage = 0;

    struct FactoryResetData {
        uint8_t stage:4;
        uint32_t magic:28;
        uint32_t uptime; // only for entropy
        uint16_t padding;
        uint16_t checksum;
    };

    uint8_t buf[63] = {0};
    tf_evse_v2_get_data_storage(&evse, DATA_STORE_PAGE_RECOVERY, buf);

    if (internet_checksum(buf, sizeof(FactoryResetData)) != 0) {
        logger.printfln("Checksum mismatch while reading recovery info from EVSE RAM. Assuming stage 0.");
        stage = 0;
    }
    else {
        FactoryResetData data;
        memcpy(&data, buf, sizeof(data));
        if (data.magic != 0x0BADB007 || data.uptime > 30000 || data.stage > 3)
            stage = 0;
        else
            stage = data.stage;
    }

    logger.printfln("Requested recovery stage %u.", stage);

    if (stage != 3) {
        logger.printfln("Writing request for stage %u into EVSE RAM in case this boot-up fails.", stage + 1);

        FactoryResetData data;
        data.stage = stage + 1;
        data.magic = FACTORY_RESET_DATA_MAGIC;
        data.uptime = millis();
        data.padding = 0;
        data.checksum = 0;
        memcpy(buf, &data, sizeof(data));
        data.checksum = internet_checksum(buf, sizeof(data));

        memcpy(buf, &data, sizeof(data));
        tf_evse_v2_set_data_storage(&evse, DATA_STORE_PAGE_RECOVERY, buf);
    }

    tf_evse_v2_destroy(&evse);
    tf_hal_destroy(&hal);

    switch (stage) {
        // Stage 0 - User can't reach the web interface anymore. Remove network configuration and disable http_auth.
        case 0:
            logger.printfln("Running stage 0: Resetting network configuration and disabling web interface login");

            mount_or_format_spiffs();
            if (api.restorePersistentConfig("users/config", &users.config))
                users.config.get("http_auth_enabled")->updateBool(false);
            api.writeConfig("users/config", &users.config);

            api.removeConfig("ethernet/config");
            api.removeConfig("wifi/sta_config");
            api.removeConfig("wifi/ap_config");
            LittleFS.end();
            logger.printfln("Stage 0 done");
            break;
        // Stage 1 - ESP crashed when booting after stage 0. Remove all configuration
        case 1:
            logger.printfln("Running stage 1: Removing configuration but keeping charge log.");
            mount_or_format_spiffs();
            api.removeAllConfig();
            LittleFS.end();
            logger.printfln("Stage 1 done");
            break;
        // Stage 2 - ESP still crashed. Format data partition. (This also removes tracked charges and the username file)
        case 2:
            logger.printfln("Running stage 2: Formatting data partition");
            mount_or_format_spiffs();
            factory_reset(false);
            logger.printfln("Stage 2 done");
            break;
        // Stage 3 - ESP still crashed after formatting the data partition. The firmware is unrecoverably broken. To prevent a fast boot loop, delay here.
        case 3:
            logger.printfln("Running stage 3: Firmware is probably broken. Delaying next crash for a minute.");
            delay(60 * 1000);
            logger.printfln("Stage 3 done");
            break;
    }
}
