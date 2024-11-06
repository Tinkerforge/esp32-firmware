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
#include "meter_sun_spec.h"
#include "tools.h"
#include "sun_spec_model_id.h"

#include "gcc_warnings.h"

#define MAX_READ_CHUNK_SIZE 125
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
    config_prototype = Config::Object({
        {"display_name", Config::Str("", 0, 65)}, // 32 chars manufacturer name; space; 32 chars model name
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(502)},
        {"device_address", Config::Uint(1, 1, 247)},
        {"manufacturer_name", Config::Str("", 0, 32)},
        {"model_name", Config::Str("", 0, 32)},
        {"serial_number", Config::Str("", 0, 32)},
        {"model_id", Config::Uint16(0)}, // 0 == invalid
        {"model_instance", Config::Uint16(0)},
    });

    errors_prototype = Config::Object({
        {"timeout", Config::Uint32(0)},
        {"inconsistency", Config::Uint32(0)},
    });

    meters.register_meter_generator(get_class(), this);

    scan_config = ConfigRoot{Config::Object({
        {"host", Config::Str("", 0, 64)},
        {"port", Config::Uint16(0)},
        {"device_address_first", Config::Uint(1, 1, 247)},
        {"device_address_last", Config::Uint(247, 1, 247)},
        {"cookie", Config::Uint32(0)},
    })};

    scan_continue_config = ConfigRoot{Config::Object({
        {"cookie", Config::Uint32(0)},
    })};

    scan_abort_config = scan_continue_config;
}

void MetersSunSpec::register_urls()
{
    api.addCommand("meters_sun_spec/scan", &scan_config, {}, [this](String &error) {
        if (scan_state != ScanState::Idle) {
            error = "Another scan is already in progress, please try again later!";
            return;
        }

        scan_new_host = scan_config.get("host")->asString();
        scan_new_port = static_cast<uint16_t>(scan_config.get("port")->asUint());
        scan_new_device_address_first = static_cast<uint8_t>(scan_config.get("device_address_first")->asUint());
        scan_new_device_address_last = static_cast<uint8_t>(scan_config.get("device_address_last")->asUint());
        scan_new_cookie = scan_config.get("cookie")->asUint();

        if (scan_new_device_address_last < scan_new_device_address_first) {
            scan_new_device_address_last = scan_new_device_address_first;
        }

        scan_new = true;
        scan_last_keep_alive = now_us();
    }, true);

    api.addCommand("meters_sun_spec/scan_continue", &scan_continue_config, {}, [this](String &error) {
        if (scan_state == ScanState::Idle) {
            return;
        }

        uint32_t cookie = scan_continue_config.get("cookie")->asUint();

        if (cookie != scan_cookie) {
            error = "Cannot continue another scan";
            return;
        }

        scan_last_keep_alive = now_us();
    }, true);

    api.addCommand("meters_sun_spec/scan_abort", &scan_abort_config, {}, [this](String &error) {
        if (scan_state == ScanState::Idle) {
            return;
        }

        uint32_t cookie = scan_abort_config.get("cookie")->asUint();

        if (cookie != scan_cookie) {
            error = "Cannot abort another scan";
            return;
        }

        scan_abort = true;
    }, true);
}

