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
        struct Slot {
            String val = "";
            uint16_t minChars = 0;
            uint16_t maxChars = 0;
            bool inUse = false;
        };
    private:
        uint16_t idx;
        Slot *getSlot();

    public:
        static bool slotEmpty(size_t i);
        static constexpr const char *variantName = "ConfString";

        String *getVal();
        const String *getVal() const;
        const Slot *getSlot() const;

        ConfString(String val, uint16_t min, uint16_t max);
        ConfString(const ConfString &cpy);
        ~ConfString();

        ConfString& operator=(const ConfString &cpy);
    };

    struct ConfFloat {
        struct Slot {
            float val;
            float min;
            float max;
        };
    private:
        uint16_t idx;
        Slot *getSlot();

    public:
        static bool slotEmpty(size_t i);
        static constexpr const char *variantName = "ConfFloat";

        float *getVal();
        const float *getVal() const;
        const Slot *getSlot() const;

        ConfFloat(float val, float min, float max);
        ConfFloat(const ConfFloat &cpy);
        ~ConfFloat();

        ConfFloat& operator=(const ConfFloat &cpy);
    };

    struct ConfInt {
        struct Slot {
            int32_t val;
            int32_t min;
            int32_t max;
        };
    private:
        uint16_t idx;

        Slot *getSlot();

    public:
        static bool slotEmpty(size_t i);
        static constexpr const char *variantName = "ConfInt";

        int32_t *getVal();
        const int32_t *getVal() const;
        const Slot *getSlot() const;

        ConfInt(int32_t val, int32_t min, int32_t max);
        ConfInt(const ConfInt &cpy);
        ~ConfInt();

        ConfInt& operator=(const ConfInt &cpy);
    };

    struct ConfUint {
        struct Slot {
            uint32_t val;
            uint32_t min;
            uint32_t max;
        };
    private:
        uint16_t idx;

        Slot *getSlot();

    public:
        static bool slotEmpty(size_t i);
        static constexpr const char *variantName = "ConfUint";

        uint32_t *getVal();
        const uint32_t *getVal() const;
        const Slot *getSlot() const;

        ConfUint(uint32_t val, uint32_t min, uint32_t max);
        ConfUint(const ConfUint &cpy);
        ~ConfUint();

        ConfUint& operator=(const ConfUint &cpy);
    };

    struct ConfBool {
        bool value;
        bool *getVal() { return &value; };
        const bool *getVal() const { return &value; };

        static constexpr const char *variantName = "ConfBool";
    };

    struct ConfArray {
        struct Slot {
            std::vector<Config> val;
            Config *prototype;
            uint32_t minElements : 12, maxElements : 12;
            int8_t variantType;
            bool inUse = false;
        };
    private:
        uint16_t idx;

        Slot *getSlot();

    public:
        static bool slotEmpty(size_t i);
        static constexpr const char *variantName = "ConfArray";

        Config *get(uint16_t i);
        const Config *get(uint16_t i) const;
        std::vector<Config> *getVal();
        const std::vector<Config> *getVal() const;
        const Slot *getSlot() const;

        ConfArray(std::vector<Config> val, Config *prototype, uint16_t minElements, uint16_t maxElements, int8_t variantType);
        ConfArray(const ConfArray &cpy);
        ~ConfArray();

        ConfArray& operator=(const ConfArray &cpy);
    };

    struct ConfObject {
        struct Slot {
            std::vector<std::pair<String, Config>> val;
            bool inUse = false;
        };
    private:
        uint16_t idx;

        Slot *getSlot();

    public:
        static bool slotEmpty(size_t i);
        static constexpr const char *variantName = "ConfObject";

        Config *get(String s);
        const Config *get(String s) const;
        std::vector<std::pair<String, Config>> *getVal();
        const std::vector<std::pair<String, Config>> *getVal() const;
        const Slot *getSlot() const;

        ConfObject(std::vector<std::pair<String, Config>> val);
        ConfObject(const ConfObject &cpy);
        ~ConfObject();

        ConfObject& operator=(const ConfObject &cpy);
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


    struct ConfVariant {
        struct Empty{uint8_t x;};
        enum class Tag: uint8_t {
            EMPTY,
            STRING,
            FLOAT,
            INT,
            UINT,
            BOOL,
            ARRAY,
            OBJECT
        };
        Tag tag = Tag::EMPTY;
        uint8_t updated;
        union Val {
            Val() : e(Empty{}) {}
            Empty e;
            ConfString s;
            ConfFloat f;
            ConfInt i;
            ConfUint u;
            ConfBool b;
            ConfArray a;
            ConfObject o;
            ~Val() {}
        } val;

        ConfVariant(ConfString s) : tag(Tag::STRING), updated(0xFF), val() {new(&val.s) ConfString{s};}
        ConfVariant(ConfFloat f)  : tag(Tag::FLOAT),  updated(0xFF), val() {new(&val.f) ConfFloat{f};}
        ConfVariant(ConfInt i)    : tag(Tag::INT),    updated(0xFF), val() {new(&val.i) ConfInt{i};}
        ConfVariant(ConfUint u)   : tag(Tag::UINT),   updated(0xFF), val() {new(&val.u) ConfUint{u};}
        ConfVariant(ConfBool b)   : tag(Tag::BOOL),   updated(0xFF), val() {new(&val.b) ConfBool{b};}
        ConfVariant(ConfArray a)  : tag(Tag::ARRAY),  updated(0xFF), val() {new(&val.a) ConfArray{a};}
        ConfVariant(ConfObject o) : tag(Tag::OBJECT), updated(0xFF), val() {new(&val.o) ConfObject{o};}

        ConfVariant() : tag(Tag::EMPTY), updated(0xFF), val() {}

        ConfVariant(const ConfVariant &cpy) {
            if (tag != Tag::EMPTY)
                destroyUnionMember();

            switch (cpy.tag) {
                case ConfVariant::Tag::EMPTY:
                    new(&val.e) Empty(cpy.val.e);
                    break;
                case ConfVariant::Tag::STRING:
                    new(&val.s) ConfString(cpy.val.s);
                    break;
                case ConfVariant::Tag::FLOAT:
                    new(&val.f) ConfFloat(cpy.val.f);
                    break;
                case ConfVariant::Tag::INT:
                    new(&val.i) ConfInt(cpy.val.i);
                    break;
                case ConfVariant::Tag::UINT:
                    new(&val.u) ConfUint(cpy.val.u);
                    break;
                case ConfVariant::Tag::BOOL:
                    new(&val.b) ConfBool(cpy.val.b);
                    break;
                case ConfVariant::Tag::ARRAY:
                    new(&val.a) ConfArray(cpy.val.a);
                    break;
                case ConfVariant::Tag::OBJECT:
                    new(&val.o) ConfObject(cpy.val.o);
                    break;
            }
            this->tag = cpy.tag;
            this->updated = cpy.updated;
        }

        ConfVariant &operator=(const ConfVariant &cpy) {
            if (this == &cpy) {
                return *this;
            }

            if (tag != Tag::EMPTY)
                destroyUnionMember();

            switch (cpy.tag) {
                case ConfVariant::Tag::EMPTY:
                    new(&val.e) Empty(cpy.val.e);
                    break;
                case ConfVariant::Tag::STRING:
                    new(&val.s) ConfString(cpy.val.s);
                    break;
                case ConfVariant::Tag::FLOAT:
                    new(&val.f) ConfFloat(cpy.val.f);
                    break;
                case ConfVariant::Tag::INT:
                    new(&val.i) ConfInt(cpy.val.i);
                    break;
                case ConfVariant::Tag::UINT:
                    new(&val.u) ConfUint(cpy.val.u);
                    break;
                case ConfVariant::Tag::BOOL:
                    new(&val.b) ConfBool(cpy.val.b);
                    break;
                case ConfVariant::Tag::ARRAY:
                    new(&val.a) ConfArray(cpy.val.a);
                    break;
                case ConfVariant::Tag::OBJECT:
                    new(&val.o) ConfObject(cpy.val.o);
                    break;
            }
            this->tag = cpy.tag;
            this->updated = cpy.updated;

            return *this;
        }

        void destroyUnionMember() {
            switch (tag) {
                case ConfVariant::Tag::EMPTY:
                    val.e.~Empty();
                    break;
                case ConfVariant::Tag::STRING:
                    val.s.~ConfString();
                    break;
                case ConfVariant::Tag::FLOAT:
                    val.f.~ConfFloat();
                    break;
                case ConfVariant::Tag::INT:
                    val.i.~ConfInt();
                    break;
                case ConfVariant::Tag::UINT:
                    val.u.~ConfUint();
                    break;
                case ConfVariant::Tag::BOOL:
                    val.b.~ConfBool();
                    break;
                case ConfVariant::Tag::ARRAY:
                    val.a.~ConfArray();
                    break;
                case ConfVariant::Tag::OBJECT:
                    val.o.~ConfObject();
                    break;
            }
        }

        ~ConfVariant() {
            destroyUnionMember();
        }
    };

    template<typename T>
    static auto apply_visitor(T visitor, ConfVariant &v) -> decltype(visitor(ConfVariant::Empty{})) {
        switch (v.tag) {
            case ConfVariant::Tag::EMPTY:
                return visitor(v.val.e);
            case ConfVariant::Tag::STRING:
                return visitor(v.val.s);
            case ConfVariant::Tag::FLOAT:
                return visitor(v.val.f);
            case ConfVariant::Tag::INT:
                return visitor(v.val.i);
            case ConfVariant::Tag::UINT:
                return visitor(v.val.u);
            case ConfVariant::Tag::BOOL:
                return visitor(v.val.b);
            case ConfVariant::Tag::ARRAY:
                return visitor(v.val.a);
            case ConfVariant::Tag::OBJECT:
                return visitor(v.val.o);
        }
#ifdef __GNUC__
        __builtin_unreachable();
#endif
    }

    template<typename T>
    static auto apply_visitor(T visitor, const ConfVariant &v) -> decltype(visitor(ConfVariant::Empty{})) {
        switch (v.tag) {
            case ConfVariant::Tag::EMPTY:
                return visitor(v.val.e);
            case ConfVariant::Tag::STRING:
                return visitor(v.val.s);
            case ConfVariant::Tag::FLOAT:
                return visitor(v.val.f);
            case ConfVariant::Tag::INT:
                return visitor(v.val.i);
            case ConfVariant::Tag::UINT:
                return visitor(v.val.u);
            case ConfVariant::Tag::BOOL:
                return visitor(v.val.b);
            case ConfVariant::Tag::ARRAY:
                return visitor(v.val.a);
            case ConfVariant::Tag::OBJECT:
                return visitor(v.val.o);
        }
#ifdef __GNUC__
        __builtin_unreachable();
#endif
    }

    ConfVariant value;

    bool was_updated(uint8_t api_backend_flag);
    void set_update_handled(uint8_t api_backend_flag);

    template<typename T>
    static int type_id()
    {
        if (std::is_same<T, ConfVariant::Empty>())
            return (int)ConfVariant::Tag::EMPTY;
        if (std::is_same<T, ConfString>())
            return (int)ConfVariant::Tag::STRING;
        if (std::is_same<T, ConfFloat>())
            return (int)ConfVariant::Tag::FLOAT;
        if (std::is_same<T, ConfInt>())
            return (int)ConfVariant::Tag::INT;
        if (std::is_same<T, ConfUint>())
            return (int)ConfVariant::Tag::UINT;
        if (std::is_same<T, ConfBool>())
            return (int)ConfVariant::Tag::BOOL;
        if (std::is_same<T, ConfArray>())
            return (int)ConfVariant::Tag::ARRAY;
        if (std::is_same<T, ConfObject>())
            return (int)ConfVariant::Tag::OBJECT;
        return -1;
    }

    template<typename T>
    bool is() const
    {
        return (int)value.tag == Config::type_id<T>();
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
                        uint16_t minElements,
                        uint16_t maxElements,
                        int variantType);

    static Config Object(std::initializer_list<std::pair<String, Config>> obj);

    static Config Null();

    static Config Uint8(uint8_t u);

    static Config Uint16(uint16_t u);

    static Config Uint32(uint32_t u);

    static Config Int8(int8_t i);

    static Config Int16(int16_t i);

    static Config Int32(int32_t i);

    class Wrap
    {
        public:
            Wrap(Config *_conf);

            Config *operator->()
            {
                return conf;
            }

            explicit operator Config*(){return conf;}

        private:
            Config *conf;

    };

    class ConstWrap
    {
        public:
            ConstWrap(const Config *_conf);

            const Config *operator->() const
            {
                return conf;
            }

            explicit operator const Config*() const {return conf;}

        private:
            const Config *conf;
    };

    Wrap get(String s);

    Wrap get(uint16_t i);

    const ConstWrap get(String s) const;

    const ConstWrap get(uint16_t i) const;

    Wrap add()
    {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to add to a node that is not an array!");
            delay(100);
            return Wrap(nullptr);
        }

        std::vector<Config> &children = this->asArray();

        const auto &arr = value.val.a;

        const auto max_elements = arr.getSlot()->maxElements;
        if (children.size() >= max_elements) {
            logger.printfln("Tried to add to an ConfArray that already has the max allowed number of elements (%u).", max_elements);
            delay(100);
            return Wrap(nullptr);
        }

        children.push_back(*arr.getSlot()->prototype);
        return Wrap(arr.getSlot()->prototype);
    }

    bool removeLast()
    {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to add to a node that is not an array!");
            delay(100);
            return false;
        }

        std::vector<Config> &children = this->asArray();
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
        std::vector<Config> &children = this->asArray();

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
        std::vector<Config> &children = this->asArray();
        return children.size();
    }

    template<typename ConfigT>
    ConfigT *get() {
        if (!this->is<ConfigT>()) {
            logger.printfln("get: Config has wrong type. This is %s, requested is %s", this->to_string().c_str(), ConfigT::variantName);
            delay(100);
            return nullptr;
        }

        return reinterpret_cast<ConfigT *>(&value.val);
    }

    template<typename ConfigT>
    const ConfigT *get() const {
        if (!this->is<ConfigT>()) {
            logger.printfln("get: Config has wrong type. This is %s, requested is %s", this->to_string().c_str(), ConfigT::variantName);
            delay(100);
            return nullptr;
        }

        return reinterpret_cast<const ConfigT *>(&value.val);
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
        T *target = get<ConfigT>()->getVal();
        T old_value = *target;
        *target = value;

        if (old_value != value)
            this->value.updated = 0xFF;

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

        const ConfArray &confArr = this->value.val.a;
        size_t toWrite = std::min(confArr.getVal()->size(), elements);

        for (size_t i = 0; i < toWrite; ++i) {
            const Config *entry = confArr.get(i);
            if (!entry->is<ConfigT>()) {
                logger.printfln("Config entry has wrong type.");
                delay(100);
                return 0;
            }
            arr[i] = *entry->get<ConfigT>()->getVal();
        }

        return toWrite;
    }

    size_t fillFloatArray(float *arr, size_t elements);

    size_t fillUint8Array(uint8_t *arr, size_t elements);
    size_t fillUint16Array(uint16_t *arr, size_t elements);
    size_t fillUint32Array(uint32_t *arr, size_t elements);

    size_t fillInt8Array(int8_t *arr, size_t elements);
    size_t fillInt16Array(int16_t *arr, size_t elements);
    size_t fillInt32Array(int32_t *arr, size_t elements);
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
