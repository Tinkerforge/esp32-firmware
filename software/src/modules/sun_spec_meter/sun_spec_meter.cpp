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

#define LAST_DEVICE_ADDRESS 247
#define SUN_SPEC_ID 0x53756e53
#define COMMON_MODEL_ID 1

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
            ++discovery_cookie;

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
            ++discovery_cookie;
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
        ++discovery_cookie;

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

    case DiscoveryState::Read: {
            uint32_t cookie = discovery_cookie;

            discovery_state = DiscoveryState::Reading;

            modbus.readHreg(discovery_host, discovery_read_address, discovery_read_buffer, discovery_read_size,
            [this, cookie](Modbus::ResultCode event, uint16_t transactionId, void *data) -> bool {
                if (discovery_state != DiscoveryState::Reading || cookie != discovery_cookie) {
                    return true;
                }

                discovery_read_address += discovery_read_size;
                discovery_read_event = event;
                discovery_read_index = 0;
                discovery_state = discovery_read_state;

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
        discovery_printfln("Reading Common Model header");

        discovery_read_size = 2;
        discovery_read_state = DiscoveryState::ReadCommonModelHeaderDone;
        discovery_state = DiscoveryState::Read;

        break;

    case DiscoveryState::ReadCommonModelHeaderDone:
        if (discovery_read_event == Modbus::ResultCode::EX_SUCCESS) {
            uint16_t model_id = discovery_read_uint16();
            uint16_t model_length = discovery_read_uint16();

            if (model_id == COMMON_MODEL_ID && (model_length == 65 || model_length == 66)) {
                discovery_printfln("Common Model header found");

                discovery_common_model_length = model_length;
                discovery_state = DiscoveryState::ReadCommonModelBlock;
            }
            else {
                discovery_printfln("No Common Model header found: %04x %04x", model_id, model_length);

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

            discovery_printfln("Manufacturer Name: %s", manufacturer_name);
            discovery_printfln("Model Name: %s", model_name);
            discovery_printfln("Options: %s", options);
            discovery_printfln("Version: %s", version);
            discovery_printfln("Serial Number: %s", serial_number);
            discovery_printfln("Device Address: %u", device_address);

            if (discovery_common_model_length == 66) {
                discovery_read_uint16(); // skip padding
            }

            discovery_state = DiscoveryState::NextDeviceAddress;// FIXME
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
    char buf[256];
    auto buf_size = sizeof(buf) / sizeof(buf[0]);

    va_start(args, fmt);
    memset(buf, 0, buf_size);
    auto written = vsnprintf(buf + 1, buf_size - 2, fmt, args);
    va_end(args);

    written = MIN(written, buf_size - 2);

    logger.printfln("SunSpec: %s", buf + 1);

    buf[0] = '\"';
    buf[written + 1] = '\"';
    buf[written + 2] = '\0';

    discovery_log_idle = false;

    ws.pushRawStateUpdate(buf, "sun_spec_meter/discovery_log");
}
