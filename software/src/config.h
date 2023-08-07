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
#include "cool_string.h"

#define STRICT_VARIANT_ASSUME_MOVE_NOTHROW true
#include "strict_variant/variant.hpp"
#include "strict_variant/mpl/find_with.hpp"

void config_pre_init();
void config_post_setup();

struct ConfigRoot;
struct ConfStringSlot;
struct ConfFloatSlot;
struct ConfIntSlot;
struct ConfUintSlot;
struct ConfArraySlot;
struct ConfObjectSlot;
struct ConfUnionSlot;

struct ConfUnionPrototype;

struct Config {
    struct ConfString {
        using Slot = ConfStringSlot;
    private:
        uint16_t idx;
        Slot *getSlot();

    public:
        static bool slotEmpty(size_t i);
        static constexpr const char *variantName = "ConfString";

        CoolString *getVal();
        const CoolString *getVal() const;
        const Slot *getSlot() const;

        ConfString(const CoolString &val, uint16_t min, uint16_t max);
        ConfString(const ConfString &cpy);
        ~ConfString();

        ConfString& operator=(const ConfString &cpy);
    };

    struct ConfFloat {
        using Slot = ConfFloatSlot;
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
        using Slot = ConfIntSlot;
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
        using Slot = ConfUintSlot;
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
        using Slot = ConfArraySlot;
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
        using Slot = ConfObjectSlot;
    private:
        uint16_t idx;
        Slot *getSlot();

    public:
        static bool slotEmpty(size_t i);
        static constexpr const char *variantName = "ConfObject";

        Config *get(const String &s);
        const Config *get(const String &s) const;
        std::vector<std::pair<String, Config>> *getVal();
        const std::vector<std::pair<String, Config>> *getVal() const;
        const Slot *getSlot() const;

        ConfObject(std::vector<std::pair<String, Config>> val);
        ConfObject(const ConfObject &cpy);
        ~ConfObject();

        ConfObject& operator=(const ConfObject &cpy);
    };

    struct ConfUnion {
        using Slot = ConfUnionSlot;
    private:
        uint16_t idx;
        Slot *getSlot();

    public:
        static bool slotEmpty(size_t i);
        static constexpr const char *variantName = "ConfUnion";

        uint8_t getTag() const;
        bool changeUnionVariant(uint8_t tag);

        Config *getVal();
        const Config *getVal() const;
        const Slot *getSlot() const;

        ConfUnion(const Config &val, uint8_t tag, uint8_t prototypes_len, const ConfUnionPrototype prototypes[]);
        ConfUnion(const ConfUnion &cpy);
        ~ConfUnion();

        ConfUnion& operator=(const ConfUnion &cpy);
    };


    struct ConfUpdateArray;
    struct ConfUpdateObject;
    struct ConfUpdateUnion;

    typedef strict_variant::variant<
        std::nullptr_t, // DON'T MOVE THIS!
        CoolString,
        float,
        uint32_t,
        int32_t,
        bool,
        ConfUpdateArray,
        ConfUpdateObject,
        ConfUpdateUnion
    > ConfUpdate;
    // This is necessary as we can't use get to distinguish between
    // a get<std::nullptr_t>() that returned nullptr because the variant
    // had another type and the same call that returned nullptr because
    // the variant has the std::nullptr_type and thus contains a nullptr.
    static bool containsNull(const ConfUpdate *update)
    {
        return update->which() == 0;
    }

    struct ConfUpdateArray {
        std::vector<ConfUpdate> elements;
    };

    struct ConfUpdateObject {
        std::vector<std::pair<String, ConfUpdate>> elements;
    };

    struct ConfUpdateUnion {
        uint8_t tag;
        ConfUpdate &value;
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
            OBJECT,
            UNION
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
            ConfUnion un;
            ~Val() {}
        } val;

