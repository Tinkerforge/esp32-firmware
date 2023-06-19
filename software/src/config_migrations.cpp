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

#include "config_migrations.h"

#include <vector>
#include <unistd.h>

#include <LittleFS.h>

#include <ArduinoJson.h>

#include "api.h"
#include "build.h"
#include "config.h"
#include "digest_auth.h"
#include "task_scheduler.h"
#include "tools.h"

struct ConfigMigration {
    const int major, minor, patch;
    void (*const fn)(void);
};

ATTRIBUTE_UNUSED
static bool read_config_file(const char *config, JsonDocument &json)
{
    String s = config;
    s.replace('/', '_');
    String filename = "/migration/" + s;

    json.clear();

    if (!LittleFS.exists(filename)) {
        logger.printfln("Skipping migration of %s: File %s not found", config, filename.c_str());
        return false;
    }

    File file = LittleFS.open(filename, "r");
    auto error = deserializeJson(json, file);
    file.close();

    if (error) {
        logger.printfln("Skipping migration of %s: JSON-deserialization failed with %s", config, error.c_str());
        return false;
    }

    return true;
}

ATTRIBUTE_UNUSED
static void write_config_file(const char *config, JsonDocument &json)
{
    String s = config;
    s.replace('/', '_');
    String filename = "/migration/" + s;

    File file = LittleFS.open(filename, "w");
    serializeJson(json, file);
    file.close();
}

ATTRIBUTE_UNUSED
static void delete_config_file(const char *config)
{
    String s = config;
    s.replace('/', '_');
    String filename = "/migration/" + s;

    LittleFS.remove(filename);
}

ATTRIBUTE_UNUSED
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

static const ConfigMigration migrations[] = {
#if defined(BUILD_NAME_WARP)
    {
        // WARP1 1.3.0 changes
        // - Renamed xyz.json.tmp to .xyz
        // - Renamed xyz.json to xyz
        1, 3, 0,
        [](){
            for_file_in("/migration", [](File *file){
                String path = file->path();
                file->close();
                if (path.endsWith(".tmp"))
                    LittleFS.remove(path);

                if (path.endsWith(".json"))
                    LittleFS.rename(path, path.substring(0, path.length() - 5));
                return true;
            });
        }
    },
#endif

#if defined(BUILD_NAME_WARP) || defined(BUILD_NAME_WARP2)
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

                            Config *button_cfg = api.getState("evse/button_configuration");
                            if (button_cfg == nullptr)
                                return;

                            uint8_t new_button_cfg = api.getState("evse/button_configuration")->get("button")->asUint();
                            if (disable_button)
                                new_button_cfg &= ~0x02;

                            api.callCommand("evse/button_configuration_update", Config::ConfUpdateObject{{
                                {"button", (uint32_t)new_button_cfg}
                            }});
                        }, 1000);
                    }, 0);
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
#endif

#if defined(BUILD_NAME_ENERGY_MANAGER)
    {
        1, 0, 2,
        // 1.0.2 changes
        // - Disable new automatic minimum current setting in charge manager if uset has a non-default minimum current set.
        [](){
            migrate_charge_manager_minimum_current();
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
            size_t read = source->read(buf, sizeof(buf) / sizeof(buf[0]));
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
    size_t migration_count = sizeof(migrations) / sizeof(migrations[0]);

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
    uint8_t major, minor, patch;
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

        major = (uint8_t)v.substring(0, first_dot).toInt();
        minor = (uint8_t)v.substring(first_dot + 1, second_dot).toInt();
        patch = (uint8_t)v.substring(second_dot + 1).toInt();
    } else if (migration_count > 0) {
        auto &last_mig = migrations[migration_count - 1];
        major = last_mig.major;
        minor = last_mig.minor;
        patch = last_mig.patch;

        write_version_file = true;
    } else {
        major = OLDEST_VERSION_MAJOR;
        minor = OLDEST_VERSION_MINOR;
        patch = OLDEST_VERSION_PATCH;

        write_version_file = true;
    }

    if (!config_type.isEmpty() && !config_type.equals(BUILD_CONFIG_TYPE)) {
        logger.printfln("Config type mismatch: firmware expects '%s' but '%s' found in flash. Expect config problems.", BUILD_CONFIG_TYPE, config_type.c_str());
        return;
    }

    bool first = true;
    for (int i = 0; i < migration_count; ++i) {
        auto &mig = migrations[i];

        bool have_to_migrate = (major < mig.major)|| (major == mig.major && minor < mig.minor) || (major == mig.major && minor == mig.minor && patch < mig.patch);
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

        logger.printfln("Migrating config from %d.%d.%d to %d.%d.%d", major, minor, patch, mig.major, mig.minor, mig.patch);
        mig.fn();

        write_version_file = true;
        migrations_executed = true;

        major = mig.major;
        minor = mig.minor;
        patch = mig.patch;
    }

    if (!write_version_file)
        return;

    File file = LittleFS.open(migrations_executed ? "/migration/version" : "/config/version", "w");

    file.printf("{\"spiffs\": \"%u.%u.%u\", \"config_type\": \"%s\"}", major, minor, patch, BUILD_CONFIG_TYPE);
    file.close();

    if (!migrations_executed)
        return;

    remove_directory("/config");
    LittleFS.rename("/migration", "/config");
}
