/* warp-charger
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

#include "sun_spec_meter.h"

#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "module_dependencies.h"
#include "TFJson.h"

#define MAX_READ_CHUNK_SIZE 10
#define LAST_DEVICE_ADDRESS 247
#define SUN_SPEC_ID 0x53756E53
#define COMMON_MODEL_ID 1
//#define INVERTER_1P_INT_MODEL_ID 101
//#define INVERTER_SP_INT_MODEL_ID 102
//#define INVERTER_3P_INT_MODEL_ID 103
//#define INVERTER_1P_FLOAT_MODEL_ID 111
//#define INVERTER_SP_FLOAT_MODEL_ID 112
#define INVERTER_3P_FLOAT_MODEL_ID 113
#define INVERTER_3P_FLOAT_BLOCK_LENGTH 60
//#define AC_METER_1P_INT_MODEL_ID 201
//#define AC_METER_SP_INT_MODEL_ID 202
//#define AC_METER_W3P_INT_MODEL_ID 203
//#define AC_METER_D3P_INT_MODEL_ID 204
//#define AC_METER_1P_FLOAT_MODEL_ID 211
//#define AC_METER_SP_FLOAT_MODEL_ID 212
#define AC_METER_W3P_FLOAT_MODEL_ID 213
#define AC_METER_W3P_FLOAT_BLOCK_LENGTH 124
//#define AC_METER_D3P_FLOAT_MODEL_ID 214
#define NON_IMPLEMENTED_UINT16 0xFFFF
#define NON_IMPLEMENTED_UINT32 0xFFFFFFFF

static const uint16_t base_addresses[] {
    40000,
    50000,
    0
};

static const char *get_modbus_event_name(Modbus::ResultCode event)
{
    switch (event) {
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
    default:   return "unkown error code";
    }
}

void SunSpecMeter::pre_setup()
{
    start_discovery = ConfigRoot{Config::Object({
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(0)},
        {"device_address", Config::Uint8(0)},
    })};
}

void SunSpecMeter::setup()
{
    modbus.client();

    initialized = true;
}

void SunSpecMeter::register_urls()
{
    api.addCommand("sun_spec_meter/start_discovery", &start_discovery, {}, [this](){
        discovery_new = true;
        discovery_new_host = start_discovery.get("host")->asString();
        discovery_new_port = start_discovery.get("port")->asUint();

        uint8_t device_address = start_discovery.get("device_address")->asUint();

        if (device_address == 0) {
            discovery_new_device_address = 1;
            discovery_new_device_address_next = 2;
        } else {
            discovery_new_device_address = device_address;
            discovery_new_device_address_next = device_address;
        }
    }, true);
}

void SunSpecMeter::loop()
{
    if (discovery_new && discovery_state != DiscoveryState::Idle) {
        discovery_state = DiscoveryState::Disconnect;
    }

    switch (discovery_state) {
    case DiscoveryState::Idle:
        if (!discovery_log_idle) {
            if (!ws.pushRawStateUpdate("", "sun_spec_meter/discovery_log")) {
                break; // need to idle the log before doing something else
            }

            discovery_log_idle = true;
        }

        if (discovery_new) {
            if (!ws.pushRawStateUpdate("\"<<<clear_discovery_log>>>\"", "sun_spec_meter/discovery_log")) {
                break; // need to clear the log before doing something else
            }

            discovery_printfln("Starting discovery");

            discovery_state = DiscoveryState::Connect;
            discovery_host = discovery_new_host;
            discovery_port = discovery_new_port;
            discovery_device_address = discovery_new_device_address;
            discovery_device_address_next = discovery_new_device_address_next;
            discovery_base_address_index = 0;
            ++discovery_read_cookie;

            discovery_new = false;
            discovery_new_host = "";
            discovery_new_port = 0;
            discovery_new_device_address = 0;
            discovery_new_device_address_next = 0;
        }

        break;

    case DiscoveryState::Connect:
        discovery_printfln("Connecting to %s:%u", discovery_host.c_str(), discovery_port);

        if (!modbus.connect(discovery_host, discovery_port)) {
            discovery_printfln("Could not connect to %s:%u", discovery_host.c_str(), discovery_port);

            discovery_state = DiscoveryState::Idle;
            ++discovery_read_cookie;
        }
        else {
            discovery_state = DiscoveryState::ReadSunSpecID;
        }

        break;

    case DiscoveryState::Disconnect:
        discovery_printfln("Disconnecting from %s", discovery_host.c_str());

        if (!modbus.disconnect(discovery_host)) {
            discovery_printfln("Could not disconnect from %s", discovery_host.c_str());
        }

        discovery_state = DiscoveryState::Idle;
        ++discovery_read_cookie;

        break;

    case DiscoveryState::NextDeviceAddress:
        discovery_base_address_index = 0;

        if (discovery_device_address == discovery_device_address_next) {
            discovery_state = DiscoveryState::Disconnect;
        }
        else {
            discovery_device_address = discovery_device_address_next;

            if (discovery_device_address_next < LAST_DEVICE_ADDRESS) {
                ++discovery_device_address_next;
            }

            discovery_state = DiscoveryState::ReadSunSpecID;
        }

        break;

    case DiscoveryState::NextBaseAddress:
        ++discovery_base_address_index;

        if (discovery_base_address_index >= ARRAY_SIZE(base_addresses)) {
            discovery_state = DiscoveryState::NextDeviceAddress;
        }
        else {
            discovery_state = DiscoveryState::ReadSunSpecID;
        }

        break;

    case DiscoveryState::Read:
        ++discovery_read_cookie;
        discovery_read_index = 0;
        discovery_state = DiscoveryState::ReadNext;

        break;

    case DiscoveryState::ReadNext: {
            uint32_t cookie = discovery_read_cookie;
            size_t read_chunk_size = MIN(discovery_read_size - discovery_read_index, MAX_READ_CHUNK_SIZE);

            discovery_state = DiscoveryState::Reading;

            modbus.readHreg(discovery_host, discovery_read_address, &discovery_read_buffer[discovery_read_index], read_chunk_size,
            [this, cookie, read_chunk_size](Modbus::ResultCode event, uint16_t transactionId, void *data) -> bool {
                if (discovery_state != DiscoveryState::Reading || cookie != discovery_read_cookie) {
                    return true;
                }

                discovery_read_address += read_chunk_size;
                discovery_read_index += read_chunk_size;
                discovery_read_event = event;

                if (discovery_read_event != Modbus::ResultCode::EX_SUCCESS || discovery_read_index >= discovery_read_size) {
                    discovery_read_index = 0;
                    discovery_state = discovery_read_state;
                }
                else {
                    discovery_state = DiscoveryState::ReadNext;
                }

                return true;
            }, discovery_device_address);
        }

        break;

    case DiscoveryState::Reading:
        break;

    case DiscoveryState::ReadSunSpecID:
        discovery_printfln("Using device address %u", discovery_device_address);
        discovery_printfln("Using base address %u", base_addresses[discovery_base_address_index]);
        discovery_printfln("Reading SunSpec ID");

        discovery_read_address = base_addresses[discovery_base_address_index];
        discovery_read_size = 2;
        discovery_read_state = DiscoveryState::ReadSunSpecIDDone;
        discovery_state = DiscoveryState::Read;

        break;

    case DiscoveryState::ReadSunSpecIDDone:
        if (discovery_read_event == Modbus::ResultCode::EX_SUCCESS) {
            uint32_t sun_spec_id = discovery_read_uint32();

            if (sun_spec_id == SUN_SPEC_ID) {
                discovery_printfln("SunSpec ID found");

                discovery_state = DiscoveryState::ReadCommonModelHeader;
            }
            else {
                discovery_printfln("No SunSpec ID found: %08x", sun_spec_id);

                discovery_state = DiscoveryState::NextBaseAddress;
            }
        }
        else {
            discovery_printfln("Could not read SunSpec ID: %s (%d)", get_modbus_event_name(discovery_read_event), discovery_read_event);

            if (discovery_read_event == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND) {
                discovery_state = DiscoveryState::NextDeviceAddress;
            }
            else {
                discovery_state = DiscoveryState::NextBaseAddress;
            }
        }

        break;

    case DiscoveryState::ReadCommonModelHeader:
        discovery_printfln("Reading Common Model");

        discovery_read_size = 2;
        discovery_read_state = DiscoveryState::ReadCommonModelHeaderDone;
        discovery_state = DiscoveryState::Read;

        break;

    case DiscoveryState::ReadCommonModelHeaderDone:
        if (discovery_read_event == Modbus::ResultCode::EX_SUCCESS) {
            uint16_t model_id = discovery_read_uint16();
            uint16_t model_length = discovery_read_uint16();

            if (model_id == COMMON_MODEL_ID && (model_length == 65 || model_length == 66)) {
                discovery_printfln("Common Model found");

                discovery_common_model_length = model_length;
                discovery_state = DiscoveryState::ReadCommonModelBlock;
            }
            else {
                discovery_printfln("No Common Model found: %u %u", model_id, model_length);

                discovery_state = DiscoveryState::NextBaseAddress;
            }
        }
        else {
            discovery_printfln("Could not read Common Model header: %s (%d)", get_modbus_event_name(discovery_read_event), discovery_read_event);

            if (discovery_read_event == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND) {
                discovery_state = DiscoveryState::NextDeviceAddress;
            }
            else {
                discovery_state = DiscoveryState::NextBaseAddress;
            }
        }

        break;

    case DiscoveryState::ReadCommonModelBlock:
        discovery_read_size = discovery_common_model_length;
        discovery_read_state = DiscoveryState::ReadCommonModelBlockDone;
        discovery_state = DiscoveryState::Read;

        break;

    case DiscoveryState::ReadCommonModelBlockDone:
        if (discovery_read_event == Modbus::ResultCode::EX_SUCCESS) {
            char manufacturer_name[32 + 1];
            char model_name[32 + 1];
            char options[16 + 1];
            char version[16 + 1];
            char serial_number[32 + 1];
            uint16_t device_address;

            discovery_read_string(manufacturer_name, sizeof(manufacturer_name));
            discovery_read_string(model_name, sizeof(model_name));
            discovery_read_string(options, sizeof(options));
            discovery_read_string(version, sizeof(version));
            discovery_read_string(serial_number, sizeof(serial_number));
            device_address = discovery_read_uint16();

            discovery_printfln("Manufacturer Name: %s\n"
                               "Model Name: %s\n"
                               "Options: %s\n"
                               "Version: %s\n"
                               "Serial Number: %s\n"
                               "Device Address: %u",
                               manufacturer_name,
                               model_name,
                               options,
                               version,
                               serial_number,
                               device_address);

            discovery_state = DiscoveryState::ReadStandardModelHeader;
        }
        else {
            discovery_printfln("Could not read Common Model block: %s (%d)", get_modbus_event_name(discovery_read_event), discovery_read_event);

            if (discovery_read_event == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND) {
                discovery_state = DiscoveryState::NextDeviceAddress;
            }
            else {
                discovery_state = DiscoveryState::NextBaseAddress;
            }
        }

        break;

    case DiscoveryState::ReadStandardModelHeader:
        discovery_printfln("Reading Standard Model");

        discovery_read_size = 2;
        discovery_read_state = DiscoveryState::ReadStandardModelHeaderDone;
        discovery_state = DiscoveryState::Read;

        break;

    case DiscoveryState::ReadStandardModelHeaderDone:
        if (discovery_read_event == Modbus::ResultCode::EX_SUCCESS) {
            uint16_t model_id = discovery_read_uint16();
            uint16_t model_length = discovery_read_uint16();

            if (model_id == NON_IMPLEMENTED_UINT16 && model_length == 0) {
                discovery_printfln("Zero Model found");

                discovery_state = DiscoveryState::NextDeviceAddress;
            }
            else if (model_id == INVERTER_3P_FLOAT_MODEL_ID && model_length == INVERTER_3P_FLOAT_BLOCK_LENGTH) {
                discovery_printfln("Inverter 3P (Float) Model found");

                discovery_standard_model_length = model_length;
                discovery_state = DiscoveryState::ReadInverter3PFloatModelBlock;
            }
            else if (model_id == AC_METER_W3P_FLOAT_MODEL_ID && model_length == AC_METER_W3P_FLOAT_BLOCK_LENGTH) {
                discovery_printfln("AC Meter W3P (Float) Model found");

                discovery_standard_model_length = model_length;
                discovery_state = DiscoveryState::ReadACMeterW3PFloatModelBlock;
            }
            else {
                discovery_printfln("Skipping Unknown Model: %u %u", model_id, model_length);

                discovery_read_address += model_length;
                discovery_state = DiscoveryState::ReadStandardModelHeader;
            }
        }
        else {
            discovery_printfln("Could not read Standard Model header: %s (%d)", get_modbus_event_name(discovery_read_event), discovery_read_event);

            if (discovery_read_event == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND) {
                discovery_state = DiscoveryState::NextDeviceAddress;
            }
            else {
                discovery_state = DiscoveryState::NextBaseAddress;
            }
        }

        break;

    case DiscoveryState::ReadInverter3PFloatModelBlock:
        discovery_read_size = discovery_standard_model_length;
        discovery_read_state = DiscoveryState::ReadInverter3PFloatModelBlockDone;
        discovery_state = DiscoveryState::Read;

        break;

    case DiscoveryState::ReadInverter3PFloatModelBlockDone:
        if (discovery_read_event == Modbus::ResultCode::EX_SUCCESS) {
            float ac_current = discovery_read_float32();
            float ac_current_a = discovery_read_float32();
            float ac_current_b = discovery_read_float32();
            float ac_current_c = discovery_read_float32();

            discovery_printfln("AC Current [A]: %f\n"
                               "AC Current A [A]: %f\n"
                               "AC Current B [A]: %f\n"
                               "AC Current C [A]: %f",
                               ac_current,
                               ac_current_a,
                               ac_current_b,
                               ac_current_c);

            float ac_voltage_a_b = discovery_read_float32();
            float ac_voltage_b_c = discovery_read_float32();
            float ac_voltage_c_a = discovery_read_float32();
            float ac_voltage_a_n = discovery_read_float32();
            float ac_voltage_b_n = discovery_read_float32();
            float ac_voltage_c_n = discovery_read_float32();

            discovery_printfln("AC Voltage A B [V]: %f\n"
                               "AC Voltage B C [V]: %f\n"
                               "AC Voltage C A [V]: %f\n"
                               "AC Voltage A N [V]: %f\n"
                               "AC Voltage B N [V]: %f\n"
                               "AC Voltage C N [V]: %f",
                               ac_voltage_a_b,
                               ac_voltage_b_c,
                               ac_voltage_c_a,
                               ac_voltage_a_n,
                               ac_voltage_b_n,
                               ac_voltage_c_n);

            // FIXME

            discovery_state = DiscoveryState::ReadStandardModelHeader;
        }
        else {
            discovery_printfln("Could not read Inverter 3P (Float) Model block: %s (%d)", get_modbus_event_name(discovery_read_event), discovery_read_event);

            if (discovery_read_event == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND) {
                discovery_state = DiscoveryState::NextDeviceAddress;
            }
            else {
                discovery_state = DiscoveryState::NextBaseAddress;
            }
        }

        break;

    case DiscoveryState::ReadACMeterW3PFloatModelBlock:
        discovery_read_size = discovery_standard_model_length;
        discovery_read_state = DiscoveryState::ReadACMeterW3PFloatModelBlockDone;
        discovery_state = DiscoveryState::Read;

        break;

    case DiscoveryState::ReadACMeterW3PFloatModelBlockDone:
        if (discovery_read_event == Modbus::ResultCode::EX_SUCCESS) {
            float ac_current = discovery_read_float32();
            float ac_current_a = discovery_read_float32();
            float ac_current_b = discovery_read_float32();
            float ac_current_c = discovery_read_float32();

            discovery_printfln("AC Current [A]: %f\n"
                               "AC Current A [A]: %f\n"
                               "AC Current B [A]: %f\n"
                               "AC Current C [A]: %f",
                               ac_current,
                               ac_current_a,
                               ac_current_b,
                               ac_current_c);

            float ac_voltage_l_n = discovery_read_float32();
            float ac_voltage_a_n = discovery_read_float32();
            float ac_voltage_b_n = discovery_read_float32();
            float ac_voltage_c_n = discovery_read_float32();
            float ac_voltage_l_l = discovery_read_float32();
            float ac_voltage_a_b = discovery_read_float32();
            float ac_voltage_b_c = discovery_read_float32();
            float ac_voltage_c_a = discovery_read_float32();

            discovery_printfln("AC Voltage L N [V]: %f\n"
                               "AC Voltage A N [V]: %f\n"
                               "AC Voltage B N [V]: %f\n"
                               "AC Voltage C N [V]: %f\n"
                               "AC Voltage L L [V]: %f\n"
                               "AC Voltage A B [V]: %f\n"
                               "AC Voltage B C [V]: %f\n"
                               "AC Voltage C A [V]: %f",
                               ac_voltage_l_n,
                               ac_voltage_a_n,
                               ac_voltage_b_n,
                               ac_voltage_c_n,
                               ac_voltage_l_l,
                               ac_voltage_a_b,
                               ac_voltage_b_c,
                               ac_voltage_c_a);

            // FIXME

            discovery_state = DiscoveryState::ReadStandardModelHeader;
        }
        else {
            discovery_printfln("Could not read AC Meter W3P (Float) Model block: %s (%d)", get_modbus_event_name(discovery_read_event), discovery_read_event);

            if (discovery_read_event == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND) {
                discovery_state = DiscoveryState::NextDeviceAddress;
            }
            else {
                discovery_state = DiscoveryState::NextBaseAddress;
            }
        }

        break;
    }

    modbus.task();
}

uint16_t SunSpecMeter::discovery_read_uint16()
{
    uint16_t result = discovery_read_buffer[discovery_read_index];

    discovery_read_index += 1;

    return result;
}

uint32_t SunSpecMeter::discovery_read_uint32()
{
    uint32_t result = ((uint32_t)discovery_read_buffer[discovery_read_index] << 16) | discovery_read_buffer[discovery_read_index + 1];

    discovery_read_index += 2;

    return result;
}

float SunSpecMeter::discovery_read_float32()
{
    float result;
    uint16_t *p = (uint16_t *)&result;

    *p = discovery_read_buffer[discovery_read_index + 1];
    *(p + 1) = discovery_read_buffer[discovery_read_index];

    discovery_read_index += 2;

    return result;
}

// length must be one longer than the expected string length for NUL termination
void SunSpecMeter::discovery_read_string(char *buffer, size_t length)
{
    for (size_t i = 0; i < length - 1; i += 2, ++discovery_read_index) {
        buffer[i] = (discovery_read_buffer[discovery_read_index] >> 8) & 0xFF;

        if (i + 1 < length) {
            buffer[i + 1] = discovery_read_buffer[discovery_read_index] & 0xFF;
        }
    }

    buffer[length - 1] = '\0';
}

void SunSpecMeter::discovery_printfln(const char *fmt, ...)
{
    va_list args;
    char buf[512];

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    char buf_json[512];
    TFJsonSerializer json{buf_json, sizeof(buf_json)};

    json.add(buf);
    json.end();

    discovery_log_idle = false;

    ws.pushRawStateUpdate(buf_json, "sun_spec_meter/discovery_log"); // FIXME: error handling

    char *p = buf;
    char *q;

    while (*p != '\0') {
        q = strchr(p, '\n');

        if (q != NULL) {
            *q = '\0';
        }

        logger.printfln("SunSpec: %s", p);

        if (q == NULL) {
            break;
        }

        p = q + 1;
    }
}
