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
#include "build.h"

#include "tools.h"
#include <ArduinoJson.h> // Include this even though its already included in TFJson.h but compiler got angry with me so just to be safe
#include <TFJson.h>
#include <optional>

// General helper functions for JSON serialization/deserialization
template <typename T> void serializeOptional(JsonObject &obj, const char *key, const std::optional<T> &value);
template <typename T> std::optional<T> deserializeOptional(const JsonObjectConst &obj, const char *key);
template <typename T> std::optional<T> deserializeOptional(const JsonObject &obj, const char *key);

// For serializing/deserializing std::vector<T>
// From https://arduinojson.org/v7/how-to/create-converters-for-stl-containers/
namespace ArduinoJson
{
template <typename T> struct Converter<std::vector<T>> {
    static void toJson(const std::vector<T> &src, JsonVariant dst)
    {
        JsonArray array = dst.to<JsonArray>();
        for (T item : src)
            array.add(item);
    }

    static std::vector<T> fromJson(JsonVariantConst src)
    {
        std::vector<T> dst;
        for (T item : src.as<JsonArrayConst>())
            dst.push_back(item);
        return dst;
    }

    static bool checkJson(JsonVariantConst src)
    {
        JsonArrayConst array = src;
        bool result = array;
        for (JsonVariantConst item : array)
            result &= item.is<T>();
        return result;
    }
};
} // namespace ArduinoJson

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
std::string toString(MonthType month);
MonthType fromString(const std::string &str, MonthType);

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

enum class UnitOfMeasurementEnumType {
    // SI units. Divisions are replaced by underscore (m/s -> m_s). Power is ommitted (m^2 -> m2)
    Unknown = 0,
    One, //Should be 1
    M,
    Kg,
    S,
    A,
    K,
    Mol,
    Cd,
    V,
    W,
    Wh,
    Va,
    Vah,
    Var,
    Varh,
    Degc,
    Degf,
    Lm,
    Ohm,
    Hz,
    Db,
    Dbm,
    Pct,
    Ppm,
    L,
    L_s,
    L_h,
    Deg,
    Rad,
    Rad_s,
    Sr,
    Gy,
    Bq,
    Bq_m3,
    Sv,
    Rd,
    C,
    F,
    H,
    J,
    N,
    N_m,
    N_s,
    Wb,
    T,
    Pa,
    Bar,
    Atm,
    Psi,
    Mmhg,
    M2,
    M3,
    M3_h,
    M_s,
    M_s2,
    M3_s,
    M_m3,
    Kg_m3,
    Kg_m,
    M2_s,
    W_m_k,
    J_k,
    One_s,
    W_m2,
    J_m2,
    S_cap,
    S_m,
    K_s,
    Pa_s,
    J_kg_k,
    Vs,
    V_m,
    V_hz,
    As,
    A_m,
    Hz_s,
    Kg_s,
    Kg_m2,
    J_wh,
    W_s,
    Ft3,
    Ft3_h,
    Ccf,
    Ccf_h,
    UsLiqGal,
    UsLiqGal_h,
    ImpGal,
    ImpGal_h,
    Btu,
    Btu_h,
    Ah,
    Kg_wh,
};
std::string toString(UnitOfMeasurementEnumType unit);
UnitOfMeasurementEnumType fromString(const std::string &str, UnitOfMeasurementEnumType);
using UnitOfMeasurementType = UnitOfMeasurementEnumType;

using NumberType = int64_t;
using ScaleType = int8_t;

struct ScaledNumberType {
    std::optional<NumberType> number;
    std::optional<ScaleType> scale;
};
bool convertToJson(const ScaledNumberType &scaledNumber, JsonObject &obj);
void convertFromJson(const JsonObject &obj, ScaledNumberType &scaledNumber);

struct ScaledNumberElementsType {
    std::optional<ElementTagType> number;
    std::optional<ElementTagType> scale;
};
bool convertToJson(const ScaledNumberElementsType &scaledNumber, JsonObject &obj);
void convertFromJson(const JsonObject &obj, ScaledNumberElementsType &scaledNumber);

