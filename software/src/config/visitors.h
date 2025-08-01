/* esp32-firmware
 * Copyright (C) 2020-2024 Erik Fleckstein <erik@tinkerforge.com>
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

#include "config/private.h"

#include "header_logger.h"

#include "tools.h"

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

    String operator()(const Config::ConfInt52 &x) const
    {
        const auto *slot = x.getSlot();
        const auto val = *x.getVal();

        if (val < slot->min)
            return String("Integer value ") + val + " was less than the allowed minimum of " + slot->min;
        if (val > slot->max)
            return String("Integer value ") + val + " was more than the allowed maximum of " + slot->max;
        return "";
    }

    String operator()(const Config::ConfUint53 &x) const
    {
        const auto *slot = x.getSlot();
        const auto val = *x.getVal();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
        if (val < slot->min)
#pragma GCC diagnostic pop
            return String("Unsigned integer value ") + val + " was less than the allowed minimum of " + slot->min;
        if (val > slot->max)
            return String("Unsigned integer value ") + val + " was more than the allowed maximum of " + slot->max;
        return "";
    }

    String operator()(const Config::ConfUint16 &x) const
    {
        return "";
    }

    String operator()(const Config::ConfInt16 &x) const
    {
        return "";
    }

    String operator()(const Config::ConfUint8 &x) const
    {
        if (x.value > x.max)
            return String("Unsigned integer value ") + x.value + " was more than the allowed maximum of " + x.max;
        return "";
    }

    String operator()(const Config::ConfInt8 &x) const
    {
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
        const size_t size = val->size();
        const auto variantType = x.getVariantType();

        if (slot->maxElements > 0 && size > slot->maxElements)
            return String("Array had ") + size + " entries, but only " + slot->maxElements + " are allowed.";
        if (slot->minElements > 0 && size < slot->minElements)
            return String("Array had ") + size + " entries, but at least " + slot->maxElements + " are required.";

        if (variantType >= 0)
            for (size_t i = 0; i < size; ++i)
                if ((int)x.get(i)->value.tag != variantType)
                    return String("[") + i + "] has wrong type";

        size_t i = 0;
        for (const Config &elem : *val) {
            String err = Config::apply_visitor(default_validator{}, elem.value);
            if (!err.isEmpty())
                return String("[") + i + "] " + err;
            ++i;
        }

        return "";
    }

    String operator()(const Config::ConfObject &x) const
    {
        for (size_t i = 0; i < x.getSlot()->schema->length; ++i) {
            String err = Config::apply_visitor(default_validator{}, x.getSlot()->values[i].value);

            if (!err.isEmpty())
                return String("[\"") + x.getSlot()->schema->keys[i].val + "\"] " + err;
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
        insertHere.set(x.getVal());
    }
    void operator()(const Config::ConfInt &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfUint &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfInt52 &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfUint53 &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfUint16 &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfInt16 &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfUint8 &x)
    {
        insertHere.set(*x.getVal());
    }
    void operator()(const Config::ConfInt8 &x)
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
            const char *key = schema->keys[i].val;
            const Config &child = slot->values[i];

            bool censored = false;
            for (size_t ktc = 0; ktc < keys_to_censor_len; ++ktc) {
                // We've made sure that both keys point to _rodata.
                // Comparing the pointers should be enough, assuming that the literal strings are deduplicated perfectly.
                if (key != keys_to_censor[ktc])
                    continue;

                if (!(child.is<Config::ConfString>() && child.asString().length() == 0)) {
                    obj[key] = nullptr;
                    censored = true;
                    break;
                }
            }
            if (censored)
                continue;

            if (child.is<Config::ConfObject>()) {
                obj.createNestedObject(key);
            } else if (child.is<Config::ConfArray>() || child.is<Config::ConfUnion>()) {
                obj.createNestedArray(key);
            } else {
                obj[key] = nullptr;
            }

            Config::apply_visitor(to_json{obj[key], keys_to_censor, keys_to_censor_len}, child.value);
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
    const char *const *keys_to_censor;
    size_t keys_to_censor_len;
};

static const uint8_t leading_zeros_to_char_count[33] = {10,10,10,9,9,9,8,8,8,7,7,7,7,6,6,6,5,5,5,4,4,4,4,3,3,3,2,2,2,1,1,1,1};

// Never underestimates length. Overestimates by 0.12 chars on average.
// Tested against the returned length of snprintf([...],"%u") for every 32 bit number.
static size_t estimate_chars_per_uint(uint32_t v)
{
#if !defined(__XTENSA_EL__) || __XTENSA_EL__ != 1
    // __builtin_clz(v) is undefined for v == 0. Set the lowest bit to work around that.
    // The workaround is not necessary on Xtensa, which uses the NSAU instruction that handles 0 correctly.
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
    // The workaround is not necessary on Xtensa, which uses the NSAU instruction that handles 0 correctly.
    v |= 1;
#endif

    return leading_zeros_to_char_count[__builtin_clz(static_cast<uint32_t>(v))] + sign;
}

static size_t estimate_chars_per_int64(  int64_t v) {return 21;}
static size_t estimate_chars_per_uint64(uint64_t v) {return 20;}

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
        return std::max(estimate_chars_per_int(x.getSlot()->max),
                        estimate_chars_per_int(x.getSlot()->min));
    }
    size_t operator()(const Config::ConfUint &x)
    {
        return estimate_chars_per_uint(x.getSlot()->max);
    }
    size_t operator()(const Config::ConfInt52 &x)
    {
        return std::max(estimate_chars_per_int64(x.getSlot()->max),
                        estimate_chars_per_int64(x.getSlot()->min));
    }
    size_t operator()(const Config::ConfUint53 &x)
    {
        return estimate_chars_per_uint64(x.getSlot()->max);
    }
    size_t operator()(const Config::ConfUint16 &x)
    {
        return 5;
    }
    size_t operator()(const Config::ConfInt16 &x)
    {
        return 6;
    }
    size_t operator()(const Config::ConfUint8 &x)
    {
        return estimate_chars_per_uint(x.max);
    }
    size_t operator()(const Config::ConfInt8 &x)
    {
        return 4;
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
            sum += schema->keys[i].length + 3; // "":
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
    size_t operator()(const Config::ConfInt52 &x)
    {
        return estimate_chars_per_int64(*x.getVal());
    }
    size_t operator()(const Config::ConfUint53 &x)
    {
        return estimate_chars_per_uint64(*x.getVal());
    }
    size_t operator()(const Config::ConfUint16 &x)
    {
        return estimate_chars_per_uint(*x.getVal());
    }
    size_t operator()(const Config::ConfInt16 &x)
    {
        return estimate_chars_per_int(*x.getVal());
    }
    size_t operator()(const Config::ConfUint8 &x)
    {
        return estimate_chars_per_uint(*x.getVal());
    }
    size_t operator()(const Config::ConfInt8 &x)
    {
        return estimate_chars_per_int(*x.getVal());
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
            sum += schema->keys[i].length + 3; // "":
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
    size_t operator()(const Config::ConfInt52 &x)
    {
        return 0;
    }
    size_t operator()(const Config::ConfUint53 &x)
    {
        return 0;
    }
    size_t operator()(const Config::ConfUint16 &x)
    {
        return 0;
    }
    size_t operator()(const Config::ConfInt16 &x)
    {
        return 0;
    }
    size_t operator()(const Config::ConfUint8 &x)
    {
        return 0;
    }
    size_t operator()(const Config::ConfInt8 &x)
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
                sum += schema->keys[i].length + 1;

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

struct UpdateResult {
    String message;
    bool changed;
};

struct from_json {
    UpdateResult operator()(Config::ConfString &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<String>())
            return {"JSON node was not a string.", false};

        bool changed = *x.getVal() != json_node.as<CoolString>();
        *x.getVal() = json_node.as<CoolString>();
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfFloat &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<float>())
            return {"JSON node was not a float.", false};

        bool changed = x.getVal() != json_node.as<float>();
        x.setVal(json_node.as<float>());
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfInt &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<int32_t>())
            return {"JSON node was not a signed integer.", false};

        bool changed = *x.getVal() != json_node.as<int32_t>();
        *x.getVal() = json_node.as<int32_t>();
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfUint &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<uint32_t>())
            return {"JSON node was not an unsigned integer.", false};

        bool changed = *x.getVal() != json_node.as<uint32_t>();
        *x.getVal() = json_node.as<uint32_t>();
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfInt52 &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<int64_t>())
            return {"JSON node was not a signed integer.", false};

        bool changed = *x.getVal() != json_node.as<int64_t>();
        *x.getVal() = json_node.as<int64_t>();
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfUint53 &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<uint64_t>())
            return {"JSON node was not an unsigned integer.", false};

        bool changed = *x.getVal() != json_node.as<uint64_t>();
        *x.getVal() = json_node.as<uint64_t>();
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfUint16 &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<uint16_t>())
            return {"JSON node was not an unsigned integer or too big (not in 0 - 65535).", false};

        bool changed = *x.getVal() != json_node.as<uint16_t>();
        *x.getVal() = json_node.as<uint16_t>();
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfInt16 &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<int16_t>())
            return {"JSON node was not a signed integer or too big (not in -32768 - 32767).", false};

        bool changed = *x.getVal() != json_node.as<int16_t>();
        *x.getVal() = json_node.as<int16_t>();
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfUint8 &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<uint8_t>())
            return {"JSON node was not an unsigned integer or too big (not in 0 - 255).", false};

        bool changed = *x.getVal() != json_node.as<uint8_t>();
        *x.getVal() = json_node.as<uint8_t>();
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfInt8 &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<int8_t>())
            return {"JSON node was not a signed integer or too big (not in -128 - 127).", false};

        bool changed = *x.getVal() != json_node.as<int8_t>();
        *x.getVal() = json_node.as<int8_t>();
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfBool &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<bool>())
            return {"JSON node was not a boolean.", false};

        bool changed = x.value != json_node.as<bool>();
        x.value = json_node.as<bool>();
        return {"", changed};
    }
    UpdateResult operator()(const Config::ConfVariant::Empty &x)
    {
        if (json_node.isNull())
            return {"", false};
        if (json_node == "" || json_node == false || json_node == 0)
            return {"", false};
        if (json_node.size() == 0 && (json_node.is<JsonArray>() || json_node.is<JsonObject>()))
            return {"", false};

        return {"JSON null node was not null or a falsy value. Use null, \"\", false, 0, [] or {}.", false};
    }
    UpdateResult operator()(Config::ConfArray &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<JsonArray>())
            return {"JSON node was not an array.", false};

        JsonArray arr = json_node.as<JsonArray>();
        auto arr_size = arr.size();

        auto *val = x.getVal();
        const auto old_size = val->size();

        bool changed = false;

        if (arr_size != old_size) {
            changed = true;
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
            auto res = Config::apply_visitor(from_json{arr[i], force_same_keys, permit_null_updates, false}, (*x.getVal())[i].value);
            if (!res.message.isEmpty())
                return {String("[") + i + "] " + res.message, false};
            (*x.getVal())[i].set_updated(res.changed ? 0xFF : 0);
            changed |= res.changed;
        }

        return {"", changed};
    }
    UpdateResult operator()(Config::ConfObject &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        const auto size = x.getSlot()->schema->length;

        {
            const auto *slot = x.getSlot();
            const auto *schema = slot->schema;
            // If a user passes a non-object to an API that expects an object with exactly one member
            // Try to use the non-object as value for the single member.
            // This allows calling for example evse/external_current_update with the payload 8000 instead of {"current": 8000}
            // Only allow this if the omitted key is not the confirm key.
            if (!json_node.is<JsonObject>() && is_root && size == 1 && Config::ConfirmKey() != schema->keys[0].val) {
                auto res =  Config::apply_visitor(from_json{json_node, force_same_keys, permit_null_updates, false}, x.getSlot()->values[0].value);
                if (!res.message.isEmpty())
                    return {String("(inferred) [\"") + schema->keys[0].val + "\"] " + res.message + "\n", false};
                else {
                    x.getSlot()->values[0].set_updated(res.changed ? 0xFF : 0);
                    return res;
                }
            }
        }

        if (!json_node.is<JsonObject>())
            return {"JSON node was not an object.", false};

        const JsonObject obj = json_node.as<JsonObject>();

        String return_str = "";
        bool more_errors = false;

        bool changed = false;

        for (size_t i = 0; i < size; ++i) {
            // Don't cache x.getSlot(): The recursive visitor can reallocate slot buffers which invalidates the returned pointer!
            const auto *slot = x.getSlot();
            const auto *key = slot->schema->keys[i].val;

            if (!obj.containsKey(key)) {
                if (!force_same_keys)
                    continue;

                if (return_str.length() < 1000)
                    return_str += String("JSON object is missing key '") + key + "'\n";
                else
                    more_errors = true;
            }

            auto res = Config::apply_visitor(from_json{obj[key], force_same_keys, permit_null_updates, false}, slot->values[i].value);
            if (obj.size() > 0)
                obj.remove(key);
            if (!res.message.isEmpty()) {
                if (return_str.length() < 1000)
                    return_str += String("[\"") + key + "\"] " + res.message + "\n";
                else
                    more_errors = true;
            }
            changed |= res.changed;
            // Don't use slot: Could already be invalidated by Config::apply_visitor!
            x.getSlot()->values[i].set_updated(res.changed ? 0xFF : 0);
        }

        if (force_same_keys) {
            for (auto i = obj.begin(); i != obj.end(); i += 1) {
                if (return_str.length() < 1000)
                    return_str += String("JSON object has unknown key '") + i->key().c_str() + "'.\n";
                else
                    more_errors = true;
            }
        }

        if (return_str.length() > 0) {
            if (more_errors)
                return_str += "More errors occurred that got filtered out.\n";
            return {return_str, false};
        }

        return {"", changed};
    }

    UpdateResult operator()(Config::ConfUnion &x)
    {
        if (json_node.isNull())
            return {permit_null_updates ? "" : "Null updates not permitted.", false};

        if (!json_node.is<JsonArray>())
            return {"JSON node was not an array.", false};

        JsonArray arr = json_node.as<JsonArray>();

        if (arr.size() != 2) {
            return {"JSON array had length != 2.", false};
        }

        uint8_t old_tag = x.getTag();
        uint8_t new_tag = old_tag;

        bool changed = false;

        const auto &arr_first = arr[0];
        if (arr_first.isNull()) {
            if (!permit_null_updates)
                return {"[0] Null updates not permitted", false};
        }
        else if (arr_first.is<uint8_t>()) {
            new_tag = arr_first.as<uint8_t>();
        } else {
            return {"[0] JSON node was not an unsigned integer.", false};
        }

        if (new_tag != old_tag) {
            changed = true;
            if (!x.changeUnionVariant(new_tag))
                return {String("[0] Unknown union tag: ") + new_tag, false};
        }

        // We can't just return res because we could have changed the tag above.
        auto res = Config::apply_visitor(from_json{arr[1], force_same_keys, permit_null_updates, false}, x.getVal()->value);
        if (!res.message.isEmpty())
            return res;

        x.getVal()->set_updated(res.changed ? 0xFF : 0);
        return {res.message, changed || res.changed};
    }

    const JsonVariant json_node;
    bool force_same_keys;
    bool permit_null_updates;
    bool is_root;
};


template<typename T>
bool extract_int(const Config::ConfUpdate *update, T *result, T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max()) {
    int64_t x = 0;
    if      (update->get< int8_t >() != nullptr) { x = *update->get< int8_t >(); }
    else if (update->get<uint8_t >() != nullptr) { x = *update->get<uint8_t >(); }
    else if (update->get< int16_t>() != nullptr) { x = *update->get< int16_t>(); }
    else if (update->get<uint16_t>() != nullptr) { x = *update->get<uint16_t>(); }
    else if (update->get< int32_t>() != nullptr) { x = *update->get< int32_t>(); }
    else if (update->get<uint32_t>() != nullptr) { x = *update->get<uint32_t>(); }
    else if (update->get< int64_t>() != nullptr) { x = *update->get< int64_t>(); }
    else if (update->get<uint64_t>() != nullptr) { x = *update->get<uint64_t>(); }
    else {
        return false;
    }

    if (x < (int64_t)min || x > (int64_t)max)
        header_printfln("from_update", "ConfUpdate int %lld out of range: allowed are [%lld; %lld]", x, (int64_t)min, (int64_t)max);

    *result = (T) x;
    return true;
}

struct from_update {
    UpdateResult operator()(Config::ConfString &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        const auto *update_val = update->get<CoolString>();
        if (update_val == nullptr)
            return {"ConfUpdate node was not a string.", false};

        bool changed = *x.getVal() != *update_val;
        *x.getVal() = *update_val;
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfFloat &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        const auto *update_val = update->get<float>();
        if (update_val == nullptr)
            return {"ConfUpdate node was not a float.", false};

        bool changed = x.getVal() != *update_val;
        x.setVal(*update_val);
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfInt &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        int32_t new_value = 0;
        if (!extract_int<int32_t>(update, &new_value, ((const Config::ConfInt &)x).getSlot()->min, ((const Config::ConfInt &)x).getSlot()->max))
            return {"ConfUpdate node was not a signed integer.", false};

        bool changed = *x.getVal() != new_value;
        *x.getVal() = new_value;
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfUint &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        uint32_t new_value = 0;
        if (!extract_int<uint32_t>(update, &new_value, ((const Config::ConfInt &)x).getSlot()->min, ((const Config::ConfInt &)x).getSlot()->max))
            return {"ConfUpdate node was not an unsigned integer.", false};

        bool changed = *x.getVal() != new_value;
        *x.getVal() = new_value;
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfInt52 &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        int64_t new_value = 0;
        if (!extract_int<int64_t>(update, &new_value, -9007199254740991l, 9007199254740991l)) // FIXME: those are +-2^53! Maybe remove ConfUint53 and change ConfInt52 to 53
            return {"ConfUpdate node was not a signed integer.", false};

        bool changed = *x.getVal() != new_value;
        *x.getVal() = new_value;
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfUint53 &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        uint64_t new_value = 0;
        if (!extract_int<uint64_t>(update, &new_value, 0, 9007199254740991l)) // FIXME: those are +-2^53! Maybe remove ConfUint53 and change ConfInt52 to 53
            return {"ConfUpdate node was not a signed integer.", false};

        bool changed = *x.getVal() != new_value;
        *x.getVal() = new_value;
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfUint16 &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        uint16_t new_value = 0;
        if (!extract_int<uint16_t>(update, &new_value))
            return {"ConfUpdate node was not an unsigned integer.", false};

        bool changed = *x.getVal() != new_value;
        *x.getVal() = new_value;
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfInt16 &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        int16_t new_value = 0;
        if (!extract_int<int16_t>(update, &new_value))
            return {"ConfUpdate node was not an unsigned integer.", false};

        bool changed = *x.getVal() != new_value;
        *x.getVal() = new_value;
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfUint8 &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        uint8_t new_value = 0;
        if (!extract_int<uint8_t>(update, &new_value))
            return {"ConfUpdate node was not an unsigned integer.", false};

        bool changed = *x.getVal() != new_value;
        *x.getVal() = new_value;
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfInt8 &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        int8_t new_value = 0;
        if (!extract_int<int8_t>(update, &new_value))
            return {"ConfUpdate node was not a signed integer.", false};

        bool changed = *x.getVal() != new_value;
        *x.getVal() = new_value;
        return {"", changed};
    }
    UpdateResult operator()(Config::ConfBool &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        const auto *update_val = update->get<bool>();
        if (update_val == nullptr)
            return {"ConfUpdate node was not a boolean.", false};

        bool changed = x.value != *update_val;
        x.value = *update_val;
        return {"", changed};
    }
    UpdateResult operator()(const Config::ConfVariant::Empty &x)
    {
        return {Config::containsNull(update) ? "" : "JSON null node was not null", false};
    }
    UpdateResult operator()(Config::ConfArray &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        const Config::ConfUpdateArray *arr = update->get<Config::ConfUpdateArray>();
        if (arr == nullptr)
            return {"ConfUpdate node was not an array.", false};

        const auto arr_size = arr->elements.size();
        auto *val = x.getVal();
        const auto old_size = val->size();

        bool changed = false;

        if (arr_size != old_size) {
            changed = true;
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
            auto res = Config::apply_visitor(from_update{&arr->elements[i]}, (*x.getVal())[i].value);
            if (!res.message.isEmpty())
                return {String("[") + i + "] " + res.message, false};

            (*x.getVal())[i].set_updated(res.changed ? 0xFF : 0);
            changed |= res.changed;
        }

        return {"", changed};
    }
    UpdateResult operator()(Config::ConfObject &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        const Config::ConfUpdateObject *obj = update->get<Config::ConfUpdateObject>();
        if (obj == nullptr) {
            return {StringSumHelper("ConfUpdate node was variant ") + update->which() + ", not an object.", false};
        }

        const auto size = x.getSlot()->schema->length;

        const auto &obj_elements = obj->elements;
        const auto obj_size = obj_elements.size();

        bool changed = false;

        if (obj_size != size)
            return {String("ConfUpdate object had ") + obj_size + " entries instead of the expected " + size, false};

        for (size_t i = 0; i < size; ++i) {
            size_t obj_idx = 0xFFFFFFFF;
            // Don't cache x.getSlot(): The recursive visitor can reallocate slot buffers which invalidates the returned pointer!
            const char *key = x.getSlot()->schema->keys[i].val;
            Config &value = x.getSlot()->values[i];
            for (size_t j = 0; j < size; ++j) {
                if (obj_elements[j].first != key)
                    continue;
                obj_idx = j;
                break;
            }
            if (obj_idx == 0xFFFFFFFF)
                return {String("Key ") + key + String("not found in ConfUpdate object"), false};

            auto res = Config::apply_visitor(from_update{&obj_elements[obj_idx].second}, value.value);
            if (!res.message.isEmpty())
                return {String("[\"") + key + "\"] " + res.message, false};

            changed |= res.changed;
            x.getSlot()->values[i].set_updated(res.changed ? 0xFF : 0);
        }

        return {"", changed};
    }
    UpdateResult operator()(Config::ConfUnion &x)
    {
        if (Config::containsNull(update))
            return {"", false};

        const Config::ConfUpdateUnion *un = update->get<Config::ConfUpdateUnion>();
        if (un == nullptr) {
            return {StringSumHelper("ConfUpdate node was variant ") + update->which() + ", not a union.", false};
        }

        bool changed = false;

        if (un->tag != as_const(x).getSlot()->tag) {
            changed = true;
            x.changeUnionVariant(un->tag);
        }

        // We can't just return res because we could have changed the tag above.
        auto res = Config::apply_visitor(from_update{&un->value}, x.getVal()->value);
        if (!res.message.isEmpty())
            return {res.message, false};

        return {res.message, changed || res.changed};
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
    uint8_t operator()(const Config::ConfInt52 &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfUint53 &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfUint16 &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfInt16 &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfUint8 &x)
    {
        return 0;
    }
    uint8_t operator()(const Config::ConfInt8 &x)
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
    void operator()(Config::ConfInt52 &x)
    {
    }
    void operator()(Config::ConfUint53 &x)
    {
    }
    void operator()(Config::ConfUint16 &x)
    {
    }
    void operator()(Config::ConfInt16 &x)
    {
    }
    void operator()(Config::ConfUint8 &x)
    {
    }
    void operator()(Config::ConfInt8 &x)
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

#ifdef DEBUG_FS_ENABLE
struct api_info {
    void operator()(const Config::ConfString &x)
    {
        sw.printf("{\"type\":\"string\",\"val\":\"%s\",\"minChars\":%hu,\"maxChars\":%hu}", x.getSlot()->val.c_str(), x.getSlot()->minChars, x.getSlot()->maxChars);
    }
    void operator()(const Config::ConfFloat &x)
    {
        sw.printf("{\"type\":\"float\",\"val\":%f}", isnan(x.getVal()) ? 0.0f : x.getVal());
    }
    void operator()(const Config::ConfInt &x)
    {
        sw.printf("{\"type\":\"int\",\"val\":%li,\"min\":%li,\"max\":%li}", x.getSlot()->val, x.getSlot()->min, x.getSlot()->max);
    }
    void operator()(const Config::ConfUint &x)
    {
        sw.printf("{\"type\":\"uint\",\"val\":%lu,\"min\":%lu,\"max\":%lu}", x.getSlot()->val, x.getSlot()->min, x.getSlot()->max);
    }
    void operator()(const Config::ConfInt52 &x)
    {
        sw.printf("{\"type\":\"int64\",\"val\":%lli,\"min\":%lli,\"max\":%lli}", x.getSlot()->val, x.getSlot()->min, x.getSlot()->max);
    }
    void operator()(const Config::ConfUint53 &x)
    {
        sw.printf("{\"type\":\"uint64\",\"val\":%llu,\"min\":%llu,\"max\":%llu}", x.getSlot()->val, x.getSlot()->min, x.getSlot()->max);
    }
    void operator()(const Config::ConfUint16 &x)
    {
        sw.printf("{\"type\":\"uint\",\"val\":%hu,\"min\":0,\"max\":65535}", *x.getVal());
    }
    void operator()(const Config::ConfInt16 &x)
    {
        sw.printf("{\"type\":\"int\",\"val\":%hi,\"min\":-32768,\"max\":32767}", *x.getVal());
    }
    void operator()(const Config::ConfUint8 &x)
    {
        sw.printf("{\"type\":\"uint\",\"val\":%hhu,\"min\":0,\"max\":%hhu}", *x.getVal(), x.max);
    }
    void operator()(const Config::ConfInt8 &x)
    {
        sw.printf("{\"type\":\"int\",\"val\":%hhi,\"min\":-128,\"max\":127}", *x.getVal());
    }
    void operator()(const Config::ConfBool &x)
    {
        sw.printf("{\"type\":\"bool\",\"val\":%s}", x.value ? "true" : "false");
    }
    void operator()(const Config::ConfVariant::Empty &x)
    {
        sw.printf("{\"type\":\"null\"}");
    }
    void operator()(const Config::ConfArray &x)
    {
        sw.printf("{\"type\":\"array\",\"prototype\":");
        Config::apply_visitor(api_info{sw}, x.getSlot()->prototype->value);
        sw.printf(",\"minElements\":%u,\"maxElements\":%u,\"variantType\":%d,\"content\":[", x.getSlot()->minElements, x.getSlot()->maxElements, x.getVariantType());
        bool first = true;
        for (const Config &c : *x.getVal()) {
            if (!first) {
                sw.printf(",");
            }
            first = false;
            Config::apply_visitor(api_info{sw}, c.value);
        }
        sw.printf("]}");
    }
    void operator()(const Config::ConfObject &x)
    {
        const auto *slot = x.getSlot();
        const auto size = slot->schema->length;

        sw.printf("{\"type\":\"object\",\"length\":%u,\"entries\":[", size);

        bool first = true;
        for (size_t i = 0; i < size; ++i) {
            if (!first) {
                sw.printf(",");
            }
            first = false;
            sw.printf("{\"key\":\"%.*s\",\"value\":", slot->schema->keys[i].length, slot->schema->keys[i].val);
            Config::apply_visitor(api_info{sw}, slot->values[i].value);
            sw.printf("}");
        }
        sw.printf("]}");
    }
    void operator()(const Config::ConfUnion &x)
    {
        sw.printf("{\"type\":\"union\",\"tag\":%u,\"prototypes_len\":%u,\"prototypes\":[", x.getSlot()->tag, x.getSlot()->prototypes_len);

        bool first = true;
        for (size_t i = 0; i < x.getSlot()->prototypes_len; ++i) {
            if (!first) {
                sw.printf(",");
            }
            first = false;
            sw.printf("{\"tag\": %u,\"value\":", x.getSlot()->prototypes[i].tag);
            Config::apply_visitor(api_info{sw}, x.getSlot()->prototypes[i].config.value);
            sw.printf("}");
        }
        sw.printf("],\"val\":");

        auto &value = x.getVal()->value;
        Config::apply_visitor(api_info{sw}, value);
        sw.printf("}");
    }

    StringWriter &sw;
};
#endif
