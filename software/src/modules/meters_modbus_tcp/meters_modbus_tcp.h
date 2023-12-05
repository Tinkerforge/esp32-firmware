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

#include "ModbusTCP.h"

#include "config.h"
#include "modules/meters/imeter.h"
#include "modules/meters/meter_generator.h"
#include "modules/meters/meter_value_id.h"
#include "module.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

#define METERS_MODBUS_TCP_REGISTER_COUNT_MAX 26

class MetersModbusTCP final : public IModule, public MeterGenerator
{
public:
    enum class ValueType {
        Int16      =  0,
        Uint16     =  1,
        Bitfield   =  3,
        Int32      = 32,
        Uint32     = 33,
        Float32    = 34,
        Bitfield32 = 36,
        Int64      = 64,
        Uint64     = 65,
        float64    = 66,
    };

    struct HostInfo {
        const char *hostname;
        uint16_t port;
        uint8_t unit_address;
    };

    struct RegisterInfo {
        TAddress address;       // {type, address}, type: {COIL, ISTS, IREG, HREG, NONE}, address: uint16_t
        ValueType value_type;   // uint16, int16, uint32, int32, etc...
        float scale;
        float offset;
        MeterValueID value_id;
    };

    // for IModule
    void pre_setup() override;
    void setup() override;
    void loop() override;

    // for MeterGenerator
    MeterClassID get_class() const override _ATTRIBUTE((const));
    virtual IMeter *new_meter(uint32_t slot, Config *state, Config *errors) override;
    virtual const Config *get_config_prototype() override _ATTRIBUTE((const));
    virtual const Config *get_state_prototype()  override _ATTRIBUTE((const));
    virtual const Config *get_errors_prototype() override _ATTRIBUTE((const));

    ModbusTCP *get_modbus_tcp_handle() _ATTRIBUTE((const));

private:
    Config register_element;

    Config config_prototype;
    Config state_prototype;
    Config errors_prototype;

    ModbusTCP mb;

    uint32_t instance_count = 0;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
