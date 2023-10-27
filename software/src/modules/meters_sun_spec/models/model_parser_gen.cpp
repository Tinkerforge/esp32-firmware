// WARNING: This file is generated.

#include <math.h>
#include <stdint.h>

#include "../model_parser.h"

#include "gcc_warnings.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic ignored "-Wfloat-equal"
    #pragma GCC diagnostic ignored "-Wpedantic"
#endif

static const float scale_factors[21] = {
              0.0000000001f,    // 10^-10
              0.000000001f,     // 10^-9
              0.00000001f,      // 10^-8
              0.0000001f,       // 10^-7
              0.000001f,        // 10^-6
              0.00001f,         // 10^-5
              0.0001f,          // 10^-4
              0.001f,           // 10^-3
              0.01f,            // 10^-2
              0.1f,             // 10^-1
              1.0f,             // 10^0
             10.0f,             // 10^1
            100.0f,             // 10^2
           1000.0f,             // 10^3
          10000.0f,             // 10^4
         100000.0f,             // 10^5
        1000000.0f,             // 10^6
       10000000.0f,             // 10^7
      100000000.0f,             // 10^8
     1000000000.0f,             // 10^9
    10000000000.0f,             // 10^10
};

static float get_scale_factor(int32_t sunssf)
{
    if (sunssf < -10) {
        if (sunssf == INT16_MIN) { // scale factor not implemented
            return 1;
        } else {
            return NAN;
        }
    } else if (sunssf > 10) {
        return NAN;
    }
    return scale_factors[sunssf + 10];

}

static inline uint32_t convert_me_uint32(uint32_t me32)
{
    return me32 << 16 | me32 >> 16;
}

static inline float convert_me_float(uint32_t me32)
{
    union {
        float result;
        uint32_t u32;
    } uni;

    uni.u32 = convert_me_uint32(me32);
    return uni.result;
}

// ============
// 001 - Common
// ============

#include "model_001.h"

