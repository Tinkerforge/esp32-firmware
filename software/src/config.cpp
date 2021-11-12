/* esp32-lib
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

struct printer {
  void operator()(const Config::ConfString &x) const { Serial.println("string"); }
  void operator()(const Config::ConfFloat &x) const { Serial.println("float"); }
  void operator()(const Config::ConfInt &x) const { Serial.println("int32_t"); }
  void operator()(const Config::ConfUint &x) const { Serial.println("uint32_t"); }
  void operator()(const Config::ConfBool &x) const { Serial.println("bool"); }
  void operator()(std::nullptr_t x) const { Serial.println("std::nullptr_t"); }
  void operator()(const Config::ConfArray &x) const {
      Serial.println("Array: ");
      for(const Config &c : x.value) {strict_variant::apply_visitor(printer{}, c.value);}
  }
  void operator()(const Config::ConfObject &x) const {
      Serial.println("Object: ");
      for(const std::pair<String, Config> &c : x.value) {Serial.print(c.first + ": "); strict_variant::apply_visitor(printer{}, c.second.value);}
  }
};

struct recursive_validator {
    bool operator()(Config::ConfString &x) { return x.validator(x); }
    bool operator()(Config::ConfFloat &x) { return x.validator(x); }
    bool operator()(Config::ConfInt &x) { return x.validator(x); }
    bool operator()(Config::ConfUint &x) { return x.validator(x); }
    bool operator()(Config::ConfBool &x) { return x.validator(x); }
    bool operator()(std::nullptr_t x) { return true; }
    bool operator()(Config::ConfArray &x) {
        // Intentionally do the recursive validation first.
        // This ensures, that the array's validator may assume, that its
        // entries itself are valid. Then only dependencies between (valid) entries
        // have to be validated.
        for(Config &elem : x.value)
            if(!strict_variant::apply_visitor(recursive_validator{}, elem.value))
                return false;

        if(!x.validator(x))
            return false;

        return true;
    }
    bool operator()(Config::ConfObject &x) {
        // Intentionally do the recursive validation first.
        // This ensures, that the object's validator may assume, that its
        // entries itself are valid. Then only dependencies between (valid) entries
        // have to be validated.
        for(std::pair<String, Config> &elem : x.value)
            if(!strict_variant::apply_visitor(recursive_validator{}, elem.second.value))
                return false;

        if(!x.validator(x))
            return false;

        return true;
    }
};

struct to_json {
    void operator()(Config::ConfString &x) {
        insertHere.set(x.value);
    }
    void operator()(Config::ConfFloat &x) {
        insertHere.set(x.value);
    }
    void operator()(Config::ConfInt &x) {
        insertHere.set(x.value);
    }
    void operator()(Config::ConfUint &x) {
        insertHere.set(x.value);
    }
    void operator()(Config::ConfBool &x) {
        insertHere.set(x.value);
    }
    void operator()(std::nullptr_t x) {
        insertHere.set(x);
    }
    void operator()(Config::ConfArray &x) {
        JsonArray arr = insertHere.as<JsonArray>();
        for(size_t i = 0; i < x.value.size(); ++i) {
            Config &child = x.value[i];

            if(child.is<Config::ConfObject>()) {
                arr.createNestedObject();
            } else if(child.is<Config::ConfArray>()) {
                arr.createNestedArray();
            } else {
                arr.add(0);
            }

            strict_variant::apply_visitor(to_json{arr[i], keys_to_censor}, x.value[i].value);
        }
    }
    void operator()(Config::ConfObject &x) {
        JsonObject obj = insertHere.as<JsonObject>();
        for(size_t i = 0; i < x.value.size(); ++i) {
            String &key = x.value[i].first;
            Config &child = x.value[i].second;

            if(child.is<Config::ConfObject>()) {
                obj.createNestedObject(key);
            } else if(child.is<Config::ConfArray>()) {
                obj.createNestedArray(key);
            } else {
                obj.getOrAddMember(key);
            }

            strict_variant::apply_visitor(to_json{obj[key], keys_to_censor}, child.value);
        }

        for(const String &key : keys_to_censor)
            if (obj.containsKey(key))
                obj[key] = nullptr;
    }

    JsonVariant insertHere;
    const std::vector<String> &keys_to_censor;
};

struct json_length_visitor {
    size_t operator()(Config::ConfString &x) {
        return x.maxChars + 1;
    }
    size_t operator()(Config::ConfFloat &x) {
        return 0;
    }
    size_t operator()(Config::ConfInt &x) {
        return 0;
    }
    size_t operator()(Config::ConfUint &x) {
        return 0;
    }
    size_t operator()(Config::ConfBool &x) {
        return 0;
    }
    size_t operator()(std::nullptr_t x) {
        return 10; //TODO: is this still necessary?
    }
    size_t operator()(Config::ConfArray &x) {
        return strict_variant::apply_visitor(json_length_visitor{}, x.prototype[0].value) * x.maxElements + JSON_ARRAY_SIZE(x.maxElements);
    }
    size_t operator()(Config::ConfObject &x) {
        size_t sum = 0;
        for(size_t i = 0; i < x.value.size(); ++i) {
            sum += x.value[i].first.length() + 1;
            size_t item_size = strict_variant::apply_visitor(json_length_visitor{}, x.value[i].second.value);
            // If the item size is 0 it is not an array or object.
            // It will fit into the variant size added below.
            if (item_size > 0)
                sum += item_size;
        }
        return sum + JSON_OBJECT_SIZE(x.value.size());
    }
};

struct from_json {
    String operator()(Config::ConfString &x) {
        if(json_node.isNull())
            return x.validator(x);

        if(!json_node.is<String>())
            return "JSON node was not a string.";
        x.value = json_node.as<String>();
        return x.validator(x);
    }
    String operator()(Config::ConfFloat &x) {
        if(json_node.isNull())
            return x.validator(x);

        if(!json_node.is<float>())
            return "JSON node was not a float.";

        // We don't allow setting float config values from an integer
        // to make sure, no additional rounding occurres.
        // To set a float config value to an integer value, for example 123.0 has to be used.
        if(json_node.is<uint32_t>() || json_node.is<int32_t>())
            return "JSON node was an integer. Please use f.e. 123.0 to set a float node to an integer value.";

        x.value = json_node.as<float>();
        return x.validator(x);
    }
    String operator()(Config::ConfInt &x) {
        if(json_node.isNull())
            return x.validator(x);

        if(!json_node.is<int32_t>())
            return "JSON node was not a signed integer.";
        x.value = json_node.as<int32_t>();
        return x.validator(x);
    }
    String operator()(Config::ConfUint &x) {
        if(json_node.isNull())
            return x.validator(x);

        if(!json_node.is<uint32_t>())
            return "JSON node was not an unsigned integer.";
        x.value = json_node.as<uint32_t>();
        return x.validator(x);
    }
    String operator()(Config::ConfBool &x) {
        if(json_node.isNull())
            return x.validator(x);

        if(!json_node.is<bool>())
            return "JSON node was not a boolean.";
        x.value = json_node.as<bool>();
        return x.validator(x);
    }
    String operator()(std::nullptr_t x) {
        return json_node.isNull() ? "" : "JSON null node was not null";
    }
    String operator()(Config::ConfArray &x) {
        if(json_node.isNull())
            return x.validator(x);

        if(!json_node.is<JsonArray>())
            return "JSON node was not an array.";

        JsonArray arr = json_node.as<JsonArray>();

        x.value.clear();
        for(size_t i = 0; i < arr.size(); ++i) {
            x.value.push_back(x.prototype[0]);
            String inner_error = strict_variant::apply_visitor(from_json{arr[i], force_same_keys}, x.value[i].value);
            if(inner_error != "")
                return String("[") + i + "]" + inner_error;
        }

        return x.validator(x);
    }
    String operator()(Config::ConfObject &x) {
        if(json_node.isNull())
            return x.validator(x);

        if(!json_node.is<JsonObject>())
            return "JSON node was not an object.";

        JsonObject obj = json_node.as<JsonObject>();

        if(force_same_keys && obj.size() != x.value.size())
            return String("JSON object had ") + obj.size() + " entries instead of the expected " + x.value.size();

        for(size_t i = 0; i < x.value.size(); ++i) {
            if (!force_same_keys && !obj.containsKey(x.value[i].first))
                continue;

            String inner_error = strict_variant::apply_visitor(from_json{obj[x.value[i].first], force_same_keys}, x.value[i].second.value);
            if(inner_error != "")
                return String("[\"") + x.value[i].first + "\"]" + inner_error;
        }

        return x.validator(x);
    }

    JsonVariant json_node;
    bool force_same_keys;
};


struct from_update {
    String operator()(Config::ConfString &x) {
        if(Config::containsNull(update))
            return x.validator(x);

        if(update->get<String>() == nullptr)
            return "ConfUpdate node was not a string.";
        x.value = *(update->get<String>());
        return x.validator(x);
    }
    String operator()(Config::ConfFloat &x) {
        if(Config::containsNull(update))
            return x.validator(x);

        if(update->get<float>() == nullptr)
            return "ConfUpdate node was not a float.";

        // We don't allow setting float config values from an integer
        // to make sure, no additional rounding occurres.
        // To set a float config value to an integer value, for example 123.0 has to be used.
        if(update->get<uint32_t>() != nullptr || update->get<int32_t>() != nullptr)
            return "ConfUpdate node was an integer. Please use f.e. 123.0 to set a float node to an integer value.";

        x.value = *(update->get<float>());
        return x.validator(x);
    }
    String operator()(Config::ConfInt &x) {
        if(Config::containsNull(update))
            return x.validator(x);

        if(update->get<int32_t>() == nullptr)
            return "ConfUpdate node was not a signed integer.";
        x.value = *(update->get<int32_t>());
        return x.validator(x);
    }
    String operator()(Config::ConfUint &x) {
        if(Config::containsNull(update))
            return x.validator(x);

        uint32_t new_val = 0;
        if(update->get<uint32_t>() == nullptr) {
            if(update->get<int32_t>() == nullptr || update->get<int32_t>() < 0)
                return "ConfUpdate node was not an unsigned integer.";

            new_val = (uint32_t) *(update->get<int32_t>());
        } else {
            new_val = *(update->get<uint32_t>());
        }
        x.value = new_val;
        return x.validator(x);
    }
    String operator()(Config::ConfBool &x) {
        if(Config::containsNull(update))
            return x.validator(x);

        if(update->get<bool>() == nullptr)
            return "ConfUpdate node was not a boolean.";
        x.value = *(update->get<bool>());
        return x.validator(x);
    }
    String operator()(std::nullptr_t x) {
        return Config::containsNull(update) ? "" : "JSON null node was not null";
    }
    String operator()(Config::ConfArray &x) {
        if(Config::containsNull(update))
            return x.validator(x);

        if(update->get<Config::ConfUpdateArray>() == nullptr)
            return "ConfUpdate node was not an array.";

        Config::ConfUpdateArray *arr = update->get<Config::ConfUpdateArray>();

        x.value.clear();
        for(size_t i = 0; i < arr->elements.size(); ++i) {
            x.value.push_back(x.prototype[0]);
            String inner_error = strict_variant::apply_visitor(from_update{&arr->elements[i]}, x.value[i].value);
            if(inner_error != "")
                return String("[") + i + "]" + inner_error;
        }

        return x.validator(x);
    }
    String operator()(Config::ConfObject &x) {
        if(Config::containsNull(update))
            return x.validator(x);

        if(update->get<Config::ConfUpdateObject>() == nullptr) {
            Serial.println(update->which());
            return "ConfUpdate node was not an object.";
        }

        Config::ConfUpdateObject *obj = update->get<Config::ConfUpdateObject>();

        if(obj->elements.size() != x.value.size())
            return String("ConfUpdate object had ") + obj->elements.size() + " entries instead of the expected " + x.value.size();

        for(size_t i = 0; i < x.value.size(); ++i) {
            size_t obj_idx = 0xFFFFFFFF;
            for(size_t j = 0; j < x.value.size(); ++j) {
                if(obj->elements[j].first != x.value[i].first)
                    continue;
                obj_idx = j;
                break;
            }
            if(obj_idx == 0xFFFFFFFF)
                return String("Key ") + x.value[i].first + String("not found in ConfUpdate object");

            String inner_error = strict_variant::apply_visitor(from_update{&obj->elements[obj_idx].second}, x.value[i].second.value);
            if(inner_error != "")
                return String("[\"") + x.value[i].first + "\"]" + inner_error;
        }

        return x.validator(x);
    }

    Config::ConfUpdate *update;
};

struct is_updated {
    bool operator()(const Config::ConfString &x) {
        return false;
    }
    bool operator()(const Config::ConfFloat &x) {
        return false;
    }
    bool operator()(const Config::ConfInt &x) {
        return false;
    }
    bool operator()(const Config::ConfUint &x) {
        return false;
    }
    bool operator()(const Config::ConfBool &x) {
        return false;
    }
    bool operator()(const std::nullptr_t x) {
        return false;
    }
    bool operator()(const Config::ConfArray &x) const {
        for(const Config &c : x.value) {
            if(c.updated || strict_variant::apply_visitor(is_updated{}, c.value))
                return true;
        }
        return false;
    }
    bool operator()(const Config::ConfObject &x) const {
        for(const std::pair<String, Config> &c : x.value) {
            if(c.second.updated || strict_variant::apply_visitor(is_updated{}, c.second.value))
                return true;
        }
        return false;
    }
};

struct set_updated_false {
    void operator()(Config::ConfString &x) {}
    void operator()(Config::ConfFloat &x) {}
    void operator()(Config::ConfInt &x) {}
    void operator()(Config::ConfUint &x) {}
    void operator()(Config::ConfBool &x) {}
    void operator()(std::nullptr_t x) {}
    void operator()(Config::ConfArray &x) {
        for(Config &c : x.value) {
            c.updated = false;
            strict_variant::apply_visitor(set_updated_false{}, c.value);
        }
    }
    void operator()(Config::ConfObject &x) {
        for(std::pair<String, Config> &c : x.value) {
            c.second.updated = false;
            strict_variant::apply_visitor(set_updated_false{}, c.second.value);
        }
    }
};

Config Config::Str(String s,
                   size_t maxChars,
                   String(*validator)(ConfString &)) {
    return Config{ConfString{s, maxChars == 0 ? s.length() : maxChars, validator}, true};
}

Config Config::Float(float d,
                     float min,
                     float max,
                     String(*validator)(ConfFloat &)) {
    return Config{ConfFloat{d, min, max, validator}, true};
}

Config Config::Int(int32_t i,
                      int32_t min,
                      int32_t max,
                      String(*validator)(ConfInt &)) {
    return Config{ConfInt{i, min, max, validator}, true};
}

Config Config::Uint(uint32_t u,
                       uint32_t min,
                       uint32_t max,
                       String(*validator)(ConfUint &)) {
    return Config{ConfUint{u, min, max, validator}, true};
}

Config Config::Bool(bool b,
                       String(*validator)(ConfBool &)) {
    return Config{ConfBool{b, validator}, true};
}

Config Config::Array(std::initializer_list<Config> arr,
                        Config prototype,
                        size_t minElements,
                        size_t maxElements,
                        int variantType,
                        String(*validator)(ConfArray &)) {
    return Config{ConfArray{arr, {prototype}, minElements, maxElements, variantType, validator}, true};
}

Config Config::Object(std::initializer_list<std::pair<String, Config>> obj,
                         String(*validator)(ConfObject &)) {
    return Config{ConfObject{obj, validator}, true};
}

Config Config::Null() { return Config{nullptr, true}; }

Config Config::Uint8(uint8_t u) {
        return Config::Uint(u, std::numeric_limits<uint8_t>::lowest(), std::numeric_limits<uint8_t>::max());
    }

Config Config::Uint16(uint16_t u) {
        return Config::Uint(u, std::numeric_limits<uint16_t>::lowest(), std::numeric_limits<uint16_t>::max());
    }

Config Config::Uint32(uint32_t u) {
        return Config::Uint(u, std::numeric_limits<uint32_t>::lowest(), std::numeric_limits<uint32_t>::max());
    }

Config Config::Int8(int8_t i) {
        return Config::Int(i, std::numeric_limits<int8_t>::lowest(), std::numeric_limits<int8_t>::max());
    }

Config Config::Int16(int16_t i) {
        return Config::Int(i, std::numeric_limits<int16_t>::lowest(), std::numeric_limits<int16_t>::max());
    }

Config Config::Int32(int32_t i) {
    return Config::Int(i, std::numeric_limits<int32_t>::lowest(), std::numeric_limits<int32_t>::max());
}

Config *Config::get(String s) {
    if (!this->is<Config::ConfObject>()) {
        logger.printfln("Config key %s not in this node: is not an object!", s.c_str());
        delay(100);
        return nullptr;
    }

    return strict_variant::get<Config::ConfObject>(&value)->get(s);
}

Config *Config::get(size_t i) {
    if (!this->is<Config::ConfArray>()){
        logger.printfln("Config index %u not in this node: is not an array!", i);
        delay(100);
        return nullptr;
    }

    return strict_variant::get<Config::ConfArray>(&value)->get(i);
}

const Config *Config::get(String s) const {
    if (!this->is<Config::ConfObject>()) {
        logger.printfln("Config key %s not in this node: is not an object!", s.c_str());
        delay(100);
        return nullptr;
    }

    return strict_variant::get<Config::ConfObject>(&value)->get(s);
}

const Config *Config::get(size_t i) const {
    if (!this->is<Config::ConfArray>()){
        logger.printfln("Config index %u not in this node: is not an array!", i);
        delay(100);
        return nullptr;
    }

    return strict_variant::get<Config::ConfArray>(&value)->get(i);
}

const String &Config::asString() const {
    return *as<String, Config::ConfString>();
}

const char *Config::asCStr() const {
    return as<String, Config::ConfString>()->c_str();
}


const float &Config::asFloat() const {
    return *as<float, Config::ConfFloat>();
}

const uint32_t &Config::asUint() const {
    return *as<uint32_t, Config::ConfUint>();
}

const int32_t &Config::asInt() const {
    return *as<int32_t, Config::ConfInt>();
}

const bool &Config::asBool() const {
    return *as<bool, Config::ConfBool>();
}

std::vector<Config>& Config::asArray()
{
    return *as<std::vector<Config>, Config::ConfArray>();
}

size_t Config::fillFloatArray(float *arr, size_t elements) {
    return fillArray<float, Config::ConfFloat>(arr, elements);
}


size_t Config::fillUint8Array(uint8_t *arr, size_t elements) {
    return fillArray<uint8_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt8Array(int8_t *arr, size_t elements) {
    return fillArray<int8_t, Config::ConfInt>(arr, elements);
}


size_t Config::fillUint16Array(uint16_t *arr, size_t elements) {
    return fillArray<uint16_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt16Array(int16_t *arr, size_t elements) {
    return fillArray<int16_t, Config::ConfInt>(arr, elements);
}

size_t Config::fillUint32Array(uint32_t *arr, size_t elements) {
    return fillArray<uint32_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt32Array(int32_t *arr, size_t elements) {
    return fillArray<int32_t, Config::ConfInt>(arr, elements);
}

size_t Config::fillUint64Array(uint32_t *arr, size_t elements) {
    return fillArray<uint32_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt64Array(int32_t *arr, size_t elements) {
    return fillArray<int32_t, Config::ConfInt>(arr, elements);
}

size_t Config::json_size() {
    return strict_variant::apply_visitor(json_length_visitor{}, value);
}

String Config::update_from_file(File file) {
    ConfVariant copy = value;
    DynamicJsonDocument doc(json_size());
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        return String("Failed to read file: ") + String(error.c_str());

    String err = strict_variant::apply_visitor(from_json{doc.as<JsonVariant>(), false}, copy);

    if (err == "") {
        value = copy;
        this->updated = true;
    }



    return err;
}

String Config::update_from_cstr(char *c, size_t len) {
    ConfVariant copy = value;
    DynamicJsonDocument doc(json_size());
    DeserializationError error = deserializeJson(doc, c, len);

    if (error) {
        return String("Failed to deserialize string: ") + String(error.c_str());
    }

    String err = strict_variant::apply_visitor(from_json{doc.as<JsonVariant>(), true}, copy);

    if (err == "") {
        value = copy;
        this->updated = true;
    }

    return err;
}

String Config::update_from_string(String s) {
    ConfVariant copy = value;
    DynamicJsonDocument doc(json_size());
    DeserializationError error = deserializeJson(doc, s);

    if (error) {
        return String("Failed to deserialize string: ") + String(error.c_str());
    }

    String err = strict_variant::apply_visitor(from_json{doc.as<JsonVariant>(), true}, copy);

    if (err == "") {
        value = copy;
        this->updated = true;
    }

    return err;
}

String Config::update_from_json(JsonVariant root) {
    ConfVariant copy = value;
    String err = strict_variant::apply_visitor(from_json{root, true}, copy);
    if (err == "") {
        value = copy;
        this->updated = true;
    }

    return err;
}

String Config::update(ConfUpdate *val)
{
    ConfVariant copy = value;
    String err = strict_variant::apply_visitor(from_update{val}, copy);
    if (err == "") {
        value = copy;
        this->updated = true;
    }

    return err;
}

void Config::save_to_file(File file)
{
    DynamicJsonDocument doc(json_size());

    JsonVariant var;
    if(is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if(is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, {}}, value);

    serializeJson(doc, file);
}

void Config::write_to_stream(Print &output)
{
    DynamicJsonDocument doc(json_size());

    JsonVariant var;
    if(is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if(is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, {}}, value);
    serializeJson(doc, output);
}

String Config::to_string() {
    return this->to_string_except({});
}

String Config::to_string_except(std::initializer_list<String> keys_to_censor) {
    DynamicJsonDocument doc(json_size());

    JsonVariant var;
    if(is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if(is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, keys_to_censor}, value);

    String result;
    serializeJson(doc, result);
    return result;
}

String Config::to_string_except(const std::vector<String> &keys_to_censor) {
    DynamicJsonDocument doc(json_size());

    JsonVariant var;
    if(is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if(is<Config::ConfArray>()) {
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
    DynamicJsonDocument doc(json_size());

    JsonVariant var;
    if(is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if(is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, keys_to_censor}, value);

    serializeJson(doc, output);
}

void Config::write_to_stream_except(Print &output, const std::vector<String> &keys_to_censor)
{
    DynamicJsonDocument doc(json_size());

    JsonVariant var;
    if(is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if(is<Config::ConfArray>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var, keys_to_censor}, value);

    serializeJson(doc, output);
}

bool Config::isValid() {
    return strict_variant::apply_visitor(recursive_validator{}, value);
}

bool Config::was_updated() {
    return updated || strict_variant::apply_visitor(is_updated{}, value);
}

void Config::set_update_handled() {
    updated = false;
    strict_variant::apply_visitor(set_updated_false{}, value);
}

Config* Config::ConfObject::get(String s) {
    for(size_t i = 0; i < this->value.size(); ++i) {
        if(this->value[i].first == s)
            return &this->value[i].second;
    }

    logger.printfln("Config key %s not found!", s.c_str());
    delay(100);
    return nullptr;
}

Config* Config::ConfArray::get(size_t i)
{
    if(i >= this->value.size()) {
        logger.printfln("Config index %u out of range!", i);
        delay(100);
        return nullptr;
    }
    return &this->value[i];
}


const Config* Config::ConfObject::get(String s) const {
    for(size_t i = 0; i < this->value.size(); ++i) {
        if(this->value[i].first == s)
            return &this->value[i].second;
    }

    logger.printfln("Config key %s not found!", s.c_str());
    delay(100);
    return nullptr;
}

const Config* Config::ConfArray::get(size_t i) const {
    if(i >= this->value.size()) {
        logger.printfln("Config index %u out of range!", i);
        delay(100);
        return nullptr;
    }
    return &this->value[i];
}
