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

#include "commondatatypes.h"
#include "../module_dependencies.h"
#include "event_log_prefix.h"

template <typename T> void serializeOptional(JsonObject &obj, const char *key, const std::optional<T> &value)
{
    if (value.has_value()) {
        obj[key] = value.value();
    }
}

template <typename T> std::optional<T> deserializeOptional(const JsonObject &obj, const char *key)
{
    if (obj.containsKey(key)) {
        JsonObjectConst objConst = obj;
        return std::optional<T>{objConst[key].as<T>()};
    }
    return std::nullopt;
}

void convertFromJson(JsonVariantConst src, ElementTagType &tag)
{
    tag = ElementTagType();
}

bool convertToJson(const ElementTagType &tag, const JsonObject &obj)
{
    obj["elementTagType"] = "";
    return true;
}

void TimePeriodType::toJson(JsonObject &obj) const
{
    serializeOptional(obj, "startTime", startTime);
    serializeOptional(obj, "endTime", endTime);
}

TimePeriodType TimePeriodType::fromJson(const JsonObject &obj)
{
    TimePeriodType timePeriod;
    timePeriod.startTime = deserializeOptional<std::string>(obj, "startTime");
    timePeriod.endTime = deserializeOptional<std::string>(obj, "endTime");
    return timePeriod;
}

void TimePeriodElementsType::toJson(JsonObject &obj) const
{
    serializeOptional(obj, "startTime", startTime);
    serializeOptional(obj, "endTime", endTime);
}
TimePeriodElementsType TimePeriodElementsType::fromJson(const JsonObject &obj)
{
    TimePeriodElementsType timePeriod;
    timePeriod.startTime = deserializeOptional<ElementTagType>(obj, "startTime");
    timePeriod.endTime = deserializeOptional<ElementTagType>(obj, "endTime");
    return timePeriod;
}

void TimeStampIntervalType::toJson(JsonObject &obj) const
{
    serializeOptional(obj, "startTime", startTime);
    serializeOptional(obj, "endTime", endTime);
}
TimeStampIntervalType TimeStampIntervalType::fromJson(const JsonObject &obj)
{
    TimeStampIntervalType timeStampInterval;
    timeStampInterval.startTime = deserializeOptional<std::string>(obj, "startTime");
    timeStampInterval.endTime = deserializeOptional<std::string>(obj, "endTime");
    return timeStampInterval;
}

std::string toString(RecurringIntervalEnumType interval)
{
    switch (interval) {
        case RecurringIntervalEnumType::Yearly:
            return "yearly";
        case RecurringIntervalEnumType::Monthly:
            return "monthly";
        case RecurringIntervalEnumType::Weekly:
            return "weekly";
        case RecurringIntervalEnumType::Daily:
            return "daily";
        case RecurringIntervalEnumType::Hourly:
            return "hourly";
        case RecurringIntervalEnumType::EveryMinute:
            return "everyMinute";
        case RecurringIntervalEnumType::EverySecond:
            return "everySecond";
        default:
            return "unknown";
    }
}
RecurringIntervalEnumType fromString(const std::string &str, RecurringIntervalEnumType)
{
    if (str == "yearly") {
        return RecurringIntervalEnumType::Yearly;
    } else if (str == "monthly") {
        return RecurringIntervalEnumType::Monthly;
    } else if (str == "weekly") {
        return RecurringIntervalEnumType::Weekly;
    } else if (str == "daily") {
        return RecurringIntervalEnumType::Daily;
    } else if (str == "hourly") {
        return RecurringIntervalEnumType::Hourly;
    } else if (str == "everyMinute") {
        return RecurringIntervalEnumType::EveryMinute;
    } else if (str == "everySecond") {
        return RecurringIntervalEnumType::EverySecond;
    }
    logger.printfln("Unknown RecurringIntervalEnumType: %s", str.c_str());
    return RecurringIntervalEnumType::Yearly; // Default value
}
bool convertToJson(const MonthType &month, const JsonObject &obj)
{
    switch (month) {
        case MonthType::January:
            obj["month"] = "january";
            break;
        case MonthType::February:
            obj["month"] = "february";
            break;
        case MonthType::March:
            obj["month"] = "march";
            break;
        case MonthType::April:
            obj["month"] = "april";
            break;
        case MonthType::May:
            obj["month"] = "may";
            break;
        case MonthType::June:
            obj["month"] = "june";
            break;
        case MonthType::July:
            obj["month"] = "july";
            break;
        case MonthType::August:
            obj["month"] = "august";
            break;
        case MonthType::September:
            obj["month"] = "september";
            break;
        case MonthType::October:
            obj["month"] = "october";
            break;
        case MonthType::November:
            obj["month"] = "november";
            break;
        case MonthType::December:
            obj["month"] = "december";
            break;
    }
    return true;
}
void convertFromJson(JsonVariantConst src, MonthType &month)
{
    std::string monthStr = src["month"].as<std::string>();
    if (monthStr == "january") {
        month = MonthType::January;
    } else if (monthStr == "february") {
        month = MonthType::February;
    } else if (monthStr == "march") {
        month = MonthType::March;
    } else if (monthStr == "april") {
        month = MonthType::April;
    } else if (monthStr == "may") {
        month = MonthType::May;
    } else if (monthStr == "june") {
        month = MonthType::June;
    } else if (monthStr == "july") {
        month = MonthType::July;
    } else if (monthStr == "august") {
        month = MonthType::August;
    } else if (monthStr == "september") {
        month = MonthType::September;
    } else if (monthStr == "october") {
        month = MonthType::October;
    } else if (monthStr == "november") {
        month = MonthType::November;
    } else if (monthStr == "december") {
        month = MonthType::December;
    }
}