void MetersSunSpec::loop()
{
    if (scan_printfln_buffer_used > 0 && deadline_elapsed(scan_printfln_last_flush + 2_s)) {
        scan_flush_log();
    }

    if (scan_state != ScanState::Idle && !scan_abort && deadline_elapsed(scan_last_keep_alive + 10_s)) {
        const char *message = "Aborting scan because no continue call was received for more than 10 seconds";

        logger.printfln("%s", message);
        scan_printfln("%s", message);

        scan_abort = true;
    }

    switch (scan_state) {
    case ScanState::Idle:
        if (scan_new) {
            scan_printfln("Starting scan");

            scan_state = ScanState::Connect;
            scan_host = scan_new_host;
            scan_port = scan_new_port;
            scan_device_address_first = scan_new_device_address_first;
            scan_device_address_last = scan_new_device_address_last;
            scan_cookie = scan_new_cookie;
            scan_device_address = scan_device_address_first;
            scan_base_address_index = 0;
            scan_read_timeout_burst = 0;
            ++scan_read_cookie;

            scan_new = false;
            scan_abort = false;
        }

        break;

    case ScanState::Connect:
        if (scan_abort) {
            scan_state = ScanState::Done;
            break;
        }

        scan_printfln("Connecting to %s:%u", scan_host.c_str(), scan_port);
        scan_state = ScanState::Connecting;

        client.connect(scan_host.c_str(), scan_port,
        [this](TFGenericTCPClientConnectResult result, int error_number) {
            if (result == TFGenericTCPClientConnectResult::Connected) {
                scan_state = ScanState::ReadSunSpecID;
            }
            else if (result == TFGenericTCPClientConnectResult::ResolveFailed) {
                if (error_number == EINVAL) {
                    scan_printfln("Couldn't resolve %s, no DNS server is configured", scan_host.c_str());
                }
                else if (error_number >= 0) {
                    scan_printfln("Couldn't resolve %s: %s (%d)", scan_host.c_str(), strerror(error_number), error_number);
                }
                else {
                    scan_printfln("Couldn't resolve %s", scan_host.c_str());
                }

                scan_state = ScanState::Done;
            }
            else if (error_number >= 0) {
                scan_printfln("Could not connect to %s:%u: %s / %s (%d)", scan_host.c_str(), scan_port, get_tf_generic_tcp_client_connect_result_name(result), strerror(error_number), error_number);
                scan_state = ScanState::Done;
            }
            else {
                scan_printfln("Could not connect to %s:%u: %s", scan_host.c_str(), scan_port, get_tf_generic_tcp_client_connect_result_name(result));
                scan_state = ScanState::Done;
            }
        },
        [this](TFGenericTCPClientDisconnectReason reason, int error_number) {
            if (reason == TFGenericTCPClientDisconnectReason::Requested) {
                scan_printfln("Disconnected from %s:%u", scan_host.c_str(), scan_port);
            }
            else if (error_number >= 0) {
                scan_printfln("Disconnected from %s:%u: %s / %s (%d)", scan_host.c_str(), scan_port, get_tf_generic_tcp_client_disconnect_reason_name(reason), strerror(error_number), error_number);
            }
            else {
                scan_printfln("Disconnected from %s:%u: %s", scan_host.c_str(), scan_port, get_tf_generic_tcp_client_disconnect_reason_name(reason));
            }

            scan_state = ScanState::Done;
        });

        break;

    case ScanState::Connecting:
        break;

    case ScanState::Disconnect:
        client.disconnect();
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

            // force the map to free its memory, clear() doesn't guaranteed that the memory gets freed
            scan_model_instances = std::unordered_map<uint16_t, uint16_t>();
        }

        break;

    case ScanState::NextDeviceAddress:
        if (scan_abort || scan_device_address >= scan_device_address_last) {
            scan_state = ScanState::Disconnect;
        }
        else {
            char buf[128];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan_cookie);
            json.addMemberNumber("progress", static_cast<float>(scan_device_address + 1u - scan_device_address_first) * 100.0f / static_cast<float>(scan_device_address_last - scan_device_address_first));
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
        if (scan_abort) {
            scan_state = ScanState::Disconnect;
            break;
        }

        ++scan_base_address_index;

        if (scan_base_address_index >= ARRAY_SIZE(base_addresses)) {
            scan_state = ScanState::NextDeviceAddress;
        }
        else {
            scan_state = ScanState::ReadSunSpecID;
        }

        break;

    case ScanState::Read:
        if (scan_abort) {
            scan_state = ScanState::Disconnect;
            break;
        }

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
        if (scan_abort) {
            scan_state = ScanState::Disconnect;
            break;
        }

        if (deadline_elapsed(scan_read_delay_deadline)) {
            scan_state = ScanState::ReadNext;
        }

        break;

    case ScanState::ReadNext: {
            if (scan_abort) {
                scan_state = ScanState::Disconnect;
                break;
            }

            uint32_t cookie = scan_read_cookie;
            size_t read_chunk_size = MIN(scan_read_size - scan_read_index, MAX_READ_CHUNK_SIZE);

            scan_state = ScanState::Reading;

            client.read(TFModbusTCPDataType::HoldingRegister,
                        scan_device_address,
                        static_cast<uint16_t>(scan_read_address),
                        static_cast<uint16_t>(read_chunk_size),
                        &scan_read_buffer[scan_read_index],
                        scan_read_timeout,
            [this, cookie, read_chunk_size](TFModbusTCPClientTransactionResult result) {
                if (scan_state != ScanState::Reading || cookie != scan_read_cookie) {
                    return;
                }

                if (result != TFModbusTCPClientTransactionResult::Timeout) {
                    scan_read_timeout = 1_s;
                    scan_read_timeout_burst = 0;
                    scan_read_retries = MAX_SCAN_READ_RETRIES;
                }
                else {
                    if (scan_read_timeout_burst < MAX_SCAN_READ_TIMEOUT_BURST) {
                        ++scan_read_timeout_burst;
                    }
                    else {
                        scan_read_timeout = 200_ms;
                        scan_read_retries = 0;
                    }
                }

                if (result == TFModbusTCPClientTransactionResult::Timeout && scan_read_retries > 0) {
                    scan_printfln("Reading timed out, retrying");

                    --scan_read_retries;
                    scan_read_delay_deadline = now_us() + 100_ms + static_cast<micros_t>(esp_random() % 2400000);
                    scan_state = ScanState::ReadDelay;
                    return;
                }

                scan_read_address += read_chunk_size;
                scan_read_index += read_chunk_size;
                scan_read_result = result;

                if (result != TFModbusTCPClientTransactionResult::Success || scan_read_index >= scan_read_size) {
                    scan_read_index = 0;
                    scan_state = scan_read_state;

                    scan_deserializer.buf = scan_read_buffer;
                    scan_deserializer.idx = 0;
                }
                else {
                    scan_state = ScanState::ReadNext;
                }
            });
        }

        break;

    case ScanState::Reading:
        break;

    case ScanState::ReadSunSpecID:
        if (scan_abort) {
            scan_state = ScanState::Disconnect;
            break;
        }

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
        if (scan_abort) {
            scan_state = ScanState::Disconnect;
            break;
        }

        if (scan_read_result == TFModbusTCPClientTransactionResult::Success) {
            uint32_t sun_spec_id = scan_deserializer.read_uint32();

            if (sun_spec_id == SUN_SPEC_ID) {
                scan_printfln("SunSpec ID found");

                scan_state = ScanState::ReadModelHeader;
            }
            else {
                scan_printfln("No SunSpec ID found (sun-spec-id: %08x)", sun_spec_id);

                scan_state = ScanState::NextBaseAddress;
            }
        }
        else {
            scan_printfln("Could not read SunSpec ID (error: %s [%d])",
                          get_tf_modbus_tcp_client_transaction_result_name(scan_read_result),
                          static_cast<int>(scan_read_result));

            scan_state = scan_get_next_state_after_read_error();
        }

        break;

    case ScanState::ReadCommonModelBlock:
        if (scan_abort) {
            scan_state = ScanState::Disconnect;
            break;
        }

        scan_printfln("Reading Common Model block");

        scan_read_size = 65; // don't read optional padding, skip it later
        scan_read_state = ScanState::ReadCommonModelBlockDone;
        scan_state = ScanState::Read;

        break;

    case ScanState::ReadCommonModelBlockDone:
        if (scan_abort) {
            scan_state = ScanState::Disconnect;
            break;
        }

        if (scan_read_result == TFModbusTCPClientTransactionResult::Success) {
            char options[16 + 1];
            char version[16 + 1];

            scan_deserializer.read_string(scan_common_manufacturer_name, sizeof(scan_common_manufacturer_name));
            scan_deserializer.read_string(scan_common_model_name, sizeof(scan_common_model_name));
            scan_deserializer.read_string(options, sizeof(options));
            scan_deserializer.read_string(version, sizeof(version));
            scan_deserializer.read_string(scan_common_serial_number, sizeof(scan_common_serial_number));

            uint16_t device_address = scan_deserializer.read_uint16();

            scan_printfln("  Manufacturer Name: %s\n"
                          "  Model Name: %s\n"
                          "  Options: %s\n"
                          "  Version: %s\n"
                          "  Serial Number: %s\n"
                          "  Device Address: %u",
                          scan_common_manufacturer_name,
                          scan_common_model_name,
                          options,
                          version,
                          scan_common_serial_number,
                          device_address);

            if (scan_block_length == 66) {
                scan_printfln("Skipping Common Model padding");

                ++scan_read_address; // skip padding
            }

            scan_state = ScanState::ReadModelHeader;
        }
        else {
            scan_printfln("Could not read Common Model block (error: %s [%d])",
                          get_tf_modbus_tcp_client_transaction_result_name(scan_read_result),
                          static_cast<int>(scan_read_result));

            scan_state = scan_get_next_state_after_read_error();
        }

        break;

    case ScanState::ReadModelHeader:
        if (scan_abort) {
            scan_state = ScanState::Disconnect;
            break;
        }

        scan_printfln("Reading Model header (address: %zu)", scan_read_address);

        scan_read_size = 2;
        scan_read_state = ScanState::ReadModelHeaderDone;
        scan_state = ScanState::Read;

        break;

    case ScanState::ReadModelHeaderDone:
        if (scan_abort) {
            scan_state = ScanState::Disconnect;
            break;
        }

        if (scan_read_result == TFModbusTCPClientTransactionResult::Success) {
            uint16_t model_id = scan_deserializer.read_uint16();
            size_t block_length = scan_deserializer.read_uint16();

            if (model_id == NON_IMPLEMENTED_UINT16 && (block_length == 0 || block_length == NON_IMPLEMENTED_UINT16)) {
                // accept non-implemented block length as a SUNGROW quirk
                scan_printfln("End Model found (block-length: %zu)", block_length);

                scan_state = ScanState::NextDeviceAddress;
            }
            else if (model_id == COMMON_MODEL_ID && (block_length == 65 || block_length == 66)) {
                scan_printfln("Common Model found (block-length: %zu)", block_length);

                scan_model_instances.clear();

                scan_model_id = model_id;
                scan_block_length = block_length;
                scan_state = ScanState::ReadCommonModelBlock;
            }
            else {
                const char *model_name = nullptr;

                for (size_t i = 0; i < sun_spec_model_specs_length; ++i) {
                    if (model_id == static_cast<uint16_t>(sun_spec_model_specs[i].model_id)) {
                        model_name = sun_spec_model_specs[i].model_name;
                        break;
                    }
                }

                if (model_name == nullptr) {
                    if (model_id >= 64000) {
                        model_name = "Vendor Specific";
                    }
                    else {
                        model_name = "Unknown";
                    }
                }

                if (scan_model_instances.find(model_id) == scan_model_instances.end()) {
                    scan_model_instances.insert({model_id, 0});
                }
                else {
                    ++scan_model_instances[model_id];
                }

                scan_printfln("%s Model found (model-id/instance: %u/%u, block-length: %zu)",
                              model_name, model_id, scan_model_instances.at(model_id), block_length);

                scan_model_id = model_id;
                scan_block_length = block_length;
                scan_state = ScanState::ReportModelResult;
            }
        }
        else {
            scan_printfln("Could not read Model header (error: %s [%d])",
                          get_tf_modbus_tcp_client_transaction_result_name(scan_read_result),
                          static_cast<int>(scan_read_result));

            scan_state = scan_get_next_state_after_read_error();
        }

        break;

    case ScanState::ReportModelResult: {
            if (scan_abort) {
                scan_state = ScanState::Disconnect;
                break;
            }

            char buf[512];
            TFJsonSerializer json{buf, sizeof(buf)};

            json.addObject();
            json.addMemberNumber("cookie", scan_cookie);
            json.addMemberString("manufacturer_name", scan_common_manufacturer_name);
            json.addMemberString("model_name", scan_common_model_name);
            json.addMemberString("serial_number", scan_common_serial_number);
            json.addMemberNumber("device_address", scan_device_address);
            json.addMemberNumber("model_id", scan_model_id);
            json.addMemberNumber("model_instance", scan_model_instances.at(scan_model_id));
            json.endObject();
            json.end();

            if (!ws.pushRawStateUpdate(buf, "meters_sun_spec/scan_result")) {
                break; // need report the scan result before doing something else
            }

            scan_read_address += scan_block_length; // skip block
            scan_state = ScanState::ReadModelHeader;
        }

        break;

    default:
        esp_system_abort("meters_sun_spec: Invalid state.");
    }

    client.tick();
}

[[gnu::const]]
MeterClassID MetersSunSpec::get_class() const
{
    return MeterClassID::SunSpec;
}

IMeter *MetersSunSpec::new_meter(uint32_t slot, Config *state, Config *errors)
{
    return new MeterSunSpec(slot, state, errors, modbus_tcp_client.get_pool());
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

MetersSunSpec::ScanState MetersSunSpec::scan_get_next_state_after_read_error()
{
    if (scan_read_result == TFModbusTCPClientTransactionResult::ModbusGatewayTargetDeviceFailedToRespond ||
        (scan_read_result == TFModbusTCPClientTransactionResult::Timeout && scan_read_retries <= 0)) {
        return ScanState::NextDeviceAddress;
    }

    return ScanState::NextBaseAddress;
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
