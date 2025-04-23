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
#include "commondatatypes.h"
#include "tools.h"
#include <ArduinoJson.h> // Include this even though its already included in TFJson.h but compiler got angry with me so just to be safe
#include <TFJson.h>
#include <optional>

using BillIdType = uint32_t;

enum class BillTypeEnumType {
    ChargingSummary,
};
std::string toString(BillTypeEnumType billType)
{
    return "chargingSummary";
};
BillTypeEnumType fromString(const std::string &str, BillTypeEnumType)
{
    return BillTypeEnumType::ChargingSummary;
}

using BillTypeType = BillTypeEnumType;

using BillPositionIdType = uint32_t;

using BillPositionCountType = BillPositionIdType;

enum class BillPositionTypeEnumType { GridElectricEnergy, SelfProducedElectricEnergy };
std::string toString(BillPositionTypeEnumType billPositionType);
BillPositionTypeEnumType fromString(const std::string &str, BillPositionTypeEnumType);

using BillPositionTypeType = BillPositionTypeEnumType;

using BillValueIdType = uint32_t;

using BillCostIdType = uint32_t;

enum class BillCostTypeEnumType {
    AbsolutePrice,
    RelativePrice,
    Co2Emission,
    RenewableEnergy,
    RadioactiveWaste,
};
std::string toString(BillCostTypeEnumType billCostType);
BillCostTypeEnumType fromString(const std::string &str, BillCostTypeEnumType);

using BillCostTypeType = BillCostTypeEnumType;

struct BillValueType {
    std::optional<BillValueIdType> valueId;
    std::optional<UnitOfMeasurementType> unitOfMeasurement;
    std::optional<ScaledNumberType> value;
    std::optional<ScaledNumberType> value_percentage;
};
void convertFromJson(const JsonObjectConst &obj, BillValueType &billValue);
bool convertToJson(const BillValueType &billValue, JsonObject &obj);

struct BillValueElementsType {
    std::optional<ElementTagType> value_id;
    std::optional<ElementTagType> unit;
    std::optional<ScaledNumberElementsType> value;
    std::optional<ScaledNumberElementsType> value_percentage;
};
bool convertToJson(const BillValueElementsType &billValue, JsonObject &obj);
void convertFromJson(const JsonObject &obj, BillValueElementsType &billValue);

struct BillCostType {
    std::optional<BillCostIdType> costID;
    std::optional<BillCostTypeType> costType;
    std::optional<BillValueIdType> valueId;
    std::optional<UnitOfMeasurementType> unit;
    std::optional<CurrencyType> currency;
    std::optional<ScaledNumberType> cost;
    std::optional<ScaledNumberType> costPercentage;
};
bool convertToJson(const BillCostType &billCost, JsonObject &obj);
void convertFromJson(const JsonObject &obj, BillCostType &billCost);

struct BillCostElementsType {
    std::optional<ElementTagType> costId;
    std::optional<ElementTagType> costType;
    std::optional<ElementTagType> valueId;
    std::optional<ElementTagType> unit;
    std::optional<ElementTagType> currency;
    std::optional<ScaledNumberElementsType> cost;
    std::optional<ScaledNumberElementsType> costPercentage;
};
bool convertToJson(const BillCostElementsType &billCost, JsonObject &obj);
void convertFromJson(const JsonObject &obj, BillCostElementsType &billCost);

struct BillPositionType {
    std::optional<BillPositionIdType> positionId;
    std::optional<BillPositionTypeType> positionType;
    std::optional<TimePeriodType> timePeriod;
    std::optional<BillValueType> value;
    std::optional<BillCostType> cost;
    std::optional<LabelType> label;
    std::optional<DescriptionType> description;
};
bool convertToJson(const BillPositionType &billPosition, JsonObject &obj);
void convertFromJson(const JsonObject &obj, BillPositionType &billPosition);

struct BillPositionElementsType {
    std::optional<ElementTagType> positionId;
    std::optional<ElementTagType> positionType;
    std::optional<TimePeriodElementsType> timePeriod;
    std::optional<BillValueElementsType> value;
    std::optional<BillCostElementsType> cost;
    std::optional<ElementTagType> label;
    std::optional<ElementTagType> description;
};
// TODO: Implement convertToJson and convertFromJson for BillPositionElementsType

struct BillDataType {
    std::optional<BillIdType> billId;
    std::optional<BillTypeType> billType;
    std::optional<ScopeTypeType> scopeType; // TODO: implement ScopeTypeType in commondatatypes.h
    std::optional<BillPositionType> total;
    std::optional<BillPositionType> position;
};
// TODO: implement convertToJson and convertFromJson for BillDataType

struct BillDataElementsType {
    std::optional<ElementTagType> billId;
    std::optional<ElementTagType> billType;
    std::optional<ElementTagType> scopeType;
    std::optional<BillPositionElementsType> total;
    std::optional<BillPositionElementsType> position;
};
// TODO: implement convertToJson and convertFromJson for BillDataElementsType

struct BillListDataType {
    std::optional<std::vector<BillDataType>> billData;
};
// TODO: implement convertToJson and convertFromJson for BillListDataType

struct BillListDataSelectorsType {
    std::optional<BillIdType> billId;
    std::optional<ScopeTypeType> scopeType;
};
// TODO: implement convertToJson and convertFromJson for BillListDataSelectorsType

struct BillConstraintsDataType {
    std::optional<BillIdType> billId;
    std::optional<BillPositionCountType> positionCountMin;
    std::optional<BillPositionCountType> positionCountMax;
};
//TODO: implement convertToJson and convertFromJson for BillConstraintsDataType

struct BillConstraintsDataElementsType {
    std::optional<ElementTagType> billId;
    std::optional<ElementTagType> positionCountMin;
    std::optional<ElementTagType> positionCountMax;
};
// TODO: implement convertToJson and convertFromJson for BillConstraintsDataElementsType

struct BillConstraintsListDataType {
    std::optional<std::vector<BillConstraintsDataType>> billConstraintsDataElements;
};
// TODO: implement convertToJson and convertFromJson for BillConstraintsListDataType

struct BillConstraintsListDataSelectorsType {
    std::optional<BillIdType> billId;
};
// TODO: implement convertToJson and convertFromJson for BillConstraintsListDataSelectorsType

struct BillDescriptionDataType {
    std::optional<BillIdType> billId;
    std::optional<bool> billWriteable;
    std::optional<bool> updateRequired;
    std::optional<std::vector<BillTypeType>> supportedBillType;
};
// TODO: implement convertToJson and convertFromJson for BillDescriptionDataType

struct BillDescroptionDataElementsType {
    std::optional<ElementTagType> billId;
    std::optional<ElementTagType> billWriteable;
    std::optional<ElementTagType> updateRequired;
    std::optional<ElementTagType> supportedBillType;
};
// TODO: implement convertToJson and convertFromJson for BillDescroptionDataElementsType

struct BillDescriptionListDataType {
    std::optional<std::vector<BillDescriptionDataType>> billDescriptionData;
};
// TODO: implement convertToJson and convertFromJson for BillDescriptionListDataType
struct BillDescriptionListDataSelectorsType {
    std::optional<BillIdType> billId;
};
// TODO: implement convertToJson and convertFromJson for BillDescriptionListDataSelectorsType
