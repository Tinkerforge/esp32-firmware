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

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "sun_spec_model_specs.h"
#include "tools/semantic_version.h"
#include "tools/hexdump.h"
#include "modules/meters/meter_location.enum.h"
#include "modules/modbus_tcp_client/modbus_tcp_tools.h"
#include "modules/meters_sun_spec/models/model_001.h"

#include "gcc_warnings.h"

#define SUN_SPEC_ID 0x53756E53
#define COMMON_MODEL_ID 1
#define NON_IMPLEMENTED_UINT16 0xFFFF
#define SUCCESSFUL_PARSE_TIMEOUT 1_min

#define trace(fmt, ...) \
    do { \
        meters_sun_spec.trace_timestamp(); \
        logger.tracefln_plain(trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

// The manufacturer name for SolarEdge devices sometimes has a trailing space
static inline bool is_solar_edge(const char *manufacturer)
{
    return strncmp(manufacturer, "SolarEdge", 32) == 0 || strncmp(manufacturer, "SolarEdge ", 32) == 0;
}

// Since KOSTAL Smart Energy Meter firmware 2.6.0 the SunSpec manufacturer name
// got changed from "KOSTAL" to "KOSTAL Solar Electric GmbH"
static inline bool is_kostal(const char *manufacturer)
{
    return strncmp(manufacturer, "KOSTAL", 32) == 0 || strncmp(manufacturer, "KOSTAL Solar Electric GmbH", 32) == 0;
}

static const uint16_t scan_base_addresses[] {
    40000,
    50000,
    0
};

MeterClassID MeterSunSpec::get_class() const
{
    return MeterClassID::SunSpec;
}

void MeterSunSpec::setup(Config *ephemeral_config)
{
    host              = ephemeral_config->get("host")->asString();
    port              = static_cast<uint16_t>(ephemeral_config->get("port")->asUint());
    device_address    = static_cast<uint8_t>(ephemeral_config->get("device_address")->asUint());
    manufacturer_name = ephemeral_config->get("manufacturer_name")->asString();
    model_name        = ephemeral_config->get("model_name")->asString();
    serial_number     = ephemeral_config->get("serial_number")->asString();
    model_id          = static_cast<uint16_t>(ephemeral_config->get("model_id")->asUint());
    model_instance    = static_cast<uint16_t>(ephemeral_config->get("model_instance")->asUint());
    dc_port_type      = ephemeral_config->get("dc_port_type")->asEnum<DCPortType>();
    model_parser      = MetersSunSpecParser::new_parser(slot, model_id, dc_port_type);

    MeterLocation default_location = MeterLocation::Unknown;

    for (size_t i = 0; i < sun_spec_model_specs_length; ++i) {
        if (model_id == static_cast<uint16_t>(sun_spec_model_specs[i].model_id)) {
            default_location = sun_spec_model_specs[i].meter_location;
            break;
        }
    }

    if (ephemeral_config->get("location")->asEnum<MeterLocation>() == MeterLocation::Unknown && default_location != MeterLocation::Unknown) {
        ephemeral_config->get("location")->updateEnum(default_location);
    }

    if (model_parser == nullptr) {
        logger.printfln_meter("No parser available for model %u", model_id);
        return;
    }

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (read_allowed) {
            if (deadline_elapsed(last_successful_parse + SUCCESSFUL_PARSE_TIMEOUT)) {
                logger.printfln("Last successful parse occurred too long ago, reconnecting to %s:%u", host.c_str(), port);
                force_reconnect();
                return;
            }

            read_allowed = false;
            start_generic_read();
        }
    }, 2_s, 1_s);
}

void MeterSunSpec::register_events()
{
    if (model_parser == nullptr) {
        return;
    }

    network.on_network_connected([this](const Config *connected) {
        if (connected->asBool()) {
            start_connection();
        }
        else {
            stop_connection();
        }

        return EventResult::OK;
    });
}

void MeterSunSpec::pre_reboot()
{
    stop_connection();
}

void MeterSunSpec::connect_callback()
{
    GenericModbusTCPClient::connect_callback();

    last_successful_parse = now_us();

    scan_start();
}

void MeterSunSpec::disconnect_callback()
{
    read_allowed = false;

    task_scheduler.cancel(this->scan_task_id);
    this->scan_task_id = 0;

    free(generic_read_request.data[0]);

    generic_read_request.data[0] = nullptr;
    generic_read_request.data[1] = nullptr;
}

