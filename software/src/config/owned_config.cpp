#include "owned_config.h"

OwnedConfig::OwnedConfigWrap::OwnedConfigWrap(const OwnedConfig *_conf) : conf(_conf)
{

}

const OwnedConfig* OwnedConfig::OwnedConfigWrap::operator->() const
{
    return conf;
}

const OwnedConfig::OwnedConfigWrap OwnedConfig::get() const
{
    if (!this->is<OwnedConfig::OwnedConfigUnion>()) {
        logger.printfln("Config is not a union!");
        esp_system_abort("");
    }
    OwnedConfig::OwnedConfigWrap wrap(&strict_variant::get<OwnedConfig::OwnedConfigUnion>(&value)->value[0]);

    return wrap;
}

const OwnedConfig::OwnedConfigWrap OwnedConfig::get(uint16_t i) const
{
    if (!this->is<OwnedConfig::OwnedConfigArray>()) {
        logger.printfln("Config is not an array!");
        esp_system_abort("");
    }

    const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigArray>(&value)->elements;

    if (i >= elements.size()) {
        logger.printfln("Config index %u out of bounds (vector size %u)!", i, elements.size());
        esp_system_abort("");
    }

    return OwnedConfig::OwnedConfigWrap(&elements[i]);
}

const OwnedConfig::OwnedConfigWrap OwnedConfig::get(const String &key) const
{
    if (!this->is<OwnedConfig::OwnedConfigObject>()) {
        logger.printfln("Config is not an object!");
        esp_system_abort("");
    }

    const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigObject>(&value)->elements;
    const auto size = elements.size();

    for (size_t i = 0; i < size; ++i) {
        const auto &val_pair = elements[i];
        if (val_pair.first == key)
            return OwnedConfig::OwnedConfigWrap(&val_pair.second);
    }

    logger.printfln("Config key %s not found!", key.c_str());
    esp_system_abort("");
}

ssize_t OwnedConfig::count() const
{
    if (!this->is<OwnedConfig::OwnedConfigArray>()) {
        logger.printfln("Config is not an array!");
        esp_system_abort("");
    }

    const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigArray>(&value)->elements;
    return elements.size();
}

const std::vector<OwnedConfig>::const_iterator OwnedConfig::cbegin() const
{
    if (!this->is<OwnedConfig::OwnedConfigArray>()) {
        logger.printfln("Config is not an array!");
        esp_system_abort("");
    }
    const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigArray>(&value)->elements;
    return elements.cbegin();
}

const std::vector<OwnedConfig>::const_iterator OwnedConfig::cend() const
{
    if (!this->is<OwnedConfig::OwnedConfigArray>()) {
        logger.printfln("Config is not an array!");
        esp_system_abort("");
    }
    const auto &elements = strict_variant::get<OwnedConfig::OwnedConfigArray>(&value)->elements;
    return elements.cend();
}

const CoolString& OwnedConfig::asString() const
{
    return *this->as<CoolString>();
}

const char* OwnedConfig::asEphemeralCStr() const
{
    return this->as<CoolString>()->c_str();
}

const char* OwnedConfig::asUnsafeCStr() const
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
