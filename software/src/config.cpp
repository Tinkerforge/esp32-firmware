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

#include <mutex>

#include "math.h"

#include "tools.h"

#define SLOT_HEADROOM 20

struct ConfStringSlot {
    CoolString val = "";
    uint16_t minChars = 0;
    uint16_t maxChars = 0;
    bool inUse = false;
};

struct ConfFloatSlot {
    float val = 0;
    float min = 0;
    float max = 0;
};

struct ConfIntSlot {
    int32_t val = 0;
    int32_t min = 0;
    int32_t max = 0;
};

struct ConfUintSlot {
    uint32_t val = 0;
    uint32_t min = 0;
    uint32_t max = 0;
};

struct ConfArraySlot {
    std::vector<Config> val;
    const Config *prototype;
    uint16_t minElements;
    uint16_t maxElements;
    int8_t variantType;
    bool inUse = false;
};

struct ConfObjectSlot {
    std::vector<std::pair<String, Config>> val;
    bool inUse = false;
};

struct ConfUnionSlot {
    uint8_t tag = 0;
    uint8_t prototypes_len = 0;
    Config val;
    const ConfUnionPrototype * prototypes = nullptr;
};

#define UINT_SLOTS 512
Config::ConfUint::Slot *uint_buf = nullptr;
size_t uint_buf_size = 0;

#define INT_SLOTS 128
Config::ConfInt::Slot *int_buf = nullptr;
size_t int_buf_size = 0;

#define FLOAT_SLOTS 384
Config::ConfFloat::Slot *float_buf = nullptr;
size_t float_buf_size = 0;

#define STRING_SLOTS 384
Config::ConfString::Slot *string_buf = nullptr;
size_t string_buf_size = 0;

#define ARRAY_SLOTS 32
Config::ConfArray::Slot *array_buf = nullptr;
size_t array_buf_size = 0;

#define OBJECT_SLOTS 256
Config::ConfObject::Slot *object_buf = nullptr;
size_t object_buf_size = 0;

#define UNION_SLOTS 32
Config::ConfUnion::Slot *union_buf = nullptr;
size_t union_buf_size = 0;

static ConfigRoot nullconf = Config{Config::ConfVariant{}};


struct default_validator {
    String operator()(const Config::ConfString &x) const
    {
        const auto *slot = x.getSlot();
        const auto length = x.getVal()->length();

        if (length < slot->minChars)
            return String("String of minimum length ") + slot->minChars + " was expected, but got " + length;

        if (slot->maxChars == 0 || length <= slot->maxChars)
            return "";

        return String("String of maximum length ") + slot->maxChars + " was expected, but got " + length;
    }

    String operator()(const Config::ConfFloat &x) const
    {
        const auto *slot = x.getSlot();
        const auto val = *x.getVal();

        if (val < slot->min)
            return String("Float value ") + val + " was less than the allowed minimum of " + slot->min;
        if (val > slot->max)
            return String("Float value ") + val + " was more than the allowed maximum of " + slot->max;
        return "";
    }

    String operator()(const Config::ConfInt &x) const
    {
        const auto *slot = x.getSlot();
        const auto val = *x.getVal();

        if (val < slot->min)
            return String("Integer value ") + val + " was less than the allowed minimum of " + slot->min;
        if (val > slot->max)
            return String("Integer value ") + val + " was more than the allowed maximum of " + slot->max;
        return "";
    }

    String operator()(const Config::ConfUint &x) const
    {
        const auto *slot = x.getSlot();
        const auto val = *x.getVal();

        if (val < slot->min)
            return String("Unsigned integer value ") + val + " was less than the allowed minimum of " + slot->min;
        if (val > slot->max)
            return String("Unsigned integer value ") + val + " was more than the allowed maximum of " + slot->max;
        return "";
    }

    String operator()(const Config::ConfBool &x) const
    {
        return "";
    }
    String operator()(const Config::ConfVariant::Empty &x)
    {
        return "";
    }

    String operator()(const Config::ConfArray &x) const
    {
        const auto *slot = x.getSlot();
        const auto *val = x.getVal();
        const auto size = val->size();

        if (slot->maxElements > 0 && size > slot->maxElements)
            return String("Array had ") + size + " entries, but only " + slot->maxElements + " are allowed.";
        if (slot->minElements > 0 && size < slot->minElements)
            return String("Array had ") + size + " entries, but at least " + slot->maxElements + " are required.";

        if (slot->variantType >= 0)
            for (int i = 0; i < size; ++i)
                if ((int)x.get(i)->value.tag != slot->variantType)
                    return String("[") + i + "] has wrong type";

        size_t i = 0;
        for (const Config &elem : *val) {
            String err = Config::apply_visitor(default_validator{}, elem.value);
            if (err != "")
                return String("[") + i + "] " + err;
            ++i;
        }

        return "";
    }

    String operator()(const Config::ConfObject &x) const
    {
        for (const std::pair<String, Config> &elem : *x.getVal()) {
            String err = Config::apply_visitor(default_validator{}, elem.second.value);

            if (err != "")
                return String("[\"") + elem.first.c_str() + "\"] " + err;
        }

        return "";
    }

    String operator()(const Config::ConfUnion &x) const {
        // Tag match is already checked in from_json.

        return Config::apply_visitor(default_validator{}, x.getVal()->value);
    }
};

