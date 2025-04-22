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

#include "bill.h"
#include "../module_dependencies.h"
#include "event_log_prefix.h"

std::string toString(BillPositionTypeEnumType billPositionType)
{

    switch (billPositionType) {
        case BillPositionTypeEnumType::GridElectricEnergy:
            return "gridElectricEnergy";
        case BillPositionTypeEnumType::SelfProducedElectricEnergy:
            return "selfProducedElectricEnergy";
        default:
            return "Unknown bill position type";
    }
}

BillPositionTypeEnumType fromString(const std::string &str, BillPositionTypeEnumType)
{
    if (str == "gridElectricEnergy") {
        return BillPositionTypeEnumType::GridElectricEnergy;
    } else if (str == "selfProducedElectricEnergy") {
        return BillPositionTypeEnumType::SelfProducedElectricEnergy;
    }
    logger.printfln("Unknown bill position type: %s", str.c_str());
    return BillPositionTypeEnumType::GridElectricEnergy; // Default value
}

std::string toString(BillCostTypeEnumType billCostType)
{
    switch (billCostType) {
        case BillCostTypeEnumType::AbsolutePrice:
            return "absolutePrice";
        case BillCostTypeEnumType::RelativePrice:
            return "relativePrice";
        case BillCostTypeEnumType::Co2Emission:
            return "co2Emission";
        case BillCostTypeEnumType::RenewableEnergy:
            return "renewableEnergy";
        case BillCostTypeEnumType::RadioactiveWaste:
            return "radioactiveWaste";
        default:
            return "Unknown bill cost type";
    }
}

BillCostTypeEnumType fromString(const std::string &str, BillCostTypeEnumType)
{
    if (str == "absolutePrice") {
        return BillCostTypeEnumType::AbsolutePrice;
    } else if (str == "relativePrice") {
        return BillCostTypeEnumType::RelativePrice;
    } else if (str == "co2Emission") {
        return BillCostTypeEnumType::Co2Emission;
    } else if (str == "renewableEnergy") {
        return BillCostTypeEnumType::RenewableEnergy;
    } else if (str == "radioactiveWaste") {
        return BillCostTypeEnumType::RadioactiveWaste;
    }
    logger.printfln("Unknown bill cost type: %s", str.c_str());
    return BillCostTypeEnumType::AbsolutePrice; // Default value
}

void convertFromJson(const JsonObjectConst &obj, BillValueType &billValue)
{
    if (obj.containsKey("valueId")) {
        billValue.valueId = obj["valueId"].as<uint32_t>();
    }
    if (obj.containsKey("unit")) {
        billValue.unitOfMeasurement = fromString(obj["unit"].as<std::string>(), UnitOfMeasurementEnumType::Unknown);
    }
    billValue.value = deserializeOptional<ScaledNumberType>(obj, "value");
    billValue.value_percentage = deserializeOptional<ScaledNumberType>(obj, "valuePercentage");
}

bool convertToJson(const BillValueType &billValue, JsonObject &obj)
{
    if (billValue.valueId.has_value()) {
        obj["valueId"] = billValue.valueId.value();
    }
    if (billValue.unitOfMeasurement.has_value()) {
        obj["unitOfMeasurement"] = toString(billValue.unitOfMeasurement.value());
    }
    serializeOptional(obj, "value", billValue.value);
    serializeOptional(obj, "valuePercentage", billValue.value_percentage);
    return true;
}

bool convertToJson(const BillValueElementsType &billValue, JsonObject &obj)
{
    serializeOptional(obj, "valueId", billValue.value_id);
    serializeOptional(obj, "unit", billValue.unit);
    serializeOptional(obj, "value", billValue.value);
    serializeOptional(obj, "valuePercentage", billValue.value_percentage);
    return true;
}

