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

#include "charge_tracker.h"

#include <memory>
#include <LittleFS.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include "pdf_charge_log.h"

struct [[gnu::packed]] ChargeStart {
    uint32_t timestamp_minutes = 0;
    float meter_start = 0.0f;
    uint8_t user_id = 0;
};

static_assert(sizeof(ChargeStart) == 9, "Unexpected size of ChargeStart");

struct [[gnu::packed]] ChargeEnd {
    uint32_t charge_duration : 24;
    float meter_end = 0.0f;
};

struct [[gnu::packed]] Charge {
    ChargeStart cs;
    ChargeEnd ce;
};

static bool repair_logic(Charge *);

static_assert(sizeof(ChargeEnd) == 7, "Unexpected size of ChargeEnd");

#define CHARGE_RECORD_SIZE (sizeof(ChargeStart) + sizeof(ChargeEnd))

static_assert(CHARGE_RECORD_SIZE == 16, "Unexpected size of ChargeStart + ChargeEnd");

// 30 files with 256 records each: 7680 records @ ~ max. 10 records per day = ~ 2 years and one month of records.
// Also update frontend when changing this!
#define CHARGE_RECORD_FILE_COUNT 30
#define CHARGE_RECORD_MAX_FILE_SIZE 4096

#define CHARGE_RECORD_LAST_CHARGES_SIZE 30

void ChargeTracker::pre_setup()
{
    last_charges_prototype = Config::Object({
        {"timestamp_minutes", Config::Uint32(0)},
        {"charge_duration", Config::Uint32(0)},
        {"user_id", Config::Uint8(0)},
        {"energy_charged", Config::Float(0)}
    });

    last_charges = Config::Array(
        {},
        &last_charges_prototype,
        0, CHARGE_RECORD_LAST_CHARGES_SIZE, Config::type_id<Config::ConfObject>()
    );

    current_charge = Config::Object({
        {"user_id", Config::Int16(-1)},
        {"meter_start", Config::Float(0)},
        {"evse_uptime_start", Config::Uint32(0)},
        {"timestamp_minutes", Config::Uint32(0)},
        {"authorization_type", Config::Uint8(0)},
        {"authorization_info", Config{Config::ConfVariant()}}
    });

    state = Config::Object({
        {"tracked_charges", Config::Uint16(0)},
        {"first_charge_timestamp", Config::Uint32(0)}
    });

    config = Config::Object({
        {"electricity_price", Config::Uint16(0)}
    });

// #if MODULE_AUTOMATION_AVAILABLE()
//     automation.register_action(
//         AutomationActionID::ChargeTrackerReset,
//         *Config::Null(),
//         [this](const Config *conf) {
//             (void)conf;
//             api.callCommand("charge_tracker/remove_all_charges", Config::ConfUpdateObject{{
//                 {"do_i_know_what_i_am_doing", true}
//             }});
//         }
//     );
// #endif
}

String ChargeTracker::chargeRecordFilename(uint32_t i)
{
    return String(CHARGE_RECORD_FOLDER) + "/charge-record-" + i + ".bin";
}

bool ChargeTracker::repair_last(float meter_start)
{
    Charge charges[3];
    charges[0].ce.meter_end = NAN;
    charges[2].cs.meter_start = meter_start;

    if (!LittleFS.exists(chargeRecordFilename(last_charge_record)))
        return true;

    File r_file = LittleFS.open(chargeRecordFilename(last_charge_record), "r+");
    if (r_file.size() % CHARGE_RECORD_SIZE != 0) {
        logger.printfln("Can't track start of charge: Last charge end was not tracked or file is damaged! Offset is %u bytes. Expected 0", r_file.size() % CHARGE_RECORD_SIZE);
        // TODO: for robustness we would have to write the last end here? Yes, but only if % == 9. Also write duration 0, so we know this is a "faked" end. Still write the correct meter state.
        return false;
    }

    if (r_file.size() > sizeof(Charge)) {
        r_file.seek(r_file.size() - sizeof(Charge) * 2);
        r_file.read(reinterpret_cast<uint8_t *>(&charges), sizeof(Charge) * 2);
    }
    else if (r_file.size() == sizeof(Charge)) {
        if (last_charge_record > 1) {
            File tmp = LittleFS.open(chargeRecordFilename(last_charge_record - 1));
            tmp.seek(tmp.size() - sizeof(Charge) * 2);
            tmp.read(reinterpret_cast<uint8_t *>(&charges), sizeof(Charge));
        }
        r_file.seek(r_file.size() - sizeof(Charge));
        r_file.read(reinterpret_cast<uint8_t *>(&charges[1]), sizeof(Charge));
    }

    if (repair_logic(&charges[1])) {
        r_file.seek(r_file.size() - sizeof(Charge));
        r_file.write(reinterpret_cast<uint8_t *>(&charges[1]), sizeof(Charge));
        logger.printfln("Repaired previous broken charge.");
        last_charges.get(last_charges.count() - 1)->get("energy_charged")->updateFloat(charges[1].ce.meter_end - charges[1].cs.meter_start);
    }
    return true;
}

