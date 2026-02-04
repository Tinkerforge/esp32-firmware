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

#pragma once

#include <functional>
#include <Arduino.h>

#include "config.h"
#include "language.h"
#include "chunked_response.h"
#include "csv_flavor.enum.h"
#include "charge_tracker_defs.h"
#include "tools/string_builder.h"

struct CSVGenerationParams {
    CSVFlavor flavor;
    int user_filter;
    int device_filter;
    uint32_t start_timestamp_min;
    uint32_t end_timestamp_min;
    uint32_t electricity_price;
    Language language;

    CSVGenerationParams() :
        flavor(CSVFlavor::Excel),
        user_filter(USER_FILTER_ALL_USERS),
        device_filter(DEVICE_FILTER_ALL_CHARGERS),
        start_timestamp_min(0),
        end_timestamp_min(UINT32_MAX),
        electricity_price(0),
        language(Language::German) {}
};

class CSVChargeLogGenerator {
public:
    /**
     * @brief Generate CSV data and stream it through the callback function
     * @param params Generation parameters (format, filters, etc.)
     * @param callback Function called with generated CSV data chunks
     */
    int generateCSV(const CSVGenerationParams& params,
                     std::function<int(const char* data, size_t length)> callback);

    /**
     * @brief Simplified CSV generation that returns complete CSV as String
     * @param params Generation parameters
     * @return Complete CSV data as String, or empty string on error
     * @note Use with caution for large datasets due to memory usage
     */
    String generateCSVString(const CSVGenerationParams& params);

private:
    void escapeCSVField(const String& field, StringWriter &output);
    String formatCSVLine(const String (&fields)[9], size_t field_count, CSVFlavor flavor);
    String formatTimestamp(uint32_t timestamp_min, Language language);
    String formatDuration(uint32_t duration_seconds);
    String formatEnergy(float energy_kwh, Language language);
    String formatPrice(float price_euros, Language language);

    bool isUserFiltered(uint8_t user_id, int user_filter);
    String getUserDisplayName(uint8_t user_id, Language language);

    bool readChargeRecords(uint32_t first_record, uint32_t last_record,
                          std::function<esp_err_t(const uint8_t* record_data, size_t record_size, bool last)> record_callback);

    String generateCSVHeader(const CSVGenerationParams& params);

    String convertToWindows1252(const String& utf8_string);
};

/**
 * @brief Translation keys for CSV headers
 *
 * Provides localized text for CSV column headers and common values.
 * This should ideally be integrated with the main translation system.
 */
struct CSVTranslations {
    static const char* getHeaderStart(Language language);
    static const char* getHeaderDisplayName(Language language);
    static const char* getHeaderEnergy(Language language);
    static const char* getHeaderDuration(Language language);
    static const char* getHeaderMeterStart(Language language);
    static const char* getHeaderMeterEnd(Language language);
    static const char* getHeaderUsername(Language language);
    static const char* getHeaderPrice(Language language);
    static const char* getUnknownUser(Language language);
    static const char* getDeletedUser(Language language);
    static const char* getUnknownChargeStart(Language language);
};
