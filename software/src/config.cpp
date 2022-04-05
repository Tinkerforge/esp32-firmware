/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "config.h"
#include "math.h"

struct printer {
    void operator()(const Config::ConfString &x) const
    {
        Serial.println("string");
    }
    void operator()(const Config::ConfFloat &x) const
    {
        Serial.println("float");
    }
    void operator()(const Config::ConfInt &x) const
    {
        Serial.println("int32_t");
    }
    void operator()(const Config::ConfUint &x) const
    {
        Serial.println("uint32_t");
    }
    void operator()(const Config::ConfBool &x) const
    {
        Serial.println("bool");
    }
    void operator()(std::nullptr_t x) const
    {
        Serial.println("std::nullptr_t");
    }
    void operator()(const Config::ConfArray &x) const
    {
        Serial.println("Array: ");
        for (const Config &c : x.value) {
            strict_variant::apply_visitor(printer{}, c.value);
        }
    }
    void operator()(const Config::ConfObject &x) const
    {
        Serial.println("Object: ");
        for (const std::pair<String, Config> &c : x.value) {
            Serial.print(c.first + ": ");
            strict_variant::apply_visitor(printer{}, c.second.value);
        }
    }
};

struct default_validator {
    String operator()(const Config::ConfString &x) const
    {
        if (x.value.length() < x.minChars)
            return String(String("String of minimum length ") + x.minChars + " was expected, but got " + x.value.length());

        if (x.maxChars == 0 || x.value.length() <= x.maxChars)
            return String("");

        return String(String("String of maximum length ") + x.maxChars + " was expected, but got " + x.value.length());
    }

    String operator()(const Config::ConfFloat &x) const
    {
        if (x.value < x.min)
            return String(String("Float value ") + x.value + " was less than the allowed minimum of " + x.min);
        if (x.value > x.max)
            return String(String("Float value ") + x.value + " was more than the allowed maximum of " + x.max);
        return String("");
    }

    String operator()(const Config::ConfInt &x) const
    {
        if (x.value < x.min)
            return String(String("Integer value ") + x.value + " was less than the allowed minimum of " + x.min);
        if (x.value > x.max)
            return String(String("Integer value ") + x.value + " was more than the allowed maximum of " + x.max);
        return String("");
    }

    String operator()(const Config::ConfUint &x) const
    {
        if (x.value < x.min)
            return String(String("Unsigned integer value ") + x.value + " was less than the allowed minimum of " + x.min);
        if (x.value > x.max)
            return String(String("Unsigned integer value ") + x.value + " was more than the allowed maximum of " + x.max);
        return String("");
    }

    String operator()(const Config::ConfBool &x) const
    {
        return String("");
    }
    String operator()(std::nullptr_t x)
    {
        return String("");
    }

    String operator()(const Config::ConfArray &x) const
    {
        if (x.maxElements > 0 && x.value.size() > x.maxElements)
            return String(String("Array had ") + x.value.size() + " entries, but only " + x.maxElements + " are allowed.");
        if (x.minElements > 0 && x.value.size() < x.minElements)
            return String(String("Array had ") + x.value.size() + " entries, but at least " + x.maxElements + " are required.");

        if (x.variantType >= 0)
            for (int i = 0; i < x.value.size(); ++i)
                if (x.value[i].value.which() != x.variantType)
                    return String(String("[") + i + "] has wrong type");

        for (const Config &elem : x.value) {
            String err = strict_variant::apply_visitor(default_validator{}, elem.value);
            if (err != "")
                return err;
        }

        return String("");
    }

    String operator()(const Config::ConfObject &x) const
    {
        for (const std::pair<String, Config> &elem : x.value) {
            String err = strict_variant::apply_visitor(default_validator{}, elem.second.value);
            if (err != "")
                return err;
        }

        return String("");
    }
};

