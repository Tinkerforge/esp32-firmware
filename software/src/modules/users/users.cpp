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

#include "LittleFS.h"

#include "task_scheduler.h"

#include "modules.h"

#include "digest_auth.h"
#include <cmath>

#define USERNAME_LENGTH 32
#define MAX_ACTIVE_USERS 16

extern TaskScheduler task_scheduler;

#define USER_SLOT_INFO_VERSION 1
struct UserSlotInfo {
    uint16_t checksum;
    uint8_t version;
    uint8_t user_id;
    uint32_t evse_uptime_on_start;
    uint32_t timestamp_minutes;
    float meter_start;
};

uint16_t calc_checksum(UserSlotInfo info) {
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

void zero_user_slot_info() {
    uint8_t buf[63] = {0};
    tf_evse_v2_set_data_storage(&evse_v2.device, 0, buf);
}

void write_user_slot_info(uint8_t user_id, uint32_t evse_uptime, uint32_t timestamp_minutes, float meter_start) {
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
    tf_evse_v2_set_data_storage(&evse_v2.device, 0, buf);
}

bool read_user_slot_info(UserSlotInfo *result) {
    uint8_t buf[63] = {0};
    tf_evse_v2_get_data_storage(&evse_v2.device, 0, buf);

    memcpy(result, buf, sizeof(UserSlotInfo));
    if (calc_checksum(*result) != 0) {
        logger.printfln("Checksum mismatch!");
        return false;
    }

    if (result->version != USER_SLOT_INFO_VERSION)
        logger.printfln("Version mismatch!");

    return result->version == USER_SLOT_INFO_VERSION;
}

Users::Users()
{
    user_config = Config::Object({
        {"users", Config::Array(
            {},
            new Config(Config::Object({
                {"id", Config::Uint8(0)},
                {"roles", Config::Uint32(0)},
                {"current", Config::Uint16(32000)},
                {"display_name", Config::Str("", 0, USERNAME_LENGTH)},
                {"username", Config::Str("", 0, USERNAME_LENGTH)},
                {"digest_hash", Config::Str("", 0, 32)},
            })),
            0, MAX_ACTIVE_USERS,
            Config::type_id<Config::ConfObject>()
        )},
        {"next_user_id", Config::Uint8(0)},
        {"http_auth_enabled", Config::Bool(false)}
    });

    add = ConfigRoot(Config::Object({
        {"id", Config::Uint8(0)},
        {"roles", Config::Uint16(0)},
        {"current", Config::Uint16(32000)},
        {"display_name", Config::Str("", 0, USERNAME_LENGTH)},
        {"username", Config::Str("", 0, USERNAME_LENGTH)},
        {"digest_hash", Config::Str("", 0, 32)},
    }), [this](Config &add) -> String {
        if (add.get("id")->asUint() != user_config.get("next_user_id")->asUint())
            return "Can't add user. Wrong next user ID";

        if (user_config.get("users")->count() == MAX_ACTIVE_USERS)
            return "Can't add user. Already have the maximum number of active users.";

        return "";
    });
    add.permit_null_updates = false;

    del = ConfigRoot(Config::Object({
        {"id", Config::Uint8(0)}
    }), [this](Config &del) -> String {
        if (del.get("id")->asUint() == 0)
            return "The anonymous user can't be deleted.";

        for(int i = 0; i < user_config.get("users")->count(); ++i) {
            if (user_config.get("users")->get(i)->get("id")->asUint() == del.get("id")->asUint()) {
                return "";
            }
        }

        return "Can't delete user. User with this ID not found.";
    });

    charge_info = Config::Object({
        {"id", Config::Int16(-1)},
        {"meter_start", Config::Float(0)},
        {"evse_uptime_start", Config::Uint32(0)},
        {"timestamp_minutes", Config::Uint32(0)}
    });

    http_auth_update = ConfigRoot(Config::Object({
        {"enabled", Config::Bool(false)}
    }), [this](Config &update) -> String {
        if (!update.get("enabled")->asBool())
            return String("");

        for(int i = 0; i < user_config.get("users")->count(); ++i) {
            if (user_config.get("users")->get(i)->get("digest_hash")->asString() != "")
                return String("");
        }

        return String("Can't enable HTTP authentication if not at least one user with a password is configured!");
    });
}

void create_username_file() {
    logger.printfln("Recreating users file");
    File f = LittleFS.open("/users/usernames", "w", true);
    const uint8_t buf[512] = {};

    for(int i = 0; i < 256 * USERNAME_LENGTH; i += sizeof(buf))
        f.write(buf, sizeof(buf));
}

void Users::setup()
{
    if (!api.restorePersistentConfig("users/config", &user_config)) {
        create_username_file();

        user_config.get("users")->add();
        Config *user = user_config.get("users")->get(user_config.get("users")->count() - 1);

        user->get("id")->updateUint(0);
        user->get("roles")->updateUint(0xFFFF);
        user->get("display_name")->updateString("Anonymous");
        user->get("username")->updateString("anonymous");
        user->get("digest_hash")->updateString("");

        uint8_t user_id = user_config.get("next_user_id")->asUint();
        ++user_id;
        user_config.get("next_user_id")->updateUint(user_id);

        API::writeConfig("users/config", &user_config);
        this->rename_user(user->get("id")->asUint(), user->get("username")->asCStr());
    }

    if (!LittleFS.exists("/users/usernames")) {
        logger.printfln("Username list does not exist! Recreating now.");
        create_username_file();
    }

    //TODO: make sure usernames are the same in user_config and usernames.bin

    bool charge_start_tracked = charge_tracker.currentlyCharging();
    bool charging = evse_v2.evse_state.get("iec61851_state")->asUint() == IEC_STATE_C;

    if (charge_start_tracked && !charging) {
        this->stop_charging(0, true);
    }

    if (!charge_start_tracked && charging) {
        // If the user slot is enabled, this can not happen, as we first write into the ESPs RAM
        // and then set the user current so that the EVSE can start charging.
        // In the user slot is disabled, we just start tracking a charge here.
        this->start_charging(0, 32000);
    }

    if (charging) {
        // If the EVSE is already charging, read back the user slot info, in case the ESP just power cycled.
        UserSlotInfo info;
        bool success = read_user_slot_info(&info);
        if (success) {
            charge_info.get("id")->updateInt(info.user_id);
            charge_info.get("meter_start")->updateFloat(info.meter_start);
            charge_info.get("evse_uptime_start")->updateUint(info.evse_uptime_on_start);
            charge_info.get("timestamp_minutes")->updateUint(info.timestamp_minutes);
        }
    }

    #ifdef MODULE_EVSE_V2_AVAILABLE
    task_scheduler.scheduleWithFixedDelay("users_iec_check", [this](){
        static uint8_t last_iec_state = 0;

        uint8_t iec_state = evse_v2.evse_state.get("iec61851_state")->asUint();
        if (iec_state == last_iec_state)
            return;

        bool user_enabled = evse_v2.evse_slots.get(CHARGING_SLOT_USER)->get("active")->asBool();

        logger.printfln("IEC state changed from %u to %u", last_iec_state, iec_state);

        if ((last_iec_state == IEC_STATE_A && iec_state == IEC_STATE_B) || iec_state == IEC_STATE_C) {
            if (!user_enabled)
                this->start_charging(0, 32000);
        } else if (iec_state == IEC_STATE_A) {
            this->stop_charging(0, true);
        }

        last_iec_state = iec_state;
    }, 1000, 1000);
    #endif

    if (user_config.get("http_auth_enabled")->asBool()) {
        server.setAuthentication([this](WebServerRequest req) -> bool {
            String auth = req.header("Authorization");
            if (auth == "") {
                return false;
            }

            if (!auth.startsWith("Digest ")) {
                return false;
            }

            auth = auth.substring(7);
            AuthFields fields = parseDigestAuth(auth.c_str());

            for(int i = 0; i < user_config.get("users")->count(); ++i) {
                if (user_config.get("users")->get(i)->get("username")->asString().equals(fields.username))
                    return checkDigestAuthentication(fields, req.methodString(), fields.username.c_str(), user_config.get("users")->get(i)->get("digest_hash")->asCStr(), nullptr, true, nullptr, nullptr, nullptr);
            }

            return false;
        });

        logger.printfln("Web interface authentication enabled.");
    }

    initialized = true;
}

void Users::register_urls()
{
    api.addRawCommand("users/modify", [this](char *c, size_t s) -> String {
        StaticJsonDocument<96> doc;

        DeserializationError error = deserializeJson(doc, c, s);

        if (error) {
            return String("Failed to deserialize string: ") + String(error.c_str());
        }

        if (doc["id"] == nullptr)
            return String("Can't modify user. User ID is null or missing.");

        uint8_t id = doc["id"];
        if (id == 0) {
            return "Can't modify the anonymous user.";
        }

        Config *user = nullptr;
        for(int i = 0; i < user_config.get("users")->count(); ++i) {
            if (user_config.get("users")->get(i)->get("id")->asUint() == id) {
                user = user_config.get("users")->get(i);
                break;
            }
        }

        if (user == nullptr) {
            return "Can't modify user. User with this ID not found.";
        }

        if (doc["roles"] != nullptr)
            user->get("roles")->updateUint((uint32_t) doc["roles"]);

        bool display_name_changed = false;
        if (doc["display_name"] != nullptr)
            display_name_changed = user->get("display_name")->updateString(doc["display_name"]);

        if (doc["username"] != nullptr)
            user->get("username")->updateString(doc["username"]);

        if (doc["current"] != nullptr)
            user->get("current")->updateUint((uint32_t) doc["current"]);

        if (doc["digest_hash"] != nullptr)
            user->get("digest_hash")->updateString(doc["digest_hash"]);

        String err = this->user_config.validate();
        if (err != "")
            return err;

        API::writeConfig("users/config", &user_config);

        if (display_name_changed)
            this->rename_user(user->get("id")->asUint(), user->get("display_name")->asCStr());

        return "";
    }, true);

    api.addState("users/config", &user_config, {"digest_hash"}, 10000);
    api.addCommand("users/add", &add, {"digest_hash"}, [this](){
        user_config.get("users")->add();
        Config *user = user_config.get("users")->get(user_config.get("users")->count() - 1);

        user->get("id")->updateUint(add.get("id")->asUint());
        user->get("roles")->updateUint(add.get("roles")->asUint());
        user->get("current")->updateUint(add.get("current")->asUint());
        user->get("display_name")->updateString(add.get("display_name")->asString());
        user->get("username")->updateString(add.get("username")->asString());
        user->get("digest_hash")->updateString(add.get("digest_hash")->asCStr());

        uint8_t user_id = user_config.get("next_user_id")->asUint();
        ++user_id;
        if (user_id == 0)
            user_id = 1;

        user_config.get("next_user_id")->updateUint(user_id);

        API::writeConfig("users/config", &user_config);
        this->rename_user(user->get("id")->asUint(), user->get("username")->asCStr());
    }, true);

    api.addCommand("users/delete", &del, {}, [this](){
        int idx = -1;
        for(int i = 0; i < user_config.get("users")->count(); ++i) {
            if (user_config.get("users")->get(i)->get("id")->asUint() == del.get("id")->asUint()) {
                idx = i;
                break;
            }
        }

        if (idx < 0) {
            logger.printfln("Can't delete user. User with this ID not found.");
            return;
        }

        user_config.get("users")->remove(idx);
        API::writeConfig("users/config", &user_config);

        Config *tags = nfc.config.get("authorized_tags");

        for(int i = 0; i < tags->count(); ++i) {
            if(tags->get(i)->get("user_id")->asUint() == del.get("id")->asUint())
                tags->get(i)->get("user_id")->updateUint(0);
        }
        API::writeConfig("nfc/config", &nfc.config);
    }, true);

    api.addState("users/charge_info", &charge_info, {}, 1000);

    api.addCommand("users/http_auth_update", &http_auth_update, {}, [this](){
        user_config.get("http_auth_enabled")->updateBool(http_auth_update.get("enabled")->asBool());
        API::writeConfig("users/config", &user_config);
    }, false);

    server.on("/users/all_usernames", HTTP_GET, [this](WebServerRequest request) {
        //std::lock_guard<std::mutex> lock{records_mutex};
        size_t len = 256 * USERNAME_LENGTH;
        char *buf = (char*)malloc(len);
        if (buf == nullptr) {
            request.send(507);
            return;
        }

        File f = LittleFS.open("/users/usernames", "r");

        size_t read = f.read((uint8_t *)buf, len);
        request.send(200, "application/octet-stream", buf, read);

        free(buf);
    });
}

void Users::loop()
{

}

uint8_t Users::next_user_id()
{
    return this->user_config.get("next_user_id")->asUint();
}

void Users::create_user(const char *name)
{
    Config *next_uid = this->user_config.get("next_user_id");
    uint8_t next_user_id = next_uid->asUint();

    this->rename_user(next_user_id, name);

    next_uid->updateUint((uint8_t)++next_user_id);
}

void Users::get_username(uint8_t user_id, char *buf)
{
    File f = LittleFS.open("/users/usernames", "r");
    f.seek(user_id * USERNAME_LENGTH, SeekMode::SeekSet);
    f.read((uint8_t *)buf, USERNAME_LENGTH);

    if (buf[0] == '\0')
        snprintf(buf, USERNAME_LENGTH, "Unknown User %u", user_id);
}

void Users::rename_user(uint8_t user_id, const char *name)
{
    File f = LittleFS.open("/users/usernames", "r+");
    uint8_t buf[32] = {0};
    f.seek(user_id * USERNAME_LENGTH, SeekMode::SeekSet);
    f.write(buf, USERNAME_LENGTH);

    f.seek(user_id * USERNAME_LENGTH, SeekMode::SeekSet);
    f.write((const uint8_t *)name, strnlen(name, USERNAME_LENGTH));
}

// Only returns true if the triggered action was a charge start.
bool Users::trigger_charge_action(uint8_t user_id)
{
    bool user_enabled = evse_v2.evse_slots.get(CHARGING_SLOT_USER)->get("active")->asBool();
    if (!user_enabled)
        return false;
    // This is called whenever a user wants to trigger a charge action.
    // I.e. when holding an NFC tag at the box or when calling the start_charging API

    uint16_t current_limit = 0;
    Config *users = user_config.get("users");
    for (int i = 0; i < users->count(); ++i) {
        if (users->get(i)->get("id")->asUint() != user_id)
            continue;

        current_limit = users->get(i)->get("current")->asUint();
    }

    if (current_limit == 0) {
        logger.printfln("Unknown user with ID %u.", user_id);
        return false;
    }

    uint8_t iec_state = IEC_STATE_A;
    #ifdef MODULE_EVSE_V2_AVAILABLE
        iec_state = evse_v2.evse_state.get("iec61851_state")->asUint();
    #endif

    switch (iec_state) {
        case IEC_STATE_B: // State B: The user wants to start charging.
            return this->start_charging(user_id, current_limit);
        case IEC_STATE_C: // State C: The user wants to stop charging.
            this->stop_charging(user_id, false);
        default: //Don't do anything in state A, D, and E/F
            break;
    }
    return false;
}

uint32_t timestamp_minutes() {
    struct timeval tv_now;

    if (!clock_synced(&tv_now))
        return 0;

    return tv_now.tv_sec / 60;
}

bool Users::start_charging(uint8_t user_id, uint16_t current_limit)
{
    // TODO: use api.getState("evse/state") or similar here instead of ifdefing everything
    #ifdef MODULE_EVSE_V2_AVAILABLE
        if (charge_tracker.currentlyCharging())
            return false;

        uint32_t evse_uptime = evse_v2.evse_low_level_state.get("uptime")->asUint();
        bool meter_avail = evse_v2.evse_hardware_configuration.get("energy_meter_type")->asUint() != 0;
        float meter_start = !meter_avail ? NAN : evse_v2.evse_energy_meter_values.get("energy_abs")->asFloat();
        uint32_t timestamp = timestamp_minutes();

        write_user_slot_info(user_id, evse_uptime, timestamp, meter_start);
        charge_tracker.startCharge(timestamp, meter_start, user_id);
        evse_v2.set_user_current(current_limit);
        charge_info.get("id")->updateInt(user_id);
        charge_info.get("meter_start")->updateFloat(meter_start);
        charge_info.get("evse_uptime_start")->updateUint(evse_uptime);
        charge_info.get("timestamp_minutes")->updateUint(timestamp);


        return true;
    #endif
}

bool Users::stop_charging(uint8_t user_id, bool force)
{
    #ifdef MODULE_EVSE_V2_AVAILABLE
        if (charge_tracker.currentlyCharging()) {
            UserSlotInfo info;
            bool success = read_user_slot_info(&info);
            // If reading the user slot info failed, we don't know which user started this charge anymore.
            // This should only happen if the EVSE power-cycles, however on a power-cycle any running charge
            // should be aborted. It is safe to allow tracking a charge end in this case for any authorized card,
            // as this should never happen anyway.
            // Allow forcing the endCharge tracking. This is necessary in the case that the car was disconnected.
            // The user is then autorized at the other end of the charging cable.
            if (!force && success && info.user_id != user_id)
                return false;

            uint32_t charge_duration = 0;
            if (success) {
                uint32_t now_seconds = evse_v2.evse_low_level_state.get("uptime")->asUint() / 1000;
                uint32_t start_seconds = info.evse_uptime_on_start / 1000;
                if (now_seconds < start_seconds) {
                    now_seconds += (0xFFFFFFFF / 1000);
                }
                charge_duration = now_seconds - start_seconds;
            }

            bool meter_avail = evse_v2.evse_hardware_configuration.get("energy_meter_type")->asUint() != 0;
            charge_tracker.endCharge(charge_duration, !meter_avail ? NAN : evse_v2.evse_energy_meter_values.get("energy_abs")->asFloat());
        }
        zero_user_slot_info();
        evse_v2.set_user_current(0);
        charge_info.get("id")->updateInt(-1);
        charge_info.get("meter_start")->updateFloat(0);
        charge_info.get("evse_uptime_start")->updateUint(0);

        return true;
    #endif
}