bool ChargeTracker::startCharge(uint32_t timestamp_minutes, float meter_start, uint8_t user_id, uint32_t evse_uptime, uint8_t auth_type, Config::ConfVariant auth_info) {
#if MODULE_REQUIRE_METER_AVAILABLE()
    if (!require_meter.allow_charging(meter_start))
        return false;
#endif

    std::lock_guard<std::mutex> lock{records_mutex};

    if (!repair_last(meter_start)) {
        return false;
    }

    ChargeStart cs;
    File file = LittleFS.open(chargeRecordFilename(this->last_charge_record), "a", true);

    if (file.size() == CHARGE_RECORD_MAX_FILE_SIZE) {
        ++this->last_charge_record;
        String new_file_name = chargeRecordFilename(this->last_charge_record);
        logger.printfln("Last charge record file %s is full. Creating the new file %s", file.name(), new_file_name.c_str());
        file.close();

        removeOldRecords();
        updateState();

        file = LittleFS.open(new_file_name, "w", true);
    }

    cs.timestamp_minutes = timestamp_minutes;
    cs.meter_start = meter_start;
    cs.user_id = user_id;

    uint8_t buf[sizeof(ChargeStart)] = {0};
    memcpy(buf, &cs, sizeof(cs));

    file.write(buf, sizeof(cs));
    logger.printfln("Tracked start of charge.");

    current_charge.get("user_id")->updateInt(user_id);
    current_charge.get("meter_start")->updateFloat(meter_start);
    current_charge.get("evse_uptime_start")->updateUint(evse_uptime);
    current_charge.get("timestamp_minutes")->updateUint(timestamp_minutes);
    current_charge.get("authorization_type")->updateUint(auth_type);
    current_charge.get("authorization_info")->value = auth_info;
    current_charge.get("authorization_info")->value.updated = 0xFF;
    return true;
}

void ChargeTracker::endCharge(uint32_t charge_duration_seconds, float meter_end)
{
    std::lock_guard<std::mutex> lock{records_mutex};
    ChargeEnd ce;

    {
        File file = LittleFS.open(chargeRecordFilename(this->last_charge_record), "a");
        if ((file.size() % CHARGE_RECORD_SIZE) != sizeof(ChargeStart)) {
            logger.printfln("Can't track end of charge: Last charge start was not tracked or file is damaged! Offset is %u bytes. Expected %u", file.size() % CHARGE_RECORD_SIZE, sizeof(ChargeStart));
            // TODO: How to handle this case? Add a charge start with the same meter value as the last end?
            // This would also mean that all the size checks of startCharge have to be duplicated!
            // If we check in ::setup() whether a charge is running, this can never happen.
            return;
        }

        ce.charge_duration = charge_duration_seconds;
        ce.meter_end = meter_end;

        uint8_t buf[sizeof(ChargeEnd)] = {0};
        memcpy(buf, &ce, sizeof(ce));

        file.write(buf, sizeof(ce));
    }
    logger.printfln("Tracked end of charge.");

    // We've just written the charge record in the file. It is always safe to read it back again.
    if (last_charges.count() == CHARGE_RECORD_LAST_CHARGES_SIZE)
        last_charges.remove(0);

    File f = LittleFS.open(chargeRecordFilename(this->last_charge_record));
    f.seek(-CHARGE_RECORD_SIZE, SeekMode::SeekEnd);
    this->readNRecords(&f, 1);

    current_charge.get("user_id")->updateInt(-1);
    current_charge.get("meter_start")->updateFloat(0);
    current_charge.get("evse_uptime_start")->updateUint(0);
    current_charge.get("timestamp_minutes")->updateUint(0);
    current_charge.get("authorization_type")->updateUint(0);
    current_charge.get("authorization_info")->value = Config::ConfVariant{};

    updateState();
}

bool ChargeTracker::is_user_tracked(uint8_t user_id)
{
    const size_t user_id_offset = offsetof(ChargeStart, user_id);

    for (int file = this->first_charge_record; file <= this->last_charge_record; ++file) {
        File f = LittleFS.open(chargeRecordFilename(file));
        size_t size = f.size();
        // LittleFS caches internally, so we can read single bytes without a huge performance loss.
        for (size_t i = 0; i < size; i += CHARGE_RECORD_SIZE) {
            f.seek(i + user_id_offset);
            int read_user_id = f.read();
            if (read_user_id < 0)
                continue;
            if (user_id == read_user_id)
                return true;
        }
    }
    return false;
}

void ChargeTracker::removeOldRecords()
{
    const size_t user_id_offset = offsetof(ChargeStart, user_id);

    uint32_t users_to_delete[8] = {0}; // one bit per user

    while (this->last_charge_record - this->first_charge_record >= 30) {
        String name = chargeRecordFilename(this->first_charge_record);
        logger.printfln("Got %u charge records. Dropping the first one (%s)", this->last_charge_record - this->first_charge_record, name.c_str());
        {
            File f = LittleFS.open(name, "r");
            size_t size = f.size();
            for (size_t i = 0; i < size; i += CHARGE_RECORD_SIZE) {
                f.seek(i + user_id_offset);
                int x = f.read();
                if (x < 0)
                    continue;
                uint8_t user_id = x;
                users_to_delete[user_id / 32] |= (1 << (user_id % 32));
            }
        }
        LittleFS.remove(name);
        ++this->first_charge_record;
    }

    //users_to_delete has now set a bit for every user_id that was used in the deleted charge records.
    //Clear this bit for every user that is still used in the current charge records.
    for (int file = this->first_charge_record; file < this->last_charge_record; ++file) {
        File f = LittleFS.open(chargeRecordFilename(file));
        size_t size = f.size();
        // LittleFS caches internally, so we can read single bytes without a huge performance loss.
        for (size_t i = 0; i < size; i += CHARGE_RECORD_SIZE) {
            f.seek(i + user_id_offset);
            int x = f.read();
            if (x < 0)
                continue;
            uint8_t user_id = x;
            users_to_delete[user_id / 32] &= ~(1 << (user_id % 32));
        }
    }

    // Now only users that are safe to remove remain.
    for (int user_id = 0; user_id < 256; ++user_id) {
        if ((users_to_delete[user_id / 32] & (1 << (user_id % 32))) != 0) {
            // remove_from_username_file only removes if the user is not configured
            users.remove_from_username_file(user_id);
        }
    }
}

