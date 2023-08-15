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
#include "modules/energy_manager/structs.h"
#include "modules/meter/meter_defs.h"
#include "modules/meters/imeter.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MeterEM final : public IMeter
{
public:
    MeterEM(uint32_t slot_, Config *state_) : slot(slot_), state(state_) {}

    uint32_t get_class() const override;
    void setup() override;
    void register_urls(String base_url) override;

    bool supports_power() override {return true;}

    bool supports_import_export() override {return true;}
    bool get_import_export(float *energy_import_kwh, float *energy_export_kwh) override;

    bool supports_line_currents() override {return true;}
    bool get_line_currents(float *l1_current_ma, float *l2_current_ma, float *l3_current_ma) override;

    void update_from_em_all_data(EnergyManagerAllData &all_data);
private:
    void update_all_values();

    uint32_t slot;
    Config *state;
    ConfigRoot errors;

    bool first_values_seen = false;
    uint32_t value_index_power  = UINT32_MAX;
    uint32_t value_index_import = UINT32_MAX;
    uint32_t value_index_export = UINT32_MAX;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
