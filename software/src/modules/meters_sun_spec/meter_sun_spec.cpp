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
//#include "modules/meters/meter_value_id.h"
#include "task_scheduler.h"
//#include "tools.h"

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

void MeterSunSpec::setup()
{
    host_name      = config->get("host")->asString();
    port           = static_cast<uint16_t>(config->get("port")->asUint());
    device_address = static_cast<uint8_t>(config->get("device_address")->asUint());
    model_id       = static_cast<uint16_t>(config->get("model_id")->asUint());

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

    uint16_t *buffer = static_cast<uint16_t *>(malloc(sizeof(uint16_t) * model_regcount * 2));
    if (!buffer) {
        logger.printfln("meter_sun_spec: Cannot alloc read buffer.");
        return;
    }

    generic_read_request.register_type = TAddress::RegType::HREG;
    generic_read_request.start_address = model_start_address;
    generic_read_request.register_count = model_regcount;

    generic_read_request.data[0] = buffer;
    generic_read_request.data[1] = buffer + model_regcount;
    generic_read_request.read_twice = true;
    generic_read_request.done_callback = [this]{ read_done_callback(); };

    start_generic_read();
}

void MeterSunSpec::read_done_callback()
{
    read_allowed = true;

    if (generic_read_request.result_code == Modbus::ResultCode::EX_SUCCESS) {
        int16_t voltA = static_cast<int16_t>(generic_read_request.data[0][8]);
        logger.printfln("read_done_cb called voltA=%i", voltA);
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
                uint16_t model_id_ = scan_deserializer.read_uint16();
                size_t block_length = scan_deserializer.read_uint16();

                if (model_id_ == NON_IMPLEMENTED_UINT16 && block_length == 0) {
                    logger.printfln("meter_sun_spec: Configured SunSpec model %u not found at %s:%u:%u", model_id, host_name.c_str(), port, device_address);
                    scan_start_delay();
                }
                else if (model_id_ == model_id) {
                    logger.printfln("meter_sun_spec: Configured SunSpec model %u found at %s:%u:%u", model_id, host_name.c_str(), port, device_address);
                    read_start(generic_read_request.start_address, 2 + block_length);
                }
                else {
                    generic_read_request.start_address += generic_read_request.register_count + block_length;
                    generic_read_request.register_count = 2;

                    start_generic_read();
                }
            }

            break;

        default:
            esp_system_abort("meter_sun_spec: Invalid state.");
    }
}
