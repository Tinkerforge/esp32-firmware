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

#include "model_parser_714.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools/float.h"

// ========================
// 714 - DER DC Measurement
// ========================

#define MODEL_714_MAX_PORT_COUNT      4
#define MODEL_714_REGISTER_COUNT      20
#define MODEL_714_PV_ID_COUNT         6
#define MODEL_714_BATTERY_ID_COUNT    6
#define MODEL_714_OTHER_ID_COUNT      6
#define MODEL_714_PORT_REGISTER_COUNT 25
#define MODEL_714_PORT_PV_ID_COUNT    4

static const MeterValueID model_714_pv_ids[] = {
    MeterValueID::CurrentPVSumExport,
    MeterValueID::VoltagePVAvg,
    MeterValueID::PowerPVSumExport,
    MeterValueID::EnergyPVSumExport,
    MeterValueID::PowerPVSumImExDiff,
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
};

static const MeterValueID model_714_battery_ids[] = {
    MeterValueID::CurrentDCChaDisDiff, // FIXME: sign?
    MeterValueID::VoltageDC,
    MeterValueID::PowerDCChaDisDiff, // FIXME: sign?
    MeterValueID::EnergyDCDischarge,
    MeterValueID::EnergyDCCharge,
    MeterValueID::Temperature,
};

static const MeterValueID model_714_other_ids[] = {
    MeterValueID::CurrentDCImExDiff, // FIXME: sign?
    MeterValueID::VoltageDC,
    MeterValueID::PowerDCImExDiff, // FIXME: sign?
    MeterValueID::EnergyDCExport,
    MeterValueID::EnergyDCImport,
    MeterValueID::Temperature,
};

struct Model714_s {
                    uint16_t ID;        //  0
                    uint16_t L;         //  1
                    uint32_t PrtAlrms;  //  2
                    uint16_t NPrt;      //  4
                    int16_t  DCA;       //  5
                    int16_t  DCW;       //  6
    [[gnu::packed]] uint64_t DCWhInj;   //  7
    [[gnu::packed]] uint64_t DCWhAbs;   //  11
                    int16_t  DCA_SF;    //  15
                    int16_t  DCV_SF;    //  16
                    int16_t  DCW_SF;    //  17
                    int16_t  DCWH_SF;   //  18
                    int16_t  Tmp_SF;    //  19
};

struct Model714_Port_s {
                    uint16_t PrtTyp;    //  0
                    uint16_t ID;        //  1
                    char     IDStr[16]; //  2
                    int16_t  DCA;       // 10
                    uint16_t DCV;       // 11
                    int16_t  DCW;       // 12
    [[gnu::packed]] uint64_t DCWhInj;   // 13
    [[gnu::packed]] uint64_t DCWhAbs;   // 17
                    int16_t  Tmp;       // 21
                    uint16_t DCSta;     // 22
    [[gnu::packed]] uint32_t DCAlrm;    // 23
};

static_assert(sizeof(Model714_s) == 40, "Model714_s has unexpected size");
static_assert(sizeof(Model714_Port_s) == 50, "Model714_Port_s has unexpected size");

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

static float uint64_to_float(uint64_t val, int32_t sunssf)
{
    if (val == UINT64_MAX) {
        return NAN;
    }

    union {
        uint64_t u64;
        uint16_t u16[4];
    } u;

    uint16_t *regs = reinterpret_cast<uint16_t *>(&val);

    u.u16[0] = regs[3];
    u.u16[1] = regs[2];
    u.u16[2] = regs[1];
    u.u16[3] = regs[0];

    return static_cast<float>(u.u64) * get_sun_spec_scale_factor(sunssf);
}

bool MetersSunSpecParser714::detect_values(const uint16_t *const register_data[2], uint32_t quirks, size_t *registers_to_read)
{
    if (!is_valid(register_data)) {
        return false;
    }

    const struct Model714_s *block1 = static_cast<const struct Model714_s *>(static_cast<const void *>(register_data[1]));

    cached_port_count = block1->NPrt;

    if (cached_port_count > MODEL_714_MAX_PORT_COUNT) {
        // FIXME: remove this limitation that is caused by the 125 register read limit
        logger.printfln_meter("SunSpec model 714 has %zu ports, only reading the first %i", cached_port_count, MODEL_714_MAX_PORT_COUNT);

        cached_port_count = MODEL_714_MAX_PORT_COUNT;
    }

    *registers_to_read = get_interesting_registers_count();

    switch (port_type) {
    case DCPortType::Photovoltaic:
        meters.declare_value_ids(slot, model_714_pv_ids, MODEL_714_PV_ID_COUNT + MODEL_714_PORT_PV_ID_COUNT * cached_port_count);
        break;

    case DCPortType::EnergyStorageSystem:
        meters.declare_value_ids(slot, model_714_battery_ids, MODEL_714_BATTERY_ID_COUNT);
        break;

    case DCPortType::ElectricVehicle:
    case DCPortType::GenericInjecting:
    case DCPortType::GenericAbsorbing:
    case DCPortType::GenericBidirectional:
    case DCPortType::DCDC:
        meters.declare_value_ids(slot, model_714_other_ids, MODEL_714_OTHER_ID_COUNT);
        break;

    case DCPortType::NotImplemented:
        return false;
    }

    return true;
}

