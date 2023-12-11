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
#include "modules/meters/meter_defs.h"
#include "modules/meters/imeter.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MeterEM final : public IMeter
{
public:
    MeterEM(uint32_t slot_, Config *state_, Config *errors_) : slot(slot_), state(state_), errors(errors_) {}

    MeterClassID get_class() const override;
    //void setup() override;
    //void register_urls(const String &base_url) override;

    bool supports_power()         override {return true;}
    bool supports_energy_import() override {return true;}
    bool supports_energy_imexsum()override {return true;}
    bool supports_energy_export() override {return true;}
    bool supports_currents()      override {return true;}
    bool supports_reset()         override {return true;}
    bool reset()                  override;

    void update_from_em_all_data(EnergyManagerAllData &all_data);
private:
    void update_all_values();

    uint32_t slot;
    Config *state;
    Config *errors;

    uint32_t meter_type = METER_TYPE_NONE;
    uint32_t value_index_power  = UINT32_MAX;
    uint32_t value_index_current[3] = {UINT32_MAX, UINT32_MAX, UINT32_MAX};

    bool meter_change_warning_printed = false;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
