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

// ======================================
// 160 - Multiple MPPT Inverter Extension
// ======================================

#define MODEL_160_MAX_MPPT_COUNT      5
#define MODEL_160_REGISTER_COUNT      10
#define MODEL_160_MPPT_REGISTER_COUNT 20

static const MeterValueID model_160_ids[] = {
    MeterValueID::CurrentPVSumExport,
    MeterValueID::VoltagePVAvg,
    MeterValueID::PowerPVSumExport,
    MeterValueID::PowerPVSumImExDiff,
    MeterValueID::EnergyPVSumExport,

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

bool MetersSunSpecParser160::detect_values(const uint16_t *const register_data[2], size_t register_count, uint32_t quirks, size_t *registers_to_read)
{
    if (register_count < MODEL_160_REGISTER_COUNT || !is_valid(register_data)) {
        return false;
    }

    const struct Model160_s *block0 = static_cast<const struct Model160_s *>(static_cast<const void *>(register_data[1]));
    size_t mppt_count = block0->N;

    if (mppt_count > MODEL_160_MAX_MPPT_COUNT) {
        // FIXME: remove this limitation that is caused by the 125 register read limit
        logger.printfln("SunSpec model 160 has %zu MPPT modules, only reading the first %i", mppt_count, MODEL_160_MAX_MPPT_COUNT);

        mppt_count = MODEL_160_MAX_MPPT_COUNT;
    }

    *registers_to_read = MODEL_160_REGISTER_COUNT + mppt_count * MODEL_160_MPPT_REGISTER_COUNT;

    meters.declare_value_ids(meter_slot, model_160_ids, 5 + 4 * mppt_count);

    return true;
}

bool MetersSunSpecParser160::parse_values(const uint16_t *const register_data[2], size_t register_count, uint32_t quirks)
{
    if (register_count < MODEL_160_REGISTER_COUNT || !is_valid(register_data)) {
        return false;
    }

    const struct Model160_s *block0 = static_cast<const struct Model160_s *>(static_cast<const void *>(register_data[1]));

    float values[ARRAY_SIZE(model_160_ids)];
    size_t mppt_count = block0->N;

    if (mppt_count > MODEL_160_MAX_MPPT_COUNT) {
        mppt_count = MODEL_160_MAX_MPPT_COUNT;
    }

    if (register_count < MODEL_160_REGISTER_COUNT + mppt_count * MODEL_160_MPPT_REGISTER_COUNT) {
        return true; // fake consistent data
    }

    values[0] = NAN;
    values[1] = NAN;
    values[2] = NAN;
    values[4] = NAN;

    for (size_t i = 0; i < mppt_count; ++i) {
        const struct Model160_MPPT_s *block0_mppt = static_cast<const struct Model160_MPPT_s *>(static_cast<const void *>(&register_data[1][MODEL_160_REGISTER_COUNT + i * MODEL_160_MPPT_REGISTER_COUNT]));

        float dca  = uint16_to_float(block0_mppt->DCA, block0->DCA_SF);
        float dcv  = uint16_to_float(block0_mppt->DCV, block0->DCV_SF);
        float dcw  = uint16_to_float(block0_mppt->DCW, block0->DCW_SF);
        float dcwh = acc32_to_float(block0_mppt->DCWH, block0->DCWH_SF) * 0.001f;

        if (!isnan(dca)) {
            if (isnan(values[0])) {
                values[0] = dca;
            }
            else {
                values[0] += dca;
            }
        }

        if (!isnan(dcv)) {
            if (isnan(values[1])) {
                values[1] = dcv;
            }
            else {
                values[1] += dcv;
            }
        }

        if (!isnan(dcw)) {
            if (isnan(values[2])) {
                values[2] = dcw;
            }
            else {
                values[2] += dcw;
            }
        }

        if (!isnan(dcwh)) {
            if (isnan(values[4])) {
                values[4] = dcwh;
            }
            else {
                values[4] += dcwh;
            }
        }

        values[5 + i * 4 + 0] = dca;
        values[5 + i * 4 + 1] = dcv;
        values[5 + i * 4 + 2] = dcw;
        values[5 + i * 4 + 3] = dcwh;
    }

    if (mppt_count > 0) {
        values[1] /= static_cast<float>(mppt_count);
    }

    values[3] = -values[2];

    meters.update_all_values(meter_slot, values);

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
    return MODEL_160_REGISTER_COUNT;
}

bool MetersSunSpecParser160::is_valid(const uint16_t *const register_data[2])
{
    const struct Model160_s *block0 = static_cast<const struct Model160_s *>(static_cast<const void *>(register_data[0]));
    const struct Model160_s *block1 = static_cast<const struct Model160_s *>(static_cast<const void *>(register_data[1]));

    if (block0->ID != 160) return false;
    if (block1->ID != 160) return false;
    if (!is_model_length_supported(block0->L)) return false;
    if (block0->L != block1->L) return false;
    if (block0->DCA_SF != block1->DCA_SF) return false;
    if (block0->DCV_SF != block1->DCV_SF) return false;
    if (block0->DCW_SF != block1->DCW_SF) return false;
    if (block0->DCWH_SF != block1->DCWH_SF) return false;

    return true;
}