static bool model_001_validator(const uint16_t * const register_data[2])
{
    const SunSpecCommonModel001_s *block0 = reinterpret_cast<const SunSpecCommonModel001_s *>(register_data[0]);
    const SunSpecCommonModel001_s *block1 = reinterpret_cast<const SunSpecCommonModel001_s *>(register_data[1]);
    if (block0->ID !=   1) return false;
    if (block1->ID !=   1) return false;
    if (block0->L  !=  65) return false;
    if (block1->L  !=  65) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_001_data = {
    1, // model_id
    &model_001_validator,
    0,  // value_count
    {    // value_data
    }
};

// ==============
// 101 - Inverter
// ==============

#include "model_101.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_101_A(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->A;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_A(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->A);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_AphA(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->AphA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_AphA(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->AphA);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_AphB(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->AphB;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_AphB(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->AphB);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_AphC(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->AphC;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_AphC(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->AphC);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PPVphAB;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->PPVphAB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PPVphBC;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->PPVphBC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PPVphCA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->PPVphCA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_PhVphA(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PhVphA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_PhVphA(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->PhVphA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_PhVphB(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PhVphB;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_PhVphB(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->PhVphB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_PhVphC(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PhVphC;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_PhVphC(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->PhVphC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_W(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_W(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->W);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_Hz(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->Hz;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_Hz(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->Hz);
    val *= get_scale_factor(model->Hz_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_VA(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_VA(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->VA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_VAr(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->VAr;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_VAr(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->VAr);
    val *= get_scale_factor(model->VAr_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_PF(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_PF(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->PF);
    val *= (get_scale_factor(model->PF_SF) * 0.01f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_WH(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint32_t val = convert_me_uint32(model->WH);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_WH(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->WH));
    val *= (get_scale_factor(model->WH_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_DCA(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->DCA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_DCA(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->DCA);
    val *= get_scale_factor(model->DCA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_DCV(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->DCV;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_DCV(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->DCV);
    val *= get_scale_factor(model->DCV_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_DCW(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->DCW;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_DCW(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->DCW);
    val *= get_scale_factor(model->DCW_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_TmpCab(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->TmpCab;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_TmpCab(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->TmpCab);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->TmpSnk;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->TmpSnk);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->TmpTrns;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->TmpTrns);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_101_TmpOt(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->TmpOt;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_101_TmpOt(const void *register_data)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    float val = static_cast<float>(model->TmpOt);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static bool model_101_validator(const uint16_t * const register_data[2])
{
    const SunSpecInverterModel101_s *block0 = reinterpret_cast<const SunSpecInverterModel101_s *>(register_data[0]);
    const SunSpecInverterModel101_s *block1 = reinterpret_cast<const SunSpecInverterModel101_s *>(register_data[1]);
    if (block0->ID != 101) return false;
    if (block1->ID != 101) return false;
    if (block0->L  !=  50) return false;
    if (block1->L  !=  50) return false;
    if (block0->A_SF != block1->A_SF) return false;
    if (block0->V_SF != block1->V_SF) return false;
    if (block0->W_SF != block1->W_SF) return false;
    if (block0->Hz_SF != block1->Hz_SF) return false;
    if (block0->VA_SF != block1->VA_SF) return false;
    if (block0->VAr_SF != block1->VAr_SF) return false;
    if (block0->PF_SF != block1->PF_SF) return false;
    if (block0->WH_SF != block1->WH_SF) return false;
    if (block0->DCA_SF != block1->DCA_SF) return false;
    if (block0->DCV_SF != block1->DCV_SF) return false;
    if (block0->DCW_SF != block1->DCW_SF) return false;
    if (block0->Tmp_SF != block1->Tmp_SF) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_101_data = {
    101, // model_id
    &model_101_validator,
    23,  // value_count
    {    // value_data
        { &get_model_101_A, &detect_model_101_A, MeterValueID::CurrentLSumExport },
        { &get_model_101_AphA, &detect_model_101_AphA, MeterValueID::CurrentL1Export },
        { &get_model_101_AphB, &detect_model_101_AphB, MeterValueID::CurrentL2Export },
        { &get_model_101_AphC, &detect_model_101_AphC, MeterValueID::CurrentL3Export },
        { &get_model_101_PPVphAB, &detect_model_101_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_101_PPVphBC, &detect_model_101_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_101_PPVphCA, &detect_model_101_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_101_PhVphA, &detect_model_101_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_101_PhVphB, &detect_model_101_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_101_PhVphC, &detect_model_101_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_101_W, &detect_model_101_W, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_101_Hz, &detect_model_101_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_101_VA, &detect_model_101_VA, MeterValueID::PowerApparentLSum },
        { &get_model_101_VAr, &detect_model_101_VAr, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_101_PF, &detect_model_101_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_101_WH, &detect_model_101_WH, MeterValueID::EnergyActiveLSumExport },
        { &get_model_101_DCA, &detect_model_101_DCA, MeterValueID::CurrentDC },
        { &get_model_101_DCV, &detect_model_101_DCV, MeterValueID::VoltageDC },
        { &get_model_101_DCW, &detect_model_101_DCW, MeterValueID::PowerDC },
        { &get_model_101_TmpCab, &detect_model_101_TmpCab, MeterValueID::Temperature1 },
        { &get_model_101_TmpSnk, &detect_model_101_TmpSnk, MeterValueID::Temperature2 },
        { &get_model_101_TmpTrns, &detect_model_101_TmpTrns, MeterValueID::Temperature3 },
        { &get_model_101_TmpOt, &detect_model_101_TmpOt, MeterValueID::Temperature4 },
    }
};

// ==============
// 102 - Inverter
// ==============

#include "model_102.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_102_A(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->A;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_A(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->A);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_AphA(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->AphA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_AphA(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->AphA);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_AphB(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->AphB;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_AphB(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->AphB);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_AphC(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->AphC;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_AphC(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->AphC);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PPVphAB;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->PPVphAB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PPVphBC;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->PPVphBC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PPVphCA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->PPVphCA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_PhVphA(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PhVphA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_PhVphA(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->PhVphA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_PhVphB(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PhVphB;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_PhVphB(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->PhVphB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_PhVphC(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PhVphC;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_PhVphC(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->PhVphC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_W(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_W(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->W);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_Hz(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->Hz;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_Hz(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->Hz);
    val *= get_scale_factor(model->Hz_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_VA(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_VA(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->VA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_VAr(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->VAr;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_VAr(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->VAr);
    val *= get_scale_factor(model->VAr_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_PF(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_PF(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->PF);
    val *= (get_scale_factor(model->PF_SF) * 0.01f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_WH(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint32_t val = convert_me_uint32(model->WH);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_WH(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->WH));
    val *= (get_scale_factor(model->WH_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_DCA(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->DCA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_DCA(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->DCA);
    val *= get_scale_factor(model->DCA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_DCV(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->DCV;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_DCV(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->DCV);
    val *= get_scale_factor(model->DCV_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_DCW(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->DCW;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_DCW(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->DCW);
    val *= get_scale_factor(model->DCW_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_TmpCab(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->TmpCab;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_TmpCab(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->TmpCab);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->TmpSnk;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->TmpSnk);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->TmpTrns;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->TmpTrns);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_102_TmpOt(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->TmpOt;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_102_TmpOt(const void *register_data)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    float val = static_cast<float>(model->TmpOt);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static bool model_102_validator(const uint16_t * const register_data[2])
{
    const SunSpecInverterModel102_s *block0 = reinterpret_cast<const SunSpecInverterModel102_s *>(register_data[0]);
    const SunSpecInverterModel102_s *block1 = reinterpret_cast<const SunSpecInverterModel102_s *>(register_data[1]);
    if (block0->ID != 102) return false;
    if (block1->ID != 102) return false;
    if (block0->L  !=  50) return false;
    if (block1->L  !=  50) return false;
    if (block0->A_SF != block1->A_SF) return false;
    if (block0->V_SF != block1->V_SF) return false;
    if (block0->W_SF != block1->W_SF) return false;
    if (block0->Hz_SF != block1->Hz_SF) return false;
    if (block0->VA_SF != block1->VA_SF) return false;
    if (block0->VAr_SF != block1->VAr_SF) return false;
    if (block0->PF_SF != block1->PF_SF) return false;
    if (block0->WH_SF != block1->WH_SF) return false;
    if (block0->DCA_SF != block1->DCA_SF) return false;
    if (block0->DCV_SF != block1->DCV_SF) return false;
    if (block0->DCW_SF != block1->DCW_SF) return false;
    if (block0->Tmp_SF != block1->Tmp_SF) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_102_data = {
    102, // model_id
    &model_102_validator,
    23,  // value_count
    {    // value_data
        { &get_model_102_A, &detect_model_102_A, MeterValueID::CurrentLSumExport },
        { &get_model_102_AphA, &detect_model_102_AphA, MeterValueID::CurrentL1Export },
        { &get_model_102_AphB, &detect_model_102_AphB, MeterValueID::CurrentL2Export },
        { &get_model_102_AphC, &detect_model_102_AphC, MeterValueID::CurrentL3Export },
        { &get_model_102_PPVphAB, &detect_model_102_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_102_PPVphBC, &detect_model_102_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_102_PPVphCA, &detect_model_102_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_102_PhVphA, &detect_model_102_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_102_PhVphB, &detect_model_102_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_102_PhVphC, &detect_model_102_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_102_W, &detect_model_102_W, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_102_Hz, &detect_model_102_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_102_VA, &detect_model_102_VA, MeterValueID::PowerApparentLSum },
        { &get_model_102_VAr, &detect_model_102_VAr, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_102_PF, &detect_model_102_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_102_WH, &detect_model_102_WH, MeterValueID::EnergyActiveLSumExport },
        { &get_model_102_DCA, &detect_model_102_DCA, MeterValueID::CurrentDC },
        { &get_model_102_DCV, &detect_model_102_DCV, MeterValueID::VoltageDC },
        { &get_model_102_DCW, &detect_model_102_DCW, MeterValueID::PowerDC },
        { &get_model_102_TmpCab, &detect_model_102_TmpCab, MeterValueID::Temperature1 },
        { &get_model_102_TmpSnk, &detect_model_102_TmpSnk, MeterValueID::Temperature2 },
        { &get_model_102_TmpTrns, &detect_model_102_TmpTrns, MeterValueID::Temperature3 },
        { &get_model_102_TmpOt, &detect_model_102_TmpOt, MeterValueID::Temperature4 },
    }
};

// ==============
// 103 - Inverter
// ==============

#include "model_103.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_103_A(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->A;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_A(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->A);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_AphA(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->AphA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_AphA(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->AphA);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_AphB(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->AphB;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_AphB(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->AphB);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_AphC(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->AphC;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_AphC(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->AphC);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PPVphAB;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->PPVphAB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PPVphBC;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->PPVphBC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PPVphCA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->PPVphCA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_PhVphA(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PhVphA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_PhVphA(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->PhVphA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_PhVphB(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PhVphB;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_PhVphB(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->PhVphB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_PhVphC(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PhVphC;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_PhVphC(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->PhVphC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_W(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_W(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->W);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_Hz(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->Hz;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_Hz(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->Hz);
    val *= get_scale_factor(model->Hz_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_VA(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_VA(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->VA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_VAr(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->VAr;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_VAr(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->VAr);
    val *= get_scale_factor(model->VAr_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_PF(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_PF(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->PF);
    val *= (get_scale_factor(model->PF_SF) * 0.01f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_WH(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint32_t val = convert_me_uint32(model->WH);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_WH(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->WH));
    val *= (get_scale_factor(model->WH_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_DCA(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->DCA;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_DCA(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->DCA);
    val *= get_scale_factor(model->DCA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_DCV(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->DCV;
    if (val == UINT16_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_DCV(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->DCV);
    val *= get_scale_factor(model->DCV_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_DCW(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->DCW;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_DCW(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->DCW);
    val *= get_scale_factor(model->DCW_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_TmpCab(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->TmpCab;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_TmpCab(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->TmpCab);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->TmpSnk;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->TmpSnk);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->TmpTrns;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->TmpTrns);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_103_TmpOt(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->TmpOt;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_103_TmpOt(const void *register_data)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    float val = static_cast<float>(model->TmpOt);
    val *= get_scale_factor(model->Tmp_SF);
    return val;
}

static bool model_103_validator(const uint16_t * const register_data[2])
{
    const SunSpecInverterModel103_s *block0 = reinterpret_cast<const SunSpecInverterModel103_s *>(register_data[0]);
    const SunSpecInverterModel103_s *block1 = reinterpret_cast<const SunSpecInverterModel103_s *>(register_data[1]);
    if (block0->ID != 103) return false;
    if (block1->ID != 103) return false;
    if (block0->L  !=  50) return false;
    if (block1->L  !=  50) return false;
    if (block0->A_SF != block1->A_SF) return false;
    if (block0->V_SF != block1->V_SF) return false;
    if (block0->W_SF != block1->W_SF) return false;
    if (block0->Hz_SF != block1->Hz_SF) return false;
    if (block0->VA_SF != block1->VA_SF) return false;
    if (block0->VAr_SF != block1->VAr_SF) return false;
    if (block0->PF_SF != block1->PF_SF) return false;
    if (block0->WH_SF != block1->WH_SF) return false;
    if (block0->DCA_SF != block1->DCA_SF) return false;
    if (block0->DCV_SF != block1->DCV_SF) return false;
    if (block0->DCW_SF != block1->DCW_SF) return false;
    if (block0->Tmp_SF != block1->Tmp_SF) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_103_data = {
    103, // model_id
    &model_103_validator,
    23,  // value_count
    {    // value_data
        { &get_model_103_A, &detect_model_103_A, MeterValueID::CurrentLSumExport },
        { &get_model_103_AphA, &detect_model_103_AphA, MeterValueID::CurrentL1Export },
        { &get_model_103_AphB, &detect_model_103_AphB, MeterValueID::CurrentL2Export },
        { &get_model_103_AphC, &detect_model_103_AphC, MeterValueID::CurrentL3Export },
        { &get_model_103_PPVphAB, &detect_model_103_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_103_PPVphBC, &detect_model_103_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_103_PPVphCA, &detect_model_103_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_103_PhVphA, &detect_model_103_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_103_PhVphB, &detect_model_103_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_103_PhVphC, &detect_model_103_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_103_W, &detect_model_103_W, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_103_Hz, &detect_model_103_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_103_VA, &detect_model_103_VA, MeterValueID::PowerApparentLSum },
        { &get_model_103_VAr, &detect_model_103_VAr, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_103_PF, &detect_model_103_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_103_WH, &detect_model_103_WH, MeterValueID::EnergyActiveLSumExport },
        { &get_model_103_DCA, &detect_model_103_DCA, MeterValueID::CurrentDC },
        { &get_model_103_DCV, &detect_model_103_DCV, MeterValueID::VoltageDC },
        { &get_model_103_DCW, &detect_model_103_DCW, MeterValueID::PowerDC },
        { &get_model_103_TmpCab, &detect_model_103_TmpCab, MeterValueID::Temperature1 },
        { &get_model_103_TmpSnk, &detect_model_103_TmpSnk, MeterValueID::Temperature2 },
        { &get_model_103_TmpTrns, &detect_model_103_TmpTrns, MeterValueID::Temperature3 },
        { &get_model_103_TmpOt, &detect_model_103_TmpOt, MeterValueID::Temperature4 },
    }
};

// ====================
// 111 - Inverter FLOAT
// ====================

#include "model_111.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_111_A(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->A);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_A(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->A);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_AphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->AphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_AphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->AphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_AphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->AphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_AphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->AphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_AphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->AphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_AphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->AphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_PhVphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_PhVphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_PhVphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_PhVphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_PhVphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_PhVphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_W(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->W);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_W(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->W);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_Hz(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->Hz);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_Hz(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->Hz);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_VA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->VA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_VA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->VA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_VAr(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->VAr);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_VAr(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->VAr);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_PF(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PF);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_PF(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->PF);
    val *= 0.01f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_WH(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->WH);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_WH(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->WH);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_DCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->DCA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_DCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->DCA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_DCV(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->DCV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_DCV(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->DCV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_DCW(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->DCW);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_DCW(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->DCW);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_TmpCab(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->TmpCab);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_TmpCab(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->TmpCab);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->TmpSnk);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->TmpSnk);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->TmpTrns);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->TmpTrns);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_111_TmpOt(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->TmpOt);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_111_TmpOt(const void *register_data)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(model->TmpOt);
    return val;
}

static bool model_111_validator(const uint16_t * const register_data[2])
{
    const SunSpecInverterFLOATModel111_s *block0 = reinterpret_cast<const SunSpecInverterFLOATModel111_s *>(register_data[0]);
    const SunSpecInverterFLOATModel111_s *block1 = reinterpret_cast<const SunSpecInverterFLOATModel111_s *>(register_data[1]);
    if (block0->ID != 111) return false;
    if (block1->ID != 111) return false;
    if (block0->L  !=  60) return false;
    if (block1->L  !=  60) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_111_data = {
    111, // model_id
    &model_111_validator,
    23,  // value_count
    {    // value_data
        { &get_model_111_A, &detect_model_111_A, MeterValueID::CurrentLSumExport },
        { &get_model_111_AphA, &detect_model_111_AphA, MeterValueID::CurrentL1Export },
        { &get_model_111_AphB, &detect_model_111_AphB, MeterValueID::CurrentL2Export },
        { &get_model_111_AphC, &detect_model_111_AphC, MeterValueID::CurrentL3Export },
        { &get_model_111_PPVphAB, &detect_model_111_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_111_PPVphBC, &detect_model_111_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_111_PPVphCA, &detect_model_111_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_111_PhVphA, &detect_model_111_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_111_PhVphB, &detect_model_111_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_111_PhVphC, &detect_model_111_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_111_W, &detect_model_111_W, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_111_Hz, &detect_model_111_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_111_VA, &detect_model_111_VA, MeterValueID::PowerApparentLSum },
        { &get_model_111_VAr, &detect_model_111_VAr, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_111_PF, &detect_model_111_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_111_WH, &detect_model_111_WH, MeterValueID::EnergyActiveLSumExport },
        { &get_model_111_DCA, &detect_model_111_DCA, MeterValueID::CurrentDC },
        { &get_model_111_DCV, &detect_model_111_DCV, MeterValueID::VoltageDC },
        { &get_model_111_DCW, &detect_model_111_DCW, MeterValueID::PowerDC },
        { &get_model_111_TmpCab, &detect_model_111_TmpCab, MeterValueID::Temperature1 },
        { &get_model_111_TmpSnk, &detect_model_111_TmpSnk, MeterValueID::Temperature2 },
        { &get_model_111_TmpTrns, &detect_model_111_TmpTrns, MeterValueID::Temperature3 },
        { &get_model_111_TmpOt, &detect_model_111_TmpOt, MeterValueID::Temperature4 },
    }
};

// ====================
// 112 - Inverter FLOAT
// ====================

#include "model_112.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_112_A(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->A);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_A(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->A);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_AphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->AphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_AphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->AphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_AphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->AphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_AphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->AphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_AphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->AphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_AphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->AphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_PhVphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_PhVphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_PhVphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_PhVphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_PhVphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_PhVphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_W(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->W);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_W(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->W);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_Hz(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->Hz);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_Hz(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->Hz);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_VA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->VA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_VA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->VA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_VAr(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->VAr);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_VAr(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->VAr);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_PF(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PF);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_PF(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->PF);
    val *= 0.01f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_WH(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->WH);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_WH(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->WH);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_DCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->DCA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_DCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->DCA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_DCV(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->DCV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_DCV(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->DCV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_DCW(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->DCW);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_DCW(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->DCW);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_TmpCab(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->TmpCab);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_TmpCab(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->TmpCab);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->TmpSnk);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->TmpSnk);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->TmpTrns);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->TmpTrns);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_112_TmpOt(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->TmpOt);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_112_TmpOt(const void *register_data)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(model->TmpOt);
    return val;
}

static bool model_112_validator(const uint16_t * const register_data[2])
{
    const SunSpecInverterFLOATModel112_s *block0 = reinterpret_cast<const SunSpecInverterFLOATModel112_s *>(register_data[0]);
    const SunSpecInverterFLOATModel112_s *block1 = reinterpret_cast<const SunSpecInverterFLOATModel112_s *>(register_data[1]);
    if (block0->ID != 112) return false;
    if (block1->ID != 112) return false;
    if (block0->L  !=  60) return false;
    if (block1->L  !=  60) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_112_data = {
    112, // model_id
    &model_112_validator,
    23,  // value_count
    {    // value_data
        { &get_model_112_A, &detect_model_112_A, MeterValueID::CurrentLSumExport },
        { &get_model_112_AphA, &detect_model_112_AphA, MeterValueID::CurrentL1Export },
        { &get_model_112_AphB, &detect_model_112_AphB, MeterValueID::CurrentL2Export },
        { &get_model_112_AphC, &detect_model_112_AphC, MeterValueID::CurrentL3Export },
        { &get_model_112_PPVphAB, &detect_model_112_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_112_PPVphBC, &detect_model_112_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_112_PPVphCA, &detect_model_112_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_112_PhVphA, &detect_model_112_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_112_PhVphB, &detect_model_112_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_112_PhVphC, &detect_model_112_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_112_W, &detect_model_112_W, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_112_Hz, &detect_model_112_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_112_VA, &detect_model_112_VA, MeterValueID::PowerApparentLSum },
        { &get_model_112_VAr, &detect_model_112_VAr, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_112_PF, &detect_model_112_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_112_WH, &detect_model_112_WH, MeterValueID::EnergyActiveLSumExport },
        { &get_model_112_DCA, &detect_model_112_DCA, MeterValueID::CurrentDC },
        { &get_model_112_DCV, &detect_model_112_DCV, MeterValueID::VoltageDC },
        { &get_model_112_DCW, &detect_model_112_DCW, MeterValueID::PowerDC },
        { &get_model_112_TmpCab, &detect_model_112_TmpCab, MeterValueID::Temperature1 },
        { &get_model_112_TmpSnk, &detect_model_112_TmpSnk, MeterValueID::Temperature2 },
        { &get_model_112_TmpTrns, &detect_model_112_TmpTrns, MeterValueID::Temperature3 },
        { &get_model_112_TmpOt, &detect_model_112_TmpOt, MeterValueID::Temperature4 },
    }
};

// ====================
// 113 - Inverter FLOAT
// ====================

#include "model_113.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_113_A(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->A);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_A(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->A);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_AphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->AphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_AphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->AphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_AphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->AphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_AphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->AphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_AphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->AphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_AphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->AphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_PPVphAB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_PPVphBC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_PPVphCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_PhVphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_PhVphA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_PhVphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_PhVphB(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_PhVphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_PhVphC(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_W(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->W);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_W(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->W);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_Hz(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->Hz);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_Hz(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->Hz);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_VA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->VA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_VA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->VA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_VAr(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->VAr);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_VAr(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->VAr);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_PF(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PF);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_PF(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->PF);
    val *= 0.01f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_WH(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->WH);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_WH(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->WH);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_DCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->DCA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_DCA(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->DCA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_DCV(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->DCV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_DCV(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->DCV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_DCW(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->DCW);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_DCW(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->DCW);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_TmpCab(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->TmpCab);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_TmpCab(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->TmpCab);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->TmpSnk);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_TmpSnk(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->TmpSnk);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->TmpTrns);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_TmpTrns(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->TmpTrns);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_113_TmpOt(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->TmpOt);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_113_TmpOt(const void *register_data)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(model->TmpOt);
    return val;
}

static bool model_113_validator(const uint16_t * const register_data[2])
{
    const SunSpecInverterFLOATModel113_s *block0 = reinterpret_cast<const SunSpecInverterFLOATModel113_s *>(register_data[0]);
    const SunSpecInverterFLOATModel113_s *block1 = reinterpret_cast<const SunSpecInverterFLOATModel113_s *>(register_data[1]);
    if (block0->ID != 113) return false;
    if (block1->ID != 113) return false;
    if (block0->L  !=  60) return false;
    if (block1->L  !=  60) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_113_data = {
    113, // model_id
    &model_113_validator,
    23,  // value_count
    {    // value_data
        { &get_model_113_A, &detect_model_113_A, MeterValueID::CurrentLSumExport },
        { &get_model_113_AphA, &detect_model_113_AphA, MeterValueID::CurrentL1Export },
        { &get_model_113_AphB, &detect_model_113_AphB, MeterValueID::CurrentL2Export },
        { &get_model_113_AphC, &detect_model_113_AphC, MeterValueID::CurrentL3Export },
        { &get_model_113_PPVphAB, &detect_model_113_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_113_PPVphBC, &detect_model_113_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_113_PPVphCA, &detect_model_113_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_113_PhVphA, &detect_model_113_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_113_PhVphB, &detect_model_113_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_113_PhVphC, &detect_model_113_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_113_W, &detect_model_113_W, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_113_Hz, &detect_model_113_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_113_VA, &detect_model_113_VA, MeterValueID::PowerApparentLSum },
        { &get_model_113_VAr, &detect_model_113_VAr, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_113_PF, &detect_model_113_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_113_WH, &detect_model_113_WH, MeterValueID::EnergyActiveLSumExport },
        { &get_model_113_DCA, &detect_model_113_DCA, MeterValueID::CurrentDC },
        { &get_model_113_DCV, &detect_model_113_DCV, MeterValueID::VoltageDC },
        { &get_model_113_DCW, &detect_model_113_DCW, MeterValueID::PowerDC },
        { &get_model_113_TmpCab, &detect_model_113_TmpCab, MeterValueID::Temperature1 },
        { &get_model_113_TmpSnk, &detect_model_113_TmpSnk, MeterValueID::Temperature2 },
        { &get_model_113_TmpTrns, &detect_model_113_TmpTrns, MeterValueID::Temperature3 },
        { &get_model_113_TmpOt, &detect_model_113_TmpOt, MeterValueID::Temperature4 },
    }
};

// ========================
// 201 - Single Phase Meter
// ========================

#include "model_201.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_201_A(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->A;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_A(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->A);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_AphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->AphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_AphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->AphA);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_AphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->AphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_AphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->AphB);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_AphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->AphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_AphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->AphC);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PhV(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PhV;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PhV(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PhV);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PhVphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PhVphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PhVphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PhVphA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PhVphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PhVphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PhVphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PhVphB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PhVphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PhVphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PhVphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PhVphC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PPV(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PPV;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PPV(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PPV);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PPVphAB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PPVphAB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PPVphAB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PPVphAB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PPVphBC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PPVphBC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PPVphBC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PPVphBC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PPVphCA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PPVphCA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PPVphCA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PPVphCA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_Hz(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->Hz;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_Hz(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->Hz);
    val *= get_scale_factor(model->Hz_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_W(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_W(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->W);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_WphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->WphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_WphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->WphA);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_WphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->WphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_WphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->WphB);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_WphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->WphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_WphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->WphC);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_VA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_VA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->VA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_VAphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VAphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_VAphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->VAphA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_VAphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VAphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_VAphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->VAphB);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_VAphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VAphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_VAphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->VAphC);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_VAR(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VAR;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_VAR(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->VAR);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_VARphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VARphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_VARphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->VARphA);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_VARphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VARphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_VARphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->VARphB);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_VARphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VARphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_VARphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->VARphC);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PF(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PF(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PF);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PFphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PFphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PFphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PFphA);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PFphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PFphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PFphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PFphB);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_PFphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PFphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_PFphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(model->PFphC);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotWhExp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotWhExp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExp));
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotWhImp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotWhImp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImp));
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVAhExp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVAhExp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExp));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVAhImp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVAhImp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImp));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhImpQ1PhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhImpQ1PhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhImpQ1PhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhImpQ1PhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhImpQ1PhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhImpQ1PhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhImpQ2PhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhImpQ2PhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhImpQ2PhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhImpQ2PhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhImpQ2PhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhImpQ2PhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhExpQ3PhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhExpQ3PhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhExpQ3PhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhExpQ3PhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhExpQ3PhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhExpQ3PhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhExpQ4PhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhExpQ4PhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhExpQ4PhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhExpQ4PhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_201_TotVArhExpQ4PhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_201_TotVArhExpQ4PhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static bool model_201_validator(const uint16_t * const register_data[2])
{
    const SunSpecSinglePhaseMeterModel201_s *block0 = reinterpret_cast<const SunSpecSinglePhaseMeterModel201_s *>(register_data[0]);
    const SunSpecSinglePhaseMeterModel201_s *block1 = reinterpret_cast<const SunSpecSinglePhaseMeterModel201_s *>(register_data[1]);
    if (block0->ID != 201) return false;
    if (block1->ID != 201) return false;
    if (block0->L  != 105) return false;
    if (block1->L  != 105) return false;
    if (block0->A_SF != block1->A_SF) return false;
    if (block0->V_SF != block1->V_SF) return false;
    if (block0->Hz_SF != block1->Hz_SF) return false;
    if (block0->W_SF != block1->W_SF) return false;
    if (block0->VA_SF != block1->VA_SF) return false;
    if (block0->VAR_SF != block1->VAR_SF) return false;
    if (block0->PF_SF != block1->PF_SF) return false;
    if (block0->TotWh_SF != block1->TotWh_SF) return false;
    if (block0->TotVAh_SF != block1->TotVAh_SF) return false;
    if (block0->TotVArh_SF != block1->TotVArh_SF) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_201_data = {
    201, // model_id
    &model_201_validator,
    61,  // value_count
    {    // value_data
        { &get_model_201_A, &detect_model_201_A, MeterValueID::CurrentLSumImExDiff },
        { &get_model_201_AphA, &detect_model_201_AphA, MeterValueID::CurrentL1ImExDiff },
        { &get_model_201_AphB, &detect_model_201_AphB, MeterValueID::CurrentL2ImExDiff },
        { &get_model_201_AphC, &detect_model_201_AphC, MeterValueID::CurrentL3ImExDiff },
        { &get_model_201_PhV, &detect_model_201_PhV, MeterValueID::VoltageLNAvg },
        { &get_model_201_PhVphA, &detect_model_201_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_201_PhVphB, &detect_model_201_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_201_PhVphC, &detect_model_201_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_201_PPV, &detect_model_201_PPV, MeterValueID::VoltageLLAvg },
        { &get_model_201_PPVphAB, &detect_model_201_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_201_PPVphBC, &detect_model_201_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_201_PPVphCA, &detect_model_201_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_201_Hz, &detect_model_201_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_201_W, &detect_model_201_W, MeterValueID::PowerActiveLSumImExDiff },
        { &get_model_201_WphA, &detect_model_201_WphA, MeterValueID::PowerActiveL1ImExDiff },
        { &get_model_201_WphB, &detect_model_201_WphB, MeterValueID::PowerActiveL2ImExDiff },
        { &get_model_201_WphC, &detect_model_201_WphC, MeterValueID::PowerActiveL3ImExDiff },
        { &get_model_201_VA, &detect_model_201_VA, MeterValueID::PowerApparentLSum },
        { &get_model_201_VAphA, &detect_model_201_VAphA, MeterValueID::PowerApparentL1 },
        { &get_model_201_VAphB, &detect_model_201_VAphB, MeterValueID::PowerApparentL2 },
        { &get_model_201_VAphC, &detect_model_201_VAphC, MeterValueID::PowerApparentL3 },
        { &get_model_201_VAR, &detect_model_201_VAR, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_201_VARphA, &detect_model_201_VARphA, MeterValueID::PowerReactiveL1IndCapDiff },
        { &get_model_201_VARphB, &detect_model_201_VARphB, MeterValueID::PowerReactiveL2IndCapDiff },
        { &get_model_201_VARphC, &detect_model_201_VARphC, MeterValueID::PowerReactiveL3IndCapDiff },
        { &get_model_201_PF, &detect_model_201_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_201_PFphA, &detect_model_201_PFphA, MeterValueID::PowerFactorL1Directional },
        { &get_model_201_PFphB, &detect_model_201_PFphB, MeterValueID::PowerFactorL2Directional },
        { &get_model_201_PFphC, &detect_model_201_PFphC, MeterValueID::PowerFactorL3Directional },
        { &get_model_201_TotWhExp, &detect_model_201_TotWhExp, MeterValueID::EnergyActiveLSumExport },
        { &get_model_201_TotWhExpPhA, &detect_model_201_TotWhExpPhA, MeterValueID::EnergyActiveL1Export },
        { &get_model_201_TotWhExpPhB, &detect_model_201_TotWhExpPhB, MeterValueID::EnergyActiveL2Export },
        { &get_model_201_TotWhExpPhC, &detect_model_201_TotWhExpPhC, MeterValueID::EnergyActiveL3Export },
        { &get_model_201_TotWhImp, &detect_model_201_TotWhImp, MeterValueID::EnergyActiveLSumImport },
        { &get_model_201_TotWhImpPhA, &detect_model_201_TotWhImpPhA, MeterValueID::EnergyActiveL1Import },
        { &get_model_201_TotWhImpPhB, &detect_model_201_TotWhImpPhB, MeterValueID::EnergyActiveL2Import },
        { &get_model_201_TotWhImpPhC, &detect_model_201_TotWhImpPhC, MeterValueID::EnergyActiveL3Import },
        { &get_model_201_TotVAhExp, &detect_model_201_TotVAhExp, MeterValueID::EnergyApparentLSumExport },
        { &get_model_201_TotVAhExpPhA, &detect_model_201_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export },
        { &get_model_201_TotVAhExpPhB, &detect_model_201_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export },
        { &get_model_201_TotVAhExpPhC, &detect_model_201_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export },
        { &get_model_201_TotVAhImp, &detect_model_201_TotVAhImp, MeterValueID::EnergyApparentLSumImport },
        { &get_model_201_TotVAhImpPhA, &detect_model_201_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import },
        { &get_model_201_TotVAhImpPhB, &detect_model_201_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import },
        { &get_model_201_TotVAhImpPhC, &detect_model_201_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import },
        { &get_model_201_TotVArhImpQ1, &detect_model_201_TotVArhImpQ1, MeterValueID::EnergyReactiveQ1LSum },
        { &get_model_201_TotVArhImpQ1PhA, &detect_model_201_TotVArhImpQ1PhA, MeterValueID::EnergyReactiveQ1L1 },
        { &get_model_201_TotVArhImpQ1PhB, &detect_model_201_TotVArhImpQ1PhB, MeterValueID::EnergyReactiveQ1L2 },
        { &get_model_201_TotVArhImpQ1PhC, &detect_model_201_TotVArhImpQ1PhC, MeterValueID::EnergyReactiveQ1L3 },
        { &get_model_201_TotVArhImpQ2, &detect_model_201_TotVArhImpQ2, MeterValueID::EnergyReactiveQ2LSum },
        { &get_model_201_TotVArhImpQ2PhA, &detect_model_201_TotVArhImpQ2PhA, MeterValueID::EnergyReactiveQ2L1 },
        { &get_model_201_TotVArhImpQ2PhB, &detect_model_201_TotVArhImpQ2PhB, MeterValueID::EnergyReactiveQ2L2 },
        { &get_model_201_TotVArhImpQ2PhC, &detect_model_201_TotVArhImpQ2PhC, MeterValueID::EnergyReactiveQ2L3 },
        { &get_model_201_TotVArhExpQ3, &detect_model_201_TotVArhExpQ3, MeterValueID::EnergyReactiveQ3LSum },
        { &get_model_201_TotVArhExpQ3PhA, &detect_model_201_TotVArhExpQ3PhA, MeterValueID::EnergyReactiveQ3L1 },
        { &get_model_201_TotVArhExpQ3PhB, &detect_model_201_TotVArhExpQ3PhB, MeterValueID::EnergyReactiveQ3L2 },
        { &get_model_201_TotVArhExpQ3PhC, &detect_model_201_TotVArhExpQ3PhC, MeterValueID::EnergyReactiveQ3L3 },
        { &get_model_201_TotVArhExpQ4, &detect_model_201_TotVArhExpQ4, MeterValueID::EnergyReactiveQ4LSum },
        { &get_model_201_TotVArhExpQ4PhA, &detect_model_201_TotVArhExpQ4PhA, MeterValueID::EnergyReactiveQ4L1 },
        { &get_model_201_TotVArhExpQ4PhB, &detect_model_201_TotVArhExpQ4PhB, MeterValueID::EnergyReactiveQ4L2 },
        { &get_model_201_TotVArhExpQ4PhC, &detect_model_201_TotVArhExpQ4PhC, MeterValueID::EnergyReactiveQ4L3 },
    }
};

// ==============================
// 202 - Split Single Phase Meter
// ==============================

#include "model_202.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_202_A(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->A;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_A(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->A);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_AphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->AphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_AphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->AphA);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_AphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->AphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_AphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->AphB);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_AphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->AphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_AphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->AphC);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PhV(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhV;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PhV(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PhV);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PhVphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PhVphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PhVphA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PhVphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PhVphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PhVphB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PhVphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PhVphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PhVphC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PPV(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PPV;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PPV(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PPV);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PhVphAB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphAB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PhVphAB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PhVphAB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PhVphBC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphBC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PhVphBC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PhVphBC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PhVphCA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphCA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PhVphCA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PhVphCA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_Hz(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->Hz;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_Hz(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->Hz);
    val *= get_scale_factor(model->Hz_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_W(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_W(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->W);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_WphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->WphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_WphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->WphA);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_WphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->WphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_WphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->WphB);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_WphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->WphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_WphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->WphC);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_VA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_VA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->VA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_VAphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VAphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_VAphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->VAphA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_VAphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VAphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_VAphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->VAphB);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_VAphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VAphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_VAphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->VAphC);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_VAR(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VAR;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_VAR(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->VAR);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_VARphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VARphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_VARphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->VARphA);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_VARphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VARphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_VARphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->VARphB);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_VARphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VARphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_VARphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->VARphC);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PF(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PF(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PF);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PFphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PFphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PFphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PFphA);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PFphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PFphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PFphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PFphB);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_PFphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PFphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_PFphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(model->PFphC);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotWhExp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotWhExp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExp));
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotWhImp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotWhImp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImp));
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVAhExp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVAhExp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExp));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVAhImp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVAhImp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImp));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhImpQ1PhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhImpQ1PhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhImpQ1PhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhImpQ1PhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhImpQ1PhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhImpQ1PhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhImpQ2PhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhImpQ2PhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhImpQ2PhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhImpQ2PhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhImpQ2PhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhImpQ2PhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhExpQ3PhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhExpQ3PhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhExpQ3PhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhExpQ3PhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhExpQ3PhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhExpQ3PhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhExpQ4PhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhExpQ4PhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhExpQ4PhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhExpQ4PhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_202_TotVArhExpQ4PhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_202_TotVArhExpQ4PhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static bool model_202_validator(const uint16_t * const register_data[2])
{
    const SunSpecSplitSinglePhaseMeterModel202_s *block0 = reinterpret_cast<const SunSpecSplitSinglePhaseMeterModel202_s *>(register_data[0]);
    const SunSpecSplitSinglePhaseMeterModel202_s *block1 = reinterpret_cast<const SunSpecSplitSinglePhaseMeterModel202_s *>(register_data[1]);
    if (block0->ID != 202) return false;
    if (block1->ID != 202) return false;
    if (block0->L  != 105) return false;
    if (block1->L  != 105) return false;
    if (block0->A_SF != block1->A_SF) return false;
    if (block0->V_SF != block1->V_SF) return false;
    if (block0->Hz_SF != block1->Hz_SF) return false;
    if (block0->W_SF != block1->W_SF) return false;
    if (block0->VA_SF != block1->VA_SF) return false;
    if (block0->VAR_SF != block1->VAR_SF) return false;
    if (block0->PF_SF != block1->PF_SF) return false;
    if (block0->TotWh_SF != block1->TotWh_SF) return false;
    if (block0->TotVAh_SF != block1->TotVAh_SF) return false;
    if (block0->TotVArh_SF != block1->TotVArh_SF) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_202_data = {
    202, // model_id
    &model_202_validator,
    61,  // value_count
    {    // value_data
        { &get_model_202_A, &detect_model_202_A, MeterValueID::CurrentLSumImExDiff },
        { &get_model_202_AphA, &detect_model_202_AphA, MeterValueID::CurrentL1ImExDiff },
        { &get_model_202_AphB, &detect_model_202_AphB, MeterValueID::CurrentL2ImExDiff },
        { &get_model_202_AphC, &detect_model_202_AphC, MeterValueID::CurrentL3ImExDiff },
        { &get_model_202_PhV, &detect_model_202_PhV, MeterValueID::VoltageLNAvg },
        { &get_model_202_PhVphA, &detect_model_202_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_202_PhVphB, &detect_model_202_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_202_PhVphC, &detect_model_202_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_202_PPV, &detect_model_202_PPV, MeterValueID::VoltageLLAvg },
        { &get_model_202_PhVphAB, &detect_model_202_PhVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_202_PhVphBC, &detect_model_202_PhVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_202_PhVphCA, &detect_model_202_PhVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_202_Hz, &detect_model_202_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_202_W, &detect_model_202_W, MeterValueID::PowerActiveLSumImExDiff },
        { &get_model_202_WphA, &detect_model_202_WphA, MeterValueID::PowerActiveL1ImExDiff },
        { &get_model_202_WphB, &detect_model_202_WphB, MeterValueID::PowerActiveL2ImExDiff },
        { &get_model_202_WphC, &detect_model_202_WphC, MeterValueID::PowerActiveL3ImExDiff },
        { &get_model_202_VA, &detect_model_202_VA, MeterValueID::PowerApparentLSum },
        { &get_model_202_VAphA, &detect_model_202_VAphA, MeterValueID::PowerApparentL1 },
        { &get_model_202_VAphB, &detect_model_202_VAphB, MeterValueID::PowerApparentL2 },
        { &get_model_202_VAphC, &detect_model_202_VAphC, MeterValueID::PowerApparentL3 },
        { &get_model_202_VAR, &detect_model_202_VAR, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_202_VARphA, &detect_model_202_VARphA, MeterValueID::PowerReactiveL1IndCapDiff },
        { &get_model_202_VARphB, &detect_model_202_VARphB, MeterValueID::PowerReactiveL2IndCapDiff },
        { &get_model_202_VARphC, &detect_model_202_VARphC, MeterValueID::PowerReactiveL3IndCapDiff },
        { &get_model_202_PF, &detect_model_202_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_202_PFphA, &detect_model_202_PFphA, MeterValueID::PowerFactorL1Directional },
        { &get_model_202_PFphB, &detect_model_202_PFphB, MeterValueID::PowerFactorL2Directional },
        { &get_model_202_PFphC, &detect_model_202_PFphC, MeterValueID::PowerFactorL3Directional },
        { &get_model_202_TotWhExp, &detect_model_202_TotWhExp, MeterValueID::EnergyActiveLSumExport },
        { &get_model_202_TotWhExpPhA, &detect_model_202_TotWhExpPhA, MeterValueID::EnergyActiveL1Export },
        { &get_model_202_TotWhExpPhB, &detect_model_202_TotWhExpPhB, MeterValueID::EnergyActiveL2Export },
        { &get_model_202_TotWhExpPhC, &detect_model_202_TotWhExpPhC, MeterValueID::EnergyActiveL3Export },
        { &get_model_202_TotWhImp, &detect_model_202_TotWhImp, MeterValueID::EnergyActiveLSumImport },
        { &get_model_202_TotWhImpPhA, &detect_model_202_TotWhImpPhA, MeterValueID::EnergyActiveL1Import },
        { &get_model_202_TotWhImpPhB, &detect_model_202_TotWhImpPhB, MeterValueID::EnergyActiveL2Import },
        { &get_model_202_TotWhImpPhC, &detect_model_202_TotWhImpPhC, MeterValueID::EnergyActiveL3Import },
        { &get_model_202_TotVAhExp, &detect_model_202_TotVAhExp, MeterValueID::EnergyApparentLSumExport },
        { &get_model_202_TotVAhExpPhA, &detect_model_202_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export },
        { &get_model_202_TotVAhExpPhB, &detect_model_202_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export },
        { &get_model_202_TotVAhExpPhC, &detect_model_202_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export },
        { &get_model_202_TotVAhImp, &detect_model_202_TotVAhImp, MeterValueID::EnergyApparentLSumImport },
        { &get_model_202_TotVAhImpPhA, &detect_model_202_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import },
        { &get_model_202_TotVAhImpPhB, &detect_model_202_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import },
        { &get_model_202_TotVAhImpPhC, &detect_model_202_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import },
        { &get_model_202_TotVArhImpQ1, &detect_model_202_TotVArhImpQ1, MeterValueID::EnergyReactiveQ1LSum },
        { &get_model_202_TotVArhImpQ1PhA, &detect_model_202_TotVArhImpQ1PhA, MeterValueID::EnergyReactiveQ1L1 },
        { &get_model_202_TotVArhImpQ1PhB, &detect_model_202_TotVArhImpQ1PhB, MeterValueID::EnergyReactiveQ1L2 },
        { &get_model_202_TotVArhImpQ1PhC, &detect_model_202_TotVArhImpQ1PhC, MeterValueID::EnergyReactiveQ1L3 },
        { &get_model_202_TotVArhImpQ2, &detect_model_202_TotVArhImpQ2, MeterValueID::EnergyReactiveQ2LSum },
        { &get_model_202_TotVArhImpQ2PhA, &detect_model_202_TotVArhImpQ2PhA, MeterValueID::EnergyReactiveQ2L1 },
        { &get_model_202_TotVArhImpQ2PhB, &detect_model_202_TotVArhImpQ2PhB, MeterValueID::EnergyReactiveQ2L2 },
        { &get_model_202_TotVArhImpQ2PhC, &detect_model_202_TotVArhImpQ2PhC, MeterValueID::EnergyReactiveQ2L3 },
        { &get_model_202_TotVArhExpQ3, &detect_model_202_TotVArhExpQ3, MeterValueID::EnergyReactiveQ3LSum },
        { &get_model_202_TotVArhExpQ3PhA, &detect_model_202_TotVArhExpQ3PhA, MeterValueID::EnergyReactiveQ3L1 },
        { &get_model_202_TotVArhExpQ3PhB, &detect_model_202_TotVArhExpQ3PhB, MeterValueID::EnergyReactiveQ3L2 },
        { &get_model_202_TotVArhExpQ3PhC, &detect_model_202_TotVArhExpQ3PhC, MeterValueID::EnergyReactiveQ3L3 },
        { &get_model_202_TotVArhExpQ4, &detect_model_202_TotVArhExpQ4, MeterValueID::EnergyReactiveQ4LSum },
        { &get_model_202_TotVArhExpQ4PhA, &detect_model_202_TotVArhExpQ4PhA, MeterValueID::EnergyReactiveQ4L1 },
        { &get_model_202_TotVArhExpQ4PhB, &detect_model_202_TotVArhExpQ4PhB, MeterValueID::EnergyReactiveQ4L2 },
        { &get_model_202_TotVArhExpQ4PhC, &detect_model_202_TotVArhExpQ4PhC, MeterValueID::EnergyReactiveQ4L3 },
    }
};

// ===================================
// 203 - Wye-Connect Three Phase Meter
// ===================================

#include "model_203.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_203_A(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->A;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_A(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->A);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_AphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->AphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_AphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->AphA);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_AphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->AphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_AphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->AphB);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_AphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->AphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_AphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->AphC);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PhV(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhV;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PhV(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PhV);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PhVphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PhVphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PhVphA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PhVphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PhVphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PhVphB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PhVphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PhVphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PhVphC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PPV(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PPV;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PPV(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PPV);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PhVphAB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphAB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PhVphAB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PhVphAB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PhVphBC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphBC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PhVphBC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PhVphBC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PhVphCA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphCA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PhVphCA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PhVphCA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_Hz(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->Hz;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_Hz(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->Hz);
    val *= get_scale_factor(model->Hz_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_W(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_W(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->W);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_WphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->WphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_WphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->WphA);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_WphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->WphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_WphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->WphB);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_WphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->WphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_WphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->WphC);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_VA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_VA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->VA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_VAphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VAphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_VAphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->VAphA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_VAphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VAphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_VAphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->VAphB);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_VAphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VAphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_VAphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->VAphC);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_VAR(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VAR;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_VAR(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->VAR);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_VARphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VARphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_VARphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->VARphA);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_VARphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VARphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_VARphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->VARphB);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_VARphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VARphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_VARphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->VARphC);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PF(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PF(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PF);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PFphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PFphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PFphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PFphA);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PFphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PFphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PFphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PFphB);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_PFphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PFphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_PFphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(model->PFphC);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotWhExp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotWhExp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExp));
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotWhImp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotWhImp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImp));
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVAhExp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVAhExp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExp));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVAhImp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVAhImp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImp));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhImpQ1PhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhImpQ1PhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhImpQ1PhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhImpQ1PhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhImpQ1PhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhImpQ1PhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhImpQ2PhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhImpQ2PhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhImpQ2PhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhImpQ2PhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhImpQ2PhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhImpQ2PhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhExpQ3PhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhExpQ3PhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhExpQ3PhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhExpQ3PhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhExpQ3PhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhExpQ3PhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhExpQ4PhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhExpQ4PhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhExpQ4PhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhExpQ4PhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_203_TotVArhExpQ4PhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_203_TotVArhExpQ4PhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static bool model_203_validator(const uint16_t * const register_data[2])
{
    const SunSpecWyeConnectThreePhaseMeterModel203_s *block0 = reinterpret_cast<const SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data[0]);
    const SunSpecWyeConnectThreePhaseMeterModel203_s *block1 = reinterpret_cast<const SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data[1]);
    if (block0->ID != 203) return false;
    if (block1->ID != 203) return false;
    if (block0->L  != 105) return false;
    if (block1->L  != 105) return false;
    if (block0->A_SF != block1->A_SF) return false;
    if (block0->V_SF != block1->V_SF) return false;
    if (block0->Hz_SF != block1->Hz_SF) return false;
    if (block0->W_SF != block1->W_SF) return false;
    if (block0->VA_SF != block1->VA_SF) return false;
    if (block0->VAR_SF != block1->VAR_SF) return false;
    if (block0->PF_SF != block1->PF_SF) return false;
    if (block0->TotWh_SF != block1->TotWh_SF) return false;
    if (block0->TotVAh_SF != block1->TotVAh_SF) return false;
    if (block0->TotVArh_SF != block1->TotVArh_SF) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_203_data = {
    203, // model_id
    &model_203_validator,
    61,  // value_count
    {    // value_data
        { &get_model_203_A, &detect_model_203_A, MeterValueID::CurrentLSumImExDiff },
        { &get_model_203_AphA, &detect_model_203_AphA, MeterValueID::CurrentL1ImExDiff },
        { &get_model_203_AphB, &detect_model_203_AphB, MeterValueID::CurrentL2ImExDiff },
        { &get_model_203_AphC, &detect_model_203_AphC, MeterValueID::CurrentL3ImExDiff },
        { &get_model_203_PhV, &detect_model_203_PhV, MeterValueID::VoltageLNAvg },
        { &get_model_203_PhVphA, &detect_model_203_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_203_PhVphB, &detect_model_203_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_203_PhVphC, &detect_model_203_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_203_PPV, &detect_model_203_PPV, MeterValueID::VoltageLLAvg },
        { &get_model_203_PhVphAB, &detect_model_203_PhVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_203_PhVphBC, &detect_model_203_PhVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_203_PhVphCA, &detect_model_203_PhVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_203_Hz, &detect_model_203_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_203_W, &detect_model_203_W, MeterValueID::PowerActiveLSumImExDiff },
        { &get_model_203_WphA, &detect_model_203_WphA, MeterValueID::PowerActiveL1ImExDiff },
        { &get_model_203_WphB, &detect_model_203_WphB, MeterValueID::PowerActiveL2ImExDiff },
        { &get_model_203_WphC, &detect_model_203_WphC, MeterValueID::PowerActiveL3ImExDiff },
        { &get_model_203_VA, &detect_model_203_VA, MeterValueID::PowerApparentLSum },
        { &get_model_203_VAphA, &detect_model_203_VAphA, MeterValueID::PowerApparentL1 },
        { &get_model_203_VAphB, &detect_model_203_VAphB, MeterValueID::PowerApparentL2 },
        { &get_model_203_VAphC, &detect_model_203_VAphC, MeterValueID::PowerApparentL3 },
        { &get_model_203_VAR, &detect_model_203_VAR, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_203_VARphA, &detect_model_203_VARphA, MeterValueID::PowerReactiveL1IndCapDiff },
        { &get_model_203_VARphB, &detect_model_203_VARphB, MeterValueID::PowerReactiveL2IndCapDiff },
        { &get_model_203_VARphC, &detect_model_203_VARphC, MeterValueID::PowerReactiveL3IndCapDiff },
        { &get_model_203_PF, &detect_model_203_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_203_PFphA, &detect_model_203_PFphA, MeterValueID::PowerFactorL1Directional },
        { &get_model_203_PFphB, &detect_model_203_PFphB, MeterValueID::PowerFactorL2Directional },
        { &get_model_203_PFphC, &detect_model_203_PFphC, MeterValueID::PowerFactorL3Directional },
        { &get_model_203_TotWhExp, &detect_model_203_TotWhExp, MeterValueID::EnergyActiveLSumExport },
        { &get_model_203_TotWhExpPhA, &detect_model_203_TotWhExpPhA, MeterValueID::EnergyActiveL1Export },
        { &get_model_203_TotWhExpPhB, &detect_model_203_TotWhExpPhB, MeterValueID::EnergyActiveL2Export },
        { &get_model_203_TotWhExpPhC, &detect_model_203_TotWhExpPhC, MeterValueID::EnergyActiveL3Export },
        { &get_model_203_TotWhImp, &detect_model_203_TotWhImp, MeterValueID::EnergyActiveLSumImport },
        { &get_model_203_TotWhImpPhA, &detect_model_203_TotWhImpPhA, MeterValueID::EnergyActiveL1Import },
        { &get_model_203_TotWhImpPhB, &detect_model_203_TotWhImpPhB, MeterValueID::EnergyActiveL2Import },
        { &get_model_203_TotWhImpPhC, &detect_model_203_TotWhImpPhC, MeterValueID::EnergyActiveL3Import },
        { &get_model_203_TotVAhExp, &detect_model_203_TotVAhExp, MeterValueID::EnergyApparentLSumExport },
        { &get_model_203_TotVAhExpPhA, &detect_model_203_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export },
        { &get_model_203_TotVAhExpPhB, &detect_model_203_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export },
        { &get_model_203_TotVAhExpPhC, &detect_model_203_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export },
        { &get_model_203_TotVAhImp, &detect_model_203_TotVAhImp, MeterValueID::EnergyApparentLSumImport },
        { &get_model_203_TotVAhImpPhA, &detect_model_203_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import },
        { &get_model_203_TotVAhImpPhB, &detect_model_203_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import },
        { &get_model_203_TotVAhImpPhC, &detect_model_203_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import },
        { &get_model_203_TotVArhImpQ1, &detect_model_203_TotVArhImpQ1, MeterValueID::EnergyReactiveQ1LSum },
        { &get_model_203_TotVArhImpQ1PhA, &detect_model_203_TotVArhImpQ1PhA, MeterValueID::EnergyReactiveQ1L1 },
        { &get_model_203_TotVArhImpQ1PhB, &detect_model_203_TotVArhImpQ1PhB, MeterValueID::EnergyReactiveQ1L2 },
        { &get_model_203_TotVArhImpQ1PhC, &detect_model_203_TotVArhImpQ1PhC, MeterValueID::EnergyReactiveQ1L3 },
        { &get_model_203_TotVArhImpQ2, &detect_model_203_TotVArhImpQ2, MeterValueID::EnergyReactiveQ2LSum },
        { &get_model_203_TotVArhImpQ2PhA, &detect_model_203_TotVArhImpQ2PhA, MeterValueID::EnergyReactiveQ2L1 },
        { &get_model_203_TotVArhImpQ2PhB, &detect_model_203_TotVArhImpQ2PhB, MeterValueID::EnergyReactiveQ2L2 },
        { &get_model_203_TotVArhImpQ2PhC, &detect_model_203_TotVArhImpQ2PhC, MeterValueID::EnergyReactiveQ2L3 },
        { &get_model_203_TotVArhExpQ3, &detect_model_203_TotVArhExpQ3, MeterValueID::EnergyReactiveQ3LSum },
        { &get_model_203_TotVArhExpQ3PhA, &detect_model_203_TotVArhExpQ3PhA, MeterValueID::EnergyReactiveQ3L1 },
        { &get_model_203_TotVArhExpQ3PhB, &detect_model_203_TotVArhExpQ3PhB, MeterValueID::EnergyReactiveQ3L2 },
        { &get_model_203_TotVArhExpQ3PhC, &detect_model_203_TotVArhExpQ3PhC, MeterValueID::EnergyReactiveQ3L3 },
        { &get_model_203_TotVArhExpQ4, &detect_model_203_TotVArhExpQ4, MeterValueID::EnergyReactiveQ4LSum },
        { &get_model_203_TotVArhExpQ4PhA, &detect_model_203_TotVArhExpQ4PhA, MeterValueID::EnergyReactiveQ4L1 },
        { &get_model_203_TotVArhExpQ4PhB, &detect_model_203_TotVArhExpQ4PhB, MeterValueID::EnergyReactiveQ4L2 },
        { &get_model_203_TotVArhExpQ4PhC, &detect_model_203_TotVArhExpQ4PhC, MeterValueID::EnergyReactiveQ4L3 },
    }
};

// =====================================
// 204 - Delta-Connect Three Phase Meter
// =====================================

#include "model_204.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_204_A(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->A;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_A(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->A);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_AphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->AphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_AphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->AphA);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_AphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->AphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_AphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->AphB);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_AphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->AphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_AphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->AphC);
    val *= get_scale_factor(model->A_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PhV(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhV;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PhV(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PhV);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PhVphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PhVphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PhVphA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PhVphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PhVphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PhVphB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PhVphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PhVphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PhVphC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PPV(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PPV;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PPV(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PPV);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PhVphAB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphAB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PhVphAB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PhVphAB);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PhVphBC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphBC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PhVphBC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PhVphBC);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PhVphCA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphCA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PhVphCA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PhVphCA);
    val *= get_scale_factor(model->V_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_Hz(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->Hz;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_Hz(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->Hz);
    val *= get_scale_factor(model->Hz_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_W(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_W(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->W);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_WphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->WphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_WphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->WphA);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_WphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->WphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_WphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->WphB);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_WphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->WphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_WphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->WphC);
    val *= get_scale_factor(model->W_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_VA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_VA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->VA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_VAphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VAphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_VAphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->VAphA);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_VAphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VAphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_VAphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->VAphB);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_VAphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VAphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_VAphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->VAphC);
    val *= get_scale_factor(model->VA_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_VAR(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VAR;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_VAR(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->VAR);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_VARphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VARphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_VARphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->VARphA);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_VARphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VARphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_VARphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->VARphB);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_VARphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VARphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_VARphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->VARphC);
    val *= get_scale_factor(model->VAR_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PF(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PF(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PF);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PFphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PFphA;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PFphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PFphA);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PFphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PFphB;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PFphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PFphB);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_PFphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PFphC;
    if (val == INT16_MIN) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_PFphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(model->PFphC);
    val *= get_scale_factor(model->PF_SF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotWhExp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotWhExp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExp));
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhExpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhExpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotWhImp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotWhImp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImp));
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotWhImpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotWhImpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVAhExp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVAhExp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExp));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhExpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhExpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVAhImp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImp);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVAhImp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImp));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVAhImpPhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVAhImpPhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhImpQ1PhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhImpQ1PhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhImpQ1PhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhImpQ1PhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhImpQ1PhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ1PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhImpQ1PhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ1PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhImpQ2PhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhImpQ2PhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhImpQ2PhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhImpQ2PhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhImpQ2PhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhImpQ2PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhImpQ2PhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhImpQ2PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhExpQ3PhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhExpQ3PhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhExpQ3PhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhExpQ3PhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhExpQ3PhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ3PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhExpQ3PhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ3PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhExpQ4PhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhA);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhExpQ4PhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhA));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhExpQ4PhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhB);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhExpQ4PhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhB));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_204_TotVArhExpQ4PhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(model->TotVArhExpQ4PhC);
    if (val == UINT32_MAX) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_204_TotVArhExpQ4PhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    float val = static_cast<float>(convert_me_uint32(model->TotVArhExpQ4PhC));
    if (val == 0.0f)
        return NAN;
    val *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return val;
}

