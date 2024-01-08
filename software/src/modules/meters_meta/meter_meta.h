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

#include "config.h"
#include "modules/event/event.h"
#include "modules/meters/imeter.h"

#if defined(__GNUC__)
    #pragma GCC diagnostic push
    #include "gcc_warnings.h"
    #pragma GCC diagnostic ignored "-Weffc++"
#endif

class MeterMeta final : public IMeter
{
public:
    enum class ConfigMode {
        Sum  = 0,
        Diff = 1,
        Add  = 2,
        Mul  = 3,
    };

    MeterMeta(uint32_t slot_) : slot(slot_) {}

    [[gnu::const]] MeterClassID get_class() const override;
    void setup(Config &ephemeral_config) override;

    bool supports_power()         override {return true;}
    bool supports_energy_import() override {return true;}
    bool supports_energy_imexsum()override {return true;}
    bool supports_energy_export() override {return true;}
    bool supports_currents()      override {return true;}

    void register_events();

    EventResult on_value_ids_change(const Config *value_ids);
    void on_values_change();
    void on_values_change_task();

private:
    uint32_t slot;

    // Cached config
    ConfigMode mode;
    uint32_t source_meter_a;
    uint32_t source_meter_b;
    int32_t  constant;

    size_t value_count = 0;
    uint8_t (* value_indices)[][2];
    bool update_pending = false;
};

#if defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif
