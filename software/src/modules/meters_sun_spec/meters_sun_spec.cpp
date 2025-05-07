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

#include "meters_sun_spec.h"

#include <esp_random.h>
#include <TFJson.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "build.h"
#include "meter_sun_spec.h"
#include "tools.h"
#include "sun_spec_model_specs.h"
#include "modules/meters/meter_location.enum.h"
#include "dc_port_type.enum.h"

#include "gcc_warnings.h"

#define MAX_READ_CHUNK_SIZE 125U
#define MAX_SCAN_READ_RETRIES 5
#define MAX_SCAN_READ_TIMEOUT_BURST 10

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
    this->trace_buffer_index = logger.alloc_trace_buffer("meters_sun_spec", 8192);

    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 65)}, // 32 chars manufacturer name; space; 32 chars model name
        {"location", Config::Enum(MeterLocation::Unknown)},
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"device_address", Config::Uint8(1)},
        {"manufacturer_name", Config::Str("", 0, 32)},
        {"model_name", Config::Str("", 0, 32)},
        {"serial_number", Config::Str("", 0, 32)},
        {"model_id", Config::Uint16(0)}, // 0 == invalid
        {"model_instance", Config::Uint16(0)},
        {"dc_port_type", Config::Enum(DCPortType::NotImplemented)},
    });

    errors_prototype = Config::Object({
        {"timeout", Config::Uint32(0)},
        {"inconsistency", Config::Uint32(0)},
    });

    meters.register_meter_generator(get_class(), this);

    scan_config = ConfigRoot{Config::Object({
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"device_address_first", Config::Uint8(1)},
        {"device_address_last", Config::Uint8(247)},
        {"cookie", Config::Uint32(0)},
    })};

    scan_continue_config = ConfigRoot{Config::Object({
        {"cookie", Config::Uint32(0)},
    })};

    scan_abort_config = scan_continue_config;
}

void MetersSunSpec::register_urls()
{
    api.addCommand("meters_sun_spec/scan", &scan_config, {}, [this](String &errmsg) {
        if (scan != nullptr) {
            errmsg = "Another scan is already in progress, please try again later!";
            return;
        }

        scan = new_psram_or_dram<Scan>();

        scan->host = scan_config.get("host")->asString();
        scan->port = static_cast<uint16_t>(scan_config.get("port")->asUint());
        scan->device_address_first = static_cast<uint8_t>(scan_config.get("device_address_first")->asUint());
        scan->device_address_last = static_cast<uint8_t>(scan_config.get("device_address_last")->asUint());
        scan->cookie = scan_config.get("cookie")->asUint();

        if (scan->device_address_last < scan->device_address_first) {
            scan->device_address_last = scan->device_address_first;
        }

        scan->device_address = scan->device_address_first;
        scan->last_keep_alive = now_us();

        scan_printfln("Starting scan (" BUILD_DISPLAY_NAME ", version: %s)", build_version_full_str());
    }, true);

    api.addCommand("meters_sun_spec/scan_continue", &scan_continue_config, {}, [this](String &errmsg) {
        if (scan == nullptr) {
            return;
        }

        uint32_t cookie = scan_continue_config.get("cookie")->asUint();

        if (cookie != scan->cookie) {
            errmsg = "Cannot continue another scan";
            return;
        }

        scan->last_keep_alive = now_us();
    }, true);

    api.addCommand("meters_sun_spec/scan_abort", &scan_abort_config, {}, [this](String &errmsg) {
        if (scan == nullptr) {
            return;
        }

        uint32_t cookie = scan_abort_config.get("cookie")->asUint();

        if (cookie != scan->cookie) {
            errmsg = "Cannot abort another scan";
            return;
        }

        scan->abort = true;
    }, true);
}

