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

extern bool config_constructors_allowed;

#define SLOT_HEADROOM 20

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

static ConfigRoot nullconf = Config{Config::ConfVariant{}};


struct default_validator {
    String operator()(const Config::ConfString &x) const
    {
        if (x.getVal()->length() < x.getSlot()->minChars)
            return String(String("String of minimum length ") + x.getSlot()->minChars + " was expected, but got " + x.getVal()->length());

        if (x.getSlot()->maxChars == 0 || x.getVal()->length() <= x.getSlot()->maxChars)
            return String("");

        return String(String("String of maximum length ") + x.getSlot()->maxChars + " was expected, but got " + x.getVal()->length());
    }

    String operator()(const Config::ConfFloat &x) const
    {
        if (*x.getVal() < x.getSlot()->min)
            return String(String("Float value ") + *x.getVal() + " was less than the allowed minimum of " + x.getSlot()->min);
        if (*x.getVal() > x.getSlot()->max)
            return String(String("Float value ") + *x.getVal() + " was more than the allowed maximum of " + x.getSlot()->max);
        return String("");
    }

    String operator()(const Config::ConfInt &x) const
    {
        if (*x.getVal() < x.getSlot()->min)
            return String(String("Integer value ") + *x.getVal() + " was less than the allowed minimum of " + x.getSlot()->min);
        if (*x.getVal() > x.getSlot()->max)
            return String(String("Integer value ") + *x.getVal() + " was more than the allowed maximum of " + x.getSlot()->max);
        return String("");
    }

    String operator()(const Config::ConfUint &x) const
    {
        if (*x.getVal() < x.getSlot()->min)
            return String(String("Unsigned integer value ") + *x.getVal() + " was less than the allowed minimum of " + x.getSlot()->min);
        if (*x.getVal() > x.getSlot()->max)
            return String(String("Unsigned integer value ") + *x.getVal() + " was more than the allowed maximum of " + x.getSlot()->max);
        return String("");
    }

    String operator()(const Config::ConfBool &x) const
    {
        return String("");
    }
    String operator()(const Config::ConfVariant::Empty &x)
    {
        return String("");
    }

    String operator()(const Config::ConfArray &x) const
    {
        const auto *slot = x.getSlot();
        const auto size = x.getVal()->size();
        if (slot->maxElements > 0 && size > slot->maxElements)
            return String(String("Array had ") + size + " entries, but only " + slot->maxElements + " are allowed.");
        if (slot->minElements > 0 && size < slot->minElements)
            return String(String("Array had ") + size + " entries, but at least " + slot->maxElements + " are required.");

        if (slot->variantType >= 0)
            for (int i = 0; i < size; ++i)
                if ((int)x.get(i)->value.tag != slot->variantType)
                    return String(String("[") + i + "] has wrong type");

        size_t i = 0;
        for (const Config &elem : *x.getVal()) {
            String err = Config::apply_visitor(default_validator{}, elem.value);
            if (err != "")
                return String("[") + i + "] " + err;
            ++i;
        }

        return String("");
    }

