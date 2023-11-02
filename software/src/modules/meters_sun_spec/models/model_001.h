// WARNING: This file is generated.

#pragma once

#include <stdint.h>
#include <stdlib.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Wattributes"
#endif

// ============
// 001 - Common
// ============

struct SunSpecCommonModel001_s {
                         uint16_t ID;      //  0
                         uint16_t L;       //  1
                         char     Mn[32];  //  2
                         char     Md[32];  // 18
                         char     Opt[16]; // 34
                         char     Vr[16];  // 42
                         char     SN[32];  // 50
                         uint16_t DA;      // 66
};

#define SUNSPEC_MODEL_001_NAME "Common"

// Total size in bytes, includes ID and length registers.
#define SUNSPEC_MODEL_001_SIZE (134)

// Register count without ID and length registers, should match content of length register.
#define SUNSPEC_MODEL_001_LENGTH (65)

union SunSpecCommonModel001_u {
    struct SunSpecCommonModel001_s model;
    uint16_t registers[SUNSPEC_MODEL_001_SIZE / sizeof(uint16_t)];
};

static_assert(sizeof(SunSpecCommonModel001_s) == 134, "Incorrect Common length.");
static_assert(sizeof(SunSpecCommonModel001_u) == 134, "Incorrect Common length.");
static_assert(sizeof(static_cast<SunSpecCommonModel001_u *>(nullptr)->registers) == 134, "Incorrect Common length.");

#if defined(__GNUC__)
   #pragma GCC diagnostic pop
#endif
