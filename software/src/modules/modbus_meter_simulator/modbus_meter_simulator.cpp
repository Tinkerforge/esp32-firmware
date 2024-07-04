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

#include "modbus_meter_simulator.h"

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "bindings/hal_common.h"
#include "modules/meters/sdm_helpers.h"
#include "tools.h"
#include "modules/meters_rs485_bricklet/meter_defs.h"

#include "gcc_warnings.h"

extern TF_HAL hal;

static const uint8_t addr2values[MODBUS_METER_SIMULATOR_REGISTER_COUNT] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,255,22,23,255,24,255,25,255,26,27,255,28,255,29,30,31,32,33,34,35,36,37,255,255,255,255,255,255,38,39,40,41,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,42,43,44,45,255,255,255,255,255,255,255,255,46,255,255,255,255,47,48,49,50,51,52,255,53,54,255,255,255,55,56,57,58,59,60,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,255,85,255,86,87};

static const uint16_t supported_meter_ids[] = {
    0,      // None
    0x0200, // SDM72
    0x0070, // SDM630
    0x0089, // SDM72v2
    0,      // SDM72CTM?
    0x0079, // SDM630MCTv2
};

static void convert_float_to_regs(uint16_t *regs, float floatval)
{
    union {
        float f;
        uint16_t regs[2];
    } value;

    value.f = floatval;

    regs[1] = value.regs[0];
    regs[0] = value.regs[1];
}

void ModbusMeterSimulator::pre_setup()
{
    config = Config::Object({
        {"meter_type",        Config::Uint8(METER_TYPE_NONE)},
        {"source_meter_slot", Config::Uint(0, 0, METERS_SLOTS - 1)},
    });
}

void ModbusMeterSimulator::setup()
{
    api.restorePersistentConfig("modbus_meter_simulator/config", &config);

    uint32_t meter_type = config.get("meter_type")->asUint();
    if (meter_type < ARRAY_SIZE(supported_meter_ids)) {
        meter_id = supported_meter_ids[meter_type];
    }

    if (meter_id == 0) {
        logger.printfln("Unsupported meter type %u. Disabling simulator.", meter_type);
        initialized = true; // Mark as initialized so that the front-end shows up.
        return;
    }

    source_meter_slot = config.get("source_meter_slot")->asUint();

    for (size_t i = 0; i < ARRAY_SIZE(value_index_cache); i++) {
        value_index_cache[i] = UINT32_MAX;
    }

    setupRS485();
    if (!initialized)
        return;

    task_scheduler.scheduleWithFixedDelay([this](){
        this->checkRS485State();
    }, 10 * 1000, 10 * 1000);
}

void ModbusMeterSimulator::register_urls()
{
    api.addPersistentConfig("modbus_meter_simulator/config", &config);
}

void ModbusMeterSimulator::register_events()
{
    if (source_meter_slot >= METERS_SLOTS)
        return;

    String value_ids_path = meters.get_path(source_meter_slot, Meters::PathType::ValueIDs);

    const Config *old_value_ids = api.getState(value_ids_path);
    if (old_value_ids->count() > 0) {
        on_value_ids_change(old_value_ids);
    } else {
        event.registerEvent(value_ids_path, {}, [this](const Config *event_value_ids) {
            return on_value_ids_change(event_value_ids);
        });
    }
}

void ModbusMeterSimulator::setupRS485()
{
    if (tf_rs485_create(&bricklet, nullptr, &hal) != TF_E_OK) {
        logger.printfln("No RS485 bricklet found. Disabling simulator.");
        return;
    }

    int result = tf_rs485_set_mode(&bricklet, TF_RS485_MODE_MODBUS_SLAVE_RTU);
    if (result != TF_E_OK) {
        logger.printfln("RS485 set mode failed (rc %d). Disabling simulator.", result);
        return;
    }

    result = tf_rs485_set_rs485_configuration(&bricklet, 9600, TF_RS485_PARITY_NONE, TF_RS485_STOPBITS_1, TF_RS485_WORDLENGTH_8, TF_RS485_DUPLEX_HALF);
    if (result != TF_E_OK) {
        logger.printfln("RS485 set config failed (rc %d). Disabling simulator.", result);
        return;
    }

    result = tf_rs485_set_modbus_configuration(&bricklet, 1, 0);
    if (result != TF_E_OK) {
        logger.printfln("RS485 set modbus config failed (rc %d). Disabling simulator.", result);
        return;
    }

    // Used callbacks
    tf_rs485_register_modbus_slave_write_multiple_registers_request_callback(&bricklet, [](struct TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t *registers, uint16_t registers_length, void *user_data) {
        task_scheduler.scheduleOnce([request_id, starting_address, registers, registers_length, user_data]() {
            static_cast<ModbusMeterSimulator *>(user_data)->modbus_slave_write_multiple_registers_request_handler(request_id, starting_address, registers, registers_length);
        }, 0);
    }, write_registers_callback_buffer, this);

    tf_rs485_register_modbus_slave_read_holding_registers_request_callback(&bricklet, [](TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t count, void *user_data) {
        task_scheduler.scheduleOnce([request_id, starting_address, count, user_data]() {
            static_cast<ModbusMeterSimulator *>(user_data)->modbus_slave_read_holding_registers_request_handler(request_id, starting_address, count);
        }, 0);
    }, this);

    tf_rs485_register_modbus_slave_read_input_registers_request_callback(&bricklet, [](TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t count, void *user_data) {
        task_scheduler.scheduleOnce([request_id, starting_address, count, user_data]() {
            static_cast<ModbusMeterSimulator *>(user_data)->modbus_slave_read_input_registers_request_handler(request_id, starting_address, count);
        }, 0);
    }, this);

    // Unused callbacks
    tf_rs485_register_modbus_slave_write_single_coil_request_callback(&bricklet, [](TF_RS485 *rs485, uint8_t request_id, uint32_t coil_address, bool coil_value, void *user_data) {
        logger.printfln("No handler implementation for request 'write single coil' with coil address %u", coil_address);
    }, this);

    tf_rs485_register_modbus_slave_write_multiple_coils_request_low_level_callback(&bricklet, [](struct TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t coils_length, uint16_t coils_chunk_offset, bool coils_chunk_data[440], void *user_data) {
        logger.printfln("No handler implementation for request 'write multiple coils' with starting address %u", starting_address);
    }, this);

    tf_rs485_register_modbus_slave_write_single_register_request_callback(&bricklet, [](TF_RS485 *device, uint8_t request_id, uint32_t register_address, uint16_t register_value, void *user_data) {
        logger.printfln("No handler implementation for request 'write single register' with register address %u", register_address);
    }, this);

    tf_rs485_register_modbus_slave_read_coils_request_callback(&bricklet, [](TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t count, void *user_data) {
        logger.printfln("No handler implementation for request 'read coils' with starting address %u", starting_address);
    }, this);

    tf_rs485_register_modbus_slave_read_discrete_inputs_request_callback(&bricklet, [](TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t count, void *user_data) {
        logger.printfln("No handler implementation for request 'read discrete inputs' with starting address %u", starting_address);
    }, this);

    initialized = true;
}