static bool model_204_validator(const uint16_t * const register_data[2])
{
    const SunSpecDeltaConnectThreePhaseMeterModel204_s *block0 = reinterpret_cast<const SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data[0]);
    const SunSpecDeltaConnectThreePhaseMeterModel204_s *block1 = reinterpret_cast<const SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data[1]);
    if (block0->ID != 204) return false;
    if (block1->ID != 204) return false;
    if (block0->L  != 105) return false;
    if (block1->L  != 105) return false;
    if (block0->A_SF != block1->A_SF) return false;
    if (block0->V_SF != block1->V_SF) return false;
    if (block0->Hz_SF != block1->Hz_SF) return false;
    if (block0->W_SF != block1->W_SF) return false;
    if (block0->VA_SF != block1->VA_SF) return false;
    if (block0->VAR_SF != block1->VAR_SF) return false;
    if (block0->PF_SF != block1->PF_SF) return false;
    if (block0->TotWh_SF != block1->TotWh_SF) return false;
    if (block0->TotVAh_SF != block1->TotVAh_SF) return false;
    if (block0->TotVArh_SF != block1->TotVArh_SF) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_204_data = {
    204, // model_id
    &model_204_validator,
    61,  // value_count
    {    // value_data
        { &get_model_204_A, &detect_model_204_A, MeterValueID::CurrentLSumImExDiff },
        { &get_model_204_AphA, &detect_model_204_AphA, MeterValueID::CurrentL1ImExDiff },
        { &get_model_204_AphB, &detect_model_204_AphB, MeterValueID::CurrentL2ImExDiff },
        { &get_model_204_AphC, &detect_model_204_AphC, MeterValueID::CurrentL3ImExDiff },
        { &get_model_204_PhV, &detect_model_204_PhV, MeterValueID::VoltageLNAvg },
        { &get_model_204_PhVphA, &detect_model_204_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_204_PhVphB, &detect_model_204_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_204_PhVphC, &detect_model_204_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_204_PPV, &detect_model_204_PPV, MeterValueID::VoltageLLAvg },
        { &get_model_204_PhVphAB, &detect_model_204_PhVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_204_PhVphBC, &detect_model_204_PhVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_204_PhVphCA, &detect_model_204_PhVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_204_Hz, &detect_model_204_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_204_W, &detect_model_204_W, MeterValueID::PowerActiveLSumImExDiff },
        { &get_model_204_WphA, &detect_model_204_WphA, MeterValueID::PowerActiveL1ImExDiff },
        { &get_model_204_WphB, &detect_model_204_WphB, MeterValueID::PowerActiveL2ImExDiff },
        { &get_model_204_WphC, &detect_model_204_WphC, MeterValueID::PowerActiveL3ImExDiff },
        { &get_model_204_VA, &detect_model_204_VA, MeterValueID::PowerApparentLSum },
        { &get_model_204_VAphA, &detect_model_204_VAphA, MeterValueID::PowerApparentL1 },
        { &get_model_204_VAphB, &detect_model_204_VAphB, MeterValueID::PowerApparentL2 },
        { &get_model_204_VAphC, &detect_model_204_VAphC, MeterValueID::PowerApparentL3 },
        { &get_model_204_VAR, &detect_model_204_VAR, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_204_VARphA, &detect_model_204_VARphA, MeterValueID::PowerReactiveL1IndCapDiff },
        { &get_model_204_VARphB, &detect_model_204_VARphB, MeterValueID::PowerReactiveL2IndCapDiff },
        { &get_model_204_VARphC, &detect_model_204_VARphC, MeterValueID::PowerReactiveL3IndCapDiff },
        { &get_model_204_PF, &detect_model_204_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_204_PFphA, &detect_model_204_PFphA, MeterValueID::PowerFactorL1Directional },
        { &get_model_204_PFphB, &detect_model_204_PFphB, MeterValueID::PowerFactorL2Directional },
        { &get_model_204_PFphC, &detect_model_204_PFphC, MeterValueID::PowerFactorL3Directional },
        { &get_model_204_TotWhExp, &detect_model_204_TotWhExp, MeterValueID::EnergyActiveLSumExport },
        { &get_model_204_TotWhExpPhA, &detect_model_204_TotWhExpPhA, MeterValueID::EnergyActiveL1Export },
        { &get_model_204_TotWhExpPhB, &detect_model_204_TotWhExpPhB, MeterValueID::EnergyActiveL2Export },
        { &get_model_204_TotWhExpPhC, &detect_model_204_TotWhExpPhC, MeterValueID::EnergyActiveL3Export },
        { &get_model_204_TotWhImp, &detect_model_204_TotWhImp, MeterValueID::EnergyActiveLSumImport },
        { &get_model_204_TotWhImpPhA, &detect_model_204_TotWhImpPhA, MeterValueID::EnergyActiveL1Import },
        { &get_model_204_TotWhImpPhB, &detect_model_204_TotWhImpPhB, MeterValueID::EnergyActiveL2Import },
        { &get_model_204_TotWhImpPhC, &detect_model_204_TotWhImpPhC, MeterValueID::EnergyActiveL3Import },
        { &get_model_204_TotVAhExp, &detect_model_204_TotVAhExp, MeterValueID::EnergyApparentLSumExport },
        { &get_model_204_TotVAhExpPhA, &detect_model_204_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export },
        { &get_model_204_TotVAhExpPhB, &detect_model_204_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export },
        { &get_model_204_TotVAhExpPhC, &detect_model_204_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export },
        { &get_model_204_TotVAhImp, &detect_model_204_TotVAhImp, MeterValueID::EnergyApparentLSumImport },
        { &get_model_204_TotVAhImpPhA, &detect_model_204_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import },
        { &get_model_204_TotVAhImpPhB, &detect_model_204_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import },
        { &get_model_204_TotVAhImpPhC, &detect_model_204_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import },
        { &get_model_204_TotVArhImpQ1, &detect_model_204_TotVArhImpQ1, MeterValueID::EnergyReactiveQ1LSum },
        { &get_model_204_TotVArhImpQ1PhA, &detect_model_204_TotVArhImpQ1PhA, MeterValueID::EnergyReactiveQ1L1 },
        { &get_model_204_TotVArhImpQ1PhB, &detect_model_204_TotVArhImpQ1PhB, MeterValueID::EnergyReactiveQ1L2 },
        { &get_model_204_TotVArhImpQ1PhC, &detect_model_204_TotVArhImpQ1PhC, MeterValueID::EnergyReactiveQ1L3 },
        { &get_model_204_TotVArhImpQ2, &detect_model_204_TotVArhImpQ2, MeterValueID::EnergyReactiveQ2LSum },
        { &get_model_204_TotVArhImpQ2PhA, &detect_model_204_TotVArhImpQ2PhA, MeterValueID::EnergyReactiveQ2L1 },
        { &get_model_204_TotVArhImpQ2PhB, &detect_model_204_TotVArhImpQ2PhB, MeterValueID::EnergyReactiveQ2L2 },
        { &get_model_204_TotVArhImpQ2PhC, &detect_model_204_TotVArhImpQ2PhC, MeterValueID::EnergyReactiveQ2L3 },
        { &get_model_204_TotVArhExpQ3, &detect_model_204_TotVArhExpQ3, MeterValueID::EnergyReactiveQ3LSum },
        { &get_model_204_TotVArhExpQ3PhA, &detect_model_204_TotVArhExpQ3PhA, MeterValueID::EnergyReactiveQ3L1 },
        { &get_model_204_TotVArhExpQ3PhB, &detect_model_204_TotVArhExpQ3PhB, MeterValueID::EnergyReactiveQ3L2 },
        { &get_model_204_TotVArhExpQ3PhC, &detect_model_204_TotVArhExpQ3PhC, MeterValueID::EnergyReactiveQ3L3 },
        { &get_model_204_TotVArhExpQ4, &detect_model_204_TotVArhExpQ4, MeterValueID::EnergyReactiveQ4LSum },
        { &get_model_204_TotVArhExpQ4PhA, &detect_model_204_TotVArhExpQ4PhA, MeterValueID::EnergyReactiveQ4L1 },
        { &get_model_204_TotVArhExpQ4PhB, &detect_model_204_TotVArhExpQ4PhB, MeterValueID::EnergyReactiveQ4L2 },
        { &get_model_204_TotVArhExpQ4PhC, &detect_model_204_TotVArhExpQ4PhC, MeterValueID::EnergyReactiveQ4L3 },
    }
};

