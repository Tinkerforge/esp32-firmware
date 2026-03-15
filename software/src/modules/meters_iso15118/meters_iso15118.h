/* esp32-firmware
 * Copyright (C) 2026 Olaf Lüke <olaf@tinkerforge.com>
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
#include "modules/meters/imeter.h"
#include "modules/meters/imeter_generator.h"

class MeterISO15118;
enum class EVDataProtocol : uint8_t;

class MetersISO15118 final : public IModule, public IMeterGenerator
{
public:
    MetersISO15118(){}
    void pre_setup() override;

    [[gnu::const]] MeterClassID get_class() const override;
    IMeter *new_meter(uint32_t slot, Config *state, Config *errors) override;
    [[gnu::const]] const Config *get_config_prototype() override;
    [[gnu::const]] const Config *get_state_prototype() override;
    [[gnu::const]] const Config *get_errors_prototype() override;

    void update_values(float soc, float capacity, float power, EVDataProtocol protocol);
    void update_soc(float soc);

    void clear_values();

private:
    Config config_prototype;
    Config state_prototype;
    Config errors_prototype;

    MeterISO15118 *meter_instance = nullptr;
};
