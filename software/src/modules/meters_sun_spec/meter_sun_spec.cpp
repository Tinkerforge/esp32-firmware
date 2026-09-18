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
#include "generated/module_dependencies.h"
#include "models/model_001.h"
#include "tools/semantic_version.h"
#include "tools/hexdump.h"

#include "gcc_warnings.h"

#define RESOLVE_TIMEOUT 1_min
#define SUCCESSFUL_PARSE_TIMEOUT 1_min

#define trace(fmt, ...) \
    do { \
        meters_sun_spec.trace_timestamp(); \
        logger.tracefln_plain(meters_sun_spec.trace_buffer_index, fmt __VA_OPT__(,) __VA_ARGS__); \
    } while (0)

static MeterLocation get_model_fixed_location(uint16_t model_id)
{
    switch (model_id) {
    case 101: return MeterLocation::Inverter;
    case 102: return MeterLocation::Inverter;
    case 103: return MeterLocation::Inverter;
    case 111: return MeterLocation::Inverter;
    case 112: return MeterLocation::Inverter;
    case 113: return MeterLocation::Inverter;
    case 120: return MeterLocation::Inverter;
    case 121: return MeterLocation::Inverter;
    case 122: return MeterLocation::Inverter;
    case 123: return MeterLocation::Inverter;
    case 124: return MeterLocation::Inverter;
    case 125: return MeterLocation::Inverter;
    case 126: return MeterLocation::Inverter;
    case 127: return MeterLocation::Inverter;
    case 128: return MeterLocation::Inverter;
    case 129: return MeterLocation::Inverter;
    case 130: return MeterLocation::Inverter;
    case 131: return MeterLocation::Inverter;
    case 132: return MeterLocation::Inverter;
    case 133: return MeterLocation::Inverter;
    case 134: return MeterLocation::Inverter;
    case 135: return MeterLocation::Inverter;
    case 136: return MeterLocation::Inverter;
    case 137: return MeterLocation::Inverter;
    case 138: return MeterLocation::Inverter;
    case 139: return MeterLocation::Inverter;
    case 140: return MeterLocation::Inverter;
    case 141: return MeterLocation::Inverter;
    case 142: return MeterLocation::Inverter;
    case 143: return MeterLocation::Inverter;
    case 144: return MeterLocation::Inverter;
    case 145: return MeterLocation::Inverter;
    case 160: return MeterLocation::PV;
    case 701: return MeterLocation::Inverter;
    case 713: return MeterLocation::Battery;
    case 801: return MeterLocation::Battery;
    case 802: return MeterLocation::Battery;
    case 803: return MeterLocation::Battery;
    case 804: return MeterLocation::Battery;
    case 805: return MeterLocation::Battery;
    case 806: return MeterLocation::Battery;
    case 807: return MeterLocation::Battery;
    case 808: return MeterLocation::Battery;
    case 809: return MeterLocation::Battery;
    default:  return MeterLocation::Unknown;
    }
}

MeterClassID MeterSunSpec::get_class() const
{
    return MeterClassID::SunSpec;
}

