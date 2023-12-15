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
#include "module.h"

#define CURRENT_POWER_SMOOTHING_SAMPLES     4

#define MODE_FAST                           0
#define MODE_OFF                            1
#define MODE_PV                             2
#define MODE_MIN_PV                         3
#define MODE_DO_NOTHING                     255

#define CLOUD_FILTER_OFF                    0
#define CLOUD_FILTER_LIGHT                  1
#define CLOUD_FILTER_MEDIUM                 2
#define CLOUD_FILTER_STRONG                 3

#define PHASE_SWITCHING_MIN                 0
#define PHASE_SWITCHING_AUTOMATIC           0
#define PHASE_SWITCHING_ALWAYS_1PHASE       1
#define PHASE_SWITCHING_ALWAYS_3PHASE       2
#define PHASE_SWITCHING_EXTERNAL_CONTROL    3
#define PHASE_SWITCHING_PV1P_FAST3P         4
#define PHASE_SWITCHING_MAX                 4

#define HYSTERESIS_MIN_TIME_MINUTES         5

class PowerManager : public IModule
{
public:
    PowerManager() {}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    //void register_events() override;
    //void loop() override;

    Config *       get_config_low_level_state() _ATTRIBUTE((const));
    const Config * get_config()                 _ATTRIBUTE((const));
    const Config * get_debug_config()           _ATTRIBUTE((const));
    Config *       get_config_charge_mode()     _ATTRIBUTE((const));

private:
    ConfigRoot low_level_state;
    ConfigRoot config;
    ConfigRoot debug_config;
    ConfigRoot charge_mode;
    ConfigRoot charge_mode_update;
};
