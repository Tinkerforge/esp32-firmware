/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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
#include <TFTools/Micros.h>

#include "module.h"
#include "config.h"

class NTP final : public IModule
{
private:
    ConfigRoot config;
    ConfigRoot state;

    String ntp_server1;
    String ntp_server2;

    micros_t sync_expires_at = 0_us;

public:
    NTP(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

    void set_synced(bool synced);
    void set_api_time(struct timeval time);

    void time_synced_NTPThread();
};
