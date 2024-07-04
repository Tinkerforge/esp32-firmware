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

#include <stdint.h>

#include "module.h"
#include "config.h"
#include "modules/event/event.h"
#include "bindings/bricklet_rs485.h"

#define MODBUS_METER_SIMULATOR_REGISTER_COUNT 196

class ModbusMeterSimulator final : public IModule
{
public:
    ModbusMeterSimulator(){}
    void pre_setup() override;
    void setup() override;
    void register_urls() override;
    void register_events() override;

private:
    void setupRS485();
    void checkRS485State();

    EventResult on_value_ids_change(const Config *value_ids);
    uint32_t register_address2cached_index(uint32_t register_address);

    void modbus_slave_write_multiple_registers_request_handler(uint8_t request_id, uint32_t starting_address, uint16_t *registers, uint16_t registers_length);
    void modbus_slave_read_holding_registers_request_handler(uint8_t request_id, uint32_t starting_address, uint16_t count);
    void modbus_slave_read_input_registers_request_handler(uint8_t request_id, uint32_t starting_address, uint16_t count);

    ConfigRoot config;

    TF_RS485 bricklet;

    uint16_t write_registers_callback_buffer[123];

    uint32_t source_meter_slot = UINT32_MAX;
    uint32_t value_index_cache[MODBUS_METER_SIMULATOR_REGISTER_COUNT];

    uint16_t meter_id = 0;
    float system_type = 3.0f;
};
