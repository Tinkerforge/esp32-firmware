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

#include "modbus_tcp_meter.h"

#include <Arduino.h>
#include "esp_netif.h"

#include "api.h"
#include "task_scheduler.h"

const char *get_modbus_result_code_name(Modbus::ResultCode event)
{
    switch(event) {
        case 0x00: return "SUCCESS";
        case 0x01: return "ILLEGAL_FUNCTION";
        case 0x02: return "ILLEGAL_ADDRESS";
        case 0x03: return "ILLEGAL_VALUE";
        case 0x04: return "SLAVE_FAILURE";
        case 0x05: return "ACKNOWLEDGE";
        case 0x06: return "SLAVE_DEVICE_BUSY";
        case 0x08: return "MEMORY_PARITY_ERROR";
        case 0x0A: return "PATH_UNAVAILABLE";
        case 0x0B: return "DEVICE_FAILED_TO_RESPOND";
        case 0xE1: return "GENERAL_FAILURE";
        case 0xE2: return "DATA_MISMACH";
        case 0xE3: return "UNEXPECTED_RESPONSE";
        case 0xE4: return "TIMEOUT";
        case 0xE5: return "CONNECTION_LOST";
        case 0xE6: return "CANCEL";
        case 0xE7: return "PASSTHROUGH";
        case 0xE8: return "FORCE_PROCESS";
        default: return "Unkown error code";
    }
}

ModbusTcpMeter::ModbusTcpMeter() {}

void ModbusTcpMeter::pre_setup()
{
    Config *register_element = new Config{Config::Object({
        //{"role", Config::Uint16(0)},
        {"ra", Config::Uint32(0)},  // register address
        {"rt", Config::Uint8(0)},   // register type: input, holding, coil, discrete
        {"vt", Config::Uint8(0)},   // value type: uint16, int16, uint32, int32, etc...
        {"s", Config::Float(1.0)},  // scale
        {"o", Config::Float(0.0)},  // offset
    })};

    Config *config_element = new Config{Config::Object({
        {"enable", Config::Bool(false)},
        //{"display_name", Config::Str("", 0, 32)},
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"register_set", Config::Array(
            {},
            register_element,
            0,
            MODBUS_TCP_METER_REGISTER_COUNT_MAX,
            Config::type_id<Config::ConfObject>()
        )},
    })};

    config = Config::Array(
        {},
        config_element,
        MODBUS_TCP_METER_COUNT_MAX,
        MODBUS_TCP_METER_COUNT_MAX,
        Config::type_id<Config::ConfObject>()
    );

    for (int i = 0; i < MODBUS_TCP_METER_COUNT_MAX; i++) {
        config.add();
    }
}

void ModbusTcpMeter::setup()
{
    api.restorePersistentConfig("modbus_tcp_meter/config", &config);
    config_in_use = config;
/*
    task_scheduler.scheduleWithFixedDelay([this](){
        read_loop();
    }, 250, 250);*/

    mb.client();

    initialized = true;
}

void ModbusTcpMeter::register_urls()
{
    api.addPersistentConfig("modbus_tcp_meter/config", &config);
}

bool ModbusTcpMeter::check_event_read(Modbus::ResultCode event)
{
    in_progress = false;
    if (event != Modbus::ResultCode::EX_SUCCESS) {
        logger.printfln("Modbus TCP Meter Error %d: %s", event, get_modbus_result_code_name(event));
        return false;
    }

    return true;
}

void ModbusTcpMeter::next_register()
{
    current_register++;
    if (current_register >= config_in_use.get(current_meter)->get("register_set")->count()) {
        next_meter();
    }
}

void ModbusTcpMeter::next_meter()
{
    current_register = 0;
    current_meter++;
    if (current_meter >= MODBUS_TCP_METER_COUNT_MAX) {
        current_meter = 0;

        // Start new read deadline after we read all registers from all meters once
        read_deadline = millis();
        //kostal_test_print();
    }
}

uint8_t ModbusTcpMeter::get_length_from_type(const uint8_t value_type)
{
    if (value_type > 32 && value_type < 64) {
        return 2;
    } else if (value_type > 64) {
        return 4;
    }

    // Use 1 as default
    return 1;
}

void ModbusTcpMeter::zero_results()
{
    for (int i = 0; i < MODBUS_TCP_METER_REGISTER_LENGTH_MAX; i++) {
        result_in_progress_uint[i] = 0;
        result_in_progress_bool[i] = false;
    }
}

