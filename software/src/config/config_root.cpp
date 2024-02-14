#include "config/private.h"

#include "config/visitors.h"

ConfigRoot::ConfigRoot(Config cfg) : Config(cfg), validator(nullptr) {}

ConfigRoot::ConfigRoot(Config cfg, Validator &&validator) : Config(cfg), validator(new Validator(std::forward<Validator>(validator))) {}

String ConfigRoot::update_from_file(File &&file)
{
    DynamicJsonDocument doc(this->json_size(false));
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        return String("Failed to read file: ") + error.c_str();

    file.close();

    return this->update_from_json(doc.as<JsonVariant>(), false, ConfigSource::File);
}

// Intentionally take a non-const char * here:
// This allows ArduinoJson to deserialize in zero-copy mode
String ConfigRoot::update_from_cstr(char *c, size_t len)
{
    ASSERT_MAIN_THREAD();
    Config copy;
    String err = this->get_updated_copy(c, len, &copy, ConfigSource::API);
    if (!err.isEmpty())
        return err;

    this->update_from_copy(&copy);
    return "";
}

String ConfigRoot::get_updated_copy(char *c, size_t payload_len, Config *out_config, ConfigSource source)
{
    DynamicJsonDocument doc(this->json_size(true));
    DeserializationError error = deserializeJson(doc, c, payload_len);

    switch (error.code()) {
        case DeserializationError::Ok:
            return this->get_updated_copy(doc.as<JsonVariant>(), true, out_config, source);
        case DeserializationError::NoMemory:
            return String("Failed to deserialize: JSON payload was longer than expected and possibly contained unknown keys.");
        case DeserializationError::EmptyInput:
            return String("Failed to deserialize: Payload was empty. Please send valid JSON.");
        case DeserializationError::IncompleteInput:
            return String("Failed to deserialize: JSON payload incomplete or truncated");
        case DeserializationError::InvalidInput:
            return String("Failed to deserialize: JSON payload could not be parsed");
        case DeserializationError::TooDeep:
            return String("Failed to deserialize: JSON payload nested too deep");
        default:
            return String("Failed to deserialize string: ") + String(error.c_str());
    }
}

String ConfigRoot::update_from_json(JsonVariant root, bool force_same_keys, ConfigSource source)
{
    Config copy;
    String err = this->get_updated_copy(root, force_same_keys, &copy, source);
    if (!err.isEmpty())
        return err;

    this->update_from_copy(&copy);
    return "";
}

String ConfigRoot::get_updated_copy(JsonVariant root, bool force_same_keys, Config *out_config, ConfigSource source)
{
    String result = this->get_updated_copy(from_json{root, force_same_keys, this->get_permit_null_updates(), true}, out_config, source);
    // The from_json visitor can report multiple errors with newlines at the end of each line. Remove the last newline.
    result.trim();
    return result;
}

template<typename T>
String ConfigRoot::get_updated_copy(T visitor, Config *out_config, ConfigSource source) {
    ASSERT_MAIN_THREAD();
    *out_config = *this;
    String err = Config::apply_visitor(visitor, out_config->value);

    if (!err.isEmpty())
        return err;

    err = Config::apply_visitor(default_validator{}, out_config->value);

    if (!err.isEmpty())
        return err;

    auto *validator = (ConfigRoot::Validator *)(((std::uintptr_t)this->validator) & (~0x01));

    if (validator != nullptr) {
        err = (*validator)(*out_config, source);
        if (!err.isEmpty())
            return err;
    }
    return "";
}

template<typename T>
String ConfigRoot::update_from_visitor(T visitor, ConfigSource source) {
    ASSERT_MAIN_THREAD();
    Config copy;

    String err = this->get_updated_copy(visitor, &copy, source);
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
    auto *validator = (ConfigRoot::Validator *)(((std::uintptr_t)this->validator) & (~0x01));

    if (validator != nullptr) {
        return (*validator)(*this, source);
    }
    return "";
}

void ConfigRoot::update_from_copy(Config *copy)
{
    ASSERT_MAIN_THREAD();
    this->value = copy->value;
    this->value.updated = 0xFF;
}

OwnedConfig ConfigRoot::get_owned_copy()
{
    return Config::apply_visitor(to_owned{}, this->value);
}

void ConfigRoot::set_permit_null_updates(bool permit_null_updates) {
    // Store permit_null_updates == true as 0 and == false as 1
    // so that the default value is permitted.
    if (permit_null_updates)
        this->validator = (ConfigRoot::Validator *)(((std::uintptr_t)this->validator) & (~0x01));
    else
        this->validator = (ConfigRoot::Validator *)(((std::uintptr_t)this->validator) | 0x01);
}

bool ConfigRoot::get_permit_null_updates() {
    // Inverted; see set_permit_null_updates.
    return (((std::uintptr_t)this->validator) & 0x01) == 0;
}
