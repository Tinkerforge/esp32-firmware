#include "ship_types.h"

#include "build.h"
#include "eebus.h"
#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"
#include <ArduinoJson.h>

namespace SHIP_TYPES
{

DeserializationResult ShipMessageDataType::json_to_type(String json)
{
    DynamicJsonDocument doc{SHIP_TYPES_MAX_JSON_SIZE};

    DeserializationError error = deserializeJson(doc, json);
    doc.shrinkToFit(); // Make this a bit smaller
    if (error) {
        logger.printfln("J2T ShipMessageData Error during JSON deserialization : %s", error.c_str());
        return DeserializationResult::ERROR;
    }

    JsonObject data = doc["data"];

    if (data.isNull()) {
        logger.printfln("J2T ShipMessageData Error: No data object found");
        return DeserializationResult::ERROR;
    }
    if (data["header"]["protocolId"] == nullptr || data["payload"] == nullptr) {
        logger.printfln("J2T ShipMessageData Error: Data invalid");
        valid = false;
        return DeserializationResult::ERROR;
    }
    protocol_id = data["header"]["protocolId"].as<String>();
    payload = data["payload"].as<String>();
    valid = true;

    JsonObject data_extension = data["extension"];
    // Optional fields

    DeserializeOptionalField(&data_extension, "extensionId", &extension_id_valid, &extension_id);
    DeserializeOptionalField(&data_extension, "binary", &extension_binary_valid, &extension_binary);
    DeserializeOptionalField(&data_extension, "string", &extension_string_valid, &extension_string);

    return DeserializationResult::SUCCESS;
}

String ShipMessageDataType::type_to_json()
{
    DynamicJsonDocument doc{SHIP_TYPES_MAX_JSON_SIZE};

    JsonObject data = doc["data"].to<JsonObject>();
    data["header"]["protocolId"] = protocol_id;
    data["payload"] = payload;

    if (extension_id_valid || extension_binary_valid || extension_string_valid) {
        JsonObject data_extension = data.createNestedObject("extension");
        if (extension_id_valid) {
            data_extension["extensionId"] = extension_id;
        }
        if (extension_binary_valid) {
            JsonArray data_extension_binary = data_extension.createNestedArray("binary");
            for (const auto &value : extension_binary) {
                data_extension_binary.add(value);
            }
        }
        if (extension_string_valid) {
            data_extension["string"] = extension_string;
        }
    }

    String output;
    doc.shrinkToFit();
    serializeJson(doc, output);
    return output;
}

void DeserializeOptionalField(JsonObject *data, const char *field_name, bool *field_valid, String *field_value)
{
    if (data->containsKey(field_name)) {
        *field_value = (*data)[field_name].as<String>();
        *field_valid = true;
    } else {
        *field_valid = false;
    }
}

template <typename T>
void DeserializeOptionalField(JsonObject *data, const char *field_name, bool *field_valid, std::vector<T> *field_value)
{
    if (data->containsKey(field_name)) {
        for (JsonVariant value : (*data)[field_name].as<JsonArray>()) {
            field_value->push_back(value.as<T>());
        }
        *field_valid = true;
    } else {
        *field_valid = false;
    }
}
} // namespace SHIP_TYPES