struct to_json {
    void operator()(const Config::ConfString &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfFloat &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfInt &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfUint &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfBool &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfVariant::Empty &x)
    {
        insertHere.set(nullptr);
    }
    void operator()(const Config::ConfArray &x)
    {
        const auto *val = x.getVal();
        const auto size = val->size();

        JsonArray arr = insertHere.as<JsonArray>();
        for (size_t i = 0; i < size; ++i) {
            const Config *child = &(*val)[i];

            if (child->is<Config::ConfObject>()) {
                arr.createNestedObject();
            } else if (child->is<Config::ConfArray>() || child->is<Config::ConfUnion>()) {
                arr.createNestedArray();
            } else {
                arr.add(0);
            }

            Config::apply_visitor(to_json{arr[i], keys_to_censor}, child->value);
        }
    }
    void operator()(const Config::ConfObject &x)
    {
        const auto *val = x.getVal();
        const auto size = val->size();

        JsonObject obj = insertHere.as<JsonObject>();
        for (size_t i = 0; i < size; ++i) {
            const auto &val_pair = (*val)[i];
            const String &key = val_pair.first;
            const Config &child = val_pair.second;

            if (child.is<Config::ConfObject>()) {
                obj.createNestedObject(key);
            } else if (child.is<Config::ConfArray>() || child.is<Config::ConfUnion>()) {
                obj.createNestedArray(key);
            } else if (!obj.containsKey(key)) {
                obj[key] = nullptr;
            }

            Config::apply_visitor(to_json{obj[key], keys_to_censor}, child.value);
        }

        for (const String &key : keys_to_censor)
            if (obj.containsKey(key) && !(obj[key].is<String>() && obj[key].as<String>().length() == 0))
                obj[key] = nullptr;
    }

    void operator()(const Config::ConfUnion &x) {
        const auto *val = x.getVal();

        JsonArray arr = insertHere.as<JsonArray>();
        arr.add(x.getSlot()->tag);
        if (val->is<Config::ConfObject>()) {
            arr.createNestedObject();
        } else if (val->is<Config::ConfArray>() || val->is<Config::ConfUnion>()) {
            arr.createNestedArray();
        } else {
            arr.add();
        }

        Config::apply_visitor(to_json{insertHere[1], keys_to_censor}, val->value);
    }

    JsonVariant insertHere;
    const std::vector<String> &keys_to_censor;
};

struct string_length_visitor {
    size_t operator()(const Config::ConfString &x)
    {
        return (x.getSlot()->maxChars) + 2; // ""
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
    size_t operator()(const Config::ConfVariant::Empty &x)
    {
        return 4;
    }
    size_t operator()(const Config::ConfArray &x)
    {
        const auto *slot = x.getSlot();

        return Config::apply_visitor(string_length_visitor{}, slot->prototype->value) * slot->maxElements +
               (slot->maxElements + 1); // [,] and n-1 ,
    }
    size_t operator()(const Config::ConfObject &x)
    {
        const auto *val = x.getVal();
        const auto size = val->size();

        size_t sum = 2; // { and }
        for (size_t i = 0; i < size; ++i) {
            const auto &val_pair = (*val)[i];
            sum += val_pair.first.length() + 2; // ""
            sum += Config::apply_visitor(string_length_visitor{}, val_pair.second.value);
        }
        return sum;
    }

    size_t operator()(const Config::ConfUnion &x) {
        const auto *slot = x.getSlot();

        size_t max_len = Config::apply_visitor(string_length_visitor{}, x.getVal()->value);
        for (size_t i = 0; i < slot->prototypes_len; ++i) {
            max_len = std::max(max_len, Config::apply_visitor(string_length_visitor{}, slot->prototypes[i].config.value));
        }
        return max_len + 6; // [255,]
    }
};

struct json_length_visitor {
    size_t operator()(const Config::ConfString &x)
    {
        return zero_copy ? 0 : ((x.getSlot()->maxChars) + 1);
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
    size_t operator()(const Config::ConfVariant::Empty &x)
    {
        return 10; // TODO: is this still necessary?
    }
    size_t operator()(const Config::ConfArray &x)
    {
        const auto *slot = x.getSlot();

        return Config::apply_visitor(json_length_visitor{zero_copy}, slot->prototype->value) * slot->maxElements + JSON_ARRAY_SIZE(slot->maxElements);
    }
    size_t operator()(const Config::ConfObject &x)
    {
        const auto *val = x.getVal();
        const auto size = val->size();

        size_t sum = 0;
        for (size_t i = 0; i < size; ++i) {
            const auto &val_pair = (*val)[i];
            if (!zero_copy)
                sum += val_pair.first.length() + 1;

            sum += Config::apply_visitor(json_length_visitor{zero_copy}, val_pair.second.value);
        }
        return sum + JSON_OBJECT_SIZE(size);
    }

    size_t operator()(const Config::ConfUnion &x) {
        const auto *slot = x.getSlot();

        size_t max_len = Config::apply_visitor(json_length_visitor{zero_copy}, x.getVal()->value);
        for (size_t i = 0; i < slot->prototypes_len; ++i) {
            max_len = std::max(max_len, Config::apply_visitor(json_length_visitor{zero_copy}, slot->prototypes[i].config.value));
        }
        return max_len + JSON_ARRAY_SIZE(2);
    }

    bool zero_copy;
};

struct from_json {
    String operator()(Config::ConfString &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? "" : "Null updates not permitted.";

        if (!json_node.is<String>())
            return "JSON node was not a string.";
        *x.getVal() = json_node.as<CoolString>();
        return "";
    }
    String operator()(Config::ConfFloat &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? "" : "Null updates not permitted.";

        if (!json_node.is<float>())
            return "JSON node was not a float.";

        *x.getVal() = json_node.as<float>();
        return "";
    }
    String operator()(Config::ConfInt &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? "" : "Null updates not permitted.";

        if (!json_node.is<int32_t>())
            return "JSON node was not a signed integer.";
        *x.getVal() = json_node.as<int32_t>();
        return "";
    }
    String operator()(Config::ConfUint &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? "" : "Null updates not permitted.";

        if (!json_node.is<uint32_t>())
            return "JSON node was not an unsigned integer.";
        *x.getVal() = json_node.as<uint32_t>();
        return "";
    }
    String operator()(Config::ConfBool &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? "" : "Null updates not permitted.";

        if (!json_node.is<bool>())
            return "JSON node was not a boolean.";
        x.value = json_node.as<bool>();
        return "";
    }
    String operator()(const Config::ConfVariant::Empty &x)
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
            return permit_null_updates ? "" : "Null updates not permitted.";

        if (!json_node.is<JsonArray>())
            return "JSON node was not an array.";

        JsonArray arr = json_node.as<JsonArray>();
        auto arr_size = arr.size();

        auto *val = x.getVal();
        const auto old_size = val->size();