void ModbusMeterSimulator::checkRS485State()
{
    uint8_t mode = 0;
    int result = tf_rs485_get_mode(&bricklet, &mode);
    if (result != TF_E_OK) {
        logger.printfln("Failed to get RS485 mode, rc: %d", result);
        return;
    }
    if (mode != TF_RS485_MODE_MODBUS_SLAVE_RTU) {
        logger.printfln("RS485 mode invalid (%u). Did the bricklet reset?", mode);
        setupRS485();
    }
}

EventResult ModbusMeterSimulator::on_value_ids_change(const Config *value_ids)
{
    if (value_ids->count() == 0) {
        return EventResult::OK;
    }

    uint32_t index_cache_compact[ARRAY_SIZE(sdm_helper_all_ids)];
    meters.fill_index_cache(source_meter_slot, ARRAY_SIZE(sdm_helper_all_ids), sdm_helper_all_ids, index_cache_compact);

    for (size_t i = 0; i < MODBUS_METER_SIMULATOR_REGISTER_COUNT; i++) {
        uint32_t compact_index = addr2values[i];
        if (compact_index >= ARRAY_SIZE(index_cache_compact)) {
            value_index_cache[i] = UINT32_MAX;
        } else {
            value_index_cache[i] = index_cache_compact[compact_index];
        }
    }

    return EventResult::Deregister;
}

uint32_t ModbusMeterSimulator::register_address2cached_index(uint32_t register_address)
{
    if (register_address == 0)
        return UINT32_MAX;

    uint32_t index = (register_address - 1) / 2;
    if (index >= ARRAY_SIZE(value_index_cache))
        return UINT32_MAX;

    return value_index_cache[index];
}

void ModbusMeterSimulator::modbus_slave_write_multiple_registers_request_handler(uint8_t request_id, uint32_t starting_address, uint16_t *registers, uint16_t registers_length)
{
    if ((starting_address == 15 || starting_address == 25) || registers_length == 2) {
        logger.printfln("Tried to set password (ignored)");
    } else if (starting_address == 11 && registers_length == 2) {
        uint16_t indices[1] = {1};
        float float_val = NAN;
        convert_to_float(registers, &float_val, indices, 1);
        logger.printfln("Setting system type to %f", static_cast<double>(float_val));
        system_type = float_val;
    } else {
        logger.printfln("Received unexpected write_multiple_registers request to address %u with length %u", starting_address, registers_length);
    }
    tf_rs485_modbus_slave_answer_write_multiple_registers_request(&bricklet, request_id);
}

void ModbusMeterSimulator::modbus_slave_read_holding_registers_request_handler(uint8_t request_id, uint32_t starting_address, uint16_t count)
{
    uint16_t regs[2];

    if (starting_address == 11 && count == 2) {
        //logger.printfln("Read system type");
        convert_float_to_regs(regs, system_type);
    } else if (starting_address == 64515 && count == 1) {
        logger.printfln("Master reading meter ID");
        regs[0] = meter_id;
    } else {
        logger.printfln("Received unexpected read_holding_registers request to address %u with length %u", starting_address, count);
        return;
    }

    int rc = tf_rs485_modbus_slave_answer_read_holding_registers_request(&bricklet, request_id, regs, count);
    if (rc != TF_E_OK)
        logger.printfln("Answering read holding registers request failed with code %i", rc);
}

void ModbusMeterSimulator::modbus_slave_read_input_registers_request_handler(uint8_t request_id, uint32_t starting_address, uint16_t count)
{
    uint16_t regs[125];

    if (count > ARRAY_SIZE(regs)) {
        logger.printfln("Received unexpected read_input_registers request to address %u with length %u", starting_address, count);
        return;
    }

    for (uint32_t offset = 0; offset < count; offset += 2) {
        uint32_t cached_index = register_address2cached_index(starting_address + offset);
        float float_val;
        meters.get_value_by_index(source_meter_slot, cached_index, &float_val);
        convert_float_to_regs(regs + offset, float_val);
    }

    int rc = tf_rs485_modbus_slave_answer_read_input_registers_request(&bricklet, request_id, regs, count);
    if (rc != TF_E_OK)
        logger.printfln("Answering read input registers request failed with code %i", rc);
}
