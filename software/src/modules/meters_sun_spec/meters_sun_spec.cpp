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
#include "esp_random.h"

#include "gcc_warnings.h"

#define MAX_READ_CHUNK_SIZE 125
#define MAX_SCAN_READ_RETRIES 5
#define MAX_SCAN_READ_TIMEOUT_BURST 10

#define DEVICE_ADDRESS_FIRST 1u
#define DEVICE_ADDRESS_LAST 247u

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
        {"display_name", Config::Str("", 0, 65)}, // 32 chars manufacturer name; space; 32 chars model name
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)}, // 0 == auto discover
        {"device_address", Config::Uint(0, 1, 247)}, // 0 == auto discover
        {"model_id", Config::Uint16(0)}, // 0 == invalid
    });

    meters.register_meter_generator(get_class(), this);

    scan = ConfigRoot{Config::Object({
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(0)},
        {"cookie", Config::Uint32(0)},
    })};
}

void MetersSunSpec::setup()
{
    modbus.client();

    initialized = true;
}

void MetersSunSpec::register_urls()
{
    api.addCommand("meters_sun_spec/scan", &scan, {}, [this](String &error){
        if (scan_state != ScanState::Idle) {
            error = "Another scan is already in progress, please try again later!";
            return;
        }

        scan_new_host = scan.get("host")->asString();
        scan_new_port = static_cast<uint16_t>(scan.get("port")->asUint());
        scan_new_cookie = scan.get("cookie")->asUint();
        scan_new = true;
    }, true);
}

