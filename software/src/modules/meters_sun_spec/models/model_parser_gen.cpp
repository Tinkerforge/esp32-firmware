// WARNING: This file is generated.

#include <math.h>
#include <stdint.h>

#include "../model_parser.h"

#include "gcc_warnings.h"

#if defined(__GNUC__)
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

static inline uint32_t convert_me_uint32(const uint32_t *me32)
{
    union {
        uint32_t u32;
        uint16_t u16[2];
    } uni;

    const uint16_t *regs = reinterpret_cast<const uint16_t *>(me32);
    uni.u16[0] = regs[1];
    uni.u16[1] = regs[0];

    return uni.u32;
}

static inline uint64_t convert_me_uint64(const uint64_t *me64)
{
    union {
        uint64_t u64;
        uint16_t u16[4];
    } uni;

    const uint16_t *regs = reinterpret_cast<const uint16_t *>(me64);
    uni.u16[0] = regs[3];
    uni.u16[1] = regs[2];
    uni.u16[2] = regs[1];
    uni.u16[3] = regs[0];

    return uni.u64;
}

static inline float convert_me_float(const uint32_t *me32)
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
    if (block0->ID !=   1) return false;
    if (block0->L  !=  65) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_001_data = {
    1, // model_id
    65, // model_length
    0, // interesting_registers_count
    false, // is_meter
    false, // read_twice
    &model_001_validator,
    0,  // value_count
    {    // value_data
    }
};

// ==============
// 101 - Inverter
// ==============

#include "model_101.h"

static float get_model_101_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->A;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_101_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->AphA;
    uint16_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0 ? UINT16_MAX : 0x8000u;
    if (val == not_implemented_val) return NAN;
    float fval;
    if ((quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0) {
        fval = static_cast<float>(val);
    } else {
        int16_t sval = static_cast<int16_t>(val);
        fval = static_cast<float>(sval);
    }
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_101_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->AphB;
    uint16_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0 ? UINT16_MAX : 0x8000u;
    if (val == not_implemented_val) return NAN;
    float fval;
    if ((quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0) {
        fval = static_cast<float>(val);
    } else {
        int16_t sval = static_cast<int16_t>(val);
        fval = static_cast<float>(sval);
    }
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_101_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->AphC;
    uint16_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0 ? UINT16_MAX : 0x8000u;
    if (val == not_implemented_val) return NAN;
    float fval;
    if ((quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0) {
        fval = static_cast<float>(val);
    } else {
        int16_t sval = static_cast<int16_t>(val);
        fval = static_cast<float>(sval);
    }
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_101_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PPVphAB;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_101_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PPVphBC;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_101_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PPVphCA;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_101_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PhVphA;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_101_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PhVphB;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_101_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->PhVphC;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_101_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->W_SF) * -1.0f);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_101_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->Hz;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Hz_SF);
    return fval;
}

static float get_model_101_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_101_VAr(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->VAr;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAr_SF);
    return fval;
}

static float get_model_101_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_INVERTER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_101_WH(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->WH);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->WH_SF) * 0.001f);
    return fval;
}

static float get_model_101_DCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->DCA;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCA_SF);
    return fval;
}

static float get_model_101_DCV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    uint16_t val = model->DCV;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCV_SF);
    return fval;
}

static float get_model_101_DCW(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->DCW;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCW_SF);
    return fval;
}

static float get_model_101_TmpCab(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->TmpCab;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_101_TmpSnk(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->TmpSnk;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_101_TmpTrns(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->TmpTrns;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_101_TmpOt(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel101_s *model = static_cast<const struct SunSpecInverterModel101_s *>(register_data);
    int16_t val = model->TmpOt;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
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
    50, // model_length
    38, // interesting_registers_count
    false, // is_meter
    true, // read_twice
    &model_101_validator,
    23,  // value_count
    {    // value_data
        { &get_model_101_A, MeterValueID::CurrentLSumExport, 6 },
        { &get_model_101_AphA, MeterValueID::CurrentL1Export, 6 },
        { &get_model_101_AphB, MeterValueID::CurrentL2Export, 6 },
        { &get_model_101_AphC, MeterValueID::CurrentL3Export, 6 },
        { &get_model_101_PPVphAB, MeterValueID::VoltageL1L2, 13 },
        { &get_model_101_PPVphBC, MeterValueID::VoltageL2L3, 13 },
        { &get_model_101_PPVphCA, MeterValueID::VoltageL3L1, 13 },
        { &get_model_101_PhVphA, MeterValueID::VoltageL1N, 13 },
        { &get_model_101_PhVphB, MeterValueID::VoltageL2N, 13 },
        { &get_model_101_PhVphC, MeterValueID::VoltageL3N, 13 },
        { &get_model_101_W, MeterValueID::PowerActiveLSumImExDiff, 15 },
        { &get_model_101_Hz, MeterValueID::FrequencyLAvg, 17 },
        { &get_model_101_VA, MeterValueID::PowerApparentLSumImExDiff, 19 },
        { &get_model_101_VAr, MeterValueID::PowerReactiveLSumIndCapDiff, 21 },
        { &get_model_101_PF, MeterValueID::PowerFactorLSumDirectional, 23 },
        { &get_model_101_WH, MeterValueID::EnergyActiveLSumExport, 26 },
        { &get_model_101_DCA, MeterValueID::CurrentDC, 28 },
        { &get_model_101_DCV, MeterValueID::VoltageDC, 30 },
        { &get_model_101_DCW, MeterValueID::PowerDC, 32 },
        { &get_model_101_TmpCab, MeterValueID::TemperatureCabinet, 37 },
        { &get_model_101_TmpSnk, MeterValueID::TemperatureHeatSink, 37 },
        { &get_model_101_TmpTrns, MeterValueID::TemperatureTransformer, 37 },
        { &get_model_101_TmpOt, MeterValueID::Temperature, 37 },
    }
};

// ==============
// 102 - Inverter
// ==============

#include "model_102.h"

static float get_model_102_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->A;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_102_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->AphA;
    uint16_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0 ? UINT16_MAX : 0x8000u;
    if (val == not_implemented_val) return NAN;
    float fval;
    if ((quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0) {
        fval = static_cast<float>(val);
    } else {
        int16_t sval = static_cast<int16_t>(val);
        fval = static_cast<float>(sval);
    }
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_102_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->AphB;
    uint16_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0 ? UINT16_MAX : 0x8000u;
    if (val == not_implemented_val) return NAN;
    float fval;
    if ((quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0) {
        fval = static_cast<float>(val);
    } else {
        int16_t sval = static_cast<int16_t>(val);
        fval = static_cast<float>(sval);
    }
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_102_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->AphC;
    uint16_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0 ? UINT16_MAX : 0x8000u;
    if (val == not_implemented_val) return NAN;
    float fval;
    if ((quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0) {
        fval = static_cast<float>(val);
    } else {
        int16_t sval = static_cast<int16_t>(val);
        fval = static_cast<float>(sval);
    }
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_102_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PPVphAB;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_102_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PPVphBC;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_102_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PPVphCA;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_102_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PhVphA;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_102_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PhVphB;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_102_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->PhVphC;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_102_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->W_SF) * -1.0f);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_102_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->Hz;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Hz_SF);
    return fval;
}

static float get_model_102_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_102_VAr(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->VAr;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAr_SF);
    return fval;
}

static float get_model_102_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_INVERTER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_102_WH(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->WH);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->WH_SF) * 0.001f);
    return fval;
}

static float get_model_102_DCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->DCA;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCA_SF);
    return fval;
}

static float get_model_102_DCV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    uint16_t val = model->DCV;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCV_SF);
    return fval;
}

static float get_model_102_DCW(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->DCW;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCW_SF);
    return fval;
}

static float get_model_102_TmpCab(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->TmpCab;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_102_TmpSnk(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->TmpSnk;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_102_TmpTrns(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->TmpTrns;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_102_TmpOt(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel102_s *model = static_cast<const struct SunSpecInverterModel102_s *>(register_data);
    int16_t val = model->TmpOt;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
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
    50, // model_length
    38, // interesting_registers_count
    false, // is_meter
    true, // read_twice
    &model_102_validator,
    23,  // value_count
    {    // value_data
        { &get_model_102_A, MeterValueID::CurrentLSumExport, 6 },
        { &get_model_102_AphA, MeterValueID::CurrentL1Export, 6 },
        { &get_model_102_AphB, MeterValueID::CurrentL2Export, 6 },
        { &get_model_102_AphC, MeterValueID::CurrentL3Export, 6 },
        { &get_model_102_PPVphAB, MeterValueID::VoltageL1L2, 13 },
        { &get_model_102_PPVphBC, MeterValueID::VoltageL2L3, 13 },
        { &get_model_102_PPVphCA, MeterValueID::VoltageL3L1, 13 },
        { &get_model_102_PhVphA, MeterValueID::VoltageL1N, 13 },
        { &get_model_102_PhVphB, MeterValueID::VoltageL2N, 13 },
        { &get_model_102_PhVphC, MeterValueID::VoltageL3N, 13 },
        { &get_model_102_W, MeterValueID::PowerActiveLSumImExDiff, 15 },
        { &get_model_102_Hz, MeterValueID::FrequencyLAvg, 17 },
        { &get_model_102_VA, MeterValueID::PowerApparentLSumImExDiff, 19 },
        { &get_model_102_VAr, MeterValueID::PowerReactiveLSumIndCapDiff, 21 },
        { &get_model_102_PF, MeterValueID::PowerFactorLSumDirectional, 23 },
        { &get_model_102_WH, MeterValueID::EnergyActiveLSumExport, 26 },
        { &get_model_102_DCA, MeterValueID::CurrentDC, 28 },
        { &get_model_102_DCV, MeterValueID::VoltageDC, 30 },
        { &get_model_102_DCW, MeterValueID::PowerDC, 32 },
        { &get_model_102_TmpCab, MeterValueID::TemperatureCabinet, 37 },
        { &get_model_102_TmpSnk, MeterValueID::TemperatureHeatSink, 37 },
        { &get_model_102_TmpTrns, MeterValueID::TemperatureTransformer, 37 },
        { &get_model_102_TmpOt, MeterValueID::Temperature, 37 },
    }
};

// ==============
// 103 - Inverter
// ==============

#include "model_103.h"

static float get_model_103_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->A;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_103_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->AphA;
    uint16_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0 ? UINT16_MAX : 0x8000u;
    if (val == not_implemented_val) return NAN;
    float fval;
    if ((quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0) {
        fval = static_cast<float>(val);
    } else {
        int16_t sval = static_cast<int16_t>(val);
        fval = static_cast<float>(sval);
    }
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_103_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->AphB;
    uint16_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0 ? UINT16_MAX : 0x8000u;
    if (val == not_implemented_val) return NAN;
    float fval;
    if ((quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0) {
        fval = static_cast<float>(val);
    } else {
        int16_t sval = static_cast<int16_t>(val);
        fval = static_cast<float>(sval);
    }
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_103_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->AphC;
    uint16_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0 ? UINT16_MAX : 0x8000u;
    if (val == not_implemented_val) return NAN;
    float fval;
    if ((quirks & SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16) == 0) {
        fval = static_cast<float>(val);
    } else {
        int16_t sval = static_cast<int16_t>(val);
        fval = static_cast<float>(sval);
    }
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_103_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PPVphAB;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_103_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PPVphBC;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_103_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PPVphCA;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_103_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PhVphA;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_103_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PhVphB;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_103_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->PhVphC;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_103_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->W_SF) * -1.0f);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_103_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->Hz;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Hz_SF);
    return fval;
}

static float get_model_103_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_103_VAr(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->VAr;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAr_SF);
    return fval;
}

static float get_model_103_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_INVERTER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_103_WH(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->WH);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->WH_SF) * 0.001f);
    return fval;
}

static float get_model_103_DCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->DCA;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCA_SF);
    return fval;
}

static float get_model_103_DCV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    uint16_t val = model->DCV;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCV_SF);
    return fval;
}

static float get_model_103_DCW(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->DCW;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCW_SF);
    return fval;
}

static float get_model_103_TmpCab(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->TmpCab;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_103_TmpSnk(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->TmpSnk;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_103_TmpTrns(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->TmpTrns;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_103_TmpOt(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterModel103_s *model = static_cast<const struct SunSpecInverterModel103_s *>(register_data);
    int16_t val = model->TmpOt;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
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
    50, // model_length
    38, // interesting_registers_count
    false, // is_meter
    true, // read_twice
    &model_103_validator,
    23,  // value_count
    {    // value_data
        { &get_model_103_A, MeterValueID::CurrentLSumExport, 6 },
        { &get_model_103_AphA, MeterValueID::CurrentL1Export, 6 },
        { &get_model_103_AphB, MeterValueID::CurrentL2Export, 6 },
        { &get_model_103_AphC, MeterValueID::CurrentL3Export, 6 },
        { &get_model_103_PPVphAB, MeterValueID::VoltageL1L2, 13 },
        { &get_model_103_PPVphBC, MeterValueID::VoltageL2L3, 13 },
        { &get_model_103_PPVphCA, MeterValueID::VoltageL3L1, 13 },
        { &get_model_103_PhVphA, MeterValueID::VoltageL1N, 13 },
        { &get_model_103_PhVphB, MeterValueID::VoltageL2N, 13 },
        { &get_model_103_PhVphC, MeterValueID::VoltageL3N, 13 },
        { &get_model_103_W, MeterValueID::PowerActiveLSumImExDiff, 15 },
        { &get_model_103_Hz, MeterValueID::FrequencyLAvg, 17 },
        { &get_model_103_VA, MeterValueID::PowerApparentLSumImExDiff, 19 },
        { &get_model_103_VAr, MeterValueID::PowerReactiveLSumIndCapDiff, 21 },
        { &get_model_103_PF, MeterValueID::PowerFactorLSumDirectional, 23 },
        { &get_model_103_WH, MeterValueID::EnergyActiveLSumExport, 26 },
        { &get_model_103_DCA, MeterValueID::CurrentDC, 28 },
        { &get_model_103_DCV, MeterValueID::VoltageDC, 30 },
        { &get_model_103_DCW, MeterValueID::PowerDC, 32 },
        { &get_model_103_TmpCab, MeterValueID::TemperatureCabinet, 37 },
        { &get_model_103_TmpSnk, MeterValueID::TemperatureHeatSink, 37 },
        { &get_model_103_TmpTrns, MeterValueID::TemperatureTransformer, 37 },
        { &get_model_103_TmpOt, MeterValueID::Temperature, 37 },
    }
};

// ====================
// 111 - Inverter FLOAT
// ====================

#include "model_111.h"

static float get_model_111_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->A);
    float fval = val;
    return fval;
}

static float get_model_111_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->AphA);
    float fval = val;
    return fval;
}