bool ChargeTracker::setupRecords()
{
    if (!LittleFS.mkdir(CHARGE_RECORD_FOLDER)) { // mkdir also returns true if the directory already exists and is a directory.
        logger.printfln("Failed to create charge record folder!");
        return false;
    }

    File folder = LittleFS.open(CHARGE_RECORD_FOLDER);
    File f;

    uint32_t found_blobs[32] = {0};
    size_t found_blobs_size = ARRAY_SIZE(found_blobs);
    int found_blob_counter = 0;

    while (f = folder.openNextFile()) {
        String name = String(f.name());
        if (f.isDirectory()) {
            logger.printfln("Unexpected directory %s in charge record folder", name.c_str());
            continue;
        }

        if (name == "use_imexsum") {
            continue;
        }

        if (!name.startsWith("charge-record-") || !name.endsWith(".bin")) {
            logger.printfln("Unexpected file %s in charge record folder", name.c_str());
            continue;
        }

        long suffix = name.substring(14, name.length() - 4).toInt();
        if (suffix == 0) {
            logger.printfln("Unexpected file %s in charge record folder", name.c_str());
            continue;
        }

        if (found_blob_counter > found_blobs_size) {
            logger.printfln("Too many charge records found!");
            return false;
        }

        found_blobs[found_blob_counter] = suffix;
        ++found_blob_counter;
    }

    if (found_blob_counter == 0) {
        this->first_charge_record = 1;
        this->last_charge_record = 1;
        return true;
    }

    std::sort(found_blobs, found_blobs + found_blob_counter);

    uint32_t first = found_blobs[0];
    uint32_t last = found_blobs[found_blob_counter - 1];

    logger.printfln("Found %u record%s: first is %u, last is %u", found_blob_counter, found_blob_counter == 1 ? "" : "s", first, last);
    for (int i = 0; i < found_blob_counter - 1; ++i) {
        if (found_blobs[i] + 1 != found_blobs[i + 1]) {
            logger.printfln("Non-consecutive charge records found! (Next after %u is %u. Expected was %u", found_blobs[i], found_blobs[i+1], found_blobs[i] + 1);
            return false;
        }

        f = LittleFS.open(chargeRecordFilename(found_blobs[i]));
        if (f.size() != CHARGE_RECORD_MAX_FILE_SIZE) {
            logger.printfln("Charge record %s doesn't have max size: %u bytes", f.name(), f.size());
            return false;
        }
    }

    String last_file_name = chargeRecordFilename(found_blobs[found_blob_counter - 1]);
    f = LittleFS.open(last_file_name, "a");
    size_t last_file_size = f.size();
    logger.printfln("Last charge record size is %u (%u, %u)", last_file_size, f.size(), (last_file_size % CHARGE_RECORD_SIZE));
    f.close();
    if (last_file_size > CHARGE_RECORD_MAX_FILE_SIZE) {
        logger.printfln("Last charge record %s is too long: %u bytes", f.name(), f.size());
        return false;
    }

    this->first_charge_record = first;
    this->last_charge_record = last;

    return true;
}

size_t ChargeTracker::completeRecordsInLastFile()
{
    File file = LittleFS.open(chargeRecordFilename(this->last_charge_record));
    return file.size() / CHARGE_RECORD_SIZE;
}

bool ChargeTracker::currentlyCharging()
{
    File file = LittleFS.open(chargeRecordFilename(this->last_charge_record));
    return (file.size() % CHARGE_RECORD_SIZE) == sizeof(ChargeStart);
}

bool charged_invalid(ChargeStart cs, ChargeEnd ce)
{
    return isnan(cs.meter_start) || isnan(ce.meter_end) || ce.meter_end < cs.meter_start;
}

void ChargeTracker::readNRecords(File *f, size_t records_to_read)
{
    uint8_t buf[CHARGE_RECORD_SIZE];
    ChargeStart cs;
    ChargeEnd ce;

    for (int i = 0; i < records_to_read; ++i) {
        memset(buf, 0, sizeof(buf));
        f->read(buf, CHARGE_RECORD_SIZE);

        memcpy(&cs, buf, sizeof(cs));
        memcpy(&ce, buf + sizeof(cs), sizeof(ce));

        auto last_charge = last_charges.add();
        last_charge->get("timestamp_minutes")->updateUint(cs.timestamp_minutes);
        last_charge->get("charge_duration")->updateUint(ce.charge_duration);
        last_charge->get("user_id")->updateUint(cs.user_id);
        last_charge->get("energy_charged")->updateFloat(charged_invalid(cs, ce) ? NAN : ce.meter_end - cs.meter_start);
    }
}

void ChargeTracker::updateState()
{
    auto records = this->last_charge_record - this->first_charge_record + 1;
    state.get("tracked_charges")->updateUint((records - 1) * (CHARGE_RECORD_MAX_FILE_SIZE / CHARGE_RECORD_SIZE) + completeRecordsInLastFile());

    File f = LittleFS.open(chargeRecordFilename(this->first_charge_record));
    ChargeStart cs;
    if (f.size() >= sizeof(cs)) {
        uint8_t buf[sizeof(cs)];

        memset(buf, 0, sizeof(buf));
        f.read(buf, sizeof(cs));

        memcpy(&cs, buf, sizeof(cs));
        state.get("first_charge_timestamp")->updateUint(cs.timestamp_minutes);
    }
}

