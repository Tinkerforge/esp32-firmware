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

#pragma once

#include <vector>

#include "ArduinoJson.h"
#include "FS.h"

#include "event_log.h"

#define STRICT_VARIANT_ASSUME_MOVE_NOTHROW true
#include "strict_variant/variant.hpp"
#include "strict_variant/mpl/find_with.hpp"

extern EventLog logger;

struct Config {
    struct ConfString {
        String value;
        uint16_t minChars;
        uint16_t maxChars;
    };

    struct ConfFloat {
        float value;
        float min;
        float max;
    };

    struct ConfInt {
        int32_t value;
        int32_t min;
        int32_t max;
    };

    struct ConfUint {
        uint32_t value;
        uint32_t min;
        uint32_t max;
    };

    struct ConfBool {
        bool value;
    };

    struct ConfArray {
        std::vector<Config> value;
        Config *prototype;
        uint32_t minElements : 12, maxElements : 12;
        int8_t variantType;

        Config *get(uint16_t i);
        const Config *get(uint16_t i) const;
    };

    struct ConfObject {
        std::vector<std::pair<String, Config>> value;

        Config *get(String s);
        const Config *get(String s) const;
    };

    struct ConfUpdateArray;
    struct ConfUpdateObject;

    typedef strict_variant::variant<
        std::nullptr_t, // DON'T MOVE THIS!
        String,
        float,
        uint32_t,
        int32_t,
        bool,
        ConfUpdateArray,
        ConfUpdateObject
    > ConfUpdate;
    // This is necessary as we can't use get to distinguish between
    // a get<std::nullptr_t>() that returned nullptr because the variant
    // had another type and the same call that returned nullptr because
    // the variant has the std::nullptr_type and thus contains a nullptr.
    static bool containsNull(ConfUpdate *update)
    {
        return update->which() == 0;
    }

    struct ConfUpdateArray {
        std::vector<ConfUpdate> elements;
    };

    struct ConfUpdateObject {
        std::vector<std::pair<String, ConfUpdate>> elements;
    };

    typedef strict_variant::variant<
        ConfString,
        ConfFloat,
        ConfInt,
        ConfUint,
        ConfBool,
        ConfArray,
        ConfObject,
        std::nullptr_t
    > ConfVariant;

    ConfVariant value;
    uint8_t updated;

    bool was_updated(uint8_t api_backend_flag);
    void set_update_handled(uint8_t api_backend_flag);

    template<typename T>
    static int type_id()
    {
        constexpr std::size_t index = decltype(value)::find_which<T>::value;
        return index;
    }

    template<typename T>
    bool is() const
    {
        return value.which() == decltype(value)::find_which<T>::value;
    }

    static Config Str(String s,
                      uint16_t minChars = 0,
                      uint16_t maxChars = 0);

    static Config Float(float d,
                        float min = std::numeric_limits<float>::lowest(),
                        float max = std::numeric_limits<float>::max());

    static Config Int(int32_t i,
                      int32_t min = std::numeric_limits<int32_t>::lowest(),
                      int32_t max = std::numeric_limits<int32_t>::max());

    static Config Uint(uint32_t u,
                       uint32_t min = std::numeric_limits<uint32_t>::lowest(),
                       uint32_t max = std::numeric_limits<uint32_t>::max());

    static Config Bool(bool b);

    static Config Array(std::initializer_list<Config> arr,
                        Config *prototype,
                        size_t minElements,
                        size_t maxElements,
                        int variantType);

    static Config Object(std::initializer_list<std::pair<String, Config>> obj);

    static Config Null();

    static Config Uint8(uint8_t u);

    static Config Uint16(uint16_t u);

    static Config Uint32(uint32_t u);

    static Config Uint64(uint32_t u);

    static Config Int8(int8_t i);

    static Config Int16(int16_t i);

    static Config Int32(int32_t i);

    static Config Int64(int32_t i);

    Config *get(String s);

    Config *get(uint16_t i);

    const Config *get(String s) const;

    const Config *get(uint16_t i) const;

