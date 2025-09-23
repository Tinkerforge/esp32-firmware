/* esp32-firmware
 * Copyright (C) 2023 Mattias Schäffersmann <mattias@tinkerforge.com>
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

#define EVENT_LOG_PREFIX "modbus_meter_sim"

#include "modbus_meter_simulator.h"

#include <math.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "options.h"
#include "bindings/hal_common.h"
#include "modules/meters/meter_defs.h"

#include "gcc_warnings.h"

extern TF_HAL hal;

static constexpr MeterValueID sdm_register_map_meter_values[MODBUS_METER_SIMULATOR_REGISTER_COUNT] = {
    MeterValueID::VoltageL1N,                        // 30001
    MeterValueID::VoltageL2N,                        // 30003
    MeterValueID::VoltageL3N,                        // 30005
    MeterValueID::CurrentL1ImExSum,                  // 30007
    MeterValueID::CurrentL2ImExSum,                  // 30009
    MeterValueID::CurrentL3ImExSum,                  // 30011
    MeterValueID::PowerActiveL1ImExDiff,             // 30013
    MeterValueID::PowerActiveL2ImExDiff,             // 30015
    MeterValueID::PowerActiveL3ImExDiff,             // 30017
    MeterValueID::PowerApparentL1ImExSum,            // 30019
    MeterValueID::PowerApparentL2ImExSum,            // 30021
    MeterValueID::PowerApparentL3ImExSum,            // 30023
    MeterValueID::PowerReactiveL1IndCapDiff,         // 30025
    MeterValueID::PowerReactiveL2IndCapDiff,         // 30027
    MeterValueID::PowerReactiveL3IndCapDiff,         // 30029
    MeterValueID::PowerFactorL1Directional,          // 30031
    MeterValueID::PowerFactorL2Directional,          // 30033
    MeterValueID::PowerFactorL3Directional,          // 30035
    MeterValueID::PhaseAngleL1,                      // 30037
    MeterValueID::PhaseAngleL2,                      // 30039
    MeterValueID::PhaseAngleL3,                      // 30041
    MeterValueID::VoltageLNAvg,                      // 30043
    MeterValueID::NotSupported,                      // 30045
    MeterValueID::CurrentLAvgImExSum,                // 30047
    MeterValueID::CurrentLSumImExSum,                // 30049
    MeterValueID::NotSupported,                      // 30051
    MeterValueID::PowerActiveLSumImExDiff,           // 30053
    MeterValueID::NotSupported,                      // 30055
    MeterValueID::PowerApparentLSumImExSum,          // 30057
    MeterValueID::NotSupported,                      // 30059
    MeterValueID::PowerReactiveLSumIndCapDiff,       // 30061
    MeterValueID::PowerFactorLSumDirectional,        // 30063
    MeterValueID::NotSupported,                      // 30065
    MeterValueID::PhaseAngleLSum,                    // 30067
    MeterValueID::NotSupported,                      // 30069
    MeterValueID::FrequencyLAvg,                     // 30071
    MeterValueID::EnergyActiveLSumImport,            // 30073
    MeterValueID::EnergyActiveLSumExport,            // 30075
    MeterValueID::EnergyReactiveLSumInductive,       // 30077
    MeterValueID::EnergyReactiveLSumCapacitive,      // 30079
    MeterValueID::EnergyApparentLSumImExSum,         // 30081
    MeterValueID::ElectricCharge,                    // 30083
    MeterValueID::NotSupported,                      // 30085
    MeterValueID::NotSupported,                      // 30087
    MeterValueID::NotSupported,                      // 30089
    MeterValueID::NotSupported,                      // 30091
    MeterValueID::NotSupported,                      // 30093
    MeterValueID::NotSupported,                      // 30095
    MeterValueID::NotSupported,                      // 30097
    MeterValueID::NotSupported,                      // 30099
    MeterValueID::NotSupported,                      // 30101
    MeterValueID::NotSupported,                      // 30103
    MeterValueID::NotSupported,                      // 30105
    MeterValueID::NotSupported,                      // 30107
    MeterValueID::NotSupported,                      // 30109
    MeterValueID::NotSupported,                      // 30111
    MeterValueID::NotSupported,                      // 30113
    MeterValueID::NotSupported,                      // 30115
    MeterValueID::NotSupported,                      // 30117
    MeterValueID::NotSupported,                      // 30119
    MeterValueID::NotSupported,                      // 30121
    MeterValueID::NotSupported,                      // 30123
    MeterValueID::NotSupported,                      // 30125
    MeterValueID::NotSupported,                      // 30127
    MeterValueID::NotSupported,                      // 30129
    MeterValueID::NotSupported,                      // 30131
    MeterValueID::NotSupported,                      // 30133
    MeterValueID::NotSupported,                      // 30135
    MeterValueID::NotSupported,                      // 30137
    MeterValueID::NotSupported,                      // 30139
    MeterValueID::NotSupported,                      // 30141
    MeterValueID::NotSupported,                      // 30143
    MeterValueID::NotSupported,                      // 30145
    MeterValueID::NotSupported,                      // 30147
    MeterValueID::NotSupported,                      // 30149
    MeterValueID::NotSupported,                      // 30151
    MeterValueID::NotSupported,                      // 30153
    MeterValueID::NotSupported,                      // 30155
    MeterValueID::NotSupported,                      // 30157
    MeterValueID::NotSupported,                      // 30159
    MeterValueID::NotSupported,                      // 30161
    MeterValueID::NotSupported,                      // 30163
    MeterValueID::NotSupported,                      // 30165
    MeterValueID::NotSupported,                      // 30167
    MeterValueID::NotSupported,                      // 30169
    MeterValueID::NotSupported,                      // 30171
    MeterValueID::NotSupported,                      // 30173
    MeterValueID::NotSupported,                      // 30175
    MeterValueID::NotSupported,                      // 30177
    MeterValueID::NotSupported,                      // 30179
    MeterValueID::NotSupported,                      // 30181
    MeterValueID::NotSupported,                      // 30183
    MeterValueID::NotSupported,                      // 30185
    MeterValueID::NotSupported,                      // 30187
    MeterValueID::NotSupported,                      // 30189
    MeterValueID::NotSupported,                      // 30191
    MeterValueID::NotSupported,                      // 30193
    MeterValueID::NotSupported,                      // 30195
    MeterValueID::NotSupported,                      // 30197
    MeterValueID::NotSupported,                      // 30199
    MeterValueID::VoltageL1L2,                       // 30201
    MeterValueID::VoltageL2L3,                       // 30203
    MeterValueID::VoltageL3L1,                       // 30205
    MeterValueID::VoltageLLAvg,                      // 30207
    MeterValueID::NotSupported,                      // 30209
    MeterValueID::NotSupported,                      // 30211
    MeterValueID::NotSupported,                      // 30213
    MeterValueID::NotSupported,                      // 30215
    MeterValueID::NotSupported,                      // 30217
    MeterValueID::NotSupported,                      // 30219
    MeterValueID::NotSupported,                      // 30221
    MeterValueID::NotSupported,                      // 30223
    MeterValueID::CurrentNImExSum,                   // 30225
    MeterValueID::NotSupported,                      // 30227
    MeterValueID::NotSupported,                      // 30229
    MeterValueID::NotSupported,                      // 30231
    MeterValueID::NotSupported,                      // 30233
    MeterValueID::VoltageTHDL1N,                     // 30235
    MeterValueID::VoltageTHDL2N,                     // 30237
    MeterValueID::VoltageTHDL3N,                     // 30239
    MeterValueID::CurrentTHDL1,                      // 30241
    MeterValueID::CurrentTHDL2,                      // 30243
    MeterValueID::CurrentTHDL3,                      // 30245
    MeterValueID::NotSupported,                      // 30247
    MeterValueID::VoltageTHDLNAvg,                   // 30249
    MeterValueID::CurrentTHDLAvg,                    // 30251
    MeterValueID::NotSupported,                      // 30253
    MeterValueID::NotSupported,                      // 30255
    MeterValueID::NotSupported,                      // 30257
    MeterValueID::NotSupported,                      // 30259
    MeterValueID::NotSupported,                      // 30261
    MeterValueID::NotSupported,                      // 30263
    MeterValueID::NotSupported,                      // 30265
    MeterValueID::NotSupported,                      // 30267
    MeterValueID::NotSupported,                      // 30269
    MeterValueID::NotSupported,                      // 30271
    MeterValueID::NotSupported,                      // 30273
    MeterValueID::NotSupported,                      // 30275
    MeterValueID::NotSupported,                      // 30277
    MeterValueID::NotSupported,                      // 30279
    MeterValueID::NotSupported,                      // 30281
    MeterValueID::NotSupported,                      // 30283
    MeterValueID::NotSupported,                      // 30285
    MeterValueID::NotSupported,                      // 30287
    MeterValueID::NotSupported,                      // 30289
    MeterValueID::NotSupported,                      // 30291
    MeterValueID::NotSupported,                      // 30293
    MeterValueID::NotSupported,                      // 30295
    MeterValueID::NotSupported,                      // 30297
    MeterValueID::NotSupported,                      // 30299
    MeterValueID::NotSupported,                      // 30301
    MeterValueID::NotSupported,                      // 30303
    MeterValueID::NotSupported,                      // 30305
    MeterValueID::NotSupported,                      // 30307
    MeterValueID::NotSupported,                      // 30309
    MeterValueID::NotSupported,                      // 30311
    MeterValueID::NotSupported,                      // 30313
    MeterValueID::NotSupported,                      // 30315
    MeterValueID::NotSupported,                      // 30317
    MeterValueID::NotSupported,                      // 30319
    MeterValueID::NotSupported,                      // 30321
    MeterValueID::NotSupported,                      // 30323
    MeterValueID::NotSupported,                      // 30325
    MeterValueID::NotSupported,                      // 30327
    MeterValueID::NotSupported,                      // 30329
    MeterValueID::NotSupported,                      // 30331
    MeterValueID::NotSupported,                      // 30333
    MeterValueID::VoltageTHDL1L2,                    // 30335
    MeterValueID::VoltageTHDL2L3,                    // 30337
    MeterValueID::VoltageTHDL3L1,                    // 30339
    MeterValueID::VoltageTHDLLAvg,                   // 30341
    MeterValueID::EnergyActiveLSumImExSum,           // 30343
    MeterValueID::EnergyReactiveLSumIndCapSum,       // 30345
    MeterValueID::EnergyActiveL1Import,              // 30347
    MeterValueID::EnergyActiveL2Import,              // 30349
    MeterValueID::EnergyActiveL3Import,              // 30351
    MeterValueID::EnergyActiveL1Export,              // 30353
    MeterValueID::EnergyActiveL2Export,              // 30355
    MeterValueID::EnergyActiveL3Export,              // 30357
    MeterValueID::EnergyActiveL1ImExSum,             // 30359
    MeterValueID::EnergyActiveL2ImExSum,             // 30361
    MeterValueID::EnergyActiveL3ImExSum,             // 30363
    MeterValueID::EnergyReactiveL1Inductive,         // 30365
    MeterValueID::EnergyReactiveL2Inductive,         // 30367
    MeterValueID::EnergyReactiveL3Inductive,         // 30369
    MeterValueID::EnergyReactiveL1Capacitive,        // 30371
    MeterValueID::EnergyReactiveL2Capacitive,        // 30373
    MeterValueID::EnergyReactiveL3Capacitive,        // 30375
    MeterValueID::EnergyReactiveL1IndCapSum,         // 30377
    MeterValueID::EnergyReactiveL2IndCapSum,         // 30379
    MeterValueID::EnergyReactiveL3IndCapSum,         // 30381
    MeterValueID::NotSupported,                      // 30383
    MeterValueID::EnergyActiveLSumImExSumResettable, // 30385
    MeterValueID::NotSupported,                      // 30387
    MeterValueID::EnergyActiveLSumImportResettable,  // 30389
    MeterValueID::EnergyActiveLSumExportResettable,  // 30391
};

static const uint16_t supported_meter_ids[] = {
    0,      // None
    0x0200, // SDM72
    0x0070, // SDM630
    0x0089, // SDM72v2
    0,      // SDM72CTM?
    0x0079, // SDM630MCTv2
};

static void convert_float_to_regs(uint16_t *regs, float floatval)
{
    union {
        float f;
        uint16_t regs[2];
    } value;

    value.f = floatval;

    regs[1] = value.regs[0];
    regs[0] = value.regs[1];
}

static float convert_regs_to_float(const uint16_t *regs)
{
    union {
        float f;
        uint16_t regs[2];
    } value;

    value.regs[1] = regs[0];
    value.regs[0] = regs[1];

    return value.f;
}

void ModbusMeterSimulator::pre_setup()
{
    config = Config::Object({
        {"meter_type",        Config::Uint8(METER_TYPE_NONE)},
        {"source_meter_slot", Config::Uint(0, 0, OPTIONS_METERS_MAX_SLOTS() - 1)},
    });
}

void ModbusMeterSimulator::setup()
{
    api.restorePersistentConfig("modbus_meter_simulator/config", &config);

    uint32_t meter_type = config.get("meter_type")->asUint();
    if (meter_type < std::size(supported_meter_ids)) {
        meter_id = supported_meter_ids[meter_type];
    }

    if (meter_id == 0) {
        logger.printfln("Unsupported meter type %lu. Disabling simulator.", meter_type);
        initialized = true; // Mark as initialized so that the front-end shows up.
        return;
    }

    source_meter_slot = config.get("source_meter_slot")->asUint();

    for (size_t i = 0; i < std::size(value_index_cache); i++) {
        value_index_cache[i] = UINT32_MAX;
    }

    memset(unsupported_input_register_cache, 0, sizeof(unsupported_input_register_cache));

    setupRS485();
    if (!initialized)
        return;

    task_scheduler.scheduleWithFixedDelay([this](){
        this->checkRS485State();
    }, 10_s, 10_s);
}

void ModbusMeterSimulator::register_urls()
{
    api.addPersistentConfig("modbus_meter_simulator/config", &config);
}

void ModbusMeterSimulator::register_events()
{
    if (source_meter_slot >= OPTIONS_METERS_MAX_SLOTS())
        return;

    String value_ids_path = meters.get_path(source_meter_slot, Meters::PathType::ValueIDs);

    const Config *old_value_ids = api.getState(value_ids_path);
    if (old_value_ids->count() > 0) {
        on_value_ids_change(old_value_ids);
    } else {
        event.registerEvent(value_ids_path, {}, [this](const Config *event_value_ids) {
            return on_value_ids_change(event_value_ids);
        });
    }
}

void ModbusMeterSimulator::setupRS485()
{
    if (tf_rs485_create(&bricklet, nullptr, &hal) != TF_E_OK) {
        logger.printfln("No RS485 bricklet found. Disabling simulator.");
        return;
    }

    int result = tf_rs485_set_mode(&bricklet, TF_RS485_MODE_MODBUS_SLAVE_RTU);
    if (result != TF_E_OK) {
        logger.printfln("RS485 set mode failed (rc %d). Disabling simulator.", result);
        return;
    }

    result = tf_rs485_set_rs485_configuration(&bricklet, 9600, TF_RS485_PARITY_NONE, TF_RS485_STOPBITS_1, TF_RS485_WORDLENGTH_8, TF_RS485_DUPLEX_HALF);
    if (result != TF_E_OK) {
        logger.printfln("RS485 set config failed (rc %d). Disabling simulator.", result);
        return;
    }

    result = tf_rs485_set_modbus_configuration(&bricklet, 1, 0);
    if (result != TF_E_OK) {
        logger.printfln("RS485 set modbus config failed (rc %d). Disabling simulator.", result);
        return;
    }

    // Used callbacks
    tf_rs485_register_modbus_slave_write_multiple_registers_request_callback(&bricklet, [](struct TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t *registers, uint16_t registers_length, void *user_data) {
        task_scheduler.scheduleOnce([request_id, starting_address, registers, registers_length, user_data]() {
            static_cast<ModbusMeterSimulator *>(user_data)->modbus_slave_write_multiple_registers_request_handler(request_id, starting_address, registers, registers_length);
        });
    }, write_registers_callback_buffer, this);

    tf_rs485_register_modbus_slave_read_holding_registers_request_callback(&bricklet, [](TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t count, void *user_data) {
        task_scheduler.scheduleOnce([request_id, starting_address, count, user_data]() {
            static_cast<ModbusMeterSimulator *>(user_data)->modbus_slave_read_holding_registers_request_handler(request_id, starting_address, count);
        });
    }, this);

    tf_rs485_register_modbus_slave_read_input_registers_request_callback(&bricklet, [](TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t count, void *user_data) {
        task_scheduler.scheduleOnce([request_id, starting_address, count, user_data]() {
            static_cast<ModbusMeterSimulator *>(user_data)->modbus_slave_read_input_registers_request_handler(request_id, starting_address, count);
        });
    }, this);

    // Unused callbacks
    tf_rs485_register_modbus_slave_write_single_coil_request_callback(&bricklet, [](TF_RS485 *rs485, uint8_t request_id, uint32_t coil_address, bool coil_value, void *user_data) {
        logger.printfln("No handler implementation for request 'write single coil' with coil address %lu", coil_address);
    }, this);

    tf_rs485_register_modbus_slave_write_multiple_coils_request_low_level_callback(&bricklet, [](struct TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t coils_length, uint16_t coils_chunk_offset, bool coils_chunk_data[440], void *user_data) {
        logger.printfln("No handler implementation for request 'write multiple coils' with starting address %lu", starting_address);
    }, this);

    tf_rs485_register_modbus_slave_write_single_register_request_callback(&bricklet, [](TF_RS485 *device, uint8_t request_id, uint32_t register_address, uint16_t register_value, void *user_data) {
        logger.printfln("No handler implementation for request 'write single register' with register address %lu", register_address);
    }, this);

    tf_rs485_register_modbus_slave_read_coils_request_callback(&bricklet, [](TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t count, void *user_data) {
        logger.printfln("No handler implementation for request 'read coils' with starting address %lu", starting_address);
    }, this);

    tf_rs485_register_modbus_slave_read_discrete_inputs_request_callback(&bricklet, [](TF_RS485 *rs485, uint8_t request_id, uint32_t starting_address, uint16_t count, void *user_data) {
        logger.printfln("No handler implementation for request 'read discrete inputs' with starting address %lu", starting_address);
    }, this);

    initialized = true;
}

void ModbusMeterSimulator::checkRS485State()
{
    uint8_t mode = 0;
    int result = tf_rs485_get_mode(&bricklet, &mode);
    if (result != TF_E_OK) {
        logger.printfln("Failed to get RS485 mode, rc: %d", result);
        return;
    }
    if (mode != TF_RS485_MODE_MODBUS_SLAVE_RTU) {
        logger.printfln("RS485 mode invalid (%u). Did the bricklet reset?", mode);
        setupRS485();
    }
}

EventResult ModbusMeterSimulator::on_value_ids_change(const Config *value_ids)
{
    if (value_ids->count() == 0) {
        return EventResult::OK;
    }

    meters.fill_index_cache(source_meter_slot, std::size(sdm_register_map_meter_values), sdm_register_map_meter_values, value_index_cache);

    have_source_values = true;

    return EventResult::Deregister;
}

uint32_t ModbusMeterSimulator::register_address2cached_index(uint32_t register_address)
{
    if (register_address == 0)
        return UINT32_MAX;

    uint32_t index = (register_address - 1) / 2;
    if (index >= std::size(value_index_cache))
        return UINT32_MAX;

    return value_index_cache[index];
}

void ModbusMeterSimulator::modbus_slave_write_multiple_registers_request_handler(uint8_t request_id, uint32_t starting_address, uint16_t *registers, uint16_t registers_length)
{
    if ((starting_address == 15 || starting_address == 25) && registers_length == 2) {
        logger.printfln("Tried to set password (ignored)");
    } else if (starting_address == 11 && registers_length == 2) {
        const float float_val = convert_regs_to_float(registers);
        logger.printfln("Setting system type to %f", static_cast<double>(float_val));
        system_type = float_val;
    } else {
        logger.printfln("Received unexpected write_multiple_registers request to address %lu with length %hu", starting_address, registers_length);
    }
    tf_rs485_modbus_slave_answer_write_multiple_registers_request(&bricklet, request_id);
}

void ModbusMeterSimulator::modbus_slave_read_holding_registers_request_handler(uint8_t request_id, uint32_t starting_address, uint16_t count)
{
    uint16_t regs[2];

    if (starting_address == 11 && count == 2) {
        //logger.printfln("Read system type");
        convert_float_to_regs(regs, system_type);
    } else if (starting_address == 64515 && count == 1) {
        logger.printfln("Master reading meter ID");
        regs[0] = meter_id;
    } else {
        logger.printfln("Received unexpected read_holding_registers request to address %lu with length %hu", starting_address, count);
        return;
    }

    int rc = tf_rs485_modbus_slave_answer_read_holding_registers_request(&bricklet, request_id, regs, count);
    if (rc != TF_E_OK)
        logger.printfln("Answering read holding registers request failed with code %i", rc);
}

void ModbusMeterSimulator::modbus_slave_read_input_registers_request_handler(uint8_t request_id, uint32_t starting_address, uint16_t count)
{
    uint16_t regs[125];

    if (count > std::size(regs)) {
        logger.printfln("Received unexpected read_input_registers request to address %lu with length %hu", starting_address, count);
        return;
    }

    bool at_least_one_fresh = false;

    for (uint32_t offset = 0; offset < count; offset += 2) {
        uint32_t cached_index = register_address2cached_index(starting_address + offset);
        float float_val;
        MeterValueAvailability avl = meters.get_value_by_index(source_meter_slot, cached_index, &float_val, 2500_ms);
        if (avl == MeterValueAvailability::Fresh) {
            at_least_one_fresh = true;
        } else {
            float_val = NAN;
        }
        convert_float_to_regs(regs + offset, float_val);
    }

    if (at_least_one_fresh) {
        int rc = tf_rs485_modbus_slave_answer_read_input_registers_request(&bricklet, request_id, regs, count);
        if (rc != TF_E_OK) {
            logger.printfln("Answering read input registers request failed with code %i", rc);
        }
    } else if (have_source_values) {
        const uint32_t mask_byte_position = starting_address / 8;
        const uint8_t mask = static_cast<uint8_t>(1UL << (starting_address % 8));

        uint8_t mask_byte = unsupported_input_register_cache[mask_byte_position];

        if ((mask_byte & mask) == 0) {
            logger.printfln("No data for read input registers request %hu @ %lu", count, starting_address);
            mask_byte |= mask;
            unsupported_input_register_cache[mask_byte_position] = mask_byte;
        }
    }
}
