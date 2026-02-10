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
#include "ev_data_protocol.enum.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"

#include "gcc_warnings.h"

static const MeterValueID meter_iso15118_value_ids[METER_ISO15118_VALUE_COUNT] = {
    MeterValueID::EVSOCPresent,
    MeterValueID::EVSOCTarget,
    MeterValueID::EVSOCMin,
    MeterValueID::EVSOCMax,
    MeterValueID::EVVoltageMax,
    MeterValueID::EVCurrentMax,
    MeterValueID::EVPowerMax,
    MeterValueID::EVCapacity,
    MeterValueID::EVPowerPresent,
    MeterValueID::EVEnergyRequest,
    MeterValueID::EVTimeToTargetSOC,
    MeterValueID::EVPowerMin,
    MeterValueID::EVCurrentMin,
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

void MeterISO15118::update_all_values(float soc, float target_soc, float min_soc, float max_soc,
                                float ev_max_voltage, float ev_max_current, float ev_max_power,
                                float ev_capacity, float ev_present_power,
                                float ev_energy_request, float ev_time_to_target_soc,
                                float ev_min_power, float ev_min_current)
{
    float values[METER_ISO15118_VALUE_COUNT] = {
        soc,
        target_soc,
        min_soc,
        max_soc,
        ev_max_voltage,
        ev_max_current,
        ev_max_power,
        ev_capacity,
        ev_present_power,
        ev_energy_request,
        ev_time_to_target_soc,
        ev_min_power,
        ev_min_current
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