        if (arr_size != old_size) {
            if (arr_size < old_size) {
                // resize() to smaller value truncates vector.
                val->resize(arr_size);
                if (arr_size < (val->capacity() / 2))
                    val->shrink_to_fit();
            } else {
                // Cannot use resize() to enlarge the vector because the new elements wouldn't be copies of the prototype.
                val->reserve(arr_size);
                const auto *prototype = as_const(x).getSlot()->prototype;
                for (size_t i = old_size; i < arr_size; ++i) {
                    val->push_back(*prototype);

                    // Must get val again because the push_back() consumes a slot, which might trigger a slot array move that invalidates the pointer.
                    val = x.getVal();
                }
            }
        }

        for (size_t i = 0; i < arr_size; ++i) {
            String inner_error = Config::apply_visitor(from_json{arr[i], force_same_keys, permit_null_updates, false}, (*val)[i].value);
            if (inner_error != "")
                return String("[") + i + "] " + inner_error;
        }

        return "";
    }
    String operator()(Config::ConfObject &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? "" : "Null updates not permitted.";

        auto *val = x.getVal();
        const auto size = val->size();

        // If a user passes a non-object to an API that expects an object with exactly one member
        // Try to use the non-object as value for the single member.
        // This allows calling for example evse/external_current_update with the payload 8000 instead of {"current": 8000}
        if (!json_node.is<JsonObject>() && is_root && size == 1) {
            auto &val_pair = (*val)[0];
            String inner_error = Config::apply_visitor(from_json{json_node, force_same_keys, permit_null_updates, false}, val_pair.second.value);
            if (inner_error != "")
                return String("(inferred) [\"") + val_pair.first + "\"] " + inner_error + "\n";
            else
                return inner_error;
        }

        if (!json_node.is<JsonObject>())
            return "JSON node was not an object.";

        const JsonObject obj = json_node.as<JsonObject>();

        String return_str = "";
        bool more_errors = false;

        for (size_t i = 0; i < size; ++i) {
            auto &val_pair = (*val)[i];
            if (!obj.containsKey(val_pair.first))
            {
                if (!force_same_keys)
                    continue;

                if (return_str.length() < 1000)
                    return_str += String("JSON object is missing key '") + val_pair.first + "'\n";
                else
                    more_errors = true;
            }

            String inner_error = Config::apply_visitor(from_json{obj[val_pair.first], force_same_keys, permit_null_updates, false}, val_pair.second.value);
            if(obj.size() > 0)
                obj.remove(val_pair.first);
            if (inner_error != "")
            {
                if (return_str.length() < 1000)
                    return_str += String("[\"") + val_pair.first + "\"] " + inner_error + "\n";
                else
                    more_errors = true;
            }

        }

        if (force_same_keys) {
            for (auto i = obj.begin(); i != obj.end(); i += 1)
            {
                if (return_str.length() < 1000)
                    return_str += String("JSON object has unknown key '") + i->key().c_str() + "'.\n";
                else
                    more_errors = true;
            }
        }

        if (return_str.length() > 0)
        {
            if (more_errors)
                return_str += "More errors occured that got filtered out.\n";
            return return_str;
        }

        return "";
    }

    String operator()(Config::ConfUnion &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? "" : "Null updates not permitted.";

        if (!json_node.is<JsonArray>())
            return "JSON node was not an array.";

        JsonArray arr = json_node.as<JsonArray>();

        if (arr.size() != 2) {
            return "JSON array had length != 2.";
        }

        uint8_t old_tag = x.getTag();
        uint8_t new_tag = old_tag;

        const auto &arr_first = arr[0];
        if (arr_first.isNull()) {
            if (!permit_null_updates)
                return "[0] Null updates not permitted";
        }
        else if (arr_first.is<uint8_t>()) {
            new_tag = arr_first.as<uint8_t>();
        } else {
            return "[0] JSON node was not an unsigned integer.";
        }


        if (new_tag != old_tag) {
            if (!x.changeUnionVariant(new_tag))
                return String("[0] Unknown union tag: ") + new_tag;
        }

        return Config::apply_visitor(from_json{arr[1], force_same_keys, permit_null_updates, false}, x.getVal()->value);
    }