enum class CurrencyEnumType {
    AED,
    AFN,
    ALL,
    AMD,
    ANG,
    AOA,
    ARS,
    AUD,
    AWG,
    AZN,
    BAM,
    BBD,
    BDT,
    BGN,
    BHD,
    BIF,
    BMD,
    BND,
    BOB,
    BOV,
    BRL,
    BSD,
    BTN,
    BWP,
    BYR,
    BZD,
    CAD,
    CDF,
    CHE,
    CHF,
    CHW,
    CLF,
    CLP,
    CNY,
    COP,
    COU,
    CRC,
    CUC,
    CUP,
    CVE,
    CZK,
    DJF,
    DKK,
    DOP,
    DZD,
    EGP,
    ERN,
    ETB,
    EUR,
    FJD,
    FKP,
    GBP,
    GEL,
    GHS,
    GIP,
    GMD,
    GNF,
    GTQ,
    GYD,
    HKD,
    HNL,
    HRK,
    HTG,
    HUF,
    IDR,
    ILS,
    INR,
    IQD,
    IRR,
    ISK,
    JMD,
    JOD,
    JPY,
    KES,
    KGS,
    KHR,
    KMF,
    KPW,
    KRW,
    KWD,
    KYD,
    KZT,
    LAK,
    LBP,
    LKR,
    LRD,
    LSL,
    LYD,
    MAD,
    MDL,
    MGA,
    MKD,
    MMK,
    MNT,
    MOP,
    MRO,
    MUR,
    MVR,
    MWK,
    MXN,
    MXV,
    MYR,
    MZN,
    NAD,
    NGN,
    NIO,
    NOK,
    NPR,
    NZD,
    OMR,
    PAB,
    PEN,
    PGK,
    PHP,
    PKR,
    PLN,
    PYG,
    QAR,
    RON,
    RSD,
    RUB,
    RWF,
    SAR_, // This is defined as a Register too
    SBD,
    SCR,
    SDG,
    SEK,
    SGD,
    SHP,
    SLL,
    SOS,
    SRD,
    SSP,
    STD,
    SVC,
    SYP,
    SZL,
    THB,
    TJS,
    TMT,
    TND,
    TOP,
    TRY,
    TTD,
    TWD,
    TZS,
    UAH,
    UGX,
    USD,
    USN,
    UYI,
    UYU,
    UZS,
    VEF,
    VND,
    VUV,
    WST,
    XAF,
    XAG,
    XAU,
    XBA,
    XBB,
    XBC,
    XBD,
    XCD,
    XDR,
    XOF,
    XPD,
    XPF,
    XPT,
    XSU,
    XTS,
    XUA,
    XXX,
    YER,
    ZAR,
    ZMW,
    ZWL,

};

std::string toString(CurrencyEnumType currency);
CurrencyEnumType fromString(const std::string &str, CurrencyEnumType);

using CurrencyType = CurrencyEnumType;

enum ScopeTypeEnumType {
    Ac = 0,
    AcCosPhiGrid,
    AcCurrentA,
    AcCurrentB,
    AcCurrentC,
    AcFrequencyGrid,
    AcPowerA,
    AcPowerB,
    AcPowerC,
    AcPowerLimitPct,
    AcPowerTotal,
    AcVoltageA,
    AcVoltageB,
    AcVoltageC,
    AcYieldDay,
    AcYieldTotal,
    DcCurrent,
    DcPower,
    DcString1,
    DcString2,
    DcString3,
    DcString4,
    DcString5,
    DcString6,
    DcTotal,
    DcVoltage,
    DhwTemperature,
    FlowTemperature,
    OutsideAirTemperature,
    ReturnTemperature,
    RoomAirTemperature,
    Charge,
    StateOfCharge,
    Discharge,
    GridConsumption,
    GridFeedIn,
    SelfConsumption,
    OverloadProtection,
    AcPower,
    AcEnergy,
    AcCurrent,
    AcVoltage,
    BatteryControl,
    SimpleIncentiveTable
};
std::string toString(ScopeTypeEnumType scope);
ScopeTypeEnumType fromString(const std::string &str, ScopeTypeEnumType);

using ScopeTypeType = ScopeTypeEnumType;

using AddressDeviceType = std::string;
using AddressEntityType = uint32_t;
using AddressFeatureType = uint32_t;

struct FeatureAddressType {
    std::optional<AddressDeviceType> device;
    std::optional<std::vector<AddressEntityType>> entity;
    std::optional<AddressFeatureType> feature;
};
bool convertToJson(const FeatureAddressType &address, JsonObject &obj);
void convertFromJson(const JsonObject &obj, FeatureAddressType &address);