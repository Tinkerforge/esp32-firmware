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

#include <math.h>

#include "event_log_prefix.h"
#include "main_dependencies.h"
#include "config/visitors.h"
#include "tools.h"
#include "string_builder.h"

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

#define ARRAY_SLOTS 64
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

bool Config::containsNull(const ConfUpdate *update)
{
    return update->which() == 0;
}

bool Config::is_null() const
{
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

Config Config::Object(std::initializer_list<std::pair<const char *, Config>> obj)
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    if (boot_stage >= BootStage::LOOP)
        esp_system_abort("constructing configs in the loop phase is not allowed!");

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

ConfigRoot *Config::Confirm()
{
    if (boot_stage < BootStage::PRE_SETUP)
        esp_system_abort("constructing configs before the pre_setup is not allowed!");

    if (confirmconf == nullptr) {
        confirmconf = new ConfigRoot{Config::Object({
            {Config::confirm_key, Config::Bool(false)}
        })};
    }

    return confirmconf;
}

String Config::ConfirmKey()
{
    return Config::confirm_key;
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

Config::Wrap Config::get()
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfUnion>()) {
        esp_system_abort("Config is not a union!");
    }
    Wrap wrap(value.val.un.getVal());

    return wrap;
}

const Config::ConstWrap Config::get() const
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfUnion>()) {
        esp_system_abort("Config is not a union!");
    }
    ConstWrap wrap(value.val.un.getVal());

    return wrap;
}

Config::Wrap Config::get(const String &s)
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfObject>()) {
        char *message;
        esp_system_abort(asprintf(&message, "Config key %s not in this node: is not an object!", s.c_str()) < 0 ? "" : message);
    }
    Wrap wrap(value.val.o.get(s));

    return wrap;
}

Config::Wrap Config::get(uint16_t i)
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        char *message;
        esp_system_abort(asprintf(&message, "Config index %u not in this node: is not an array!", i) < 0 ? "" : message);
    }
    Wrap wrap(value.val.a.get(i));

    return wrap;
}

const Config::ConstWrap Config::get(const String &s) const
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfObject>()) {
        char *message;
        esp_system_abort(asprintf(&message, "Config key %s not in this node: is not an object!", s.c_str()) < 0 ? "" : message);
    }
    ConstWrap wrap(value.val.o.get(s));

    return wrap;
}

const Config::ConstWrap Config::get(uint16_t i) const
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        char *message;
        esp_system_abort(asprintf(&message, "Config index %u not in this node: is not an array!", i) < 0 ? "" : message);
    }
    ConstWrap wrap(value.val.a.get(i));

    return wrap;
}

Config::Wrap Config::add()
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        esp_system_abort("Tried to add to a node that is not an array!");
    }

    std::vector<Config> &children = this->asArray();

    const auto &arr = value.val.a;
    const auto *slot = arr.getSlot();

    const auto max_elements = slot->maxElements;
    if (children.size() >= max_elements) {
        char *message;
        esp_system_abort(asprintf(&message, "Tried to add to an ConfArray that already has the max allowed number of elements (%u).", max_elements) < 0 ? "" : message);
    }

    auto copy = *slot->prototype;

    // Copying the prototype might invalidate the children reference
    // when ConfArray slots are moved, so asArray() must be called again.
    children = this->asArray();

    children.push_back(std::move(copy));
    this->set_updated(0xFF);
    return Wrap(&children.back());
}

bool Config::removeLast()
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        esp_system_abort("Tried to remove the last element from a node that is not an array!");
    }

    std::vector<Config> &children = this->asArray();
    if (children.size() == 0)
        return false;

    children.pop_back();
    this->set_updated(0xFF);
    return true;
}

bool Config::removeAll()
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        esp_system_abort("Tried to remove all from a node that is not an array!");
    }

    std::vector<Config> &children = this->asArray();

    children.clear();
    this->set_updated(0xFF);
    return true;
}

bool Config::remove(size_t i)
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        esp_system_abort("Tried to remove from a node that is not an array!");
    }
    std::vector<Config> &children = this->asArray();

    if (children.size() <= i)
        return false;

    children.erase(children.begin() + i);
    this->set_updated(0xFF);
    return true;
}

size_t Config::count() const
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        esp_system_abort("Tried to get count of a node that is not an array!");
    }
    const std::vector<Config> &children = this->asArray();
    return children.size();
}

std::vector<Config>::iterator Config::begin()
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        esp_system_abort("Tried to get begin iterator of a node that is not an array!");
    }
    return this->asArray().begin();
}

