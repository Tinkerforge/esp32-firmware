#include "meter_defs.h"

void convert_to_float(const uint16_t *regs, float *target, const uint16_t *indices, size_t count)
{
    union {
        float f;
        uint16_t regs[2];
    } value;

    for (size_t i = 0; i < count; ++i) {
        size_t reg = indices[i] - 1; // -1: convert from register to address
        value.regs[1] = regs[reg + 0];
        value.regs[0] = regs[reg + 1];
        target[i] = value.f;
    }
}
