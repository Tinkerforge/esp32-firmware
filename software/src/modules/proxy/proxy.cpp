/* esp32-firmware
 * Copyright (C) 2020-2021 Erik Fleckstein <erik@tinkerforge.com>
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

#include "proxy.h"

#include "bindings/display_names.h"
#include "bindings/errors.h"
#include "bindings/hal_common.h"

#include "task_scheduler.h"

#include "api.h"

#include "web_server.h"

extern TF_HAL hal;
extern WebServer server;

extern TaskScheduler task_scheduler;

extern API api;

Proxy::Proxy()
{
    devices = Config::Array(
        {},
        new Config{Config::Object({
            {"uid", Config::Str("", 0, 7)},
            {"port", Config::Str("", 0, 1)},
            {"name", Config::Str("", 0, 50)},
        })},
        0, 12, Config::type_id<Config::ConfObject>()
    );

    error_counters = Config::Object({
        {"A", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        },
        {"B", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        },
        {"C", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        },
        {"D", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        },
        {"E", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        },
        {"F", Config::Object({
                {"SpiTfpChecksum", Config::Uint32(0)},
                {"SpiTfpFrame", Config::Uint32(0)},
                {"TfpFrame", Config::Uint32(0)},
                {"TfpUnexpected", Config::Uint32(0)},
            })
        }
    });
}

void Proxy::setup()
{
    tf_net_create(&net, NULL, 0, NULL);
    tf_hal_set_net(&hal, &net);

    uint16_t i = 0;
    char uid[7] = {0};
    char port_name;
    uint16_t device_id;

    while (tf_hal_get_device_info(&hal, i, uid, &port_name, &device_id) == TF_E_OK) {
        devices.add();

        devices.get(devices.count() - 1)->get("uid")->updateString(String(uid));
        devices.get(devices.count() - 1)->get("port")->updateString(String(port_name));
        devices.get(devices.count() - 1)->get("name")->updateString(String(tf_get_device_display_name(device_id)));
        ++i;
    }

    initialized = true;
}

void Proxy::register_urls()
{
    api.addState("proxy/error_counters", &error_counters, {}, 1000);
    api.addState("proxy/devices", &devices, {}, 10000);

    task_scheduler.scheduleWithFixedDelay([this](){
        for(char c = 'A'; c <= 'F'; ++c) {
            uint32_t spitfp_checksum, spitfp_frame, tfp_frame, tfp_unexpected;

            tf_hal_get_error_counters(&hal, c, &spitfp_checksum, &spitfp_frame, &tfp_frame, &tfp_unexpected);

            error_counters.get(String(c))->get("SpiTfpChecksum")->updateUint(spitfp_checksum);
            error_counters.get(String(c))->get("SpiTfpFrame")->updateUint(spitfp_frame);
            error_counters.get(String(c))->get("TfpFrame")->updateUint(tfp_frame);
            error_counters.get(String(c))->get("TfpUnexpected")->updateUint(tfp_unexpected);
        }
    }, 5000, 5000);
}

void Proxy::loop()
{
}
