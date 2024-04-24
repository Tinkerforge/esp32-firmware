/* esp32-firmware
 * Copyright (C) 2023 Thomas Hein
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

#include <map>
#include <math.h>

#include "modules/meters/imeter.h"
#include "modules/meters/meter_value_id.h"
#include "obis.h"

#if defined(__GNUC__)
  #pragma GCC diagnostic push
  #include "gcc_warnings.h"
  #pragma GCC diagnostic ignored "-Weffc++"
#endif

struct cmpMeterValueID
{
  bool operator()(MeterValueID left, MeterValueID right) const
  {
    return static_cast<uint16_t>(left) < static_cast<uint16_t>(right);
  }
};

class MeterSMA final : public IMeter
{
public:
  MeterSMA(uint32_t slot);

  MeterClassID get_class() const override;
  void setup(const Config &ephemeral_config) override;

  bool supports_power() override          {return true;}
  bool supports_energy_import() override  {return true;}
  bool supports_energy_imexsum() override {return true;}
  bool supports_energy_export() override  {return true;}
  bool supports_currents() override       {return true;}

private:
  void update_all_values();

  uint32_t _slot;
  std::map<MeterValueID, obis*, cmpMeterValueID> _values;
};

#if defined(__GNUC__)
  #pragma GCC diagnostic pop
#endif
