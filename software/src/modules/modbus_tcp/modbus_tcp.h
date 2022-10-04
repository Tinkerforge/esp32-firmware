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

#pragma pack(push, 1)
struct meter_bitmap_s
{
    uint8_t phase_one_active:1;
    uint8_t phase_two_active:1;
    uint8_t phase_three_active:1;
    uint8_t phase_one_connected:1;
    uint8_t phase_two_connected:1;
    uint8_t phase_three_connected:1;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct feature_bitmap_s
{
    uint8_t evse:1;
    uint8_t meter:1;
    uint8_t meter_phases:1;
    uint8_t meter_all_values:1;
};
#pragma pack(pop)

struct holding_s
{
    uint32_t reboot;
};


struct input_s
{
    uint32_t table_version;
    uint32_t firmware_major;
    uint32_t firmware_minor;
    uint32_t firmware_patch;
    uint32_t firmware_build_ts;
    uint32_t box_id;
    uint32_t feature_bitmask;
    uint32_t uptime;
};

struct evse_s
{
    uint32_t iec_state;
    uint32_t charger_state;
    uint32_t current_user;
    uint32_t start_time_min;
    uint32_t charging_time_sec;
    uint32_t max_current;
    uint32_t slots[11];
};

struct meter_s
{
    uint32_t meter_type;
    float power;
    float energy_absolute;
    float energy_raltive;
};

struct meter_all_values_s
{
    float meter_values[85];
};

struct evse_holding_s
{
    uint32_t enable_charging;
    uint32_t allowed_current;
};

struct meter_holding_s
{
    uint32_t trigger_reset;
};


class ModbusTcp
{
    public:

        ModbusTcp() {};
        void pre_setup();
        void setup();
        void register_urls();
        void loop();

        bool initialized = false;

    private:

        ConfigRoot config;

        holding_s *holding_base;
        evse_holding_s *evse_holding_base;
        meter_holding_s *meter_holding_base;

        input_s *input_base;
        evse_s *evse_base;
        meter_s *meter_base;
        meter_all_values_s *meter_all_values_base;

        meter_bitmap_s *meter_phases;
        feature_bitmap_s *features;
};