    const JsonVariant json_node;
    bool force_same_keys;
    bool permit_null_updates;
    bool is_root;
};

struct from_update {
    String operator()(Config::ConfString &x)
    {
        if (Config::containsNull(update))
            return "";

        const auto *update_val = update->get<CoolString>();
        if (update_val == nullptr)
            return "ConfUpdate node was not a string.";
        *x.getVal() = *update_val;
        return "";
    }
    String operator()(Config::ConfFloat &x)
    {
        if (Config::containsNull(update))
            return "";

        const auto *update_val = update->get<float>();
        if (update_val == nullptr)
            return "ConfUpdate node was not a float.";

        *x.getVal() = *update_val;
        return "";
    }
    String operator()(Config::ConfInt &x)
    {
        if (Config::containsNull(update))
            return "";

        const auto *update_val = update->get<int32_t>();
        if (update_val == nullptr)
            return "ConfUpdate node was not a signed integer.";
        *x.getVal() = *update_val;
        return "";
    }
    String operator()(Config::ConfUint &x)
    {
        if (Config::containsNull(update))
            return "";

        uint32_t new_val = 0;
        const auto *update_val_uint = update->get<uint32_t>();
        if (update_val_uint == nullptr) {
            const auto *update_val_int = update->get<int32_t>();
            if (update_val_int == nullptr || *update_val_int < 0)
                return "ConfUpdate node was not an unsigned integer.";

            new_val = (uint32_t) *update_val_int;
        } else {
            new_val = *update_val_uint;
        }
        *x.getVal() = new_val;
        return "";
    }
    String operator()(Config::ConfBool &x)
    {
        if (Config::containsNull(update))
            return "";

        const auto *update_val = update->get<bool>();
        if (update_val == nullptr)
            return "ConfUpdate node was not a boolean.";
        x.value = *update_val;
        return "";
    }
    String operator()(const Config::ConfVariant::Empty &x)
    {
        return Config::containsNull(update) ? "" : "JSON null node was not null";
    }
    String operator()(Config::ConfArray &x)
    {
        if (Config::containsNull(update))
            return "";

        const Config::ConfUpdateArray *arr = update->get<Config::ConfUpdateArray>();
        if (arr == nullptr)
            return "ConfUpdate node was not an array.";

        const auto arr_size = arr->elements.size();
        auto *val = x.getVal();
        const auto old_size = val->size();

        if (arr_size != old_size) {
            if (arr_size < old_size) {
                // resize() to smaller value truncates vector.
                val->resize(arr_size);
                if (arr_size < (val->capacity() / 2))
                    val->shrink_to_fit();
            } else {
                // Cannot use resize() to enlarge the vector because the new elements wouldn't be copies of the prototype.
                val->reserve(arr_size);
                const auto *prototype = as_const(x).getSlot()->prototype;
                for (size_t i = old_size; i < arr_size; ++i) {
                    val->push_back(*prototype);

                    // Must get val again because the push_back() consumes a slot, which might trigger a slot array move that invalidates the pointer.
                    val = x.getVal();
                }
            }
        }

        for (size_t i = 0; i < arr_size; ++i) {
            String inner_error = Config::apply_visitor(from_update{&arr->elements[i]}, (*val)[i].value);
            if (inner_error != "")
                return String("[") + i + "] " + inner_error;
        }

        return "";
    }
    String operator()(Config::ConfObject &x)
    {
        if (Config::containsNull(update))
            return "";

        const Config::ConfUpdateObject *obj = update->get<Config::ConfUpdateObject>();
        if (obj == nullptr) {
            Serial.println(update->which());
            return "ConfUpdate node was not an object.";
        }

        auto *val = x.getVal();
        const auto size = val->size();

        const auto &obj_elements = obj->elements;
        const auto obj_size = obj_elements.size();
        if (obj_size != size)
            return String("ConfUpdate object had ") + obj_size + " entries instead of the expected " + size;

        for (size_t i = 0; i < size; ++i) {
            size_t obj_idx = 0xFFFFFFFF;
            auto &val_pair = (*val)[i];
            for (size_t j = 0; j < size; ++j) {
                if (obj_elements[j].first != val_pair.first)
                    continue;
                obj_idx = j;
                break;
            }
            if (obj_idx == 0xFFFFFFFF)
                return String("Key ") + val_pair.first + String("not found in ConfUpdate object");

            String inner_error = Config::apply_visitor(from_update{&obj_elements[obj_idx].second}, val_pair.second.value);
            if (inner_error != "")
                return String("[\"") + val_pair.first + "\"] " + inner_error;
        }

        return "";
    }
    String operator()(Config::ConfUnion &x) {
        if (Config::containsNull(update))
            return "";

        const Config::ConfUpdateUnion *un = update->get<Config::ConfUpdateUnion>();
        if (un == nullptr) {
            Serial.println(update->which());
            return "ConfUpdate node was not a union.";
        }

        if (un->tag != as_const(x).getSlot()->tag) {
            x.changeUnionVariant(un->tag);
        }

        return Config::apply_visitor(from_update{&un->value}, x.getVal()->value);
    }