void MetersSunSpec::loop()
{
    if (scan_printfln_buffer_used > 0 && deadline_elapsed(scan_printfln_last_flush + 2000000_usec)) {
        scan_flush_log();
    }

    switch (scan_state) {
    case ScanState::Idle:
        if (scan_new) {
            scan_printfln("Starting scan");

            scan_state = ScanState::Resolve;
            scan_host = scan_new_host;
            scan_port = scan_new_port;
            scan_cookie = scan_new_cookie;
            scan_device_address = DEVICE_ADDRESS_FIRST;
            scan_base_address_index = 0;
            scan_read_timeout_burst = 0;
            ++scan_read_cookie;

            scan_new = false;
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
            scan_printfln("Scan finished");
            scan_flush_log();

            char buf[128];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan_cookie);
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
            char buf[128];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan_cookie);
            json.addMemberNumber("progress", static_cast<float>(scan_device_address + 1u - DEVICE_ADDRESS_FIRST) * 100.0f / static_cast<float>(DEVICE_ADDRESS_LAST - DEVICE_ADDRESS_FIRST));
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "meters_sun_spec/scan_progress")) {
                break; // need report scan progress before doing something else
            }

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

        if (scan_read_timeout_burst < MAX_SCAN_READ_TIMEOUT_BURST) {
            scan_read_retries = MAX_SCAN_READ_RETRIES;
        }
        else {
            scan_read_retries = 0;
        }

        scan_state = ScanState::ReadNext;

        break;

    case ScanState::ReadDelay:
        if (deadline_elapsed(scan_read_delay_deadline)) {
            scan_state = ScanState::ReadNext;
        }

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

                if (result != Modbus::ResultCode::EX_TIMEOUT) {
                    modbus.setTimeout(1000);

                    scan_read_timeout_burst = 0;
                    scan_read_retries = MAX_SCAN_READ_RETRIES;
                }
                else {
                    if (scan_read_timeout_burst < MAX_SCAN_READ_TIMEOUT_BURST) {
                        ++scan_read_timeout_burst;
                    }
                    else {
                        modbus.setTimeout(200);

                        scan_read_retries = 0;
                    }
                }

                if (result == Modbus::ResultCode::EX_TIMEOUT && scan_read_retries > 0) {
                    scan_printfln("Reading timed out, retrying");

                    --scan_read_retries;
                    scan_read_delay_deadline = now_us() + 100000_usec + static_cast<micros_t>(esp_random() % 2400000);
                    scan_state = ScanState::ReadDelay;

                    return true;
                }

                scan_read_address += read_chunk_size;
                scan_read_index += read_chunk_size;
                scan_read_result = result;

                if (result != Modbus::ResultCode::EX_SUCCESS || scan_read_index >= scan_read_size) {
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
        scan_printfln("Using device address %u\n"
                      "Using base address %u\n"
                      "Reading SunSpec ID",
                      scan_device_address,
                      base_addresses[scan_base_address_index]);

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
                scan_printfln("No SunSpec ID found (sun-spec-id: %08x)", sun_spec_id);

                scan_state = ScanState::NextBaseAddress;
            }
        }
        else {
            scan_printfln("Could not read SunSpec ID (error: %s [%d])", get_modbus_result_code_name(scan_read_result), scan_read_result);

            if (scan_read_result == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND ||
                (scan_read_result == Modbus::ResultCode::EX_TIMEOUT && scan_read_retries <= 0)) {
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
                scan_printfln("Common Model found (block-length: %zu)", block_length);

                scan_common_block_length = block_length;
                scan_state = ScanState::ReadCommonModelBlock;
            }
            else {
                scan_printfln("No Common Model found (model-id: %u, block-length: %zu)", model_id, block_length);

                scan_state = ScanState::NextBaseAddress;
            }
        }
        else {
            scan_printfln("Could not read Common Model header (error: %s [%d])", get_modbus_result_code_name(scan_read_result), scan_read_result);

            if (scan_read_result == Modbus::ResultCode::EX_TIMEOUT) {
                scan_read_size = 2;
                scan_read_state = ScanState::ReadCommonModelHeaderDone;
                scan_state = ScanState::Read;
            }
            else if (scan_read_result == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND ||
                     (scan_read_result == Modbus::ResultCode::EX_TIMEOUT && scan_read_retries <= 0)) {
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
            scan_printfln("Could not read Common Model block (error: %s [%d])", get_modbus_result_code_name(scan_read_result), scan_read_result);

            if (scan_read_result == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND ||
                (scan_read_result == Modbus::ResultCode::EX_TIMEOUT && scan_read_retries <= 0)) {
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
                const char *model_name = "Unknown";

                for (size_t i = 0; i < sun_spec_model_specs_length; ++i) {
                    if (model_id == static_cast<uint16_t>(sun_spec_model_specs[i].model_id)) {
                        model_name = sun_spec_model_specs[i].model_name;
                        break;
                    }
                }

                scan_printfln("Found %s Model (model-id: %u, block-length: %zu)", model_name, model_id, block_length);

                scan_standard_model_id = model_id;
                scan_standard_block_length = block_length;
                scan_state = ScanState::ReportStandardModelResult;
            }
        }
        else {
            scan_printfln("Could not read Standard Model header (error: %s [%d])", get_modbus_result_code_name(scan_read_result), scan_read_result);

            if (scan_read_result == Modbus::ResultCode::EX_DEVICE_FAILED_TO_RESPOND ||
                (scan_read_result == Modbus::ResultCode::EX_TIMEOUT && scan_read_retries <= 0)) {
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
            json.addMemberNumber("cookie", scan_cookie);
            json.addMemberString("manufacturer_name", scan_common_manufacturer_name);
            json.addMemberString("model_name", scan_common_model_name);
            json.addMemberString("options", scan_common_options);
            json.addMemberString("version", scan_common_version);
            json.addMemberString("serial_number", scan_common_serial_number);
            json.addMemberNumber("device_address", scan_device_address);
            json.addMemberNumber("model_id", scan_standard_model_id);
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

[[gnu::const]]
MeterClassID MetersSunSpec::get_class() const
{
    return MeterClassID::SunSpec;
}

IMeter * MetersSunSpec::new_meter(uint32_t slot, Config *state, Config *errors)
{
    // Must get ModbusTCP handle here because IMeters are created before our setup() ran.
    return new MeterSunSpec(slot, state, errors, meters_modbus_tcp.get_modbus_tcp_handle());
}

[[gnu::const]]
const Config * MetersSunSpec::get_config_prototype()
{
    return &config_prototype;
}

[[gnu::const]]
const Config * MetersSunSpec::get_state_prototype()
{
    return Config::Null();
}

[[gnu::const]]
const Config * MetersSunSpec::get_errors_prototype()
{
    return Config::Null();
}

void MetersSunSpec::scan_flush_log()
{
    char buf[1024];
    TFJsonSerializer json{buf, sizeof(buf)};

    json.addObject();
    json.addMemberNumber("cookie", scan_cookie);
    json.addMemberString("message", scan_printfln_buffer);
    json.endObject();
    json.end();

    scan_printfln_buffer_used = 0;
    scan_printfln_last_flush = now_us();

    ws.pushRawStateUpdate(buf, "meters_sun_spec/scan_log"); // FIXME: error handling
}

void MetersSunSpec::scan_printfln(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    size_t used = vsnprintf_u(nullptr, 0, fmt, args);
    va_end(args);

    if (scan_printfln_buffer_used + used + 1 /* for \n */ >= sizeof(scan_printfln_buffer)) {
        scan_flush_log();
    }

    scan_printfln_buffer_used += vsnprintf_u(scan_printfln_buffer + scan_printfln_buffer_used, sizeof(scan_printfln_buffer) - scan_printfln_buffer_used, fmt, args);

    scan_printfln_buffer[scan_printfln_buffer_used++] = '\n';
    scan_printfln_buffer[scan_printfln_buffer_used] = '\0';
}