// ========================
// 211 - Single Phase Meter
// ========================

#include "model_211.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_211_A(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->A);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_A(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->A);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_AphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->AphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_AphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->AphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_AphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->AphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_AphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->AphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_AphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->AphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_AphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->AphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PhV(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PhV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PhV(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PhV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PhVphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PhVphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PhVphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PhVphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PhVphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PhVphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PPV(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PPV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PPV(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PPV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PPVphAB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PPVphAB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PPVphBC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PPVphBC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PPVphCA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PPVphCA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_Hz(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->Hz);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_Hz(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->Hz);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_W(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->W);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_W(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->W);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_WphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->WphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_WphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->WphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_WphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->WphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_WphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->WphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_WphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->WphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_WphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->WphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_VA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_VA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_VAphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VAphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_VAphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VAphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_VAphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VAphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_VAphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VAphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_VAphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VAphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_VAphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VAphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_VAR(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VAR);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_VAR(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VAR);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_VARphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VARphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_VARphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VARphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_VARphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VARphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_VARphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VARphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_VARphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VARphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_VARphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->VARphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PF(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PF);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PF(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PFphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PFphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PFphA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PFphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PFphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PFphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PFphB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PFphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_PFphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PFphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_PFphC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->PFphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotWhExp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhExp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotWhExp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhExp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotWhImp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhImp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotWhImp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhImp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVAhExp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhExp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVAhExp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhExp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVAhImp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhImp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVAhImp(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhImp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhImpQ1phA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhImpQ1phA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhImpQ1phB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhImpQ1phB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhImpQ1phC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhImpQ1phC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhImpQ2phA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhImpQ2phA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhImpQ2phB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhImpQ2phB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhImpQ2phC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhImpQ2phC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhExpQ3phA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhExpQ3phA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhExpQ3phB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhExpQ3phB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhExpQ3phC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhExpQ3phC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhExpQ4phA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhExpQ4phA(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhExpQ4phB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhExpQ4phB(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_211_TotVArhExpQ4phC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_211_TotVArhExpQ4phC(const void *register_data)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phC);
    val *= 0.001f;
    return val;
}

