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

#define CHARGE_RECORD_FOLDER "/charge-records"

#include "config.h"
#include "chunked_response.h"
#include <functional>
#include <vector>
#include <Arduino.h>
#include "csv_flavor.enum.h"

String chargeRecordFilename(uint32_t i);

#define USER_FILTER_ALL_USERS -2
#define USER_FILTER_DELETED_USERS -1

struct CSVGenerationParams {
    CSVFlavor flavor;
    int user_filter;
    uint32_t start_timestamp_min;
    uint32_t end_timestamp_min;
    uint32_t electricity_price;
    bool english;

    CSVGenerationParams() :
        flavor(CSVFlavor::Excel),
        user_filter(USER_FILTER_ALL_USERS),
        start_timestamp_min(0),
        end_timestamp_min(UINT32_MAX),
        electricity_price(0),
        english(false) {}
};

class CSVChargeLogGenerator {
public:
    CSVChargeLogGenerator();
    ~CSVChargeLogGenerator();

    /**
     * @brief Generate CSV data and stream it through the callback function
     * @param params Generation parameters (format, filters, etc.)
     * @param callback Function called with generated CSV data chunks
     * @return true if generation was successful, false on error
     */
    bool generateCSV(const CSVGenerationParams& params,
                     std::function<bool(const char* data, size_t length)> callback);

    /**
     * @brief Simplified CSV generation that returns complete CSV as String
     * @param params Generation parameters
     * @return Complete CSV data as String, or empty string on error
     * @note Use with caution for large datasets due to memory usage
     */
    String generateCSVString(const CSVGenerationParams& params);

private:
    String escapeCSVField(const String& field);
    String formatCSVLine(const std::vector<String>& fields, CSVFlavor flavor);
    String formatTimestamp(uint32_t timestamp_min, bool english);
    String formatDuration(uint32_t duration_seconds);
    String formatEnergy(float energy_kwh);
    String formatPrice(float price_euros);

    bool isUserFiltered(uint8_t user_id, int user_filter);
    String getUserDisplayName(uint8_t user_id);
    String getUserName(uint8_t user_id);

    bool readChargeRecords(uint32_t first_record, uint32_t last_record,
                          std::function<bool(const uint8_t* record_data, size_t record_size)> record_callback);

    String generateCSVHeader(const CSVGenerationParams& params);

    String convertToWindows1252(const String& utf8_string);

    static const size_t BUFFER_SIZE = 4096;
    char* buffer;
};


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

static_assert(sizeof(ChargeEnd) == 7, "Unexpected size of ChargeEnd");

struct [[gnu::packed]] Charge {
    ChargeStart cs;
    ChargeEnd ce;
};

/**
 * @brief Translation keys for CSV headers
 *
 * Provides localized text for CSV column headers and common values.
 * This should ideally be integrated with the main translation system.
 */
struct CSVTranslations {
    static const char* getHeaderStart(bool english);
    static const char* getHeaderDisplayName(bool english);
    static const char* getHeaderEnergy(bool english);
    static const char* getHeaderDuration(bool english);
    static const char* getHeaderMeterStart(bool english);
    static const char* getHeaderMeterEnd(bool english);
    static const char* getHeaderUsername(bool english);
    static const char* getHeaderPrice(bool english);
    static const char* getUnknownUser(bool english);
    static const char* getDeletedUser(bool english);
    static const char* getUnknownChargeStart(bool english);
};