    bool add()
    {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to add to a node that is not an array!");
            delay(100);
            return false;
        }
        std::vector<Config> &children = strict_variant::get<Config::ConfArray>(&value)->value;
        children.push_back(*strict_variant::get<Config::ConfArray>(&value)->prototype);
        return true;
    }

    bool removeLast()
    {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to add to a node that is not an array!");
            delay(100);
            return false;
        }
        std::vector<Config> &children = strict_variant::get<Config::ConfArray>(&value)->value;
        if (children.size() == 0)
            return false;

        children.pop_back();
        return true;
    }

    bool remove(size_t i)
    {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to add to a node that is not an array!");
            delay(100);
            return false;
        }
        std::vector<Config> &children = strict_variant::get<Config::ConfArray>(&value)->value;

        if (children.size() <= i)
            return false;

        children.erase(children.begin() + i);
        return true;
    }

    ssize_t count()
    {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to get count of a node that is not an array!");
            delay(100);
            return -1;
        }
        const std::vector<Config> &children = strict_variant::get<Config::ConfArray>(&value)->value;
        return children.size();
    }

    template<typename T, typename ConfigT>
    T *as() {
        if (!this->is<ConfigT>()) {
            logger.printfln("as: Config has wrong type. This is %s", this->to_string().c_str());
            delay(100);
            return nullptr;
        }
        return &strict_variant::get<ConfigT>(&value)->value;
    }

    template<typename T, typename ConfigT>
    const T *as() const {
        if (!this->is<ConfigT>()) {
            logger.printfln("const as: Config has wrong type. This is %s", this->to_string().c_str());
            delay(100);
            return nullptr;
        }
        return &strict_variant::get<ConfigT>(&value)->value;
    }

    const String &asString() const;

    const char *asCStr() const;

    const float &asFloat() const;

    const uint32_t &asUint() const;

    const int32_t &asInt() const;

    const bool &asBool() const;

    std::vector<Config> &asArray();

    template<typename T, typename ConfigT>
    bool update_value(T value) {
        if (!this->is<ConfigT>()) {
            logger.printfln("update_value: Config has wrong type. This is %s. new value is %s", this->to_string().c_str(), String(value).c_str());
            delay(100);
            return false;
        }
        T *target = as<T, ConfigT>();
        T old_value = *target;
        *target = value;

        if (old_value != value)
            this->updated = 0xFF;

        return old_value != value;
    }

    bool updateString(String value)
    {
        return update_value<String, ConfString>(value);
    }

    bool updateInt(int32_t value)
    {
        return update_value<int32_t, ConfInt>(value);
    }

    bool updateUint(uint32_t value)
    {
        return update_value<uint32_t, ConfUint>(value);
    }

    bool updateFloat(float value)
    {
        return update_value<float, ConfFloat>(value);
    }

    bool updateBool(bool value)
    {
        return update_value<bool, ConfBool>(value);
    }

    template<typename T, typename ConfigT>
    size_t fillArray(T *arr, size_t elements) {
        if (!this->is<ConfArray>()) {
            logger.printfln("Can't fill array, Config is not an array");
            delay(100);
            return 0;
        }

        ConfArray *confArr = strict_variant::get<ConfArray>(&value);
        size_t toWrite = std::min(confArr->value.size(), elements);

        for (size_t i = 0; i < toWrite; ++i) {
            Config &entry = confArr->value[i];
            if (!entry.is<ConfigT>()) {
                logger.printfln("Config entry has wrong type.");
                delay(100);
                return 0;
            }
            arr[i] = strict_variant::get<ConfigT>(&entry.value)->value;
        }

        return toWrite;
    }

    size_t fillFloatArray(float *arr, size_t elements);

    size_t fillUint8Array(uint8_t *arr, size_t elements);
    size_t fillUint16Array(uint16_t *arr, size_t elements);
    size_t fillUint32Array(uint32_t *arr, size_t elements);
    size_t fillUint64Array(uint32_t *arr, size_t elements);

    size_t fillInt8Array(int8_t *arr, size_t elements);
    size_t fillInt16Array(int16_t *arr, size_t elements);
    size_t fillInt32Array(int32_t *arr, size_t elements);
    size_t fillInt64Array(int32_t *arr, size_t elements);