void MetersSunSpec::loop()
{
    if (scan == nullptr) {
        return;
    }

    if (scan->printfln_buffer_used > 0 && deadline_elapsed(scan->printfln_last_flush + 2_s)) {
        scan_flush_log();
    }

    if (!scan->abort && deadline_elapsed(scan->last_keep_alive + 10_s)) {
        const char *message = "Aborting scan because no continue call was received for more than 10 seconds";

        logger.printfln("%s", message);
        scan_printfln("%s", message);

        scan->abort = true;
    }

    switch (scan->state) {
    case ScanState::Connect:
        if (scan->abort) {
            scan->state = ScanState::Done;
            break;
        }

        scan_printfln("Connecting to %s:%u", scan->host.c_str(), scan->port);
        scan->state = ScanState::Connecting;

        scan->client.connect(scan->host.c_str(), scan->port,
        [this](TFGenericTCPClientConnectResult result, int error_number) {
            if (result == TFGenericTCPClientConnectResult::Connected) {
                scan->state = ScanState::ReadSunSpecID;
            }
            else {
                char buf[256] = "";

                GenericTCPClientConnectorBase::format_connect_error(result, error_number, scan->host.c_str(), scan->port, buf, sizeof(buf));
                scan_printfln("%s", buf);

                scan->state = ScanState::Done;
            }
        },
        [this](TFGenericTCPClientDisconnectReason reason, int error_number) {
            char buf[256] = "";

            GenericTCPClientConnectorBase::format_disconnect_reason(reason, error_number, scan->host.c_str(), scan->port, buf, sizeof(buf));
            scan_printfln("%s", buf);

            scan->state = ScanState::Done;
        });

        break;

    case ScanState::Connecting:
        break;

    case ScanState::Disconnect:
        scan->client.disconnect();
        break;

    case ScanState::Done: {
            scan_printfln("Scan finished");
            scan_flush_log();

            char buf[128];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan->cookie);
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "meters_sun_spec/scan_done")) {
                break; // need to report the scan as done before doing something else
            }

            delete_psram_or_dram(scan);
            scan = nullptr;
        }

        return; // don't tick the destructed client

    case ScanState::NextDeviceAddress:
        if (scan->abort || scan->device_address >= scan->device_address_last) {
            scan->state = ScanState::Disconnect;
        }
        else {
            char buf[128];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan->cookie);
            json.addMemberNumber("progress", static_cast<float>(scan->device_address + 1u - scan->device_address_first) * 100.0f / static_cast<float>(scan->device_address_last - scan->device_address_first));
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "meters_sun_spec/scan_progress")) {
                break; // need to report scan progress before doing something else
            }

            ++scan->device_address;
            scan->base_address_index = 0;
            scan->state = ScanState::ReadSunSpecID;
        }

        break;

    case ScanState::NextBaseAddress:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        ++scan->base_address_index;

        if (scan->base_address_index >= ARRAY_SIZE(base_addresses)) {
            scan->state = ScanState::NextDeviceAddress;
        }
        else {
            scan->state = ScanState::ReadSunSpecID;
        }

        break;

    case ScanState::Read:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        scan->read_index = 0;

        if (scan->read_timeout_burst < MAX_SCAN_READ_TIMEOUT_BURST) {
            scan->read_retries = MAX_SCAN_READ_RETRIES;
        }
        else {
            scan->read_retries = 0;
        }

        scan->state = ScanState::ReadNext;

        break;

    case ScanState::ReadDelay:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        if (deadline_elapsed(scan->read_delay_deadline)) {
            scan->state = ScanState::ReadNext;
        }

        break;

    case ScanState::ReadNext: {
            if (scan->abort) {
                scan->state = ScanState::Disconnect;
                break;
            }

            size_t read_chunk_size = std::min(scan->read_size - scan->read_index, MAX_READ_CHUNK_SIZE);

            scan->state = ScanState::Reading;

            scan->client.transact(scan->device_address,
                                  TFModbusTCPFunctionCode::ReadHoldingRegisters,
                                  static_cast<uint16_t>(scan->read_address),
                                  static_cast<uint16_t>(read_chunk_size),
                                  &scan->read_buffer[scan->read_index],
                                  scan->read_timeout,
            [this, read_chunk_size](TFModbusTCPClientTransactionResult result) {
                if (scan->state != ScanState::Reading) {
                    return;
                }

                if (result != TFModbusTCPClientTransactionResult::Timeout) {
                    scan->read_timeout = 1_s;
                    scan->read_timeout_burst = 0;
                    scan->read_retries = MAX_SCAN_READ_RETRIES;
                }
                else {
                    if (scan->read_timeout_burst < MAX_SCAN_READ_TIMEOUT_BURST) {
                        ++scan->read_timeout_burst;
                    }
                    else {
                        scan->read_timeout = 200_ms;
                        scan->read_retries = 0;
                    }
                }

                if (result == TFModbusTCPClientTransactionResult::Timeout && scan->read_retries > 0) {
                    scan_printfln("Reading timed out, retrying");

                    --scan->read_retries;
                    scan->read_delay_deadline = now_us() + 100_ms + static_cast<micros_t>(esp_random() % 2400000);
                    scan->state = ScanState::ReadDelay;
                    return;
                }

                scan->read_address += read_chunk_size;
                scan->read_index += read_chunk_size;
                scan->read_result = result;

                if (result != TFModbusTCPClientTransactionResult::Success || scan->read_index >= scan->read_size) {
                    scan->read_index = 0;
                    scan->state = scan->read_state;

                    scan->deserializer.buf = scan->read_buffer;
                    scan->deserializer.idx = 0;
                }
                else {
                    scan->state = ScanState::ReadNext;
                }
            });
        }

        break;

    case ScanState::Reading:
        break;

    case ScanState::ReadSunSpecID:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        scan_printfln("Using device address %u\n"
                      "Using base address %u\n"
                      "Reading SunSpec ID",
                      scan->device_address,
                      base_addresses[scan->base_address_index]);

        scan->read_address = base_addresses[scan->base_address_index];
        scan->read_size = 2;
        scan->read_state = ScanState::ReadSunSpecIDDone;
        scan->state = ScanState::Read;

        break;

    case ScanState::ReadSunSpecIDDone:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        if (scan->read_result == TFModbusTCPClientTransactionResult::Success) {
            uint32_t sun_spec_id = scan->deserializer.read_uint32();

            if (sun_spec_id == SUN_SPEC_ID) {
                scan_printfln("SunSpec ID found");

                scan->state = ScanState::ReadModelID;
            }
            else {
                // this is not an error, this might just be no SunSpec device
                scan_printfln("No SunSpec ID found (sun-spec-id: 0x%08lx)", sun_spec_id);

                scan->state = ScanState::NextBaseAddress;
            }
        }
        else {
            // this is not an error, this might just be no SunSpec device
            scan_printfln("Could not read SunSpec ID (error: %s [%d])",
                          get_tf_modbus_tcp_client_transaction_result_name(scan->read_result),
                          static_cast<int>(scan->read_result));

            scan->state = scan_get_next_state_after_read_error();
        }

        break;

    case ScanState::ReadCommonModelBlock:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        scan_printfln("Reading Common Model block");

        scan->read_size = 65; // don't read optional padding, skip it later
        scan->read_state = ScanState::ReadCommonModelBlockDone;
        scan->state = ScanState::Read;

        break;

    case ScanState::ReadCommonModelBlockDone:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        if (scan->read_result == TFModbusTCPClientTransactionResult::Success) {
            char options[16 + 1];
            char version[16 + 1];

            scan->deserializer.read_string(scan->common_manufacturer_name, sizeof(scan->common_manufacturer_name));
            scan->deserializer.read_string(scan->common_model_name, sizeof(scan->common_model_name));
            scan->deserializer.read_string(options, sizeof(options));
            scan->deserializer.read_string(version, sizeof(version));
            scan->deserializer.read_string(scan->common_serial_number, sizeof(scan->common_serial_number));

            uint16_t device_address = scan->deserializer.read_uint16();

            scan_printfln("  Manufacturer Name: %s\n"
                          "  Model Name: %s\n"
                          "  Options: %s\n"
                          "  Version: %s\n"
                          "  Serial Number: %s\n"
                          "  Device Address: %u",
                          scan->common_manufacturer_name,
                          scan->common_model_name,
                          options,
                          version,
                          scan->common_serial_number,
                          device_address);

            if (scan->block_length == 66) {
                scan_printfln("Skipping Common Model padding");

                ++scan->read_address; // skip padding
            }

            scan->state = ScanState::ReadModelID;
        }
        else {
            scan_printfln("Error: Could not read Common Model block (error: %s [%d])",
                          get_tf_modbus_tcp_client_transaction_result_name(scan->read_result),
                          static_cast<int>(scan->read_result));

            scan->error_state = scan_get_next_state_after_read_error();
            scan->state = ScanState::ReportError;
        }

        break;

    case ScanState::ReadModelID:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        scan_printfln("Reading Model ID (address: %zu)", scan->read_address);

        scan->read_size = 1;
        scan->read_state = ScanState::ReadModelIDDone;
        scan->state = ScanState::Read;

        break;

    case ScanState::ReadModelIDDone:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        if (scan->read_result == TFModbusTCPClientTransactionResult::Success) {
            scan->model_id = scan->deserializer.read_uint16();

            scan_printfln("Found Model %u", scan->model_id);

            if (scan->model_id == 3) {
                scan_printfln("Assuming block length of 58 registers");
                scan->read_address += 1 + 58; // skip model length and block
                scan->state = ScanState::ReadModelID;
            }
            else if (scan->model_id == 4) {
                scan_printfln("Assuming block length of 60 registers");
                scan->read_address += 1 + 60; // skip model length and block
                scan->state = ScanState::ReadModelID;
            }
            else if (scan->model_id == 5) {
                scan_printfln("Assuming block length of 88 registers");
                scan->read_address += 1 + 88; // skip model length and block
                scan->state = ScanState::ReadModelID;
            }
            else if (scan->model_id == 6) {
                scan_printfln("Assuming block length of 90 registers");
                scan->read_address += 1 + 90; // skip model length and block
                scan->state = ScanState::ReadModelID;
            }
            else {
                scan->state = ScanState::ReadModelBlockLength;
            }
        }
        else {
            scan_printfln("Error: Could not read Model ID (error: %s [%d])",
                          get_tf_modbus_tcp_client_transaction_result_name(scan->read_result),
                          static_cast<int>(scan->read_result));

            scan->error_state = scan_get_next_state_after_read_error();
            scan->state = ScanState::ReportError;
        }

        break;

    case ScanState::ReadModelBlockLength:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        scan_printfln("Reading Model %u block length (address: %zu)", scan->model_id, scan->read_address);

        scan->read_size = 1;
        scan->read_state = ScanState::ReadModelBlockLengthDone;
        scan->state = ScanState::Read;

        break;

    case ScanState::ReadModelBlockLengthDone:
        if (scan->abort) {
            scan->state = ScanState::Disconnect;
            break;
        }

        if (scan->read_result == TFModbusTCPClientTransactionResult::Success) {
            size_t block_length = scan->deserializer.read_uint16();

            if (scan->model_id == NON_IMPLEMENTED_UINT16) {
                scan_printfln("End Model found (model-id: %u, block-length: %zu)", scan->model_id, block_length);

                if (block_length != 0 && block_length != NON_IMPLEMENTED_UINT16) { // accept non-implemented block length as Sungrow quirk
                    scan_printfln("Error: End Model has unsupported block length");

                    scan->error_state = ScanState::NextDeviceAddress;
                    scan->state = ScanState::ReportError;
                }
                else {
                    scan->state = ScanState::NextDeviceAddress;
                }
            }
            else if (scan->model_id == COMMON_MODEL_ID) {
                scan_printfln("Common Model found (model-id: %u, block-length: %zu)", scan->model_id, block_length);

                if (block_length != 65 && block_length != 66) {
                    scan_printfln("Error: Common Model has unsupported block length");

                    scan->error_state = scan_get_next_state_after_read_error();
                    scan->state = ScanState::ReportError;
                }
                else {
                    scan->model_instances.clear();

                    scan->block_length = block_length;
                    scan->state = ScanState::ReadCommonModelBlock;
                }
            }
            else {
                const char *model_name = nullptr;

                for (size_t i = 0; i < sun_spec_model_specs_length; ++i) {
                    if (scan->model_id == static_cast<uint16_t>(sun_spec_model_specs[i].model_id)) {
                        model_name = sun_spec_model_specs[i].model_name;
                        break;
                    }
                }

                if (model_name == nullptr) {
                    if (scan->model_id >= 64000) {
                        model_name = "Vendor Specific";
                    }
                    else {
                        model_name = "Unknown";
                    }
                }

                if (scan->model_instances.find(scan->model_id) == scan->model_instances.end()) {
                    scan->model_instances.insert({scan->model_id, 0});
                }
                else {
                    ++scan->model_instances[scan->model_id];
                }

                scan_printfln("%s Model found (model-id/instance: %u/%u, block-length: %zu)",
                              model_name, scan->model_id, scan->model_instances.at(scan->model_id), block_length);

                // FIXME: validate block length

                scan->block_length = block_length;
                scan->state = ScanState::ReportModelResult;
            }
        }
        else {
            scan_printfln("Error: Could not read Model %u block length (error: %s [%d])",
                          scan->model_id,
                          get_tf_modbus_tcp_client_transaction_result_name(scan->read_result),
                          static_cast<int>(scan->read_result));

            scan->error_state = scan_get_next_state_after_read_error();
            scan->state = ScanState::ReportError;
        }

        break;

    case ScanState::ReportModelResult: {
            if (scan->abort) {
                scan->state = ScanState::Disconnect;
                break;
            }

            char buf[512];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan->cookie);
            json.addMemberString("manufacturer_name", scan->common_manufacturer_name);
            json.addMemberString("model_name", scan->common_model_name);
            json.addMemberString("serial_number", scan->common_serial_number);
            json.addMemberNumber("device_address", scan->device_address);
            json.addMemberNumber("model_id", scan->model_id);
            json.addMemberNumber("model_instance", scan->model_instances.at(scan->model_id));
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "meters_sun_spec/scan_result")) {
                break; // need to report the scan result before doing something else
            }

            scan->read_address += scan->block_length; // skip block
            scan->state = ScanState::ReadModelID;
        }

        break;

    case ScanState::ReportError: {
            if (scan->abort) {
                scan->state = ScanState::Disconnect;
                break;
            }

            char buf[64];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan->cookie);
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "meters_sun_spec/scan_error")) {
                break; // need to report the scan error before doing something else
            }

            scan->state = scan->error_state;
        }

        break;

    default:
        esp_system_abort("meters_sun_spec: Invalid state.");
    }

    scan->client.tick();
}

