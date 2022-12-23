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

#define IND_ACK 1001
#define IND_NACK 1002
#define IND_NAG 1003

class Users
{
public:
    Users(){}
    void pre_setup();
    void setup();
    void register_urls();
    void loop();

    uint8_t next_user_id();
    void rename_user(uint8_t user_id, const String &username, const String &display_name);
    void remove_from_username_file(uint8_t user_id);
    void search_next_free_user();

    #define TRIGGER_CHARGE_ANY 0
    #define TRIGGER_CHARGE_START 1
    #define TRIGGER_CHARGE_STOP 2
    bool trigger_charge_action(uint8_t user_id, uint8_t auth_type, Config::ConfVariant auth_info, int action = TRIGGER_CHARGE_ANY);

    void remove_username_file();

    bool initialized = false;

    ConfigRoot user_config;
    ConfigRoot add;
    ConfigRoot remove;
    ConfigRoot http_auth;
    ConfigRoot http_auth_update;

    bool start_charging(uint8_t user_id, uint16_t current_limit, uint8_t auth_type, Config::ConfVariant auth_info);
    bool stop_charging(uint8_t user_id, bool force);
    int16_t get_blink_state();
    void set_blink_state(int16_t state);

private:
    int16_t blink_state = -1;
};

void set_led(int16_t mode);