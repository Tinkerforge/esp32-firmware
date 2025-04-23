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

 #include "../module_dependencies.h"
 #include "event_log_prefix.h"
 
#include "usecaseinformation.h"

bool convertToJson(const UseCaseSupportType &useCaseSupport, JsonObject &obj)
{
    serializeOptional(obj, "useCaseName", useCaseSupport.useCaseName);
    serializeOptional(obj, "useCaseVersion", useCaseSupport.useCaseVersion);
    serializeOptional(obj, "useCaseAvailable", useCaseSupport.useCaseAvailable);
    if (useCaseSupport.scenarioSupport.has_value()) {
        JsonArray scenarioSupportArray = obj.createNestedArray("scenarioSupport");
        for (const auto &scenario : useCaseSupport.scenarioSupport.value()) {
            scenarioSupportArray.add(scenario);
        }
    }
    return true;
}

void convertFromJson(const JsonObject &obj, UseCaseSupportType &useCaseSupport)
{
    useCaseSupport.useCaseName = deserializeOptional<UseCaseNameType>(obj, "useCaseName");
    useCaseSupport.useCaseVersion = deserializeOptional<SpecificationVersionType>(obj, "useCaseVersion");
    useCaseSupport.useCaseAvailable = deserializeOptional<bool>(obj, "useCaseAvailable");
    useCaseSupport.scenarioSupport = deserializeOptional<std::vector<UseCaseScenarioSupportType>>(obj, "scenarioSupport");
}

bool convertToJson(const UseCaseSupportElementsType &useCaseSupport, JsonObject &obj)
{
    serializeOptional(obj, "useCaseName", useCaseSupport.useCaseName);
    serializeOptional(obj, "useCaseVersion", useCaseSupport.useCaseVersion);
    serializeOptional(obj, "useCaseAvailable", useCaseSupport.useCaseAvailable);
    serializeOptional(obj, "scenarioSupport", useCaseSupport.scenarioSupport);
    
    return true;
}

void convertFromJson(const JsonObject &obj, UseCaseSupportElementsType &useCaseSupport)
{
    useCaseSupport.useCaseName = deserializeOptional<ElementTagType>(obj, "useCaseName");
    useCaseSupport.useCaseVersion = deserializeOptional<ElementTagType>(obj, "useCaseVersion");
    useCaseSupport.useCaseAvailable = deserializeOptional<ElementTagType>(obj, "useCaseAvailable");
    useCaseSupport.scenarioSupport = deserializeOptional<ElementTagType>(obj, "scenarioSupport");
}

bool convertToJson(const UseCaseSupportSelectorsType &useCaseSupport, JsonObject &obj)
{
    serializeOptional(obj, "useCaseName", useCaseSupport.useCaseName);
    serializeOptional(obj, "useCaseVersion", useCaseSupport.useCaseVersion);
    serializeOptional(obj, "scenarioSupport", useCaseSupport.scenarioSupport);
    
    return true;
}

void convertFromJson(const JsonObject &obj, UseCaseSupportSelectorsType &useCaseSupport)
{
    useCaseSupport.useCaseName = deserializeOptional<UseCaseNameType>(obj, "useCaseName");
    useCaseSupport.useCaseVersion = deserializeOptional<SpecificationVersionType>(obj, "useCaseVersion");
    useCaseSupport.scenarioSupport = deserializeOptional<UseCaseScenarioSupportType>(obj, "scenarioSupport");
}

bool convertToJson(const UseCaseInformationDataType &useCaseInformation, JsonObject &obj)
{
    serializeOptional(obj, "address", useCaseInformation.address);
    serializeOptional(obj, "actor", useCaseInformation.actor);
    serializeOptional(obj, "useCaseSupport", useCaseInformation.useCaseSupport);
    
    return true;
}

void convertFromJson(const JsonObject &obj, UseCaseInformationDataType &useCaseInformation)
{
    useCaseInformation.address = deserializeOptional<FeatureAddressType>(obj, "address");
    useCaseInformation.actor = deserializeOptional<UseCaseActorType>(obj, "actor");
    useCaseInformation.useCaseSupport = deserializeOptional<UseCaseSupportType>(obj, "useCaseSupport");
}

bool convertToJson(const UseCaseInformationDataElementsType &useCaseInformation, JsonObject &obj)
{
    serializeOptional(obj, "address", useCaseInformation.address);
    serializeOptional(obj, "actor", useCaseInformation.actor);
    serializeOptional(obj, "useCaseSupport", useCaseInformation.useCaseSupport);
    
    return true;
}

void convertFromJson(const JsonObject &obj, UseCaseInformationDataElementsType &useCaseInformation)
{
    useCaseInformation.address = deserializeOptional<ElementTagType>(obj, "address");
    useCaseInformation.actor = deserializeOptional<ElementTagType>(obj, "actor");
    useCaseInformation.useCaseSupport = deserializeOptional<ElementTagType>(obj, "useCaseSupport");
}

bool convertToJson(const UseCaseInformationListDataType &useCaseInformation, JsonObject &obj)
{
    serializeOptional(obj, "useCaseInformation", useCaseInformation.useCaseInformation);
    return true;
}
void convertFromJson(const JsonObject &obj, UseCaseInformationListDataType &useCaseInformation)
{
    useCaseInformation.useCaseInformation = deserializeOptional<std::vector<UseCaseInformationDataType>>(obj, "useCaseInformation");
}

bool convertToJson(const UseCaseInformationListDataSelectorsType &useCaseInformation, JsonObject &obj)
{
    serializeOptional(obj, "address", useCaseInformation.address);
    serializeOptional(obj, "actor", useCaseInformation.actor);
    serializeOptional(obj, "useCaseSupport", useCaseInformation.useCaseSupport);
    
    return true;
}

void convertFromJson(const JsonObject &obj, UseCaseInformationListDataSelectorsType &useCaseInformation)
{
    useCaseInformation.address = deserializeOptional<FeatureAddressType>(obj, "address");
    useCaseInformation.actor = deserializeOptional<UseCaseActorType>(obj, "actor");
    useCaseInformation.useCaseSupport = deserializeOptional<UseCaseSupportType>(obj, "useCaseSupport");
}
