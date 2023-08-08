/* warp-charger
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "config.h"

#include "module.h"

#define MODBUSIP_USE_DNS 1
//#define MODBUSIP_TIMEOUT 3

#include <ModbusTCP.h>

enum class DiscoveryState {
    Idle,
    Connect,
    Disconnect,
    NextDeviceAddress,
    NextBaseAddress,
    Read,
    Reading,
    ReadNext,
    ReadSunSpecID,
    ReadSunSpecIDDone,
    ReadCommonModelHeader,
    ReadCommonModelHeaderDone,
    ReadCommonModelBlock,
    ReadCommonModelBlockDone,
    ReadStandardModelHeader,
    ReadStandardModelHeaderDone,
    ReadInverter3PFloatModelBlock,
    ReadInverter3PFloatModelBlockDone,
    ReadACMeterW3PFloatModelBlock,
    ReadACMeterW3PFloatModelBlockDone,
};

class SunSpecMeter final : public IModule
{
public:
    SunSpecMeter(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;

private:
    uint16_t discovery_read_uint16();
    uint32_t discovery_read_uint32();
    float discovery_read_float32();
    void discovery_read_string(char *buffer, size_t length);
    void discovery_printfln(const char *fmt, ...) __attribute__((__format__(__printf__, 2, 3)));

    ModbusTCP modbus;
    ConfigRoot start_discovery;

    bool discovery_new = false;
    String discovery_new_host = "";
    uint16_t discovery_new_port = 0;
    uint8_t discovery_new_device_address = 0;
    uint8_t discovery_new_device_address_next = 0;

    bool discovery_log_idle = false;
    DiscoveryState discovery_state = DiscoveryState::Idle;
    String discovery_host = "";
    uint16_t discovery_port = 0;
    uint8_t discovery_device_address = 0;
    uint8_t discovery_device_address_next = 0;
    size_t discovery_base_address_index = 0;
    uint16_t discovery_read_address;
    size_t discovery_read_size;
    uint16_t discovery_read_buffer[124];
    uint32_t discovery_read_cookie = 0;
    size_t discovery_read_index;
    Modbus::ResultCode discovery_read_event;
    DiscoveryState discovery_read_state;
    uint16_t discovery_common_model_length;
    uint16_t discovery_standard_model_length;
};
