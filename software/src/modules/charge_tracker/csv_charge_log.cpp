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
#include <LittleFS.h>
#include <cmath>
#include <vector>
#include <mutex>
#include <cstring>
#include <Arduino.h>


String chargeRecordFilename(uint32_t i)
{
    char buf[64];
    StringWriter sw(buf, sizeof(buf));
    sw.printf(CHARGE_RECORD_FOLDER "/charge-record-%lu.bin", i);
    return String(buf, sw.getLength());
}

const char* CSVTranslations::getHeaderStart(bool english) {
    return english ? "Start" : "Start";
}

const char* CSVTranslations::getHeaderDisplayName(bool english) {
    return english ? "Display Name" : "Anzeigename";
}

const char* CSVTranslations::getHeaderEnergy(bool english) {
    return english ? "Energy (kWh)" : "Energie (kWh)";
}

const char* CSVTranslations::getHeaderDuration(bool english) {
    return english ? "Duration (s)" : "Dauer (s)";
}

const char* CSVTranslations::getHeaderMeterStart(bool english) {
    return english ? "Meter Start (kWh)" : "Zählerstand Start (kWh)";
}

const char* CSVTranslations::getHeaderMeterEnd(bool english) {
    return english ? "Meter End (kWh)" : "Zählerstand Ende (kWh)";
}

const char* CSVTranslations::getHeaderUsername(bool english) {
    return english ? "Username" : "Benutzername";
}

const char* CSVTranslations::getHeaderPrice(bool english) {
    return english ? "Price (€)" : "Preis (€)";
}

const char* CSVTranslations::getUnknownUser(bool english) {
    return english ? "Unknown User" : "Unbekannter Benutzer";
}

const char* CSVTranslations::getDeletedUser(bool english) {
    return english ? "Deleted User" : "Gelöschter Benutzer";
}

const char* CSVTranslations::getUnknownChargeStart(bool english) {
    return english ? "Unknown" : "Unbekannt";
}

CSVChargeLogGenerator::CSVChargeLogGenerator() {
    buffer = static_cast<char*>(malloc(BUFFER_SIZE));
}

CSVChargeLogGenerator::~CSVChargeLogGenerator() {
    if (buffer) {
        free(buffer);
    }
}

String CSVChargeLogGenerator::escapeCSVField(const String& field) {
    String escaped = "\"";

    for (size_t i = 0; i < field.length(); i++) {
        char c = field.charAt(i);
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }

    escaped += "\"";
    return escaped;
}

String CSVChargeLogGenerator::formatCSVLine(const std::vector<String>& fields, CSVFlavor flavor) {
    String line;
    const char* separator = (flavor == CSVFlavor::Excel) ? ";" : ",";
    const char* line_ending = (flavor == CSVFlavor::Excel) ? "\r\n" : "\n";

    for (size_t i = 0; i < fields.size(); i++) {
        if (i > 0) {
            line += separator;
        }
        line += escapeCSVField(fields[i]);
    }

    line += line_ending;
    return line;
}

