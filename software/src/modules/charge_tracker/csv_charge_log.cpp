/* esp32-firmware
 * Copyright (C) 2025 Frederic Henrichs <frederic@tinkerforge.com>
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

#include "csv_charge_log.h"
#include "charge_tracker.h"
#include "module_dependencies.h"
#include "tools/string_builder.h"
#include "tools/malloc.h"
#include "event_log_prefix.h"
#include <LittleFS.h>
#include <cmath>
#include <vector>
#include <mutex>
#include <cstring>
#include <Arduino.h>

#define EVENT_LOG_PREFIX "csv_charge_log"
#define MAX_ACCUMULATED 2048

const char* CSVTranslations::getHeaderStart(Language language) {
    return (language == Language::English) ? "Start time" : "Startzeit";
}

const char* CSVTranslations::getHeaderDisplayName(Language language) {
    return (language == Language::English) ? "Display name" : "Anzeigename";
}

const char* CSVTranslations::getHeaderEnergy(Language language) {
    return (language == Language::English) ? "Charged energy in kWh" : "Geladene Energie in kWh";
}

const char* CSVTranslations::getHeaderDuration(Language language) {
    return (language == Language::English) ? "Charge duration in s" : "Ladedauer in s";
}

const char* CSVTranslations::getHeaderMeterStart(Language language) {
    return (language == Language::English) ? "Meter reading start" : "Zählerstand Start";
}

const char* CSVTranslations::getHeaderMeterEnd(Language language) {
    return (language == Language::English) ? "Meter reading end" : "Zählerstand Ende";
}

const char* CSVTranslations::getHeaderUsername(Language language) {
    return (language == Language::English) ? "Username" : "Benutzername";
}

const char* CSVTranslations::getHeaderPrice(Language language) {
    return (language == Language::English) ? "Charging costs in € Working price " : "Ladekosten in €; Arbeitspreis ";
}

const char* CSVTranslations::getUnknownUser(Language language) {
    return (language == Language::English) ? "Unknown User" : "Unbekannter Benutzer";
}

const char* CSVTranslations::getDeletedUser(Language language) {
    return (language == Language::English) ? "Deleted User" : "Gelöschter Benutzer";
}

const char* CSVTranslations::getUnknownChargeStart(Language language) {
    return (language == Language::English) ? "Unknown" : "Unbekannt";
}

void CSVChargeLogGenerator::escapeCSVField(const String& field, StringWriter &output) {
    output.putc('"');

    const size_t length = field.length();

    for (size_t i = 0; i < length; i++) {
        const char c = field[i];

        if (c == '"') {
            output.puts("\"\"");
        } else {
            output.putc(c);
        }
    }

    output.putc('"');
}

String CSVChargeLogGenerator::formatCSVLine(const String (&fields)[9], CSVFlavor flavor) {
    char buf[256];
    StringWriter line(buf, std::size(buf));

    const char separator = (flavor == CSVFlavor::Excel) ? ';' : ',';

    for (size_t i = 0; i < 9; i++) {
        escapeCSVField(fields[i], line);
        line.putc(separator);
    }

    line.setLength(line.getLength() - 1); // Remove trailing separator.
    line.puts(flavor == CSVFlavor::Excel ? "\r\n" : "\n");

    return String(line.getPtr(), line.getLength());
}

String CSVChargeLogGenerator::formatTimestamp(uint32_t timestamp_min, Language language) {
    if (timestamp_min == 0) {
        return CSVTranslations::getUnknownChargeStart(language);
    }

    time_t timestamp = ((int64_t)timestamp_min) * 60;
    struct tm t;
    localtime_r(&timestamp, &t);

    char buf[128];
    if (language == Language::English) {
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
                t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min);
    } else {
        snprintf(buf, sizeof(buf), "%02d.%02d.%04d %02d:%02d",
                t.tm_mday, t.tm_mon + 1, t.tm_year + 1900, t.tm_hour, t.tm_min);
    }

    return String(buf);
}

String CSVChargeLogGenerator::formatDuration(uint32_t duration_seconds) {
    return String(duration_seconds);
}

String CSVChargeLogGenerator::formatEnergy(float energy_kwh) {
    if (std::isnan(energy_kwh) || energy_kwh < 0) {
        return "N/A";
    }

    char buf[16];
    snprintf(buf, sizeof(buf), "%.3f", energy_kwh);
    return String(buf);
}

String CSVChargeLogGenerator::formatPrice(float price_euros) {
    if (std::isnan(price_euros) || price_euros < 0) {
        return "N/A";
    }

    char buf[16];
    snprintf(buf, sizeof(buf), "%.2f", price_euros);
    return String(buf);
}

bool CSVChargeLogGenerator::isUserFiltered(uint8_t user_id, int user_filter) {
    switch (user_filter) {
        case USER_FILTER_ALL_USERS:
            return false;

        case USER_FILTER_DELETED_USERS:
            return charge_tracker.is_user_tracked(user_id);

        default:
            return user_id != user_filter;
    }
}

String CSVChargeLogGenerator::getUserDisplayName(uint8_t user_id, Language language) {
    char display_name_buf[33] = {0};

    size_t name_len = users.get_display_name(user_id, display_name_buf);

    if (name_len == 0 || (user_id == 0 && strcmp(display_name_buf, "Anonymous") == 0)) {
        return CSVTranslations::getUnknownUser(language);
    }

    if (!charge_tracker.is_user_tracked(user_id)) {
        return CSVTranslations::getDeletedUser(language);
    }

    return String(display_name_buf, name_len);
}

String CSVChargeLogGenerator::getUserName(uint8_t user_id, Language language) {
    if (user_id == 0) {
        return CSVTranslations::getUnknownUser(language);
    }

    if (!charge_tracker.is_user_tracked(user_id)) {
        return CSVTranslations::getDeletedUser(language);
    }

    char username_buf[16];
    snprintf(username_buf, sizeof(username_buf), "user_%d", user_id);
    return String(username_buf);
}

String CSVChargeLogGenerator::generateCSVHeader(const CSVGenerationParams& params) {
    String headers[9];

    headers[0] = CSVTranslations::getHeaderStart(params.language);
    headers[1] = CSVTranslations::getHeaderDisplayName(params.language);
    headers[2] = CSVTranslations::getHeaderEnergy(params.language);
    headers[3] = CSVTranslations::getHeaderDuration(params.language);
    //headers[4] already blank
    headers[5] = CSVTranslations::getHeaderMeterStart(params.language);
    headers[6] = CSVTranslations::getHeaderMeterEnd(params.language);
    headers[7] = CSVTranslations::getHeaderUsername(params.language);

    if (params.electricity_price > 0) {
        char price_header[64];
        float price_per_kwh = params.electricity_price / 10000.0f;
        snprintf(price_header, sizeof(price_header), "%s %.2f ct/kWh",
                CSVTranslations::getHeaderPrice(params.language), price_per_kwh * 100);
        headers[8] = String(price_header);
    } else {
        //headers[8] already blank
    }

    return formatCSVLine(headers, params.flavor);
}

String CSVChargeLogGenerator::convertToWindows1252(const String& utf8_string) {
    String result;
    result.reserve(utf8_string.length());

    for (size_t i = 0; i < utf8_string.length(); i++) {
        uint8_t byte = utf8_string.charAt(i);

        // ASCII characters (0-127) are the same in both encodings
        if (byte < 0x80) {
            result += (char)byte;
        } else if (byte == 0xC2 && i + 1 < utf8_string.length()) {
            // UTF-8 sequence starting with 0xC2
            uint8_t next_byte = utf8_string.charAt(i + 1);
            if (next_byte >= 0x80 && next_byte <= 0x9F) {
                // Characters 0x80-0x9F in Windows-1252
                result += (char)next_byte;
                i++; // Skip next byte as it's part of this sequence
            } else {
                result += '?'; // Unknown character
                i++; // Skip next byte
            }
        } else if (byte == 0xC3 && i + 1 < utf8_string.length()) {
            // UTF-8 sequence starting with 0xC3
            uint8_t next_byte = utf8_string.charAt(i + 1);
            if (next_byte >= 0x80 && next_byte <= 0xBF) {
                // Map to Windows-1252 characters 0xC0-0xFF
                result += (char)(next_byte + 0x40);
                i++; // Skip next byte as it's part of this sequence
            } else {
                result += '?'; // Unknown character
                i++; // Skip next byte
            }
        } else {
            // Other multi-byte UTF-8 sequences or invalid bytes
            result += '?'; // Replace with placeholder
        }
    }
    return result;
}

bool CSVChargeLogGenerator::readChargeRecords(uint32_t first_record, uint32_t last_record,
                                              std::function<esp_err_t(const uint8_t* record_data, size_t record_size, bool last)> record_callback) {

    uint8_t buffer[sizeof(Charge)];
    for (uint32_t file_idx = first_record; file_idx <= last_record; file_idx++) {
        String filename = chargeRecordFilename(file_idx);

        if (!LittleFS.exists(filename)) {
            continue;
        }

        File file = LittleFS.open(filename, "r");
        if (!file) {
            continue;
        }

        size_t file_size = file.size();
        const size_t record_size = sizeof(Charge);

        size_t complete_records = file_size / record_size;

        for (size_t i = 0; i < complete_records; i++) {
            size_t bytes_read = file.read(buffer, record_size);

            if (bytes_read != record_size) {
                logger.printfln("Failed to read complete charge record from file %s", filename.c_str());
                break;
            }

            bool last = i == (complete_records - 1);
            if (record_callback(buffer, record_size, last) != ESP_OK) {
                logger.printfln("Record callback requested to stop reading further records.");
                file.close();
                return false;
            }
        }

        file.close();

        vTaskDelay(1); // After every file, give other tasks a chance to run and reset their watchdogs.
    }

    return true;
}

void CSVChargeLogGenerator::generateCSV(const CSVGenerationParams& params,
                                        std::function<esp_err_t(const char* data, size_t length)> callback) {
    std::lock_guard<std::mutex> lock(charge_tracker.records_mutex);

    String header_line;

    if (params.flavor == CSVFlavor::Excel) {
        header_line = "sep=;\r\n";
    }

    header_line += generateCSVHeader(params);

    String final_header = (params.flavor == CSVFlavor::Excel) ?
                         convertToWindows1252(header_line) : header_line;

    int header_result = callback(final_header.c_str(), final_header.length());
    if (header_result != ESP_OK) {
        logger.printfln("Failed to send CSV header. Cancelling CSV generation.");
        return;
    }

    display_name_entry *display_name_cache = static_cast<decltype(display_name_cache)>(malloc_iram_or_psram_or_dram(MAX_PASSIVE_USERS * sizeof(display_name_cache[0])));
    if (!display_name_cache) {
        logger.printfln("Failed to generate CSV: No memory");
        return;
    }
    for (size_t i = 0; i < MAX_PASSIVE_USERS; i++) {
        display_name_cache[i].length = UINT32_MAX;
    }

    String accumulated_data;
    accumulated_data.reserve(MAX_ACCUMULATED);

    readChargeRecords(charge_tracker.first_charge_record, charge_tracker.last_charge_record,
        [&](const uint8_t* record_data, size_t record_size, bool last) -> esp_err_t {
            const Charge* record = reinterpret_cast<const Charge*>(record_data);

            if (record->cs.timestamp_minutes < params.start_timestamp_min ||
                record->cs.timestamp_minutes > params.end_timestamp_min) {
                return ESP_OK;
            }

            if (isUserFiltered(record->cs.user_id, params.user_filter)) {
                return ESP_OK;
            }

            float energy_charged = NAN;
            if (!std::isnan(record->cs.meter_start) && !std::isnan(record->ce.meter_end) &&
                record->ce.meter_end >= record->cs.meter_start) {
                energy_charged = record->ce.meter_end - record->cs.meter_start;
            }

            float price_euros = 0.0f;
            if (params.electricity_price > 0 && !std::isnan(energy_charged) && energy_charged >= 0) {
                float price_per_kwh = params.electricity_price / 10000.0f;
                price_euros = energy_charged * price_per_kwh;
            }

            String fields[9];
            char display_name[33];
            get_display_name(record->cs.user_id, display_name, display_name_cache);
            fields[0] = formatTimestamp(record->cs.timestamp_minutes, params.language);
            fields[1] = display_name;
            fields[2] = formatEnergy(energy_charged);
            fields[3] = formatDuration(record->ce.charge_duration);
            //fields[4] already blank
            fields[5] = formatEnergy(record->cs.meter_start);
            fields[6] = formatEnergy(record->ce.meter_end);
            fields[7] = display_name;

            if (params.electricity_price > 0) {
                fields[8] = formatPrice(price_euros);
            } else {
                //fields[8] already blank
            }

            String csv_line = formatCSVLine(fields, params.flavor);

            if (params.flavor == CSVFlavor::Excel) {
                csv_line = convertToWindows1252(csv_line);
            }

            accumulated_data += csv_line;

            if (accumulated_data.length() < MAX_ACCUMULATED && !last) {
                return ESP_OK; // Continue accumulating
            }

            int callback_result = callback(accumulated_data.c_str(), accumulated_data.length());
            if (callback_result != ESP_OK) {
                logger.printfln("Failed to send CSV data chunk.");
            } else {
                accumulated_data.clear();
            }

            return callback_result;
        });

    // This might not be necessary if accumulated_data is cleared on error as well.
    if (accumulated_data.length() > 0) {
        int callback_result = callback(accumulated_data.c_str(), accumulated_data.length());
        if (callback_result < 0) {
            logger.printfln("Failed to send final CSV data chunk.");
        }
    }

    free(display_name_cache);
}

String CSVChargeLogGenerator::generateCSVString(const CSVGenerationParams& params) {
    String result;

    generateCSV(params, [&result](const char* data, size_t length) -> int {
        result.concat(data, length);
        return static_cast<int>(length);
    });

    return result;
}
