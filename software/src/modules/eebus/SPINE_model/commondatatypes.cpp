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

template <typename T> std::optional<T> deserializeOptional(const JsonObjectConst &obj, const char *key)
{
    if (obj.containsKey(key)) {
        JsonObjectConst objConst = obj;
        return std::optional<T>{objConst[key].as<T>()};
    }
    return std::nullopt;
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
    if (variant.is<JsonObject>()) {

        JsonObjectConst obj = variant.as<JsonObjectConst>();
        days.monday = deserializeOptional<ElementTagType>(obj, "monday");
        days.tuesday = deserializeOptional<ElementTagType>(obj, "tuesday");
        days.wednesday = deserializeOptional<ElementTagType>(obj, "wednesday");
        days.thursday = deserializeOptional<ElementTagType>(obj, "thursday");
        days.friday = deserializeOptional<ElementTagType>(obj, "friday");
        days.saturday = deserializeOptional<ElementTagType>(obj, "saturday");
        days.sunday = deserializeOptional<ElementTagType>(obj, "sunday");
    }
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

std::string toString(UnitOfMeasurementEnumType unit)
{
    switch (unit) {
        case UnitOfMeasurementEnumType::Unknown:
            return "unknown";
        case UnitOfMeasurementEnumType::One:
            return "1";
        case UnitOfMeasurementEnumType::M:
            return "m";
        case UnitOfMeasurementEnumType::Kg:
            return "kg";
        case UnitOfMeasurementEnumType::S:
            return "s";
        case UnitOfMeasurementEnumType::A:
            return "A";
        case UnitOfMeasurementEnumType::K:
            return "K";
        case UnitOfMeasurementEnumType::Mol:
            return "mol";
        case UnitOfMeasurementEnumType::Cd:
            return "cd";
        case UnitOfMeasurementEnumType::V:
            return "V";
        case UnitOfMeasurementEnumType::W:
            return "W";
        case UnitOfMeasurementEnumType::Wh:
            return "Wh";
        case UnitOfMeasurementEnumType::Va:
            return "VA";
        case UnitOfMeasurementEnumType::Vah:
            return "VAh";
        case UnitOfMeasurementEnumType::Var:
            return "var";
        case UnitOfMeasurementEnumType::Varh:
            return "varh";
        case UnitOfMeasurementEnumType::Degc:
            return "degC";
        case UnitOfMeasurementEnumType::Degf:
            return "degF";
        case UnitOfMeasurementEnumType::Lm:
            return "Lm";
        case UnitOfMeasurementEnumType::Ohm:
            return "Ohm";
        case UnitOfMeasurementEnumType::Hz:
            return "Hz";
        case UnitOfMeasurementEnumType::Db:
            return "dB";
        case UnitOfMeasurementEnumType::Dbm:
            return "dBm";
        case UnitOfMeasurementEnumType::Pct:
            return "pct";
        case UnitOfMeasurementEnumType::Ppm:
            return "ppm";
        case UnitOfMeasurementEnumType::L:
            return "l";
        case UnitOfMeasurementEnumType::L_s:
            return "l/s";
        case UnitOfMeasurementEnumType::L_h:
            return "l/h";
        case UnitOfMeasurementEnumType::Deg:
            return "deg";
        case UnitOfMeasurementEnumType::Rad:
            return "rad";
        case UnitOfMeasurementEnumType::Rad_s:
            return "rad/s";
        case UnitOfMeasurementEnumType::Sr:
            return "sr";
        case UnitOfMeasurementEnumType::Gy:
            return "Gy";
        case UnitOfMeasurementEnumType::Bq:
            return "Bq";
        case UnitOfMeasurementEnumType::Bq_m3:
            return "Bq/m^3";
        case UnitOfMeasurementEnumType::Sv:
            return "Sv";
        case UnitOfMeasurementEnumType::Rd:
            return "Rd";
        case UnitOfMeasurementEnumType::C:
            return "C";
        case UnitOfMeasurementEnumType::F:
            return "F";
        case UnitOfMeasurementEnumType::H:
            return "H";
        case UnitOfMeasurementEnumType::J:
            return "J";
        case UnitOfMeasurementEnumType::N:
            return "N";
        case UnitOfMeasurementEnumType::N_m:
            return "N_m";
        case UnitOfMeasurementEnumType::N_s:
            return "N_s";
        case UnitOfMeasurementEnumType::Wb:
            return "Wb";
        case UnitOfMeasurementEnumType::T:
            return "T";
        case UnitOfMeasurementEnumType::Pa:
            return "Pa";
        case UnitOfMeasurementEnumType::Bar:
            return "bar";
        case UnitOfMeasurementEnumType::Atm:
            return "atm";
        case UnitOfMeasurementEnumType::Psi:
            return "psi";
        case UnitOfMeasurementEnumType::Mmhg:
            return "mmHg";
        case UnitOfMeasurementEnumType::M2:
            return "m^2";
        case UnitOfMeasurementEnumType::M3:
            return "m^3";
        case UnitOfMeasurementEnumType::M3_h:
            return "m^3/h";
        case UnitOfMeasurementEnumType::M_s:
            return "m/s";
        case UnitOfMeasurementEnumType::M_s2:
            return "m/s^2";
        case UnitOfMeasurementEnumType::M3_s:
            return "m^3/s";
        case UnitOfMeasurementEnumType::M_m3:
            return "m/m^3";
        case UnitOfMeasurementEnumType::Kg_m3:
            return "kg/m^3";
        case UnitOfMeasurementEnumType::Kg_m:
            return "kg_m";
        case UnitOfMeasurementEnumType::M2_s:
            return "m^2/s";
        case UnitOfMeasurementEnumType::W_m_k:
            return "W/m_K";
        case UnitOfMeasurementEnumType::J_k:
            return "J/K";
        case UnitOfMeasurementEnumType::One_s:
            return "1/s";
        case UnitOfMeasurementEnumType::W_m2:
            return "W/m^2";
        case UnitOfMeasurementEnumType::J_m2:
            return "J/m^2";
        case UnitOfMeasurementEnumType::S_cap:
            return "S";
        case UnitOfMeasurementEnumType::S_m:
            return "S/m";
        case UnitOfMeasurementEnumType::K_s:
            return "K/s";
        case UnitOfMeasurementEnumType::Pa_s:
            return "Pa/s";
        case UnitOfMeasurementEnumType::J_kg_k:
            return "J/kg_K";
        case UnitOfMeasurementEnumType::Vs:
            return "Vs";
        case UnitOfMeasurementEnumType::V_m:
            return "V/m";
        case UnitOfMeasurementEnumType::V_hz:
            return "V/Hz";
        case UnitOfMeasurementEnumType::As:
            return "As";
        case UnitOfMeasurementEnumType::A_m:
            return "A/m";
        case UnitOfMeasurementEnumType::Hz_s:
            return "Hz/s";
        case UnitOfMeasurementEnumType::Kg_s:
            return "kg/s";
        case UnitOfMeasurementEnumType::Kg_m2:
            return "kg/m^2";
        case UnitOfMeasurementEnumType::J_wh:
            return "J/Wh";
        case UnitOfMeasurementEnumType::W_s:
            return "W/s";
        case UnitOfMeasurementEnumType::Ft3:
            return "ft^3";
        case UnitOfMeasurementEnumType::Ft3_h:
            return "ft^3/h";
        case UnitOfMeasurementEnumType::Ccf:
            return "ccf";
        case UnitOfMeasurementEnumType::Ccf_h:
            return "ccf/h";
        case UnitOfMeasurementEnumType::UsLiqGal:
            return "US.liq.gal";
        case UnitOfMeasurementEnumType::UsLiqGal_h:
            return "US.liq.gal/h";
        case UnitOfMeasurementEnumType::ImpGal:
            return "Imp.gal";
        case UnitOfMeasurementEnumType::ImpGal_h:
            return "Imp.gal/h";
        case UnitOfMeasurementEnumType::Btu:
            return "Btu";
        case UnitOfMeasurementEnumType::Btu_h:
            return "Btu/h";
        case UnitOfMeasurementEnumType::Ah:
            return "Ah";
        case UnitOfMeasurementEnumType::Kg_wh:
            return "kg/Wh";
        default:
            return "unknown";
    }
}

UnitOfMeasurementEnumType fromString(const std::string &str, UnitOfMeasurementEnumType)
{
    // Yes this is not very speed efficient but it saves on code size. Should not be called that often anyway
    for (int i = 0; i != static_cast<int>(UnitOfMeasurementEnumType::Kg_wh); i++) {
        if (str == toString(static_cast<UnitOfMeasurementEnumType>(i))) {
            return static_cast<UnitOfMeasurementEnumType>(i);
        }
    }
    return UnitOfMeasurementEnumType::Unknown;
}

bool convertToJson(const ScaledNumberType &scaledNumber, JsonObject &obj)
{
    if (scaledNumber.number.has_value()) {
        obj["number"] = scaledNumber.number.value();
    }
    if (scaledNumber.scale.has_value()) {
        obj["scale"] = scaledNumber.scale.value();
    }
    return true;
}

void convertFromJson(const JsonObject &obj, ScaledNumberType &scaledNumber)
{
    scaledNumber.number = deserializeOptional<NumberType>(obj, "number");
    scaledNumber.scale = deserializeOptional<ScaleType>(obj, "scale");
}

bool convertToJson(const ScaledNumberElementsType &scaledNumber, JsonObject &obj)
{
    if (scaledNumber.number.has_value()) {
        obj["number"] = scaledNumber.number.value();
    }
    if (scaledNumber.scale.has_value()) {
        obj["scale"] = scaledNumber.scale.value();
    }
    return true;
}
void convertFromJson(const JsonObject &obj, ScaledNumberElementsType &scaledNumber)
{
    scaledNumber.number = deserializeOptional<ElementTagType>(obj, "number");
    scaledNumber.scale = deserializeOptional<ElementTagType>(obj, "scale");
}

std::string toString(CurrencyEnumType currency)
{
    switch (currency) {
        case CurrencyEnumType::AED:
            return "AED";
        case CurrencyEnumType::AFN:
            return "AFN";
        case CurrencyEnumType::ALL:
            return "ALL";
        case CurrencyEnumType::AMD:
            return "AMD";
        case CurrencyEnumType::ANG:
            return "ANG";
        case CurrencyEnumType::AOA:
            return "AOA";
        case CurrencyEnumType::ARS:
            return "ARS";
        case CurrencyEnumType::AUD:
            return "AUD";
        case CurrencyEnumType::AWG:
            return "AWG";
        case CurrencyEnumType::AZN:
            return "AZN";
        case CurrencyEnumType::BAM:
            return "BAM";
        case CurrencyEnumType::BBD:
            return "BBD";
        case CurrencyEnumType::BDT:
            return "BDT";
        case CurrencyEnumType::BGN:
            return "BGN";
        case CurrencyEnumType::BHD:
            return "BHD";
        case CurrencyEnumType::BIF:
            return "BIF";
        case CurrencyEnumType::BMD:
            return "BMD";
        case CurrencyEnumType::BND:
            return "BND";
        case CurrencyEnumType::BOB:
            return "BOB";
        case CurrencyEnumType::BOV:
            return "BOV";
        case CurrencyEnumType::BRL:
            return "BRL";
        case CurrencyEnumType::BSD:
            return "BSD";
        case CurrencyEnumType::BTN:
            return "BTN";
        case CurrencyEnumType::BWP:
            return "BWP";
        case CurrencyEnumType::BYR:
            return "BYR";
        case CurrencyEnumType::BZD:
            return "BZD";
        case CurrencyEnumType::CAD:
            return "CAD";
        case CurrencyEnumType::CDF:
            return "CDF";
        case CurrencyEnumType::CHE:
            return "CHE";
        case CurrencyEnumType::CHF:
            return "CHF";
        case CurrencyEnumType::CHW:
            return "CHW";
        case CurrencyEnumType::CLF:
            return "CLF";
        case CurrencyEnumType::CLP:
            return "CLP";
        case CurrencyEnumType::CNY:
            return "CNY";
        case CurrencyEnumType::COP:
            return "COP";
        case CurrencyEnumType::COU:
            return "COU";
        case CurrencyEnumType::CRC:
            return "CRC";
        case CurrencyEnumType::CUC:
            return "CUC";
        case CurrencyEnumType::CUP:
            return "CUP";
        case CurrencyEnumType::CVE:
            return "CVE";
        case CurrencyEnumType::CZK:
            return "CZK";
        case CurrencyEnumType::DJF:
            return "DJF";
        case CurrencyEnumType::DKK:
            return "DKK";
        case CurrencyEnumType::DOP:
            return "DOP";
        case CurrencyEnumType::DZD:
            return "DZD";
        case CurrencyEnumType::EGP:
            return "EGP";
        case CurrencyEnumType::ERN:
            return "ERN";
        case CurrencyEnumType::ETB:
            return "ETB";
        case CurrencyEnumType::EUR:
            return "EUR";
        case CurrencyEnumType::FJD:
            return "FJD";
        case CurrencyEnumType::FKP:
            return "FKP";
        case CurrencyEnumType::GBP:
            return "GBP";
        case CurrencyEnumType::GEL:
            return "GEL";
        case CurrencyEnumType::GHS:
            return "GHS";
        case CurrencyEnumType::GIP:
            return "GIP";
        case CurrencyEnumType::GMD:
            return "GMD";
        case CurrencyEnumType::GNF:
            return "GNF";
        case CurrencyEnumType::GTQ:
            return "GTQ";
        case CurrencyEnumType::GYD:
            return "GYD";
        case CurrencyEnumType::HKD:
            return "HKD";
        case CurrencyEnumType::HNL:
            return "HNL";
        case CurrencyEnumType::HRK:
            return "HRK";
        case CurrencyEnumType::HTG:
            return "HTG";
        case CurrencyEnumType::HUF:
            return "HUF";
        case CurrencyEnumType::IDR:
            return "IDR";
        case CurrencyEnumType::ILS:
            return "ILS";
        case CurrencyEnumType::INR:
            return "INR";
        case CurrencyEnumType::IQD:
            return "IQD";
        case CurrencyEnumType::IRR:
            return "IRR";
        case CurrencyEnumType::ISK:
            return "ISK";
        case CurrencyEnumType::JMD:
            return "JMD";
        case CurrencyEnumType::JOD:
            return "JOD";
        case CurrencyEnumType::JPY:
            return "JPY";
        case CurrencyEnumType::KES:
            return "KES";
        case CurrencyEnumType::KGS:
            return "KGS";
        case CurrencyEnumType::KHR:
            return "KHR";
        case CurrencyEnumType::KMF:
            return "KMF";
        case CurrencyEnumType::KPW:
            return "KPW";
        case CurrencyEnumType::KRW:
            return "KRW";
        case CurrencyEnumType::KWD:
            return "KWD";
        case CurrencyEnumType::KYD:
            return "KYD";
        case CurrencyEnumType::KZT:
            return "KZT";
        case CurrencyEnumType::LAK:
            return "LAK";
        case CurrencyEnumType::LBP:
            return "LBP";
        case CurrencyEnumType::LKR:
            return "LKR";
        case CurrencyEnumType::LRD:
            return "LRD";
        case CurrencyEnumType::LSL:
            return "LSL";
        case CurrencyEnumType::LYD:
            return "LYD";
        case CurrencyEnumType::MAD:
            return "MAD";
        case CurrencyEnumType::MDL:
            return "MDL";
        case CurrencyEnumType::MGA:
            return "MGA";
        case CurrencyEnumType::MKD:
            return "MKD";
        case CurrencyEnumType::MMK:
            return "MMK";
        case CurrencyEnumType::MNT:
            return "MNT";
        case CurrencyEnumType::MOP:
            return "MOP";
        case CurrencyEnumType::MRO:
            return "MRO";
        case CurrencyEnumType::MUR:
            return "MUR";
        case CurrencyEnumType::MVR:
            return "MVR";
        case CurrencyEnumType::MWK:
            return "MWK";
        case CurrencyEnumType::MXN:
            return "MXN";
        case CurrencyEnumType::MXV:
            return "MXV";
        case CurrencyEnumType::MYR:
            return "MYR";
        case CurrencyEnumType::MZN:
            return "MZN";
        case CurrencyEnumType::NAD:
            return "NAD";
        case CurrencyEnumType::NGN:
            return "NGN";
        case CurrencyEnumType::NIO:
            return "NIO";
        case CurrencyEnumType::NOK:
            return "NOK";
        case CurrencyEnumType::NPR:
            return "NPR";
        case CurrencyEnumType::NZD:
            return "NZD";
        case CurrencyEnumType::OMR:
            return "OMR";
        case CurrencyEnumType::PAB:
            return "PAB";
        case CurrencyEnumType::PEN:
            return "PEN";
        case CurrencyEnumType::PGK:
            return "PGK";
        case CurrencyEnumType::PHP:
            return "PHP";
        case CurrencyEnumType::PKR:
            return "PKR";
        case CurrencyEnumType::PLN:
            return "PLN";
        case CurrencyEnumType::PYG:
            return "PYG";
        case CurrencyEnumType::QAR:
            return "QAR";
        case CurrencyEnumType::RON:
            return "RON";
        case CurrencyEnumType::RSD:
            return "RSD";
        case CurrencyEnumType::RUB:
            return "RUB";
        case CurrencyEnumType::RWF:
            return "RWF";
        case CurrencyEnumType::SAR_:
            return "SAR";
        case CurrencyEnumType::SBD:
            return "SBD";
        case CurrencyEnumType::SCR:
            return "SCR";
        case CurrencyEnumType::SDG:
            return "SDG";
        case CurrencyEnumType::SEK:
            return "SEK";
        case CurrencyEnumType::SGD:
            return "SGD";
        case CurrencyEnumType::SHP:
            return "SHP";
        case CurrencyEnumType::SLL:
            return "SLL";
        case CurrencyEnumType::SOS:
            return "SOS";
        case CurrencyEnumType::SRD:
            return "SRD";
        case CurrencyEnumType::SSP:
            return "SSP";
        case CurrencyEnumType::STD:
            return "STD";
        case CurrencyEnumType::SVC:
            return "SVC";
        case CurrencyEnumType::SYP:
            return "SYP";
        case CurrencyEnumType::SZL:
            return "SZL";
        case CurrencyEnumType::THB:
            return "THB";
        case CurrencyEnumType::TJS:
            return "TJS";
        case CurrencyEnumType::TMT:
            return "TMT";
        case CurrencyEnumType::TND:
            return "TND";
        case CurrencyEnumType::TOP:
            return "TOP";
        case CurrencyEnumType::TRY:
            return "TRY";
        case CurrencyEnumType::TTD:
            return "TTD";
        case CurrencyEnumType::TWD:
            return "TWD";
        case CurrencyEnumType::TZS:
            return "TZS";
        case CurrencyEnumType::UAH:
            return "UAH";
        case CurrencyEnumType::UGX:
            return "UGX";
        case CurrencyEnumType::USD:
            return "USD";
        case CurrencyEnumType::USN:
            return "USN";
        case CurrencyEnumType::UYI:
            return "UYI";
        case CurrencyEnumType::UYU:
            return "UYU";
        case CurrencyEnumType::UZS:
            return "UZS";
        case CurrencyEnumType::VEF:
            return "VEF";
        case CurrencyEnumType::VND:
            return "VND";
        case CurrencyEnumType::VUV:
            return "VUV";
        case CurrencyEnumType::WST:
            return "WST";
        case CurrencyEnumType::XAF:
            return "XAF";
        case CurrencyEnumType::XAG:
            return "XAG";
        case CurrencyEnumType::XAU:
            return "XAU";
        case CurrencyEnumType::XBA:
            return "XBA";
        case CurrencyEnumType::XBB:
            return "XBB";
        case CurrencyEnumType::XBC:
            return "XBC";
        case CurrencyEnumType::XBD:
            return "XBD";
        case CurrencyEnumType::XCD:
            return "XCD";
        case CurrencyEnumType::XDR:
            return "XDR";
        case CurrencyEnumType::XOF:
            return "XOF";
        case CurrencyEnumType::XPD:
            return "XPD";
        case CurrencyEnumType::XPF:
            return "XPF";
        case CurrencyEnumType::XPT:
            return "XPT";
        case CurrencyEnumType::XSU:
            return "XSU";
        case CurrencyEnumType::XTS:
            return "XTS";
        case CurrencyEnumType::XUA:
            return "XUA";
        case CurrencyEnumType::XXX:
            return "XXX";
        case CurrencyEnumType::YER:
            return "YER";
        case CurrencyEnumType::ZAR:
            return "ZAR";
        case CurrencyEnumType::ZMW:
            return "ZMW";
        case CurrencyEnumType::ZWL:
            return "ZWL";
        default:
            return "EUR"; // Default is Euro
    }
}
CurrencyEnumType fromString(const std::string &str, CurrencyEnumType) {
    for (int i = 0; i != static_cast<int>(CurrencyEnumType::ZWL); i++) {
        if (str == toString(static_cast<CurrencyEnumType>(i))) {
            return static_cast<CurrencyEnumType>(i);
        }
    }
    return CurrencyEnumType::EUR; // Default value
}