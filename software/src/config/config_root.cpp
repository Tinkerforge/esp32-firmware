#include "config/private.h"

#include "config/visitors.h"

ConfigRoot::ConfigRoot(Config cfg) : Config(cfg), validator(nullptr) {}

ConfigRoot::ConfigRoot(Config cfg, std::function<String(Config &)> validator) : Config(cfg), validator(validator) {}

String ConfigRoot::update_from_file(File &file)
{
    DynamicJsonDocument doc(this->json_size(false));
    DeserializationError error = deserializeJson(doc, file);
    if (error)
        return String("Failed to read file: ") + error.c_str();

    return this->update_from_json(doc.as<JsonVariant>(), false);
}

// Intentionally take a non-const char * here:
// This allows ArduinoJson to deserialize in zero-copy mode
String ConfigRoot::update_from_cstr(char *c, size_t len)
{
    Config copy;
    String err = this->get_updated_copy(c, len, &copy);
    if (err != "")
        return err;

    this->update_from_copy(&copy);
    return "";
}

String ConfigRoot::get_updated_copy(char *c, size_t payload_len, Config *out_config) {
    DynamicJsonDocument doc(this->json_size(true));
    DeserializationError error = deserializeJson(doc, c, payload_len);

    switch (error.code()) {
        case DeserializationError::Ok:
            return this->get_updated_copy(doc.as<JsonVariant>(), true, out_config);
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

String ConfigRoot::update_from_json(JsonVariant root, bool force_same_keys)
{
    Config copy;
    String err = this->get_updated_copy(root, force_same_keys, &copy);
    if (err != "")
        return err;

    this->update_from_copy(&copy);
    return "";
}

String ConfigRoot::get_updated_copy(JsonVariant root, bool force_same_keys, Config *out_config)
{
    return this->get_updated_copy(from_json{root, force_same_keys, this->permit_null_updates, true}, out_config);
}


template<typename T>
String ConfigRoot::get_updated_copy(T visitor, Config *out_config) {
    *out_config = *this;
    String err = Config::apply_visitor(visitor, out_config->value);

    if (err != "")
        return err;

    err = Config::apply_visitor(default_validator{}, out_config->value);

    if (err != "")
        return err;

    if (this->validator != nullptr) {
        err = this->validator(*out_config);
        if (err != "")
            return err;
    }
    return "";
}

template<typename T>
String ConfigRoot::update_from_visitor(T visitor) {
    Config copy;

    String err = this->get_updated_copy(visitor, &copy);
    if (err != "")
        return err;

    this->update_from_copy(&copy);
    return "";
}

String ConfigRoot::update(const Config::ConfUpdate *val)
{
    return this->update_from_visitor(from_update{val});
}

String ConfigRoot::validate()
{
    if (this->validator != nullptr) {
        return this->validator(*this);
    }
    return "";
}

void ConfigRoot::update_from_copy(Config *copy) {
    this->value = copy->value;
    this->value.updated = true;
}