static float get_model_111_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->AphB);
    float fval = val;
    return fval;
}

static float get_model_111_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->AphC);
    float fval = val;
    return fval;
}

static float get_model_111_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->PPVphAB);
    float fval = val;
    return fval;
}

static float get_model_111_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->PPVphBC);
    float fval = val;
    return fval;
}

static float get_model_111_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->PPVphCA);
    float fval = val;
    return fval;
}

static float get_model_111_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->PhVphA);
    float fval = val;
    return fval;
}

static float get_model_111_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->PhVphB);
    float fval = val;
    return fval;
}

static float get_model_111_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->PhVphC);
    float fval = val;
    return fval;
}

static float get_model_111_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->W);
    float fval = val;
    fval *= -1.0f;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_111_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->Hz);
    float fval = val;
    return fval;
}

static float get_model_111_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->VA);
    float fval = val;
    return fval;
}

static float get_model_111_VAr(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->VAr);
    float fval = val;
    return fval;
}

static float get_model_111_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->PF);
    float fval = val;
    fval *= 0.01f;
    return fval;
}

static float get_model_111_WH(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->WH);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_111_DCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->DCA);
    float fval = val;
    return fval;
}

static float get_model_111_DCV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->DCV);
    float fval = val;
    return fval;
}

static float get_model_111_DCW(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->DCW);
    float fval = val;
    return fval;
}

static float get_model_111_TmpCab(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->TmpCab);
    float fval = val;
    return fval;
}

static float get_model_111_TmpSnk(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->TmpSnk);
    float fval = val;
    return fval;
}

static float get_model_111_TmpTrns(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->TmpTrns);
    float fval = val;
    return fval;
}

static float get_model_111_TmpOt(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel111_s *model = static_cast<const struct SunSpecInverterFLOATModel111_s *>(register_data);
    float val = convert_me_float(&model->TmpOt);
    float fval = val;
    return fval;
}

static bool model_111_validator(const uint16_t * const register_data[2])
{
    const SunSpecInverterFLOATModel111_s *block0 = reinterpret_cast<const SunSpecInverterFLOATModel111_s *>(register_data[0]);
    if (block0->ID != 111) return false;
    if (block0->L  !=  60) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_111_data = {
    111, // model_id
    60, // model_length
    48, // interesting_registers_count
    false, // is_meter
    false, // read_twice
    &model_111_validator,
    23,  // value_count
    {    // value_data
        { &get_model_111_A, MeterValueID::CurrentLSumExport, 3 },
        { &get_model_111_AphA, MeterValueID::CurrentL1Export, 5 },
        { &get_model_111_AphB, MeterValueID::CurrentL2Export, 7 },
        { &get_model_111_AphC, MeterValueID::CurrentL3Export, 9 },
        { &get_model_111_PPVphAB, MeterValueID::VoltageL1L2, 11 },
        { &get_model_111_PPVphBC, MeterValueID::VoltageL2L3, 13 },
        { &get_model_111_PPVphCA, MeterValueID::VoltageL3L1, 15 },
        { &get_model_111_PhVphA, MeterValueID::VoltageL1N, 17 },
        { &get_model_111_PhVphB, MeterValueID::VoltageL2N, 19 },
        { &get_model_111_PhVphC, MeterValueID::VoltageL3N, 21 },
        { &get_model_111_W, MeterValueID::PowerActiveLSumImExDiff, 23 },
        { &get_model_111_Hz, MeterValueID::FrequencyLAvg, 25 },
        { &get_model_111_VA, MeterValueID::PowerApparentLSumImExDiff, 27 },
        { &get_model_111_VAr, MeterValueID::PowerReactiveLSumIndCapDiff, 29 },
        { &get_model_111_PF, MeterValueID::PowerFactorLSumDirectional, 31 },
        { &get_model_111_WH, MeterValueID::EnergyActiveLSumExport, 33 },
        { &get_model_111_DCA, MeterValueID::CurrentDC, 35 },
        { &get_model_111_DCV, MeterValueID::VoltageDC, 37 },
        { &get_model_111_DCW, MeterValueID::PowerDC, 39 },
        { &get_model_111_TmpCab, MeterValueID::TemperatureCabinet, 41 },
        { &get_model_111_TmpSnk, MeterValueID::TemperatureHeatSink, 43 },
        { &get_model_111_TmpTrns, MeterValueID::TemperatureTransformer, 45 },
        { &get_model_111_TmpOt, MeterValueID::Temperature, 47 },
    }
};

// ====================
// 112 - Inverter FLOAT
// ====================

#include "model_112.h"

static float get_model_112_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->A);
    float fval = val;
    return fval;
}

static float get_model_112_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->AphA);
    float fval = val;
    return fval;
}

static float get_model_112_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->AphB);
    float fval = val;
    return fval;
}

static float get_model_112_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->AphC);
    float fval = val;
    return fval;
}

static float get_model_112_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->PPVphAB);
    float fval = val;
    return fval;
}

static float get_model_112_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->PPVphBC);
    float fval = val;
    return fval;
}

static float get_model_112_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->PPVphCA);
    float fval = val;
    return fval;
}

static float get_model_112_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->PhVphA);
    float fval = val;
    return fval;
}

static float get_model_112_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->PhVphB);
    float fval = val;
    return fval;
}

static float get_model_112_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->PhVphC);
    float fval = val;
    return fval;
}

static float get_model_112_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->W);
    float fval = val;
    fval *= -1.0f;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_112_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->Hz);
    float fval = val;
    return fval;
}

static float get_model_112_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->VA);
    float fval = val;
    return fval;
}

static float get_model_112_VAr(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->VAr);
    float fval = val;
    return fval;
}

static float get_model_112_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->PF);
    float fval = val;
    fval *= 0.01f;
    return fval;
}

static float get_model_112_WH(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->WH);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_112_DCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->DCA);
    float fval = val;
    return fval;
}

static float get_model_112_DCV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->DCV);
    float fval = val;
    return fval;
}

static float get_model_112_DCW(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->DCW);
    float fval = val;
    return fval;
}

static float get_model_112_TmpCab(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->TmpCab);
    float fval = val;
    return fval;
}

static float get_model_112_TmpSnk(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->TmpSnk);
    float fval = val;
    return fval;
}

static float get_model_112_TmpTrns(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->TmpTrns);
    float fval = val;
    return fval;
}

static float get_model_112_TmpOt(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel112_s *model = static_cast<const struct SunSpecInverterFLOATModel112_s *>(register_data);
    float val = convert_me_float(&model->TmpOt);
    float fval = val;
    return fval;
}

static bool model_112_validator(const uint16_t * const register_data[2])
{
    const SunSpecInverterFLOATModel112_s *block0 = reinterpret_cast<const SunSpecInverterFLOATModel112_s *>(register_data[0]);
    if (block0->ID != 112) return false;
    if (block0->L  !=  60) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_112_data = {
    112, // model_id
    60, // model_length
    48, // interesting_registers_count
    false, // is_meter
    false, // read_twice
    &model_112_validator,
    23,  // value_count
    {    // value_data
        { &get_model_112_A, MeterValueID::CurrentLSumExport, 3 },
        { &get_model_112_AphA, MeterValueID::CurrentL1Export, 5 },
        { &get_model_112_AphB, MeterValueID::CurrentL2Export, 7 },
        { &get_model_112_AphC, MeterValueID::CurrentL3Export, 9 },
        { &get_model_112_PPVphAB, MeterValueID::VoltageL1L2, 11 },
        { &get_model_112_PPVphBC, MeterValueID::VoltageL2L3, 13 },
        { &get_model_112_PPVphCA, MeterValueID::VoltageL3L1, 15 },
        { &get_model_112_PhVphA, MeterValueID::VoltageL1N, 17 },
        { &get_model_112_PhVphB, MeterValueID::VoltageL2N, 19 },
        { &get_model_112_PhVphC, MeterValueID::VoltageL3N, 21 },
        { &get_model_112_W, MeterValueID::PowerActiveLSumImExDiff, 23 },
        { &get_model_112_Hz, MeterValueID::FrequencyLAvg, 25 },
        { &get_model_112_VA, MeterValueID::PowerApparentLSumImExDiff, 27 },
        { &get_model_112_VAr, MeterValueID::PowerReactiveLSumIndCapDiff, 29 },
        { &get_model_112_PF, MeterValueID::PowerFactorLSumDirectional, 31 },
        { &get_model_112_WH, MeterValueID::EnergyActiveLSumExport, 33 },
        { &get_model_112_DCA, MeterValueID::CurrentDC, 35 },
        { &get_model_112_DCV, MeterValueID::VoltageDC, 37 },
        { &get_model_112_DCW, MeterValueID::PowerDC, 39 },
        { &get_model_112_TmpCab, MeterValueID::TemperatureCabinet, 41 },
        { &get_model_112_TmpSnk, MeterValueID::TemperatureHeatSink, 43 },
        { &get_model_112_TmpTrns, MeterValueID::TemperatureTransformer, 45 },
        { &get_model_112_TmpOt, MeterValueID::Temperature, 47 },
    }
};

// ====================
// 113 - Inverter FLOAT
// ====================

#include "model_113.h"

static float get_model_113_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->A);
    float fval = val;
    return fval;
}

static float get_model_113_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->AphA);
    float fval = val;
    return fval;
}

static float get_model_113_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->AphB);
    float fval = val;
    return fval;
}

static float get_model_113_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->AphC);
    float fval = val;
    return fval;
}

static float get_model_113_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->PPVphAB);
    float fval = val;
    return fval;
}

static float get_model_113_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->PPVphBC);
    float fval = val;
    return fval;
}

static float get_model_113_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->PPVphCA);
    float fval = val;
    return fval;
}

static float get_model_113_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->PhVphA);
    float fval = val;
    return fval;
}

static float get_model_113_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->PhVphB);
    float fval = val;
    return fval;
}

static float get_model_113_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->PhVphC);
    float fval = val;
    return fval;
}

static float get_model_113_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->W);
    float fval = val;
    fval *= -1.0f;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_113_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->Hz);
    float fval = val;
    return fval;
}

static float get_model_113_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->VA);
    float fval = val;
    return fval;
}

static float get_model_113_VAr(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->VAr);
    float fval = val;
    return fval;
}

static float get_model_113_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->PF);
    float fval = val;
    fval *= 0.01f;
    return fval;
}

static float get_model_113_WH(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->WH);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_113_DCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->DCA);
    float fval = val;
    return fval;
}

static float get_model_113_DCV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->DCV);
    float fval = val;
    return fval;
}

static float get_model_113_DCW(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->DCW);
    float fval = val;
    return fval;
}

static float get_model_113_TmpCab(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->TmpCab);
    float fval = val;
    return fval;
}

static float get_model_113_TmpSnk(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->TmpSnk);
    float fval = val;
    return fval;
}

static float get_model_113_TmpTrns(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->TmpTrns);
    float fval = val;
    return fval;
}

static float get_model_113_TmpOt(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecInverterFLOATModel113_s *model = static_cast<const struct SunSpecInverterFLOATModel113_s *>(register_data);
    float val = convert_me_float(&model->TmpOt);
    float fval = val;
    return fval;
}

static bool model_113_validator(const uint16_t * const register_data[2])
{
    const SunSpecInverterFLOATModel113_s *block0 = reinterpret_cast<const SunSpecInverterFLOATModel113_s *>(register_data[0]);
    if (block0->ID != 113) return false;
    if (block0->L  !=  60) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_113_data = {
    113, // model_id
    60, // model_length
    48, // interesting_registers_count
    false, // is_meter
    false, // read_twice
    &model_113_validator,
    23,  // value_count
    {    // value_data
        { &get_model_113_A, MeterValueID::CurrentLSumExport, 3 },
        { &get_model_113_AphA, MeterValueID::CurrentL1Export, 5 },
        { &get_model_113_AphB, MeterValueID::CurrentL2Export, 7 },
        { &get_model_113_AphC, MeterValueID::CurrentL3Export, 9 },
        { &get_model_113_PPVphAB, MeterValueID::VoltageL1L2, 11 },
        { &get_model_113_PPVphBC, MeterValueID::VoltageL2L3, 13 },
        { &get_model_113_PPVphCA, MeterValueID::VoltageL3L1, 15 },
        { &get_model_113_PhVphA, MeterValueID::VoltageL1N, 17 },
        { &get_model_113_PhVphB, MeterValueID::VoltageL2N, 19 },
        { &get_model_113_PhVphC, MeterValueID::VoltageL3N, 21 },
        { &get_model_113_W, MeterValueID::PowerActiveLSumImExDiff, 23 },
        { &get_model_113_Hz, MeterValueID::FrequencyLAvg, 25 },
        { &get_model_113_VA, MeterValueID::PowerApparentLSumImExDiff, 27 },
        { &get_model_113_VAr, MeterValueID::PowerReactiveLSumIndCapDiff, 29 },
        { &get_model_113_PF, MeterValueID::PowerFactorLSumDirectional, 31 },
        { &get_model_113_WH, MeterValueID::EnergyActiveLSumExport, 33 },
        { &get_model_113_DCA, MeterValueID::CurrentDC, 35 },
        { &get_model_113_DCV, MeterValueID::VoltageDC, 37 },
        { &get_model_113_DCW, MeterValueID::PowerDC, 39 },
        { &get_model_113_TmpCab, MeterValueID::TemperatureCabinet, 41 },
        { &get_model_113_TmpSnk, MeterValueID::TemperatureHeatSink, 43 },
        { &get_model_113_TmpTrns, MeterValueID::TemperatureTransformer, 45 },
        { &get_model_113_TmpOt, MeterValueID::Temperature, 47 },
    }
};

// ========================
// 201 - Single Phase Meter
// ========================

#include "model_201.h"

static float get_model_201_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->A;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_201_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->AphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_201_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->AphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_201_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->AphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_201_PhV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PhV;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_201_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PhVphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_201_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PhVphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_201_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PhVphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_201_PPV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PPV;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_201_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PPVphAB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_201_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PPVphBC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_201_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PPVphCA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_201_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->Hz;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Hz_SF);
    return fval;
}

