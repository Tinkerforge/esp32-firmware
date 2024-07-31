/* esp32-firmware
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
#include "users.h"

#include <cmath>
#include <memory>
#include <LittleFS.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "digest_auth.h"

#define USERNAME_FILE "/users/all_usernames"

// We have to do access the evse/evse_v2 configs manually
// because a lot of the code runs in setup(), i.e. before APIs
// are registered.
void set_data_storage(uint8_t *buf)
{
    evse_common.set_data_storage(DATA_STORE_PAGE_CHARGE_TRACKER, buf);
}

void get_data_storage(uint8_t *buf)
{
    evse_common.get_data_storage(DATA_STORE_PAGE_CHARGE_TRACKER, buf);
}

void zero_user_slot_info()
{
    uint8_t buf[63] = {0};
    set_data_storage(buf);
}

uint8_t get_charger_state()
{
    return evse_common.get_state().get("charger_state")->asUint();
}

Config *get_user_slot()
{
    return (Config *)evse_common.get_slots().get(CHARGING_SLOT_USER);
}

float get_energy()
{
    float energy = NAN;
    evse_common.get_charger_meter_energy(&energy);
    return energy;
}

#define USER_SLOT_INFO_VERSION 1
struct UserSlotInfo {
    uint16_t checksum;
    uint8_t version;
    uint8_t user_id;
    uint32_t evse_uptime_on_start;
    uint32_t timestamp_minutes;
    float meter_start;
};

uint16_t calc_checksum(const UserSlotInfo &info)
{
    uint32_t float_buf = 0;
    memcpy(&float_buf, &info.meter_start, sizeof(float_buf));

    uint32_t checksum = info.checksum
                      + ((((uint32_t)info.version) << 8) | info.user_id)
                      + (info.evse_uptime_on_start >> 16)
                      + (info.evse_uptime_on_start & 0xFF)
                      + (info.timestamp_minutes >> 16)
                      + (info.timestamp_minutes & 0xFF)
                      + (float_buf >> 16)
                      + (float_buf & 0xFF);

    uint32_t carry = checksum >> 16;
    checksum = (checksum & 0xFFFF) + carry;
    checksum = ~checksum;
    return checksum;
}

void write_user_slot_info(uint8_t user_id, uint32_t evse_uptime, uint32_t timestamp_minutes, float meter_start)
{
    UserSlotInfo info;
    info.checksum = 0;
    info.version = USER_SLOT_INFO_VERSION;
    info.user_id = user_id;
    info.evse_uptime_on_start = evse_uptime;
    info.timestamp_minutes = timestamp_minutes;
    info.meter_start = meter_start;

    info.checksum = calc_checksum(info);

    uint8_t buf[63] = {0};
    memcpy(buf, &info, sizeof(info));
    set_data_storage(buf);
}

bool read_user_slot_info(UserSlotInfo *result)
{
    uint8_t buf[63] = {0};
    get_data_storage(buf);

    memcpy(result, buf, sizeof(UserSlotInfo));
    if (calc_checksum(*result) != 0) {
        logger.printfln("Checksum mismatch!");
        return false;
    }

    if (result->version != USER_SLOT_INFO_VERSION)
        logger.printfln("Version mismatch!");

    return result->version == USER_SLOT_INFO_VERSION;
}

void Users::pre_setup()
{
    config = Config::Object({
        {"users", Config::Array(
            {
                Config::Object({
                    {"id", Config::Uint8(0)},
                    {"roles", Config::Uint32(0xFFFFFFFF)},
                    {"current", Config::Uint16(32000)},
                    {"display_name", Config::Str("Anonymous", 0, USERNAME_LENGTH)},
                    {"username", Config::Str("anonymous", 0, USERNAME_LENGTH)},
                    {"digest_hash", Config::Str("", 0, 32)}
                })
            },
            new Config(Config::Object({
                {"id", Config::Uint8(0)},
                {"roles", Config::Uint32(0)},
                {"current", Config::Uint16(32000)},
                {"display_name", Config::Str("", 0, USERNAME_LENGTH)},
                {"username", Config::Str("", 0, USERNAME_LENGTH)},
                {"digest_hash", Config::Str("", 0, 32)},
            })),
            1, MAX_ACTIVE_USERS,
            Config::type_id<Config::ConfObject>()
        )},
        {"next_user_id", Config::Uint8(0)},
        {"http_auth_enabled", Config::Bool(false)}
    });

    add = ConfigRoot{Config::Object({
        {"id", Config::Uint8(0)},
        {"roles", Config::Uint32(0)},
        {"current", Config::Uint(32000, 0, 32000)},
        {"display_name", Config::Str("", 0, USERNAME_LENGTH)},
        {"username", Config::Str("", 0, USERNAME_LENGTH)},
        {"digest_hash", Config::Str("", 0, 32)},
    }), [this](Config &add, ConfigSource source) -> String {
        if (config.get("next_user_id")->asUint() == 0)
            return "Can't add user. All user IDs in use.";

        if (add.get("id")->asUint() != config.get("next_user_id")->asUint())
            return "Can't add user. Wrong next user ID";

        if (config.get("users")->count() == MAX_ACTIVE_USERS)
            return "Can't add user. Already have the maximum number of active users.";

        for(int i = 0; i < config.get("users")->count(); ++i)
            if (config.get("users")->get(i)->get("username")->asString() == add.get("username")->asString())
                return "Can't add user. A user with this username already exists.";

        {
            char username[33] = {0};
            File f = LittleFS.open(USERNAME_FILE, "r");
            for(size_t i = 0; i < f.size(); i += USERNAME_ENTRY_LENGTH) {
                f.seek(i);
                f.read((uint8_t *) username, USERNAME_LENGTH);
                if (add.get("username")->asString() == username)
                    return "Can't add user. A user with this username already has tracked charges.";
            }
        }

        return "";
    }};
    add.set_permit_null_updates(false);

    modify = ConfigRoot{Config::Object({
        {"id", Config::Uint(256, 0, 256)}, // 256 is used as marker value that the ID was not written
        {"roles", Config::Uint32(0)},
        {"current", Config::Uint(32000 + 1, 0, 32000 + 1)}, // 32000 + 1 is also a marker value
        {"display_name", Config::Str("___MARKER___VALUE___", 0, USERNAME_LENGTH)},
        {"username", Config::Str("___MARKER___VALUE___", 0, USERNAME_LENGTH)},
        {"digest_hash", Config::Str("___MARKER___VALUE___", 0, 32)},
    }), [this](Config &modify, ConfigSource source) -> String {
        auto id = modify.get("id")->asUint();
        auto roles = modify.get("roles")->asUint();
        auto current = modify.get("current")->asUint();
        auto display_name = modify.get("display_name")->asString();
        auto username = modify.get("username")->asString();
        auto digest_hash = modify.get("digest_hash")->asString();

        bool id_passed = id < 256 ;
        bool roles_passed = roles != 0;
        bool current_passed = current < 32000 + 1;
        bool display_name_passed = display_name != "___MARKER___VALUE___";
        bool username_passed = username != "___MARKER___VALUE___";
        bool digest_hash_passed = digest_hash != "___MARKER___VALUE___";

        if (!id_passed)
            return "Can't modify user. User ID is null or missing.";

        // Only allow modification of display name for anonymous.

        if (id == 0) {
            if (username_passed)
                return "Can't modify anonymous user. Username needs to be null or missing.";
            if (digest_hash_passed)
                return "Can't modify anonymous user. Digest_hash needs to be null or missing.";
            if (roles_passed)
                return "Can't modify anonymous user. Roles need to be null or missing.";
            if (current_passed)
                return "Can't modify anonymous user. Current need to be null or missing.";
        }

        Config *user = nullptr;
        for(int i = 0; i < config.get("users")->count(); ++i) {
            if (config.get("users")->get(i)->get("id")->asUint() == id) {
                user = (Config *)config.get("users")->get(i);
                break;
            }
        }

        if (user == nullptr) {
            return "Can't modify user. User with this ID not found.";
        }

        if (username_passed
            && !digest_hash_passed
            && user->get("username")->asString() != username
            && !user->get("digest_hash")->asString().isEmpty()) {
            return "Changing the username without updating the digest hash is not allowed!";
        }

        for(int i = 0; i < config.get("users")->count(); ++i) {
            if (config.get("users")->get(i)->get("id")->asUint() == id)
                continue;

            if (config.get("users")->get(i)->get("username")->asString() == username) {
                return "Can't modify user. Another user with the same username already exists.";
            }
        }

        {
            char other_name[33] = {0};
            File f = LittleFS.open(USERNAME_FILE, "r");
            for(size_t i = 0; i < f.size(); i += USERNAME_ENTRY_LENGTH) {
                if ((i / USERNAME_ENTRY_LENGTH) == id)
                    continue;

                f.seek(i);
                f.read((uint8_t *) other_name, USERNAME_LENGTH);
                if (username == other_name)
                    return "Can't modify user. A user with this username already has tracked charges.";
            }
        }

        if (!roles_passed)
            modify.get("roles")->updateUint(user->get("roles")->asUint());
        if (!current_passed)
            modify.get("current")->updateUint(user->get("current")->asUint());
        if (!display_name_passed)
            modify.get("display_name")->updateString(user->get("display_name")->asString());
        if (!username_passed)
            modify.get("username")->updateString(user->get("username")->asString());
        if (!digest_hash_passed)
            modify.get("digest_hash")->updateString(user->get("digest_hash")->asString());

        return "";
    }};

    remove = ConfigRoot{Config::Object({
        {"id", Config::Uint8(0)}
    }), [this](Config &remove, ConfigSource source) -> String {
        if (remove.get("id")->asUint() == 0)
            return "The anonymous user can't be removed.";

        for (int i = 0; i < config.get("users")->count(); ++i) {
            if (config.get("users")->get(i)->get("id")->asUint() == remove.get("id")->asUint()) {
                return "";
            }
        }

        return "Can't remove user. User with this ID not found.";
    }};

    http_auth_update = ConfigRoot{Config::Object({
        {"enabled", Config::Bool(false)}
    }), [this](Config &update, ConfigSource source) -> String {
        if (!update.get("enabled")->asBool())
            return "";

        for (int i = 0; i < config.get("users")->count(); ++i) {
            if (!config.get("users")->get(i)->get("digest_hash")->asString().isEmpty())
                return "";
        }

        return "Can't enable HTTP authentication if not at least one user with a password is configured!";
    }};
}

void create_username_file()
{
    logger.printfln("Recreating users file");
    File f = LittleFS.open(USERNAME_FILE, "w", true);
    const uint8_t buf[512] = {};

    for (int i = 0; i < MAX_PASSIVE_USERS * USERNAME_ENTRY_LENGTH; i += sizeof(buf))
        f.write(buf, sizeof(buf));
}

void Users::setup()
{
    api.restorePersistentConfig("users/config", &config);

    if (!LittleFS.exists(USERNAME_FILE)) {
        logger.printfln("Username list does not exist! Recreating now.");
        create_username_file();
        for (int i = 0; i < config.get("users")->count(); ++i) {
            Config *user = (Config *)config.get("users")->get(i);
            this->rename_user(user->get("id")->asUint(), user->get("username")->asString(), user->get("display_name")->asString());
        }
    }

    // Next user id is 0 if there is no free user left.
    // After a reboot maybe tracked charges were removed.
    if (config.get("next_user_id")->asUint() == 0)
        search_next_free_user();

    Config *user_slot = get_user_slot();
    bool charge_start_tracked = charge_tracker.currentlyCharging();
    bool charging = get_charger_state() == 2 || get_charger_state() == 3
                    || (user_slot->get("active")->asBool() && user_slot->get("max_current")->asUint() == 32000);

    if (charge_start_tracked && !charging) {
        float override_value = get_energy();
        // The energy value can be NaN if the meter is not readable yet.
        // This will be repaired when starting the next charge.
        this->stop_charging(0, true, override_value);
    }

    if (charging) {
        // If the EVSE is already charging, read back the user slot info, in case the ESP just power cycled.
        UserSlotInfo info;
        bool success = read_user_slot_info(&info);
        if (success) {
            if (!charge_start_tracked) {
                charge_tracker.startCharge(info.timestamp_minutes, info.meter_start, info.user_id, info.evse_uptime_on_start, USERS_AUTH_TYPE_LOST, Config::ConfVariant{});
            } else {
                // Don't track a start, but restore the current_charge API anyway.
                charge_tracker.current_charge.get("user_id")->updateInt(info.user_id);
                charge_tracker.current_charge.get("meter_start")->updateFloat(info.meter_start);
                charge_tracker.current_charge.get("evse_uptime_start")->updateUint(info.evse_uptime_on_start);
                charge_tracker.current_charge.get("timestamp_minutes")->updateUint(info.timestamp_minutes);
                charge_tracker.current_charge.get("authorization_type")->updateUint(USERS_AUTH_TYPE_LOST);
            }
        } else if (!charge_start_tracked)
            this->start_charging(0, 32000, USERS_AUTH_TYPE_NONE, Config::ConfVariant{});
    }

    auto outer_charger_state = get_charger_state();
    task_scheduler.scheduleWithFixedDelay([this, outer_charger_state](){
        static uint8_t last_charger_state = outer_charger_state;

        uint8_t charger_state = get_charger_state();
        if (charger_state == last_charger_state)
            return;

        logger.printfln("Charger state changed from %u to %u", last_charger_state, charger_state);
        last_charger_state = charger_state;

        // stop_charging and start_charging will check
        // if a start/stop was already tracked, so it is safe
        // to call those methods more often than needed.
        switch(charger_state) {
            case CHARGER_STATE_NOT_PLUGGED_IN:
                this->stop_charging(0, true);
                break;
            case CHARGER_STATE_WAITING_FOR_RELEASE:
                break;
            case CHARGER_STATE_READY_TO_CHARGE:
            case CHARGER_STATE_CHARGING:
                if (!get_user_slot()->get("active")->asBool())
                    this->start_charging(0, 32000, USERS_AUTH_TYPE_NONE, Config::ConfVariant{});
                break;
            case CHARGER_STATE_ERROR:
                break;
        }
    }, 1000, 1000);

    initialized = true;

    if (config.get("http_auth_enabled")->asBool()) {
        bool user_with_password_found = false;
        for (int i = 0; i < config.get("users")->count(); ++i) {
            if (!config.get("users")->get(i)->get("digest_hash")->asString().isEmpty()) {
                user_with_password_found = true;
                break;
            }
        }

        if (!user_with_password_found) {
            logger.printfln("Web interface authentication can not be enabled: No user with set password found.");
            return;
        }

        server.onAuthenticate_HTTPThread([this](WebServerRequest req) -> bool {
            String auth = req.header("Authorization");
            if (auth.isEmpty()) {
                return false;
            }

            if (!auth.startsWith("Digest ")) {
                return false;
            }

            auth = auth.substring(7);
            AuthFields fields = parseDigestAuth(auth.c_str());

            bool result = false;

            // If this times out, result stays false.
            task_scheduler.await([this, &req, &fields, &result]() {
                for (int i = 0; i < config.get("users")->count(); ++i) {
                    if (config.get("users")->get(i)->get("username")->asString().equals(fields.username)) {
                        result = checkDigestAuthentication(fields, req.methodString(), fields.username.c_str(), config.get("users")->get(i)->get("digest_hash")->asEphemeralCStr(), nullptr, true, nullptr, nullptr, nullptr); // use of emphemeral C string ok
                        break;
                    }
                }
            });

            return result;
        });

        logger.printfln("Web interface authentication enabled.");
    }
}

void Users::search_next_free_user()
{
    uint8_t user_id = config.get("next_user_id")->asUint();
    uint8_t start_uid = user_id;
    user_id++;
    {
        File f = LittleFS.open(USERNAME_FILE, "r+");
        while (start_uid != user_id) {
            if (user_id == 0)
                user_id++;
            f.seek(user_id * USERNAME_ENTRY_LENGTH, SeekMode::SeekSet);
            char user_name_byte = 0;
            f.readBytes(&user_name_byte, 1);
            if (user_name_byte == '\0')
                break;
            user_id++;
        };
    }
    if (user_id == start_uid)
        user_id = 0;

    config.get("next_user_id")->updateUint(user_id);
}

size_t Users::get_display_name(uint8_t user_id, char *ret_buf)
{
    for (auto &cfg : config.get("users")) {
        if (cfg.get("id")->asUint() == user_id) {
            const String &s = cfg.get("display_name")->asString();
            strncpy(ret_buf, s.c_str(), 32);
            return min(s.length(), 32u);
        }
    }
    File f = LittleFS.open(USERNAME_FILE, "r");
    f.seek(user_id * USERNAME_ENTRY_LENGTH + USERNAME_LENGTH, SeekMode::SeekSet);
    f.read((uint8_t *)ret_buf, DISPLAY_NAME_LENGTH);
    return strnlen(ret_buf, 32);
}

bool Users::is_user_configured(uint8_t user_id)
{
    for (auto &cfg : config.get("users"))
        if (cfg.get("id")->asUint() == user_id)
            return true;

    return false;
}

static void check_waiting_for_start()
{
    const Config *user_slot = (const Config *)api.getState("evse/slots", false)->get(CHARGING_SLOT_USER);

    if (!user_slot->get("active")->asBool())
        return;

    bool waiting_for_start = (api.getState("evse/state", false)->get("iec61851_state")->asUint() == 1)
                          && (user_slot->get("max_current")->asUint() == 0);

    if (waiting_for_start)
        evse_led.set_module(EvseLed::Blink::Nag, 2000);
}

void Users::register_urls()
{
    // No users (except anonymous) configured: Make sure the EVSE's user slot is disabled.
    bool user_slot = false;

    if (api.hasFeature("evse"))
        user_slot = evse_common.get_slots().get(CHARGING_SLOT_USER)->get("active")->asBool();

    if (config.get("users")->count() <= 1 && user_slot) {
        logger.printfln("User slot enabled, but no users configured. Disabling user slot.");
        api.callCommand("evse/user_enabled_update", Config::ConfUpdateObject{{
            {"enabled", false}
        }});
    }

    api.addCommand("users/modify", &modify, {"digest_hash"}, [this](String &result){
        auto id = modify.get("id")->asUint();

        Config *user = nullptr;
        for(int i = 0; i < config.get("users")->count(); ++i) {
            if (config.get("users")->get(i)->get("id")->asUint() == id) {
                user = (Config *)config.get("users")->get(i);
                break;
            }
        }

        // Validity was already checked, but we have to search the user config anyway.
        if (user == nullptr) {
            result = "Can't modify user. User with this ID not found.";
        }

        user->get("roles")->updateUint(modify.get("roles")->asUint());
        user->get("current")->updateUint(modify.get("current")->asUint());
        bool display_name_changed = user->get("display_name")->updateString(modify.get("display_name")->asString());
        bool username_changed = user->get("username")->updateString(modify.get("username")->asString());
        user->get("digest_hash")->updateString(modify.get("digest_hash")->asString());

        String err = this->config.validate(ConfigSource::API);
        if (!err.isEmpty())
            result = err;

        API::writeConfig("users/config", &config);

        if (display_name_changed || username_changed)
            this->rename_user(user->get("id")->asUint(), user->get("username")->asString(), user->get("display_name")->asString());

        modify.get("id")->updateUint(256);
        modify.get("roles")->updateUint(0);
        modify.get("current")->updateUint(32000 + 1);
        modify.get("display_name")->updateString("___MARKER___VALUE___");
        modify.get("username")->updateString("___MARKER___VALUE___");
        modify.get("digest_hash")->updateString("___MARKER___VALUE___");
    }, true);

    api.addState("users/config", &config, {"digest_hash"});
    api.addCommand("users/add", &add, {"digest_hash"}, [this](){
        config.get("users")->add();
        Config *user = (Config *)config.get("users")->get(config.get("users")->count() - 1);

        user->get("id")->updateUint(add.get("id")->asUint());
        user->get("roles")->updateUint(add.get("roles")->asUint());
        user->get("current")->updateUint(add.get("current")->asUint());
        user->get("display_name")->updateString(add.get("display_name")->asString());
        user->get("username")->updateString(add.get("username")->asString());
        user->get("digest_hash")->updateString(add.get("digest_hash")->asString());

        search_next_free_user();

        API::writeConfig("users/config", &config);
        this->rename_user(user->get("id")->asUint(), user->get("username")->asString(), user->get("display_name")->asString());
    }, true);

    api.addCommand("users/remove", &remove, {}, [this](){
        int idx = -1;
        for(int i = 0; i < config.get("users")->count(); ++i) {
            if (config.get("users")->get(i)->get("id")->asUint() == remove.get("id")->asUint()) {
                idx = i;
                break;
            }
        }

        if (idx < 0) {
            logger.printfln("Can't remove user. User with this ID not found.");
            return;
        }

        config.get("users")->remove(idx);
        API::writeConfig("users/config", &config);

#if MODULE_NFC_AVAILABLE()
        nfc.remove_user(remove.get("id")->asUint());
#endif

        if (!charge_tracker.is_user_tracked(remove.get("id")->asUint()))
        {
            this->rename_user(remove.get("id")->asUint(), "", "");
            // If this user still has tracked charges, we can't recycle their ID, so it is correct
            // to check this here (and not one level up).
            if (config.get("next_user_id")->asUint() == 0)
            {
                config.get("next_user_id")->updateUint(remove.get("id")->asUint());
                API::writeConfig("users/config", &config);
            }
        }
    }, true);


    api.addCommand("users/http_auth_update", &http_auth_update, {}, [this](){
        bool enable = http_auth_update.get("enabled")->asBool();
        if (!enable)
            server.runInHTTPThread([](void *arg) {
                server.onAuthenticate_HTTPThread([](WebServerRequest req){return true;});
            }, nullptr);

        config.get("http_auth_enabled")->updateBool(enable);
        API::writeConfig("users/config", &config);
    }, false);

    server.on_HTTPThread("/users/all_usernames", HTTP_GET, [this](WebServerRequest request) {
        //std::lock_guard<std::mutex> lock{records_mutex};
        size_t len = MAX_PASSIVE_USERS * USERNAME_ENTRY_LENGTH;
        auto buf = heap_alloc_array<char>(len);
        if (buf == nullptr) {
            return request.send(507);
        }

        size_t read = 0;

        {
            File f = LittleFS.open(USERNAME_FILE, "r");
            read = f.read((uint8_t *)buf.get(), len);
        }

        return request.send(200, "application/octet-stream", buf.get(), read);
    });

#if MODULE_EVSE_LED_AVAILABLE()
    task_scheduler.scheduleWithFixedDelay([](){check_waiting_for_start();}, 1000, 1000);
#endif
}

uint8_t Users::next_user_id()
{
    return this->config.get("next_user_id")->asUint();
}

void Users::rename_user(uint8_t user_id, const String &username, const String &display_name)
{
    char buf[USERNAME_ENTRY_LENGTH] = {0};
    username.toCharArray(buf, USERNAME_LENGTH);
    display_name.toCharArray(buf + USERNAME_LENGTH, DISPLAY_NAME_LENGTH);

    File f = LittleFS.open(USERNAME_FILE, "r+");
    f.seek(user_id * USERNAME_ENTRY_LENGTH, SeekMode::SeekSet);
    f.write((const uint8_t *)buf, USERNAME_ENTRY_LENGTH);
}

void Users::remove_from_username_file(uint8_t user_id)
{
    Config *users = (Config *)config.get("users");
    for (int i = 0; i < users->count(); ++i) {
        if (users->get(i)->get("id")->asUint() == user_id) {
            return;
        }
    }

    this->rename_user(user_id, "", "");
    if (config.get("next_user_id")->asUint() == 0) {
        config.get("next_user_id")->updateUint(user_id);
        API::writeConfig("users/config", &config);
    }
}

// Only returns true if the triggered action was a charge start.
bool Users::trigger_charge_action(uint8_t user_id, uint8_t auth_type, Config::ConfVariant auth_info, int action, micros_t deadtime_post_stop, micros_t deadtime_post_start)
{
    bool user_enabled = get_user_slot()->get("active")->asBool();
    if (!user_enabled)
        return false;
    // This is called whenever a user wants to trigger a charge action.
    // I.e. when holding an NFC tag at the box or when calling the start_charging API

    uint16_t current_limit = 0;
    Config *users = (Config *)config.get("users");
    for (int i = 0; i < users->count(); ++i) {
        if (users->get(i)->get("id")->asUint() != user_id)
            continue;

        current_limit = users->get(i)->get("current")->asUint();
    }

    if (current_limit == 0) {
        logger.printfln("Unknown user with ID %u.", user_id);
        return false;
    }

    uint8_t iec_state = evse_common.get_state().get("iec61851_state")->asUint();
    uint32_t tscs = evse_common.get_low_level_state().get("time_since_state_change")->asUint();

    switch (iec_state) {
        case IEC_STATE_B: // State B: The user wants to start charging. If we already have a tracked charge, stop charging to allow switching to another user.
            if (charge_tracker.currentlyCharging()) {
                if ((action == TRIGGER_CHARGE_ANY || action == TRIGGER_CHARGE_STOP) && deadline_elapsed(last_charge_action_triggered + deadtime_post_start))
                    this->stop_charging(user_id, false);
                return false;
            }
            if ((action == TRIGGER_CHARGE_ANY || action == TRIGGER_CHARGE_START) && deadline_elapsed(last_charge_action_triggered + deadtime_post_stop))
                return this->start_charging(user_id, current_limit, auth_type, auth_info);
            return false;
        case IEC_STATE_C: // State C: The user wants to stop charging.
            // Debounce here a bit, an impatient user can otherwise accidentially trigger a stop if a start_charging takes too long.
            if (tscs > 3000 && (action == TRIGGER_CHARGE_ANY || action == TRIGGER_CHARGE_STOP) && deadline_elapsed(last_charge_action_triggered + deadtime_post_start))
                this->stop_charging(user_id, false);
            return false;
        default: //Don't do anything in state A, D, and E/F
            break;
    }
    return false;
}

void Users::remove_username_file()
{
    if (LittleFS.exists(USERNAME_FILE))
        LittleFS.remove(USERNAME_FILE);
}

bool Users::start_charging(uint8_t user_id, uint16_t current_limit, uint8_t auth_type, Config::ConfVariant auth_info)
{
    last_charge_action_triggered = now_us();

    if (charge_tracker.currentlyCharging())
        return false;

    uint32_t evse_uptime = evse_common.get_low_level_state().get("uptime")->asUint();
    float meter_start = get_energy();
    uint32_t timestamp = timestamp_minutes();

    if (!charge_tracker.startCharge(timestamp, meter_start, user_id, evse_uptime, auth_type, auth_info))
        return false;
    write_user_slot_info(user_id, evse_uptime, timestamp, meter_start);
    evse_common.set_user_current(current_limit);

    return true;
}

bool Users::stop_charging(uint8_t user_id, bool force, float meter_abs)
{
    last_charge_action_triggered = now_us();

    if (charge_tracker.currentlyCharging()) {
        UserSlotInfo info;
        bool success = read_user_slot_info(&info);
        // If reading the user slot info failed, we don't know which user started this charge anymore.
        // This should only happen if the EVSE power-cycles, however on a power-cycle any running charge
        // should be aborted. It is safe to allow tracking a charge end in this case for any authorized card,
        // as this should never happen anyway.
        // Allow forcing the endCharge tracking. This is necessary in the case that the car was disconnected.
        // The user is then authorized at the other end of the charging cable.
        if (!force && success && info.user_id != user_id)
            return false;

        uint32_t charge_duration = 0;
        if (success) {
            uint32_t now_seconds = evse_common.get_low_level_state().get("uptime")->asUint() / 1000;
            uint32_t start_seconds = info.evse_uptime_on_start / 1000;
            if (now_seconds < start_seconds) {
                now_seconds += (0xFFFFFFFF / 1000);
            }
            charge_duration = now_seconds - start_seconds;
        }

        if (meter_abs)
            charge_tracker.endCharge(charge_duration, meter_abs);
        else
            charge_tracker.endCharge(charge_duration, get_energy());
    }

    zero_user_slot_info();
    evse_common.set_user_current(0);

    return true;
}
