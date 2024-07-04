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

#include "hidden_proxy.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "event_log_prefix.h"
#include "module_dependencies.h"
#include "bindings/hal_common.h"

extern TF_HAL hal;
extern int8_t green_led_pin;
static TaskHandle_t xTaskBuffer;

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

static void blinky(void *arg)
{
    for (;;) {
        digitalWrite(green_led_pin, true);
        delay(200);
        digitalWrite(green_led_pin, false);
        delay(200);
    }
}

static bool blinky_running = false;

void HiddenProxy::start_proxy()
{
    if (net != nullptr) {
        logger.printfln("TF_Net already allocated?");
    } else {
        net = (TF_Net *)malloc(sizeof(TF_Net));
        if (net == nullptr) {
            logger.printfln("Failed to allocate TF_Net");
            return;
        }
    }

    if (blinky_running)
        return;
    blinky_running = true;

    if (green_led_pin >= 0)
        xTaskCreate(blinky,
            "proxy_blink",
            2048,
            nullptr,
            tskIDLE_PRIORITY,
            &xTaskBuffer);

    tf_hal_set_net(&hal, nullptr);
    tf_net_create(net, nullptr, 0, nullptr);
    tf_hal_set_net(&hal, net);
}

void HiddenProxy::stop_proxy()
{
    if (!blinky_running)
        return;
    blinky_running = false;

    if (green_led_pin >= 0)
        vTaskDelete(xTaskBuffer);

    tf_hal_set_net(&hal, nullptr);
    tf_net_destroy(net);
    free(net);
    net = nullptr;
}

void HiddenProxy::register_urls()
{
    server.on("/hidden_proxy/enable", HTTP_GET, [this](WebServerRequest request) {
        start_proxy();
        return request.send(200);
    });

    server.on("/hidden_proxy/disable", HTTP_GET, [this](WebServerRequest request) {
        stop_proxy();
        return request.send(200);
    });
}