void ModbusTcpMeter::read_register(const char *host, const Config *register_config, uint16_t *result)
{
    // Set in_progress. We only read one register at a time
    in_progress = true;

    const uint8_t  register_type    = register_config->get("rt")->asUint();
    const uint32_t register_address = register_config->get("ra")->asUint();
    const uint8_t  value_type       = register_config->get("vt")->asUint();
    const uint8_t  register_length  = get_length_from_type(value_type);

    //logger.printfln("Read register host %s, type %u, address %u, length %u", host, register_type, register_address, register_length);

    zero_results();
    switch (register_type) {
        case MODBUS_TCP_REGISTER_TYPE_INPUT: {
            memset(result_in_progress_uint, 0, sizeof(uint16_t) * MODBUS_TCP_METER_REGISTER_LENGTH_MAX);
            mb.readIreg(host, register_address, result_in_progress_uint, register_length, [this, result](Modbus::ResultCode event, uint16_t tid, void* data)->bool {
                if(check_event_read(event)) {
                    for(int i = 0; i < MODBUS_TCP_METER_REGISTER_COUNT_MAX; i++) {
                        result[i] = result_in_progress_uint[i];
                    }
                    logger.printfln("Read input result %d %d %d %d", result[0], result[1], result[2], result[3]);
                }
                return true;
            });
            break;
        }

        case MODBUS_TCP_REGISTER_TYPE_HOLDING: {
            mb.readHreg(host, register_address, result_in_progress_uint, register_length, [this, result](Modbus::ResultCode event, uint16_t tid, void* data)->bool {
                if(check_event_read(event)) {
                    for(int i = 0; i < MODBUS_TCP_METER_REGISTER_COUNT_MAX; i++) {
                        result[i] = result_in_progress_uint[i];
                    }
                    //logger.printfln("Read holding result %d %d %d %d", result[0], result[1], result[2], result[3]);
                }
                return true;
            });
            break;
        }

        case MODBUS_TCP_REGISTER_TYPE_COIL: {
            mb.readCoil(host, register_address, result_in_progress_bool, register_length, [this, result](Modbus::ResultCode event, uint16_t tid, void* data)->bool {
                if(check_event_read(event)) {
                    for(int i = 0; i < MODBUS_TCP_METER_REGISTER_COUNT_MAX; i++) {
                        result[i] = result_in_progress_bool[i];
                    }
                    logger.printfln("Read coil result %d %d %d %d", result[0], result[1], result[2], result[3]);
                }
                return true;
            });
            break;
        }

        case MODBUS_TCP_REGISTER_TYPE_DISCRETE: {
            mb.readIsts(host, register_address, result_in_progress_bool, register_length, [this, result](Modbus::ResultCode event, uint16_t tid, void* data)->bool {
                if(check_event_read(event)) {
                    for(int i = 0; i < MODBUS_TCP_METER_REGISTER_COUNT_MAX; i++) {
                        result[i] = result_in_progress_bool[i];
                    }
                    logger.printfln("Read discrete result %d %d %d %d", result[0], result[1], result[2], result[3]);
                }
                return true;
            });

            break;
        }

        default: {
            logger.printfln("Unknown register type: %d", register_type);
            break;
        }
    }
}

void ModbusTcpMeter::loop()
{
    if (!in_progress && deadline_elapsed(read_deadline + 1000)) {
        if (config_in_use.count() == 0)
            return;

        const Config *meter = static_cast<const Config *>(config_in_use.get(current_meter));
        const char *host = meter->get("host")->asEphemeralCStr();

        // Ignore meter with empty host
        if ((host == nullptr) || (strlen(host) == 0)) {
            next_meter();
            return;
        }

        if (mb.isConnected(host)) { // Check if connection to Modbus slave is established
            const Config *register_config = (const Config *)meter->get("register_set")->get(current_register);
            read_register(host, register_config, results[current_meter][current_register]);
            next_register();
        } else {
            if (!mb.connect(host)) { // Try to connect if no connection
                logger.printfln("modbus_tcp_meter: Failed to connect to %s", host);
            }
        }
    }
    mb.task();
}

