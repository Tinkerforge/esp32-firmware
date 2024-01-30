#include "config/private.h"

bool Config::ConfUnion::slotEmpty(size_t i)
{
    return union_buf[i].prototypes == nullptr;
}

Config::ConfUnion::Slot *Config::ConfUnion::allocSlotBuf(size_t elements)
{
    return new Config::ConfUnion::Slot[elements];
}

void Config::ConfUnion::freeSlotBuf(Config::ConfUnion::Slot *buf)
{
    delete[] buf;
}

bool Config::ConfUnion::changeUnionVariant(uint8_t tag)
{
    auto &slot = union_buf[idx];
    for (int i = 0; i < slot.prototypes_len; ++i) {
        if (slot.prototypes[i].tag == tag) {
            union_buf[idx].tag = tag;
            slot.val = slot.prototypes[i].config;
            return true;
        }
    }

    return false;
}
uint8_t Config::ConfUnion::getTag() const { return union_buf[idx].tag; }

Config* Config::ConfUnion::getVal() { return &union_buf[idx].val; }
const Config* Config::ConfUnion::getVal() const { return &union_buf[idx].val; }

const Config::ConfUnion::Slot* Config::ConfUnion::getSlot() const { return &union_buf[idx]; }
Config::ConfUnion::Slot* Config::ConfUnion::getSlot() { return &union_buf[idx]; }

Config::ConfUnion::ConfUnion(const Config &val, uint8_t tag, uint8_t prototypes_len, const ConfUnionPrototypeInternal prototypes[])
{
    idx = nextSlot<Config::ConfUnion>(union_buf, union_buf_size);

    auto *slot = this->getSlot();
    slot->val = val;
    slot->tag = tag;
    slot->prototypes_len = prototypes_len;
    slot->prototypes = prototypes;
}

Config::ConfUnion::ConfUnion(const ConfUnion &cpy)
{
    idx = nextSlot<Config::ConfUnion>(union_buf, union_buf_size);

    // If cpy->inUse is false, it is okay that we don't mark this slot as inUse.

    // this->getSlot() is evaluated before the RHS of the assignment is copied over.
    // This results in the LHS pointing to a deallocated array if copying the RHS
    // resizes the slot array. Copying into a temp value (which resizes the array if necessary)
    // and moving this value in the slot works.
    auto tmp = *cpy.getSlot();
    *this->getSlot() = std::move(tmp);
}

Config::ConfUnion::~ConfUnion()
{
    auto *slot = this->getSlot();
    slot->val = *Config::Null();
    slot->tag = 0;
    slot->prototypes_len = 0;
    slot->prototypes = nullptr;
}

Config::ConfUnion &Config::ConfUnion::operator=(const ConfUnion &cpy)
{
    if (this == &cpy) {
        return *this;
    }

    *this->getSlot() = *cpy.getSlot();

    return *this;
}
