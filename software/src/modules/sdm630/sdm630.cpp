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

#include "sdm630.h"

#include "bindings/errors.h"

#include "api.h"
#include "event_log.h"
#include "tools.h"
#include "task_scheduler.h"
#include "web_server.h"
#include "modules.h"

extern EventLog logger;

extern TF_HAL hal;
extern WebServer server;
extern TaskScheduler task_scheduler;

extern API api;

// Maps to registers in SDM630Register
const uint16_t sdm630_registers_to_read[] = {
	1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39,41,43,47,49,53,57,61,63,67,71,73,75,77,79,81,83,85,87,101,103,105,107,201,203,205,207,225,235,237,239,241,243,245,249,251,259,261,263,265,267,269,335,337,339,341,343,345,347,349,351,353,355,357,359,361,363,365,367,369,371,373,375,377,379,381
};

const uint16_t sdm630_registers_fast_to_read[] = {
	53, 343, 7, 9, 11 // power, energy_abs, current_per_phase
};

#define ALL_VALUES_POWER 24
#define ALL_VALUES_ENERGY_ABS 65
#define ALL_VALUES_VOLTAGE_PHASE_0 0
#define ALL_VALUES_VOLTAGE_PHASE_1 1
#define ALL_VALUES_VOLTAGE_PHASE_2 2
#define ALL_VALUES_CURRENT_PHASE_0 3
#define ALL_VALUES_CURRENT_PHASE_1 4
#define ALL_VALUES_CURRENT_PHASE_2 5

#define PHASE_ACTIVE_CURRENT_THRES 0.3f // ampere
#define PHASE_CONNECTED_VOLTAGE_THRES 180.0f // volts

static uint16_t write_buf[400];
static uint16_t read_buf[400];
static float all_values[ALL_VALUES_COUNT];

void convert_to_all_values() {
    union {
        float f;
        uint16_t regs[2];
    } value;

    for(size_t i = 0; i < sizeof(sdm630_registers_to_read) / sizeof(sdm630_registers_to_read[0]); ++i) {
        size_t reg = sdm630_registers_to_read[i] - 1; // -1: convert from register to address
        value.regs[1] = read_buf[reg + 0];
        value.regs[0] = read_buf[reg + 1];
        all_values[i] = value.f;
    }
}

SDM630::SDM630() : DeviceModule("rs485", "RS485", "energy meter", std::bind(&SDM630::setupRS485, this))
{
    error_counters = Config::Object({
        {"meter", Config::Uint32(0)},
        {"bricklet", Config::Uint32(0)},
        {"bricklet_reset", Config::Uint32(0)},
    });

    user_data.expected_request_id = 0;
    user_data.value_to_write = nullptr;
    user_data.done = SDM630::UserDataDone::DONE;
}

void read_input_registers_handler(struct TF_RS485 *rs485, uint8_t request_id, int8_t exception_code, uint16_t *input_registers, uint16_t input_registers_length, void *user_data) {
    SDM630::UserData *ud = (SDM630::UserData *) user_data;

    if (request_id != ud->expected_request_id || ud->expected_request_id == 0) {
        logger.printfln("Unexpected request id %u, expected %u", request_id, ud->expected_request_id);
        ud->done = SDM630::UserDataDone::ERROR;
        return;
    }

    if (exception_code != 0) {
        logger.printfln("Request %u: Exception code %d", request_id, exception_code);
        ud->done = SDM630::UserDataDone::ERROR;
        return;
    }

    if (ud->value_to_write == nullptr) {
        logger.printfln("value to write was nullptr");
        ud->done = SDM630::UserDataDone::ERROR;
        return;
    }

    memcpy(ud->value_to_write, input_registers, input_registers_length * sizeof(uint16_t));

    //if (energy_meter.state.get("state")->asUint() == 0 && value.f == 0)
    //    energy_meter.updateMeterState(1, ENERGY_METER_TYPE_SDM630);

    if (energy_meter.state.get("state")->asUint() != 2)
        energy_meter.updateMeterState(2, ENERGY_METER_TYPE_SDM630);

    ud->done = SDM630::UserDataDone::DONE;
}

void write_multiple_registers_handler(struct TF_RS485 *device, uint8_t request_id, int8_t exception_code, void *user_data)
{
    SDM630::UserData *ud = (SDM630::UserData *)user_data;

    if (request_id != ud->expected_request_id || ud->expected_request_id == 0) {
        logger.printfln("Unexpected request id %u, expected %u", request_id, ud->expected_request_id);
        ud->done = SDM630::UserDataDone::ERROR;
        return;
    }

    if (exception_code != 0) {
        logger.printfln("Exception code %d", exception_code);
        ud->done = SDM630::UserDataDone::ERROR;
        return;
    }

    ud->done = SDM630::UserDataDone::DONE;
}

void SDM630::setupRS485()
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

    tf_rs485_register_modbus_master_read_input_registers_response_callback(&device, read_input_registers_handler, write_buf, &user_data);
    tf_rs485_register_modbus_master_write_multiple_registers_response_callback(&device, write_multiple_registers_handler, &user_data);

    initialized = true;
}

