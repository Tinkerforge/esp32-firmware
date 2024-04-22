// WARNING: This file is generated.

#pragma once

#include <stdint.h>
#include <stdlib.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Wattributes"
#endif

// ==========================
// 713 - DER Storage Capacity
// ==========================

struct SunSpecDERStorageCapacityModel713_s {
                    uint16_t ID;      // 0
                    uint16_t L;       // 1
                    uint16_t WHRtg;   // 2
                    uint16_t WHAvail; // 3
                    uint16_t SoC;     // 4
                    uint16_t SoH;     // 5
                    uint16_t Sta;     // 6
                    int16_t  WH_SF;   // 7
                    int16_t  Pct_SF;  // 8
};

#define SUNSPEC_MODEL_713_NAME "DER Storage Capacity"

// Total size in bytes, includes ID and length registers.
#define SUNSPEC_MODEL_713_SIZE (18)

// Register count without ID and length registers, should match content of length register.
#define SUNSPEC_MODEL_713_LENGTH (7)

union SunSpecDERStorageCapacityModel713_u {
    struct SunSpecDERStorageCapacityModel713_s model;
    uint16_t registers[SUNSPEC_MODEL_713_SIZE / sizeof(uint16_t)];
};

static_assert(sizeof(SunSpecDERStorageCapacityModel713_s) == 18, "Incorrect DER Storage Capacity length.");
static_assert(sizeof(SunSpecDERStorageCapacityModel713_u) == 18, "Incorrect DER Storage Capacity length.");
static_assert(sizeof(static_cast<SunSpecDERStorageCapacityModel713_u *>(nullptr)->registers) == 18, "Incorrect DER Storage Capacity length.");

#if defined(__GNUC__)
   #pragma GCC diagnostic pop
#endif
