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

#define EVENT_LOG_PREFIX "meters_speedwire"

#include "meter_sma_speedwire.h"

#include <math.h>
#include <arpa/inet.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "tools.h"

#include "gcc_warnings.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic ignored "-Wattributes"
    #pragma GCC diagnostic ignored "-Wpacked"
#endif

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

void MeterSMASpeedwire::setup(Config *ephemeral_config)
{
    serial_number = ephemeral_config->get("serial_number")->asUint();

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
}

void MeterSMASpeedwire::register_events()
{
    network.on_network_connected([this](const Config *connected) {
        if (!connected->asBool()) {
            return EventResult::OK; // Try again on next connected event.
        }

        const char *group = "239.12.255.254";
        uint16_t port = 9522;

        if (!udp.beginMulticast({group}, port)) {
            logger.printfln_meter("Couldn't join multicast group %s:%u", group, port);
            return EventResult::OK; // Try again on next connected event.
        }

        logger.printfln_meter("Joined multicast group %s:%u", group, port);

        // Tested Speedwire products send one packet per second.
        // Poll twice a second to reduce latency and packet backlog.
        task_scheduler.scheduleWithFixedDelay([this]() {
            parse_packet();
        }, 500_ms);

        return EventResult::Deregister;
    });
}

int MeterSMASpeedwire::parse_header(SpeedwireHeader *header)
{
    header->length         = ntohs(header->length);
    header->tag0           = ntohs(header->tag0);
    header->group          = ntohl(header->group);
    header->data_length    = ntohs(header->data_length);
    header->tag            = ntohs(header->tag);
    header->protocol_id    = ntohs(header->protocol_id);
    header->susy_id        = ntohs(header->susy_id);
    header->serial_number  = ntohl(header->serial_number);
    header->measuring_time = ntohl(header->measuring_time);

    meters_sma_speedwire.trace_timestamp();
    logger.tracefln_plain(trace_buffer_index, "m%lu v%c%c%c l%u t%u g%lu d%u t%u pi%u si%u sn%lu mt%lu",
                          slot,
                          header->vendor[0], header->vendor[1], header->vendor[2],
                          header->length,
                          header->tag0,
                          header->group,
                          header->data_length,
                          header->tag,
                          header->protocol_id,
                          header->susy_id,
                          header->serial_number,
                          header->measuring_time); // FIXME: also trace data

    if (serial_number != 0 && header->serial_number != serial_number) {
        return 0;
    }

    if (header->vendor[0] != 'S' || header->vendor[1] != 'M' || header->vendor[2] != 'A' || header->vendor[3] != '\0') {
        logger.printfln_meter("Invalid vendor: %c%c%c", header->vendor[0], header->vendor[1], header->vendor[2]);
        return 0;
    }

    if (header->length != 4) {
        logger.printfln_meter("Invalid length: %u", header->length);
        return 0;
    }

    if (header->tag != 16) {
        logger.printfln_meter("Invalid tag: %u", header->tag);
        return 0;
    }

    if (header->data_length > sizeof(SpeedwirePacket::data)) {
        logger.printfln_meter("Invalid data length: %u", header->data_length);
        return 0;
    }

    // Protocol ID 0x6065 is for control packets. This is not officially documented by SMA, but it is reverse-engineered.
    // See e.g. https://github.com/erijo/energy-utils/blob/master/sma.py#L194
    // We ignore these packets for now.
    if (header->protocol_id == 0x6065) {
        return 0;
    }

    // Protocol ID 0x6081 is used for communication between SMA devices and SMA EV Charger.
    // We ignore these packets for now.
    if (header->protocol_id == 0x6081) {
        return 0;
    }

    if (header->protocol_id != 0x6069) {
        logger.printfln_meter("Invalid protocol ID: %u", header->protocol_id);
        return 0;
    }

    return header->data_length - 10; // The length includes part of the header, we only return the lenght of the actual data part
}

void MeterSMASpeedwire::parse_packet()
{
    if (udp.parsePacket() > 0) {
        SpeedwirePacket packet;
        memset(&packet, 0, sizeof(packet));

        const int read_length = udp.read(reinterpret_cast<char*>(&packet), sizeof(packet));
        const int data_length = parse_header(&packet.header);
        if (data_length <= 0) {
            return;
        }

        const int data_length_with_header = data_length + static_cast<int>(sizeof(SpeedwireHeader));
        if (read_length < data_length_with_header) {
            logger.printfln_meter("Speedwire packet too short: %d < %d", read_length, data_length_with_header);
            return;
        }

        if (!values_parsed) {
            parse_values(packet.data, data_length);
            values_parsed = true;
        }

        float values[METERS_SMA_SPEEDWIRE_VALUE_COUNT];

        for (size_t i = 0; i < ARRAY_SIZE(obis_value_positions); i++) {
            size_t position = obis_value_positions[i];
            if (position == 0) {
                values[i] = NAN;
            } else if (position >= static_cast<size_t>(data_length)) {
                logger.printfln_meter("Access beyond data length for OBIS value %zu: position %zu >= %i", i, position, data_length);
                values[i] = NAN;
            } else {
                const obis_value_mapping &mapping = obis_value_mappings[i];
                values[i] = mapping.parser_fn(packet.data + position) * mapping.scaling_factor;
            }
        }

        values[METERS_SMA_SPEEDWIRE_VALUE_COUNT - 1] = values[power_import_index] - values[power_export_index];

        meters.update_all_values(slot, values);
    }
}

void MeterSMASpeedwire::parse_values(const uint8_t *buf, int buflen)
{
    // TODO: Properly parse packet structure instead of probing values.
    for (size_t i = 0; i < METERS_SMA_SPEEDWIRE_OBIS_COUNT; i++) {
        for (int pos = 0; pos < buflen - 4; pos += 4) {
            const uint8_t *value_start = buf + pos;
            obis_code obis_code{value_start[0], value_start[1], value_start[2], value_start[3]};
            if (obis_code.u32 == obis_value_mappings[i].obis.u32) {
                pos += 4;
                obis_value_positions[i] = static_cast<size_t>(pos);
                goto value_found;
            }
        }

        // OBIS value not found, set position to 0.
        obis_value_positions[i] = 0;

value_found:
        {}
    }
}