// We assume that after scale and offset is applied, every value fits in a float
#define AS_UINT16_ARR(x) ((uint16_t *)&(x))
float ModbusTcpMeter::get_value(const uint8_t meter_num, const uint8_t register_num)
{
    if (meter_num >= MODBUS_TCP_METER_COUNT_MAX) {
        return 0;
    }
    if (register_num >= MODBUS_TCP_METER_REGISTER_COUNT_MAX) {
        return 0;
    }

    Config *register_config = (Config*)config_in_use.get(current_meter)->get("register_set")->get(current_register);
    const uint8_t  value_type = register_config->get("vt")->asUint();
    const float    scale      = register_config->get("s")->asFloat();
    const float    offset     = register_config->get("o")->asFloat();

    float result = 0;

    // TODO: I am not sure of endianess between the 16 bit values, we may have to shift the indices around
    switch (value_type) {
        case MODBUS_TCP_VALUE_TYPE_UINT16: {
            uint16_t tmp = 0;
            AS_UINT16_ARR(tmp)[0] = results[meter_num][register_num][0];

            result = (tmp + offset) * scale;
            break;
        }

        case MODBUS_TCP_VALUE_TYPE_UINT32: {
            uint32_t tmp = 0;
            AS_UINT16_ARR(tmp)[0] = results[meter_num][register_num][0];
            AS_UINT16_ARR(tmp)[1] = results[meter_num][register_num][1];

            result = (tmp + offset) * scale;
            break;
        }

        case MODBUS_TCP_VALUE_TYPE_UINT64: {
            uint64_t tmp = 0;
            AS_UINT16_ARR(tmp)[0] = results[meter_num][register_num][0];
            AS_UINT16_ARR(tmp)[1] = results[meter_num][register_num][1];
            AS_UINT16_ARR(tmp)[2] = results[meter_num][register_num][2];
            AS_UINT16_ARR(tmp)[3] = results[meter_num][register_num][3];

            result = (tmp + offset) * scale;
            break;
        }

        case MODBUS_TCP_VALUE_TYPE_INT16: {
            int16_t tmp = 0;
            AS_UINT16_ARR(tmp)[0] = results[meter_num][register_num][0];

            result = (tmp + offset) * scale;
            break;
        }

        case MODBUS_TCP_VALUE_TYPE_INT32: {
            int32_t tmp = 0;
            AS_UINT16_ARR(tmp)[0] = results[meter_num][register_num][0];
            AS_UINT16_ARR(tmp)[1] = results[meter_num][register_num][1];

            result = (tmp + offset) * scale;
            break;
        }

        case MODBUS_TCP_VALUE_TYPE_INT64: {
            int64_t tmp = 0;
            AS_UINT16_ARR(tmp)[0] = results[meter_num][register_num][0];
            AS_UINT16_ARR(tmp)[1] = results[meter_num][register_num][1];
            AS_UINT16_ARR(tmp)[2] = results[meter_num][register_num][2];
            AS_UINT16_ARR(tmp)[3] = results[meter_num][register_num][3];

            result = (tmp + offset) * scale;
            break;
        }

        case MODBUS_TCP_VALUE_TYPE_FLOAT32: {
            float tmp = 0;
            AS_UINT16_ARR(tmp)[0] = results[meter_num][register_num][0];
            AS_UINT16_ARR(tmp)[1] = results[meter_num][register_num][1];

            result = (tmp + offset) * scale;
            break;
        }

        case MODBUS_TCP_VALUE_TYPE_FLOAT64: {
            double tmp = 0;
            AS_UINT16_ARR(tmp)[0] = results[meter_num][register_num][0];
            AS_UINT16_ARR(tmp)[1] = results[meter_num][register_num][1];
            AS_UINT16_ARR(tmp)[2] = results[meter_num][register_num][2];
            AS_UINT16_ARR(tmp)[3] = results[meter_num][register_num][3];

            result = (tmp + offset) * scale;
            break;
        }

        case MODBUS_TCP_VALUE_TYPE_BITFIELD16: {
            uint16_t tmp = 0;
            AS_UINT16_ARR(tmp)[0] = results[meter_num][register_num][0];

            result = (tmp + offset) * scale;
            break;
        }

        case MODBUS_TCP_VALUE_TYPE_BITFIELD32: {
            uint32_t tmp = 0;
            AS_UINT16_ARR(tmp)[0] = results[meter_num][register_num][0];
            AS_UINT16_ARR(tmp)[1] = results[meter_num][register_num][1];

            result = (tmp + offset) * scale;
            break;
        }

        default: {
            logger.printfln("Unknown value type: %d", value_type);
            return 0;
        }
    }

    return result;
}

// TODO: Testing only, remove me
void ModbusTcpMeter::kostal_test_print()
{
    logger.printfln("Meter 0, Register 0 -> Voltage L1 (V)  -> %f", get_value(0, 0));
    logger.printfln("Meter 0, Register 1 -> Power      (W)  -> %f", get_value(0, 1));
    logger.printfln("Meter 0, Register 2 -> Exported   (Wh) -> %f", get_value(0, 2));
}
