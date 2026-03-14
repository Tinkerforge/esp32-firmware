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

#include "meter_iso15118.h"
#include "generated/ev_data_protocol.enum.h"

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"

#include "gcc_warnings.h"

static const MeterValueID meter_iso15118_value_ids[METER_ISO15118_VALUE_COUNT] = {
    MeterValueID::StateOfCharge,
    MeterValueID::Capacity,
    MeterValueID::PowerDCChaDisSum,
};

MeterISO15118::MeterISO15118(uint32_t slot_, Config *state_, Config *errors_)
    : slot(slot_)
    , state(state_)
    , errors(errors_)
{
}

[[gnu::const]]
MeterClassID MeterISO15118::get_class() const
{
    return MeterClassID::ISO15118;
}

void MeterISO15118::setup(Config * /*config*/)
{
    meters.declare_value_ids(slot, meter_iso15118_value_ids, METER_ISO15118_VALUE_COUNT);
}

void MeterISO15118::update_all_values(float soc, float capacity, float power)
{
    float values[METER_ISO15118_VALUE_COUNT] = {
        soc,
        capacity,
        power
    };

    meters.update_all_values(slot, values);
}

void MeterISO15118::clear_all_values()
{
    float values[METER_ISO15118_VALUE_COUNT];
    for (size_t i = 0; i < METER_ISO15118_VALUE_COUNT; i++) {
        values[i] = NAN;
    }

    meters.update_all_values(slot, values);
    state->get("protocol")->updateEnum(EVDataProtocol::None);
}

void MeterISO15118::set_protocol(EVDataProtocol protocol)
{
    state->get("protocol")->updateEnum(protocol);
}
