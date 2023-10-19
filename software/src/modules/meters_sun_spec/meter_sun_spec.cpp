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

static const uint16_t discovery_base_addresses[] {
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
        this->access_in_progress = true;
        this->start_connection();
    }, 1000);

    task_scheduler.scheduleWithFixedDelay([this]() {
        if (!this->access_in_progress) {
            this->access_in_progress = true;
            this->start_generic_read();
        };
    }, 2000, 1000);
}

void MeterSunSpec::connect_callback()
{
    discovery_start();
}

void MeterSunSpec::read_start(size_t model_start_address, size_t model_regcount)
{
    free(generic_read_request.data[0]);
    free(generic_read_request.data[1]);

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

    generic_read_request.done_callback_arg = this;
    generic_read_request.done_callback = [](void *arg) {
        MeterSunSpec *mss = static_cast<MeterSunSpec *>(arg);
        mss->read_done_callback();
    };

    start_generic_read();
}

void MeterSunSpec::read_done_callback()
{
    access_in_progress = false;
    int16_t voltA = static_cast<int16_t>(generic_read_request.data[0][8]);
    logger.printfln("read_done_cb called voltA=%i", voltA);
}

void MeterSunSpec::discovery_restart()
{
    task_scheduler.scheduleOnce([this](){
        this->discovery_start();
    }, 10000);
}

void MeterSunSpec::discovery_start()
{
    free(generic_read_request.data[0]);
    free(generic_read_request.data[1]);

    generic_read_request.data[0] = nullptr;
    generic_read_request.data[1] = nullptr;

    uint16_t *buffer = static_cast<uint16_t *>(malloc(sizeof(uint16_t) * 68));
    if (!buffer) {
        logger.printfln("meter_sun_spec: Cannot alloc read buffer.");
        return;
    }

    discovery_base_address_index = 0;
    discovery_state = DiscoveryState::Idle;
    discovery_state_next = DiscoveryState::ReadSunSpecID;
    discovery_deserializer.buf = buffer;

    generic_read_request.register_type = TAddress::RegType::HREG;
    generic_read_request.start_address = discovery_base_addresses[discovery_base_address_index];
    generic_read_request.register_count = 2;
    generic_read_request.data[0] = buffer;
    generic_read_request.read_twice = false;

    generic_read_request.done_callback_arg = this;
    generic_read_request.done_callback = [](void *arg) {
        MeterSunSpec *mss = static_cast<MeterSunSpec *>(arg);
        mss->discovery_deserializer.idx = 0;
        mss->discovery_state = mss->discovery_state_next;
        mss->discovery_next();
    };

    start_generic_read();
}

void MeterSunSpec::discovery_next()
{
    if (generic_read_request.result_code != Modbus::ResultCode::EX_SUCCESS) {
        logger.printfln("meter_sun_spec: Modbus read error: %s (%d)", get_modbus_result_code_name(generic_read_request.result_code), generic_read_request.result_code);
        discovery_restart();
        return;
    }

    switch (discovery_state) {
        case DiscoveryState::Idle:
            break;

        case DiscoveryState::ReadSunSpecID: {
                uint32_t sun_spec_id = discovery_deserializer.read_uint32();

                if (sun_spec_id == SUN_SPEC_ID) {
                    generic_read_request.start_address += generic_read_request.register_count;
                    generic_read_request.register_count = 2;
                    discovery_state_next = DiscoveryState::ReadCommonModelHeader;

                    start_generic_read();
                }
                else {
                    ++discovery_base_address_index;

                    if (discovery_base_address_index >= ARRAY_SIZE(discovery_base_addresses)) {
                        logger.printfln("meter_sun_spec: No SunSpec device found");
                        discovery_restart();
                    }
                    else {
                        generic_read_request.start_address = discovery_base_addresses[discovery_base_address_index];
                        generic_read_request.register_count = 2;
                        discovery_state_next = DiscoveryState::ReadSunSpecID;

                        start_generic_read();
                    }
                }
            }

            break;

        case DiscoveryState::ReadCommonModelHeader: {
                uint16_t common_model_id = discovery_deserializer.read_uint16();
                size_t block_length = discovery_deserializer.read_uint16();

                if (common_model_id == COMMON_MODEL_ID && (block_length == 65 || block_length == 66)) {
                    generic_read_request.start_address += generic_read_request.register_count;
                    generic_read_request.register_count = block_length;
                    discovery_state_next = DiscoveryState::ReadCommonModelBlock;

                    start_generic_read();
                }
                else {
                    logger.printfln("meter_sun_spec: No SunSpec Common Model found");
                    discovery_restart();
                }
            }

            break;

        case DiscoveryState::ReadCommonModelBlock: {
                char manufacturer_name[32 + 1];
                char model_name[32 + 1];
                char options[16 + 1];
                char version[16 + 1];
                char serial_number[32 + 1];
                uint16_t device_address_;

                discovery_deserializer.read_string(manufacturer_name, sizeof(manufacturer_name));
                discovery_deserializer.read_string(model_name, sizeof(model_name));
                discovery_deserializer.read_string(options, sizeof(options));
                discovery_deserializer.read_string(version, sizeof(version));
                discovery_deserializer.read_string(serial_number, sizeof(serial_number));
                device_address_ = discovery_deserializer.read_uint16();

                logger.printfln("meter_sun_spec: Found Common Model:\n"
                                "  Manufacturer Name: %s\n"
                                "  Model Name: %s\n"
                                "  Options: %s\n"
                                "  Version: %s\n"
                                "  Serial Number: %s\n"
                                "  Device Address: %u",
                                manufacturer_name,
                                model_name,
                                options,
                                version,
                                serial_number,
                                device_address_);

                generic_read_request.start_address += generic_read_request.register_count;
                generic_read_request.register_count = 2;
                discovery_state_next = DiscoveryState::ReadStandardModelHeader;

                start_generic_read();
            }

            break;

        case DiscoveryState::ReadStandardModelHeader: {
                uint16_t standard_model_id = discovery_deserializer.read_uint16();
                size_t block_length = discovery_deserializer.read_uint16();

                if (standard_model_id == NON_IMPLEMENTED_UINT16 && block_length == 0) {
                    logger.printfln("meter_sun_spec: Configured SunSpec Standard Model not found");
                    discovery_restart();
                }
                else if (standard_model_id == model_id) {
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