static float get_model_201_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_201_WphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->WphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_201_WphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->WphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_201_WphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->WphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_201_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_201_VAphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VAphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_201_VAphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VAphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_201_VAphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VAphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_201_VAR(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VAR;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_201_VARphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VARphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_201_VARphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VARphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_201_VARphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->VARphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_201_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_201_PFphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PFphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_201_PFphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PFphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_201_PFphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    int16_t val = model->PFphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_201_TotWhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExp);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotWhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotWhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotWhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotWhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImp);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotWhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotWhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotWhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVAhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExp);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVAhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVAhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVAhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVAhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImp);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVAhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVAhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVAhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhImpQ1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhImpQ1PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhImpQ1PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhImpQ1PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhImpQ2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhImpQ2PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhImpQ2PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhImpQ2PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhExpQ3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhExpQ3PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhExpQ3PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhExpQ3PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhExpQ4(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhExpQ4PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhExpQ4PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_201_TotVArhExpQ4PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel201_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel201_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
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
    105, // model_length
    105, // interesting_registers_count
    true, // is_meter
    true, // read_twice
    &model_201_validator,
    61,  // value_count
    {    // value_data
        { &get_model_201_A, MeterValueID::CurrentLSumImExSum, 6 },
        { &get_model_201_AphA, MeterValueID::CurrentL1ImExSum, 6 },
        { &get_model_201_AphB, MeterValueID::CurrentL2ImExSum, 6 },
        { &get_model_201_AphC, MeterValueID::CurrentL3ImExSum, 6 },
        { &get_model_201_PhV, MeterValueID::VoltageLNAvg, 15 },
        { &get_model_201_PhVphA, MeterValueID::VoltageL1N, 15 },
        { &get_model_201_PhVphB, MeterValueID::VoltageL2N, 15 },
        { &get_model_201_PhVphC, MeterValueID::VoltageL3N, 15 },
        { &get_model_201_PPV, MeterValueID::VoltageLLAvg, 15 },
        { &get_model_201_PPVphAB, MeterValueID::VoltageL1L2, 15 },
        { &get_model_201_PPVphBC, MeterValueID::VoltageL2L3, 15 },
        { &get_model_201_PPVphCA, MeterValueID::VoltageL3L1, 15 },
        { &get_model_201_Hz, MeterValueID::FrequencyLAvg, 17 },
        { &get_model_201_W, MeterValueID::PowerActiveLSumImExDiff, 22 },
        { &get_model_201_WphA, MeterValueID::PowerActiveL1ImExDiff, 22 },
        { &get_model_201_WphB, MeterValueID::PowerActiveL2ImExDiff, 22 },
        { &get_model_201_WphC, MeterValueID::PowerActiveL3ImExDiff, 22 },
        { &get_model_201_VA, MeterValueID::PowerApparentLSumImExDiff, 27 },
        { &get_model_201_VAphA, MeterValueID::PowerApparentL1ImExDiff, 27 },
        { &get_model_201_VAphB, MeterValueID::PowerApparentL2ImExDiff, 27 },
        { &get_model_201_VAphC, MeterValueID::PowerApparentL3ImExDiff, 27 },
        { &get_model_201_VAR, MeterValueID::PowerReactiveLSumIndCapDiff, 32 },
        { &get_model_201_VARphA, MeterValueID::PowerReactiveL1IndCapDiff, 32 },
        { &get_model_201_VARphB, MeterValueID::PowerReactiveL2IndCapDiff, 32 },
        { &get_model_201_VARphC, MeterValueID::PowerReactiveL3IndCapDiff, 32 },
        { &get_model_201_PF, MeterValueID::PowerFactorLSumDirectional, 37 },
        { &get_model_201_PFphA, MeterValueID::PowerFactorL1Directional, 37 },
        { &get_model_201_PFphB, MeterValueID::PowerFactorL2Directional, 37 },
        { &get_model_201_PFphC, MeterValueID::PowerFactorL3Directional, 37 },
        { &get_model_201_TotWhExp, MeterValueID::EnergyActiveLSumExport, 54 },
        { &get_model_201_TotWhExpPhA, MeterValueID::EnergyActiveL1Export, 54 },
        { &get_model_201_TotWhExpPhB, MeterValueID::EnergyActiveL2Export, 54 },
        { &get_model_201_TotWhExpPhC, MeterValueID::EnergyActiveL3Export, 54 },
        { &get_model_201_TotWhImp, MeterValueID::EnergyActiveLSumImport, 54 },
        { &get_model_201_TotWhImpPhA, MeterValueID::EnergyActiveL1Import, 54 },
        { &get_model_201_TotWhImpPhB, MeterValueID::EnergyActiveL2Import, 54 },
        { &get_model_201_TotWhImpPhC, MeterValueID::EnergyActiveL3Import, 54 },
        { &get_model_201_TotVAhExp, MeterValueID::EnergyApparentLSumExport, 71 },
        { &get_model_201_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export, 71 },
        { &get_model_201_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export, 71 },
        { &get_model_201_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export, 71 },
        { &get_model_201_TotVAhImp, MeterValueID::EnergyApparentLSumImport, 71 },
        { &get_model_201_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import, 71 },
        { &get_model_201_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import, 71 },
        { &get_model_201_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import, 71 },
        { &get_model_201_TotVArhImpQ1, MeterValueID::EnergyReactiveLSumQ1, 104 },
        { &get_model_201_TotVArhImpQ1PhA, MeterValueID::EnergyReactiveL1Q1, 104 },
        { &get_model_201_TotVArhImpQ1PhB, MeterValueID::EnergyReactiveL2Q1, 104 },
        { &get_model_201_TotVArhImpQ1PhC, MeterValueID::EnergyReactiveL3Q1, 104 },
        { &get_model_201_TotVArhImpQ2, MeterValueID::EnergyReactiveLSumQ2, 104 },
        { &get_model_201_TotVArhImpQ2PhA, MeterValueID::EnergyReactiveL1Q2, 104 },
        { &get_model_201_TotVArhImpQ2PhB, MeterValueID::EnergyReactiveL2Q2, 104 },
        { &get_model_201_TotVArhImpQ2PhC, MeterValueID::EnergyReactiveL3Q2, 104 },
        { &get_model_201_TotVArhExpQ3, MeterValueID::EnergyReactiveLSumQ3, 104 },
        { &get_model_201_TotVArhExpQ3PhA, MeterValueID::EnergyReactiveL1Q3, 104 },
        { &get_model_201_TotVArhExpQ3PhB, MeterValueID::EnergyReactiveL2Q3, 104 },
        { &get_model_201_TotVArhExpQ3PhC, MeterValueID::EnergyReactiveL3Q3, 104 },
        { &get_model_201_TotVArhExpQ4, MeterValueID::EnergyReactiveLSumQ4, 104 },
        { &get_model_201_TotVArhExpQ4PhA, MeterValueID::EnergyReactiveL1Q4, 104 },
        { &get_model_201_TotVArhExpQ4PhB, MeterValueID::EnergyReactiveL2Q4, 104 },
        { &get_model_201_TotVArhExpQ4PhC, MeterValueID::EnergyReactiveL3Q4, 104 },
    }
};

// ==============================
// 202 - Split Single Phase Meter
// ==============================

#include "model_202.h"

static float get_model_202_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->A;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_202_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->AphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_202_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->AphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_202_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->AphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_202_PhV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhV;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_202_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_202_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_202_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_202_PPV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PPV;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_202_PhVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphAB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_202_PhVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphBC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_202_PhVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PhVphCA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_202_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->Hz;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Hz_SF);
    return fval;
}

static float get_model_202_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_202_WphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->WphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_202_WphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->WphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_202_WphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->WphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_202_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_202_VAphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VAphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_202_VAphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VAphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_202_VAphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VAphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_202_VAR(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VAR;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_202_VARphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VARphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_202_VARphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VARphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_202_VARphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->VARphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_202_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_202_PFphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PFphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_202_PFphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PFphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_202_PFphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    int16_t val = model->PFphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_202_TotWhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExp);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotWhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotWhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotWhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotWhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImp);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotWhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotWhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotWhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVAhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExp);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVAhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVAhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVAhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVAhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImp);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVAhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVAhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVAhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhImpQ1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhImpQ1PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhImpQ1PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhImpQ1PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhImpQ2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhImpQ2PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhImpQ2PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhImpQ2PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhExpQ3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhExpQ3PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhExpQ3PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhExpQ3PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhExpQ4(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhExpQ4PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhExpQ4PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_202_TotVArhExpQ4PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel202_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel202_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
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
    105, // model_length
    105, // interesting_registers_count
    true, // is_meter
    true, // read_twice
    &model_202_validator,
    61,  // value_count
    {    // value_data
        { &get_model_202_A, MeterValueID::CurrentLSumImExSum, 6 },
        { &get_model_202_AphA, MeterValueID::CurrentL1ImExSum, 6 },
        { &get_model_202_AphB, MeterValueID::CurrentL2ImExSum, 6 },
        { &get_model_202_AphC, MeterValueID::CurrentL3ImExSum, 6 },
        { &get_model_202_PhV, MeterValueID::VoltageLNAvg, 15 },
        { &get_model_202_PhVphA, MeterValueID::VoltageL1N, 15 },
        { &get_model_202_PhVphB, MeterValueID::VoltageL2N, 15 },
        { &get_model_202_PhVphC, MeterValueID::VoltageL3N, 15 },
        { &get_model_202_PPV, MeterValueID::VoltageLLAvg, 15 },
        { &get_model_202_PhVphAB, MeterValueID::VoltageL1L2, 15 },
        { &get_model_202_PhVphBC, MeterValueID::VoltageL2L3, 15 },
        { &get_model_202_PhVphCA, MeterValueID::VoltageL3L1, 15 },
        { &get_model_202_Hz, MeterValueID::FrequencyLAvg, 17 },
        { &get_model_202_W, MeterValueID::PowerActiveLSumImExDiff, 22 },
        { &get_model_202_WphA, MeterValueID::PowerActiveL1ImExDiff, 22 },
        { &get_model_202_WphB, MeterValueID::PowerActiveL2ImExDiff, 22 },
        { &get_model_202_WphC, MeterValueID::PowerActiveL3ImExDiff, 22 },
        { &get_model_202_VA, MeterValueID::PowerApparentLSumImExDiff, 27 },
        { &get_model_202_VAphA, MeterValueID::PowerApparentL1ImExDiff, 27 },
        { &get_model_202_VAphB, MeterValueID::PowerApparentL2ImExDiff, 27 },
        { &get_model_202_VAphC, MeterValueID::PowerApparentL3ImExDiff, 27 },
        { &get_model_202_VAR, MeterValueID::PowerReactiveLSumIndCapDiff, 32 },
        { &get_model_202_VARphA, MeterValueID::PowerReactiveL1IndCapDiff, 32 },
        { &get_model_202_VARphB, MeterValueID::PowerReactiveL2IndCapDiff, 32 },
        { &get_model_202_VARphC, MeterValueID::PowerReactiveL3IndCapDiff, 32 },
        { &get_model_202_PF, MeterValueID::PowerFactorLSumDirectional, 37 },
        { &get_model_202_PFphA, MeterValueID::PowerFactorL1Directional, 37 },
        { &get_model_202_PFphB, MeterValueID::PowerFactorL2Directional, 37 },
        { &get_model_202_PFphC, MeterValueID::PowerFactorL3Directional, 37 },
        { &get_model_202_TotWhExp, MeterValueID::EnergyActiveLSumExport, 54 },
        { &get_model_202_TotWhExpPhA, MeterValueID::EnergyActiveL1Export, 54 },
        { &get_model_202_TotWhExpPhB, MeterValueID::EnergyActiveL2Export, 54 },
        { &get_model_202_TotWhExpPhC, MeterValueID::EnergyActiveL3Export, 54 },
        { &get_model_202_TotWhImp, MeterValueID::EnergyActiveLSumImport, 54 },
        { &get_model_202_TotWhImpPhA, MeterValueID::EnergyActiveL1Import, 54 },
        { &get_model_202_TotWhImpPhB, MeterValueID::EnergyActiveL2Import, 54 },
        { &get_model_202_TotWhImpPhC, MeterValueID::EnergyActiveL3Import, 54 },
        { &get_model_202_TotVAhExp, MeterValueID::EnergyApparentLSumExport, 71 },
        { &get_model_202_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export, 71 },
        { &get_model_202_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export, 71 },
        { &get_model_202_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export, 71 },
        { &get_model_202_TotVAhImp, MeterValueID::EnergyApparentLSumImport, 71 },
        { &get_model_202_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import, 71 },
        { &get_model_202_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import, 71 },
        { &get_model_202_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import, 71 },
        { &get_model_202_TotVArhImpQ1, MeterValueID::EnergyReactiveLSumQ1, 104 },
        { &get_model_202_TotVArhImpQ1PhA, MeterValueID::EnergyReactiveL1Q1, 104 },
        { &get_model_202_TotVArhImpQ1PhB, MeterValueID::EnergyReactiveL2Q1, 104 },
        { &get_model_202_TotVArhImpQ1PhC, MeterValueID::EnergyReactiveL3Q1, 104 },
        { &get_model_202_TotVArhImpQ2, MeterValueID::EnergyReactiveLSumQ2, 104 },
        { &get_model_202_TotVArhImpQ2PhA, MeterValueID::EnergyReactiveL1Q2, 104 },
        { &get_model_202_TotVArhImpQ2PhB, MeterValueID::EnergyReactiveL2Q2, 104 },
        { &get_model_202_TotVArhImpQ2PhC, MeterValueID::EnergyReactiveL3Q2, 104 },
        { &get_model_202_TotVArhExpQ3, MeterValueID::EnergyReactiveLSumQ3, 104 },
        { &get_model_202_TotVArhExpQ3PhA, MeterValueID::EnergyReactiveL1Q3, 104 },
        { &get_model_202_TotVArhExpQ3PhB, MeterValueID::EnergyReactiveL2Q3, 104 },
        { &get_model_202_TotVArhExpQ3PhC, MeterValueID::EnergyReactiveL3Q3, 104 },
        { &get_model_202_TotVArhExpQ4, MeterValueID::EnergyReactiveLSumQ4, 104 },
        { &get_model_202_TotVArhExpQ4PhA, MeterValueID::EnergyReactiveL1Q4, 104 },
        { &get_model_202_TotVArhExpQ4PhB, MeterValueID::EnergyReactiveL2Q4, 104 },
        { &get_model_202_TotVArhExpQ4PhC, MeterValueID::EnergyReactiveL3Q4, 104 },
    }
};

