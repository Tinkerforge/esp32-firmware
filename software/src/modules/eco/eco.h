/* esp32-firmware
 * Copyright (C) 2024 Olaf Lüke <olaf@tinkerforge.com>
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

#include "module.h"
#include "config.h"
#include "departure.enum.h"

#include "modules/cm_networking/cm_networking_defs.h"

class Eco final : public IModule
{
public:
    enum class ChargeDecision : uint8_t {
        Normal = 0,
        Fast = 1,
    };

private:
    void update();

    ConfigRoot config;
    ConfigRoot charge_plan;
    ConfigRoot state;
    ConfigRoot state_chargers_prototype;

    size_t trace_buffer_index;
    micros_t last_seen_plug_in[MAX_CONTROLLED_CHARGERS];

    uint32_t last_charge_15m;
    ChargeDecision last_charge_decision;

    void disable_charge_plan();
    void set_chargers_state_chart_data(const uint8_t charger_id, bool *chart_input, const uint8_t chart_input_length, const bool use_default_chart);
    std::pair<uint8_t, uint32_t> get_end_time_1m(const Departure departure, const uint32_t desired_amount_1m, const uint32_t time_1m, const time_t save_time_1s);


public:
    Eco(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    ChargeDecision get_charge_decision(const uint8_t charger_id);

    ChargeDecision charge_decision[MAX_CONTROLLED_CHARGERS];
};
