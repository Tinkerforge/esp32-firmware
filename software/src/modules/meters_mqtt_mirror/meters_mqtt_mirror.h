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

#include "meter_mqtt_mirror.h"

#include <stdint.h>

#include "config.h"
#include "modules/meters/imeter.h"
#include "modules/meters/meter_generator.h"
#include "module.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MetersMqttMirror final : public IModule, public MeterGenerator
{
public:
    // for IModule
    void pre_setup() override;

    // for MeterGenerator
    MeterClassID get_class() const override _ATTRIBUTE((const));
    virtual IMeter *new_meter(uint32_t slot, Config *state, Config *errors) override;
    virtual const Config *get_config_prototype() override _ATTRIBUTE((const));
    virtual const Config *get_state_prototype()  override _ATTRIBUTE((const));
    virtual const Config *get_errors_prototype() override _ATTRIBUTE((const));

private:
    Config config_prototype;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
