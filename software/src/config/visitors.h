#pragma once

#include "config/private.h"

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
        for (size_t i = 0; i < x.getSlot()->schema->length; ++i) {
            String err = Config::apply_visitor(default_validator{}, x.getSlot()->values[i].value);

            if (err != "")
                return String("[\"") + x.getSlot()->schema->keys[i] + "\"] " + err;
        }

        return "";
    }

    String operator()(const Config::ConfUnion &x) const
    {
        // Tag match is already checked in from_json.

        return Config::apply_visitor(default_validator{}, x.getVal()->value);
    }
};

struct to_json {
    void operator()(const Config::ConfString &x)
    {
        insertHere.set(x.getVal()->c_str());
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

            Config::apply_visitor(to_json{arr[i], keys_to_censor, keys_to_censor_len}, child->value);
        }
    }
    void operator()(const Config::ConfObject &x)
    {
        const auto *slot = x.getSlot();
        const auto *schema = slot->schema;
        const auto size = schema->length;

        JsonObject obj = insertHere.as<JsonObject>();
        for (size_t i = 0; i < size; ++i) {
            const char *key = schema->keys[i];
            const Config &child = slot->values[i];

            if (child.is<Config::ConfObject>()) {
                obj.createNestedObject(key);
            } else if (child.is<Config::ConfArray>() || child.is<Config::ConfUnion>()) {
                obj.createNestedArray(key);
            } else if (!obj.containsKey(key)) {
                obj[key] = nullptr;
            }

            Config::apply_visitor(to_json{obj[key], keys_to_censor, keys_to_censor_len}, child.value);
        }

        for (size_t i = 0; i < keys_to_censor_len; ++i) {
            const String &key = keys_to_censor[i];
            if (obj.containsKey(key) && !(obj[key].is<String>() && obj[key].as<String>().length() == 0))
                obj[key] = nullptr;
        }
    }

    void operator()(const Config::ConfUnion &x)
    {
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

        Config::apply_visitor(to_json{insertHere[1], keys_to_censor, keys_to_censor_len}, val->value);
    }

    JsonVariant insertHere;
    const String *keys_to_censor;
    size_t keys_to_censor_len;
};

static const uint8_t leading_zeros_to_char_count[33] = {10,10,10,9,9,9,8,8,8,7,7,7,7,6,6,6,5,5,5,4,4,4,4,3,3,3,2,2,2,1,1,1,1};

// Never underestimates length. Overestimates by 0.12 chars on average.
// Tested against the returned length of snprintf([...],"%u") for every 32 bit number.
static size_t estimate_chars_per_uint(uint32_t v) {
#if !defined(__XTENSA_EL__) || __XTENSA_EL__ != 1
    // __builtin_clz(v) is undefined for v == 0. Set the lowest bit to work around that.
    // The workaround is not neccessary on Xtensa, which uses the NSAU instruction that handles 0 correctly.
    v |= 1;
#endif

    return leading_zeros_to_char_count[__builtin_clz(v)];
}

// Never underestimates length. Overestimates by 0.23 chars on average.
// Tested against the returned length of snprintf([...],"%d") for every 32 bit number.
static size_t estimate_chars_per_int(int32_t v)
{
    uint32_t sign = (static_cast<uint32_t>(v)) >> 31;

    v = v ^ (v >> 31); // Approximate abs(v). Negative values are off by 1, which doesn't matter for the estimation.

#if !defined(__XTENSA_EL__) || __XTENSA_EL__ != 1
    // __builtin_clz(v) is undefined for v == 0. Set the lowest bit to work around that.
    // The workaround is not neccessary on Xtensa, which uses the NSAU instruction that handles 0 correctly.
    v |= 1;
#endif

    return leading_zeros_to_char_count[__builtin_clz(static_cast<uint32_t>(v))] + sign;
}

struct max_string_length_visitor {
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
        return max(estimate_chars_per_int(x.getSlot()->max),
                   estimate_chars_per_int(x.getSlot()->min));
    }
    size_t operator()(const Config::ConfUint &x)
    {
        return estimate_chars_per_uint(x.getSlot()->max);
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

        return Config::apply_visitor(max_string_length_visitor{}, slot->prototype->value) * slot->maxElements +
               (slot->maxElements + 1); // [,] and n-1 ,
    }
    size_t operator()(const Config::ConfObject &x)
    {
        const auto *slot = x.getSlot();
        const auto *schema = slot->schema;
        const auto size = schema->length;

        size_t sum = 2; // { and }
        for (size_t i = 0; i < size; ++i) {
            sum += schema->key_lengths[i] + 3; // "":
            sum += Config::apply_visitor(max_string_length_visitor{}, slot->values[i].value);
        }
        sum += size - 1; // ,
        return sum;
    }

    size_t operator()(const Config::ConfUnion &x)
    {
        const auto *slot = x.getSlot();

        size_t max_len = Config::apply_visitor(max_string_length_visitor{}, x.getVal()->value);
        uint8_t max_tag = 0;
        for (size_t i = 0; i < slot->prototypes_len; ++i) {
            max_len = std::max(max_len, Config::apply_visitor(max_string_length_visitor{}, slot->prototypes[i].config.value));
            max_tag = std::max(max_tag, slot->prototypes[i].tag);
        }
        max_len += estimate_chars_per_uint(max_tag); // tag
        return max_len + 3; // [,]
    }
};

