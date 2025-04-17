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

#include "build.h"

#include "tools.h"
#include <ArduinoJson.h> // Include this even though its already included in TFJson.h but compiler got angry with me so just to be safe
#include <TFJson.h>
#include <optional>

template <typename T> void serializeOptional(JsonObject &obj, const char *key, const std::optional<T> &value);

template <typename T> std::optional<T> deserializeOptional(const JsonObject &obj, const char *key);

struct ElementTagType {
};

void convertFromJson(JsonVariantConst src, ElementTagType &tag);
bool convertToJson(const ElementTagType &tag, JsonObject &obj);

using LabelType = std::string;
using DescriptionType = std::string;
using SpecificationVersionType = std::string;

struct TimePeriodType {
    std::optional<std::string> startTime;
    std::optional<std::string> endTime;

    void toJson(JsonObject &obj) const;
    static TimePeriodType fromJson(const JsonObject &obj);
};

struct TimePeriodElementsType {
    std::optional<ElementTagType> startTime;
    std::optional<ElementTagType> endTime;

    void toJson(JsonObject &obj) const;
    static TimePeriodElementsType fromJson(const JsonObject &obj);
};

struct TimeStampIntervalType {
    std::optional<std::string> startTime;
    std::optional<std::string> endTime;

    void toJson(JsonObject &obj) const;
    static TimeStampIntervalType fromJson(const JsonObject &obj);
};

using AbsolutOreRelativeTimeType = std::string;

enum class RecurringIntervalEnumType { Yearly, Monthly, Weekly, Daily, Hourly, EveryMinute, EverySecond };
std::string toString(RecurringIntervalEnumType interval);
RecurringIntervalEnumType fromString(const std::string &str, RecurringIntervalEnumType);

using RecurringIntervalType = RecurringIntervalEnumType;

enum class MonthType { January, February, March, April, May, June, July, August, September, October, November, December };
bool convertToJson(const MonthType &month, const JsonObject &obj);
void convertFromJson(JsonVariantConst src, MonthType &month);

using DayOfMonthType = uint8_t;
using CalendarWeekType = uint8_t;

enum class DayOfWeekType { Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday };
std::string toString(DayOfWeekType day);
DayOfWeekType fromString(const std::string &str, DayOfWeekType);

using OccurenceType = std::string;

struct DaysOfWeekType {
    std::optional<ElementTagType> monday;
    std::optional<ElementTagType> tuesday;
    std::optional<ElementTagType> wednesday;
    std::optional<ElementTagType> thursday;
    std::optional<ElementTagType> friday;
    std::optional<ElementTagType> saturday;
    std::optional<ElementTagType> sunday;
};
bool convertToJson(const DaysOfWeekType &days, JsonVariant variant);
void convertFromJson(const JsonObject &obj, DaysOfWeekType &days);


using OccurenceType = std::string;

enum class OccurenceEnumType { First, Second, Third, Fourth, Last };
std::string toString(OccurenceEnumType occurence);
OccurenceEnumType fromString(const std::string &str, OccurenceEnumType);

struct AbsoluteOrReccuringTimeType {
    std::optional<std::string> dateTime;
    std::optional<MonthType> month;
    std::optional<DayOfMonthType> dayOfMonth;
    std::optional<CalendarWeekType> calendarWeek;
    std::optional<OccurenceType> dayOfWeekOccurence;
    std::optional<DaysOfWeekType> daysOfWeek;
    std::optional<std::string> time;
    std::optional<std::string> relative;
};
void convertFromJson(const JsonObject &obj, AbsoluteOrReccuringTimeType &time);
void convertToJson(const AbsoluteOrReccuringTimeType &time, JsonObject &obj);