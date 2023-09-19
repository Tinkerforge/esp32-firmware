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

#include "config/private.h"

#include "config/visitors.h"

#include "math.h"

#include "tools.h"

#define UINT_SLOTS 512
Config::ConfUint::Slot *uint_buf = nullptr;
size_t uint_buf_size = 0;

#define INT_SLOTS 128
Config::ConfInt::Slot *int_buf = nullptr;
size_t int_buf_size = 0;

#define FLOAT_SLOTS 384
Config::ConfFloat::Slot *float_buf = nullptr;
size_t float_buf_size = 0;

#define STRING_SLOTS 384
Config::ConfString::Slot *string_buf = nullptr;
size_t string_buf_size = 0;

#define ARRAY_SLOTS 32
Config::ConfArray::Slot *array_buf = nullptr;
size_t array_buf_size = 0;

#define OBJECT_SLOTS 256
Config::ConfObject::Slot *object_buf = nullptr;
size_t object_buf_size = 0;

#define UNION_SLOTS 32
Config::ConfUnion::Slot *union_buf = nullptr;
size_t union_buf_size = 0;

static ConfigRoot nullconf = Config{Config::ConfVariant{}};
static ConfigRoot *confirmconf;

bool Config::containsNull(const ConfUpdate *update) {
    return update->which() == 0;
}

bool Config::is_null() const {
    return value.tag == ConfVariant::Tag::EMPTY;
}

Config Config::Str(const String &s, uint16_t minChars, uint16_t maxChars)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfString{CoolString(s), minChars, maxChars}};
}

Config Config::Float(float d, float min, float max)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfFloat{d, min, max}};
}

Config Config::Int(int32_t i, int32_t min, int32_t max)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfInt{i, min, max}};
}

Config Config::Uint(uint32_t u, uint32_t min, uint32_t max)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfUint{u, min, max}};
}

Config Config::Bool(bool b)
{
    // Allow constructing bool configs:
    // Those are not stored in slots so no static initialization order problems can emerge here.

    return Config{ConfBool{b}};
}

Config Config::Array(std::initializer_list<Config> arr, const Config *prototype, uint16_t minElements, uint16_t maxElements, int variantType)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfArray{arr, prototype, minElements, maxElements, (int8_t)variantType}};
}

Config Config::Object(std::initializer_list<std::pair<String, Config>> obj)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfObject{obj}};
}

Config Config::Union(Config value, uint8_t tag, const ConfUnionPrototypeInternal prototypes[], uint8_t prototypes_len)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    return Config{ConfUnion{value, tag, prototypes_len, prototypes}};
}

ConfigRoot *Config::Null()
{
    // Allow constructing null configs:
    // Those are not stored in slots so no static initialization order problems can emerge here.

    return &nullconf;
}

ConfigRoot *Config::Confirm() {
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    if (confirmconf == nullptr) {
        confirmconf = new ConfigRoot(Config::Object({
            {Config::ConfirmKey(), Config::Bool(false)}
        }));
    }

    return confirmconf;
}

String Config::ConfirmKey() {
    return "do_i_know_what_i_am_doing";
}

Config Config::Uint8(uint8_t u)
{
    return Config::Uint(u, std::numeric_limits<uint8_t>::lowest(), std::numeric_limits<uint8_t>::max());
}

Config Config::Uint16(uint16_t u)
{
    return Config::Uint(u, std::numeric_limits<uint16_t>::lowest(), std::numeric_limits<uint16_t>::max());
}

Config Config::Uint32(uint32_t u)
{
    return Config::Uint(u, std::numeric_limits<uint32_t>::lowest(), std::numeric_limits<uint32_t>::max());
}

Config Config::Int8(int8_t i)
{
    return Config::Int(i, std::numeric_limits<int8_t>::lowest(), std::numeric_limits<int8_t>::max());
}

Config Config::Int16(int16_t i)
{
    return Config::Int(i, std::numeric_limits<int16_t>::lowest(), std::numeric_limits<int16_t>::max());
}

Config Config::Int32(int32_t i)
{
    return Config::Int(i, std::numeric_limits<int32_t>::lowest(), std::numeric_limits<int32_t>::max());
}

Config::Wrap Config::get() {
    if (!this->is<Config::ConfUnion>()) {
        logger.printfln("Config is not a union!");
        delay(100);
        return Wrap(nullptr);
    }
    Wrap wrap(value.val.un.getVal());

    return wrap;
}

const Config::ConstWrap Config::get() const {
    if (!this->is<Config::ConfUnion>()) {
        logger.printfln("Config is not a union!");
        delay(100);
        return ConstWrap(nullptr);
    }
    ConstWrap wrap(value.val.un.getVal());

    return wrap;
}