bool MeterSunSpec::alloc_read_buffer(size_t model_regcount)
{
    free(generic_read_request.data[0]);

    generic_read_request.data[0] = nullptr;
    generic_read_request.data[1] = nullptr;

    bool read_twice = model_parser->must_read_twice();
    size_t buffer_regcount = read_twice ? model_regcount * 2 : model_regcount;
    uint16_t *buffer = static_cast<uint16_t *>(malloc(sizeof(uint16_t) * buffer_regcount));

    if (buffer == nullptr) {
        logger.printfln_meter("Cannot alloc read buffer");
        return false;
    }

    generic_read_request.data[0] = buffer;

    if (read_twice) {
        generic_read_request.data[1] = buffer + model_regcount;
    }

    generic_read_request.read_twice = read_twice;

    return true;
}

void MeterSunSpec::trace_response()
{
    if (generic_read_request.result != TFModbusTCPClientTransactionResult::Success) {
        trace("m%lu a%zu c%zu e%lu",
              slot,
              generic_read_request.start_address,
              generic_read_request.register_count,
              static_cast<uint32_t>(generic_read_request.result));
    }
    else {
        char data_buf[125 * 4 + 1]; // 4 nibble per register for 125 registers plus \n
        size_t data_buf_used;

        for (size_t i = 0; i < 2; ++i) {
            if (generic_read_request.data[i] != nullptr) {
                trace("m%lu a%zu c%zu d%zu",
                      slot,
                      generic_read_request.start_address,
                      generic_read_request.register_count,
                      i);

                data_buf_used = hexdump(generic_read_request.data[i], generic_read_request.register_count, data_buf, ARRAY_SIZE(data_buf), HexdumpCase::Lower);
                data_buf[data_buf_used] = '\n';
                ++data_buf_used;

                logger.trace_plain(trace_buffer_index, data_buf, data_buf_used);
            }
        }
    }
}

void MeterSunSpec::read_start(size_t model_regcount)
{
    if (!alloc_read_buffer(model_regcount)) {
        return;
    }

    generic_read_request.register_type = ModbusRegisterType::HoldingRegister;
    generic_read_request.register_count = model_regcount;
    generic_read_request.done_callback = [this]{ read_done_callback(); };

    start_generic_read();
}

void MeterSunSpec::read_done_callback()
{
    read_allowed = true;

    trace_response();

    if (generic_read_request.result != TFModbusTCPClientTransactionResult::Success) {
        if (generic_read_request.result == TFModbusTCPClientTransactionResult::Timeout) {
            auto timeout = errors->get("timeout");
            timeout->updateUint(timeout->asUint() + 1);
        }

        return;
    }

    if (!values_declared) {
        size_t registers_to_read = generic_read_request.register_count;

        if (!model_parser->detect_values(generic_read_request.data, quirks, &registers_to_read)) {
            logger.printfln_meter("Detecting values of model %hu failed", model_id);
            return;
        }

        MeterValueID phase_voltage_ids[3] = {
            MeterValueID::VoltageL1N,
            MeterValueID::VoltageL2N,
            MeterValueID::VoltageL3N,
        };

        meters.fill_index_cache(slot, ARRAY_SIZE(phase_voltage_ids), phase_voltage_ids, phase_voltage_index_cache);

        for (size_t i = 0; i < ARRAY_SIZE(phase_voltage_index_cache); ++i) {
            if (phase_voltage_index_cache[i] != UINT32_MAX) {
                logger.printfln_meter("Checking phase voltages for float-is-le32 quirk");
                check_phase_voltages = true;
                break;
            }
        }

        values_declared = true;

        bool more_registers_to_read = registers_to_read > generic_read_request.register_count;

        generic_read_request.register_count = registers_to_read;

        if (more_registers_to_read) {
            if (!alloc_read_buffer(registers_to_read)) {
                read_allowed = false;
            }

            return;
        }
    }

    if (!model_parser->parse_values(generic_read_request.data, quirks)) {
        auto inconsistency = errors->get("inconsistency");
        inconsistency->updateUint(inconsistency->asUint() + 1);
        // TODO: Read again if parsing failed?
        return;
    }
    else {
        last_successful_parse = now_us();
    }

    if (check_phase_voltages) {
        bool parse_again = false;

        for (size_t i = 0; i < ARRAY_SIZE(phase_voltage_index_cache); ++i) {
            if (phase_voltage_index_cache[i] == UINT32_MAX) {
                continue;
            }

            float value = 0;

            meters.get_value_by_index(slot, phase_voltage_index_cache[i], &value);

            if (value < -5 || value > 280) {
                logger.printfln_meter("Enabling float-is-le32 quirk due to abnormal L%zu-N voltage value: %.1f V", i + 1, static_cast<double>(value));
                quirks |= SUN_SPEC_QUIRKS_FLOAT_IS_LE32;
                parse_again = true;
            }
            else if (value > 100) {
                logger.printfln_meter("Check for float-is-le32 quirk completed due to normal L%zu-N voltage value: %.1f V", i + 1, static_cast<double>(value));
            }
            else {
                continue; // phase voltage in no-mans-land, cannot decide
            }

            check_phase_voltages = false;
            break;
        }

        if (parse_again) {
            if (!model_parser->parse_values(generic_read_request.data, quirks)) {
                auto inconsistency = errors->get("inconsistency");
                inconsistency->updateUint(inconsistency->asUint() + 1);
                // TODO: Read again if parsing failed?
            }
            else {
                last_successful_parse = now_us();
            }
        }
    }
}

