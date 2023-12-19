// WARNING: This file is generated.

#pragma once

#include <stdint.h>
#include <stdlib.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Wattributes"
#endif

// =====================================
// 214 - Delta-Connect Three Phase Meter
// =====================================

struct SunSpecDeltaConnectThreePhaseMeterModel214_s {
                    uint16_t ID;              //   0
                    uint16_t L;               //   1
    [[gnu::packed]] uint32_t A;               //   2
    [[gnu::packed]] uint32_t AphA;            //   4
    [[gnu::packed]] uint32_t AphB;            //   6
    [[gnu::packed]] uint32_t AphC;            //   8
    [[gnu::packed]] uint32_t PhV;             //  10
    [[gnu::packed]] uint32_t PhVphA;          //  12
    [[gnu::packed]] uint32_t PhVphB;          //  14
    [[gnu::packed]] uint32_t PhVphC;          //  16
    [[gnu::packed]] uint32_t PPV;             //  18
    [[gnu::packed]] uint32_t PPVphAB;         //  20
    [[gnu::packed]] uint32_t PPVphBC;         //  22
    [[gnu::packed]] uint32_t PPVphCA;         //  24
    [[gnu::packed]] uint32_t Hz;              //  26
    [[gnu::packed]] uint32_t W;               //  28
    [[gnu::packed]] uint32_t WphA;            //  30
    [[gnu::packed]] uint32_t WphB;            //  32
    [[gnu::packed]] uint32_t WphC;            //  34
    [[gnu::packed]] uint32_t VA;              //  36
    [[gnu::packed]] uint32_t VAphA;           //  38
    [[gnu::packed]] uint32_t VAphB;           //  40
    [[gnu::packed]] uint32_t VAphC;           //  42
    [[gnu::packed]] uint32_t VAR;             //  44
    [[gnu::packed]] uint32_t VARphA;          //  46
    [[gnu::packed]] uint32_t VARphB;          //  48
    [[gnu::packed]] uint32_t VARphC;          //  50
    [[gnu::packed]] uint32_t PF;              //  52
    [[gnu::packed]] uint32_t PFphA;           //  54
    [[gnu::packed]] uint32_t PFphB;           //  56
    [[gnu::packed]] uint32_t PFphC;           //  58
    [[gnu::packed]] uint32_t TotWhExp;        //  60
    [[gnu::packed]] uint32_t TotWhExpPhA;     //  62
    [[gnu::packed]] uint32_t TotWhExpPhB;     //  64
    [[gnu::packed]] uint32_t TotWhExpPhC;     //  66
    [[gnu::packed]] uint32_t TotWhImp;        //  68
    [[gnu::packed]] uint32_t TotWhImpPhA;     //  70
    [[gnu::packed]] uint32_t TotWhImpPhB;     //  72
    [[gnu::packed]] uint32_t TotWhImpPhC;     //  74
    [[gnu::packed]] uint32_t TotVAhExp;       //  76
    [[gnu::packed]] uint32_t TotVAhExpPhA;    //  78
    [[gnu::packed]] uint32_t TotVAhExpPhB;    //  80
    [[gnu::packed]] uint32_t TotVAhExpPhC;    //  82
    [[gnu::packed]] uint32_t TotVAhImp;       //  84
    [[gnu::packed]] uint32_t TotVAhImpPhA;    //  86
    [[gnu::packed]] uint32_t TotVAhImpPhB;    //  88
    [[gnu::packed]] uint32_t TotVAhImpPhC;    //  90
    [[gnu::packed]] uint32_t TotVArhImpQ1;    //  92
    [[gnu::packed]] uint32_t TotVArhImpQ1phA; //  94
    [[gnu::packed]] uint32_t TotVArhImpQ1phB; //  96
    [[gnu::packed]] uint32_t TotVArhImpQ1phC; //  98
    [[gnu::packed]] uint32_t TotVArhImpQ2;    // 100
    [[gnu::packed]] uint32_t TotVArhImpQ2phA; // 102
    [[gnu::packed]] uint32_t TotVArhImpQ2phB; // 104
    [[gnu::packed]] uint32_t TotVArhImpQ2phC; // 106
    [[gnu::packed]] uint32_t TotVArhExpQ3;    // 108
    [[gnu::packed]] uint32_t TotVArhExpQ3phA; // 110
    [[gnu::packed]] uint32_t TotVArhExpQ3phB; // 112
    [[gnu::packed]] uint32_t TotVArhExpQ3phC; // 114
    [[gnu::packed]] uint32_t TotVArhExpQ4;    // 116
    [[gnu::packed]] uint32_t TotVArhExpQ4phA; // 118
    [[gnu::packed]] uint32_t TotVArhExpQ4phB; // 120
    [[gnu::packed]] uint32_t TotVArhExpQ4phC; // 122
    [[gnu::packed]] uint32_t Evt;             // 124
};

#define SUNSPEC_MODEL_214_NAME "Delta-Connect Three Phase Meter"

// Total size in bytes, includes ID and length registers.
#define SUNSPEC_MODEL_214_SIZE (252)

// Register count without ID and length registers, should match content of length register.
#define SUNSPEC_MODEL_214_LENGTH (124)

union SunSpecDeltaConnectThreePhaseMeterModel214_u {
    struct SunSpecDeltaConnectThreePhaseMeterModel214_s model;
    uint16_t registers[SUNSPEC_MODEL_214_SIZE / sizeof(uint16_t)];
};

static_assert(sizeof(SunSpecDeltaConnectThreePhaseMeterModel214_s) == 252, "Incorrect Delta-Connect Three Phase Meter length.");
static_assert(sizeof(SunSpecDeltaConnectThreePhaseMeterModel214_u) == 252, "Incorrect Delta-Connect Three Phase Meter length.");
static_assert(sizeof(static_cast<SunSpecDeltaConnectThreePhaseMeterModel214_u *>(nullptr)->registers) == 252, "Incorrect Delta-Connect Three Phase Meter length.");

#if defined(__GNUC__)
   #pragma GCC diagnostic pop
#endif
