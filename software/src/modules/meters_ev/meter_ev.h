/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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

#include "config.h"
#include "modules/meters/imeter.h"
#include "modules/meters/meter_value_id.h"
#include "ev_data_protocol.enum.h"

#define METER_EV_VALUE_COUNT 13

class MeterEV final : public IMeter
{
public:
    MeterEV(uint32_t slot, Config *state, Config *errors);

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(Config *config) override;

    // Update all values at once (NAN values are handled by the meters module)
    void update_all_values(float soc, float target_soc, float min_soc, float max_soc,
                           float ev_max_voltage, float ev_max_current, float ev_max_power,
                           float ev_capacity, float ev_present_power,
                           float ev_energy_request, float ev_time_to_target_soc,
                           float ev_min_power, float ev_min_current);

    // Clear all values (set to NAN)
    void clear_all_values();

    // Set the active protocol
    void set_protocol(EVDataProtocol protocol);

private:
    uint32_t slot;
    Config *state;
    Config *errors;
};
