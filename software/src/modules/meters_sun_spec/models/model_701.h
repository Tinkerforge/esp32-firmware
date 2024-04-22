// WARNING: This file is generated.

#pragma once

#include <stdint.h>
#include <stdlib.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Wattributes"
#endif

// ========================
// 701 - DER AC Measurement
// ========================

struct SunSpecDERACMeasurementModel701_s {
                    uint16_t ID;             //   0
                    uint16_t L;              //   1
                    uint16_t ACType;         //   2
                    uint16_t St;             //   3
                    uint16_t InvSt;          //   4
                    uint16_t ConnSt;         //   5
    [[gnu::packed]] uint32_t Alrm;           //   6
    [[gnu::packed]] uint32_t DERMode;        //   8
                    int16_t  W;              //  10
                    int16_t  VA;             //  11
                    int16_t  Var;            //  12
                    int16_t  PF;             //  13
                    int16_t  A;              //  14
                    uint16_t LLV;            //  15
                    uint16_t LNV;            //  16
    [[gnu::packed]] uint32_t Hz;             //  17
    [[gnu::packed]] uint64_t TotWhInj;       //  19
    [[gnu::packed]] uint64_t TotWhAbs;       //  23
    [[gnu::packed]] uint64_t TotVarhInj;     //  27
    [[gnu::packed]] uint64_t TotVarhAbs;     //  31
                    int16_t  TmpAmb;         //  35
                    int16_t  TmpCab;         //  36
                    int16_t  TmpSnk;         //  37
                    int16_t  TmpTrns;        //  38
                    int16_t  TmpSw;          //  39
                    int16_t  TmpOt;          //  40
                    int16_t  WL1;            //  41
                    int16_t  VAL1;           //  42
                    int16_t  VarL1;          //  43
                    int16_t  PFL1;           //  44
                    int16_t  AL1;            //  45
                    uint16_t VL1L2;          //  46
                    uint16_t VL1;            //  47
    [[gnu::packed]] uint64_t TotWhInjL1;     //  48
    [[gnu::packed]] uint64_t TotWhAbsL1;     //  52
    [[gnu::packed]] uint64_t TotVarhInjL1;   //  56
    [[gnu::packed]] uint64_t TotVarhAbsL1;   //  60
                    int16_t  WL2;            //  64
                    int16_t  VAL2;           //  65
                    int16_t  VarL2;          //  66
                    int16_t  PFL2;           //  67
                    int16_t  AL2;            //  68
                    uint16_t VL2L3;          //  69
                    uint16_t VL2;            //  70
    [[gnu::packed]] uint64_t TotWhInjL2;     //  71
    [[gnu::packed]] uint64_t TotWhAbsL2;     //  75
    [[gnu::packed]] uint64_t TotVarhInjL2;   //  79
    [[gnu::packed]] uint64_t TotVarhAbsL2;   //  83
                    int16_t  WL3;            //  87
                    int16_t  VAL3;           //  88
                    int16_t  VarL3;          //  89
                    int16_t  PFL3;           //  90
                    int16_t  AL3;            //  91
                    uint16_t VL3L1;          //  92
                    uint16_t VL3;            //  93
    [[gnu::packed]] uint64_t TotWhInjL3;     //  94
    [[gnu::packed]] uint64_t TotWhAbsL3;     //  98
    [[gnu::packed]] uint64_t TotVarhInjL3;   // 102
    [[gnu::packed]] uint64_t TotVarhAbsL3;   // 106
                    uint16_t ThrotPct;       // 110
    [[gnu::packed]] uint32_t ThrotSrc;       // 111
                    int16_t  A_SF;           // 113
                    int16_t  V_SF;           // 114
                    int16_t  Hz_SF;          // 115
                    int16_t  W_SF;           // 116
                    int16_t  PF_SF;          // 117
                    int16_t  VA_SF;          // 118
                    int16_t  Var_SF;         // 119
                    int16_t  TotWh_SF;       // 120
                    int16_t  TotVarh_SF;     // 121
                    int16_t  Tmp_SF;         // 122
                    char     MnAlrmInfo[64]; // 123
};

#define SUNSPEC_MODEL_701_NAME "DER AC Measurement"

// Total size in bytes, includes ID and length registers.
#define SUNSPEC_MODEL_701_SIZE (310)

// Register count without ID and length registers, should match content of length register.
#define SUNSPEC_MODEL_701_LENGTH (153)

union SunSpecDERACMeasurementModel701_u {
    struct SunSpecDERACMeasurementModel701_s model;
    uint16_t registers[SUNSPEC_MODEL_701_SIZE / sizeof(uint16_t)];
};

static_assert(sizeof(SunSpecDERACMeasurementModel701_s) == 310, "Incorrect DER AC Measurement length.");
static_assert(sizeof(SunSpecDERACMeasurementModel701_u) == 310, "Incorrect DER AC Measurement length.");
static_assert(sizeof(static_cast<SunSpecDERACMeasurementModel701_u *>(nullptr)->registers) == 310, "Incorrect DER AC Measurement length.");

#if defined(__GNUC__)
   #pragma GCC diagnostic pop
#endif
