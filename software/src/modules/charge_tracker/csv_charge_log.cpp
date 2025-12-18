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
#include "pdfgen.h" // for pdf_utf8_to_pdfencoding
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
    return (language == Language::English) ? "Charging costs in € (Working price)" : "Ladekosten in € (Arbeitspreis)";
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

String CSVChargeLogGenerator::formatCSVLine(const String (&fields)[9], size_t field_count, CSVFlavor flavor) {
    char buf[256];
    StringWriter line(buf, std::size(buf));

    const char separator = (flavor == CSVFlavor::Excel) ? ';' : ',';

    for (size_t i = 0; i < field_count; i++) {
        escapeCSVField(fields[i], line);
        if (i < field_count - 1) {
            line.putc(separator);
        }
    }

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



String CSVChargeLogGenerator::formatEnergy(float energy_kwh, Language language) {
    if (std::isnan(energy_kwh) || energy_kwh < 0) {
        return "N/A";
    }

    StringBuilder energy = StringBuilder(16);
    energy.printf("%.3f", energy_kwh);

    String energy_str = energy.toString();
    if (language == Language::German) {
        energy_str.replace('.', ',');
    }

    return energy_str;
}

String CSVChargeLogGenerator::formatPrice(float price_euros, Language language) {
    if (std::isnan(price_euros) || price_euros < 0) {
        return "N/A";
    }

    StringBuilder price = StringBuilder(16);
    price.printf("%.2f", price_euros);

    String price_str = price.toString();
    if (language == Language::German) {
        price_str.replace('.', ',');
    }

    return price_str;
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
    size_t field_count = 7;

    headers[0] = CSVTranslations::getHeaderStart(params.language);
    headers[1] = CSVTranslations::getHeaderDisplayName(params.language);
    headers[2] = CSVTranslations::getHeaderEnergy(params.language);
    headers[3] = CSVTranslations::getHeaderDuration(params.language);
    // headers[4] is already empty
    headers[5] = CSVTranslations::getHeaderMeterStart(params.language);
    headers[6] = CSVTranslations::getHeaderMeterEnd(params.language);
    headers[7] = CSVTranslations::getHeaderUsername(params.language);

    if (params.electricity_price > 0) {
        char price_header[64];
        float price_per_kwh = params.electricity_price / 10000.0f;
        snprintf(price_header, sizeof(price_header), "%s %.2f ct/kWh",
        CSVTranslations::getHeaderPrice(params.language), price_per_kwh * 100);

        String price_header_str = String(price_header);
        if (params.language == Language::German) {
            price_header_str.replace('.', ',');
        }

        headers[8] = price_header_str;
        field_count = 9;
    }

    return formatCSVLine(headers, field_count, params.flavor);
}

String CSVChargeLogGenerator::convertToWindows1252(const String& utf8_string) {
    String result;
    result.reserve(utf8_string.length());

    auto utf8_len = utf8_string.length();

    for (size_t i = 0; i < utf8_len; ) {
        uint8_t csv_char;
        auto code_len = pdf_utf8_to_pdfencoding(utf8_string.c_str() + i, utf8_len - i, &csv_char);

        if (code_len < 0) {
            result += '?';
        } else {
            result += (char)csv_char;
        }

        i += code_len;
    }
    return result;
}

bool CSVChargeLogGenerator::readChargeRecords(uint32_t first_record, uint32_t last_record,
                                              std::function<esp_err_t(const uint8_t* record_data, size_t record_size, bool last)> record_callback) {

    uint8_t buffer[sizeof(Charge)];
    for (uint32_t file_idx = first_record; file_idx <= last_record; file_idx++) {
        String filename = chargeRecordFilename(file_idx, nullptr);

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

int CSVChargeLogGenerator::generateCSV(const CSVGenerationParams& params,
                                        std::function<esp_err_t(const char* data, size_t length)> callback) {
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
        return -1;
    }

    // TODO use unique_ptr_any here
    display_name_entry *display_name_cache = static_cast<decltype(display_name_cache)>(malloc_iram_or_psram_or_dram(MAX_PASSIVE_USERS * sizeof(display_name_cache[0])));
    defer { free(display_name_cache); };

    if (display_name_cache == nullptr) {
        logger.printfln("Failed to generate CSV: No memory");
        return -1;
    }
    for (size_t i = 0; i < MAX_PASSIVE_USERS; i++) {
        display_name_cache[i].length = UINT32_MAX;
    }

    String accumulated_data;
    accumulated_data.reserve(MAX_ACCUMULATED);

    // Helper lambda to process a single charge record and add it to accumulated_data
    auto process_charge = [&](const Charge* record, bool last) -> esp_err_t {
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
        size_t field_count = 7;
        char display_name[33];
        get_display_name(record->cs.user_id, display_name, display_name_cache, params.language);
        fields[0] = formatTimestamp(record->cs.timestamp_minutes, params.language);
        fields[1] = display_name;
        fields[2] = formatEnergy(energy_charged, params.language);
        fields[3] = formatDuration(record->ce.charge_duration);
        fields[5] = formatEnergy(record->cs.meter_start, params.language);
        fields[6] = formatEnergy(record->ce.meter_end, params.language);
        fields[7] = display_name;

        if (params.electricity_price > 0) {
            fields[8] = formatPrice(price_euros, params.language);
            field_count = 9;
        }

        String csv_line = formatCSVLine(fields, field_count, params.flavor);

        if (params.flavor == CSVFlavor::Excel) {
            csv_line = convertToWindows1252(csv_line);
        }

        accumulated_data += csv_line;

        if (accumulated_data.length() < MAX_ACCUMULATED && !last) {
            return ESP_OK;
        }

        int callback_result = callback(accumulated_data.c_str(), accumulated_data.length());
        if (callback_result != ESP_OK) {
            logger.printfln("Failed to send CSV data chunk.");
        } else {
            accumulated_data.clear();
        }

        vTaskDelay(1); // Yield to allow other tasks to run

        return callback_result;
    };

#if OPTIONS_PRODUCT_ID_IS_WARP()
    std::lock_guard<std::mutex> lock(charge_tracker.records_mutex);
    readChargeRecords(charge_tracker.first_charge_record, charge_tracker.last_charge_record,
        [&](const uint8_t* record_data, size_t record_size, bool last) -> esp_err_t {
            const Charge* record = reinterpret_cast<const Charge*>(record_data);

            auto timestamp = record->cs.timestamp_minutes;
            auto filter_start = params.start_timestamp_min;
            auto filter_end = params.end_timestamp_min;

            if (timestamp != 0 && filter_start != 0 && timestamp < filter_start) {
                return ESP_OK;
            }

            if (timestamp != 0 && filter_end != 0 && timestamp > filter_end) {
                return ESP_OK;
            }

            if (isUserFiltered(record->cs.user_id, params.user_filter)) {
                return ESP_OK;
            }

            return process_charge(record, last);
        });
#else
    size_t filtered_count = 0;
    ExportCharge *filtered_charges = charge_tracker.getFilteredCharges(params.user_filter, params.start_timestamp_min, params.end_timestamp_min, &filtered_count);

    if (filtered_charges != nullptr) {
        for (size_t i = 0; i < filtered_count; ++i) {
            bool last = (i == filtered_count - 1);
            esp_err_t result = process_charge(&filtered_charges[i].charge, last);
            if (result != ESP_OK) {
                break;
            }
        }
        delete[] filtered_charges;
    }
#endif

    // This might not be necessary if accumulated_data is cleared on error as well.
    if (accumulated_data.length() > 0) {
        int callback_result = callback(accumulated_data.c_str(), accumulated_data.length());
        if (callback_result < 0) {
            logger.printfln("Failed to send final CSV data chunk.");
            return callback_result;
        }
    }

    return 0;
}

String CSVChargeLogGenerator::generateCSVString(const CSVGenerationParams& params) {
    String result;

    generateCSV(params, [&result](const char* data, size_t length) -> int {
        result.concat(data, length);
        return static_cast<int>(length);
    });

    return result;
}
