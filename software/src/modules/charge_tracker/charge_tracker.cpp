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

#include <esp_task.h>
#include <memory>
#include <LittleFS.h>
#include <stdlib_noniso.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools/malloc.h"
#include "tools/fs.h"
#include "pdf_charge_log.h"
#include "csv_charge_log.h"
#include "file_type.enum.h"
#include "csv_flavor.enum.h"
#include "generation_state.enum.h"
#include "../system/language.enum.h"
#include "charge_tracker_defs.h"

#define PDF_LETTERHEAD_MAX_SIZE 512

#if defined(__GNUC__)
    #pragma GCC diagnostic push

    // pdPASS expands to an old-style cast that is also useless
    #pragma GCC diagnostic ignored "-Wold-style-cast"
    #pragma GCC diagnostic ignored "-Wuseless-cast"
#endif

static constexpr BaseType_t pdPASS_safe = pdPASS;

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif


static bool repair_logic(Charge *);


#define CHARGE_RECORD_SIZE (sizeof(ChargeStart) + sizeof(ChargeEnd))

static_assert(CHARGE_RECORD_SIZE == 16, "Unexpected size of ChargeStart + ChargeEnd");
static_assert(DISPLAY_NAME_LENGTH == 32, "Unexpected display name length");

#define MAX_CONFIGURED_CHARGELOG_USERS 5
// We currently use an uint8_t to provide the size of the configured users array to the upload task.
static_assert(MAX_CONFIGURED_CHARGELOG_USERS <= 255, "MAX_CONFIGURED_CHARGELOG_USERS must be <= 255");

#define MAX_UPLOAD_RETRIES 5

// 30 files with 256 records each: 7680 records @ ~ max. 10 records per day = ~ 2 years and one month of records.
// Also update frontend when changing this!

// Define static member variable for ChargeLogGenerationLockHelper
std::atomic<GenerationState> ChargeLogGenerationLockHelper::generation_lock_state{GenerationState::Ready};

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

String chargeRecordFilename(uint32_t i)
{
    char buf[64];
    StringWriter sw(buf, sizeof(buf));
    sw.printf(CHARGE_RECORD_FOLDER "/charge-record-%lu.bin", i);
    return String(buf, sw.getLength());
}

static bool wants_send(uint32_t current_time_min, uint32_t last_send_time_min)
{
    if (last_send_time_min == 0) {
        return true;
    }

    time_t last_send_time = static_cast<time_t>(last_send_time_min) * 60;
    tm last_send;
    localtime_r(&last_send_time, &last_send);

    time_t current_time = static_cast<time_t>(current_time_min) * 60;
    tm current;
    localtime_r(&current_time, &current);

    // Only send once per month
    if (last_send.tm_year == current.tm_year && last_send.tm_mon == current.tm_mon) {
        return false;
    }

    return true;
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
        {"first_charge_timestamp", Config::Uint32(0)},
        {"generator_state", Config::Enum(GenerationState::Ready)}
    });

#if MODULE_REMOTE_ACCESS_AVAILABLE()
    charge_log_send_prototype = Config::Object({
        {"user_id", Config::Int(-1, -2, 255)},
        {"file_type", Config::Enum(FileType::PDF)},
        {"language", Config::Enum(Language::German)},
        {"letterhead", Config::Str("", 0, PDF_LETTERHEAD_MAX_SIZE)},
        {"user_filter", Config::Int8(0)},
        {"csv_delimiter", Config::Enum(CSVFlavor::Excel)},
        {"last_upload_timestamp_min", Config::Uint32(0)}
    });