struct string_length_visitor {
    size_t operator()(const Config::ConfString &x)
    {
        return (x.getVal()->length()) + 2; // ""
    }
    size_t operator()(const Config::ConfFloat &x)
    {
        // ArduinoJson will switch to scientific notation >= 1e9.
        // The max length then is -1.123456789e-12 (max. 9 decimal places,
        // exponents max 2 chars because of FLT_MAX)
        return 16;
    }
    size_t operator()(const Config::ConfInt &x)
    {
        return estimate_chars_per_int(*x.getVal());
    }
    size_t operator()(const Config::ConfUint &x)
    {
        return estimate_chars_per_uint(*x.getVal());
    }
    size_t operator()(const Config::ConfBool &x)
    {
        return (*x.getVal()) ? 4 : 5;
    }
    size_t operator()(const Config::ConfVariant::Empty &x)
    {
        return 4; //null
    }
    size_t operator()(const Config::ConfArray &x)
    {
        const auto *val = x.getVal();
        const auto size = val->size();

        size_t sum = 2; // []
        for (size_t i = 0; i < size; ++i) {
            sum += Config::apply_visitor(string_length_visitor{}, (*val)[i].value) + 1; // ,
        }

        return sum;
    }
    size_t operator()(const Config::ConfObject &x)
    {
        const auto *slot = x.getSlot();
        const auto *schema = slot->schema;
        const auto size = schema->length;

        size_t sum = 2; // { and }
        for (size_t i = 0; i < size; ++i) {
            sum += schema->key_lengths[i] + 3; // "":
            sum += Config::apply_visitor(string_length_visitor{}, slot->values[i].value);
        }
        sum += size - 1; // ,
        return sum;
    }

    size_t operator()(const Config::ConfUnion &x)
    {
        return Config::apply_visitor(string_length_visitor{}, x.getVal()->value) + estimate_chars_per_uint(x.getTag()) + 3; // [,]
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
        const auto *slot = x.getSlot();
        const auto *schema = slot->schema;
        const auto size = schema->length;

        size_t sum = 0;
        for (size_t i = 0; i < size; ++i) {
            if (!zero_copy)
                sum += schema->key_lengths[i] + 1;

            sum += Config::apply_visitor(json_length_visitor{zero_copy}, slot->values[i].value);
        }
        return sum + JSON_OBJECT_SIZE(size);
    }

    size_t operator()(const Config::ConfUnion &x)
    {
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
            // Must always call getVal() because a nested array might grow and trigger a slot array move that would invalidate any kept reference on the outer array.
            String inner_error = Config::apply_visitor(from_json{arr[i], force_same_keys, permit_null_updates, false}, (*x.getVal())[i].value);
            if (inner_error != "")
                return String("[") + i + "] " + inner_error;
        }