// ===================================
// 203 - Wye-Connect Three Phase Meter
// ===================================

#include "model_203.h"

static float get_model_203_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->A;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_203_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->AphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_203_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->AphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_203_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->AphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_203_PhV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhV;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_203_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_203_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_203_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_203_PPV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PPV;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_203_PhVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphAB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_203_PhVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphBC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_203_PhVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PhVphCA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_203_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->Hz;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Hz_SF);
    return fval;
}

static float get_model_203_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_203_WphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->WphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_203_WphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->WphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_203_WphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->WphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_203_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_203_VAphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VAphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_203_VAphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VAphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_203_VAphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VAphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_203_VAR(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VAR;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_203_VARphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VARphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_203_VARphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VARphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_203_VARphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->VARphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_203_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_203_PFphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PFphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_203_PFphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PFphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_203_PFphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    int16_t val = model->PFphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_203_TotWhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExp);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotWhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotWhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotWhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotWhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImp);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotWhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotWhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotWhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVAhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExp);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVAhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVAhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVAhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVAhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImp);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVAhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVAhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVAhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhImpQ1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhImpQ1PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhImpQ1PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhImpQ1PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhImpQ2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhImpQ2PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhImpQ2PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhImpQ2PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhExpQ3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhExpQ3PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhExpQ3PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhExpQ3PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhExpQ4(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhExpQ4PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhExpQ4PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_203_TotVArhExpQ4PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel203_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel203_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
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
    105, // model_length
    105, // interesting_registers_count
    true, // is_meter
    true, // read_twice
    &model_203_validator,
    61,  // value_count
    {    // value_data
        { &get_model_203_A, MeterValueID::CurrentLSumImExSum, 6 },
        { &get_model_203_AphA, MeterValueID::CurrentL1ImExSum, 6 },
        { &get_model_203_AphB, MeterValueID::CurrentL2ImExSum, 6 },
        { &get_model_203_AphC, MeterValueID::CurrentL3ImExSum, 6 },
        { &get_model_203_PhV, MeterValueID::VoltageLNAvg, 15 },
        { &get_model_203_PhVphA, MeterValueID::VoltageL1N, 15 },
        { &get_model_203_PhVphB, MeterValueID::VoltageL2N, 15 },
        { &get_model_203_PhVphC, MeterValueID::VoltageL3N, 15 },
        { &get_model_203_PPV, MeterValueID::VoltageLLAvg, 15 },
        { &get_model_203_PhVphAB, MeterValueID::VoltageL1L2, 15 },
        { &get_model_203_PhVphBC, MeterValueID::VoltageL2L3, 15 },
        { &get_model_203_PhVphCA, MeterValueID::VoltageL3L1, 15 },
        { &get_model_203_Hz, MeterValueID::FrequencyLAvg, 17 },
        { &get_model_203_W, MeterValueID::PowerActiveLSumImExDiff, 22 },
        { &get_model_203_WphA, MeterValueID::PowerActiveL1ImExDiff, 22 },
        { &get_model_203_WphB, MeterValueID::PowerActiveL2ImExDiff, 22 },
        { &get_model_203_WphC, MeterValueID::PowerActiveL3ImExDiff, 22 },
        { &get_model_203_VA, MeterValueID::PowerApparentLSumImExDiff, 27 },
        { &get_model_203_VAphA, MeterValueID::PowerApparentL1ImExDiff, 27 },
        { &get_model_203_VAphB, MeterValueID::PowerApparentL2ImExDiff, 27 },
        { &get_model_203_VAphC, MeterValueID::PowerApparentL3ImExDiff, 27 },
        { &get_model_203_VAR, MeterValueID::PowerReactiveLSumIndCapDiff, 32 },
        { &get_model_203_VARphA, MeterValueID::PowerReactiveL1IndCapDiff, 32 },
        { &get_model_203_VARphB, MeterValueID::PowerReactiveL2IndCapDiff, 32 },
        { &get_model_203_VARphC, MeterValueID::PowerReactiveL3IndCapDiff, 32 },
        { &get_model_203_PF, MeterValueID::PowerFactorLSumDirectional, 37 },
        { &get_model_203_PFphA, MeterValueID::PowerFactorL1Directional, 37 },
        { &get_model_203_PFphB, MeterValueID::PowerFactorL2Directional, 37 },
        { &get_model_203_PFphC, MeterValueID::PowerFactorL3Directional, 37 },
        { &get_model_203_TotWhExp, MeterValueID::EnergyActiveLSumExport, 54 },
        { &get_model_203_TotWhExpPhA, MeterValueID::EnergyActiveL1Export, 54 },
        { &get_model_203_TotWhExpPhB, MeterValueID::EnergyActiveL2Export, 54 },
        { &get_model_203_TotWhExpPhC, MeterValueID::EnergyActiveL3Export, 54 },
        { &get_model_203_TotWhImp, MeterValueID::EnergyActiveLSumImport, 54 },
        { &get_model_203_TotWhImpPhA, MeterValueID::EnergyActiveL1Import, 54 },
        { &get_model_203_TotWhImpPhB, MeterValueID::EnergyActiveL2Import, 54 },
        { &get_model_203_TotWhImpPhC, MeterValueID::EnergyActiveL3Import, 54 },
        { &get_model_203_TotVAhExp, MeterValueID::EnergyApparentLSumExport, 71 },
        { &get_model_203_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export, 71 },
        { &get_model_203_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export, 71 },
        { &get_model_203_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export, 71 },
        { &get_model_203_TotVAhImp, MeterValueID::EnergyApparentLSumImport, 71 },
        { &get_model_203_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import, 71 },
        { &get_model_203_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import, 71 },
        { &get_model_203_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import, 71 },
        { &get_model_203_TotVArhImpQ1, MeterValueID::EnergyReactiveLSumQ1, 104 },
        { &get_model_203_TotVArhImpQ1PhA, MeterValueID::EnergyReactiveL1Q1, 104 },
        { &get_model_203_TotVArhImpQ1PhB, MeterValueID::EnergyReactiveL2Q1, 104 },
        { &get_model_203_TotVArhImpQ1PhC, MeterValueID::EnergyReactiveL3Q1, 104 },
        { &get_model_203_TotVArhImpQ2, MeterValueID::EnergyReactiveLSumQ2, 104 },
        { &get_model_203_TotVArhImpQ2PhA, MeterValueID::EnergyReactiveL1Q2, 104 },
        { &get_model_203_TotVArhImpQ2PhB, MeterValueID::EnergyReactiveL2Q2, 104 },
        { &get_model_203_TotVArhImpQ2PhC, MeterValueID::EnergyReactiveL3Q2, 104 },
        { &get_model_203_TotVArhExpQ3, MeterValueID::EnergyReactiveLSumQ3, 104 },
        { &get_model_203_TotVArhExpQ3PhA, MeterValueID::EnergyReactiveL1Q3, 104 },
        { &get_model_203_TotVArhExpQ3PhB, MeterValueID::EnergyReactiveL2Q3, 104 },
        { &get_model_203_TotVArhExpQ3PhC, MeterValueID::EnergyReactiveL3Q3, 104 },
        { &get_model_203_TotVArhExpQ4, MeterValueID::EnergyReactiveLSumQ4, 104 },
        { &get_model_203_TotVArhExpQ4PhA, MeterValueID::EnergyReactiveL1Q4, 104 },
        { &get_model_203_TotVArhExpQ4PhB, MeterValueID::EnergyReactiveL2Q4, 104 },
        { &get_model_203_TotVArhExpQ4PhC, MeterValueID::EnergyReactiveL3Q4, 104 },
    }
};

// =====================================
// 204 - Delta-Connect Three Phase Meter
// =====================================

#include "model_204.h"

static float get_model_204_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->A;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_204_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->AphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_204_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->AphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_204_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->AphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_204_PhV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhV;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_204_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_204_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_204_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_204_PPV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PPV;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_204_PhVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphAB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_204_PhVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphBC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_204_PhVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PhVphCA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_204_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->Hz;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Hz_SF);
    return fval;
}

static float get_model_204_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_204_WphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->WphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_204_WphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->WphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_204_WphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->WphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_204_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_204_VAphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VAphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_204_VAphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VAphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_204_VAphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VAphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_204_VAR(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VAR;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_204_VARphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VARphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_204_VARphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VARphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_204_VARphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->VARphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VAR_SF);
    return fval;
}

static float get_model_204_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_204_PFphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PFphA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_204_PFphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PFphB;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_204_PFphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    int16_t val = model->PFphC;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    if ((quirks & SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY) == 0) {
        fval *= 0.01f;
    }
    return fval;
}

static float get_model_204_TotWhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExp);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotWhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotWhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotWhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhExpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotWhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImp);
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotWhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotWhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotWhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotWhImpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVAhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExp);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVAhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVAhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVAhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhExpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVAhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImp);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVAhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVAhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVAhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVAhImpPhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVAh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhImpQ1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhImpQ1PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhImpQ1PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhImpQ1PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ1PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhImpQ2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhImpQ2PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhImpQ2PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhImpQ2PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhImpQ2PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhExpQ3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhExpQ3PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhExpQ3PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhExpQ3PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ3PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhExpQ4(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhExpQ4PhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhA);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhExpQ4PhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhB);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
}

