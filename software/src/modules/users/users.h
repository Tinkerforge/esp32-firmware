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

#include "module.h"
#include "config.h"

#define USERS_AUTH_TYPE_NONE 0
#define USERS_AUTH_TYPE_LOST 1
#define USERS_AUTH_TYPE_NFC 2
#define USERS_AUTH_TYPE_NFC_INJECTION 3

#define IND_ACK 1001
#define IND_NACK 1002
#define IND_NAG 1003

#define USERNAME_LENGTH 32
#define DISPLAY_NAME_LENGTH 32
#define USERNAME_ENTRY_LENGTH (USERNAME_LENGTH + DISPLAY_NAME_LENGTH)
#define MAX_PASSIVE_USERS 256

#if defined(BOARD_HAS_PSRAM)
#define MAX_ACTIVE_USERS 33
#else
#define MAX_ACTIVE_USERS 17
#endif

class Users final : public IModule
{
public:
    Users(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;

    uint8_t next_user_id();
    void rename_user(uint8_t user_id, const String &username, const String &display_name);
    void remove_from_username_file(uint8_t user_id);
    void search_next_free_user();
    size_t get_display_name(uint8_t user_id, char *ret_buf);
    bool is_user_configured(uint8_t user_id);

    #define TRIGGER_CHARGE_ANY 0
    #define TRIGGER_CHARGE_START 1
    #define TRIGGER_CHARGE_STOP 2
    bool trigger_charge_action(uint8_t user_id, uint8_t auth_type, Config::ConfVariant auth_info, int action, micros_t deadtime_post_stop, micros_t deadtime_post_start);

    void remove_username_file();

    ConfigRoot config;
    ConfigRoot add;
    ConfigRoot modify;
    ConfigRoot remove;
    ConfigRoot http_auth;
    ConfigRoot http_auth_update;

    bool start_charging(uint8_t user_id, uint16_t current_limit, uint8_t auth_type, Config::ConfVariant auth_info);
    bool stop_charging(uint8_t user_id, bool force, float meter_abs = 0);

    micros_t last_charge_action_triggered = 0_us;
};

void set_led(int16_t mode);
