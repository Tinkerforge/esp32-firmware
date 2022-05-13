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

#include "modbus_reader.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"
#include "task_scheduler.h"
#include "web_server.h"
#include "modules.h"

#include "sdm630_defs.h"
#include "sdm72dmv2_defs.h"
#include "sdm72dm_defs.h"

extern EventLog logger;

extern TF_HAL hal;
extern WebServer server;
extern TaskScheduler task_scheduler;

extern API api;

static uint16_t write_buf[100];
static uint16_t registers[400];

static MeterInfo *supported_meters[] = {
    &sdm72dm,
    &sdm630,
    &sdm72dmv2
};

static MeterInfo *meter_in_use = nullptr;

ModbusReader::ModbusReader() : DeviceModule("rs485", "RS485", "energy meter", std::bind(&ModbusReader::setupRS485, this))
{
    error_counters = Config::Object({
        {"meter", Config::Uint32(0)},
        {"bricklet", Config::Uint32(0)},
        {"bricklet_reset", Config::Uint32(0)},
    });

    user_data.expected_request_id = 0;
    user_data.value_to_write = nullptr;
    user_data.done = ModbusReader::UserDataDone::DONE;
}

void read_meter_type_handler(struct TF_RS485 *rs485, uint8_t request_id, int8_t exception_code, uint16_t *holding_registers, uint16_t holding_registers_length, void *user_data) {
    ModbusReader::UserData *ud = (ModbusReader::UserData *) user_data;

    if (request_id != ud->expected_request_id || ud->expected_request_id == 0) {
        logger.printfln("Unexpected request id %u, expected %u", request_id, ud->expected_request_id);
        ud->done = ModbusReader::UserDataDone::ERROR;
        return;
    }

    if (exception_code != 0) {
        logger.printfln("Request %u: Exception code %d", request_id, exception_code);
        //ud->done = ModbusReader::UserDataDone::ERROR;
        return;
    }

    if (ud->value_to_write == nullptr) {
        logger.printfln("value to write was nullptr");
        ud->done = ModbusReader::UserDataDone::ERROR;
        return;
    }

    memcpy(ud->value_to_write, holding_registers, holding_registers_length * sizeof(uint16_t));

    ud->done = ModbusReader::UserDataDone::DONE;

    uint16_t meter_id = *ud->value_to_write;

    for(size_t i = 0; i < sizeof(supported_meters) / sizeof(supported_meters[0]); ++i) {
        if (meter_id != supported_meters[i]->meter_id)
            continue;

        meter_in_use = supported_meters[i];
        energy_meter.updateMeterState(2, meter_in_use->meter_type);
        logger.printfln("%s detected.", meter_in_use->meter_name);
        return;
    }

    logger.printfln("Found unknown meter type 0x%x. Assuming this is a SDM72DM.", meter_id);
    meter_in_use = supported_meters[0];
    energy_meter.updateMeterState(2, meter_in_use->meter_type);
}

void read_input_registers_handler(struct TF_RS485 *rs485, uint8_t request_id, int8_t exception_code, uint16_t *input_registers, uint16_t input_registers_length, void *user_data) {
    ModbusReader::UserData *ud = (ModbusReader::UserData *) user_data;

    if (request_id != ud->expected_request_id || ud->expected_request_id == 0) {
        logger.printfln("Unexpected request id %u, expected %u", request_id, ud->expected_request_id);
        ud->done = ModbusReader::UserDataDone::ERROR;
        return;
    }

    if (exception_code != 0) {
        logger.printfln("Request %u: Exception code %d", request_id, exception_code);
        ud->done = ModbusReader::UserDataDone::ERROR;
        return;
    }

    if (ud->value_to_write == nullptr) {
        logger.printfln("value to write was nullptr");
        ud->done = ModbusReader::UserDataDone::ERROR;
        return;
    }

    memcpy(ud->value_to_write, input_registers, input_registers_length * sizeof(uint16_t));

    //if (energy_meter.state.get("state")->asUint() == 0 && value.f == 0)
    //    energy_meter.updateMeterState(1);

    if (energy_meter.state.get("state")->asUint() != 2)
        energy_meter.updateMeterState(2);

    ud->done = ModbusReader::UserDataDone::DONE;
}