[[gnu::const]]
MeterClassID MetersSunSpec::get_class() const
{
    return MeterClassID::SunSpec;
}

IMeter *MetersSunSpec::new_meter(uint32_t slot, Config *state, Config *errors)
{
    return new MeterSunSpec(slot, state, errors, modbus_tcp_client.get_pool(), trace_buffer_index);
}

[[gnu::const]]
const Config *MetersSunSpec::get_config_prototype()
{
    return &config_prototype;
}

[[gnu::const]]
const Config *MetersSunSpec::get_state_prototype()
{
    return Config::Null();
}

[[gnu::const]]
const Config *MetersSunSpec::get_errors_prototype()
{
    return &errors_prototype;
}

void MetersSunSpec::trace_timestamp()
{
    if (last_trace_timestamp < 0_us || deadline_elapsed(last_trace_timestamp + 1_s)) {
        last_trace_timestamp = now_us();
        logger.trace_timestamp(trace_buffer_index);
    }
}

MetersSunSpec::ScanState MetersSunSpec::scan_get_next_state_after_read_error()
{
    if (scan == nullptr) {
        return ScanState::Done;
    }

    if (scan->read_result == TFModbusTCPClientTransactionResult::ModbusGatewayPathUnvailable
     || scan->read_result == TFModbusTCPClientTransactionResult::ModbusGatewayTargetDeviceFailedToRespond
     || (scan->read_result == TFModbusTCPClientTransactionResult::Timeout && scan->read_retries <= 0)) {
        return ScanState::NextDeviceAddress;
    }

    return ScanState::NextBaseAddress;
}

