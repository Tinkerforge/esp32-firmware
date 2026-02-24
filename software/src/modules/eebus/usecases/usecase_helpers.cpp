/* esp32-firmware
 * Copyright (C) 2025 Julius Dill <julius@tinkerforge.com>
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

#include "usecase_helpers.h"
#include "../eebus.h"
#include <cmath>

extern EEBus eebus;

namespace EEBUS_USECASE_HELPERS
{

std::string get_spine_device_name()
{
    // This returns the device name as defined in EEBUS SPINE TS ProtocolSpecification
    std::string name = ("d:_n:" + eebus.get_eebus_name()).c_str();
    return name;
}

String get_result_error_number_string(const int error_number)
{
    ResultErrorNumber error = static_cast<ResultErrorNumber>(error_number);
    switch (error) {
        case ResultErrorNumber::NoError:
            return "NoError";
        case ResultErrorNumber::GeneralError:
            return "GeneralError";
        case ResultErrorNumber::Timeout:
            return "Timeout";
        case ResultErrorNumber::Overload:
            return "Overload";
        case ResultErrorNumber::DestinationUnknown:
            return "DestinationUnknown";
        case ResultErrorNumber::DestinationUnreachable:
            return "DestinationUnreachable";
        case ResultErrorNumber::CommandNotSupported:
            return "CommandNotSupported";
        case ResultErrorNumber::CommandRejected:
            return "CommandRejected";
        case ResultErrorNumber::RestrictedFunctionExchangeCombinationNotSupported:
            return "RestrictedFunctionExchangeCombinationNotSupported";
        case ResultErrorNumber::BindingRequired:
            return "BindingRequired";
    }
    return "UnknownError";
}

void build_result_data(const JsonObject &response, ResultErrorNumber error_number, const char *description)
{
    if (error_number != ResultErrorNumber::NoError) {
        eebus.trace_fmtln("Usecases: Building result data with error number %s and description: %s", get_result_error_number_string(static_cast<int>(error_number)).c_str(), description);
    }
    ResultDataType result{};
    result.description = description;
    result.errorNumber = static_cast<uint8_t>(error_number);
    response["resultData"] = result;
}

std::string iso_duration_to_string(seconds_t duration)
{
    uint64_t duration_uint = duration.as<uint64_t>();
    std::string unit = "S";
    if (duration_uint % 60 == 0) {
        duration_uint = duration_uint / 60;
        unit = "M";
    }
    if (duration_uint % 60 == 0) {
        duration_uint = duration_uint / 60;
        unit = "H";
    }
    return "PT" + std::to_string(duration_uint) + unit;
}

seconds_t iso_duration_to_seconds(const std::string &iso_duration)
{
    int64_t duration_seconds = 0;
    bool in_time_part = false;
    int accumulated = 0;

    for (char c : iso_duration) {
        if (c == 'P') {
            continue;
        }
        if (c == 'T') {
            in_time_part = true;
            accumulated = 0;
            continue;
        }
        if (c >= '0' && c <= '9') {
            accumulated = accumulated * 10 + (c - '0');
            continue;
        }
        // Date components (before T)
        if (!in_time_part) {
            if (c == 'Y') {
                duration_seconds += accumulated * 31536000; // Approximate, not accounting for leap years
            } else if (c == 'M') {
                duration_seconds += accumulated * 2592000; // Approximate, assuming 30 days in a month
            } else if (c == 'D') {
                duration_seconds += accumulated * 86400;
            }
        } else {
            // Time components (after T)
            if (c == 'H') {
                duration_seconds += accumulated * 3600;
            } else if (c == 'M') {
                duration_seconds += accumulated * 60;
            } else if (c == 'S') {
                duration_seconds += accumulated;
            }
        }
        accumulated = 0;
    }
    return seconds_t(duration_seconds);
}

time_t iso_timestamp_to_unix(const char *iso_timestamp, time_t *t)
{
    struct tm tm_time{};
    // Try parsing ISO 8601 format: YYYY-MM-DDTHH:MM:SSZ or YYYY-MM-DDTHH:MM:SS+HH:MM
    const char *remaining = strptime(iso_timestamp, "%Y-%m-%dT%H:%M:%S", &tm_time);
    if (remaining == nullptr) {
        return false;
    }
    // Use mktime to convert. Note: mktime interprets tm_time as local time.
    // For proper UTC handling, we need to adjust for timezone.
    tm_time.tm_isdst = 0;
    *t = mktime(&tm_time);
    // Adjust for local timezone offset to get UTC
    struct tm gm_tm{};
    gmtime_r(t, &gm_tm);
    time_t gm_t = mktime(&gm_tm);
    *t += (*t - gm_t); // Adjust for timezone difference
    return true;
}

String unix_to_iso_timestamp(time_t unix_time)
{
    tm t{};
    gmtime_r(&unix_time, &t);
    constexpr int ISO_8601_MAX_LEN = 36;
    char buf[ISO_8601_MAX_LEN];
    strftime(buf, ISO_8601_MAX_LEN, "%FT%TZ", &t);
    return buf;
}

String spine_address_to_string(const FeatureAddressType &address)
{
    std::string out;
    if (address.device.has_value()) {
        out += address.device.get();
    }
    if (address.entity.has_value()) {
        if (!out.empty()) {
            out += "/";
        }
        out += "[";
        const auto &entities = address.entity.get();
        for (size_t i = 0; i < entities.size(); ++i) {
            out += std::to_string(entities[i]);
            if (i + 1 < entities.size()) {
                out += ",";
            }
        }
        out += "]";
    }
    if (address.feature.has_value()) {
        if (!out.empty()) {
            out += "/";
        }
        out += std::to_string(address.feature.get());
    }
    return {out.c_str()};
}

bool compare_spine_addresses(const FeatureAddressType &addr1, const FeatureAddressType &addr2)
{
    if (addr1.device.get() != addr2.device) {
        return false;
    }
    if (addr1.entity.get() != addr2.entity) {
        return false;
    }
    if (addr1.feature.get() != addr2.feature) {
        return false;
    }
    return true;
}

float scaled_numbertype_to_float(const ScaledNumberType &number)
{
    if (number.scale.has_value()) {
        return static_cast<float>(number.number.get()) * std::pow(10.0f, static_cast<float>(number.scale.get()));
    }
    return static_cast<float>(number.number.get());
}

int scaled_numbertype_to_int(const ScaledNumberType &number)
{
    if (number.scale.has_value()) {
        return static_cast<int>(static_cast<float>(number.number.get()) * std::pow(10.0f, static_cast<float>(number.scale.get())));
    }
    return static_cast<int>(number.number.get());
}

} // namespace EEBUS_USECASE_HELPERS
