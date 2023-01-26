/* esp32-firmware
 * Copyright (C) 2022 Erik Fleckstein <erik@tinkerforge.com>
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
#include "ocpp/ChargePoint.h"

class Ocpp {
public:
    Ocpp(){}
    void pre_setup();
    void setup();
    void register_urls();
    void loop();

    void on_tag_seen(const char *tag_id);

    bool initialized = false;

    OcppChargePoint cp;

    void(*tag_seen_cb)(int32_t, const char *, void *) = nullptr;
    void *tag_seen_cb_user_data = nullptr;

    ConfigRoot config;
    ConfigRoot config_in_use;
    ConfigRoot state;
    ConfigRoot configuration;
    ConfigRoot change_configuration;
};