void MetersSunSpec::scan_flush_log()
{
    if (scan == nullptr) {
        return;
    }

    char buf[1024];
    TFJsonSerializer json{buf, sizeof(buf)};

    json.addObject();
    json.addMemberNumber("cookie", scan->cookie);
    json.addMemberString("message", scan->printfln_buffer);
    json.endObject();
    json.end();

    scan->printfln_buffer_used = 0;
    scan->printfln_last_flush = now_us();

    ws.pushRawStateUpdate(buf, "meters_sun_spec/scan_log"); // FIXME: error handling
}

void MetersSunSpec::scan_printfln(const char *fmt, ...)
{
    if (scan == nullptr) {
        return;
    }

    va_list args;
    va_start(args, fmt);
    size_t used = vsnprintf_u(nullptr, 0, fmt, args);
    va_end(args);

    if (scan->printfln_buffer_used + used + 1 /* for \n */ >= sizeof(scan->printfln_buffer)) {
        scan_flush_log();
    }

    scan->printfln_buffer_used += vsnprintf_u(scan->printfln_buffer + scan->printfln_buffer_used, sizeof(scan->printfln_buffer) - scan->printfln_buffer_used, fmt, args);

    scan->printfln_buffer[scan->printfln_buffer_used++] = '\n';
    scan->printfln_buffer[scan->printfln_buffer_used] = '\0';
}