Config::Wrap Config::get(const String &s)
{

    if (!this->is<Config::ConfObject>()) {
        logger.printfln("Config key %s not in this node: is not an object!", s.c_str());
        delay(100);
        return Wrap(nullptr);
    }
    Wrap wrap(value.val.o.get(s));

    return wrap;
}

Config::Wrap Config::get(uint16_t i)
{

    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Config index %u not in this node: is not an array!", i);
        delay(100);
        return Wrap(nullptr);
    }
    Wrap wrap(value.val.a.get(i));

    return wrap;
}

const Config::ConstWrap Config::get(const String &s) const
{
    if (!this->is<Config::ConfObject>()) {
        logger.printfln("Config key %s not in this node: is not an object!", s.c_str());
        delay(100);
        return ConstWrap(nullptr);
    }
    ConstWrap wrap(value.val.o.get(s));

    return wrap;
}

const Config::ConstWrap Config::get(uint16_t i) const
{
    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Config index %u not in this node: is not an array!", i);
        delay(100);
        return ConstWrap(nullptr);
    }
    ConstWrap wrap(value.val.a.get(i));

    return wrap;
}

Config::Wrap Config::add() {
    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Tried to add to a node that is not an array!");
        delay(100);
        return Wrap(nullptr);
    }

    std::vector<Config> &children = this->asArray();

    const auto &arr = value.val.a;
    const auto *slot = arr.getSlot();

    const auto max_elements = slot->maxElements;
    if (children.size() >= max_elements) {
        logger.printfln("Tried to add to an ConfArray that already has the max allowed number of elements (%u).", max_elements);
        delay(100);
        return Wrap(nullptr);
    }

    auto copy = *slot->prototype;

    // Copying the prototype might invalidate the children reference
    // when ConfArray slots are moved, so asArray() must be called again.
    children = this->asArray();

    children.push_back(std::move(copy));

    return Wrap(&children.back());
}

bool Config::removeLast()
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

bool Config::removeAll()
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

bool Config::remove(size_t i)
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

ssize_t Config::count() const
{
    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Tried to get count of a node that is not an array!");
        delay(100);
        return -1;
    }
    const std::vector<Config> &children = this->asArray();
    return children.size();
}

std::vector<Config>::iterator Config::begin()
{
    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Tried to get begin iterator of a node that is not an array!");
        delay(100);
        return std::vector<Config>::iterator();
    }
    return this->asArray().begin();
}

std::vector<Config>::iterator Config::end()
{
    if (!this->is<Config::ConfArray>()) {
        logger.printfln("Tried to get end iterator of a node that is not an array!");
        delay(100);
        return std::vector<Config>::iterator();
    }
    return this->asArray().end();
}

const CoolString &Config::asString() const
{
    return *this->get<ConfString>()->getVal();
}

const char *Config::asEphemeralCStr() const
{
    return this->get<ConfString>()->getVal()->c_str();
}

const char *Config::asUnsafeCStr() const
{
    return this->get<ConfString>()->getVal()->c_str();
}

float Config::asFloat() const
{
    return *this->get<ConfFloat>()->getVal();
}

uint32_t Config::asUint() const
{
    return *this->get<ConfUint>()->getVal();
}

int32_t Config::asInt() const
{
    return *this->get<ConfInt>()->getVal();
}

bool Config::asBool() const
{
    return *this->get<ConfBool>()->getVal();
}

std::vector<Config> &Config::asArray()
{
    return *this->get<ConfArray>()->getVal();
}

const std::vector<Config> &Config::asArray() const
{
    return *this->get<ConfArray>()->getVal();
}

bool Config::updateString(const String &value)
{
    return update_value<String, ConfString>(value, "String");
}

bool Config::updateInt(int32_t value)
{
    return update_value<int32_t, ConfInt>(value, "int32_t");
}

bool Config::updateUint(uint32_t value)
{
    return update_value<uint32_t, ConfUint>(value, "uint32_t");
}

bool Config::updateFloat(float value)
{
    return update_value<float, ConfFloat>(value, "float");
}

bool Config::updateBool(bool value)
{
    return update_value<bool, ConfBool>(value, "bool");
}

size_t Config::fillFloatArray(float *arr, size_t elements)
{
    return fillArray<float, Config::ConfFloat>(arr, elements);
}

