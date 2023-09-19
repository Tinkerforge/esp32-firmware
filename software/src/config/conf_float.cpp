#include "config/private.h"

bool Config::ConfFloat::slotEmpty(size_t i) {
    return float_buf[i].val == 0
        && float_buf[i].min == 0
        && float_buf[i].max == 0;
}
Config::ConfFloat::Slot *Config::ConfFloat::allocSlotBuf(size_t elements) {
    return new Config::ConfFloat::Slot[elements];
}

void Config::ConfFloat::freeSlotBuf(Config::ConfFloat::Slot *buf) {
    delete[] buf;
}

float* Config::ConfFloat::getVal() { return &float_buf[idx].val; }
const float* Config::ConfFloat::getVal() const { return &float_buf[idx].val; }

const Config::ConfFloat::Slot *Config::ConfFloat::getSlot() const { return &float_buf[idx]; }
Config::ConfFloat::Slot *Config::ConfFloat::getSlot() { return &float_buf[idx]; }

Config::ConfFloat::ConfFloat(float val, float min, float max)
{
    idx = nextSlot<Config::ConfFloat>(float_buf, float_buf_size);
    auto *slot = this->getSlot();
    slot->val = val;
    slot->min = min;
    slot->max = max;
}

Config::ConfFloat::ConfFloat(const ConfFloat &cpy)
{
    idx = nextSlot<Config::ConfFloat>(float_buf, float_buf_size);
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfFloat::~ConfFloat()
{
    auto *slot = this->getSlot();
    slot->val = 0;
    slot->min = 0;
    slot->max = 0;
}

Config::ConfFloat& Config::ConfFloat::operator=(const ConfFloat &cpy) {
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}