void ChargeTracker::setup()
{
    initialized = this->setupRecords();
    if (!initialized) {
        return;
    }

    if (!LittleFS.exists(chargeRecordFilename(this->last_charge_record)))
        LittleFS.open(chargeRecordFilename(this->last_charge_record), "w", true);

    repair_charges();

    api.restorePersistentConfig("charge_tracker/config", &config);

    // Fill charge_tracker/last_charges
    bool charging = currentlyCharging();
    size_t records_in_last_file = completeRecordsInLastFile();

    if (records_in_last_file < CHARGE_RECORD_LAST_CHARGES_SIZE && LittleFS.exists(chargeRecordFilename(this->last_charge_record - 1))) {
        size_t records_to_read = CHARGE_RECORD_LAST_CHARGES_SIZE - records_in_last_file;
        File f = LittleFS.open(chargeRecordFilename(this->last_charge_record - 1));
        f.seek(-(records_to_read * CHARGE_RECORD_SIZE), SeekMode::SeekEnd);

        this->readNRecords(&f, records_to_read);
    }

    size_t records_to_read = min(records_in_last_file, (size_t)CHARGE_RECORD_LAST_CHARGES_SIZE);
    {
        File f = LittleFS.open(chargeRecordFilename(this->last_charge_record));
        f.seek(-(records_to_read * CHARGE_RECORD_SIZE) - (charging ? sizeof(ChargeStart) : 0), SeekMode::SeekEnd);
        this->readNRecords(&f, records_to_read);
    }

    updateState();
}

bool user_configured(const uint8_t configured_users[MAX_ACTIVE_USERS], uint8_t user_id)
{
    for (int i = 0; i < MAX_ACTIVE_USERS; ++i) {
        if (configured_users[i] == user_id) {
            return true;
        }
    }
    return false;
}

static size_t timestamp_min_to_date_time_string(char buf[17], uint32_t timestamp_min, bool english)
{
    const char * const unknown = english ? "unknown" : "unbekannt";
    size_t unknown_len =  english ? ARRAY_SIZE("unknown") : ARRAY_SIZE("unbekannt");

    if (timestamp_min == 0) {
        memcpy(buf, unknown, unknown_len);
        return unknown_len - 1; // exclude null terminator
    }
    time_t timestamp = ((int64_t)timestamp_min) * 60;
    struct tm t;
    localtime_r(&timestamp, &t);

    if (english)
        return sprintf_u(buf, "%4.4i-%2.2i-%2.2i %2.2i:%2.2i", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min);

    return sprintf_u(buf, "%2.2i.%2.2i.%4.4i %2.2i:%2.2i", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min);
}

static_assert(DISPLAY_NAME_LENGTH == 32, "Unexpected display name length");
struct display_name_entry {
    uint32_t length;
    uint32_t name[DISPLAY_NAME_LENGTH / sizeof(uint32_t)];
};

static size_t get_display_name(uint8_t user_id, char *ret_buf, display_name_entry *display_name_cache)
{
    if (display_name_cache[user_id].length > DISPLAY_NAME_LENGTH) {
        size_t length = 0;
        uint32_t buf[9] = {}; // Make sure that short names are zero-padded.
        task_scheduler.await([&length, user_id, &buf]() {length = users.get_display_name(user_id, reinterpret_cast<char *>(buf));});
        if (length > sizeof(display_name_cache[user_id].name)) {
            logger.printfln("Returned user name too long: [%.*s]", length, reinterpret_cast<char *>(buf));
            display_name_cache[user_id].length = 0;
        } else {
            // Ignore actual name length in order to copy zero-padding as well.
            uint32_t *src = buf;
            uint32_t *dst_start = display_name_cache[user_id].name;
            uint32_t *dst_end = dst_start + ARRAY_SIZE(display_name_cache[user_id].name);
            do {
                *dst_start++ = *src++;
            } while (dst_start < dst_end);

            display_name_cache[user_id].length = length;
        }
    }

    uint32_t *src_start = display_name_cache[user_id].name;
    uint32_t *src_end = src_start + (display_name_cache[user_id].length + 3) / 4; // Round up; copies some buffer slack that must be zero-filled. See padding above.
    uint32_t *dst = reinterpret_cast<uint32_t *>(ret_buf);
    while (src_start < src_end) {
        *dst++ = *src_start++;
    }

    return display_name_cache[user_id].length;
}