struct to_json {
    void operator()(const Config::ConfString &x)
    {
        insertHere.set(x.value);
    }
    void operator()(const Config::ConfFloat &x)
    {
        insertHere.set(x.value);
    }
    void operator()(const Config::ConfInt &x)
    {
        insertHere.set(x.value);
    }
    void operator()(const Config::ConfUint &x)
    {
        insertHere.set(x.value);
    }
    void operator()(const Config::ConfBool &x)
    {
        insertHere.set(x.value);
    }
    void operator()(const std::nullptr_t x)
    {
        insertHere.set(x);
    }
    void operator()(const Config::ConfArray &x)
    {
        JsonArray arr = insertHere.as<JsonArray>();
        for (size_t i = 0; i < x.value.size(); ++i) {
            const Config &child = x.value[i];

            if (child.is<Config::ConfObject>()) {
                arr.createNestedObject();
            } else if (child.is<Config::ConfArray>()) {
                arr.createNestedArray();
            } else {
                arr.add(0);
            }

            strict_variant::apply_visitor(to_json{arr[i], keys_to_censor}, x.value[i].value);
        }
    }
    void operator()(const Config::ConfObject &x)
    {
        JsonObject obj = insertHere.as<JsonObject>();
        for (size_t i = 0; i < x.value.size(); ++i) {
            const String &key = x.value[i].first;
            const Config &child = x.value[i].second;

            if (child.is<Config::ConfObject>()) {
                obj.createNestedObject(key);
            } else if (child.is<Config::ConfArray>()) {
                obj.createNestedArray(key);
            } else {
                obj.getOrAddMember(key);
            }

            strict_variant::apply_visitor(to_json{obj[key], keys_to_censor}, child.value);
        }

        for (const String &key : keys_to_censor)
            if (obj.containsKey(key) && !(obj[key].is<String>() && obj[key].as<String>().length() == 0))
                obj[key] = nullptr;
    }

    JsonVariant insertHere;
    const std::vector<String> &keys_to_censor;
};

struct string_length_visitor {
    size_t operator()(const Config::ConfString &x)
    {
        return x.maxChars + 2; // ""
    }
    size_t operator()(const Config::ConfFloat &x)
    {
        // Educated guess, FLT_MAX is ~3*10^38 however it is unlikely that a user will send enough float values longer than 20.
        return 20;
    }
    size_t operator()(const Config::ConfInt &x)
    {
        return 11;
    }
    size_t operator()(const Config::ConfUint &x)
    {
        return 10;
    }
    size_t operator()(const Config::ConfBool &x)
    {
        return 5;
    }
    size_t operator()(std::nullptr_t x)
    {
        return 4;
    }
    size_t operator()(const Config::ConfArray &x)
    {
        return strict_variant::apply_visitor(string_length_visitor{}, x.prototype->value) * x.maxElements +
               (x.maxElements + 1); // [,] and n-1 ,
    }
    size_t operator()(const Config::ConfObject &x)
    {
        size_t sum = 2; // { and }
        for (size_t i = 0; i < x.value.size(); ++i) {
            sum += x.value[i].first.length() + 2; // ""
            sum += strict_variant::apply_visitor(string_length_visitor{}, x.value[i].second.value);
        }
        return sum;
    }
};

struct json_length_visitor {
    size_t operator()(const Config::ConfString &x)
    {
        return zero_copy ? 0 : (x.maxChars + 1);
    }
    size_t operator()(const Config::ConfFloat &x)
    {
        return 0;
    }
    size_t operator()(const Config::ConfInt &x)
    {
        return 0;
    }
    size_t operator()(const Config::ConfUint &x)
    {
        return 0;
    }
    size_t operator()(const Config::ConfBool &x)
    {
        return 0;
    }
    size_t operator()(std::nullptr_t x)
    {
        return 10; // TODO: is this still necessary?
    }
    size_t operator()(const Config::ConfArray &x)
    {
        return strict_variant::apply_visitor(json_length_visitor{zero_copy}, x.prototype->value) * x.maxElements + JSON_ARRAY_SIZE(x.maxElements);
    }
    size_t operator()(const Config::ConfObject &x)
    {
        size_t sum = 0;
        for (size_t i = 0; i < x.value.size(); ++i) {
            if (!zero_copy)
                sum += x.value[i].first.length() + 1;

            sum += strict_variant::apply_visitor(json_length_visitor{zero_copy}, x.value[i].second.value);
        }
        return sum + JSON_OBJECT_SIZE(x.value.size());
    }

