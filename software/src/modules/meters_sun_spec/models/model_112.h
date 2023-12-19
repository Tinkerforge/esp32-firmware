// WARNING: This file is generated.

#pragma once

#include <stdint.h>
#include <stdlib.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Wattributes"
#endif

// ====================
// 112 - Inverter FLOAT
// ====================

struct SunSpecInverterFLOATModel112_s {
                    uint16_t ID;      //  0
                    uint16_t L;       //  1
    [[gnu::packed]] uint32_t A;       //  2
    [[gnu::packed]] uint32_t AphA;    //  4
    [[gnu::packed]] uint32_t AphB;    //  6
    [[gnu::packed]] uint32_t AphC;    //  8
    [[gnu::packed]] uint32_t PPVphAB; // 10
    [[gnu::packed]] uint32_t PPVphBC; // 12
    [[gnu::packed]] uint32_t PPVphCA; // 14
    [[gnu::packed]] uint32_t PhVphA;  // 16
    [[gnu::packed]] uint32_t PhVphB;  // 18
    [[gnu::packed]] uint32_t PhVphC;  // 20
    [[gnu::packed]] uint32_t W;       // 22
    [[gnu::packed]] uint32_t Hz;      // 24
    [[gnu::packed]] uint32_t VA;      // 26
    [[gnu::packed]] uint32_t VAr;     // 28
    [[gnu::packed]] uint32_t PF;      // 30
    [[gnu::packed]] uint32_t WH;      // 32
    [[gnu::packed]] uint32_t DCA;     // 34
    [[gnu::packed]] uint32_t DCV;     // 36
    [[gnu::packed]] uint32_t DCW;     // 38
    [[gnu::packed]] uint32_t TmpCab;  // 40
    [[gnu::packed]] uint32_t TmpSnk;  // 42
    [[gnu::packed]] uint32_t TmpTrns; // 44
    [[gnu::packed]] uint32_t TmpOt;   // 46
                    uint16_t St;      // 48
                    uint16_t StVnd;   // 49
    [[gnu::packed]] uint32_t Evt1;    // 50
    [[gnu::packed]] uint32_t Evt2;    // 52
    [[gnu::packed]] uint32_t EvtVnd1; // 54
    [[gnu::packed]] uint32_t EvtVnd2; // 56
    [[gnu::packed]] uint32_t EvtVnd3; // 58
    [[gnu::packed]] uint32_t EvtVnd4; // 60
};

#define SUNSPEC_MODEL_112_NAME "Inverter FLOAT"

// Total size in bytes, includes ID and length registers.
#define SUNSPEC_MODEL_112_SIZE (124)

// Register count without ID and length registers, should match content of length register.
#define SUNSPEC_MODEL_112_LENGTH (60)

union SunSpecInverterFLOATModel112_u {
    struct SunSpecInverterFLOATModel112_s model;
    uint16_t registers[SUNSPEC_MODEL_112_SIZE / sizeof(uint16_t)];
};

static_assert(sizeof(SunSpecInverterFLOATModel112_s) == 124, "Incorrect Inverter FLOAT length.");
static_assert(sizeof(SunSpecInverterFLOATModel112_u) == 124, "Incorrect Inverter FLOAT length.");
static_assert(sizeof(static_cast<SunSpecInverterFLOATModel112_u *>(nullptr)->registers) == 124, "Incorrect Inverter FLOAT length.");

#if defined(__GNUC__)
   #pragma GCC diagnostic pop
#endif