size_t Config::fillUint8Array(uint8_t *arr, size_t elements)
{
    return fillArray<uint8_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt8Array(int8_t *arr, size_t elements)
{
    return fillArray<int8_t, Config::ConfInt>(arr, elements);
}

size_t Config::fillUint16Array(uint16_t *arr, size_t elements)
{
    return fillArray<uint16_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt16Array(int16_t *arr, size_t elements)
{
    return fillArray<int16_t, Config::ConfInt>(arr, elements);
}

size_t Config::fillUint32Array(uint32_t *arr, size_t elements)
{
    return fillArray<uint32_t, Config::ConfUint>(arr, elements);
}

size_t Config::fillInt32Array(int32_t *arr, size_t elements)
{
    return fillArray<int32_t, Config::ConfInt>(arr, elements);
}

size_t Config::json_size(bool zero_copy) const
{
    return Config::apply_visitor(json_length_visitor{zero_copy}, value);
}

size_t Config::max_string_length() const
{
    return Config::apply_visitor(string_length_visitor{}, value);
}

DynamicJsonDocument Config::to_json(const std::vector<String> &keys_to_censor) const {
    DynamicJsonDocument doc(json_size(false));

    JsonVariant var;
    if (is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if (is<Config::ConfArray>() || is<Config::ConfUnion>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    Config::apply_visitor(::to_json{var, keys_to_censor}, value);
    return doc;
}

void Config::save_to_file(File &file)
{
    auto doc = this->to_json({});

    if (doc.overflowed()) {
        logger.printfln("JSON doc overflow while writing file %s! Doc capacity is %u. Truncated doc follows.", file.name(), doc.capacity());
        String str;
        serializeJson(doc, str);
        logger.write(str.c_str(), str.length());
    }
    serializeJson(doc, file);
}

void Config::write_to_stream(Print &output)
{
    write_to_stream_except(output, {});
}

void Config::write_to_stream_except(Print &output, const std::vector<String> &keys_to_censor)
{
    auto doc = this->to_json(keys_to_censor);

    if (doc.overflowed()) {
        logger.printfln("JSON doc overflow while writing to stream! Doc capacity is %u. Truncated doc follows.", doc.capacity());
        String str;
        serializeJson(doc, str);
        logger.write(str.c_str(), str.length());
    }
    serializeJson(doc, output);
}

String Config::to_string() const
{
    return this->to_string_except({});
}

String Config::to_string_except(const std::vector<String> &keys_to_censor) const
{
    auto doc = this->to_json(keys_to_censor);

    String result;
    serializeJson(doc, result);

    if (doc.overflowed()) {
        logger.printfln("JSON doc overflow while converting to string! Doc capacity is %u. Truncated doc follows.", doc.capacity());
        logger.write(result.c_str(), result.length());
    }
    return result;
}

uint8_t Config::was_updated(uint8_t api_backend_flag)
{
    return (value.updated & api_backend_flag) | Config::apply_visitor(is_updated{api_backend_flag}, value);
}

void Config::clear_updated(uint8_t api_backend_flag)
{
    value.updated &= ~api_backend_flag;
    Config::apply_visitor(set_updated_false{api_backend_flag}, value);
}

void Config::set_updated(uint8_t api_backend_flag) {
    value.updated |= api_backend_flag;
}

void config_pre_init()
{
    uint_buf = Config::ConfUint::allocSlotBuf(UINT_SLOTS);
    int_buf = Config::ConfInt::allocSlotBuf(INT_SLOTS);
    float_buf = Config::ConfFloat::allocSlotBuf(FLOAT_SLOTS);
    string_buf = Config::ConfString::allocSlotBuf(STRING_SLOTS);
    array_buf = Config::ConfArray::allocSlotBuf(ARRAY_SLOTS);
    object_buf = Config::ConfObject::allocSlotBuf(OBJECT_SLOTS);
    union_buf = Config::ConfUnion::allocSlotBuf(UNION_SLOTS);

    uint_buf_size = UINT_SLOTS;
    int_buf_size = INT_SLOTS;
    float_buf_size = FLOAT_SLOTS;
    string_buf_size = STRING_SLOTS;
    array_buf_size = ARRAY_SLOTS;
    object_buf_size = OBJECT_SLOTS;
    union_buf_size = UNION_SLOTS;
}

template<typename T>
static void shrinkToFit(typename T::Slot * &buf, size_t &buf_size) {
    size_t highest = 0;
    for (size_t i = 0; i < buf_size; i++)
        if (!T::slotEmpty(i))
            highest = i;

    auto new_buf = T::allocSlotBuf(highest + SLOT_HEADROOM);

    for(size_t i = 0; i <= highest; ++i)
        new_buf[i] = std::move(buf[i]);
    T::freeSlotBuf(buf);
    buf = new_buf;
    buf_size = highest + SLOT_HEADROOM;
}

void config_post_setup() {
    shrinkToFit<Config::ConfUint>(uint_buf, uint_buf_size);
    shrinkToFit<Config::ConfInt>(int_buf, int_buf_size);
    shrinkToFit<Config::ConfFloat>(float_buf, float_buf_size);
    shrinkToFit<Config::ConfString>(string_buf, string_buf_size);
    shrinkToFit<Config::ConfArray>(array_buf, array_buf_size);
    shrinkToFit<Config::ConfObject>(object_buf, object_buf_size);
    shrinkToFit<Config::ConfUnion>(union_buf, union_buf_size);
}
