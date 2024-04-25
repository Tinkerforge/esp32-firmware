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

#pragma once

#include <stdint.h>
#include <WiFiUdp.h>

#include "modules/meters/imeter.h"
#include "modules/meters/meter_value_id.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

#define METERS_SMA_SPEEDWIRE_OBIS_COUNT  59
#define METERS_SMA_SPEEDWIRE_VALUE_COUNT 60

static_assert(METERS_SMA_SPEEDWIRE_OBIS_COUNT + 1 == METERS_SMA_SPEEDWIRE_VALUE_COUNT, "OBIS/value count mismatch. Need one value more than OBIS count.");

class MeterSMASpeedwire final : public IMeter
{
public:
    MeterSMASpeedwire(uint32_t slot_) : slot(slot_) {}

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;

    bool supports_power() override          {return true;}
    bool supports_energy_import() override  {return true;}
    bool supports_energy_export() override  {return true;}
    //bool supports_currents() override       {return true;}

private:
    void parse_packet();
    void parse_values(const uint8_t *buf, int buflen);

    uint32_t slot;
    uint32_t power_export_index = 0;
    uint32_t power_import_index = 0;
    size_t   obis_value_positions[METERS_SMA_SPEEDWIRE_OBIS_COUNT];
    bool     values_parsed = false;
    WiFiUDP  udp;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
