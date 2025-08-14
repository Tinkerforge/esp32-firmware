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

#include "config/private.h"
#include "config/visitors.h"

#include "gcc_warnings.h"

ConfigRoot::ConfigRoot() : validator(nullptr) {}

ConfigRoot::ConfigRoot(Config cfg) : Config(cfg), validator(nullptr) {}

ConfigRoot::ConfigRoot(Config cfg, Validator &&validator_) : Config(cfg), validator(new Validator(std::move(validator_))) {}

String ConfigRoot::update_from_file(File &&file)
{
    DynamicJsonDocument doc(std::max(4096u, this->json_size(false)));
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        return String("Failed to read file: ") + error.c_str();

    file.close();

    return this->update_from_json(doc.as<JsonVariant>(), false, ConfigSource::File);
}

// Intentionally take a non-const char * here:
// This allows ArduinoJson to deserialize in zero-copy mode
String ConfigRoot::update_from_cstr(char *c, size_t len, const Config::Key *config_path, size_t config_path_len)
{
    ASSERT_MAIN_THREAD();
    Config copy;
    String err = this->get_updated_copy(c, len, &copy, ConfigSource::API, config_path, config_path_len);
    if (!err.isEmpty())
        return err;

    this->update_from_copy(&copy);
    return "";
}

// Don't inline; keeps the buffer off the stack when there isn't any error.
[[gnu::noinline]]
static String get_updated_copy_error(DeserializationError error, size_t payload_len)
{
    char buf[160];
    StringWriter sw(buf, sizeof(buf));

    sw.puts("Failed to deserialize: ");

    switch (error.code()) {
        case DeserializationError::Ok:
            break;
        case DeserializationError::NoMemory:
            sw.puts("JSON payload was longer than expected and possibly contained unknown keys.");
            break;
        case DeserializationError::EmptyInput:
            sw.puts("Payload was empty. Please send valid JSON.");
            break;
        case DeserializationError::IncompleteInput:
            sw.puts("JSON payload incomplete or truncated.");
            break;
        case DeserializationError::InvalidInput:
            sw.puts("JSON payload could not be parsed.");
            break;
        case DeserializationError::TooDeep:
            sw.puts("JSON payload nested too deeply.");
            break;
        default:
            sw.puts(error.c_str());
            break;
    }

    sw.printf(" Payload length was %zu.", payload_len);

    return String(buf, sw.getLength());
}

String ConfigRoot::get_updated_copy(char *c, size_t payload_len, Config *out_config, ConfigSource source, const Config::Key *config_path, size_t config_path_len)
{
    DynamicJsonDocument doc(this->json_size(true));
    DeserializationError error = deserializeJson(doc, c, payload_len);

    if (error.code() != DeserializationError::Ok) {
        return get_updated_copy_error(error, payload_len);
    }

    return this->get_updated_copy(doc.as<JsonVariant>(), true, out_config, source, config_path, config_path_len);
}

String ConfigRoot::update_from_json(JsonVariant root, bool force_same_keys, ConfigSource source, const Config::Key *config_path, size_t config_path_len)
{
    Config copy;
    String err = this->get_updated_copy(root, force_same_keys, &copy, source, config_path, config_path_len);
    if (!err.isEmpty())
        return err;

    this->update_from_copy(&copy);
    return "";
}

String ConfigRoot::get_updated_copy(JsonVariant root, bool force_same_keys, Config *out_config, ConfigSource source, const Config::Key *config_path, size_t config_path_len)
{
    String result = this->get_updated_copy(from_json{root, force_same_keys, this->get_permit_null_updates(), true}, out_config, source, config_path, config_path_len);
    // The from_json visitor can report multiple errors with newlines at the end of each line. Remove the last newline.
    result.trim();
    return result;
}

template<typename T>
String ConfigRoot::get_updated_copy(T visitor, Config *out_config, ConfigSource source, const Config::Key *config_path, size_t config_path_len) {
    ASSERT_MAIN_THREAD();
    *out_config = *this;
    Config *sub_config = out_config->walk(config_path, config_path_len);
    if (sub_config == nullptr)
        return "path not found";

    UpdateResult res = Config::apply_visitor(visitor, sub_config->value);

    if (!res.message.isEmpty())
        return res.message;

    out_config->set_updated(res.changed ? 0xFF : 0);

    String err = Config::apply_visitor(default_validator{}, out_config->value);

    if (!err.isEmpty())
        return err;

    auto *validator_ = reinterpret_cast<ConfigRoot::Validator *>(reinterpret_cast<std::uintptr_t>(this->validator) & (~0x01u));

    if (validator_ != nullptr) {
        err = (*validator_)(*out_config, source);
        if (!err.isEmpty())
            return err;
    }
    return "";
}

template<typename T>
String ConfigRoot::update_from_visitor(T visitor, ConfigSource source, const Config::Key *config_path, size_t config_path_len) {
    ASSERT_MAIN_THREAD();
    Config copy;

    String err = this->get_updated_copy(visitor, &copy, source, config_path, config_path_len);
    if (!err.isEmpty())
        return err;

    this->update_from_copy(&copy);
    return "";
}

String ConfigRoot::update(const Config::ConfUpdate *val)
{
    ASSERT_MAIN_THREAD();
    return this->update_from_visitor(from_update{val}, ConfigSource::Code);
}

String ConfigRoot::validate(ConfigSource source)
{
    ASSERT_MAIN_THREAD();
    auto *validator_ = reinterpret_cast<ConfigRoot::Validator *>(reinterpret_cast<std::uintptr_t>(this->validator) & (~0x01u));

    if (validator_ != nullptr) {
        return (*validator_)(*this, source);
    }
    return "";
}

void ConfigRoot::update_from_copy(Config *copy)
{
    ASSERT_MAIN_THREAD();
    this->value = copy->value;
}

void ConfigRoot::set_permit_null_updates(bool permit_null_updates) {
    // Store permit_null_updates == true as 0 and == false as 1
    // so that the default value is permitted.
    if (permit_null_updates)
        this->validator = reinterpret_cast<ConfigRoot::Validator *>(reinterpret_cast<std::uintptr_t>(this->validator) & (~0x01u));
    else
        this->validator = reinterpret_cast<ConfigRoot::Validator *>(reinterpret_cast<std::uintptr_t>(this->validator) | 0x01u);
}

bool ConfigRoot::get_permit_null_updates() {
    // Inverted; see set_permit_null_updates.
    return (reinterpret_cast<std::uintptr_t>(this->validator) & 0x01) == 0;
}

#ifdef DEBUG_FS_ENABLE
void ConfigRoot::print_api_info(StringWriter &sw) {
    Config::apply_visitor(api_info{sw}, this->value);
}
#endif
