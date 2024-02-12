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
#include "modules/meters_modbus_tcp/modbus_tcp_tools.h"
#include "module_dependencies.h"

#include "event_log.h"
#include "modules/meters_sun_spec/models/model_001.h"
#include "task_scheduler.h"

#include "gcc_warnings.h"

#define SUN_SPEC_ID 0x53756E53
#define COMMON_MODEL_ID 1
#define NON_IMPLEMENTED_UINT16 0xFFFF

static const uint16_t scan_base_addresses[] {
    40000,
    50000,
    0
};

MeterClassID MeterSunSpec::get_class() const
{
    return MeterClassID::SunSpec;
}

void MeterSunSpec::setup(const Config &ephemeral_config)
{
    host_name      = ephemeral_config.get("host")->asString();
    port           = static_cast<uint16_t>(ephemeral_config.get("port")->asUint());
    device_address = static_cast<uint8_t>(ephemeral_config.get("device_address")->asUint());
    model_id       = static_cast<uint16_t>(ephemeral_config.get("model_id")->asUint());

    model_parser = MetersSunSpecParser::new_parser(slot, model_id);
    if (!model_parser) {
        logger.printfln("meter_sun_spec: No parser available for model %u", model_id);
        return;
    }

    task_scheduler.scheduleOnce([this]() {
        this->read_allowed = false;
        this->start_connection();
    }, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (this->read_allowed) {
            this->read_allowed = false;
            this->start_generic_read();
        };
    }, 2000, 1000);
}

void MeterSunSpec::connect_callback()
{
    scan_start();
}

void MeterSunSpec::disconnect_callback()
{
    read_allowed = false;
}

void MeterSunSpec::read_start(size_t model_start_address, size_t model_regcount)
{
    free(generic_read_request.data[0]);

    generic_read_request.data[0] = nullptr;
    generic_read_request.data[1] = nullptr;

    bool read_twice = model_parser->must_read_twice();
    size_t buffer_regcount = read_twice ? model_regcount * 2 : model_regcount;

    uint16_t *buffer = static_cast<uint16_t *>(malloc(sizeof(uint16_t) * buffer_regcount));
    if (!buffer) {
        logger.printfln("meter_sun_spec: Cannot alloc read buffer.");
        return;
    }

    generic_read_request.register_type = TAddress::RegType::HREG;
    generic_read_request.start_address = model_start_address;
    generic_read_request.register_count = model_regcount;

    generic_read_request.data[0] = buffer;
    if (read_twice)
        generic_read_request.data[1] = buffer + model_regcount;
    generic_read_request.read_twice = read_twice;
    generic_read_request.done_callback = [this]{ read_done_callback(); };

    start_generic_read();
}

void MeterSunSpec::read_done_callback()
{
    read_allowed = true;

    if (generic_read_request.result_code != Modbus::ResultCode::EX_SUCCESS) {
        logger.printfln("meter_sun_spec: Read unsuccessful (%i)", generic_read_request.result_code);
        return;
    }

    if (!values_declared) {
        size_t registers_to_read = 0;
        if (!model_parser->detect_values(generic_read_request.data, quirks, &registers_to_read)) {
            logger.printfln("meter_sun_spec: Detecting values of model %u in slot %u failed.", model_id, slot);
            return;
        }
        values_declared = true;
        generic_read_request.register_count = registers_to_read;
    }

    if (!model_parser->parse_values(generic_read_request.data, quirks)) {
        logger.printfln("meter_sun_spec: Parsing model %u data in slot %u failed.", model_id, slot);
        // TODO: Read again if parsing failed?
    }
}

void MeterSunSpec::scan_start_delay()
{
    task_scheduler.scheduleOnce([this](){
        this->scan_start();
    }, 10000);
}

void MeterSunSpec::scan_start()
{
    free(generic_read_request.data[0]);

    generic_read_request.data[0] = nullptr;
    generic_read_request.data[1] = nullptr;

    // Buffer must be big enough for the Common model.
    uint16_t *buffer = static_cast<uint16_t *>(malloc(sizeof(uint16_t) * 68));
    if (!buffer) {
        logger.printfln("meter_sun_spec: Cannot alloc read buffer.");
        return;
    }

    scan_base_address_index = 0;
    scan_state = ScanState::Idle;
    scan_state_next = ScanState::ReadSunSpecID;
    scan_deserializer.buf = buffer;

    generic_read_request.register_type = TAddress::RegType::HREG;
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
    }, 1000 + (esp_random() % 4000));
}

void MeterSunSpec::scan_next()
{
    if (generic_read_request.result_code != Modbus::ResultCode::EX_SUCCESS) {
        logger.printfln("meter_sun_spec: Modbus read error during scan: %s (%d)", get_modbus_result_code_name(generic_read_request.result_code), generic_read_request.result_code);
        scan_read_delay();
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
                    scan_state_next = ScanState::ReadModelHeader;

                    start_generic_read();
                }
                else {
                    ++scan_base_address_index;

                    if (scan_base_address_index >= ARRAY_SIZE(scan_base_addresses)) {
                        logger.printfln("meter_sun_spec: No SunSpec device found at %s:%u:%u", host_name.c_str(), port, device_address);
                        scan_start_delay();
                    }
                    else {
                        generic_read_request.start_address = scan_base_addresses[scan_base_address_index];
                        generic_read_request.register_count = 2;
                        scan_state_next = ScanState::ReadSunSpecID;

                        start_generic_read();
                    }
                }
            }

            break;

        case ScanState::ReadModelHeader: {
                uint16_t scan_model_id = scan_deserializer.read_uint16();
                size_t block_length = scan_deserializer.read_uint16();

                if (scan_model_id == NON_IMPLEMENTED_UINT16 && block_length == 0) {
                    logger.printfln("meter_sun_spec: Configured SunSpec model %u not found at %s:%u:%u", model_id, host_name.c_str(), port, device_address);
                    scan_start_delay();
                }
                else if (scan_model_id == model_id) {
                    logger.printfln("meter_sun_spec: Configured SunSpec model %u found at %s:%u:%u:%u", model_id, host_name.c_str(), port, device_address, generic_read_request.start_address);
                    read_start(generic_read_request.start_address, 2 + block_length);
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
                    modbus_bswap_registers(common_model->registers + 2, 16); // 16 registers for only manufacturer name, 64 registers for everything
                    const SunSpecCommonModel001_s *m = &common_model->model;

                    //logger.printfln("meter_sun_spec: Device is %s %s %s %s %s", m->Mn, m->Md, m->Opt, m->Vr, m->SN);

                    if (strcmp(m->Mn, "KOSTAL") == 0) {
                        quirks |= SUN_SPEC_QUIRKS_ACC32_IS_INT32;
                    } else if (strcmp(m->Mn, "SMA") == 0) {
                        if (model_id >= 100 && model_id < 200) {
                            quirks |= SUN_SPEC_QUIRKS_INVERTER_CURRENT_IS_INT16;
                        }
                    }

                    if (quirks) {
                        logger.printfln("meter_sun_spec: Enabling quirks mode 0x%x for %s device.", quirks, m->Mn);
                    }
                }
                else {
                    logger.printfln("meter_sun_spec: Read full model %u for no reason.", scan_model_id);
                }

                generic_read_request.start_address += 2 + block_length;
                generic_read_request.register_count = 2;
                scan_state_next = ScanState::ReadModelHeader;

                start_generic_read();
            }

            break;

        default:
            esp_system_abort("meter_sun_spec: Invalid state.");
    }
}
