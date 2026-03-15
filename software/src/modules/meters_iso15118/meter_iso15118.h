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
#include "modules/meters/generated/meter_value_id.h"
#include "generated/ev_data_protocol.enum.h"

#define METER_ISO15118_VALUE_COUNT 3

class MeterISO15118 final : public IMeter
{
public:
    MeterISO15118(uint32_t slot, Config *state, Config *errors);

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(Config *config) override;

    void update_all_values(float soc, float capacity, float power);
    void update_soc(float soc);

    void clear_all_values();
    void set_protocol(EVDataProtocol protocol);

private:
    uint32_t slot;
    Config *state;
    Config *errors;
};