static float get_model_204_TotVArhExpQ4PhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel204_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->TotVArhExpQ4PhC);
    if (val == 0 && !detection) return NAN;
    uint32_t not_implemented_val = (quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) == 0 ? UINT32_MAX : 0x80000000u;
    if (val == not_implemented_val) return NAN;
    if (val > INT32_MAX && quirks & SUN_SPEC_QUIRKS_ACC32_IS_INT32) val = -val;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVArh_SF) * 0.001f);
    return fval;
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
    105, // model_length
    105, // interesting_registers_count
    true, // is_meter
    true, // read_twice
    &model_204_validator,
    61,  // value_count
    {    // value_data
        { &get_model_204_A, MeterValueID::CurrentLSumImExSum, 6 },
        { &get_model_204_AphA, MeterValueID::CurrentL1ImExSum, 6 },
        { &get_model_204_AphB, MeterValueID::CurrentL2ImExSum, 6 },
        { &get_model_204_AphC, MeterValueID::CurrentL3ImExSum, 6 },
        { &get_model_204_PhV, MeterValueID::VoltageLNAvg, 15 },
        { &get_model_204_PhVphA, MeterValueID::VoltageL1N, 15 },
        { &get_model_204_PhVphB, MeterValueID::VoltageL2N, 15 },
        { &get_model_204_PhVphC, MeterValueID::VoltageL3N, 15 },
        { &get_model_204_PPV, MeterValueID::VoltageLLAvg, 15 },
        { &get_model_204_PhVphAB, MeterValueID::VoltageL1L2, 15 },
        { &get_model_204_PhVphBC, MeterValueID::VoltageL2L3, 15 },
        { &get_model_204_PhVphCA, MeterValueID::VoltageL3L1, 15 },
        { &get_model_204_Hz, MeterValueID::FrequencyLAvg, 17 },
        { &get_model_204_W, MeterValueID::PowerActiveLSumImExDiff, 22 },
        { &get_model_204_WphA, MeterValueID::PowerActiveL1ImExDiff, 22 },
        { &get_model_204_WphB, MeterValueID::PowerActiveL2ImExDiff, 22 },
        { &get_model_204_WphC, MeterValueID::PowerActiveL3ImExDiff, 22 },
        { &get_model_204_VA, MeterValueID::PowerApparentLSumImExDiff, 27 },
        { &get_model_204_VAphA, MeterValueID::PowerApparentL1ImExDiff, 27 },
        { &get_model_204_VAphB, MeterValueID::PowerApparentL2ImExDiff, 27 },
        { &get_model_204_VAphC, MeterValueID::PowerApparentL3ImExDiff, 27 },
        { &get_model_204_VAR, MeterValueID::PowerReactiveLSumIndCapDiff, 32 },
        { &get_model_204_VARphA, MeterValueID::PowerReactiveL1IndCapDiff, 32 },
        { &get_model_204_VARphB, MeterValueID::PowerReactiveL2IndCapDiff, 32 },
        { &get_model_204_VARphC, MeterValueID::PowerReactiveL3IndCapDiff, 32 },
        { &get_model_204_PF, MeterValueID::PowerFactorLSumDirectional, 37 },
        { &get_model_204_PFphA, MeterValueID::PowerFactorL1Directional, 37 },
        { &get_model_204_PFphB, MeterValueID::PowerFactorL2Directional, 37 },
        { &get_model_204_PFphC, MeterValueID::PowerFactorL3Directional, 37 },
        { &get_model_204_TotWhExp, MeterValueID::EnergyActiveLSumExport, 54 },
        { &get_model_204_TotWhExpPhA, MeterValueID::EnergyActiveL1Export, 54 },
        { &get_model_204_TotWhExpPhB, MeterValueID::EnergyActiveL2Export, 54 },
        { &get_model_204_TotWhExpPhC, MeterValueID::EnergyActiveL3Export, 54 },
        { &get_model_204_TotWhImp, MeterValueID::EnergyActiveLSumImport, 54 },
        { &get_model_204_TotWhImpPhA, MeterValueID::EnergyActiveL1Import, 54 },
        { &get_model_204_TotWhImpPhB, MeterValueID::EnergyActiveL2Import, 54 },
        { &get_model_204_TotWhImpPhC, MeterValueID::EnergyActiveL3Import, 54 },
        { &get_model_204_TotVAhExp, MeterValueID::EnergyApparentLSumExport, 71 },
        { &get_model_204_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export, 71 },
        { &get_model_204_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export, 71 },
        { &get_model_204_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export, 71 },
        { &get_model_204_TotVAhImp, MeterValueID::EnergyApparentLSumImport, 71 },
        { &get_model_204_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import, 71 },
        { &get_model_204_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import, 71 },
        { &get_model_204_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import, 71 },
        { &get_model_204_TotVArhImpQ1, MeterValueID::EnergyReactiveLSumQ1, 104 },
        { &get_model_204_TotVArhImpQ1PhA, MeterValueID::EnergyReactiveL1Q1, 104 },
        { &get_model_204_TotVArhImpQ1PhB, MeterValueID::EnergyReactiveL2Q1, 104 },
        { &get_model_204_TotVArhImpQ1PhC, MeterValueID::EnergyReactiveL3Q1, 104 },
        { &get_model_204_TotVArhImpQ2, MeterValueID::EnergyReactiveLSumQ2, 104 },
        { &get_model_204_TotVArhImpQ2PhA, MeterValueID::EnergyReactiveL1Q2, 104 },
        { &get_model_204_TotVArhImpQ2PhB, MeterValueID::EnergyReactiveL2Q2, 104 },
        { &get_model_204_TotVArhImpQ2PhC, MeterValueID::EnergyReactiveL3Q2, 104 },
        { &get_model_204_TotVArhExpQ3, MeterValueID::EnergyReactiveLSumQ3, 104 },
        { &get_model_204_TotVArhExpQ3PhA, MeterValueID::EnergyReactiveL1Q3, 104 },
        { &get_model_204_TotVArhExpQ3PhB, MeterValueID::EnergyReactiveL2Q3, 104 },
        { &get_model_204_TotVArhExpQ3PhC, MeterValueID::EnergyReactiveL3Q3, 104 },
        { &get_model_204_TotVArhExpQ4, MeterValueID::EnergyReactiveLSumQ4, 104 },
        { &get_model_204_TotVArhExpQ4PhA, MeterValueID::EnergyReactiveL1Q4, 104 },
        { &get_model_204_TotVArhExpQ4PhB, MeterValueID::EnergyReactiveL2Q4, 104 },
        { &get_model_204_TotVArhExpQ4PhC, MeterValueID::EnergyReactiveL3Q4, 104 },
    }
};

// ========================
// 211 - Single Phase Meter
// ========================

#include "model_211.h"

static float get_model_211_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->A);
    float fval = val;
    return fval;
}

static float get_model_211_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->AphA);
    float fval = val;
    return fval;
}

static float get_model_211_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->AphB);
    float fval = val;
    return fval;
}

static float get_model_211_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->AphC);
    float fval = val;
    return fval;
}

static float get_model_211_PhV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PhV);
    float fval = val;
    return fval;
}

static float get_model_211_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PhVphA);
    float fval = val;
    return fval;
}

static float get_model_211_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PhVphB);
    float fval = val;
    return fval;
}

static float get_model_211_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PhVphC);
    float fval = val;
    return fval;
}

static float get_model_211_PPV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PPV);
    float fval = val;
    return fval;
}

static float get_model_211_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PPVphAB);
    float fval = val;
    return fval;
}

static float get_model_211_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PPVphBC);
    float fval = val;
    return fval;
}

static float get_model_211_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PPVphCA);
    float fval = val;
    return fval;
}

static float get_model_211_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->Hz);
    float fval = val;
    return fval;
}

static float get_model_211_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->W);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_211_WphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->WphA);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_211_WphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->WphB);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_211_WphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->WphC);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_211_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->VA);
    float fval = val;
    return fval;
}

static float get_model_211_VAphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->VAphA);
    float fval = val;
    return fval;
}

static float get_model_211_VAphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->VAphB);
    float fval = val;
    return fval;
}

static float get_model_211_VAphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->VAphC);
    float fval = val;
    return fval;
}

static float get_model_211_VAR(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->VAR);
    float fval = val;
    return fval;
}

static float get_model_211_VARphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->VARphA);
    float fval = val;
    return fval;
}

static float get_model_211_VARphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->VARphB);
    float fval = val;
    return fval;
}

static float get_model_211_VARphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->VARphC);
    float fval = val;
    return fval;
}

static float get_model_211_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PF);
    float fval = val;
    return fval;
}

static float get_model_211_PFphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PFphA);
    float fval = val;
    return fval;
}

static float get_model_211_PFphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PFphB);
    float fval = val;
    return fval;
}

static float get_model_211_PFphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->PFphC);
    float fval = val;
    return fval;
}

static float get_model_211_TotWhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotWhExp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotWhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotWhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotWhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotWhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotWhImp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotWhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotWhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotWhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVAhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVAhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVAhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVAhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVAhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVAhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVAhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVAhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhImpQ1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhImpQ1phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhImpQ1phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhImpQ1phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhImpQ2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhImpQ2phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhImpQ2phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhImpQ2phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhExpQ3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhExpQ3phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhExpQ3phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhExpQ3phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhExpQ4(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhExpQ4phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhExpQ4phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_211_TotVArhExpQ4phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSinglePhaseMeterModel211_s *model = static_cast<const struct SunSpecSinglePhaseMeterModel211_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static bool model_211_validator(const uint16_t * const register_data[2])
{
    const SunSpecSinglePhaseMeterModel211_s *block0 = reinterpret_cast<const SunSpecSinglePhaseMeterModel211_s *>(register_data[0]);
    if (block0->ID != 211) return false;
    if (block0->L  != 124) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_211_data = {
    211, // model_id
    124, // model_length
    124, // interesting_registers_count
    true, // is_meter
    false, // read_twice
    &model_211_validator,
    61,  // value_count
    {    // value_data
        { &get_model_211_A, MeterValueID::CurrentLSumImExSum, 3 },
        { &get_model_211_AphA, MeterValueID::CurrentL1ImExSum, 5 },
        { &get_model_211_AphB, MeterValueID::CurrentL2ImExSum, 7 },
        { &get_model_211_AphC, MeterValueID::CurrentL3ImExSum, 9 },
        { &get_model_211_PhV, MeterValueID::VoltageLNAvg, 11 },
        { &get_model_211_PhVphA, MeterValueID::VoltageL1N, 13 },
        { &get_model_211_PhVphB, MeterValueID::VoltageL2N, 15 },
        { &get_model_211_PhVphC, MeterValueID::VoltageL3N, 17 },
        { &get_model_211_PPV, MeterValueID::VoltageLLAvg, 19 },
        { &get_model_211_PPVphAB, MeterValueID::VoltageL1L2, 21 },
        { &get_model_211_PPVphBC, MeterValueID::VoltageL2L3, 23 },
        { &get_model_211_PPVphCA, MeterValueID::VoltageL3L1, 25 },
        { &get_model_211_Hz, MeterValueID::FrequencyLAvg, 27 },
        { &get_model_211_W, MeterValueID::PowerActiveLSumImExDiff, 29 },
        { &get_model_211_WphA, MeterValueID::PowerActiveL1ImExDiff, 31 },
        { &get_model_211_WphB, MeterValueID::PowerActiveL2ImExDiff, 33 },
        { &get_model_211_WphC, MeterValueID::PowerActiveL3ImExDiff, 35 },
        { &get_model_211_VA, MeterValueID::PowerApparentLSumImExDiff, 37 },
        { &get_model_211_VAphA, MeterValueID::PowerApparentL1ImExDiff, 39 },
        { &get_model_211_VAphB, MeterValueID::PowerApparentL2ImExDiff, 41 },
        { &get_model_211_VAphC, MeterValueID::PowerApparentL3ImExDiff, 43 },
        { &get_model_211_VAR, MeterValueID::PowerReactiveLSumIndCapDiff, 45 },
        { &get_model_211_VARphA, MeterValueID::PowerReactiveL1IndCapDiff, 47 },
        { &get_model_211_VARphB, MeterValueID::PowerReactiveL2IndCapDiff, 49 },
        { &get_model_211_VARphC, MeterValueID::PowerReactiveL3IndCapDiff, 51 },
        { &get_model_211_PF, MeterValueID::PowerFactorLSumDirectional, 53 },
        { &get_model_211_PFphA, MeterValueID::PowerFactorL1Directional, 55 },
        { &get_model_211_PFphB, MeterValueID::PowerFactorL2Directional, 57 },
        { &get_model_211_PFphC, MeterValueID::PowerFactorL3Directional, 59 },
        { &get_model_211_TotWhExp, MeterValueID::EnergyActiveLSumExport, 61 },
        { &get_model_211_TotWhExpPhA, MeterValueID::EnergyActiveL1Export, 63 },
        { &get_model_211_TotWhExpPhB, MeterValueID::EnergyActiveL2Export, 65 },
        { &get_model_211_TotWhExpPhC, MeterValueID::EnergyActiveL3Export, 67 },
        { &get_model_211_TotWhImp, MeterValueID::EnergyActiveLSumImport, 69 },
        { &get_model_211_TotWhImpPhA, MeterValueID::EnergyActiveL1Import, 71 },
        { &get_model_211_TotWhImpPhB, MeterValueID::EnergyActiveL2Import, 73 },
        { &get_model_211_TotWhImpPhC, MeterValueID::EnergyActiveL3Import, 75 },
        { &get_model_211_TotVAhExp, MeterValueID::EnergyApparentLSumExport, 77 },
        { &get_model_211_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export, 79 },
        { &get_model_211_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export, 81 },
        { &get_model_211_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export, 83 },
        { &get_model_211_TotVAhImp, MeterValueID::EnergyApparentLSumImport, 85 },
        { &get_model_211_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import, 87 },
        { &get_model_211_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import, 89 },
        { &get_model_211_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import, 91 },
        { &get_model_211_TotVArhImpQ1, MeterValueID::EnergyReactiveLSumQ1, 93 },
        { &get_model_211_TotVArhImpQ1phA, MeterValueID::EnergyReactiveL1Q1, 95 },
        { &get_model_211_TotVArhImpQ1phB, MeterValueID::EnergyReactiveL2Q1, 97 },
        { &get_model_211_TotVArhImpQ1phC, MeterValueID::EnergyReactiveL3Q1, 99 },
        { &get_model_211_TotVArhImpQ2, MeterValueID::EnergyReactiveLSumQ2, 101 },
        { &get_model_211_TotVArhImpQ2phA, MeterValueID::EnergyReactiveL1Q2, 103 },
        { &get_model_211_TotVArhImpQ2phB, MeterValueID::EnergyReactiveL2Q2, 105 },
        { &get_model_211_TotVArhImpQ2phC, MeterValueID::EnergyReactiveL3Q2, 107 },
        { &get_model_211_TotVArhExpQ3, MeterValueID::EnergyReactiveLSumQ3, 109 },
        { &get_model_211_TotVArhExpQ3phA, MeterValueID::EnergyReactiveL1Q3, 111 },
        { &get_model_211_TotVArhExpQ3phB, MeterValueID::EnergyReactiveL2Q3, 113 },
        { &get_model_211_TotVArhExpQ3phC, MeterValueID::EnergyReactiveL3Q3, 115 },
        { &get_model_211_TotVArhExpQ4, MeterValueID::EnergyReactiveLSumQ4, 117 },
        { &get_model_211_TotVArhExpQ4phA, MeterValueID::EnergyReactiveL1Q4, 119 },
        { &get_model_211_TotVArhExpQ4phB, MeterValueID::EnergyReactiveL2Q4, 121 },
        { &get_model_211_TotVArhExpQ4phC, MeterValueID::EnergyReactiveL3Q4, 123 },
    }
};

// ==============================
// 212 - Split Single Phase Meter
// ==============================

#include "model_212.h"

static float get_model_212_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->A);
    float fval = val;
    return fval;
}

static float get_model_212_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->AphA);
    float fval = val;
    return fval;
}

static float get_model_212_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->AphB);
    float fval = val;
    return fval;
}

static float get_model_212_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->AphC);
    float fval = val;
    return fval;
}

static float get_model_212_PhV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PhV);
    float fval = val;
    return fval;
}

static float get_model_212_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PhVphA);
    float fval = val;
    return fval;
}

static float get_model_212_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PhVphB);
    float fval = val;
    return fval;
}

static float get_model_212_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PhVphC);
    float fval = val;
    return fval;
}

static float get_model_212_PPV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PPV);
    float fval = val;
    return fval;
}

static float get_model_212_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PPVphAB);
    float fval = val;
    return fval;
}

static float get_model_212_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PPVphBC);
    float fval = val;
    return fval;
}

