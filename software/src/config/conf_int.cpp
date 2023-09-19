#include "config/private.h"

bool Config::ConfInt::slotEmpty(size_t i) {
    return int_buf[i].val == 0
        && int_buf[i].min == 0
        && int_buf[i].max == 0;
}

Config::ConfInt::Slot *Config::ConfInt::allocSlotBuf(size_t elements) {
    return (Config::ConfInt::Slot *) heap_caps_calloc(elements, sizeof(Config::ConfInt::Slot), MALLOC_CAP_32BIT);
}

void Config::ConfInt::freeSlotBuf(Config::ConfInt::Slot *buf) {
    heap_caps_free(buf);
}

int32_t* Config::ConfInt::getVal() { return &int_buf[idx].val; }
const int32_t* Config::ConfInt::getVal() const { return &int_buf[idx].val; }

const Config::ConfInt::Slot *Config::ConfInt::getSlot() const { return &int_buf[idx]; }
Config::ConfInt::Slot *Config::ConfInt::getSlot() { return &int_buf[idx]; }

Config::ConfInt::ConfInt(int32_t val, int32_t min, int32_t max)
{
    idx = nextSlot<Config::ConfInt>(int_buf, int_buf_size);
    auto *slot = this->getSlot();
    slot->val = val;
    slot->min = min;
    slot->max = max;
}

Config::ConfInt::ConfInt(const ConfInt &cpy)
{
    idx = nextSlot<Config::ConfInt>(int_buf, int_buf_size);
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfInt::~ConfInt()
{
    auto *slot = this->getSlot();
    slot->val = 0;
    slot->min = 0;
    slot->max = 0;
}

Config::ConfInt& Config::ConfInt::operator=(const ConfInt &cpy) {
    if (this == &cpy)
        return *this;

    *this->getSlot() = *cpy.getSlot();

    return *this;
}
