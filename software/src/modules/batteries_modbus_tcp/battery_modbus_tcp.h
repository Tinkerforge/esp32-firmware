/* esp32-firmware
 * Copyright (C) 2024 Matthias Bolte <matthias@tinkerforge.com>
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
#include <TFModbusTCPClientPool.h>

#include "config.h"
#include "batteries_modbus_tcp.h"
#include "battery_modbus_tcp_table_id.enum.h"
#include "modules/batteries/ibattery.h"
#include "modules/modbus_tcp_client/generic_tcp_client_pool_connector.h"

class BatteryModbusTCP final : protected GenericTCPClientPoolConnector, public IBattery
{
public:
    BatteryModbusTCP(uint32_t slot_, Config *state_, Config *errors_, TFModbusTCPClientPool *pool_) :
        GenericTCPClientPoolConnector("batteries_mbtcp", format_battery_slot(slot_), pool_), slot(slot_), state(state_), errors(errors_) {}

    [[gnu::const]] BatteryClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;
    void register_events() override;
    void pre_reboot() override;

    bool supports_action(Action action) override;
    bool start_action(Action action) override;
    bool get_current_action(Action *action) override;

private:
    void connect_callback() override;
    void disconnect_callback() override;
    void write_next();

    uint32_t slot;
    Config *state;
    Config *errors;

    BatteryModbusTCPTableID table_id;
    const BatteriesModbusTCP::ValueTable *tables[4] = {nullptr, nullptr, nullptr, nullptr};

    bool has_current_action = false;
    Action current_action;
    size_t current_action_index;

    // custom
    BatteriesModbusTCP::ValueTable *custom_table_permit_grid_charge;
    BatteriesModbusTCP::ValueTable *custom_table_revoke_grid_charge_override;
    BatteriesModbusTCP::ValueTable *custom_table_forbid_discharge;
    BatteriesModbusTCP::ValueTable *custom_table_revoke_discharge_override;
};