void write_multiple_registers_handler(struct TF_RS485 *device, uint8_t request_id, int8_t exception_code, void *user_data)
{
    ModbusReader::UserData *ud = (ModbusReader::UserData *)user_data;

    if (request_id != ud->expected_request_id || ud->expected_request_id == 0) {
        logger.printfln("Unexpected request id %u, expected %u", request_id, ud->expected_request_id);
        ud->done = ModbusReader::UserDataDone::ERROR;
        return;
    }

    if (exception_code != 0) {
        logger.printfln("Exception code %d", exception_code);
        ud->done = ModbusReader::UserDataDone::ERROR;
        return;
    }

    ud->done = ModbusReader::UserDataDone::DONE;
}

void ModbusReader::setupRS485()
{
    if (!this->DeviceModule::setup_device()) {
        return;
    }

    int result = tf_rs485_set_mode(&device, TF_RS485_MODE_MODBUS_MASTER_RTU);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("RS485 set mode failed (rc %d). Disabling energy meter support.", result);
        }
        return;
    }

    result = tf_rs485_set_rs485_configuration(&device, 9600, TF_RS485_PARITY_NONE, TF_RS485_STOPBITS_1, TF_RS485_WORDLENGTH_8, TF_RS485_DUPLEX_HALF);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("RS485 set config failed (rc %d). Disabling energy meter support.", result);
        }
        return;
    }

    result = tf_rs485_set_modbus_configuration(&device, 1, 1000);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("RS485 set modbus config failed (rc %d). Disabling energy meter support.", result);
        }
        return;
    }

    user_data.value_to_write = &this->meter_type;
    tf_rs485_register_modbus_master_read_holding_registers_response_callback(&device, read_meter_type_handler, &meter_type, &user_data);
    tf_rs485_modbus_master_read_holding_registers(&device, 1, 64515, 1, &user_data.expected_request_id);

    tf_rs485_register_modbus_master_read_input_registers_response_callback(&device, read_input_registers_handler, write_buf, &user_data);
    tf_rs485_register_modbus_master_write_multiple_registers_response_callback(&device, write_multiple_registers_handler, &user_data);

    initialized = true;
}

void ModbusReader::checkRS485State()
{
    uint8_t mode = 0;
    int result = tf_rs485_get_mode(&device, &mode);
    if (result != TF_E_OK) {
        if (!is_in_bootloader(result)) {
            logger.printfln("Failed to get RS485 mode, rc: %d", result);
            error_counters.get("bricklet")->updateUint(error_counters.get("bricklet")->asUint() + 1);
        }
        return;
    }
    if (mode != TF_RS485_MODE_MODBUS_MASTER_RTU) {
        logger.printfln("RS485 mode invalid (%u). Did the bricklet reset?", mode);
        error_counters.get("bricklet_reset")->updateUint(error_counters.get("bricklet_reset")->asUint() + 1);
        setupRS485();
    }
}

void ModbusReader::setup()
{
    setupRS485();
    if (!device_found)
        return;

    task_scheduler.scheduleWithFixedDelay([this](){
        this->checkRS485State();
    }, 5 * 60 * 1000, 5 * 60 * 1000);
}

void ModbusReader::register_urls()
{
    api.addState("meter/error_counters", &error_counters, {}, 1000);

    energy_meter.registerResetCallback([this]() {
        this->reset_requested = true;
    });

    this->DeviceModule::register_urls();
}