        return "";
    }
    String operator()(Config::ConfObject &x)
    {
        if (json_node.isNull())
            return permit_null_updates ? "" : "Null updates not permitted.";

        const auto size = x.getSlot()->schema->length;

        {
            const auto *slot = x.getSlot();
            const auto *schema = slot->schema;
            // If a user passes a non-object to an API that expects an object with exactly one member
            // Try to use the non-object as value for the single member.
            // This allows calling for example evse/external_current_update with the payload 8000 instead of {"current": 8000}
            // Only allow this if the omitted key is not the confirm key.
            if (!json_node.is<JsonObject>() && is_root && size == 1 && Config::ConfirmKey() != schema->keys[0]) {
                String inner_error = Config::apply_visitor(from_json{json_node, force_same_keys, permit_null_updates, false}, x.getSlot()->values[0].value);
                if (inner_error != "")
                    return String("(inferred) [\"") + schema->keys[0] + "\"] " + inner_error + "\n";
                else
                    return inner_error;
            }
        }

        if (!json_node.is<JsonObject>())
            return "JSON node was not an object.";

        const JsonObject obj = json_node.as<JsonObject>();

        String return_str = "";
        bool more_errors = false;

        for (size_t i = 0; i < size; ++i) {
            // Don't cache x.getSlot(): The recursive visitor can reallocate slot buffers which invalidates the returned pointer!
            const auto *slot = x.getSlot();
            const auto *key = slot->schema->keys[i];

            if (!obj.containsKey(key))
            {
                if (!force_same_keys)
                    continue;

                if (return_str.length() < 1000)
                    return_str += String("JSON object is missing key '") + key + "'\n";
                else
                    more_errors = true;
            }

            String inner_error = Config::apply_visitor(from_json{obj[key], force_same_keys, permit_null_updates, false}, slot->values[i].value);
            if(obj.size() > 0)
                obj.remove(key);
            if (inner_error != "")
            {
                if (return_str.length() < 1000)
                    return_str += String("[\"") + key + "\"] " + inner_error + "\n";
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
                return_str += "More errors occurred that got filtered out.\n";
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
            // Must always call getVal() because a nested array might grow and trigger a slot array move that would invalidate any kept reference on the outer array.
            String inner_error = Config::apply_visitor(from_update{&arr->elements[i]}, (*x.getVal())[i].value);
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

        const auto size = x.getSlot()->schema->length;

        const auto &obj_elements = obj->elements;
        const auto obj_size = obj_elements.size();
        if (obj_size != size)
            return String("ConfUpdate object had ") + obj_size + " entries instead of the expected " + size;

        for (size_t i = 0; i < size; ++i) {
            size_t obj_idx = 0xFFFFFFFF;
            // Don't cache x.getSlot(): The recursive visitor can reallocate slot buffers which invalidates the returned pointer!
            const char *key = x.getSlot()->schema->keys[i];
            Config &value = x.getSlot()->values[i];
            for (size_t j = 0; j < size; ++j) {
                if (obj_elements[j].first != key)
                    continue;
                obj_idx = j;
                break;
            }
            if (obj_idx == 0xFFFFFFFF)
                return String("Key ") + key + String("not found in ConfUpdate object");

            String inner_error = Config::apply_visitor(from_update{&obj_elements[obj_idx].second}, value.value);
            if (inner_error != "")
                return String("[\"") + key + "\"] " + inner_error;
        }

        return "";
    }
    String operator()(Config::ConfUnion &x)
    {
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
    uint8_t operator()(const Config::ConfString &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfFloat &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfInt &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfUint &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfBool &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfVariant::Empty &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfArray &x) const
    {
        uint8_t result = 0;
        for (const Config &c : *x.getVal()) {
            result |= c.value.updated & api_backend_flag;
            result |= Config::apply_visitor(is_updated{api_backend_flag}, c.value);
        }
        return result;
    }
    uint8_t operator()(const Config::ConfObject &x) const
    {
        const auto *slot = x.getSlot();
        const auto size = slot->schema->length;

        uint8_t result = 0;
        for (size_t i = 0; i < size; ++i) {
            result |= slot->values[i].value.updated & api_backend_flag;
            result |= Config::apply_visitor(is_updated{api_backend_flag}, slot->values[i].value);
        }
        return result;
    }
    uint8_t operator()(const Config::ConfUnion &x) const
    {
        const auto &value = x.getVal()->value;
        return (value.updated & api_backend_flag) | Config::apply_visitor(is_updated{api_backend_flag}, value);
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
        const auto *slot = x.getSlot();
        const auto size = slot->schema->length;

        for (size_t i = 0; i < size; ++i) {
            slot->values[i].value.updated &= ~api_backend_flag;
            Config::apply_visitor(set_updated_false{api_backend_flag}, slot->values[i].value);
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

struct to_owned {
    OwnedConfig operator()(const Config::ConfString &x)
    {
        return OwnedConfig{*x.getVal()};
    }
    OwnedConfig operator()(const Config::ConfFloat &x)
    {
        return OwnedConfig{*x.getVal()};
    }
    OwnedConfig operator()(const Config::ConfInt &x)
    {
        return OwnedConfig{*x.getVal()};
    }
    OwnedConfig operator()(const Config::ConfUint &x)
    {
        return OwnedConfig{*x.getVal()};
    }
    OwnedConfig operator()(const Config::ConfBool &x)
    {
        return OwnedConfig{*x.getVal()};
    }
    OwnedConfig operator()(const Config::ConfVariant::Empty &x)
    {
        return OwnedConfig{nullptr};
    }
    OwnedConfig operator()(const Config::ConfArray &x)
    {
        std::vector<OwnedConfig> result;

        for (const Config &c : *x.getVal()) {
            result.push_back(Config::apply_visitor(to_owned{}, c.value));
        }

        return OwnedConfig{OwnedConfig::OwnedConfigArray{result}};
    }
    OwnedConfig operator()(const Config::ConfObject &x)
    {
        std::vector<std::pair<String, OwnedConfig>> result;

        const auto *slot = x.getSlot();
        const auto *schema = slot->schema;
        const auto size = schema->length;

        for (size_t i = 0; i < size; ++i) {
            result.push_back({String(schema->keys[i], schema->key_lengths[i]), Config::apply_visitor(to_owned{}, slot->values[i].value)});
        }

        return OwnedConfig{OwnedConfig::OwnedConfigObject{result}};
    }
    OwnedConfig operator()(const Config::ConfUnion &x)
    {
        std::vector<OwnedConfig> result;
        result.reserve(1);
        result.push_back(Config::apply_visitor(to_owned{}, x.getVal()->value));
        result.shrink_to_fit();
        return OwnedConfig(OwnedConfig::OwnedConfigUnion{x.getTag(), result});
    }
};
