/* esp32-firmware
 * Copyright (C) 2023 Erik Fleckstein <erik@tinkerforge.com>
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

#include "meter_rs485_bricklet.h"
#include "module_dependencies.h"

#include <stdint.h>

#include "config.h"
#include "modules/meters/imeter.h"
#include "modules/meters/meter_generator.h"
#include "module.h"

#include "bindings/bricklet_rs485.h"

#include "device_module.h"
#include "rs485_bricklet_firmware_bin.embedded.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    //#include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MetersRS485Bricklet final : public DeviceModule<TF_RS485,
                                                  rs485_bricklet_firmware_bin_data,
                                                  rs485_bricklet_firmware_bin_length,
                                                  tf_rs485_create,
                                                  tf_rs485_get_bootloader_mode,
                                                  tf_rs485_reset,
                                                  tf_rs485_destroy,
                                                  false>, public MeterGenerator
{
public:
    MetersRS485Bricklet() : DeviceModule("rs485", "RS485", "Modbus Meter", [this](){this->setupRS485();}) {}

    // for DeviceModule
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void loop() override;

    // for MeterGenerator
    MeterClassID get_class() const override _ATTRIBUTE((const));
    virtual IMeter *new_meter(uint32_t slot, Config *state, Config *errors) override;
    virtual const Config *get_config_prototype() override _ATTRIBUTE((const));
    virtual const Config *get_state_prototype()  override _ATTRIBUTE((const));
    virtual const Config *get_errors_prototype() override _ATTRIBUTE((const));

    void setupRS485();
    void checkRS485State();

private:
    ConfigRoot config_prototype;
    ConfigRoot state_prototype;
    ConfigRoot errors_prototype;
    ConfigRoot sdm630_reset_prototype;

    MeterRS485Bricklet *meter_instance = nullptr;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