static float get_model_212_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PPVphCA);
    float fval = val;
    return fval;
}

static float get_model_212_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->Hz);
    float fval = val;
    return fval;
}

static float get_model_212_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->W);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_212_WphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->WphA);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_212_WphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->WphB);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_212_WphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->WphC);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_212_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->VA);
    float fval = val;
    return fval;
}

static float get_model_212_VAphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->VAphA);
    float fval = val;
    return fval;
}

static float get_model_212_VAphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->VAphB);
    float fval = val;
    return fval;
}

static float get_model_212_VAphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->VAphC);
    float fval = val;
    return fval;
}

static float get_model_212_VAR(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->VAR);
    float fval = val;
    return fval;
}

static float get_model_212_VARphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->VARphA);
    float fval = val;
    return fval;
}

static float get_model_212_VARphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->VARphB);
    float fval = val;
    return fval;
}

static float get_model_212_VARphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->VARphC);
    float fval = val;
    return fval;
}

static float get_model_212_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PF);
    float fval = val;
    return fval;
}

static float get_model_212_PFphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PFphA);
    float fval = val;
    return fval;
}

static float get_model_212_PFphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PFphB);
    float fval = val;
    return fval;
}

static float get_model_212_PFphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->PFphC);
    float fval = val;
    return fval;
}

static float get_model_212_TotWhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotWhExp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotWhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotWhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotWhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotWhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotWhImp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotWhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotWhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotWhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVAhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVAhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVAhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVAhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVAhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVAhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVAhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVAhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhImpQ1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhImpQ1phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhImpQ1phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhImpQ1phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhImpQ2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhImpQ2phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhImpQ2phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhImpQ2phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhExpQ3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhExpQ3phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhExpQ3phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhExpQ3phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhExpQ4(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhExpQ4phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhExpQ4phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_212_TotVArhExpQ4phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecSplitSinglePhaseMeterModel212_s *model = static_cast<const struct SunSpecSplitSinglePhaseMeterModel212_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static bool model_212_validator(const uint16_t * const register_data[2])
{
    const SunSpecSplitSinglePhaseMeterModel212_s *block0 = reinterpret_cast<const SunSpecSplitSinglePhaseMeterModel212_s *>(register_data[0]);
    if (block0->ID != 212) return false;
    if (block0->L  != 124) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_212_data = {
    212, // model_id
    124, // model_length
    124, // interesting_registers_count
    true, // is_meter
    false, // read_twice
    &model_212_validator,
    61,  // value_count
    {    // value_data
        { &get_model_212_A, MeterValueID::CurrentLSumImExSum, 3 },
        { &get_model_212_AphA, MeterValueID::CurrentL1ImExSum, 5 },
        { &get_model_212_AphB, MeterValueID::CurrentL2ImExSum, 7 },
        { &get_model_212_AphC, MeterValueID::CurrentL3ImExSum, 9 },
        { &get_model_212_PhV, MeterValueID::VoltageLNAvg, 11 },
        { &get_model_212_PhVphA, MeterValueID::VoltageL1N, 13 },
        { &get_model_212_PhVphB, MeterValueID::VoltageL2N, 15 },
        { &get_model_212_PhVphC, MeterValueID::VoltageL3N, 17 },
        { &get_model_212_PPV, MeterValueID::VoltageLLAvg, 19 },
        { &get_model_212_PPVphAB, MeterValueID::VoltageL1L2, 21 },
        { &get_model_212_PPVphBC, MeterValueID::VoltageL2L3, 23 },
        { &get_model_212_PPVphCA, MeterValueID::VoltageL3L1, 25 },
        { &get_model_212_Hz, MeterValueID::FrequencyLAvg, 27 },
        { &get_model_212_W, MeterValueID::PowerActiveLSumImExDiff, 29 },
        { &get_model_212_WphA, MeterValueID::PowerActiveL1ImExDiff, 31 },
        { &get_model_212_WphB, MeterValueID::PowerActiveL2ImExDiff, 33 },
        { &get_model_212_WphC, MeterValueID::PowerActiveL3ImExDiff, 35 },
        { &get_model_212_VA, MeterValueID::PowerApparentLSumImExDiff, 37 },
        { &get_model_212_VAphA, MeterValueID::PowerApparentL1ImExDiff, 39 },
        { &get_model_212_VAphB, MeterValueID::PowerApparentL2ImExDiff, 41 },
        { &get_model_212_VAphC, MeterValueID::PowerApparentL3ImExDiff, 43 },
        { &get_model_212_VAR, MeterValueID::PowerReactiveLSumIndCapDiff, 45 },
        { &get_model_212_VARphA, MeterValueID::PowerReactiveL1IndCapDiff, 47 },
        { &get_model_212_VARphB, MeterValueID::PowerReactiveL2IndCapDiff, 49 },
        { &get_model_212_VARphC, MeterValueID::PowerReactiveL3IndCapDiff, 51 },
        { &get_model_212_PF, MeterValueID::PowerFactorLSumDirectional, 53 },
        { &get_model_212_PFphA, MeterValueID::PowerFactorL1Directional, 55 },
        { &get_model_212_PFphB, MeterValueID::PowerFactorL2Directional, 57 },
        { &get_model_212_PFphC, MeterValueID::PowerFactorL3Directional, 59 },
        { &get_model_212_TotWhExp, MeterValueID::EnergyActiveLSumExport, 61 },
        { &get_model_212_TotWhExpPhA, MeterValueID::EnergyActiveL1Export, 63 },
        { &get_model_212_TotWhExpPhB, MeterValueID::EnergyActiveL2Export, 65 },
        { &get_model_212_TotWhExpPhC, MeterValueID::EnergyActiveL3Export, 67 },
        { &get_model_212_TotWhImp, MeterValueID::EnergyActiveLSumImport, 69 },
        { &get_model_212_TotWhImpPhA, MeterValueID::EnergyActiveL1Import, 71 },
        { &get_model_212_TotWhImpPhB, MeterValueID::EnergyActiveL2Import, 73 },
        { &get_model_212_TotWhImpPhC, MeterValueID::EnergyActiveL3Import, 75 },
        { &get_model_212_TotVAhExp, MeterValueID::EnergyApparentLSumExport, 77 },
        { &get_model_212_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export, 79 },
        { &get_model_212_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export, 81 },
        { &get_model_212_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export, 83 },
        { &get_model_212_TotVAhImp, MeterValueID::EnergyApparentLSumImport, 85 },
        { &get_model_212_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import, 87 },
        { &get_model_212_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import, 89 },
        { &get_model_212_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import, 91 },
        { &get_model_212_TotVArhImpQ1, MeterValueID::EnergyReactiveLSumQ1, 93 },
        { &get_model_212_TotVArhImpQ1phA, MeterValueID::EnergyReactiveL1Q1, 95 },
        { &get_model_212_TotVArhImpQ1phB, MeterValueID::EnergyReactiveL2Q1, 97 },
        { &get_model_212_TotVArhImpQ1phC, MeterValueID::EnergyReactiveL3Q1, 99 },
        { &get_model_212_TotVArhImpQ2, MeterValueID::EnergyReactiveLSumQ2, 101 },
        { &get_model_212_TotVArhImpQ2phA, MeterValueID::EnergyReactiveL1Q2, 103 },
        { &get_model_212_TotVArhImpQ2phB, MeterValueID::EnergyReactiveL2Q2, 105 },
        { &get_model_212_TotVArhImpQ2phC, MeterValueID::EnergyReactiveL3Q2, 107 },
        { &get_model_212_TotVArhExpQ3, MeterValueID::EnergyReactiveLSumQ3, 109 },
        { &get_model_212_TotVArhExpQ3phA, MeterValueID::EnergyReactiveL1Q3, 111 },
        { &get_model_212_TotVArhExpQ3phB, MeterValueID::EnergyReactiveL2Q3, 113 },
        { &get_model_212_TotVArhExpQ3phC, MeterValueID::EnergyReactiveL3Q3, 115 },
        { &get_model_212_TotVArhExpQ4, MeterValueID::EnergyReactiveLSumQ4, 117 },
        { &get_model_212_TotVArhExpQ4phA, MeterValueID::EnergyReactiveL1Q4, 119 },
        { &get_model_212_TotVArhExpQ4phB, MeterValueID::EnergyReactiveL2Q4, 121 },
        { &get_model_212_TotVArhExpQ4phC, MeterValueID::EnergyReactiveL3Q4, 123 },
    }
};

// ===================================
// 213 - Wye-Connect Three Phase Meter
// ===================================

#include "model_213.h"

static float get_model_213_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->A);
    float fval = val;
    return fval;
}

static float get_model_213_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->AphA);
    float fval = val;
    return fval;
}

static float get_model_213_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->AphB);
    float fval = val;
    return fval;
}

static float get_model_213_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->AphC);
    float fval = val;
    return fval;
}

static float get_model_213_PhV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PhV);
    float fval = val;
    return fval;
}

static float get_model_213_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PhVphA);
    float fval = val;
    return fval;
}

static float get_model_213_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PhVphB);
    float fval = val;
    return fval;
}

static float get_model_213_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PhVphC);
    float fval = val;
    return fval;
}

static float get_model_213_PPV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PPV);
    float fval = val;
    return fval;
}

static float get_model_213_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PPVphAB);
    float fval = val;
    return fval;
}

static float get_model_213_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PPVphBC);
    float fval = val;
    return fval;
}

static float get_model_213_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PPVphCA);
    float fval = val;
    return fval;
}

static float get_model_213_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->Hz);
    float fval = val;
    return fval;
}

static float get_model_213_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->W);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_213_WphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->WphA);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_213_WphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->WphB);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_213_WphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->WphC);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_213_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->VA);
    float fval = val;
    return fval;
}

static float get_model_213_VAphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->VAphA);
    float fval = val;
    return fval;
}

static float get_model_213_VAphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->VAphB);
    float fval = val;
    return fval;
}

static float get_model_213_VAphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->VAphC);
    float fval = val;
    return fval;
}

static float get_model_213_VAR(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->VAR);
    float fval = val;
    return fval;
}

static float get_model_213_VARphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->VARphA);
    float fval = val;
    return fval;
}

static float get_model_213_VARphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->VARphB);
    float fval = val;
    return fval;
}

static float get_model_213_VARphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->VARphC);
    float fval = val;
    return fval;
}

static float get_model_213_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PF);
    float fval = val;
    return fval;
}

static float get_model_213_PFphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PFphA);
    float fval = val;
    return fval;
}

static float get_model_213_PFphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PFphB);
    float fval = val;
    return fval;
}

static float get_model_213_PFphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->PFphC);
    float fval = val;
    return fval;
}

