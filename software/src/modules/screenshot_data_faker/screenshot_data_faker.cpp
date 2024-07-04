/* esp32-firmware
 * Copyright (C) 2022 Erik Fleckstein <erik@tinkerforge.com>
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

#include "screenshot_data_faker.h"

#include <LittleFS.h>

#include "module_dependencies.h"
#include "build.h"

struct [[gnu::packed]] ChargeStart {
    uint32_t timestamp_minutes = 0;
    float meter_start = 0.0f;
    uint8_t user_id = 0;
};

struct [[gnu::packed]] ChargeEnd {
    uint32_t charge_duration : 24;
    float meter_end = 0.0f;
};

ConfigRoot values;

void ScreenshotDataFaker::pre_setup()
{
    values = Config::Object({
        {"power", Config::Float(0.0)},
        {"energy_rel", Config::Float(123.456)},
        {"energy_abs", Config::Float(123.456)},
    });
}

int16_t meter_history[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,847,10481,10482,10457,10428,10418,10377,10366,10338,10338,10207,10154,10158,10158,10154,10144,10114,10152,10173,10185,10189,10197,10185,10172,10166,10168,10207,10219,10174,10136,10163,10152,10112,10051,10036,10025,10009,10027,10051,10031,10038,10017,10051,8671,3702,8513,10001,10016,10022,9999,10103,10211,10159,10108,10129,10167,10170,10170,10191,10211,10228,10246,10229,10240,8144,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,18233,20072,20096,19955,19918,19907,19879,19909,19963,19929,19920,19884,19954,19950,20010,19786,19825,19870,19865,19804,17486,16559,16266,12921,9966,7637,5974,4722,3780,3197,2635,2363,1825,1827,202,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const char *user_config = "{\"users\":[{\"id\":0,\"roles\":4294967295,\"current\":32000,\"display_name\":\"Anonymous\",\"username\":\"anonymous\",\"digest_hash\":\"\"},{\"id\":1,\"roles\":65535,\"current\":32000,\"display_name\":\"Herr von und zu Testuser\",\"username\":\"testuser\",\"digest_hash\":\"abc123\"},{\"id\":2,\"roles\":65535,\"current\":32000,\"display_name\":\"Baroness Blahington\",\"username\":\"testuser2\",\"digest_hash\":\"\"},{\"id\":3,\"roles\":65535,\"current\":16000,\"display_name\":\"Earl of Blahyton\",\"username\":\"testuser3\",\"digest_hash\":\"\"}],\"next_user_id\":4,\"http_auth_enabled\":false}";
const char *nfc_config = "{\"authorized_tags\":[{\"user_id\":1,\"tag_type\":2,\"tag_id\":\"7F:42:23:7F\"},{\"user_id\":2,\"tag_type\":1,\"tag_id\":\"DE:AD:BE:EF\"}]}";
const char *charge_manager_config = "{\"enable_charge_manager\":true,\"enable_watchdog\":false,\"default_available_current\":32000,\"maximum_available_current\":32000,\"minimum_current\":6000,\"verbose\":false,\"chargers\":[{\"host\":\"127.0.0.1\",\"name\":\"warp2-dev-box\"},{\"host\":\"192.168.142.142\",\"name\":\"warp2-emu1\"},{\"host\":\"192.168.142.143\",\"name\":\"warp2-emu2\"},{\"host\":\"192.168.142.144\",\"name\":\"warp2-emu3\"}]}";
const char *network_config = "{\"hostname\":\"warp2-dev-box\",\"enable_mdns\":true}";
const char *wifi_ap_config = "{\"enable_ap\":true,\"ap_fallback_only\":false,\"ssid\":\"warp2-dev-box\",\"hide_ssid\":false,\"passphrase\":null,\"channel\":1,\"ip\":\"10.0.0.1\",\"gateway\":\"10.0.0.1\",\"subnet\":\"255.255.255.0\"}";
const char *mqtt_config = "{\"enable_mqtt\":false,\"broker_host\":\"\",\"broker_port\":1883,\"broker_username\":\"\",\"broker_password\":\"\",\"global_topic_prefix\":\"warp2/dev-box\",\"client_name\":\"warp2-dev-box\",\"interval\":1}";
const char *charge_tracker_config = "{\"electricity_price\": 3401}";

void ScreenshotDataFaker::setup()
{
    LittleFS.open("/config/users_config", "w").write((const uint8_t *)user_config, strlen(user_config));
    LittleFS.open("/config/nfc_config", "w").write((const uint8_t *)nfc_config, strlen(nfc_config));
    LittleFS.open("/config/charge_manager_config", "w").write((const uint8_t *)charge_manager_config, strlen(charge_manager_config));
    LittleFS.open("/config/network_config", "w").write((const uint8_t *)network_config, strlen(network_config));
    LittleFS.open("/config/wifi_ap_config", "w").write((const uint8_t *)wifi_ap_config, strlen(wifi_ap_config));
    LittleFS.open("/config/mqtt_config", "w").write((const uint8_t *)mqtt_config, strlen(mqtt_config));
#ifdef SCREENSHOT_DATA_FAKER_PRO
    LittleFS.open("/config/charge_tracker_config", "w").write((const uint8_t *)charge_tracker_config, strlen(charge_tracker_config));
#endif

    {
        LittleFS.mkdir("/charge-records");
        File file = LittleFS.open("/charge-records/charge-record-1.bin", "w");
        ChargeStart cs;
        cs.timestamp_minutes = (build_timestamp() / 60) - 1540;
#ifdef SCREENSHOT_DATA_FAKER_PRO
        cs.meter_start = 0;
#else
        cs.meter_start = NAN;
#endif
        cs.user_id = 1;

        ChargeEnd ce;
        ce.charge_duration = 256 * 60 + 27;
#ifdef SCREENSHOT_DATA_FAKER_PRO
        ce.meter_end = 36.913f;
#else
        ce.meter_end = NAN;
#endif

        uint8_t buf[sizeof(ChargeStart)] = {0};
        memcpy(buf, &cs, sizeof(cs));
        file.write(buf, sizeof(cs));

        uint8_t buf2[sizeof(ChargeEnd)] = {0};
        memcpy(buf2, &ce, sizeof(ce));
        file.write(buf2, sizeof(ce));

        cs.timestamp_minutes = (build_timestamp() / 60) - 272;
#ifdef SCREENSHOT_DATA_FAKER_PRO
        cs.meter_start = ce.meter_end;
#else
        cs.meter_start = NAN;
#endif
        cs.user_id = 2;

        ce.charge_duration = 9727;
#ifdef SCREENSHOT_DATA_FAKER_PRO
        ce.meter_end += 33.610f;
#else
        ce.meter_end = NAN;
#endif

        memset(buf, 0, sizeof(ChargeStart));
        memset(buf2, 0, sizeof(ChargeEnd));
        memcpy(buf, &cs, sizeof(cs));
        file.write(buf, sizeof(cs));

        memcpy(buf2, &ce, sizeof(ce));
        file.write(buf2, sizeof(ce));
    }

    initialized = true;
}

void ScreenshotDataFaker::register_urls()
{
    api.callCommand("evse/user_enabled_update", Config::ConfUpdateObject{{
        {"enabled", true}
    }});

    api.callCommand("evse/management_enabled_update", Config::ConfUpdateObject{{
        {"enabled", true}
    }});

#ifdef SCREENSHOT_DATA_FAKER_PRO
    meters.meter_slots[0].power_history.history.clear();
    for (int i = 0; i < ARRAY_SIZE(meter_history); ++i)
        meters.meter_slots[0].power_history.history.push(meter_history[i]);
#endif
}