    bool zero_copy;
};

struct from_json {
    String operator()(Config::ConfString &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        if (!json_node.is<String>())
            return "JSON node was not a string.";
        x.value = json_node.as<String>();
        return String("");
    }
    String operator()(Config::ConfFloat &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        if (!json_node.is<float>())
            return "JSON node was not a float.";

        x.value = json_node.as<float>();
        return String("");
    }
    String operator()(Config::ConfInt &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        if (!json_node.is<int32_t>())
            return "JSON node was not a signed integer.";
        x.value = json_node.as<int32_t>();
        return String("");
    }
    String operator()(Config::ConfUint &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        if (!json_node.is<uint32_t>())
            return "JSON node was not an unsigned integer.";
        x.value = json_node.as<uint32_t>();
        return String("");
    }
    String operator()(Config::ConfBool &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        if (!json_node.is<bool>())
            return "JSON node was not a boolean.";
        x.value = json_node.as<bool>();
        return String("");
    }
    String operator()(std::nullptr_t x)
    {
        if (json_node.isNull())
            return "";
        if (json_node == "" || json_node == false || json_node == 0)
            return "";
        if (json_node.size() == 0 && (json_node.is<JsonArray>() || json_node.is<JsonObject>()))
            return "";

        return "JSON null node was not null or a falsy value. Use null, \"\", false, 0, [] or {}.";
    }
    String operator()(Config::ConfArray &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        if (!json_node.is<JsonArray>())
            return "JSON node was not an array.";

        JsonArray arr = json_node.as<JsonArray>();

        x.value.clear();
        for (size_t i = 0; i < arr.size(); ++i) {
            x.value.push_back(*x.prototype);
            String inner_error = strict_variant::apply_visitor(from_json{arr[i], force_same_keys, permit_null_updates, false}, x.value[i].value);
            if (inner_error != "")
                return String("[") + i + "]" + inner_error;
        }

        return String("");
    }
    String operator()(Config::ConfObject &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        // If a user passes a non-object to an API that expects an object with exactly one member
        // Try to use the non-object as value for the single member.
        // This allows calling for example evse/external_current_update with the payload 8000 instead of {"current": 8000}
        if (!json_node.is<JsonObject>() && is_root && x.value.size() == 1) {
            String inner_error = strict_variant::apply_visitor(from_json{json_node, force_same_keys, permit_null_updates, false}, x.value[0].second.value);
            if (inner_error != "")
                return String("(inferred) [\"") + x.value[0].first + "\"] " + inner_error;
            else
                return inner_error;
        }

        if (!json_node.is<JsonObject>())
            return "JSON node was not an object.";

        JsonObject obj = json_node.as<JsonObject>();

        if (force_same_keys && obj.size() != x.value.size())
            return String("JSON object had ") + obj.size() + " entries instead of the expected " + x.value.size();

        for (size_t i = 0; i < x.value.size(); ++i) {
            if (!force_same_keys && !obj.containsKey(x.value[i].first))
                continue;

            String inner_error = strict_variant::apply_visitor(from_json{obj[x.value[i].first], force_same_keys, permit_null_updates, false}, x.value[i].second.value);
            if (inner_error != "")
                return String("[\"") + x.value[i].first + "\"]" + inner_error;
        }

        return String("");
    }

    JsonVariant json_node;
    bool force_same_keys;
    bool permit_null_updates;
    bool is_root;
};

