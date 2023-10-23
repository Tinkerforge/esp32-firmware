/* esp32-firmware
 * Copyright (C) 2023 Matthias Bolte <matthias@tinkerforge.com>
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

#include "meter_sun_spec.h"
#include "meters_sun_spec.h"
#include "api.h"
#include "event_log.h"
#include "task_scheduler.h"
#include "tools.h"
#include "module_dependencies.h"
#include "TFJson.h"
#include "sun_spec_model_id.h"

#include "gcc_warnings.h"

#define MAX_READ_CHUNK_SIZE 10

#define DEVICE_ADDRESS_FIRST 1
#define DEVICE_ADDRESS_LAST 247

#define SUN_SPEC_ID 0x53756E53

#define COMMON_MODEL_ID 1

#define NON_IMPLEMENTED_UINT16 0xFFFF
#define NON_IMPLEMENTED_UINT32 0xFFFFFFFF

static const uint16_t base_addresses[] {
    40000,
    50000,
    0
};

void MetersSunSpec::pre_setup()
{
    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 32)},
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)}, // 0 == auto discover
        {"device_address", Config::Uint(0, 1, 247)}, // 0 == auto discover
        {"model_id", Config::Uint16(0)}, // 0 == invalid
    });

    meters.register_meter_generator(get_class(), this);

    scan = ConfigRoot{Config::Object({
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(0)},
    })};
}

void MetersSunSpec::setup()
{
    modbus.client();

    initialized = true;
}

void MetersSunSpec::register_urls()
{
    api.addCommand("meters_sun_spec/scan", &scan, {}, [this](){
        if (scan_state != ScanState::Idle) {
            return;
        }

        scan_new = true;
        scan_new_host = scan.get("host")->asString();
        scan_new_port = static_cast<uint16_t>(scan.get("port")->asUint());
    }, true);
}

void MetersSunSpec::loop()
{
    if (scan_new && scan_state != ScanState::Idle) {
        scan_state = ScanState::Disconnect;
    }

    switch (scan_state) {
    case ScanState::Idle:
        if (!scan_log_idle) {
            if (!ws.pushRawStateUpdate("\"\"", "meters_sun_spec/scan_log")) {
                break; // need to idle the log before doing something else
            }

            scan_log_idle = true;
        }

        if (scan_new) {
            if (!ws.pushRawStateUpdate("\"<<<clear_scan_log>>>\"", "meters_sun_spec/scan_log")) {
                break; // need to clear the log before doing something else
            }

            scan_printfln("Starting scan");

            scan_state = ScanState::Resolve;
            scan_host = scan_new_host;
            scan_port = scan_new_port;
            scan_device_address = DEVICE_ADDRESS_FIRST;
            scan_base_address_index = 0;
            ++scan_read_cookie;

            scan_new = false;
            scan_new_host = "";
            scan_new_port = 0;
        }

        break;

    case ScanState::Resolve:
        scan_printfln("Resolving %s", scan_host.c_str());

        scan_host_data.user = this;
        scan_state = ScanState::Resolving;

        dns_gethostbyname_addrtype_lwip_ctx_async(scan_host.c_str(), [](dns_gethostbyname_addrtype_lwip_ctx_async_data *data) {
            MetersSunSpec *mss = static_cast<MetersSunSpec *>(data->user);

            if (data->err == ERR_OK) {
                if (data->addr_ptr == nullptr) {
                    mss->scan_printfln("Could not resolve %s", mss->scan_host.c_str());

                    mss->scan_state = ScanState::Done;
                }
                else if (data->addr_ptr->type != IPADDR_TYPE_V4) {
                    mss->scan_printfln("Could not resolve %s to an IPv4 address", mss->scan_host.c_str());

                    mss->scan_state = ScanState::Done;
                }
                else {
                    mss->scan_host_address = data->addr_ptr->u_addr.ip4.addr;
                    mss->scan_state = MetersSunSpec::ScanState::Connect;
                }
            }
            else {
                if (data->err == ERR_VAL) {
                    mss->scan_printfln("Could not resolve %s, no DNS server is configured", mss->scan_host.c_str());
                } else {
                    mss->scan_printfln("Could not resolve %s (error: %d)", mss->scan_host.c_str(), data->err);
                }

                mss->scan_state = ScanState::Done;
            }
        }, &scan_host_data, LWIP_DNS_ADDRTYPE_IPV4);

        break;

    case ScanState::Resolving:
        break;

    case ScanState::Connect:
        scan_printfln("Connecting to %s:%u", scan_host.c_str(), scan_port);

        if (!modbus.connect(scan_host_address, scan_port)) {
            scan_printfln("Could not connect to %s:%u", scan_host.c_str(), scan_port);

            scan_state = ScanState::Done;
        }
        else {
            scan_state = ScanState::ReadSunSpecID;
        }

        break;

    case ScanState::Disconnect:
        scan_printfln("Disconnecting from %s", scan_host.c_str());

        if (!modbus.disconnect(scan_host_address)) {
            scan_printfln("Could not disconnect from %s", scan_host.c_str());
        }

        scan_state = ScanState::Done;

        break;

    case ScanState::Done: {
            char buf[512];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.add("host", scan_host.c_str());
            json.add("port", static_cast<uint64_t>(scan_port));
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "meters_sun_spec/scan_done")) {
                break; // need report the scan as done before doing something else
            }

            ++scan_read_cookie;
            scan_state = ScanState::Idle;
        }

        break;

    case ScanState::NextDeviceAddress:
        if (scan_device_address >= DEVICE_ADDRESS_LAST) {
            scan_state = ScanState::Disconnect;
        }
        else {
            ++scan_device_address;
            scan_base_address_index = 0;
            scan_state = ScanState::ReadSunSpecID;
        }

        break;

    case ScanState::NextBaseAddress:
        ++scan_base_address_index;

        if (scan_base_address_index >= ARRAY_SIZE(base_addresses)) {
            scan_state = ScanState::NextDeviceAddress;
        }
        else {
            scan_state = ScanState::ReadSunSpecID;
        }

        break;

    case ScanState::Read:
        ++scan_read_cookie;
        scan_read_index = 0;
        scan_state = ScanState::ReadNext;

        break;

    case ScanState::ReadNext: {
            uint32_t cookie = scan_read_cookie;
            size_t read_chunk_size = MIN(scan_read_size - scan_read_index, MAX_READ_CHUNK_SIZE);

            scan_state = ScanState::Reading;

            modbus.readHreg(scan_host_address, static_cast<uint16_t>(scan_read_address), &scan_read_buffer[scan_read_index], static_cast<uint16_t>(read_chunk_size),
            [this, cookie, read_chunk_size](Modbus::ResultCode result, uint16_t transactionId, void *data) -> bool {
                if (scan_state != ScanState::Reading || cookie != scan_read_cookie) {
                    return true;
                }

                scan_read_address += read_chunk_size;
                scan_read_index += read_chunk_size;
                scan_read_result = result;

                if (scan_read_result != Modbus::ResultCode::EX_SUCCESS || scan_read_index >= scan_read_size) {
                    scan_read_index = 0;
                    scan_state = scan_read_state;

                    scan_deserializer.buf = scan_read_buffer;
                    scan_deserializer.idx = 0;
                }
                else {
                    scan_state = ScanState::ReadNext;
                }

                return true;
            }, scan_device_address);
        }

        break;

    case ScanState::Reading:
        break;

    case ScanState::ReadSunSpecID:
        scan_printfln("Using device address %u", scan_device_address);
        scan_printfln("Using base address %u", base_addresses[scan_base_address_index]);
        scan_printfln("Reading SunSpec ID");

        scan_read_address = base_addresses[scan_base_address_index];
        scan_read_size = 2;
        scan_read_state = ScanState::ReadSunSpecIDDone;
        scan_state = ScanState::Read;

        break;

    case ScanState::ReadSunSpecIDDone:
        if (scan_read_result == Modbus::ResultCode::EX_SUCCESS) {
            uint32_t sun_spec_id = scan_deserializer.read_uint32();

            if (sun_spec_id == SUN_SPEC_ID) {
                scan_printfln("SunSpec ID found");

                scan_state = ScanState::ReadCommonModelHeader;
            }
            else {
                scan_printfln("No SunSpec ID found: %08x", sun_spec_id);

                scan_state = ScanState::NextBaseAddress;
            }
        }
        else {
            scan_printfln("Could not read SunSpec ID: %s (%d)", get_modbus_result_code_name(scan_read_result), scan_read_result);

            if (scan_read_result == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND) {
                scan_state = ScanState::NextDeviceAddress;
            }
            else {
                scan_state = ScanState::NextBaseAddress;
            }
        }

        break;

    case ScanState::ReadCommonModelHeader:
        scan_printfln("Reading Common Model");

        scan_read_size = 2;
        scan_read_state = ScanState::ReadCommonModelHeaderDone;
        scan_state = ScanState::Read;

        break;

    case ScanState::ReadCommonModelHeaderDone:
        if (scan_read_result == Modbus::ResultCode::EX_SUCCESS) {
            uint16_t model_id = scan_deserializer.read_uint16();
            size_t block_length = scan_deserializer.read_uint16();

            if (model_id == COMMON_MODEL_ID && (block_length == 65 || block_length == 66)) {
                scan_printfln("Common Model found");

                scan_common_block_length = block_length;
                scan_state = ScanState::ReadCommonModelBlock;
            }
            else {
                scan_printfln("No Common Model found: %u %u", model_id, block_length);

                scan_state = ScanState::NextBaseAddress;
            }
        }
        else {
            scan_printfln("Could not read Common Model header: %s (%d)", get_modbus_result_code_name(scan_read_result), scan_read_result);

            if (scan_read_result == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND) {
                scan_state = ScanState::NextDeviceAddress;
            }
            else {
                scan_state = ScanState::NextBaseAddress;
            }
        }

        break;

    case ScanState::ReadCommonModelBlock:
        scan_read_size = scan_common_block_length;
        scan_read_state = ScanState::ReadCommonModelBlockDone;
        scan_state = ScanState::Read;

        break;

    case ScanState::ReadCommonModelBlockDone:
        if (scan_read_result == Modbus::ResultCode::EX_SUCCESS) {
            scan_deserializer.read_string(scan_common_manufacturer_name, sizeof(scan_common_manufacturer_name));
            scan_deserializer.read_string(scan_common_model_name, sizeof(scan_common_model_name));
            scan_deserializer.read_string(scan_common_options, sizeof(scan_common_options));
            scan_deserializer.read_string(scan_common_version, sizeof(scan_common_version));
            scan_deserializer.read_string(scan_common_serial_number, sizeof(scan_common_serial_number));

            uint16_t device_address = scan_deserializer.read_uint16();

            scan_printfln("Manufacturer Name: %s\n"
                          "Model Name: %s\n"
                          "Options: %s\n"
                          "Version: %s\n"
                          "Serial Number: %s\n"
                          "Device Address: %u",
                          scan_common_manufacturer_name,
                          scan_common_model_name,
                          scan_common_options,
                          scan_common_version,
                          scan_common_serial_number,
                          device_address);

            scan_state = ScanState::ReadStandardModelHeader;
        }
        else {
            scan_printfln("Could not read Common Model block: %s (%d)", get_modbus_result_code_name(scan_read_result), scan_read_result);

            if (scan_read_result == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND) {
                scan_state = ScanState::NextDeviceAddress;
            }
            else {
                scan_state = ScanState::NextBaseAddress;
            }
        }

        break;

    case ScanState::ReadStandardModelHeader:
        scan_printfln("Reading Standard Model");

        scan_read_size = 2;
        scan_read_state = ScanState::ReadStandardModelHeaderDone;
        scan_state = ScanState::Read;

        break;

    case ScanState::ReadStandardModelHeaderDone:
        if (scan_read_result == Modbus::ResultCode::EX_SUCCESS) {
            uint16_t model_id = scan_deserializer.read_uint16();
            size_t block_length = scan_deserializer.read_uint16();

            if (model_id == NON_IMPLEMENTED_UINT16 && block_length == 0) {
                scan_printfln("End Model found");

                scan_state = ScanState::NextDeviceAddress;
            }
            else {
                bool found = false;

                for (size_t i = 0; i < sun_spec_model_specs_length; ++i) {
                    if (model_id == static_cast<uint16_t>(sun_spec_model_specs[i].model_id)) {
                        scan_printfln("%s Model [%u] found", sun_spec_model_specs[i].model_name, model_id);

                        if (block_length != sun_spec_model_specs[i].block_length) {
                            scan_printfln("%s Model [%u] has unexpected length (actual: %zu, expected: %u)",
                                          sun_spec_model_specs[i].model_name, model_id, block_length, sun_spec_model_specs[i].block_length);
                        }
                        else {
                            found = true;
                            break;
                        }
                    }
                }

                if (found) {
                    scan_standard_model_id = model_id;
                    scan_standard_block_length = block_length;
                    scan_state = ScanState::ReportStandardModelResult;
                }
                else {
                    scan_printfln("Skipping Unknown Model [%u] with length %zu", model_id, block_length);

                    scan_read_address += block_length;
                    scan_state = ScanState::ReadStandardModelHeader;
                }
            }
        }
        else {
            scan_printfln("Could not read Standard Model header: %s (%d)", get_modbus_result_code_name(scan_read_result), scan_read_result);

            if (scan_read_result == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND) {
                scan_state = ScanState::NextDeviceAddress;
            }
            else {
                scan_state = ScanState::NextBaseAddress;
            }
        }

        break;

    case ScanState::ReportStandardModelResult: {
            char buf[512];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.add("host", scan_host.c_str());
            json.add("port", static_cast<uint64_t>(scan_port));
            json.add("manufacturer_name", scan_common_manufacturer_name);
            json.add("model_name", scan_common_model_name);
            json.add("options", scan_common_options);
            json.add("version", scan_common_version);
            json.add("serial_number", scan_common_serial_number);
            json.add("device_address", static_cast<uint64_t>(scan_device_address));
            json.add("model_id", static_cast<uint64_t>(scan_standard_model_id));
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "meters_sun_spec/scan_result")) {
                break; // need report the scan result before doing something else
            }

            scan_read_address += scan_standard_block_length;
            scan_state = ScanState::ReadStandardModelHeader;
        }

        break;

    default:
        esp_system_abort("meters_sun_spec: Invalid state.");
    }

    modbus.task();
}

_ATTRIBUTE((const))
MeterClassID MetersSunSpec::get_class() const
{
    return MeterClassID::SunSpec;
}

IMeter * MetersSunSpec::new_meter(uint32_t slot, Config *state, Config *config, Config *errors)
{
    // Must get ModbusTCP handle here because IMeters are created before our setup() ran.
    return new MeterSunSpec(slot, config, state, errors, meters_modbus_tcp.get_modbus_tcp_handle());
}

_ATTRIBUTE((const))
const Config * MetersSunSpec::get_config_prototype()
{
    return &config_prototype;
}

_ATTRIBUTE((const))
const Config * MetersSunSpec::get_state_prototype()
{
    return Config::Null();
}

_ATTRIBUTE((const))
const Config * MetersSunSpec::get_errors_prototype()
{
    return Config::Null();
}

void MetersSunSpec::scan_printfln(const char *fmt, ...)
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

    scan_log_idle = false;

    ws.pushRawStateUpdate(buf_json, "meters_sun_spec/scan_log"); // FIXME: error handling

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