static float get_model_213_TotWhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotWhExp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotWhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotWhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotWhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotWhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotWhImp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotWhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotWhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotWhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVAhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVAhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVAhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVAhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVAhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVAhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVAhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVAhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhImpQ1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhImpQ1phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhImpQ1phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhImpQ1phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhImpQ2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhImpQ2phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhImpQ2phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhImpQ2phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhExpQ3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhExpQ3phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhExpQ3phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhExpQ3phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhExpQ4(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhExpQ4phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhExpQ4phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_213_TotVArhExpQ4phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecWyeConnectThreePhaseMeterModel213_s *model = static_cast<const struct SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static bool model_213_validator(const uint16_t * const register_data[2])
{
    const SunSpecWyeConnectThreePhaseMeterModel213_s *block0 = reinterpret_cast<const SunSpecWyeConnectThreePhaseMeterModel213_s *>(register_data[0]);
    if (block0->ID != 213) return false;
    if (block0->L  != 124) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_213_data = {
    213, // model_id
    124, // model_length
    124, // interesting_registers_count
    true, // is_meter
    false, // read_twice
    &model_213_validator,
    61,  // value_count
    {    // value_data
        { &get_model_213_A, MeterValueID::CurrentLSumImExSum, 3 },
        { &get_model_213_AphA, MeterValueID::CurrentL1ImExSum, 5 },
        { &get_model_213_AphB, MeterValueID::CurrentL2ImExSum, 7 },
        { &get_model_213_AphC, MeterValueID::CurrentL3ImExSum, 9 },
        { &get_model_213_PhV, MeterValueID::VoltageLNAvg, 11 },
        { &get_model_213_PhVphA, MeterValueID::VoltageL1N, 13 },
        { &get_model_213_PhVphB, MeterValueID::VoltageL2N, 15 },
        { &get_model_213_PhVphC, MeterValueID::VoltageL3N, 17 },
        { &get_model_213_PPV, MeterValueID::VoltageLLAvg, 19 },
        { &get_model_213_PPVphAB, MeterValueID::VoltageL1L2, 21 },
        { &get_model_213_PPVphBC, MeterValueID::VoltageL2L3, 23 },
        { &get_model_213_PPVphCA, MeterValueID::VoltageL3L1, 25 },
        { &get_model_213_Hz, MeterValueID::FrequencyLAvg, 27 },
        { &get_model_213_W, MeterValueID::PowerActiveLSumImExDiff, 29 },
        { &get_model_213_WphA, MeterValueID::PowerActiveL1ImExDiff, 31 },
        { &get_model_213_WphB, MeterValueID::PowerActiveL2ImExDiff, 33 },
        { &get_model_213_WphC, MeterValueID::PowerActiveL3ImExDiff, 35 },
        { &get_model_213_VA, MeterValueID::PowerApparentLSumImExDiff, 37 },
        { &get_model_213_VAphA, MeterValueID::PowerApparentL1ImExDiff, 39 },
        { &get_model_213_VAphB, MeterValueID::PowerApparentL2ImExDiff, 41 },
        { &get_model_213_VAphC, MeterValueID::PowerApparentL3ImExDiff, 43 },
        { &get_model_213_VAR, MeterValueID::PowerReactiveLSumIndCapDiff, 45 },
        { &get_model_213_VARphA, MeterValueID::PowerReactiveL1IndCapDiff, 47 },
        { &get_model_213_VARphB, MeterValueID::PowerReactiveL2IndCapDiff, 49 },
        { &get_model_213_VARphC, MeterValueID::PowerReactiveL3IndCapDiff, 51 },
        { &get_model_213_PF, MeterValueID::PowerFactorLSumDirectional, 53 },
        { &get_model_213_PFphA, MeterValueID::PowerFactorL1Directional, 55 },
        { &get_model_213_PFphB, MeterValueID::PowerFactorL2Directional, 57 },
        { &get_model_213_PFphC, MeterValueID::PowerFactorL3Directional, 59 },
        { &get_model_213_TotWhExp, MeterValueID::EnergyActiveLSumExport, 61 },
        { &get_model_213_TotWhExpPhA, MeterValueID::EnergyActiveL1Export, 63 },
        { &get_model_213_TotWhExpPhB, MeterValueID::EnergyActiveL2Export, 65 },
        { &get_model_213_TotWhExpPhC, MeterValueID::EnergyActiveL3Export, 67 },
        { &get_model_213_TotWhImp, MeterValueID::EnergyActiveLSumImport, 69 },
        { &get_model_213_TotWhImpPhA, MeterValueID::EnergyActiveL1Import, 71 },
        { &get_model_213_TotWhImpPhB, MeterValueID::EnergyActiveL2Import, 73 },
        { &get_model_213_TotWhImpPhC, MeterValueID::EnergyActiveL3Import, 75 },
        { &get_model_213_TotVAhExp, MeterValueID::EnergyApparentLSumExport, 77 },
        { &get_model_213_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export, 79 },
        { &get_model_213_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export, 81 },
        { &get_model_213_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export, 83 },
        { &get_model_213_TotVAhImp, MeterValueID::EnergyApparentLSumImport, 85 },
        { &get_model_213_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import, 87 },
        { &get_model_213_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import, 89 },
        { &get_model_213_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import, 91 },
        { &get_model_213_TotVArhImpQ1, MeterValueID::EnergyReactiveLSumQ1, 93 },
        { &get_model_213_TotVArhImpQ1phA, MeterValueID::EnergyReactiveL1Q1, 95 },
        { &get_model_213_TotVArhImpQ1phB, MeterValueID::EnergyReactiveL2Q1, 97 },
        { &get_model_213_TotVArhImpQ1phC, MeterValueID::EnergyReactiveL3Q1, 99 },
        { &get_model_213_TotVArhImpQ2, MeterValueID::EnergyReactiveLSumQ2, 101 },
        { &get_model_213_TotVArhImpQ2phA, MeterValueID::EnergyReactiveL1Q2, 103 },
        { &get_model_213_TotVArhImpQ2phB, MeterValueID::EnergyReactiveL2Q2, 105 },
        { &get_model_213_TotVArhImpQ2phC, MeterValueID::EnergyReactiveL3Q2, 107 },
        { &get_model_213_TotVArhExpQ3, MeterValueID::EnergyReactiveLSumQ3, 109 },
        { &get_model_213_TotVArhExpQ3phA, MeterValueID::EnergyReactiveL1Q3, 111 },
        { &get_model_213_TotVArhExpQ3phB, MeterValueID::EnergyReactiveL2Q3, 113 },
        { &get_model_213_TotVArhExpQ3phC, MeterValueID::EnergyReactiveL3Q3, 115 },
        { &get_model_213_TotVArhExpQ4, MeterValueID::EnergyReactiveLSumQ4, 117 },
        { &get_model_213_TotVArhExpQ4phA, MeterValueID::EnergyReactiveL1Q4, 119 },
        { &get_model_213_TotVArhExpQ4phB, MeterValueID::EnergyReactiveL2Q4, 121 },
        { &get_model_213_TotVArhExpQ4phC, MeterValueID::EnergyReactiveL3Q4, 123 },
    }
};

// =====================================
// 214 - Delta-Connect Three Phase Meter
// =====================================

#include "model_214.h"

static float get_model_214_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->A);
    float fval = val;
    return fval;
}

static float get_model_214_AphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->AphA);
    float fval = val;
    return fval;
}

static float get_model_214_AphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->AphB);
    float fval = val;
    return fval;
}

static float get_model_214_AphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->AphC);
    float fval = val;
    return fval;
}

static float get_model_214_PhV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PhV);
    float fval = val;
    return fval;
}

static float get_model_214_PhVphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PhVphA);
    float fval = val;
    return fval;
}

static float get_model_214_PhVphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PhVphB);
    float fval = val;
    return fval;
}

static float get_model_214_PhVphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PhVphC);
    float fval = val;
    return fval;
}

static float get_model_214_PPV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PPV);
    float fval = val;
    return fval;
}

static float get_model_214_PPVphAB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PPVphAB);
    float fval = val;
    return fval;
}

static float get_model_214_PPVphBC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PPVphBC);
    float fval = val;
    return fval;
}

static float get_model_214_PPVphCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PPVphCA);
    float fval = val;
    return fval;
}

static float get_model_214_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->Hz);
    float fval = val;
    return fval;
}

static float get_model_214_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->W);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_214_WphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->WphA);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_214_WphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->WphB);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_214_WphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->WphC);
    float fval = val;
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_214_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->VA);
    float fval = val;
    return fval;
}

static float get_model_214_VAphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->VAphA);
    float fval = val;
    return fval;
}

static float get_model_214_VAphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->VAphB);
    float fval = val;
    return fval;
}

static float get_model_214_VAphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->VAphC);
    float fval = val;
    return fval;
}

static float get_model_214_VAR(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->VAR);
    float fval = val;
    return fval;
}

static float get_model_214_VARphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->VARphA);
    float fval = val;
    return fval;
}

static float get_model_214_VARphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->VARphB);
    float fval = val;
    return fval;
}

static float get_model_214_VARphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->VARphC);
    float fval = val;
    return fval;
}

static float get_model_214_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PF);
    float fval = val;
    return fval;
}

static float get_model_214_PFphA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PFphA);
    float fval = val;
    return fval;
}

static float get_model_214_PFphB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PFphB);
    float fval = val;
    return fval;
}

static float get_model_214_PFphC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->PFphC);
    float fval = val;
    return fval;
}

static float get_model_214_TotWhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotWhExp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotWhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotWhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotWhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotWhExpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotWhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotWhImp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotWhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotWhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotWhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotWhImpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVAhExp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVAhExpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVAhExpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVAhExpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVAhExpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVAhImp(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImp);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVAhImpPhA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVAhImpPhB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVAhImpPhC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVAhImpPhC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhImpQ1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhImpQ1phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhImpQ1phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhImpQ1phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ1phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhImpQ2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhImpQ2phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhImpQ2phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhImpQ2phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhImpQ2phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhExpQ3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhExpQ3phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhExpQ3phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhExpQ3phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ3phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhExpQ4(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhExpQ4phA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phA);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhExpQ4phB(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phB);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static float get_model_214_TotVArhExpQ4phC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *model = static_cast<const struct SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data);
    float val = convert_me_float(&model->TotVArhExpQ4phC);
    float fval = val;
    fval *= 0.001f;
    return fval;
}

static bool model_214_validator(const uint16_t * const register_data[2])
{
    const SunSpecDeltaConnectThreePhaseMeterModel214_s *block0 = reinterpret_cast<const SunSpecDeltaConnectThreePhaseMeterModel214_s *>(register_data[0]);
    if (block0->ID != 214) return false;
    if (block0->L  != 124) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_214_data = {
    214, // model_id
    124, // model_length
    124, // interesting_registers_count
    true, // is_meter
    false, // read_twice
    &model_214_validator,
    61,  // value_count
    {    // value_data
        { &get_model_214_A, MeterValueID::CurrentLSumImExSum, 3 },
        { &get_model_214_AphA, MeterValueID::CurrentL1ImExSum, 5 },
        { &get_model_214_AphB, MeterValueID::CurrentL2ImExSum, 7 },
        { &get_model_214_AphC, MeterValueID::CurrentL3ImExSum, 9 },
        { &get_model_214_PhV, MeterValueID::VoltageLNAvg, 11 },
        { &get_model_214_PhVphA, MeterValueID::VoltageL1N, 13 },
        { &get_model_214_PhVphB, MeterValueID::VoltageL2N, 15 },
        { &get_model_214_PhVphC, MeterValueID::VoltageL3N, 17 },
        { &get_model_214_PPV, MeterValueID::VoltageLLAvg, 19 },
        { &get_model_214_PPVphAB, MeterValueID::VoltageL1L2, 21 },
        { &get_model_214_PPVphBC, MeterValueID::VoltageL2L3, 23 },
        { &get_model_214_PPVphCA, MeterValueID::VoltageL3L1, 25 },
        { &get_model_214_Hz, MeterValueID::FrequencyLAvg, 27 },
        { &get_model_214_W, MeterValueID::PowerActiveLSumImExDiff, 29 },
        { &get_model_214_WphA, MeterValueID::PowerActiveL1ImExDiff, 31 },
        { &get_model_214_WphB, MeterValueID::PowerActiveL2ImExDiff, 33 },
        { &get_model_214_WphC, MeterValueID::PowerActiveL3ImExDiff, 35 },
        { &get_model_214_VA, MeterValueID::PowerApparentLSumImExDiff, 37 },
        { &get_model_214_VAphA, MeterValueID::PowerApparentL1ImExDiff, 39 },
        { &get_model_214_VAphB, MeterValueID::PowerApparentL2ImExDiff, 41 },
        { &get_model_214_VAphC, MeterValueID::PowerApparentL3ImExDiff, 43 },
        { &get_model_214_VAR, MeterValueID::PowerReactiveLSumIndCapDiff, 45 },
        { &get_model_214_VARphA, MeterValueID::PowerReactiveL1IndCapDiff, 47 },
        { &get_model_214_VARphB, MeterValueID::PowerReactiveL2IndCapDiff, 49 },
        { &get_model_214_VARphC, MeterValueID::PowerReactiveL3IndCapDiff, 51 },
        { &get_model_214_PF, MeterValueID::PowerFactorLSumDirectional, 53 },
        { &get_model_214_PFphA, MeterValueID::PowerFactorL1Directional, 55 },
        { &get_model_214_PFphB, MeterValueID::PowerFactorL2Directional, 57 },
        { &get_model_214_PFphC, MeterValueID::PowerFactorL3Directional, 59 },
        { &get_model_214_TotWhExp, MeterValueID::EnergyActiveLSumExport, 61 },
        { &get_model_214_TotWhExpPhA, MeterValueID::EnergyActiveL1Export, 63 },
        { &get_model_214_TotWhExpPhB, MeterValueID::EnergyActiveL2Export, 65 },
        { &get_model_214_TotWhExpPhC, MeterValueID::EnergyActiveL3Export, 67 },
        { &get_model_214_TotWhImp, MeterValueID::EnergyActiveLSumImport, 69 },
        { &get_model_214_TotWhImpPhA, MeterValueID::EnergyActiveL1Import, 71 },
        { &get_model_214_TotWhImpPhB, MeterValueID::EnergyActiveL2Import, 73 },
        { &get_model_214_TotWhImpPhC, MeterValueID::EnergyActiveL3Import, 75 },
        { &get_model_214_TotVAhExp, MeterValueID::EnergyApparentLSumExport, 77 },
        { &get_model_214_TotVAhExpPhA, MeterValueID::EnergyApparentL1Export, 79 },
        { &get_model_214_TotVAhExpPhB, MeterValueID::EnergyApparentL2Export, 81 },
        { &get_model_214_TotVAhExpPhC, MeterValueID::EnergyApparentL3Export, 83 },
        { &get_model_214_TotVAhImp, MeterValueID::EnergyApparentLSumImport, 85 },
        { &get_model_214_TotVAhImpPhA, MeterValueID::EnergyApparentL1Import, 87 },
        { &get_model_214_TotVAhImpPhB, MeterValueID::EnergyApparentL2Import, 89 },
        { &get_model_214_TotVAhImpPhC, MeterValueID::EnergyApparentL3Import, 91 },
        { &get_model_214_TotVArhImpQ1, MeterValueID::EnergyReactiveLSumQ1, 93 },
        { &get_model_214_TotVArhImpQ1phA, MeterValueID::EnergyReactiveL1Q1, 95 },
        { &get_model_214_TotVArhImpQ1phB, MeterValueID::EnergyReactiveL2Q1, 97 },
        { &get_model_214_TotVArhImpQ1phC, MeterValueID::EnergyReactiveL3Q1, 99 },
        { &get_model_214_TotVArhImpQ2, MeterValueID::EnergyReactiveLSumQ2, 101 },
        { &get_model_214_TotVArhImpQ2phA, MeterValueID::EnergyReactiveL1Q2, 103 },
        { &get_model_214_TotVArhImpQ2phB, MeterValueID::EnergyReactiveL2Q2, 105 },
        { &get_model_214_TotVArhImpQ2phC, MeterValueID::EnergyReactiveL3Q2, 107 },
        { &get_model_214_TotVArhExpQ3, MeterValueID::EnergyReactiveLSumQ3, 109 },
        { &get_model_214_TotVArhExpQ3phA, MeterValueID::EnergyReactiveL1Q3, 111 },
        { &get_model_214_TotVArhExpQ3phB, MeterValueID::EnergyReactiveL2Q3, 113 },
        { &get_model_214_TotVArhExpQ3phC, MeterValueID::EnergyReactiveL3Q3, 115 },
        { &get_model_214_TotVArhExpQ4, MeterValueID::EnergyReactiveLSumQ4, 117 },
        { &get_model_214_TotVArhExpQ4phA, MeterValueID::EnergyReactiveL1Q4, 119 },
        { &get_model_214_TotVArhExpQ4phB, MeterValueID::EnergyReactiveL2Q4, 121 },
        { &get_model_214_TotVArhExpQ4phC, MeterValueID::EnergyReactiveL3Q4, 123 },
    }
};

// ========================
// 701 - DER AC Measurement
// ========================

#include "model_701.h"

static float get_model_701_W(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->W;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_701_VA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->VA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_701_Var(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->Var;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Var_SF);
    return fval;
}

static float get_model_701_PF(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->PF;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    return fval;
}

static float get_model_701_A(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->A;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_701_LLV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint16_t val = model->LLV;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_701_LNV(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint16_t val = model->LNV;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_701_Hz(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint32_t val = convert_me_uint32(&model->Hz);
    if (val == UINT32_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Hz_SF);
    return fval;
}

static float get_model_701_TotWhInj(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotWhInj);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotWhAbs(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotWhAbs);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotVarhInj(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotVarhInj);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVarh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotVarhAbs(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotVarhAbs);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVarh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TmpCab(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->TmpCab;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_701_TmpSnk(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->TmpSnk;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_701_TmpTrns(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->TmpTrns;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_701_TmpOt(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->TmpOt;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Tmp_SF);
    return fval;
}

static float get_model_701_WL1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->WL1;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_701_VAL1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->VAL1;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_701_VarL1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->VarL1;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Var_SF);
    return fval;
}

static float get_model_701_PFL1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->PFL1;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    return fval;
}

static float get_model_701_AL1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->AL1;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_701_VL1L2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint16_t val = model->VL1L2;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_701_VL1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint16_t val = model->VL1;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_701_TotWhInjL1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotWhInjL1);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotWhAbsL1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotWhAbsL1);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotVarhInjL1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotVarhInjL1);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVarh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotVarhAbsL1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotVarhAbsL1);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVarh_SF) * 0.001f);
    return fval;
}

