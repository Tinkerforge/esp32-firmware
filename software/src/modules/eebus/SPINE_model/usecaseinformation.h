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
#include <map>
#include <optional>

enum class UseCaseActorEnumType {
    Unknown = 0, // No Known Actor
    Ev,          // EV,  EEBUS CEVC Specification 3.2.1
    Evse,        // EVSE, EEBUS EVCS Specification 3.2
    Cem,         // CEM, FOR Actor Energy Guard and Actor Controllable System
    Gridguard    // GridGuard, for an Actor which only implmenents Energy Guard
    // Add Additional Actors defined in Usecases here
};
std::map<UseCaseActorEnumType, std::string> useCaseActorEnumTypeToString = {{UseCaseActorEnumType::Ev, "EV"},
                                                                            {UseCaseActorEnumType::Evse, "EVSE"},
                                                                            {UseCaseActorEnumType::Cem, "CEM"},
                                                                            {UseCaseActorEnumType::Gridguard, "GridGuard"}};
std::string toString(UseCaseActorEnumType useCaseActor)
{
    if (useCaseActorEnumTypeToString.find(useCaseActor) == useCaseActorEnumTypeToString.end()) {
        return "Unknown";
    }
    return useCaseActorEnumTypeToString[useCaseActor];
}
UseCaseActorEnumType fromString(const std::string &str, UseCaseActorEnumType)
{
    for (const auto &[key, value] : useCaseActorEnumTypeToString) {
        if (value == str) {
            return key;
        }
    }
    return UseCaseActorEnumType::Unknown; // Default value
}

using UseCaseActorType = UseCaseActorEnumType;

enum class UseCaseNameEnumType {
    EvChargingSummary, // evChargingSummary, EEBUS UC TS EVCS 3.1.2

};

std::map<UseCaseNameEnumType, std::string> useCaseNameEnumTypeToString = {{UseCaseNameEnumType::EvChargingSummary, "evChargingSummary"}};
std::string toString(UseCaseNameEnumType useCaseName)
{
    if (useCaseNameEnumTypeToString.find(useCaseName) == useCaseNameEnumTypeToString.end()) {
        return "Unknown";
    }
    return useCaseNameEnumTypeToString[useCaseName];
}
UseCaseNameEnumType fromString(const std::string &str, UseCaseNameEnumType)
{
    for (const auto &[key, value] : useCaseNameEnumTypeToString) {
        if (value == str) {
            return key;
        }
    }
    return UseCaseNameEnumType::EvChargingSummary; // Default value
}

using UseCaseNameType = UseCaseNameEnumType;
using UseCaseScenarioSupportType = uint32_t;

struct UseCaseSupportType {
    std::optional<UseCaseNameType> useCaseName;
    std::optional<SpecificationVersionType> useCaseVersion;
    std::optional<bool> useCaseAvailable;
    std::optional<std::vector<UseCaseScenarioSupportType>> scenarioSupport;
};
bool convertToJson(const UseCaseSupportType &useCaseSupport, JsonObject &obj);
void convertFromJson(const JsonObject &obj, UseCaseSupportType &useCaseSupport);

struct UseCaseSupportElementsType {
    std::optional<ElementTagType> useCaseName;
    std::optional<ElementTagType> useCaseVersion;
    std::optional<ElementTagType> useCaseAvailable;
    std::optional<ElementTagType> scenarioSupport;
};
bool convertToJson(const UseCaseSupportElementsType &useCaseSupport, JsonObject &obj);
void convertFromJson(const JsonObject &obj, UseCaseSupportElementsType &useCaseSupport);

struct UseCaseSupportSelectorsType {
    std::optional<UseCaseNameType> useCaseName;
    std::optional<SpecificationVersionType> useCaseVersion;
    std::optional<UseCaseScenarioSupportType> scenarioSupport;
};
bool convertToJson(const UseCaseSupportSelectorsType &useCaseSupport, JsonObject &obj);
void convertFromJson(const JsonObject &obj, UseCaseSupportSelectorsType &useCaseSupport);

struct UseCaseInformationDataType {
    std::optional<FeatureAddressType> address;
    std::optional<UseCaseActorType> actor;
    std::optional<UseCaseSupportType> useCaseSupport;
};
bool convertToJson(const UseCaseInformationDataType &useCaseInformation, JsonObject &obj);
void convertFromJson(const JsonObject &obj, UseCaseInformationDataType &useCaseInformation);

struct UseCaseInformationDataElementsType {
    std::optional<ElementTagType> address;
    std::optional<ElementTagType> actor;
    std::optional<ElementTagType> useCaseSupport;
};
bool convertToJson(const UseCaseInformationDataElementsType &useCaseInformation, JsonObject &obj);
void convertFromJson(const JsonObject &obj, UseCaseInformationDataElementsType &useCaseInformation);

struct UseCaseInformationListDataType {
    std::optional<std::vector<UseCaseInformationDataType>> useCaseInformation;
};
bool convertToJson(const UseCaseInformationListDataType &useCaseInformation, JsonObject &obj);
void convertFromJson(const JsonObject &obj, UseCaseInformationListDataType &useCaseInformation);

struct UseCaseInformationListDataSelectorsType {
    std::optional<FeatureAddressType> address;
    std::optional<UseCaseActorType> actor;
    std::optional<UseCaseSupportType> useCaseSupport;
};
bool convertToJson(const UseCaseInformationListDataSelectorsType &useCaseInformation, JsonObject &obj);
void convertFromJson(const JsonObject &obj, UseCaseInformationListDataSelectorsType &useCaseInformation);