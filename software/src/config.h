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

#include "config/owned_config.h"

#ifdef DEBUG_FS_ENABLE
extern TaskHandle_t mainTaskHandle;
#define ASSERT_MAIN_THREAD() do { \
        if (mainTaskHandle != xTaskGetCurrentTaskHandle()) { \
            esp_system_abort("Accessing the config is only allowed in the main thread!"); \
        } \
    } while (0)
#else
#define ASSERT_MAIN_THREAD() do {} while (0)
#endif

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

struct ConfUnionPrototypeInternal;

template<typename T>
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
        static Slot *allocSlotBuf(size_t elements);
        static void freeSlotBuf(Slot *buf);

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
        static Slot *allocSlotBuf(size_t elements);
        static void freeSlotBuf(Slot *buf);

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
        static Slot *allocSlotBuf(size_t elements);
        static void freeSlotBuf(Slot *buf);

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
        static Slot *allocSlotBuf(size_t elements);
        static void freeSlotBuf(Slot *buf);

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
        bool *getVal();
        const bool *getVal() const;

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
        static Slot *allocSlotBuf(size_t elements);
        static void freeSlotBuf(Slot *buf);

        Config *get(uint16_t i);
        const Config *get(uint16_t i) const;
        std::vector<Config> *getVal();
        const std::vector<Config> *getVal() const;
        const Slot *getSlot() const;

        ConfArray(std::vector<Config> val, const Config *prototype, uint16_t minElements, uint16_t maxElements, int8_t variantType);
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
        static Slot *allocSlotBuf(size_t elements);
        static void freeSlotBuf(Slot *buf);

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
        static Slot *allocSlotBuf(size_t elements);
        static void freeSlotBuf(Slot *buf);

        uint8_t getTag() const;
        bool changeUnionVariant(uint8_t tag);

        Config *getVal();
        const Config *getVal() const;
        const Slot *getSlot() const;

        ConfUnion(const Config &val, uint8_t tag, uint8_t prototypes_len, const ConfUnionPrototypeInternal prototypes[]);
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
    static bool containsNull(const ConfUpdate *update);

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
            Val();
            Empty e;
            ConfString s;
            ConfFloat f;
            ConfInt i;
            ConfUint u;
            ConfBool b;
            ConfArray a;
            ConfObject o;
            ConfUnion un;
            ~Val();
        } val;

        ConfVariant(ConfString s);
        ConfVariant(ConfFloat f);
        ConfVariant(ConfInt i);
        ConfVariant(ConfUint u);
        ConfVariant(ConfBool b);
        ConfVariant(ConfArray a);
        ConfVariant(ConfObject o);
        ConfVariant(ConfUnion un);

        ConfVariant();

        ConfVariant(const ConfVariant &cpy);

        ConfVariant &operator=(const ConfVariant &cpy);

        void destroyUnionMember();

        ~ConfVariant();

        const char *getVariantName() const;
    };

    template<typename T>
    static auto apply_visitor(T visitor, ConfVariant &v) -> decltype(visitor(ConfVariant::Empty{})) {
        ASSERT_MAIN_THREAD();
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
        ASSERT_MAIN_THREAD();
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

    uint8_t was_updated(uint8_t api_backend_flag);
    void clear_updated(uint8_t api_backend_flag);
    void set_updated(uint8_t api_backend_flag);

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

    bool is_null() const;

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
                        const Config *prototype,
                        uint16_t minElements,
                        uint16_t maxElements,
                        int variantType);

    static Config Object(std::initializer_list<std::pair<String, Config>> obj);

    template<typename T>
    static void check_enum_template_type() {
        static_assert(std::is_enum<T>::value, "ConfUnion tag type must be enum");

        // This is a complicated way to express
        // static_assert(std::is_same<std::underlying_type<T>::type, uint8_t>::value, "Underlying type of ConfUnion tag type must be uint8_t");
        // but I don't get the simpler assert to compile.
        // So check for the alignment properties, because those are what we care about.
        struct foobar{T foo; uint8_t bar;};
        static_assert(offsetof(foobar, bar) == 1, "Underlying type of ConfUnion tag type must be uint8_t");

        static_assert(offsetof(ConfUnionPrototype<T>, tag) == offsetof(ConfUnionPrototypeInternal, tag), "Tag offset mismatch between ConfUnionPrototype<T> and ConfUnionPrototypeInternal");
        static_assert(offsetof(ConfUnionPrototype<T>, config) == offsetof(ConfUnionPrototypeInternal, config), "Config offset mismatch between ConfUnionPrototype<T> and ConfUnionPrototypeInternal");
    }

    template<typename T>
    static Config Union(Config value, T tag, const ConfUnionPrototype<T> prototypes[], uint8_t prototypes_len) {
        Config::check_enum_template_type<T>();
        return Union(value, (uint8_t) tag, (ConfUnionPrototypeInternal*) prototypes, prototypes_len);
    }
private:
    static Config Union(Config value, uint8_t tag, const ConfUnionPrototypeInternal prototypes[], uint8_t prototypes_len);
