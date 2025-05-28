/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "model_parser_160.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools/float.h"

// ======================================
// 160 - Multiple MPPT Inverter Extension
// ======================================

#define MODEL_160_MAX_MPPT_COUNT      5
#define MODEL_160_REGISTER_COUNT      10
#define MODEL_160_ID_COUNT            6
#define MODEL_160_MPPT_REGISTER_COUNT 20
#define MODEL_160_MPPT_ID_COUNT       4

static const MeterValueID model_160_ids[] = {
    MeterValueID::CurrentPVSumExport,
    MeterValueID::VoltagePVAvg,
    MeterValueID::PowerPVSumExport,
    MeterValueID::PowerPVSumImExDiff,
    MeterValueID::EnergyPVSumExport,
    MeterValueID::Temperature,

    MeterValueID::CurrentPV1Export,
    MeterValueID::VoltagePV1,
    MeterValueID::PowerPV1Export,
    MeterValueID::EnergyPV1Export,

    MeterValueID::CurrentPV2Export,
    MeterValueID::VoltagePV2,
    MeterValueID::PowerPV2Export,
    MeterValueID::EnergyPV2Export,

    MeterValueID::CurrentPV3Export,
    MeterValueID::VoltagePV3,
    MeterValueID::PowerPV3Export,
    MeterValueID::EnergyPV3Export,

    MeterValueID::CurrentPV4Export,
    MeterValueID::VoltagePV4,
    MeterValueID::PowerPV4Export,
    MeterValueID::EnergyPV4Export,

    MeterValueID::CurrentPV5Export,
    MeterValueID::VoltagePV5,
    MeterValueID::PowerPV5Export,
    MeterValueID::EnergyPV5Export,
};

struct Model160_s {
    uint16_t ID;        //  0
    uint16_t L;         //  1
    int16_t  DCA_SF;    //  2
    int16_t  DCV_SF;    //  3
    int16_t  DCW_SF;    //  4
    int16_t  DCWH_SF;   //  5
    uint32_t Evt;       //  6
    uint16_t N;         //  8
    uint16_t TmsPer;    //  9
};

struct Model160_MPPT_s {
    uint16_t ID;        //  0
    char     IDStr[16]; //  1
    uint16_t DCA;       //  9
    uint16_t DCV;       // 10
    uint16_t DCW;       // 11
    uint32_t DCWH;      // 12
    uint32_t Tms;       // 14
    int16_t  Tmp;       // 16
    uint16_t DCSt;      // 17
    uint32_t DCEvt;     // 18
};

static_assert(sizeof(Model160_s) == 20, "Model160_s has unexpected size");
static_assert(sizeof(Model160_MPPT_s) == 40, "Model160_MPPT_s has unexpected size");

extern float get_sun_spec_scale_factor(int32_t sunssf);

static float int16_to_float(int16_t val, int32_t sunssf)
{
    if (val == INT16_MIN) {
        return NAN;
    }

    return static_cast<float>(val) * get_sun_spec_scale_factor(sunssf);
}

static float uint16_to_float(uint16_t val, int32_t sunssf)
{
    if (val == UINT16_MAX) {
        return NAN;
    }

    return static_cast<float>(val) * get_sun_spec_scale_factor(sunssf);
}

static float acc32_to_float(uint32_t val, int32_t sunssf)
{
    if (val == 0) {
        return NAN;
    }

    union {
        uint32_t u32;
        uint16_t u16[2];
    } u;

    uint16_t *regs = reinterpret_cast<uint16_t *>(&val);

    u.u16[0] = regs[1];
    u.u16[1] = regs[0];

    return static_cast<float>(u.u32) * get_sun_spec_scale_factor(sunssf);
}

bool MetersSunSpecParser160::detect_values(const uint16_t *const register_data[2], uint32_t quirks, size_t *registers_to_read)
{
    if (!is_valid(register_data)) {
        return false;
    }

    const struct Model160_s *block1 = static_cast<const struct Model160_s *>(static_cast<const void *>(register_data[1]));

    cached_mppt_count = block1->N;

    if (cached_mppt_count > MODEL_160_MAX_MPPT_COUNT) {
        // FIXME: remove this limitation that is caused by the 125 register read limit
        logger.printfln_meter("SunSpec model 160 has %zu MPPT modules, only reading the first %i", cached_mppt_count, MODEL_160_MAX_MPPT_COUNT);

        cached_mppt_count = MODEL_160_MAX_MPPT_COUNT;
    }

    *registers_to_read = get_interesting_registers_count();

    meters.declare_value_ids(slot, model_160_ids, MODEL_160_ID_COUNT + MODEL_160_MPPT_ID_COUNT * cached_mppt_count);

    return true;
}