static bool model_211_validator(const uint16_t * const register_data[2])
{
    const SunSpecSinglePhaseMeterModel211_s *block0 = reinterpret_cast<const SunSpecSinglePhaseMeterModel211_s *>(register_data[0]);
    const SunSpecSinglePhaseMeterModel211_s *block1 = reinterpret_cast<const SunSpecSinglePhaseMeterModel211_s *>(register_data[1]);
    if (block0->ID != 211) return false;
    if (block1->ID != 211) return false;
    if (block0->L  != 124) return false;
    if (block1->L  != 124) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_211_data = {
    211, // model_id
    &model_211_validator,
    61,  // value_count
    {    // value_data
        { &get_model_211_A, &detect_model_211_A, MeterValueID::CurrentLSumImExDiff },
        { &get_model_211_AphA, &detect_model_211_AphA, MeterValueID::CurrentL1ImExDiff },
        { &get_model_211_AphB, &detect_model_211_AphB, MeterValueID::CurrentL2ImExDiff },
        { &get_model_211_AphC, &detect_model_211_AphC, MeterValueID::CurrentL3ImExDiff },
        { &get_model_211_PhV, &detect_model_211_PhV, MeterValueID::VoltageLNAvg },
        { &get_model_211_PhVphA, &detect_model_211_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_211_PhVphB, &detect_model_211_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_211_PhVphC, &detect_model_211_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_211_PPV, &detect_model_211_PPV, MeterValueID::VoltageLLAvg },
        { &get_model_211_PPVphAB, &detect_model_211_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_211_PPVphBC, &detect_model_211_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_211_PPVphCA, &detect_model_211_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_211_Hz, &detect_model_211_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_211_W, &detect_model_211_W, MeterValueID::PowerActiveLSumImExDiff },
        { &get_model_211_WphA, &detect_model_211_WphA, MeterValueID::PowerActiveL1ImExDiff },
        { &get_model_211_WphB, &detect_model_211_WphB, MeterValueID::PowerActiveL2ImExDiff },
        { &get_model_211_WphC, &detect_model_211_WphC, MeterValueID::PowerActiveL3ImExDiff },
        { &get_model_211_VA, &detect_model_211_VA, MeterValueID::PowerApparentLSum },
        { &get_model_211_VAphA, &detect_model_211_VAphA, MeterValueID::PowerApparentL1 },
        { &get_model_211_VAphB, &detect_model_211_VAphB, MeterValueID::PowerApparentL2 },
        { &get_model_211_VAphC, &detect_model_211_VAphC, MeterValueID::PowerApparentL3 },
        { &get_model_211_VAR, &detect_model_211_VAR, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_211_VARphA, &detect_model_211_VARphA, MeterValueID::PowerReactiveL1IndCapDiff },
        { &get_model_211_VARphB, &detect_model_211_VARphB, MeterValueID::PowerReactiveL2IndCapDiff },
        { &get_model_211_VARphC, &detect_model_211_VARphC, MeterValueID::PowerReactiveL3IndCapDiff },
        { &get_model_211_PF, &detect_model_211_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_211_PFphA, &detect_model_211_PFphA, MeterValueID::PowerFactorL1Directional },
        { &get_model_211_PFphB, &detect_model_211_PFphB, MeterValueID::PowerFactorL2Directional },
        { &get_model_211_PFphC, &detect_model_211_PFphC, MeterValueID::PowerFactorL3Directional },
        { &get_model_211_TotWhExp, &detect_model_211_TotWhExp, MeterValueID::EnergyActiveLSumExport },
        { &get_model_211_TotWhExpPhA, &detect_model_211_TotWhExpPhA, MeterValueID::EnergyActiveL1Export },
        { &get_model_211_TotWhExpPhB, &detect_model_211_TotWhExpPhB, MeterValueID::EnergyActiveL2Export },
        { &get_model_211_TotWhExpPhC, &detect_model_211_TotWhExpPhC, MeterValueID::EnergyActiveL3Export },
        { &get_model_211_TotWhImp, &detect_model_211_TotWhImp, MeterValueID::EnergyActiveLSumImport },
        { &get_model_211_TotWhImpPhA, &detect_model_211_TotWhImpPhA, MeterValueID::EnergyActiveL1Import },
        { &get_model_211_TotWhImpPhB, &detect_model_211_TotWhImpPhB, MeterValueID::EnergyActiveL2Import },
        { &get_model_211_TotWhImpPhC, &detect_model_211_TotWhImpPhC, MeterValueID::EnergyActiveL3Import },
        { &get_model_211_TotVAhExp, &detect_model_211_TotVAhExp, MeterValueID::EnergyApparentLSumExport },
        { &get_model_211_TotVAhExpPhA, &detect_model_211_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export },
        { &get_model_211_TotVAhExpPhB, &detect_model_211_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export },
        { &get_model_211_TotVAhExpPhC, &detect_model_211_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export },
        { &get_model_211_TotVAhImp, &detect_model_211_TotVAhImp, MeterValueID::EnergyApparentLSumImport },
        { &get_model_211_TotVAhImpPhA, &detect_model_211_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import },
        { &get_model_211_TotVAhImpPhB, &detect_model_211_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import },
        { &get_model_211_TotVAhImpPhC, &detect_model_211_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import },
        { &get_model_211_TotVArhImpQ1, &detect_model_211_TotVArhImpQ1, MeterValueID::EnergyReactiveQ1LSum },
        { &get_model_211_TotVArhImpQ1phA, &detect_model_211_TotVArhImpQ1phA, MeterValueID::EnergyReactiveQ1L1 },
        { &get_model_211_TotVArhImpQ1phB, &detect_model_211_TotVArhImpQ1phB, MeterValueID::EnergyReactiveQ1L2 },
        { &get_model_211_TotVArhImpQ1phC, &detect_model_211_TotVArhImpQ1phC, MeterValueID::EnergyReactiveQ1L3 },
        { &get_model_211_TotVArhImpQ2, &detect_model_211_TotVArhImpQ2, MeterValueID::EnergyReactiveQ2LSum },
        { &get_model_211_TotVArhImpQ2phA, &detect_model_211_TotVArhImpQ2phA, MeterValueID::EnergyReactiveQ2L1 },
        { &get_model_211_TotVArhImpQ2phB, &detect_model_211_TotVArhImpQ2phB, MeterValueID::EnergyReactiveQ2L2 },
        { &get_model_211_TotVArhImpQ2phC, &detect_model_211_TotVArhImpQ2phC, MeterValueID::EnergyReactiveQ2L3 },
        { &get_model_211_TotVArhExpQ3, &detect_model_211_TotVArhExpQ3, MeterValueID::EnergyReactiveQ3LSum },
        { &get_model_211_TotVArhExpQ3phA, &detect_model_211_TotVArhExpQ3phA, MeterValueID::EnergyReactiveQ3L1 },
        { &get_model_211_TotVArhExpQ3phB, &detect_model_211_TotVArhExpQ3phB, MeterValueID::EnergyReactiveQ3L2 },
        { &get_model_211_TotVArhExpQ3phC, &detect_model_211_TotVArhExpQ3phC, MeterValueID::EnergyReactiveQ3L3 },
        { &get_model_211_TotVArhExpQ4, &detect_model_211_TotVArhExpQ4, MeterValueID::EnergyReactiveQ4LSum },
        { &get_model_211_TotVArhExpQ4phA, &detect_model_211_TotVArhExpQ4phA, MeterValueID::EnergyReactiveQ4L1 },
        { &get_model_211_TotVArhExpQ4phB, &detect_model_211_TotVArhExpQ4phB, MeterValueID::EnergyReactiveQ4L2 },
        { &get_model_211_TotVArhExpQ4phC, &detect_model_211_TotVArhExpQ4phC, MeterValueID::EnergyReactiveQ4L3 },
    }
};

// ==============================
// 212 - Split Single Phase Meter
// ==============================

#include "model_212.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_212_A(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->A);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_A(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->A);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_AphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->AphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_AphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->AphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_AphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->AphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_AphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->AphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_AphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->AphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_AphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->AphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PhV(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PhV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PhV(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PhV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PhVphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PhVphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PhVphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PhVphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PhVphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PhVphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PPV(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PPV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PPV(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PPV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PPVphAB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PPVphAB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PPVphBC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PPVphBC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PPVphCA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PPVphCA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_Hz(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->Hz);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_Hz(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->Hz);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_W(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->W);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_W(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->W);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_WphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->WphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_WphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->WphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_WphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->WphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_WphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->WphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_WphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->WphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_WphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->WphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_VA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_VA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_VAphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VAphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_VAphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VAphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_VAphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VAphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_VAphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VAphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_VAphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VAphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_VAphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VAphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_VAR(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VAR);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_VAR(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VAR);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_VARphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VARphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_VARphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VARphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_VARphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VARphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_VARphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VARphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_VARphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VARphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_VARphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->VARphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PF(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PF);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PF(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PFphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PFphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PFphA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PFphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PFphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PFphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PFphB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PFphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_PFphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PFphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_PFphC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->PFphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotWhExp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhExp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotWhExp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhExp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotWhImp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhImp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotWhImp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhImp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVAhExp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhExp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVAhExp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhExp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVAhImp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhImp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVAhImp(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhImp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhImpQ1phA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhImpQ1phA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhImpQ1phB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhImpQ1phB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhImpQ1phC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhImpQ1phC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhImpQ2phA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhImpQ2phA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhImpQ2phB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhImpQ2phB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhImpQ2phC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhImpQ2phC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhExpQ3phA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhExpQ3phA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhExpQ3phB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhExpQ3phB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhExpQ3phC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhExpQ3phC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhExpQ4phA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhExpQ4phA(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhExpQ4phB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhExpQ4phB(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_212_TotVArhExpQ4phC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_212_TotVArhExpQ4phC(const void *register_data)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phC);
    val *= 0.001f;
    return val;
}