struct from_update {
    String operator()(Config::ConfString &x)
    {
        if (Config::containsNull(update))
            return String("");

        if (update->get<String>() == nullptr)
            return "ConfUpdate node was not a string.";
        x.value = *(update->get<String>());
        return String("");
    }
    String operator()(Config::ConfFloat &x)
    {
        if (Config::containsNull(update))
            return String("");

        if (update->get<float>() == nullptr)
            return "ConfUpdate node was not a float.";

        x.value = *(update->get<float>());
        return String("");
    }
    String operator()(Config::ConfInt &x)
    {
        if (Config::containsNull(update))
            return String("");

        if (update->get<int32_t>() == nullptr)
            return "ConfUpdate node was not a signed integer.";
        x.value = *(update->get<int32_t>());
        return String("");
    }
    String operator()(Config::ConfUint &x)
    {
        if (Config::containsNull(update))
            return String("");

        uint32_t new_val = 0;
        if (update->get<uint32_t>() == nullptr) {
            if (update->get<int32_t>() == nullptr || *(update->get<int32_t>()) < 0)
                return "ConfUpdate node was not an unsigned integer.";

            new_val = (uint32_t) *(update->get<int32_t>());
        } else {
            new_val = *(update->get<uint32_t>());
        }
        x.value = new_val;
        return String("");
    }
    String operator()(Config::ConfBool &x)
    {
        if (Config::containsNull(update))
            return String("");

        if (update->get<bool>() == nullptr)
            return "ConfUpdate node was not a boolean.";
        x.value = *(update->get<bool>());
        return String("");
    }
    String operator()(std::nullptr_t x)
    {
        return Config::containsNull(update) ? "" : "JSON null node was not null";
    }
    String operator()(Config::ConfArray &x)
    {
        if (Config::containsNull(update))
            return String("");

        if (update->get<Config::ConfUpdateArray>() == nullptr)
            return "ConfUpdate node was not an array.";

        Config::ConfUpdateArray *arr = update->get<Config::ConfUpdateArray>();

        x.value.clear();
        for (size_t i = 0; i < arr->elements.size(); ++i) {
            x.value.push_back(*x.prototype);
            String inner_error = strict_variant::apply_visitor(from_update{&arr->elements[i]}, x.value[i].value);
            if (inner_error != "")
                return String("[") + i + "]" + inner_error;
        }

        return String("");
    }
    String operator()(Config::ConfObject &x)
    {
        if (Config::containsNull(update))
            return String("");

        if (update->get<Config::ConfUpdateObject>() == nullptr) {
            Serial.println(update->which());
            return "ConfUpdate node was not an object.";
        }

        Config::ConfUpdateObject *obj = update->get<Config::ConfUpdateObject>();

        if (obj->elements.size() != x.value.size())
            return String("ConfUpdate object had ") + obj->elements.size() + " entries instead of the expected " + x.value.size();

        for (size_t i = 0; i < x.value.size(); ++i) {
            size_t obj_idx = 0xFFFFFFFF;
            for (size_t j = 0; j < x.value.size(); ++j) {
                if (obj->elements[j].first != x.value[i].first)
                    continue;
                obj_idx = j;
                break;
            }
            if (obj_idx == 0xFFFFFFFF)
                return String("Key ") + x.value[i].first + String("not found in ConfUpdate object");

            String inner_error = strict_variant::apply_visitor(from_update{&obj->elements[obj_idx].second}, x.value[i].second.value);
            if (inner_error != "")
                return String("[\"") + x.value[i].first + "\"]" + inner_error;
        }

        return String("");
    }

    Config::ConfUpdate *update;
};

struct is_updated {
    bool operator()(const Config::ConfString &x)
    {
        return false;
    }
    bool operator()(const Config::ConfFloat &x)
    {
        return false;
    }
    bool operator()(const Config::ConfInt &x)
    {
        return false;
    }
    bool operator()(const Config::ConfUint &x)
    {
        return false;
    }
    bool operator()(const Config::ConfBool &x)
    {
        return false;
    }
    bool operator()(const std::nullptr_t x)
    {
        return false;
    }
    bool operator()(const Config::ConfArray &x) const
    {
        for (const Config &c : x.value) {
            if (((c.updated & api_backend_flag) != 0) || strict_variant::apply_visitor(is_updated{api_backend_flag}, c.value))
                return true;
        }
        return false;
    }
    bool operator()(const Config::ConfObject &x) const
    {
        for (const std::pair<String, Config> &c : x.value) {
            if (((c.second.updated & api_backend_flag) != 0) || strict_variant::apply_visitor(is_updated{api_backend_flag}, c.second.value))
                return true;
        }
        return false;
    }
    uint8_t api_backend_flag;
};