        ConfVariant(ConfString s) : tag(Tag::STRING), updated(0xFF), val() {new(&val.s)  ConfString{s};}
        ConfVariant(ConfFloat f)  : tag(Tag::FLOAT),  updated(0xFF), val() {new(&val.f)  ConfFloat{f};}
        ConfVariant(ConfInt i)    : tag(Tag::INT),    updated(0xFF), val() {new(&val.i)  ConfInt{i};}
        ConfVariant(ConfUint u)   : tag(Tag::UINT),   updated(0xFF), val() {new(&val.u)  ConfUint{u};}
        ConfVariant(ConfBool b)   : tag(Tag::BOOL),   updated(0xFF), val() {new(&val.b)  ConfBool{b};}
        ConfVariant(ConfArray a)  : tag(Tag::ARRAY),  updated(0xFF), val() {new(&val.a)  ConfArray{a};}
        ConfVariant(ConfObject o) : tag(Tag::OBJECT), updated(0xFF), val() {new(&val.o)  ConfObject{o};}
        ConfVariant(ConfUnion un) : tag(Tag::UNION),  updated(0xFF), val() {new(&val.un) ConfUnion{un};}

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
                case ConfVariant::Tag::UNION:
                    new(&val.un) ConfUnion(cpy.val.un);
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
                case ConfVariant::Tag::UNION:
                    new(&val.un) ConfUnion(cpy.val.un);
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
                case ConfVariant::Tag::UNION:
                    val.un.~ConfUnion();
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
            case ConfVariant::Tag::UNION:
                return visitor(v.val.un);
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
            case ConfVariant::Tag::UNION:
                return visitor(v.val.un);
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
        if (std::is_same<T, ConfUnion>())
            return (int)ConfVariant::Tag::UNION;
        return -1;
    }

    template<typename T>
    bool is() const
    {
        return (int)value.tag == Config::type_id<T>();
    }

    bool is_null() const {
        return value.tag == ConfVariant::Tag::EMPTY;
    }

    static Config Str(const String &s,
                      uint16_t minChars,
                      uint16_t maxChars);

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

    static Config Union(Config value, uint8_t tag, const ConfUnionPrototype prototypes[], uint8_t prototypes_len);

    static ConfigRoot *Null();

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

            // Allowing to call begin and end directly on
            // the wrapper makes it easier to use
            // range-based for loops.
            std::vector<Config>::iterator begin() {
                return conf->begin();
            }

            std::vector<Config>::iterator end() {
                return conf->end();
            }

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

    // for ConfUnion
    Wrap get();

    // for ConfObject
    Wrap get(const String &s);

    // for ConfArray
    Wrap get(uint16_t i);

    // for ConfUnion
    const ConstWrap get() const;

    // for ConfObject
    const ConstWrap get(const String &s) const;

    // for ConfArray
    const ConstWrap get(uint16_t i) const;

    Wrap add();

    bool removeLast()
    {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to remove the last element from a node that is not an array!");
            delay(100);
            return false;
        }

        std::vector<Config> &children = this->asArray();
        if (children.size() == 0)
            return false;

        children.pop_back();
        return true;
    }

    bool removeAll()
    {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to remove all from a node that is not an array!");
            delay(100);
            return false;
        }

        std::vector<Config> &children = this->asArray();

        children.clear();

        return true;
    }

    bool remove(size_t i)
    {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to remove from a node that is not an array!");
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

    std::vector<Config>::iterator begin() {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to get count of a node that is not an array!");
            delay(100);
            return std::vector<Config>::iterator();
        }
        return this->asArray().begin();
    }

    std::vector<Config>::iterator end() {
        if (!this->is<Config::ConfArray>()) {
            logger.printfln("Tried to get count of a node that is not an array!");
            delay(100);
            return std::vector<Config>::iterator();
        }
        return this->asArray().end();
    }

    uint8_t getTag() const {
        if (!this->is<Config::ConfUnion>()) {
            logger.printfln("Tried to get tag of a node that is not a union!");
            delay(100);
            return -1;
        }
        return this->get<ConfUnion>()->getTag();
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

    const CoolString &asString() const;

    const char *asEphemeralCStr() const;
    const char *asUnsafeCStr() const;

    float asFloat() const;

    uint32_t asUint() const;

    int32_t asInt() const;

    template<typename T>
    T asEnum() const {
        if (this->is<ConfUint>()) {
            return (T) this->asUint();
        } else if (this->is<ConfInt>()) {
            return (T) this->asInt();
        } else {
            logger.printfln("asEnum: Config has wrong type. This is %s, (not a ConfInt or ConfUint)", this->to_string().c_str());
            delay(100);
            return (T) this->asUint();
        }
    }

    bool asBool() const;

private:
    // This is a gigantic footgun: The reference is invalidated after the module setup,
    // because of the ConfSlot array shrinkToFit calls.
    std::vector<Config> &asArray();

public:
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

    bool updateString(const String &value)
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

    DynamicJsonDocument to_json(const std::vector<String> &keys_to_censor) const;

    void save_to_file(File &file);

    void write_to_stream(Print &output);
    void write_to_stream_except(Print &output, const std::vector<String> &keys_to_censor);

    String to_string() const;
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

    void update_from_copy(Config *copy);

    String update_from_file(File &file);

    // Intentionally take a non-const char * here:
    // This allows ArduinoJson to deserialize in zero-copy mode
    String update_from_cstr(char *c, size_t payload_len);
    String get_updated_copy(char *c, size_t payload_len, Config *out_config);

    String update_from_json(JsonVariant root, bool force_same_keys);
    String get_updated_copy(JsonVariant root, bool force_same_keys, Config *out_config);

    String update(const Config::ConfUpdate *val);

    String validate();

private:
    template<typename T>
    String update_from_visitor(T visitor);

    template<typename T>
    String get_updated_copy(T visitor, Config *out_config);
};

struct ConfUnionPrototype {
    uint8_t tag;
    Config config;
};