void SDM630::checkRS485State()
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

void SDM630::setup()
{
    setupRS485();
    if (!device_found)
        return;

    task_scheduler.scheduleWithFixedDelay([this](){
        this->checkRS485State();
    }, 5 * 60 * 1000, 5 * 60 * 1000);
}

void SDM630::register_urls()
{
    api.addState("meter/error_counters", &error_counters, {}, 1000);

    energy_meter.registerResetCallback([this](){
        this->reset_requested = true;
    });

    this->DeviceModule::register_urls();
}

void SDM630::loop()
{
    this->DeviceModule::loop();
    if (!initialized)
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

        user_data.done = UserDataDone::NOT_DONE;
        user_data.value_to_write = nullptr;
        user_data.expected_request_id = 0;

        uint16_t payload = 0x0003;
        is_in_bootloader(tf_rs485_modbus_master_write_multiple_registers(&device, 1, 61457, &payload, 1, &user_data.expected_request_id));
        if (user_data.expected_request_id == 0) {
            this->checkRS485State();
        }
        return;
    }

    uint16_t *to_write = nullptr;
    uint32_t start_address = 0;
    uint32_t num_to_read = 0;

    switch (modbus_read_state) {
        case 0:
            to_write = &read_buf[0];
            start_address = 1;
            num_to_read = 88;
            break;
        /*case 1:
            to_write = &read_buf[100];
            start_address = 101;
            num_to_read = 8;
            break;*/
        case 1:
            to_write = &read_buf[200];
            start_address = 201;
            num_to_read = 8;
            break;
        case 2:
            to_write = &read_buf[224];
            start_address = 225;
            num_to_read = 2;
            break;
        /*case 4:
            to_write = &read_buf[234];
            start_address = 235;
            num_to_read = 36;
            break;*/
        case 3:
            //to_write = &read_buf[334];
            //start_address = 335;
            to_write = &read_buf[342];
            start_address = 343;
            num_to_read = 40;
            break;
        default:
            break;
    }

    auto last_user_data_done = user_data.done;

    user_data.value_to_write = to_write;
    user_data.done = UserDataDone::NOT_DONE;
    user_data.expected_request_id = 0;
    is_in_bootloader(tf_rs485_modbus_master_read_input_registers(&device, 1, start_address, num_to_read, &user_data.expected_request_id));
    if (user_data.expected_request_id == 0) {
        logger.printfln("Failed to read energy meter registers starting at %u: request_id: %u", start_address, user_data.expected_request_id);
        this->checkRS485State();
    }

    if (modbus_read_state < 3)
        ++modbus_read_state;
    else {
        modbus_read_state = 0;

        if (last_user_data_done == UserDataDone::DONE) {
            // Try to measure each 500 ms, but don't pile up measurements
            // if we are already a complete slot behind.
            next_read_deadline_ms = next_read_deadline_ms + 500;
            logger.printfln("%u", next_read_deadline_ms - millis());
            if (deadline_elapsed(next_read_deadline_ms))
                next_read_deadline_ms = millis() + 500;

            convert_to_all_values();

            energy_meter.updateMeterValues(all_values[ALL_VALUES_POWER], all_values[ALL_VALUES_ENERGY_ABS], all_values[ALL_VALUES_ENERGY_ABS]);

            bool phases_active[3] = {
                all_values[ALL_VALUES_CURRENT_PHASE_0] > PHASE_ACTIVE_CURRENT_THRES,
                all_values[ALL_VALUES_CURRENT_PHASE_1] > PHASE_ACTIVE_CURRENT_THRES,
                all_values[ALL_VALUES_CURRENT_PHASE_2] > PHASE_ACTIVE_CURRENT_THRES
            };

            bool phases_connected[3] = {
                all_values[ALL_VALUES_VOLTAGE_PHASE_0] > PHASE_CONNECTED_VOLTAGE_THRES,
                all_values[ALL_VALUES_VOLTAGE_PHASE_1] > PHASE_CONNECTED_VOLTAGE_THRES,
                all_values[ALL_VALUES_VOLTAGE_PHASE_2] > PHASE_CONNECTED_VOLTAGE_THRES
            };

            energy_meter.updateMeterPhases(phases_connected, phases_active);
            energy_meter.updateMeterAllValues(all_values);
            logger.printfln("%u", next_read_deadline_ms - millis());
        } else if (last_user_data_done == UserDataDone::ERROR) {
            next_read_deadline_ms = millis() + 500;
            error_counters.get("meter")->updateUint(error_counters.get("meter")->asUint() + 1);
        } else {
            next_read_deadline_ms = millis() + 500;
            error_counters.get("bricklet")->updateUint(error_counters.get("bricklet")->asUint() + 1);
        }
    }

    // This protects against lost callback responses.
    // If the callback packet is lost,
    // user_data.done would never be set to ::DONE.
    callback_deadline_ms = millis() + 3000;
}
