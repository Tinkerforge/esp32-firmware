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

#include "modules.h"

#include <esp_random.h>

struct ChargeStart {
    uint32_t timestamp_minutes = 0;
    float meter_start = 0.0f;
    uint8_t user_id = 0;
} __attribute__((packed));

static_assert(sizeof(ChargeStart) == 9, "Unexpected size of ChargeStart");

struct ChargeEnd {
    uint32_t charge_duration : 24;
    float meter_end = 0.0f;
} __attribute__((packed));

static_assert(sizeof(ChargeEnd) == 7, "Unexpected size of ChargeEnd");

#define CHARGE_RECORD_SIZE (sizeof(ChargeStart) + sizeof(ChargeEnd))

static_assert(CHARGE_RECORD_SIZE == 16, "Unexpected size of ChargeStart + ChargeEnd");

#define CHARGE_RECORD_FOLDER "/charge-records"
// 30 files with 256 records each: 7680 records @ ~ max. 10 records per day = ~ 2 years and one month of records.
#define CHARGE_RECORD_FILE_COUNT 30
#define CHARGE_RECORD_MAX_FILE_SIZE 4096

#define CHARGE_RECORD_LAST_CHARGES_SIZE 30

ChargeTracker::ChargeTracker()
{
    last_charges = Config::Array({},
        new Config{Config::Object({
            {"timestamp_minutes", Config::Uint32(0)},
            {"charge_duration", Config::Uint32(0)},
            {"user_id", Config::Uint8(0)},
            {"energy_charged", Config::Float(0)}
        })}, 0, CHARGE_RECORD_LAST_CHARGES_SIZE, Config::type_id<Config::ConfObject>());

    current_charge = Config::Object({
        {"user_id", Config::Int16(-1)},
        {"meter_start", Config::Float(0)},
        {"evse_uptime_start", Config::Uint32(0)},
        {"timestamp_minutes", Config::Uint32(0)}
    });

    state = Config::Object({
        {"tracked_charges", Config::Uint16(0)},
        {"first_charge_timestamp", Config::Uint32(0)}
    });
}

String ChargeTracker::chargeRecordFilename(uint32_t i) {
    return String(CHARGE_RECORD_FOLDER) + "/charge-record-" + i + ".bin";
}

void ChargeTracker::startCharge(uint32_t timestamp_minutes, float meter_start, uint8_t user_id, uint32_t evse_uptime) {
    std::lock_guard<std::mutex> lock{records_mutex};
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

    if ((file.size() % CHARGE_RECORD_SIZE) != 0) {
        logger.printfln("Can't track start of charge: Last charge end was not tracked or file is damaged! Offset is %u bytes. Expected 0", file.size() % CHARGE_RECORD_SIZE);
        // TODO: for robustness we would have to write the last end here? Yes, but only if % == 9. Also write duration 0, so we know this is a "faked" end. Still write the correct meter state.
        return;
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
}

void ChargeTracker::endCharge(uint32_t charge_duration_seconds, float meter_end) {
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

    updateState();
}

void ChargeTracker::removeOldRecords()
{
    while (this->last_charge_record - this->first_charge_record > 30) {
        String name = chargeRecordFilename(this->first_charge_record);
        logger.printfln("Got %u charge records. Dropping the first one (%s)", this->last_charge_record - this->first_charge_record, name.c_str());
        LittleFS.remove(name);
        ++this->first_charge_record;
    }
}

bool ChargeTracker::setupRecords() {
    if (!LittleFS.mkdir(CHARGE_RECORD_FOLDER)) { //mkdir also returns true if the directory already exists and is a directory.
        logger.printfln("Failed to create charge record folder!");
        return false;
    }

    File folder = LittleFS.open(CHARGE_RECORD_FOLDER);
    File f;

    uint32_t found_blobs[32] = {0};
    size_t found_blobs_size = sizeof(found_blobs)/sizeof(found_blobs[0]);
    int found_blob_counter = 0;

    while (f = folder.openNextFile()) {
        String name = String(f.name());
        if (f.isDirectory()) {
            logger.printfln("Unexpected directory %s in charge record folder", name.c_str());
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

    logger.printfln("Found %u records. First is %u, last is %u", found_blob_counter, first, last);
    for(int i = 0; i < found_blob_counter - 1; ++i) {
        if (found_blobs[i] + 1 != found_blobs[i+1]) {
            logger.printfln("Non-consecutive charge records found! (Next after %u is %u. Expected was %u", found_blobs[i], found_blobs[i+1], found_blobs[i] + 1);
            return false;
        }

        f = LittleFS.open(chargeRecordFilename(found_blobs[i]));
        if (f.size() != CHARGE_RECORD_MAX_FILE_SIZE) {
            logger.printfln("Charge record %s is too long: %u bytes", f.name(), f.size());
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

    removeOldRecords();
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

void ChargeTracker::readNRecords(File *f, size_t records_to_read) {
    uint8_t buf[CHARGE_RECORD_SIZE];
    ChargeStart cs;
    ChargeEnd ce;

    for(int i = 0; i < records_to_read; ++i) {
        memset(buf, 0, sizeof(buf));
        f->read(buf, CHARGE_RECORD_SIZE);

        memcpy(&cs, buf, sizeof(cs));
        memcpy(&ce, buf + sizeof(cs), sizeof(ce));

        last_charges.add();
        last_charges.get(last_charges.count() - 1)->get("timestamp_minutes")->updateUint(cs.timestamp_minutes);
        last_charges.get(last_charges.count() - 1)->get("charge_duration")->updateUint(ce.charge_duration);
        last_charges.get(last_charges.count() - 1)->get("user_id")->updateUint(cs.user_id);
        last_charges.get(last_charges.count() - 1)->get("energy_charged")->updateFloat((cs.meter_start == NAN || ce.meter_end == NAN) ? NAN : ce.meter_end - cs.meter_start);
    }
}

void ChargeTracker::updateState() {
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

void ChargeTracker::register_urls()
{
    server.on("/charge_tracker/charge_log", HTTP_GET, [this](WebServerRequest request) {
        std::lock_guard<std::mutex> lock{records_mutex};

        char *url_buf = (char*)malloc(CHARGE_RECORD_MAX_FILE_SIZE);
        if (url_buf == nullptr) {
            request.send(507);
            return;
        }

        File file = LittleFS.open(chargeRecordFilename(this->last_charge_record));
        size_t file_size = (this->last_charge_record - this->first_charge_record) * CHARGE_RECORD_MAX_FILE_SIZE + file.size();
        String file_size_string = String(file_size);
        request.addResponseHeader("Content-Length", file_size_string.c_str());

        request.beginChunkedResponse(200, "application/octet-stream");
        for(int i = this->first_charge_record; i <= this->last_charge_record; ++i) {
            File f = LittleFS.open(chargeRecordFilename(i));
            int read = f.read((uint8_t *)url_buf, CHARGE_RECORD_MAX_FILE_SIZE);
            int trunc = read - (read % CHARGE_RECORD_SIZE);
            request.sendChunk(url_buf, trunc);
        }
        request.endChunkedResponse();
        free(url_buf);
    });

    api.addState("charge_tracker/last_charges", &last_charges, {}, 1000);
    api.addState("charge_tracker/current_charge", &current_charge, {}, 1000);
    api.addState("charge_tracker/state", &state, {}, 1000);
}

void ChargeTracker::loop()
{
}
