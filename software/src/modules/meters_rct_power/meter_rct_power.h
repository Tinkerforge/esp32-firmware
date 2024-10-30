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

#include "modules/meters/imeter.h"
#include "modules/modbus_tcp_client/generic_tcp_client_pool_connector.h"
#include "rct_power_client.h"
#include "rct_power_client_pool.h"
#include "virtual_meter.enum.h"

class MeterRCTPower final : protected GenericTCPClientPoolConnector, public IMeter
{
public:
    MeterRCTPower(uint32_t slot_, Config *state_, Config *errors_, RCTPowerClientPool *pool) :
        GenericTCPClientPoolConnector("meter_rct_power", pool), slot(slot_), state(state_), errors(errors_) {}

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;
    void register_events() override;
    void pre_reboot() override;

    bool supports_power() override          {return true;}
    bool supports_energy_import() override  {return true;}
    bool supports_energy_export() override  {return true;}
    //bool supports_currents() override       {return true;}

private:
    void connect_callback() override;
    void disconnect_callback() override;
    void read_next();

    uint32_t slot;
    Config *state;
    Config *errors;
    VirtualMeter virtual_meter      = VirtualMeter::None;
    const RCTValueSpec *value_specs = nullptr;
    size_t value_specs_length       = 0;
    size_t value_specs_index        = 0;
    bool read_allowed               = false;
};
