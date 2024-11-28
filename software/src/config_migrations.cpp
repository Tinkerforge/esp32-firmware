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

#define EVENT_LOG_PREFIX "migrations"

#include "config_migrations.h"

#include <vector>
#include <unistd.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "event_log_prefix.h"
#include "main_dependencies.h"
#include "build.h"
#include "config.h"
#include "digest_auth.h"
#include "tools.h"

struct ConfigMigration {
    int major, minor, patch;
    void (*const fn)();

    inline bool version_less_than(const struct ConfigMigration &mig) const {
        return version_less_than(mig.major, mig.minor, mig.patch);
    }

    inline bool version_less_than(uint8_t major, uint8_t minor, uint8_t patch) const {
        return (this->major <  major)
            || (this->major == major && this->minor <  minor)
            || (this->major == major && this->minor == minor && this->patch < patch);
    }
};

[[gnu::unused]]
static bool rename_config_file(const char *config, const char *new_name) {
    String s = config;
    s.replace('/', '_');
    String filename = "/migration/" + s;

    s = new_name;
    s.replace('/', '_');
    String target_filename = "/migration/" + s;

    if (!LittleFS.exists(filename)) {
        logger.printfln("Skipping migration of %s: File %s not found", config, filename.c_str());
        return false;
    }

    LittleFS.rename(filename, target_filename);
    return true;
}

[[gnu::unused]]
static bool read_config_file(const char *config, JsonDocument &json)
{
    String s = config;
    s.replace('/', '_');
    String filename = "/migration/" + s;

    // ArduinoJson will clear the document when calling deserializeJson.
    // Clear it anyway, in case any error happens.
    json.clear();

    if (!LittleFS.exists(filename)) {
        logger.printfln("Skipping migration of %s: File %s not found", config, filename.c_str());
        return false;
    }

    File file = LittleFS.open(filename, "r");
    auto error = deserializeJson(json, file);
    file.close();

    if (error) {
        // Not sure if this is necessary, but the documentation does not tell
        // what the state of a JsonDocument is after deserialization fails.
        json.clear();
        logger.printfln("Skipping migration of %s: JSON-deserialization failed with %s", config, error.c_str());
        return false;
    }

    return true;
}

[[gnu::unused]]
static void write_config_file(const char *config, JsonDocument &json)
{
    String s = config;
    s.replace('/', '_');
    String filename = "/migration/" + s;

    File file = LittleFS.open(filename, "w");
    serializeJson(json, file);
    file.close();
}

[[gnu::unused]]
static void write_config_file(const char *config, const char *content)
{
    String s = config;
    s.replace('/', '_');
    String filename = "/migration/" + s;

    File file = LittleFS.open(filename, "w");
    file.write(reinterpret_cast<const uint8_t *>(content), strlen(content));
    file.close();
}

[[gnu::unused]]
static void delete_config_file(const char *config)
{
    String s = config;
    s.replace('/', '_');
    String filename = "/migration/" + s;

    LittleFS.remove(filename);
}

[[gnu::unused]]
static void migrate_charge_manager_minimum_current()
{
    DynamicJsonDocument json{16384};

    if (read_config_file("charge_manager/config", json)) {
        if (!json.containsKey("minimum_current_auto")) {
            uint32_t minimum_current_old = json["minimum_current"].as<uint32_t>();
            if (minimum_current_old > 6000) {
                json["minimum_current_auto"        ] = false;
                json["minimum_current_1p"          ] = minimum_current_old;
                json["minimum_current_vehicle_type"] = 0;
                write_config_file("charge_manager/config", json);
            }
        } else {
            logger.printfln("Looks like charge_manager/config has already been migrated.");
        }
    }
}

#define KEY_DIRECTORY "/remote-access-keys"
#define WG_KEY_LENGTH 44
[[gnu::unused]]
static inline String get_key_path(uint8_t user_id, uint8_t key_id) {
    return String(KEY_DIRECTORY "/") + user_id + "_" + key_id;
}

[[gnu::unused]]
static void store_key(uint8_t user_id, uint8_t key_id, const char *pri, const char *psk, const char *pub) {
    File f = LittleFS.open(get_key_path(user_id, key_id), "w+");
    // TODO: more robust writing
    f.write((const uint8_t *)pri, WG_KEY_LENGTH);
    f.write((const uint8_t *)psk, WG_KEY_LENGTH);
    f.write((const uint8_t *)pub, WG_KEY_LENGTH);
}

