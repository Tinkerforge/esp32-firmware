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
// 714 - DER DC Measurement
// ========================

struct SunSpecDERDCMeasurementModel714_s {
                    uint16_t ID;       //  0
                    uint16_t L;        //  1
    [[gnu::packed]] uint32_t PrtAlrms; //  2
                    uint16_t NPrt;     //  4
                    int16_t  DCA;      //  5
                    int16_t  DCW;      //  6
    [[gnu::packed]] uint64_t DCWhInj;  //  7
    [[gnu::packed]] uint64_t DCWhAbs;  // 11
                    int16_t  DCA_SF;   // 15
                    int16_t  DCV_SF;   // 16
                    int16_t  DCW_SF;   // 17
                    int16_t  DCWH_SF;  // 18
                    int16_t  Tmp_SF;   // 19
};

#define SUNSPEC_MODEL_714_NAME "DER DC Measurement"

// Total size in bytes, includes ID and length registers.
#define SUNSPEC_MODEL_714_SIZE (40)

// Register count without ID and length registers, should match content of length register.
#define SUNSPEC_MODEL_714_LENGTH (18)

union SunSpecDERDCMeasurementModel714_u {
    struct SunSpecDERDCMeasurementModel714_s model;
    uint16_t registers[SUNSPEC_MODEL_714_SIZE / sizeof(uint16_t)];
};

static_assert(sizeof(SunSpecDERDCMeasurementModel714_s) == 40, "Incorrect DER DC Measurement length.");
static_assert(sizeof(SunSpecDERDCMeasurementModel714_u) == 40, "Incorrect DER DC Measurement length.");
static_assert(sizeof(static_cast<SunSpecDERDCMeasurementModel714_u *>(nullptr)->registers) == 40, "Incorrect DER DC Measurement length.");

#if defined(__GNUC__)
   #pragma GCC diagnostic pop
#endif