static bool model_212_validator(const uint16_t * const register_data[2])
{
    const SunSpecSplitSinglePhaseMeterModel212_s *block0 = reinterpret_cast<const SunSpecSplitSinglePhaseMeterModel212_s *>(register_data[0]);
    const SunSpecSplitSinglePhaseMeterModel212_s *block1 = reinterpret_cast<const SunSpecSplitSinglePhaseMeterModel212_s *>(register_data[1]);
    if (block0->ID != 212) return false;
    if (block1->ID != 212) return false;
    if (block0->L  != 124) return false;
    if (block1->L  != 124) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_212_data = {
    212, // model_id
    &model_212_validator,
    61,  // value_count
    {    // value_data
        { &get_model_212_A, &detect_model_212_A, MeterValueID::CurrentLSumImExDiff },
        { &get_model_212_AphA, &detect_model_212_AphA, MeterValueID::CurrentL1ImExDiff },
        { &get_model_212_AphB, &detect_model_212_AphB, MeterValueID::CurrentL2ImExDiff },
        { &get_model_212_AphC, &detect_model_212_AphC, MeterValueID::CurrentL3ImExDiff },
        { &get_model_212_PhV, &detect_model_212_PhV, MeterValueID::VoltageLNAvg },
        { &get_model_212_PhVphA, &detect_model_212_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_212_PhVphB, &detect_model_212_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_212_PhVphC, &detect_model_212_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_212_PPV, &detect_model_212_PPV, MeterValueID::VoltageLLAvg },
        { &get_model_212_PPVphAB, &detect_model_212_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_212_PPVphBC, &detect_model_212_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_212_PPVphCA, &detect_model_212_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_212_Hz, &detect_model_212_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_212_W, &detect_model_212_W, MeterValueID::PowerActiveLSumImExDiff },
        { &get_model_212_WphA, &detect_model_212_WphA, MeterValueID::PowerActiveL1ImExDiff },
        { &get_model_212_WphB, &detect_model_212_WphB, MeterValueID::PowerActiveL2ImExDiff },
        { &get_model_212_WphC, &detect_model_212_WphC, MeterValueID::PowerActiveL3ImExDiff },
        { &get_model_212_VA, &detect_model_212_VA, MeterValueID::PowerApparentLSum },
        { &get_model_212_VAphA, &detect_model_212_VAphA, MeterValueID::PowerApparentL1 },
        { &get_model_212_VAphB, &detect_model_212_VAphB, MeterValueID::PowerApparentL2 },
        { &get_model_212_VAphC, &detect_model_212_VAphC, MeterValueID::PowerApparentL3 },
        { &get_model_212_VAR, &detect_model_212_VAR, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_212_VARphA, &detect_model_212_VARphA, MeterValueID::PowerReactiveL1IndCapDiff },
        { &get_model_212_VARphB, &detect_model_212_VARphB, MeterValueID::PowerReactiveL2IndCapDiff },
        { &get_model_212_VARphC, &detect_model_212_VARphC, MeterValueID::PowerReactiveL3IndCapDiff },
        { &get_model_212_PF, &detect_model_212_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_212_PFphA, &detect_model_212_PFphA, MeterValueID::PowerFactorL1Directional },
        { &get_model_212_PFphB, &detect_model_212_PFphB, MeterValueID::PowerFactorL2Directional },
        { &get_model_212_PFphC, &detect_model_212_PFphC, MeterValueID::PowerFactorL3Directional },
        { &get_model_212_TotWhExp, &detect_model_212_TotWhExp, MeterValueID::EnergyActiveLSumExport },
        { &get_model_212_TotWhExpPhA, &detect_model_212_TotWhExpPhA, MeterValueID::EnergyActiveL1Export },
        { &get_model_212_TotWhExpPhB, &detect_model_212_TotWhExpPhB, MeterValueID::EnergyActiveL2Export },
        { &get_model_212_TotWhExpPhC, &detect_model_212_TotWhExpPhC, MeterValueID::EnergyActiveL3Export },
        { &get_model_212_TotWhImp, &detect_model_212_TotWhImp, MeterValueID::EnergyActiveLSumImport },
        { &get_model_212_TotWhImpPhA, &detect_model_212_TotWhImpPhA, MeterValueID::EnergyActiveL1Import },
        { &get_model_212_TotWhImpPhB, &detect_model_212_TotWhImpPhB, MeterValueID::EnergyActiveL2Import },
        { &get_model_212_TotWhImpPhC, &detect_model_212_TotWhImpPhC, MeterValueID::EnergyActiveL3Import },
        { &get_model_212_TotVAhExp, &detect_model_212_TotVAhExp, MeterValueID::EnergyApparentLSumExport },
        { &get_model_212_TotVAhExpPhA, &detect_model_212_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export },
        { &get_model_212_TotVAhExpPhB, &detect_model_212_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export },
        { &get_model_212_TotVAhExpPhC, &detect_model_212_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export },
        { &get_model_212_TotVAhImp, &detect_model_212_TotVAhImp, MeterValueID::EnergyApparentLSumImport },
        { &get_model_212_TotVAhImpPhA, &detect_model_212_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import },
        { &get_model_212_TotVAhImpPhB, &detect_model_212_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import },
        { &get_model_212_TotVAhImpPhC, &detect_model_212_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import },
        { &get_model_212_TotVArhImpQ1, &detect_model_212_TotVArhImpQ1, MeterValueID::EnergyReactiveQ1LSum },
        { &get_model_212_TotVArhImpQ1phA, &detect_model_212_TotVArhImpQ1phA, MeterValueID::EnergyReactiveQ1L1 },
        { &get_model_212_TotVArhImpQ1phB, &detect_model_212_TotVArhImpQ1phB, MeterValueID::EnergyReactiveQ1L2 },
        { &get_model_212_TotVArhImpQ1phC, &detect_model_212_TotVArhImpQ1phC, MeterValueID::EnergyReactiveQ1L3 },
        { &get_model_212_TotVArhImpQ2, &detect_model_212_TotVArhImpQ2, MeterValueID::EnergyReactiveQ2LSum },
        { &get_model_212_TotVArhImpQ2phA, &detect_model_212_TotVArhImpQ2phA, MeterValueID::EnergyReactiveQ2L1 },
        { &get_model_212_TotVArhImpQ2phB, &detect_model_212_TotVArhImpQ2phB, MeterValueID::EnergyReactiveQ2L2 },
        { &get_model_212_TotVArhImpQ2phC, &detect_model_212_TotVArhImpQ2phC, MeterValueID::EnergyReactiveQ2L3 },
        { &get_model_212_TotVArhExpQ3, &detect_model_212_TotVArhExpQ3, MeterValueID::EnergyReactiveQ3LSum },
        { &get_model_212_TotVArhExpQ3phA, &detect_model_212_TotVArhExpQ3phA, MeterValueID::EnergyReactiveQ3L1 },
        { &get_model_212_TotVArhExpQ3phB, &detect_model_212_TotVArhExpQ3phB, MeterValueID::EnergyReactiveQ3L2 },
        { &get_model_212_TotVArhExpQ3phC, &detect_model_212_TotVArhExpQ3phC, MeterValueID::EnergyReactiveQ3L3 },
        { &get_model_212_TotVArhExpQ4, &detect_model_212_TotVArhExpQ4, MeterValueID::EnergyReactiveQ4LSum },
        { &get_model_212_TotVArhExpQ4phA, &detect_model_212_TotVArhExpQ4phA, MeterValueID::EnergyReactiveQ4L1 },
        { &get_model_212_TotVArhExpQ4phB, &detect_model_212_TotVArhExpQ4phB, MeterValueID::EnergyReactiveQ4L2 },
        { &get_model_212_TotVArhExpQ4phC, &detect_model_212_TotVArhExpQ4phC, MeterValueID::EnergyReactiveQ4L3 },
    }
};

// ===================================
// 213 - Wye-Connect Three Phase Meter
// ===================================

#include "model_213.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_213_A(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->A);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_A(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->A);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_AphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->AphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_AphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->AphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_AphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->AphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_AphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->AphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_AphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->AphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_AphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->AphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PhV(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PhV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PhV(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PhV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PhVphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PhVphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PhVphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PhVphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PhVphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PhVphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PPV(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PPV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PPV(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PPV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PPVphAB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PPVphAB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PPVphBC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PPVphBC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PPVphCA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PPVphCA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_Hz(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->Hz);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_Hz(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->Hz);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_W(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->W);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_W(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->W);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_WphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->WphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_WphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->WphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_WphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->WphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_WphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->WphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_WphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->WphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_WphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->WphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_VA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_VA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_VAphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VAphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_VAphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VAphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_VAphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VAphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_VAphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VAphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_VAphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VAphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_VAphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VAphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_VAR(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VAR);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_VAR(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VAR);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_VARphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VARphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_VARphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VARphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_VARphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VARphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_VARphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VARphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_VARphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VARphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_VARphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->VARphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PF(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PF);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PF(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PFphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PFphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PFphA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PFphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PFphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PFphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PFphB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PFphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_PFphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PFphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_PFphC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->PFphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotWhExp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhExp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotWhExp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhExp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotWhImp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhImp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotWhImp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhImp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVAhExp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhExp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVAhExp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhExp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVAhImp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhImp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVAhImp(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhImp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhImpQ1phA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhImpQ1phA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhImpQ1phB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhImpQ1phB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhImpQ1phC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhImpQ1phC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhImpQ2phA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhImpQ2phA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhImpQ2phB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhImpQ2phB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhImpQ2phC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhImpQ2phC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhExpQ3phA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhExpQ3phA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhExpQ3phB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhExpQ3phB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhExpQ3phC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhExpQ3phC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhExpQ4phA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhExpQ4phA(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhExpQ4phB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhExpQ4phB(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_213_TotVArhExpQ4phC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_213_TotVArhExpQ4phC(const void *register_data)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phC);
    val *= 0.001f;
    return val;
}