void MeterSunSpec::scan_start_delay()
{
    task_scheduler.cancel(this->scan_task_id);

    this->scan_task_id = task_scheduler.scheduleOnce([this](){
        this->scan_task_id = 0;
        this->scan_start();
    }, 10_s);
}

void MeterSunSpec::scan_start()
{
    free(generic_read_request.data[0]);

    generic_read_request.data[0] = nullptr;
    generic_read_request.data[1] = nullptr;

    // Buffer must be big enough for the Common model.
    uint16_t *buffer = static_cast<uint16_t *>(malloc(sizeof(uint16_t) * 68));
    if (!buffer) {
        logger.printfln_meter("Cannot alloc read buffer");
        return;
    }

    log_read_errors = false; // don't log errors while probing for the correct base address
    scan_base_address_index = 0;
    scan_state = ScanState::Idle;
    scan_state_next = ScanState::ReadSunSpecID;
    scan_deserializer.buf = buffer;
    scan_device_found = false;
    scan_model_counter = model_instance;

    generic_read_request.register_type = ModbusRegisterType::HoldingRegister;
    generic_read_request.start_address = scan_base_addresses[scan_base_address_index];
    generic_read_request.register_count = 2;
    generic_read_request.data[0] = buffer;
    generic_read_request.read_twice = false;
    generic_read_request.done_callback = [this]{ scan_next(); };

    start_generic_read();
}

void MeterSunSpec::scan_read_delay()
{
    task_scheduler.scheduleOnce([this](){
        this->start_generic_read();
    }, 1_s + (millis_t{esp_random() % 4000}));
}

void MeterSunSpec::scan_next_base_address()
{
    ++scan_base_address_index;

    if (scan_base_address_index >= ARRAY_SIZE(scan_base_addresses)) {
        logger.printfln_meter("No SunSpec device found at %s:%u:%u", host.c_str(), port, device_address);
        scan_start_delay();
    }
    else {
        generic_read_request.start_address = scan_base_addresses[scan_base_address_index];
        generic_read_request.register_count = 2;
        scan_state_next = ScanState::ReadSunSpecID;

        start_generic_read();
    }
}