std::vector<Config>::iterator Config::end()
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        esp_system_abort("Tried to get end iterator of a node that is not an array!");
    }
    return this->asArray().end();
}

std::vector<Config>::const_iterator Config::begin() const
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        esp_system_abort("Tried to get begin iterator of a node that is not an array!");
    }
    return this->asArray().cbegin();
}

std::vector<Config>::const_iterator Config::end() const
{
    ASSERT_MAIN_THREAD();
    if (!this->is<Config::ConfArray>()) {
        esp_system_abort("Tried to get end iterator of a node that is not an array!");
    }
    return this->asArray().cend();
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
    return this->get<ConfFloat>()->getVal();
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

bool Config::clearString()
{
    if (!this->is<ConfString>()) {
        esp_system_abort("Config is not a string!");
    }
    CoolString *val = this->get<ConfString>()->getVal();
    val->clear();
    val->shrinkToFit();
    return true;
}

// Specialize update_value and fillArray:
// ConfFloat::getVal does not return a pointer to the value
// because it is stored as uint32_t in the IRAM.
template<>
inline bool Config::update_value<float, Config::ConfFloat>(float value, const char *value_type) {
    ASSERT_MAIN_THREAD();
    if (!this->is<ConfFloat>()) {
        char *message;
        int result = -1;
#ifndef DEBUG_FS_ENABLE
        result = asprintf(&message, "update_value: Config has wrong type. This is a %s. new value is a %s", this->value.getVariantName(), value_type);
#else
        result = asprintf(&message, "update_value: Config has wrong type. This is a %s. new value is a %s\nContent is %s\nvalue is %s", this->value.getVariantName(), value_type, this->to_string().c_str(), String(value).c_str());
#endif
        esp_system_abort(result < 0 ? "" : message);
    }
    float old_value = get<ConfFloat>()->getVal();
    get<ConfFloat>()->setVal(value);

    if (old_value != value)
        this->value.updated = 0xFF;

    return old_value != value;
}

template<>
inline size_t Config::fillArray<float, Config::ConfFloat>(float *arr, size_t elements) {
    ASSERT_MAIN_THREAD();
    if (!this->is<ConfArray>()) {
        esp_system_abort("Can't fill array, Config is not an array");
    }

    const ConfArray &confArr = this->value.val.a;
    size_t toWrite = std::min(confArr.getVal()->size(), elements);

    for (size_t i = 0; i < toWrite; ++i) {
        const Config *entry = confArr.get(i);
        if (!entry->is<Config::ConfFloat>()) {
            esp_system_abort("Config entry has wrong type.");
        }
        arr[i] = entry->get<Config::ConfFloat>()->getVal();
    }

    return toWrite;
}

bool Config::updateString(const String &value)
{
    if (!value) {
        esp_system_abort("Can't update string, value is invalid string!");
    }
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
    return Config::apply_visitor(max_string_length_visitor{}, value);
}

size_t Config::string_length() const
{
    return Config::apply_visitor(string_length_visitor{}, value);
}

DynamicJsonDocument Config::to_json(const char *const *keys_to_censor, size_t keys_to_censor_len) const
{
    DynamicJsonDocument doc(json_size(true));

    JsonVariant var;
    if (is<Config::ConfObject>()) {
        var = doc.to<JsonObject>();
    } else if (is<Config::ConfArray>() || is<Config::ConfUnion>()) {
        var = doc.to<JsonArray>();
    } else {
        var = doc.as<JsonVariant>();
    }
    Config::apply_visitor(::to_json{var, keys_to_censor, keys_to_censor_len}, value);
    return doc;
}

void Config::save_to_file(File &file)
{
    auto doc = this->to_json(nullptr, 0);

    if (doc.overflowed()) {
        auto capacity = doc.capacity();
        if (capacity == 0) {
            logger.printfln("JSON doc overflow while writing file %s! Doc capacity is zero but needed %u.", file.name(), json_size(false));
        } else {
            logger.printfln("JSON doc overflow while writing file %s! Doc capacity is %u. Truncated doc follows.", file.name(), capacity);
            String str;
            serializeJson(doc, str);
            logger.write(str.c_str(), str.length());
        }
    }
    serializeJson(doc, file);
}

void Config::write_to_stream(Print &output)
{
    write_to_stream_except(output, nullptr, 0);
}

void Config::write_to_stream_except(Print &output, const char *const *keys_to_censor, size_t keys_to_censor_len)
{
    auto doc = this->to_json(keys_to_censor, keys_to_censor_len);

    if (doc.overflowed()) {
        auto capacity = doc.capacity();
        if (capacity == 0) {
            logger.printfln("JSON doc overflow while writing to stream! Doc capacity is zero but needed %u.", json_size(false));
        } else {
            logger.printfln("JSON doc overflow while writing to stream! Doc capacity is %u. Truncated doc follows.", capacity);
            String str;
            serializeJson(doc, str);
            logger.write(str.c_str(), str.length());
        }
    }
    serializeJson(doc, output);
}

String Config::to_string() const
{
    return this->to_string_except(nullptr, 0);
}

String Config::to_string_except(const char *const *keys_to_censor, size_t keys_to_censor_len) const
{
    auto doc = this->to_json(keys_to_censor, keys_to_censor_len);

    String result;
    serializeJson(doc, result);

    // Check for overflow after serializing: Allows to print truncated doc.
    if (doc.overflowed()) {
        auto capacity = doc.capacity();
        if (capacity == 0) {
            logger.printfln("JSON doc overflow while converting to string! Doc capacity is zero but needed %u.", json_size(false));
        } else {
            logger.printfln("JSON doc overflow while converting to string! Doc capacity is %u. Truncated doc follows.", capacity);
            logger.write(result.c_str(), result.length());
        }
    }
    return result;
}

void Config::to_string_except(const char *const *keys_to_censor, size_t keys_to_censor_len, StringBuilder *sb) const
{
    auto doc = this->to_json(keys_to_censor, keys_to_censor_len);
    char *ptr = sb->getRemainingPtr();
    size_t written = serializeJson(doc, ptr, sb->getRemainingLength());

    sb->setLength(sb->getLength() + written);

    if (doc.overflowed()) {
        auto capacity = doc.capacity();
        if (capacity == 0) {
            logger.printfln("JSON doc overflow while converting to string! Doc capacity is zero but needed %u.", json_size(false));
        } else {
            logger.printfln("JSON doc overflow while converting to string! Doc capacity is %u. Truncated doc follows.", capacity);
            logger.write(ptr, written);
        }
    }
}

uint8_t Config::was_updated(uint8_t api_backend_flag)
{
    ASSERT_MAIN_THREAD();
    return (value.updated & api_backend_flag) | Config::apply_visitor(is_updated{api_backend_flag}, value);
}

void Config::clear_updated(uint8_t api_backend_flag)
{
    ASSERT_MAIN_THREAD();
    value.updated &= ~api_backend_flag;
    Config::apply_visitor(set_updated_false{api_backend_flag}, value);
}

void Config::set_updated(uint8_t api_backend_flag)
{
    ASSERT_MAIN_THREAD();
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
    ASSERT_MAIN_THREAD();
    size_t last_used_slot = 0;
    int empty_slots = 0;

    // Search for last used slot first.
    // All empty slots behind the last used slot will be cut off.
    size_t pos = buf_size;
    while (pos > 0) {
        pos--;
        if (!T::slotEmpty(pos)) {
            last_used_slot = pos;
            break;
        }
    }
    while (pos > 0) {
        pos--;
        if (T::slotEmpty(pos)) {
            empty_slots++;
        }
    }

    // Shrink the buffer so that the last used slot fits
    // (we are not allowed to move used slots in the buffer!)
    // and we have SLOT_HEADROOM free slots.
    // If there are empty slots before the last used one, prefer those.
    // If there are not enough, add some empty slots behind the last used one.
    size_t new_size = last_used_slot + 1 + (size_t)std::max(0, SLOT_HEADROOM - empty_slots);

    // Don't increase buffer size. It will be increased automatically when required.
    if (new_size >= buf_size)
        return;

    auto new_buf = T::allocSlotBuf(new_size);

    for (size_t i = 0; i <= last_used_slot; ++i)
        new_buf[i] = std::move(buf[i]);

    T::freeSlotBuf(buf);
    buf = new_buf;
    buf_size = new_size;
}

void config_post_setup()
{
    shrinkToFit<Config::ConfUint>(uint_buf, uint_buf_size);
    shrinkToFit<Config::ConfInt>(int_buf, int_buf_size);
    shrinkToFit<Config::ConfFloat>(float_buf, float_buf_size);
    shrinkToFit<Config::ConfString>(string_buf, string_buf_size);
    shrinkToFit<Config::ConfArray>(array_buf, array_buf_size);
    shrinkToFit<Config::ConfObject>(object_buf, object_buf_size);
    shrinkToFit<Config::ConfUnion>(union_buf, union_buf_size);
}
