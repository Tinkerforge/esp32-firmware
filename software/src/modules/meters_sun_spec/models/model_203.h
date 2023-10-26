// WARNING: This file is generated.

#pragma once

#include <stdint.h>
#include <stdlib.h>

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Wattributes"
#endif

// ===================================
// 203 - Wye-Connect Three Phase Meter
// ===================================

struct SunSpecWyeConnectThreePhaseMeterModel203_s {
                         uint16_t ID;              //   0
                         uint16_t L;               //   1
                         int16_t  A;               //   2
                         int16_t  AphA;            //   3
                         int16_t  AphB;            //   4
                         int16_t  AphC;            //   5
                         int16_t  A_SF;            //   6
                         int16_t  PhV;             //   7
                         int16_t  PhVphA;          //   8
                         int16_t  PhVphB;          //   9
                         int16_t  PhVphC;          //  10
                         int16_t  PPV;             //  11
                         int16_t  PhVphAB;         //  12
                         int16_t  PhVphBC;         //  13
                         int16_t  PhVphCA;         //  14
                         int16_t  V_SF;            //  15
                         int16_t  Hz;              //  16
                         int16_t  Hz_SF;           //  17
                         int16_t  W;               //  18
                         int16_t  WphA;            //  19
                         int16_t  WphB;            //  20
                         int16_t  WphC;            //  21
                         int16_t  W_SF;            //  22
                         int16_t  VA;              //  23
                         int16_t  VAphA;           //  24
                         int16_t  VAphB;           //  25
                         int16_t  VAphC;           //  26
                         int16_t  VA_SF;           //  27
                         int16_t  VAR;             //  28
                         int16_t  VARphA;          //  29
                         int16_t  VARphB;          //  30
                         int16_t  VARphC;          //  31
                         int16_t  VAR_SF;          //  32
                         int16_t  PF;              //  33
                         int16_t  PFphA;           //  34
                         int16_t  PFphB;           //  35
                         int16_t  PFphC;           //  36
                         int16_t  PF_SF;           //  37
    _ATTRIBUTE((packed)) uint32_t TotWhExp;        //  38
    _ATTRIBUTE((packed)) uint32_t TotWhExpPhA;     //  40
    _ATTRIBUTE((packed)) uint32_t TotWhExpPhB;     //  42
    _ATTRIBUTE((packed)) uint32_t TotWhExpPhC;     //  44
    _ATTRIBUTE((packed)) uint32_t TotWhImp;        //  46
    _ATTRIBUTE((packed)) uint32_t TotWhImpPhA;     //  48
    _ATTRIBUTE((packed)) uint32_t TotWhImpPhB;     //  50
    _ATTRIBUTE((packed)) uint32_t TotWhImpPhC;     //  52
                         int16_t  TotWh_SF;        //  54
    _ATTRIBUTE((packed)) uint32_t TotVAhExp;       //  55
    _ATTRIBUTE((packed)) uint32_t TotVAhExpPhA;    //  57
    _ATTRIBUTE((packed)) uint32_t TotVAhExpPhB;    //  59
    _ATTRIBUTE((packed)) uint32_t TotVAhExpPhC;    //  61
    _ATTRIBUTE((packed)) uint32_t TotVAhImp;       //  63
    _ATTRIBUTE((packed)) uint32_t TotVAhImpPhA;    //  65
    _ATTRIBUTE((packed)) uint32_t TotVAhImpPhB;    //  67
    _ATTRIBUTE((packed)) uint32_t TotVAhImpPhC;    //  69
                         int16_t  TotVAh_SF;       //  71
    _ATTRIBUTE((packed)) uint32_t TotVArhImpQ1;    //  72
    _ATTRIBUTE((packed)) uint32_t TotVArhImpQ1PhA; //  74
    _ATTRIBUTE((packed)) uint32_t TotVArhImpQ1PhB; //  76
    _ATTRIBUTE((packed)) uint32_t TotVArhImpQ1PhC; //  78
    _ATTRIBUTE((packed)) uint32_t TotVArhImpQ2;    //  80
    _ATTRIBUTE((packed)) uint32_t TotVArhImpQ2PhA; //  82
    _ATTRIBUTE((packed)) uint32_t TotVArhImpQ2PhB; //  84
    _ATTRIBUTE((packed)) uint32_t TotVArhImpQ2PhC; //  86
    _ATTRIBUTE((packed)) uint32_t TotVArhExpQ3;    //  88
    _ATTRIBUTE((packed)) uint32_t TotVArhExpQ3PhA; //  90
    _ATTRIBUTE((packed)) uint32_t TotVArhExpQ3PhB; //  92
    _ATTRIBUTE((packed)) uint32_t TotVArhExpQ3PhC; //  94
    _ATTRIBUTE((packed)) uint32_t TotVArhExpQ4;    //  96
    _ATTRIBUTE((packed)) uint32_t TotVArhExpQ4PhA; //  98
    _ATTRIBUTE((packed)) uint32_t TotVArhExpQ4PhB; // 100
    _ATTRIBUTE((packed)) uint32_t TotVArhExpQ4PhC; // 102
                         int16_t  TotVArh_SF;      // 104
    _ATTRIBUTE((packed)) uint32_t Evt;             // 105
};

#define SUNSPEC_MODEL_203_NAME "Wye-Connect Three Phase Meter"

// Total size in bytes, includes ID and length registers.
#define SUNSPEC_MODEL_203_SIZE (214)

// Register count without ID and length registers, should match content of length register.
#define SUNSPEC_MODEL_203_LENGTH (105)

union SunSpecWyeConnectThreePhaseMeterModel203_u {
    struct SunSpecWyeConnectThreePhaseMeterModel203_s model;
    uint16_t registers[SUNSPEC_MODEL_203_SIZE / sizeof(uint16_t)];
};

static_assert(sizeof(SunSpecWyeConnectThreePhaseMeterModel203_s) == 214, "Incorrect Wye-Connect Three Phase Meter length.");
static_assert(sizeof(SunSpecWyeConnectThreePhaseMeterModel203_u) == 214, "Incorrect Wye-Connect Three Phase Meter length.");
static_assert(sizeof(static_cast<SunSpecWyeConnectThreePhaseMeterModel203_u *>(nullptr)->registers) == 214, "Incorrect Wye-Connect Three Phase Meter length.");

#if defined(__GNUC__)
   #pragma GCC diagnostic pop
#endif
