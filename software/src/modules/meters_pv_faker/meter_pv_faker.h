/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#include <math.h>

#include "config.h"
#include "modules/meters/meter_defs.h"
#include "modules/meters/imeter.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MeterPvFaker final : public IMeter
{
public:
    MeterPvFaker(uint32_t slot_) : slot(slot_) {}

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;

    bool supports_power()         override {return true;}
    //bool supports_energy_import() override {return true;}
    //bool supports_energy_imexsum()override {return true;}
    //bool supports_energy_export() override {return true;}
    //bool supports_currents()      override {return true;}

private:
    void update_illuminance(uint32_t illuminance);
    void update_limit(uint32_t illuminance);

    uint32_t slot;

    // Cached config
    int64_t  peak_power;
    uint32_t zero_at_lux;
    uint32_t peak_at_lux;
    int64_t  peak_at_lux_shifted;

    int64_t  limited_power;

    float values[2] = {0};
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
