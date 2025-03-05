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
// 802 - Battery Base Model
// ========================

struct SunSpecBatteryBaseModelModel802_s {
                    uint16_t ID;               //  0
                    uint16_t L;                //  1
                    uint16_t AHRtg;            //  2
                    uint16_t WHRtg;            //  3
                    uint16_t WChaRteMax;       //  4
                    uint16_t WDisChaRteMax;    //  5
                    uint16_t DisChaRte;        //  6
                    uint16_t SoCMax;           //  7
                    uint16_t SoCMin;           //  8
                    uint16_t SocRsvMax;        //  9
                    uint16_t SoCRsvMin;        // 10
                    uint16_t SoC;              // 11
                    uint16_t DoD;              // 12
                    uint16_t SoH;              // 13
    [[gnu::packed]] uint32_t NCyc;             // 14
                    uint16_t ChaSt;            // 16
                    uint16_t LocRemCtl;        // 17
                    uint16_t Hb;               // 18
                    uint16_t CtrlHb;           // 19
                    uint16_t AlmRst;           // 20
                    uint16_t Typ;              // 21
                    uint16_t State;            // 22
                    uint16_t StateVnd;         // 23
    [[gnu::packed]] uint32_t WarrDt;           // 24
    [[gnu::packed]] uint32_t Evt1;             // 26
    [[gnu::packed]] uint32_t Evt2;             // 28
    [[gnu::packed]] uint32_t EvtVnd1;          // 30
    [[gnu::packed]] uint32_t EvtVnd2;          // 32
                    uint16_t V;                // 34
                    uint16_t VMax;             // 35
                    uint16_t VMin;             // 36
                    uint16_t CellVMax;         // 37
                    uint16_t CellVMaxStr;      // 38
                    uint16_t CellVMaxMod;      // 39
                    uint16_t CellVMin;         // 40
                    uint16_t CellVMinStr;      // 41
                    uint16_t CellVMinMod;      // 42
                    uint16_t CellVAvg;         // 43
                    int16_t  A;                // 44
                    uint16_t AChaMax;          // 45
                    uint16_t ADisChaMax;       // 46
                    int16_t  W;                // 47
                    uint16_t ReqInvState;      // 48
                    int16_t  ReqW;             // 49
                    uint16_t SetOp;            // 50
                    uint16_t SetInvState;      // 51
                    int16_t  AHRtg_SF;         // 52
                    int16_t  WHRtg_SF;         // 53
                    int16_t  WChaDisChaMax_SF; // 54
                    int16_t  DisChaRte_SF;     // 55
                    int16_t  SoC_SF;           // 56
                    int16_t  DoD_SF;           // 57
                    int16_t  SoH_SF;           // 58
                    int16_t  V_SF;             // 59
                    int16_t  CellV_SF;         // 60
                    int16_t  A_SF;             // 61
                    int16_t  AMax_SF;          // 62
                    int16_t  W_SF;             // 63
};

#define SUNSPEC_MODEL_802_NAME "Battery Base Model"

// Total size in bytes, includes ID and length registers.
#define SUNSPEC_MODEL_802_SIZE (128)

// Register count without ID and length registers, should match content of length register.
#define SUNSPEC_MODEL_802_LENGTH (62)

union SunSpecBatteryBaseModelModel802_u {
    struct SunSpecBatteryBaseModelModel802_s model;
    uint16_t registers[SUNSPEC_MODEL_802_SIZE / sizeof(uint16_t)];
};

static_assert(sizeof(SunSpecBatteryBaseModelModel802_s) == 128, "Incorrect Battery Base Model length.");
static_assert(sizeof(SunSpecBatteryBaseModelModel802_u) == 128, "Incorrect Battery Base Model length.");
static_assert(sizeof(static_cast<SunSpecBatteryBaseModelModel802_u *>(nullptr)->registers) == 128, "Incorrect Battery Base Model length.");

#if defined(__GNUC__)
   #pragma GCC diagnostic pop
#endif
