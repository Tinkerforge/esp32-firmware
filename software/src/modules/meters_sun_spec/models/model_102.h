// WARNING: This file is generated.

#pragma once

#include <stdint.h>
#include <stdlib.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Wattributes"
#endif

// ==============
// 102 - Inverter
// ==============

struct SunSpecInverterModel102_s {
                    uint16_t ID;      //  0
                    uint16_t L;       //  1
                    uint16_t A;       //  2
                    uint16_t AphA;    //  3
                    uint16_t AphB;    //  4
                    uint16_t AphC;    //  5
                    int16_t  A_SF;    //  6
                    uint16_t PPVphAB; //  7
                    uint16_t PPVphBC; //  8
                    uint16_t PPVphCA; //  9
                    uint16_t PhVphA;  // 10
                    uint16_t PhVphB;  // 11
                    uint16_t PhVphC;  // 12
                    int16_t  V_SF;    // 13
                    int16_t  W;       // 14
                    int16_t  W_SF;    // 15
                    uint16_t Hz;      // 16
                    int16_t  Hz_SF;   // 17
                    int16_t  VA;      // 18
                    int16_t  VA_SF;   // 19
                    int16_t  VAr;     // 20
                    int16_t  VAr_SF;  // 21
                    int16_t  PF;      // 22
                    int16_t  PF_SF;   // 23
    [[gnu::packed]] uint32_t WH;      // 24
                    int16_t  WH_SF;   // 26
                    uint16_t DCA;     // 27
                    int16_t  DCA_SF;  // 28
                    uint16_t DCV;     // 29
                    int16_t  DCV_SF;  // 30
                    int16_t  DCW;     // 31
                    int16_t  DCW_SF;  // 32
                    int16_t  TmpCab;  // 33
                    int16_t  TmpSnk;  // 34
                    int16_t  TmpTrns; // 35
                    int16_t  TmpOt;   // 36
                    int16_t  Tmp_SF;  // 37
                    uint16_t St;      // 38
                    uint16_t StVnd;   // 39
    [[gnu::packed]] uint32_t Evt1;    // 40
    [[gnu::packed]] uint32_t Evt2;    // 42
    [[gnu::packed]] uint32_t EvtVnd1; // 44
    [[gnu::packed]] uint32_t EvtVnd2; // 46
    [[gnu::packed]] uint32_t EvtVnd3; // 48
    [[gnu::packed]] uint32_t EvtVnd4; // 50
};

#define SUNSPEC_MODEL_102_NAME "Inverter"

// Total size in bytes, includes ID and length registers.
#define SUNSPEC_MODEL_102_SIZE (104)

// Register count without ID and length registers, should match content of length register.
#define SUNSPEC_MODEL_102_LENGTH (50)

union SunSpecInverterModel102_u {
    struct SunSpecInverterModel102_s model;
    uint16_t registers[SUNSPEC_MODEL_102_SIZE / sizeof(uint16_t)];
};

static_assert(sizeof(SunSpecInverterModel102_s) == 104, "Incorrect Inverter length.");
static_assert(sizeof(SunSpecInverterModel102_u) == 104, "Incorrect Inverter length.");
static_assert(sizeof(static_cast<SunSpecInverterModel102_u *>(nullptr)->registers) == 104, "Incorrect Inverter length.");

#if defined(__GNUC__)
   #pragma GCC diagnostic pop
#endif