void MeterSunSpec::scan_next()
{
    trace_response();

    if (generic_read_request.result != TFModbusTCPClientTransactionResult::Success) {
        if (generic_read_request.result == TFModbusTCPClientTransactionResult::NotConnected) {
            logger.printfln_meter("Connection got lost while scanning %s:%u:%u", host.c_str(), port, device_address);
            return;
        }

        if (generic_read_request.result == TFModbusTCPClientTransactionResult::Timeout) {
            auto timeout = errors->get("timeout");
            timeout->updateUint(timeout->asUint() + 1);
        }

        if (scan_state_next == ScanState::ReadSunSpecID) {
            scan_next_base_address();
        }
        else {
            scan_read_delay();
        }

        return;
    }

    scan_deserializer.idx = 0;
    scan_state = scan_state_next;

    switch (scan_state) {
        case ScanState::Idle:
            break;

        case ScanState::ReadSunSpecID: {
                uint32_t sun_spec_id = scan_deserializer.read_uint32();

                if (sun_spec_id == SUN_SPEC_ID) {
                    generic_read_request.start_address += generic_read_request.register_count;
                    generic_read_request.register_count = 2;
                    log_read_errors = true; // log errors again after the correct base address was found
                    scan_state_next = ScanState::ReadModelHeader;

                    start_generic_read();
                }
                else {
                    scan_next_base_address();
                }
            }

            break;

        case ScanState::ReadModelHeader: {
                uint16_t scan_model_id = scan_deserializer.read_uint16();
                size_t block_length = scan_deserializer.read_uint16();

                if (scan_model_id == NON_IMPLEMENTED_UINT16) { // End model found
                    logger.printfln_meter("Configured SunSpec model %u/%u not found at %s:%u:%u",
                                          model_id, model_instance, host.c_str(), port, device_address);
                    scan_start_delay();
                }
                else if (scan_device_found && scan_model_id == model_id) {
                    if (scan_model_counter > 0) {
                        --scan_model_counter;

                        generic_read_request.start_address += generic_read_request.register_count + block_length;
                        generic_read_request.register_count = 2;

                        start_generic_read();
                    }
                    else {
                        if (!model_parser->is_model_length_supported(block_length)) {
                            logger.printfln_meter("Configured SunSpec model %u/%u found but has unsupported length: %u",
                                                  model_id, model_instance, block_length);
                            scan_start_delay();
                        }
                        else {
                            scan_state_next = ScanState::Idle;

                            logger.printfln_meter("Configured SunSpec model %u/%u found at %s:%u:%u:%u",
                                                  model_id, model_instance, host.c_str(), port, device_address, generic_read_request.start_address);
                            read_start(model_parser->get_interesting_registers_count());
                        }
                    }
                }
                else if (scan_model_id == 1) { // Common model
                    generic_read_request.register_count = 67;
                    scan_state_next = ScanState::ReadModel;

                    start_generic_read();
                }
                else {
                    generic_read_request.start_address += generic_read_request.register_count + block_length;
                    generic_read_request.register_count = 2;

                    start_generic_read();
                }
            }

            break;

        case ScanState::ReadModel: {
                uint16_t scan_model_id = scan_deserializer.read_uint16();
                size_t block_length = scan_deserializer.read_uint16();

                if (scan_model_id == 1) { // Common model
                    SunSpecCommonModel001_u *common_model = reinterpret_cast<SunSpecCommonModel001_u *>(generic_read_request.data[0]);
                    modbus_bswap_registers(common_model->registers + 2, 64);
                    const SunSpecCommonModel001_s *m = &common_model->model;

                    logger.printfln_meter("Looking for device Mn='%s' Md='%s' SN='%s'", manufacturer_name.c_str(), model_name.c_str(), serial_number.c_str());

                    if (manufacturer_name.length() == 0 && model_name.length() == 0 && serial_number.length() == 0) {
                        scan_device_found = true;
                    }
                    else if (is_solar_edge(m->Mn) &&
                             strncmp(m->Md, "SE-RGMTR-1D-240C-A", 32) == 0 &&
                             strncmp(m->SN, "0", 32) == 0 &&
                             is_solar_edge(manufacturer_name.c_str()) &&
                             strncmp(model_name.c_str(), "MTR-240-3PC1-D-A-MW", 32) == 0) {
                        // Sometimes SolarEdge inverters report a MTR-240-3PC1-D-A-MW meter wrongly
                        // as a SE-RGMTR-1D-240C-A meter with serial number 0. Work around this by
                        // accepting a SE-RGMTR-1D-240C-A meter with serial number 0 when looking
                        // for a MTR-240-3PC1-D-A-MW meter.
                        scan_device_found = true;
                    }
                    else if (is_solar_edge(m->Mn) &&
                             strncmp(m->Md, "MTR-240-3PC1-D-A-MW", 32) == 0 &&
                             is_solar_edge(manufacturer_name.c_str()) &&
                             strncmp(model_name.c_str(), "SE-RGMTR-1D-240C-A", 32) == 0 &&
                             strncmp(serial_number.c_str(), "0", 32) == 0) {
                        // A MTR-240-3PC1-D-A-MW meter might have been configured while it was wrongly
                        // reported as SE-RGMTR-1D-240C-A meter with serial number 0. But now it is
                        // correctly reported again. Work around this by accepting a MTR-240-3PC1-D-A-MW
                        // meter when looking for a SE-RGMTR-1D-240C-A meter with serial number 0.
                        scan_device_found = true;
                    }
                    else {
                        bool manufacturer_match = strncmp(m->Mn, manufacturer_name.c_str(), 32) == 0 ||
                                                  (is_solar_edge(m->Mn) && is_solar_edge(manufacturer_name.c_str())) ||
                                                  (is_kostal(m->Mn) && is_kostal(manufacturer_name.c_str()));

                        scan_device_found = manufacturer_match &&
                                            strncmp(m->Md, model_name.c_str(), 32) == 0 &&
                                            strncmp(m->SN, serial_number.c_str(), 32) == 0;
                    }

                    logger.printfln_meter("Device Mn='%.*s' Md='%.*s' Opt='%.*s' Vr='%.*s' SN='%.*s' is %smatching",
                                          static_cast<int>(strnlen(m->Mn, 32)), m->Mn,
                                          static_cast<int>(strnlen(m->Md, 32)), m->Md,
                                          static_cast<int>(strnlen(m->Opt, 16)), m->Opt,
                                          static_cast<int>(strnlen(m->Vr, 16)), m->Vr,
                                          static_cast<int>(strnlen(m->SN, 32)), m->SN,
                                          !scan_device_found ? "not " :"");

                    if (scan_device_found) {
                        if (is_kostal(m->Mn)) {
                            bool acc32_is_int32 = true;

                            if (strncmp(m->Md, "KOSTAL Smart Energy Meter", 25) == 0) {
                                // create null-terminated string from unterminated character sequence
                                char version_str[17];
                                memcpy(version_str, m->Vr, 16);
                                version_str[16] = 0;

                                SemanticVersion version;

                                if (!version.from_string(version_str, SemanticVersion::WithoutTimestamp)) {
                                    logger.printfln_meter("Could not parse KOSTAL Smart Energy Meter version: %s", version_str);
                                }
                                else if (version.compare(SemanticVersion{2, 6, 0}) >= 0) {
                                    acc32_is_int32 = false;
                                }
                            }

                            if (acc32_is_int32) {
                                quirks |= SUN_SPEC_QUIRKS_ACC32_IS_INT32;
                            }

                            quirks |= SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY;
                        }
                        else if (strncmp(m->Mn, "SMA", 32) == 0) {
                            quirks |= SUN_SPEC_QUIRKS_INTEGER_INVERTER_CURRENT_IS_INT16;
                            quirks |= SUN_SPEC_QUIRKS_INTEGER_INVERTER_POWER_FACTOR_IS_UNITY;
                        }
                        else if (is_solar_edge(m->Mn)) {
                            if (model_id >= 200 && model_id < 300) {
                                // Only meters are inverted, inverters are not.
                                quirks |= SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED;
                            }

                            quirks |= SUN_SPEC_QUIRKS_DER_PHASE_CURRENT_IS_UINT16;
                            quirks |= SUN_SPEC_QUIRKS_DER_PHASE_POWER_FACTOR_IS_UINT16;
                        }
                        else if (strncmp(m->Mn, "SUNGROW", 32) == 0) {
                            quirks |= SUN_SPEC_QUIRKS_INTEGER_INVERTER_POWER_FACTOR_IS_UNITY;
                        }
                        else if (strncmp(m->Mn, "TQ-Systems GmbH", 32) == 0) {
                            quirks |= SUN_SPEC_QUIRKS_ACC32_IS_INT32;
                            quirks |= SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY;
                        }

                        if (quirks) {
                            logger.printfln_meter("Enabling quirks mode 0x%02lx for %.32s device", quirks, m->Mn);
                        }
                    }
                }
                else {
                    logger.printfln_meter("Read full model %u for no reason", scan_model_id);
                }

                generic_read_request.start_address += 2 + block_length;
                generic_read_request.register_count = 2;
                scan_state_next = ScanState::ReadModelHeader;

                start_generic_read();
            }

            break;

        default:
            esp_system_abort("meter_sun_spec: Invalid state during scan");
    }
}