static char *tracked_charge_to_string(char *buf, ChargeStart cs, ChargeEnd ce, bool english, uint32_t electricity_price, display_name_entry *display_name_cache)
{
    buf += 1 + timestamp_min_to_date_time_string(buf, cs.timestamp_minutes, english);

    size_t name_len = get_display_name(cs.user_id, buf, display_name_cache);
    buf += 1 + name_len;

    if (charged_invalid(cs, ce)) {
        memcpy(buf, "N/A", ARRAY_SIZE("N/A"));
        buf += ARRAY_SIZE("N/A");
    } else {
        float charged = ce.meter_end - cs.meter_start;
        if (charged <= 999.999f) {
            int written = sprintf_u(buf, "%.3f", charged);
            if (!english)
                for (int i = 0; i < written; ++i)
                    if (buf[i] == '.')
                        buf[i] = ',';
            buf += 1 + written;
        }
        else {
            memcpy(buf, ">=1000", ARRAY_SIZE(">=1000"));
            buf += ARRAY_SIZE(">=1000");
        }
    }

    // charge duration is a bitfield value of 24 bits.
    // This results in a maximum duration of 2^24/3600 ~ 4660 hours.
    // We handle up to 9999 hours here -> No need for a fallback.
    int hours = ce.charge_duration / 3600;
    ce.charge_duration = ce.charge_duration % 3600;
    int minutes = ce.charge_duration / 60;
    ce.charge_duration = ce.charge_duration % 60;
    int seconds = ce.charge_duration;

    buf += 1 + sprintf_u(buf, "%i:%2.2i:%2.2i", hours, minutes, seconds);

    if (isnan(cs.meter_start)) {
        memcpy(buf, "N/A", ARRAY_SIZE("N/A"));
        buf += ARRAY_SIZE("N/A");
    } else {
        int written = sprintf_u(buf, "%.3f", cs.meter_start);
        if (!english)
            for (int i = 0; i < written; ++i)
                if (buf[i] == '.')
                    buf[i] = ',';
        buf += 1 + written;
    }

    if (electricity_price == 0) {
        memcpy(buf, "---", ARRAY_SIZE("---"));
        buf += ARRAY_SIZE("---");
    } else if (charged_invalid(cs, ce)) {
        memcpy(buf, "N/A", ARRAY_SIZE("N/A"));
        buf += ARRAY_SIZE("N/A");
    } else {
        double charged = ce.meter_end - cs.meter_start;
        uint32_t cost = round(charged * electricity_price / 100.0f);
        if (cost > 999999) {
            memcpy(buf, ">=10000", ARRAY_SIZE(">=10000"));
            buf += ARRAY_SIZE(">=10000");
        } else {
            buf += 1 + sprintf_u(buf, "%d%c%02d", cost / 100, english ? '.' : ',', cost % 100);
        }
    }
    return buf;
}

static bool repair_logic(Charge *buf)
{
    bool repaired = false;
    uint8_t state = 0;

    // There are only known issues with broken charges with a meter end of 0.
    // We can add the same logic for the start too if needed.
    state |= !isnan(buf[-1].ce.meter_end) << 3;
    state |= (!isnan(buf[0].cs.meter_start) && buf[0].cs.meter_start != 0) << 2;
    state |= (!isnan(buf[0].ce.meter_end) && buf[0].ce.meter_end != 0) << 1;
    state |= !isnan(buf[1].cs.meter_start);

    // We have five cases that can be repaired/ have to be repaired. state is a bitmap.
    switch (state)
    {
    // The end of a charge is missing but we got the beginning and the beginning of the next charge.
    case 10:
    case 11:
        if (buf[-1].ce.meter_end <= buf[0].ce.meter_end
                && buf[0].ce.meter_end - buf[-1].ce.meter_end < CHARGE_TRACKER_MAX_REPAIR) {
            if (buf[0].cs.meter_start == 0 && buf[-1].ce.meter_end < CHARGE_TRACKER_MAX_REPAIR)
                break;

            buf[0].cs.meter_start = buf[-1].ce.meter_end;
            repaired = true;
        }
        break;

    // The start of a charge is missing but we got the end of it and the end of the previous charge.
    case 5:
    case 13:
        if (buf[0].cs.meter_start <= buf[1].cs.meter_start
                && buf[1].cs.meter_start - buf[0].cs.meter_start < CHARGE_TRACKER_MAX_REPAIR) {
            // If the Meter is new and somehow meter start and meter end is 0 this is not a broken charge.
            if (buf[0].ce.meter_end == 0 && buf[0].cs.meter_start == 0)
                break;

            buf[0].ce.meter_end = buf[1].cs.meter_start;
            repaired = true;
        }
        break;

    // We got no meter values of the charge but we got the end of the previous and the start of the next.
    case 9:
        if (buf[-1].ce.meter_end <= buf[1].cs.meter_start
                && buf[1].cs.meter_start - buf[-1].ce.meter_end < CHARGE_TRACKER_MAX_REPAIR
                && !(buf[-1].ce.meter_end == 0 && buf[0].cs.meter_start == 0 && buf[0].ce.meter_end == 0 && buf[1].cs.meter_start == 0)) {
            buf[0].cs.meter_start = buf[-1].ce.meter_end;
            buf[0].ce.meter_end = buf[1].cs.meter_start;
            repaired = true;
        }
        break;

    default:
        break;
    }
    return repaired;
}

void ChargeTracker::repair_charges()
{
    auto buf = heap_alloc_array<Charge>(258);
    uint32_t num_repaired = 0;
    Charge transfer;
    transfer.ce.meter_end = NAN;

    for (int i = this->first_charge_record; i <= this->last_charge_record; ++i) {
        bool file_needs_repair = false;
        memset(reinterpret_cast<uint8_t *>(&buf[1]), 0, sizeof(Charge) * 257);

        File f = LittleFS.open(chargeRecordFilename(i));
        if (i < this->last_charge_record) {
            File next_f = LittleFS.open(chargeRecordFilename(i + 1));
            int read = next_f.read(reinterpret_cast<uint8_t *>(&buf[257]), sizeof(Charge));
            if (read != sizeof(Charge))
                buf[257].cs.meter_start = NAN;
        }
        else
            buf[257].cs.meter_start = NAN;

        int read = f.read(reinterpret_cast<uint8_t *>(&buf[1]), sizeof(Charge) * 257);
        if (read == -1 || read == 0) {
            break;
        }

        for (int a = 1; a < read / sizeof(Charge); a++) {
            if (repair_logic(&buf[a])) {
                file_needs_repair = true;
                num_repaired++;
            }
        }
        if (file_needs_repair) {
            File write_f = LittleFS.open(chargeRecordFilename(i), "w");
            write_f.write(reinterpret_cast<uint8_t *>(&buf[1]), read);
        }
        buf[0] = buf[256];
    }
    if (num_repaired != 0) {
        logger.printfln("Repaired %u charge-entries.", num_repaired);
    }
}

