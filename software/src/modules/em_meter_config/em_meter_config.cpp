/* esp32-firmware
 * Copyright (C) 2022 Olaf Lüke <olaf@tinkerforge.com>
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

#include "em_meter_config.h"

//#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "modules.h"

void EMMeterConfig::pre_setup()
{
    // States
    config = Config::Object({
        {"meter_source", Config::Uint8(0)}    });
}

void EMMeterConfig::setup()
{
    api.restorePersistentConfig("energy_manager/meter_config", &config);

    config_in_use = config;

    if (config_in_use.get("meter_source")->asUint() == 100) {
        meter.updateMeterState(2, METER_TYPE_CUSTOM_BASIC);
    }

    initialized = true;
}

void EMMeterConfig::register_urls()
{
    api.addPersistentConfig("energy_manager/meter_config", &config, {}, 1000);
}