static bool model_213_validator(const uint16_t * const register_data[2])
{
    const SunSpecWyeConnectThreePhaseMeterModel213_s *block0 = reinterpret_cast<const SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data[0]);
    const SunSpecWyeConnectThreePhaseMeterModel213_s *block1 = reinterpret_cast<const SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data[1]);
    if (block0->ID != 213) return false;
    if (block1->ID != 213) return false;
    if (block0->L  != 124) return false;
    if (block1->L  != 124) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_213_data = {
    213, // model_id
    &model_213_validator,
    61,  // value_count
    {    // value_data
        { &get_model_213_A, &detect_model_213_A, MeterValueID::CurrentLSumImExDiff },
        { &get_model_213_AphA, &detect_model_213_AphA, MeterValueID::CurrentL1ImExDiff },
        { &get_model_213_AphB, &detect_model_213_AphB, MeterValueID::CurrentL2ImExDiff },
        { &get_model_213_AphC, &detect_model_213_AphC, MeterValueID::CurrentL3ImExDiff },
        { &get_model_213_PhV, &detect_model_213_PhV, MeterValueID::VoltageLNAvg },
        { &get_model_213_PhVphA, &detect_model_213_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_213_PhVphB, &detect_model_213_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_213_PhVphC, &detect_model_213_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_213_PPV, &detect_model_213_PPV, MeterValueID::VoltageLLAvg },
        { &get_model_213_PPVphAB, &detect_model_213_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_213_PPVphBC, &detect_model_213_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_213_PPVphCA, &detect_model_213_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_213_Hz, &detect_model_213_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_213_W, &detect_model_213_W, MeterValueID::PowerActiveLSumImExDiff },
        { &get_model_213_WphA, &detect_model_213_WphA, MeterValueID::PowerActiveL1ImExDiff },
        { &get_model_213_WphB, &detect_model_213_WphB, MeterValueID::PowerActiveL2ImExDiff },
        { &get_model_213_WphC, &detect_model_213_WphC, MeterValueID::PowerActiveL3ImExDiff },
        { &get_model_213_VA, &detect_model_213_VA, MeterValueID::PowerApparentLSum },
        { &get_model_213_VAphA, &detect_model_213_VAphA, MeterValueID::PowerApparentL1 },
        { &get_model_213_VAphB, &detect_model_213_VAphB, MeterValueID::PowerApparentL2 },
        { &get_model_213_VAphC, &detect_model_213_VAphC, MeterValueID::PowerApparentL3 },
        { &get_model_213_VAR, &detect_model_213_VAR, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_213_VARphA, &detect_model_213_VARphA, MeterValueID::PowerReactiveL1IndCapDiff },
        { &get_model_213_VARphB, &detect_model_213_VARphB, MeterValueID::PowerReactiveL2IndCapDiff },
        { &get_model_213_VARphC, &detect_model_213_VARphC, MeterValueID::PowerReactiveL3IndCapDiff },
        { &get_model_213_PF, &detect_model_213_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_213_PFphA, &detect_model_213_PFphA, MeterValueID::PowerFactorL1Directional },
        { &get_model_213_PFphB, &detect_model_213_PFphB, MeterValueID::PowerFactorL2Directional },
        { &get_model_213_PFphC, &detect_model_213_PFphC, MeterValueID::PowerFactorL3Directional },
        { &get_model_213_TotWhExp, &detect_model_213_TotWhExp, MeterValueID::EnergyActiveLSumExport },
        { &get_model_213_TotWhExpPhA, &detect_model_213_TotWhExpPhA, MeterValueID::EnergyActiveL1Export },
        { &get_model_213_TotWhExpPhB, &detect_model_213_TotWhExpPhB, MeterValueID::EnergyActiveL2Export },
        { &get_model_213_TotWhExpPhC, &detect_model_213_TotWhExpPhC, MeterValueID::EnergyActiveL3Export },
        { &get_model_213_TotWhImp, &detect_model_213_TotWhImp, MeterValueID::EnergyActiveLSumImport },
        { &get_model_213_TotWhImpPhA, &detect_model_213_TotWhImpPhA, MeterValueID::EnergyActiveL1Import },
        { &get_model_213_TotWhImpPhB, &detect_model_213_TotWhImpPhB, MeterValueID::EnergyActiveL2Import },
        { &get_model_213_TotWhImpPhC, &detect_model_213_TotWhImpPhC, MeterValueID::EnergyActiveL3Import },
        { &get_model_213_TotVAhExp, &detect_model_213_TotVAhExp, MeterValueID::EnergyApparentLSumExport },
        { &get_model_213_TotVAhExpPhA, &detect_model_213_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export },
        { &get_model_213_TotVAhExpPhB, &detect_model_213_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export },
        { &get_model_213_TotVAhExpPhC, &detect_model_213_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export },
        { &get_model_213_TotVAhImp, &detect_model_213_TotVAhImp, MeterValueID::EnergyApparentLSumImport },
        { &get_model_213_TotVAhImpPhA, &detect_model_213_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import },
        { &get_model_213_TotVAhImpPhB, &detect_model_213_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import },
        { &get_model_213_TotVAhImpPhC, &detect_model_213_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import },
        { &get_model_213_TotVArhImpQ1, &detect_model_213_TotVArhImpQ1, MeterValueID::EnergyReactiveQ1LSum },
        { &get_model_213_TotVArhImpQ1phA, &detect_model_213_TotVArhImpQ1phA, MeterValueID::EnergyReactiveQ1L1 },
        { &get_model_213_TotVArhImpQ1phB, &detect_model_213_TotVArhImpQ1phB, MeterValueID::EnergyReactiveQ1L2 },
        { &get_model_213_TotVArhImpQ1phC, &detect_model_213_TotVArhImpQ1phC, MeterValueID::EnergyReactiveQ1L3 },
        { &get_model_213_TotVArhImpQ2, &detect_model_213_TotVArhImpQ2, MeterValueID::EnergyReactiveQ2LSum },
        { &get_model_213_TotVArhImpQ2phA, &detect_model_213_TotVArhImpQ2phA, MeterValueID::EnergyReactiveQ2L1 },
        { &get_model_213_TotVArhImpQ2phB, &detect_model_213_TotVArhImpQ2phB, MeterValueID::EnergyReactiveQ2L2 },
        { &get_model_213_TotVArhImpQ2phC, &detect_model_213_TotVArhImpQ2phC, MeterValueID::EnergyReactiveQ2L3 },
        { &get_model_213_TotVArhExpQ3, &detect_model_213_TotVArhExpQ3, MeterValueID::EnergyReactiveQ3LSum },
        { &get_model_213_TotVArhExpQ3phA, &detect_model_213_TotVArhExpQ3phA, MeterValueID::EnergyReactiveQ3L1 },
        { &get_model_213_TotVArhExpQ3phB, &detect_model_213_TotVArhExpQ3phB, MeterValueID::EnergyReactiveQ3L2 },
        { &get_model_213_TotVArhExpQ3phC, &detect_model_213_TotVArhExpQ3phC, MeterValueID::EnergyReactiveQ3L3 },
        { &get_model_213_TotVArhExpQ4, &detect_model_213_TotVArhExpQ4, MeterValueID::EnergyReactiveQ4LSum },
        { &get_model_213_TotVArhExpQ4phA, &detect_model_213_TotVArhExpQ4phA, MeterValueID::EnergyReactiveQ4L1 },
        { &get_model_213_TotVArhExpQ4phB, &detect_model_213_TotVArhExpQ4phB, MeterValueID::EnergyReactiveQ4L2 },
        { &get_model_213_TotVArhExpQ4phC, &detect_model_213_TotVArhExpQ4phC, MeterValueID::EnergyReactiveQ4L3 },
    }
};

