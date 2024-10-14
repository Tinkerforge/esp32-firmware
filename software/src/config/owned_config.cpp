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

#include "owned_config.h"

#include "tools.h"

OwnedConfig::OwnedConfigWrap::OwnedConfigWrap(const OwnedConfig *_conf) : conf(_conf)
{
}

const OwnedConfig *OwnedConfig::OwnedConfigWrap::operator->() const
{
    return conf;
}

const OwnedConfig::OwnedConfigWrap OwnedConfig::get() const
{
    if (!this->is<OwnedConfig::OwnedConfigUnion>()) {
        esp_system_abort("Config is not a union!");
    }
    OwnedConfig::OwnedConfigWrap wrap(&strict_variant::get<OwnedConfig::OwnedConfigUnion>(&value)->value[0]);

    return wrap;
}

[[gnu::noinline]]
[[gnu::noreturn]]
static void abort_on_index_oob(size_t index, size_t size)
{
    char msg[64];
    snprintf(msg, ARRAY_SIZE(msg), "Config index %zu out of bounds (vector size %zu)!", index, size);
    esp_system_abort(msg);
}

const OwnedConfig::OwnedConfigWrap OwnedConfig::get(uint16_t i) const
{
    if (!this->is<OwnedConfig::OwnedConfigArray>()) {
        esp_system_abort("Config is not an array!");
    }

    const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigArray>(&value)->elements;

    if (i >= elements.size()) {
        abort_on_index_oob(i, elements.size());
    }

    return OwnedConfig::OwnedConfigWrap(&elements[i]);
}

[[gnu::noinline]]
[[gnu::noreturn]]
static void abort_on_key_not_found(const char *key)
{
    char msg[64];
    snprintf(msg, ARRAY_SIZE(msg), "Config key %s not found!", key);
    esp_system_abort(msg);
}

const OwnedConfig::OwnedConfigWrap OwnedConfig::get(const String &key) const
{
    if (!this->is<OwnedConfig::OwnedConfigObject>()) {
        esp_system_abort("Config is not an object!");
    }

    const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigObject>(&value)->elements;
    const auto size = elements.size();

    for (size_t i = 0; i < size; ++i) {
        const auto &val_pair = elements[i];
        if (val_pair.first == key)
            return OwnedConfig::OwnedConfigWrap(&val_pair.second);
    }

    abort_on_key_not_found(key.c_str());
}

size_t OwnedConfig::count() const
{
    if (!this->is<OwnedConfig::OwnedConfigArray>()) {
        esp_system_abort("Config is not an array!");
    }

    const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigArray>(&value)->elements;
    return elements.size();
}

const std::vector<OwnedConfig>::const_iterator OwnedConfig::cbegin() const
{
    if (!this->is<OwnedConfig::OwnedConfigArray>()) {
        esp_system_abort("Config is not an array!");
    }
    const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigArray>(&value)->elements;
    return elements.cbegin();
}

const std::vector<OwnedConfig>::const_iterator OwnedConfig::cend() const
{
    if (!this->is<OwnedConfig::OwnedConfigArray>()) {
        esp_system_abort("Config is not an array!");
    }
    const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigArray>(&value)->elements;
    return elements.cend();
}

const CoolString &OwnedConfig::asString() const
{
    return *this->as<CoolString>();
}

const char *OwnedConfig::asEphemeralCStr() const
{
    return this->as<CoolString>()->c_str();
}

const char *OwnedConfig::asUnsafeCStr() const
{
    return this->as<CoolString>()->c_str();
}

float OwnedConfig::asFloat() const
{
    return *this->as<float>();
}

uint32_t OwnedConfig::asUint() const
{
    return *this->as<uint32_t>();
}

int32_t OwnedConfig::asInt() const
{
    return *this->as<int32_t>();
}

bool OwnedConfig::asBool() const
{
    return *this->as<bool>();
}

size_t OwnedConfig::fillFloatArray(float *arr, size_t elements) const
{
    return fillArray<float, float>(arr, elements);
}

size_t OwnedConfig::fillUint8Array(uint8_t *arr, size_t elements) const
{
    return fillArray<uint8_t, uint32_t>(arr, elements);
}

size_t OwnedConfig::fillInt8Array(int8_t *arr, size_t elements) const
{
    return fillArray<int8_t, int32_t>(arr, elements);
}

size_t OwnedConfig::fillUint16Array(uint16_t *arr, size_t elements) const
{
    return fillArray<uint16_t, uint32_t>(arr, elements);
}

size_t OwnedConfig::fillInt16Array(int16_t *arr, size_t elements) const
{
    return fillArray<int16_t, int32_t>(arr, elements);
}

size_t OwnedConfig::fillUint32Array(uint32_t *arr, size_t elements) const
{
    return fillArray<uint32_t, uint32_t>(arr, elements);
}

size_t OwnedConfig::fillInt32Array(int32_t *arr, size_t elements) const
{
    return fillArray<int32_t, int32_t>(arr, elements);
}
