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

#include <vector>
#include <memory>
#include <esp_system.h>

#define STRICT_VARIANT_ASSUME_MOVE_NOTHROW true
#include <strict_variant/variant.hpp>
#include <strict_variant/mpl/find_with.hpp>

#include "cool_string.h"

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

    size_t count() const;
    const std::vector<OwnedConfig>::const_iterator cbegin() const;
    const std::vector<OwnedConfig>::const_iterator cend() const;

    template<typename ConfigT>
    const ConfigT *as() const {
        if (!this->is<ConfigT>()) {
            esp_system_abort("as: Config has wrong type.");
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
            esp_system_abort("asEnum: Config has wrong type.");
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
            esp_system_abort("Can't fill array, Config is not an array");
        }

        const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigArray>(&value)->elements;
        size_t toWrite = std::min(elements.size(), count);

        for (size_t i = 0; i < toWrite; ++i) {
            const OwnedConfig *entry = &elements[i];
            if (!entry->is<ConfigT>()) {
                esp_system_abort("Config entry has wrong type.");
            }
            arr[i] = *strict_variant::get<ConfigT>(&entry->value);
        }

        return toWrite;
    }
};