const RegRead *ModbusReader::getNextRead(bool *trigger_fast_read_done, bool *trigger_slow_read_done) {
    *trigger_fast_read_done = false;
    *trigger_slow_read_done = false;

    if (modbus_read_state_fast != meter_in_use->to_read_fast_len) {
        last_read_was_fast = true;
        const RegRead *result = &meter_in_use->to_read_fast[modbus_read_state_fast];
        ++modbus_read_state_fast;
        return result;
    }

    // fast state machine is done. do 1 slow read then run the fast state machine again.

    if (last_read_was_fast) {
        *trigger_fast_read_done = true;

        if (modbus_read_state_slow == meter_in_use->to_read_slow_len) {
            modbus_read_state_slow = 0;
        }
        if (meter_in_use->to_read_slow_len != 0) {
            last_read_was_fast = false;
            const RegRead *result = &meter_in_use->to_read_slow[modbus_read_state_slow];
            ++modbus_read_state_slow;
            return result;
        }
    }
    if (modbus_read_state_slow == meter_in_use->to_read_slow_len) {
        *trigger_slow_read_done = true;
    }

    modbus_read_state_fast = 0;
    last_read_was_fast = true;
    const RegRead *result = &meter_in_use->to_read_fast[modbus_read_state_fast];
    ++modbus_read_state_fast;
    return result;
}

void ModbusReader::loop()
{
    this->DeviceModule::loop();
    if (!initialized || meter_in_use == nullptr)
        return;

    if (user_data.done == UserDataDone::NOT_DONE && !deadline_elapsed(callback_deadline_ms))
        return;

    if (user_data.done == UserDataDone::NOT_DONE) {
        logger.printfln("rs485 deadline reached!");
        this->checkRS485State();
    }

    if (user_data.done != UserDataDone::NOT_DONE && !deadline_elapsed(next_read_deadline_ms))
        return;

    if (reset_requested) {
        reset_requested = false;

        if (meter_in_use->custom_reset_fn != nullptr){
            meter_in_use->custom_reset_fn();
        } else {
            user_data.done = UserDataDone::NOT_DONE;
            user_data.value_to_write = nullptr;
            user_data.expected_request_id = 0;

            uint16_t payload = 0x0003;
            is_in_bootloader(tf_rs485_modbus_master_write_multiple_registers(&device, 1, 61457, &payload, 1, &user_data.expected_request_id));
            if (user_data.expected_request_id == 0) {
                this->checkRS485State();
            }
        }
        return;
    }

    bool trigger_fast_read_done;
    bool trigger_slow_read_done;
    const RegRead *next_read = getNextRead(&trigger_fast_read_done, &trigger_slow_read_done);

    auto last_user_data_done = user_data.done;

    user_data.value_to_write = &registers[next_read->start - 1];
    user_data.done = UserDataDone::NOT_DONE;
    user_data.expected_request_id = 0;
    is_in_bootloader(tf_rs485_modbus_master_read_input_registers(&device, 1, next_read->start, next_read->len, &user_data.expected_request_id));
    if (user_data.expected_request_id == 0) {
        logger.printfln("Failed to read energy meter registers starting at %u: request_id: %u", next_read->start, user_data.expected_request_id);
        this->checkRS485State();
    }

    if (trigger_fast_read_done) {
        meter_in_use->fast_read_done_fn(registers);
    }

    if (trigger_slow_read_done) {
        meter_in_use->slow_read_done_fn(registers);
    }

    if (last_user_data_done == UserDataDone::DONE) {
        if (trigger_slow_read_done) {
            // Try to measure each 500 ms, but don't pile up measurements
            // if we are already a complete slot behind.
            next_read_deadline_ms = next_read_deadline_ms + 500;
            if (deadline_elapsed(next_read_deadline_ms))
                next_read_deadline_ms = millis() + 500;
        }
    } else if (last_user_data_done == UserDataDone::ERROR) {
        next_read_deadline_ms = millis() + 500;
        error_counters.get("meter")->updateUint(error_counters.get("meter")->asUint() + 1);
    } else {
        next_read_deadline_ms = millis() + 500;
        error_counters.get("bricklet")->updateUint(error_counters.get("bricklet")->asUint() + 1);
    }

    // This protects against lost callback responses.
    // If the callback packet is lost,
    // user_data.done would never be set to ::DONE.
    callback_deadline_ms = millis() + 3000;
}
