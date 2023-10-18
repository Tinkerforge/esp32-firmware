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

MeterClassID MeterSunSpec::get_class() const
{
    return MeterClassID::SunSpec;
}

void MeterSunSpec::setup()
{
    host_name      = config->get("host")->asString();
    port           = static_cast<uint16_t>(config->get("port")->asUint());
    device_address = static_cast<uint8_t>(config->get("device_address")->asUint());




    logger.printfln("starting tasks");

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
    // Perform re-discovery magic here. I tell you 203, you tell me 40069.
    uint16_t model_start_address = 40069;
    uint16_t model_regcount = 107; // TODO: Get from model info.

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