struct set_updated_false {
    void operator()(Config::ConfString &x)
    {
    }
    void operator()(Config::ConfFloat &x)
    {
    }
    void operator()(Config::ConfInt &x)
    {
    }
    void operator()(Config::ConfUint &x)
    {
    }
    void operator()(Config::ConfBool &x)
    {
    }
    void operator()(std::nullptr_t x)
    {
    }
    void operator()(Config::ConfArray &x)
    {
        for (Config &c : x.value) {
            c.updated &= ~api_backend_flag;
            strict_variant::apply_visitor(set_updated_false{api_backend_flag}, c.value);
        }
    }
    void operator()(Config::ConfObject &x)
    {
        for (std::pair<String, Config> &c : x.value) {
            c.second.updated &= ~api_backend_flag;
            strict_variant::apply_visitor(set_updated_false{api_backend_flag}, c.second.value);
        }
    }
    uint8_t api_backend_flag;
};

Config Config::Str(String s, uint16_t minChars, uint16_t maxChars)
{
    return Config{ConfString{s, minChars, maxChars == 0 ? (uint16_t)s.length() : maxChars}, (uint8_t)0xFF};
}

Config Config::Float(float d, float min, float max)
{
    return Config{ConfFloat{d, min, max}, (uint8_t)0xFF};
}

Config Config::Int(int32_t i, int32_t min, int32_t max)
{
    return Config{ConfInt{i, min, max}, (uint8_t)0xFF};
}

Config Config::Uint(uint32_t u, uint32_t min, uint32_t max)
{
    return Config{ConfUint{u, min, max}, (uint8_t)0xFF};
}

Config Config::Bool(bool b)
{
    return Config{ConfBool{b}, (uint8_t)0xFF};
}

Config Config::Array(std::initializer_list<Config> arr, Config *prototype, size_t minElements, size_t maxElements, int variantType)
{
    return Config{ConfArray{arr, prototype, minElements, maxElements, (int8_t)variantType}, (uint8_t)0xFF};
}

Config Config::Object(std::initializer_list<std::pair<String, Config>> obj)
{
    return Config{ConfObject{obj}, (uint8_t)0xFF};
}

Config Config::Null()
{
    return Config{nullptr, (uint8_t)0xFF};
}

Config Config::Uint8(uint8_t u)
{
    return Config::Uint(u, std::numeric_limits<uint8_t>::lowest(), std::numeric_limits<uint8_t>::max());
}

Config Config::Uint16(uint16_t u)
{
    return Config::Uint(u, std::numeric_limits<uint16_t>::lowest(), std::numeric_limits<uint16_t>::max());
}

Config Config::Uint32(uint32_t u)
{
    return Config::Uint(u, std::numeric_limits<uint32_t>::lowest(), std::numeric_limits<uint32_t>::max());
}

Config Config::Int8(int8_t i)
{
    return Config::Int(i, std::numeric_limits<int8_t>::lowest(), std::numeric_limits<int8_t>::max());
}

Config Config::Int16(int16_t i)
{
    return Config::Int(i, std::numeric_limits<int16_t>::lowest(), std::numeric_limits<int16_t>::max());
}

Config Config::Int32(int32_t i)
{
    return Config::Int(i, std::numeric_limits<int32_t>::lowest(), std::numeric_limits<int32_t>::max());
}

Config *Config::get(String s)
{
    if (!this->is<Config::ConfObject>()) {
        logger.printfln("Config key %s not in this node: is not an object!", s.c_str());
        delay(100);
        return nullptr;
    }

    return strict_variant::get<Config::ConfObject>(&value)->get(s);
}

