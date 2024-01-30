#include "config/private.h"

bool Config::ConfObject::slotEmpty(size_t i)
{
    return object_buf[i].schema == nullptr;
}

Config::ConfObject::Slot *Config::ConfObject::allocSlotBuf(size_t elements)
{
    return new Config::ConfObject::Slot[elements];
}

void Config::ConfObject::freeSlotBuf(Config::ConfObject::Slot *buf)
{
    delete[] buf;
}

Config *Config::ConfObject::get(const String &needle)
{
    const auto *slot = this->getSlot();
    const auto schema = slot->schema;
    const auto size = schema->length;

    const auto needle_length = needle.length();

    for (size_t i = 0; i < size; ++i) {
        if (schema->key_lengths[i] != needle_length)
            continue;

        if (memcmp(schema->keys[i], needle.c_str(), needle_length) == 0)
            return &slot->values[i];
    }

    logger.printfln("Config key %s not found!", needle.c_str());
    esp_system_abort("");
}

const Config *Config::ConfObject::get(const String &needle) const
{
    const auto *slot = this->getSlot();
    const auto schema = slot->schema;
    const auto size = schema->length;

    const auto needle_length = needle.length();

    for (size_t i = 0; i < size; ++i) {
        if (schema->key_lengths[i] != needle_length)
            continue;

        if (memcmp(schema->keys[i], needle.c_str(), needle_length) == 0)
            return &slot->values[i];
    }

    logger.printfln("Config key %s not found!", needle.c_str());
    esp_system_abort("");
}

const Config::ConfObject::Slot *Config::ConfObject::getSlot() const { return &object_buf[idx]; }
Config::ConfObject::Slot *Config::ConfObject::getSlot() { return &object_buf[idx]; }

Config::ConfObject::ConfObject(std::vector<std::pair<String, Config>> &&val)
{
    auto len = val.size();
    auto schema = new ConfObjectSchema{len, heap_alloc_array<uint8_t>(len), heap_alloc_array<char *>(len)};

    size_t buf_len = 0;
    for (int i = 0; i < len; ++i) {
        if (val[i].first.length() > 255)
            esp_system_abort("ConfObject key was longer than 255 chars!");

        schema->key_lengths[i] = val[i].first.length();
        buf_len += schema->key_lengths[i] + 1;
    }

    char *key_buf = (char *)heap_caps_calloc_prefer(buf_len, sizeof(char), 2, MALLOC_CAP_SPIRAM, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

    size_t written = 0;
    for (int i = 0; i < len; ++i) {
        schema->keys[i] = key_buf + written;
        memcpy(key_buf + written, val[i].first.c_str(), schema->key_lengths[i]);
        written += schema->key_lengths[i];
        key_buf[written] = '\0';
        ++written;
    }

    idx = nextSlot<Config::ConfObject>(object_buf, object_buf_size);
    auto *slot = this->getSlot();
    slot->schema = schema;
    slot->values = heap_alloc_array<Config>(len);

    for (int i = 0; i < len; ++i) {
        this->getSlot()->values[i] = std::move(val[i].second); //TODO: move here?
    }
}

Config::ConfObject::ConfObject(const ConfObject &cpy)
{
    idx = nextSlot<Config::ConfObject>(object_buf, object_buf_size);

    // TODO: could we just use *this = cpy here?

    // We have to mark this slot as in use here:
    // (by setting the schema pointer first)
    // This object could contain a nested object that will be copied over
    // The inner object's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as in use first.
    this->getSlot()->schema = cpy.getSlot()->schema;

    const auto len = cpy.getSlot()->schema->length;
    auto values = heap_alloc_array<Config>(len);
    for (int i = 0; i < len; ++i)
        values[i] = cpy.getSlot()->values[i];

    // Must call getSlot() again because any reference would be invalidated
    // if the copy triggers a move of the slots.
    this->getSlot()->values = std::move(values);
}

Config::ConfObject::~ConfObject()
{
    auto *slot = this->getSlot();
    slot->schema = nullptr;
    slot->values = nullptr;
}

Config::ConfObject &Config::ConfObject::operator=(const ConfObject &cpy)
{
    if (this == &cpy)
        return *this;

    // We have to mark this slot as in use here:
    // (by setting the schema pointer first)
    // This object could contain a nested object that will be copied over
    // The inner object's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as in use first.
    this->getSlot()->schema = cpy.getSlot()->schema;

    const auto len = cpy.getSlot()->schema->length;
    auto values = heap_alloc_array<Config>(len);
    for (int i = 0; i < len; ++i)
        values[i] = cpy.getSlot()->values[i];

    // Must call getSlot() again because any reference would be invalidated
    // if the copy triggers a move of the slots.
    this->getSlot()->values = std::move(values);

    return *this;
}