    const Config::ConfUpdate *update;
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
    bool operator()(const Config::ConfVariant::Empty &x)
    {
        return false;
    }
    bool operator()(const Config::ConfArray &x) const
    {
        for (const Config &c : *x.getVal()) {
            if (((c.value.updated & api_backend_flag) != 0) || Config::apply_visitor(is_updated{api_backend_flag}, c.value))
                return true;
        }
        return false;
    }
    bool operator()(const Config::ConfObject &x) const
    {
        for (const std::pair<String, Config> &c : *x.getVal()) {
            if (((c.second.value.updated & api_backend_flag) != 0) || Config::apply_visitor(is_updated{api_backend_flag}, c.second.value))
                return true;
        }
        return false;
    }
    bool operator()(const Config::ConfUnion &x) const
    {
        const auto &value = x.getVal()->value;
        return ((value.updated & api_backend_flag) != 0) || Config::apply_visitor(is_updated{api_backend_flag}, value);
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
    void operator()(const Config::ConfVariant::Empty &x)
    {
    }
    void operator()(Config::ConfArray &x)
    {
        for (Config &c : *x.getVal()) {
            c.value.updated &= ~api_backend_flag;
            Config::apply_visitor(set_updated_false{api_backend_flag}, c.value);
        }
    }
    void operator()(Config::ConfObject &x)
    {
        for (std::pair<String, Config> &c : *x.getVal()) {
            c.second.value.updated &= ~api_backend_flag;
            Config::apply_visitor(set_updated_false{api_backend_flag}, c.second.value);
        }
    }
    void operator()(Config::ConfUnion &x)
    {
        auto &value = x.getVal()->value;
        value.updated &= ~api_backend_flag;
        Config::apply_visitor(set_updated_false{api_backend_flag}, value);
    }
    uint8_t api_backend_flag;
};

template<typename T>
static size_t nextSlot(typename T::Slot *&buf, size_t &buf_size) {
    for (size_t i = 0; i < buf_size; i++)
    {
        if (!T::slotEmpty(i))
            continue;

        return i;
    }

    auto new_buf = new typename T::Slot[buf_size + SLOT_HEADROOM];

    for(size_t i = 0; i < buf_size; ++i)
        new_buf[i] = std::move(buf[i]);

    delete[] buf;
    buf = new_buf;
    size_t result = buf_size;
    buf_size = buf_size + SLOT_HEADROOM;
    return result;
}


bool Config::ConfUnion::slotEmpty(size_t i) {
    return union_buf[i].prototypes == nullptr;
}

bool Config::ConfUnion::changeUnionVariant(uint8_t tag) {
    auto &slot = union_buf[idx];
    for(int i = 0; i < slot.prototypes_len; ++i) {
        if (slot.prototypes[i].tag == tag) {
            union_buf[idx].tag = tag;
            slot.val = slot.prototypes[i].config;
            return true;
        }
    }

    return false;
}
uint8_t Config::ConfUnion::getTag() const { return union_buf[idx].tag; }

Config* Config::ConfUnion::getVal() { return &union_buf[idx].val; }
const Config* Config::ConfUnion::getVal() const { return &union_buf[idx].val; }

const Config::ConfUnion::Slot* Config::ConfUnion::getSlot() const { return &union_buf[idx]; }
Config::ConfUnion::Slot* Config::ConfUnion::getSlot() { return &union_buf[idx]; }

Config::ConfUnion::ConfUnion(const Config &val, uint8_t tag, uint8_t prototypes_len, const ConfUnionPrototype prototypes[])
{
    idx = nextSlot<Config::ConfUnion>(union_buf, union_buf_size);

    auto *slot = this->getSlot();
    slot->val = val;
    slot->tag = tag;
    slot->prototypes_len = prototypes_len;
    slot->prototypes = prototypes;
}

Config::ConfUnion::ConfUnion(const ConfUnion &cpy)
{
    idx = nextSlot<Config::ConfUnion>(union_buf, union_buf_size);

    // If cpy->inUse is false, it is okay that we don't mark this slot as inUse.

    // this->getSlot() is evaluated before the RHS of the assignment is copied over.
    // This results in the LHS pointing to a deallocated array if copying the RHS
    // resizes the slot array. Copying into a temp value (which resizes the array if necessary)
    // and moving this value in the slot works.
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfUnion::~ConfUnion()
{
    auto *slot = this->getSlot();
    slot->val = nullconf;
    slot->tag = 0;
    slot->prototypes_len = 0;
    slot->prototypes = nullptr;
}

Config::ConfUnion& Config::ConfUnion::operator=(const ConfUnion &cpy)
{
    if (this == &cpy) {
        return *this;
    }

    *this->getSlot() = *cpy.getSlot();

    return *this;
}


bool Config::ConfString::slotEmpty(size_t i) {
    return !string_buf[i].inUse;
}

CoolString* Config::ConfString::getVal() { return &string_buf[idx].val; }
const CoolString* Config::ConfString::getVal() const { return &string_buf[idx].val; }

const Config::ConfString::Slot* Config::ConfString::getSlot() const { return &string_buf[idx]; }
Config::ConfString::Slot* Config::ConfString::getSlot() { return &string_buf[idx]; }

Config::ConfString::ConfString(const CoolString &val, uint16_t minChars, uint16_t maxChars)
{
    idx = nextSlot<Config::ConfString>(string_buf, string_buf_size);
    auto *slot = this->getSlot();
    slot->inUse = true;

    slot->val = val;
    slot->minChars = minChars;
    slot->maxChars = maxChars;
}

Config::ConfString::ConfString(const ConfString &cpy)
{
    idx = nextSlot<Config::ConfString>(string_buf, string_buf_size);

    // If cpy->inUse is false, it is okay that we don't mark this slot as inUse.

    // this->getSlot() is evaluated before the RHS of the assignment is copied over.
    // This results in the LHS pointing to a deallocated array if copying the RHS
    // resizes the slot array. Copying into a temp value (which resizes the array if necessary)
    // and moving this value in the slot works.
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfString::~ConfString()
{
    auto *slot = this->getSlot();
    string_buf[idx].inUse = false;

    slot->val.clear();
    slot->val.shrinkToFit();
    slot->minChars = 0;
    slot->maxChars = 0;
}

Config::ConfString& Config::ConfString::operator=(const ConfString &cpy)
{
    if (this == &cpy) {
        return *this;
    }

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

bool Config::ConfFloat::slotEmpty(size_t i) {
    return float_buf[i].val == 0
        && float_buf[i].min == 0
        && float_buf[i].max == 0;
}

float* Config::ConfFloat::getVal() { return &float_buf[idx].val; }
const float* Config::ConfFloat::getVal() const { return &float_buf[idx].val; }

const Config::ConfFloat::Slot *Config::ConfFloat::getSlot() const { return &float_buf[idx]; }
Config::ConfFloat::Slot *Config::ConfFloat::getSlot() { return &float_buf[idx]; }

Config::ConfFloat::ConfFloat(float val, float min, float max)
{
    idx = nextSlot<Config::ConfFloat>(float_buf, float_buf_size);
    auto *slot = this->getSlot();
    slot->val = val;
    slot->min = min;
    slot->max = max;
}

Config::ConfFloat::ConfFloat(const ConfFloat &cpy)
{
    idx = nextSlot<Config::ConfFloat>(float_buf, float_buf_size);
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfFloat::~ConfFloat()
{
    auto *slot = this->getSlot();
    slot->val = 0;
    slot->min = 0;
    slot->max = 0;
}

Config::ConfFloat& Config::ConfFloat::operator=(const ConfFloat &cpy) {
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

bool Config::ConfInt::slotEmpty(size_t i) {
    return int_buf[i].val == 0
        && int_buf[i].min == 0
        && int_buf[i].max == 0;
}

int32_t* Config::ConfInt::getVal() { return &int_buf[idx].val; }
const int32_t* Config::ConfInt::getVal() const { return &int_buf[idx].val; }

const Config::ConfInt::Slot *Config::ConfInt::getSlot() const { return &int_buf[idx]; }
Config::ConfInt::Slot *Config::ConfInt::getSlot() { return &int_buf[idx]; }

Config::ConfInt::ConfInt(int32_t val, int32_t min, int32_t max)
{
    idx = nextSlot<Config::ConfInt>(int_buf, int_buf_size);
    auto *slot = this->getSlot();
    slot->val = val;
    slot->min = min;
    slot->max = max;
}

Config::ConfInt::ConfInt(const ConfInt &cpy)
{
    idx = nextSlot<Config::ConfInt>(int_buf, int_buf_size);
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfInt::~ConfInt()
{
    auto *slot = this->getSlot();
    slot->val = 0;
    slot->min = 0;
    slot->max = 0;
}

Config::ConfInt& Config::ConfInt::operator=(const ConfInt &cpy) {
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

bool Config::ConfUint::slotEmpty(size_t i) {
    return uint_buf[i].val == 0
        && uint_buf[i].min == 0
        && uint_buf[i].max == 0;
}

uint32_t* Config::ConfUint::getVal() { return &uint_buf[idx].val; }
const uint32_t* Config::ConfUint::getVal() const { return &uint_buf[idx].val; }

const Config::ConfUint::Slot *Config::ConfUint::getSlot() const { return &uint_buf[idx]; }
Config::ConfUint::Slot *Config::ConfUint::getSlot() { return &uint_buf[idx]; }

Config::ConfUint::ConfUint(uint32_t val, uint32_t min, uint32_t max)
{
    idx = nextSlot<Config::ConfUint>(uint_buf, uint_buf_size);
    auto *slot = this->getSlot();
    slot->val = val;
    slot->min = min;
    slot->max = max;
}

Config::ConfUint::ConfUint(const ConfUint &cpy)
{
    idx = nextSlot<Config::ConfUint>(uint_buf, uint_buf_size);
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfUint::~ConfUint()
{
    auto *slot = this->getSlot();
    slot->val = 0;
    slot->min = 0;
    slot->max = 0;
}

Config::ConfUint& Config::ConfUint::operator=(const ConfUint &cpy) {
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

bool Config::ConfArray::slotEmpty(size_t i) {
    return !array_buf[i].inUse;
}

Config *Config::ConfArray::get(uint16_t i)
{
    auto *val = this->getVal();
    if (i >= val->size()) {
        logger.printfln("Config index %u out of range!", i);
        delay(100);
        return nullptr;
    }
    return &(*val)[i];
}
const Config *Config::ConfArray::get(uint16_t i) const
{
    const auto *val = this->getVal();
    if (i >= val->size()) {
        logger.printfln("Config index %u out of range!", i);
        delay(100);
        return nullptr;
    }
    return &(*val)[i];
}

std::vector<Config> *Config::ConfArray::getVal() { return &array_buf[idx].val; }
const std::vector<Config> *Config::ConfArray::getVal() const { return &array_buf[idx].val; }

const Config::ConfArray::Slot *Config::ConfArray::getSlot() const { return &array_buf[idx]; }
Config::ConfArray::Slot *Config::ConfArray::getSlot() { return &array_buf[idx]; }

Config::ConfArray::ConfArray(std::vector<Config> val, const Config *prototype, uint16_t minElements, uint16_t maxElements, int8_t variantType)
{
    idx = nextSlot<Config::ConfArray>(array_buf, array_buf_size);
    auto *slot = this->getSlot();
    slot->inUse = true;

    slot->val = val;
    slot->prototype = prototype;
    slot->minElements = minElements;
    slot->variantType = variantType;

    if (maxElements < minElements) {
        slot->maxElements = minElements;
        logger.printfln("ConfArray of variantType %i: Requested maxElements of %u raised to fit minElements of %u.", variantType, maxElements, minElements);
    } else {
        slot->maxElements = maxElements;
    }
}

Config::ConfArray::ConfArray(const ConfArray &cpy)
{
    idx = nextSlot<Config::ConfArray>(array_buf, array_buf_size);
    // We have to mark this slot as in use here:
    // This array could contain a nested array that will be copied over
    // The inner array's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as inUse first.
    this->getSlot()->inUse = true;

    auto tmp = *cpy.getSlot();

    // Must call getSlot() again because any reference would be invalidated
    // if the copy triggers a move of the slots.
    *this->getSlot() = std::move(tmp);
}

Config::ConfArray::~ConfArray()
{
    auto *slot = this->getSlot();
    slot->inUse = false;

    slot->val.clear();
    slot->prototype = nullptr;
    slot->minElements = 0;
    slot->maxElements = 0;
    slot->variantType = 0;
}

Config::ConfArray& Config::ConfArray::operator=(const ConfArray &cpy) {
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

bool Config::ConfObject::slotEmpty(size_t i) {
    return !object_buf[i].inUse;
}

Config *Config::ConfObject::get(const String &s)
{
    auto *val = this->getVal();
    const auto size = val->size();

    for (size_t i = 0; i < size; ++i) {
        auto &val_pair = (*val)[i];
        if (val_pair.first == s)
            return &val_pair.second;
    }

    logger.printfln("Config key %s not found!", s.c_str());
    delay(100);
    return nullptr;
}

const Config *Config::ConfObject::get(const String &s) const
{
    const auto *val = this->getVal();
    const auto size = val->size();

    for (size_t i = 0; i < size; ++i) {
        const auto &val_pair = (*val)[i];
        if (val_pair.first == s)
            return &val_pair.second;
    }

    logger.printfln("Config key %s not found!", s.c_str());
    delay(100);
    return nullptr;
}

std::vector<std::pair<String, Config>> *Config::ConfObject::getVal() { return &object_buf[idx].val; }
const std::vector<std::pair<String, Config>> *Config::ConfObject::getVal() const { return &object_buf[idx].val; }

const Config::ConfObject::Slot *Config::ConfObject::getSlot() const { return &object_buf[idx]; }
Config::ConfObject::Slot *Config::ConfObject::getSlot() { return &object_buf[idx]; }

Config::ConfObject::ConfObject(std::vector<std::pair<String, Config>> val)
{
    idx = nextSlot<Config::ConfObject>(object_buf, object_buf_size);
    auto *slot = this->getSlot();
    slot->inUse = true;

    slot->val = val;
}

Config::ConfObject::ConfObject(const ConfObject &cpy)
{
    idx = nextSlot<Config::ConfObject>(object_buf, object_buf_size);
    // We have to mark this slot as in use here:
    // This object could contain a nested object that will be copied over
    // The inner object's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as inUse first.
    this->getSlot()->inUse = true;

    auto tmp = *cpy.getSlot();
    // Must call getSlot() again because any reference would be invalidated
    // if the copy triggers a move of the slots.
    *this->getSlot() = std::move(tmp);
}

Config::ConfObject::~ConfObject()
{
    auto *slot = this->getSlot();
    slot->inUse = false;

    slot->val.clear();
}

Config::ConfObject& Config::ConfObject::operator=(const ConfObject &cpy) {
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config Config::Str(const String &s, uint16_t minChars, uint16_t maxChars)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfString{CoolString(s), minChars, maxChars}};
}

Config Config::Float(float d, float min, float max)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfFloat{d, min, max}};
}

Config Config::Int(int32_t i, int32_t min, int32_t max)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfInt{i, min, max}};
}

Config Config::Uint(uint32_t u, uint32_t min, uint32_t max)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfUint{u, min, max}};
}

Config Config::Bool(bool b)
{
    // Allow constructing bool configs:
    // Those are not stored in slots so no static initialization order problems can emerge here.

    return Config{ConfBool{b}};
}

Config Config::Array(std::initializer_list<Config> arr, const Config *prototype, uint16_t minElements, uint16_t maxElements, int variantType)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfArray{arr, prototype, minElements, maxElements, (int8_t)variantType}};
}

Config Config::Object(std::initializer_list<std::pair<String, Config>> obj)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfObject{obj}};
}

Config Config::Union(Config value, uint8_t tag, const ConfUnionPrototype prototypes[], uint8_t prototypes_len)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfUnion{value, tag, prototypes_len, prototypes}};
}