void convertFromJson(const JsonObject &obj, BillValueElementsType &billValue)
{
    billValue.value_id = deserializeOptional<ElementTagType>(obj, "valueId");
    billValue.unit = deserializeOptional<ElementTagType>(obj, "unit");
    billValue.value = deserializeOptional<ScaledNumberElementsType>(obj, "value");
    billValue.value_percentage = deserializeOptional<ScaledNumberElementsType>(obj, "valuePercentage");
}

bool convertToJson(const BillCostType &billCost, JsonObject &obj)
{
    serializeOptional(obj, "costId", billCost.costID);
    serializeOptional(obj, "costType", billCost.costType);
    serializeOptional(obj, "valueId", billCost.valueId);
    serializeOptional(obj, "unit", billCost.unit);
    serializeOptional(obj, "currency", billCost.currency);
    serializeOptional(obj, "cost", billCost.cost);
    serializeOptional(obj, "costPercentage", billCost.costPercentage);
    return true;
}

void convertFromJson(const JsonObject &obj, BillCostType &billCost)
{
    billCost.costID = deserializeOptional<BillCostIdType>(obj, "costID");
    billCost.costType = deserializeOptional<BillCostTypeType>(obj, "costType");
    billCost.valueId = deserializeOptional<BillValueIdType>(obj, "valueId");
    billCost.unit = deserializeOptional<UnitOfMeasurementType>(obj, "unit");
    billCost.currency = deserializeOptional<CurrencyType>(obj, "currency");
    billCost.cost = deserializeOptional<ScaledNumberType>(obj, "cost");
    billCost.costPercentage = deserializeOptional<ScaledNumberType>(obj, "costPercentage");
}

bool convertToJson(const BillCostElementsType &billCost, JsonObject &obj)
{
    serializeOptional(obj, "costId", billCost.costId);
    serializeOptional(obj, "costType", billCost.costType);
    serializeOptional(obj, "valueId", billCost.valueId);
    serializeOptional(obj, "unit", billCost.unit);
    serializeOptional(obj, "currency", billCost.currency);
    serializeOptional(obj, "cost", billCost.cost);
    serializeOptional(obj, "costPercentage", billCost.costPercentage);
    return true;
}
void convertFromJson(const JsonObject &obj, BillCostElementsType &billCost)
{
    billCost.costId = deserializeOptional<ElementTagType>(obj, "costId");
    billCost.costType = deserializeOptional<ElementTagType>(obj, "costType");
    billCost.valueId = deserializeOptional<ElementTagType>(obj, "valueId");
    billCost.unit = deserializeOptional<ElementTagType>(obj, "unit");
    billCost.currency = deserializeOptional<ElementTagType>(obj, "currency");
    billCost.cost = deserializeOptional<ScaledNumberElementsType>(obj, "cost");
    billCost.costPercentage = deserializeOptional<ScaledNumberElementsType>(obj, "costPercentage");
}

bool convertToJson(const BillPositionType &billPosition, JsonObject &obj) {
    serializeOptional(obj, "positionId", billPosition.positionId);
    serializeOptional(obj, "positionType", billPosition.positionType);
    serializeOptional(obj, "timePeriod", billPosition.timePeriod);
    serializeOptional(obj, "value", billPosition.value);
    serializeOptional(obj, "cost", billPosition.cost);
    serializeOptional(obj, "label", billPosition.label);
    serializeOptional(obj, "description", billPosition.description);
    return true;
}
void convertFromJson(const JsonObject &obj, BillPositionType &billPosition) {
    billPosition.positionId = deserializeOptional<BillPositionIdType>(obj, "positionId");
    billPosition.positionType = deserializeOptional<BillPositionTypeType>(obj, "positionType");
    billPosition.timePeriod = deserializeOptional<TimePeriodType>(obj, "timePeriod");
    billPosition.value = deserializeOptional<BillValueType>(obj, "value");
    billPosition.cost = deserializeOptional<BillCostType>(obj, "cost");
    billPosition.label = deserializeOptional<LabelType>(obj, "label");
    billPosition.description = deserializeOptional<DescriptionType>(obj, "description");
}