Config *Config::get(uint16_t i)
{
    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Config index %u not in this node: is not an array!", i);
        delay(100);
        return nullptr;
    }

    return strict_variant::get<Config::ConfArray>(&value)->get(i);
}

const Config *Config::get(String s) const
{
    if (!this->is<Config::ConfObject>()) {
        logger.printfln("Config key %s not in this node: is not an object!", s.c_str());
        delay(100);
        return nullptr;
    }

    return strict_variant::get<Config::ConfObject>(&value)->get(s);
}

const Config *Config::get(uint16_t i) const
{
    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Config index %u not in this node: is not an array!", i);
        delay(100);
        return nullptr;
    }

    return strict_variant::get<Config::ConfArray>(&value)->get(i);
}

const String &Config::asString() const
{
    return *as<String, Config::ConfString>();
}

const char *Config::asCStr() const
{
    return as<String, Config::ConfString>()->c_str();
}

const float &Config::asFloat() const
{
    return *as<float, Config::ConfFloat>();
}

const uint32_t &Config::asUint() const
{
    return *as<uint32_t, Config::ConfUint>();
}

const int32_t &Config::asInt() const
{
    return *as<int32_t, Config::ConfInt>();
}

const bool &Config::asBool() const
{
    return *as<bool, Config::ConfBool>();
}

std::vector<Config> &Config::asArray()
{
    return *as<std::vector<Config>, Config::ConfArray>();
}

size_t Config::fillFloatArray(float *arr, size_t elements)
{
    return fillArray<float, Config::ConfFloat>(arr, elements);
}