ConfigRoot *Config::Null()
{
    // Allow constructing null configs:
    // Those are not stored in slots so no static initialization order problems can emerge here.

    return &nullconf;
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

Config::Wrap Config::get() {
    if (!this->is<Config::ConfUnion>()) {
        logger.printfln("Config is not a union!");
        delay(100);
        return Wrap(nullptr);
    }
    Wrap wrap(value.val.un.getVal());

    return wrap;
}

const Config::ConstWrap Config::get() const {
    if (!this->is<Config::ConfUnion>()) {
        logger.printfln("Config is not a union!");
        delay(100);
        return ConstWrap(nullptr);
    }
    ConstWrap wrap(value.val.un.getVal());

    return wrap;
}



Config::Wrap Config::get(const String &s)
{

    if (!this->is<Config::ConfObject>()) {
        logger.printfln("Config key %s not in this node: is not an object!", s.c_str());
        delay(100);
        return Wrap(nullptr);
    }
    Wrap wrap(value.val.o.get(s));

    return wrap;
}

Config::Wrap Config::get(uint16_t i)
{

    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Config index %u not in this node: is not an array!", i);
        delay(100);
        return Wrap(nullptr);
    }
    Wrap wrap(value.val.a.get(i));

    return wrap;
}

const Config::ConstWrap Config::get(const String &s) const
{
    if (!this->is<Config::ConfObject>()) {
        logger.printfln("Config key %s not in this node: is not an object!", s.c_str());
        delay(100);
        return ConstWrap(nullptr);
    }
    ConstWrap wrap(value.val.o.get(s));

    return wrap;
}

