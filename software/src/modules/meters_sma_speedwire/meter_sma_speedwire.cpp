/* esp32-firmware
 * Copyright (C) 2023 Thomas Hein
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

#define EVENT_LOG_PREFIX "meters_sma_swire"

#include "meter_sma_speedwire.h"

#include <math.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"

#include "gcc_warnings.h"

static float convert_uint32(const uint8_t *buf)
{
    uint32_t u32 = static_cast<uint32_t>(buf[0]) << 24 |
                   static_cast<uint32_t>(buf[1]) << 16 |
                   static_cast<uint32_t>(buf[2]) <<  8 |
                   static_cast<uint32_t>(buf[3]) <<  0;

    return static_cast<float>(u32);
}

static float convert_uint64(const uint8_t *buf)
{
    uint64_t u64 = static_cast<uint64_t>(buf[0]) << 56 |
                   static_cast<uint64_t>(buf[1]) << 48 |
                   static_cast<uint64_t>(buf[2]) << 40 |
                   static_cast<uint64_t>(buf[3]) << 32 |
                   static_cast<uint64_t>(buf[4]) << 24 |
                   static_cast<uint64_t>(buf[5]) << 16 |
                   static_cast<uint64_t>(buf[6]) <<  8 |
                   static_cast<uint64_t>(buf[7]) <<  0;

    return static_cast<float>(u64);
}

union obis_code {
    uint8_t  u8[4];
    uint32_t u32;
};

struct obis_value_mapping {
    MeterValueID value_id;
    obis_code obis;
    float scaling_factor;
    float (*parser_fn)(const uint8_t *buf);
};

static const obis_value_mapping obis_value_mappings[] {
    {MeterValueID::PowerActiveLSumImport,        {0,  1, 4, 0},      1/10.0F, convert_uint32},  // Sum
    {MeterValueID::PowerActiveL1Import,          {0, 21, 4, 0},      1/10.0F, convert_uint32},  // L1
    {MeterValueID::PowerActiveL2Import,          {0, 41, 4, 0},      1/10.0F, convert_uint32},  // L2
    {MeterValueID::PowerActiveL3Import,          {0, 61, 4, 0},      1/10.0F, convert_uint32},  // L3

    {MeterValueID::EnergyActiveLSumImport,       {0,  1, 8, 0}, 1/3600000.0F, convert_uint64},  // Sum
    {MeterValueID::EnergyActiveL1Import,         {0, 21, 8, 0}, 1/3600000.0F, convert_uint64},  // L1
    {MeterValueID::EnergyActiveL2Import,         {0, 41, 8, 0}, 1/3600000.0F, convert_uint64},  // L2
    {MeterValueID::EnergyActiveL3Import,         {0, 61, 8, 0}, 1/3600000.0F, convert_uint64},  // L3

    {MeterValueID::PowerActiveLSumExport,        {0,  2, 4, 0},      1/10.0F, convert_uint32},  // Sum
    {MeterValueID::PowerActiveL1Export,          {0, 22, 4, 0},      1/10.0F, convert_uint32},  // L1
    {MeterValueID::PowerActiveL2Export,          {0, 42, 4, 0},      1/10.0F, convert_uint32},  // L2
    {MeterValueID::PowerActiveL3Export,          {0, 62, 4, 0},      1/10.0F, convert_uint32},  // L3

    {MeterValueID::EnergyActiveLSumExport,       {0,  2, 8, 0}, 1/3600000.0F, convert_uint64},  // Sum
    {MeterValueID::EnergyActiveL1Export,         {0, 22, 8, 0}, 1/3600000.0F, convert_uint64},  // L1
    {MeterValueID::EnergyActiveL2Export,         {0, 42, 8, 0}, 1/3600000.0F, convert_uint64},  // L2
    {MeterValueID::EnergyActiveL3Export,         {0, 62, 8, 0}, 1/3600000.0F, convert_uint64},  // L3

    {MeterValueID::PowerReactiveLSumInductive,   {0,  3, 4, 0},      1/10.0F, convert_uint32},  // Sum
    {MeterValueID::PowerReactiveL1Inductive,     {0, 23, 4, 0},      1/10.0F, convert_uint32},  // L1
    {MeterValueID::PowerReactiveL2Inductive,     {0, 43, 4, 0},      1/10.0F, convert_uint32},  // L2
    {MeterValueID::PowerReactiveL3Inductive,     {0, 63, 4, 0},      1/10.0F, convert_uint32},  // L3

    {MeterValueID::EnergyReactiveLSumInductive,  {0,  3, 8, 0}, 1/3600000.0F, convert_uint64},  // Sum
    {MeterValueID::EnergyReactiveL1Inductive,    {0, 23, 8, 0}, 1/3600000.0F, convert_uint64},  // L1
    {MeterValueID::EnergyReactiveL2Inductive,    {0, 43, 8, 0}, 1/3600000.0F, convert_uint64},  // L2
    {MeterValueID::EnergyReactiveL3Inductive,    {0, 63, 8, 0}, 1/3600000.0F, convert_uint64},  // L3

    {MeterValueID::PowerReactiveLSumCapacitive,  {0,  4, 4, 0},      1/10.0F, convert_uint32},  // Sum
    {MeterValueID::PowerReactiveL1Capacitive,    {0, 24, 4, 0},      1/10.0F, convert_uint32},  // L1
    {MeterValueID::PowerReactiveL2Capacitive,    {0, 44, 4, 0},      1/10.0F, convert_uint32},  // L2
    {MeterValueID::PowerReactiveL3Capacitive,    {0, 64, 4, 0},      1/10.0F, convert_uint32},  // L3

    {MeterValueID::EnergyReactiveLSumCapacitive, {0,  4, 8, 0}, 1/3600000.0F, convert_uint64},  // Sum
    {MeterValueID::EnergyReactiveL1Capacitive,   {0, 24, 8, 0}, 1/3600000.0F, convert_uint64},  // L1
    {MeterValueID::EnergyReactiveL2Capacitive,   {0, 44, 8, 0}, 1/3600000.0F, convert_uint64},  // L2
    {MeterValueID::EnergyReactiveL3Capacitive,   {0, 64, 8, 0}, 1/3600000.0F, convert_uint64},  // L3

    {MeterValueID::PowerApparentLSumImport,      {0,  9, 4, 0},      1/10.0F, convert_uint32},  // Sum
    {MeterValueID::PowerApparentL1Import,        {0, 29, 4, 0},      1/10.0F, convert_uint32},  // L1
    {MeterValueID::PowerApparentL2Import,        {0, 49, 4, 0},      1/10.0F, convert_uint32},  // L2
    {MeterValueID::PowerApparentL3Import,        {0, 69, 4, 0},      1/10.0F, convert_uint32},  // L3

    {MeterValueID::EnergyApparentLSumImport,     {0,  9, 8, 0}, 1/3600000.0F, convert_uint64},  // Sum
    {MeterValueID::EnergyApparentL1Import,       {0, 29, 8, 0}, 1/3600000.0F, convert_uint64},  // L1
    {MeterValueID::EnergyApparentL2Import,       {0, 49, 8, 0}, 1/3600000.0F, convert_uint64},  // L2
    {MeterValueID::EnergyApparentL3Import,       {0, 69, 8, 0}, 1/3600000.0F, convert_uint64},  // L3

    {MeterValueID::PowerApparentLSumExport,      {0, 10, 4, 0},      1/10.0F, convert_uint32},  // Sum
    {MeterValueID::PowerApparentL1Export,        {0, 30, 4, 0},      1/10.0F, convert_uint32},  // L1
    {MeterValueID::PowerApparentL2Export,        {0, 50, 4, 0},      1/10.0F, convert_uint32},  // L2
    {MeterValueID::PowerApparentL3Export,        {0, 70, 4, 0},      1/10.0F, convert_uint32},  // L3

    {MeterValueID::EnergyApparentLSumExport,     {0, 10, 8, 0}, 1/3600000.0F, convert_uint64},  // Sum
    {MeterValueID::EnergyApparentL1Export,       {0, 30, 8, 0}, 1/3600000.0F, convert_uint64},  // L1
    {MeterValueID::EnergyApparentL2Export,       {0, 50, 8, 0}, 1/3600000.0F, convert_uint64},  // L2
    {MeterValueID::EnergyApparentL3Export,       {0, 70, 8, 0}, 1/3600000.0F, convert_uint64},  // L3

    // Power factors are always positive, for both import and export
    {MeterValueID::PowerFactorLSum,              {0, 13, 4, 0},    1/1000.0F, convert_uint32},  // Sum
    {MeterValueID::PowerFactorL1,                {0, 33, 4, 0},    1/1000.0F, convert_uint32},  // L1
    {MeterValueID::PowerFactorL2,                {0, 53, 4, 0},    1/1000.0F, convert_uint32},  // L2
    {MeterValueID::PowerFactorL3,                {0, 73, 4, 0},    1/1000.0F, convert_uint32},  // L3

    {MeterValueID::VoltageL1N,                   {0, 32, 4, 0},    1/1000.0F, convert_uint32},  // L1
    {MeterValueID::VoltageL2N,                   {0, 52, 4, 0},    1/1000.0F, convert_uint32},  // L2
    {MeterValueID::VoltageL3N,                   {0, 72, 4, 0},    1/1000.0F, convert_uint32},  // L3

    // Currents are always positive, for both import and export
    {MeterValueID::CurrentL1ImExSum,             {0, 31, 4, 0},    1/1000.0F, convert_uint32},  // L1
    {MeterValueID::CurrentL2ImExSum,             {0, 51, 4, 0},    1/1000.0F, convert_uint32},  // L2
    {MeterValueID::CurrentL3ImExSum,             {0, 71, 4, 0},    1/1000.0F, convert_uint32},  // L3

    {MeterValueID::FrequencyLAvg,                {0, 14, 4, 0},    1/1000.0F, convert_uint32},
};

static_assert(ARRAY_SIZE(obis_value_mappings) == METERS_SMA_SPEEDWIRE_OBIS_COUNT, "obis_value_mappings size mismatch");

MeterClassID MeterSMASpeedwire::get_class() const
{
    return MeterClassID::SMASpeedwire;
}

void MeterSMASpeedwire::setup(const Config & /*ephemeral_config*/)
{
    IPAddress mc_groupIP(239, 12, 255, 254);
    if (udp.beginMulticast(mc_groupIP, 9522)) {
        logger.printfln("Joined multicast group %s:9522", mc_groupIP.toString().c_str());
    } else {
        logger.printfln("Couldn't join multicast group %s:9522", mc_groupIP.toString().c_str());
        return;
    }

    MeterValueID valueIds[METERS_SMA_SPEEDWIRE_VALUE_COUNT];

    for (size_t i = 0; i < ARRAY_SIZE(obis_value_mappings); i++) {
        MeterValueID value_id = obis_value_mappings[i].value_id;
        valueIds[i] = value_id;

        if (value_id == MeterValueID::PowerActiveLSumImport) {
            power_import_index = i;
        } else if (value_id == MeterValueID::PowerActiveLSumExport) {
            power_export_index = i;
        }
    }

    valueIds[METERS_SMA_SPEEDWIRE_VALUE_COUNT - 1] = MeterValueID::PowerActiveLSumImExDiff;

    meters.declare_value_ids(slot, valueIds, ARRAY_SIZE(valueIds));

    // Tested Speedwire products send one packet per second.
    // Poll twice a second to reduce latency and packet backlog.
    task_scheduler.scheduleWithFixedDelay([this]() {
        parse_packet();
    }, 500_ms);
}

