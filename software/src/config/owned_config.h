#pragma once

#include <vector>
#include <memory>

#include "event_log.h"
#include "cool_string.h"

#define STRICT_VARIANT_ASSUME_MOVE_NOTHROW true
#include "strict_variant/variant.hpp"
#include "strict_variant/mpl/find_with.hpp"

struct OwnedConfig {
    struct OwnedConfigArray;
    struct OwnedConfigObject;
    struct OwnedConfigUnion;

    typedef strict_variant::variant<
        std::nullptr_t,
        CoolString,
        float,
        uint32_t,
        int32_t,
        bool,
        OwnedConfigArray,
        OwnedConfigObject,
        OwnedConfigUnion
    > OwnedConfigVariant;

    struct OwnedConfigArray {
        std::vector<OwnedConfig> elements;
    };

    struct OwnedConfigObject {
        std::vector<std::pair<String, OwnedConfig>> elements;
    };

    struct OwnedConfigUnion {
        uint8_t tag;
        std::vector<OwnedConfig> value;
    };

    class OwnedConfigWrap
    {
        public:
            OwnedConfigWrap(const OwnedConfig *_conf);

            const OwnedConfig *operator->() const;

            explicit operator const OwnedConfig*() const;

        private:
            const OwnedConfig *conf;
    };

    const OwnedConfigWrap get() const;
    const OwnedConfigWrap get(uint16_t i) const;
    const OwnedConfigWrap get(const String &key) const;

    ssize_t count() const;
    const std::vector<OwnedConfig>::const_iterator cbegin() const;
    const std::vector<OwnedConfig>::const_iterator cend() const;

    template<typename ConfigT>
    const ConfigT *as() const {
        if (!this->is<ConfigT>()) {
            logger.printfln("as: Config has wrong type.");
            esp_system_abort("");
        }
        return strict_variant::get<ConfigT>(&value);
    }

    const CoolString &asString() const;

    const char *asEphemeralCStr() const;
    const char *asUnsafeCStr() const;

    float asFloat() const;

    uint32_t asUint() const;

    int32_t asInt() const;

    template<typename T>
    T asEnum() const {
        if (this->is<uint32_t>()) {
            return (T) this->asUint();
        } else if (this->is<int32_t>()) {
            return (T) this->asInt();
        } else {
            logger.printfln("asEnum: Config has wrong type.");
            esp_system_abort("");
        }
    }

    bool asBool() const;

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

    size_t fillFloatArray(float *arr, size_t elements) const;

    size_t fillUint8Array(uint8_t *arr, size_t elements) const;
    size_t fillUint16Array(uint16_t *arr, size_t elements) const;
    size_t fillUint32Array(uint32_t *arr, size_t elements) const;

    size_t fillInt8Array(int8_t *arr, size_t elements) const;
    size_t fillInt16Array(int16_t *arr, size_t elements) const;
    size_t fillInt32Array(int32_t *arr, size_t elements) const;

    OwnedConfig(OwnedConfigVariant value) : value(value){}
    OwnedConfig() : value(nullptr) {}

private:
    OwnedConfigVariant value;

    template<typename T, typename ConfigT>
    size_t fillArray(T *arr, size_t count) const{
        if (!this->is<OwnedConfigArray>()) {
            logger.printfln("Can't fill array, Config is not an array");
            esp_system_abort("");
        }

        const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigArray>(&value)->elements;
        size_t toWrite = std::min(elements.size(), count);

        for (size_t i = 0; i < toWrite; ++i) {
            const OwnedConfig *entry = &elements[i];
            if (!entry->is<ConfigT>()) {
                logger.printfln("Config entry has wrong type.");
                esp_system_abort("");
            }
            arr[i] = *strict_variant::get<ConfigT>(&entry->value);
        }

        return toWrite;
    }
};