bool MetersSunSpecParser160::parse_values(const uint16_t *const register_data[2], uint32_t quirks)
{
    if (!is_valid(register_data)) {
        return false;
    }

    const struct Model160_s *block1 = static_cast<const struct Model160_s *>(static_cast<const void *>(register_data[1]));
    size_t mppt_count = block1->N;

    if (mppt_count > cached_mppt_count) {
        mppt_count = cached_mppt_count;
    }

    float values[ARRAY_SIZE(model_160_ids)];

    values[0] = NAN;
    values[1] = NAN;
    values[2] = NAN;
    values[4] = NAN;
    values[5] = NAN;

    for (size_t mppt_idx = 0; mppt_idx < mppt_count; ++mppt_idx) {
        const struct Model160_MPPT_s *block1_mppt = static_cast<const struct Model160_MPPT_s *>(static_cast<const void *>(&register_data[1][MODEL_160_REGISTER_COUNT + mppt_idx * MODEL_160_MPPT_REGISTER_COUNT]));

        // FIXME: maybe use DCSt to exclude disabled modules?

        float dca  = uint16_to_float(block1_mppt->DCA,  block1->DCA_SF);
        float dcv  = uint16_to_float(block1_mppt->DCV,  block1->DCV_SF);
        float dcw  = uint16_to_float(block1_mppt->DCW,  block1->DCW_SF);
        float dcwh =  acc32_to_float(block1_mppt->DCWH, block1->DCWH_SF) * 0.001f;
        float tmp  =  int16_to_float(block1_mppt->Tmp,  0);

        values[0] = nan_safe_sum(values[0], dca);
        values[1] = nan_safe_sum(values[1], dcv);
        values[2] = nan_safe_sum(values[2], dcw);
        values[4] = nan_safe_sum(values[4], dcwh);
        values[5] = nan_safe_sum(values[5], tmp);

        values[MODEL_160_ID_COUNT + mppt_idx * MODEL_160_MPPT_ID_COUNT + 0] = dca;
        values[MODEL_160_ID_COUNT + mppt_idx * MODEL_160_MPPT_ID_COUNT + 1] = dcv;
        values[MODEL_160_ID_COUNT + mppt_idx * MODEL_160_MPPT_ID_COUNT + 2] = dcw;
        values[MODEL_160_ID_COUNT + mppt_idx * MODEL_160_MPPT_ID_COUNT + 3] = dcwh;
    }

    if (mppt_count > 0) {
        values[1] /= static_cast<float>(mppt_count);
        values[5] /= static_cast<float>(mppt_count);
    }

    values[3] = zero_safe_negation(values[2]);

    meters.update_all_values(slot, values);

    return true;
}

[[gnu::const]]
bool MetersSunSpecParser160::must_read_twice()
{
    return true;
}

[[gnu::const]]
bool MetersSunSpecParser160::is_model_length_supported(uint32_t model_length)
{
    return model_length >= (MODEL_160_REGISTER_COUNT - 2) && ((model_length - (MODEL_160_REGISTER_COUNT - 2)) % MODEL_160_MPPT_REGISTER_COUNT) == 0;
}

[[gnu::const]]
uint32_t MetersSunSpecParser160::get_interesting_registers_count()
{
    return MODEL_160_REGISTER_COUNT + cached_mppt_count * MODEL_160_MPPT_REGISTER_COUNT;
}

bool MetersSunSpecParser160::is_valid(const uint16_t *const register_data[2])
{
    const struct Model160_s *block0 = static_cast<const struct Model160_s *>(static_cast<const void *>(register_data[0]));
    const struct Model160_s *block1 = static_cast<const struct Model160_s *>(static_cast<const void *>(register_data[1]));

    if (block0->ID      != 160)                return false;
    if (block1->ID      != 160)                return false;
    if (!is_model_length_supported(block0->L)) return false;
    if (block0->L       != block1->L)          return false;
    if (block0->DCA_SF  != block1->DCA_SF)     return false;
    if (block0->DCV_SF  != block1->DCV_SF)     return false;
    if (block0->DCW_SF  != block1->DCW_SF)     return false;
    if (block0->DCWH_SF != block1->DCWH_SF)    return false;

    return true;
}
