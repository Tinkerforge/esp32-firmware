#include "Arduino.h"

#include <stdint.h>
#include <string.h>

#include "tools.h"
#include "api.h"
extern API api;

#include "modules.h"

#define BUTTON_MIN_PRESS_THRES 10000
#define BUTTON_STAGE_0_PRESS_THRES 15000
#define BUTTON_STAGE_1_PRESS_THRES 20000
#define BUTTON_STAGE_2_PRESS_THRES 25000
#define BUTTON_MAX_PRESS_THRES 30000

#include "../ac011k_hardware/ac011k_hardware.h"

void evse_v2_button_recovery_handler() {
    uint32_t start = millis();

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BUTTON, INPUT);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, HIGH);

    uint8_t stage = 0;
    bool first = true;
    bool btn = digitalRead(BUTTON);

    while(btn == LOW && !deadline_elapsed(start + BUTTON_MAX_PRESS_THRES)) {
        btn = digitalRead(BUTTON);
 
        if (first) {
            logger.printfln("Button SW3 is pressed. Waiting for release.");
            first = false;
            digitalWrite(GREEN_LED, LOW);
        }
        if (deadline_elapsed(start + BUTTON_MIN_PRESS_THRES) && !deadline_elapsed(start + BUTTON_STAGE_0_PRESS_THRES)) {
            led_blink(GREEN_LED, 200, 1, 0);
            stage = 0;
        }
        if (deadline_elapsed(start + BUTTON_STAGE_0_PRESS_THRES) && !deadline_elapsed(start + BUTTON_STAGE_1_PRESS_THRES)) {
            led_blink(GREEN_LED, 200, 1, 0);
            led_blink(RED_LED, 200, 1, 0);
            stage = 1;
        }
        if (deadline_elapsed(start + BUTTON_STAGE_1_PRESS_THRES) && !deadline_elapsed(start + BUTTON_STAGE_2_PRESS_THRES)) {
            digitalWrite(GREEN_LED, HIGH);
            led_blink(RED_LED, 200, 1, 0);
            stage = 2;
        }
        if (deadline_elapsed(start + BUTTON_MAX_PRESS_THRES)) {
            digitalWrite(RED_LED, HIGH);
            logger.printfln("Button SW3 is pressed for more than %.3f seconds. Assuming this must be an error. Continuing normal boot.", BUTTON_MAX_PRESS_THRES / 1000.0);
            return;
        }
    }
 
    if (!deadline_elapsed(start + 10)) {
        // we considder this as no button press
        return;
    }
 
    if (!deadline_elapsed(start + BUTTON_MIN_PRESS_THRES)) {
        logger.printfln("Button SW3 was pressed for less than %.3f seconds. Continuing normal boot.", BUTTON_MIN_PRESS_THRES / 1000.0);
        return;
    }
 
    logger.printfln("Button SW3 was pressed for %.3f seconds on startup. Starting recovery routine.", (millis() - start) / 1000.0);
 
    /*
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
            api.restorePersistentConfig("users/config", &users.user_config);
            users.user_config.get("http_auth_enabled")->updateBool(false);
            api.writeConfig("users/config", &users.user_config);

            api.removeConfig("ethernet/config");
            api.removeConfig("wifi/sta_config");
            api.removeConfig("wifi/ap_config");
            logger.printfln("Stage 0 done");
            break;
        // Stage 1 - ESP crashed when booting after stage 0. Remove all configuration
        case 1:
            logger.printfln("Running stage 1: Removing configuration but keeping charge log.");
            mount_or_format_spiffs();
            api.removeAllConfig();
            logger.printfln("Stage 1 done");
            break;
        // Stage 2 - ESP still crashed. Format data partition. (This also removes tracked charges and the username file)
        case 2:
            logger.printfln("Running stage 2: Formatting data partition");
            LittleFS.begin(false, "/spiffs", 10, "spiffs");
            LittleFS.format();
            LittleFS.end();
            logger.printfln("Stage 2 done");
            break;
        // Stage 3 - ESP still crashed after formatting the data partition. The firmware is unrecoverably broken. To prevent a fast boot loop, delay here.
        case 3:
            logger.printfln("Running stage 3: Firmware is probably broken. Delaying next crash for a minute.");
            delay(60 * 1000);
            logger.printfln("Stage 3 done");
            break;
    }
    */
}