// =====================================
// 214 - Delta-Connect Three Phase Meter
// =====================================

#include "model_214.h"

static MetersSunSpecParser::ValueDetectionResult detect_model_214_A(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->A);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_A(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->A);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_AphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->AphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_AphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->AphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_AphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->AphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_AphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->AphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_AphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->AphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_AphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->AphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PhV(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PhV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PhV(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PhV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PhVphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PhVphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PhVphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PhVphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PhVphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PhVphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PhVphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PhVphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PhVphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PPV(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PPV);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PPV(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PPV);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PPVphAB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PPVphAB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PPVphAB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PPVphBC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PPVphBC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PPVphBC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PPVphCA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PPVphCA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PPVphCA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_Hz(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->Hz);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_Hz(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->Hz);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_W(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->W);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_W(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->W);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_WphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->WphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_WphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->WphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_WphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->WphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_WphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->WphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_WphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->WphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_WphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->WphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_VA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_VA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_VAphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VAphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_VAphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VAphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_VAphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VAphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_VAphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VAphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_VAphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VAphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_VAphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VAphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_VAR(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VAR);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_VAR(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VAR);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_VARphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VARphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_VARphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VARphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_VARphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VARphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_VARphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VARphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_VARphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VARphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_VARphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->VARphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PF(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PF);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PF(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PF);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PFphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PFphA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PFphA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PFphA);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PFphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PFphB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PFphB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PFphB);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_PFphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PFphC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_PFphC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->PFphC);
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotWhExp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhExp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotWhExp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhExp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotWhExpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotWhExpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotWhExpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhExpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotWhImp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhImp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotWhImp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhImp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotWhImpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotWhImpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotWhImpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotWhImpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVAhExp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhExp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVAhExp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhExp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVAhExpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVAhExpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVAhExpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhExpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVAhImp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhImp);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVAhImp(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhImp);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVAhImpPhA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVAhImpPhB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVAhImpPhC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVAhImpPhC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhImpQ1(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhImpQ1phA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhImpQ1phA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhImpQ1phB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhImpQ1phB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhImpQ1phC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhImpQ1phC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ1phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhImpQ2(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhImpQ2phA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhImpQ2phA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhImpQ2phB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhImpQ2phB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhImpQ2phC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhImpQ2phC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhImpQ2phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhExpQ3(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhExpQ3phA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhExpQ3phA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhExpQ3phB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhExpQ3phB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhExpQ3phC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhExpQ3phC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ3phC);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhExpQ4(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhExpQ4phA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phA);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhExpQ4phA(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phA);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhExpQ4phB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phB);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhExpQ4phB(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phB);
    val *= 0.001f;
    return val;
}

static MetersSunSpecParser::ValueDetectionResult detect_model_214_TotVArhExpQ4phC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phC);
    if (isnan(val)) {
        return MetersSunSpecParser::ValueDetectionResult::Unavailable;
    } else {
        return MetersSunSpecParser::ValueDetectionResult::Available;
    }
}

static float get_model_214_TotVArhExpQ4phC(const void *register_data)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(model->TotVArhExpQ4phC);
    val *= 0.001f;
    return val;
}

static bool model_214_validator(const uint16_t * const register_data[2])
{
    const SunSpecDeltaConnectThreePhaseMeterModel214_s *block0 = reinterpret_cast<const SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data[0]);
    const SunSpecDeltaConnectThreePhaseMeterModel214_s *block1 = reinterpret_cast<const SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data[1]);
    if (block0->ID != 214) return false;
    if (block1->ID != 214) return false;
    if (block0->L  != 124) return false;
    if (block1->L  != 124) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_214_data = {
    214, // model_id
    &model_214_validator,
    61,  // value_count
    {    // value_data
        { &get_model_214_A, &detect_model_214_A, MeterValueID::CurrentLSumImExDiff },
        { &get_model_214_AphA, &detect_model_214_AphA, MeterValueID::CurrentL1ImExDiff },
        { &get_model_214_AphB, &detect_model_214_AphB, MeterValueID::CurrentL2ImExDiff },
        { &get_model_214_AphC, &detect_model_214_AphC, MeterValueID::CurrentL3ImExDiff },
        { &get_model_214_PhV, &detect_model_214_PhV, MeterValueID::VoltageLNAvg },
        { &get_model_214_PhVphA, &detect_model_214_PhVphA, MeterValueID::VoltageL1N },
        { &get_model_214_PhVphB, &detect_model_214_PhVphB, MeterValueID::VoltageL2N },
        { &get_model_214_PhVphC, &detect_model_214_PhVphC, MeterValueID::VoltageL3N },
        { &get_model_214_PPV, &detect_model_214_PPV, MeterValueID::VoltageLLAvg },
        { &get_model_214_PPVphAB, &detect_model_214_PPVphAB, MeterValueID::VoltageL1L2 },
        { &get_model_214_PPVphBC, &detect_model_214_PPVphBC, MeterValueID::VoltageL2L3 },
        { &get_model_214_PPVphCA, &detect_model_214_PPVphCA, MeterValueID::VoltageL3L1 },
        { &get_model_214_Hz, &detect_model_214_Hz, MeterValueID::FrequencyLAvg },
        { &get_model_214_W, &detect_model_214_W, MeterValueID::PowerActiveLSumImExDiff },
        { &get_model_214_WphA, &detect_model_214_WphA, MeterValueID::PowerActiveL1ImExDiff },
        { &get_model_214_WphB, &detect_model_214_WphB, MeterValueID::PowerActiveL2ImExDiff },
        { &get_model_214_WphC, &detect_model_214_WphC, MeterValueID::PowerActiveL3ImExDiff },
        { &get_model_214_VA, &detect_model_214_VA, MeterValueID::PowerApparentLSum },
        { &get_model_214_VAphA, &detect_model_214_VAphA, MeterValueID::PowerApparentL1 },
        { &get_model_214_VAphB, &detect_model_214_VAphB, MeterValueID::PowerApparentL2 },
        { &get_model_214_VAphC, &detect_model_214_VAphC, MeterValueID::PowerApparentL3 },
        { &get_model_214_VAR, &detect_model_214_VAR, MeterValueID::PowerReactiveLSumIndCapDiff },
        { &get_model_214_VARphA, &detect_model_214_VARphA, MeterValueID::PowerReactiveL1IndCapDiff },
        { &get_model_214_VARphB, &detect_model_214_VARphB, MeterValueID::PowerReactiveL2IndCapDiff },
        { &get_model_214_VARphC, &detect_model_214_VARphC, MeterValueID::PowerReactiveL3IndCapDiff },
        { &get_model_214_PF, &detect_model_214_PF, MeterValueID::PowerFactorLSumDirectional },
        { &get_model_214_PFphA, &detect_model_214_PFphA, MeterValueID::PowerFactorL1Directional },
        { &get_model_214_PFphB, &detect_model_214_PFphB, MeterValueID::PowerFactorL2Directional },
        { &get_model_214_PFphC, &detect_model_214_PFphC, MeterValueID::PowerFactorL3Directional },
        { &get_model_214_TotWhExp, &detect_model_214_TotWhExp, MeterValueID::EnergyActiveLSumExport },
        { &get_model_214_TotWhExpPhA, &detect_model_214_TotWhExpPhA, MeterValueID::EnergyActiveL1Export },
        { &get_model_214_TotWhExpPhB, &detect_model_214_TotWhExpPhB, MeterValueID::EnergyActiveL2Export },
        { &get_model_214_TotWhExpPhC, &detect_model_214_TotWhExpPhC, MeterValueID::EnergyActiveL3Export },
        { &get_model_214_TotWhImp, &detect_model_214_TotWhImp, MeterValueID::EnergyActiveLSumImport },
        { &get_model_214_TotWhImpPhA, &detect_model_214_TotWhImpPhA, MeterValueID::EnergyActiveL1Import },
        { &get_model_214_TotWhImpPhB, &detect_model_214_TotWhImpPhB, MeterValueID::EnergyActiveL2Import },
        { &get_model_214_TotWhImpPhC, &detect_model_214_TotWhImpPhC, MeterValueID::EnergyActiveL3Import },
        { &get_model_214_TotVAhExp, &detect_model_214_TotVAhExp, MeterValueID::EnergyApparentLSumExport },
        { &get_model_214_TotVAhExpPhA, &detect_model_214_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export },
        { &get_model_214_TotVAhExpPhB, &detect_model_214_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export },
        { &get_model_214_TotVAhExpPhC, &detect_model_214_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export },
        { &get_model_214_TotVAhImp, &detect_model_214_TotVAhImp, MeterValueID::EnergyApparentLSumImport },
        { &get_model_214_TotVAhImpPhA, &detect_model_214_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import },
        { &get_model_214_TotVAhImpPhB, &detect_model_214_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import },
        { &get_model_214_TotVAhImpPhC, &detect_model_214_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import },
        { &get_model_214_TotVArhImpQ1, &detect_model_214_TotVArhImpQ1, MeterValueID::EnergyReactiveQ1LSum },
        { &get_model_214_TotVArhImpQ1phA, &detect_model_214_TotVArhImpQ1phA, MeterValueID::EnergyReactiveQ1L1 },
        { &get_model_214_TotVArhImpQ1phB, &detect_model_214_TotVArhImpQ1phB, MeterValueID::EnergyReactiveQ1L2 },
        { &get_model_214_TotVArhImpQ1phC, &detect_model_214_TotVArhImpQ1phC, MeterValueID::EnergyReactiveQ1L3 },
        { &get_model_214_TotVArhImpQ2, &detect_model_214_TotVArhImpQ2, MeterValueID::EnergyReactiveQ2LSum },
        { &get_model_214_TotVArhImpQ2phA, &detect_model_214_TotVArhImpQ2phA, MeterValueID::EnergyReactiveQ2L1 },
        { &get_model_214_TotVArhImpQ2phB, &detect_model_214_TotVArhImpQ2phB, MeterValueID::EnergyReactiveQ2L2 },
        { &get_model_214_TotVArhImpQ2phC, &detect_model_214_TotVArhImpQ2phC, MeterValueID::EnergyReactiveQ2L3 },
        { &get_model_214_TotVArhExpQ3, &detect_model_214_TotVArhExpQ3, MeterValueID::EnergyReactiveQ3LSum },
        { &get_model_214_TotVArhExpQ3phA, &detect_model_214_TotVArhExpQ3phA, MeterValueID::EnergyReactiveQ3L1 },
        { &get_model_214_TotVArhExpQ3phB, &detect_model_214_TotVArhExpQ3phB, MeterValueID::EnergyReactiveQ3L2 },
        { &get_model_214_TotVArhExpQ3phC, &detect_model_214_TotVArhExpQ3phC, MeterValueID::EnergyReactiveQ3L3 },
        { &get_model_214_TotVArhExpQ4, &detect_model_214_TotVArhExpQ4, MeterValueID::EnergyReactiveQ4LSum },
        { &get_model_214_TotVArhExpQ4phA, &detect_model_214_TotVArhExpQ4phA, MeterValueID::EnergyReactiveQ4L1 },
        { &get_model_214_TotVArhExpQ4phB, &detect_model_214_TotVArhExpQ4phB, MeterValueID::EnergyReactiveQ4L2 },
        { &get_model_214_TotVArhExpQ4phC, &detect_model_214_TotVArhExpQ4phC, MeterValueID::EnergyReactiveQ4L3 },
    }
};


const MetersSunSpecParser::AllModelData meters_sun_spec_all_model_data {
    15, // model_count
    { // model_data
        &model_001_data,
        &model_101_data,
        &model_102_data,
        &model_103_data,
        &model_111_data,
        &model_112_data,
        &model_113_data,
        &model_201_data,
        &model_202_data,
        &model_203_data,
        &model_204_data,
        &model_211_data,
        &model_212_data,
        &model_213_data,
        &model_214_data,
    }
};