    String operator()(const Config::ConfObject &x) const
    {
        for (const std::pair<String, Config> &elem : *x.getVal()) {
            String err = Config::apply_visitor(default_validator{}, elem.second.value);

            if (err != "")
                return String("[\"") + elem.first.c_str() + "\"] " + err;
        }

        return String("");
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
        JsonArray arr = insertHere.as<JsonArray>();
        for (size_t i = 0; i < x.getVal()->size(); ++i) {
            const Config *child = x.get(i);

            if (child->is<Config::ConfObject>()) {
                arr.createNestedObject();
            } else if (child->is<Config::ConfArray>()) {
                arr.createNestedArray();
            } else {
                arr.add(0);
            }

            Config::apply_visitor(to_json{arr[i], keys_to_censor}, child->value);
        }
    }
    void operator()(const Config::ConfObject &x)
    {
        JsonObject obj = insertHere.as<JsonObject>();
        for (size_t i = 0; i < x.getVal()->size(); ++i) {
            const String &key = x.getVal()->at(i).first;
            const Config &child = x.getVal()->at(i).second;

            if (child.is<Config::ConfObject>()) {
                obj.createNestedObject(key);
            } else if (child.is<Config::ConfArray>()) {
                obj.createNestedArray(key);
            } else {
                obj.getOrAddMember(key);
            }

            Config::apply_visitor(to_json{obj[key], keys_to_censor}, child.value);
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
        return Config::apply_visitor(string_length_visitor{}, x.getSlot()->prototype->value) * x.getSlot()->maxElements +
               (x.getSlot()->maxElements + 1); // [,] and n-1 ,
    }
    size_t operator()(const Config::ConfObject &x)
    {
        size_t sum = 2; // { and }
        for (size_t i = 0; i < x.getVal()->size(); ++i) {
            sum += x.getVal()->at(i).first.length() + 2; // ""
            sum += Config::apply_visitor(string_length_visitor{}, x.getVal()->at(i).second.value);
        }
        return sum;
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
        return Config::apply_visitor(json_length_visitor{zero_copy}, x.getSlot()->prototype->value) * x.getSlot()->maxElements + JSON_ARRAY_SIZE(x.getSlot()->maxElements);
    }
    size_t operator()(const Config::ConfObject &x)
    {
        size_t sum = 0;
        for (size_t i = 0; i < x.getVal()->size(); ++i) {
            if (!zero_copy)
                sum += x.getVal()->at(i).first.length() + 1;

            sum += Config::apply_visitor(json_length_visitor{zero_copy}, x.getVal()->at(i).second.value);
        }
        return sum + JSON_OBJECT_SIZE(x.getVal()->size());
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
        *x.getVal() = json_node.as<String>();
        return String("");
    }
    String operator()(Config::ConfFloat &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        if (!json_node.is<float>())
            return "JSON node was not a float.";

        *x.getVal() = json_node.as<float>();
        return String("");
    }
    String operator()(Config::ConfInt &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        if (!json_node.is<int32_t>())
            return "JSON node was not a signed integer.";
        *x.getVal() = json_node.as<int32_t>();
        return String("");
    }
    String operator()(Config::ConfUint &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        if (!json_node.is<uint32_t>())
            return "JSON node was not an unsigned integer.";
        *x.getVal() = json_node.as<uint32_t>();
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
            return permit_null_updates ? String("") : String("Null updates not permitted.");

        if (!json_node.is<JsonArray>())
            return "JSON node was not an array.";

        JsonArray arr = json_node.as<JsonArray>();

        // C++17 adds https://en.cppreference.com/w/cpp/utility/as_const
        // until then we have to use this to make sure the const version of getSlot() is called.
        const auto *prototype = ((const Config::ConfArray&)x).getSlot()->prototype;

        x.getVal()->clear();
        for (size_t i = 0; i < arr.size(); ++i) {
            x.getVal()->push_back(*prototype);
            String inner_error = Config::apply_visitor(from_json{arr[i], force_same_keys, permit_null_updates, false}, x.get(i)->value);
            if (inner_error != "")
                return String("[") + i + "] " + inner_error;
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
        if (!json_node.is<JsonObject>() && is_root && x.getVal()->size() == 1) {
            String inner_error = Config::apply_visitor(from_json{json_node, force_same_keys, permit_null_updates, false}, x.getVal()->at(0).second.value);
            if (inner_error != "")
                return String("(inferred) [\"") + x.getVal()->at(0).first + "\"] " + inner_error + "\n";
            else
                return inner_error;
        }

        if (!json_node.is<JsonObject>())
            return "JSON node was not an object.";

        const JsonObject obj = json_node.as<JsonObject>();

        String return_str = "";
        bool more_errors = false;

        for (size_t i = 0; i < x.getVal()->size(); ++i) {
            if (!obj.containsKey(x.getVal()->at(i).first))
            {
                if (!force_same_keys)
                    continue;

                if (return_str.length() < 1000)
                    return_str += String("JSON object is missing key '") + x.getVal()->at(i).first + "'\n";
                else
                    more_errors = true;
            }

            String inner_error = Config::apply_visitor(from_json{obj[x.getVal()->at(i).first], force_same_keys, permit_null_updates, false}, x.getVal()->at(i).second.value);
            if(obj.size() > 0)
                obj.remove(x.getVal()->at(i).first);
            if (inner_error != "")
            {
                if (return_str.length() < 1000)
                    return_str += String("[\"") + x.getVal()->at(i).first + "\"] " + inner_error + "\n";
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

        return String("");
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
            return String("");

        if (update->get<String>() == nullptr)
            return "ConfUpdate node was not a string.";
        *x.getVal() = *(update->get<String>());
        return String("");
    }
    String operator()(Config::ConfFloat &x)
    {
        if (Config::containsNull(update))
            return String("");

        if (update->get<float>() == nullptr)
            return "ConfUpdate node was not a float.";

        *x.getVal() = *(update->get<float>());
        return String("");
    }
    String operator()(Config::ConfInt &x)
    {
        if (Config::containsNull(update))
            return String("");

        if (update->get<int32_t>() == nullptr)
            return "ConfUpdate node was not a signed integer.";
        *x.getVal() = *(update->get<int32_t>());
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
        *x.getVal() = new_val;
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
    String operator()(const Config::ConfVariant::Empty &x)
    {
        return Config::containsNull(update) ? "" : "JSON null node was not null";
    }
    String operator()(Config::ConfArray &x)
    {
        if (Config::containsNull(update))
            return String("");

        if (update->get<Config::ConfUpdateArray>() == nullptr)
            return "ConfUpdate node was not an array.";

        const Config::ConfUpdateArray *arr = update->get<Config::ConfUpdateArray>();

        // C++17 adds https://en.cppreference.com/w/cpp/utility/as_const
        // until then we have to use this to make sure the const version of getSlot() is called.
        const auto *prototype = ((const Config::ConfArray&)x).getSlot()->prototype;

        x.getVal()->clear();
        for (size_t i = 0; i < arr->elements.size(); ++i) {
            x.getVal()->push_back(*prototype);
            String inner_error = Config::apply_visitor(from_update{&arr->elements[i]}, x.get(i)->value);
            if (inner_error != "")
                return String("[") + i + "] " + inner_error;
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

        const Config::ConfUpdateObject *obj = update->get<Config::ConfUpdateObject>();

        if (obj->elements.size() != x.getVal()->size())
            return String("ConfUpdate object had ") + obj->elements.size() + " entries instead of the expected " + x.getVal()->size();

        for (size_t i = 0; i < x.getVal()->size(); ++i) {
            size_t obj_idx = 0xFFFFFFFF;
            for (size_t j = 0; j < x.getVal()->size(); ++j) {
                if (obj->elements[j].first != x.getVal()->at(i).first)
                    continue;
                obj_idx = j;
                break;
            }
            if (obj_idx == 0xFFFFFFFF)
                return String("Key ") + x.getVal()->at(i).first + String("not found in ConfUpdate object");

            String inner_error = Config::apply_visitor(from_update{&obj->elements[obj_idx].second}, x.getVal()->at(i).second.value);
            if (inner_error != "")
                return String("[\"") + x.getVal()->at(i).first + "\"] " + inner_error;
        }

        return String("");
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

bool Config::ConfString::slotEmpty(size_t i) {
    return !string_buf[i].inUse;
}

String* Config::ConfString::getVal() { return &string_buf[idx].val; }
const String* Config::ConfString::getVal() const { return &string_buf[idx].val; }

const Config::ConfString::Slot* Config::ConfString::getSlot() const { return &string_buf[idx]; }
Config::ConfString::Slot* Config::ConfString::getSlot() { return &string_buf[idx]; }

Config::ConfString::ConfString(const String &val, uint16_t minChars, uint16_t maxChars)
{
    idx = nextSlot<Config::ConfString>(string_buf, string_buf_size);
    this->getSlot()->inUse = true;

    this->getSlot()->val = val;
    this->getSlot()->minChars = minChars;
    this->getSlot()->maxChars = maxChars;
}

Config::ConfString::ConfString(const ConfString &cpy)
{
    idx = nextSlot<Config::ConfString>(string_buf, string_buf_size);

    // If cpy->inUse is false, it is okay that we don't mark this slot as inUse.
    *this->getSlot() = *cpy.getSlot();
}

Config::ConfString::~ConfString()
{
    string_buf[idx].inUse = false;

    this->getSlot()->val.clear();
    this->getSlot()->minChars = 0;
    this->getSlot()->maxChars = 0;
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
    this->getSlot()->val = val;
    this->getSlot()->min = min;
    this->getSlot()->max = max;
}

Config::ConfFloat::ConfFloat(const ConfFloat &cpy)
{
    idx = nextSlot<Config::ConfFloat>(float_buf, float_buf_size);
    *this->getSlot() = *cpy.getSlot();
}

Config::ConfFloat::~ConfFloat()
{
    this->getSlot()->val = 0;
    this->getSlot()->min = 0;
    this->getSlot()->max = 0;
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
    this->getSlot()->val = val;
    this->getSlot()->min = min;
    this->getSlot()->max = max;
}

Config::ConfInt::ConfInt(const ConfInt &cpy)
{
    idx = nextSlot<Config::ConfInt>(int_buf, int_buf_size);
    *this->getSlot() = *cpy.getSlot();
}

Config::ConfInt::~ConfInt()
{
    this->getSlot()->val = 0;
    this->getSlot()->min = 0;
    this->getSlot()->max = 0;
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
    this->getSlot()->val = val;
    this->getSlot()->min = min;
    this->getSlot()->max = max;
}

Config::ConfUint::ConfUint(const ConfUint &cpy)
{
    idx = nextSlot<Config::ConfUint>(uint_buf, uint_buf_size);
    *this->getSlot() = *cpy.getSlot();
}

Config::ConfUint::~ConfUint()
{
    this->getSlot()->val = 0;
    this->getSlot()->min = 0;
    this->getSlot()->max = 0;
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
    if (i >= this->getVal()->size()) {
        logger.printfln("Config index %u out of range!", i);
        delay(100);
        return nullptr;
    }
    return &this->getVal()->at(i);
}
const Config *Config::ConfArray::get(uint16_t i) const
{
    if (i >= this->getVal()->size()) {
        logger.printfln("Config index %u out of range!", i);
        delay(100);
        return nullptr;
    }
    return &this->getVal()->at(i);
}

std::vector<Config> *Config::ConfArray::getVal() { return &array_buf[idx].val; }
const std::vector<Config> *Config::ConfArray::getVal() const { return &array_buf[idx].val; }

const Config::ConfArray::Slot *Config::ConfArray::getSlot() const { return &array_buf[idx]; }
Config::ConfArray::Slot *Config::ConfArray::getSlot() { return &array_buf[idx]; }

Config::ConfArray::ConfArray(std::vector<Config> val, Config *prototype, uint16_t minElements, uint16_t maxElements, int8_t variantType)
{
    idx = nextSlot<Config::ConfArray>(array_buf, array_buf_size);
    this->getSlot()->inUse = true;

    this->getSlot()->val = val;
    this->getSlot()->prototype = prototype;
    this->getSlot()->minElements = minElements;
    this->getSlot()->maxElements = maxElements;
    this->getSlot()->variantType = variantType;
}

Config::ConfArray::ConfArray(const ConfArray &cpy)
{
    idx = nextSlot<Config::ConfArray>(array_buf, array_buf_size);
    // We have to mark this slot as in use here:
    // This array could contain a nested array that will be copied over
    // The inner array's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as inUse first.
    this->getSlot()->inUse = true;

    *this->getSlot() = *cpy.getSlot();
}

Config::ConfArray::~ConfArray()
{
    this->getSlot()->inUse = false;

    this->getSlot()->val.clear();
    this->getSlot()->prototype = nullptr;
    this->getSlot()->minElements = 0;
    this->getSlot()->maxElements = 0;
    this->getSlot()->variantType = 0;
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
    for (size_t i = 0; i < this->getVal()->size(); ++i) {
        if (this->getVal()->at(i).first == s)
            return &this->getVal()->at(i).second;
    }

    logger.printfln("Config key %s not found!", s.c_str());
    delay(100);
    return nullptr;
}

const Config *Config::ConfObject::get(const String &s) const
{
    for (size_t i = 0; i < this->getVal()->size(); ++i) {
        if (this->getVal()->at(i).first == s)
            return &this->getVal()->at(i).second;
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
    this->getSlot()->inUse = true;

    this->getSlot()->val = val;
}

Config::ConfObject::ConfObject(const ConfObject &cpy)
{
    idx = nextSlot<Config::ConfObject>(object_buf, object_buf_size);
    // We have to mark this slot as in use here:
    // This object could contain a nested object that will be copied over
    // The inner object's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as inUse first.
    this->getSlot()->inUse = true;

    *this->getSlot() = *cpy.getSlot();
}

Config::ConfObject::~ConfObject()
{
    this->getSlot()->inUse = false;

    this->getSlot()->val.clear();
}

Config::ConfObject& Config::ConfObject::operator=(const ConfObject &cpy) {
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}

Config Config::Str(const String &s, uint16_t minChars, uint16_t maxChars)
{
    if (!config_constructors_allowed)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfString{s, minChars, maxChars == 0 ? (uint16_t)s.length() : maxChars}};
}

Config Config::Float(float d, float min, float max)
{
    if (!config_constructors_allowed)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfFloat{d, min, max}};
}

Config Config::Int(int32_t i, int32_t min, int32_t max)
{
    if (!config_constructors_allowed)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfInt{i, min, max}};
}

Config Config::Uint(uint32_t u, uint32_t min, uint32_t max)
{
    if (!config_constructors_allowed)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfUint{u, min, max}};
}

Config Config::Bool(bool b)
{
    // Allow constructing bool configs:
    // Those are not stored in slots so no static initialization order problems can emerge here.

    return Config{ConfBool{b}};
}

Config Config::Array(std::initializer_list<Config> arr, Config *prototype, uint16_t minElements, uint16_t maxElements, int variantType)
{
    if (!config_constructors_allowed)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfArray{arr, prototype, minElements, maxElements, (int8_t)variantType}};
}

Config Config::Object(std::initializer_list<std::pair<String, Config>> obj)
{
    if (!config_constructors_allowed)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfObject{obj}};
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

const String &Config::asString() const
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

void Config::save_to_file(File &file)
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
    Config::apply_visitor(to_json{var, {}}, value);

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
    Config::apply_visitor(to_json{var, {}}, value);
    serializeJson(doc, output);
}

String Config::to_string() const
{
    return this->to_string_except({});
}

String Config::to_string_except(const std::initializer_list<String> &keys_to_censor) const
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
    Config::apply_visitor(to_json{var, keys_to_censor}, value);

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
    Config::apply_visitor(to_json{var, keys_to_censor}, value);

    String result;
    serializeJson(doc, result);
    return result;
}

void Config::write_to_stream_except(Print &output, const std::initializer_list<String> &keys_to_censor)
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
    Config::apply_visitor(to_json{var, keys_to_censor}, value);

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
    Config::apply_visitor(to_json{var, keys_to_censor}, value);

    serializeJson(doc, output);
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

String ConfigRoot::update_from_file(File &file)
{
    DynamicJsonDocument doc(this->json_size(false));
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        return String("Failed to read file: ") + String(error.c_str());

    return this->update_from_json(doc.as<JsonVariant>(), false);
}

// Intentionally take a non-const char * here:
// This allows ArduinoJson to deserialize in zero-copy mode
String ConfigRoot::update_from_cstr(char *c, size_t len)
{
    DynamicJsonDocument doc(this->json_size(true));
    DeserializationError error = deserializeJson(doc, c, len);

    switch (error.code()) {
        case DeserializationError::Ok:
            return this->update_from_json(doc.as<JsonVariant>(), true);
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
    Config copy = *this;
    String err = Config::apply_visitor(from_json{root, force_same_keys, this->permit_null_updates, true}, copy.value);

    if (err != "")
        return err;

    err = Config::apply_visitor(default_validator{}, copy.value);

    if (err != "")
        return err;

    if (this->validator != nullptr) {
        err = this->validator(copy);
        if (err != "")
            return err;
    }

    this->value = copy.value;
    this->value.updated = true;

    return err;
}

String ConfigRoot::update(const Config::ConfUpdate *val)
{
    Config copy = *this;
    String err = Config::apply_visitor(from_update{val}, copy.value);
    if (err != "")
        return err;

    err = Config::apply_visitor(default_validator{}, copy.value);

    if (err != "")
        return err;

    if (this->validator != nullptr) {
        err = this->validator(copy);
        if (err != "")
            return err;
    }

    this->value = copy.value;
    this->value.updated = true;

    return err;
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

void config_preinit()
{
    uint_buf = new Config::ConfUint::Slot[UINT_SLOTS];
    int_buf = new Config::ConfInt::Slot[INT_SLOTS];
    float_buf = new Config::ConfFloat::Slot[FLOAT_SLOTS];
    string_buf = new Config::ConfString::Slot[STRING_SLOTS];
    array_buf = new Config::ConfArray::Slot[ARRAY_SLOTS];
    object_buf = new Config::ConfObject::Slot[OBJECT_SLOTS];

    uint_buf_size = UINT_SLOTS;
    int_buf_size = INT_SLOTS;
    float_buf_size = FLOAT_SLOTS;
    string_buf_size = STRING_SLOTS;
    array_buf_size = ARRAY_SLOTS;
    object_buf_size = OBJECT_SLOTS;
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

void config_postsetup() {
    shrinkToFit<Config::ConfUint>(uint_buf, uint_buf_size);
    shrinkToFit<Config::ConfInt>(int_buf, int_buf_size);
    shrinkToFit<Config::ConfFloat>(float_buf, float_buf_size);
    shrinkToFit<Config::ConfString>(string_buf, string_buf_size);
    shrinkToFit<Config::ConfArray>(array_buf, array_buf_size);
    shrinkToFit<Config::ConfObject>(object_buf, object_buf_size);
}

Config::ConstWrap::ConstWrap(const Config *_conf)
{
    conf = _conf;
}
