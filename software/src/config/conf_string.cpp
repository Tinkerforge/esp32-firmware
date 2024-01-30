#include "config/private.h"

bool Config::ConfString::slotEmpty(size_t i)
{
    return !string_buf[i].inUse;
}

Config::ConfString::Slot *Config::ConfString::allocSlotBuf(size_t elements)
{
    return new Config::ConfString::Slot[elements];
}

void Config::ConfString::freeSlotBuf(Config::ConfString::Slot *buf)
{
    delete[] buf;
}

CoolString* Config::ConfString::getVal() { return &string_buf[idx].val; }
const CoolString* Config::ConfString::getVal() const { return &string_buf[idx].val; }

const Config::ConfString::Slot* Config::ConfString::getSlot() const { return &string_buf[idx]; }
Config::ConfString::Slot* Config::ConfString::getSlot() { return &string_buf[idx]; }

Config::ConfString::ConfString(const CoolString &val, uint16_t minChars, uint16_t maxChars)
{
    idx = nextSlot<Config::ConfString>(string_buf, string_buf_size);
    auto *slot = this->getSlot();
    slot->inUse = true;

    slot->val = val;
    slot->minChars = minChars;
    slot->maxChars = maxChars;
}

Config::ConfString::ConfString(const ConfString &cpy)
{
    idx = nextSlot<Config::ConfString>(string_buf, string_buf_size);

    // If cpy->inUse is false, it is okay that we don't mark this slot as inUse.

    // this->getSlot() is evaluated before the RHS of the assignment is copied over.
    // This results in the LHS pointing to a deallocated array if copying the RHS
    // resizes the slot array. Copying into a temp value (which resizes the array if necessary)
    // and moving this value in the slot works.
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfString::~ConfString()
{
    auto *slot = this->getSlot();
    string_buf[idx].inUse = false;

    slot->val.clear();
    slot->val.shrinkToFit();
    slot->minChars = 0;
    slot->maxChars = 0;
}

Config::ConfString &Config::ConfString::operator=(const ConfString &cpy)
{
    if (this == &cpy) {
        return *this;
    }

    *this->getSlot() = *cpy.getSlot();

    return *this;
}