bool MetersSunSpecParser714::parse_values(const uint16_t *const register_data[2], uint32_t quirks)
{
    if (!is_valid(register_data)) {
        return false;
    }

    const struct Model714_s *block1 = static_cast<const struct Model714_s *>(static_cast<const void *>(register_data[1]));
    size_t port_count = block1->NPrt;

    if (port_count > cached_port_count) {
        port_count = cached_port_count;
    }

    float values[ARRAY_SIZE(model_714_pv_ids)];

    values[0] = NAN;
    values[1] = NAN;
    values[2] = NAN;
    values[3] = NAN;
    values[4] = NAN;
    values[5] = NAN;

    size_t matching_port_count = 0;
    size_t pv_idx = 0;

    for (size_t port_idx = 0; port_idx < port_count; ++port_idx) {
        const struct Model714_Port_s *block1_port = static_cast<const struct Model714_Port_s *>(static_cast<const void *>(&register_data[1][MODEL_714_REGISTER_COUNT + port_idx * MODEL_714_PORT_REGISTER_COUNT]));

        if (block1_port->PrtTyp != static_cast<uint16_t>(port_type)) {
            continue;
        }

        // FIXME: maybe use DCSta to exclude disabled ports?

        ++matching_port_count;

        float dca      =  int16_to_float(block1_port->DCA,     block1->DCA_SF);
        float dcv      = uint16_to_float(block1_port->DCV,     block1->DCV_SF);
        float dcw      =  int16_to_float(block1_port->DCW,     block1->DCW_SF);
        float dcwh_inj = uint64_to_float(block1_port->DCWhInj, block1->DCWH_SF) * 0.001f;
        float dcwh_abs = uint64_to_float(block1_port->DCWhAbs, block1->DCWH_SF) * 0.001f;
        float tmp      =  int16_to_float(block1_port->Tmp,     block1->Tmp_SF);

        values[0] = nan_safe_sum(values[0], dca);
        values[1] = nan_safe_sum(values[1], dcv);
        values[2] = nan_safe_sum(values[2], dcw);
        values[3] = nan_safe_sum(values[3], dcwh_inj);
        values[4] = nan_safe_sum(values[4], dcwh_abs);
        values[5] = nan_safe_sum(values[5], tmp);

        if (port_type == DCPortType::Photovoltaic) {
            values[MODEL_714_PV_ID_COUNT + pv_idx * MODEL_714_PORT_PV_ID_COUNT + 0] = dca;
            values[MODEL_714_PV_ID_COUNT + pv_idx * MODEL_714_PORT_PV_ID_COUNT + 1] = dcv;
            values[MODEL_714_PV_ID_COUNT + pv_idx * MODEL_714_PORT_PV_ID_COUNT + 2] = dcw;
            values[MODEL_714_PV_ID_COUNT + pv_idx * MODEL_714_PORT_PV_ID_COUNT + 3] = dcwh_inj;

            ++pv_idx;
        }
    }

    if (matching_port_count > 0) {
        values[1] /= static_cast<float>(matching_port_count);
        values[5] /= static_cast<float>(matching_port_count);
    }

    if (port_type == DCPortType::Photovoltaic) {
        values[4] = zero_safe_negation(values[2]);
    }

    meters.update_all_values(slot, values);

    return true;
}

[[gnu::const]]
bool MetersSunSpecParser714::must_read_twice()
{
    return true;
}

[[gnu::const]]
bool MetersSunSpecParser714::is_model_length_supported(uint32_t model_length)
{
    return model_length >= (MODEL_714_REGISTER_COUNT - 2) && ((model_length - (MODEL_714_REGISTER_COUNT - 2)) % MODEL_714_PORT_REGISTER_COUNT) == 0;
}

[[gnu::const]]
uint32_t MetersSunSpecParser714::get_interesting_registers_count()
{
    return MODEL_714_REGISTER_COUNT + cached_port_count * MODEL_714_PORT_REGISTER_COUNT;
}

bool MetersSunSpecParser714::is_valid(const uint16_t *const register_data[2])
{
    const struct Model714_s *block0 = static_cast<const struct Model714_s *>(static_cast<const void *>(register_data[0]));
    const struct Model714_s *block1 = static_cast<const struct Model714_s *>(static_cast<const void *>(register_data[1]));

    if (block0->ID      != 714)                return false;
    if (block1->ID      != 714)                return false;
    if (!is_model_length_supported(block0->L)) return false;
    if (block0->L       != block1->L)          return false;
    if (block0->DCA_SF  != block1->DCA_SF)     return false;
    if (block0->DCV_SF  != block1->DCV_SF)     return false;
    if (block0->DCW_SF  != block1->DCW_SF)     return false;
    if (block0->DCWH_SF != block1->DCWH_SF)    return false;
    if (block0->Tmp_SF  != block1->Tmp_SF)     return false;

    return true;
}
