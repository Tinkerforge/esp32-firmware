/* esp32-firmware
 * Copyright (C) 2023 Olaf Lüke <olaf@tinkerforge.com>
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

#define MODBUSIP_USE_DNS 1

#include <ModbusTCP.h>

#define MODBUS_TCP_METER_COUNT_MAX 4
#define MODBUS_TCP_METER_REGISTER_COUNT_MAX 10
#define MODBUS_TCP_METER_REGISTER_LENGTH_MAX 4

#define MODBUS_TCP_REGISTER_TYPE_INPUT 0
#define MODBUS_TCP_REGISTER_TYPE_HOLDING 1
#define MODBUS_TCP_REGISTER_TYPE_COIL 2
#define MODBUS_TCP_REGISTER_TYPE_DISCRETE 3

#define MODBUS_TCP_VALUE_TYPE_INT16 0
#define MODBUS_TCP_VALUE_TYPE_UINT16 1
#define MODBUS_TCP_VALUE_TYPE_BITFIELD16 3
#define MODBUS_TCP_VALUE_TYPE_INT32 32
#define MODBUS_TCP_VALUE_TYPE_UINT32 33
#define MODBUS_TCP_VALUE_TYPE_FLOAT32 34
#define MODBUS_TCP_VALUE_TYPE_BITFIELD32 36
#define MODBUS_TCP_VALUE_TYPE_INT64 64
#define MODBUS_TCP_VALUE_TYPE_UINT64 65
#define MODBUS_TCP_VALUE_TYPE_FLOAT64 66

class ModbusTcpMeter final : public IModule
{
public:
    ModbusTcpMeter();
    void pre_setup();
    void setup();
    void register_urls();
    void loop();
    bool check_event_read(Modbus::ResultCode event);
    void next_register();
    void next_meter();
    void zero_results();
    void read_register(const char *host, Config *register_config, uint16_t *result);
    uint8_t get_length_from_type(const uint8_t value_type);
    int64_t get_value(const uint8_t meter_num, const uint8_t register_num);

    void kostal_test_print();

private:
    ConfigRoot config;
    ConfigRoot config_in_use;
    ModbusTCP mb;
    uint16_t result_in_progress_uint[4];
    bool result_in_progress_bool[4];
    bool in_progress = false;
    uint8_t current_meter    = 0;
    uint8_t current_register = 0;
    uint32_t read_deadline = 0;

    uint16_t results[MODBUS_TCP_METER_COUNT_MAX][MODBUS_TCP_METER_REGISTER_COUNT_MAX][MODBUS_TCP_METER_REGISTER_LENGTH_MAX];

};