public:

    static ConfigRoot *Null();

    static ConfigRoot *Confirm();
    static String ConfirmKey();

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

            Config *operator->();

            explicit operator Config*();

            // Allowing to call begin and end directly on
            // the wrapper makes it easier to use
            // range-based for loops.
            std::vector<Config>::iterator begin();
            std::vector<Config>::iterator end();

        private:
            Config *conf;

    };

    class ConstWrap
    {
        public:
            ConstWrap(const Config *_conf);

            const Config *operator->() const;

            explicit operator const Config*() const;

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
    bool removeLast();
    bool removeAll();
    bool remove(size_t i);
    ssize_t count() const;
    std::vector<Config>::iterator begin();
    std::vector<Config>::iterator end();

    template<typename T>
    T getTag() const {
        Config::check_enum_template_type<T>();
        if (!this->is<Config::ConfUnion>()) {
            logger.printfln("Tried to get tag of a node that is not a union!");
            esp_system_abort("");
        }
        return (T) this->get<ConfUnion>()->getTag();
    }

private:
    template<typename ConfigT>
    ConfigT *get() {
        ASSERT_MAIN_THREAD();
        if (!this->is<ConfigT>()) {
            logger.printfln("get: Config has wrong type. This is %s, requested is %s", this->value.getVariantName(), ConfigT::variantName);
#ifdef DEBUG_FS_ENABLE
            logger.printfln("Content is %s", this->to_string().c_str());
#endif
            esp_system_abort("");
        }

        return reinterpret_cast<ConfigT *>(&value.val);
    }

    template<typename ConfigT>
    const ConfigT *get() const {
        ASSERT_MAIN_THREAD();
        if (!this->is<ConfigT>()) {
            logger.printfln("get: Config has wrong type. This is %s, requested is %s", this->value.getVariantName(), ConfigT::variantName);
#ifdef DEBUG_FS_ENABLE
            logger.printfln("Content is %s", this->to_string().c_str());
#endif
            esp_system_abort("");
        }

        return reinterpret_cast<const ConfigT *>(&value.val);
    }

public:
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
            logger.printfln("asEnum: Config has wrong type. This is %s, (not a ConfInt or ConfUint)", this->value.getVariantName());
#ifdef DEBUG_FS_ENABLE
            logger.printfln("Content is %s", this->to_string().c_str());
#endif
            esp_system_abort("");
        }
    }

    bool asBool() const;

private:
    // This is a gigantic footgun: The reference is invalidated after the module setup,
    // because of the ConfSlot array shrinkToFit calls.
    std::vector<Config> &asArray();
    const std::vector<Config> &asArray() const;

    template<typename T, typename ConfigT>
    bool update_value(T value, const char *value_type) {
        ASSERT_MAIN_THREAD();
        if (!this->is<ConfigT>()) {
            logger.printfln("update_value: Config has wrong type. This is a %s. new value is a %s", this->value.getVariantName(), value_type);
#ifdef DEBUG_FS_ENABLE
            logger.printfln("Content is %s", this->to_string().c_str());
            logger.printfln("value is is %s", String(value).c_str());
#endif
            esp_system_abort("");
        }
        T *target = get<ConfigT>()->getVal();
        T old_value = *target;
        *target = value;

        if (old_value != value)
            this->value.updated = 0xFF;

        return old_value != value;
    }

public:
    bool updateString(const String &value);
    bool updateInt(int32_t value);
    bool updateUint(uint32_t value);
    bool updateFloat(float value);
    bool updateBool(bool value);

private:
    template<typename T, typename ConfigT>
    size_t fillArray(T *arr, size_t elements) {
        ASSERT_MAIN_THREAD();
        if (!this->is<ConfArray>()) {
            logger.printfln("Can't fill array, Config is not an array");
            esp_system_abort("");
        }

        const ConfArray &confArr = this->value.val.a;
        size_t toWrite = std::min(confArr.getVal()->size(), elements);

        for (size_t i = 0; i < toWrite; ++i) {
            const Config *entry = confArr.get(i);
            if (!entry->is<ConfigT>()) {
                logger.printfln("Config entry has wrong type.");
                esp_system_abort("");
            }
            arr[i] = *entry->get<ConfigT>()->getVal();
        }

        return toWrite;
    }

public:
    size_t fillFloatArray(float *arr, size_t elements);

    size_t fillUint8Array(uint8_t *arr, size_t elements);
    size_t fillUint16Array(uint16_t *arr, size_t elements);
    size_t fillUint32Array(uint32_t *arr, size_t elements);

    size_t fillInt8Array(int8_t *arr, size_t elements);
    size_t fillInt16Array(int16_t *arr, size_t elements);
    size_t fillInt32Array(int32_t *arr, size_t elements);

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

    ConfigRoot(Config cfg);

    ConfigRoot(Config cfg, std::function<String(Config &)> validator);

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

    OwnedConfig get_owned_copy();

private:
    template<typename T>
    String update_from_visitor(T visitor);

    template<typename T>
    String get_updated_copy(T visitor, Config *out_config);
};

template<typename T>
struct ConfUnionPrototype {
    T tag;
    Config config;

    ConfUnionPrototype(T tag, Config config) : tag(tag), config(config) {
        Config::check_enum_template_type<T>();
    }
};

struct ConfUnionPrototypeInternal {
    uint8_t tag;
    Config config;
};
