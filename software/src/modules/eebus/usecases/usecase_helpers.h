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

#pragma once

#include "../spine_types.h"
#include "config.h"
#include <string>

/**
 * @brief Helper functions for EEBUS use case implementations.
 *
 * This namespace contains utility functions used across multiple EEBUS use cases,
 * including SPINE address handling, ISO 8601 time format conversion, and result
 * data generation for SPINE protocol responses.
 */
namespace EEBUS_USECASE_HELPERS
{
/**
 * @brief Get the SPINE device name for this device.
 * @return The device name in SPINE format: "d:_n:<manufacturer>-<model>-<serial>"
 */
std::string get_spine_device_name();

/**
 * @brief Error numbers as defined in EEBUS SPINE TS ResourceSpecification 3.11 Table 19.
 */
enum class ResultErrorNumber { NoError = 0, GeneralError, Timeout, Overload, DestinationUnknown, DestinationUnreachable, CommandNotSupported, CommandRejected, RestrictedFunctionExchangeCombinationNotSupported, BindingRequired };

/**
 * @brief Get the string representation of a result error number.
 * @param error_number The error number to convert.
 * @return The string representation of the error number.
 */
String get_result_error_number_string(int error_number);

/**
 * @brief Generate a result data object and write it to the response.
 *
 * Generally required when the header demands an acknowledgement or a result.
 *
 * @param response The JsonObject to write the result data to.
 * @param error_number The error number to set in the result data. Default is NoError.
 * @param description The description of the error. Default is an empty string.
 */
void build_result_data(const JsonObject &response, ResultErrorNumber error_number = ResultErrorNumber::NoError, const char *description = "");

/**
 * @brief Convert a duration in seconds to an ISO 8601 duration string.
 *
 * Will be automatically converted to full minutes or hours if possible.
 *
 * @param duration Duration in seconds.
 * @return The ISO 8601 duration string (e.g., "PT30S", "PT5M", "PT1H").
 */
std::string iso_duration_to_string(seconds_t duration);

/**
 * @brief Convert an ISO 8601 duration string to seconds.
 * @param iso_duration The ISO 8601 duration string.
 * @return The duration in seconds.
 */
seconds_t iso_duration_to_seconds(const std::string &iso_duration);

/**
 * @brief Convert an ISO 8601 timestamp to Unix time.
 * @param iso_timestamp The ISO 8601 timestamp string.
 * @param t Pointer to store the result.
 * @return True if conversion was successful.
 */
time_t iso_timestamp_to_unix(const char *iso_timestamp, time_t *t);

/**
 * @brief Convert Unix time to an ISO 8601 timestamp string.
 * @param unix_time The Unix timestamp.
 * @return The ISO 8601 timestamp string.
 */
String unix_to_iso_timestamp(time_t unix_time);

/**
 * @brief Convert a SPINE FeatureAddressType to a human-readable string.
 * @param address The SPINE feature address.
 * @return String representation in format "device/[entity]/feature".
 */
String spine_address_to_string(const FeatureAddressType &address);

/**
 * @brief Compare two SPINE feature addresses for equality.
 * @param addr1 First address to compare.
 * @param addr2 Second address to compare.
 * @return True if addresses are equal.
 */
bool compare_spine_addresses(const FeatureAddressType &addr1, const FeatureAddressType &addr2);

/**
 * @brief Convert a ScaledNumberType to a floating point number.
 *
 * Applies the scale factor: result = number * 10^scale
 *
 * @param number The ScaledNumberType to convert.
 * @return The floating point value.
 */
float scaled_numbertype_to_float(const ScaledNumberType &number);

/**
 * @brief Convert a ScaledNumberType to an integer.
 *
 * Applies the scale factor: result = number * 10^scale
 *
 * @param number The ScaledNumberType to convert.
 * @return The integer value.
 */
int scaled_numbertype_to_int(const ScaledNumberType &number);

} // namespace EEBUS_USECASE_HELPERS
