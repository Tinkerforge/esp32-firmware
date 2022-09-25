#pragma once

#include <stdint.h>
#include <stddef.h>

#define PHASE_ACTIVE_CURRENT_THRES 0.3f // ampere
#define PHASE_CONNECTED_VOLTAGE_THRES 180.0f // volts

struct RegRead {
    uint16_t start;
    uint16_t len;
};

struct MeterInfo {
    uint16_t meter_id; // read from holding register 64515
    uint8_t meter_type; // will be written into meter/state["type"] if holding register 64515 contains the meter_id

    const RegRead *to_read_slow;
    size_t to_read_slow_len;

    const RegRead *to_read_fast;
    size_t to_read_fast_len;

    void (*const slow_read_done_fn)(const uint16_t *registers);
    void (*const fast_read_done_fn)(const uint16_t *registers);
    const char *meter_name;
    void (*const custom_reset_fn)(); // set to nullptr if reset via register 61457 is supported
};

void convert_to_float(const uint16_t *regs, float *target, const uint16_t *indices, size_t count);
