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
#include <TFGenericTCPClient.h>

#include "modules/meters/imeter.h"
#include "modules/meters/meter_value_id.h"
#include "modules/modbus_tcp_client/generic_tcp_client_connector.h"
#include "virtual_meter.enum.h"

struct RCTValueSpec
{
    uint32_t id;
    float scale_factor;
};

class RCTPowerClient final : public TFGenericTCPClient
{
public:
    RCTPowerClient(uint32_t slot_) : slot(slot_) {}

    void setup(const Config &ephemeral_config);
    void read_next_value();

private:
    void close_hook() override;
    void tick_hook() override;
    bool receive_hook() override;

    uint32_t slot;
    VirtualMeter virtual_meter = VirtualMeter::None;
    const RCTValueSpec *value_specs = nullptr;
    size_t value_specs_length = 0;
    size_t value_specs_index = 0;
    bool wait_for_start = true;
    uint8_t last_received_byte = 0;
    uint8_t pending_response[12];
    size_t pending_response_used = 0;
    uint32_t bootloader_magic_number = 0;
    micros_t bootloader_last_detected = 0_s;
};

class MeterRCTPower final : protected GenericTCPClientConnector, public IMeter
{
public:
    MeterRCTPower(uint32_t slot) : GenericTCPClientConnector("meter_rct_power", &client, &shared_client), client(slot), shared_client(&client) {}

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(const Config &ephemeral_config) override;
    void register_events() override;
    void pre_reboot() override;

    bool supports_power() override          {return true;}
    bool supports_energy_import() override  {return true;}
    bool supports_energy_export() override  {return true;}
    //bool supports_currents() override       {return true;}

    void connect_callback() override;
    void disconnect_callback() override;

private:
    RCTPowerClient client;
    TFGenericTCPSharedClient shared_client;
};