// Don't use LittleFS.[...] directly in migrations if not necessary!
// Prefer to use the functions above.
static const ConfigMigration migrations[] = {
#if BUILD_IS_WARP()
    {
        // WARP1 1.3.0 changes
        // - Renamed xyz.json.tmp to .xyz
        // - Renamed xyz.json to xyz
        1, 3, 0,
        [](){
            for_file_in("/migration", [](File *file){
                String path = file->path();
                file->close();
                // Safe to use LittleFS here: We already know the correct path of the file.
                if (path.endsWith(".tmp"))
                    LittleFS.remove(path);

                if (path.endsWith(".json"))
                    LittleFS.rename(path, path.substring(0, path.length() - 5));
                return true;
            });
        }
    },
#endif

#if BUILD_IS_WARP() || BUILD_IS_WARP2()
    {
        2, 0, 0,
        // 2.0.0 changes
        // - changed IPs from int array to strings in ethernet/config, wifi/sta_config and wifi/ap_config
        // - removed hostname from those configs. Added network/config with the hostname
        // - removed authentication/config; replaced with users/config
        // - nfc/config removed tag names, changed tag_ids to strings, added user_ids
        [](){
            DynamicJsonDocument json{16384};

            const auto ip_to_string = [](JsonVariant ip) {
                String s;
                s += ip[0].as<String>() + '.' + ip[1].as<String>() + '.' + ip[2].as<String>() + '.' + ip[3].as<String>();
                ip.set(s);
            };

            String default_hostname = String(BUILD_HOST_PREFIX) + '-';
            String ethernet_hostname = default_hostname;
            if (read_config_file("ethernet/config", json)) {
                if (json.containsKey("hostname")) {
                    ip_to_string(json["ip"]);
                    ip_to_string(json["gateway"]);
                    ip_to_string(json["subnet"]);
                    ip_to_string(json["dns"]);
                    ip_to_string(json["dns2"]);
                    ethernet_hostname = json["hostname"].as<String>();
                    json.remove("hostname");
                    write_config_file("ethernet/config", json);
                } else {
                    logger.printfln("Looks like ethernet/config has already been migrated.");
                }
            }

            String sta_hostname = default_hostname;
            if (read_config_file("wifi/sta_config", json)) {
                if (json.containsKey("hostname")) {
                    ip_to_string(json["ip"]);
                    ip_to_string(json["gateway"]);
                    ip_to_string(json["subnet"]);
                    ip_to_string(json["dns"]);
                    ip_to_string(json["dns2"]);
                    sta_hostname = json["hostname"].as<String>();
                    json.remove("hostname");
                    write_config_file("wifi/sta_config", json);
                } else {
                    logger.printfln("Looks like wifi/sta_config has already been migrated.");
                }
            }

            String ap_hostname = default_hostname;
            if (read_config_file("wifi/ap_config", json)) {
                if (json.containsKey("hostname")) {
                    ip_to_string(json["ip"]);
                    ip_to_string(json["gateway"]);
                    ip_to_string(json["subnet"]);
                    ap_hostname = json["hostname"].as<String>();
                    json.remove("hostname");
                    write_config_file("wifi/ap_config", json);
                } else {
                    logger.printfln("Looks like wifi/ap_config has already been migrated.");
                }
            }

            // If one or more hostnames where changed, use the changed hostname.
            // Prefer ethernet first, then sta, then ap hostname.
            String new_hostname = default_hostname;
            if (ap_hostname != default_hostname)
                new_hostname = ap_hostname;
            if (sta_hostname != default_hostname)
                new_hostname = sta_hostname;
            if (ethernet_hostname != default_hostname)
                new_hostname = ethernet_hostname;

            if (new_hostname != default_hostname){
                json.to<JsonObject>();
                json["enable_mdns"] = true;
                json["hostname"] = new_hostname;
                write_config_file("network/config", json);
            }

            {
                DynamicJsonDocument users_json{1024};

                users_json.to<JsonObject>();
                auto users = users_json.createNestedArray("users");
                auto user = users.createNestedObject();

                user["id"] = 0;
                user["roles"] = 0xFFFFFFFF;
                user["current"] = 32000;
                user["display_name"] = "Anonymous";
                user["username"] = "anonymous";
                user["digest_hash"] = "";

                users_json["next_user_id"] = 1;
                users_json["http_auth_enabled"] = false;

                if (read_config_file("authentication/config", json)) {
                    bool enable = json["enable_auth"];
                    String username = json["username"];
                    String password = json["password"];

                    users_json["http_auth_enabled"] = enable;

                    if (enable) {
                        user = users.createNestedObject();
                        user["id"] = users_json["next_user_id"];
                        user["roles"] = 0xFFFFFFFF;
                        user["current"] = 32000;
                        user["display_name"] = username;
                        user["username"] = username;
                        user["digest_hash"] = generateDigestHash(username.c_str(), password.c_str(), DEFAULT_REALM);

                        users_json["next_user_id"] = users_json["next_user_id"].as<uint8_t>() + 1;
                    }
                    delete_config_file("authentication/config");
                }

                if (read_config_file("nfc/config", json)) {
                    bool enable_nfc = json["require_tag_to_start"];
                    bool disable_button = json["require_tag_to_stop"];

                    json.remove("require_tag_to_start");
                    json.remove("require_tag_to_stop");

                    for(int i = 0; i < json["authorized_tags"].size(); ++i) {
                        auto tag = json["authorized_tags"][i];

                        user = users.createNestedObject();
                        user["id"] = users_json["next_user_id"];
                        user["roles"] = 0xFFFFFFFF;
                        user["current"] = 32000;
                        user["display_name"] = tag["tag_name"];
                        user["username"] = String("nfc_user_") + (i + 1);
                        user["digest_hash"] = "";

                        users_json["next_user_id"] = users_json["next_user_id"].as<uint8_t>() + 1;

                        tag.remove("tag_name");
                        tag["user_id"] = user["id"];

                        String tag_id = "";
                        for(int j = 0; j < tag["tag_id"].size(); ++j) {
                            uint8_t tag_byte = tag["tag_id"][j].as<uint8_t>();
                            String tag_byte_str = String(tag_byte, 16);
                            if (tag_byte < 0x10)
                                tag_byte_str = "0" + tag_byte_str;
                            tag_byte_str.toUpperCase();
                            tag_id += tag_byte_str;
                            tag_id += ":";
                        }

                        tag_id = tag_id.substring(0, tag_id.length() - 1);
                        tag["tag_id"] = tag_id;
                    }
                    write_config_file("nfc/config", json);

                    // The EVSE is not initialized yet.
                    // We have to make sure the code runs only when the EVSE state was polled once.
                    // To do this we use a task that starts another task:
                    // The outer task will be the first (or one of the first) that run after all
                    // ::setups() etc. are executed. This task then starts the inner task that will
                    // run a second later, i.e. _after_ all tasks that were spawned in one of the
                    // ::setup() methods. The EVSE state polling is one of those tasks.
                    task_scheduler.scheduleOnce([enable_nfc, disable_button](){
                        task_scheduler.scheduleOnce([enable_nfc, disable_button](){
                            api.callCommand("evse/user_enabled_update", Config::ConfUpdateObject{{
                                {"enabled", enable_nfc}
                            }});

                            const Config *button_cfg = api.getState("evse/button_configuration");
                            if (button_cfg == nullptr)
                                return;

                            uint8_t new_button_cfg = button_cfg->get("button")->asUint();
                            if (disable_button)
                                new_button_cfg &= ~0x02;

                            api.callCommand("evse/button_configuration_update", Config::ConfUpdateObject{{
                                {"button", (uint32_t)new_button_cfg}
                            }});
                        }, 1_s);
                    });
                }
                write_config_file("users/config", users_json);
            }
        }
    },
    {
        2, 1, 3,
        // 2.1.3 changes
        // - Disable new automatic minimum current setting in charge manager if uset has a non-default minimum current set.
        [](){
            migrate_charge_manager_minimum_current();
        }
    },
    {
        2, 2, 0,
        // 2.2.0 changes
        // - Add a marker file to continue using EnergyImExSum until the next factory reset or deletion of tracked charges.
        // - Move meter/sdm630_reset to meters/0/sdm630_reset, track total, import and export value on reset
        // - Move meter/last_reset to meters/0/last_reset
        [](){
            // Have to directly use LittleFS.[...] here: charge-records are not moved to /migration
            if (LittleFS.exists("/charge-records")) {
                File f = LittleFS.open("/charge-records/use_imexsum", "w");
                f.write((uint8_t)'T');
            }

            DynamicJsonDocument json{1024};
            DynamicJsonDocument json2{1024};
            if (read_config_file("meter/sdm630_reset", json)) {
                float energy_total = json.as<float>();
                json2["energy_total"] = energy_total;
                json2["energy_import"] = 0.0f;
                json2["energy_export"] = 0.0f;
                write_config_file("meters/0/sdm630_reset", json2);
            }

            rename_config_file("meter/last_reset", "meters/0/last_reset");
        }
    },
#endif

#if BUILD_IS_ENERGY_MANAGER()
    {
        1, 0, 2,
        // 1.0.2 changes
        // - Disable new automatic minimum current setting in charge manager if uset has a non-default minimum current set.
        [](){
            migrate_charge_manager_minimum_current();
        }
    },
    {
        2, 0, 0,
        // 2.0.0 changes
        // - Migrate energy manager rules to automation.
        // - Migrate some settings from Energy Manager config to Power Manager config.
        // - Migrate meter config to new meters framework.
        [] () {
            // - Migrate energy manager rules to automation.
            {
                DynamicJsonDocument json{16384};
                DynamicJsonDocument tasks{16384};

                if (read_config_file("energy_manager/config", json)) {
                    auto config = json.as<JsonObject>();

                    // It seems like there is an error in the != operator overload for ints in arduino json which causes statements that should return false to return true.
                    if (config["relay_config"].as<int>() == 1) {
                        int when = config["relay_rule_when"];
                        DynamicJsonDocument task{16384};
                        auto trigger = task.createNestedArray("trigger");
                        trigger.add(0);
                        auto trigger_config = trigger.createNestedObject();
                        switch (when) {
                            case 0:
                                trigger[0] = 12;
                                trigger_config["state"] = config["relay_rule_is"].as<int>() == 1 ? false : true;
                                break;

                            case 1:
                                trigger[0] = 13;
                                trigger_config["state"] = config["relay_rule_is"].as<int>() == 1 ? false : true;
                                break;

                            case 2:
                                trigger[0] = 14;
                                trigger_config["phase"] = config["relay_rule_is"].as<int>() == 2 ? 1 : 3;
                                break;

                            case 3:
                                trigger[0] = 15;
                                trigger_config["contactor_okay"] = config["relay_rule_is"].as<int>() == 5 ? true : false;
                                break;

                            case 4:
                                trigger[0] = 16;
                                trigger_config["power_available"] = config["relay_rule_is"].as<int>() == 6 ? true : false;
                                break;

                            case 5:
                                trigger[0] = 17;;
                                trigger_config["drawing_power"] = config["relay_rule_is"].as<int>() == 8 ? true : false;
                        }
                        auto action = task.createNestedArray("action");
                        action.add(13);
                        auto action_config = action.createNestedObject();
                        action_config["state"] = true;
                        tasks.add(task);
                    }

                    if (config["input3_rule_then"].as<int>() != 0) {
                        DynamicJsonDocument task{16384};
                        auto trigger = task.createNestedArray("trigger");
                        trigger.add(12);
                        auto trigger_config = trigger.createNestedObject();
                        trigger_config["state"] = config["input3_rule_is"].as<int>() == 0 ? true : false;
                        auto action = task.createNestedArray("action");
                        action.add(0);
                        auto action_config = action.createNestedObject();

                        DynamicJsonDocument reset_task{16384};
                        auto reset_trigger = reset_task.createNestedArray("trigger");
                        reset_trigger.add(12);
                        auto reset_trigger_config = reset_trigger.createNestedObject();
                        reset_trigger_config["state"] = config["input3_rule_is"].as<int>() == 0 ? false : true;
                        auto reset_action = reset_task.createNestedArray("action");
                        reset_action.add(0);
                        auto reset_action_config = reset_action.createNestedObject();

                        bool ignore_high = false;
                        bool ignore_low = false;
                        switch (config["input3_rule_then"].as<int>()) {
                            case 2:
                                action[0] = 15;
                                action_config["slot"] = 0;
                                action_config["block"] = true;
                                reset_action[0] = 15;
                                reset_action_config["slot"] = 0;
                                reset_action_config["block"] = false;
                                break;

                            case 3:
                                action[0] = 14;
                                action_config["current"] = config["input3_rule_then_limit"];
                                reset_action[0] = 14;
                                reset_action_config["current"] = -1;
                                break;

                            case 4:
                            {
                                int on_high = config["input3_rule_then_on_high"];
                                if (on_high == 255) {
                                    ignore_high = true;
                                } else {
                                    action[0] = 12;
                                    action_config["mode"] = on_high;
                                    trigger_config["state"] = true;
                                }
                                int on_low = config["input3_rule_then_on_low"];
                                if (on_low == 255) {
                                    ignore_low = true;
                                } else {
                                    reset_action[0] = 12;
                                    reset_action_config["mode"] = on_low;
                                    reset_trigger_config["state"] = false;
                                }
                            }
                                break;
                        }
                        if (!ignore_high) {
                            tasks.add(task);
                        }
                        if (!ignore_low) {
                            tasks.add(reset_task);
                        }
                    }

                    if (config["input4_rule_then"].as<int>() != 0 && config["input4_rule_then"].as<int>() != 1) {
                        DynamicJsonDocument task{16384};
                        auto trigger = task.createNestedArray("trigger");
                        trigger.add(13);
                        auto trigger_config = trigger.createNestedObject();
                        trigger_config["state"] = config["input4_rule_is"].as<int>() == 0 ? true : false;
                        auto action = task.createNestedArray("action");
                        action.add(0);
                        auto action_config = action.createNestedObject();

                        DynamicJsonDocument reset_task{16384};
                        auto reset_trigger = reset_task.createNestedArray("trigger");
                        reset_trigger.add(13);
                        auto reset_trigger_config = reset_trigger.createNestedObject();
                        reset_trigger_config["state"] = config["input4_rule_is"].as<int>() == 0 ? false : true;
                        auto reset_action = reset_task.createNestedArray("action");
                        reset_action.add(0);
                        auto reset_action_config = reset_action.createNestedObject();

                        bool ignore_high = false;
                        bool ignore_low = false;
                        switch (config["input4_rule_then"].as<int>()) {
                            case 2:
                                action[0] = 15;
                                action_config["slot"] = 0;
                                action_config["block"] = true;
                                reset_action[0] = 15;
                                reset_action_config["slot"] = 0;
                                reset_action_config["block"] = false;
                                break;

                            case 3:
                                action[0] = 14;
                                action_config["current"] = config["input4_rule_then_limit"];
                                reset_action[0] = 14;
                                reset_action_config["current"] = -1;
                                break;

                            case 4:
                            {
                                int on_high = config["input4_rule_then_on_high"];
                                if (on_high == 255) {
                                    ignore_high = true;
                                } else {
                                    action[0] = 12;
                                    action_config["mode"] = on_high;
                                    trigger_config["state"] = true;
                                }
                                int on_low = config["input4_rule_then_on_low"];
                                if (on_low == 255) {
                                    ignore_low = true;
                                } else {
                                    reset_action[0] = 12;
                                    reset_action_config["mode"] = on_low;
                                    reset_trigger_config["state"] = false;
                                }
                            }
                                break;
                        }
                        if (!ignore_high) {
                            tasks.add(task);
                        }
                        if (!ignore_low) {
                            tasks.add(reset_task);
                        }
                    }

                    if (config["auto_reset_mode"].as<bool>()) {
                        DynamicJsonDocument task{16384};
                        auto trigger = task.createNestedArray("trigger");
                        trigger.add(1);
                        auto trigger_config = trigger.createNestedObject();
                        trigger_config["mday"] = -1;
                        trigger_config["mday"] = -1;
                        trigger_config["hour"] = config["auto_reset_time"].as<int>() / 60;
                        trigger_config["minute"] = config["auto_reset_time"].as<int>() % 60;

                        auto action = task.createNestedArray("action");
                        action.add(0);
                        auto action_config = action.createNestedObject();
                        action[0] = 12;
                        action_config["mode"] = 4;
                        tasks.add(task);
                    }

                    DynamicJsonDocument automation_json{16384};
                    if (!read_config_file("automation/config", automation_json)) {
                        automation_json.createNestedArray("tasks");
                    }
                    auto automation_config = automation_json.as<JsonObject>();
                    for (auto task : tasks.as<JsonArray>()) {
                        automation_config["tasks"].add(task);
                    }
                    write_config_file("automation/config", automation_json);

                    config.remove("auto_reset_mode");
                    config.remove("auto_reset_time");
                    config.remove("relay_config");
                    config.remove("relay_rule_when");
                    config.remove("relay_rule_when");
                    config.remove("relay_rule_is");
                    config.remove("input3_rule_then");
                    config.remove("input3_rule_then_limit");
                    config.remove("input3_rule_is");
                    config.remove("input3_rule_then_on_high");
                    config.remove("input3_rule_then_on_low");
                    config.remove("input4_rule_then");
                    config.remove("input4_rule_then_limit");
                    config.remove("input4_rule_is");
                    config.remove("input4_rule_then_on_high");
                    config.remove("input4_rule_then_on_low");

                    write_config_file("energy_manager/config", json);
                }
            }

            // Migrate some settings from Energy Manager config to Power Manager config.
            {
                StaticJsonDocument<384> em_cfg;

                if (read_config_file("energy_manager/config", em_cfg)) {
                    StaticJsonDocument<384> pm_cfg;
                    pm_cfg.to<JsonObject>();

                    pm_cfg["phase_switching_mode"]   = em_cfg["phase_switching_mode"];
                    pm_cfg["excess_charging_enable"] = em_cfg["excess_charging_enable"];
                    pm_cfg["default_mode"]           = em_cfg["default_mode"];
                    pm_cfg["meter_slot_grid_power"]  = em_cfg["meter_slot_grid_power"];
                    pm_cfg["target_power_from_grid"] = em_cfg["target_power_from_grid"];
                    pm_cfg["guaranteed_power"]       = em_cfg["guaranteed_power"];
                    pm_cfg["cloud_filter_mode"]      = em_cfg["cloud_filter_mode"];

                    em_cfg.remove("phase_switching_mode");
                    em_cfg.remove("excess_charging_enable");
                    em_cfg.remove("default_mode");
                    em_cfg.remove("meter_slot_grid_power");
                    em_cfg.remove("target_power_from_grid");
                    em_cfg.remove("guaranteed_power");
                    em_cfg.remove("cloud_filter_mode");

                    write_config_file("energy_manager/config", em_cfg);
                    write_config_file("power_manager/config", pm_cfg);
                }

                rename_config_file("energy_manager/debug_config", "power_manager/debug_config");
            }

            // Migrate meter config to new meters framework.
            {
                const char *old_config_path = "energy_manager/meter_config";
                DynamicJsonDocument old_json{128};

                if (read_config_file(old_config_path, old_json)) {
                    if (old_json.containsKey("meter_source")) {
                        uint32_t meter_source = old_json["meter_source"].as<uint32_t>();
                        if (meter_source == 100) {
                            const char *new_config_str = "[4,{\"display_name\":\"API-Stromzähler\",\"value_ids\":[1,2,3,13,17,21,39,48,57,122,130,138,83,91,99,353,354,355,365,366,367,7,29,33,74,154,115,356,368,364,209,211,273,275,341,388,4,5,6,8,25,369,370,371,377,378,379,375,380,372,373,374,376,213,277,161,177,193,163,179,195,165,181,197,225,241,257,227,243,259,229,245,261,214,210,212]}]";
                            write_config_file("meters/0/config", new_config_str);
                        }
                    }
                    delete_config_file(old_config_path);
                }
            }
        }
    },
    {
        2, 0, 3,
        // 2.0.3 changes
        // - Reset charge manager's requested current margin back to 3A.
        [](){
            DynamicJsonDocument json{16384};

            if (read_config_file("charge_manager/config", json)) {
                if (json.containsKey("requested_current_margin")) {
                    uint32_t requested_current_margin_old = json["requested_current_margin"].as<uint32_t>();
                    if (requested_current_margin_old == 6000) {
                        logger.printfln("CM migration: Resetting requested_current_margin to 3000.");
                        json["requested_current_margin"] = 3000;
                        write_config_file("charge_manager/config", json);
                    } else {
                        logger.printfln("CM migration: Not resetting custom requested_current_margin of %u.", requested_current_margin_old);
                    }
                } else {
                    logger.printfln("CM migration skipped, requested_current_margin not present");
                }
            }
        }
    },
#endif

#if BUILD_IS_WARP() || BUILD_IS_WARP2() || BUILD_IS_WARP3() || BUILD_IS_ENERGY_MANAGER()
    {
        #if BUILD_IS_ENERGY_MANAGER()
        2, 2, 0,
        #elif BUILD_IS_WARP()
        2, 4, 2,
        #else // WARP2, 3
        2, 5, 0,
        #endif
        // Changes
        // - Move remote access keys into separate directory
        // - Rename relay_host_port to relay_port
        [](){
            {
                DynamicJsonDocument cfg{4096};
                if (read_config_file("remote_access/config", cfg)) {
                    cfg["relay_port"] = cfg["relay_host_port"];
                    cfg.remove("relay_host_port");
                    write_config_file("remote_access/config", cfg);
                }
            }

            if (LittleFS.exists(KEY_DIRECTORY)) {
                return;
            }

            DynamicJsonDocument mgmt{4096};
            DynamicJsonDocument conn{4096};
            if (!read_config_file("remote_access/management_connection", mgmt) || !read_config_file("remote_access/remote_connection_config", conn)) {
                return;
            }

            LittleFS.mkdir(KEY_DIRECTORY);

            store_key(0, 0, mgmt["private_key"].as<const char *>(), mgmt["psk"].as<const char *>(), mgmt["remote_public_key"].as<const char *>());
            for(int i = 0; i < 5; ++i) {
                auto key = conn["connections"][i];
                store_key(1, i, key["private_key"].as<const char *>(), key["psk"].as<const char *>(), key["remote_public_key"].as<const char *>());
            }

            delete_config_file("remote_access/management_connection");
            delete_config_file("remote_access/remote_connection_config");
        }
    },
#endif

#if BUILD_IS_WARP() || BUILD_IS_WARP2() || BUILD_IS_WARP3() || BUILD_IS_ENERGY_MANAGER() || BUILD_IS_SMART_ENERGY_BROKER()
    {
        #if BUILD_IS_WARP() || BUILD_IS_WARP2() || BUILD_IS_WARP3()
        2, 6, 2,
        #elif BUILD_IS_ENERGY_MANAGER()
        2, 2, 1,
        #else // TODO: Update SEB firmware below.
        0, 0, 0,
        #endif
        // Changes
        // - PM phase switching modes 1phase, 3phase and PV1+Fast3 have been removed, change to automatic
        [](){
            #if BUILD_IS_SMART_ENERGY_BROKER()
            static_assert(BUILD_VERSION_MAJOR == 1, "Fix migration for SEB");
            static_assert(BUILD_VERSION_MINOR == 0, "Fix migration for SEB");
            static_assert(BUILD_VERSION_PATCH == 0, "Fix migration for SEB");
            #endif
            // - PM phase switching modes 1phase, 3phase and PV1+Fast3 have been removed, change to automatic
            {
                StaticJsonDocument<512> pm_cfg;

                if (read_config_file("power_manager/config", pm_cfg)) {
                    uint32_t phase_switching_mode = pm_cfg["phase_switching_mode"].as<uint32_t>();

                    if (phase_switching_mode != 0 && phase_switching_mode != 3) { // Not automatic or external (EVCC)
                        pm_cfg["phase_switching_mode"] = 0; // Set to automatic
                        write_config_file("power_manager/config", pm_cfg);
                    }
                }
            }
        }
    },
#endif
};