/*
    template<typename T, typename ConfigT>
    void fromArray(T *arr, size_t elements) {
        if (!this->is<ConfArray>()) {
            Serial.println("Can't from array, config is not an array");
            delay(100);
            return;
        }

        ConfArray *confArr = strict_variant::get<ConfArray>(&this->value);

        confArr->value.clear();
        for(size_t i = 0; i < elements; ++i) {
            confArr->value.push_back(confArr->prototype[0]);
            *confArr->value[i].as<T, ConfigT>() = arr[i];
            ConfigT *inner = strict_variant::get<ConfigT>(confArr->value[i]);
            String inner_error = inner->validator(*inner);
            if(inner_error != "") {
                //return String("[") + i + "]" + inner_error;
                Serial.println( String("[") + i + "]" + inner_error);
                delay(100);
                return;
            }
        }

        String error = confArr->validator(*confArr);
        if(error != "") {
            Serial.println(error);
            delay(100);
        }
    }
*/
    size_t json_size(bool zero_copy) const;
    size_t max_string_length() const;

    void save_to_file(File file);

    void write_to_stream(Print &output);
    void write_to_stream_except(Print &output, std::initializer_list<String> keys_to_censor);
    void write_to_stream_except(Print &output, const std::vector<String> &keys_to_censor);

    String to_string() const;
    String to_string_except(std::initializer_list<String> keys_to_censor) const;
    String to_string_except(const std::vector<String> &keys_to_censor) const;
};

struct ConfigRoot : public Config {
public:
    ConfigRoot() = default;

    ConfigRoot(Config cfg) : Config(cfg), validator(nullptr)
    {
    }

    ConfigRoot(Config cfg, std::function<String(Config &)> validator) : Config(cfg), validator(validator)
    {
    }

    std::function<String(Config &)> validator;
    bool permit_null_updates = true;

    String update_from_file(File file);

    String update_from_cstr(char *c, size_t payload_len);

    String update_from_string(String s);

    String update_from_json(JsonVariant root);

    String update(Config::ConfUpdate *val);

    String validate();
};

/*void test() {
    Config value = Config::Object({
        {"ssid", Config::Str("", 32)},
        {"bssid", Config::Array({
                Config::Uint8(0),
                Config::Uint8(1),
                Config::Uint8(2),
                Config::Uint8(3),
                Config::Uint8(4),
                Config::Uint8(5)
                },
                Config::Uint8(0),
                6,
                6,
                Config::type_id<Config::ConfUint>()
            )
        },
        {"bssid_lock", Config::Bool(false)},
        {"passphrase", Config::Str("", 64, [](const Config::ConfString &s) {
                return s.value.length() == 0 ||
                    (s.value.length() >= 8 && s.value.length() <= 63) || //FIXME: check if there are only ASCII characters here.
                    (s.value.length() == 64) ? String("") : String("passphrase must be of length zero, or 8 to 63, or 64 if PSK."); //FIXME: check if there are only hex digits here.
            })
        },
        {"ip", Config::Uint32(0)},
        {"gateway", Config::Uint32(0)},
        {"subnet", Config::Uint32(0)},
        {"dns", Config::Uint32(0)},
        {"dns2", Config::Uint32(0)},
    });
    bool result = strict_variant::apply_visitor(recursive_validator{}, value.value);
    Serial.println(result);

    const char* json = "{\"ssid\":\"01234567890123456789012345678901\",\"bssid\":[0,1,2,3,4,5],\"bssid_lock\":true,\"passphrase\":\"01234567\",\"ip\":0,\"gateway\":0,\"subnet\":0,\"dns\":0,\"dns2\":0}";
    const size_t capacity = JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(9) + 120;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, json);

    String error = strict_variant::apply_visitor(Config::from_json{doc.as<JsonVariant>()}, value.value);

    Serial.println(error);
    Serial.println(value.value.get<Config::ConfObject>()->value[0].second.value.get<Config::ConfString>()->value);


    Serial.printf("Capacity is %d, visited capacity is %d\n", capacity, value.json_size());

    DynamicJsonDocument doc2(value.json_size());

    JsonVariant var;
    if(value.is<Config::ConfObject>()) {
        var = doc2.to<JsonObject>();
    } else if(value.is<Config::ConfArray>()) {
        var = doc2.to<JsonArray>();
    } else {
        var = doc2.as<JsonVariant>();
    }
    strict_variant::apply_visitor(to_json{var}, value.value);

    //empty doc
    //root is obj? doc.to<JsonObject>()
    //root is arr? doc.to<JsonArray>()

    uint32_t bssidSecondByte = *value.get("bssid")->get(1)->asUint();

    serializeJson(doc2, Serial);
    Serial.println("");
    Serial.println(doc2.capacity());
    Serial.println(doc2.memoryUsage());
}
*/