void MeterSunSpec::setup(Config *ephemeral_config)
{
    snprintf(trace_log_message_prefix, sizeof(trace_log_message_prefix), "m%lur ", slot);

    host              = ephemeral_config->get("host")->asString();
    port              = ephemeral_config->get("port")->asUint16();
    device_address    = ephemeral_config->get("device_address")->asUint8();
    manufacturer_name = ephemeral_config->get("manufacturer_name")->asString();
    model_name        = ephemeral_config->get("model_name")->asString();
    serial_number     = ephemeral_config->get("serial_number")->asString();
    model_id          = ephemeral_config->get("model_id")->asUint16();
    model_instance    = ephemeral_config->get("model_instance")->asUint16();
    model_parser      = MetersSunSpecParser::new_parser(slot, manufacturer_name.c_str(), model_name.c_str(), model_id, ephemeral_config->get("dc_port_type")->asEnum<DCPortType>());

    MeterLocation fixed_location = get_model_fixed_location(model_id);

    if (fixed_location != MeterLocation::Unknown) {
        ephemeral_config->get("location")->updateEnum(fixed_location);
    }

    location = ephemeral_config->get("location")->asEnum<MeterLocation>();

    if (model_parser == nullptr) {
        logger.printfln_meter("No parser available for model %u", model_id);
        return;
    }

    task_scheduler.scheduleUncancelable([this]() {
        if (read_allowed) {
            read_allowed = false;

            if (deadline_elapsed(last_successful_parse + SUCCESSFUL_PARSE_TIMEOUT)) {
                logger.printfln("Last successful parse occurred too long ago, reconnecting to %s:%u", host.c_str(), port);
                force_reconnect();
                return;
            }

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

void MeterSunSpec::connect_callback(TFGenericTCPClientConnectResult result, TFGenericTCPClientPoolShareLevel share_level)
{
    GenericModbusTCPClient::connect_callback(result, share_level);

    trace("m%lu c%d sl%d", slot, static_cast<int>(result), static_cast<int>(share_level));

    if (result != TFGenericTCPClientConnectResult::Connected) {
        return;
    }

    last_connect = now_us();
    last_successful_parse = now_us();

    resolve_start();
}

void MeterSunSpec::disconnect_callback(TFGenericTCPClientDisconnectReason reason, TFGenericTCPClientPoolShareLevel share_level)
{
    trace("m%lu d%d sl%d", slot, static_cast<int>(reason), static_cast<int>(share_level));

    read_allowed = false;

    task_scheduler.cancel(resolve_start_delayed_task_id);
    resolve_start_delayed_task_id = 0;

    if (resolver != nullptr) {
        resolver->destroy();
        resolver = nullptr;
    }

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

                logger.trace_plain(meters_sun_spec.trace_buffer_index, data_buf, data_buf_used);
            }
        }
    }
}

void MeterSunSpec::read_start(size_t start_address, size_t model_regcount)
{
    if (!alloc_read_buffer(model_regcount)) {
        return; // this is fatal, the reading will not be restarted till the next reconnect
    }

    generic_read_request.start_address = start_address;
    generic_read_request.register_type = ModbusRegisterType::HoldingRegister;
    generic_read_request.register_count = model_regcount;
    generic_read_request.done_callback = [this]{ read_done(); };

    start_generic_read();
}

void MeterSunSpec::read_done()
{
    read_allowed = true;

    trace_response();

    if (generic_read_request.result != TFModbusTCPClientTransactionResult::Success) {
        if (generic_read_request.result == TFModbusTCPClientTransactionResult::Aborted) {
            // an abort is triggered before a connection close or before a forced
            // reconnect, stop reading. in both cases the reading will be restarted
            // by the automatic reconnect
            read_allowed = false;
        }
        else if (generic_read_request.result == TFModbusTCPClientTransactionResult::Timeout) {
            record_timeout();
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
                read_allowed = false; // this is fatal, the reading will not be restarted till the next reconnect
            }

            return;
        }
    }

    if (!model_parser->parse_values(generic_read_request.data, quirks)) {
        auto inconsistency = errors->get("inconsistency");
        inconsistency->updateUint(inconsistency->asUint() + 1);
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
            }
            else {
                last_successful_parse = now_us();
            }
        }
    }
}

void MeterSunSpec::record_timeout()
{
    auto timeout = errors->get("timeout");
    timeout->updateUint(timeout->asUint() + 1);
}

void MeterSunSpec::resolve_start_delayed()
{
    task_scheduler.cancel(resolve_start_delayed_task_id);
    resolve_start_delayed_task_id = 0;

    if (shared_client == nullptr || shared_client->get_connection_status() != TFGenericTCPClientConnectionStatus::Connected) {
        return;
    }

    resolve_start_delayed_task_id = task_scheduler.scheduleOnce([this](){
        resolve_start_delayed_task_id = 0;

        if (deadline_elapsed(last_connect + RESOLVE_TIMEOUT)) {
            logger.printfln_meter("Looking for SunSpec model %u/%u takes too long, reconnecting to %s:%u",
                                  model_id, model_instance, host.c_str(), port);
            force_reconnect();
            return;
        }

        resolve_start();
    }, 5_s);
}