void MeterSMASpeedwire::parse_packet()
{
    if (udp.parsePacket() > 0) {
        uint8_t buf[1024];
        int len = udp.read(buf, sizeof(buf));

        if (len == 608) { // Supports only the currently known packet length.
            if (!values_parsed) {
                parse_values(buf, len);
                values_parsed = true;
            }

            float values[METERS_SMA_SPEEDWIRE_VALUE_COUNT];

            for (size_t i = 0; i < ARRAY_SIZE(obis_value_positions); i++) {
                size_t position = obis_value_positions[i];
                if (position > 0) {
                    auto mapping = obis_value_mappings[i];
                    values[i] = mapping.parser_fn(buf + position) * mapping.scaling_factor;
                } else {
                    values[i] = NAN;
                }
            }

            values[METERS_SMA_SPEEDWIRE_VALUE_COUNT - 1] = values[power_import_index] - values[power_export_index];

            meters.update_all_values(slot, values);
        }
    }
}

void MeterSMASpeedwire::parse_values(const uint8_t *buf, int buflen)
{
    // TODO: Properly parse packet structure instead of probing values.
    for (size_t i = 0; i < METERS_SMA_SPEEDWIRE_OBIS_COUNT; i++) {
        for (int pos = 28; pos < buflen - 4; pos += 4) {
            const uint8_t *value_start = buf + pos;
            obis_code obis_code{value_start[0], value_start[1], value_start[2], value_start[3]};
            if (obis_code.u32 == obis_value_mappings[i].obis.u32) {
                pos += 4;
                obis_value_positions[i] = static_cast<size_t>(pos);
                break;
            }
        }
    }
}
