#include "Arduino.h"

#include <stdint.h>
#include <string.h>

#include "tools.h"
#include "api.h"
extern API api;

#include "modules.h"

static const int BUTTON_STAGE_IS_PRESSED = 100;
static const int BUTTON_STAGE_DELETE_NETWORK = 10000;
static const int BUTTON_STAGE_DELETE_CONFIG = 15000;
static const int BUTTON_STAGE_DELETE_ALL = 20000;
static const int BUTTON_STAGE_OVERTIME = 25000;

enum EraseStages {

    doNothingNoPress = 0,
    deleteNetwork = 1,
    deleteConfig = 2,
    deleteAll = 3,
    doNothingWithPress = 4
};

#include "../ac011k_hardware/ac011k_hardware.h"

void evse_v2_button_recovery_handler() {
    uint32_t start = millis();

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BUTTON, INPUT);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, HIGH);

    EraseStages stage = doNothingNoPress;
    bool oneTimePrint = true;
    bool btn = digitalRead(BUTTON);

    while(btn == LOW && !deadline_elapsed(start + BUTTON_STAGE_OVERTIME)) {
        btn = digitalRead(BUTTON);
        
        if (deadline_elapsed(start + BUTTON_STAGE_IS_PRESSED) && !deadline_elapsed(start + BUTTON_STAGE_DELETE_NETWORK)) {
            // log one time, that the button is pressed.
            if (oneTimePrint) {
                logger.printfln("Button SW3 is pressed. Waiting for release.");
                oneTimePrint = false;
                digitalWrite(GREEN_LED, LOW);
            }
            stage = doNothingWithPress;
        }
        if (deadline_elapsed(start + BUTTON_STAGE_DELETE_NETWORK) && !deadline_elapsed(start + BUTTON_STAGE_DELETE_CONFIG)) {
            led_blink(GREEN_LED, 200, 1, 0);
            stage = deleteNetwork;
        }
        if (deadline_elapsed(start + BUTTON_STAGE_DELETE_CONFIG) && !deadline_elapsed(start + BUTTON_STAGE_DELETE_ALL)) {
            led_blink(GREEN_LED, 200, 1, 0);
            led_blink(RED_LED, 200, 1, 0);
            stage = deleteConfig;
        }
        if (deadline_elapsed(start + BUTTON_STAGE_DELETE_ALL) && !deadline_elapsed(start + BUTTON_STAGE_OVERTIME)) {
            digitalWrite(GREEN_LED, HIGH);
            led_blink(RED_LED, 200, 1, 0);
            stage = deleteAll;
        }
    }
    // check overtime exit from loop
    if (deadline_elapsed(start + BUTTON_STAGE_OVERTIME)){
        stage = doNothingWithPress;
    }

    switch (stage) {
        case doNothingNoPress:
            break;
        case doNothingWithPress:
            // switch securly all leds off
            digitalWrite(RED_LED, HIGH);
            digitalWrite(GREEN_LED, HIGH);
            logger.printfln("Button SW3 is pressed for more than %.3f seconds. Assuming this must be an error. Continuing normal boot.", BUTTON_STAGE_OVERTIME / 1000.0);
            break;
        case deleteNetwork:
            logger.printfln("Resetting network configuration and disabling web interface login");

            mount_or_format_spiffs();
            api.restorePersistentConfig("users/config", &users.user_config);
            users.user_config.get("http_auth_enabled")->updateBool(false);
            api.writeConfig("users/config", &users.user_config);

            api.removeConfig("wifi/sta_config");
            api.removeConfig("wifi/ap_config");
            logger.printfln("network reset done");
            break;
        case deleteConfig:
            logger.printfln("Removing configuration but keeping charge log.");
            mount_or_format_spiffs();
            api.removeAllConfig();
            logger.printfln("config reset done");
            break;
        case deleteAll:
            logger.printfln("Formatting data partition");
            LittleFS.begin(false, "/spiffs", 10, "spiffs");
            LittleFS.format();
            LittleFS.end();
            logger.printfln("factory reset done");
            break;
        default:
            logger.printfln("Oh oh! unknown state in preinit.cpp: %d", stage);
            break;
    }
}
