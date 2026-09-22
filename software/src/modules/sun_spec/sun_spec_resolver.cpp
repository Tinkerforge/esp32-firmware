/* esp32-firmware
 * Copyright (C) 2026 Matthias Bolte <matthias@tinkerforge.com>
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

#include "sun_spec_resolver.h"

#include "event_log_prefix.h"
#include "generated/module_dependencies.h"
#include "tools/hexdump.h"
#include "modules/modbus_tcp_client/modbus_tcp_tools.h"

#include "gcc_warnings.h"

#define SUN_SPEC_ID 0x53756E53
#define COMMON_MODEL_ID 1
#define END_MODEL_ID 0xFFFF

#define print(fmt, ...) printfln_("%s" fmt, print_prefix __VA_OPT__(,) __VA_ARGS__)
#define trace(fmt, ...) tracefln_("%s" fmt, trace_prefix __VA_OPT__(,) __VA_ARGS__)

// The manufacturer name for SolarEdge devices sometimes has a trailing space
bool sun_spec_is_solar_edge(const char *manufacturer)
{
    return strcmp(manufacturer, "SolarEdge") == 0 || strcmp(manufacturer, "SolarEdge ") == 0;
}

// Since KOSTAL Smart Energy Meter firmware 2.6.0 the SunSpec manufacturer name
// got changed from "KOSTAL" to "KOSTAL Solar Electric GmbH"
bool sun_spec_is_kostal(const char *manufacturer)
{
    return strcmp(manufacturer, "KOSTAL") == 0 || strcmp(manufacturer, "KOSTAL Solar Electric GmbH") == 0;
}

// Do a prefix match to accept any KOSTAL Smart Energy Meter. Known models
// are "KOSTAL Smart Energy Meter G1" and "KOSTAL Smart Energy Meter G2"
bool sun_spec_is_kostal_smart_energy_meter(const char *model)
{
    return strncmp(model, "KOSTAL Smart Energy Meter", 25) == 0;
}

static const uint16_t base_addresses[] {
    40000,
    50000,
    0
};

SunSpecResolver *SunSpecResolver::create(const char *print_prefix,
                                         SunSpecResolverVLogFLnCallback &&vprintfln_callback,
                                         const char *trace_prefix,
                                         SunSpecResolverVLogFLnCallback &&vtracefln_callback,
                                         SunSpecResolverTracePlainCallback &&trace_plain_callback,
                                         TFGenericTCPSharedClient *shared_client,
                                         uint8_t device_address,
                                         const char *manufacturer_name,
                                         const char *model_name,
                                         const char *serial_number,
                                         uint16_t model_id,
                                         uint16_t model_instance,
                                         SunSpecResolverResultCallback &&result_callback,
                                         SunSpecResolverTimeoutCallback &&timeout_callback)
{
    SunSpecResolver *resolver = new SunSpecResolver;

    resolver->print_prefix = print_prefix;
    resolver->vprintfln_callback = std::move(vprintfln_callback);
    resolver->trace_prefix = trace_prefix;
    resolver->vtracefln_callback = std::move(vtracefln_callback);
    resolver->trace_plain_callback = std::move(trace_plain_callback);
    resolver->shared_client = shared_client;
    resolver->device_address = device_address;
    resolver->manufacturer_name = manufacturer_name;
    resolver->model_name = model_name;
    resolver->serial_number = serial_number;
    resolver->model_id = model_id;
    resolver->model_instance = model_instance;
    resolver->timeout_callback = std::move(timeout_callback);
    resolver->result_callback = std::move(result_callback);

    resolver->model_counter = model_instance;
    resolver->deserializer.buf = resolver->buffer;
    resolver->start_address = base_addresses[resolver->base_address_index];
    resolver->data_count = 2;

    resolver->print("Looking for SunSpec model %u/%u at %s:%u:%u",
                    model_id, model_instance, shared_client->get_host(), shared_client->get_port(), device_address);

    resolver->read();

    return resolver;
}

void SunSpecResolver::destroy()
{
    task_scheduler.cancel(read_task_id);
    read_task_id = 0;

    if (read_pending) {
        destroy_requested = true;
        return;
    }

    delete this;
}

void SunSpecResolver::printfln_(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintfln_callback(fmt, args);
    va_end(args);
}

void SunSpecResolver::tracefln_(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vtracefln_callback(fmt, args);
    va_end(args);
}

void SunSpecResolver::read()
{
    if (read_pending) {
        esp_system_abort("Previous read pending while trying to read");
    }

    read_pending = true;

    static_cast<TFModbusTCPSharedClient *>(shared_client)->transact(device_address,
                                                                    TFModbusTCPFunctionCode::ReadHoldingRegisters,
                                                                    start_address,
                                                                    data_count,
                                                                    static_cast<void *>(buffer),
                                                                    2_s,
    [this](TFModbusTCPClientTransactionResult result, const char *error_message) {
        read_pending = false;

        if (destroy_requested) {
            delete this;
            return;
        }

        if (last_read_result == result) {
            ++last_read_result_burst_length;
        }
        else {
            last_read_result = result;
            last_read_result_burst_length = 1;
        }

        if (result != TFModbusTCPClientTransactionResult::Success) {
            ++error_counter;

            trace("a%u c%u e%lu%s%s",
                  start_address,
                  data_count,
                  static_cast<uint32_t>(result),
                  error_message != nullptr ? " / " : "",
                  error_message != nullptr ? error_message : "");

            if (log_read_errors && (result != TFModbusTCPClientTransactionResult::Timeout || (last_read_result_burst_length % 10) == 0)) {
                print("Modbus error repeated %zu time%s while reading %u register%s starting at address %u: %s (%d)%s%s",
                      last_read_result_burst_length,
                      last_read_result_burst_length > 1 ? "s" : "",
                      data_count,
                      data_count > 1 ? "s" : "",
                      start_address,
                      get_tf_modbus_tcp_client_transaction_result_name(result),
                      static_cast<int>(result),
                      error_message != nullptr ? " / " : "",
                      error_message != nullptr ? error_message : "");
            }

            if (result == TFModbusTCPClientTransactionResult::NotConnected
             || result == TFModbusTCPClientTransactionResult::Aborted) {
                report_result(nullptr, 0, 0);
                return;
            }

            if (result == TFModbusTCPClientTransactionResult::Timeout) {
                timeout_callback();
            }

            if (state_next == State::ReadSunSpecID) {
                next_base_address();
                return;
            }

            if (error_counter >= 20) {
                print("Too many errors while looking for SunSpec model %u/%u at %s:%u:%u",
                      model_id, model_instance, shared_client->get_host(), shared_client->get_port(), device_address);
                report_result(nullptr, 0, 0);
                return;
            }

            task_scheduler.cancel(read_task_id);
            read_task_id = task_scheduler.scheduleOnce([this]() {
                read_task_id = 0;

                read();
            }, 1_s + (millis_t{esp_random() % 2000}));

            return;
        }

        char data_buf[ARRAY_SIZE(buffer) * 4 + 1]; // 4 nibble per register plus \n
        size_t data_buf_used;

        trace("a%u c%u", start_address, data_count);

        data_buf_used = hexdump(buffer, data_count, data_buf, ARRAY_SIZE(data_buf), HexdumpCase::Lower);
        data_buf[data_buf_used] = '\n';
        ++data_buf_used;

        trace_plain_callback(data_buf, data_buf_used);

        next();
    });
}

void SunSpecResolver::next_base_address()
{
    ++base_address_index;

    if (base_address_index >= ARRAY_SIZE(base_addresses)) {
        print("No SunSpec device at %s:%u:%u",
              shared_client->get_host(), shared_client->get_port(), device_address);
        report_result(nullptr, 0, 0);
        return;
    }

    error_counter = 0;
    state_next = State::ReadSunSpecID;
    start_address = base_addresses[base_address_index];
    data_count = 2;

    read();
}

void SunSpecResolver::next()
{
    deserializer.idx = 0;
    state = state_next;

    switch (state) {
    case State::Idle:
        esp_system_abort("Idle state during resolve");

    case State::ReadSunSpecID:
        if (deserializer.read_uint32() == SUN_SPEC_ID) {
            state_next = State::ReadModelHeader;
            start_address += data_count;
            data_count = 2;
            log_read_errors = true; // start to log errors after the correct base address is found

            read();
        }
        else {
            next_base_address();
        }

        break;

    case State::ReadModelHeader: {
            uint16_t candidate_model_id = deserializer.read_uint16();
            uint16_t candidate_block_length = deserializer.read_uint16();

            if (candidate_model_id == END_MODEL_ID) {
                print("SunSpec model %u/%u not found at %s:%u:%u",
                      model_id, model_instance, shared_client->get_host(), shared_client->get_port(), device_address);
                report_result(nullptr, 0, 0);
            }
            else if (device_found && candidate_model_id == model_id) {
                if (model_counter > 0) {
                    --model_counter;

                    start_address += static_cast<uint16_t>(data_count + candidate_block_length);
                    data_count = 2;

                    read();
                }
                else {
                    print("SunSpec model %u/%u found at %s:%u:%u:%u",
                          model_id, model_instance, shared_client->get_host(), shared_client->get_port(), device_address, start_address);
                    report_result(&common_model, start_address, candidate_block_length);
                }
            }
            else if (candidate_model_id == COMMON_MODEL_ID) {
                state_next = State::ReadModel;
                data_count = 67;

                read();
            }
            else {
                start_address += static_cast<uint16_t>(data_count + candidate_block_length);
                data_count = 2;

                read();
            }
        }

        break;

    case State::ReadModel: {
            uint16_t candidate_model_id = deserializer.read_uint16();
            uint16_t candidate_block_length = deserializer.read_uint16();

            if (candidate_model_id == COMMON_MODEL_ID) {
                deserializer.read_string(common_model.Mn, sizeof(common_model.Mn));
                deserializer.read_string(common_model.Md, sizeof(common_model.Md));
                deserializer.read_string(common_model.Opt, sizeof(common_model.Opt));
                deserializer.read_string(common_model.Vr, sizeof(common_model.Vr));
                deserializer.read_string(common_model.SN, sizeof(common_model.SN));

                print("Looking for SunSpec device Mn='%s' Md='%s' SN='%s'", manufacturer_name, model_name, serial_number);

                if (*manufacturer_name == '\0' && *model_name == '\0' && *serial_number == '\0') {
                    device_found = true;
                }
                else if (sun_spec_is_solar_edge(common_model.Mn) &&
                         strcmp(common_model.Md, "SE-RGMTR-1D-240C-A") == 0 &&
                         strcmp(common_model.SN, "0") == 0 &&
                         sun_spec_is_solar_edge(manufacturer_name) &&
                         strcmp(model_name, "MTR-240-3PC1-D-A-MW") == 0) {
                    // Sometimes SolarEdge inverters report a MTR-240-3PC1-D-A-MW meter wrongly
                    // as a SE-RGMTR-1D-240C-A meter with serial number 0. Work around this by
                    // accepting a SE-RGMTR-1D-240C-A meter with serial number 0 when looking
                    // for a MTR-240-3PC1-D-A-MW meter.
                    device_found = true;
                }
                else if (sun_spec_is_solar_edge(common_model.Mn) &&
                         strcmp(common_model.Md, "MTR-240-3PC1-D-A-MW") == 0 &&
                         sun_spec_is_solar_edge(manufacturer_name) &&
                         strcmp(model_name, "SE-RGMTR-1D-240C-A") == 0 &&
                         strcmp(serial_number, "0") == 0) {
                    // A MTR-240-3PC1-D-A-MW meter might have been configured while it was wrongly
                    // reported as SE-RGMTR-1D-240C-A meter with serial number 0. But now it is
                    // correctly reported again. Work around this by accepting a MTR-240-3PC1-D-A-MW
                    // meter when looking for a SE-RGMTR-1D-240C-A meter with serial number 0.
                    device_found = true;
                }
                else {
                    bool manufacturer_match = strcmp(common_model.Mn, manufacturer_name) == 0 ||
                                              (sun_spec_is_solar_edge(common_model.Mn) && sun_spec_is_solar_edge(manufacturer_name)) ||
                                              (sun_spec_is_kostal(common_model.Mn) && sun_spec_is_kostal(manufacturer_name));

                    device_found = manufacturer_match &&
                                   strcmp(common_model.Md, model_name) == 0 &&
                                   strcmp(common_model.SN, serial_number) == 0;
                }

                print("SunSpec device Mn='%s' Md='%s' Opt='%s' Vr='%s' SN='%s' is %smatching",
                      common_model.Mn, common_model.Md, common_model.Opt, common_model.Vr, common_model.SN, !device_found ? "not " :"");
            }
            else {
                print("Read full SunSpec model %u for no reason", candidate_model_id);
            }

            state_next = State::ReadModelHeader;
            start_address += static_cast<uint16_t>(2 + candidate_block_length);
            data_count = 2;

            read();
        }

        break;

    default:
        esp_system_abortf<48>("Invalid state during resolve: %d", static_cast<int>(state));
    }
}

void SunSpecResolver::report_result(SunSpecResolverCommonModel *common_model_, uint16_t start_address_, uint16_t block_length)
{
    result_callback(common_model_, start_address_, block_length);
    destroy();
}
