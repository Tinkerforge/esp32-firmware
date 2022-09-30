/* esp32-firmware
 * Copyright (C) 2022 Frederic Henrichs <frederic@tinkerforge.com>
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

#define MODBUS_TABLE_VERSION 1

struct holding_s
{
    uint32_t reboot;
    uint32_t enable_charging;
    uint32_t allowed_current;
};

struct input_s
{
    uint32_t table_version;
    uint32_t box_id;
    uint32_t iec_state;
    uint32_t charger_state;
    int32_t current_user;
    uint32_t start_time_min;
    uint32_t charging_time_sec;
    uint32_t max_current;
};


class ModbusTCP
{
    public:

        ModbusTCP() {};
        void pre_setup();
        void setup();
        void register_urls();
        void loop();

        bool initialized = false;

    private:

        ConfigRoot config;

        struct holding_s *holding_base;
        struct input_s *input_base;

};