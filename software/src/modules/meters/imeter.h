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
#include <WString.h>

#include "config.h"
#include "meter_class_id.enum.h"

class IMeter
{
public:
    virtual ~IMeter() = default;

    virtual MeterClassID get_class() const = 0;
    virtual void setup(const Config &ephemeral_config) {}
    virtual void register_urls(const String &base_url) {}
    virtual void pre_reboot() {}

    virtual bool supports_power()         {return false;}
    virtual bool supports_energy_import() {return false;}
    virtual bool supports_energy_imexsum(){return false;}
    virtual bool supports_energy_export() {return false;}
    virtual bool supports_currents()      {return false;}
    //virtual bool supports_phases() {return false;}

    // Should be true iff the meter has [...]Resettable values
    virtual bool supports_reset()         {return false;}
    // Should reset _all_ [...]Resettable values.
    // Return false if the reset failed.
    virtual bool reset()                  {return true; }
};