bool prepare_migrations()
{
    /*
        Normal Migrations work as follows:
        M0. Check /config/version to determine necessary migrations
        M1. copy the /config/ folder to /migration/
        M3. run all migrations in /migration/
        M4. increase /migration/version
        M5. rename /migration to /config/

        Interruption check:
        - Use registered migrations to find out minimum required version
        - If /config/version is older:
        - Remove /migration/ if it exists, go to M0
        - If not we are done.
    */

    if (LittleFS.exists("/migration"))
        remove_directory("/migration");

    LittleFS.mkdir("/migration");
    return for_file_in("/config", [](File *source) {
        uint8_t buf[1024] = {0};
        String name = source->name();

        File target = LittleFS.open(String("/migration/") + name, "w");
        while (source->available()) {
            size_t read = source->read(buf, ARRAY_SIZE(buf));
            size_t written = target.write(buf, read);

            if (written != read) {
                logger.printfln("Failed to write file %s: written %u read %u", target.name(), written, read);
                return false;
            }
        }
        return true;
    });
}

void migrate_config()
{
    size_t migration_count = ARRAY_SIZE(migrations);

    if (!LittleFS.exists("/config") && LittleFS.exists("/migration/version")) {
        // The migration is done, we were interrupted while moving over the migrated files to /config.
        remove_directory("/config");
        LittleFS.rename("/migration", "/config");
        return;
    }

    /*
        Pre 2.0.0 configs are just in /
        2.0.0 and higher configs are in /config/
        This requires a "special" migration first:
        Create the config folder, move all configs there, /version last
        (This is the marker that this migration is done)

        Interruption check:
        - Check if /version exists. If yes run again.
        Moving files is atomic (guaranteed by LittleFS),
        so if /version still exists we can just move all files
        into /config/ again.
    */
    if (!LittleFS.exists("/config/version")) {
        logger.printfln("Moving all config files into config folder");
        if (LittleFS.exists("/config"))
            remove_directory("/config");
        LittleFS.mkdir("/config");

        for_file_in("/", [](File *file) {
            String name = file->name();
            file->close();
            if (name != "spiffs.json") // do version last.
                LittleFS.rename(String("/") + name, String("/config/") + name);
            return true;
        });

        if (LittleFS.exists("/spiffs.json"))
            LittleFS.rename("/spiffs.json", "/config/version");
    }

    bool write_version_file = false;
    bool migrations_executed = false;

    String config_type;
    ConfigMigration current{0, 0, 0, nullptr};
    if (LittleFS.exists("/config/version")) {
        StaticJsonDocument<256> doc;
        File f = LittleFS.open("/config/version");
        deserializeJson(doc, f);

        if (doc.containsKey("config_type")) {
            config_type = doc["config_type"].as<String>();
        } else {
            write_version_file = true;
        }

        String v = doc["spiffs"];
        int dash = v.indexOf('-');
        if (dash >= 0) {
            v = v.substring(0, dash);
            write_version_file = true;
        }

        size_t first_dot = v.indexOf('.');
        size_t second_dot = v.indexOf('.', first_dot + 1);

        current.major = (uint8_t)v.substring(0, first_dot).toInt();
        current.minor = (uint8_t)v.substring(first_dot + 1, second_dot).toInt();
        current.patch = (uint8_t)v.substring(second_dot + 1).toInt();
    } else if (migration_count > 0) {
        auto &last_mig = migrations[migration_count - 1];
        current.major = last_mig.major;
        current.minor = last_mig.minor;
        current.patch = last_mig.patch;

        write_version_file = true;
    } else {
        current.major = OLDEST_VERSION_MAJOR;
        current.minor = OLDEST_VERSION_MINOR;
        current.patch = OLDEST_VERSION_PATCH;

        write_version_file = true;
    }

    if (!config_type.isEmpty() && !config_type.equals(BUILD_CONFIG_TYPE)) {
        logger.printfln("Config type mismatch: firmware expects '%s' but '%s' found in flash. Expect config problems.", BUILD_CONFIG_TYPE, config_type.c_str());
        return;
    }

    bool first = true;
    for (int i = 0; i < migration_count; ++i) {
        auto &mig = migrations[i];

        if (i > 0) {
            auto &last_mig = migrations[i - 1];
            if (!last_mig.version_less_than(mig))
                esp_system_abort("Config migration order broken!");
        }

        bool have_to_migrate = current.version_less_than(mig);
        if (!have_to_migrate) {
            continue;
        }

        if (first) {
            logger.printfln("Preparing migrations");
            if (!prepare_migrations()) {
                logger.printfln("Preparing migrations failed");
                return;
            }
            first = false;
        }

        logger.printfln("Migrating config from %d.%d.%d to %d.%d.%d", current.major, current.minor, current.patch, mig.major, mig.minor, mig.patch);
        mig.fn();

        write_version_file = true;
        migrations_executed = true;

        current.major = mig.major;
        current.minor = mig.minor;
        current.patch = mig.patch;
    }

    if (!write_version_file)
        return;

    File file = LittleFS.open(migrations_executed ? "/migration/version" : "/config/version", "w");

    file.printf("{\"spiffs\": \"%u.%u.%u\", \"config_type\": \"%s\"}", current.major, current.minor, current.patch, BUILD_CONFIG_TYPE);
    file.close();

    if (!migrations_executed)
        return;

    remove_directory("/config");
    LittleFS.rename("/migration", "/config");
}