String CSVChargeLogGenerator::formatTimestamp(uint32_t timestamp_min, bool english) {
    if (timestamp_min == 0) {
        return CSVTranslations::getUnknownChargeStart(english);
    }

    time_t timestamp = ((int64_t)timestamp_min) * 60;
    struct tm t;
    localtime_r(&timestamp, &t);

    char buf[128];
    if (english) {
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

String CSVChargeLogGenerator::getUserDisplayName(uint8_t user_id) {
    char display_name_buf[33] = {0};

    size_t name_len = users.get_display_name(user_id, display_name_buf);

    if (name_len == 0 || (user_id == 0 && strcmp(display_name_buf, "Anonymous") == 0)) {
        return CSVTranslations::getUnknownUser(false);
    }

    if (!charge_tracker.is_user_tracked(user_id)) {
        return CSVTranslations::getDeletedUser(false);
    }

    return String(display_name_buf, name_len);
}

String CSVChargeLogGenerator::getUserName(uint8_t user_id) {
    if (user_id == 0) {
        return CSVTranslations::getUnknownUser(false);
    }

    if (!charge_tracker.is_user_tracked(user_id)) {
        return CSVTranslations::getDeletedUser(false);
    }

    char username_buf[16];
    snprintf(username_buf, sizeof(username_buf), "user_%d", user_id);
    return String(username_buf);
}

String CSVChargeLogGenerator::generateCSVHeader(const CSVGenerationParams& params) {
    std::vector<String> headers;

    headers.push_back(CSVTranslations::getHeaderStart(params.english));
    headers.push_back(CSVTranslations::getHeaderDisplayName(params.english));
    headers.push_back(CSVTranslations::getHeaderEnergy(params.english));
    headers.push_back(CSVTranslations::getHeaderDuration(params.english));
    headers.push_back("");
    headers.push_back(CSVTranslations::getHeaderMeterStart(params.english));
    headers.push_back(CSVTranslations::getHeaderMeterEnd(params.english));
    headers.push_back(CSVTranslations::getHeaderUsername(params.english));

    if (params.electricity_price > 0) {
        char price_header[64];
        float price_per_kwh = params.electricity_price / 10000.0f;
        snprintf(price_header, sizeof(price_header), "%s %.2f ct/kWh",
                CSVTranslations::getHeaderPrice(params.english), price_per_kwh * 100);
        headers.push_back(String(price_header));
    } else {
        headers.push_back("");
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
    return utf8_string;
}

bool CSVChargeLogGenerator::readChargeRecords(uint32_t first_record, uint32_t last_record,
                                              std::function<bool(const uint8_t* record_data, size_t record_size)> record_callback) {
    if (!buffer) {
        return false;
    }

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
            size_t bytes_read = file.read(reinterpret_cast<uint8_t*>(buffer), record_size);

            if (bytes_read != record_size) {
                break;
            }

            if (!record_callback(reinterpret_cast<const uint8_t*>(buffer), record_size)) {
                file.close();
                return false;
            }
        }

        file.close();
    }

    return true;
}

bool CSVChargeLogGenerator::generateCSV(const CSVGenerationParams& params,
                                        std::function<bool(const char* data, size_t length)> callback) {
    if (!buffer) {
        return false;
    }

    std::lock_guard<std::mutex> lock(charge_tracker.records_mutex);

    String header_line;

    if (params.flavor == CSVFlavor::Excel) {
        header_line = "sep=;\r\n";
    }

    header_line += generateCSVHeader(params);

    String final_header = (params.flavor == CSVFlavor::Excel) ?
                         convertToWindows1252(header_line) : header_line;

    if (!callback(final_header.c_str(), final_header.length())) {
        return false;
    }

    String accumulated_data;
    const size_t MAX_ACCUMULATED = 2048;

    bool success = readChargeRecords(charge_tracker.first_charge_record, charge_tracker.last_charge_record,
        [&](const uint8_t* record_data, size_t record_size) -> bool {
            if (record_size != sizeof(Charge)) {
                return true;
            }

            const Charge* record = reinterpret_cast<const Charge*>(record_data);

            if (record->cs.timestamp_minutes != 0) {
                if (record->cs.timestamp_minutes < params.start_timestamp_min) {
                    accumulated_data = final_header;
                    return true;
                }

                if (record->cs.timestamp_minutes > params.end_timestamp_min) {
                    return false;
                }
            }

            if (isUserFiltered(record->cs.user_id, params.user_filter)) {
                return true;
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

            std::vector<String> fields;
            fields.push_back(formatTimestamp(record->cs.timestamp_minutes, params.english));
            fields.push_back(getUserDisplayName(record->cs.user_id));
            fields.push_back(formatEnergy(energy_charged));
            fields.push_back(formatDuration(record->ce.charge_duration));
            fields.push_back("");
            fields.push_back(formatEnergy(record->cs.meter_start));
            fields.push_back(formatEnergy(record->ce.meter_end));
            fields.push_back(getUserName(record->cs.user_id));

            if (params.electricity_price > 0) {
                fields.push_back(formatPrice(price_euros));
            } else {
                fields.push_back("");
            }

            String csv_line = formatCSVLine(fields, params.flavor);

            if (params.flavor == CSVFlavor::Excel) {
                csv_line = convertToWindows1252(csv_line);
            }

            accumulated_data += csv_line;

            if (accumulated_data.length() >= MAX_ACCUMULATED) {
                if (!callback(accumulated_data.c_str(), accumulated_data.length())) {
                    return false;
                }
                accumulated_data.clear();
            }

            return true;
        });

    if (success && accumulated_data.length() > 0) {
        success = callback(accumulated_data.c_str(), accumulated_data.length());
    }

    return success;
}

String CSVChargeLogGenerator::generateCSVString(const CSVGenerationParams& params) {
    String result;

    bool success = generateCSV(params, [&result](const char* data, size_t length) -> bool {
        result += String(data, length);
        return true;
    });

    return success ? result : String();
}