size_t Config::fillUint8Array(uint8_t *arr, size_t elements)
{
    return fillArray<uint8_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt8Array(int8_t *arr, size_t elements)
{
    return fillArray<int8_t, Config::ConfInt>(arr, elements);
}

size_t Config::fillUint16Array(uint16_t *arr, size_t elements)
{
    return fillArray<uint16_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt16Array(int16_t *arr, size_t elements)
{
    return fillArray<int16_t, Config::ConfInt>(arr, elements);
}

size_t Config::fillUint32Array(uint32_t *arr, size_t elements)
{
    return fillArray<uint32_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt32Array(int32_t *arr, size_t elements)
{
    return fillArray<int32_t, Config::ConfInt>(arr, elements);
}

size_t Config::fillUint64Array(uint32_t *arr, size_t elements)
{
    return fillArray<uint32_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt64Array(int32_t *arr, size_t elements)
{
    return fillArray<int32_t, Config::ConfInt>(arr, elements);
}

size_t Config::json_size(bool zero_copy) const
{
    return strict_variant::apply_visitor(json_length_visitor{zero_copy}, value);
}

size_t Config::max_string_length() const
{
    return strict_variant::apply_visitor(string_length_visitor{}, value);
}

void Config::save_to_file(File file)
{
    DynamicJsonDocument doc(json_size(false));

    JsonVariant var;
    if (is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if (is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, {}}, value);

    serializeJson(doc, file);
}

void Config::write_to_stream(Print &output)
{
    DynamicJsonDocument doc(json_size(false));

    JsonVariant var;
    if (is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if (is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, {}}, value);
    serializeJson(doc, output);
}

String Config::to_string() const
{
    return this->to_string_except({});
}

String Config::to_string_except(std::initializer_list<String> keys_to_censor) const
{
    DynamicJsonDocument doc(json_size(false));

    JsonVariant var;
    if (is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if (is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, keys_to_censor}, value);

    String result;
    serializeJson(doc, result);
    return result;
}

String Config::to_string_except(const std::vector<String> &keys_to_censor) const
{
    DynamicJsonDocument doc(json_size(false));

    JsonVariant var;
    if (is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if (is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, keys_to_censor}, value);

    String result;
    serializeJson(doc, result);
    return result;
}

void Config::write_to_stream_except(Print &output, std::initializer_list<String> keys_to_censor)
{
    DynamicJsonDocument doc(json_size(false));

    JsonVariant var;
    if (is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if (is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, keys_to_censor}, value);

    serializeJson(doc, output);
}

void Config::write_to_stream_except(Print &output, const std::vector<String> &keys_to_censor)
{
    DynamicJsonDocument doc(json_size(false));

    JsonVariant var;
    if (is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if (is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, keys_to_censor}, value);

    serializeJson(doc, output);
}

bool Config::was_updated(uint8_t api_backend_flag)
{
    return ((updated & api_backend_flag) != 0) || strict_variant::apply_visitor(is_updated{api_backend_flag}, value);
}

void Config::set_update_handled(uint8_t api_backend_flag)
{
    updated = false;
    strict_variant::apply_visitor(set_updated_false{api_backend_flag}, value);
}

Config *Config::ConfObject::get(String s)
{
    for (size_t i = 0; i < this->value.size(); ++i) {
        if (this->value[i].first == s)
            return &this->value[i].second;
    }

    logger.printfln("Config key %s not found!", s.c_str());
    delay(100);
    return nullptr;
}

Config *Config::ConfArray::get(uint16_t i)
{
    if (i >= this->value.size()) {
        logger.printfln("Config index %u out of range!", i);
        delay(100);
        return nullptr;
    }
    return &this->value[i];
}

const Config *Config::ConfObject::get(String s) const
{
    for (size_t i = 0; i < this->value.size(); ++i) {
        if (this->value[i].first == s)
            return &this->value[i].second;
    }

    logger.printfln("Config key %s not found!", s.c_str());
    delay(100);
    return nullptr;
}

const Config *Config::ConfArray::get(uint16_t i) const
{
    if (i >= this->value.size()) {
        logger.printfln("Config index %u out of range!", i);
        delay(100);
        return nullptr;
    }
    return &this->value[i];
}

String ConfigRoot::update_from_file(File file)
{
    DynamicJsonDocument doc(this->json_size(false));
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        return String("Failed to read file: ") + String(error.c_str());

    return this->update_from_json(doc.as<JsonVariant>());
}

String ConfigRoot::update_from_cstr(char *c, size_t len)
{
    DynamicJsonDocument doc(this->json_size(true));
    DeserializationError error = deserializeJson(doc, c, len);

    if (error) {
        return String("Failed to deserialize string: ") + String(error.c_str());
    }

    return this->update_from_json(doc.as<JsonVariant>());
}

String ConfigRoot::update_from_string(String s)
{
    DynamicJsonDocument doc(this->json_size(false));
    DeserializationError error = deserializeJson(doc, s);

    if (error) {
        return String("Failed to deserialize string: ") + String(error.c_str());
    }

    return this->update_from_json(doc.as<JsonVariant>());
}

String ConfigRoot::update_from_json(JsonVariant root)
{
    Config copy = *this;
    String err = strict_variant::apply_visitor(from_json{root, !this->permit_null_updates, this->permit_null_updates, true}, copy.value);

    if (err != "")
        return err;

    err = strict_variant::apply_visitor(default_validator{}, copy.value);

    if (err != "")
        return err;

    if (this->validator != nullptr) {
        err = this->validator(copy);
        if (err != "")
            return err;
    }

    this->value = copy.value;
    this->updated = true;

    return err;
}

String ConfigRoot::update(Config::ConfUpdate *val)
{
    Config copy = *this;
    String err = strict_variant::apply_visitor(from_update{val}, copy.value);
    if (err != "")
        return err;

    err = strict_variant::apply_visitor(default_validator{}, copy.value);

    if (err != "")
        return err;

    if (this->validator != nullptr) {
        err = this->validator(copy);
        if (err != "")
            return err;
    }

    this->value = copy.value;
    this->updated = true;

    return err;
}

String ConfigRoot::validate()
{
    if (this->validator != nullptr) {
        return this->validator(*this);
    }
    return "";
}