std::string toString(DayOfWeekType day)
{
    switch (day) {
        case DayOfWeekType::Monday:
            return "monday";
        case DayOfWeekType::Tuesday:
            return "tuesday";
        case DayOfWeekType::Wednesday:
            return "wednesday";
        case DayOfWeekType::Thursday:
            return "thursday";
        case DayOfWeekType::Friday:
            return "friday";
        case DayOfWeekType::Saturday:
            return "saturday";
        case DayOfWeekType::Sunday:
            return "sunday";
        default:
            return "Unknown day";
    }
}
DayOfWeekType fromString(const std::string &str, DayOfWeekType)
{
    if (str == "monday") {
        return DayOfWeekType::Monday;
    } else if (str == "tuesday") {
        return DayOfWeekType::Tuesday;
    } else if (str == "wednesday") {
        return DayOfWeekType::Wednesday;
    } else if (str == "thursday") {
        return DayOfWeekType::Thursday;
    } else if (str == "friday") {
        return DayOfWeekType::Friday;
    } else if (str == "saturday") {
        return DayOfWeekType::Saturday;
    } else if (str == "sunday") {
        return DayOfWeekType::Sunday;
    }
    logger.printfln("Unknown day: %s", str.c_str());
    return DayOfWeekType::Monday; // Default value
}

void convertFromJson(JsonVariantConst variant, DaysOfWeekType &days)
{
    JsonObject obj = variant.as<JsonObject>();
    days.monday = deserializeOptional<ElementTagType>(obj, "monday");
    days.tuesday = deserializeOptional<ElementTagType>(obj, "tuesday");
    days.wednesday = deserializeOptional<ElementTagType>(obj, "wednesday");
    days.thursday = deserializeOptional<ElementTagType>(obj, "thursday");
    days.friday = deserializeOptional<ElementTagType>(obj, "friday");
    days.saturday = deserializeOptional<ElementTagType>(obj, "saturday");
    days.sunday = deserializeOptional<ElementTagType>(obj, "sunday");
}
bool convertToJson(const DaysOfWeekType &days, JsonVariant variant)
{
    JsonObject obj = variant.as<JsonObject>();
    serializeOptional(obj, "monday", days.monday);
    serializeOptional(obj, "tuesday", days.tuesday);
    serializeOptional(obj, "wednesday", days.wednesday);
    serializeOptional(obj, "thursday", days.thursday);
    serializeOptional(obj, "friday", days.friday);
    serializeOptional(obj, "saturday", days.saturday);
    serializeOptional(obj, "sunday", days.sunday);
    return true;
}

std::string toString(OccurenceEnumType occurence)
{
    switch (occurence) {
        case OccurenceEnumType::First:
            return "first";
        case OccurenceEnumType::Second:
            return "second";
        case OccurenceEnumType::Third:
            return "third";
        case OccurenceEnumType::Fourth:
            return "fourth";
        case OccurenceEnumType::Last:
            return "last";
        default:
            return "Unknown occurence";
    }
}
OccurenceEnumType fromString(const std::string &str, OccurenceEnumType)
{
    if (str == "first") {
        return OccurenceEnumType::First;
    } else if (str == "second") {
        return OccurenceEnumType::Second;
    } else if (str == "third") {
        return OccurenceEnumType::Third;
    } else if (str == "fourth") {
        return OccurenceEnumType::Fourth;
    } else if (str == "last") {
        return OccurenceEnumType::Last;
    }
    logger.printfln("Unknown occurence: %s", str.c_str());
    return OccurenceEnumType::First; // Default value
}

void convertFromJson(const JsonObject &obj, AbsoluteOrReccuringTimeType &time)
{
    time.dateTime = deserializeOptional<std::string>(obj, "dateTime");
    time.month = deserializeOptional<MonthType>(obj, "month");
    time.dayOfMonth = deserializeOptional<DayOfMonthType>(obj, "dayOfMonth");
    time.calendarWeek = deserializeOptional<CalendarWeekType>(obj, "calendarWeek");
    time.dayOfWeekOccurence = deserializeOptional<OccurenceType>(obj, "dayOfWeekOccurence");
    time.daysOfWeek = deserializeOptional<DaysOfWeekType>(obj, "daysOfWeek");
    time.time = deserializeOptional<std::string>(obj, "time");
    time.relative = deserializeOptional<std::string>(obj, "relative");
}
void convertToJson(const AbsoluteOrReccuringTimeType &time, JsonObject &obj)
{
    serializeOptional(obj, "dateTime", time.dateTime);
    serializeOptional(obj, "month", time.month);
    serializeOptional(obj, "dayOfMonth", time.dayOfMonth);
    serializeOptional(obj, "calendarWeek", time.calendarWeek);
    serializeOptional(obj, "dayOfWeekOccurence", time.dayOfWeekOccurence);
    serializeOptional(obj, "daysOfWeek", time.daysOfWeek);
    serializeOptional(obj, "time", time.time);
    serializeOptional(obj, "relative", time.relative);
}