#endif

    config = Config::Object({
        {"electricity_price", Config::Uint16(0)},
#if MODULE_REMOTE_ACCESS_AVAILABLE()
        {"remote_upload_configs", Config::Array(
            {},
            &charge_log_send_prototype,
            0, MAX_CONFIGURED_CHARGELOG_USERS, Config::type_id<Config::ConfObject>()
        )},
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

    last_charges.reserve(CHARGE_RECORD_LAST_CHARGES_SIZE);

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

static size_t timestamp_min_to_date_time_string(char buf[17], uint32_t timestamp_min, Language language)
{
    const char * const unknown = (language == Language::English) ? "unknown" : "unbekannt";
    size_t unknown_len =  (language == Language::English) ? ARRAY_SIZE("unknown") : ARRAY_SIZE("unbekannt");

    if (timestamp_min == 0) {
        memcpy(buf, unknown, unknown_len);
        return unknown_len - 1; // exclude null terminator
    }
    time_t timestamp = ((int64_t)timestamp_min) * 60;
    struct tm t;
    localtime_r(&timestamp, &t);

    if (language == Language::English)
        return sprintf_u(buf, "%4.4i-%2.2i-%2.2i %2.2i:%2.2i", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min);

    return sprintf_u(buf, "%2.2i.%2.2i.%4.4i %2.2i:%2.2i", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min);
}

size_t get_display_name(uint8_t user_id, char *ret_buf, display_name_entry *display_name_cache)
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

static char *tracked_charge_to_string(char *buf, ChargeStart cs, ChargeEnd ce, Language language, uint32_t electricity_price, display_name_entry *display_name_cache)
{
    buf += 1 + timestamp_min_to_date_time_string(buf, cs.timestamp_minutes, language);

    size_t name_len = get_display_name(cs.user_id, buf, display_name_cache);
    buf += 1 + name_len;

    if (charged_invalid(cs, ce)) {
        memcpy(buf, "N/A", ARRAY_SIZE("N/A"));
        buf += ARRAY_SIZE("N/A");
    } else {
        float charged = ce.meter_end - cs.meter_start;
        if (charged <= 999.999f) {
            int written = sprintf_u(buf, "%.3f", charged);
            if (language == Language::German)
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
        if (language == Language::German)
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
            buf += 1 + sprintf_u(buf, "%ld%c%02ld", cost / 100, (language == Language::English) ? '.' : ',', cost % 100);
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
            return request.send_plain(507);
        }

        const size_t fsize = file_size(LittleFS, chargeRecordFilename(this->last_charge_record));
        const size_t file_size = (this->last_charge_record - this->first_charge_record) * CHARGE_RECORD_MAX_FILE_SIZE + fsize;

        // Don't do a chunked response without any chunk. The webserver does strange things in this case
        if (file_size == 0) {
            return request.send_bytes(200);
        }

        request.beginChunkedResponse_bytes(200);
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

        Language language = Language::German;
        auto letterhead_buf = heap_alloc_array<char>(PDF_LETTERHEAD_MAX_SIZE + 1);
        auto letterhead = letterhead_buf.get();
        int letterhead_lines = 0;
        std::unique_ptr<ChargeLogGenerationLockHelper> lock_helper = ChargeLogGenerationLockHelper::try_lock(GenerationState::LocalDownload);
        if (lock_helper == nullptr) {
            return request.send_plain(429, "Another charge log generation is already in progress");
        }

        {
            if (request.contentLength() > 1024) {
                return request.send_plain(413);
            }

            auto buf = heap_alloc_array<char>(1024);
            auto received = request.receive(buf.get(), 1024);

            if (received < 0) {
                return request.send_plain(500, "Failed to receive request payload");
            }

            StaticJsonDocument<192> doc;
            DeserializationError error = deserializeJson(doc, buf.get(), received);

            if (error) {
                char error_string[64];
                StringWriter sw(error_string, ARRAY_SIZE(error_string));
                sw.puts("Failed to deserialize string: ");
                sw.puts(error.c_str());
                return request.send_plain(400, sw);
            }

            if (!bool(doc["api_not_final_acked"])) {
                return request.send_plain(400, "Please acknowledge that this API is subject to change!");
            }

            user_filter = doc["user_filter"];
            start_timestamp_min = doc["start_timestamp_min"];
            end_timestamp_min = doc["end_timestamp_min"];
            uint8_t language_val = doc["language"];
            language = static_cast<Language>(language_val);

            if (current_timestamp_min == 0) {
                current_timestamp_min = doc["current_timestamp_min"];
            }

            bool letterhead_passed = doc.containsKey("letterhead") && !doc["letterhead"].isNull();

            if (letterhead_passed) {
                if (strlen(doc["letterhead"]) > PDF_LETTERHEAD_MAX_SIZE) {
                    return request.send_plain(400, "Letterhead is too long!");
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

        const auto callback = [this, &request](const void *data, size_t len) -> int {
            // sendChunk complains if called with length == 0, because this indicates the end of the chunked response.
            // endChunkedResponse is called after the PDF generation is done.
            if (len == 0)
                return ESP_OK;
            return request.sendChunk(static_cast<const char *>(data), len);
        };

        request.beginChunkedResponse(200, "application/pdf");
        this->generate_pdf(callback, user_filter, start_timestamp_min, end_timestamp_min, current_timestamp_min, language, letterhead, letterhead_lines, &request);
        return request.endChunkedResponse();
    });

    server.on_HTTPThread("/charge_tracker/csv", HTTP_PUT, [this](WebServerRequest request) {
        logger.printfln("Beginning CSV generation. Please ignore timeout errors (rc -1 etc.) until it is done.");
        #define USER_FILTER_ALL_USERS -2
        #define USER_FILTER_DELETED_USERS -1
        int user_filter = USER_FILTER_ALL_USERS;
        uint32_t start_timestamp_min = 0;
        uint32_t end_timestamp_min = 0;
        uint32_t current_timestamp_min = rtc.timestamp_minutes();
        Language language = Language::German;
        int csv_delimiter = (int)CSVFlavor::Excel;
        std::unique_ptr<ChargeLogGenerationLockHelper> lock_helper = ChargeLogGenerationLockHelper::try_lock(GenerationState::LocalDownload);
        if (lock_helper == nullptr) {
            return request.send_plain(429, "Another charge log generation is already in progress");
        }

        {
            if (request.contentLength() > 1024) {
                return request.send_plain(413);
            }

            auto buf = heap_alloc_array<char>(1024);
            auto received = request.receive(buf.get(), 1024);

            if (received < 0) {
                return request.send_plain(500, "Failed to receive request payload");
            }

            StaticJsonDocument<192> doc;
            DeserializationError error = deserializeJson(doc, buf.get(), received);

            if (error) {
                char error_string[64];
                StringWriter sw(error_string, ARRAY_SIZE(error_string));
                sw.puts("Failed to deserialize string: ");
                sw.puts(error.c_str());
                return request.send_plain(400, sw);
            }

            if (!bool(doc["api_not_final_acked"])) {
                return request.send_plain(400, "Please acknowledge that this API is subject to change!");
            }

            user_filter = doc["user_filter"];
            start_timestamp_min = doc["start_timestamp_min"];
            end_timestamp_min = doc["end_timestamp_min"];
            uint8_t language_val = doc["language"];
            language = static_cast<Language>(language_val);
            csv_delimiter = doc["csv_delimiter"];

            if (current_timestamp_min == 0) {
                current_timestamp_min = doc["current_timestamp_min"];
            }
        }

        CSVGenerationParams csv_params;
        csv_params.user_filter = user_filter;
        csv_params.start_timestamp_min = start_timestamp_min;
        csv_params.end_timestamp_min = end_timestamp_min;
        csv_params.language = language;
        csv_params.flavor = (CSVFlavor)csv_delimiter;
        task_scheduler.await([this, &csv_params]() {
            csv_params.electricity_price = this->config.get("electricity_price")->asUint();
        });

        const auto callback = [this, &request](const char* buffer, size_t len) -> int {
            return request.sendChunk(buffer, len);
        };

        request.beginChunkedResponse(200, "text/csv");
        CSVChargeLogGenerator csv_generator;
        csv_generator.generateCSV(csv_params, std::move(callback));
        return request.endChunkedResponse();
    });

#if MODULE_REMOTE_ACCESS_AVAILABLE()

    server.on_HTTPThread("/charge_tracker/send_charge_log", HTTP_PUT, [this](WebServerRequest request) {
        std::unique_ptr<ChargeLogGenerationLockHelper> lock_helper = ChargeLogGenerationLockHelper::try_lock(GenerationState::ManualRemoteSend);
        if (lock_helper == nullptr) {
            return request.send_plain(429, "Another charge log generation is already in progress");
        }

        int user_filter = -2;
        uint32_t start_timestamp_min = 0;
        uint32_t end_timestamp_min = 0;
        Language language = Language::German;
        FileType file_type = FileType::PDF;
        int csv_delimiter = (int)CSVFlavor::Excel;
        uint32_t cookie = 0;
        uint32_t remote_upload_config_count = 0;
        String remote_access_user_uuid;

        auto letterhead_buf = heap_alloc_array<char>(PDF_LETTERHEAD_MAX_SIZE + 1);
        auto letterhead = letterhead_buf.get();
        {
            if (request.contentLength() > 1024) {
                return request.send_plain(413);
            }

            auto buf = heap_alloc_array<char>(1024);
            auto received = request.receive(buf.get(), 1024);

            if (received < 0) {
                return request.send_plain(500, "Failed to receive request payload");
            }

            StaticJsonDocument<192> doc;
            DeserializationError error = deserializeJson(doc, buf.get(), received);

            if (error) {
                char error_string[64];
                StringWriter sw(error_string, ARRAY_SIZE(error_string));
                sw.puts("Failed to deserialize string: ");
                sw.puts(error.c_str());
                return request.send_plain(400, sw);
            }

            if (!bool(doc["api_not_final_acked"])) {
                return request.send_plain(400, "Please acknowledge that this API is subject to change!");
            }

            user_filter = doc["user_filter"];
            start_timestamp_min = doc["start_timestamp_min"];
            end_timestamp_min = doc["end_timestamp_min"];
            uint8_t language_val = doc["language"];
            language = static_cast<Language>(language_val);
            uint8_t file_type_val = doc["file_type"];
            file_type = static_cast<FileType>(file_type_val);
            csv_delimiter = doc["csv_delimiter"];
            cookie = doc["cookie"];

            if (!doc.containsKey("cookie")) {
                return request.send_plain(400, "Missing cookie parameter");
            }

            if (!doc.containsKey("remote_access_user_uuid")) {
                return request.send_plain(400, "Missing remote_access_user_uuid parameter");
            }
            remote_access_user_uuid = String(doc["remote_access_user_uuid"].as<const char*>());

            // Handle letterhead for PDF file type
            bool letterhead_passed = doc.containsKey("letterhead") && !doc["letterhead"].isNull();

            if (letterhead_passed) {
                if (strlen(doc["letterhead"]) > PDF_LETTERHEAD_MAX_SIZE) {
                    return request.send_plain(400, "Letterhead is too long!");
                }

                strncpy(letterhead, doc["letterhead"], PDF_LETTERHEAD_MAX_SIZE + 1);
            }

            task_scheduler.await([this, &remote_upload_config_count, letterhead, letterhead_passed](){
                remote_upload_config_count = config.get("remote_upload_configs")->count();

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
        }

        char *letterhead_ptr = letterhead_buf.release();
        ChargeLogGenerationLockHelper *lock_helper_ptr = lock_helper.release();
        task_scheduler.scheduleOnce([this, cookie, user_filter, start_timestamp_min, end_timestamp_min, language, file_type, csv_delimiter, letterhead_ptr, lock_helper_ptr, remote_access_user_uuid]() {
                this->start_charge_log_upload_for_user(
                cookie,
                user_filter,
                start_timestamp_min,
                end_timestamp_min,
                language,
                file_type,
                static_cast<CSVFlavor>(csv_delimiter),
                std::unique_ptr<char[]>(letterhead_ptr),
                std::unique_ptr<ChargeLogGenerationLockHelper>(lock_helper_ptr),
                remote_access_user_uuid);
        });
        return request.send_plain(200, "Charge-log upload for specific config started");
    });

    if (config.get("remote_upload_configs")->count() > 0) {
        task_scheduler.scheduleWithFixedDelay([this]() {
            // In case someone deletes all remote upload configs without restarting, we also stop trying to send.
            if (config.get("remote_upload_configs")->count() == 0) {
                task_scheduler.cancel(task_scheduler.currentTaskId());
                return;
            }

            // only send when no charge is in progress. This avoids not sending a charge that started at the end of the last month
            if (this->currentlyCharging()) {
                return;
            }

            timeval tv;
            bool is_synced = rtc.clock_synced(&tv);
            if (!is_synced) {
                return;
            }

            tm now;
            localtime_r(&tv.tv_sec, &now);

            uint32_t earliest_send = 0;
            for (const Config &cfg : config.get("remote_upload_configs")) {
                const uint32_t last_send_minutes = cfg.get("last_upload_timestamp_min")->asUint();
                if (earliest_send > last_send_minutes) {
                    earliest_send = last_send_minutes;
                }
            }

            // Already sent for this month?
            if (!wants_send(tv.tv_sec / 60, earliest_send)) {
                return;
            }

            this->upload_charge_logs();
        }, 10_s, 4_h);
        // Delay one minute so that all other start-up tasks can finish, because sending the protocols causes high load for quite some time.
        // Check only once every four hours, to avoid a stampede event at midnight on the 2nd of a month.
        // Protocols will thus be uploaded between midnight and 4:00 on the 2nd of a month.
    }

#endif
}

#if MODULE_REMOTE_ACCESS_AVAILABLE()

static void push_upload_result_error(uint32_t cookie, const char *error_msg)
{
    char buf[256];
    TFJsonSerializer json{buf, sizeof(buf)};

    json.addObject();
    json.addMemberNumber("cookie", cookie);
    json.addMemberString("error", error_msg);
    json.endObject();
    json.end();

    ws.pushRawStateUpdate(buf, "charge_tracker/upload_result");
}

static void push_upload_result_success(uint32_t cookie)
{
    char buf[128];
    TFJsonSerializer json{buf, sizeof(buf)};

    json.addObject();
    json.addMemberNumber("cookie", cookie);
    json.addMemberString("error", "");
    json.endObject();
    json.end();

    ws.pushRawStateUpdate(buf, "charge_tracker/upload_result");
}

static esp_err_t check_remote_client_status(std::unique_ptr<RemoteUploadRequest> upload_args)
{
    int status = upload_args->remote_client->read_response_status();
    if (status == ESP_ERR_HTTP_EAGAIN) {
        // The parent task holds the pointer to upload_args and turns it into a unique_ptr again.
        upload_args.release();
        return ESP_ERR_HTTP_EAGAIN;
    } else if (status == 200) {
        logger.printfln("Charge-log generation and remote upload completed successfully. Status: %d", status);
        upload_args->remote_client->close_chunked_request();

        // Push success to websocket for manual uploads
        if (upload_args->use_format_overrides && upload_args->cookie != 0) {
            push_upload_result_success(upload_args->cookie);
        } else {
            task_scheduler.await([&upload_args]() {
                charge_tracker.config.get("remote_upload_configs")->get(static_cast<uint8_t>(upload_args->config_index))->get("last_upload_timestamp_min")->updateUint(rtc.timestamp_minutes());
                API::writeConfig("charge_tracker/config", &charge_tracker.config);
            });
        }
    } else {
        logger.printfln("Charge-log generation and remote upload failed. Status: %d", status);
        upload_args->remote_client->close_chunked_request();

        // Push error to websocket for manual uploads
        if (upload_args->use_format_overrides && upload_args->cookie != 0) {
            char error_msg[64];
            snprintf(error_msg, sizeof(error_msg), "Upload failed with status %d", status);
            push_upload_result_error(upload_args->cookie, error_msg);
        }
    }
    return ESP_OK;
}

static String build_filename(const time_t start, const time_t end, FileType file_type, Language language) {
    struct tm gen_tm;
    time_t generation_time = time(nullptr);
    localtime_r(&generation_time, &gen_tm);

    char buf[128];
    StringWriter fname(buf, std::size(buf));

    fname.printf("%s-%s-%s-%04d-%02d-%02dT%02d-%02d-%02d-%03d.%s",
                 device_name.name.get("type")->asUnsafeCStr(),
                 device_name.name.get("uid" )->asUnsafeCStr(),
                 language == Language::English ? "charge-log" : "Ladelog",
                 gen_tm.tm_year + 1900,
                 gen_tm.tm_mon + 1,
                 gen_tm.tm_mday,
                 gen_tm.tm_hour,
                 gen_tm.tm_min,
                 gen_tm.tm_sec,
                 0, // milliseconds (not available from struct tm)
                 file_type == FileType::PDF ? "pdf" : "csv");

    return fname.toString();
}

static esp_err_t send_binary_chunk(AsyncHTTPSClient &remote_client, const uint8_t *buffer, size_t len)
{
    if (len == 0) {
        return ESP_OK;
    }

    const int sent = remote_client.send_chunk(reinterpret_cast<const char *>(buffer), len);
    if (sent != static_cast<int>(len)) {
        logger.printfln("Failed to send chunk to remote access server");
        return ESP_FAIL;
    }
    return ESP_OK;
}

// since this function can block for a long time, it must not be called from the main thread
void ChargeTracker::send_file(std::unique_ptr<RemoteUploadRequest> upload_args) {
    logger.printfln("Starting charge-log generation and remote upload...");

    String charger_uuid;
    String password;
    String url;
    int cert_id;
    String filename;
    String display_name;

    auto &upload_args_ref = *upload_args;

    auto ret = task_scheduler.await([this, &filename, &charger_uuid, &password, &url, &cert_id,
        &upload_args_ref, &display_name]()
    {
        if (!upload_args_ref.use_format_overrides) {
            Config::Wrap charge_log_send = config.get("remote_upload_configs")->get(static_cast<uint8_t>(upload_args_ref.config_index));

            upload_args_ref.language = charge_log_send->get("language")->asEnum<Language>();
            upload_args_ref.file_type = charge_log_send->get("file_type")->asEnum<FileType>();
            upload_args_ref.csv_delimiter = charge_log_send->get("csv_delimiter")->asEnum<CSVFlavor>();
            upload_args_ref.user_filter = charge_log_send->get("user_filter")->asInt();

            if (upload_args_ref.file_type == FileType::PDF) {
                const String &letterhead_string = charge_log_send->get("letterhead")->asString();
                const size_t letterhead_terminated_length = letterhead_string.length() + 1;

                upload_args_ref.letterhead = heap_alloc_array<char>(letterhead_terminated_length);
                strncpy(upload_args_ref.letterhead.get(), letterhead_string.c_str(), letterhead_terminated_length);
            }
        }

        filename = build_filename(((time_t)upload_args_ref.start_timestamp_min) * 60, ((time_t)upload_args_ref.end_timestamp_min) * 60, upload_args_ref.file_type, upload_args_ref.language);

        const Config *remote_access_config = api.getState("remote_access/config");

        charger_uuid = remote_access_config->get("uuid")->asString();
        password = remote_access_config->get("password")->asString();
        cert_id = remote_access_config->get("cert_id")->asInt();

        display_name = device_name.display_name.get("display_name")->asString();
        if (device_name.display_name.get("display_name")->asString() != device_name.name.get("name")->asString())
            display_name += " (" + device_name.name.get("name")->asString() + ")";

        char sw_url_buf[128];
        StringWriter sw_url(sw_url_buf, std::size(sw_url_buf));

        sw_url.printf("https://%s:%hu/api/send_chargelog_to_user",
                      remote_access_config->get("relay_host")->asUnsafeCStr(),
                      remote_access_config->get("relay_port")->asUint16());

        url = sw_url.toString();
    });

    if (ret != TaskScheduler::AwaitResult::Done) {
        logger.printfln("Failed to await task scheduler for charge log generation");
        if (upload_args->use_format_overrides && upload_args->cookie != 0) {
            push_upload_result_error(upload_args->cookie, "Failed to prepare upload (task scheduler timeout)");
        }
        return;
    }

    if (charger_uuid.isEmpty() || password.isEmpty() || upload_args->remote_access_user_uuid.isEmpty()) {
        logger.printfln("Missing remote access credentials.");
        if (upload_args->use_format_overrides && upload_args->cookie != 0) {
            push_upload_result_error(upload_args->cookie, "Missing remote access credentials");
        }
        return;
    }

    upload_args->remote_client = std::make_unique<AsyncHTTPSClient>();

    // Generate boundary for multipart form-data
    String boundary = "----WebKitFormBoundary" + String(esp_random(), HEX);
    String content_type = "multipart/form-data; boundary=" + boundary;

    upload_args->remote_client->set_header("Content-Type", content_type.c_str());
    const char *lang_header = (upload_args->language == Language::English) ? "en" : "de";
    upload_args->remote_client->set_header("X-Lang", lang_header);

    if (upload_args->remote_client->start_chunked_request(url.c_str(), cert_id, HTTP_METHOD_POST) == -1) {
        logger.printfln("Failed to send charge-log");
        if (upload_args->use_format_overrides && upload_args->cookie != 0) {
            push_upload_result_error(upload_args->cookie, "Failed to start HTTP request");
        }
        return;
    }

    // Send JSON part
    {
        StringBuilder json_part(600);
        json_part.printf("--%s\r\n", boundary.c_str());
        json_part.printf("Content-Disposition: form-data; name=\"json\"\r\n");
        json_part.printf("Content-Type: application/json\r\n\r\n");
        json_part.printf("{"
                         "\"charger_uuid\":\"%s\","
                         "\"password\":\"%s\","
                         "\"user_uuid\":\"%s\","
                         "\"filename\":\"%s\","
                         "\"display_name\":\"%s\","
                         "\"monthly_send\":%s"
                         "}\r\n",
                         charger_uuid.c_str(),
                         password.c_str(),
                         upload_args->remote_access_user_uuid.c_str(),
                         filename.c_str(),
                         display_name.c_str(),
                         upload_args->use_format_overrides ? "false" : "true");

        upload_args->remote_client->send_chunk(json_part.getPtr(), json_part.getLength());
    }

    // Start chargelog part
    {
        StringBuilder chargelog_part_header(200);
        chargelog_part_header.printf("--%s\r\n", boundary.c_str());
        chargelog_part_header.printf("Content-Disposition: form-data; name=\"chargelog\"; filename=\"%s\"\r\n", filename.c_str());
        chargelog_part_header.printf("Content-Type: application/octet-stream\r\n\r\n");

        upload_args->remote_client->send_chunk(chargelog_part_header.getPtr(), chargelog_part_header.getLength());
    }

    if (upload_args->file_type == FileType::PDF) {
        const int letterhead_lines = read_letterhead_lines(upload_args->letterhead.get());

        this->generate_pdf(
            [&remote_client = *upload_args->remote_client](const void *buffer, size_t len) -> esp_err_t {
                return send_binary_chunk(remote_client, static_cast<const uint8_t *>(buffer), len);
            },
            upload_args->user_filter,
            upload_args->start_timestamp_min,
            upload_args->end_timestamp_min,
            rtc.timestamp_minutes(),
            upload_args->language, upload_args->letterhead.get(), letterhead_lines, nullptr);
    } else {
        CSVGenerationParams csv_params;
        csv_params.user_filter = upload_args->user_filter;
        csv_params.start_timestamp_min = upload_args->start_timestamp_min;
        csv_params.end_timestamp_min = upload_args->end_timestamp_min;
        csv_params.language = upload_args->language;
        csv_params.flavor = upload_args->csv_delimiter;
        task_scheduler.await([this, &csv_params]() {
            csv_params.electricity_price = this->config.get("electricity_price")->asUint();
        });

        auto csv_stream_cb = [&remote_client = *upload_args->remote_client](const char* buffer, size_t len) -> esp_err_t {
            return send_binary_chunk(remote_client, reinterpret_cast<const uint8_t *>(buffer), len);
        };

        CSVChargeLogGenerator csv_generator;
        csv_generator.generateCSV(csv_params, std::move(csv_stream_cb));
    }

    // Close multipart form-data
    {
        StringBuilder multipart_footer(50);
        multipart_footer.printf("\r\n--%s--\r\n", boundary.c_str());
        upload_args->remote_client->send_chunk(multipart_footer.getPtr(), multipart_footer.getLength());
    }

    upload_args->remote_client->finish_chunked_request();

    RemoteUploadRequest *upload_args_ptr = upload_args.release();

    while (check_remote_client_status(std::unique_ptr<RemoteUploadRequest>{upload_args_ptr}) == ESP_ERR_HTTP_EAGAIN) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

static constexpr uint32_t UPLOAD_TASK_STACK_SIZE = 6144;

static void upload_charge_logs_task(void *arg)
{
#if MODULE_WATCHDOG_AVAILABLE()
    int  wd_handle = watchdog.add("charge_log_upload", "Uploading charge log took longer than 5 minutes", 10_min, 30_min, false);
#endif

    std::unique_ptr<RemoteUploadRequest> upload_args{static_cast<RemoteUploadRequest *>(arg)};

    if (!upload_args->use_format_overrides) {
        const time_t t_now = time(nullptr);

        tm last_month_start;
        localtime_r(&t_now, &last_month_start);

        tm last_month_end;
        last_month_end = last_month_start;

        last_month_start.tm_mday = 1;
        if (last_month_start.tm_mon == 0) {
            last_month_start.tm_mon = 11;
            last_month_start.tm_year--;
        } else {
            last_month_start.tm_mon--;
        }
        last_month_start.tm_hour = 0;
        last_month_start.tm_min = 0;
        last_month_start.tm_sec = 0;
        last_month_start.tm_isdst = -1;

        last_month_end.tm_mday = 1;
        last_month_end.tm_hour = 0;
        last_month_end.tm_min = 0;
        last_month_end.tm_sec = 0;
        last_month_end.tm_isdst = -1;

        const Config *remote_access_config = api.getState("remote_access/config");
        for (int user_idx = 0; user_idx < upload_args->config_count; user_idx++) {
            uint32_t last_upload;
            String user_uuid;

            task_scheduler.await([&last_upload, &user_uuid, user_idx, remote_access_config]() {
                Config::Wrap upload_config = charge_tracker.config.get("remote_upload_configs")->get(user_idx);
                last_upload = upload_config->get("last_upload_timestamp_min")->asUint();
                size_t user_id = static_cast<size_t>(upload_config->get("user_id")->asInt());
                for (const Config &user_cfg : remote_access_config->get("users")) {
                    if (user_cfg.get("id")->asUint8() == static_cast<uint8_t>(user_id)) {
                        user_uuid = user_cfg.get("uuid")->asString();
                        break;
                    }
                }
            });

            if (!wants_send(t_now / 60, last_upload)) {
                continue;
            }

            auto upload_request = std::make_unique<RemoteUploadRequest>();
            upload_request->config_index = user_idx;
            upload_request->start_timestamp_min = static_cast<uint32_t>(mktime(&last_month_start)) / 60;
            upload_request->end_timestamp_min = static_cast<uint32_t>(mktime(&last_month_end)) / 60;
            upload_request->user_filter = upload_args->user_filter;
            upload_request->remote_access_user_uuid = user_uuid;
            upload_request->remote_client = std::make_unique<AsyncHTTPSClient>();

            charge_tracker.send_file(std::move(upload_request));
#if MODULE_WATCHDOG_AVAILABLE()
            watchdog.reset(wd_handle);
#endif
        }
    } else {
        upload_args->remote_client = std::make_unique<AsyncHTTPSClient>();

        charge_tracker.send_file(std::move(upload_args));
#if MODULE_WATCHDOG_AVAILABLE()
        watchdog.reset(wd_handle);
#endif
    }

#if MODULE_WATCHDOG_AVAILABLE()
    watchdog.remove(wd_handle);
#endif

    // Decunstructors are not called after vTaskDelete, so we must release the pointer here
    upload_args = nullptr;
    vTaskDelete(NULL); // exit RTOS task
}

void ChargeTracker::upload_charge_logs()
{
    uint32_t remote_upload_config_count = config.get("remote_upload_configs")->count();

    RemoteUploadRequest *args = new RemoteUploadRequest;
    args->config_count = remote_upload_config_count;
    args->config_index = -1; // -1 for all users
    args->user_filter = -3; // Use config default
    args->start_timestamp_min = 0; // Use default last month calculation
    args->end_timestamp_min = 0;
    args->generation_lock = ChargeLogGenerationLockHelper::try_lock(GenerationState::RemoteSend);
    if (args->generation_lock == nullptr) {
        logger.printfln("Could not acquire charge log generation lock for remote upload");
        delete args;
        return;
    }

    const BaseType_t ret = xTaskCreatePinnedToCore(upload_charge_logs_task, "ChargeLogUpload", UPLOAD_TASK_STACK_SIZE, args, 1, nullptr, 0);

    if (ret != pdPASS_safe) {
        logger.printfln("ChargeLogUpload task could not be created: %s (0x%lx)", esp_err_to_name(ret), static_cast<uint32_t>(ret));
        delete args;
    }
}

void ChargeTracker::start_charge_log_upload_for_user(uint32_t cookie, const int user_filter, const uint32_t start_timestamp_min, const uint32_t end_timestamp_min, const Language language, const FileType file_type, const CSVFlavor csv_delimiter, std::unique_ptr<char[]> letterhead, std::unique_ptr<ChargeLogGenerationLockHelper> generation_lock, const String &remote_access_user_uuid)
{
    RemoteUploadRequest *task_args = new RemoteUploadRequest;
    task_args->cookie = cookie;
    task_args->user_filter = user_filter;
    task_args->start_timestamp_min = start_timestamp_min;
    task_args->end_timestamp_min = end_timestamp_min;
    task_args->language = language;
    task_args->file_type = file_type;
    task_args->csv_delimiter = csv_delimiter;
    task_args->use_format_overrides = true;
    task_args->letterhead = std::move(letterhead);
    task_args->generation_lock = std::move(generation_lock);
    task_args->remote_access_user_uuid = remote_access_user_uuid;

    const BaseType_t ret = xTaskCreatePinnedToCore(upload_charge_logs_task, "ChargeLogUpload", UPLOAD_TASK_STACK_SIZE, task_args, 1, nullptr, 0);

    if (ret != pdPASS_safe) {
        logger.printfln("ChargeLogUpload task could not be created: %s (0x%lx)", esp_err_to_name(ret), static_cast<uint32_t>(ret));
    }
}
#endif

void ChargeTracker::generate_pdf(
    std::function<int(const void *buffer, size_t len)> &&callback,
    int user_filter,
    uint32_t start_timestamp_min,
    uint32_t end_timestamp_min,
    uint32_t current_timestamp_min,
    Language language,
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
            request->send_plain(500, "Failed to generate PDF: Task timed out");
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
            request->send_plain(500, "Failed to generate PDF: No memory");
        } else {
            logger.printfln("Failed to generate PDF: No memory");
        }
        return;
    }
    for (size_t i = 0; i < MAX_PASSIVE_USERS; i++) {
        display_name_cache[i].length = UINT32_MAX;
    }
    char *stats_head = stats_buf;
    stats_head += 1 + sprintf_u(stats_head, "%s: %s", (language == Language::English) ? "Charger" : "Wallbox", dev_name.c_str());
    stats_head += sprintf_u(stats_head, "%s: ", (language == Language::English) ? "Exported on" : "Exportiert am");
    stats_head += 1 + timestamp_min_to_date_time_string(stats_head, current_timestamp_min, language);
    stats_head += sprintf_u(stats_head, "%s: ", (language == Language::English) ? "Exported users" : "Exportierte Benutzer");
    if (user_filter == -2)
        stats_head += sprintf_u(stats_head, "%s", (language == Language::English) ? "all users" : "Alle Benutzer");
    else if (user_filter == -1)
        stats_head += sprintf_u(stats_head, "%s", (language == Language::English) ? "deleted users" : "Gelöschte Benutzer");
    else
        stats_head += get_display_name(user_filter, stats_head, display_name_cache);
    ++stats_head;
    stats_head += sprintf_u(stats_head, "%s: ", (language == Language::English) ? "Exported period" : "Exportierter Zeitraum");
    if (start_timestamp_min == 0)
        stats_head += sprintf_u(stats_head, "%s", (language == Language::English) ? "record start" : "Aufzeichnungsbeginn");
    else
        stats_head += timestamp_min_to_date_time_string(stats_head, start_timestamp_min, language);
    stats_head += sprintf_u(stats_head, "%s", (language == Language::English) ? " to " : " bis ");
    if (end_timestamp_min == 0)
        stats_head += sprintf_u(stats_head, "%s", (language == Language::English) ? "record end" : (start_timestamp_min == 0 ? "-ende" : "Aufzeichnungsende"));
    else
        stats_head += timestamp_min_to_date_time_string(stats_head, end_timestamp_min, language);
    ++stats_head;
    stats_head += sprintf_u(stats_head, "%s: ", (language == Language::English) ? "Total energy of exported charges" : "Gesamtenergie exportierter Ladevorgänge");
    if (charged_sum <= 999999999.999f) {
        int written = sprintf_u(stats_head, "%.3f kWh", charged_sum);
        if (language == Language::German)
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
                        (language == Language::English) ? "Total cost" : "Gesamtkosten",
                        charged_cost_sum / 100, charged_cost_sum % 100,
                        electricity_price / 100.0f,
                        seen_charges_without_meter ? ((language == Language::English) ? " Incomplete!" : " Unvollständig!") : "");
        if (language == Language::German)
            for (int i = 0; i < written; ++i)
                if (stats_head[i] == '.')
                    stats_head[i] = ',';
        stats_head += 1 + written;
    }
    std::lock_guard<std::mutex> lock2{pdf_mutex};
    int current_file = (first_file > -1 ? first_file : this->first_charge_record);
    int current_charge = (first_charge > -1 ? first_charge : 0);
    last_file = (last_file >= 0) ? last_file : this->last_charge_record;

#define TABLE_LINE_LEN (17 /* start date: 01.02.3456 12:34\0 or 3456-02-01 12:34\0 */ \
                      + 33 /* display name: max 32 chars + \0 */ \
                      + 8  /* charged: (assumed max) "999.999\0" kWh else truncated to "> 1000\0" */ \
                      + 11 /* charge duration max "9999:59:59\0" */ \
                      + 16 /* meter start max 99'999'999.999\0 */ \
                      + 8) /* cost max 9999.99\0 else truncated to >10000 */

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
                       (language == Language::English) ? "WARP Charge Log" : "WARP Ladelog",
                       stats_buf, (electricity_price == 0) ? 5 : 6,
                       letterhead, letterhead_lines,
                       (language == Language::English) ? table_header_en : table_header_de,
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
                        language,
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
                table_lines_head = tracked_charge_to_string(table_lines_head, cs, ce, language, electricity_price, display_name_cache);
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
}

std::unique_ptr<ChargeLogGenerationLockHelper> ChargeLogGenerationLockHelper::try_lock(GenerationState kind) {
    GenerationState expected = GenerationState::Ready;
    if (!generation_lock_state.compare_exchange_strong(expected, kind)) {
        return nullptr;
    }

    ensure_running_in_main_task([kind]() {
        charge_tracker.state.get("generator_state")->updateEnum<GenerationState>(kind);
    });

    return std::make_unique<ChargeLogGenerationLockHelper>();
}

ChargeLogGenerationLockHelper::~ChargeLogGenerationLockHelper() {
    ensure_running_in_main_task([]() {
        charge_tracker.state.get("generator_state")->updateEnum<GenerationState>(GenerationState::Ready);
    });
    generation_lock_state.store(GenerationState::Ready);
}