static float get_model_701_WL2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->WL2;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_701_VAL2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->VAL2;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_701_VarL2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->VarL2;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Var_SF);
    return fval;
}

static float get_model_701_PFL2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->PFL2;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    return fval;
}

static float get_model_701_AL2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->AL2;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_701_VL2L3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint16_t val = model->VL2L3;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_701_VL2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint16_t val = model->VL2;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_701_TotWhInjL2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotWhInjL2);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotWhAbsL2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotWhAbsL2);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotVarhInjL2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotVarhInjL2);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVarh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotVarhAbsL2(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotVarhAbsL2);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVarh_SF) * 0.001f);
    return fval;
}

static float get_model_701_WL3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->WL3;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->W_SF);
    if (quirks & SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED) fval = -fval;
    return fval;
}

static float get_model_701_VAL3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->VAL3;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->VA_SF);
    return fval;
}

static float get_model_701_VarL3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->VarL3;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Var_SF);
    return fval;
}

static float get_model_701_PFL3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->PFL3;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->PF_SF);
    return fval;
}

static float get_model_701_AL3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    int16_t val = model->AL3;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->A_SF);
    return fval;
}

static float get_model_701_VL3L1(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint16_t val = model->VL3L1;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_701_VL3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint16_t val = model->VL3;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->V_SF);
    return fval;
}

static float get_model_701_TotWhInjL3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotWhInjL3);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotWhAbsL3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotWhAbsL3);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotWh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotVarhInjL3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotVarhInjL3);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVarh_SF) * 0.001f);
    return fval;
}

static float get_model_701_TotVarhAbsL3(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERACMeasurementModel701_s *model = static_cast<const struct SunSpecDERACMeasurementModel701_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->TotVarhAbsL3);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= (get_scale_factor(model->TotVarh_SF) * 0.001f);
    return fval;
}

static bool model_701_validator(const uint16_t * const register_data[2])
{
    const SunSpecDERACMeasurementModel701_s *block0 = reinterpret_cast<const SunSpecDERACMeasurementModel701_s *>(register_data[0]);
    const SunSpecDERACMeasurementModel701_s *block1 = reinterpret_cast<const SunSpecDERACMeasurementModel701_s *>(register_data[1]);
    if (block0->ID != 701) return false;
    if (block1->ID != 701) return false;
    if (block0->L  != 153) return false;
    if (block1->L  != 153) return false;
    if (block0->A_SF != block1->A_SF) return false;
    if (block0->V_SF != block1->V_SF) return false;
    if (block0->Hz_SF != block1->Hz_SF) return false;
    if (block0->W_SF != block1->W_SF) return false;
    if (block0->PF_SF != block1->PF_SF) return false;
    if (block0->VA_SF != block1->VA_SF) return false;
    if (block0->Var_SF != block1->Var_SF) return false;
    if (block0->TotWh_SF != block1->TotWh_SF) return false;
    if (block0->TotVarh_SF != block1->TotVarh_SF) return false;
    if (block0->Tmp_SF != block1->Tmp_SF) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_701_data = {
    701, // model_id
    153, // model_length
    123, // interesting_registers_count
    false, // is_meter
    true, // read_twice
    &model_701_validator,
    49,  // value_count
    {    // value_data
        { &get_model_701_W, MeterValueID::PowerActiveLSumImExDiff, 116 },
        { &get_model_701_VA, MeterValueID::PowerApparentLSumImExDiff, 118 },
        { &get_model_701_Var, MeterValueID::PowerReactiveLSumIndCapDiff, 119 },
        { &get_model_701_PF, MeterValueID::PowerFactorLSumDirectional, 117 },
        { &get_model_701_A, MeterValueID::CurrentLSumImExSum, 113 },
        { &get_model_701_LLV, MeterValueID::VoltageLLAvg, 114 },
        { &get_model_701_LNV, MeterValueID::VoltageLNAvg, 114 },
        { &get_model_701_Hz, MeterValueID::FrequencyLAvg, 115 },
        { &get_model_701_TotWhInj, MeterValueID::EnergyActiveLSumExport, 120 },
        { &get_model_701_TotWhAbs, MeterValueID::EnergyActiveLSumImport, 120 },
        { &get_model_701_TotVarhInj, MeterValueID::EnergyReactiveLSumInductive, 121 },
        { &get_model_701_TotVarhAbs, MeterValueID::EnergyReactiveLSumCapacitive, 121 },
        { &get_model_701_TmpCab, MeterValueID::TemperatureCabinet, 122 },
        { &get_model_701_TmpSnk, MeterValueID::TemperatureHeatSink, 122 },
        { &get_model_701_TmpTrns, MeterValueID::TemperatureTransformer, 122 },
        { &get_model_701_TmpOt, MeterValueID::Temperature, 122 },
        { &get_model_701_WL1, MeterValueID::PowerActiveL1ImExDiff, 116 },
        { &get_model_701_VAL1, MeterValueID::PowerApparentL1ImExDiff, 118 },
        { &get_model_701_VarL1, MeterValueID::PowerReactiveL1IndCapDiff, 119 },
        { &get_model_701_PFL1, MeterValueID::PowerFactorL1Directional, 117 },
        { &get_model_701_AL1, MeterValueID::CurrentL1ImExSum, 113 },
        { &get_model_701_VL1L2, MeterValueID::VoltageL1L2, 114 },
        { &get_model_701_VL1, MeterValueID::VoltageL1N, 114 },
        { &get_model_701_TotWhInjL1, MeterValueID::EnergyActiveL1Export, 120 },
        { &get_model_701_TotWhAbsL1, MeterValueID::EnergyActiveL1Import, 120 },
        { &get_model_701_TotVarhInjL1, MeterValueID::EnergyReactiveL1Inductive, 121 },
        { &get_model_701_TotVarhAbsL1, MeterValueID::EnergyReactiveL1Capacitive, 121 },
        { &get_model_701_WL2, MeterValueID::PowerActiveL2ImExDiff, 116 },
        { &get_model_701_VAL2, MeterValueID::PowerApparentL2ImExDiff, 118 },
        { &get_model_701_VarL2, MeterValueID::PowerReactiveL2IndCapDiff, 119 },
        { &get_model_701_PFL2, MeterValueID::PowerFactorL2Directional, 117 },
        { &get_model_701_AL2, MeterValueID::CurrentL2ImExSum, 113 },
        { &get_model_701_VL2L3, MeterValueID::VoltageL2L3, 114 },
        { &get_model_701_VL2, MeterValueID::VoltageL2N, 114 },
        { &get_model_701_TotWhInjL2, MeterValueID::EnergyActiveL2Export, 120 },
        { &get_model_701_TotWhAbsL2, MeterValueID::EnergyActiveL2Import, 120 },
        { &get_model_701_TotVarhInjL2, MeterValueID::EnergyReactiveL2Inductive, 121 },
        { &get_model_701_TotVarhAbsL2, MeterValueID::EnergyReactiveL2Capacitive, 121 },
        { &get_model_701_WL3, MeterValueID::PowerActiveL3ImExDiff, 116 },
        { &get_model_701_VAL3, MeterValueID::PowerApparentL3ImExDiff, 118 },
        { &get_model_701_VarL3, MeterValueID::PowerReactiveL3IndCapDiff, 119 },
        { &get_model_701_PFL3, MeterValueID::PowerFactorL3Directional, 117 },
        { &get_model_701_AL3, MeterValueID::CurrentL3ImExSum, 113 },
        { &get_model_701_VL3L1, MeterValueID::VoltageL3L1, 114 },
        { &get_model_701_VL3, MeterValueID::VoltageL3N, 114 },
        { &get_model_701_TotWhInjL3, MeterValueID::EnergyActiveL3Export, 120 },
        { &get_model_701_TotWhAbsL3, MeterValueID::EnergyActiveL3Import, 120 },
        { &get_model_701_TotVarhInjL3, MeterValueID::EnergyReactiveL3Inductive, 121 },
        { &get_model_701_TotVarhAbsL3, MeterValueID::EnergyReactiveL3Capacitive, 121 },
    }
};

// ==========================
// 713 - DER Storage Capacity
// ==========================

#include "model_713.h"

static float get_model_713_SoC(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERStorageCapacityModel713_s *model = static_cast<const struct SunSpecDERStorageCapacityModel713_s *>(register_data);
    uint16_t val = model->SoC;
    if (val == UINT16_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->Pct_SF);
    return fval;
}

static bool model_713_validator(const uint16_t * const register_data[2])
{
    const SunSpecDERStorageCapacityModel713_s *block0 = reinterpret_cast<const SunSpecDERStorageCapacityModel713_s *>(register_data[0]);
    const SunSpecDERStorageCapacityModel713_s *block1 = reinterpret_cast<const SunSpecDERStorageCapacityModel713_s *>(register_data[1]);
    if (block0->ID != 713) return false;
    if (block1->ID != 713) return false;
    if (block0->L  !=   7) return false;
    if (block1->L  !=   7) return false;
    if (block0->WH_SF != block1->WH_SF) return false;
    if (block0->Pct_SF != block1->Pct_SF) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_713_data = {
    713, // model_id
    7, // model_length
    9, // interesting_registers_count
    false, // is_meter
    true, // read_twice
    &model_713_validator,
    1,  // value_count
    {    // value_data
        { &get_model_713_SoC, MeterValueID::StateOfCharge, 8 },
    }
};

// ========================
// 714 - DER DC Measurement
// ========================

#include "model_714.h"

static float get_model_714_DCA(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERDCMeasurementModel714_s *model = static_cast<const struct SunSpecDERDCMeasurementModel714_s *>(register_data);
    int16_t val = model->DCA;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCA_SF);
    return fval;
}

static float get_model_714_DCW(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERDCMeasurementModel714_s *model = static_cast<const struct SunSpecDERDCMeasurementModel714_s *>(register_data);
    int16_t val = model->DCW;
    if (val == INT16_MIN) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCW_SF);
    return fval;
}

static float get_model_714_DCWhInj(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERDCMeasurementModel714_s *model = static_cast<const struct SunSpecDERDCMeasurementModel714_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->DCWhInj);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCWH_SF);
    return fval;
}

static float get_model_714_DCWhAbs(const void *register_data, uint32_t quirks, bool detection)
{
    const struct SunSpecDERDCMeasurementModel714_s *model = static_cast<const struct SunSpecDERDCMeasurementModel714_s *>(register_data);
    uint64_t val = convert_me_uint64(&model->DCWhAbs);
    if (val == UINT64_MAX) return NAN;
    float fval = static_cast<float>(val);
    fval *= get_scale_factor(model->DCWH_SF);
    return fval;
}

static bool model_714_validator(const uint16_t * const register_data[2])
{
    const SunSpecDERDCMeasurementModel714_s *block0 = reinterpret_cast<const SunSpecDERDCMeasurementModel714_s *>(register_data[0]);
    const SunSpecDERDCMeasurementModel714_s *block1 = reinterpret_cast<const SunSpecDERDCMeasurementModel714_s *>(register_data[1]);
    if (block0->ID != 714) return false;
    if (block1->ID != 714) return false;
    if (block0->L  !=  18) return false;
    if (block1->L  !=  18) return false;
    if (block0->DCA_SF != block1->DCA_SF) return false;
    if (block0->DCV_SF != block1->DCV_SF) return false;
    if (block0->DCW_SF != block1->DCW_SF) return false;
    if (block0->DCWH_SF != block1->DCWH_SF) return false;
    if (block0->Tmp_SF != block1->Tmp_SF) return false;
    return true;
}

static const MetersSunSpecParser::ModelData model_714_data = {
    714, // model_id
    18, // model_length
    19, // interesting_registers_count
    false, // is_meter
    true, // read_twice
    &model_714_validator,
    4,  // value_count
    {    // value_data
        { &get_model_714_DCA, MeterValueID::CurrentDC, 15 },
        { &get_model_714_DCW, MeterValueID::PowerDC, 17 },
        { &get_model_714_DCWhInj, MeterValueID::EnergyActiveLSumExport, 18 },
        { &get_model_714_DCWhAbs, MeterValueID::EnergyActiveLSumImport, 18 },
    }
};


const MetersSunSpecParser::AllModelData meters_sun_spec_all_model_data {
    18, // model_count
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
        &model_701_data,
        &model_713_data,
        &model_714_data,
    }
};