const Config::ConstWrap Config::get(uint16_t i) const
{
    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Config index %u not in this node: is not an array!", i);
        delay(100);
        return ConstWrap(nullptr);
    }
    ConstWrap wrap(value.val.a.get(i));

    return wrap;
}

const CoolString &Config::asString() const
{
    return *this->get<ConfString>()->getVal();
}

const char *Config::asEphemeralCStr() const
{
    return this->get<ConfString>()->getVal()->c_str();
}

const char *Config::asUnsafeCStr() const
{
    return this->get<ConfString>()->getVal()->c_str();
}

float Config::asFloat() const
{
    return *this->get<ConfFloat>()->getVal();
}

uint32_t Config::asUint() const
{
    return *this->get<ConfUint>()->getVal();
}

int32_t Config::asInt() const
{
    return *this->get<ConfInt>()->getVal();
}

bool Config::asBool() const
{
    return *this->get<ConfBool>()->getVal();
}

std::vector<Config> &Config::asArray()
{
    return *this->get<ConfArray>()->getVal();
}

const std::vector<Config> &Config::asArray() const
{
    return *this->get<ConfArray>()->getVal();
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

size_t Config::json_size(bool zero_copy) const
{
    return Config::apply_visitor(json_length_visitor{zero_copy}, value);
}

size_t Config::max_string_length() const
{
    return Config::apply_visitor(string_length_visitor{}, value);
}

DynamicJsonDocument Config::to_json(const std::vector<String> &keys_to_censor) const {
    DynamicJsonDocument doc(json_size(false));

    JsonVariant var;
    if (is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if (is<Config::ConfArray>() || is<Config::ConfUnion>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    Config::apply_visitor(::to_json{var, keys_to_censor}, value);
    return doc;
}

void Config::save_to_file(File &file)
{
    auto doc = this->to_json({});

    if (doc.overflowed()) {
        logger.printfln("JSON doc overflow while writing file %s! Doc capacity is %u. Truncated doc follows.", file.name(), doc.capacity());
        String str;
        serializeJson(doc, str);
        logger.write(str.c_str(), str.length());
    }
    serializeJson(doc, file);
}

void Config::write_to_stream(Print &output)
{
    write_to_stream_except(output, {});
}

void Config::write_to_stream_except(Print &output, const std::vector<String> &keys_to_censor)
{
    auto doc = this->to_json(keys_to_censor);

    if (doc.overflowed()) {
        logger.printfln("JSON doc overflow while writing to stream! Doc capacity is %u. Truncated doc follows.", doc.capacity());
        String str;
        serializeJson(doc, str);
        logger.write(str.c_str(), str.length());
    }
    serializeJson(doc, output);
}

String Config::to_string() const
{
    return this->to_string_except({});
}

String Config::to_string_except(const std::vector<String> &keys_to_censor) const
{
    auto doc = this->to_json(keys_to_censor);

    String result;
    serializeJson(doc, result);

    if (doc.overflowed()) {
        logger.printfln("JSON doc overflow while converting to string! Doc capacity is %u. Truncated doc follows.", doc.capacity());
        logger.write(result.c_str(), result.length());
    }
    return result;
}

bool Config::was_updated(uint8_t api_backend_flag)
{
    return ((value.updated & api_backend_flag) != 0) || Config::apply_visitor(is_updated{api_backend_flag}, value);
}

void Config::set_update_handled(uint8_t api_backend_flag)
{
    value.updated = false;
    Config::apply_visitor(set_updated_false{api_backend_flag}, value);
}

static std::recursive_mutex update_mutex;

void ConfigRoot::update_from_copy(Config *copy) {
    std::lock_guard<std::recursive_mutex> l{update_mutex};
    this->value = copy->value;
    this->value.updated = true;
}

String ConfigRoot::update_from_file(File &file)
{
    DynamicJsonDocument doc(this->json_size(false));
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        return String("Failed to read file: ") + error.c_str();

    return this->update_from_json(doc.as<JsonVariant>(), false);
}

// Intentionally take a non-const char * here:
// This allows ArduinoJson to deserialize in zero-copy mode
String ConfigRoot::update_from_cstr(char *c, size_t len)
{
    Config copy;
    String err = this->get_updated_copy(c, len, &copy);
    if (err != "")
        return err;

    this->update_from_copy(&copy);
    return "";
}

String ConfigRoot::get_updated_copy(char *c, size_t payload_len, Config *out_config) {
    DynamicJsonDocument doc(this->json_size(true));
    DeserializationError error = deserializeJson(doc, c, payload_len);

    switch (error.code()) {
        case DeserializationError::Ok:
            return this->get_updated_copy(doc.as<JsonVariant>(), true, out_config);
        case DeserializationError::NoMemory:
            return String("Failed to deserialize: JSON payload was longer than expected and possibly contained unknown keys.");
        case DeserializationError::EmptyInput:
            return String("Failed to deserialize: Payload was empty. Please send valid JSON.");
        case DeserializationError::IncompleteInput:
            return String("Failed to deserialize: JSON payload incomplete or truncated");
        case DeserializationError::InvalidInput:
            return String("Failed to deserialize: JSON payload could not be parsed");
        case DeserializationError::TooDeep:
            return String("Failed to deserialize: JSON payload nested too deep");
        default:
            return String("Failed to deserialize string: ") + String(error.c_str());
    }
}

String ConfigRoot::update_from_json(JsonVariant root, bool force_same_keys)
{
    Config copy;
    String err = this->get_updated_copy(root, force_same_keys, &copy);
    if (err != "")
        return err;

    this->update_from_copy(&copy);
    return "";
}

String ConfigRoot::get_updated_copy(JsonVariant root, bool force_same_keys, Config *out_config)
{
    return this->get_updated_copy(from_json{root, force_same_keys, this->permit_null_updates, true}, out_config);
}


template<typename T>
String ConfigRoot::get_updated_copy(T visitor, Config *out_config) {
    std::lock_guard<std::recursive_mutex> l{update_mutex};
    *out_config = *this;
    String err = Config::apply_visitor(visitor, out_config->value);

    if (err != "")
        return err;

    err = Config::apply_visitor(default_validator{}, out_config->value);

    if (err != "")
        return err;

    if (this->validator != nullptr) {
        err = this->validator(*out_config);
        if (err != "")
            return err;
    }
    return "";
}

template<typename T>
String ConfigRoot::update_from_visitor(T visitor) {
    Config copy;

    String err = this->get_updated_copy(visitor, &copy);
    if (err != "")
        return err;

    this->update_from_copy(&copy);
    return "";
}

String ConfigRoot::update(const Config::ConfUpdate *val)
{
    return this->update_from_visitor(from_update{val});
}

String ConfigRoot::validate()
{
    if (this->validator != nullptr) {
        return this->validator(*this);
    }
    return "";
}

Config::Wrap::Wrap(Config *_conf)
{
    conf = _conf;
}

void config_pre_init()
{
    uint_buf = new Config::ConfUint::Slot[UINT_SLOTS];
    int_buf = new Config::ConfInt::Slot[INT_SLOTS];
    float_buf = new Config::ConfFloat::Slot[FLOAT_SLOTS];
    string_buf = new Config::ConfString::Slot[STRING_SLOTS];
    array_buf = new Config::ConfArray::Slot[ARRAY_SLOTS];
    object_buf = new Config::ConfObject::Slot[OBJECT_SLOTS];
    union_buf = new Config::ConfUnion::Slot[UNION_SLOTS];

    uint_buf_size = UINT_SLOTS;
    int_buf_size = INT_SLOTS;
    float_buf_size = FLOAT_SLOTS;
    string_buf_size = STRING_SLOTS;
    array_buf_size = ARRAY_SLOTS;
    object_buf_size = OBJECT_SLOTS;
    union_buf_size = UNION_SLOTS;
}

template<typename T>
static void shrinkToFit(typename T::Slot * &buf, size_t &buf_size) {
    size_t highest = 0;
    for (size_t i = 0; i < buf_size; i++)
        if (!T::slotEmpty(i))
            highest = i;

    auto new_buf = new typename T::Slot[highest + SLOT_HEADROOM];

    for(size_t i = 0; i <= highest; ++i)
        new_buf[i] = std::move(buf[i]);
    delete[] buf;
    buf = new_buf;
    buf_size = highest + SLOT_HEADROOM;
}

void config_post_setup() {
    shrinkToFit<Config::ConfUint>(uint_buf, uint_buf_size);
    shrinkToFit<Config::ConfInt>(int_buf, int_buf_size);
    shrinkToFit<Config::ConfFloat>(float_buf, float_buf_size);
    shrinkToFit<Config::ConfString>(string_buf, string_buf_size);
    shrinkToFit<Config::ConfArray>(array_buf, array_buf_size);
    shrinkToFit<Config::ConfObject>(object_buf, object_buf_size);
    shrinkToFit<Config::ConfUnion>(union_buf, union_buf_size);
}

Config::ConstWrap::ConstWrap(const Config *_conf)
{
    conf = _conf;
}

Config::Wrap Config::add() {
    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Tried to add to a node that is not an array!");
        delay(100);
        return Wrap(nullptr);
    }

    std::vector<Config> &children = this->asArray();

    const auto &arr = value.val.a;
    const auto *slot = arr.getSlot();

    const auto max_elements = slot->maxElements;
    if (children.size() >= max_elements) {
        logger.printfln("Tried to add to an ConfArray that already has the max allowed number of elements (%u).", max_elements);
        delay(100);
        return Wrap(nullptr);
    }

    children.push_back(*slot->prototype);

    // The push_back() might invalidate the children reference
    // when ConfArray slots are moved, so asArray() must be called again.
    return Wrap(&this->asArray().back());
}
