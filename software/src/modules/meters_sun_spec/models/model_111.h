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
// 111 - Inverter FLOAT
// ====================

struct SunSpecInverterFLOATModel111_s {
                         uint16_t ID;      //  0
                         uint16_t L;       //  1
    _ATTRIBUTE((packed)) uint32_t A;       //  2
    _ATTRIBUTE((packed)) uint32_t AphA;    //  4
    _ATTRIBUTE((packed)) uint32_t AphB;    //  6
    _ATTRIBUTE((packed)) uint32_t AphC;    //  8
    _ATTRIBUTE((packed)) uint32_t PPVphAB; // 10
    _ATTRIBUTE((packed)) uint32_t PPVphBC; // 12
    _ATTRIBUTE((packed)) uint32_t PPVphCA; // 14
    _ATTRIBUTE((packed)) uint32_t PhVphA;  // 16
    _ATTRIBUTE((packed)) uint32_t PhVphB;  // 18
    _ATTRIBUTE((packed)) uint32_t PhVphC;  // 20
    _ATTRIBUTE((packed)) uint32_t W;       // 22
    _ATTRIBUTE((packed)) uint32_t Hz;      // 24
    _ATTRIBUTE((packed)) uint32_t VA;      // 26
    _ATTRIBUTE((packed)) uint32_t VAr;     // 28
    _ATTRIBUTE((packed)) uint32_t PF;      // 30
    _ATTRIBUTE((packed)) uint32_t WH;      // 32
    _ATTRIBUTE((packed)) uint32_t DCA;     // 34
    _ATTRIBUTE((packed)) uint32_t DCV;     // 36
    _ATTRIBUTE((packed)) uint32_t DCW;     // 38
    _ATTRIBUTE((packed)) uint32_t TmpCab;  // 40
    _ATTRIBUTE((packed)) uint32_t TmpSnk;  // 42
    _ATTRIBUTE((packed)) uint32_t TmpTrns; // 44
    _ATTRIBUTE((packed)) uint32_t TmpOt;   // 46
                         uint16_t St;      // 48
                         uint16_t StVnd;   // 49
    _ATTRIBUTE((packed)) uint32_t Evt1;    // 50
    _ATTRIBUTE((packed)) uint32_t Evt2;    // 52
    _ATTRIBUTE((packed)) uint32_t EvtVnd1; // 54
    _ATTRIBUTE((packed)) uint32_t EvtVnd2; // 56
    _ATTRIBUTE((packed)) uint32_t EvtVnd3; // 58
    _ATTRIBUTE((packed)) uint32_t EvtVnd4; // 60
};

#define SUNSPEC_MODEL_111_NAME "Inverter FLOAT"

// Total size in bytes, includes ID and length registers.
#define SUNSPEC_MODEL_111_SIZE (124)

// Register count without ID and length registers, should match content of length register.
#define SUNSPEC_MODEL_111_LENGTH (60)

union SunSpecInverterFLOATModel111_u {
    struct SunSpecInverterFLOATModel111_s model;
    uint16_t registers[SUNSPEC_MODEL_111_SIZE / sizeof(uint16_t)];
};

static_assert(sizeof(SunSpecInverterFLOATModel111_s) == 124, "Incorrect Inverter FLOAT length.");
static_assert(sizeof(SunSpecInverterFLOATModel111_u) == 124, "Incorrect Inverter FLOAT length.");
static_assert(sizeof(static_cast<SunSpecInverterFLOATModel111_u *>(nullptr)->registers) == 124, "Incorrect Inverter FLOAT length.");

#if defined(__GNUC__)
   #pragma GCC diagnostic pop
#endif
