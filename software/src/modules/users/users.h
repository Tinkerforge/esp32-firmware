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

#include "config.h"

class Users {
public:
    Users();
    void setup();
    void register_urls();
    void loop();

    uint8_t next_user_id();
    void create_user(const char *name);
    void get_username(uint8_t user_id, char *buf);
    void rename_user(uint8_t user_id, const char *name);

    bool trigger_charge_action(uint8_t user_id);

    bool initialized = false;

    ConfigRoot user_config;
    ConfigRoot add;
    ConfigRoot del;
    ConfigRoot http_auth;
    ConfigRoot http_auth_update;

    bool start_charging(uint8_t user_id, uint16_t current_limit);
    bool stop_charging(uint8_t user_id, bool force);
};
