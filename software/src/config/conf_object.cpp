#include "config/private.h"

bool Config::ConfObject::slotEmpty(size_t i) {
    return !object_buf[i].inUse;
}

Config::ConfObject::Slot *Config::ConfObject::allocSlotBuf(size_t elements) {
    return new Config::ConfObject::Slot[elements];
}

void Config::ConfObject::freeSlotBuf(Config::ConfObject::Slot *buf) {
    delete[] buf;
}

Config *Config::ConfObject::get(const String &s)
{
    auto *val = this->getVal();
    const auto size = val->size();

    for (size_t i = 0; i < size; ++i) {
        auto &val_pair = (*val)[i];
        if (val_pair.first == s)
            return &val_pair.second;
    }

    logger.printfln("Config key %s not found!", s.c_str());
    delay(100);
    return nullptr;
}

const Config *Config::ConfObject::get(const String &s) const
{
    const auto *val = this->getVal();
    const auto size = val->size();

    for (size_t i = 0; i < size; ++i) {
        const auto &val_pair = (*val)[i];
        if (val_pair.first == s)
            return &val_pair.second;
    }

    logger.printfln("Config key %s not found!", s.c_str());
    delay(100);
    return nullptr;
}

std::vector<std::pair<String, Config>> *Config::ConfObject::getVal() { return &object_buf[idx].val; }
const std::vector<std::pair<String, Config>> *Config::ConfObject::getVal() const { return &object_buf[idx].val; }

const Config::ConfObject::Slot *Config::ConfObject::getSlot() const { return &object_buf[idx]; }
Config::ConfObject::Slot *Config::ConfObject::getSlot() { return &object_buf[idx]; }

Config::ConfObject::ConfObject(std::vector<std::pair<String, Config>> val)
{
    idx = nextSlot<Config::ConfObject>(object_buf, object_buf_size);
    auto *slot = this->getSlot();
    slot->inUse = true;

    slot->val = val;
}

Config::ConfObject::ConfObject(const ConfObject &cpy)
{
    idx = nextSlot<Config::ConfObject>(object_buf, object_buf_size);
    // We have to mark this slot as in use here:
    // This object could contain a nested object that will be copied over
    // The inner object's copy constructor then takes the first free slot, i.e.
    // ours if we don't mark it as inUse first.
    this->getSlot()->inUse = true;

    auto tmp = *cpy.getSlot();
    // Must call getSlot() again because any reference would be invalidated
    // if the copy triggers a move of the slots.
    *this->getSlot() = std::move(tmp);
}

Config::ConfObject::~ConfObject()
{
    auto *slot = this->getSlot();
    slot->inUse = false;

    slot->val.clear();
}

Config::ConfObject& Config::ConfObject::operator=(const ConfObject &cpy) {
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}
