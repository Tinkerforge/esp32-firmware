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
#include "tools/malloc.h"
#include "tools/fs.h"
#include "pdf_charge_log.h"
#include "file_type.enum.h"

#define PDF_LETTERHEAD_MAX_SIZE 512

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

static int read_letterhead_lines(char *letterhead) {
    int letterhead_lines = 1;

    for (size_t i = 0; i < PDF_LETTERHEAD_MAX_SIZE; ++i) {
        if (letterhead[i] == '\0') {
            break;
        }

        if (letterhead[i] == '\n') {
            letterhead[i] = '\0';

            if (letterhead_lines == 6) {
                break;
            }

            ++letterhead_lines;
        }
    }
    return letterhead_lines;
}

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

#if MODULE_REMOTE_ACCESS_AVAILABLE()
    charge_log_send_prototype = Config::Object({
        {"user_id", Config::Int8(-1)},
        {"file_type", Config::Enum(FileType::PDF)},
        {"english", Config::Bool(false)},
        {"letterhead", Config::Str("", 0, PDF_LETTERHEAD_MAX_SIZE)},
        {"user_filter", Config::Int(0)},
    });
#endif

    config = Config::Object({
        {"electricity_price", Config::Uint16(0)},
#if MODULE_REMOTE_ACCESS_AVAILABLE()
        {"charge_log_send", Config::Array(
            {},
            &charge_log_send_prototype,
            0, OPTIONS_REMOTE_ACCESS_MAX_USERS(), Config::type_id<Config::ConfObject>()
        )},
        {"last_file_send", Config::Uint32(0)},
#endif
    });

    pdf_letterhead_config = Config::Object({
        {"letterhead", Config::Str("", 0, PDF_LETTERHEAD_MAX_SIZE)}
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
    char buf[64];
    StringWriter sw(buf, sizeof(buf));

    sw.printf(CHARGE_RECORD_FOLDER "/charge-record-%lu.bin", i);

    return String(buf, sw.getLength());
}

bool ChargeTracker::repair_last(float meter_start)
{
    Charge charges[3];
    charges[0].ce.meter_end = NAN;
    charges[2].cs.meter_start = meter_start;

    if (!file_exists(LittleFS, chargeRecordFilename(last_charge_record)))
        return true;

    File r_file = LittleFS.open(chargeRecordFilename(last_charge_record), "r+");
    if (r_file.size() % CHARGE_RECORD_SIZE != 0) {
        logger.printfln("Can't repair last charge: Last charge end was not tracked or file is damaged! Offset is %u bytes. Expected 0", r_file.size() % CHARGE_RECORD_SIZE);
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
            auto tmp_size = tmp.size();
            if (tmp_size % CHARGE_RECORD_SIZE != 0 || tmp_size < sizeof(Charge) * 2) {
                logger.printfln("Can't repair last charge: Penultimate tracked charge file is damaged! Offset is %u bytes. Expected 0", tmp_size % CHARGE_RECORD_SIZE);
                return false;
            }
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
    bool have_user_to_delete = false;

    while (this->last_charge_record - this->first_charge_record >= 30) {
        String name = chargeRecordFilename(this->first_charge_record);
        logger.printfln("Got %lu charge records. Dropping the first one (%s)", this->last_charge_record - this->first_charge_record, name.c_str());
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
                have_user_to_delete = true;
            }
        }
        LittleFS.remove(name);
        ++this->first_charge_record;
    }

    // Skip parsing all charge records if there aren't any users to delete.
    if (!have_user_to_delete) {
        return;
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

    // Two more to handle power cycles where a new record was created but the oldest one was not yet deleted.
    uint32_t found_blobs[CHARGE_RECORD_FILE_COUNT + 2] = {0};
    size_t found_blobs_size = ARRAY_SIZE(found_blobs);
    int found_blob_counter = 0;

    while (File f = folder.openNextFile()) {
        String name{f.name()};
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

        if (found_blob_counter >= found_blobs_size) {
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

    logger.printfln("Found %u record%s: first is %lu, last is %lu", found_blob_counter, found_blob_counter == 1 ? "" : "s", first, last);
    for (int i = 0; i < found_blob_counter - 1; ++i) {
        if (found_blobs[i] + 1 != found_blobs[i + 1]) {
            logger.printfln("Non-consecutive charge records found! (Next after %lu is %lu. Expected was %lu", found_blobs[i], found_blobs[i+1], found_blobs[i] + 1);
            return false;
        }

        const String fname = chargeRecordFilename(found_blobs[i]);
        const size_t fsize = file_size(LittleFS, fname);
        if (fsize != CHARGE_RECORD_MAX_FILE_SIZE) {
            logger.printfln("Charge record %s doesn't have max size: %u bytes", fname.c_str(), fsize);
            return false;
        }
    }

    const String last_file_name = chargeRecordFilename(found_blobs[found_blob_counter - 1]);
    const size_t last_file_size = file_size(LittleFS, last_file_name);
    logger.printfln("Last charge record size is %u (%u, %u)", last_file_size, last_file_size / CHARGE_RECORD_SIZE, (last_file_size % CHARGE_RECORD_SIZE));
    if (last_file_size > CHARGE_RECORD_MAX_FILE_SIZE) {
        logger.printfln("Last charge record %s is too long: %u bytes", last_file_name.c_str(), last_file_size);
        return false;
    }

    this->first_charge_record = first;
    this->last_charge_record = last;

    return true;
}

size_t ChargeTracker::completeRecordsInLastFile()
{
    const size_t fsize = file_size(LittleFS, chargeRecordFilename(this->last_charge_record));
    return fsize / CHARGE_RECORD_SIZE;
}

bool ChargeTracker::currentlyCharging()
{
    const size_t fsize = file_size(LittleFS, chargeRecordFilename(this->last_charge_record));
    return (fsize % CHARGE_RECORD_SIZE) == sizeof(ChargeStart);
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

    const String last_file_name = chargeRecordFilename(this->last_charge_record);
    if (!file_exists(LittleFS, last_file_name))
        LittleFS.open(last_file_name, "w", true);

    repair_charges();

    api.restorePersistentConfig("charge_tracker/config", &config);
    api.restorePersistentConfig("charge_tracker/pdf_letterhead_config", &pdf_letterhead_config);

    // Fill charge_tracker/last_charges
    bool charging = currentlyCharging();
    size_t records_in_last_file = completeRecordsInLastFile();

    const String penultimate_file_name = chargeRecordFilename(this->last_charge_record - 1);
    if (records_in_last_file < CHARGE_RECORD_LAST_CHARGES_SIZE && file_exists(LittleFS, penultimate_file_name)) {
        size_t records_to_read = CHARGE_RECORD_LAST_CHARGES_SIZE - records_in_last_file;
        File f = LittleFS.open(penultimate_file_name);
        f.seek(-(records_to_read * CHARGE_RECORD_SIZE), SeekMode::SeekEnd);

        this->readNRecords(&f, records_to_read);
    }

    size_t records_to_read = min(records_in_last_file, (size_t)CHARGE_RECORD_LAST_CHARGES_SIZE);
    if (records_to_read > 0) {
        File f = LittleFS.open(last_file_name);
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

    ret_buf[display_name_cache[user_id].length] = '\0';
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
            buf += 1 + sprintf_u(buf, "%ld%c%02ld", cost / 100, english ? '.' : ',', cost % 100);
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
        logger.printfln("Repaired %lu charge-entries.", num_repaired);
    }
}

void ChargeTracker::register_urls()
{
    // We have to do this here, not at the end of setup,
    // because removeOldRecords calls users.remove_from_username_file
    // which requires that the user config is already loaded from flash.
    // This happens in users::setup() i.e. _after_ charge_tracker::setup()
    removeOldRecords();
    updateState();

    api.addPersistentConfig("charge_tracker/config", &config);
    api.addPersistentConfig("charge_tracker/pdf_letterhead_config", &pdf_letterhead_config, {}, {"letterhead"});

    server.on_HTTPThread("/charge_tracker/charge_log", HTTP_GET, [this](WebServerRequest request) {
        std::lock_guard<std::mutex> lock{records_mutex};

        auto url_buf = heap_alloc_array<char>(CHARGE_RECORD_MAX_FILE_SIZE);
        if (url_buf == nullptr) {
            return request.send(507);
        }

        const size_t fsize = file_size(LittleFS, chargeRecordFilename(this->last_charge_record));
        const size_t file_size = (this->last_charge_record - this->first_charge_record) * CHARGE_RECORD_MAX_FILE_SIZE + fsize;

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
    api.addState("charge_tracker/current_charge", &current_charge, {}, {"tag_id", "tag_type"});
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
        auto letterhead_buf = heap_alloc_array<char>(PDF_LETTERHEAD_MAX_SIZE + 1);
        auto letterhead = letterhead_buf.get();
        int letterhead_lines = 0;

        {
            StaticJsonDocument<192> doc;
            auto buf = heap_alloc_array<char>(1024);

            if (request.contentLength() > 1024) {
                return request.send(413);
            }

            auto received = request.receive(buf.get(), 1024);

            if (received < 0) {
                return request.send(500, "text/plain", "Failed to receive request payload");
            }

            DeserializationError error = deserializeJson(doc, buf.get(), received);

            if (error) {
                char error_string[64];
                StringWriter sw(error_string, ARRAY_SIZE(error_string));
                sw.puts("Failed to deserialize string: ");
                sw.puts(error.c_str());
                return request.send(400, "text/plain", error_string, static_cast<ssize_t>(sw.getLength()));
            }

            if (!bool(doc["api_not_final_acked"])) {
                return request.send(400, "text/plain", "Please acknowledge that this API is subject to change!");
            }

            user_filter = doc["user_filter"] | USER_FILTER_ALL_USERS;
            start_timestamp_min = doc["start_timestamp_min"] | 0l;
            end_timestamp_min = doc["end_timestamp_min"] | 0l;
            english = doc["english"] | false;

            if (current_timestamp_min == 0) {
                current_timestamp_min = doc["current_timestamp_min"] | 0l;
            }

            bool letterhead_passed = doc.containsKey("letterhead") && !doc["letterhead"].isNull();

            if (letterhead_passed) {
                if (strlen(doc["letterhead"]) > PDF_LETTERHEAD_MAX_SIZE) {
                    return request.send(400, "text/plain", "Letterhead is too long!");
                }

                strncpy(letterhead, doc["letterhead"], PDF_LETTERHEAD_MAX_SIZE + 1);
            }

            task_scheduler.await([this, letterhead, letterhead_passed](){
                auto saved_letterhead = this->pdf_letterhead_config.get("letterhead");
                if (!letterhead_passed) {
                    strncpy(letterhead, saved_letterhead->asEphemeralCStr(), PDF_LETTERHEAD_MAX_SIZE + 1);
                    return;
                }

                if (saved_letterhead->asString() != letterhead) {
                    saved_letterhead->updateString(letterhead);
                    API::writeConfig("charge_tracker/pdf_letterhead_config", &this->pdf_letterhead_config);
                }
            });

            letterhead_lines = read_letterhead_lines(letterhead);
        }

        const auto callback = [this, &request](const void *data, size_t len, bool last_data) -> int {
            return request.sendChunk(static_cast<const char *>(data), len);
        };

        request.beginChunkedResponse(200, "application/pdf");
        this->generate_pdf(callback, user_filter, start_timestamp_min, end_timestamp_min, current_timestamp_min, english, letterhead, letterhead_lines, &request);
        return request.endChunkedResponse();
    });

#if MODULE_REMOTE_ACCESS_AVAILABLE()
    if (config.get("charge_log_send")->count() > 0) {
        task_scheduler.scheduleWithFixedDelay([this]() {
            timeval tv;
            bool is_synced = rtc.clock_synced(&tv);
            if (!is_synced || upload_in_progress) {
                return;
            }

            tm now;
            localtime_r(&tv.tv_sec, &now);
            uint32_t current_minutes = tv.tv_sec / 60;

            tm last_send;
            uint32_t last_send_minutes = config.get("last_file_send")->asUint();
            time_t last_send_time = last_send_minutes * 60;
            localtime_r(&last_send_time, &last_send);

            tm last_month_start = last_send;
            tm last_month_end = last_send;
            last_month_start.tm_mday = 1;
            last_month_end.tm_mday = 1;
            last_month_end.tm_mon++;
            last_month_end.tm_hour = 0;
            last_month_end.tm_min = 0;
            time_t last_month_start_tv = mktime(&last_month_start);
            time_t last_month_end_tv = mktime(&last_month_end);

            SendChargeLogArgs *args = new SendChargeLogArgs;;
            args->that = this;
            args->last_month_start_min = last_month_start_tv / 60;
            args->last_month_end_min = last_month_end_tv / 60;
            args->array_size = config.get("charge_log_send")->count();

            if (upload_retry_count > 0 && current_minutes >= next_retry_time_minutes) {
                logger.printfln("Retrying PDF upload (attempt %lu/%lu)", upload_retry_count + 1, MAX_RETRY_COUNT + 1);
                server.runInHTTPThread([](void *arg) {
                    auto send_chargelog_args = static_cast<SendChargeLogArgs *>(arg);
                    auto that = send_chargelog_args->that;
                    for (int i = 0; i < send_chargelog_args->array_size; ++i) {
                        that->send_pdf(send_chargelog_args->last_month_start_min,
                                    send_chargelog_args->last_month_end_min, i);
                    }
                    that->upload_in_progress = false;
                    delete send_chargelog_args;
                }, args);
                return;
            }

            bool should_send = false;
            if (now.tm_mday >= 2 &&
                (last_send_minutes == 0
                    || last_send.tm_year != now.tm_year
                    || last_send.tm_mon != now.tm_mon))
            {
                should_send = true;
            }
            if (should_send) // Only start new upload if not retrying
            {
                server.runInHTTPThread([](void *arg) {
                    auto send_chargelog_args = static_cast<SendChargeLogArgs *>(arg);
                    auto that = send_chargelog_args->that;
                    for (int i = 0; i < send_chargelog_args->array_size; ++i) {
                        that->send_pdf( send_chargelog_args->last_month_start_min,
                                    send_chargelog_args->last_month_end_min, i);
                    }
                    that->upload_in_progress = false;
                    delete send_chargelog_args;
                }, args);
            } else {
                delete args;
            }
        }, 10_s, 10_s);
    }

    api.addCommand("charge_tracker/reset_last_send", Config::Null(), {}, [this](String &errmsg) {
        timeval tv;
        bool is_synced = rtc.clock_synced(&tv);
        if (!is_synced) {
            errmsg = "RTC not synced";
            return;
        }

        tm now;
        localtime_r(&tv.tv_sec, &now);

        // Set to last month
        if (now.tm_mon == 0) {
            now.tm_mon = 11;
            now.tm_year--;
        } else {
            now.tm_mon--;
        }

        time_t last_month_time = mktime(&now);
        uint32_t last_month_minutes = last_month_time / 60;

        config.get("last_file_send")->updateUint(last_month_minutes);
        API::writeConfig("charge_tracker/config", &config);
    }, true);
#endif
}

// since this function can block for a long time, it must not be called from the main thread
void ChargeTracker::send_pdf(uint32_t start_timestamp_min, uint32_t end_timestamp_min,
                             int user_idx) {
    if (upload_in_progress) {
        return;
    }
    upload_in_progress = true;
    logger.printfln("Starting PDF generation and remote upload...");

    String charger_uuid;
    String password;
    String user_uuid;
    String url;
    int cert_id;
    std::unique_ptr<char[]> letterhead;
    int letterhead_lines;
    int user_filter;
    bool english;

    auto ret = task_scheduler.await([this, &charger_uuid, &password, &user_uuid, &url, &cert_id,
            &letterhead, &letterhead_lines, &user_filter, &english, user_idx]()
    {
        Config::Wrap charge_log_send = config.get("charge_log_send")->get(user_idx);
        letterhead = heap_alloc_array<char>(PDF_LETTERHEAD_MAX_SIZE + 1);
        strncpy(letterhead.get(), charge_log_send->get("letterhead")->asEphemeralCStr(), PDF_LETTERHEAD_MAX_SIZE + 1);
        letterhead_lines = read_letterhead_lines(letterhead.get());
        user_filter = charge_log_send->get("user_filter")->asInt();
        english = charge_log_send->get("english")->asBool();
        const int user_id = charge_log_send->get("user_id")->asInt();

        charger_uuid = remote_access.config.get("uuid")->asString();
        password = remote_access.config.get("password")->asString();
        if (remote_access.config.get("users")->count() > 0) {
            for (const auto &user : remote_access.config.get("users")) {
                if (user.get("id")->asUint() == user_id) {
                    user_uuid = user.get("uuid")->asString();
                    break;
                }
            }
        }
        if (charger_uuid.isEmpty() || password.isEmpty() || user_uuid.isEmpty()) {
            logger.printfln("Missing remote access credentials: charger_uuid=%s, password=%s, user_uuid=%s",
                            charger_uuid.c_str(), password.c_str(), user_uuid.c_str());
            return -1;
        }

        url = "https://" + remote_access.config.get("relay_host")->asString() + "/api/send_chargelog_to_user";
        cert_id = remote_access.config.get("cert_id")->asInt();
        return 0;
    });
    if (ret != TaskScheduler::AwaitResult::Done) {
        return;
    }

    remote_client = std::make_unique<AsyncHTTPSClient>();
    remote_client->set_header("Content-Type", "application/json");

    if (remote_client->start_chunked_request(url.c_str(), cert_id, HTTP_METHOD_POST) == -1) {
        handle_upload_retry();
        logger.printfln("Failed to send Charge-Log");
        return;
    };

    String json_header = "{";
    json_header += "\"charger_uuid\":\"" + charger_uuid + "\",";
    json_header += "\"password\":\"" + password + "\",";
    json_header += "\"user_uuid\":\"" + user_uuid + "\",";
    json_header += "\"chargelog\":[";
    remote_client->send_chunk(json_header.c_str(), json_header.length());

    bool first = true;

    auto pdf_stream_cb = [this, &first](const void *data, size_t len, bool last_data) -> int {
        const uint8_t *data_u8 = static_cast<const uint8_t *>(data);
        String chunk;
        for (size_t i = 0; i < len; ++i) {
            if (!first || i > 0) chunk += ",";
            chunk += String(data_u8[i]);
        }
        if (len > 0) first = false;
        int sent = remote_client->send_chunk(chunk.c_str(), chunk.length());
        if (!chunk.isEmpty() && sent != chunk.length()) {
            logger.printfln("Failed to send PDF chunk to remote access server");
            return -1; // Indicate error
        }
        return ESP_OK;
    };

    this->generate_pdf(
        pdf_stream_cb,
        user_filter,
        start_timestamp_min,
        end_timestamp_min,
        rtc.timestamp_minutes(),
        english, letterhead.get(), letterhead_lines, nullptr);

    String json_footer = "]}";
    remote_client->send_chunk(json_footer.c_str(), json_footer.length());
    remote_client->finish_chunked_request();

    task_scheduler.scheduleOnce([this]() {
        this->check_remote_client_status();
    }, 1_s);
}

void ChargeTracker::handle_upload_retry()
{
    upload_retry_count++;
    uint32_t delay_minutes = BASE_RETRY_DELAY_MINUTES * (1 << (upload_retry_count - 1));
    if (delay_minutes > 480) {
        delay_minutes = 480;
    }

    if (upload_retry_count <= MAX_RETRY_COUNT) {
        next_retry_time_minutes = rtc.timestamp_minutes() + delay_minutes;

        logger.printfln("PDF upload failed. Retry %lu/%lu scheduled in %lu minutes",
                      upload_retry_count, MAX_RETRY_COUNT, delay_minutes);
    } else {
        upload_retry_count = MAX_RETRY_COUNT;
        next_retry_time_minutes = delay_minutes;
        logger.printfln("PDF upload failed. Maximum retry attempts (%lu) exceeded. Giving up.",
                      MAX_RETRY_COUNT);
    }
}

void ChargeTracker::check_remote_client_status()
{
#if MODULE_REMOTE_ACCESS_AVAILABLE()
    if (!remote_client) {
        logger.printfln("PDF generation and remote upload completed. Client was destroyed.");
        return;
    }

    int status = remote_client->read_response_status();
    if (status == ESP_ERR_HTTP_EAGAIN) {
        task_scheduler.scheduleOnce([this]() {
            this->check_remote_client_status();
        }, 1_s);
    } else if (status == 200) {
        upload_retry_count = 0;
        next_retry_time_minutes = 0;
        config.get("last_file_send")->updateUint(rtc.timestamp_minutes());
        API::writeConfig("charge_tracker/config", &config);
        logger.printfln("PDF generation and remote upload completed successfully. Status: %d", status);
        remote_client->close_chunked_request();
        remote_client = nullptr;
        upload_in_progress = false;
    } else {
        logger.printfln("PDF generation and remote upload failed. Status: %d", status);
        handle_upload_retry();
        remote_client->close_chunked_request();
        remote_client = nullptr;
        upload_in_progress = false;
    }
#endif
}

void ChargeTracker::generate_pdf(
    std::function<int(const void *data, size_t len, bool last_data)> &&callback,
    int user_filter,
    uint32_t start_timestamp_min,
    uint32_t end_timestamp_min,
    uint32_t current_timestamp_min,
    bool english,
    const char *letterhead,
    int letterhead_lines,
    WebServerRequest *request
) {
    char stats_buf[384];
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
    String dev_name = "unknown device";
    auto await_result = task_scheduler.await([this, configured_users, &electricity_price, &dev_name]() mutable {
        electricity_price = this->config.get("electricity_price")->asUint();
        for (size_t i = 0; i < users.config.get("users")->count(); ++i) {
            configured_users[i] = users.config.get("users")->get(i)->get("id")->asUint();
        }
#if MODULE_DEVICE_NAME_AVAILABLE()
        dev_name = device_name.display_name.get("display_name")->asString();
        if (device_name.display_name.get("display_name")->asString() != device_name.name.get("name")->asString())
            dev_name += " (" + device_name.name.get("name")->asString() + ")";
#endif
    });
    if (await_result == TaskScheduler::AwaitResult::Timeout) {
        if (request != nullptr) {
            request->send(500, "text/plain", "Failed to generate PDF: Task timed out");
        } else {
            logger.printfln("Failed to generate PDF: Task timed out");
        }
        return;
    }
    {
        char charge_buf[sizeof(ChargeStart) + sizeof(ChargeEnd)];
        ChargeStart cs;
        ChargeEnd ce;
        for (int i = this->first_charge_record; i <= this->last_charge_record; ++i) {
            File f = LittleFS.open(chargeRecordFilename(i));
            for (int j = 0; j < (CHARGE_RECORD_MAX_FILE_SIZE / CHARGE_RECORD_SIZE); ++j) {
                if (f.read((uint8_t *)charge_buf, CHARGE_RECORD_SIZE) != CHARGE_RECORD_SIZE)
                    goto search_done;
                memcpy(&cs, charge_buf, sizeof(ChargeStart));
                memcpy(&ce, charge_buf + sizeof(ChargeStart), sizeof(ChargeEnd));
                if (cs.timestamp_minutes != 0 && start_timestamp_min != 0 && cs.timestamp_minutes < start_timestamp_min) {
                    charge_records = 0;
                    first_file = -1;
                    first_charge = -1;
                    charged_sum = 0;
                    charged_cost_sum = 0;
                    continue;
                }
                if (cs.timestamp_minutes != 0 && end_timestamp_min != 0 && cs.timestamp_minutes > end_timestamp_min) {
                    last_file = i;
                    last_charge = j;
                    goto search_done;
                }
                bool include_user = user_filter == -2 || (user_filter == -1 && !user_configured(configured_users, cs.user_id)) || cs.user_id == user_filter;
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
    display_name_entry *display_name_cache = static_cast<decltype(display_name_cache)>(malloc_iram_or_psram_or_dram(MAX_PASSIVE_USERS * sizeof(display_name_cache[0])));
    if (!display_name_cache) {
        if (request != nullptr) {
            request->send(500, "text/plain", "Failed to generate PDF: No memory");
        } else {
            logger.printfln("Failed to generate PDF: No memory");
        }
        return;
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
    stats_head += sprintf_u(stats_head, "%s: ", english ? "Total energy of exported charges" : "Gesamtenergie exportierter Ladevorgänge");
    if (charged_sum <= 999999999.999f) {
        int written = sprintf_u(stats_head, "%.3f kWh", charged_sum);
        if (!english)
            for (int i = 0; i < written; ++i)
                if (stats_head[i] == '.')
                    stats_head[i] = ',';
        stats_head += 1 + written;
    }
    else {
        memcpy(stats_head, ">=1000000000 kWh", ARRAY_SIZE(">=1000000000 kWh"));
        stats_head += ARRAY_SIZE(">=1000000000 kWh");
    }
    if (electricity_price != 0) {
        int written = sprintf_u(stats_head, "%s: %ld.%02ld€ (%.2f ct/kWh)%s",
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
    std::lock_guard<std::mutex> lock2{pdf_mutex};
    int current_file = (first_file > -1 ? first_file : this->first_charge_record);
    int current_charge = (first_charge > -1 ? first_charge : 0);
    last_file = (last_file >= 0) ? last_file : this->last_charge_record;
#define TABLE_LINE_LEN (17 + 33 + 8 + 11 + 16 + 8)
    char table_lines_buffer[8 * TABLE_LINE_LEN];
    File f;
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
    bool any_charges_tracked = charge_records > 0;
    if (!any_charges_tracked)
        charge_records = 1;
    init_pdf_generator(callback,
                       english ? "WARP Charge Log" : "WARP Ladelog",
                       stats_buf, (electricity_price == 0) ? 5 : 6,
                       letterhead, letterhead_lines,
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
                bool include_user = user_filter == -2 || (user_filter == -1 && !user_configured(configured_users, cs.user_id)) || cs.user_id == user_filter;
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
            if ((lines_generated == 8))
                break;
            if (current_file == last_file && current_charge >= last_charge) {
                f.close();
                break;
            }
        }
        *table_lines = table_lines_buffer;
        vTaskDelay(1);
        return lines_generated;
    });
    free(display_name_cache);
    logger.printfln("PDF generation done.");
    callback(nullptr, 0, true);
}
