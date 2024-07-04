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

#include <stdint.h>

#include "module.h"
#include "config.h"
#include "modules/event/event.h"
#include "modules/meters/meter_defs.h"

#define PHASE_CONNECTED_VOLTAGE_THRES 180.0f // V
#define PHASE_ACTIVE_CURRENT_THRES      0.3f // A

class MetersLegacyAPI final : public IModule
{
public:
    MetersLegacyAPI(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    uint32_t get_linked_meter_slot();

private:
    EventResult on_value_ids_change(const Config *value_ids);
    void on_values_change(const Config *values);
    void on_last_reset_change(const Config *last_reset);

    ConfigRoot state;
    ConfigRoot config;

    // For old meter
    ConfigRoot legacy_state;
    ConfigRoot legacy_values;
    ConfigRoot legacy_phases;
    ConfigRoot legacy_all_values;
    ConfigRoot legacy_last_reset;

    // Writable end-points for old API meter
    ConfigRoot legacy_state_update;
    ConfigRoot legacy_values_update;
    ConfigRoot legacy_phases_update;
    ConfigRoot legacy_all_values_update;

    uint32_t linked_meter_slot = UINT32_MAX;
    size_t   linked_meter_value_count = 0;
    uint16_t value_indices_legacy_all_values_to_linked_meter[METER_ALL_VALUES_LEGACY_COUNT];
    uint16_t value_indices_legacy_values_to_linked_meter[METER_VALUES_LEGACY_COUNT];
    bool     legacy_api_enabled    = false;
    bool     has_all_values        = false;
    bool     has_phases            = false;
    bool     meter_setup_done      = false;
    bool     meter_writable        = false;
    bool     phases_overridden     = false;
    bool     show_blank_value_id_update_warnings = false;
};