void MeterSunSpec::resolve_start()
{
    if (resolver != nullptr) {
        resolver->destroy();
    }

    resolver = SunSpecResolver::create(event_log_prefix_override,
                                       event_log_message_prefix,
                                       []() { meters_sun_spec.trace_timestamp(); },
                                       meters_sun_spec.trace_buffer_index,
                                       trace_log_message_prefix,
                                       shared_client,
                                       device_address,
                                       manufacturer_name.c_str(),
                                       model_name.c_str(),
                                       serial_number.c_str(),
                                       model_id,
                                       model_instance,
                                       [this](SunSpecResolverCommonModel *common_model, size_t start_address, size_t block_length) { resolve_result(common_model, start_address, block_length); },
                                       [this]() { record_timeout(); });
}

void MeterSunSpec::resolve_result(SunSpecResolverCommonModel *common_model, size_t start_address, size_t block_length)
{
    resolver = nullptr;

    if (common_model == nullptr) {
        resolve_start_delayed();
        return;
    }

    if (!model_parser->is_model_length_supported(block_length)) {
        logger.printfln_meter("SunSpec model %u/%u at %s:%u:%u:%u has unsupported length: %u",
                              model_id, model_instance, host.c_str(), port, device_address, start_address, block_length);
        resolve_start_delayed();
        return;
    }

    quirks = 0;

    if (sun_spec_is_kostal(common_model->Mn)) {
        bool acc32_is_int32 = true;

        if (sun_spec_is_kostal_smart_energy_meter(common_model->Md)) {
            SemanticVersion version;

            if (!version.from_string(common_model->Vr, SemanticVersion::WithoutTimestamp)) {
                logger.printfln_meter("Could not parse KOSTAL Smart Energy Meter version: %s", common_model->Vr);
            }
            else if (version.compare(SemanticVersion{2, 6, 0}) >= 0) {
                // KOSTAL fixed this bug in version 2.6.0
                acc32_is_int32 = false;
            }
        }

        if (acc32_is_int32) {
            quirks |= SUN_SPEC_QUIRKS_ACC32_IS_INT32;
        }

        quirks |= SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY;
    }
    else if (strcmp(common_model->Mn, "SMA") == 0) {
        quirks |= SUN_SPEC_QUIRKS_INTEGER_INVERTER_CURRENT_IS_INT16;
        quirks |= SUN_SPEC_QUIRKS_INTEGER_INVERTER_POWER_FACTOR_IS_UNITY;
    }
    else if (sun_spec_is_solar_edge(common_model->Mn)) {
        if (model_id >= 200 && model_id < 300) {
            // Only meters are inverted, inverters are not.
            quirks |= SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED;
        }

        quirks |= SUN_SPEC_QUIRKS_DER_PHASE_CURRENT_IS_UINT16;
        quirks |= SUN_SPEC_QUIRKS_DER_PHASE_POWER_FACTOR_IS_UINT16;
    }
    else if (strcmp(common_model->Mn, "WattNode") == 0) {
        quirks |= SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED;
        quirks |= SUN_SPEC_QUIRKS_PHASE_TO_PHASE_VOLTAGE_IS_UINT16;
    }
    else if (strcmp(common_model->Mn, "SUNGROW") == 0) {
        quirks |= SUN_SPEC_QUIRKS_INTEGER_INVERTER_POWER_FACTOR_IS_UNITY;
    }
    else if (strcmp(common_model->Mn, "TQ-Systems GmbH") == 0) {
        quirks |= SUN_SPEC_QUIRKS_ACC32_IS_INT32;
        quirks |= SUN_SPEC_QUIRKS_INTEGER_METER_POWER_FACTOR_IS_UNITY;
    }
    else if (strcmp(common_model->Mn, "Fronius") == 0) {
        if (model_id >= 200 && model_id < 300 && location == MeterLocation::Load) {
            // in the Fronius world model import is posivtive and export is negative,
            // the same as in our world model. but in the Fronius world model loads
            // are viewed from the perspective of the inverter, not viewed from the
            // perspective of the load itself. this means that a load that is importing
            // has a positive power value in our world model, but in the Fronius world
            // model the inverter is exporting to the load, resulting in a negative
            // power value reported by the meter. therefore, we need to invert the power
            // value reported by Fronius load meters.
            quirks |= SUN_SPEC_QUIRKS_ACTIVE_POWER_IS_INVERTED;
        }
    }

    if (quirks != 0) {
        logger.printfln_meter("Enabling SunSpec quirks mode 0x%02lx for %s device", quirks, common_model->Mn);
    }

    read_start(start_address, model_parser->get_interesting_registers_count());
}
