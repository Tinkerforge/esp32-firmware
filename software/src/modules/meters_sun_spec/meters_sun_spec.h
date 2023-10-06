/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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
#include "modules/meters/meter_generator.h"
#include "module.h"
#include "lwip/ip_addr.h"
#include <ModbusTCP.h>
/*
#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif*/

class MetersSunSpec final : public IModule, public MeterGenerator
{
public:
    // for IModule
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;

    // for MeterGenerator
    MeterClassID get_class() const override _ATTRIBUTE((const));
    virtual IMeter *new_meter(uint32_t slot, Config *state, Config *config, Config *errors) override;
    virtual const Config *get_config_prototype() override _ATTRIBUTE((const));
    virtual const Config *get_state_prototype()  override _ATTRIBUTE((const));
    virtual const Config *get_errors_prototype() override _ATTRIBUTE((const));

//private:
    enum class DiscoveryState {
        Idle,
        Resolve,
        Resolving,
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

    uint16_t discovery_read_uint16();
    uint32_t discovery_read_uint32();
    float discovery_read_float32();
    void discovery_read_string(char *buffer, size_t length);
    void discovery_printfln(const char *fmt, ...) __attribute__((__format__(__printf__, 2, 3)));

    Config config_prototype;

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
    dns_gethostbyname_addrtype_lwip_ctx_async_data discovery_host_data;
    IPAddress discovery_host_address;
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

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