void ChargeTracker::register_urls()
{
    // We have to do this here, not at the end of setup,
    // because removeOldRecords calls users.remove_from_username_file
    // which requires that the user config is already loaded from flash.
    // This happens in users::setup() i.e. _after_ charge_tracker::setup()
    removeOldRecords();

    api.addPersistentConfig("charge_tracker/config", &config);

    server.on_HTTPThread("/charge_tracker/charge_log", HTTP_GET, [this](WebServerRequest request) {
        std::lock_guard<std::mutex> lock{records_mutex};

        auto url_buf = heap_alloc_array<char>(CHARGE_RECORD_MAX_FILE_SIZE);
        if (url_buf == nullptr) {
            return request.send(507);
        }

        File file = LittleFS.open(chargeRecordFilename(this->last_charge_record));
        size_t file_size = (this->last_charge_record - this->first_charge_record) * CHARGE_RECORD_MAX_FILE_SIZE + file.size();

        // Don't do a chunked response without any chunk. The webserver does strange things in this case
        if (file_size == 0) {
            return request.send(200, "application/octet-stream", "", 0);
        }

        request.beginChunkedResponse(200, "application/octet-stream");
        for (int i = this->first_charge_record; i <= this->last_charge_record; ++i) {
            File f = LittleFS.open(chargeRecordFilename(i));
            int read = f.read((uint8_t *)url_buf.get(), CHARGE_RECORD_MAX_FILE_SIZE);
            int trunc = read - (read % CHARGE_RECORD_SIZE);
            request.sendChunk(url_buf.get(), trunc);
        }
        return request.endChunkedResponse();
    });

    api.addState("charge_tracker/last_charges", &last_charges);
    api.addState("charge_tracker/current_charge", &current_charge);
    api.addState("charge_tracker/state", &state);

    api.addCommand("charge_tracker/remove_all_charges", Config::Confirm(), {Config::confirm_key}, [this](String &errmsg) {
        if (!Config::Confirm()->get(Config::ConfirmKey())->asBool()) {
            errmsg = "Tracked charges will NOT be removed";
            return;
        }

        remove_directory(CHARGE_RECORD_FOLDER);
        users.remove_username_file();

        trigger_reboot("removing all tracked charges", 1_s);
    }, true);

    server.on_HTTPThread("/charge_tracker/pdf", HTTP_PUT, [this](WebServerRequest request) {
        logger.printfln("Beginning PDF generation. Please ignore timeout errors (rc -1 etc.) until it is done.");
        #define USER_FILTER_ALL_USERS -2
        #define USER_FILTER_DELETED_USERS -1
        int user_filter = USER_FILTER_ALL_USERS;
        uint32_t start_timestamp_min = 0;
        uint32_t end_timestamp_min = 0;
        uint32_t current_timestamp_min = rtc.timestamp_minutes();

        bool english = false;
        #define LETTERHEAD_SIZE 512
        auto letterhead = heap_alloc_array<char>(LETTERHEAD_SIZE);
        int letterhead_lines = 0;

        {
            StaticJsonDocument<192> doc;
            auto buf = heap_alloc_array<char>(1024);
            if (request.contentLength() > 1024)
                return request.send(413);
            request.receive(buf.get(), 1024);

            DeserializationError error = deserializeJson(doc, buf.get(), 1024);
            if (error) {
                String errorString = String("Failed to deserialize string: ") + error.c_str();
                return request.send(400, "text/plain", errorString.c_str());
            }
            if (!bool(doc["api_not_final_acked"]))
                return request.send(400, "text/plain", "Please acknowledge that this API is subject to change!");

            user_filter = doc["user_filter"] | USER_FILTER_ALL_USERS;
            start_timestamp_min = doc["start_timestamp_min"] | 0l;
            end_timestamp_min = doc["end_timestamp_min"] | 0l;
            english = doc["english"] | false;
            if (current_timestamp_min == 0)
                current_timestamp_min = doc["current_timestamp_min"] | 0l;
            if (doc.containsKey("letterhead")) {
                const char *lh = doc["letterhead"];
                letterhead_lines = 1;
                for (size_t i = 0; i < LETTERHEAD_SIZE; ++i) {
                    if (lh[i] == '\0')
                        break;
                    if (lh[i] == '\n') {
                        letterhead[i] = '\0';
                        if (letterhead_lines == 6)
                            break;
                        ++letterhead_lines;
                    }
                    else
                        letterhead[i] = lh[i];
                }
            }
        }

        char stats_buf[384];//55  9 "Wallbox: " + 32 display name + 13 " (warp2-AbCd)" + \0
                            //31  13 "Exportiert am " + 17 (date time + \0)
                            //55  22  "Exportierte Benutzer: " + 32 display name + \0
                            //63  "Exportierter Zeitraum: Aufzeichnungsbeginn - Aufzeichnungsende" + \0
                            //60  41 "Gesamtenergie exportierter Ladevorgänge: " + 19 "999.999.999,999kWh" + \0
                            //50 "Gesamtbetrag 99999.99€ (Strompreis 123.45 ct/kWh)" + \0
                            //= 314

        double charged_sum = 0;
        uint32_t charged_cost_sum = 0;
        bool seen_charges_without_meter = false;

        int charge_records = 0;
        int first_file = -1;
        int first_charge = -1;
        int last_file = -1;
        int last_charge = -1;

        std::lock_guard<std::mutex> lock{records_mutex};

        uint8_t configured_users[MAX_ACTIVE_USERS] = {};
        uint32_t electricity_price;
        String dev_name;
        auto await_result = task_scheduler.await([this, configured_users, &electricity_price, &dev_name]() mutable {
            electricity_price = this->config.get("electricity_price")->asUint();
            for (size_t i = 0; i < users.config.get("users")->count(); ++i) {
                configured_users[i] = users.config.get("users")->get(i)->get("id")->asUint();
            }
            dev_name = device_name.display_name.get("display_name")->asString();
            if (device_name.display_name.get("display_name")->asString() != device_name.name.get("name")->asString())
                dev_name += " (" + device_name.name.get("name")->asString() + ")";
        });

        if (await_result == TaskScheduler::AwaitResult::Timeout) {
            return request.send(500, "text/plain", "Failed to generate PDF: Task timed out");
        }

        {
            char charge_buf[sizeof(ChargeStart) + sizeof(ChargeEnd)];
            ChargeStart cs;
            ChargeEnd ce;

            for (int i = this->first_charge_record; i <= this->last_charge_record; ++i) {
                File f = LittleFS.open(chargeRecordFilename(i));

                for (int j = 0; j < (CHARGE_RECORD_MAX_FILE_SIZE / CHARGE_RECORD_SIZE); ++j) {
                    if (f.read((uint8_t *)charge_buf, CHARGE_RECORD_SIZE) != CHARGE_RECORD_SIZE)
                        // This file is not "full". We don't have any tracked charges left.
                        goto search_done;

                    memcpy(&cs, charge_buf, sizeof(ChargeStart));
                    memcpy(&ce, charge_buf + sizeof(ChargeStart), sizeof(ChargeEnd));

                    if (cs.timestamp_minutes != 0 && start_timestamp_min != 0 && cs.timestamp_minutes < start_timestamp_min) {
                        // We know when this charge started and it was before the requested start date.
                        // This means that all charges before and including this one can't be relevant.
                        charge_records = 0;
                        first_file = -1;
                        first_charge = -1;
                        charged_sum = 0;
                        charged_cost_sum = 0;
                        continue;
                    }

                    if (cs.timestamp_minutes != 0 && end_timestamp_min != 0 && cs.timestamp_minutes > end_timestamp_min) {
                        // This charge started after the requested end date. We are done searching.
                        last_file = i;
                        last_charge = j;
                        goto search_done;
                    }

                    bool include_user = user_filter == USER_FILTER_ALL_USERS || (user_filter == USER_FILTER_DELETED_USERS && !user_configured(configured_users, cs.user_id)) || cs.user_id == user_filter;
                    if (!include_user)
                        continue;

                    if (first_file == -1)
                        first_file = i;

                    if (first_charge == -1)
                        first_charge = j;

                    last_file = i;
                    last_charge = j;
                    ++charge_records;

                    if (charged_invalid(cs, ce))
                        seen_charges_without_meter = true;
                    else {
                        double charged = ce.meter_end - cs.meter_start;
                        charged_sum += charged;
                        if (electricity_price != 0)
                            charged_cost_sum += round(charged * electricity_price / 100.0f);
                    }
                }
            }
        }
search_done:

        display_name_entry *display_name_cache = static_cast<decltype(display_name_cache)>(heap_caps_malloc(MAX_PASSIVE_USERS * sizeof(display_name_cache[0]), MALLOC_CAP_32BIT));
        if (!display_name_cache) {
            return request.send(500, "text/plain", "Failed to generate PDF: No memory");;
        }

        for (size_t i = 0; i < MAX_PASSIVE_USERS; i++) {
            display_name_cache[i].length = UINT32_MAX;
        }

        char *stats_head = stats_buf;
        stats_head += 1 + sprintf_u(stats_head, "%s: %s", english ? "Charger" : "Wallbox", dev_name.c_str());

        stats_head += sprintf_u(stats_head, "%s: ", english ? "Exported on" : "Exportiert am");
        stats_head += 1 + timestamp_min_to_date_time_string(stats_head, current_timestamp_min, english);

        stats_head += sprintf_u(stats_head, "%s: ", english ? "Exported users" : "Exportierte Benutzer");
        if (user_filter == -2)
            stats_head += sprintf_u(stats_head, "%s", english ? "all users" : "Alle Benutzer");
        else if (user_filter == -1)
            stats_head += sprintf_u(stats_head, "%s", english ? "deleted users" : "Gelöschte Benutzer");
        else
            stats_head += get_display_name(user_filter, stats_head, display_name_cache);
        ++stats_head;

        stats_head += sprintf_u(stats_head, "%s: ", english ? "Exported period" : "Exportierter Zeitraum");
        if (start_timestamp_min == 0)
            stats_head += sprintf_u(stats_head, "%s", english ? "record start" : "Aufzeichnungsbeginn");
        else
            stats_head += timestamp_min_to_date_time_string(stats_head, start_timestamp_min, english);

        stats_head += sprintf_u(stats_head, "%s", english ? " to " : " bis ");

        if (end_timestamp_min == 0)
            stats_head += sprintf_u(stats_head, "%s", english ? "record end" : (start_timestamp_min == 0 ? "-ende" : "Aufzeichnungsende"));
        else
            stats_head += timestamp_min_to_date_time_string(stats_head, end_timestamp_min, english);
        ++stats_head;

        int written = sprintf_u(stats_head, "%s: %9.3f kWh", english ? "Total energy of exported charges" : "Gesamtenergie exportierter Ladevorgänge", charged_sum);
        if (!english)
            for (int i = 0; i < written; ++i)
                if (stats_head[i] == '.')
                    stats_head[i] = ',';
        stats_head += 1 + written;

        if (electricity_price != 0) {
            written = sprintf_u(stats_head, "%s: %d.%02d€ (%.2f ct/kWh)%s",
                            english ? "Total cost" : "Gesamtkosten",
                            charged_cost_sum / 100, charged_cost_sum % 100,
                            electricity_price / 100.0f,
                            seen_charges_without_meter ? (english ? " Incomplete!" : " Unvollständig!") : "");
            if (!english)
                for (int i = 0; i < written; ++i)
                    if (stats_head[i] == '.')
                        stats_head[i] = ',';
            stats_head += 1 + written;
        }

        // TODO: this is currently unnecessary, however if we support other ways of requesting a PDF
        // we have to lock the pdf generator.
        std::lock_guard<std::mutex> lock2{pdf_mutex};

        int current_file = (first_file > -1 ? first_file : this->first_charge_record);
        int current_charge = (first_charge > -1 ? first_charge : 0);
        last_file = (last_file >= 0) ? last_file : this->last_charge_record;

#define TABLE_LINE_LEN (17 /*start date: 01.02.3456 12:34\0 or 3456-02-01 12:34\0*/ \
                      + 33 /*display name: max 32 chars + \0*/ \
                      + 8  /*charged: (assumed max) "999.999\0" kWh else truncated to "> 1000\0"*/ \
                      + 11 /* charge duration max "9999:59:59\0"*/ \
                      + 16 /* meter start max 99'999'999.999\0*/ \
                      + 8) /* cost max 9999.99\0 else truncated to >10000*/

        char table_lines_buffer[8 * TABLE_LINE_LEN];
        File f;

        request.beginChunkedResponse(200, "application/pdf");

        const char * table_header_de = "Startzeit\0"
                                       "Benutzer\0"
                                       "geladen (kWh)\0"
                                       "Ladedauer\0"
                                       "Zählerstand Start\0"
                                       "Kosten (€)";

        const char * table_header_en = "Start time\0"
                                       "User\0"
                                       "Charged (kWh)\0"
                                       "Duration\0"
                                       "Meter start\0"
                                       "Cost (€)";

        // If there are no charges tracked, still generate one page, the table header etc.
        // We will skip creating the table content later.
        bool any_charges_tracked = charge_records > 0;
        if (!any_charges_tracked)
            charge_records = 1;

        init_pdf_generator(&request,
                           english ? "WARP Charge Log" : "WARP Ladelog",
                           stats_buf, (electricity_price == 0) ? 5 : 6,
                           letterhead.get(), letterhead_lines,
                           english ? table_header_en : table_header_de,
                           charge_records,
                           [this,
                            user_filter,
                            &table_lines_buffer,
                            &f,
                            first_file,
                            first_charge,
                            last_file,
                            last_charge,
                            &current_file,
                            &current_charge,
                            electricity_price,
                            english,
                            configured_users,
                            &display_name_cache,
                            any_charges_tracked]
                           (const char * * table_lines) {
            memset(table_lines_buffer, 0, ARRAY_SIZE(table_lines_buffer));

            int lines_generated = 0;
            char *table_lines_head = table_lines_buffer;

            char charge_buf[CHARGE_RECORD_SIZE];
            ChargeStart cs;
            ChargeEnd ce;

            // Skip creating the table content if there were no charges tracked.
            // charge_records is set to 1 in this case to still generate the page,
            // table header etc.
            while (any_charges_tracked && current_file <= last_file) {
                if (current_charge >= (CHARGE_RECORD_MAX_FILE_SIZE / CHARGE_RECORD_SIZE)) {
                    current_charge = 0;
                }

                if (!f) {
                    f =  LittleFS.open(chargeRecordFilename(current_file));
                    f.seek(CHARGE_RECORD_SIZE * current_charge);
                }

                for (; current_charge < (CHARGE_RECORD_MAX_FILE_SIZE / CHARGE_RECORD_SIZE); ++current_charge) {
                    if ((lines_generated == 8) || (current_file == last_file && current_charge > last_charge))
                        break;
                    if (f.read((uint8_t *)charge_buf, CHARGE_RECORD_SIZE) != CHARGE_RECORD_SIZE)
                        break;

                    memcpy(&cs, charge_buf, sizeof(ChargeStart));
                    memcpy(&ce, charge_buf + sizeof(ChargeStart), sizeof(ChargeEnd));

                    // No need to filter via start/end: we already know the first and last charge to be shown.
                    bool include_user = user_filter == USER_FILTER_ALL_USERS || (user_filter == USER_FILTER_DELETED_USERS && !user_configured(configured_users, cs.user_id)) || cs.user_id == user_filter;
                    if (!include_user)
                        continue;


                    table_lines_head = tracked_charge_to_string(table_lines_head, cs, ce, english, electricity_price, display_name_cache);
                    ++lines_generated;
                }

                if (current_charge >= (CHARGE_RECORD_MAX_FILE_SIZE / CHARGE_RECORD_SIZE)) {
                    f.close();
                    ++current_file;
                    current_charge = 0;
                }

                // >= last_charge is intentionally here: we already have this charge
                if ((lines_generated == 8))
                    break;

                if (current_file == last_file && current_charge >= last_charge) {
                    f.close();
                    break;
                }
            }

            *table_lines = table_lines_buffer;

            // The HTTP task generating the PDF has a higher priority than the main loop and will starve it of CPU time.
            // Suspend execution for a moment after every block so that the main loop has a chance to run.
            vTaskDelay(1);

            return lines_generated;
        });
        free(display_name_cache);
        logger.printfln("PDF generation done.");
        return request.endChunkedResponse();
    });
}
